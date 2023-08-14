#ifndef KDP2_INF_GENERIC_RAW_H
#define KDP2_INF_GENERIC_RAW_H

#include <stdint.h>
#include "kp_struct.h"
#include "buffer_object.h"

#define KDP2_INF_ID_GENERIC_RAW 10
#define KDP2_INF_ID_GENERIC_RAW_BYPASS_PRE_PROC 17

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t resize_mode;
    uint32_t padding_mode;
    uint32_t image_format;
    uint32_t normalize_mode;
    uint32_t crop_count;
    kp_inf_crop_box_t inf_crop[MAX_CROP_BOX];
} __attribute__((aligned(4))) kdp2_ipc_generic_raw_inf_image_header_t;

// input header for 'Generic RAW inference'
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t inference_number;
    uint32_t model_id;
    kdp2_ipc_generic_raw_inf_image_header_t image_header;
} __attribute__((aligned(4))) kdp2_ipc_generic_raw_inf_header_t;

// result header for 'Generic RAW inference'
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t num_of_pre_proc_info;
    kp_hw_pre_proc_info_t pre_proc_info[MAX_INPUT_NODE_COUNT];
    uint32_t product_id;   // enum kp_product_id_t.
    uint32_t inf_number;
    uint32_t crop_number;
    uint32_t is_last_crop; // 0: not last crop box, 1: last crop box
    uint8_t raw_data[];    // just imply following raw output data
} __attribute__((aligned(4))) kdp2_ipc_generic_raw_result_t;

// input header for 'Generic RAW inference Bypass Pre-Process'
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t inference_number;
    uint32_t model_id;
    uint32_t image_buffer_size;
} __attribute__((aligned(4))) kdp2_ipc_generic_raw_inf_bypass_pre_proc_header_t;

// result header for 'Generic RAW inference Bypass Pre-Process'
typedef kdp2_ipc_generic_raw_result_t kdp2_ipc_generic_raw_bypass_pre_proc_result_t;

#endif
