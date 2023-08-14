/**
 * @file        kp_internal.h
 * @brief       internal data structure
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#include "kp_usb.h"

#define MAX_GROUP_DEVICE 20

typedef struct
{
    // public
    int timeout;
    int num_device;
    kp_product_id_t product_id;
    kp_model_nef_descriptor_t loaded_model_desc;
    kp_ddr_manage_attr_t ddr_attr;

    // private
    int cur_send; // record current sending device index
    int cur_recv; // record current receiving device index
    kp_usb_device_t *ll_device[MAX_GROUP_DEVICE];

} __attribute__((aligned(4))) _kp_devices_group_t;

#define MAX_RAW_OUTPUT_NODE 50 // guess this number is enough !!!???

#define IMAGE_FORMAT_RAW_OUTPUT 0x10000000
#define IMAGE_FORMAT_BYPASS_POST 0x00010000 // not working

#define IMAGE_FORMAT_PARALLEL_PROC 0x08000000
#define IMAGE_FORMAT_SUB128 0x80000000
#define IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT 0x00400000

// image format
#define NPU_FORMAT_RGB565 0x60

// this is used to replace 'kdp2_ipc_generic_raw_result_t' internally for fixed-point or floating-ppoint conversion
// as different platform has different raw data format
typedef struct
{
    kp_product_id_t product_id;
} __attribute__((aligned(4))) raw_output_replace_header_t;

// channel ordering convert code
typedef enum
{
    KP_CHANNEL_ORDERING_CVT_NONE = 0,
    KP_CHANNEL_ORDERING_CVT_CHW2HCW = 1,
    KP_CHANNEL_ORDERING_CVT_HCW2CHW = 2,
    KP_CHANNEL_ORDERING_CVT_CHW2HWC = 3,
    KP_CHANNEL_ORDERING_CVT_HCW2HWC = 4
} kp_channel_ordering_convert_t;

/**
 * @brief kneron plus firmware boot mode (KL630)
 * @note please sync this enum in kl630/kdp_apps/kmdw/libkutils/include/boot_config.h
 */
typedef enum
{
    BOOT_MODE_UNKNOWN = 0,                  /**< unknown boot mode */
    BOOT_MODE_USB = 1,                      /**< USB boot mode */
    BOOT_MODE_FLASH = 2,                    /**< flash boot mode */
    BOOT_MODE_TOTAL = 3,                    /**< total boot mode num */
} boot_mode_t;
