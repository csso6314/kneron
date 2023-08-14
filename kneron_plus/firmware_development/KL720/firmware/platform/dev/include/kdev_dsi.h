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
#include "kdrv_mipi_dsi.h"
#include "kdev_status.h"

typedef enum {
    PAGE0 = 0, //compressed_pixel_stream_packet
    PAGE1,
    PAGE2,
    PAGE3,
    PAGE4,
    PAGE5,
    PAGE6,
    PAGE7,
    PAGE8,
    PAGE9,
    PAGE10
}kdrv_mipi_dsi_reg_page;

void kdev_dsi_panel_init(kdrv_mipi_dsi_a2dc_t *wdata);
void kdev_dsi_panel_bist(uint8_t en);
void kdev_dsi_panel_lp_mode(bool en);
void kdev_dsi_panel_set_page(kdrv_mipi_dsi_a2dc_t wdata, uint8_t page);
kdev_status_t kdev_dsi_panel_initialize(void);

