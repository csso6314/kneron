/*
 * KDP Sensor driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "kdev_sensor.h"
#include "kdev_status.h"
#include "kmdw_console.h"

struct device_cam_sensor_context {
    sensor_ops           *ops;
};
static struct device_cam_sensor_context camera_sensor_ctx[IMGSRC_NUM];

sensor_ops*              kdev_cam_sensor_get_drv_ops(uint32_t cam_idx) { return camera_sensor_ctx[cam_idx].ops; }
void kdev_cam_sensor_set_drv_ops(uint32_t cam_idx, sensor_ops *ops)
{
#if IMGSRC_NUM > 0
    if (cam_idx < IMGSRC_NUM && ops)
    {
        camera_sensor_ctx[cam_idx].ops = ops;
    }
#endif
}

void kdev_cam_sensor_register(uint32_t cam_idx)
{
#if ((IMGSRC_IN_0 == YES) && (IMGSRC_0_IN_PORT == IMG_SRC_IN_PORT_MIPI))
    if (0 == cam_idx)
        kdev_cam_sensor_register_0(cam_idx);
#endif
#if (IMGSRC_IN_1 == YES && (IMGSRC_1_IN_PORT == IMG_SRC_IN_PORT_MIPI))
    if (1 == cam_idx)
        kdev_cam_sensor_register_1(cam_idx);
#endif
}

