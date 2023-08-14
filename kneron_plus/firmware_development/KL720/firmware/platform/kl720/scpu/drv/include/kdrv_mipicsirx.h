/******************************************************************************
*  Copyright 2020, Kneron, Inc.
*  ---------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Kneron, Inc. (C) 2020
******************************************************************************/

/******************************************************************************
*  Filename:
*  ---------
*  kdrv_mipicsirx.h
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  Configuration for KL720
*
*  Author:
*  -------
*
*
*===========================================================================
*
******************************************************************************/
/**@addtogroup  KDRV_MIPICSIRX  KDRV_MIPICSIRX
 * @{
 * @brief       Kneron mipicsirx driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef _KDRV_MIPICSIRX_H_
#define _KDRV_MIPICSIRX_H_
/******************************************************************************
Head Block of The File
******************************************************************************/
#include "kdrv_status.h"
#include "kdrv_camera.h"

typedef struct csi_para{
    uint32_t timer_count_number;
    uint32_t hs_rx_timeout_value;
    uint32_t mapping_control;
    uint32_t vstu;
    uint32_t vstr;
    uint32_t vster;
    uint32_t hstr;
    uint32_t pftr;
    uint32_t phy_settle_cnt;
}csi_para;
typedef enum {
    CSIRX_VSTU_LINE_,
    CSIRX_VSTU_PIXEL_
}csirx_vstu_opt;

/**
* @brief       kdrv_csirx_set_para
*              Set mipicsirx related register for IP enable.
* @param[in]   input_type   sensor input type
* @param[in]   csirx_idx      csirx idx
* @param[in]   sensor_idx   sensor idx
* @param[in]   fmt          camera related format setting
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csirx_set_para(uint32_t csirx_idx, cam_format *format, csi_para* para);

/**
* @brief       kdrv_csirx_start
*              Set mipicsirx related register for IP start.
* @param[in]   input_type   sensor input type
* @param[in]   csirx_idx      csirx_idx
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csirx_start(uint32_t csirx_idx, uint32_t num);

/**
* @brief       kdrv_csirx_set_power
*              Set mipicsirx power related register.
* @param[in]   csirx_idx      csirx_idx
* @param[in]   on           csirx power status, 1: ON, 0:Off
* @return      kdrv_status_t
*/
//kdrv_status_t kdrv_csirx_set_power(uint32_t csirx_idx, uint32_t on);
kdrv_status_t kdrv_csirx_set_enable(uint32_t csirx_idx, uint32_t enable);
/**
* @brief       kdrv_csirx_reset
*              Reset mipicsirx.
* @param[in]   csirx_idx      csirx_idx
* @param[in]   on           csirx power status, 1: ON, 0:Off
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csirx_reset(uint32_t csirx_idx, uint32_t sensor_idx);

/**
* @brief       kdrv_csirx_stop
*              Set mipicsirx related register for IP stop.
* @param[in]   csirx_idx      csirx_idx
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csirx_stop(uint32_t csirx_idx);

/**
* @brief       kdrv_csirx_initialize
*              Initiclize mipicsirx related variable.
* @param[in]   csirx_idx      csirx idx
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_csirx_initialize(uint32_t csirx_idx);

#endif // _KDRV_MIPICSIRX_H_
