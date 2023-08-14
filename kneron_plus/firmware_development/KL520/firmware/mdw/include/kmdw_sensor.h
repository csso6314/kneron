/**
 * @file        kmdw_sensor.h
*  @brief       middleware for sensor device
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_SENSOR_H__
#define __KMDW_SENSOR_H__

#include <cmsis_os2.h>
#include "base.h"
#include "kmdw_camera.h"
#include "kdev_sensor.h"

#define fourcc(a, b, c, d) \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define PIX_FMT_YCBCR   fourcc('Y', 'B', 'Y', 'R')  /**< fourcc YBYR */
#define PIX_FMT_RGB565  fourcc('R', 'G', 'B', 'P')  /**< fourcc RGBP */
#define PIX_FMT_RAW10   fourcc('R', 'A', '1', '0')  /**< fourcc ra10 */
#define PIX_FMT_RAW8    fourcc('R', 'A', 'W', '8')  /**< fourcc raw8 */

enum colorspace {
    COLORSPACE_RGB          = 0,
    COLORSPACE_YUV          = 1,
    COLORSPACE_RAW          = 2,
};

struct sensor_device {
    uint16_t addr;                /* chip address - NOTE: 7bit */
};

struct sensor_init_seq {
    uint16_t addr;
    uint8_t value;
}__attribute__((packed));

struct sensor_datafmt_info {
    uint32_t fourcc;
    enum colorspace colorspace;
};

struct sensor_win_size {
    uint32_t width;
    uint32_t height;
};

/**
 * @brief       set sensor power function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   on          power on/off, 1:on, 0:off
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_s_power(uint32_t cam_idx, uint32_t on);

/**
 * @brief       sensor reset function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_reset(uint32_t cam_idx);

/**
 * @brief       set sensor stream function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   enable      stream enable/disable, 1:enable, 0:disable
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_s_stream(uint32_t cam_idx, uint32_t enable);

/**
 * @brief       set sensor enum function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   index       index
 * @param[out]   fourcc      point of fourcc
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_enum_fmt(uint32_t cam_idx, uint32_t index, uint32_t *fourcc);

/**
 * @brief       set sensor format function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   format      point of cam_format
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_set_fmt(uint32_t cam_idx, struct cam_format *format);

/**
 * @brief       get sensor format function
 *
 * @param[in]   cam_idx     camera id
 * @param[out]  format      point of cam_format
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_get_fmt(uint32_t cam_idx, struct cam_format *format);

/**
 * @brief       get sensor gain function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   gain1       aec gain parameter 1
 * @param[in]   gain2       aec gain parameter 2
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_set_gain(uint32_t cam_idx, uint32_t gain1, uint32_t gain2);

/**
 * @brief       sensor set ae controller ROI area function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   aec_p       point of cam_sensor_aec
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_set_aec(uint32_t cam_idx, struct cam_sensor_aec *aec_p);

/**
 * @brief       sensor set exposure time function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   gain1       exposure time parameter 1
 * @param[in]   gain2       exposure time parameter 2
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_set_exp_time(uint32_t cam_idx, uint32_t gain1, uint32_t gain2);

/**
 * @brief       sensor get lum and other parameter function
 *
 * @param[in]   cam_idx     camera id
 * @param[out]  expo        exposure time parameter
 * @param[out]  pre_gain    exposure time parameter
 * @param[out]  post_gain   exposure time parameter
 * @param[out]  global_gain exposure time parameter
 * @param[in]   y_average   exposure time parameter 2
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_get_lux(uint32_t cam_idx, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average);

/**
 * @brief       sensor set nir led on/off function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   on          LED on/off cmd, 1: on, 0:off
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_led_switch(uint32_t cam_idx, uint32_t on);

/**
 * @brief       sensor set image mirror on/off function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   enable      enable/disable cmd, 1: enable, 0:disable
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_set_mirror(uint32_t cam_idx, uint32_t enable);

/**
 * @brief       sensor set image flip on/off function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   enable      enable/disable cmd, 1: enable, 0:disable
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_set_flip(uint32_t cam_idx, uint32_t enable);

/**
 * @brief       sensor get device ID function
 *
 * @param[in]   cam_idx     camera id
 * @return      sensor id
 */
uint32_t kmdw_sensor_get_dev_id(uint32_t cam_idx);

/**
 * @brief       register sensor
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   sensor_idx  sensor id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_sensor_register(uint32_t cam_idx, uint32_t sensor_idx);
#endif // __KMDW_SENSOR_H__
