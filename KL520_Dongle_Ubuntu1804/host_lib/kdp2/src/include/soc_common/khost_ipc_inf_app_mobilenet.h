#ifndef __KHOST_IPC_INF_APP_MOBILENET_H__
#define __KHOST_IPC_INF_APP_MOBILENET_H__

#include "khost_ipc_generic.h"

typedef struct khost_ipc_inf_mobilenet_send_s  khost_ipc_inf_mobilenet_send_t;
typedef struct khost_ipc_inf_mobilenet_recv_s  khost_ipc_inf_mobilenet_recv_t;

#define CROP_BOX_MAX 16
#define CLASSIFICATION_MAX 5
#define MOBILENET_RAW_OFFSET (sizeof(struct khost_ipc_inf_mobilenet_recv_t))

struct khost_crop_box_s {
    uint32_t  box_id;
    int32_t   top;
    int32_t   bottom;
    int32_t   left;
    int32_t   right;
};

struct khost_class_s {
    uint32_t index;    // index of the class
    float    score;    // probability score of the class
};

/********************************************************/
/***************** send MobileNet data ******************/
/********************************************************/
/*
 *         |    0    |    1    |    2    |    3    |
 *         -----------------------------------------
 *  0      |              total_size               |
 *         -----------------------------------------
 *  4      |              desc_type                |
 *         -----------------------------------------
 *  8      |              request_id               |
 *         -----------------------------------------
 * 12~27   |              image config             |
 *         -----------------------------------------
 * 28~75   |              model config             |
 *         -----------------------------------------
 * 76      |              box_count                |
 *         -----------------------------------------
 * 80      |              boxes[0].box_id          |
 *         -----------------------------------------
 * 84      |              boxes[0].top             |
 *         -----------------------------------------
 * 88      |              boxes[0].bottom          |
 *         -----------------------------------------
 * 92      |              boxes[0].left            |
 *         -----------------------------------------
 * 96      |              boxes[o].right           |
 *         -----------------------------------------
 * 100~399 |              boxes[...]               |
 *         -----------------------------------------
 */

struct khost_ipc_inf_mobilenet_send_s {
    /* 1. header */
    uint32_t                        total_size;
    uint32_t                        desc_type;    // see enum KHOST_DESC_TYPE

    /* 2. app specific part */
    uint32_t                        request_id;
    struct khost_ipc_image_config_s image;
    struct khost_ipc_model_config_s model;
    uint32_t                        box_count;    // 0: whole image
    struct khost_crop_box_s         boxes[CROP_BOX_MAX];
} __attribute__ ((aligned (4)));

/********************************************************/
/****************** recv MobileNet data *****************/
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
 * 20    |              box_id                   |
 *       -----------------------------------------
 * 24    |              class_count              |
 *       -----------------------------------------
 * 28    |              classes[0].index         |
 *       -----------------------------------------
 * 32    |              classes[0].score         |
 *       -----------------------------------------
 * 36~67 |              classes[...]             |
 *       -----------------------------------------
 * 68~n  |              options:raw_output       |
 *       -----------------------------------------
 */

struct khost_ipc_inf_mobilenet_recv_s {
    /* 1. header */
    uint32_t                      total_size;
    uint32_t                      desc_type;    // see enum KHOST_DESC_TYPE
    uint32_t                      return_code;  // see enum KHOST_IPC_RETURN_CODE

    /* 2. app specific part */
    uint32_t                      request_id;
    bool                          is_output_raw;
    uint32_t                      box_id;
    uint32_t                      class_count;
    struct khost_class_s          classes[CLASSIFICATION_MAX];

    /* 3. raw output should append here*/
} __attribute__ ((aligned (4)));

#endif // __KHOST_IPC_INF_APP_MOBILENET_H__
