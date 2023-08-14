#ifndef __KHOST_IPC_INF_USER__
#define __KHOST_IPC_INF_USER__

#include "khost_ipc_generic.h"

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

typedef struct
{
    /* 1. header */
    uint32_t total_size;
    uint32_t desc_type; // see enum KHOST_DESC_TYPE

    /* 2. app specific part */
    uint32_t request_id;
    struct khost_ipc_image_config_s image;
    struct khost_ipc_model_config_s model;
} __attribute__((aligned(4))) khost_ipc_inf_user_send_t;

typedef struct
{
    /* 1. header */
    uint32_t                      total_size;
    uint32_t                      desc_type;    // see enum KHOST_DESC_TYPE
    uint32_t                      return_code;  // see enum KHOST_IPC_RETURN_CODE

    /* 2. app specific part */
    uint32_t                      request_id;
    bool                          is_output_raw;

} __attribute__((aligned(4))) khost_ipc_inf_user_recv_t;

#endif
