/*
 * Kneron Application general functions
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "model_type.h"
#include "kmdw_console.h"

#include "kmdw_inference_app.h"
#include "kmdw_fifoq_manager.h"
#include "demo_customize_inf_single_model.h"

/**
 * @brief describe a yolo post-process configurations for yolo v5 series
 */
typedef struct
{
    float prob_thresh;
    float nms_thresh;
    uint32_t max_detection_per_class;
    uint16_t anchor_row;
    uint16_t anchor_col;
    uint16_t stride_size;
    uint16_t reserved_size;
    uint32_t data[40];
} __attribute__((aligned(4))) kp_app_yolo_post_proc_config_t;

static kp_app_yolo_post_proc_config_t post_proc_params_v3 = {
    .prob_thresh = 0.2,
    .nms_thresh = 0.45,
    .max_detection_per_class = YOLO_GOOD_BOX_MAX,
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

void demo_customize_inf_single_model(uint32_t job_id, int num_input_buf, void **inf_input_buf_list)
{
    if (1 != num_input_buf) {
        kmdw_inference_app_send_status_code(job_id, KP_FW_WRONG_INPUT_BUFFER_COUNT_110);
        return;
    }

    // 'inf_input_buf' and 'inf_result_buf' are provided by kdp2 middleware
    // the content of 'inf_input_buf' is transmitted from host SW = header + image
    // 'inf_result_buf' is used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)

    // now get an available free result buffer
    // normally the begin part of result buffer should contain app-defined result header
    // and the rest is for ncpu/npu inference output data
    int result_buf_size;
    void *inf_result_buf = kmdw_fifoq_manager_result_get_free_buffer(&result_buf_size);

    demo_customize_inf_single_model_header_t *input_header = (demo_customize_inf_single_model_header_t *)inf_input_buf_list[0];
    demo_customize_inf_single_model_result_t *output_result = (demo_customize_inf_single_model_result_t *)inf_result_buf;

    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    // image buffer address should be just after the header
    inf_config.num_image = 1;
    inf_config.image_list[0].image_buf = (void *)((uint32_t)input_header + sizeof(demo_customize_inf_single_model_header_t));
    inf_config.image_list[0].image_width = input_header->width;
    inf_config.image_list[0].image_height = input_header->height;
    inf_config.image_list[0].image_channel = 3;                                       // assume RGB565
    inf_config.image_list[0].image_format = KP_IMAGE_FORMAT_RGB565;                   // assume RGB565
    inf_config.image_list[0].image_norm = KP_NORMALIZE_KNERON;                        // this depends on model
    inf_config.image_list[0].image_resize = KP_RESIZE_ENABLE;                         // enable resize
    inf_config.image_list[0].image_padding = KP_PADDING_CORNER;                       // enable padding on corner
    inf_config.model_id = TINY_YOLO_V3_224_224_3;                       // this depends on model
    inf_config.ncpu_result_buf = (void *)&(output_result->yolo_result); // give result buffer for ncpu/npu, callback will carry it
    inf_config.user_define_data = (void *)&post_proc_params_v3;         // yolo post-process configurations for yolo v3 series

    // run preprocessing and inference, trigger ncpu/npu to do the work
    // if enable_parallel=true (works only for single model), result callback is needed
    // however if inference error then no callback will be invoked
    int inf_status = kmdw_inference_app_execute(&inf_config);

    // header_stamp is a must to correctly transfer result data back to host SW
    output_result->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    output_result->header_stamp.total_size = sizeof(demo_customize_inf_single_model_result_t);
    output_result->header_stamp.job_id = job_id;
    output_result->header_stamp.status_code = inf_status;

    // send output result buffer back to host SW
    kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
}
