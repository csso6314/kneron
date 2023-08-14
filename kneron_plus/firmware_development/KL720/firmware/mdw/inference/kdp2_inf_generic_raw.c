/*
 * Kneron Application general functions
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kmdw_console.h"

#include "kmdw_inference_app.h"
#include "kmdw_fifoq_manager.h"
#include "kdp2_inf_generic_raw.h"

uint32_t kdp2_get_raw_output_info_size(void)
{
    return sizeof(kdp2_ipc_generic_raw_result_t) + sizeof(_720_raw_cnn_res_t);
}

void kdp2_generic_raw_inference(int num_input_buf, void **inf_input_buf_list)
{
    // 'inf_input_buf' and 'result_buf' are provided by kdp2 middleware
    // the content of 'inf_input_buf' is transmitted from host SW = header + image
    // 'result_buf' is used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)

    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    kdp2_ipc_generic_raw_inf_header_t *input_header = (kdp2_ipc_generic_raw_inf_header_t *)inf_input_buf_list[0];
    int crop_count = input_header->image_header.crop_count;

    inf_config.num_image = num_input_buf;

    kp_pad_value_t pad_value[num_input_buf];

    for (int i = 0; i < num_input_buf; i++) {
        input_header = (kdp2_ipc_generic_raw_inf_header_t *)inf_input_buf_list[i];

        // image buffer address should be just after the header
        inf_config.image_list[i].image_buf = (void *)((uint32_t)input_header + sizeof(kdp2_ipc_generic_raw_inf_header_t));
        inf_config.image_list[i].image_width = input_header->image_header.width;
        inf_config.image_list[i].image_height = input_header->image_header.height;
        inf_config.image_list[i].image_channel = (input_header->image_header.image_format == KP_IMAGE_FORMAT_RAW8) ? 1 : 3;
        inf_config.image_list[i].image_format = input_header->image_header.image_format;
        inf_config.image_list[i].image_norm = input_header->image_header.normalize_mode;
        inf_config.image_list[i].enable_crop = (0 < input_header->image_header.crop_count);      // use crop
        inf_config.image_list[i].image_resize = input_header->image_header.resize_mode;          // user's choice
        inf_config.image_list[i].image_padding = input_header->image_header.padding_mode;        // user's choice

        memset(&pad_value[i], 0, sizeof(kp_pad_value_t));
        inf_config.image_list[i].pad_value = &pad_value[i];
    }

    inf_config.model_id = input_header->model_id;
    inf_config.enable_raw_output = true;            // raw output no post-processing
    inf_config.result_callback = NULL;
    inf_config.user_define_data = NULL;

    // need to know model raw output size for result transfer size
    uint32_t model_raw_out_size = kmdw_inference_app_get_model_raw_output_size(inf_config.model_id);

    if (0 == crop_count) {
        int output_header_buf_size;
        void *result_buf = kmdw_fifoq_manager_result_get_free_buffer(&output_header_buf_size);
        void *ncpu_result_buf = (void *)((uint32_t)result_buf + sizeof(kdp2_ipc_generic_raw_result_t));

        inf_config.ncpu_result_buf = ncpu_result_buf;   // give result buffer for ncpu/npu

        // run preprocessing and inference, trigger ncpu/npu to do the work
        int ret = kmdw_inference_app_execute(&inf_config);

        kdp2_ipc_generic_raw_result_t *output_header = (kdp2_ipc_generic_raw_result_t *)result_buf;

        // header_stamp is a must to correctly transfer result data back to host SW
        output_header->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
        output_header->header_stamp.job_id = KDP2_INF_ID_GENERIC_RAW;
        output_header->product_id = KP_DEVICE_KL720;
        output_header->inf_number = input_header->inference_number;         // sync the inference number
        output_header->crop_number = 0;                                     // sync the crop number
        output_header->is_last_crop = 1;
        output_header->num_of_pre_proc_info = num_input_buf;

        for (int i = 0; i < num_input_buf; i++) {
            uint32_t model_input_width = 0;
            uint32_t model_input_height = 0;

            kmdw_inference_get_model_input_image_size(inf_config.model_id, i, &model_input_width, &model_input_height);

            output_header->pre_proc_info[i].img_width = inf_config.image_list[i].image_width;
            output_header->pre_proc_info[i].img_height = inf_config.image_list[i].image_height;
            output_header->pre_proc_info[i].pad_top = inf_config.image_list[i].pad_value->pad_top;
            output_header->pre_proc_info[i].pad_bottom = inf_config.image_list[i].pad_value->pad_bottom;
            output_header->pre_proc_info[i].pad_left = inf_config.image_list[i].pad_value->pad_left;
            output_header->pre_proc_info[i].pad_right = inf_config.image_list[i].pad_value->pad_right;
            output_header->pre_proc_info[i].resized_img_width = model_input_width - inf_config.image_list[i].pad_value->pad_left - inf_config.image_list[i].pad_value->pad_right;
            output_header->pre_proc_info[i].resized_img_height = model_input_height - inf_config.image_list[i].pad_value->pad_top - inf_config.image_list[i].pad_value->pad_bottom;
            output_header->pre_proc_info[i].model_input_width = model_input_width;
            output_header->pre_proc_info[i].model_input_height = model_input_height;

            memset(&output_header->pre_proc_info[i].crop_area, 0, sizeof(kp_inf_crop_box_t));
        }

        if (ret == KP_SUCCESS) {
            output_header->header_stamp.status_code = KP_SUCCESS;
            output_header->header_stamp.total_size = sizeof(kdp2_ipc_generic_raw_result_t) + sizeof(_720_raw_cnn_res_t) + model_raw_out_size;
        } else {
            // some sort of inference error
            output_header->header_stamp.status_code = ret;
            output_header->header_stamp.total_size = sizeof(kdp2_ipc_generic_raw_result_t);
        }

        kmdw_fifoq_manager_result_enqueue((void *)output_header, output_header_buf_size, false);
    } else {
        // remember: one crop, one inference result !
        for (int c = 0; c < crop_count; c++)
        {
            // now get an available free result buffer
            // normally the begin part of result buffer should contain app-defined result header
            // and the rest is for ncpu/npu inference output data
            int output_header_buf_size;
            void *result_buf = kmdw_fifoq_manager_result_get_free_buffer(&output_header_buf_size);

            // leave some space for result header
            void *ncpu_result_buf = (void *)((uint32_t)result_buf + sizeof(kdp2_ipc_generic_raw_result_t));

            for (int i = 0; i < num_input_buf; i++) {
                input_header = (kdp2_ipc_generic_raw_inf_header_t *)inf_input_buf_list[i];
                inf_config.image_list[i].crop_area = input_header->image_header.inf_crop[c];

                memset(inf_config.image_list[i].pad_value, 0, sizeof(kp_pad_value_t));
            }

            inf_config.ncpu_result_buf = ncpu_result_buf; // give result buffer for ncpu/npu

            // run preprocessing and inference, trigger ncpu/npu to do the work
            int ret = kmdw_inference_app_execute(&inf_config);

            kdp2_ipc_generic_raw_result_t *output_header = (kdp2_ipc_generic_raw_result_t *)result_buf;

            // header_stamp is a must to correctly transfer result data back to host SW
            output_header->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
            output_header->header_stamp.job_id = KDP2_INF_ID_GENERIC_RAW;
            output_header->product_id = KP_DEVICE_KL720;
            output_header->inf_number = input_header->inference_number;                         // sync the inference number
            output_header->crop_number = input_header->image_header.inf_crop[c].crop_number;    // sync the crop number
            output_header->is_last_crop = (c == crop_count - 1) ? 1 : 0;
            output_header->num_of_pre_proc_info = num_input_buf;

            for (int i = 0; i < num_input_buf; i++) {
                uint32_t model_input_width = 0;
                uint32_t model_input_height = 0;

                kmdw_inference_get_model_input_image_size(inf_config.model_id, i, &model_input_width, &model_input_height);

                output_header->pre_proc_info[i].img_width = inf_config.image_list[i].crop_area.width;
                output_header->pre_proc_info[i].img_height = inf_config.image_list[i].crop_area.height;
                output_header->pre_proc_info[i].pad_top = inf_config.image_list[i].pad_value->pad_top;
                output_header->pre_proc_info[i].pad_bottom = inf_config.image_list[i].pad_value->pad_bottom;
                output_header->pre_proc_info[i].pad_left = inf_config.image_list[i].pad_value->pad_left;
                output_header->pre_proc_info[i].pad_right = inf_config.image_list[i].pad_value->pad_right;
                output_header->pre_proc_info[i].resized_img_width = model_input_width - inf_config.image_list[i].pad_value->pad_left - inf_config.image_list[i].pad_value->pad_right;
                output_header->pre_proc_info[i].resized_img_height = model_input_height - inf_config.image_list[i].pad_value->pad_top - inf_config.image_list[i].pad_value->pad_bottom;
                output_header->pre_proc_info[i].model_input_width = model_input_width;
                output_header->pre_proc_info[i].model_input_height = model_input_height;

                memcpy(&output_header->pre_proc_info[i].crop_area, &inf_config.image_list[i].crop_area, sizeof(kp_inf_crop_box_t));
            }

            if (ret == KP_SUCCESS) {
                output_header->header_stamp.status_code = KP_SUCCESS;
                output_header->header_stamp.total_size = sizeof(kdp2_ipc_generic_raw_result_t) + sizeof(_720_raw_cnn_res_t) + model_raw_out_size;
            } else {
                // some sort of inference error
                output_header->header_stamp.status_code = ret;
                output_header->header_stamp.total_size = sizeof(kdp2_ipc_generic_raw_result_t);
            }

            kmdw_fifoq_manager_result_enqueue((void *)output_header, output_header_buf_size, false);
        }
    }
}

void kdp2_generic_raw_inference_bypass_pre_proc(int num_input_buf, void **inf_input_buf_list)
{
    // 'inf_input_buf' and 'result_buf' are provided by kdp2 middleware
    // the content of 'inf_input_buf' is transmitted from host SW = header + image
    // 'result_buf' is used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)

    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    kdp2_ipc_generic_raw_inf_bypass_pre_proc_header_t *input_header;

    inf_config.num_image = num_input_buf;

    for (int i = 0; i < num_input_buf; i++) {
        input_header = (kdp2_ipc_generic_raw_inf_bypass_pre_proc_header_t *)inf_input_buf_list[i];

        // image buffer address should be just after the header
        inf_config.image_list[i].image_buf = (void *)((uint32_t)input_header + sizeof(kdp2_ipc_generic_raw_inf_bypass_pre_proc_header_t));
        inf_config.image_list[i].image_buf_size = input_header->image_buffer_size;
        inf_config.image_list[i].bypass_pre_proc = true;
        inf_config.image_list[i].pad_value = NULL;
    }

    // now get an available free result buffer
    // normally the begin part of result buffer should contain app-defined result header
    // and the rest is for ncpu/npu inference output data
    int output_header_buf_size;
    void *result_buf = kmdw_fifoq_manager_result_get_free_buffer(&output_header_buf_size);

    // leave some space for result header
    void *ncpu_result_buf = (void *)((uint32_t)result_buf + sizeof(kdp2_ipc_generic_raw_bypass_pre_proc_result_t));

    inf_config.model_id = input_header->model_id;
    inf_config.enable_raw_output = true;            // raw output no post-processing
    inf_config.ncpu_result_buf = ncpu_result_buf;   // give result buffer for ncpu/npu
    inf_config.result_callback = NULL;
    inf_config.user_define_data = NULL;

    // run preprocessing and inference, trigger ncpu/npu to do the work
    int ret = kmdw_inference_app_execute(&inf_config);

    kdp2_ipc_generic_raw_bypass_pre_proc_result_t *output_header = (kdp2_ipc_generic_raw_bypass_pre_proc_result_t *)result_buf;

    // need to know model raw output size for result transfer size
    uint32_t model_raw_out_size = kmdw_inference_app_get_model_raw_output_size(inf_config.model_id);

    // header_stamp is a must to correctly transfer result data back to host SW
    output_header->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    output_header->header_stamp.job_id = KDP2_INF_ID_GENERIC_RAW_BYPASS_PRE_PROC;
    output_header->product_id = KP_DEVICE_KL720;
    output_header->inf_number = input_header->inference_number;         // sync the inference number
    output_header->num_of_pre_proc_info = 0;
    output_header->crop_number = 0;
    output_header->is_last_crop = 1;

    if (ret == KP_SUCCESS) {
        output_header->header_stamp.status_code = KP_SUCCESS;
        output_header->header_stamp.total_size = sizeof(kdp2_ipc_generic_raw_bypass_pre_proc_result_t) + sizeof(_720_raw_cnn_res_t) + model_raw_out_size;
    } else {
        // some sort of inference error
        output_header->header_stamp.status_code = ret;
        output_header->header_stamp.total_size = sizeof(kdp2_ipc_generic_raw_bypass_pre_proc_result_t);
    }

    kmdw_fifoq_manager_result_enqueue((void *)output_header, output_header_buf_size, false);
}
