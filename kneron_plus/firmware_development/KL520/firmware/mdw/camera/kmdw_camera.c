/*
 * KDP Camera driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "board.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include "kmdw_camera.h"
#ifdef KDP_UVC
#include "uvc_dev.h"
#endif

struct kmdw_camera_s {
    uint32_t        inuse;
    struct cam_ops  *ops;
} camera_s[IMGSRC_NUM];

#ifdef KL520
extern kmdw_status_t kmdw_cam_kl520_init(void);
#endif

kmdw_status_t kmdw_camera_init(void)
{
#ifdef KDP_UVC
    kmdw_cam_uvc_init();
#else
#ifdef KL520
    kmdw_cam_kl520_init();
#endif
#endif
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_camera_open(uint32_t cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->open == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->open(cam_idx);
}

kmdw_status_t kmdw_camera_close(uint32_t cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->close == NULL)
        return KMDW_STATUS_ERROR;

    camera_s[cam_idx].ops->close(cam_idx);
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_camera_get_device_info(uint32_t cam_idx, struct cam_capability *cap)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->query_capability == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->query_capability(cam_idx, cap);
}

kmdw_status_t kmdw_camera_set_frame_format(uint32_t cam_idx, struct cam_format *format)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->set_format == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->set_format(cam_idx, format);
}

kmdw_status_t kmdw_camera_get_frame_format(uint32_t cam_idx, struct cam_format *format)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->get_format == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->get_format(cam_idx, format);
}

kmdw_status_t kmdw_camera_buffer_init(uint32_t cam_idx, uint32_t buf_addr_0, uint32_t buf_addr_1)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->buffer_init == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->buffer_init(cam_idx, buf_addr_0, buf_addr_1);
}

kmdw_status_t kmdw_camera_start(uint32_t cam_idx, kmdw_camera_callback_t img_cb)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->start_capture == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->start_capture(cam_idx, img_cb);
}

kmdw_status_t kmdw_camera_stop(uint32_t cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->stop_capture == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->stop_capture(cam_idx);
}

kmdw_status_t kmdw_camera_buffer_prepare(uint32_t cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->buffer_prepare == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->buffer_prepare(cam_idx);
}

kmdw_status_t kmdw_camera_buffer_capture(uint32_t cam_idx, uint32_t *addr, uint32_t *size)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->buffer_capture == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->buffer_capture(cam_idx, addr, size);
}

kmdw_status_t kmdw_camera_stream_on(uint32_t cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->stream_on == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->stream_on(cam_idx);
}

kmdw_status_t kmdw_camera_stream_off(uint32_t cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->stream_off == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->stream_off(cam_idx);
}

kmdw_status_t kmdw_camera_set_gain(uint32_t cam_idx, uint32_t gain1, uint32_t gain2)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->set_gain == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->set_gain(cam_idx, gain1, gain2);
}

kmdw_status_t kmdw_camera_set_aec(uint32_t cam_idx, struct cam_sensor_aec *aec_p)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->set_aec == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->set_aec(cam_idx, aec_p);
}

kmdw_status_t kmdw_camera_set_exp_time(uint32_t cam_idx, uint32_t gain1, uint32_t gain2)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->set_exp_time == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->set_exp_time(cam_idx, gain1, gain2);
}

kmdw_status_t kmdw_camera_get_lux(uint32_t cam_idx, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->get_lux == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->get_lux(cam_idx, expo, pre_gain, post_gain, global_gain, y_average);
}

kmdw_status_t kmdw_camera_led_switch(uint32_t cam_idx, uint32_t on)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->led_switch == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->led_switch(cam_idx, on);
}

kmdw_status_t kmdw_camera_set_mirror(uint32_t cam_idx, uint32_t enable)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->set_mirror == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->set_mirror(cam_idx, enable);
}

kmdw_status_t kmdw_camera_set_flip(uint32_t cam_idx, uint32_t enable)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->set_flip == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->set_flip(cam_idx, enable);
}

uint32_t kmdw_camera_get_device_id(uint32_t cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->get_device_id == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->get_device_id(cam_idx);
}

kmdw_status_t kmdw_camera_ioctl(uint32_t cam_idx, uint32_t cid, void *data, uint16_t len)
{
    if (cam_idx >= IMGSRC_NUM)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].ops->ioctl == NULL)
        return KMDW_STATUS_ERROR;

    return camera_s[cam_idx].ops->ioctl(cam_idx, cid, data, len);
}

kmdw_status_t kmdw_camera_controller_register(uint32_t cam_idx, struct cam_ops *cam_ops_p)
{
    if (cam_idx >= IMGSRC_NUM || cam_ops_p == NULL)
        return KMDW_STATUS_ERROR;

    if (camera_s[cam_idx].inuse)
        return KMDW_STATUS_ERROR;

    camera_s[cam_idx].ops = cam_ops_p;
    camera_s[cam_idx].inuse = 1;

    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_camera_controller_unregister(uint32_t cam_idx, struct cam_ops *cam_ops_p)
{
    if (cam_idx >= IMGSRC_NUM || cam_ops_p == NULL)
        return KMDW_STATUS_ERROR;

    if (!camera_s[cam_idx].inuse)
        return KMDW_STATUS_ERROR;

    camera_s[cam_idx].ops = NULL;
    camera_s[cam_idx].inuse = 0;

    return KMDW_STATUS_OK;
}


