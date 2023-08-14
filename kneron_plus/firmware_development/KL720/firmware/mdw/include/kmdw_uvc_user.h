/**
 * @file        kmdw_uvc_user.h
 * @brief       Kneron USB UVC user header
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_UVC_USER_H__
#define __KMDW_UVC_USER_H__

typedef uint32_t (*frame_done_cb)(uint32_t frame_ptr, uint32_t frame_size);

#define CAM_FLAGS_LOOP_BACK     BIT0  /**< flag of CAM_FLAGS_LOOP_BACK */

struct cam_user_config {
    uint32_t        ctrl_flags;
    osThreadId_t    notify_tid;
    uint32_t        ready_evt;
    uint32_t        unready_evt;
    int             img_size;
    uint32_t        init_addr1;
    uint32_t        init_addr2;
    frame_done_cb   cam_frame_down_cb;
};

int kmdw_uvc_user_start(struct cam_user_config *cam_cfg_p);
int kmdw_uvc_user_stop(void);


#endif
