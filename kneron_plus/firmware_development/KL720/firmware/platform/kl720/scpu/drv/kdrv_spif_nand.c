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
*  KL720
*
*  Description:
*  ------------
*  This SPI Flash driver is for Generic SPI Flash Access
*  HW: Faraday FTSPI020
*
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/
#include "kdrv_SPI020_nand.h"
#include "kdrv_spif_nand.h"
#include "kdrv_gdma3.h"
#include "io.h"

//#define SPIF_DBG
#ifdef SPIF_DBG
#define spif_msg(fmt, ...)  printf("[KDRV_SPIF] " fmt, ##__VA_ARGS__)
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

#define kdrv_spif_io_reg ((kdrv_spif_io_reg_t *)(SCU_EXT_REG_BASE  + 0x200))	

void kdrv_spif_initialize(uint32_t clock)
{
    uint32_t   reg;
#if 1 //default is 12mA
    int mA;
    int i=0;
    /*16mA*/
    //mA = 0; //4mA
    //mA = 1; //8mA
    //mA = 2; //12mA
    mA = 3; //16mA
    spif_msg("Set SPI driving = %d mA\n", (mA+1)*4);
    for(i=0; i<6; i++)
    {
        reg = kdrv_spif_io_reg->dw.PA[i];                //SPI IO control
        spif_msg("reg = 0x%2X\n",reg);
        reg &= ~0x00600000;             //clear bit21, bit22
        spif_msg("reg = 0x%2X\n",reg);
        reg |= (mA<<21);        //select driving strength
        reg |= (1<<23);        //select slew rate slow
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
    reg |= SPI_CLK_MODE0 | clock; // SCPU:200MHz, Flash: 100/50/25MHz
    regSPIF_ctrl->st.dw.kdrv_spif_cr = reg; 
    regSPIF_ctrl->st.bf.kdrv_spif_cr.XIP_port_sel = 0; /* make sure it's in Command slave port */
    kdrv_spif_pre_log();
}

void kdrv_spif_memxfer_initialize(uint8_t flash_mode, uint8_t mem_mode)
{
    uint32_t reg;
  	uint32_t ntemp;
    int i=0;

  	//set as mode0 for SPI020_IO and driving 12mA
  	ntemp = ( 0<<0  | ( 3<<7 )  );
    for(i=0; i<6; i++)
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
    #if SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_2;
    #elif SPI_BUS_SPEED == SPI_BUS_SPEED_50MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4;
    #elif SPI_BUS_SPEED == SPI_BUS_SPEED_25MHZ
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
    if(bGigaDeive_Fseries && (cmd1 == SPI020_6B_CMD1))
    {
        regSPIF_ctrl->st.dw.kdrv_spif_cmd0 = cmd0;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd1 = SPI020_6B_CMD1_f;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd2 = cmd2;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd3 = SPI020_6B_CMD3_f;
    }
    else if(bGigaDeive_Fseries && (cmd1 == SPI020_03_CMD1))
    {
        regSPIF_ctrl->st.dw.kdrv_spif_cmd0 = cmd0;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd1 = SPI020_03_CMD1_f;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd2 = cmd2;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd3 = SPI020_03_CMD3_f;
    }
    else
    {
        regSPIF_ctrl->st.dw.kdrv_spif_cmd0 = cmd0;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd1 = cmd1;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd2 = cmd2;
        regSPIF_ctrl->st.dw.kdrv_spif_cmd3 = cmd3;
    }
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
    return (regSPIF_info->st.bf.kdrv_spif_feature.RXFIFO_DEPTH << 2);
}

/* Get the tx fifo depth, unit in byte */
uint32_t kdrv_spif_txfifo_depth(void)
{
//      unsigned int read;
    return (regSPIF_info->st.bf.kdrv_spif_feature.TXFIFO_DEPTH << 2);
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

void kdrv_spif_read_data(/*uint8_t*/uint32_t *buf, uint32_t length)
{
    uint32_t  access_byte;//, tmp_read;

    while(length > 0)
    {
        kdrv_spif_wait_rx_full();
        access_byte = min_t(length, kdrv_spif_rxfifo_depth());
        length -= access_byte;
        while(access_byte > 0)
        {
            *buf = regSPIF_data->dw.kdrv_spif_dp;
            buf ++;
            if(access_byte>=4)
                access_byte -= 4;
            else
                access_byte=0;
        }
    }
}

void kdrv_spif_check_status_till_ready_2(void)
{
#if 0
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

uint8_t kdrv_spif_get_flash_status(uint32_t status_index)
{
    uint32_t status;
    uint16_t gSPI_RX_buff_index;
    
	kdrv_spif_set_commands( status_index, SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w);
	gSPI_RX_buff_index = 0;
	kdrv_spif_read_Rx_FIFO( &status, &gSPI_RX_buff_index, 1 );
	kdrv_spif_wait_command_complete();
    return (uint8_t)status;
}

void kdrv_spif_reset_device(void)
{
    kdrv_spif_set_commands(SPI020_FF_CMD0, SPI020_FF_CMD1, SPI020_FF_CMD2, SPI020_FF_CMD3);
    kdrv_spif_check_status_till_ready();
}

extern void kdrv_delay_us(uint32_t usec);
void kdrv_spif_write_data_nand(uint8_t  type, uint32_t dst, uint8_t *buf, uint32_t length)
{
    int32_t  access_byte;
    uint32_t write_len, write_len2;
    uint32_t total_lens = length;
    uint32_t write_dst_addr = dst;
    while (total_lens > 0) {
        kdrv_spif_set_commands(SPI020_06_CMD0, SPI020_06_CMD1, SPI020_06_CMD2, SPI020_06_CMD3);
        kdrv_spif_wait_command_complete();
        write_len = min_t(total_lens, spi_nand_data_buf_size);
        spif_msg("write_len 0x%X bytes!\n",write_len);
        if(type & FLASH_QUAD_RW)
        {
            kdrv_spif_set_commands(0, SPI020_32_CMD1, write_len, SPI020_32_CMD3);
        }
        else
        {
            kdrv_spif_set_commands(0, SPI020_02_CMD1, write_len, SPI020_02_CMD3);
        }
        
        write_len2 = write_len;
        while(write_len2 > 0)
        {
            kdrv_spif_wait_tx_empty();
            access_byte = min_t(write_len2, kdrv_spif_txfifo_depth());
            write_len2 -= access_byte;
            while(access_byte > 0)
            {
                regSPIF_data->dw.kdrv_spif_dp = *((uint32_t *)buf);
                buf += 4;
                access_byte -= 4;
            }
        }        
        kdrv_spif_check_status_till_ready();
        uint32_t page_addr = write_dst_addr/spi_nand_data_buf_size;
        kdrv_spif_set_commands(page_addr, SPI020_10_CMD1, SPI020_10_CMD2, SPI020_10_CMD3); 
        //kdrv_delay_us(50);
        kdrv_spif_check_status_till_ready();
        write_dst_addr += write_len;
        total_lens -= write_len;
        spif_msg("execute program to page address 0x%X total %d bytes!\n",page_addr,write_len);
    }
}

void kdrv_spif_write_data_nand_dma(uint8_t  type, uint32_t dst, uint8_t *buf, uint32_t length)
{
    uint32_t write_len;
    uint32_t total_lens = length;
    uint32_t write_dst_addr = dst;
    gdma_setting_t dma_settings;
    regSPIF_irq->st.bf.kdrv_spif_icr.DMA_EN = 1;
    kdrv_gdma_handle_t ch = kdrv_gdma_acquire_handle();
    kdrv_spif_set_commands(SPI020_06_CMD0, SPI020_06_CMD1, SPI020_06_CMD2, SPI020_06_CMD3);
    kdrv_spif_wait_command_complete();
    while (total_lens > 0) {
        write_len = min_t(total_lens, spi_nand_data_buf_size);
        spif_msg("write_len 0x%X bytes!\n",write_len);
        if(type & FLASH_QUAD_RW)
        {
            kdrv_spif_set_commands(0, SPI020_32_CMD1, write_len, SPI020_32_CMD3);
        }
        else
        {
            kdrv_spif_set_commands(0, SPI020_02_CMD1, write_len, SPI020_02_CMD3);
        }
        
        //DMA
        dma_settings.burst_size = GDMA_BURST_SIZE_1;
        dma_settings.dma_mode = GDMA_HW_HANDSHAKE_MODE;
        dma_settings.dst_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_settings.src_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_settings.dma_dst_req = GDMA_HW_REQ_SPIF;
        dma_settings.dma_src_req = GDMA_HW_REQ_NONE;
        dma_settings.dst_addr_ctrl = GDMA_FIXED_ADDRESS;
        dma_settings.src_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        kdrv_gdma_configure_setting(ch, (gdma_setting_t *) &dma_settings);
        kdrv_gdma_transfer(ch,(uint32_t)&regSPIF_data->dw.kdrv_spif_dp, (uint32_t)buf, (uint32_t)total_lens, NULL, NULL);
        regSPIF_irq->st.bf.kdrv_spif_icr.cmd_cmplt_intr_en = 1;
        kdrv_spif_wait_command_complete();

        uint32_t page_addr = write_dst_addr/spi_nand_data_buf_size;
        kdrv_spif_set_commands(page_addr, SPI020_10_CMD1, SPI020_10_CMD2, SPI020_10_CMD3); 
        //kdrv_delay_us(50);
        kdrv_spif_check_status_till_ready();
        write_dst_addr += write_len;
        buf +=  write_len;
        total_lens -= write_len;
        spif_msg("execute program to page address 0x%X total %d bytes!\n",page_addr,write_len);
    }
    kdrv_gdma_release_handle(ch);
    regSPIF_irq->st.bf.kdrv_spif_icr.DMA_EN = 0;
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

void kdrv_spif_mark_badblock_flag(uint32_t iblock)
{
    int32_t  access_byte;
    uint32_t write_len;
    kdrv_spif_set_commands(SPI020_06_CMD0, SPI020_06_CMD1, SPI020_06_CMD2, SPI020_06_CMD3);
    kdrv_spif_wait_command_complete();
    write_len = spi_nand_data_buf_size+4;
    spif_msg("write_len 0x%X bytes!\n",write_len);
    kdrv_spif_set_commands(0, SPI020_02_CMD1, write_len, SPI020_02_CMD3);
    
    while(write_len > 0)
    {
        kdrv_spif_wait_tx_empty();
        access_byte = min_t(write_len, kdrv_spif_txfifo_depth());
        write_len -= access_byte;
        while(access_byte > 0)
        {
            regSPIF_data->dw.kdrv_spif_dp = 0;
            access_byte -= 4;
        }
    }        
    kdrv_spif_check_status_till_ready();
    kdrv_spif_set_commands(iblock<<6, SPI020_10_CMD1, SPI020_10_CMD2, SPI020_10_CMD3); 
    kdrv_spif_check_status_till_ready();
    spif_msg("execute program to page address 0x%X total %d bytes!\n",page_addr,write_len);
}

