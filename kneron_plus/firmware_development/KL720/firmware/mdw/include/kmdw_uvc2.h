
/**
 * @file        kmdw_uvc2.h
 * @brief       uvc 2.0 APIs
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_UVC2_H__
#define __KMDW_UVC2_H__

#include "kmdw_usbh2.h"

typedef enum
{
    UVC_SET_CUR = 0x01,
    UVC_GET_CUR = 0x81,
    // others
} kmdw_uvc2_vs_request_t;

typedef enum
{
    UVC_VS_PROBE_CONTROL = 0x100,
    UVC_VS_COMMIT_CONTROL = 0x200,
    // others
} kmdw_uvc2_vs_control_selector_t;

typedef struct __attribute__((__packed__))
{
    uint16_t bmHint;
    uint8_t bFormatIndex;
    uint8_t bFrameIndex;
    uint32_t dwFrameInterval;
    uint16_t wKeyFrameRate;
    uint16_t wPFrameRate;
    uint16_t wCompQuality;
    uint16_t wCompWindowSize;
    uint16_t wDelay;
    uint32_t dwMaxVideoFrameSize;
    uint32_t dwMaxPayloadTransferSize;
} kmdw_uvc2_probe_commit_control_t;

typedef void (*kmdw_uvc2_get_frame_callback_t)(uint32_t *frame_ptr, uint32_t frame_size);

kmdw_usbh2_pipe_t kmdw_uvc2_isoch_create(uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval, kmdw_uvc2_get_frame_callback_t frame_cb);
kmdw_usbh2_status_t kmdw_uvc2_isoch_start(kmdw_usbh2_pipe_t pipe_hndl);
kmdw_usbh2_status_t kmdw_uvc2_isoch_stop(kmdw_usbh2_pipe_t pipe_hndl);
kmdw_usbh2_status_t kmdw_uvc2_vs_control(kmdw_uvc2_vs_request_t vs_req, kmdw_uvc2_vs_control_selector_t cs, kmdw_uvc2_probe_commit_control_t *upc_ctrl);
kmdw_usbh2_status_t kmdw_uvc2_queue_frame(kmdw_usbh2_pipe_t pipe, uint32_t *frame_ptr, uint32_t size);


#endif
