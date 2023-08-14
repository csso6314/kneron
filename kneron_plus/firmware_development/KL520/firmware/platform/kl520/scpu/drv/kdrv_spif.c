/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
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
*  kdrv_spif.c
*
*  Project:
*  --------
*  KL520
*
*  Description:
*  ------------
*  This SPI Flash driver is for Generic SPI Flash Access
*  HW: Faraday FTSPI020
*
*  Author:
*  -------
*  Teresa Chen
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

#include "kdrv_SPI020.h"
#include "kdrv_spif.h"
#include "io.h"

//#define SPIF_DBG
#ifdef SPIF_DBG
#include "kmdw_console.h"
#define spif_msg(fmt, ...) info_msg("[KDRV_SPIF] " fmt, ##__VA_ARGS__)
#else
#define spif_msg(fmt, ...)
#endif

#define min_t(x,y) ( x < y ? x: y )

typedef volatile union
{
	struct
	{
		uint32_t PA[6]; //0x100~0x114
	} dw;                         //double word
} kdrv_spif_io_reg_t;

#define kdrv_spif_io_reg ((kdrv_spif_io_reg_t *)(SCU_EXTREG_PA_BASE  + 0x100))	

void kdrv_spif_initialize(uint32_t clock)
{
    uint32_t   reg;
#if 1 //default is 12mA
    int mA;
    /*16mA*/
    //mA = 0; //4mA
    //mA = 1; //8mA
    //mA = 2; //12mA
    mA = 3; //16mA
    spif_msg("Set SPI driving = %d mA\n", (mA+1)*4);
    for(int i=0; i<6; i++)
    {
        spif_msg("addr = 0x%2X\n",addr);
        reg = kdrv_spif_io_reg->dw.PA[i];                //SPI IO control
        spif_msg("reg = 0x%2X\n",reg);
        reg &= ~0x000000C0;             //clear bit6, bit7
        spif_msg("reg = 0x%2X\n",reg);
        reg |= (mA<<6);        //select driving strength
        reg |= (1<<8);        //select slew rate slow
        spif_msg("reg = 0x%2X\n",reg);
        kdrv_spif_io_reg->dw.PA[i] = reg;
    }
#endif
#if 1
    regSPIF_ctrl->st.bf.kdrv_spif_cr.abort = 1;
    /* Wait reset completion */
    do {
        if(regSPIF_ctrl->st.bf.kdrv_spif_cr.abort == 0)
            break;
    } while(1);
#endif
    /* Set control register */
    reg = regSPIF_ctrl->st.dw.kdrv_spif_cr; // 0x80
    reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
    reg |= SPI_CLK_MODE0 | clock; // SCPU:200MHz, Flash: 50MHz
    regSPIF_ctrl->st.dw.kdrv_spif_cr = reg; 
    regSPIF_ctrl->st.bf.kdrv_spif_cr.XIP_port_sel = 0; /* make sure it's in Command slave port */
    kdrv_spif_pre_log();
}

void kdrv_spif_memxfer_initialize(uint8_t flash_mode, uint8_t mem_mode)
{
    uint32_t reg;
  	uint32_t ntemp;

  	//set as mode0 for SPI020_IO and driving 12mA
  	ntemp = ( 0<<0  | ( 3<<7 )  );
    for(int i=0; i<6; i++)
        kdrv_spif_io_reg->dw.PA[i] = ntemp;
    if (!(flash_mode & MEMXFER_INITED)) {
        regSPIF_ctrl->st.bf.kdrv_spif_cr.abort = 1;
        do
        {
            if(regSPIF_ctrl->st.bf.kdrv_spif_cr.abort == 0)
             break;
        }while(1);

        /* Set control register */
        reg = regSPIF_ctrl->st.dw.kdrv_spif_cr; // 0x80
        reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
        #if defined(SPI_BUS_SPEED) && (SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ)
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_2;
        #elif defined(SPI_BUS_SPEED) && (SPI_BUS_SPEED == SPI_BUS_SPEED_50MHZ)
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4;
        #else //SPI_BUS_SPEED == SPI_BUS_SPEED_25MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_8;
        #endif
        regSPIF_ctrl->st.dw.kdrv_spif_cr = reg;
        regSPIF_ctrl->st.bf.kdrv_spif_cr.XIP_port_sel = 0; /* make sure it's in Command slave port */
    }
    kdrv_spif_pre_log();
}

void kdrv_spif_set_commands(uint32_t cmd0, uint32_t cmd1, uint32_t cmd2, uint32_t cmd3)
{
    //spif_msg("cmd = 0x%4X 0x%4X 0x%4X 0x%4X\n",cmd0,cmd1,cmd2,cmd3);
    regSPIF_ctrl->st.dw.kdrv_spif_cmd0 = cmd0;
    regSPIF_ctrl->st.dw.kdrv_spif_cmd1 = cmd1;
    regSPIF_ctrl->st.dw.kdrv_spif_cmd2 = cmd2;
    regSPIF_ctrl->st.dw.kdrv_spif_cmd3 = cmd3;
}

/* Wait until command complete */
void kdrv_spif_wait_command_complete(void)
{
    uint32_t  reg;

    do {
        reg = regSPIF_irq->st.dw.kdrv_spif_isr;
    } while ((reg & SPI020_CMD_CMPLT)==0x0);
//  outw(SPI020REG_INTR_ST, (reg | SPI020_CMD_CMPLT));/* clear command complete status */
    regSPIF_irq->st.bf.kdrv_spif_isr.cmd_cmplt_sts=1;/* clear command complete status */
}

uint32_t gflash_clock_log;
void kdrv_spif_pre_log( void )
{
    gflash_clock_log = regSPIF_ctrl->st.dw.kdrv_spif_cr;
}

void kdrv_spif_switch_org( void )
{
    //	Reset SPI IP
    regSPIF_ctrl->st.bf.kdrv_spif_cr.abort=1;
    /* Wait reset completion */
    do {
        if(regSPIF_ctrl->st.bf.kdrv_spif_cr.abort==0)
            break;
    } while(1);
    regSPIF_ctrl->st.dw.kdrv_spif_cr = gflash_clock_log;
}

void kdrv_spif_switch_low_speed( void )
{
    uint32_t	reg ;
    //	//Reset SPI IP
    regSPIF_ctrl->st.bf.kdrv_spif_cr.abort=1;
    /* Wait reset completion */
    do {
        if(regSPIF_ctrl->st.bf.kdrv_spif_cr.abort==0)
            break;
    } while(1);
    /* Set control register */
    reg = regSPIF_ctrl->st.dw.kdrv_spif_cr;
    reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
    reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4;
    regSPIF_ctrl->st.dw.kdrv_spif_cr = reg;
}

uint8_t kdrv_spif_rx_FIFO_empty_check(void)
{
    if ( ( regSPIF_ctrl->st.bf.kdrv_spif_sr.RXFIFO_Ready ) != 0)
    {
        return 1;	//empty
    }
    return 0;//at leat 1 data
}


/* Wait until the rx fifo ready */
void kdrv_spif_wait_rx_full(void)
{
    while(!regSPIF_ctrl->st.bf.kdrv_spif_sr.RXFIFO_Ready);
}

/* Wait until the tx fifo ready */
void kdrv_spif_wait_tx_empty(void)
{
    while(!regSPIF_ctrl->st.bf.kdrv_spif_sr.TXFIFO_Ready);
}

/* Get the rx fifo depth, unit in byte */
uint32_t kdrv_spif_rxfifo_depth(void)
{
    return (uint8_t)(regSPIF_info->st.bf.kdrv_spif_feature.RXFIFO_DEPTH << 2);
}

/* Get the tx fifo depth, unit in byte */
uint32_t kdrv_spif_txfifo_depth(void)
{
//      unsigned int read;
    return (uint8_t)(regSPIF_info->st.bf.kdrv_spif_feature.TXFIFO_DEPTH << 2);
}

void kdrv_spif_read_Rx_FIFO( uint32_t *buf_word, uint16_t *buf_word_index, uint32_t target_byte )
{
    while( 1 )
    {
      while( kdrv_spif_rx_FIFO_empty_check() == 1 )
      {
        *( buf_word + *buf_word_index )= regSPIF_data->dw.kdrv_spif_dp;
        *buf_word_index = (*buf_word_index) + 1;
      }
      if(  (*buf_word_index*4) >= target_byte )
      {
      	return;
      }
    }
}

uint32_t kdrv_spif_read_compare(/*uint8_t*/uint32_t *buf, uint32_t length)
{
    uint32_t access_byte;//, tmp_read;
    uint32_t ret=0;

    while(length > 0)
    {
        kdrv_spif_wait_rx_full();
        access_byte = min_t(length, kdrv_spif_rxfifo_depth());
        length -= access_byte;
        while(access_byte > 0)
        {
            if(*buf == regSPIF_data->dw.kdrv_spif_dp)
                ret += 4;
            buf ++;
            if(access_byte>=4)
                access_byte -= 4;
            else
                access_byte=0;
        }
    }
    return ret;
}

void kdrv_spif_read_data(uint32_t *buf, uint32_t length)
{
    uint32_t  access_byte;//, tmp_read;

    while(length > 0)
    {
        kdrv_spif_wait_rx_full();
        access_byte = min_t(length, kdrv_spif_rxfifo_depth());
        length -= access_byte;
        while(access_byte > 0)
        {
            *buf= regSPIF_data->dw.kdrv_spif_dp;
            buf ++;
            if(access_byte>=4)
                access_byte -= 4;
            else
                access_byte=0;
            #if 0
            switch(access_byte)
            {
            case 1:
                tmp_read = inw((int8_t * )SPI020REG_DATAPORT);
                *buf = tmp_read&0xFF;
                access_byte = 0;//break while loop 
                break;
            case 2:
                tmp_read = inw((int8_t * )SPI020REG_DATAPORT);
                *buf = tmp_read&0xFF;
                buf++;
                *buf = (tmp_read&0xFF00)>>8;
                access_byte = 0;// break while loop 
                break;
            case 3:// read chip id will use this case 
                tmp_read = inw((int8_t * )SPI020REG_DATAPORT);
                *buf = tmp_read&0x00FF;
                buf++;
                *buf = (tmp_read&0xFF00)>>8;
                buf++;
                *buf = (tmp_read&0xFF0000)>>16;
                access_byte = 0;// break while loop 
                break;
            default:// access_byte>=4 
                *(uint32_t *)buf= inw((int8_t * )SPI020REG_DATAPORT);
                buf +=4;
                access_byte -= 4;
                break;
            }
            #endif
        }
    }
}

void kdrv_spif_check_status_till_ready_2(void)
{
#if 1
    uint32_t gSPI_RX_buff[4];
    uint16_t gSPI_RX_buff_index;
	volatile uint32_t countdown = 0;
    
	while(1)
	{
        countdown =1000;
		kdrv_spif_set_commands( SPI020_05_CMD0_w, SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w);
		gSPI_RX_buff_index = 0;
		kdrv_spif_read_Rx_FIFO( gSPI_RX_buff, &gSPI_RX_buff_index, 1 );
		kdrv_spif_wait_command_complete();

		if( (gSPI_RX_buff[0] & 0x01) == 0x00)
		{
			break;
		}
		while(countdown--);
	}
#else
//  main_delay_count(0x5100);

    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);
//  main_delay_count(0x80);
    /* wait for command complete */
    kdrv_spif_wait_command_complete();
#endif
}

void kdrv_spif_check_status_till_ready(void)
{
    /* savecodesize, move into here */
    kdrv_spif_wait_command_complete();

    /* read status */
    kdrv_spif_check_status_till_ready_2();

}

void kdrv_spif_check_quad_status_till_ready(void)
{
    /* savecodesize, move into here */
    kdrv_spif_wait_command_complete();

    /* read status */
    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);
//  main_delay_count(0x80);
    /* wait for command complete */
    kdrv_spif_wait_command_complete();

}

void kdrv_spif_write_data(uint8_t *buf, uint32_t length)
{
    int32_t  access_byte;

    /* This function assume length is multiple of 4 */
    while(length > 0) {
        kdrv_spif_wait_tx_empty();
        access_byte = min_t(length, kdrv_spif_txfifo_depth());
        length -= access_byte;
        while(access_byte > 0) {
            regSPIF_data->dw.kdrv_spif_dp = *((uint32_t *)buf);
            buf += 4;
            access_byte -= 4;
        }
    }
}

