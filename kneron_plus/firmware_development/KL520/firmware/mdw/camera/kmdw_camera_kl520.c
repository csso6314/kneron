/*
 * KDP Camera driver for KL520
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <string.h>
#include "kmdw_camera.h"
#include "kmdw_sensor.h"
#include "kmdw_console.h"
#include "kdrv_clock.h"
#include "kdrv_dpi2ahb.h"
#include "kdrv_mipicsirx.h"

#define CAM_DEBUG

#ifdef CAM_DEBUG
#define cam_msg(fmt, ...) err_msg("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define cam_msg(fmt, ...)
#endif

struct kmdw_cam_context {
    uint32_t id;
    uint32_t sensor_id;
    uint32_t cam_input_type;
    uint32_t capabilities;
    struct cam_format fmt;
};

struct kmdw_cam_context cam_ctx[KDP_CAM_NUM];

/* API */
static kmdw_status_t kmdw_cam_open(uint32_t cam_idx)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_idx];

    if(ctx->cam_input_type != IMGSRC_IN_PORT_MIPI)
        return KMDW_STATUS_OK;
    cam_msg("cam: %d\n", cam_idx);
    kdrv_clock_set_csiclk(cam_idx, 1);
    kdrv_csi2rx_set_power(cam_idx, 1);
    kdrv_csi2rx_reset(cam_idx, ctx->sensor_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_close(uint32_t cam_idx)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_idx];
    if(ctx->cam_input_type != IMGSRC_IN_PORT_MIPI)
        return KMDW_STATUS_OK;
    cam_msg("cam: %d\n", cam_idx);
    kdrv_csi2rx_set_power(cam_idx, 0);
    kdrv_clock_set_csiclk(cam_idx, 0);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_query_capability(uint32_t cam_idx, struct cam_capability *cap)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_idx];

    cam_msg("cam: %d\n", cam_idx);

    ctx->capabilities = CAP_VIDEO_CAPTURE | CAP_STREAMING | CAP_DEVICE_CAPS;

    strcpy(cap->driver, "kl520_camera");
    strcpy(cap->desc, "kl520_camera");
    cap->version = 0x00010001;
    cap->capabilities = ctx->capabilities;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_format(uint32_t cam_idx, struct cam_format *format)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_idx];
    uint32_t bpp;

    ctx->fmt = *format;

    if (format->pixelformat == IMG_FORMAT_RGB565)
        bpp = 2;
    else if (format->pixelformat == IMG_FORMAT_RAW8)
    {
        if(cam_ctx[cam_idx].cam_input_type == IMGSRC_IN_PORT_DPI)
            bpp = 2;
        else
            bpp = 1;
    }

    ctx->fmt.sizeimage = format->width * format->height * bpp;

    cam_msg("cam %d: w=%d h=%d p=0x%x f=%d b=%d s=%d c=%d\n", cam_idx,
            ctx->fmt.width, ctx->fmt.height, ctx->fmt.pixelformat, ctx->fmt.field,
            ctx->fmt.bytesperline, ctx->fmt.sizeimage, ctx->fmt.colorspace);

    kdrv_dpi2ahb_enable(ctx->id, format);
    kdrv_csi2rx_enable(ctx->cam_input_type, ctx->id, ctx->sensor_id, format);
    kmdw_sensor_set_fmt(cam_idx, &ctx->fmt);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_get_format(uint32_t cam_idx, struct cam_format *format)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_idx];

    cam_msg("cam: %d\n", cam_idx);

    *format = ctx->fmt;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_init(uint32_t cam_idx, uint32_t buf_addr_0, uint32_t buf_addr_1)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_idx];

    cam_msg("cam %d: size=%d\n", cam_idx, ctx->fmt.sizeimage);
    
    kdrv_dpi2ahb_buf_init(cam_idx, buf_addr_0, buf_addr_1);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_start_capture(uint32_t cam_idx, kmdw_camera_callback_t img_cb)
{
    cam_msg("cam: %d\n", cam_idx);
    kdrv_csi2rx_start(cam_ctx[cam_idx].cam_input_type, cam_idx);
    kdrv_dpi2ahb_start((uint32_t)cam_idx, img_cb);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stop_capture(uint32_t cam_idx)
{
    cam_msg("cam: %d\n", cam_idx);
    kdrv_dpi2ahb_stop((uint32_t)cam_idx);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_prepare(uint32_t cam_idx)
{
    cam_msg("cam: %d\n", cam_idx);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_capture(uint32_t cam_idx, uint32_t *addr, uint32_t *size)
{
    cam_msg("cam: %d\n", cam_idx);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stream_on(uint32_t cam_idx)
{
    cam_msg("cam: %d\n", cam_idx);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stream_off(uint32_t cam_idx)
{
    cam_msg("cam: %d\n", cam_idx);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_gain(uint32_t cam_idx, uint32_t gain1, uint32_t gain2)
{
    cam_msg("cam: %d: gain1 %d, gain2 %d\n", cam_idx, gain1, gain2);

    return kmdw_sensor_set_gain(cam_idx, gain1, gain2);
}

static kmdw_status_t kmdw_cam_set_aec(uint32_t cam_idx, struct cam_sensor_aec *aec_p)
{
    cam_msg("cam: %d\n", cam_idx);

    return kmdw_sensor_set_aec(cam_idx, aec_p);
}

static kmdw_status_t kmdw_cam_set_exp_time(uint32_t cam_idx, uint32_t gain1, uint32_t gain2)
{
    cam_msg("cam: %d: gain1 %d, gain2 %d\n", cam_idx, gain1, gain2);

    return kmdw_sensor_set_exp_time(cam_idx, gain1, gain2);
}

static kmdw_status_t kmdw_cam_get_lux(uint32_t cam_idx, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average)
{
    cam_msg("cam: %d\n", cam_idx);

    return kmdw_sensor_get_lux(cam_idx, expo, pre_gain, post_gain, global_gain, y_average);
}

static kmdw_status_t kmdw_cam_led_switch(uint32_t cam_idx, uint32_t on)
{
    cam_msg("cam: %d\n", cam_idx);

    return kmdw_sensor_led_switch(cam_idx, on);
}

static kmdw_status_t kmdw_cam_set_mirror(uint32_t cam_idx, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_idx);

    kmdw_sensor_set_mirror(cam_idx, enable);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_flip(uint32_t cam_idx, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_idx);

    kmdw_sensor_set_flip(cam_idx, enable);

    return KMDW_STATUS_OK;
}

static uint32_t kmdw_cam_get_device_id(uint32_t cam_idx)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_idx);

    return kmdw_sensor_get_dev_id(cam_idx);
}

static struct cam_ops kdp520_camera_ops = {
    .open               = kmdw_cam_open,
    .close              = kmdw_cam_close,
    .query_capability   = kmdw_cam_query_capability,
    .set_format         = kmdw_cam_set_format,
    .get_format         = kmdw_cam_get_format,
    .buffer_init        = kmdw_cam_buffer_init,
    .start_capture      = kmdw_cam_start_capture,
    .stop_capture       = kmdw_cam_stop_capture,
    .buffer_prepare     = kmdw_cam_buffer_prepare,
    .buffer_capture     = kmdw_cam_buffer_capture,
    .stream_on          = kmdw_cam_stream_on,
    .stream_off         = kmdw_cam_stream_off,
    .set_gain           = kmdw_cam_set_gain,
    .set_aec            = kmdw_cam_set_aec,
    .set_exp_time       = kmdw_cam_set_exp_time,
    .get_lux            = kmdw_cam_get_lux,
    .led_switch         = kmdw_cam_led_switch,
    .set_mirror         = kmdw_cam_set_mirror,
    .set_flip           = kmdw_cam_set_flip,
    .get_device_id      = kmdw_cam_get_device_id,
};

static void kmdw_cam_clock_init()
{
    kdrv_clock_enable(CLK_PLL3);
    kdrv_clock_mgr_change_pll3_clock(/* mnp */ CAM_CLK_MS, CAM_CLK_NS, CAM_CLK_PS,
                                /* csi0 */ CSI0_TXESC, CSI0_CSI, CSI0_VC0,
                                /* csi1 */ CSI1_TXESC, CSI1_CSI, CSI1_VC0);

    kdrv_delay_us(10 * 5);
    kdrv_clock_enable(CLK_PLL3_OUT1);
    kdrv_clock_enable(CLK_PLL3_OUT2);
}
void kmdw_cam_mipi_init(uint32_t cam_idx)
{
    critical_msg("[%s] init  %d\n", __func__,cam_idx);
    kdrv_csi2rx_initialize(cam_idx);
}

void kmdw_cam_dpi_init(uint32_t cam_idx)
{
    critical_msg("[%s] init  %d\n", __func__);
    if(cam_idx == KDP_CAM_0)
    {
        SCU_EXTREG_MISC_SET_DPI_MUX_0_sel(1);
    }
    else if(cam_idx == KDP_CAM_1)
    {
        SCU_EXTREG_MISC_SET_DPI_MUX_1_sel(1);
    }
}

void kmdw_cam_port_init(uint32_t cam_idx)
{
    if(cam_ctx[cam_idx].cam_input_type == IMGSRC_IN_PORT_MIPI)
    {
        kmdw_cam_mipi_init(cam_idx);
    }
    else if(cam_ctx[cam_idx].cam_input_type == IMGSRC_IN_PORT_DPI)
    {
        kmdw_cam_dpi_init(cam_idx);
    }
}

kmdw_status_t kmdw_cam_kl520_init(void)
{
    uint32_t cam_id;
    cam_msg("init\n");
    for(cam_id = 0; cam_id < CAM_ID_MAX ; cam_id++)
    {
        cam_ctx[cam_id].cam_input_type= (cam_id)? IMGSRC_IN_1_PORT : IMGSRC_IN_0_PORT;
        if(cam_ctx[cam_id].cam_input_type != IMGSRC_IN_PORT_NONE)
        {
            cam_ctx[cam_id].id = cam_id;
            cam_ctx[cam_id].sensor_id = (cam_id)?IMGSRC_1_SENSORID: IMGSRC_0_SENSORID;
            kmdw_camera_controller_register(cam_id, &kdp520_camera_ops);
            kmdw_sensor_register(cam_id, cam_ctx[cam_id].sensor_id);
            kmdw_cam_port_init(cam_id);
            kdrv_dpi2ahb_initialize(cam_id);
        }
    }
    kmdw_cam_clock_init();

    return KMDW_STATUS_OK;
}
