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
/**@addtogroup  KDRV_SPIF_NAND  KDRV_SPIF_NAND
 * @{
 * @brief       Kneron spi nand flash driver - SPI NAND Flash interface API.
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_SPIF_H__
#define __KDRV_SPIF_H__

#include "base.h"
#include "kdrv_status.h"

#define MEMXFER_OPS_NONE 	0x00
#define MEMXFER_OPS_CPU 	0x01
#define MEMXFER_OPS_DMA 	0x02

#define MEMXFER_INITED      0x10
#define MEMXFER_OPS_MASK MEMXFER_OPS_CPU | MEMXFER_OPS_DMA
/*-----------------------------------------------------------------------
 *                          Puclic flash driver API 
 *-----------------------------------------------------------------------*/
/**
* @brief        Initialize spi flash include hardware setting, operation frequency, and flash status check.
*
* @param[in]    N/A
* @return       N/A
*
* @note        This API MUST be called before using the Read/write APIs for spi flash.
*/
void kdrv_spif_initialize(uint32_t clock);
/**
* @brief        Initialize spi flash for memxfer include hardware setting, operation frequency, and flash status check.
*
* @param[in]   flash_mode   flash operating mode
* @param[in]    mem_mode     memory operating mode
* @return      N/A
*/
void kdrv_spif_memxfer_initialize(uint8_t flash_mode, uint8_t mem_mode);
/**
* @brief       Uninitialize spi flash and clear related variables
*
* @param[in]   N/A
* @return      @ref kdrv_status_t
*/
kdrv_status_t kdrv_spif_uninitialize(void);

/**
* @brief        set spi communication commands including read/write by 3/4bytes address, dummy byte size, operation mode, etc
*
* @param[in]    cmd0 ~ 3
* @return       N/A
*/
void kdrv_spif_set_commands(uint32_t cmd0, uint32_t cmd1, uint32_t cmd2, uint32_t cmd3);
/**
* @brief        Check status bit to wait until command completed
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_wait_command_complete(void);
/**
* @brief        Wait until the RX FIFO is full so ready to read 
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_wait_rx_full(void);
/**
* @brief        Wait until the TX FIFO is empty so ready to write
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_wait_tx_empty(void);
/**
* @brief        Check the RX FIFO size, unit in byte
*
* @param[in]    N/A
* @return       >0          RX FIFO depth
*/
uint32_t kdrv_spif_rxfifo_depth(void);
/**
* @brief        Check the TX FIFO size, unit in byte
*
* @param[in]    N/A
* @return       >0          TX FIFO depth
*/
uint32_t kdrv_spif_txfifo_depth(void);
/**
* @brief        read data from specific index in spi flash
*
* @param[in]    *buf        buffer for the data read from flash 
*               length      data size
* @return       N/A
*/
void kdrv_spif_read_data(/*uint8_t*/uint32_t *buf, uint32_t length);
/**
* @brief        write data to specific index in spi flash
*
* @param[in]    *buf        buffer for the data to write to flash
* @param[in]     length      data size
* @return       N/A
*/
void kdrv_spif_write_data(uint8_t *buf, uint32_t length);
/**
* @brief        write data to specific index in spi flash
*
* @param[in]    type        Normal program or Quad program
* @param[in]    dst         program destination address
* @param[in]    *buf        buffer for the data to write to flash
* @param[in]    length      data size
* @return       N/A
*/
void kdrv_spif_write_data_nand(uint8_t  type, uint32_t dst, uint8_t *buf, uint32_t length);
void kdrv_spif_write_data_nand_dma(uint8_t  type, uint32_t dst, uint8_t *buf, uint32_t length);
/**
* @brief        read Rx FIFO data
*
* @param[in]    *buf_word       buffer for the data read from flash
* @param[in]    *buf_word_index start from specific flash index
* @param[in]    target_byte     data size
* @return       N/A
*/
void kdrv_spif_read_Rx_FIFO( uint32_t *buf_word, uint16_t *buf_word_index, uint32_t target_byte );
/**
* @brief        check status till the progress is done and ready for next step
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_check_status_till_ready_2(void);
/**
* @brief        wait command completed and check status till it's ready
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_check_status_till_ready(void);
/**
* @brief        software reset command to reset device
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_reset_device(void);
/**
* @brief        read flash status-3 to check ecc and program/erase failure
*
* @param[in]    status register address
* @return       a 8-bit status read from flash
*/
uint8_t kdrv_spif_get_flash_status(uint32_t status_index);
/**
* @brief        wait quad read command completed and check status till ready
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_check_quad_status_till_ready(void);
/**
* @brief        to remeber the original settings for SPI flash
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_pre_log(void);
/**
* @brief        to switch back to original SPI flash settings
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_switch_org(void);
/**
* @brief        to switch to low speed (50Mhz) SPI flash settings
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_switch_low_speed(void);
/**
* @brief        to mark bad block flag to the block which found ECC error
*
* @param[in]    iblock      the block which found ECC error
* @return       N/A
*/
void kdrv_spif_mark_badblock_flag(uint32_t iblock);
extern bool bGigaDeive_Fseries;
#endif/* __KDRV_SPIF_H__ */
/** @}*/
