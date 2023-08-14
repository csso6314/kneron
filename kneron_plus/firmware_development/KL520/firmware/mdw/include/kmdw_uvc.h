/**
 * @file        kmdw_uvc.h
 * @brief       Kneron usbh uvc control APIs
 *
 * @copyright   Copyright (c) 2022 Kneron Inc. All rights reserved.
 */

#ifndef __USBH_UVC_H__
#define __USBH_UVC_H__

typedef enum
{
    SET_CUR = 0x01,
    GET_CUR = 0x81,
    GET_MIN = 0x82,
    GET_MAX = 0x83,	
    // others
} UVC_VS_Request_t;

typedef enum
{
    VS_PROBE_CONTROL = 0x100,
    VS_COMMIT_CONTROL = 0x200,
    // others
} UVC_VS_ControlSelector_t;

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
} UVC_PROBE_COMMIT_CONTROL;

// Callback function called when UVC class device is connected and
extern uint8_t USBH_UVC_Configure(uint8_t device, const USB_DEVICE_DESCRIPTOR *ptr_dev_desc, const USB_CONFIGURATION_DESCRIPTOR *ptr_cfg_desc);

extern USBH_PIPE_HANDLE USBH_UVC_PipeCreate_Isoch(uint8_t device, uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval);

extern usbStatus USBH_UVC_PipeStart_Isoch(USBH_PIPE_HANDLE pipe_hndl);

extern usbStatus USBH_UVC_PipeStop_Isoch(USBH_PIPE_HANDLE pipe_hndl);

// Callback function called when custom class device is connected
extern usbStatus USBH_UVC_Initialize(uint8_t instance);

extern usbStatus USBH_UVC_VS_Control(uint8_t device, UVC_VS_Request_t vs_req, UVC_VS_ControlSelector_t cs, UVC_PROBE_COMMIT_CONTROL *upc_ctrl);

extern usbStatus USBH_UVC_Queue_Frame(USBH_PIPE_HANDLE pipe, uint32_t *frame_ptr, uint32_t size);

// Callback function called when a frame is complete
#ifdef KDP_UVC
extern void USBH_UVC_Get_Frame(uint32_t *frame_ptr, uint32_t *frame_size, int *index);
#else
void USBH_UVC_Get_Frame(uint32_t *frame_ptr, uint32_t frame_size);
#endif
#endif
