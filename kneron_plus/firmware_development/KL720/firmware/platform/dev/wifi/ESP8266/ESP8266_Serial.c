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

#include <string.h>
#include <stdio.h>
#include "regbase.h"
#include "pinmux.h"
#include "kdp_uart.h"
#include "ESP8266_Serial.h"
#include "WiFi_ESP8266_Config.h"



/* Serial buffer sizes 
#ifndef SERIAL_TXBUF_SZ
#define SERIAL_TXBUF_SZ   0x1000
#endif*/

#ifndef SERIAL_RXBUF_SZ
#define SERIAL_RXBUF_SZ   0x1000
#endif

/* Static functions */
static void WIFI_COM_Callback (uint32_t event);

typedef struct {
  //ARM_DRIVER_USART *drv;  /* UART driver */
  uint32_t rxc;           /* Rx buffer count */
  //uint32_t rxi;           /* Rx buffer index */
//  uint32_t txi;           /* Tx buffer index */
  uint8_t  txb;           /* Tx busy flag    */
  kdp_uart_hdl_t  handle; /*uart port*/
  uint8_t  r[3];          /* Reserved        */
} WIFI_SERIAL_COM;

static uint8_t RxBuf[SERIAL_RXBUF_SZ];
//static uint8_t TxBuf[SERIAL_TXBUF_SZ];

static WIFI_SERIAL_COM wifi_serial_com;
//#define AT_UART_PORT DRVUART_PORT4 //default commucate uart port(wifi module <------> kneron) 

/**
  Callback from the USART driver
*/

static void WIFI_COM_Callback (uint32_t event) {
  //int32_t stat;
  uint32_t flags;

  flags = 0U;

	if (event & (ARM_USART_EVENT_RX_TIMEOUT | ARM_USART_EVENT_RECEIVE_COMPLETE)) {
		
		flags |= SERIAL_CB_RX_DATA_AVAILABLE;
		
		if(event & ARM_USART_EVENT_RECEIVE_COMPLETE)
		{
			
			/* Increment counter of received bytes */
			//wifi_serial_com.rxc += SERIAL_RXBUF_SZ;
		}
		
  }

  if (event & ARM_USART_EVENT_SEND_COMPLETE) {
    flags |= SERIAL_CB_TX_DATA_COMPLETED;
    /* Clear tx busy flag */
    wifi_serial_com.txb = 0U;
  } 
  /* Send events */
  Serial_Cb (flags);
}

/**
  Event callback.
*/
__WEAK void Serial_Cb (uint32_t event) {
  (void)event;
}


/**
  Initialize serial interface.

  \return 0:ok, 1:error
*/
int32_t Serial_Initialize (void) {

	UINT32	data;
  /* Initialize serial control structure */
  wifi_serial_com.rxc = 0U;
 // wifi_serial_com.rxi = 0U;
//  wifi_serial_com.txi = 0U;
  wifi_serial_com.txb = 0U;
  wifi_serial_com.handle = 0U;
 
 	data = inw( SCU_EXTREG_PA_BASE + 0x18C );
	data &= 0xFFFFFFF8;		//clear low 3bit
	data &= 0xFFFFFFE7;		//clear bit 3 and bit4
	
	outw( SCU_EXTREG_PA_BASE + 0x18C, data | 0x6 | 1<<4 );
	data = inw( SCU_EXTREG_PA_BASE + 0x190 );
	data &= 0xFFFFFFF8;		//clear low 3bit
	data &= 0xFFFFFFE7;		//clear bit 3 and bit4
	
	outw( SCU_EXTREG_PA_BASE + 0x190, data | 0x6 | 1<<4 );

   wifi_serial_com.handle = kdp_uart_open(wifi_serial_com.handle, UART_MODE_ASYN_RX | UART_MODE_SYNC_TX, WIFI_COM_Callback);	
  if (wifi_serial_com.handle != UART_FAIL)
  {
    //printf(" WIFI UART open failed\n");
    return -1;
  }
	
  
	if ((kdp_uart_power_control(wifi_serial_com.handle, ARM_POWER_FULL)) != UART_API_RETURN_SUCCESS)
        return -2;
	
  KDP_UART_CONFIG_t cfg;
	cfg.baudrate = BAUD_115200;
	cfg.data_bits = 8;
	cfg.frame_length = 0;
	cfg.stop_bits = 1;
	cfg.parity_mode = PARITY_NONE;
	cfg.fifo_en = true;

   if ((kdp_uart_control(wifi_serial_com.handle, UART_CTRL_CONFIG, (void *)&cfg)) == UART_API_RETURN_SUCCESS) {
        kdp_uart_read( wifi_serial_com.handle, RxBuf, SERIAL_RXBUF_SZ ); 
    }else
	 {
			return -3;
	 }
		
  return (0);
}


/**
  Uninitialize serial interface.

  \return 0:ok
*/
int32_t Serial_Uninitialize (void) {

  kdp_uart_close(wifi_serial_com.handle);

  memset (RxBuf, 0x00, SERIAL_RXBUF_SZ);
  //memset (TxBuf, 0x00, SERIAL_TXBUF_SZ);

  return (0);
}

/**
  Set serial interface baudrate.
  
  \param[in]  baudrate    desired baudrate
  \return 0:ok, 1:error
*/
int32_t Serial_SetBaudrate (uint32_t baudrate) {
 
	int32_t sts;
	wifi_serial_com.rxc = 0U;
  //wifi_serial_com.rxi = 0U;
//  wifi_serial_com.txi = 0U;
  wifi_serial_com.txb = 0U;
 
  KDP_UART_CONFIG_t cfg;
  cfg.baudrate = baudrate; //default baudrate 115200
  cfg.data_bits = 8;
  cfg.frame_length = 0;
  cfg.stop_bits = 1;
  cfg.parity_mode = PARITY_NONE;
  cfg.fifo_en = true;     
	
   sts= kdp_uart_control(wifi_serial_com.handle, UART_CTRL_CONFIG, (void *)&cfg);
  if (sts == UART_API_RETURN_SUCCESS) {
     kdp_uart_read( wifi_serial_com.handle, RxBuf, SERIAL_RXBUF_SZ );   
  }

	return (sts);
 
}

/**
  Get number of bytes free in transmit buffer.

  \return number of bytes
*/
uint32_t Serial_GetTxFree (void) {
  uint32_t n;

  if (Serial_GetTxCount() != 0U) {
    n = 0;
  } else {
    //n = SERIAL_TXBUF_SZ;
		n = 0x1000;
  }

  return (n);
}

/**
  Try to send len of characters from the specified buffer.

  If there is not enough space in the transmit buffer, number
  of characters sent will be less than specified with len.

  \return number of bytes actually sent or -1 in case of error
*/
int32_t Serial_SendBuf (const uint8_t *buf, uint32_t len) {
  
  int32_t  n;
  kdp_uart_api_sts_t  stat;


  stat = kdp_uart_write(wifi_serial_com.handle, (uint8_t *)buf, len);

  if (stat == UART_API_RETURN_SUCCESS) {
    wifi_serial_com.txb = 1U;
    n = len;
  }
  else {
    wifi_serial_com.txb = 0U;
    n = -1;
  }

  return n;
}


/**
  Read len characters from the serial receive buffers and put them into buffer buf.

  \return number of characters read
*/
int32_t Serial_ReadBuf(uint8_t *buf, uint32_t len) {
  
	uint32_t i, n;
	static uint32_t rxi = 0;
	wifi_serial_com.rxc = Serial_GetRxCount(); //get total data count
	n = wifi_serial_com.rxc;
	n -= rxi; // get reserved data count
	
	if(len < n)
	{
		n = len;
	}
	
	for (i = 0U; i < n; i++) 
	{
		 buf[i] = RxBuf[rxi++];
	}

	if(rxi == wifi_serial_com.rxc)
	{	
		rxi = 0;
		memset(RxBuf,0, wifi_serial_com.rxc);
		wifi_serial_com.rxc = 0;
	}
	
  return ( n);
}

void Serial_rx_clear()
{
	kdp_uart_read(wifi_serial_com.handle, RxBuf, SERIAL_RXBUF_SZ);
}
/**
  Retrieve total number of bytes to read
*/
uint32_t Serial_GetRxCount(void) {
  uint32_t u;
	
  u  = kdp_uart_GetRxCount(wifi_serial_com.handle);
  return (u);
	
}

uint32_t Serial_GetTxCount(void) {
  uint32_t n;

  n = kdp_uart_GetTxCount(wifi_serial_com.handle);
  return (n);
}



