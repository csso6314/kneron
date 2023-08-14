/**
 * Kneron Peripheral API - I2S
 *
 * Copyright (C) 2020 Kneron, Inc. All rights reserved.
 *
 */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include "base.h"
#include "kdrv_i2s.h"
#include "kdrv_status.h"
#include "regbase.h"
#include "kdrv_io.h"
#include "kdrv_cmsis_core.h"
#include "kdrv_scu_ext.h"
/* Private define ------------------------------------------------------------*/
/*
An I2S bus uses three signal lines for data transfer: a frame clock, a bit clock, and a data line
The Philips standard for these signals uses the names WS for word select, SCK for the clock, and SD for the data
IC manufacturers seem to rarely use these names in their IC data sheets.
Word select is also commonly called LRCLK, for left/right clock.
SCK may be called BCLK, for bit clock or SCLK for serial clock.
An I2S data stream can carry one or two channels of data with a typical bit clock rate between 512 kHz, for an 8 kHz 
sampling rate, and 12.288 MHz, for a 192 kHz sampling rate. 
*/
#define SSP_I2S_MODE                3
typedef volatile struct {
    uint32_t sclk_ph:               1; /*SPI only*/
    uint32_t sclk_po:               1; /*SPI only*/
    uint32_t operation_mode:        2;
    uint32_t data_justfy:           1; /*I2S only, 0: padding in back, 1: padding in front*/
    uint32_t frame_sync_po:         1; /*I2S or MWR only*/
    uint32_t bit_seq_ind:           1; /*I2S or SPI only*/
    uint32_t loop_back:             1;
    uint32_t frame_sync_dist:       2; /*I2S only */
    uint32_t rsvd:                  1;
    uint32_t is_flash:              1;
    uint32_t format:                3; /*0: TI SSP, 1: SPI, 2: MicroWire, 3: i2s*/
    uint32_t spi_frame_po:          1; /*SPI only*/
    uint32_t sclk_fb:               1; /*0: sclk_in would be from input pin, 1: sclk_in from sclk_out_r. Need in Master mode*/
    uint32_t fs_fb:                 1; /*0: fs_in would be from input pin, 1: fs_in from fs_out_r. Need in Master mode*/
    uint32_t flash_tx:              1; /*SPI only and is_flash to be set*/
    uint32_t spi_cont_tx:           1; 
    uint32_t lcd_dcxs:              1;
    uint32_t lcd_dcx:               1;
    uint32_t rsvd2:                 10;
}kdrv_ssp_control_t;

typedef volatile struct {
    uint32_t sclk_div:              16;
    uint32_t serial_data_len:       7; /* The actual transmit len would be this value+1. For i2s, max 127 */
    uint32_t rsvd:                  1;
    uint32_t padding_data_len:      8;
}kdrv_ssp_control1_t;

typedef volatile struct {
    uint32_t ssp_en:                1;
    uint32_t tx_output_en:          1; /* For i2s, 0: record mode, 1: paly/record mode */
    uint32_t rx_fifo_clear:         1;
    uint32_t tx_fifo_clear:         1;
    uint32_t rsvd:                  2;
    uint32_t ssp_rst:               1; /* write 1 to reset */
    uint32_t rx_en:                 1; /* For i2s, 0: tx is disabled, 1: rx is enabled */
    uint32_t tx_en:                 1;
    uint32_t frame_sync_output:     1; /* SPI only */
    uint32_t frame_sync_output_sel: 2; /* SPI only */
    uint32_t rsvd2:                 20;
}kdrv_ssp_control2_t;

typedef volatile struct {
    uint32_t rx_fifo_ovr_run_en:    1;
    uint32_t tx_fifo_udr_run_en:    1;
    uint32_t rx_fifo_threshold_en:  1;
    uint32_t tx_fifo_threshold_en:  1;
    uint32_t rx_dma_en:             1;
    uint32_t tx_dma_en:             1;
    uint32_t rsvd:                  1;
    uint32_t rx_fifo_threshold:     5;
    uint32_t tx_fifo_threshold:     5;
    uint32_t rx_fifo_thre_uint:     1;
    uint32_t tx_complete_en:        1;
    uint32_t rsvd2:                 13;
}kdrv_ssp_int_ctl_t;

typedef volatile struct {
    uint32_t rx_fifo_full:          1;
    uint32_t tx_fifo_not_full:      1;
    uint32_t is_busy:               1;
    uint32_t rsvd:                  1;
    uint32_t rx_fifo_num:           6;
    uint32_t rsvd2:                 2;
    uint32_t tx_fifo_num:           6;
    uint32_t rsvd3:                 14;
}kdrv_ssp_stat_t;


typedef volatile struct {
    uint32_t rx_fifo_ovr_run:       1;
    uint32_t tx_fifo_udr_run:       1;
    uint32_t rx_fifo_threshold:     1;
    uint32_t tx_fifo_threshold:     1;
    uint32_t rsvd:                  1;
    uint32_t tx_complete:           1;
    uint32_t rsvd2:                 26;
}kdrv_ssp_int_stat_t;

typedef volatile struct {
    uint32_t pad_cycle_len:         10; /*relevant only when SPI frame format specified*/
    uint32_t rsvd:                  2;
    uint32_t diff_pad_data_len_en:  1;
    uint32_t rsvd2:                 3;
    uint32_t diff_pad_data_len:     8;
    uint32_t rsvd3:                 8;
}kdrv_ssp_control3_t;

typedef volatile struct {
    uint32_t rel_ver:               8;
    uint32_t minor_ver:             8;
    uint32_t major_ver:             8;
    uint32_t rsvd:                  8;
}kdrv_ssp_revision_t;

typedef volatile struct {
    uint32_t fifo_width:            8; // 0x1f, actual: 0x20
    uint32_t rx_fifo_depth:         8; // 0x0f, actual: 0x10
    uint32_t tx_fifo_depth:         8; // 0x0f, actual: 0x10
    uint32_t rsv:                   1;
    uint32_t i2s_fcfg:              1; // 1
    uint32_t spimwr_fcfg:           1; // 1
    uint32_t ssp_fcfg:              1; // 1
    uint32_t rsvd2:                 1; 
    uint32_t ext_fsnum:             2; // 0
    uint32_t rsvd3:                 1;
}kdrv_ssp_feature_t;

#define SSP_CTL(n)      ((kdrv_ssp_control_t *)(SSP0_REG_BASE+(n*0x00100000)+0x00))
#define SSP_CTL1(n)     ((kdrv_ssp_control1_t *)(SSP0_REG_BASE+(n*0x00100000)+0x04))
#define SSP_CTL2(n)     ((kdrv_ssp_control2_t *)(SSP0_REG_BASE+(n*0x00100000)+0x08))
#define SSP_STAT(n)     ((kdrv_ssp_stat_t *)(SSP0_REG_BASE+ (n*0x00100000) + 0x0C))
#define SSP_INT_CTL(n)  ((kdrv_ssp_int_ctl_t *)(SSP0_REG_BASE+ (n*0x00100000) + 0x10))
#define SSP_INT_STAT(n) (kdrv_ssp_int_stat_t *)(SSP0_REG_BASE+ (n*0x00100000) + 0x14))
#define SSP_TXRXDR(n)   ((uint32_t *)(SSP0_REG_BASE+ (n*0x00100000) + 0x18))
#define SSP_CTL3(n)     ((kdrv_ssp_control3_t *)(SSP0_REG_BASE+ (n*0x00100000) + 0x1C))
#define SSP_REVISION(n) ((kdrv_ssp_revision_t *)(SSP0_REG_BASE+ (n*0x00100000) + 0x60))
#define SSP_FEATURE(n)  ((kdrv_ssp_feature_t *)(SSP0_REG_BASE+ (n*0x00100000) + 0x64))

static kdrv_i2s_attr_context _i2s_attr[TOTAL_KDRV_I2S_INSTANCE] = {0};

/* Function define ------------------------------------------------------------*/

kdrv_status_t kdrv_i2s_init(uint8_t num, kdrv_i2s_attr_context *attr){
    kdrv_i2s_attr_context  *handle;
    if(attr == NULL || num > 2 || num == 0)
        return KDRV_STATUS_ERROR;
    
    handle = &attr[0];
    memcpy(&_i2s_attr[handle->instance], handle, sizeof(kdrv_i2s_attr_context));
    
    if(num == 2){
        handle = &attr[1];
        memcpy(&_i2s_attr[handle->instance], handle, sizeof(kdrv_i2s_attr_context));
    }
    
    // Gate the MCLK
    uint32_t reg = inw(SCU_EXTREG_CLOCK_ENABLE_REG0);
    reg &= ~(CLK_REG0_I2S_MCLK_EN);
    outw(SCU_EXTREG_CLOCK_ENABLE_REG0, reg);
    // enable ssp clock
    if(handle->instance == 0){
        SET_CLOCK_EN(SSP0_CLK_EN, 1);
    }
    else if(handle->instance == 1){
        SET_CLOCK_EN(SSP1_CLK_EN, 1);
    }

    // reset SSP
    SSP_CTL2(handle->instance)->ssp_rst = 1;
    // disable SSP
    SSP_CTL2(handle->instance)->ssp_en = 0;
    
    if(handle->mode == I2S_MASTER_MONO || handle->mode == I2S_MASTER_STEREO){
        // master mode
        SSP_CTL(handle->instance)->format = SSP_I2S_MODE; //i2s mode
        SSP_CTL(handle->instance)->operation_mode = handle->mode;
        SSP_CTL(handle->instance)->bit_seq_ind = handle->bit_seq;
        
        //SSP_CTL->fs_fb = 1;
        //SSP_CTL->sclk_fb = 1;
        
        // Adjust pad and fsd according to the frame format
        // Basic format: frame_sync_dist = 1
        // Right justify: pdl = padding num, data_justfy = 0, frame_sync_dist = 0, 
        // Left justify: pdl = padding num, data_justfy = 1, frame_sync_dist = 0
        if(handle->padding_len == 0){
            SSP_CTL(handle->instance)->frame_sync_dist = 1;
        }
        else{
            SSP_CTL(handle->instance)->frame_sync_dist = 0;
        }
        SSP_CTL(handle->instance)->data_justfy = handle->padding_mode;
        SSP_CTL1(handle->instance)->padding_data_len = handle->padding_len;
        SSP_CTL1(handle->instance)->serial_data_len = handle->data_len - 1;

        // BCLK = 100MHz/(2*(sclk_div + 1))
        // freq = sample_freq * len(bit) * 2(dual channel)
        float freq = (100000000.0) / (2 * (handle->sample_freq * (2 * (handle->padding_len + handle->data_len))));
        SSP_CTL1(handle->instance)->sclk_div = (uint16_t)(freq) - 1;

        SSP_INT_CTL(handle->instance)->tx_complete_en = 1;
        SSP_INT_CTL(handle->instance)->tx_fifo_threshold_en = 0;
        SSP_INT_CTL(handle->instance)->tx_dma_en = 0;
        SSP_INT_CTL(handle->instance)->rx_dma_en = 0;

        // enable SSP
        SSP_CTL2(handle->instance)->tx_output_en = 0;
        SSP_CTL2(handle->instance)->tx_en = 0;
        SSP_CTL2(handle->instance)->rx_en = 0;
        SSP_CTL2(handle->instance)->rx_fifo_clear = 1;
        SSP_CTL2(handle->instance)->tx_fifo_clear = 1;
        SSP_CTL2(handle->instance)->ssp_en = 1;

    }
    else{
        // slave mode
        SSP_CTL(handle->instance)->format = SSP_I2S_MODE; //i2s mode
        SSP_CTL(handle->instance)->operation_mode = handle->mode;
        SSP_CTL(handle->instance)->bit_seq_ind = handle->bit_seq;
        
        if(handle->padding_len == 0){
            SSP_CTL(handle->instance)->frame_sync_dist = 1;
        }
        else{
            SSP_CTL(handle->instance)->frame_sync_dist = 0;
        }
        SSP_CTL(handle->instance)->data_justfy = handle->padding_mode;
        SSP_CTL1(handle->instance)->padding_data_len = handle->padding_len;
        SSP_CTL1(handle->instance)->serial_data_len = handle->data_len - 1;
        // enable SSP
        SSP_CTL2(handle->instance)->tx_output_en = 0;
        SSP_CTL2(handle->instance)->tx_en = 0;
        SSP_CTL2(handle->instance)->rx_en = 0;
        SSP_CTL2(handle->instance)->rx_fifo_clear = 1;
        SSP_CTL2(handle->instance)->tx_fifo_clear = 1;
        SSP_CTL2(handle->instance)->ssp_en = 1;
    }
    if(handle->output_mclk){                
        // enable MCLK
        reg = inw(SCU_EXTREG_CLOCK_ENABLE_REG0);
        reg |= CLK_REG0_I2S_MCLK_EN;
        outw(SCU_EXTREG_CLOCK_ENABLE_REG0, reg);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_i2s_send(kdrv_i2s_instance_t instance, uint32_t val){
    kdrv_i2s_attr_context  *handle;
    handle = &_i2s_attr[instance];
    if(SSP_STAT(handle->instance)->tx_fifo_not_full){
        *(SSP_TXRXDR(handle->instance)) = val;
    }
    else{
        return KDRV_STATUS_ERROR;
    }
    return KDRV_STATUS_OK;
}


kdrv_status_t kdrv_i2s_read(kdrv_i2s_instance_t instance, uint32_t *val){
    kdrv_i2s_attr_context  *handle;
    handle = &_i2s_attr[instance];
    *val = 0;
    if(SSP_STAT(handle->instance)->rx_fifo_num != 0){
        *val = *(SSP_TXRXDR(handle->instance));
    }
    else{
        return KDRV_STATUS_ERROR;
    }
    return KDRV_STATUS_OK;
}

void kdrv_i2s_enable_tx(kdrv_i2s_instance_t instance, uint8_t enable){
    kdrv_i2s_attr_context  *handle;
    handle = &_i2s_attr[instance];
    SSP_CTL2(handle->instance)->tx_en = enable;
    SSP_CTL2(handle->instance)->tx_output_en = enable;
}

void kdrv_i2s_enable_rx(kdrv_i2s_instance_t instance, uint8_t enable){
    kdrv_i2s_attr_context  *handle;
    handle = &_i2s_attr[instance];
    SSP_CTL2(handle->instance)->rx_en = enable;
    if(enable){
        SSP_CTL2(handle->instance)->tx_en = 0;
        SSP_CTL2(handle->instance)->tx_output_en = 0;
    }
}


kdrv_status_t kdrv_i2s_dma_send(kdrv_i2s_instance_t instance, kdrv_gdma_handle_t dma_handle, uint32_t *buf, uint32_t size, gdma_xfer_callback_t xfer_isr_cb){
    kdrv_i2s_attr_context  *handle;
    handle = &_i2s_attr[instance];

    SSP_INT_CTL(handle->instance)->tx_dma_en = 1;
    SSP_INT_CTL(handle->instance)->rx_dma_en = 0;
    gdma_setting_t dma_setting;
    // configure the data width with the i2s data bits
    if(handle->data_len == 8){
        dma_setting.dst_width = GDMA_TXFER_WIDTH_8_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_8_BITS;
    }
    else if(handle->data_len == 16){
        dma_setting.dst_width = GDMA_TXFER_WIDTH_16_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_16_BITS;
    }
    else if(handle->data_len > 16){
        dma_setting.dst_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_32_BITS;
    }
    dma_setting.burst_size = GDMA_BURST_SIZE_1;
    dma_setting.dst_addr_ctrl = GDMA_FIXED_ADDRESS;
    dma_setting.src_addr_ctrl = GDMA_INCREMENT_ADDRESS;
    dma_setting.dma_mode = GDMA_HW_HANDSHAKE_MODE;
    
    if(handle->instance == I2S_INSTANCE_0){
        dma_setting.dma_dst_req = GDMA_HW_REQ_SSP0_TX;
    }
    else if(handle->instance == I2S_INSTANCE_1){
        dma_setting.dma_dst_req = GDMA_HW_REQ_SSP1_TX;
    }
    
    dma_setting.dma_src_req = GDMA_HW_REQ_NONE;
    kdrv_gdma_configure_setting(dma_handle, &dma_setting);
	
    uint32_t dstAddr = (uint32_t)(SSP_TXRXDR(handle->instance));
    uint32_t srcAddr = (uint32_t)buf;
    kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, dstAddr, srcAddr, size, xfer_isr_cb, NULL);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_i2s_dma_read(kdrv_i2s_instance_t instance, kdrv_gdma_handle_t dma_handle, uint32_t *buf, uint32_t size, gdma_xfer_callback_t xfer_isr_cb){
    kdrv_i2s_attr_context  *handle;
    handle = &_i2s_attr[instance];
    
    SSP_INT_CTL(handle->instance)->tx_dma_en = 0;
    SSP_INT_CTL(handle->instance)->rx_dma_en = 1;
    gdma_setting_t dma_setting;
    // configure the data width with the i2s data bits
    if(handle->data_len == 8){
        dma_setting.dst_width = GDMA_TXFER_WIDTH_8_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_8_BITS;
    }
    else if(handle->data_len == 16){
        dma_setting.dst_width = GDMA_TXFER_WIDTH_16_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_16_BITS;
    }
    else if(handle->data_len > 16){
        dma_setting.dst_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_32_BITS;
    }
    dma_setting.burst_size = GDMA_BURST_SIZE_1;
    dma_setting.dst_addr_ctrl = GDMA_INCREMENT_ADDRESS;
    dma_setting.src_addr_ctrl = GDMA_FIXED_ADDRESS;
    dma_setting.dma_mode = GDMA_HW_HANDSHAKE_MODE;
    
	dma_setting.dma_dst_req = GDMA_HW_REQ_NONE;
    
    if(handle->instance == I2S_INSTANCE_0){
        dma_setting.dma_src_req = GDMA_HW_REQ_SSP0_RX;
    }
    else if(handle->instance == I2S_INSTANCE_1){
        dma_setting.dma_src_req = GDMA_HW_REQ_SSP1_RX;
    }
    
    kdrv_gdma_configure_setting(dma_handle, &dma_setting);
	
    uint32_t dstAddr = (uint32_t)buf;
    uint32_t srcAddr = (uint32_t)(SSP_TXRXDR(handle->instance));
    kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, dstAddr, srcAddr, size, xfer_isr_cb, NULL);

    return KDRV_STATUS_OK;
}
