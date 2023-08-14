#pragma once

/**
 * @file      kdp2_app.h
 * @brief     APP API
 *
 * The APP inference functions provide sophisticated functionaly for differnt applications.
 * 
 * Differnt set of APP inference APIs would need different models to make it work.
 *
 * @copyright (c) 2021 Kneron Inc. All right reserved.
 */

#include <stdint.h>

#include "kdp2_struct.h"

/**
 * @brief send image for tiny yolo v3 inference
 *
 * @param device a connected device handle.
 * @param inference_number  inference sequence number used to sync result receive function.
 * @param image_buffer image buffer.
 * @param width image width.
 * @param width image height.
 * @param format image format, refer to kdp2_image_format_t.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_app_tiny_yolo_v3_send(kdp2_device_t device, uint32_t inference_number, uint8_t *image_buffer,
                               uint32_t width, uint32_t height, kdp2_image_format_t format);

/**
 * @brief send image for tiny yolo v3 inference
 *
 * @param device a connected device handle.
 * @param inference_number  a return value, inference sequence number used to sync with image send function.
 * @param box_count a return value, indicating number of bounding boxes
 * @param boxes an input array of type kdp2_bounding_box_t.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_app_tiny_yolo_v3_receive(kdp2_device_t device, uint32_t *inference_number, uint32_t *box_count,
                                  kdp2_bounding_box_t boxes[]);
