#ifndef __KHOST_IPC_INF_APP_TINY_YOLO__
#define __KHOST_IPC_INF_APP_TINY_YOLO__

#include "khost_ipc_generic.h"

typedef struct khost_ipc_inf_tiny_yolo_send_s khost_ipc_inf_tiny_yolo_send_t;
typedef struct khost_ipc_inf_tiny_yolo_recv_s khost_ipc_inf_tiny_yolo_recv_t;

#define BOUNDING_BOX_MAX 10
#define TINY_YOLO_RAW_OFFSET (sizeof(struct khost_ipc_inf_tiny_yolo_recv_t))

/********************************************************/
/***************** send Tiny Yolo data ******************/
/********************************************************/
/*
 *       |    0    |    1    |    2    |    3    |
 *       -----------------------------------------
 *  0    |              total_size               |
 *       -----------------------------------------
 *  4    |              desc_type                |
 *       -----------------------------------------
 *  8    |              request_id               |
 *       -----------------------------------------
 * 12~27 |              image config             |
 *       -----------------------------------------
 * 28~75 |              model config             |
 *       -----------------------------------------
 */

struct khost_ipc_inf_tiny_yolo_send_s {
    /* 1. header */
    uint32_t                        total_size;
    uint32_t                        desc_type;    // see enum KHOST_DESC_TYPE

    /* 2. app specific part */
    uint32_t                        request_id;
    struct khost_ipc_image_config_s image;
    struct khost_ipc_model_config_s model;
} __attribute__ ((aligned (4)));

/********************************************************/
/****************** recv Tiny Yolo data *****************/
/********************************************************/
/*
 *       |    0    |    1    |    2    |    3    |
 *       -----------------------------------------
 *  0    |              total_size               |
 *       -----------------------------------------
 *  4    |              desc_type                |
 *       -----------------------------------------
 *  8    |              return_code              |
 *       -----------------------------------------
 * 12    |              request_id               |
 *       -----------------------------------------
 * 16    |     is_output_raw |                   |
 *       -----------------------------------------
 * 20    |              class_count              |
 *       -----------------------------------------
 * 24    |              box_count                |
 *       -----------------------------------------
 * 28    |              boxes[0].x1              |
 *       -----------------------------------------
 * 32    |              boxes[0].y1              |
 *       -----------------------------------------
 * 36    |              boxes[0].x2              |
 *       -----------------------------------------
 * 40    |              boxes[0].y2              |
 *       -----------------------------------------
 * 44    |              boxes[0].scroe           |
 *       ----------------------------------------
 * 48    |              boxes[0].class_num       |
 *       -----------------------------------------
 * 52~267|              boxes[...]               |
 *       -----------------------------------------
 * 268~n |              options:raw_output       |
 *       -----------------------------------------
 */

struct khost_ipc_inf_tiny_yolo_recv_s {
    /* 1. header */
    uint32_t                      total_size;
    uint32_t                      desc_type;    // see enum KHOST_DESC_TYPE
    uint32_t                      return_code;  // see enum KHOST_IPC_RETURN_CODE

    /* 2. app specific part */
    uint32_t                      request_id;
    bool                          is_output_raw;
    uint32_t                      class_count;
    uint32_t                      box_count;
    struct khost_bounding_box_s   boxes[BOUNDING_BOX_MAX];

    /* 3. raw output should append here*/
} __attribute__ ((aligned (4)));

#endif // __KHOST_IPC_INF_APP_TINY_YOLO__
