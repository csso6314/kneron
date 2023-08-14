/*
 * KDP Sensor driver header
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDEV_CAM_SENSOR_H__
#define __KDEV_CAM_SENSOR_H__

#include "project.h"
#include "base.h"
#include "kdev_status.h"
#include "kmdw_camera.h"
#include "kmdw_sensor.h"
typedef struct kdev_sensor_context {
    uint32_t sensor_type;
    uint32_t data_align_en;
    uint32_t tile_avg_en;
    uint32_t vstr0; // DPI VC0 V Sync Timing register
    uint32_t vster; // DPI V Sync Timing Extended register
    uint32_t pftr; // Pixel FIFO threshold register
    uint32_t addr; // slave address - NOTE: 7bit
    uint32_t device_id;
}kdev_sensor_context;

typedef struct sensor_ops {
    kdev_status_t   (*s_power)      (uint32_t on);
    kdev_status_t   (*reset)        (void);
    kdev_status_t   (*s_stream)     (uint32_t enable);
    kdev_status_t   (*enum_fmt)     (uint32_t index, uint32_t *fourcc);
    kdev_status_t   (*get_fmt)      (cam_format *format);
    kdev_status_t   (*set_fmt)      (cam_format *format);
    kdev_status_t   (*set_gain)     (uint32_t gain1, uint32_t gain2);
    kdev_status_t   (*set_aec)      (cam_sensor_aec *aec_p);
    kdev_status_t   (*set_exp_time) (uint32_t exp_time);
    kdev_status_t   (*get_lux)      (uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average);
    kdev_status_t   (*led_switch)   (uint32_t on);
    kdev_status_t   (*set_mirror)   (uint32_t enable);
    kdev_status_t   (*set_flip)     (uint32_t enable);
    kdev_status_t   (*set_fps)      (uint8_t fps);
    kdev_status_t   (*get_exp_time) (void);
    kdev_status_t   (*get_reg_data) (uint16_t nReg);
    kdev_status_t   (*set_inc) (uint8_t en);
    kdev_status_t   (*set_blc_mode) (uint8_t en);
    uint32_t        (*get_dev_id)   (void);
    kdev_status_t   (*set_addr)     (uint32_t address, uint32_t port_id);
    kdev_status_t   (*init)         (void);
}sensor_ops;

struct kdev_sensor_context* kdev_cam_sensor_get_context(uint32_t cam_idx);
sensor_ops* kdev_cam_sensor_get_drv_ops(uint32_t cam_idx); //Make sure this is only used within flash middleware
void kdev_cam_sensor_set_drv_ops(uint32_t sensor_idx, sensor_ops *ops);

#if (IMGSRC_IN_0 == YES)
extern void kdev_cam_sensor_register_0(uint32_t cam_idx);
#endif
#if (IMGSRC_IN_1 == YES)
extern void kdev_cam_sensor_register_1(uint32_t cam_idx);
#endif

#define KDEV_CAM_SENSOR_DRIVER_REGISTER(device_type, id, device_ops) \
    void kdev_##device_type##_register_##id(uint32_t i) { kdev_##device_type##_set_drv_ops(i, device_ops); }

/* device : camera sensor api */
void kdev_cam_sensor_register(uint32_t cam_idx);

kdev_status_t kdev_sensor_get_calibration_data(unsigned char *cali_data, uint8_t len);
#endif // __KDEV_CAM_SENSOR_H__
