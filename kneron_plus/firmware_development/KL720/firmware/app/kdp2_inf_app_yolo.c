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
#include "kdp2_inf_app_yolo.h"

#include "model_type.h"

static kp_app_yolo_post_proc_config_t post_proc_params_v5s = {
    .prob_thresh = 0.15,
    .nms_thresh = 0.5,
    .max_detection_per_class = 100,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        10, 13, 16, 30, 33, 23,
        30, 61, 62, 45, 59, 119,
        116, 90, 156, 198, 373, 326,
        // strides[3]
        8, 16, 32,
    },
};

static kp_app_yolo_post_proc_config_t post_proc_params_v5m = {
    .prob_thresh = 0.3,
    .nms_thresh = 0.45,
    .max_detection_per_class = 100,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        10, 13, 16, 30, 33, 23,
        30, 61, 62, 45, 59, 119,
        116, 90, 156, 198, 373, 326,
        // strides[3]
        8, 16, 32,
    },
};

static kp_app_yolo_post_proc_config_t post_proc_params_v3_tiny_416 = {
    .prob_thresh = 0.6,
    .nms_thresh = 0.45,
    .max_detection_per_class = 100,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        81, 82, 135, 169, 344, 319,
        23, 27, 37, 58, 81, 82,
        0, 0, 0, 0, 0, 0, // -> not used in tiny yolo v3 post-proc
        // strides[3] -> not used in tiny yolo v3 post-proc
        8, 16, 32,
    },
};

static kp_app_yolo_post_proc_config_t post_proc_params_v3_416 = {
    .prob_thresh = 0.6,
    .nms_thresh = 0.45,
    .max_detection_per_class = 100,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        117, 125, 138, 284, 303, 309,
        56, 70, 32, 128, 62, 213,
        4, 9, 13, 24, 24, 50, // -> not used in tiny yolo v3 post-proc
        // strides[3] -> not used in tiny yolo v3 post-proc
        8, 16, 32,
    },
};

typedef struct
{
    int model_id;
    int param_size;
    void *post_proc_params;
} map_model_post_proc_t;

#define MAX_MODEL_PAIRS 4
static map_model_post_proc_t model_pp[MAX_MODEL_PAIRS] = {0}; // 4 pairs of modle-post_proc enough ?

static map_model_post_proc_t get_model_post_proc_param(int model_id)
{
    map_model_post_proc_t mapping = {0};

    // looking for model's post-proc params, if none apply some defaults
    for (int i = 0; i < MAX_MODEL_PAIRS; i++)
    {
        if (model_pp[i].model_id == model_id)
        {
            // found matched model id with post-proc params
            mapping = model_pp[i];
            break;
        }
        else if (model_pp[i].model_id == 0)
        {
            // register some default settings
            model_pp[i].model_id = model_id;
            switch (model_id)
            {
            case KNERON_YOLOV5S_COCO80_640_640_3:
                model_pp[i].param_size = sizeof(post_proc_params_v5s);
                model_pp[i].post_proc_params = (void *)&post_proc_params_v5s;
                break;
            case KNERON_YOLOV5m_COCO80_640_640_3:
            case KNERON_PERSONDETECTION_YOLOV5s_480_256_3:
            case KNERON_PERSONDETECTION_YOLOV5sParklot_480_256_3:
                model_pp[i].param_size = sizeof(post_proc_params_v5m);
                model_pp[i].post_proc_params = (void *)&post_proc_params_v5m;
                break;
            case TINY_YOLO_V3_416_416_3:
                model_pp[i].param_size = sizeof(post_proc_params_v3_tiny_416);
                model_pp[i].post_proc_params = (void *)&post_proc_params_v3_tiny_416;
                break;
            case YOLO_V3_416_416_3:
                model_pp[i].param_size = sizeof(post_proc_params_v3_416);
                model_pp[i].post_proc_params = (void *)&post_proc_params_v3_416;
                break;
            default:
                // cannot find matched post-proc config
                break;
            }

            mapping = model_pp[i];
            break;
        }
    }

    return mapping;
}

void kdp2_app_yolo_config_post_process_parameters(int job_id, int num_input_buf, void **inf_input_buf_list)
{
    if (1 != num_input_buf) {
        kmdw_inference_app_send_status_code(job_id, KP_FW_WRONG_INPUT_BUFFER_COUNT_110);
        return;
    }

    kdp2_ipc_app_yolo_post_proc_config_t *yolo_pp_config = (kdp2_ipc_app_yolo_post_proc_config_t *)inf_input_buf_list[0];

    if (yolo_pp_config->set_or_get == 1)
    {
        // setting post-proc configs with specified model_id
        for (int i = 0; i < MAX_MODEL_PAIRS; i++)
        {
            if (model_pp[i].model_id == yolo_pp_config->model_id || model_pp[i].model_id == 0)
            {
                model_pp[i].model_id = yolo_pp_config->model_id; // for model_pp[i].model_id == 0
                if (model_pp[i].post_proc_params == NULL || yolo_pp_config->param_size > model_pp[i].param_size)
                {
                    model_pp[i].post_proc_params = malloc(yolo_pp_config->param_size);
                    if (model_pp[i].post_proc_params == NULL)
                    {
                        kmdw_printf("[app_yolo]: error ! no memory for malloc post-proc parameters\n");
                        kmdw_inference_app_send_status_code(job_id, KP_FW_CONFIG_POST_PROC_ERROR_MALLOC_FAILED_105);
                        return; // failed return
                    }

                    model_pp[i].param_size = yolo_pp_config->param_size;
                }

                memcpy(model_pp[i].post_proc_params, (void *)yolo_pp_config->param_data, yolo_pp_config->param_size);
                kmdw_inference_app_send_status_code(job_id, KP_SUCCESS);
                return; // sucessful return
            }
        }

        kmdw_inference_app_send_status_code(job_id, KP_FW_CONFIG_POST_PROC_ERROR_NO_SPACE_106);
        return; // failed return
    }
    else
    {
        // getting post-proc configs with specified model_id
        // get a result buffer to save pp parameters
        int result_buf_size;
        kdp2_ipc_app_yolo_post_proc_config_t *return_pp_config = (kdp2_ipc_app_yolo_post_proc_config_t *)kmdw_fifoq_manager_result_get_free_buffer(&result_buf_size);

        map_model_post_proc_t mapping = get_model_post_proc_param(yolo_pp_config->model_id);

        return_pp_config->header_stamp = yolo_pp_config->header_stamp;
        return_pp_config->header_stamp.status_code = KP_SUCCESS;
        return_pp_config->header_stamp.total_size = sizeof(kdp2_ipc_app_yolo_post_proc_config_t);
        return_pp_config->set_or_get = 0;
        return_pp_config->model_id = yolo_pp_config->model_id;
        return_pp_config->param_size = mapping.param_size;
        if (mapping.param_size > 0)
            memcpy((void *)return_pp_config->param_data, mapping.post_proc_params, mapping.param_size);

        // send pp params back to host SW
        kmdw_fifoq_manager_result_enqueue((void *)return_pp_config, result_buf_size, false);
    }
}

void kdp2_app_yolo_result_callback(int status, void *inf_result_buf, int inf_result_buf_size, void *ncpu_result_buf)
{
    // when ncpu has done post-process then it comes to here with inference result buffer filled
    // 'inf_result_buf' is used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)
    // 'ncpu_result_buf' is post-processing result buffer done by ncpu
    // normally 'inf_result_buf' contains 'ncpu_result_buf', and user should send 'inf_result_buf' to host SW

    kdp2_ipc_app_yolo_result_t *app_yolo_result = (kdp2_ipc_app_yolo_result_t *)inf_result_buf;
    kp_app_yolo_result_t *yolo_data = (kp_app_yolo_result_t *)ncpu_result_buf;

    if (status != KP_SUCCESS)
    {
        app_yolo_result->header_stamp.status_code = status;
        app_yolo_result->header_stamp.total_size = sizeof(kdp2_ipc_app_yolo_result_t) - sizeof(kp_app_yolo_result_t);
        // send error status result back to host
        kmdw_fifoq_manager_result_enqueue((void *)inf_result_buf, inf_result_buf_size, false);
        return;
    }

    if (yolo_data->box_count > YOLO_GOOD_BOX_MAX)
    {
        kmdw_printf("[app_yolo]: error ! too many bounding boxes = %d!!!\n", yolo_data->box_count);
        yolo_data->box_count = YOLO_GOOD_BOX_MAX;
    }

    app_yolo_result->header_stamp.status_code = KP_SUCCESS;
    app_yolo_result->header_stamp.total_size = sizeof(kdp2_ipc_app_yolo_result_t) - sizeof(kp_app_yolo_result_t) +
                                               sizeof(yolo_data->class_count) + sizeof(yolo_data->box_count) + yolo_data->box_count * sizeof(kp_bounding_box_t);

    // send output result buffer back to host SW
    kmdw_fifoq_manager_result_enqueue((void *)inf_result_buf, inf_result_buf_size, false);
}

void kdp2_app_yolo_inference(int job_id, int num_input_buf, void **inf_input_buf_list)
{
    // 'inf_input_buf' and 'inf_result_buf' are provided by kdp2 middleware
    // the content of 'inf_input_buf' is transmitted from host SW = header + image
    // 'inf_result_buf' is used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)

    // now get an available free result buffer
    // normally the begin part of result buffer should contain app-defined result header
    // and the rest is for ncpu/npu inference output data
    int result_buf_size;
    void *inf_result_buf = kmdw_fifoq_manager_result_get_free_buffer(&result_buf_size);

    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    kdp2_ipc_app_yolo_inf_header_t *app_yolo_header;
    kdp2_ipc_app_yolo_result_t *app_yolo_result = (kdp2_ipc_app_yolo_result_t *)inf_result_buf;

    inf_config.num_image = num_input_buf;

    for (int i = 0; i < num_input_buf; i++) {
        app_yolo_header = (kdp2_ipc_app_yolo_inf_header_t *)inf_input_buf_list[i];

        // image buffer address should be just after the header
        inf_config.image_list[i].image_buf = (void *)((uint32_t)app_yolo_header + sizeof(kdp2_ipc_app_yolo_inf_header_t));
        inf_config.image_list[i].image_width = app_yolo_header->width;
        inf_config.image_list[i].image_height = app_yolo_header->height;
        inf_config.image_list[i].image_channel = app_yolo_header->channel;
        inf_config.image_list[i].image_format = app_yolo_header->image_format;
        inf_config.image_list[i].image_norm = app_yolo_header->model_normalize;
        inf_config.image_list[i].image_resize = KP_RESIZE_ENABLE;                         // enable resize
        inf_config.image_list[i].image_padding = KP_PADDING_CORNER;                       // enable padding on corner
        inf_config.image_list[i].pad_value = NULL;
    }

    inf_config.model_id = app_yolo_header->model_id;
    inf_config.enable_parallel = true;                                      // only works for single model and post-process in ncpu
    inf_config.inf_result_buf = inf_result_buf;                             // for callback
    inf_config.inf_result_buf_size = result_buf_size;
    inf_config.ncpu_result_buf = (void *)&(app_yolo_result->yolo_data);     // give result buffer for ncpu/npu, callback will carry it
    inf_config.result_callback = kdp2_app_yolo_result_callback;

    map_model_post_proc_t mapping = get_model_post_proc_param(inf_config.model_id);
    inf_config.user_define_data = mapping.post_proc_params; // FIXME: if NULL what happen ?

    // pre-set something for result output
    // header_stamp is a must to correctly transfer result data back to host SW
    app_yolo_result->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    app_yolo_result->header_stamp.total_size = 0;
    app_yolo_result->header_stamp.job_id = job_id;
    app_yolo_result->inf_number = ((kdp2_ipc_app_yolo_inf_header_t *)inf_input_buf_list[0])->inf_number; // sync the inference number

    // run preprocessing and inference, trigger ncpu/npu to do the work
    // if enable_parallel=true (works only for single model), result callback is needed
    // however if inference error then no callback will be invoked
    int ret = kmdw_inference_app_execute(&inf_config);
    if (ret != KP_SUCCESS)
    {
        // some sort of inference error
        app_yolo_result->header_stamp.status_code = ret;
        app_yolo_result->header_stamp.total_size = sizeof(kdp2_ipc_app_yolo_result_t) - sizeof(kp_app_yolo_result_t);

        // send error status result back to host
        kmdw_fifoq_manager_result_enqueue((void *)inf_result_buf, result_buf_size, false);
    }
}
