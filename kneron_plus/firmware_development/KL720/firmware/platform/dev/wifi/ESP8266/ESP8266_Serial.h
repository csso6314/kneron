/* -----------------------------------------------------------------------------
 * Copyright (c) 2019 Arm Limited (or its affiliates). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * $Date:        12. November 2019
 * $Revision:    V1.0
 *
 * Project:      Simple serial buffer
 * -------------------------------------------------------------------------- */

#ifndef ESP8266_SERIAL_H__
#define ESP8266_SERIAL_H__

#include <stdint.h>

/* Callback events */
#define SERIAL_CB_RX_DATA_AVAILABLE    1U
#define SERIAL_CB_TX_DATA_COMPLETED    2U
#define SERIAL_CB_RX_ERROR             4U
#define SERIAL_CB_TX_ERROR             8U


#define SERIAL_SPI 1

#if SERIAL_SPI

#include <cmsis_os2.h>

#define FLAG_SPI_HANDSHAKE   0x0001
#define FLAG_SPI_LOAD_DATA   0x0002

typedef enum {
    SPI_NULL = 0,
    SPI_READ,             // slave -> master
    SPI_WRITE             // maste -> slave
} serial_spi_mode_t;


/* SPI data cmd definition */
#define SPI_MASTER_WRITE_DATA_TO_SLAVE_CMD     2
#define SPI_MASTER_READ_DATA_FROM_SLAVE_CMD    3

/* SPI status cmd definition */
#define SPI_MASTER_WRITE_STATUS_TO_SLAVE_CMD   1
#define SPI_MASTER_READ_STATUS_FROM_SLAVE_CMD  4

#define SPI_TRANS_VARIABLE_ADDR 0

#define SPI_WAIT_HANDSHAKE  1U
#define SPI_MAX_BYTES 64
#define SPI_RECEIVE_COMPLETE_EVENT 1U
#define SPI_SEND_COMPLETE_EVENT 2U

extern osEventFlagsId_t Handshake_Eventflag_id;
extern osThreadId_t    com_SPI_tid;

int32_t Serial_spi_master_tran_data(serial_spi_mode_t mode, uint8_t* data, uint32_t len);
int32_t Handshake_Wait(osEventFlagsId_t evflags_id, uint32_t event, uint32_t timeout);
//int Serial_spi_master_get_trans_len();
void Serial_spi_master_set_trans_len(uint32_t len);

#endif

int32_t  Serial_Initialize (void);
int32_t  Serial_Uninitialize (void);
int32_t  Serial_SetBaudrate (uint32_t baudrate);
int32_t  Serial_SendBuf (const uint8_t *buf, uint32_t len);
int32_t  Serial_ReadBuf(uint8_t *buf, uint32_t len);
uint32_t Serial_GetRxCount(void);
uint32_t Serial_GetTxCount(void);
uint32_t Serial_GetTxFree (void);
void Serial_rx_clear(void);

void Serial_Cb (uint32_t cb_event);

#endif /* ESP8266_SERIAL_H__ */
