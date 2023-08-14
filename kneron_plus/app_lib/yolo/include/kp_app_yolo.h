/**
 * @file        kp_app_yolo.h
 * @brief       APP yolo inference API
 *
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "kp_struct.h"

typedef struct
{
    uint32_t model_id;              // specify model id
    kp_normalize_mode_t model_norm; // specify model normalization
} __attribute__((aligned(4))) kp_app_yolo_config_t;

/**
 * @brief describe a yolo post-process configurations for yolo series
 */
typedef struct
{
    float prob_thresh;                /**< probability thresh */
    float nms_thresh;                 /**< NMS thresh */
    uint32_t max_detection_per_class; /**< MAX number of detection per class */
    uint16_t anchor_row;              /**< number of rows for anchor */
    uint16_t anchor_col;              /**< number of cols for anchor */
    uint16_t stride_size;             /**< stride size */
    uint16_t reserved_size;           /**< set it to 0 for now */
    uint32_t data[40];                /**< anchor and stride array */
} __attribute__((aligned(4))) kp_app_yolo_post_proc_config_t;
/*
example:
{
    .prob_thresh = 0.15,
    .nms_thresh = 0.5,
    .max_detection_per_class = 20,
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
    }
}
*/


#define YOLO_GOOD_BOX_MAX 100 /**< maximum number of bounding boxes for Yolo models */

/**
 * @brief describe a yolo output result after post-processing
 */
typedef struct
{
    uint8_t padding[APP_PADDING_BYTES];         /**< padding data for communication, useless for users */
    uint32_t class_count;                       /**< total class count detectable by model */
    uint32_t box_count;                         /**< boxes of all classes */
    kp_bounding_box_t boxes[YOLO_GOOD_BOX_MAX]; /**< box information */
} __attribute__((aligned(4))) kp_app_yolo_result_t;

/**
 * @brief get yolo series post-process parameters
 *
 * @param devices a set of devices handle.
 * @param model_id model ID to apply to.
 * @param pp_params yolo post-process parameters, refer to kp_app_yolo_post_proc_config_t.
 *
 * @return refer to KP_API_RETURN_CODE.
 */
int kp_app_yolo_get_post_proc_parameters(kp_device_group_t devices, int model_id, kp_app_yolo_post_proc_config_t *pp_params);

/**
 * @brief set yolo series post-process parameters
 *
 * @param devices a set of devices handle.
 * @param model_id model ID to apply to.
 * @param pp_params yolo post-process parameters, refer to kp_app_yolo_post_proc_config_t.
 *
 * @return refer to KP_API_RETURN_CODE.
 */
int kp_app_yolo_set_post_proc_parameters(kp_device_group_t devices, int model_id, kp_app_yolo_post_proc_config_t *pp_params);

/**
 * @brief send image for yolo series inference
 *
 * @param devices a set of devices handle.
 * @param inference_number  inference sequence number used to sync result receive function.
 * @param image_buffer image buffer.
 * @param width image width.
 * @param height image height.
 * @param format image format, refer to kp_image_format_t.
 * @param misc_config miscellaneous configurations, if = NULL using default settings (auto find model_id and use KP_NORMALIZE_KNERON).
 *
 * @return refer to KP_API_RETURN_CODE.
 */
int kp_app_yolo_inference_send(kp_device_group_t devices, uint32_t inference_number, uint8_t *image_buffer,
                               uint32_t width, uint32_t height, kp_image_format_t format, kp_app_yolo_config_t *misc_config);

/**
 * @brief receive inference result of yolo series inference
 *
 * @param devices a set of devices handle.
 * @param inference_number  a return value, inference sequence number used to sync with image send function.
 * @param yolo_result output buffer, refer to kp_app_yolo_result_t
 *
 * @return refer to KP_API_RETURN_CODE.
 */
int kp_app_yolo_inference_receive(kp_device_group_t devices, uint32_t *inference_number, kp_app_yolo_result_t *yolo_result);
