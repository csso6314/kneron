#ifndef __JPEG_INTERFACE_H__
#define __JPEG_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#if TARGET_NCPU
#include "base.h"
#else
#ifndef BOOLEAN_DEFINED
#ifndef boolean
typedef char  boolean;
#endif
#define BOOLEAN_DEFINED
#endif
#endif

#define MAX_INT_FOR_ALIGN           0x10000000

//reserve possible max size, depends on the max input image dimention, 8 is a redundant for safe
//#define MAX_FHD_YUV_SIZE      (1920*1080*2 + 8)
//#define MAX_FHD_JPEG_SIZE     1920*1080

// global memory blocks which can be reused between jpeg enc/dec
extern uint32_t jpeg_4M_reserved_blk;
extern uint32_t jpeg_2M_reserved_blk;

/* kneron specific feature*/
typedef struct  {
    uint32_t gen_gif:1;    /* generate gif image for validation */
    uint32_t gen_rgb:1;    /* generate RGB format data */
    uint32_t reserved:30;
} kdp_jpeg_ftr_t;

typedef enum {
    JPEG_OPERATION_FAILED = -1,   /* Failure in doing operation */
    JPEG_OPERATION_SUCCESS = 1,   /* Operation Succeded */
    JPEG_OPERATION_INVALID_PARM,  /* Inavlid parameter provided */
    JPEG_OPERATION_NOT_SUPPORTED, /* Parameter/operation not supported */
    OPER_ALIGN_32 = MAX_INT_FOR_ALIGN,
} jpeg_oper_sts_t;


typedef enum {
    JPEG_FMT_JCS_UNKNOWN,       /* error/unspecified */
    JPEG_FMT_JCS_GRAYSCALE,     /* monochrome */
    JPEG_FMT_JCS_RGB,       /* red/green/blue */
    JPEG_FMT_JCS_BGR565,
	JPEG_FMT_JCS_RGB565,
    JPEG_FMT_JCS_YCbCr,     /* Y/Cb/Cr (also known as YUV) */
    JPEG_FMT_JCS_YUYV,      /* Y/U/Y/V */
    JPEG_FMT_JCS_CMYK,      /* C/M/Y/K */
    JPEG_FMT_JCS_YCCK,       /* Y/Cb/Cr/K */

    JPEG_YUV_420 = JPEG_FMT_JCS_YCbCr,
    JPEG_YUYV = JPEG_FMT_JCS_YUYV,
    JPEG_YUV_422,
    JPEG_YUV_444,
    FMT_ALIGN_32 = MAX_INT_FOR_ALIGN,
} jpeg_fmt_t;


typedef struct {
    jpeg_fmt_t fmt __attribute__((aligned (4)));
    int        width;
    int        height;
    uint8_t    *yBuf;
    uint8_t    *uvBuf;
    int        yStride;
    int        uvStride;
} yuv_buf_t;

typedef struct {
    uint32_t buf_addr;
    int32_t  buf_filled_len;
} jpeg_buf_t;


typedef struct {

    /* src img bufs */
    yuv_buf_t src_buf;

    /* output buf */
    uint32_t dst_buf;
    uint32_t dst_buf_size;

    /* jpeg quality: range 0~100 */
    uint32_t quality;

    /* rotation informaiton */
    uint32_t rotation;

    /* frame time stamp (sequential number), used for tracking/debug */
    int32_t frame_seq;

} jpeg_encode_params_t;


typedef struct {
	int32_t   rslt_type;    //NCPU_TO_SCPU_RESULT_TYPE

    jpeg_oper_sts_t sts __attribute__((aligned (4)));

    /* output buf */
    jpeg_buf_t dst_buf;

    /* frame time stamp, used for tracking/debug */
    int32_t frame_seq;

} jpeg_encode_result_t;

typedef struct {

    /* src img buf */
    jpeg_buf_t src_buf;

    jpeg_fmt_t out_fmt __attribute__((aligned (4)));
    uint32_t width;
    uint32_t height;

    /* output buf */
    uint32_t dst_yuv_buf;
    uint32_t dst_buf_size;

    /* gif buf generated for validation */
    uint32_t gif_buf;
    uint32_t gif_buf_size;

    /* restart for multiple pass decoding for motion jpeg */
    boolean  restart;

    /* kneron specific features enablement */
    kdp_jpeg_ftr_t ftr;

    /* frame time stamp, used for tracking/debug */
    int32_t frame_seq;
} jpeg_decode_params_t;

typedef struct {

	int32_t   rslt_type;    //NCPU_TO_SCPU_RESULT_TYPE

    jpeg_oper_sts_t sts __attribute__((aligned (4)));

    /* output buf */
    yuv_buf_t yuv_buf;

    /* gif buf generated for validation */
    jpeg_buf_t gif_buf;

    /* frame time stamp, used for tracking/debug */
    int32_t frame_seq;

} jpeg_decode_result_t;

#endif

