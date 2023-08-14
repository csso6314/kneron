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

/**@addtogroup  KDEV_PANEL  KDEV_PANEL
* @{
* @brief        Kneron panel device interface for MZT_480x272 and ST778_240x320 driver
*
* @copyright    Copyright (C) 2020 Kneron, Inc. All rights reserved.
*/

#ifndef __KDEV_PANEL_H__
#define __KDEV_PANEL_H__

#include "kdrv_display.h"
#include "kdev_status.h"

/**
 * @brief       Initializes kdev panel driver
 *
 * @param[in]   display_drv     see @ref kdrv_display_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 *
 * @note        This API MUST be called before using the Read/write APIs for I2C.
 */
kdev_status_t kdev_panel_initialize(kdrv_display_t *display_drv);

kdev_status_t kdev_panel_clear(kdrv_display_t *display_drv, u32 color);

uint16_t kdev_panel_read_display_id(kdrv_display_t *display_drv);

kdev_status_t kdev_panel_refresh(kdrv_display_t* display_drv);

#endif /* __KDEV_PANEL_H__ */
/** @}*/
