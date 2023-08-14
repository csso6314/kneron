/********************************************************************
* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
*
* The information contained herein is property of Kneron, Inc.
* Terms and conditions of usage are described in detail in Kneron
* STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information.
* NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
* from the file.
********************************************************************/

/**@addtogroup  KDEV_SENSOR  KDEV_SENSOR
* @{
* @brief        Kneron sensor device interface
*
* @copyright    Copyright (C) 2020 Kneron, Inc. All rights reserved.
*/

#ifndef __KDEV_SENSOR_H__
#define __KDEV_SENSOR_H__

#include "kdev_status.h"
#include "kmdw_sensor.h"

struct sensor_ops {
    kdev_status_t   (*s_power)     (uint32_t on);
    kdev_status_t   (*reset)       (void);
    kdev_status_t   (*s_stream)    (uint32_t enable);
    kdev_status_t   (*enum_fmt)    (uint32_t index, uint32_t *fourcc);
    kdev_status_t   (*get_fmt)     (struct cam_format *format);
    kdev_status_t   (*set_fmt)     (struct cam_format *format);
    kdev_status_t   (*set_gain)    (uint32_t gain1, uint32_t gain2);
    kdev_status_t   (*set_aec)     (struct cam_sensor_aec *aec_p);
    kdev_status_t   (*set_exp_time)(uint32_t gain1, uint32_t gain2);
    kdev_status_t   (*get_lux)     (uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average);
    kdev_status_t   (*led_switch)  (uint32_t on);
    kdev_status_t   (*set_mirror)  (uint32_t enable);
    kdev_status_t   (*set_flip)    (uint32_t enable);
    uint32_t        (*get_dev_id)  (void);
};

#endif /* __KDEV_SENSOR_H__ */
/** @}*/
