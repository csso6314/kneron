/*
 *  uvc_video.c  
 *
 * Copyright (C) 2019 - 2020 Kneron, Inc. All rights reserved.
 *
 */
#include "uvc_utils.h"
#include <kdp_usb.h>
#include <uvc_video.h>
#include <uvc_ctrl.h>
#include <uvc_dev.h>
#include "kmdw_console.h"
#include "kmdw_usbh.h"
#include "kmdw_uvc.h"
#include "kmdw_camera.h"
#include "media/kdp_inference.h"

#define NSEC_PER_SEC     1000000000L
#ifdef  KDP_UVC
#ifdef KDP_UVC_DEBUG
#define uvc_msg(fmt, ...) MSG(LOG_ERROR, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define uvc_msg(fmt, ...)
#endif

#define UINT_MAX (~0U)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

extern struct kapp_camera_settings {
    uint32_t width;             /**< Camera image width*/
    uint32_t height;            /**< Camera image hieght*/
    uint32_t pixelformat;       /**< Camera image pixel format*/
} cam_settings[IMGSRC_NUM];

static int uvc_find_frame_index(struct uvc_format *format,  uint16_t wWidth,	uint16_t wHeight)
{
    int i;

    for ( i = 0; i < format->nframes; i++) {
        struct uvc_frame *frame = &format->frame[i];
        if ((frame->wWidth == wWidth) && (frame->wHeight == wHeight))
            break;
    }
    if (i == format->nframes)
        return -1;
    format->cur_frame = &format->frame[i];
    format->cur_frame_num = i + 1;
    return 0;
}

static int uvc_find_best_alt(struct uvc_streaming *stream)
{
    struct uvc_vs_alt_intf *intf = stream->if_alt;
    int i;

    unsigned int bandwidth;

    uint16_t psize;
    unsigned int best_psize = UINT_MAX;

    uint8_t altsetting = 0;

    uint32_t interval = stream->cur_format->cur_frame->dwFrameInterval[0];

    bandwidth = stream->cur_format->cur_frame->wWidth
                * stream->cur_format->cur_frame->wHeight
                * stream->cur_format->bpp;

    bandwidth *= 10000000 / interval + 1;
    bandwidth /= 1000; // one usb
    bandwidth /= 8;
    bandwidth += 12;
    bandwidth = bandwidth > 1024? bandwidth : 1024;

    if (bandwidth == 0) {
        bandwidth = 1;
    }

    for (i = 0; i < stream->num_alt; i++) {
        psize = intf[i].maxpacketsize;
        psize = (psize & 0x07ff) * (1 + ((psize >> 11) & 3));
       
        if (psize >= bandwidth && psize <= best_psize) {
            altsetting = intf[i].alt_num;
            best_psize = psize;
            break;
        }
    }
    if (0 == altsetting)
        return -1;

    stream->vs_ctrl_info->curr->wmHint = 1;
    stream->vs_ctrl_info->curr->dwFrameInterval = interval;
    stream->vs_ctrl_info->curr->bFormatIndex = stream->cur_format->index;
    stream->vs_ctrl_info->curr->bFrameIndex = stream->cur_format->cur_frame->bFrameIndex;
    stream->curr_altnum = altsetting;
#ifdef KDP_UVC_DEBUG		
//    kmdw_printf("@@ stream->vs_ctrl_info->curr->wmHint %x\n", stream->vs_ctrl_info->curr->wmHint);
//    kmdw_printf("@@ stream->vs_ctrl_info->curr->dwFrameInterval %x\n", stream->vs_ctrl_info->curr->dwFrameInterval);
//    kmdw_printf("@@ stream->vs_ctrl_info->curr->bFormatIndex %x\n", stream->vs_ctrl_info->curr->bFormatIndex);
//    kmdw_printf("@@ stream->vs_ctrl_info->curr->bFrameIndex %x\n", stream->vs_ctrl_info->curr->bFrameIndex);
 //     kmdw_printf("@@ stream->curr_altnum %x\n",  stream->curr_altnum);  
  //  kmdw_printf("@@ intf[i].maxpacketsize 0x%x\n",  intf[i].maxpacketsize);
#endif
 
    return 0;
}

static int stream_negotiation(struct uvc_streaming *stream)
{
   
     if (0 != USBH_UVC_VS_Control(0, SET_CUR, VS_PROBE_CONTROL, (UVC_PROBE_COMMIT_CONTROL *) stream->vs_ctrl_info->curr))
        return -1;         
     if (0 != USBH_UVC_VS_Control(0, GET_MIN, VS_PROBE_CONTROL, (UVC_PROBE_COMMIT_CONTROL *) stream->vs_ctrl_info->minimum))
        return -1;         
     if (0 != USBH_UVC_VS_Control(0, GET_MAX, VS_PROBE_CONTROL, (UVC_PROBE_COMMIT_CONTROL *) stream->vs_ctrl_info->maximum))
        return -1;         
     if (0 != USBH_UVC_VS_Control(0, SET_CUR, VS_PROBE_CONTROL, (UVC_PROBE_COMMIT_CONTROL *) stream->vs_ctrl_info->curr))
        return -1;         
     if (0 != USBH_UVC_VS_Control(0, GET_CUR, VS_PROBE_CONTROL, (UVC_PROBE_COMMIT_CONTROL *) stream->vs_ctrl_info->curr))
        return -1;         
     if (0 != USBH_UVC_VS_Control(0, SET_CUR, VS_COMMIT_CONTROL, (UVC_PROBE_COMMIT_CONTROL *) stream->vs_ctrl_info->curr))
        return -1;

    return 0;
}

int uvc_video_disable(struct uvc_streaming *stream)
{
    if (stream->running == true) {

        USBH_Pipe_ISOCH_Stop(stream->isoch_pipe);
        stream->running = false;
    }
    return 0;
}

int uvc_video_enable(struct uvc_streaming *stream)
{
    int ret = 0;

    if (stream->running == false) {
        stream_negotiation(stream);
        USBH_DeviceRequest_SetInterface(0, stream->ifnum, stream->curr_altnum);
        USBH_UVC_PipeStart_Isoch(stream->isoch_pipe);
        stream->running = true;
    }
    return ret;
}

int uvc_video_init(struct uvc_device *udev)
{
    int i, j;
    struct uvc_format *format = (struct uvc_format *) NULL;

   for (i = 0; i < udev->num_vs_inf; i++) {
        for (j = 0; j  < udev->stream[i].nformats; j++) {
            format = &udev->stream[i].format[j];
            if (format->type == UVC_VS_FORMAT_UNCOMPRESSED)
                break;
        }
        if (j < udev->stream[i].nformats)
            break;
    }
   
    if ((j == udev->stream[i].nformats) || (i == udev->num_vs_inf))
        return -1;
    
    udev->curr_stream->cur_format = format;
    udev->curr_stream = &udev->stream[i];
    if (0 > uvc_find_frame_index(udev->curr_stream->cur_format, cam_settings[KDP_CAM_0].width,	cam_settings[KDP_CAM_0].height))
        return -1;
    if (0 > uvc_find_best_alt(udev->curr_stream))
        return -1;

    return 0;
}
#endif
