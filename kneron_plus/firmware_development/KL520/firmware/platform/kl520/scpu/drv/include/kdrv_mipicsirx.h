/**
 * @file        kdrv_mipicsirx.h
 * @brief       Kneron mipicsirx driver
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef _KDRV_MIPICSIRX_H_
#define _KDRV_MIPICSIRX_H_

#include "kdrv_status.h"

/**
* @brief Enumeration of index of CSI2RX camera
*/
enum csirx_cam_e {
    CSI2RX_CAM_0,       /**< Enum 0, CSI2RX camera 0 */
    CSI2RX_CAM_1,       /**< Enum 1, CSI2RX camera 1 */
    CSI2RX_CAM_NUM,     /**< Enum 2, total of CSI2RX cameras */
};


/**
 * @brief       Initiclize mipicsirx related variable.
 * @param[in]   cam_idx      cam idx
 * @return      kdrv_status_t
 */
kdrv_status_t kdrv_csi2rx_initialize(uint32_t cam_idx);

/**
* @brief       Set mipicsirx related register for IP enable.
* @param[in]   input_type   sensor input type
* @param[in]   cam_idx      cam idx
* @param[in]   sensor_idx   sensor idx
* @param[in]   fmt          camera related format setting
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csi2rx_enable(uint32_t input_type, uint32_t cam_idx, uint32_t sensor_idx, struct cam_format* fmt);

/**
* @brief       Set mipicsirx related register for IP start.
* @param[in]   input_type   sensor input type
* @param[in]   cam_idx      cam_idx
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csi2rx_start(uint32_t input_type, uint32_t cam_idx);

/**
* @brief       Set mipicsirx power related register.
* @param[in]   cam_idx      cam_idx
* @param[in]   on           csirx power status, 1: ON, 0:Off
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csi2rx_set_power(uint32_t cam_idx, uint32_t on);

/**
* @brief       Reset mipicsirx.
* @param[in]   cam_idx      cam_idx
* @param[in]   on           csirx power status, 1: ON, 0:Off
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csi2rx_reset(uint32_t cam_idx, uint32_t sensor_idx);

#endif // _KDRV_MIPICSIRX_H_
