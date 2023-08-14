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
*  kdrv_dpi2ahb.h
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
/**@addtogroup  KDRV_DPI2AHB  KDRV_DPI2AHB
 * @{
 * @brief       Kneron dpi2ahb driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef _KDRV_DPI2AHB_H_
#define _KDRV_DPI2AHB_H_

#include "kdrv_scu_ext.h"
#include "kdrv_status.h"
#include "kdrv_camera.h"
enum {
    D2A_0,
    D2A_1,
    D2A_NUM,    // = IMGSRC_NUM
};
typedef enum{
    DPI_SRC_CSIRX,
    DPI_SRC_EXT_DPI,
    DPI_SRC_NUM
}dpi_src_opt;

typedef struct d2a_para{
    uint32_t d2a_input_sorce;
    uint32_t d2a_fifo_threshold;
    uint32_t d2a_drop_frame_num;
    uint32_t d2a_packet_type;
    uint32_t d2a_data_align;
    uint32_t d2a_vsync_polarity;
    uint32_t d2a_hsync_polarity;
    uint32_t d2a_tile_ave_en;
    uint32_t d2a_tile_ave_size;
}d2a_para;

typedef void (*kdrv_dpi2ahb_callback_t)(uint32_t d2a_idx, uint32_t img_buf, uint32_t *p_new_img);

#define TILE_BLOCK_MAX_W        10
#define TILE_BLOCK_MAX_H        6
#define TILE_BLOCKS_MAX         (TILE_BLOCK_MAX_W * TILE_BLOCK_MAX_H)   // bytes
#define TILE_REGS_MAX           (TILE_BLOCKS_MAX / 4)                   // u32's

/**
* @brief       kdrv_dpi2ahb_set_para
*              Enable dpi2ahb IP,
* @param[in]   d2a_idx  d2a_idx
* @param[in]   fmt      camera related format setting
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_dpi2ahb_set_para(uint32_t d2a_idx, cam_format* fmt, d2a_para* para);

/**
* @brief       kdrv_dpi2ahb_stop
*              Stop dpi2ahb interrup, disable IRQ.
* @param[in]   d2a_idx  d2a_idx
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_dpi2ahb_stop(uint32_t d2a_idx);

/**
* @brief       kdrv_dpi2ahb_start
*              Start dpi2ahb interrup, enable IRQ.
* @param[in]   img_cb      image complete callback function
* @param[in]   d2a_idx  d2a_idx
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_dpi2ahb_start(uint32_t d2a_idx, kdrv_dpi2ahb_callback_t img_cb);

/**
* @brief       kdrv_dpi2ahb_buf_init
*              Set dpi2ahb page buffer default address.
* @param[in]   d2a_idx     d2a_idx
* @param[in]   buf_addr_0  buffer address 0
* @param[in]   buf_addr_1  buffer address 1
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_dpi2ahb_buf_init(uint32_t d2a_idx, uint32_t buf_addr_0, uint32_t buf_addr_1);

/**
* @brief       kdrv_dpi2ahb_initialize
*              Init dpi2ahb IRQ and reset IP.
* @param[in]   d2a_idx  d2a_idx
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_dpi2ahb_initialize(uint32_t d2a_idx);

/**
* @brief       kdrv_dpi2ahb_src_config
*              Init image streaming input from ext dpi or csirx data port.
* @param[in]   d2a_idx  d2a_idx
* @param[in]   src      src opt
* @return      kdrv_status_t
*/
kdrv_status_t kdrv_dpi2ahb_src_config(uint32_t d2a_idx, dpi_src_opt src);
#endif // _KDRV_DPI2AHB_H_
