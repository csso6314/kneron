/**
 * @file        kmdw_camera.h
 * @brief       Kneron camera middleware for camera driver
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */


#ifndef __KMDW_CAMERA_H__
#define __KMDW_CAMERA_H__

#include "base.h"
#include "kmdw_status.h"
#include "kdrv_camera.h"
#include "kdrv_mipicsirx.h"
#include "kdrv_dpi2ahb.h"

#define CAP_VIDEO_CAPTURE   0x00000001  /**< Is a video capture device */
#define CAP_STREAMING       0x00000002  /**< can stream on/off */
#define CAP_DEVICE_CAPS     0x00000004  /**< can query capabilities */

enum {
    CID_SCANNING_MODE = 0x1,
    CID_AUTO_EXPOSURE_MODE,
    CID_AUTO_EXPOSURE_PRIORITY,
    CID_EXPOSURE_TIME_ABSOLUTE,
    CID_EXPOSURE_TIME_RELATIVE,
    CID_FOCUS_ABSOLUTE,
    CID_FOCUS_RELATIVE,
    CID_IRIS_ABSOLUTE,
    CID_IRIS_RELATIVE,
    CID_ZOOM_ABSOLUTE,
    CID_ZOOM_RELATIVE,
    CID_PANTILT_ABSOLUTE,
    CID_PANTILT_RELATIVE,
    CID_ROLL_ABSOLUTE,
    CID_ROLL_RELATIVE,
    CID_FOCUS_AUTO,
    CID_PRIVACY,
    CID_FOCUS_SIMPLE,
    CID_DIGITAL_WINDOW,
    CID_REGION_OF_INTEREST,
    CID_BRIGHTNESS,
    CID_CONTRAST,
    CID_HUE,
    CID_SATURATION,
    CID_SHARPNESS,
    CID_GAMMA,
    CID_WHITE_BALANCE_TEMPERATURE,
    CID_WHITE_BALANCE_COMPONENT,
    CID_BACKLIGHT_COMPENSATION,
    CID_GAIN,
    CID_POWER_LINE_FREQUENCY,
    CID_HUE_AUTO,
    CID_WHITE_BALANCE_TEMPERATURE_AUTO,
    CID_WHITE_BALANCE_COMPONENT_AUTO,
    CID_DIGITAL_MULTIPLIER,
    CID_DIGITAL_MULTIPLIER_LIMIT,
    CID_CONTRAST_AUTO,
    CID_LIST_ALL = 0xFF,
};

enum {
    KDP_CAM_0,
    KDP_CAM_1,
    KDP_CAM_NUM,    // = IMGSRC_NUM
};

enum camera_state {
    CAMERA_STATE_IDLE = 0,
    CAMERA_STATE_INITED,
    CAMERA_STATE_RUNNING,
    CAMERA_STATE_IN_FDR_INFERENCE,
    CAMERA_STATE_IN_FDR_REGISTRATION,
    CAMERA_STATE_IN_FDR_AUTO_REGISTRATION,
    CAMERA_STATE_IN_FDR_REGISTRATION_CONFIRM,
    CAMERA_STATE_IN_FDR_BOTH_REGISTRATION,
    CAMERA_STATE_IN_FDR_BOTH_REGISTRATION_CONFIRM,
    CAMERA_STATE_IN_FDR_BOTH_INFERENCE,
    CAMERA_STATE_IN_FDR_REGISTRATION_POSE_JUSTIFY,
};

typedef struct kmdw_cam_context {
    uint32_t cam_input_type;
    uint32_t sensor_id;
    uint32_t sensor_devaddress;
    uint32_t i2c_port_id;
    uint32_t capabilities;
    uint32_t tile_avg_en;       //TODO: Need move to d2a
    uint32_t mipi_lane_num;     //TODO: Need move to csirx
    cam_format fmt;
    csi_para csi_para_;
    d2a_para d2a_para_;
}kmdw_cam_context;

typedef struct cam_capability {
    char driver[16];
    char desc[16];
    uint32_t version;
    uint32_t capabilities;
}cam_capability;

typedef struct cam_sensor_aec {
    uint16_t      x1;
    uint16_t      x2;
    uint16_t      y1;
    uint16_t      y2;
    uint16_t      center_x1;
    uint16_t      center_x2;
    uint16_t      center_y1;
    uint16_t      center_y2;
}cam_sensor_aec;

typedef void (*kmdw_camera_callback_t)(uint32_t cam_idx, uint32_t img_buf, uint32_t *p_new_img);

typedef struct cam_ops {
    kmdw_status_t (*open)(uint32_t cam_idx);
    kmdw_status_t (*close)(uint32_t cam_idx);
    kmdw_status_t (*set_format)(uint32_t cam_idx, cam_format *format);
    kmdw_status_t (*get_format)(uint32_t cam_idx, cam_format *format);
    kmdw_status_t (*buffer_init)(uint32_t cam_idx, uint32_t buf_addr_0, uint32_t buf_addr_1);
    kmdw_status_t (*start_capture)(uint32_t cam_idx, kmdw_camera_callback_t img_cb);
    kmdw_status_t (*stop_capture)(uint32_t cam_idx);
    kmdw_status_t (*buffer_prepare)(uint32_t cam_idx);
    kmdw_status_t (*buffer_capture)(uint32_t cam_idx, uint32_t *addr, uint32_t *size);
    kmdw_status_t (*stream_on)(uint32_t cam_idx);
    kmdw_status_t (*stream_off)(uint32_t cam_idx);
    kmdw_status_t (*query_capability)(uint32_t cam_idx, struct cam_capability *cap);
    kmdw_status_t (*set_gain)(uint32_t cam_idx, uint32_t gain1, uint32_t gain2);
    kmdw_status_t (*set_aec)(uint32_t cam_idx, struct cam_sensor_aec *aec_p);
    kmdw_status_t (*set_exp_time)(uint32_t cam_idx, uint32_t exp_time);
    kmdw_status_t (*get_lux)(uint32_t cam_idx, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average);
    kmdw_status_t (*led_switch)(uint32_t cam_idx, uint32_t on);
    kmdw_status_t (*set_mirror)(uint32_t cam_idx, uint32_t enable);
    kmdw_status_t (*set_flip)(uint32_t cam_idx, uint32_t enable);
    kmdw_status_t (*get_expo)(uint32_t cam_idx);
    kmdw_status_t (*set_inc)(uint32_t cam_idx, uint32_t enable);
    uint32_t      (*get_device_id)(uint32_t cam_idx);
    kmdw_status_t (*ioctl)(uint32_t cam_idx, uint32_t cid, void *data, uint16_t len);
    kmdw_status_t (*set_addr)(uint32_t cam_id, uint32_t address, uint32_t port_id);
    kmdw_status_t (*set_clock)(void);
    kmdw_status_t (*set_cam_port)(uint32_t cam_idx, uint32_t input_port_type);
}cam_ops;

/**
 * @brief       Initializes camera setting
 *
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_init(void);

/**
 * @brief       camera open function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_open(uint32_t cam_idx);

/**
 * @brief       camera close function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_close(uint32_t cam_idx);

/**
 * @brief       camera get device information function
 *
 * @param[in]   cam_idx     camera id
 * @param[out]  cap         point of camera capability information.
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_get_device_info(uint32_t cam_idx, struct cam_capability *cap);

/**
 * @brief       camera set frame format function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   cap         point of format information.
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_set_frame_format(uint32_t cam_idx, cam_format *format);

/**
 * @brief       camera get frame format function
 *
 * @param[in]   cam_idx     camera id
 * @param[out]   cap         point of format information.
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_get_frame_format(uint32_t cam_idx, cam_format *format);

/**
 * @brief       camera buffer init function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   buf_addr_0  buffer address 0
 * @param[in]   buf_addr_1  buffer address 1
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_buffer_init(uint32_t cam_idx, uint32_t buf_addr_0, uint32_t buf_addr_1);

/**
 * @brief       camera start function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   img_cb      image complete callback function
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_start(uint32_t cam_idx, kmdw_camera_callback_t img_cb);

/**
 * @brief       camera stop function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_stop(uint32_t cam_idx);

/**
 * @brief       camera buffer prepare function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_buffer_prepare(uint32_t cam_idx);

/**
 * @brief       camera buffer capture function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_buffer_capture(uint32_t cam_idx, uint32_t *addr, uint32_t *size);

/**
 * @brief       camera streaming on function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_stream_on(uint32_t cam_idx);

/**
 * @brief       camera streaming off function
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_stream_off(uint32_t cam_idx);

/**
 * @brief       camera set gain function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   gain1       gain parameter 1
 * @param[in]   gain2       gain parameter 2
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_set_gain(uint32_t cam_idx, uint32_t gain1, uint32_t gain2);

/**
 * @brief       camera set ae controller ROI area function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   aec_p       point of cam_sensor_aec
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_set_aec(uint32_t cam_idx, struct cam_sensor_aec *aec_p);

/**
 * @brief       camera set exposure time function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   gain1       exposure time parameter 1
 * @param[in]   gain2       exposure time parameter 2
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_set_exp_time(uint32_t cam_idx, uint32_t exp_time);

/**
 * @brief       camera get lum and other parameter function
 *
 * @param[in]   cam_idx     camera id
 * @param[out]  expo        exposure time parameter
 * @param[out]  pre_gain    exposure time parameter
 * @param[out]  post_gain   exposure time parameter
 * @param[out]  global_gain exposure time parameter
 * @param[in]   y_average   exposure time parameter 2
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_get_lux(uint32_t cam_idx, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average);

/**
 * @brief       camera set nir led on/off function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   on          LED on/off cmd, 1: on, 0:off
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_led_switch(uint32_t cam_idx, uint32_t on);

/**
 * @brief       camera set mirror on/off function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   on          mirror on/off cmd, 1: on, 0:off
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_set_mirror(uint32_t cam_idx, uint32_t enable);

/**
 * @brief       camera set flip on/off function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   on          mirror on/off cmd, 1: on, 0:off
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_set_flip(uint32_t cam_idx, uint32_t enable);

/**
 * @brief       camera ioctl function
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   cid         control command id
 * @param[in|out] *data     poniter to the parameter structure, a control command specific
 * @param[in]   len         structure length
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_ioctl(uint32_t cam_idx, uint32_t cid, void *data, uint16_t len);

/**
 * @brief       get exposure time
 *
 * @param[in]   cam_idx     camera id
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_get_exp_time(uint32_t cam_idx);

/**
 * @brief       register specific cam ops with cam_idx
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   cam_ops_p   incidence for each cam_idx
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_controller_register(uint32_t cam_idx, struct cam_ops *cam_ops_p);

/**
 * @brief       unregister specific cam ops with cam_idx
 *
 * @param[in]   cam_idx     camera id
 * @param[in]   cam_ops_p   incidence for each cam_idx
 * @return      kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_camera_controller_unregister(uint32_t cam_idx, struct cam_ops *cam_ops_p);

#endif // __KMDW_CAMERA_H__
