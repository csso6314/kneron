#ifndef __KHOST_IPC_INF_APP_FID_2D_H__
#define __KHOST_IPC_INF_APP_FID_2D_H__

#include "khost_ipc_generic.h"

typedef struct khost_ipc_inf_fid_2d_send_s khost_ipc_inf_fid_2d_send_t;
typedef struct khost_ipc_inf_fid_2d_recv_s khost_ipc_inf_fid_2d_recv_t;

enum KHOST_INF_FID_2D_MODE {
    KHOST_INF_FID_2D_NONE = 0,
    KHOST_INF_FID_2D_REGISTER = 1,
    KHOST_INF_FID_2D_VERIFY
};

#define FID_2D_RECV_SIZE (FD_RES_LENGTH + FR_RES_LENGTH + LM_RES_LENGTH + LV_RES_LENGTH)

struct khost_ipc_inf_fid_2d_register_s {
    uint32_t user_id;
    float    threshold;
    uint32_t image_idx;
    uint32_t res_mask;    // result mask. bit(0): FD result; bit(1): LM data; bit(2): FM feature map; bit(3): liveness
};

struct khost_ipc_inf_fid_2d_verify_s {
    float    threshold;
    uint32_t res_mask;    // result mask. bit(0): FD result; bit(1): LM data; bit(2): FM feature map; bit(3): liveness
};

/********************************************************/
/****************** send fid 2d data ********************/
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
 * 76    |              mode                     |
 *       -----------------------------------------    -----------------------------------------
 * 80    |              data.reg.user_id         |    |            data.verify.threshold      |
 *       -----------------------------------------    -----------------------------------------
 * 84    |              data.reg.threshold       |    |            data.verify.res_mask       |
 *       -----------------------------------------    -----------------------------------------
 * 88    |              data.reg.image_idx       |
 *       -----------------------------------------
 * 92    |              data.reg.res_mask        |
 *       -----------------------------------------
*/

struct khost_ipc_inf_fid_2d_send_s {
    /* 1. header */
    uint32_t                        total_size;
    uint32_t                        desc_type;    // see enum KHOST_DESC_TYPE

    /* 2. app specific part */
    uint32_t                        request_id;
    struct khost_ipc_image_config_s image;
    struct khost_ipc_model_config_s model;
    uint32_t                        mode;         // see enum KHOST_INF_FID_2D_MODE
    union {
        struct khost_ipc_inf_fid_2d_register_s reg;
        struct khost_ipc_inf_fid_2d_verify_s   verify;
    } data;
 } __attribute__ ((aligned (4)));

/********************************************************/
/******************* recv fid 2d data *******************/
/********************************************************/
/*
 *         |    0    |    1    |    2    |    3    |
 *         -----------------------------------------
 *  0      |              total_size               |
 *         -----------------------------------------
 *  4      |              desc_type                |
 *         -----------------------------------------
 *  8      |              return_code              |
 *         -----------------------------------------
 * 12      |              request_id               |
 *         -----------------------------------------
 * 16      |              res_mask                 |
 *         -----------------------------------------
 * 20      |              mode                     |
 *         -----------------------------------------
 * 24      |              user_id                  |
 *          -----------------------------------------
 * 28~1083 |              res                      |
 *         -----------------------------------------
 */

struct khost_ipc_inf_fid_2d_recv_s {
    /* 1. header */
    uint32_t                      total_size;
    uint32_t                      desc_type;                // see enum KHOST_DESC_TYPE
    uint32_t                      return_code;              // see enum KHOST_IPC_RETURN_CODE

    /* 2. app specific part */
    uint32_t                      request_id;
    uint32_t                      res_mask;
    uint32_t                      mode;                     // see enum KHOST_INF_FID_2D_MODE
    uint32_t                      user_id;                  // 0: not found
    char                          res[FID_2D_RECV_SIZE];    // post-process output for FD, FR, LM, and LV according by res_mask

} __attribute__ ((aligned (4)));

#endif    //__KHOST_IPC_INF_APP_FID_2D_H__
