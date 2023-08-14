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
#include "cmsis_os2.h"
#include <stdlib.h>
#include <string.h>

#include "kdrv_cmsis_core.h"
#include "kdev_flash.h"
#include "io.h"
#include "kdrv_clock.h" // for kdrv_delay_us()
#include "kmdw_console.h"
//#define FLASH_WB_DBG
#ifdef FLASH_WB_DBG
#define flash_msg(fmt, ...) kmdw_printf("[WINBOND_FLASH] " fmt, ##__VA_ARGS__)
#else
#define flash_msg(fmt, ...)
#endif
#define min_t(x,y) ( x < y ? x: y )
#define max_blocks 1024
#define backup_blocks 6
#define possible_bad_block 100

spi_flash_t flash_info;
#if ( FLASH_CODING_GET_INFO_EN == YES )
kdrv_spif_parameter_t st_flash_info;
#endif
static uint16_t bbm_lut[max_blocks+backup_blocks]={0};
bool bGigaDeive_Fseries=0;
uint32_t lut_block=0;
uint8_t skip_lut_check=0;

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

uint8_t kdev_flash_read_BBM(uint32_t block_index);
void kdev_flash_128kErase(uint32_t  offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_64K) return 1; */
    uint32_t addr = offset / SPI020_BLOCK_128SIZE;
    uint16_t aa = (uint16_t) addr;
    addr = bbm_lut[aa];

    if(0xFF == kdev_flash_read_BBM(addr)) /* non-FFh check */
    {

        kdev_flash_write_control(1);/* send write enabled */
    
        /* fill in command 0~3 */
        kdrv_spif_set_commands(addr<<6, SPI020_D8_CMD1, SPI020_D8_CMD2, SPI020_D8_CMD3);
        /* wait for command complete */
        kdrv_spif_check_status_till_ready();
    }
}

bool kdev_flash_probe(spi_flash_t *flash)
{
    uint32_t chip_id=0;

    bool probe_90_instruction=false;
    /* fill in command 0~3 */
    //Read Manufacturer and Device Identification by JEDEC ID(0x9F)
    kdrv_spif_set_commands(SPI020_9F_CMD0, SPI020_9F_CMD1, SPI020_9F_CMD2, SPI020_9F_CMD3);
    /* read data */
    kdrv_spif_read_data(/*(uint8_t  *)*/&chip_id, 0x3);
    /* NAND FLASH */
    /* need 8 dummy clocks but IP doesn't support it because there will be no dummy cycle when the address length is 0 */
    //chip_id = chip_id >> 8; /*discard first byte because it's dummy clocks */
    /* wait for command complete */
    kdrv_spif_wait_command_complete();

    //flash->manufacturer = (chip_id>>24);
    flash->manufacturer = (uint8_t )chip_id;
    if (flash->manufacturer != FLASH_WB_DEV && flash->manufacturer != FLASH_GD_DEV && flash->manufacturer != FLASH_MXIC_DEV) {
       /* fill in command 0~3 */
       //Read Manufacturer and Device Identification by 0x90
        kdrv_spif_set_commands(SPI020_9F_CMD0_f, SPI020_9F_CMD1_f, SPI020_9F_CMD2_f, SPI020_9F_CMD3_f);
        /* read data */
        kdrv_spif_read_data(/*(uint8_t  *)*/&chip_id, 0x03/*0x4*/);
        /* wait for command complete */
        kdrv_spif_wait_command_complete();
        //flash->manufacturer = (chip_id>>24);
        flash->manufacturer = (uint8_t )chip_id;
        if(flash->manufacturer == FLASH_GD_DEV)
            probe_90_instruction=true;
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
    uint32_t  sizeId;

    bGigaDeive_Fseries=kdev_flash_probe(&flash_info);

#ifdef FLASH_WB_DBG
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
    if(flash_info.manufacturer == 0xFF)
    {
        return KDEV_STATUS_ERROR;
    }
    if(flash_info.manufacturer == FLASH_GD_DEV)
    {
        flash_info.flash_size = 0x20000;
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

    flash_msg("Manufacturer ID = 0x%02X (%s)\n", flash_info.manufacturer, flash_manu);
    flash_msg("Device ID       = 0x");
    flash_msg("%04X\n", flash_info.flash_id);

    flash_msg("Flash Size      = ");
    if((flash_info.flash_size%1024)==0x00) {
        flash_msg("%dkByte(%dMByte)\n", flash_info.flash_size,flash_info.flash_size>>10);
    } else {
        flash_msg("%dkByte\n", flash_info.flash_size);
    }
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_check_cumulativeECCstauts(void)
{
    kdev_status_t ret=KDEV_STATUS_OK;
#ifndef SPIF_SKIP_ERR_CHECK
    uint8_t status = kdrv_spif_get_flash_status(ADR_STATUS_REG_3);

    if(status & SR3_P_FAIL)
        ret |= KDEV_STATUS_PFAIL;
    if(status & SR3_E_FAIL)
        ret |= KDEV_STATUS_EFAIL;
    if(flash_info.manufacturer == FLASH_GD_DEV)
    {
        if(bGigaDeive_Fseries)
        {
            status = status & 0x70;
            status >>= 4;
            if(status>=7) //Bit errors>8, error exceeded. And cannot be corrected.
                ret |= KDEV_STATUS_ECC_ERROR;
            else if(status>3)
                ret |= KDEV_STATUS_ECC_CORRECT;
        }
        else
        {
            status = (status & 0x30) >> 2;
            status |= (kdrv_spif_get_flash_status(ADR_STATUS_REG_5) & 0x30) >> 4;
            if(status>7) //Bit errors(>7) greater than ECC capability and not corrected
                ret |= KDEV_STATUS_ECC_ERROR;
            else if(status>4)
                ret |= KDEV_STATUS_ECC_CORRECT;
        }
    }
    else //Winbond
    {
        if(status&SR3_ECC_1)
            ret |= KDEV_STATUS_ECC_ERROR;
        else if(status&SR3_ECC_CHECK)
            ret |= KDEV_STATUS_ECC_CORRECT;
    }
#endif
    return ret;
}

/* Bad Block Marker byte0 of page0 in each block */
uint8_t kdev_flash_read_BBM(uint32_t block_index)
{
    uint32_t page_src = block_index << 6;
    uint32_t readdata;
    int32_t read_lens=sizeof(readdata);

    kdrv_spif_set_commands(page_src, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
    //kdrv_delay_us(60);
    kdrv_spif_check_status_till_ready();

    kdrv_spif_set_commands(0x800, SPI020_03_CMD1, read_lens, SPI020_03_CMD3);
    kdrv_spif_wait_rx_full();
    readdata = regSPIF_data->dw.kdrv_spif_dp;
    kdrv_spif_wait_command_complete();/* wait for command complete */

    if((uint8_t)readdata == 0xFF)
    {
        kdev_status_t kdev_status=kdev_flash_check_cumulativeECCstauts();
        if(kdev_status != KDEV_STATUS_OK)
        {
            readdata = 0; //current block is not in good condition
            kdrv_spif_reset_device(); //to clear status
            kmdw_printf("LOG_ERROR: kdev_flash_read_BBM block %d reports status fail 0x%X\n",block_index,kdev_status);
        }
    }
    //flash_msg("Spare byte 0 in block %d= 0x%X, page addr 0x%X\n", block_index, readdata, page_src );
    return (uint8_t)readdata;
}
uint16_t kdev_flash_find_next_good_block(uint16_t iblock)
{
    uint16_t i=0;
    uint8_t tmp=0;
    uint16_t next_good_block=0;

    for(i=0;i<possible_bad_block;i++)
    {
        tmp = kdev_flash_read_BBM(iblock+i);
        if(tmp == 0xFF) /* non-FFh check */
        {
            next_good_block=iblock+i;
            break;
        }
    }
    return next_good_block;
}
uint16_t kdev_flash_find_all_bad_block(uint16_t *buf)
{
    uint16_t i,j=0;
    uint8_t tmp=0;

    for(i=0;i<max_blocks;i++)
    {
        tmp = kdev_flash_read_BBM(i);
        if(tmp != 0xFF) /* non-FFh check */
        {
            *(buf+j)=i;
            j++;
        }
    }
    return j;
}
uint8_t kdev_flash_find_is_bad_block(uint16_t iblock, uint16_t *buf)
{
    uint8_t i=0;
    uint8_t ret=0xFF;
    for(i=0;i<possible_bad_block;i++)
    {
        if(iblock == *(buf+i))
        {
            ret=i;
        }
    }
    return ret;
}

void kdev_flash_scan_all_BBM(void)
{
    uint16_t i=0;
    uint16_t mblock = max_blocks - backup_blocks;
    uint16_t tt1=0;
    uint16_t tt2=0;
    uint16_t bottom_up = max_blocks - backup_blocks - 1;
    uint16_t buff[possible_bad_block]={0};
    uint16_t permanent_bb_count = kdev_flash_find_all_bad_block(&buff[0]);

    uint8_t tmp=0;
    uint16_t mblock_tmp = mblock;
    for(i=mblock_tmp;i<(mblock_tmp+3);i++)
    {
        tmp = kdev_flash_read_BBM(i);
        if(tmp != 0xFF) /* non-FFh check */
        {
            if(0xFF == kdev_flash_read_BBM(bottom_up))
            {
                bbm_lut[bottom_up] = i;
                bbm_lut[i] = bottom_up;
                flash_msg("1a bbm_lut[%d] = %d; bbm_lut[%d] = %d;\n", bottom_up, i, i, bottom_up );
            }
            else
            {
                bottom_up -=1;
                mblock -=1;
                if(0xFF == kdev_flash_read_BBM(bottom_up))
                {
                    bbm_lut[bottom_up] = i;
                    bbm_lut[i] = bottom_up;
                    flash_msg("2a bbm_lut[%d] = %d; bbm_lut[%d] = %d;\n", bottom_up, i, i, bottom_up );
               }
            }
            bottom_up -=1;
            mblock -=1;
        }
        else
            bbm_lut[i] = i;
    }

    if(permanent_bb_count)
    {
        for(i=0;i<permanent_bb_count;i++)
        {
            flash_msg("bad block[%d] = 0x%0X / %d\n", i, buff[i], buff[i]);
            bbm_lut[bottom_up] = buff[i];
            bottom_up--;
        }
    }
    mblock = bottom_up;
    tt1=0;
    tt2=0;
    for(i=0; i<=mblock; i++)
    {
        tt2=kdev_flash_find_next_good_block(tt1);
        if(tt1!=tt2)
            tt1=tt2+1;
        else
            tt1++;
        bbm_lut[i]=tt2;
        flash_msg("bbm_lut[%d] = 0x%0X / %d\n", i, bbm_lut[i], bbm_lut[i]);
    }
    flash_msg("total scan %d \n", mblock);

    //last 3 blocks to save LUT
    bbm_lut[max_blocks] = bottom_up;
    bbm_lut[max_blocks+1] = 1;
    bbm_lut[max_blocks+2] = 3;
    bbm_lut[max_blocks+3] = 2;
    bbm_lut[max_blocks+4] = 4;
    bbm_lut[max_blocks-1] = max_blocks-1;
    bbm_lut[max_blocks-2] = max_blocks-2;
    bbm_lut[max_blocks-3] = max_blocks-3;

    //write to flash
    uint32_t * write_buf = (uint32_t *)&bbm_lut[0];
    for(i=3;i>0;i--)
    {
        if(0xFF == kdev_flash_read_BBM(max_blocks-i))
        {
            lut_block = max_blocks-i;
            bbm_lut[max_blocks-1] = max_blocks-i;
            kdev_flash_128kErase((max_blocks-i)*SPI020_BLOCK_128SIZE);
            skip_lut_check = 1;
            kdev_flash_programdata(((max_blocks-i)*SPI020_BLOCK_128SIZE),write_buf,((max_blocks+backup_blocks)*2));
            skip_lut_check = 0;
        }
    }
}

kdev_status_t kdev_flash_read_LUT(void)
{
    uint16_t nrx_buff_word_index = 0;
    uint32_t LUT_buf[20];
    int32_t i=0;
    kdrv_spif_set_commands(SPI020_A5_CMD0, SPI020_A5_CMD1, 80, SPI020_A5_CMD3);
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( LUT_buf, &nrx_buff_word_index, 80 ); // 20*4
    kdrv_spif_wait_command_complete();
    for(i=0; i<20; i++)
    {
        flash_msg("Read BBM Look Up Table[%d] = 0x%04X\n", i, LUT_buf[i] );
    }
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_read_SFDP(void)
{
    uint32_t SFDP_buf[0x200];
    uint16_t read_lens = 0x800;
    int32_t access_byte;
    uint16_t index=0;
    int32_t rx_fifo_depth = (int32_t)kdrv_spif_rxfifo_depth();

    kdrv_spif_set_commands(0x00000001, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3); //page1
    kdrv_spif_check_status_till_ready();

    kdrv_spif_set_commands(0, SPI020_03_CMD1, read_lens, SPI020_03_CMD3);

    while (read_lens > 0)
    {
        kdrv_spif_wait_rx_full();

        access_byte = min_t(read_lens, rx_fifo_depth);
        read_lens -= access_byte;

        while (access_byte > 0)
        {
            SFDP_buf[index] = regSPIF_data->dw.kdrv_spif_dp;
            index++;
            access_byte -= 4;
        }
    }

    flash_msg("flash_info.flash_id 0x%X flash_info.manufacturer 0x%X\n", flash_info.flash_id, flash_info.manufacturer);
    flash_msg("signature = 0x%X!\n",SFDP_buf[0]);//st_flash_info.signature = *buf;//nrx_buff_word[nrx_buff_word_index-1];//FLASH_SIGNATURE;
    if(SFDP_buf[0] == 0x49464E4F)
    {
        flash_msg("ID = 0x%X!\n",SFDP_buf[16]);//st_flash_info.ID = *(buf+16);//nrx_buff_word[nrx_buff_word_index-1] & 0XFFFFFFFF;
        st_flash_info.erase_4K_support = 0;// no support
        flash_msg("Number of data bytes per page = 0x%X \n",SFDP_buf[20]);//st_flash_info.flash_size_KByte = 0;//(nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
        flash_msg("Number of pages per block = 0x%X \n",SFDP_buf[23]);//st_flash_info.flash_size_KByte = 0;//(nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
        flash_msg("Number of blocks per logical unit = 0x%X \n",SFDP_buf[24]);//st_flash_info.flash_size_KByte = 0;//(nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
        flash_msg("size = %d kbytes !\n",(SFDP_buf[20]*SFDP_buf[23]*SFDP_buf[24])>>10);//st_flash_info.flash_size_KByte = 0;//(nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
        st_flash_info.flash_size_KByte = (SFDP_buf[20]*SFDP_buf[23]*SFDP_buf[24]);
        st_flash_info.block_size_Bytes = SFDP_buf[24];
        st_flash_info.page_size_Bytes = SFDP_buf[23];
        st_flash_info.sector_size_Bytes = SFDP_buf[20];

#ifdef FLASH_WB_DBG
        char device_model[20]={0};
        int32_t i=0;
        for(i=0; i<3; i++)
        {
            device_model[i*4] = (uint8_t)SFDP_buf[8+i];
            device_model[(i*4)+1] = (uint8_t)(SFDP_buf[8+i]>>8);
            device_model[(i*4)+2] = (uint8_t)(SFDP_buf[8+i]>>16);
            device_model[(i*4)+3] = (uint8_t)(SFDP_buf[8+i]>>24);
        }
        flash_msg("Device Manufacturer = %s\n", device_model);
        for(i=0; i<5; i++)
        {
            device_model[i*4] = (uint8_t)SFDP_buf[11+i];
            device_model[(i*4)+1] = (uint8_t)(SFDP_buf[11+i]>>8);
            device_model[(i*4)+2] = (uint8_t)(SFDP_buf[11+i]>>16);
            device_model[(i*4)+3] = (uint8_t)(SFDP_buf[11+i]>>24);
        }
        flash_msg("Device Model = %s\n", device_model);
#endif
        uint16_t bad_blocks_num = ((uint8_t)SFDP_buf[26])<<8;
        bad_blocks_num += (uint8_t)(SFDP_buf[25]>>24);
        flash_msg("Bad Blocks maximum per uint = %d!\n",bad_blocks_num);

        uint16_t tmp = (uint16_t)(SFDP_buf[33]>>8);
        flash_msg("Maximum page program time = %d us!\n",tmp);
        tmp = ((uint8_t)SFDP_buf[34])<<8;
        tmp += (uint8_t)(SFDP_buf[33]>>24);
        flash_msg("Maximum block erase time = %d us!\n",tmp);
        tmp = (uint16_t)(SFDP_buf[34]>>8);
        flash_msg("Maximum page read time = %d us!\n",tmp);

        //st_flash_info.sector_size_Bytes = 0;//1<<(nrx_buff_word[ nrx_buff_word_index-1 ]&0xFF);
        //st_flash_info.total_sector_numbers = 0;//(ntemp / st_flash_info.sector_size_Bytes)+1;
        //st_flash_info.block_size_Bytes = 0;//( 1<<( nrx_buff_word[ nrx_buff_word_index-1 ] & 0xFF ) )/st_flash_info.sector_size_Bytes ;
        //st_flash_info.page_size_Bytes = 0;//256;
    }
    else
    {
        flash_msg("No Parameter page!!!!\n");
        st_flash_info.erase_4K_support = 0;// no support
        st_flash_info.flash_size_KByte = 0x8000000;
        st_flash_info.block_size_Bytes = 0x400;
        st_flash_info.page_size_Bytes = 0x40;
        st_flash_info.sector_size_Bytes = 0x800;
    }

    flash_msg("Number of pages per block = 0x%X \n",st_flash_info.page_size_Bytes);//st_flash_info.flash_size_KByte = 0;//(nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
    flash_msg("Number of blocks per logical unit = 0x%X \n",st_flash_info.block_size_Bytes);//st_flash_info.flash_size_KByte = 0;//(nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
    flash_msg("size = %d kbytes !\n",st_flash_info.flash_size_KByte>>10);//st_flash_info.flash_size_KByte = 0;//(nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
    return KDEV_STATUS_OK;
}

/* WB Flash */
void kdev_flash_read_status(void)
{
    uint16_t nrx_buff_word_index = 0;
    uint32_t RDSR1=0; //C0h
    uint32_t RDSR2=0; //A0h
    uint32_t RDCR=0; //B0h

    kdev_flash_write_control(1);
    flash_msg("kdev_flash_write_control done\n");
    kdrv_spif_set_commands( ADR_STATUS_REG_3 , SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR1, &nrx_buff_word_index, 0x01 );
    flash_msg("Status C0h = 0x%2X\n", (uint8_t)RDSR1 );
    kdrv_spif_wait_command_complete();

    kdrv_spif_set_commands(ADR_STATUS_REG_1, SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR2, &nrx_buff_word_index, 0x01 );
    flash_msg("Protection A0h = 0x%2X\n", (uint8_t)RDSR2 );
    kdrv_spif_wait_command_complete();

    kdrv_spif_set_commands(ADR_STATUS_REG_1, SPI020_01_CMD1_w, SPI020_01_CMD2_w, SPI020_01_CMD3_w );
    RDSR2 = 0;// &= ~0x7C;
    kdrv_spif_write_data((uint8_t *)&RDSR2, 1);
    kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
    flash_msg("Protection A0h  = 0x%2X\n", (uint8_t)RDSR2);

    kdrv_spif_set_commands(ADR_STATUS_REG_2, SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDCR, &nrx_buff_word_index, 0x01 );
    flash_msg("Configuration B0h = 0x%2X\n", (uint8_t)RDCR );
    kdrv_spif_wait_command_complete();

    kdrv_spif_set_commands(ADR_STATUS_REG_2, SPI020_01_CMD1_w, SPI020_01_CMD2_w, SPI020_01_CMD3_w );
    RDCR = (SR2_ECC_E|SR2_BUF);/* ECC_EN | Buffer Read Mode */
    RDCR |= BIT6; /* OTP_EN */
    if(flash_info.manufacturer == FLASH_GD_DEV)
        RDCR |= BIT0; /* QE */
    kdrv_spif_write_data((uint8_t *)&RDCR, 1);
    kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
    flash_msg("write OTP-E 0x%X in status register1 done!\n",(uint8_t)RDCR);

    kdev_flash_read_SFDP();

    kdrv_spif_set_commands(ADR_STATUS_REG_2, SPI020_01_CMD1_w, SPI020_01_CMD2_w, SPI020_01_CMD3_w );
    RDCR &= ~BIT6;
    if(flash_info.manufacturer == FLASH_GD_DEV)
        RDCR |= BIT0; /* QE */
    kdrv_spif_write_data((uint8_t *)&RDCR, 1);
    kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
    flash_msg("write OTP-E 0x%X in status register1 done!\n",(uint8_t)RDCR);
    kdrv_delay_us(10);

    regSPIF_ctrl->st.bf.kdrv_spif_cr.abort = 1;
    /* Wait reset completion */
    do {
        if(regSPIF_ctrl->st.bf.kdrv_spif_cr.abort == 0)
            break;
    } while(1);

    kdrv_spif_set_commands(ADR_STATUS_REG_2, SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDCR, &nrx_buff_word_index, 0x01 );
    flash_msg("Configuration B0h = 0x%2X\n", (uint8_t)RDCR );
    kdrv_spif_wait_command_complete();
}

uint32_t kdev_flash_LUT_SWAP(uint32_t address)
{
    uint32_t page_src = address / SPI020_BLOCK_128SIZE;
    uint32_t addr = address % SPI020_BLOCK_128SIZE;
    uint32_t readdata = 0;//page_src;//bbm_lut[tmp];
    int32_t read_lens=sizeof(readdata);
    kdrv_spif_set_commands(lut_block << 6, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
    //kdrv_delay_us(60);
    kdrv_spif_check_status_till_ready();

    kdrv_spif_set_commands(page_src<<1, SPI020_03_CMD1, read_lens, SPI020_03_CMD3);
    kdrv_spif_wait_rx_full();
    readdata = regSPIF_data->dw.kdrv_spif_dp;
    readdata = readdata & 0x0000FFFF;
    kdrv_spif_wait_command_complete();

    return (addr+(readdata*SPI020_BLOCK_128SIZE));
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

    if(KDEV_STATUS_ERROR == kdev_flash_read_flash_id())
        return KDEV_STATUS_ERROR;
    kdrv_spif_reset_device(); //to clear status
    kdev_flash_read_status();

    kdrv_spif_reset_device(); //to clear status
    // uint32_t lut_block=0;
    if(0xFF != kdev_flash_read_BBM(max_blocks-1))
    {
        if(0xFF != kdev_flash_read_BBM(max_blocks-2))
        {
            lut_block = max_blocks-3; //1021
        }
        else
        {
            lut_block = max_blocks-2; //1022
        }
    }
    else
    {
        lut_block = max_blocks-1; //1023
    }
    skip_lut_check = 1;
    kdev_flash_readdata((lut_block*SPI020_BLOCK_128SIZE), bbm_lut, ((max_blocks+backup_blocks)*2));
    skip_lut_check = 0;
    //if((bbm_lut[0]!=0)||((bbm_lut[max_blocks+1]!=1)||(bbm_lut[max_blocks+4]!=4)))
    if((bbm_lut[max_blocks+1]!=1)||(bbm_lut[max_blocks+4]!=4))
    {
        //LUT is not initialized yet, give it a default setting
        kdev_flash_scan_all_BBM();
        //for(int32_t i=0;i<max_blocks;i++)
        //    bbm_lut[i]=i;
    }

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

int32_t kdev_flash_read_compare(void *buf, uint32_t src, size_t bytes, uint8_t type)
{
    int32_t access_byte;
    uint32_t read_data;
    //int32_t rx_fifo_depth;
    uint32_t page_addr_start;
    //uint16_t column_addr_start;
    uint32_t total_pages;
    uint32_t page_shift_dummy;
    uint32_t read_lens;
    uint32_t left_lens;
    int32_t ret=0;
    int32_t i=0;
    uint32_t *read_buf = (uint32_t  *)buf;
    uint32_t tmp = 0;

    //if ((bytes & 0x3) > 0) return -1;

    uint32_t address = 0;
#if 1
    //BBM init check
    tmp = src / SPI020_BLOCK_128SIZE;
    address = src % SPI020_BLOCK_128SIZE;
    tmp = bbm_lut[tmp];
    address = address + (tmp*SPI020_BLOCK_128SIZE);
#endif
    page_addr_start = address / spi_nand_data_buf_size; //memory area page 0~65535
    //column_addr_start = 0;

    total_pages = (bytes+(spi_nand_data_buf_size-1)) / spi_nand_data_buf_size;
    left_lens = bytes % spi_nand_data_buf_size;
    //rx_fifo_depth = (int32_t)kdrv_spif_rxfifo_depth();

    for(i=0; i<total_pages; i++)
    {
        page_shift_dummy = page_addr_start+i;
        //flash_msg("page_shift_dummy 0x%X = (%x + %d) <<8 OK!\n",page_shift_dummy, page_addr_start, i);
        //kdrv_spif_set_commands(page_shift_dummy, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
        read_lens = ((i==(total_pages-1))&&(left_lens!=0)) ? left_lens : spi_nand_data_buf_size;
#if 0
        uint16_t tmp = kdev_flash_read(type, page_shift_dummy, read_lens, read_buf);
        ret += tmp;
        //if(0 == memcmp((const void*)(read_buf), (const void*)(buf), spi_nand_data_buf_size/4))
        //    ret+=read_lens;
        read_buf+=(spi_nand_data_buf_size/4);
#else
        kdrv_spif_set_commands(page_shift_dummy, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
        //kdrv_delay_us(50);
        kdrv_spif_check_status_till_ready();


        if(type & FLASH_QUAD_RW)
        {
            if(flash_info.manufacturer == FLASH_GD_DEV)
            {
                if(type & FLASH_IO_RW) /* Quad IO */
                {
                    if((flash_info.flash_id & 0xFF) < 0x80) // gd 5series
                        kdrv_spif_set_commands(0, SPI020_EB_CMD1_5E, read_lens, SPI020_EB_CMD3);
                    else
                        kdrv_spif_set_commands(0, SPI020_EB_CMD1, read_lens, SPI020_EB_CMD3);
                }
                else /* Quad Output */
                    kdrv_spif_set_commands(0, SPI020_6B_CMD1, read_lens, SPI020_6B_CMD3);
            }
            else
            {
                if(type & FLASH_IO_RW) /* Quad IO */
                    kdrv_spif_set_commands(0, SPI020_EC_CMD1, read_lens, SPI020_EC_CMD3);
                else /* Quad Output */
                    kdrv_spif_set_commands(0, SPI020_6C_CMD1, read_lens, SPI020_6C_CMD3);
            }
        }
        else
        {
            kdrv_spif_set_commands(0, SPI020_03_CMD1, read_lens, SPI020_03_CMD3);
        }

        while (read_lens > 0)
        {
            kdrv_spif_wait_rx_full();

            access_byte = min_t(read_lens, kdrv_spif_rxfifo_depth());
            read_lens -= access_byte;

            while (access_byte > 0)
            {
                read_data = regSPIF_data->dw.kdrv_spif_dp;//u32Lib_LeRead32((unsigned char* )SPI020REG_DATAPORT);
                if(*read_buf == read_data)//outw(write_addr, read_data);
                    ret+=4;
                read_buf++;
                access_byte -= 4;
            }
        }
        kdrv_spif_check_status_till_ready();/* wait for command complete */
#endif
    }

#ifndef MIXING_MODE_OPEN_RENDERER
    //kdrv_spif_wait_command_complete();/* wait for command complete */
#endif

#if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(0);
#endif

    return ret;
}

void kdev_flash_pageread(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf)
{
    uint32_t  *read_buf;
    int32_t access_byte;

    kdrv_spif_set_commands(offset, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
    //kdrv_delay_us(50);
    kdrv_spif_check_status_till_ready();


    if(type & FLASH_QUAD_RW)
    {
        if(flash_info.manufacturer == FLASH_GD_DEV)
        {
            if(type & FLASH_IO_RW) /* Quad IO */
            {
                if((flash_info.flash_id & 0xFF) < 0x80) // gd 5series
                    kdrv_spif_set_commands(0, SPI020_EB_CMD1_5E, len, SPI020_EB_CMD3);
                else
                    kdrv_spif_set_commands(0, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            }
            else /* Quad Output */
                kdrv_spif_set_commands(0, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
        }
        else
        {
            if(type & FLASH_IO_RW) /* Quad IO */
                kdrv_spif_set_commands(0, SPI020_EC_CMD1, len, SPI020_EC_CMD3);
            else /* Quad Output */
                kdrv_spif_set_commands(0, SPI020_6C_CMD1, len, SPI020_6C_CMD3);
        }
    }
    else
    {
        kdrv_spif_set_commands(0, SPI020_03_CMD1, len, SPI020_03_CMD3);
    }

    //flash_msg("read data bytes %d bytes!\n",len);
    read_buf = (uint32_t  *)buf;
    while (len > 0)
    {
        kdrv_spif_wait_rx_full();

        access_byte = min_t(len, kdrv_spif_rxfifo_depth());
        len -= access_byte;

        while (access_byte > 0)
        {
            *read_buf = regSPIF_data->dw.kdrv_spif_dp;//u32Lib_LeRead32((unsigned char* )SPI020REG_DATAPORT);
            read_buf++;
            access_byte -= 4;
        }
    }
    kdrv_spif_check_status_till_ready();/* wait for command complete */
}

void kdev_flash_read(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf)
{
#if 1
    uint32_t  *read_buf;
    int32_t access_byte;

    kdrv_spif_set_commands(offset, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
    //kdrv_delay_us(50);
    kdrv_spif_check_status_till_ready();


    if(type & FLASH_QUAD_RW)
    {
        if(flash_info.manufacturer == FLASH_GD_DEV)
        {
            if(type & FLASH_IO_RW) /* Quad IO */
            {
                if((flash_info.flash_id & 0xFF) < 0x80) // gd 5series
                    kdrv_spif_set_commands(0, SPI020_EB_CMD1_5E, len, SPI020_EB_CMD3);
                else
                    kdrv_spif_set_commands(0, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            }
            else /* Quad Output */
                kdrv_spif_set_commands(0, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
        }
        else
        {
            if(type & FLASH_IO_RW) /* Quad IO */
                kdrv_spif_set_commands(0, SPI020_EC_CMD1, len, SPI020_EC_CMD3);
            else /* Quad Output */
                kdrv_spif_set_commands(0, SPI020_6C_CMD1, len, SPI020_6C_CMD3);
        }
    }
    else
    {
        kdrv_spif_set_commands(0, SPI020_03_CMD1, len, SPI020_03_CMD3);
    }

    read_buf = (uint32_t  *)buf;
    while (len > 0)
    {
        kdrv_spif_wait_rx_full();

        access_byte = min_t(len, kdrv_spif_rxfifo_depth());
        len -= access_byte;

        while (access_byte > 0)
        {
            *read_buf = regSPIF_data->dw.kdrv_spif_dp;//u32Lib_LeRead32((unsigned char* )SPI020REG_DATAPORT);
            read_buf++;
            access_byte -= 4;
        }
    }
    /*read_buf = (uint32_t  *)buf;
    kdrv_spif_read_data(read_buf, len);*//* read data */
    kdrv_spif_check_status_till_ready();/* wait for command complete */
#else
    uint32_t  *read_buf;

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #endif

    //uint32_t page_shift_dummy;
    uint32_t page_addr_start = offset / spi_nand_data_buf_size;
    uint32_t total_pages = (len+(spi_nand_data_buf_size-1)) / spi_nand_data_buf_size;
    int32_t i=0;
    uint32_t read_length = len;
    uint16_t read_lens = 0;
    int32_t access_byte;
    uint32_t read_data;
    uint32_t rx_fifo_depth = (uint32_t)kdrv_spif_rxfifo_depth();
    read_buf = (uint32_t  *)buf;
    for(i=0; i<total_pages; i++)
    {
        //page_shift_dummy = (uint16_t)(page_addr_start+i);
        //kdrv_spif_set_commands(page_shift_dummy, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
        kdrv_spif_set_commands(page_addr_start+i, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
        //kdrv_delay_us(50);
        kdrv_spif_check_status_till_ready();

        if(read_length > spi_nand_data_buf_size)
        {
            read_lens = spi_nand_data_buf_size;
            read_length -= read_lens;
        }
        else
        {
            read_lens = read_length;
            read_length -= read_lens;
        }
        //read_lens = spi_nand_data_buf_size;

        if(type & FLASH_QUAD_RW)
        {
            if(type & FLASH_IO_RW) /* Quad IO */
                kdrv_spif_set_commands(0, SPI020_EC_CMD1, read_lens, SPI020_EC_CMD3);
            else /* Quad Output */
                kdrv_spif_set_commands(0, SPI020_6C_CMD1, read_lens, SPI020_6C_CMD3);
        }
        else
        {
            kdrv_spif_set_commands(0, SPI020_03_CMD1, read_lens, SPI020_03_CMD3);
        }

        //kdrv_spif_read_data(read_buf, read_lens);/* read data */
        //kdrv_spif_check_status_till_ready();/* wait for command complete */

        while (read_lens > 0)
        {
            kdrv_spif_wait_rx_full();

            access_byte = min_t(read_lens, rx_fifo_depth);
            read_lens -= access_byte;

            while (access_byte > 0)
            {
                read_data = regSPIF_data->dw.kdrv_spif_dp;//u32Lib_LeRead32((unsigned char* )SPI020REG_DATAPORT);
                *read_buf = read_data;//outw(write_addr, read_data);
                read_buf++;
                access_byte -= 4;
            }
        }
        kdrv_spif_check_status_till_ready();/* wait for command complete */
    }
#endif

#if 0
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
                kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #endif
        } else {
            //fLib_printf("Daul (0x3B) read\n");
            #if FLASH_4BYTES_CMD_EN
                kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #endif
        }
    } else if(type & FLASH_QUAD_RW) {
        if(type & FLASH_IO_RW) {
            kdrv_spif_set_commands(offset, SPI020_EC_CMD1, len, SPI020_EC_CMD3);
        } else {
            kdrv_spif_set_commands(offset, SPI020_6C_CMD1, len, SPI020_6C_CMD3);
        }
    } else if(type & FLASH_FAST_READ) {
        //fLib_printf("Fast (0x0B) read\n");
        #if FLASH_4BYTES_CMD_EN
            kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #endif
    } else {/* normal read */
        //fLib_printf("Normal (0x03) read\n");
        kdrv_spif_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
    }

    if (type & FLASH_DMA_READ) {
        return;
    }

    read_buf = (uint32_t  *)buf;
    kdrv_spif_read_data(read_buf, len);/* read data */
    kdrv_spif_wait_command_complete();/* wait for command complete */
#endif
}

void kdev_flash_pagecopy(uint32_t  src_page, uint32_t  dst_page, uint32_t cnt)
{
    int32_t i=0;

    //BBM LUT check start
    uint32_t src = src_page;
    uint32_t dst = dst_page;
    uint32_t tmp;
    tmp = src_page / st_flash_info.page_size_Bytes;
    src = src_page % st_flash_info.page_size_Bytes;
    tmp = bbm_lut[tmp];
    src = src + (tmp*st_flash_info.page_size_Bytes);

    tmp = dst_page / st_flash_info.page_size_Bytes;
    dst = dst_page % st_flash_info.page_size_Bytes;
    tmp = bbm_lut[tmp];
    dst = dst + (tmp*st_flash_info.page_size_Bytes);
    //BBM LUT check end

    //erase dst page block? NO, should erase block before page copy not during page copy
    //kdev_flash_128kErase(dst_page*spi_nand_data_buf_size);

    for(i=0; i<cnt; i++)
    {
        kdrv_spif_set_commands(src+i, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
        //kdrv_delay_us(50);
        kdrv_spif_check_status_till_ready();

        kdrv_spif_set_commands(SPI020_06_CMD0, SPI020_06_CMD1, SPI020_06_CMD2, SPI020_06_CMD3);
        kdrv_spif_wait_command_complete();
        kdrv_spif_set_commands(dst+i, SPI020_10_CMD1, SPI020_10_CMD2, SPI020_10_CMD3);
        //kdrv_delay_us(50);
        kdrv_spif_check_status_till_ready();
    }
}

/* Small size programming */
static uint8_t *spif_enum_buf = NULL;       // for GET_DESCRIPTOR use
static uint8_t *spif_enum_buff = NULL;       // for GET_DESCRIPTOR use
static uint32_t backup_block_index = 0;
kdev_status_t kdev_flash_block_backup(uint8_t Option, uint32_t addr, const void *data, uint32_t cnt)
{
    uint16_t backup_block = 0;
    uint32_t page_keep_start, page_keep_end, page_keep_mid1, page_keep_mid2=0;
    uint32_t program_page_cnt=0;
    uint16_t tmp;
    uint32_t acc_index, acc_cnt;
    uint32_t i,j=0;
    uint8_t *databuf;
    //uint8_t spif_enum_buff[spi_nand_data_buf_size];
    spif_enum_buff = (uint8_t *)malloc(spi_nand_data_buf_size);

    if(cnt>=SPI020_BLOCK_128SIZE)
        return KDEV_STATUS_ERROR;
    
    backup_block = max_blocks - backup_blocks;
    tmp = bbm_lut[backup_block+backup_block_index];
    if( 0xFF != kdev_flash_read_BBM(tmp))
    {
        flash_msg("need to scan all blocks and rearrange LUT again!");
        backup_block_index = (backup_block_index >= 2)? 0 : (backup_block_index+1);
        tmp = bbm_lut[backup_block+backup_block_index];
        if( 0xFF != kdev_flash_read_BBM(tmp))
            return KDEV_STATUS_ERROR;
    }
    backup_block = tmp;
    backup_block_index = (backup_block_index >= 2)? 0 : (backup_block_index+1);

    //erase dst page block? NO, should erase block before page copy not during page copy
    kdev_flash_128kErase(backup_block*SPI020_BLOCK_128SIZE);

    //backup data before programming zone
    page_keep_start = addr / SPI020_BLOCK_128SIZE; //which block
    page_keep_start <<= 6; //which page
    page_keep_mid1 = addr % SPI020_BLOCK_128SIZE;
    page_keep_mid1 = page_keep_mid1 / FLASH_PAGE_SIZE; //page count

    kdev_flash_pagecopy(page_keep_start, (backup_block<<6), page_keep_mid1);
    program_page_cnt = page_keep_mid1;
    page_keep_mid1 = page_keep_start+page_keep_mid1; //which page

    //backup data after programming zone
    page_keep_mid2 = (addr+cnt) % SPI020_BLOCK_128SIZE; //left amount A
    page_keep_end = page_keep_mid2? (SPI020_BLOCK_128SIZE - page_keep_mid2) : 0; //left amount after programming zone
    page_keep_mid2 = (page_keep_mid2 % FLASH_PAGE_SIZE)? ((page_keep_mid2 / FLASH_PAGE_SIZE)+1):(page_keep_mid2 / FLASH_PAGE_SIZE); //page bondary of programming zone
    page_keep_end = page_keep_end / FLASH_PAGE_SIZE;

    kdev_flash_pagecopy((page_keep_start+page_keep_mid2), ((backup_block<<6)+page_keep_mid2), page_keep_end);

    //handle real programming zone
    page_keep_mid2 = (page_keep_mid2) ? page_keep_mid2 : spi_nand_pages_per_block; //(addr+cnt) == SPI020_BLOCK_128SIZE
    program_page_cnt = (page_keep_mid2>program_page_cnt)?(page_keep_mid2 - program_page_cnt):1;
    databuf = (uint8_t *) data;
    acc_cnt = cnt;
    for(i=0; i<program_page_cnt; i++)
    {
        kdev_flash_readdata( ((page_keep_mid1+i)*FLASH_PAGE_SIZE), spif_enum_buff, spi_nand_data_buf_size);//read one page
        if(i==0)
        {
            acc_index = addr % FLASH_PAGE_SIZE;
            uint32_t aaa = ((acc_index+acc_cnt)<spi_nand_data_buf_size)?acc_cnt:spi_nand_data_buf_size;
            for(j=acc_index; j<aaa; j++)
            {
                uint8_t ttt =*databuf;
                *(spif_enum_buff+j) = ttt;
                databuf++;
                acc_cnt--;
            }
        }
        else
        {
            acc_index = min_t(acc_cnt,FLASH_PAGE_SIZE);
            for(j=0; j<acc_index; j++)
            {
                *(spif_enum_buff+j) = *databuf;
                databuf++;
                acc_cnt--;
            }
        }
        uint32_t address = (backup_block<<6)+(page_keep_mid1-page_keep_start)+i;
        address = address * FLASH_PAGE_SIZE;
        kdev_flash_programdata(address,spif_enum_buff,spi_nand_data_buf_size);
    }
    free(spif_enum_buff);
    //end
    kdev_flash_128kErase(page_keep_start*FLASH_PAGE_SIZE);
    kdev_flash_pagecopy((backup_block<<6), page_keep_start, 64);
    return KDEV_STATUS_OK;
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

    kdrv_spif_switch_org();

#if 0
    //fLib_printf("write: offset:%x\n", offset);
    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    if(type & FLASH_QUAD_RW) {
        kdrv_spif_set_commands(offset, SPI020_32_CMD1, len, SPI020_32_CMD3);
    } else {
        kdrv_spif_set_commands(offset, SPI020_02_CMD1, len, SPI020_02_CMD3);
    }

    if (type & FLASH_DMA_WRITE) {
        regSPIF_irq->st.dw.kdrv_spif_icr = SPI020_cmd_cmplt_intr_en | SPI020_DMA_EN;/* enable DMA function */
        return;
    }
#endif
    write_buf = (uint8_t  *)buf+buf_offset;
    //flash_msg("kdrv_spif_write_data_nand offset:%x, len=%x\n",offset, len);
    kdrv_spif_write_data_nand(type, offset, write_buf, len);
    //kdrv_spif_check_status_till_ready();
}

kdev_status_t kdev_flash_running_error_handler(uint32_t addr)
{
    kdev_status_t kdev_status = KDEV_STATUS_OK;
    uint32_t iblock = addr / SPI020_BLOCK_128SIZE;
    int32_t i;

    if(!skip_lut_check) //normal read operation
    {
        kdev_status = kdev_flash_check_cumulativeECCstauts();
        if(kdev_status != KDEV_STATUS_OK)
        {
            //mark this block as bad block
            kdrv_spif_mark_badblock_flag(iblock);
            //running error handling
            flash_msg("kdev_flash_check_cumulativeECCstauts addr 0x%X status 0x%X\n",addr,kdev_status);
            if(kdev_status & KDEV_STATUS_ECC_ERROR)
            {
                kmdw_printf("LOG_CRITICAL: ECC ERROR and CAN'T be CORRECTTED!\nPLEASE ERASE and REPROGRAM FLASH to recover this critical issue!! address 0x%X 0x%X\n",addr, kdev_status);
            }
            else if(kdev_status & KDEV_STATUS_ECC_CORRECT)
            {
                //block backup!!!!!!
                kmdw_printf("LOG_CRITICAL: addr 0x%X need backup/swap block, and update LUT!\n",addr);
                iblock = addr / SPI020_BLOCK_128SIZE;
                uint16_t freeblock = bbm_lut[max_blocks];
                uint32_t *write_buf = (uint32_t *)&bbm_lut[0];
                for(i=0;i<backup_blocks;i++)
                {
                    if(0xFF == kdev_flash_read_BBM(freeblock-i))
                    {
                        kdev_flash_128kErase((freeblock-i)*SPI020_BLOCK_128SIZE);
                        kdev_flash_pagecopy(iblock<<6,(freeblock-i)<<6,64);
                        bbm_lut[iblock] = freeblock-i;
                        bbm_lut[(freeblock-i)]=iblock;
                        bbm_lut[max_blocks]=freeblock-i-1;

                        kdev_flash_128kErase(lut_block*SPI020_BLOCK_128SIZE);
                        skip_lut_check = 1;
                        kdev_flash_programdata((lut_block*SPI020_BLOCK_128SIZE),write_buf,((max_blocks+backup_blocks)*2));
                        skip_lut_check = 0;
                        break;
                    }
                    else if(i == (backup_blocks-1))
                    {
                        kmdw_printf("LOG_CRITICAL: Can't find a good block, Failed 5 times in a row!!");
                    }
                }
            }
            kdrv_spif_reset_device(); //to clear status
        }
    }
    return kdev_status;
}

#if 1
kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt)
{
    kdev_status_t kdev_status = KDEV_STATUS_OK;
    uint32_t page_addr_start;
    uint32_t page_addr_end;
    uint32_t total_pages;
    uint32_t page_shift_dummy;
    uint32_t total_lens = cnt;
    uint32_t read_lens = 0;
    //uint32_t left_lens;
    int32_t i=0;
    uint8_t *read_buf = (uint8_t  *)data;
    uint32_t data_start_addr;
    uint32_t data_start_size;
    uint32_t tmp = 0;

    if(total_lens > SPI020_BLOCK_128SIZE)
    {
        kmdw_printf("read size must within 1 block");
        return KDEV_STATUS_ERROR;
    }

    spif_enum_buf = (uint8_t *)malloc(spi_nand_data_buf_size);

    //BBM init check
    uint32_t address = addr;
    if(!skip_lut_check) //normal read operation
    {
        tmp = addr / SPI020_BLOCK_128SIZE;
        address = addr % SPI020_BLOCK_128SIZE;
        tmp = bbm_lut[tmp];
        address = address + (tmp*SPI020_BLOCK_128SIZE);
    }
    page_addr_start = address / spi_nand_data_buf_size; //memory area page 0~65535
    page_addr_end = (address+cnt) / spi_nand_data_buf_size;
    total_pages = (page_addr_end - page_addr_start) + 1;//(cnt+(spi_nand_data_buf_size-1)) / spi_nand_data_buf_size;
    //left_lens = cnt % spi_nand_data_buf_size;

    data_start_addr = address % spi_nand_data_buf_size;
    data_start_size = spi_nand_data_buf_size - data_start_addr;
    if(cnt<data_start_size)
        data_start_size= cnt;

    for(i=0; i<total_pages; i++)
    {
        page_shift_dummy = page_addr_start+i;
        kdev_flash_pageread(FLASH_OP_MODE, page_shift_dummy, spi_nand_data_buf_size, spif_enum_buf);//read one page
        if(i==0)
        {
            memcpy(read_buf,(spif_enum_buf+data_start_addr),data_start_size);
            read_buf+=data_start_size;
            total_lens-=data_start_size;
        }
        else
        {
            //flash_msg("page_shift_dummy 0x%X = (%x + %d) <<8 OK!\n",page_shift_dummy, page_addr_start, i);
            read_lens = (total_lens<spi_nand_data_buf_size) ? total_lens : spi_nand_data_buf_size;
            memcpy(read_buf,spif_enum_buf,read_lens);
            read_buf+=read_lens;
            total_lens-=read_lens;
        }
    }
    kdev_status = kdev_flash_running_error_handler(addr);
    free(spif_enum_buf);
    return kdev_status;
}
#else
kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt)
{
    uint32_t page_addr_start;
    uint32_t total_pages;
    uint32_t page_shift_dummy;
    uint32_t read_lens;
    uint32_t left_lens;
    int32_t i=0;
    uint32_t *read_buf = (uint32_t  *)data;

    //BBM init check
    block_start = addr / SPI020_BLOCK_128SIZE;
    address = addr % SPI020_BLOCK_128SIZE;
    block_checked = bbm_lut[block_start];
    address = address + (block_checked*SPI020_BLOCK_128SIZE);

    page_addr_start = address / spi_nand_data_buf_size; //memory area page 0~65535
    total_pages = (cnt+(spi_nand_data_buf_size-1)) / spi_nand_data_buf_size;
    left_lens = cnt % spi_nand_data_buf_size;

    for(i=0; i<total_pages; i++)
    {
        page_shift_dummy = page_addr_start+i;
        //err_msg("page_shift_dummy 0x%X = (%x + %d) <<8 OK!\n",page_shift_dummy, page_addr_start, i);
        read_lens = ((i==(total_pages-1))&&(left_lens!=0)) ? left_lens : spi_nand_data_buf_size;

        kdev_flash_read(FLASH_OP_MODE, page_shift_dummy, read_lens, read_buf);
        read_buf+=(spi_nand_data_buf_size/4);
    }
    return KDEV_STATUS_OK;
}
#endif

kdev_status_t kdev_flash_programming(uint8_t Option, uint32_t addr, const void *data, uint32_t cnt)
{
    uint16_t wloop = 0;
    uint16_t i = 0;
    uint16_t final = 0;
    uint32_t address = addr;
    uint32_t tmp = 0;

    if (cnt % FLASH_PAGE_SIZE == 0)
        wloop = (cnt / FLASH_PAGE_SIZE);
    else
        wloop = (cnt / FLASH_PAGE_SIZE) + 1;

    if(!skip_lut_check)
    {
        tmp = addr / SPI020_BLOCK_128SIZE;
        address = addr % SPI020_BLOCK_128SIZE;
        tmp = bbm_lut[tmp];
        address = address + (tmp*SPI020_BLOCK_128SIZE);
    }

    for(i=0; i<wloop; i++)
    {
        if(i == (wloop - 1))
        {
            final = cnt-(i*FLASH_PAGE_SIZE); //should <= 256
            kdev_flash_write(Option, (address+(i*FLASH_PAGE_SIZE)), final, (void *)data, (i*FLASH_PAGE_SIZE));
        }
        else
        {
            kdev_flash_write(Option, (address+(i*FLASH_PAGE_SIZE)), FLASH_PAGE_SIZE, (void *)data, (i*FLASH_PAGE_SIZE));
        }
    }
    return KDEV_STATUS_OK;
}
kdev_status_t kdev_flash_programdata(uint32_t addr, const void *data, uint32_t cnt)
{
    kdev_status_t status;
    uint8_t Option = FLASH_OP_MODE;
    status = kdev_flash_programming(Option, addr, data, cnt);
    status = kdev_flash_check_cumulativeECCstauts();
    if(status != KDEV_STATUS_OK)
    {
        kdrv_spif_reset_device(); //to clear status
        //running error handling
        kmdw_printf("LOG_CRITICAL: kdev_flash_programdata addr 0x%X reports status error 0x%X!\n",addr,status);
    }
    return status;
}

kdev_status_t kdev_flash_programdata_partial(uint32_t addr, const void *data, uint32_t cnt)
{
    kdev_status_t status;
    uint8_t Option = FLASH_OP_MODE;

    flash_msg("kdev_flash_programming addr = %d! cnt = %d!", addr, cnt);

    //check is_cross_two_block?
    uint32_t block = addr / SPI020_BLOCK_128SIZE;
    uint32_t left = ((block+1) * SPI020_BLOCK_128SIZE) - addr;
    if(left >= cnt)
    {
        status = kdev_flash_block_backup(Option, addr, data, cnt);
        status = kdev_flash_check_cumulativeECCstauts();
        if(status != KDEV_STATUS_OK)
        {
            kdrv_spif_reset_device(); //to clear status
            //running error handling
            kmdw_printf("LOG_CRITICAL: kdev_flash_programdata addr 0x%X reports status error 0x%X!\n",addr,status);
        }
    }
    else /* cross 2 blocks */
    {
        uint8_t *databuf;
        databuf = (uint8_t *)data;
        status = kdev_flash_block_backup(Option, addr, databuf, left);
        status = kdev_flash_block_backup(Option, (addr+left), (databuf+left), (cnt-left));
        status = kdev_flash_check_cumulativeECCstauts();
        if(status != KDEV_STATUS_OK)
        {
            kdrv_spif_reset_device(); //to clear status
            //running error handling
            kmdw_printf("LOG_CRITICAL: kdev_flash_programdata addr 0x%X reports status error 0x%X!\n",addr,status);
        }
    }
    return status;
}

kdev_status_t kdev_flash_programdata_memxfer(uint32_t addr, const void *data, uint32_t cnt)
{
    kdev_status_t status;

    flash_msg("kdev_flash_programming addr = %d! cnt = %d!", addr, cnt);

    if(cnt<SPI020_BLOCK_128SIZE)
    {
        status = kdev_flash_programdata_partial(addr, data, cnt);
    }
    else
    {
        kmdw_printf("LOG_CRITICAL: Can't use this function to program more than 1 block data!!\n",addr,status);
        status = KDEV_STATUS_ERROR;
        /* program size > 128k Bytes(1block), please consider to assigned specific blocks area for better file management. */
    }
    return status;
}

kdev_status_t kdev_flash_erase_sector(uint32_t addr)
{
    kdev_flash_128kErase(addr); //for program all
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_erase_multi_sector(uint32_t start_addr, uint32_t end_addr)
{
    uint16_t i=0;
    uint32_t nstart_index = 0;
    uint32_t nend_index = 0;
    nstart_index = (start_addr / SPI020_BLOCK_128SIZE);
    nend_index = (end_addr / SPI020_BLOCK_128SIZE);
    if( ( end_addr%SPI020_BLOCK_128SIZE) == 0  && nend_index > nstart_index )
    {
        nend_index --;
    }
    flash_msg("_flash_erase_multi_sectors start_addr = %X! end_addr = %X!", start_addr, end_addr);
    flash_msg("_flash_erase_multi_sectors start_index = %d! end_index = %d!", nstart_index, nend_index);
    if(  (nstart_index <= nend_index)  ||  (nend_index < st_flash_info.total_sector_numbers) )
    {
        for(i=nstart_index; i<=nend_index; i++)
        {
            flash_msg("_flash_erase_multi_sectors addr = %d*%d=0x%X!", i, SPI020_BLOCK_128SIZE, i*SPI020_BLOCK_128SIZE);
            kdev_flash_128kErase(i*SPI020_BLOCK_128SIZE);
            flash_msg("_flash_erase_multi_sectors addr = %d*%d=0x%X done!", i, SPI020_BLOCK_128SIZE, i*SPI020_BLOCK_128SIZE);
        }
        return KDEV_STATUS_OK;
    }
    return KDEV_STATUS_ERROR;
}

kdev_status_t kdev_flash_erase_chip(void)
{
    uint32_t i=0;

    kdrv_spif_reset_device(); //to clear status
    for(i=0; i<st_flash_info.block_size_Bytes; i++)
    {
        if(0xFF == kdev_flash_read_BBM(i)) /* non-FFh check */
        {
            kdev_flash_write_control(1);/* send write enabled */

            /* block address */
            kdrv_spif_set_commands(i<<6, SPI020_D8_CMD1, SPI020_D8_CMD2, SPI020_D8_CMD3);
            /* wait for command complete */
            kdrv_spif_check_status_till_ready();
            kdev_status_t kdev_status = kdev_flash_check_cumulativeECCstauts();
            if(kdev_status != KDEV_STATUS_OK)
            {
                kdrv_spif_reset_device(); //to clear status
                //running error handling
                kmdw_printf("LOG_ERROR: kdev_flash_erase_chip block %d reports status fail 0x%X\n",i,kdev_status);
            }
        }
    }
    kdrv_spif_reset_device(); //to clear status
    kdev_flash_scan_all_BBM();

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
    status = kdev_flash_read_flash_id();
    flash_msg("Read Flash SFDP %s", (status==KDEV_STATUS_OK ? "PASS" : "FAIL"));
    return status;
}


kdev_status_t kdev_memxfer_flash_to_ddr(uint32_t dst, uint32_t src, size_t bytes, uint8_t mode)
{

    int32_t read_length;
    int32_t access_byte;
    uint32_t write_addr;
    uint32_t read_data;
    int32_t rx_fifo_depth;
    uint32_t page_addr_start;
    uint32_t total_pages;
    uint32_t page_shift_dummy;
    uint32_t read_lens;

    if ((bytes & 0x3) > 0) return KDEV_STATUS_ERROR;

    page_addr_start = src / spi_nand_data_buf_size; //memory area page 0~65535
    //column_addr_start = 0; //data buffer

    read_length = bytes;
    total_pages = (bytes+(spi_nand_data_buf_size-1)) / spi_nand_data_buf_size;
    write_addr = dst;
    rx_fifo_depth = (int32_t)kdrv_spif_rxfifo_depth();

    for(int32_t i=0; i<total_pages; i++)
    {

        page_shift_dummy = (uint16_t)(page_addr_start+i);
        if(read_length > spi_nand_data_buf_size)
        {
            read_lens = spi_nand_data_buf_size;
            read_length -= read_lens;
        }
        else
        {
            read_lens = read_length;
            read_length -= read_lens;
        }

        kdrv_spif_set_commands(page_shift_dummy, SPI020_13_CMD1, SPI020_13_CMD2, SPI020_13_CMD3);
        //kdrv_delay_us(60);
        kdrv_spif_check_status_till_ready();


        /*if(i == 0)
        {
            if(read_lens>(2048 - column_addr_start))
                kdrv_spif_set_commands(column_addr_start, SPI020_03_CMD1, (2048 - column_addr_start), SPI020_03_CMD3);
            else
                kdrv_spif_set_commands(column_addr_start, SPI020_03_CMD1, read_lens, SPI020_03_CMD3);
        }
        else*/
        if(mode==0x6C)
        {
            kdrv_spif_set_commands(0, SPI020_6C_CMD1, read_lens, SPI020_6C_CMD3);
        }
        else if(mode==0xEC)
        {
            kdrv_spif_set_commands(0, SPI020_EC_CMD1, read_lens, SPI020_EC_CMD3);
        }
        else if(mode==0xBC)
        {
            kdrv_spif_set_commands(0, SPI020_BC_CMD1, read_lens, SPI020_BC_CMD3);
        }
        else if(mode==0x3C)
        {
            kdrv_spif_set_commands(0, SPI020_3C_CMD1, read_lens, SPI020_3C_CMD3);
        }
        else
        {
            kdrv_spif_set_commands(0, SPI020_03_CMD1, read_lens, SPI020_03_CMD3);
        }

        while (read_lens > 0)
        {
            kdrv_spif_wait_rx_full();

            access_byte = min_t(read_lens, rx_fifo_depth);
            read_lens -= access_byte;
            while (access_byte > 0)
            {
                read_data = regSPIF_data->dw.kdrv_spif_dp;//u32Lib_LeRead32((unsigned char* )SPI020REG_DATAPORT);
                *(volatile unsigned int *)(write_addr) = read_data;//outw(write_addr, read_data);
                write_addr += 4;
                access_byte -= 4;
            }
        }

        kdrv_spif_check_status_till_ready();/* wait for command complete */
        //flash_msg("read 1 page 2048 bytes done!!");

    }

#ifndef MIXING_MODE_OPEN_RENDERER
    //kdrv_spif_wait_command_complete();/* wait for command complete */
#endif

    return KDEV_STATUS_OK;
}

