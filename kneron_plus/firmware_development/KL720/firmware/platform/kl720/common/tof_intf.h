#ifndef __TOF_INTERFACE_H__
#define __TOF_INTERFACE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "kp_struct.h"

#define TOF_DEPTH_IMG_SIZE (640 * 240 * 2)
#define TOF_IR_IMG_SIZE (640 * 240 * 2)
#define TOF_TEMP_DEPTH_SIZE (640 * 240 * 2 + 640 * 480) * 2 //(640*480*2+640*480)*2
#define TOF_INIT_TABLE_SIZE (640 * 480 * 2)
typedef struct
{
    uint32_t buf_addr;
    int32_t buf_filled_len;
} tof_buf_t;

typedef struct
{
    uint32_t gen_gif : 1; /* generate gif image for validation */
    uint32_t gen_rgb : 1; /* generate RGB format data */
    uint32_t reserved : 30;
} kdp_tof_ftr_t;

typedef struct
{
    float cx;
    float cy;
    float fx;
    float fy;
    float k1;
    float k2;
    float k3;
    float k4;
    float p1;
    float p2;
} ir_params;

typedef struct
{
    uint8_t flag; // bit0: p1 p2 is loaded
    /* src img buf */
    // 640 * 241 * 5
    // 640(header) + 640*240(noise) + 640(header) + 640*240(img1) + 640(header) + 640*240(img2) + 640(header) + 640*240(img3) + 640(header) + 640*240(img4)
    tof_buf_t src_buf;

    /* src params (by device) */
    unsigned char *CalibParameter;

    int16_t *non_linear_H;

    int16_t *gamma_table;

    int16_t *p1_table;
    int16_t *p2_table;

    int16_t *shift_table;
    int16_t *table_gen;
    float *gradient_lut;
    float *temperature_coeff;

    ir_params *ir_params;

    /* */
    uint32_t width;
    uint32_t height;

    /*temp for depth computation*/
    uint32_t temp_depth_buf;
    uint32_t temp_depth_buf_size;

    /* output depth buf */
    uint32_t dst_depth_buf;
    uint32_t dst_depth_buf_size;

    /* output ir buf */
    uint32_t dst_ir_buf;
    uint32_t dst_ir_buf_size;

    /* frame time stamp, used for tracking/debug */
    int32_t frame_seq;

} tof_decode_params_t;

typedef enum
{
    TOF_DECODE_OPERATION_FAILED = -1,   /* Failure in doing operation */
    TOF_DECODE_OPERATION_SUCCESS = 1,   /* Operation Succeded */
    TOF_DECODE_OPERATION_INVALID_PARM,  /* Inavlid parameter provided */
    TOF_DECODE_OPERATION_NOT_SUPPORTED, /* Parameter/operation not supported */
    TOF_DECODE_OPER_ALIGN_32 = MAX_INT_FOR_ALIGN,
} tof_oper_sts_t;

typedef enum
{
    TOF_IR_AEC_OPERATION_FAILED = -1,   /* Failure in doing operation */
    TOF_IR_AEC_OPERATION_SUCCESS = 1,   /* Operation Succeded */
    TOF_IR_AEC_OPERATION_INVALID_PARM,  /* Inavlid parameter provided */
    TOF_IR_AEC_OPERATION_NOT_SUPPORTED, /* Parameter/operation not supported */
    TOF_IR_AEC_OPER_ALIGN_32 = MAX_INT_FOR_ALIGN,
} tof_ir_aec_oper_sts_t;

typedef struct
{
    // uint32_t buf_addr;
    // int32_t  buf_filled_len;
    int32_t width;
    int32_t height;
    // Depth resolution:640 x 240 x 2byte (uint16 Depth16 format).
    uint32_t buf; // uint16_t *buf;
} depth_buf_t;

typedef struct
{
    // uint32_t buf_addr;
    // int32_t  buf_filled_len;
    int32_t width;
    int32_t height;
    // IR resolution:640 x 240(uint8).
    uint32_t buf; // uint16_t *buf;
} ir_buf_t;

typedef struct
{

    int32_t rslt_type; // NCPU_TO_SCPU_RESULT_TYPE

    tof_oper_sts_t sts __attribute__((aligned(4)));

    /* depth image buf */
    depth_buf_t depth_buf;

    /* ir image buf */
    ir_buf_t ir_buf;

    /* frame time stamp, used for tracking/debug */
    int32_t frame_seq;

} tof_decode_result_t;

typedef struct
{
    ir_buf_t src_buf;
    bounding_box_t *ir_aec_bbox;
    bool is_target_found;
} ir_bright_params_t;

typedef struct
{
    int32_t rslt_type; // NCPU_TO_SCPU_RESULT_TYPE
    tof_ir_aec_oper_sts_t sts __attribute__((aligned(4)));
    uint8_t rslt_bright;
} ir_bright_result_t;

#define MAX_INT_FOR_ALIGN 0x10000000

    /* kneron specific feature*/

    // typedef enum {
    //     tof_FMT_JCS_UNKNOWN,       /* error/unspecified */
    //     tof_FMT_JCS_GRAYSCALE,     /* monochrome */
    //     tof_FMT_JCS_RGB,       /* red/green/blue */
    //     tof_FMT_JCS_BGR565,
    // 	tof_FMT_JCS_RGB565,
    //     tof_FMT_JCS_YCbCr,     /* Y/Cb/Cr (also known as YUV) */
    //     tof_FMT_JCS_YUYV,      /* Y/U/Y/V */
    //     tof_FMT_JCS_CMYK,      /* C/M/Y/K */
    //     tof_FMT_JCS_YCCK,       /* Y/Cb/Cr/K */

    //     tof_YUV_420 = tof_FMT_JCS_YCbCr,
    //     tof_YUYV = tof_FMT_JCS_YUYV,
    //     tof_YUV_422,
    //     tof_YUV_444,
    // } tof_fmt_t;

    // typedef struct {

    //     /* src img buf */
    //     tof_buf_t src_buf;

    //     tof_fmt_t out_fmt __attribute__((aligned (4)));
    //     uint32_t width;
    //     uint32_t height;

    //     /* output buf */
    //     uint32_t dst_yuv_buf;
    //     uint32_t dst_buf_size;

    //     /* gif buf generated for validation */
    //     uint32_t gif_buf;
    //     uint32_t gif_buf_size;

    //     /* restart for multiple pass decoding for motion tof */
    //     boolean  restart;

    //     /* kneron specific features enablement */
    //     kdp_tof_ftr_t ftr;

    //     /* frame time stamp, used for tracking/debug */
    //     int32_t frame_seq;
    // } tof_decode_params_t;

    // typedef struct {

    // 	int32_t   rslt_type;    //NCPU_TO_SCPU_RESULT_TYPE

    //     tof_oper_sts_t sts __attribute__((aligned (4)));

    //     /* output buf */
    //     yuv_buf_t yuv_buf;

    //     /* gif buf generated for validation */
    //     tof_buf_t gif_buf;

    //     /* frame time stamp, used for tracking/debug */
    //     int32_t frame_seq;

    // } tof_decode_result_t;

#endif
