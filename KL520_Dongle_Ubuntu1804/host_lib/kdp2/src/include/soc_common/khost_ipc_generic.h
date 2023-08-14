#ifndef __KHOST_IPC_GENERIC_H__
#define __KHOST_IPC_GENERIC_H__

#include <stdint.h>

#define KHOST_IPC_VERSION 2009

#define IMAGE_BUFFER_SIZE (2 * 1280 * 720 + 500) // image + descriptor
#define RESULT_BUFFER_SIZE (1 * 1024 * 1024)
#define MAX_QUEUE_NUM 3

#define IMAGE_NO_CROP 0
#define MAX_MODELS 10

enum KHOST_DESC_TYPE {
    KHOST_DESC_NONE = 0,

    KHOST_SYS_VERSION = 1,

    KHOST_INF_IMAGE_TINY_YOLO = 100,
    KHOST_INF_IMAGE_FID_2D,
    KHOST_INF_IMAGE_FD_AGE_GENGER,
    KHOST_INF_IMAGE_MOBILENET,
    KHOST_INF_IMAGE_OD,    // object detection
    KHOST_INF_IMAGE_PD,    // person detection
    KHOST_INF_IMAGE_USER = 1000,
    KHOST_INF_IMAGE_USER_DEFINED = 6666,
};

enum KHOST_IPC_RETURN_CODE {
    KHOST_IPC_RETURN_SUCCESS = 0,

    /* Generic */
    KHOST_IPC_ERROR_INVALID_MODEL_ID = 1,
    KHOST_IPC_ERROR_INVALID_DESCRIPTOR_TYPE,
    KHOST_IPC_ERROR_OVER_OUTPUT_SIZE,                  // need to hard reset Kneron device
    KHOST_IPC_ERROR_MEM_ALLOCATION_FAIL,               // need to hard reset Kneron device

    /* KAPP related */
    KHOST_IPC_ERROR_KAPP_ABORT,
    KHOST_IPC_ERROR_KAPP_ERR,

    /* FID-2D app related */
    KHOST_IPC_ERROR_FID_2D_INVALID_INF_MODE = 100,
    KHOST_IPC_ERROR_FID_2D_SMALL_FACE,                 /// face too small
    KHOST_IPC_ERROR_FID_2D_FDR_WRONG_USAGE,            /// KAPP_FID_CODES
    KHOST_IPC_ERROR_FID_2D_FDR_NO_FACE,                /// no face
    KHOST_IPC_ERROR_FID_2D_FDR_BAD_POSE,               /// base face pose
    KHOST_IPC_ERROR_FID_2D_FDR_FR_FAIL,                /// FR failed
    KHOST_IPC_ERROR_FID_2D_LM_LOW_CONFIDENT,           /// LM low confident
    KHOST_IPC_ERROR_FID_2D_LM_BLUR,                    /// LM blur
    KHOST_IPC_ERROR_FID_2D_RGB_Y_DARK,

    /* Database related */
    KHOST_IPC_ERROR_DB_ADD_USER_FAIL = 200,
    KHOST_IPC_ERROR_DB_NO_SPACE,      // db register
    KHOST_IPC_ERROR_DB_ALREADY_SAVED, // db register
    KHOST_IPC_ERROR_DB_NO_MATCH,      // db compare
    KHOST_IPC_ERROR_DB_REG_FIRST,     // db add
    KHOST_IPC_ERROR_DB_DEL_FAIL,      // db delete


    KHOST_IPC_ERROR_OTHER = 8888,                       // need to hard reset Kneron device
};

struct khost_bounding_box_s {
    float   x1;        // top-left corner: x
    float   y1;        // top-left corner: y
    float   x2;        // bottom-right corner: x
    float   y2;        // bottom-right corner: y
    float   score;     // probability score
    int32_t class_num; // class # (of many) with highest probability
};

/********************************************************/
/******************* send image config ******************/
/********************************************************/
/*
 *       |    0    |    1    |    2    |    3    |
 *       -----------------------------------------
 *  0    |                 width                 |
 *       -----------------------------------------
 *  4    |                 height                |
 *       -----------------------------------------
 *  8    |                 channel               |
 *       -----------------------------------------
 * 12    |                 config                |
 *       -----------------------------------------
 */
struct khost_ipc_image_config_s {
    uint32_t width;
    uint32_t height;
    uint32_t channel;
    uint32_t config;
};

/********************************************************/
/******************* send model config ******************/
/********************************************************/
/*
 *       |    0    |    1    |    2    |    3    |
 *       -----------------------------------------
 *  0    | use_builtin_model |                   |
 *       -----------------------------------------
 *  4    |               model_count             |
 *       -----------------------------------------
 *  8    |               model_ids[0]            |
 *       -----------------------------------------
 * 12~47 |               model_idx[...]          |
 *       -----------------------------------------
 */
struct khost_ipc_model_config_s {
    bool     use_builtin_model;        // true: use built-in models; false: use download models
    uint32_t model_count;
    uint32_t model_ids[MAX_MODELS];    // defined in model_type.h or user-defined id
} __attribute__ ((aligned (4)));

#endif // __KHOST_IPC_GENERIC_H__
