#pragma once

#define KDP2_INF_ID_GENERIC_RAW 10
#define KDP2_INF_ID_GENERIC_RAW_BYPASS_PRE_PROC 17

// FIXME ?
// Parsing KL720 raw output
// Copied from beethoven_sw\firmware\platform\kl720\common\model_res.h
typedef struct
{
    uint32_t start_offset;
    uint32_t buf_len;
    uint32_t node_id;
    uint32_t supernum;
    uint32_t data_format;
    uint32_t row_start;
    uint32_t col_start;
    uint32_t ch_start;
    uint32_t row_length;
    uint32_t col_length;
    uint32_t ch_length;
    uint32_t output_index;
    uint32_t output_radix;
    uint32_t output_scale;
} _720_raw_onode_t;

typedef struct
{
    uint32_t total_raw_len;
    int32_t total_nodes;
    _720_raw_onode_t onode_a[40];
    uint8_t data[];
} _720_raw_cnn_res_t;

typedef struct
{
    uint32_t idx;  //sequence number of out sub-nodes
    uint32_t fmt;
    uint32_t batch;
    uint32_t ch_length;
    uint32_t row_length;
    uint32_t col_length;
    uint32_t buf_addr;
    uint32_t buf_len;
    uint32_t scale;
    uint32_t radix;
    uint32_t start_offset;
    uint32_t buf_aligned_len;
    uint32_t quant_vect_len;
} _630_raw_onode_t;

typedef struct
{
    uint32_t total_raw_len;
    int32_t total_nodes;
    _630_raw_onode_t onode_a[40];
    uint8_t data[];
} _630_raw_cnn_res_t;

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
