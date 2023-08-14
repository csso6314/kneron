/*
 * KDP Sensor driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "board.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "kmdw_sensor.h"

#if defined(BOARD_96) || defined(BOARD_DVP_EXAMPLE)
extern struct sensor_ops* kdev_sensor_gc2145_get_ops(void);
extern struct sensor_ops* kdev_sensor_sc132gs_get_ops(void);
#endif

struct kmdw_sensor_s {
    uint32_t            inuse;
    struct sensor_ops   *ops;
} sensor_s[CAM_ID_MAX];


kmdw_status_t kmdw_sensor_s_power(uint32_t cam_idx, uint32_t on)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->s_power == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->s_power(on);
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

kmdw_status_t kmdw_sensor_set_fmt(uint32_t cam_idx, struct cam_format *format)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].ops->set_fmt == NULL)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].inuse == 0)
        return KMDW_STATUS_ERROR;
    return (kmdw_status_t)sensor_s[cam_idx].ops->set_fmt(format);
}

kmdw_status_t kmdw_sensor_get_fmt(uint32_t cam_idx, struct cam_format *format)
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

kmdw_status_t kmdw_sensor_set_exp_time(uint32_t cam_idx, uint32_t gain1, uint32_t gain2)
{
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;

    if (sensor_s[cam_idx].ops->set_exp_time == NULL)
        return KMDW_STATUS_ERROR;

    return (kmdw_status_t)sensor_s[cam_idx].ops->set_exp_time(gain1, gain2);
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
struct sensor_ops *  kmdw_sensor_get_ops(uint32_t sensor_idx)
{
    struct sensor_ops *pops = NULL;
    if (sensor_idx >= SENSOR_ID_MAX)
        return NULL;
    switch(sensor_idx)
    {
        case SENSOR_ID_HMX2056:
            break;
        case SENSOR_ID_OV9286:
            break;
        case SENSOR_ID_HMXRICA:
            break;
        case SENSOR_ID_GC2145:
            pops = kdev_sensor_gc2145_get_ops();
            break;
        case SENSOR_ID_SC132GS:
            pops = kdev_sensor_sc132gs_get_ops();
            break;
        default:
            break;
    }
    return pops;
}
kmdw_status_t kmdw_sensor_register(uint32_t cam_idx, uint32_t sensor_idx)
{
    struct sensor_ops   *pops;
    if (cam_idx >= CAM_ID_MAX)
        return KMDW_STATUS_ERROR;
    if (sensor_idx >= SENSOR_ID_MAX)
        return KMDW_STATUS_ERROR;
    pops =  kmdw_sensor_get_ops(sensor_idx);
    if(pops == NULL)
        return KMDW_STATUS_ERROR;
    if (sensor_s[cam_idx].inuse)
        return KMDW_STATUS_ERROR;

    sensor_s[cam_idx].ops = pops;
    sensor_s[cam_idx].inuse = 1;

    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_sensor_unregister(uint32_t cam_idx, struct sensor_ops *sensor_ops_p)
{
    if (cam_idx >= CAM_ID_MAX || sensor_ops_p == NULL)
        return KMDW_STATUS_ERROR;

    if (!sensor_s[cam_idx].inuse)
        return KMDW_STATUS_ERROR;

    sensor_s[cam_idx].ops = NULL;
    sensor_s[cam_idx].inuse = 0;

    return KMDW_STATUS_OK;
}
