/**
 * @file        kmdw_ssp.h
 * @brief       ssp api spi APIs
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */
#ifndef __KMDW_SSP_H__
#define __KMDW_SSP_H__


#include "kdrv_ssp.h"

uint8_t kmdw_ssp_api_spi_sample(void);
void kmdw_ssp_api_spi_slave_req_init(void);
void kmdw_ssp_api_spi_slave_inactive(void);
void kmdw_ssp_api_spi_slave_active(void);
void kmdw_ssp_api_spi_clear_rx_hw(void);
void kmdw_ssp_api_spi_clear_tx_hw(void);
void kmdw_ssp_api_spi_clear_rx_buff_size(struct st_ssp_spi *stspi );
void kmdw_ssp_api_spi_clear_tx_buff_size(struct st_ssp_spi *stspi );
void kmdw_ssp_api_spi_clear_tx_current_buff_size( void );
void kmdw_ssp_api_spi_clear_tx_done_flag( void );
void kmdw_ssp_api_spi_write_tx_buff( uint8_t *src, uint16_t nlen );
uint8_t kmdw_ssp_api_spi_init(kdrv_ssp_spi_dev_id_t handle, enum e_spi edata, ARM_SPI_SignalEvent_t cb);
uint8_t kmdw_ssp_api_spi_receive(struct st_ssp_spi *stspi);
uint8_t kmdw_ssp_api_spi_enable(kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *stspi);
uint8_t kmdw_ssp_api_spi_disable(kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *stspi);
uint8_t kmdw_ssp_api_spi_transfer(kdrv_ssp_spi_dev_id_t handle);
uint8_t kmdw_ssp_api_spi_transfer_checks(kdrv_ssp_spi_dev_id_t handle);

#endif
