/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */

/******************************************************************************
*  Filename:
*  ---------
*  ipc.h
*
*  Description:
*  ------------
*
*
******************************************************************************/

#ifndef _IPC_H_
#define _IPC_H_

/******************************************************************************
Head Block of The File
******************************************************************************/

#include <stdint.h>
#include "model_type.h"
#include "model_res.h"
#include "base.h"
#include "jpeg_intf.h"
#include "tof_intf.h"

#if _BOARD_SN720HAPS_H_ == 1
#define ADDR_ADJUST_OFFSET_FOR_HAPS 0x01d2b000

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
/* IPC memory */
//----------------------------

/*  Used for KL720 */
#define SCPU_IPC_MEM_ADDR  (0x85060000 + ADDR_ADJUST_OFFSET_FOR_HAPS)
#define SCPU_IPC_MEM_ADDR2 (0x85061000 + ADDR_ADJUST_OFFSET_FOR_HAPS)
#define IPC_NPU_REQ_IMG_MSG_ADDR   (SCPU_IPC_MEM_ADDR2+0x100)

#else
#include "membase.h"

#define SCPU_IPC_MEM_ADDR           DDR_MEM_IPC_ADDR
#define SCPU_IPC_MEM_ADDR2          (SCPU_IPC_MEM_ADDR + 0x1C00)

#endif

#define SCPU2NCPU_ID		('s'<<24 | 'c'<<16 | 'p'<<8 | 'u')
#define NCPU2SCPU_ID		('n'<<24 | 'c'<<16 | 'p'<<8 | 'u')

#define MULTI_MODEL_MAX         16      /* Max active models in memory */
#define IPC_IMAGE_ACTIVE_MAX    2       /* Max active images for NCPU/NPU */
#define IPC_IMAGE_MAX 10          /* Max cycled buffer for images */
#define IPC_MODEL_MAX           (MULTI_MODEL_MAX * IPC_IMAGE_ACTIVE_MAX)

/* Image process cmd_flags set by scpu, TODO: Check all state command */
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
#define IMAGE_STATE_TOF_CALC_IR_BRIGHT_DONE 12
#define IMAGE_STATE_TOF_CALC_IR_BRIGHT_FAIL 13

/* Image process status set by ncpu */
#define IMAGE_STATE_IDLE                    0
#define IMAGE_STATE_NPU_BUSY                1
//#define IMAGE_STATE_NPU_DONE                2
#define IMAGE_STATE_POST_PROCESSING         IMAGE_STATE_NPU_DONE
#define IMAGE_STATE_POST_PROCESSING_DONE    3
//#define IMAGE_STATE_DONE                    IMAGE_STATE_POST_PROCESSING_DONE
#define IMAGE_STATE_ERR                     (-1)
#define IMAGE_STATE_TIMEOUT                 (7)

/* Image format flags and config values */
typedef enum {
    /* normalization control
     * ------------------*/
    IMAGE_FORMAT_SUB128                 = (int)BIT31,   /* 1: sub 128 for each value */
    IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT    = BIT22,        /* 1: right shift for 1-bit (normalization)*/


    /* cv rotate control
     * ------------------*/
    IMAGE_FORMAT_ROT_MASK               = (BIT30 | BIT29),
    IMAGE_FORMAT_ROT_SHIFT              = 29,
    /*  -- setting values of ROT -- */
    IMAGE_FORMAT_ROT_CLOCKWISE          = 0x01,         /* ROT 90  */
    IMAGE_FORMAT_ROT_COUNTER_CLOCKWISE  = 0x02,         /* ROT 270 */
    //IMAGE_FORMAT_ROT_180                = 0x03,         // TODO, ROT 180


    /* flow control
     * ------------------*/
    //IMAGE_FORMAT_RAW_OUTPUT             = BIT28       !!! move to "bypass control"
    IMAGE_FORMAT_PARALLEL_PROC          = BIT27,        /* 1: parallel execution of NPU and NCPU */
    //IMAGE_FORMAT_NOT_KEEP_RATIO         = BIT26,      // TODO, duplicated

    //IMAGE_FORMAT_MODEL_AGE_GENDER       = BIT24,      // TODO, should remove


    /* scale/crop control
     * -------------------*/
    //IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT  = BIT22,        !!! move to "normalization functions"
    IMAGE_FORMAT_SYMMETRIC_PADDING      = BIT21,        /* 1: symmetic padding;
                                                           0: corner padding */

    IMAGE_FORMAT_CHANGE_ASPECT_RATIO    = BIT20,        /* 1: scale without padding;
                                                           0: scale with padding */


    /* flow control - 2
     * ------------------*/
    IMAGE_FORMAT_BYPASS_PRE             = BIT19,        /* 1: bypass pre-process */
    IMAGE_FORMAT_BYPASS_NPU_OP          = BIT18,        /* 1: bypass NPU OP */
    IMAGE_FORMAT_BYPASS_CPU_OP          = BIT17,        /* 1: bypass CPU OP */

    IMAGE_FORMAT_BYPASS_POST            = BIT16,        /* 1: bypass post-process (output NPU result directly) */
    IMAGE_FORMAT_RAW_OUTPUT             = BIT28,        /* 1: bypass post-process (include meta data for data parsing )*/


    /* supported image foramts            BIT7 - BIT0
     * --------------------------------------*/
    IMAGE_FORMAT_NPU                    = 0x00FF,       /* settings: NPU_FORMAT_XXXX */

} dsp_img_fmt_t;

/*********************************************
 *  settings for IMAGE_FORMAT_NPU
 *********************************************/
#define NPU_FORMAT_RGBA8888          0x00
#define NPU_FORMAT_YUV422            0x10   /* similiar to Y0CBY1CR */
#define NPU_FORMAT_NIR               0x20

/* Determine the exact format with the data byte sequence in DDR memory:
 * [lowest byte]...[highest byte]
 */
#define NPU_FORMAT_YCBCR422          0x30   /* alias of CRY1CBY0 */
#define NPU_FORMAT_YCBCR422_CRY1CBY0 0x30
#define NPU_FORMAT_YCBCR422_CBY1CRY0 0x31
#define NPU_FORMAT_YCBCR422_Y1CRY0CB 0x32
#define NPU_FORMAT_YCBCR422_Y1CBY0CR 0x33
#define NPU_FORMAT_YCBCR422_CRY0CBY1 0x34
#define NPU_FORMAT_YCBCR422_CBY0CRY1 0x35
#define NPU_FORMAT_YCBCR422_Y0CRY1CB 0x36
#define NPU_FORMAT_YCBCR422_Y0CBY1CR 0x37   /* Y0CbY1CrY2CbY3Cr... */

#define NPU_FORMAT_YUV444            0x40
#define NPU_FORMAT_YCBCR444          0x50
#define NPU_FORMAT_RGB565            0x60
// ------------------------------------------

#define MAX_CNN_NODES 45 //NetputNode, CPU nodes, Out Nodes, etc
#define MAX_OUT_NODES 40  //max Out Nodes

#define MAX_INT_FOR_ALIGN           0x10000000
#define NCPU_CLOCK_CNT_PER_MS       500000

#define KP_DEBUG_BUF_SIZE (8 * 1024 * 1024) // FIXME, max is 1920x1080 RGB8888

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
/* Model structure */
typedef struct kdp_model_s {
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
} kdp_model_t;

typedef struct kdp_model_s kdp_model_info_t;

/* Result structure of a model */
typedef struct result_buf_s {
    int32_t     model_id;
    uint32_t    result_mem_addr;
    int32_t     result_mem_len;
    int32_t     result_ret_len;
} result_buf_t;

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
     * refer to dsp_img_fmt_t
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

typedef struct {
    uint32_t w;
	uint32_t h;
	uint32_t c;
} img_dim_t;

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
    CMD_TOF_CALC_IR_BRIGHT,
    CMD_SCPU_NCPU_TOTAL,
    CMD_ALIGN_32 = MAX_INT_FOR_ALIGN,
} scpu_ncpu_cmd_t;

/* in every IPC interrupt triggered by NCPU, SCPU check in_comm_p to see the data type */
typedef enum {
    NCPU_REQUEST_NEW_IMG = 1,
    NCPU_EXEC_RESULT,
    MSG_ALIGN_32 = MAX_INT_FOR_ALIGN,
} ncpu_scpu_ipc_msg_type_t;

/* overall SCPU/DSP status*/
typedef enum {
    NCPU_STS_READY = 0,  //DSP is ready to run new task
    NCPU_STS_BUSY,       // one of CNN/JPEG ENC/JPEG DEC is running, cannot accept new  task now
    NCPU_STS_INVALID_PARAM,  // invalid IPC parameters
    STS_ALIGN_32 = MAX_INT_FOR_ALIGN,
} ncpu_status_t;

typedef enum {
    POST_PROC_FAIL = -1,
    POST_PROC_SUCCESS = 0,
    RET_ALIGN_32 = MAX_INT_FOR_ALIGN,
} post_proc_return_sts_t;

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

    struct result_buf_s result;

    /* Test: SCPU total */
    uint32_t    tick_start;
    uint32_t    tick_end;
    uint32_t    tick_got_ncpu_ack;

    /* Test: NCPU processes */
    uint32_t    tick_start_parse;
    uint32_t    tick_end_parse;
    uint32_t    tick_start_inproc;
    uint32_t    tick_end_inproc;
    uint32_t    tick_start_pre;
    uint32_t    tick_end_pre;
    uint32_t    tick_start_npu;
    uint32_t    tick_cnn_interrupt_rvd;
    uint32_t    tick_end_npu;
    uint32_t    tick_start_post;
    uint32_t    tick_end_post;
    uint32_t    tick_start_dram_copy;
    uint32_t    tick_end_dram_copy;
    uint32_t    tick_rslt_got_scpu_ack;
    uint32_t    tick_ncpu_img_req;
    uint32_t    tick_ncpu_img_ack;
    uint32_t    tick_last_img_req;
} kdp_img_raw_t;

struct nir_camera_tune_s{
    uint8_t     init_tile;
    uint8_t     nir_mode;
    float       init_nir_gain;
    float       nir_gain;
    uint32_t    nir_cur_exp_time;
    uint32_t    calibration_count;
    float       registered_offsetX;
    float       registered_offsetY;
    uint8_t     rgb_led_flag;
    uint8_t     rgb_avg_luma;
    float       x_scaling;
    uint8_t     d_offset;
    uint8_t     pass_type;
    bool       ignore_rgb_led;
    bool       bctc;
    uint8_t     input_nir_led_on_tile;
    uint8_t     nir_led_flag;
    uint8_t     input_distance;
    uint32_t    rgb_cur_exp_time;
    uint32_t    rgb_init_exp_time; 
    uint8_t     pre_gain;
    uint8_t     post_gain;
    uint8_t     global_gain;
    uint8_t     y_average;
    float       rgb_lm_score;
    float       nir_lv_cnn_face_real_score;
    float       fuse_lv_cnn_real_score;
};

/* Image result structure */
typedef struct kdp_img_result_s {
	post_proc_return_sts_t   status __attribute__((aligned (4)));
    /* Image sequence number */
    int         seq_num;

    /* result memory addr */
    //dummy information
    uint32_t    result_mem_addr;
} kdp_img_result_t;

typedef struct {
    uint32_t    fmt;

    parameter_t param;

    img_dim_t   src_dim;
    img_dim_t   dst_dim;

    uint32_t    src_addr;
    uint32_t    src_data_len;

    uint32_t    dst_addr;
    uint32_t    dst_buf_size;
	uint32_t    dst_filled_len;

    int32_t     seq_num;

	int32_t     bUseHwInproc;   /* 1: use NPU HW inproc; 0: use DSP SW solution */

	uint32_t   tmp_buf_addr;    /* this tmp_buf_addr is needed for SW crop/resize, not for HW inproc */

} crop_resize_param_t;

typedef enum {
    CROP_RESIZE_OPERATION_FAILED = -1,   /* Failure in doing operation */
    CROP_RESIZE_OPERATION_SUCCESS = 1,   /* Operation Succeded */
    CROP_RESIZE_OPERATION_INVALID_PARM,  /* Inavlid parameter provided */
    RESIZE_ALIGN_32 = MAX_INT_FOR_ALIGN,
} crop_resize_oper_sts_t;

typedef struct {

	int32_t   rslt_type;    //NCPU_TO_SCPU_RESULT_TYPE

    crop_resize_oper_sts_t sts __attribute__((aligned (4)));

    /* output buf */
    uint32_t    out_addr;
    uint32_t    out_len;

    int32_t seq_num;

} crop_resize_result_t;

/* Structure of nCPU->sCPU IPC Message data */
typedef struct ncpu_to_scpu_req_img_s {
    int32_t bHandledByScpu;
    int32_t ipc_type; //ncpu_scpu_ipc_msg_type_t
    int32_t sts;      //ncpu_status_t
} ncpu_to_scpu_req_img_t;

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

typedef struct
{
    int model_id;
    uint32_t cycle_before_preprocess;
    uint64_t sum_cycles_preprocess;

    uint32_t cycle_before_inference;
    uint64_t sum_cycles_inference;

    uint32_t cycle_before_postprocess;
    uint64_t sum_cycles_postprocess;

    uint32_t cycle_before_cpu_op;
    uint64_t sum_cycles_cpu_op;
    uint64_t sum_cpu_op_count;

    uint32_t sum_frame_count;
} kp_model_profile_cycle_t;

/* Structure of sCPU->nCPU Message */
typedef struct scpu_to_ncpu_s {
    uint32_t    id;        /* = 'scpu' */
    volatile uint32_t    bNcpuReceived;
    uint32_t    cmd;            // scpu_ncpu_cmd_t

    /*
     * debug control flags (dbg.h):
     *   bits 19-16: scpu debug level
     *   bits 03-00: ncpu debug level
     */
    uint32_t    debug_flags;
    uint32_t    kp_dbg_checkpoinots;

    /* Active models in memory and running */
    int32_t             num_models;  //usually, num_models=1 (only one active model)
    struct kdp_model_s  models[IPC_MODEL_MAX];            //to save active modelInfo
    int32_t             models_type[IPC_MODEL_MAX];       //to save model type
    int32_t         model_slot_index;

    /* working buffer in case in-proc is necessary, raw img will copy to here for in-proc,
       in-proc output will be placed in the input mem address in setup.bin */
    uint32_t    input_mem_addr2;
    int32_t     input_mem_len2;

    /* Memory for DME */
    uint32_t    output_mem_addr2;
    int32_t     output_mem_len2;

    /* Memory for post processing (shared) */
    uint32_t    output_mem_addr3;

    kdp_img_raw_t *pRawimg;     //SCPU need to alloc for every image
    uint32_t       ncpu_img_req_msg_addr;  // ncpu_to_scpu_req_img_t *, SCPU always get result from here

    uint32_t    log_buf_base;
    int32_t     log_buf_len;

	/* support features extension or for standalone non-cnn features */
	void*       pExtInParam;       //pointer to extended parameter data structure
	int32_t     nLenExtInParam;    //length of extended parameter data structure

    void*       pExtOutRslt;        //pointer to extended feature result data structure
    int32_t     nLenExtOutRslt;     //Length of extended feature result data

    void*       pCpuNodeBuffer;     // pointer to working buffer for Cpu Node
    int32_t     nLenCpuNodeBuffer;  // Length of working buffer for Cpu Node

    /* Raw image information */
    struct kdp_img_raw_s raw_images[IPC_IMAGE_MAX];

    /* Memory for post processing (shared) */
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
    NCPU_IR_BRIGHT_RESULT,
    NCPU_RESULT_TYPE_MAX,
    RES_ALIGN_32 = MAX_INT_FOR_ALIGN,
} NCPU_TO_SCPU_RESULT_TYPE;

/* Structure of nCPU->sCPU IPC Message data */
typedef struct ncpu_to_scpu_postproc_result_s {
    int32_t model_slot_index; // RUN which model for this image
    kdp_img_result_t img_result;
    uint32_t OrigRawImgAddr;
} ncpu_to_scpu_postproc_result_t;

typedef struct ncpu_to_scpu_s {
    volatile boolean print_log;
    uint8_t    *p_log_buf_base;
    uint32_t    		id;        /* = 'ncpu' */
    int32_t         bHandledByScpu;
    ncpu_scpu_ipc_msg_type_t   ipc_type __attribute__((aligned (4)));           //ncpu_scpu_ipc_msg_type_t
    ncpu_status_t    sts __attribute__((aligned (4)));                //overall NCPU/DSP status
    NCPU_TO_SCPU_RESULT_TYPE out_type __attribute__((aligned (4)));

    union {
        ncpu_to_scpu_postproc_result_t postproc;
        jpeg_encode_result_t jpeg_encode;
        jpeg_decode_result_t jpeg_decode;
        tof_decode_result_t tof_decode;
        ir_bright_result_t tof_ir_bright;
	    crop_resize_result_t crop_resize;
    } result;
    uint32_t   extRsltAddr;

    ncpu_to_scpu_req_img_t      req_img;
    volatile int32_t kp_dbg_status;
} ncpu_to_scpu_result_t;

/*50k log buffer*/
#define MAX_LOG_LENGTH 256
#define LOG_QUEUE_NUM  200
#define FLAG_LOGGER_SCPU_IN      (1U << 0)
#define FLAG_LOGGER_NCPU_IN      (1U << 1)

enum {
    LOGGER_SCPU_IN = 0,
    LOGGER_NCPU_IN,
    LOGGER_OUT,
    LOGGER_TOTAL
};

typedef struct {
    volatile boolean init_done;
    volatile boolean willing[LOGGER_TOTAL];
    volatile uint8_t w_idx;
    volatile uint8_t r_idx;
    volatile uint8_t turn;
    uint8_t *p_msg;
} logger_mgt_t;


#endif //_IPC_H_

