#include <string.h>
#include <stdio.h>

#include "ESP8266_Serial.h"
#include "WiFi_ESP8266_Config.h"

#include "kdrv_ssp.h"
#include "kmdw_ssp.h"
//#include "pinmux.h"
#include "cmsis_os2.h"

/* Serial buffer sizes 
#ifndef SERIAL_TXBUF_SZ
#define SERIAL_TXBUF_SZ   0x1000
#endif*/
#define DEBUG_SPI_PORT SSP_SPI_PORT0

#ifndef SERIAL_RXBUF_SZ
#define SERIAL_RXBUF_SZ   0x1000
#endif

#define SPI_MASTER_HANDSHAKEGPIO 
//static serial_spi_mode_t trans_mode = SPI_NULL;

osMutexId_t spi_mutex_id;

/* Static functions */
void WIFI_COM_Callback (uint32_t event);
extern volatile uint32_t spi_rx_count;


typedef struct {
  uint32_t rxc;           /* Rx buffer count */
  //uint32_t rxi;           /* Rx buffer index */
//  uint32_t txi;           /* Tx buffer index */
  uint8_t  txb;           /* Tx busy flag    */
  uint8_t  r[3];          /* Reserved        */
} WIFI_SERIAL_COM;

uint8_t RxBuf[SERIAL_RXBUF_SZ];
//static uint8_t TxBuf[SERIAL_TXBUF_SZ];

static WIFI_SERIAL_COM wifi_serial_com;

/**
  Callback from the USART driver
*/

void WIFI_COM_Callback (uint32_t event) {
  //int32_t stat;
  uint32_t flags;

  flags = 0U;

	if (event & (SPI_RECEIVE_COMPLETE_EVENT)) {
		
		flags |= SERIAL_CB_RX_DATA_AVAILABLE;
  }

  if (event & SPI_SEND_COMPLETE_EVENT) {
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
__weak void Serial_Cb (uint32_t event) {
  (void)event;
}


/**
  Initialize serial interface.

  \return 0:ok, 1:error
*/
int32_t Serial_Initialize (void) {

  //ssp_spi_master_init();
  if( kmdw_ssp_api_spi_init(DEBUG_SPI_PORT, e_spi_init_master,NULL) != 1 )
  {
      return 1;
  }
  spi_mutex_id = osMutexNew(NULL);
  return (0);
}


/**
  Uninitialize serial interface.

  \return 0:ok
*/
int32_t Serial_Uninitialize (void) {

  memset (RxBuf, 0x00, SERIAL_RXBUF_SZ);
  //memset (TxBuf, 0x00, SERIAL_TXBUF_SZ);

  return (0);
}

int32_t  Serial_SetBaudrate (uint32_t baudrate)
{
  return 0;
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



int32_t Serial_SendBuf (const uint8_t *buf, uint32_t len) {
    
  int32_t ret = 0;
	
	osMutexAcquire (spi_mutex_id, osWaitForever);
	
  Serial_spi_master_set_trans_len(len);

  ret = Handshake_Wait(Handshake_Eventflag_id, SPI_WAIT_HANDSHAKE, 200);
  if(ret < 0)
  {
    return -1;
  }
	
	Serial_spi_master_tran_data(SPI_WRITE, (uint8_t *)buf ,len);
  Handshake_Wait(Handshake_Eventflag_id, SPI_WAIT_HANDSHAKE, 20);
 
  // send 0 to clear send length, and tell Slave send done
  Serial_spi_master_set_trans_len(0);
  osMutexRelease (spi_mutex_id);
  wifi_serial_com.txb = 1U;

  return len;
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
  //#include "dbg.h" 
	//dbg_msg_console("[%s]%d,info:%s\n",__FUNCTION__,__LINE__,buf);
  return ( n);
}

void Serial_rx_clear()
{
 	spi_rx_count = 0;
}
/**
  Retrieve total number of bytes to read
*/
uint32_t Serial_GetRxCount(void) {
  uint32_t u;

  u = spi_rx_count;

  return (u);
	
}

uint32_t Serial_GetTxCount(void) {
  uint32_t n;
  
  return (n);
}



