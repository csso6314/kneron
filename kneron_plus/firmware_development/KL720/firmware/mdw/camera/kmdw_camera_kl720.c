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
#include "kdrv_camera.h"

#define CAM_DEBUG

#ifdef CAM_DEBUG
#define cam_msg(fmt, ...) kmdw_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define cam_msg(fmt, ...)
#endif

kmdw_cam_context cam_ctx[KDP_CAM_NUM] =
{
    {
        .cam_input_type             = IMGSRC_0_IN_PORT,
        .sensor_id                  = IMGSRC_0_SENSORID,
        .sensor_devaddress          = IMGSRC_0_DEV_ADDR,
        .i2c_port_id                = IMGSRC_0_PORT_ID,
        .tile_avg_en                = IMGSRC_0_TILE_AVG,
        .mipi_lane_num              = IMGSRC_0_MIPI_LANE,
        .fmt=
        {
            .width                  = IMGSRC_0_WIDTH,
            .height                 = IMGSRC_0_HEIGHT,
            .pixelformat            = IMGSRC_0_FORMAT,
        },
        .csi_para_ =
        {
            .timer_count_number     = CSIRX_0_TCN,
            .hs_rx_timeout_value    = CSIRX_0_HRTV,
            .mapping_control        = CSIRX_0_MCR,
            .vstu                   = CSIRX_0_VSTU,
            .vstr                   = CSIRX_0_VSTR,
            .vster                  = CSIRX_0_VSTER,
            .hstr                   = CSIRX_0_HSTR,
            .pftr                   = CSIRX_0_PFTR,
            .phy_settle_cnt         = CSIRX_0_SETTLE_CNT
        },
        .d2a_para_ =
        {
            .d2a_input_sorce        = D2A_0_INPUT_SRC,
            .d2a_fifo_threshold     = D2A_0_FIFO_THRE,
            .d2a_drop_frame_num     = D2A_0_DROP_FRAME_NUM,
            .d2a_packet_type        = D2A_0_PACKET_TYPE,
            .d2a_data_align         = D2A_0_DATA_ALIGN,
            .d2a_vsync_polarity     = D2A_0_VSYNC_PL,
            .d2a_hsync_polarity     = D2A_0_HSYNC_PL,
            .d2a_tile_ave_en        = D2A_0_TILE_AVG_EN,
            .d2a_tile_ave_size      = D2A_0_TILE_AVG_SIZE,
        },
    },
    {
        .cam_input_type             = IMGSRC_1_IN_PORT,
        .sensor_id                  = IMGSRC_1_SENSORID,
        .sensor_devaddress          = IMGSRC_1_DEV_ADDR,
        .i2c_port_id                = IMGSRC_1_PORT_ID,
        .tile_avg_en                = IMGSRC_1_TILE_AVG,
        .mipi_lane_num              = IMGSRC_1_MIPI_LANE,
        .fmt=
        {
            .width                  = IMGSRC_1_WIDTH,
            .height                 = IMGSRC_1_HEIGHT,
            .pixelformat            = IMGSRC_1_FORMAT,
        },
        .csi_para_ =
        {
            .timer_count_number     = CSIRX_1_TCN,
            .hs_rx_timeout_value    = CSIRX_1_HRTV,
            .mapping_control        = CSIRX_1_MCR,
            .vstu                   = CSIRX_1_VSTU,
            .vstr                   = CSIRX_1_VSTR,
            .vster                  = CSIRX_1_VSTER,
            .hstr                   = CSIRX_1_HSTR,
            .pftr                   = CSIRX_1_PFTR,
            .phy_settle_cnt         = CSIRX_1_SETTLE_CNT
        },
        .d2a_para_ =
        {
            .d2a_input_sorce        = D2A_1_INPUT_SRC,
            .d2a_fifo_threshold     = D2A_1_FIFO_THRE,
            .d2a_drop_frame_num     = D2A_1_DROP_FRAME_NUM,
            .d2a_packet_type        = D2A_1_PACKET_TYPE,
            .d2a_data_align         = D2A_1_DATA_ALIGN,
            .d2a_vsync_polarity     = D2A_1_VSYNC_PL,
            .d2a_hsync_polarity     = D2A_1_HSYNC_PL,
            .d2a_tile_ave_en        = D2A_1_TILE_AVG_EN,
            .d2a_tile_ave_size      = D2A_1_TILE_AVG_SIZE,
        }
    }
};

__weak kdrv_status_t kdrv_csirx_enable(uint32_t csirx_idx, cam_format *format, uint32_t vstr0, uint32_t vster, uint32_t pftr)
{
    return KDRV_STATUS_OK;
}
__weak kdrv_status_t kdrv_csirx_start(uint32_t csirx_idx, uint32_t num)
{
    return KDRV_STATUS_OK;
}
__weak kdrv_status_t kdrv_csirx_set_para(uint32_t csirx_idx, cam_format *format, csi_para* para)
{
    return KDRV_STATUS_OK;
}
/* API */
static kmdw_status_t kmdw_cam_set_cam_port(uint32_t cam_id, uint32_t input_port_type);
static kmdw_status_t kmdw_cam_open(uint32_t cam_id)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];

    kmdw_cam_set_cam_port(cam_id, cam_ctx[cam_id].d2a_para_.d2a_input_sorce);
    kdrv_clock_set_csiclk(cam_id, 1);
    kdrv_csirx_set_enable(cam_id, 1);

    if(ctx->cam_input_type != IMG_SRC_IN_PORT_MIPI)
        return KMDW_STATUS_OK;
    kmdw_sensor_init(cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_close(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];
    if(ctx->cam_input_type != IMG_SRC_IN_PORT_MIPI)
        return KMDW_STATUS_OK;
    kdrv_clock_set_csiclk(cam_id, 0);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_query_capability(uint32_t cam_id, struct cam_capability *cap)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];

    cam_msg("cam: %d\n", cam_id);

    ctx->capabilities = CAP_VIDEO_CAPTURE | CAP_STREAMING | CAP_DEVICE_CAPS;

    strcpy(cap->driver, "kl720_camera");
    strcpy(cap->desc, "kl720_camera");
    cap->version = 0x00010001;
    cap->capabilities = ctx->capabilities;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_format(uint32_t cam_id, cam_format *format)
{
    kmdw_cam_context *ctx = &cam_ctx[cam_id];
    uint32_t bpp;

    ctx->fmt = *format;

    if (format->pixelformat == IMG_FORMAT_RGB565)
        bpp = 2;
    else if (format->pixelformat == IMG_FORMAT_RAW8)
    {
        if(ctx->cam_input_type == IMG_SRC_IN_PORT_DPI)
            bpp = 2;
        else
            bpp = 1;
    }
    if(ctx->sensor_id == SENSOR_ID_IRS2877C)
        ctx->fmt.sizeimage = format->width * format->height * 2;
    else
        ctx->fmt.sizeimage = format->width * format->height * bpp;

    cam_msg("cam %d: w=%d h=%d p=0x%x f=%d b=%d s=%d c=%d\n", cam_id,
            ctx->fmt.width, ctx->fmt.height, ctx->fmt.pixelformat, ctx->fmt.field,
            ctx->fmt.bytesperline, ctx->fmt.sizeimage, ctx->fmt.colorspace);


    kdrv_dpi2ahb_set_para(cam_id, &ctx->fmt, &ctx->d2a_para_);
    if(ctx->cam_input_type == IMG_SRC_IN_PORT_MIPI)
        kdrv_csirx_set_para( cam_id, &ctx->fmt, &ctx->csi_para_);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_get_format(uint32_t cam_id, cam_format *format)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];

    cam_msg("cam: %d\n", cam_id);

    *format = ctx->fmt;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_init(uint32_t cam_id, uint32_t buf_addr_0, uint32_t buf_addr_1)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];

    cam_msg("cam %d: size=%d\n", cam_id, ctx->fmt.sizeimage);

    kdrv_dpi2ahb_buf_init(cam_id, buf_addr_0, buf_addr_1);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_start_capture(uint32_t cam_id, kmdw_camera_callback_t img_cb)
{
    cam_msg("cam: %d\n", cam_id);

    if(cam_ctx[cam_id].cam_input_type == IMG_SRC_IN_PORT_MIPI)
        kdrv_csirx_start(cam_id, cam_ctx[cam_id].mipi_lane_num);
    kdrv_dpi2ahb_start(cam_id, img_cb);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stop_capture(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);

    if(cam_ctx[cam_id].cam_input_type == IMG_SRC_IN_PORT_MIPI)
    {
        kmdw_sensor_s_stream(cam_id, 0);
        kdrv_csirx_stop(cam_id);
    }
    kdrv_dpi2ahb_stop((uint32_t)cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_prepare(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_capture(uint32_t cam_id, uint32_t *addr, uint32_t *size)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stream_on(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stream_off(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_gain(uint32_t cam_id, uint32_t gain1, uint32_t gain2)
{
    cam_msg("cam: %d: gain1 %d, gain2 %d\n", cam_id, gain1, gain2);

    return kmdw_sensor_set_gain(cam_id, gain1, gain2);
}

static kmdw_status_t kmdw_cam_set_aec(uint32_t cam_id, struct cam_sensor_aec *aec_p)
{
    cam_msg("cam: %d\n", cam_id);

    return kmdw_sensor_set_aec(cam_id, aec_p);
}

static kmdw_status_t kmdw_cam_set_exp_time(uint32_t cam_id, uint32_t exp_time)
{
    cam_msg("cam: %d: exp_time %d\n", cam_id, exp_time);

    return kmdw_sensor_set_exp_time(cam_id, exp_time);
}

static kmdw_status_t kmdw_cam_get_lux(uint32_t cam_id, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average)
{
    cam_msg("cam: %d\n", cam_id);

    return kmdw_sensor_get_lux(cam_id, expo, pre_gain, post_gain, global_gain, y_average);
}

static kmdw_status_t kmdw_cam_led_switch(uint32_t cam_id, uint32_t on)
{
    cam_msg("cam: %d\n", cam_id);

    return kmdw_sensor_led_switch(cam_id, on);
}

static kmdw_status_t kmdw_cam_set_mirror(uint32_t cam_id, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_mirror(cam_id, enable);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_flip(uint32_t cam_id, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_flip(cam_id, enable);

    return KMDW_STATUS_OK;
}

static uint32_t kmdw_cam_get_device_id(uint32_t cam_id)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    return kmdw_sensor_get_dev_id(cam_id);
}

static kmdw_status_t kmdw_cam_get_expo(uint32_t cam_id)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    return kmdw_sensor_get_expo(cam_id);
}

static kmdw_status_t kmdw_cam_set_inc(uint32_t cam_id, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_inc(cam_id, enable);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_devaddress(uint32_t cam_id, uint32_t address, uint32_t port_id)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_devaddress(cam_id, address, port_id);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_clock()
{
    return KMDW_STATUS_OK;
}
static kmdw_status_t kmdw_cam_set_cam_port(uint32_t cam_id, uint32_t input_port_type)
{
    kdrv_dpi2ahb_src_config(cam_id, (dpi_src_opt) input_port_type);
    return KMDW_STATUS_OK;
}

cam_ops camera_ops = {
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
    .get_expo           = kmdw_cam_get_expo,
    .set_inc            = kmdw_cam_set_inc,
    .set_addr           = kmdw_cam_set_devaddress,
    .set_clock          = kmdw_cam_set_clock,
    .set_cam_port       = kmdw_cam_set_cam_port
};




