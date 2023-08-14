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
*  kdev_flash_winbond.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  This SPI Flash driver is specific for Winbond SPI Flash Access
*  HW: Faraday FTSPI020
*
******************************************************************************/
#include "kdev_flash.h"
#include "io.h"
//#define FLASH_DBG
#ifdef FLASH_DBG
#include "kmdw_console.h"
#define flash_msg(fmt, ...) info_msg("[NOR FLASH] " fmt, ##__VA_ARGS__)
#else
#define flash_msg(fmt, ...)
#endif
#if defined (FLASH_SIZE) && (FLASH_SIZE < FLASH_SIZE_256MBIT)
#define FLASH_4BYTES_CMD_EN     0x00
#else
#define FLASH_4BYTES_CMD_EN     0x01
#endif

spi_flash_t flash_info;
#if ( FLASH_CODING_GET_INFO_EN == YES )
kdrv_spif_parameter_t st_flash_info;
#endif

#if FLASH_4BYTES_CMD_EN
bool kdev_flash_is_4byte_address(void)
{
    bool ret = false;

    if( flash_info.flash_size > FLASH_3BYTE_ADDR_MAX )
        ret = true;
    return ret;
}
#endif

void kdev_flash_4Bytes_ctrl(uint8_t  enable)
{
#if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address())
    {
        if (enable) {
            kdrv_spif_set_commands(SPI020_B7_CMD0, SPI020_B7_CMD1, SPI020_B7_CMD2, SPI020_B7_CMD3);
        } else {
            kdrv_spif_set_commands(SPI020_E9_CMD0, SPI020_E9_CMD1, SPI020_E9_CMD2, SPI020_E9_CMD3);
        }
        /* wait for command complete */
        kdrv_spif_wait_command_complete();
    }
#endif
}

void kdev_flash_write_control(uint8_t  enable)
{
    /* fill in command 0~3 */
    if (enable) {
        kdrv_spif_set_commands(SPI020_06_CMD0, SPI020_06_CMD1, SPI020_06_CMD2, SPI020_06_CMD3);
    } else {
        kdrv_spif_set_commands(SPI020_04_CMD0, SPI020_04_CMD1, SPI020_04_CMD2, SPI020_04_CMD3);
    }
    /* wait for command complete */
    kdrv_spif_wait_command_complete();
}

void kdev_flash_write_control_volatile(uint8_t  enable)
{
    /* fill in command 0~3 */
    if (enable) {
        kdrv_spif_set_commands(SPI020_50_CMD0, SPI020_50_CMD1, SPI020_50_CMD2, SPI020_50_CMD3);
    } else {
        kdrv_spif_set_commands(SPI020_04_CMD0, SPI020_04_CMD1, SPI020_04_CMD2, SPI020_04_CMD3);
    }
    /* wait for command complete */
    kdrv_spif_wait_command_complete();
}

void kdev_flash_64kErase(uint32_t  offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_64K) return 1; */

    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    #if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address())
        kdrv_spif_set_commands(offset, SPI020_DC_CMD1, SPI020_DC_CMD2, SPI020_DC_CMD3);
    else
        kdrv_spif_set_commands(offset, SPI020_D8_CMD1, SPI020_D8_CMD2, SPI020_D8_CMD3);
    #else
    kdrv_spif_set_commands(offset, SPI020_D8_CMD1, SPI020_D8_CMD2, SPI020_D8_CMD3);
    #endif
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
}

void kdev_flash_32kErase(uint32_t  offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_64K) return 1; */

    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    kdrv_spif_set_commands(offset, SPI020_52_CMD1, SPI020_52_CMD2, SPI020_52_CMD3);
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
}

void kdev_flash_4kErase(uint32_t  offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_4K) return 1; */

    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    #if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address())
        kdrv_spif_set_commands(offset, SPI020_21_CMD1, SPI020_21_CMD2, SPI020_21_CMD3);
    else
        kdrv_spif_set_commands(offset, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
    #else
    kdrv_spif_set_commands(offset, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
    #endif
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
}

uint32_t  kdev_flash_probe(spi_flash_t *flash)
{
    uint32_t   chip_id=0;

    uint32_t   probe_90_instruction=0;
    /* fill in command 0~3 */
    //Read Manufacturer and Device Identification by JEDEC ID(0x9F)
    kdrv_spif_set_commands(SPI020_9F_CMD0, SPI020_9F_CMD1, SPI020_9F_CMD2, SPI020_9F_CMD3);
    /* read data */
    kdrv_spif_read_data(/*(uint8_t  *)*/&chip_id, 0x3);
    /* wait for command complete */
    kdrv_spif_wait_command_complete();

    //flash->manufacturer = (chip_id>>24);
    flash->manufacturer = (uint8_t )chip_id;
    if (flash->manufacturer == 0x00 || flash->manufacturer == 0xFF) {
       /* fill in command 0~3 */
       //Read Manufacturer and Device Identification by 0x90
        kdrv_spif_set_commands(SPI020_90_CMD0, SPI020_90_CMD1, SPI020_90_CMD2, SPI020_90_CMD3);
        /* read data */
        kdrv_spif_read_data(/*(uint8_t  *)*/&chip_id, 0x02/*0x4*/);
        /* wait for command complete */
        kdrv_spif_wait_command_complete();
        //flash->manufacturer = (chip_id>>24);
        flash->manufacturer = (uint8_t )chip_id;
        probe_90_instruction=1;
    }
    flash->flash_id = (chip_id>>8);
    return probe_90_instruction;
}

/* ===================================
 * Init SPI controller and flash device.
 * Init flash information and register functions.
 * =================================== */
kdev_status_t kdev_flash_read_flash_id(void)
{
    uint32_t  probe_90_instruction;
    uint32_t  sizeId;

    probe_90_instruction=kdev_flash_probe(&flash_info);

#ifdef FLASH_DBG
    char *flash_manu;
    switch (flash_info.manufacturer) {
        case FLASH_WB_DEV:
            flash_manu = "WINBOND";
            break;
        case FLASH_MXIC_DEV:
            flash_manu = "MXIC";
            break;
        case FLASH_Micron_DEV:
            flash_manu = "Micron";
            break;
        case FLASH_GD_DEV:
            flash_manu = "GigaDevice";
            break;
        case FLASH_ZBIT_DEV:
            flash_manu = "Zbit";
            break;
        default:
            flash_manu = "Unknown";
            break;
    }
#endif
    if(probe_90_instruction)
    {
        sizeId = flash_info.flash_id & 0x00FF;
        if(sizeId >= FLASH_SIZE_1MB_ID)
            flash_info.flash_size = 0x400 * (1<<(sizeId-FLASH_SIZE_1MB_ID+1));
    }
    else
    {
        sizeId = (flash_info.flash_id & 0xFF00)>>8;
        if(sizeId >= FLASH_SIZE_512MB_ID) {
            flash_info.flash_size = 0x400 * (1<<(sizeId-FLASH_SIZE_1MB_ID-FLASH_SIZE_SHIFT));
            flash_msg("flash_size 0x%2X >= 512MB = %d kbytes\n",sizeId, flash_info.flash_size);
        }
        else if(sizeId >= FLASH_SIZE_1MB_ID) {
            flash_info.flash_size = 0x400 * (1<<(sizeId-FLASH_SIZE_1MB_ID));
            flash_msg("flash_size 0x%2X = %d kbytes\n",sizeId, flash_info.flash_size);
        }
    }

    flash_msg("Manufacturer ID = 0x%02X (%s)\n", flash_info.manufacturer,flash_manu);
    flash_msg("Device ID       = 0x");
    if(probe_90_instruction)
        flash_msg("%02X\n", flash_info.flash_id);
    else
        flash_msg("%04X\n", flash_info.flash_id);

    flash_msg("Flash Size      = ");
    if((flash_info.flash_size%1024)==0x00) {
        flash_msg("%dkByte(%dMByte)\n", flash_info.flash_size,flash_info.flash_size>>10);
    } else {
        flash_msg("%dkByte\n", flash_info.flash_size);
    }

    st_flash_info.ID = flash_info.manufacturer;
    st_flash_info.erase_4K_support = 1;
    st_flash_info.flash_size_KByte = flash_info.flash_size;
    st_flash_info.sector_size_Bytes = SPI020_SECTOR_SIZE;
    st_flash_info.total_sector_numbers = (st_flash_info.flash_size_KByte * 1024) / st_flash_info.sector_size_Bytes;
    return KDEV_STATUS_OK;
}

/* WB Flash */
void kdev_flash_read_status(void)
{
    uint16_t nrx_buff_word_index = 0;
    uint32_t RDSR1=0; //05h
    uint32_t RDSR2=0; //35h
    uint32_t RDSR3=0; //15h

    kdrv_spif_set_commands( SPI020_05_CMD0_w , SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR1, &nrx_buff_word_index, 0x01 );
    flash_msg("SPI020_05_CMD RDSR1=0x%2X\n", (uint8_t)RDSR1 );
    kdrv_spif_wait_command_complete();

    kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3 );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR2, &nrx_buff_word_index, 0x01 );
    flash_msg("SPI020_35_CMD RDSR2=0x%2X\n", (uint8_t)RDSR2 );
    kdrv_spif_wait_command_complete();

    kdrv_spif_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3 );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR3, &nrx_buff_word_index, 0x01 );
    flash_msg("SPI020_15_CMD RDSR3=0x%2X\n", (uint8_t)RDSR3 );
    kdrv_spif_wait_command_complete();

    flash_msg("Manufacturer ID = 0x%02X \n", flash_info.manufacturer);

    if(flash_info.manufacturer == FLASH_GD_DEV)
    {
        RDSR1 &= ~0xFC; /* disable Status Register & Block protect bit7,6,5,4,3,2 */
        if( (RDSR3  & 0x0C) ) /* check Program/Erase Error bits */
        {
            kdev_flash_write_control(1);
            flash_msg("need to clean program/erase error 0x%2X \n", RDSR3&~0x0C );
            kdrv_spif_set_commands(SPI020_30_CMD0, SPI020_30_CMD1, SPI020_30_CMD2, SPI020_30_CMD3 );
            flash_msg("SPI020_30_CMD0 done\n");
            kdrv_spif_wait_command_complete();
        }
        RDSR3  &= ~0x70; /* driver output strength 00 100% & clear ADP bit*/

        RDSR2 &= ~0x40; /* clear Status Register Protect 1 bit6 */
        #if defined(FLASH_QUAD_IO_EN) && (FLASH_QUAD_IO_EN == 1)
        flash_msg("Set QE enabled \n");
        RDSR2 |= BIT1;
        #endif
    }
    else if(flash_info.manufacturer == FLASH_MXIC_DEV)
    {
        RDSR1 &= ~0x3C; /* disable Block protect bit5,4,3,2*/
        RDSR3 &= ~0x08; /* disable Top/Bottom protect */
        RDSR3 |= 0x07; /* driver output strength 111 30Ohms(25L512) / 15Ohms(25L256) */
        /* check Quad mode */
        if(FLASH_OP_MODE & FLASH_QUAD_RW)
        {
            /* need to use command 01h */
            flash_msg("Set FLASH_MXIC_DEV QE enabled \n");
            RDSR1 |= BIT6; //QE enabled
        }
    }
    else //FLASH_WB_DEV
    {
        RDSR1 &= ~0xFC; /* disable Status Register & Block protect bit7,6,5,4,3,2 */
        
        RDSR2 &= ~BIT6; /* clear CMP */
        #if defined(FLASH_QUAD_IO_EN) && (FLASH_QUAD_IO_EN == 1)
        flash_msg("Set QE enabled \n");
        RDSR2 |= BIT1;
        #endif
        
        RDSR3 &= ~0x06; /* clear WPS & ADP bit */
        RDSR3 &= ~0x60; /* driver output strength 00 100% */
        flash_msg("need to set driver strength 0x%2X \n", (uint8_t)RDSR3 );
    }

    if(flash_info.manufacturer == FLASH_MXIC_DEV)
    {
        uint8_t buf[2];
        buf[0] = (uint8_t)RDSR1;
        buf[1] = (uint8_t)RDSR3;
        flash_msg("RDSR1(%4X) RDSR3(%4X) \n", buf[0], buf[1] );

        kdev_flash_write_control(1);
        flash_msg("kdev_flash_write_control done\n");
        kdrv_spif_set_commands(SPI020_01_CMD0, SPI020_01_CMD1, SPI020_01_CMD2, SPI020_01_CMD3 );
        kdrv_spif_write_data(buf, 2);
        flash_msg("spi020_check_status_til_ready\n");
        kdrv_spif_wait_command_complete();//kdrv_spif_check_status_till_ready();
    }
    else
    {
        flash_msg("RDSR1(%4X) RDSR2(%4X) RDSR3(%4X) \n", (uint8_t)RDSR1, (uint8_t)RDSR2, (uint8_t)RDSR3);

        kdev_flash_write_control_volatile(1);
        kdrv_spif_set_commands(SPI020_01_CMD0, SPI020_01_CMD1, 1, SPI020_01_CMD3 );
        kdrv_spif_write_data((uint8_t*)&RDSR1, 1);
        kdrv_spif_check_status_till_ready();
    
        kdev_flash_write_control_volatile(1);
        kdrv_spif_set_commands(SPI020_31_CMD0, SPI020_31_CMD1, SPI020_31_CMD2, SPI020_31_CMD3 );
        kdrv_spif_write_data((uint8_t*)(&RDSR2), 1);
        kdrv_spif_check_status_till_ready();
        #if defined(FLASH_QUAD_IO_EN) && (FLASH_QUAD_IO_EN == 1)
        flash_msg("FLASH_WB/GD/ZBIT Set QE OK!! \n");
        #endif
    
        kdev_flash_write_control_volatile(1);
        kdrv_spif_set_commands(SPI020_11_CMD0, SPI020_11_CMD1, SPI020_11_CMD2, SPI020_11_CMD3 );
        kdrv_spif_write_data((uint8_t*)&RDSR3, 1);
        kdrv_spif_check_status_till_ready();
    }
}

kdev_status_t kdev_flash_read_SFDP(void)
{
    #define SPI_Rx_SIZE     (5)
    uint16_t    nrx_buff_word_index = 0;
    uint32_t    nrx_buff_word[ SPI_Rx_SIZE ];
    uint32_t    ntemp =0;
    kdrv_spif_switch_low_speed();
    //check status
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( 0x00 , SPI020_5A_CMD1, 0x04, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
    kdrv_spif_wait_command_complete();
    st_flash_info.signature = nrx_buff_word[nrx_buff_word_index-1];//FLASH_SIGNATURE;

    //check
    if( nrx_buff_word[nrx_buff_word_index-1] != FLASH_SIGNATURE )
    {
        st_flash_info.ID = flash_info.manufacturer;
        st_flash_info.erase_4K_support = 1;
        st_flash_info.flash_size_KByte = flash_info.flash_size;
        st_flash_info.sector_size_Bytes = SPI020_SECTOR_SIZE;
        st_flash_info.total_sector_numbers = (st_flash_info.flash_size_KByte * 1024) / st_flash_info.sector_size_Bytes;
        return KDEV_STATUS_ERROR;
    }

    //get ptr
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( 0x0C , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdrv_spif_wait_command_complete();
    st_flash_info.PTP = nrx_buff_word[nrx_buff_word_index-1] & 0XFF;

    //get ID
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( 0x10 , SPI020_5A_CMD1, 0x04, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
    kdrv_spif_wait_command_complete();
    st_flash_info.ID = nrx_buff_word[nrx_buff_word_index-1] & 0XFFFFFFFF;

    if( st_flash_info.ID== 0x00 || st_flash_info.ID==0xFF  )
    {
        nrx_buff_word_index =0;
        kdrv_spif_set_commands( SPI020_9F_CMD0 , SPI020_9F_CMD1, SPI020_9F_CMD2, SPI020_9F_CMD3 );
        kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, SPI020_9F_CMD2 );
        kdrv_spif_wait_command_complete();
        st_flash_info.ID = nrx_buff_word[nrx_buff_word_index-1] & 0xFF;
    }

    //get 4K erase support
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( st_flash_info.PTP + 0, SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdrv_spif_wait_command_complete();
    st_flash_info.erase_4K_support = nrx_buff_word[nrx_buff_word_index-1] & 0x3;

    //get size
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( st_flash_info.PTP+4 , SPI020_5A_CMD1, 0x04, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
    kdrv_spif_wait_command_complete();
    st_flash_info.flash_size_KByte = (nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
    ntemp = nrx_buff_word[nrx_buff_word_index-1]>>3;

    //get sector size 0x1C
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( st_flash_info.PTP+0x1C , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdrv_spif_wait_command_complete();
    st_flash_info.sector_size_Bytes = 1<<(nrx_buff_word[ nrx_buff_word_index-1 ]&0xFF);
    st_flash_info.total_sector_numbers = (ntemp / st_flash_info.sector_size_Bytes)+1;

    //get sector size 0x20
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( st_flash_info.PTP+0x20 , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdrv_spif_wait_command_complete();
    st_flash_info.block_size_Bytes = ( 1<<( nrx_buff_word[ nrx_buff_word_index-1 ] & 0xFF ) )/st_flash_info.sector_size_Bytes ;

    //get page size
    nrx_buff_word_index =0;
    kdrv_spif_set_commands( st_flash_info.PTP+0x28 , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdrv_spif_wait_command_complete();
    ntemp = nrx_buff_word[nrx_buff_word_index-1]&0xFF;

    #if 0
    //20191219 add
    nrx_buff_word_index =0;
    kdrv_spif_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3);
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdrv_spif_wait_command_complete();
    nrx_buff_word_index =0;
    kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3);//bessel:wait interrupt instead of delay
    kdrv_spif_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdrv_spif_wait_command_complete();


    #endif

    kdrv_spif_switch_org();

    if( (ntemp>>4) == FLASH_PAGE_SIZE_256_CODE )
    {
        st_flash_info.page_size_Bytes = 256;
    }
    else
    {
        st_flash_info.page_size_Bytes = 0;
        return KDEV_STATUS_ERROR;
    }

    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_initialize(void)//ARM_Flash_SignalEvent_t cb_event)
{
    #if defined(FLASH_COMM) && (FLASH_COMM == FLASH_COMM_SPEED_100MHZ)
    kdrv_spif_initialize(SPI_CLK_DIVIDER_2);//reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_2; // SCPU:200MHz, Flash: 100MHz
    #elif defined(FLASH_COMM) && (FLASH_COMM == FLASH_COMM_SPEED_50MHZ)
    kdrv_spif_initialize(SPI_CLK_DIVIDER_4);//reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4; // SCPU:200MHz, Flash: 50MHz
    #else
    kdrv_spif_initialize(SPI_CLK_DIVIDER_8);//reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_8; // SCPU:200MHz, Flash: 25MHz
    #endif
    kdev_flash_read_flash_id();
    kdev_flash_read_status();
    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #else
    kdev_flash_4Bytes_ctrl(0);
    #endif
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_uninitialize(void)
{
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_power_control(ARM_POWER_STATE state)
{
    switch (state) {
    case ARM_POWER_OFF:
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        break;

    default:
        return KDEV_STATUS_ERROR;
    }
    return KDEV_STATUS_OK;
}

uint32_t kdev_flash_read_compare(void *buf, uint32_t  offset, uint32_t  len, uint8_t  type)
{
    uint32_t  *read_buf;//uint8_t                *read_buf;
    uint32_t ret=0;

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #endif

    if (type & FLASH_DMA_READ) {
        regSPIF_irq->st.dw.kdrv_spif_icr = SPI020_cmd_cmplt_intr_en | SPI020_DMA_EN;/* enable DMA function */
    }

    /* fill in command 0~3 */
    if (type & FLASH_DTR_RW) {
        if (type & FLASH_DUAL_READ)
                kdrv_spif_set_commands(offset, SPI020_BD_CMD1, len, SPI020_BD_CMD3);
            else if(type & FLASH_QUAD_RW)
                kdrv_spif_set_commands(offset, SPI020_ED_CMD1, len, SPI020_ED_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_0D_CMD1, len, SPI020_0D_CMD3);
    } else if (type & FLASH_DUAL_READ) {
        if(type & FLASH_IO_RW) {
            //fLib_printf("Daul (0xBB) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_BC_CMD1, len, SPI020_BC_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #endif
        } else {
            //fLib_printf("Daul (0x3B) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_3C_CMD1, len, SPI020_3C_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #endif
        }
    } else if(type & FLASH_QUAD_RW) {
        if(type & FLASH_IO_RW) {
            //fLib_printf("Quad (0xEB) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_EC_CMD1, len, SPI020_EC_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            #endif
        } else {
            //fLib_printf("Quad (0x6B) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_6C_CMD1, len, SPI020_6C_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
            #else
                kdrv_spif_set_commands(offset, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
            #endif
        }
    } else if(type & FLASH_FAST_READ) {
        //fLib_printf("Fast (0x0B) read\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_0C_CMD1, len, SPI020_0C_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #endif
    } else {/* normal read */
        //fLib_printf("Normal (0x03) read\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_13_CMD1, len, SPI020_13_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
        #endif
    }

    if (type & FLASH_DMA_READ) {
        return 0;
    }

    read_buf = (uint32_t  *)buf;
    ret = kdrv_spif_read_compare(read_buf, len);/* read data */
    kdrv_spif_wait_command_complete();/* wait for command complete */

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(0);
    #endif
    return ret;
}

void kdev_flash_read(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf)
{
    uint32_t  *read_buf;//uint8_t                *read_buf;

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #endif

    if (type & FLASH_DMA_READ) {
        regSPIF_irq->st.dw.kdrv_spif_icr = SPI020_cmd_cmplt_intr_en | SPI020_DMA_EN;/* enable DMA function */
    }

    /* fill in command 0~3 */
    if (type & FLASH_DTR_RW) { /* Please check flash datasheet which can support DTR or not */
        if (type & FLASH_DUAL_READ)
                kdrv_spif_set_commands(offset, SPI020_BD_CMD1, len, SPI020_BD_CMD3);
            else if(type & FLASH_QUAD_RW)
                kdrv_spif_set_commands(offset, SPI020_ED_CMD1, len, SPI020_ED_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_0D_CMD1, len, SPI020_0D_CMD3);
    } else if (type & FLASH_DUAL_READ) {
        if(type & FLASH_IO_RW) {
            //fLib_printf("Daul (0xBB) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_BC_CMD1, len, SPI020_BC_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #endif
        } else {
            //fLib_printf("Daul (0x3B) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_3C_CMD1, len, SPI020_3C_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #endif
        }
    } else if(type & FLASH_QUAD_RW) {
        if(type & FLASH_IO_RW) {
            //fLib_printf("Quad (0xEB) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_EC_CMD1, len, SPI020_EC_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            #endif
        } else {
            //fLib_printf("Quad (0x6B) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_6C_CMD1, len, SPI020_6C_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
            #else
                kdrv_spif_set_commands(offset, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
            #endif
        }
    } else if(type & FLASH_FAST_READ) {
        //fLib_printf("Fast (0x0B) read\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_0C_CMD1, len, SPI020_0C_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #endif
    } else {/* normal read */ /* Please be noted that 03h command only can support max. 50MHz */
        //fLib_printf("Normal (0x03) read\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_13_CMD1, len, SPI020_13_CMD3);
        else
            #if defined(SPI_BUS_SPEED) && (SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ)
            kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
            #endif
        #else
            #if defined(SPI_BUS_SPEED) && (SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ)
            kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
            #endif
        #endif
    }

    if (type & FLASH_DMA_READ) {
        return;
    }

    read_buf = (uint32_t  *)buf;
    kdrv_spif_read_data(read_buf, len);/* read data */
    kdrv_spif_wait_command_complete();/* wait for command complete */

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(0);
    #endif
}

void kdev_flash_dma_read_stop(void)
{
    kdrv_spif_wait_command_complete();/* wait for command complete */
    regSPIF_irq->st.dw.kdrv_spif_icr = SPI020_cmd_cmplt_intr_en;/* disable DMA function */
}

void kdev_flash_dma_write_stop(void)
{
    kdrv_spif_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
    regSPIF_irq->st.dw.kdrv_spif_icr = SPI020_cmd_cmplt_intr_en;/* disable DMA function */
    kdrv_spif_check_status_till_ready_2();
}

uint8_t  kdev_flash_r_state_OpCode_35(void)
{
    uint16_t nrx_buff_word_index = 0;
    uint32_t RDSR2=0; //35h
    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3 );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR2, &nrx_buff_word_index, 0x01 );
    //fLib_printf("SPI020_35_CMD1 buf[0]=0x%2X\n", RDSR2 );
    kdrv_spif_wait_command_complete();
    return (uint8_t)RDSR2;
}

void kdev_flash_write(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf, uint32_t  buf_offset)
{
    uint8_t  *write_buf;

    /* This function does not take care about 4 bytes alignment */
    /* if ((uint32_t )(para->buf) % 4) return 1; */
    kdrv_spif_switch_org();

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #endif

    //fLib_printf("write: offset:%x\n", offset);
    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    if(type & FLASH_QUAD_RW) {
        //fLib_printf("Quad (0x32) write\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_34_CMD1, len, SPI020_34_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_32_CMD1, len, SPI020_32_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_32_CMD1, len, SPI020_32_CMD3);
        #endif
    } else {
        //fLib_printf("Normal (0x02) write\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_12_CMD1, len, SPI020_12_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_02_CMD1, len, SPI020_02_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_02_CMD1, len, SPI020_02_CMD3);
        #endif
    }

    if (type & FLASH_DMA_WRITE) {
        regSPIF_irq->st.dw.kdrv_spif_icr = SPI020_cmd_cmplt_intr_en | SPI020_DMA_EN;/* enable DMA function */
        return;
    }

    write_buf = (uint8_t  *)buf+buf_offset;
        //fLib_printf("write_buf:%x, len=%x\n",write_buf, len);
    kdrv_spif_write_data(write_buf, len);
    kdrv_spif_check_status_till_ready();

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(0);
    #endif
    return;
}

kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt)
{
    uint8_t Option;

    Option = FLASH_OP_MODE;
    kdev_flash_read(Option, addr , cnt , data);
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_programming(uint8_t Option, uint32_t addr, const void *data, uint32_t cnt)
{
    uint16_t wloop = 0;
    uint16_t i = 0;
    uint16_t final = 0;

    if (cnt % FLASH_PAGE_SIZE == 0)
        wloop = (cnt / FLASH_PAGE_SIZE);
    else
        wloop = (cnt / FLASH_PAGE_SIZE) + 1;

    for(i=0; i<wloop; i++)
    {
        if(i == (wloop - 1))
        {
            final = cnt-(i*FLASH_PAGE_SIZE); //should <= 256
            kdev_flash_write(Option, (addr+(i*FLASH_PAGE_SIZE)), final, (void *)data, (i*FLASH_PAGE_SIZE));
        }
        else
        {
            kdev_flash_write(Option, (addr+(i*FLASH_PAGE_SIZE)), FLASH_PAGE_SIZE, (void *)data, (i*FLASH_PAGE_SIZE));
        }
    }
    return KDEV_STATUS_OK;
}
kdev_status_t kdev_flash_programdata(uint32_t addr, const void *data, uint32_t cnt)
{
    kdev_status_t status;
    uint8_t Option = FLASH_OP_MODE;
    status = kdev_flash_programming(Option, addr, data, cnt);
    return status;
}
kdev_status_t kdev_flash_programdata_memxfer(uint32_t addr, const void *data, uint32_t cnt)
{
    kdev_status_t status;
    uint8_t Option = FLASH_OP_MODE;
    flash_msg("kdev_flash_programming addr = %d! cnt = %d!", addr, cnt);

    //if((addr % FLASH_MINI_BLOCK_SIZE)||(cnt % FLASH_MINI_BLOCK_SIZE)) //not align to 4K size
    if(addr % FLASH_MINI_BLOCK_SIZE) //not align to 4K size
    {
        flash_msg("LOG_CRITICAL: address and size must align to 4K bytes!!\n");
        status = KDEV_STATUS_ERROR;
    }
    else
    {
        uint32_t sectors = cnt / FLASH_MINI_BLOCK_SIZE;
        if(cnt % FLASH_MINI_BLOCK_SIZE)
            sectors++;
        for(int i=0; i<sectors; i++)
        {
            kdev_flash_erase_sector(addr+(i*FLASH_MINI_BLOCK_SIZE));
        }
        status = kdev_flash_programming(Option, addr, data, cnt);
    }
    return status;
}

kdev_status_t kdev_flash_erase_sector(uint32_t addr)
{
    kdev_flash_4kErase(addr); //for program partial
    //kdev_flash_64kErase(addr); //for program all
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_erase_multi_sector(uint32_t start_addr, uint32_t end_addr)
{
    uint16_t i=0;
    uint32_t nstart_index = 0;
    uint32_t nend_index = 0;

    st_flash_info.sector_size_Bytes = st_flash_info.sector_size_Bytes ? st_flash_info.sector_size_Bytes : SPI020_SECTOR_SIZE;
    nstart_index = start_addr / st_flash_info.sector_size_Bytes;
    nend_index = end_addr/st_flash_info.sector_size_Bytes;
    if( ( end_addr%st_flash_info.sector_size_Bytes) == 0  && nend_index > nstart_index )
    {
        nend_index --;
    }
    flash_msg("_flash_erase_multi_sectors start_addr = %X! end_addr = %X!", start_addr, end_addr);
    flash_msg("_flash_erase_multi_sectors start_index = %d! end_index = %d!", nstart_index, nend_index);
    if(  (nstart_index <= nend_index)  ||  (nend_index < st_flash_info.total_sector_numbers) )
    {
        for(i=nstart_index; i<=nend_index; i++)
        {
            flash_msg("_flash_erase_multi_sectors addr = %d*%d=0x%X!", i, st_flash_info.sector_size_Bytes, i*st_flash_info.sector_size_Bytes);
            kdev_flash_4kErase(i*st_flash_info.sector_size_Bytes);
            flash_msg("_flash_erase_multi_sectors addr = %d*%d=0x%X done!", i, st_flash_info.sector_size_Bytes, i*st_flash_info.sector_size_Bytes);
        }
        return KDEV_STATUS_OK;
    }
    return KDEV_STATUS_ERROR;
}

kdev_status_t kdev_flash_erase_chip(void)
{
    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_C7_CMD0, SPI020_C7_CMD1, SPI020_C7_CMD2, SPI020_C7_CMD3);
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
    return KDEV_STATUS_OK;
}

kdev_flash_status_t kdev_flash_get_status(void)
{
    kdev_flash_status_t status;
    uint32_t flash_status;

    kdrv_spif_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);
    kdrv_spif_wait_command_complete();
    /* read data */
    flash_status = regSPIF_irq->st.bf.kdrv_spif_spisr.SPI_read_status;
    *(uint32_t*)&status = flash_status;
    return status;
}

kdev_status_t kdev_flash_get_info(void)
{
    kdev_status_t status;
    status = kdev_flash_read_SFDP();
    flash_msg("Read Flash SFDP %s", (status==KDEV_STATUS_OK ? "PASS" : "FAIL"));
    return status;
}

