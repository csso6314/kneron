/**
 * @file        kp_app_yolo_ipc.h
 * @brief       App yolo ipc structure
 *
 * @version     0.1
 * @date        2021-05-06
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#include "kp_app_yolo.h"

/********** KDP2_INF_ID_APP_YOLO **********/

// post-proc config data struct shared for setting or getting
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t set_or_get; // get = 0, set = 1
    uint32_t model_id;
    uint32_t param_size;
    uint8_t param_data[200]; // contains kp_app_yolo_post_proc_config_*** body

} __attribute__((aligned(4))) kdp2_ipc_app_yolo_post_proc_config_t;

// input header for 'Kneron APP Yolo Inference'
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;

    uint32_t inf_number;
    uint32_t width;
    uint32_t height;
    uint32_t channel;
    uint32_t model_id;
    uint32_t image_format;    // kp_image_format_t
    uint32_t model_normalize; // kp_normalize_mode_t

} __attribute__((aligned(4))) kdp2_ipc_app_yolo_inf_header_t;

// result (header + data) for 'Kneron APP Yolo Inference'
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t inf_number;
    // above = padding[APP_PADDING_BYTES]
    kp_app_yolo_result_t yolo_data;

} __attribute__((aligned(4))) kdp2_ipc_app_yolo_result_t;
