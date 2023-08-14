#include <string.h>

#include "cmsis_os2.h"
#include "rtx_os.h"
#include "cmsis_compiler.h"

#include "ESP8266_Serial.h"

#include "kdrv_ssp.h"
#include "kdrv_gpio.h"
#include "kdrv_pinmux.h"

extern struct st_ssp_spi driver_ssp_master_ctx;

osThreadId_t    com_SPI_tid;
volatile uint32_t spi_rx_count = 0;
static serial_spi_mode_t spi_trans_mode = SPI_NULL;
//#define GPIO_SPI_HANDSHAKE GPIO_2

extern uint8_t RxBuf[0x1000];

struct st_ssp_spi *stspi_ctx = &driver_ssp_master_ctx;

static uint8_t Handshake_EventFlagsCb[osRtxEventFlagsCbSize] __ALIGNED(4) __attribute__((section(".bss.os.evflags.cb")));
osEventFlagsId_t Handshake_Eventflag_id = NULL;
/*
const osEventFlagsAttr_t Handshake_EventFlags_Attr = {
  .name    = "SPI Handshake Wait",
  .cb_mem  = Handshake_EventFlagsCb,
  .cb_size = sizeof(Handshake_EventFlagsCb)
};
*/
osEventFlagsId_t Handshake_event_init(osEventFlagsAttr_t * attr)
{
    osEventFlagsId_t flag = NULL ;
    flag = osEventFlagsNew (attr);
    return flag;
}

int32_t Handshake_Wait(osEventFlagsId_t evflags_id, uint32_t event, uint32_t timeout)
{
  int32_t rval;
  uint32_t flags;

  flags = osEventFlagsWait (evflags_id, event, osFlagsWaitAny, timeout);
  if ((flags & osFlagsError) == 0) {
    /* Got response */
    rval = 0;
  }
  else {
    if (flags == osFlagsErrorTimeout) {
      /* Timeout */
      rval = -1;
    }
    else {
      /* Internal error */
      rval = -2;
    }
  }
  osEventFlagsClear(evflags_id, event);
  return (rval);
}

#define INTERRUPT_PIN (KDRV_PIN_X_DPI_DATAO8) // FIXME, choose a GPIO pin
#define INTERRUPT_GPIO (GPIO_PIN_28) // FIXME, choose a GPIO pin

static void gpio_callback(kdrv_gpio_pin_t pin, void *arg)
{
  if(pin == INTERRUPT_GPIO)
  {
    osThreadFlagsSet(com_SPI_tid, SPI_READ); //ready to send data
  }
}

void ssp_spi_handshake_init()
{
	// FIXME
	//kdp520_gpio_setmode(pin_num);
  //kdp520_gpio_setedgemode(1 << pin_num, 0); // set interrupt trigger will be done by single edge
	//kdp520_gpio_enableint( pin_num, GPIO_EDGE, GPIO_Rising);// enable rising edge interrupt

  kdrv_pinmux_config(INTERRUPT_PIN, PIN_MODE_7, PIN_PULL_DOWN,  PIN_DRIVING_8MA);
  kdrv_gpio_set_interrupt(INTERRUPT_GPIO, false);
  kdrv_gpio_set_attribute(INTERRUPT_GPIO, (GPIO_DIR_INPUT | GPIO_INT_EDGE_RISING));
  kdrv_gpio_set_debounce(INTERRUPT_GPIO, true, 1000);
  kdrv_gpio_set_interrupt(INTERRUPT_GPIO, true);
  kdrv_gpio_register_callback(gpio_callback, NULL);
}

void kmdw_spi_device_transmit(struct st_ssp_spi* stspi, serial_spi_mode_t mode, uint8_t* buf, uint32_t send_len, uint32_t recv_len)
{
	uint32_t res_call = 0;
	
	if(mode == SPI_READ)
	{
		  res_call = 1;
	}
	kdrv_ssp_write_buff(stspi, buf, send_len);
	kdrv_ssp_SPI_master_transmit(stspi, recv_len,res_call);
}

/*
	     command（1byte）	address（1byte）	data（len<=64byte）
write 	0x3	             0x0	            actual data
read  	0x2	             0x0	            actual data
*/
int32_t Serial_spi_master_tran_data(serial_spi_mode_t mode, uint8_t* data, uint32_t len)
{
    uint8_t send_buf[72] = {0};

    uint32_t recv_len =0;
		uint32_t send_len = 0;
		
     if (mode == SPI_READ) {
         send_buf[0] = SPI_MASTER_READ_DATA_FROM_SLAVE_CMD;
     }
     else if (mode == SPI_WRITE){
         send_buf[0] = SPI_MASTER_WRITE_DATA_TO_SLAVE_CMD;
				 //dbg_msg_console("[%s]%d,LEN:%s\n",__FUNCTION__,__LINE__,data);
     }
		 send_len = len;
    send_buf[1] = SPI_TRANS_VARIABLE_ADDR;

    memcpy(&send_buf[2],(uint8_t *)data, len);
		
    kmdw_spi_device_transmit(&driver_ssp_master_ctx, mode, send_buf,send_len+2, recv_len);
    return 0;
}



int32_t Serial_spi_master_get_trans_len()
{
	int32_t len = 0;
    uint8_t send_buf[15] = {0};

    send_buf[0] = SPI_MASTER_READ_STATUS_FROM_SLAVE_CMD;
		
	memset((void *)driver_ssp_master_ctx.Rx_buffer, 0, 5);
    kmdw_spi_device_transmit(&driver_ssp_master_ctx, SPI_READ, send_buf,5,0);
		
	len = (driver_ssp_master_ctx.Rx_buffer[1]) | (driver_ssp_master_ctx.Rx_buffer[2]<<8) | (driver_ssp_master_ctx.Rx_buffer[3] << 16) | (driver_ssp_master_ctx.Rx_buffer[4] <<24);
	
	return len;
}

// SPI status transmit function(Master sendto slave data length) , address length is 0 bit(no address)
void Serial_spi_master_set_trans_len(uint32_t len)
{
    uint8_t send_buf[5] = {0};

    send_buf[0] = SPI_MASTER_WRITE_STATUS_TO_SLAVE_CMD;

    memcpy(&send_buf[1],&len,4);

    kmdw_spi_device_transmit(&driver_ssp_master_ctx, SPI_WRITE, send_buf,5,0);
		
	if(len == 0)
	{
		 kdrv_ssp_clear_tx_buf_index( &driver_ssp_master_ctx );
	}
}

void kdp520_spi_com_thread(void *arg)
{
   uint32_t flags;
   uint32_t transmit_data[64];


   uint32_t transmit_len = 0;
   uint32_t read_len = 0;
   uint8_t ridx = 0;
	
	 //uint8_t Rxbuf[64] = {0};

   while(1)
   {
      memset((uint8_t*)transmit_data, 0x0, 64);
      flags = osThreadFlagsWait(FLAG_SPI_HANDSHAKE, osFlagsWaitAny, osWaitForever);
      osThreadFlagsClear(flags);
      
      switch(spi_trans_mode)
      {
          case SPI_NULL: //some data need to write or read
            if(driver_ssp_master_ctx.Tx_buffer_index)
            {
                spi_trans_mode = SPI_WRITE;
                transmit_len = driver_ssp_master_ctx.Tx_buffer_index;
                osThreadFlagsSet(com_SPI_tid, FLAG_SPI_HANDSHAKE); //ready to send data
            }
            else
            {
                // Check if there is any data to receive
                transmit_len  = Serial_spi_master_get_trans_len();
                if(transmit_len > 0)
                {
                    spi_trans_mode = SPI_READ;
                    osThreadFlagsSet(com_SPI_tid, FLAG_SPI_HANDSHAKE); //ready to send data
                }
            }

          break;

          case SPI_WRITE:
            read_len =  transmit_len > SPI_MAX_BYTES ? SPI_MAX_BYTES : transmit_len;
						transmit_len -= read_len;
						osEventFlagsSet(Handshake_Eventflag_id, SPI_WAIT_HANDSHAKE); //ready to send data
            if(transmit_len == 0)
            {
                spi_trans_mode = SPI_NULL;
				driver_ssp_master_ctx.Tx_buffer_index = 0;
            }
          break;

          case SPI_READ:
            read_len =  transmit_len > SPI_MAX_BYTES ? SPI_MAX_BYTES : transmit_len;
            Serial_spi_master_tran_data(SPI_READ, (uint8_t *)transmit_data, read_len);
						
						memcpy(RxBuf+ridx,(char *)&driver_ssp_master_ctx.Rx_buffer[2],read_len); //need to modify Rxbuf to RxBuf,global variable		
            
            ridx+=read_len;
            transmit_len -= read_len;
             
             if(transmit_len == 0)
             {
                 spi_rx_count = read_len;
                 spi_trans_mode = SPI_NULL;
                 ridx = 0;
							 
								 extern void WIFI_COM_Callback (uint32_t event) ;
                 WIFI_COM_Callback(SPI_RECEIVE_COMPLETE_EVENT);
                 if(driver_ssp_master_ctx.Tx_buffer_index)
                 {
                     //osThreadFlagsSet(com_SPI_tid, FLAG_SPI_HANDSHAKE); //ready to send data
                 }
             }
          break;
      }
     
      
   } 
}
void kdp520_spi_com(void)
{
    
     osThreadAttr_t attr = {
        .stack_size = 512,
        .priority = osPriorityRealtime
    };

    ssp_spi_handshake_init();
    Handshake_Eventflag_id = Handshake_event_init(NULL);

    com_SPI_tid = osThreadNew( (osThreadFunc_t)kdp520_spi_com_thread, NULL, &attr );

}
