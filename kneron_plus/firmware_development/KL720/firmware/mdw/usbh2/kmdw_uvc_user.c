/*
 * Kneron USB UVC user driver
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"

#include "base.h"
#include "kmdw_console.h"
#include "kmdw_memory.h"
#include "kmdw_usbh2.h"
#include "kmdw_uvc2.h"
#include "kmdw_uvc_user.h"

/*----------------------------------------------------------------------------
*      Thread for usb host examples
*---------------------------------------------------------------------------*/

struct cam_user_config  cam_user_cfg = {0};

static kmdw_usbh2_pipe_t isoch_pipe;

static void uvc_frame_cb(uint32_t *frame_ptr, uint32_t frame_size)
{
    uint32_t buf_addr;

    if (frame_size != cam_user_cfg.img_size) {
        err_msg("[uvc_frame_cb] frame_size %u != %u\n", frame_size, cam_user_cfg.img_size);
    }

    if (cam_user_cfg.cam_frame_down_cb) {
        buf_addr = cam_user_cfg.cam_frame_down_cb((uint32_t)frame_ptr, frame_size);
        kmdw_uvc2_queue_frame(isoch_pipe, (uint32_t *)buf_addr, cam_user_cfg.img_size);
    }
}

static void device_configured_callback(const kmdw_usbh2_device_descriptor_t *ptr_dev_desc, const kmdw_usbh2_configuration_descriptor_t *ptr_cfg_desc)
{
    err_msg("[UVC config CB] VID/PID = %x/%x\n", ptr_dev_desc->idVendor, ptr_dev_desc->idProduct);

    if (ptr_dev_desc->idVendor != 0x1871 || ptr_dev_desc->idProduct != 0x0142)
    {
        err_msg("[UVC config CB] this is not AVEO USB camera, cannot configure it!\n");
        return;
    }

    // interface 1, alternate 5
    isoch_pipe = kmdw_uvc2_isoch_create(0x83, 0x13FC, 1, uvc_frame_cb);

    // Enqueue 2 buffers
    kmdw_uvc2_queue_frame(isoch_pipe, (uint32_t *)cam_user_cfg.init_addr1, cam_user_cfg.img_size);
    kmdw_uvc2_queue_frame(isoch_pipe, (uint32_t *)cam_user_cfg.init_addr2, cam_user_cfg.img_size);
    
    // set class interface 1 altr 0 for VideoStreaming interface
    kmdw_usbh2_set_interface(1, 0);

    kmdw_uvc2_probe_commit_control_t uvc_ctrl;

    uvc_ctrl.bmHint = 0x0001;
    uvc_ctrl.bFormatIndex = 1;
    uvc_ctrl.bFrameIndex = 1;
    uvc_ctrl.dwFrameInterval = 333333;
    uvc_ctrl.wKeyFrameRate = 0;
    uvc_ctrl.wPFrameRate = 0;
    uvc_ctrl.wCompQuality = 0;
    uvc_ctrl.wCompWindowSize = 0;
    uvc_ctrl.wDelay = 0;
    uvc_ctrl.dwMaxVideoFrameSize = 0;
    uvc_ctrl.dwMaxPayloadTransferSize = 0;

    // VideoStreaming request - UVC_SET_CUR - Probe Control
    kmdw_uvc2_vs_control(UVC_SET_CUR, UVC_VS_PROBE_CONTROL, &uvc_ctrl);

    // VideoStreaming request - UVC_GET_CUR - Probe Control
    kmdw_uvc2_vs_control(UVC_GET_CUR, UVC_VS_PROBE_CONTROL, &uvc_ctrl);

    // VideoStreaming request - UVC_SET_CUR - Commit Control
    kmdw_uvc2_vs_control(UVC_SET_CUR, UVC_VS_COMMIT_CONTROL, &uvc_ctrl);

    // set class interface 1 altr 5 to start video streaming
    kmdw_usbh2_set_interface(1, 5);

    kmdw_uvc2_isoch_start(isoch_pipe);

    // Send Ready Event
    osThreadFlagsSet(cam_user_cfg.notify_tid, cam_user_cfg.ready_evt);

    return;
}

static void device_disconnected_callback(void)
{
    osThreadFlagsSet(cam_user_cfg.notify_tid, cam_user_cfg.unready_evt);
}

#define UVC_START_EVENT     0x01
#define UVC_STOP_EVENT      0x02
#define UVC_TIMER_EVENT     0x100

static osThreadId_t tid_fake_uvc_user = NULL;
static int inited = 0;

static void fake_uvc_user_thread(void *argument)
{
    static int ping_pong = 0;
    uint32_t flags, timeout;

    // Self starter
    osThreadFlagsSet(tid_fake_uvc_user, UVC_START_EVENT);

    timeout = osWaitForever;
    for (;;) {
        flags = osThreadFlagsWait(UVC_START_EVENT | UVC_STOP_EVENT | UVC_TIMER_EVENT, osFlagsWaitAny, timeout);

        if (flags == osFlagsErrorTimeout) {
            osThreadFlagsSet(tid_fake_uvc_user, UVC_TIMER_EVENT);
            continue;
        }

        if (flags == UVC_STOP_EVENT) {
            timeout = osWaitForever;
            continue;
        }

        if (flags == UVC_START_EVENT) {
            osThreadFlagsSet(cam_user_cfg.notify_tid, cam_user_cfg.ready_evt);
            timeout = 30;
            continue;
        }

        if (flags != UVC_TIMER_EVENT)
            err_msg("Fake UVC user thread: unknown flags 0x%x\n", flags);

        if (ping_pong == 0) {
            cam_user_cfg.init_addr1 = cam_user_cfg.cam_frame_down_cb(cam_user_cfg.init_addr1, cam_user_cfg.img_size);
        } else {
            cam_user_cfg.init_addr2 = cam_user_cfg.cam_frame_down_cb(cam_user_cfg.init_addr2, cam_user_cfg.img_size);
        }
        ping_pong = !ping_pong;
    }
}

int kmdw_uvc_user_start(struct cam_user_config *cam_cfg_p)
{
    info_msg("[kmdw_uvc_user_start] initializing ...\n");

    cam_user_cfg = *cam_cfg_p;

    if (cam_user_cfg.ctrl_flags & CAM_FLAGS_LOOP_BACK) {
        if (!inited) {
            tid_fake_uvc_user = osThreadNew(fake_uvc_user_thread, NULL, NULL);
            osThreadSetPriority(tid_fake_uvc_user, osPriorityNormal3);
            inited = 1;
        } else {
            osThreadFlagsSet(tid_fake_uvc_user, UVC_START_EVENT);
        }
    } else {
        info_msg("[kmdw_uvc_user_start] please connect USB camera to the USB2 host port ...\n");

        // init USB host through usbh2 middleware
        kmdw_usbh2_status_t usb_status = kmdw_usbh2_initialize(device_configured_callback, device_disconnected_callback);
        if (usb_status != USBH_OK) {
            err_msg("[kmdw_uvc_user_start] USBH2 middleware initialization ... FAILED\n");
            return 0;
        }
    }

    return 1;
}

int kmdw_uvc_user_stop(void)
{
    if (cam_user_cfg.ctrl_flags & CAM_FLAGS_LOOP_BACK) {
        if (inited)
            osThreadFlagsSet(tid_fake_uvc_user, UVC_STOP_EVENT);
    } else {
//        kmdw_uvc2_isoch_stop(isoch_pipe);
//        kmdw_usbh2_isoch_delete(isoch_pipe);
        kmdw_usbh2_uninitialize();
    }

    return 1;
}
