/* Copyright (c) 2021 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */

/******************************************************************************
Head Block of The File
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "kmdw_console.h"
#include "regbase.h"
#include "ARMCM4_FP.h"
#include "kdrv_timer.h"
#include "kdrv_mipi_dsi.h"

//kdev_panel_dsi_ili9881
#define mipi_dsi_vsa			MIPI_DSI_VSA           
#define mipi_dsi_vfp			MIPI_DSI_VFP           
#define mipi_dsi_vbp			MIPI_DSI_VBP           
#define mipi_dsi_vactive		MIPI_DSI_VACTIVE       
#define mipi_dsi_hsa			MIPI_DSI_HSA           
#define mipi_dsi_hfp			MIPI_DSI_HFP           
#define mipi_dsi_hbp			MIPI_DSI_HBP           
#define mipi_dsi_hactive		MIPI_DSI_HACTIVE       
#define mipi_dsi_bpp			MIPI_DSI_BPP           
#define mipi_dsi_clk		    MIPI_DSI_CLK           
#define mipi_dsi_fifo_threshold	MIPI_DSI_FIFO_THRESHOLD

struct kdev_panel_dsi_dcs_cmd {
        uint8_t cmd;
        size_t len;
        uint8_t data[32];
};

