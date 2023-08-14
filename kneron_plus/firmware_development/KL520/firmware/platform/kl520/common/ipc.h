/*
 * Kneron IPC Header for KL520
 *
 * Copyright (C) 2018-2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef KNERON_IPC_H
#define KNERON_IPC_H

#include <stdint.h>
#include "model_type.h"
#include "model_res.h"

#ifdef USE_64
typedef uint64_t kdp_size_t;
#else
typedef uint32_t kdp_size_t;
#endif

#ifndef BOOLEAN_DEFINED
#ifndef boolean
typedef char  boolean;
#endif
#define BOOLEAN_DEFINED
#endif


/* IPC memory */
//----------------------------
/* N i/d RAM */
#ifdef TARGET_NCPU
#define S_D_RAM_ADDR                0x20200000
#define N_D_RAM_ADDR                0x0FFF0000
#endif
#ifdef TARGET_SCPU
#define S_D_RAM_ADDR                0x10200000
#define N_D_RAM_ADDR                0x2FFF0000
#endif

#define S_D_RAM_SIZE                0x18000          /* 96 KB */
#define N_D_RAM_SIZE                0x10000          /* 64 KB */

#define IPC_RAM_SIZE                0x2000           /* 8K Bytes : split 7 : 1 */
#define IPC_MEM_OFFSET              (S_D_RAM_SIZE - IPC_RAM_SIZE)
#define IPC_MEM_OFFSET2             (S_D_RAM_SIZE - IPC_RAM_SIZE / 8)
#define IPC_MEM_ADDR                (S_D_RAM_ADDR + IPC_MEM_OFFSET)
#define IPC_MEM_ADDR2               (S_D_RAM_ADDR + IPC_MEM_OFFSET2)
#define SCPU_IPC_MEM_ADDR           IPC_MEM_ADDR
#define SCPU_IPC_MEM_ADDR2          IPC_MEM_ADDR2
//----------------------------

#define SCPU2NCPU_ID		('s'<<24 | 'c'<<16 | 'p'<<8 | 'u')
#define NCPU2SCPU_ID		('n'<<24 | 'c'<<16 | 'p'<<8 | 'u')

#define MULTI_MODEL_MAX         16      /* Max active models in memory */
#define IPC_IMAGE_ACTIVE_MAX    2       /* Max active images for NCPU/NPU */
#define IPC_IMAGE_MAX           5       /* Max cycled buffer for images */
#define IPC_MODEL_MAX           (MULTI_MODEL_MAX * IPC_IMAGE_ACTIVE_MAX)

/* Image process cmd_flags set by scpu */
#define IMAGE_STATE_INACTIVE          0
#define IMAGE_STATE_ACTIVE            1
#define IMAGE_STATE_NPU_DONE          2
#define IMAGE_STATE_DONE              3
#define IMAGE_STATE_JPEG_ENC_DONE     4
#define IMAGE_STATE_JPEG_DEC_DONE     5
#define IMAGE_STATE_ERR_DSP_BUSY      6
#define IMAGE_STATE_JPEG_ENC_FAIL     7
#define IMAGE_STATE_JPEG_DEC_FAIL     8
#define IMAGE_STATE_RECEIVING         9   //need check with mozart firmware
#define IMAGE_STATE_TOF_DEC_DONE      10
#define IMAGE_STATE_TOF_DEC_FAIL      11

/* Image process status set by ncpu */
#define IMAGE_STATE_IDLE                    0
#define IMAGE_STATE_NPU_BUSY                1
// #define IMAGE_STATE_NPU_DONE                2
#define IMAGE_STATE_POST_PROCESSING         IMAGE_STATE_NPU_DONE
#define IMAGE_STATE_POST_PROCESSING_DONE    3
// #define IMAGE_STATE_DONE                    IMAGE_STATE_POST_PROCESSING_DONE
#define IMAGE_STATE_TIMEOUT                 (7)
#define IMAGE_STATE_PREPROC_ERROR           (-1)
#define IMAGE_STATE_NPU_ERROR               (-2)
#define IMAGE_STATE_POSTPROC_ERROR          (-3)

/* NPU error code (sync with kp_struct.h) */
#define IMAGE_STATE_NCPU_ERR_BEGIN            200
#define IMAGE_STATE_NCPU_INVALID_IMAGE        201

/* Image format flags */
#define IMAGE_FORMAT_SUB128                 BIT31
#define IMAGE_FORMAT_ROT_MASK               (BIT30 | BIT29)
#define IMAGE_FORMAT_ROT_SHIFT              29
#define IMAGE_FORMAT_ROT_CLOCKWISE          0x01
#define IMAGE_FORMAT_ROT_COUNTER_CLOCKWISE  0x02

/* raw output format:
 * ([output_num][height_outnode1][channel_outnode1][width_outnode1][radix_outnode1][scale_outnode1][h2][c2][w2][r2][s2][...]
 * [h_n][c_n][w_n][r_n][s_n][fixed_point_datanode1][fixed_point_datanode2][...][fixed_point_datanodeN])
 * 1 byte for each fixed-point data. 4 bytes for each of other data.
 * fixed-point data is converted to float data with formula of fp_value / (scale * (2 ^ radix)).
 */
#define IMAGE_FORMAT_RAW_OUTPUT             BIT28
#define IMAGE_FORMAT_PARALLEL_PROC          BIT27

#define IMAGE_FORMAT_MODEL_AGE_GENDER       BIT24

/* right shift for 1-bit if 1 */
#define IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT    BIT22
#define IMAGE_FORMAT_SYMMETRIC_PADDING      BIT21
#define IMAGE_FORMAT_PAD_SHIFT              21

#define IMAGE_FORMAT_CHANGE_ASPECT_RATIO    BIT20

#define IMAGE_FORMAT_BYPASS_PRE             BIT19
#define IMAGE_FORMAT_BYPASS_NPU_OP          BIT18
#define IMAGE_FORMAT_BYPASS_CPU_OP          BIT17
#define IMAGE_FORMAT_BYPASS_POST            BIT16

#define IMAGE_FORMAT_NPU            0x00FF
#define NPU_FORMAT_RGBA8888         0x00
#define NPU_FORMAT_NIR              0x20
/* Support YCBCR (YUV) */
#define NPU_FORMAT_YCBCR422         0x30
#define NPU_FORMAT_YCBCR444         0x50
#define NPU_FORMAT_RGB565           0x60

/* Determine the exact format with the data byte sequence in DDR memory: [lowest byte]...[highest byte] */
#define NPU_FORMAT_YCBCR422_CRY1CBY0 0x30
#define NPU_FORMAT_YCBCR422_CBY1CRY0 0x31
#define NPU_FORMAT_YCBCR422_Y1CRY0CB 0x32
#define NPU_FORMAT_YCBCR422_Y1CBY0CR 0x33
#define NPU_FORMAT_YCBCR422_CRY0CBY1 0x34
#define NPU_FORMAT_YCBCR422_CBY0CRY1 0x35
#define NPU_FORMAT_YCBCR422_Y0CRY1CB 0x36
#define NPU_FORMAT_YCBCR422_Y0CBY1CR 0x37  // Y0CbY1CrY2CbY3Cr...

#define MAX_INT_FOR_ALIGN           0x10000000

/* Model structure */
struct kdp_model_s {
    /* Model type */
    uint32_t    model_type; //defined in model_type.h

    /* Model version */
    uint32_t    model_version;

    /* Input in memory */
    uint32_t    input_mem_addr;
    int32_t     input_mem_len;

    /* Output in memory */
    uint32_t    output_mem_addr;
    int32_t     output_mem_len;

    /* Working buffer */
    uint32_t    buf_addr;
    int32_t     buf_len;

    /* command.bin in memory */
    uint32_t    cmd_mem_addr;
    int32_t     cmd_mem_len;

    /* weight.bin in memory */
    uint32_t    weight_mem_addr;
    int32_t     weight_mem_len;

    /* setup.bin in memory */
    uint32_t    setup_mem_addr;
    int32_t     setup_mem_len;
};
typedef struct kdp_model_s kdp_model_info_t;

/* Result structure of a model */
struct result_buf_s {
    int32_t     model_id;
    uint32_t    result_mem_addr;
    int32_t     result_mem_len;
    int32_t     result_ret_len;
};

#define MAX_PARAMS_LEN          40 /* uint32_t */
#define MAX_INPUT_NODE_COUNT  5

/* Parameter structure of a raw image */
typedef struct parameter_s {
    /* Crop parameters or other purposes */
    int         crop_top;
    int         crop_bottom;
    int         crop_left;
    int         crop_right;

    /* Pad parameters or other purposes */
    int         pad_top;
    int         pad_bottom;
    int         pad_left;
    int         pad_right;

    float       scale_width;
    float       scale_height;
} parameter_t;

typedef struct kdp_img_info_s {
    /* input image in memory */
    uint32_t    image_mem_addr;
    int32_t     image_mem_len;

    /* raw image dimensions */
    uint32_t    input_row;
    uint32_t    input_col;
    uint32_t    input_channel;

    /* Raw image format and pre-process flags
     * bit-31: = 1 : subtract 128
     * bit 30:29 00: no rotation; 01: rotate clockwise; 10: rotate counter clockwise; 11: reserved
     * bit 7:0: format
     */
    uint32_t    format;

    /* Parameter structure */
    struct parameter_s params_s;
} kdp_img_info_t;

struct kdp_img_cfg {
    uint32_t num_image;
    kdp_img_info_t image_list[MAX_INPUT_NODE_COUNT];
    uint32_t inf_format;
    uint32_t image_buf_active_index; // scpu_to_ncpu->active_img_index
};

struct kdp_crop_box_s {
    int32_t top;
    int32_t bottom;
    int32_t left;
    int32_t right;
};

struct kdp_pad_value_s {
    int32_t pad_top;
    int32_t pad_bottom;
    int32_t pad_left;
    int32_t pad_right;
};

/* Raw image structure */
typedef struct kdp_img_raw_s {
    /* Image state: 1 = active, 0 = inactive */
    int         state;

    /* Image sequence number */
    int         seq_num;

    /* Image ref index */
    int         ref_idx;

    /* List of raw images */
    uint32_t num_image;
    kdp_img_info_t image_list[MAX_INPUT_NODE_COUNT];

    /* Parallel and raw output flags
     * refer to dsp_img_fmt_t
     */
    uint32_t inf_format;

    /* Shared parameters for raw image */
    uint32_t    ext_params[MAX_PARAMS_LEN];

    struct result_buf_s results[MULTI_MODEL_MAX];

    /* Test: SCPU total */
    uint32_t    tick_start;
    uint32_t    tick_end;

    /* Test: NCPU processes */
    uint32_t    tick_start_pre;
    uint32_t    tick_end_pre;
    uint32_t    tick_start_npu;
    uint32_t    tick_end_npu;
    uint32_t    tick_start_post;
    uint32_t    tick_end_post;
} kdp_img_raw_t;

/* Image result structure */
typedef struct kdp_img_result_s {
    /* Processing status: 2 = done, 1 = running, 0 = unused */
    int         status;

    /* Image sequence number */
    int         seq_num;

    /* result memory addr */
    //dummy information
    uint32_t    result_mem_addr;
} kdp_img_result_t;

/* Structure of nCPU->sCPU IPC Message data */
typedef struct ncpu_to_scpu_req_img_s {
    int32_t bHandledByScpu;
    int32_t ipc_type; //ncpu_scpu_ipc_msg_type_t
    int32_t sts;      //ncpu_status_t
} ncpu_to_scpu_req_img_t;

/* in every IPC interrupt triggered by NCPU, SCPU check in_comm_p to see the data type */
typedef enum {
    NCPU_REQUEST_NEW_IMG = 1,
    NCPU_EXEC_RESULT,
    MSG_ALIGN_32 = MAX_INT_FOR_ALIGN,
} ncpu_scpu_ipc_msg_type_t;

/* overall SCPU/DSP status*/
typedef enum {
    NCPU_STS_ERROR = -1,
    NCPU_STS_READY = 0,  //DSP is ready to run new task
    NCPU_STS_BUSY,       // one of CNN/JPEG ENC/JPEG DEC is running, cannot accept new  task now
    NCPU_STS_INVALID_PARAM,  // invalid IPC parameters
    STS_ALIGN_32 = MAX_INT_FOR_ALIGN,
} ncpu_status_t;

typedef struct
{
    int model_id;
    uint32_t tick_before_preprocess;
    uint32_t sum_ticks_preprocess;

    uint32_t tick_before_inference;
    uint32_t sum_ticks_inference;

    uint32_t tick_before_postprocess;
    uint32_t sum_ticks_postprocess;

    uint32_t tick_before_cpu_op;
    uint32_t sum_ticks_cpu_op;
    uint32_t sum_cpu_op_count;

    uint32_t sum_frame_count;
} kp_model_profile_t;

/* Structure of sCPU->nCPU Message */
typedef struct scpu_to_ncpu_s {
    uint32_t    id;        /* = 'scpu' */
    volatile uint32_t    bNcpuReceived;
    uint32_t    cmd;            // Run / Stop

    /*
     * debug control flags (dbg.h):
     *   bits 19-16: scpu debug level
     *   bits 03-00: ncpu debug level
     */
    uint32_t    debug_flags;
    uint32_t    kp_dbg_checkpoinots;

    int32_t             active_img_index_rgb_liveness;

    /* Active models in memory and running */
    int32_t             num_models;  //usually, num_models=1 (only one active model)
    struct kdp_model_s  models[IPC_MODEL_MAX];            //to save active modelInfo
    int32_t             models_type[IPC_MODEL_MAX];       //to save model type
    int32_t             model_slot_index;

    /* Raw image information */
    struct kdp_img_raw_s raw_images[IPC_IMAGE_MAX];
    kdp_img_raw_t   *pRawimg;     //SCPU need to alloc for every image
    uint32_t        active_img_index;

    uint32_t        ncpu_img_req_msg_addr;  // ncpu_to_scpu_req_img_t *, SCPU always get result from here

    /* Input/Output working buffers for NPU */
    uint32_t    input_mem_addr2;
    int32_t     input_mem_len2;

    /* Memory for parallel processing */
    uint32_t    output_mem_addr2;
    int32_t     output_mem_len2;

    /* Memory for pre processing command */
    uint32_t    inproc_mem_addr;

    /* Memory for post processing parameters */
    uint32_t    output_mem_addr3;
    uint32_t    output_mem_addr4;

    void *      kp_dbg_buffer;

    uint32_t    kp_dbg_enable_profile; // 1: enable, 0: disable
    kp_model_profile_t kp_model_profile_records[MULTI_MODEL_MAX];
} scpu_to_ncpu_t;

typedef enum {
    NCPU_NONE_RESULT = -1,
    NCPU_POSTPROC_RESULT = 1,
    NCPU_JPEG_ENC_RESULT,
    NCPU_JPEG_DEC_RESULT,
    NCPU_CROP_RESIZE_RESULT,
    NCPU_TOF_DEC_RESULT,
    NCPU_RESULT_TYPE_MAX,
    RES_ALIGN_32 = MAX_INT_FOR_ALIGN,
} NCPU_TO_SCPU_RESULT_TYPE;

/* Structure of nCPU->sCPU IPC Message data */
typedef struct ncpu_to_scpu_postproc_result_s {
    int32_t model_slot_index; // RUN which model for this image
    kdp_img_result_t img_result;
    uint32_t OrigRawImgAddr;
} ncpu_to_scpu_postproc_result_t;

/* Structure of nCPU->sCPU Message */
typedef struct ncpu_to_scpu_s {
    uint32_t    id;        /* = 'ncpu' */
    int32_t bHandledByScpu;
    ncpu_scpu_ipc_msg_type_t ipc_type __attribute__((aligned (4)));           //ncpu_scpu_ipc_msg_type_t
    ncpu_status_t sts __attribute__((aligned (4)));                //overall NCPU/DSP status
    NCPU_TO_SCPU_RESULT_TYPE out_type __attribute__((aligned (4)));

    union {
        ncpu_to_scpu_postproc_result_t postproc;
    } result;

    /* Images result info corresponding to raw_images[] */
    struct kdp_img_result_s img_results[IPC_IMAGE_MAX];

    ncpu_to_scpu_req_img_t req_img;
    volatile int32_t kp_dbg_status;
} ncpu_to_scpu_result_t;

/* scpu_to_ncpu: cmd */
typedef enum {
    CMD_INVALID,
    CMD_INIT,
    CMD_RUN_NPU,
    CMD_SLEEP_NPU,
    CMD_JPEG_ENCODE,
    CMD_JPEG_DECODE,
    CMD_CROP_RESIZE,
    CMD_TOF_DECODE,
    CMD_SCPU_NCPU_TOTAL,
    CMD_ALIGN_32 = MAX_INT_FOR_ALIGN,
} scpu_ncpu_cmd_t;

struct nir_camera_tune_s {
    uint32_t    init_tile;
    uint32_t    led_flag;
    uint32_t    nir_mode;
    float       init_nir_gain;
    float       nir_gain;
    uint32_t    nir_cur_exp_time;
    uint32_t    calibration_count;
    float       registered_offsetX;
    float       registered_offsetY;
};

struct lv_params_s {
    uint32_t dual_landmarks[DUAL_LAND_MARK_POINTS * 2];
    struct nir_camera_tune_s nir_tune;
};

#endif
