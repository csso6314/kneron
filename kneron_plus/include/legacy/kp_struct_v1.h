/**
 * @file        kp_struct_v1.h
 * @brief       Legacy Kneron PLUS data structure
 *
 * **(To be deprecated in future release)**
 *
 * @version     0.1
 * @date        2022-05-23
 *
 * @copyright   Copyright (c) 2022 Kneron Inc. All rights reserved.
 */

#pragma once

#include <stdint.h>

#include "kp_struct.h"

/**
 * @brief inference descriptor for images
 */
typedef struct
{
    uint32_t inference_number;              /**< inference sequence number */
    uint32_t model_id;                      /**< target inference model ID */
    uint32_t width;                         /**< image width */
    uint32_t height;                        /**< image height */
    uint32_t resize_mode;                   /**< resize mode, refer to kp_resize_mode_t */
    uint32_t padding_mode;                  /**< padding mode, refer to kp_resize_mode_t */
    uint32_t image_format;                  /**< image format, refer to kp_image_format_t */
    uint32_t normalize_mode;                /**< inference normalization, refer to kp_normalize_mode_t */
    uint32_t crop_count;                    /**< crop count */
    kp_inf_crop_box_t inf_crop[MAX_CROP_BOX]; /**< box information to crop */
} __attribute__((aligned(4))) kp_generic_raw_image_header_t;

/**
 * @brief inference RAW output descriptor
 */
typedef struct
{
    uint32_t inference_number;              /**< inference sequence number */
    uint32_t crop_number;                   /**< crop box sequence number */
    uint32_t num_output_node;               /**< total number of output nodes */
    uint32_t product_id;                    /**< product id, refer to kp_product_id_t */
    kp_hw_pre_proc_info_t pre_proc_info;    /**< hardware pre-process related value */
} __attribute__((packed, aligned(4))) kp_generic_raw_result_header_t;

/**
 * @brief inference descriptor for images bypass pre-processing
 */
typedef struct
{
    uint32_t inference_number;              /**< inference sequence number */
    uint32_t model_id;                      /**< target inference model ID */
    uint32_t image_buffer_size;             /**< image buffer size */
} __attribute__((packed, aligned(4))) kp_generic_raw_bypass_pre_proc_image_header_t;

/**
 * @brief inference RAW output descriptor for bypass pre-processing
 */
typedef struct
{
    uint32_t inference_number;              /**< inference sequence number */
    uint32_t crop_number;                   /**< crop box sequence number */
    uint32_t num_output_node;               /**< total number of output nodes */
    uint32_t product_id;                    /**< product id, refer to kp_product_id_t */
} __attribute__((packed, aligned(4))) kp_generic_raw_bypass_pre_proc_result_header_t;

