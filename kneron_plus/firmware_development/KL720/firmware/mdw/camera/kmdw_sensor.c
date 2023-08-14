/*
 * KDP Sensor driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

//#include "board_kl520.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "kmdw_sensor.h"

struct kmdw_sensor_s {
    uint32_t            inuse;
    uint32_t            i2c_port_id;
    uint32_t            dev_address;
    sensor_ops          *ops;
} sensor_s[CAM_ID_MAX];

__weak sensor_ops*              kdev_cam_sensor_get_drv_ops(uint32_t cam_idx) { return NULL; };
__weak void kdev_cam_sensor_register(uint32_t cam_idx){};
kmdw_status_t kmdw_sensor_s_power(uint32_t cam_idx, uint32_t on)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->s_power == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->s_power(on);
}

kmdw_status_t kmdw_sensor_init(uint32_t cam_idx)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].ops->init == NULL)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].inuse == 0)
        return KMDW_STATUS_ERROR;
    return (kmdw_status_t)sensor_s[cam_idx].ops->init();
}

kmdw_status_t kmdw_sensor_reset(uint32_t cam_idx)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->reset == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->reset();
}

kmdw_status_t kmdw_sensor_s_stream(uint32_t cam_idx, uint32_t enable)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->s_stream == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->s_stream(enable);
}

kmdw_status_t kmdw_sensor_enum_fmt(uint32_t cam_idx, uint32_t index, uint32_t *fourcc)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->enum_fmt == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->enum_fmt(index, fourcc);
}

kmdw_status_t kmdw_sensor_set_fmt(uint32_t cam_idx, cam_format *format)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].ops->set_fmt == NULL)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].inuse == 0)
        return KMDW_STATUS_ERROR;
    return (kmdw_status_t)sensor_s[cam_idx].ops->set_fmt(format);
}

kmdw_status_t kmdw_sensor_get_fmt(uint32_t cam_idx, cam_format *format)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->get_fmt == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->get_fmt(format);
}
kmdw_status_t kmdw_sensor_set_gain(uint32_t cam_idx, uint32_t gain1, uint32_t gain2)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_gain == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_gain(gain1, gain2);
}

kmdw_status_t kmdw_sensor_set_aec(uint32_t cam_idx, struct cam_sensor_aec *aec_p)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_aec == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_aec(aec_p);
}

kmdw_status_t kmdw_sensor_set_exp_time(uint32_t cam_idx, uint32_t exp_time)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_exp_time == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_exp_time(exp_time);
}

kmdw_status_t kmdw_sensor_get_lux(uint32_t cam_idx, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->get_lux == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->get_lux(expo, pre_gain, post_gain, global_gain, y_average);
}

kmdw_status_t kmdw_sensor_led_switch(uint32_t cam_idx, uint32_t on)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->led_switch == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->led_switch(on);
}

kmdw_status_t kmdw_sensor_set_mirror(uint32_t cam_idx, uint32_t enable)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_mirror == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_mirror(enable);
}

kmdw_status_t kmdw_sensor_set_flip(uint32_t cam_idx, uint32_t enable)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_flip == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_flip(enable);
}

uint32_t kmdw_sensor_get_dev_id(uint32_t cam_idx)
{
    if (cam_idx >= CAM_ID_MAX)
        return 1;

    if (sensor_s[cam_idx].ops->get_dev_id == NULL)
        return 1;

    return sensor_s[cam_idx].ops->get_dev_id();
}

kmdw_status_t kmdw_sensor_get_expo(uint32_t cam_idx)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->get_exp_time == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->get_exp_time();
}

kmdw_status_t kmdw_sensor_set_inc(uint32_t cam_idx, uint32_t enable)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_inc == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_inc(enable);
}

kmdw_status_t kmdw_sensor_set_devaddress(uint32_t cam_idx, uint32_t address, uint32_t port_id)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_addr == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_addr(address, port_id);
}

sensor_ops *  kmdw_sensor_get_ops(uint32_t sensor_idx)
{
    return 0;
}

kmdw_status_t kmdw_sensor_register(uint32_t cam_idx, kmdw_cam_context* cam_ctx)
{
    sensor_ops   *pops;
    kdev_cam_sensor_register(cam_idx);
    pops =  kdev_cam_sensor_get_drv_ops(cam_idx);
    if(pops == NULL)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].inuse)
        return KMDW_STATUS_ERROR;

    sensor_s[cam_idx].ops           = pops;
    sensor_s[cam_idx].dev_address   = cam_ctx->sensor_devaddress;
    sensor_s[cam_idx].i2c_port_id   = cam_ctx->i2c_port_id;
    sensor_s[cam_idx].inuse         = 1;
    kmdw_sensor_set_devaddress(cam_idx, sensor_s[cam_idx].dev_address, sensor_s[cam_idx].i2c_port_id);
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_sensor_unregister(uint32_t cam_idx, sensor_ops *sensor_ops_p)
{
    if (cam_idx >= CAM_ID_MAX || sensor_ops_p == NULL)
        return KMDW_STATUS_ERROR;

    if (!sensor_s[cam_idx].inuse)
        return KMDW_STATUS_ERROR;

    sensor_s[cam_idx].ops           = NULL;
    sensor_s[cam_idx].dev_address   = NULL;
    sensor_s[cam_idx].i2c_port_id   = NULL;
    sensor_s[cam_idx].inuse         = 0;

    return KMDW_STATUS_OK;
}
