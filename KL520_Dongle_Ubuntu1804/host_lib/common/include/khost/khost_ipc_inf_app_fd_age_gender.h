#ifndef __KHOST_IPC_INF_APP_FD_AGE_GENDER_H__
#define __KHOST_IPC_INF_APP_FD_AGE_GENDER_H__

#include "khost_ipc_generic.h"

typedef struct khost_ipc_inf_fd_age_gender_send_s khost_ipc_inf_fd_age_gender_send_t;
typedef struct khost_ipc_inf_fd_age_gender_recv_s khost_ipc_inf_fd_age_gender_recv_t;

#define FD_MAX 10

struct khost_age_gender_s {
    uint32_t age;
    uint32_t ismale;  // 0: female; 1: male
};

struct khost_fd_age_gender_s {
    struct khost_bounding_box_s box;
    struct khost_age_gender_s   ag;
};

/********************************************************/
/*************** send FD Age Gender data ****************/
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

struct khost_ipc_inf_fd_age_gender_send_s {
    /* 1. header */
    uint32_t                        total_size;
    uint32_t                        desc_type;    // see enum KHOST_DESC_TYPE

    /* 2. app specific part */
    uint32_t                        request_id;
    struct khost_ipc_image_config_s image;
    struct khost_ipc_model_config_s model;
} __attribute__ ((aligned (4)));

/********************************************************/
/**************** recv FD Age Gender data ***************/
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
 * 16    |              fd_count                 |
 *       -----------------------------------------
 * 20    |              fds[0].box.x1            |
 *       -----------------------------------------
 * 24    |              fds[0].box.y1            |
 *       -----------------------------------------
 * 28    |              fds[0].box.x2            |
 *       -----------------------------------------
 * 32    |              fds[0].box.y2            |
 *       -----------------------------------------
 * 36    |              fds[0].box.scroe         |
 *       ----------------------------------------
 * 40    |              fds[0].box.class_num     |
 *       -----------------------------------------
 * 44    |              fds[0].ag.age            |
 *       -----------------------------------------
 * 48    |              fds[0].ag.ismale         |
 *       -----------------------------------------
 * 52~339|              fds[...]                 |
 *       -----------------------------------------
 */

struct khost_ipc_inf_fd_age_gender_recv_s {
    /* 1. header */
    uint32_t                      total_size;
    uint32_t                      desc_type;     // see enum KHOST_DESC_TYPE
    uint32_t                      return_code;   // see enum KHOST_IPC_RETURN_CODE

    /* 2. app specific part */
    uint32_t                      request_id;
    uint32_t                      fd_count;      // face count
    struct khost_fd_age_gender_s  fds[FD_MAX];

    /* 3. raw output should append here*/
} __attribute__ ((aligned (4)));

#endif    //__KHOST_IPC_INF_APP_FD_AGE_GENDER_H__
