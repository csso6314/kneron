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
*  Filename:
*  ---------
*  kdev_dsi_panel_ili9881.c
*
*  Project:
*  --------
*  KL530
*
*  Description:
*  ------------
*  This is display panel driver
*
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/
#include "kdev_dsi_panel_ili9881.h"
#include "kdev_dsi.h"
#include "kdrv_io.h"
#include "kdrv_lcdc.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
struct kdev_panel_dsi_dcs_cmd init_ili9881c_seq[] = {
	//PAGE3
        {0xFF, 3, {0x98, 0x81, 0x03} },
        {0x02, 1, {0x00} },
        {0x03, 1, {0x73} },
        {0x04, 1, {0x00} },
        {0x05, 1, {0x00} },
        {0x06, 1, {0x0A} },
        {0x07, 1, {0x00} },
        {0x08, 1, {0x00} },
        {0x09, 1, {0x01} },
        {0x0A, 1, {0x00} },
        {0x0B, 1, {0x00} },
        {0x0C, 1, {0x01} },
        {0x0D, 1, {0x00} },
        {0x0E, 1, {0x00} },
        {0x0F, 1, {0x1D} },
        {0x10, 1, {0x1D} },
        {0x11, 1, {0x00} },
        {0x12, 1, {0x00} },
        {0x13, 1, {0x00} },
        {0x14, 1, {0x00} },
        {0x15, 1, {0x00} },
        {0x16, 1, {0x00} },
        {0x17, 1, {0x00} },
        {0x18, 1, {0x00} },
        {0x19, 1, {0x00} },
        {0x1A, 1, {0x00} },
        {0x1B, 1, {0x00} },
        {0x1C, 1, {0x00} },
        {0x1D, 1, {0x00} },
        {0x1E, 1, {0x40} },
        {0x1F, 1, {0x80} },
        {0x20, 1, {0x06} },
        {0x21, 1, {0x02} },
        {0x22, 1, {0x00} },
        {0x23, 1, {0x00} },
        {0x24, 1, {0x00} },
        {0x25, 1, {0x00} },
        {0x26, 1, {0x00} },
        {0x27, 1, {0x00} },
        {0x28, 1, {0x33} },
        {0x29, 1, {0x03} },
        {0x2A, 1, {0x00} },
        {0x2B, 1, {0x00} },
        {0x2C, 1, {0x00} },
        {0x2D, 1, {0x00} },
        {0x2E, 1, {0x00} },
        {0x2F, 1, {0x00} },
        {0x30, 1, {0x00} },
        {0x31, 1, {0x00} },
        {0x32, 1, {0x00} },
        {0x33, 1, {0x00} },
        {0x34, 1, {0x04} },
        {0x35, 1, {0x00} },
        {0x36, 1, {0x00} },
        {0x37, 1, {0x00} },
        {0x38, 1, {0x3C} },
        {0x39, 1, {0x00} },
        {0x3A, 1, {0x40} },
        {0x3B, 1, {0x40} },
        {0x3C, 1, {0x00} },
        {0x3D, 1, {0x00} },
        {0x3E, 1, {0x00} },
        {0x3F, 1, {0x00} },
        {0x40, 1, {0x00} },	
        {0x41, 1, {0x00} },
        {0x42, 1, {0x00} },
        {0x43, 1, {0x00} },
        {0x44, 1, {0x00} },
	//GIP_2
        {0x50, 1, {0x01} },
        {0x51, 1, {0x23} },
        {0x52, 1, {0x45} },
        {0x53, 1, {0x67} },
        {0x54, 1, {0x89} },
        {0x55, 1, {0xAB} },
        {0x56, 1, {0x01} },
        {0x57, 1, {0x23} },
        {0x58, 1, {0x45} },
        {0x59, 1, {0x67} },
        {0x5A, 1, {0x89} },
        {0x5B, 1, {0xAB} },
        {0x5C, 1, {0xCD} },
        {0x5D, 1, {0xEF} },
	//GIP_3
        {0x5E, 1, {0x11} },
        {0x5F, 1, {0x01} },
        {0x60, 1, {0x00} },
        {0x61, 1, {0x15} },
        {0x62, 1, {0x14} },
        {0x63, 1, {0x0E} },
        {0x64, 1, {0x0F} },
        {0x65, 1, {0x0C} },
        {0x66, 1, {0x0D} },
        {0x67, 1, {0x06} },
        {0x68, 1, {0x02} },
        {0x69, 1, {0x07} },
        {0x6A, 1, {0x02} },
        {0x6B, 1, {0x02} },
        {0x6C, 1, {0x02} },
        {0x6D, 1, {0x02} },
        {0x6E, 1, {0x02} },
        {0x6F, 1, {0x02} },
        {0x70, 1, {0x02} },
        {0x71, 1, {0x02} },
        {0x72, 1, {0x02} },
        {0x73, 1, {0x02} },
        {0x74, 1, {0x02} },
        {0x75, 1, {0x01} },
        {0x76, 1, {0x00} },
        {0x77, 1, {0x14} },
        {0x78, 1, {0x15} },
        {0x79, 1, {0x0E} },
        {0x7A, 1, {0x0F} },
        {0x7B, 1, {0x0C} },
        {0x7C, 1, {0x0D} },
        {0x7D, 1, {0x06} },
        {0x7E, 1, {0x02} },
        {0x7F, 1, {0x07} },
        {0x80, 1, {0x02} },
        {0x81, 1, {0x02} },
        {0x82, 1, {0x02} },
        {0x83, 1, {0x02} },
        {0x84, 1, {0x02} },
        {0x85, 1, {0x02} },
        {0x86, 1, {0x02} },
        {0x87, 1, {0x02} },
        {0x88, 1, {0x02} },
        {0x89, 1, {0x02} },
        {0x8A, 1, {0x02} },
	//PAGE4
        {0xFF, 3, {0x98, 0x81, 0x04} },
        {0x6C, 1, {0x15} },
        {0x6E, 1, {0x2B} },
        {0x6F, 1, {0x33} }, //VGH & VGL OUTPUT
        {0x8D, 1, {0x18} },
        {0x87, 1, {0xBA} },
        {0x26, 1, {0x76} },
        {0xB2, 1, {0xD1} }, //Reload Gamma Setting
        {0xB5, 1, {0x06} },
        {0x3A, 1, {0x24} },
        {0x35, 1, {0x1F} },
        //{0x2F, 1, {0x01} }, //BIST Mode enable
        //{0x2D, 1, {0x80} }, //BIST Mode Function
	//PAGE1
        {0xFF, 3, {0x98, 0x81, 0x01} },
        {0x22, 1, {0x09} },
        {0x31, 1, {0x00} }, //Column inversion
        {0x40, 1, {0x33} },
        {0x53, 1, {0xA2} },
        {0x55, 1, {0x92} },
        {0x50, 1, {0x96} },
        {0x51, 1, {0x96} },
        {0x60, 1, {0x22} },
        {0x61, 1, {0x00} },
        {0x62, 1, {0x19} },
        {0x63, 1, {0x00} },
	//P GAMMA START
        {0xA0, 1, {0x08} },
        {0xA1, 1, {0x11} },
        {0xA2, 1, {0x19} },
        {0xA3, 1, {0x0D} },
        {0xA4, 1, {0x0D} },
        {0xA5, 1, {0x1E} },
        {0xA6, 1, {0x14} },
        {0xA7, 1, {0x17} },
        {0xA8, 1, {0x4F} },
        {0xA9, 1, {0x1A} },
        {0xAA, 1, {0x27} },
        {0xAB, 1, {0x49} },
        {0xAC, 1, {0x1A} },
        {0xAD, 1, {0x18} },
        {0xAE, 1, {0x4C} },
        {0xAF, 1, {0x22} },
        {0xB0, 1, {0x27} },
        {0xB1, 1, {0x4B} },
        {0xB2, 1, {0x60} },
        {0xB3, 1, {0x39} },
	//N GAMMA START
        {0xC0, 1, {0x08} },
        {0xC1, 1, {0x11} },
        {0xC2, 1, {0x19} },
        {0xC3, 1, {0x0D} },
        {0xC4, 1, {0x0D} },
        {0xC5, 1, {0x1E} },
        {0xC6, 1, {0x14} },
        {0xC7, 1, {0x17} },
        {0xC8, 1, {0x4F} },
        {0xC9, 1, {0x1A} },
        {0xCA, 1, {0x27} },
        {0xCB, 1, {0x49} },
        {0xCC, 1, {0x1A} },
        {0xCD, 1, {0x18} },
        {0xCE, 1, {0x4C} },
        {0xCF, 1, {0x33} },
        {0xD0, 1, {0x27} },
        {0xD1, 1, {0x4B} },
        {0xD2, 1, {0x60} },
        {0xD3, 1, {0x39} },
        //{0xB7, 1, {0x03} },//2 lane
        {0xB7, 1, {0x02} },//4 lane
        {0xB6, 1, {0xa0} },
	//PAGE0
        {0xFF, 3, {0x98, 0x81, 0x00} },
        {0x11, 0, {0x00} },
        {0x35, 1, {0x00} },
        {0x36, 1, {0x03} },
        {0x29, 0, {0x00} },
        {0x37, 0, {0x01} }, //Max. return packet
};

void kdev_dsi_panel_init(kdrv_mipi_dsi_a2dc_t *wdata)
{
	uint32_t i = 0;

	/* DCS long write, derived from the CTD's pseudo code */
	//Transmitting DCS Write command
	for (i = 0; i < ARRAY_SIZE(init_ili9881c_seq); i++) {
		kdrv_mipi_dsi_dcs_short_write(wdata, init_ili9881c_seq[i].cmd,
						init_ili9881c_seq[i].data,
						init_ili9881c_seq[i].len);
	}

}

void kdev_dsi_panel_set_page(kdrv_mipi_dsi_a2dc_t wdata, uint8_t page)
{
	uint8_t cmd;
	uint8_t data[4];
	uint8_t len = 0;

	cmd = 0xFF;
	data[0] = 0x98;
	data[1] = 0x81;
	data[2] = page;
	len = 3;
	
	kdrv_mipi_dsi_dcs_short_write(&wdata, cmd,
			data,
			len);
}

void kdev_dsi_panel_bist(uint8_t en)
{
	uint8_t cmd;
	uint8_t data[4];
	uint8_t len = 0;
	kdrv_mipi_dsi_a2dc_t wdata;
	
	memset((void *) &wdata, 0, 4);
	
	//switch page4
    kdev_dsi_panel_set_page(wdata, PAGE4);

	if (en == 1) {
		cmd = 0x2F;
		data[0] = 0x1;
		len = 1;
		kdrv_mipi_dsi_dcs_short_write(&wdata, cmd,
				data,
				len);
		cmd = 0x2D;
		data[0] = 0x80;
		len = 1;
		kdrv_mipi_dsi_dcs_short_write(&wdata, cmd,
				data,
				len);
	} else {
		cmd = 0x2F;
		data[0] = 0x0;
		len = 1;
		kdrv_mipi_dsi_dcs_short_write(&wdata, cmd,
				data,
				len);
		cmd = 0x2D;
		data[0] = 0x80;
		len = 1;
		kdrv_mipi_dsi_dcs_short_write(&wdata, cmd,
				data,
				len);
	}
	
	//switch page0
    kdev_dsi_panel_set_page(wdata, PAGE0);
}

void kdev_dsi_panel_lp_mode(bool en)
{
    regDSI_reg->st.bf.kdrv_mipi_dsi_cr1.LPDTE = en;
}

kdev_status_t kdev_dsi_panel_initialize(void)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    kdrv_mipi_dsi_attr_context_t attr;
    kdrv_mipi_dsi_a2dc_t wdata;
    
    attr.vsa = mipi_dsi_vsa;
    attr.vfp = mipi_dsi_vfp;
    attr.vbp = mipi_dsi_vbp; 
    attr.vactive = mipi_dsi_vactive; 
    attr.hsa = mipi_dsi_hsa;    
    attr.hfp = mipi_dsi_hfp;
    attr.hbp = mipi_dsi_hbp;
    attr.hactive = mipi_dsi_hactive;
    attr.dsi_clk = mipi_dsi_clk;
    attr.bpp = mipi_dsi_bpp;
	attr.fifo_threshold = mipi_dsi_fifo_threshold;
    #if (MIPI_DSI_PACKET_TYPE == DISPLAY_MIPI_DSI_RGB888)
        attr.panel_type = lcdc_8bits_per_channel;
    #else
        attr.panel_type = lcdc_6bits_per_channel;
    #endif
    
	kmdw_printf("vfp: %d, vbp: %d, vsa: %d\n", attr.vfp, attr.vbp, attr.vsa);
	kmdw_printf("hfp: %d, hbp: %d, hsa: %d\n", attr.hfp, attr.hbp, attr.hsa);
	kmdw_printf("resolution: %d x %d\n", attr.hactive, attr.vactive);
	//kmdw_printf("event mode: %d\n", attr.event_mode);
    kdrv_mipi_dsi_initialize(attr);
    kdev_dsi_panel_init(&wdata);
	kdrv_mipi_dsi_dcs_read(0x0a);

    kdrv_lcdc_attr_context_t lcdc_attr;
    memcpy((void *)&lcdc_attr, (void *)&attr, sizeof(kdrv_mipi_dsi_attr_context_t));
    kdrv_lcdc_DPI_initialize(lcdc_attr);
    kdrv_mipi_dsi_wait_DBI_ready();
    return ret;
}
