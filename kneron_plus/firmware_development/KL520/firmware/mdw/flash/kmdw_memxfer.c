#include <string.h>
#include "io.h"
#include "project.h"

#include "kdrv_spif.h"
#include "kdev_flash.h"
#include "kmdw_memxfer.h"
#include "kmdw_dfu.h"
#include "kmdw_console.h"

#define MEMXFER_INITED      0x10
#define SPI_QUAD_MODE


#if FLASH_4BYTES_CMD_EN
extern void kdev_flash_4Bytes_ctrl(uint8_t enable);
#endif

#define _get_min(x,y) ( x < y ? x: y )

static uint8_t _flash_mode = MEMXFER_OPS_NONE;
static uint8_t _mem_mode = MEMXFER_OPS_NONE;

static uint8_t flash_device_id = 0;

extern void kdrv_spif_set_commands(uint32_t cmd0, uint32_t cmd1, uint32_t cmd2, uint32_t cmd3);
extern void kdrv_spif_wait_command_complete(void);
extern void kdev_flash_write_control(uint8_t  enable);
extern void kdrv_spif_check_quad_status_till_ready(void);
extern void kdev_flash_64kErase(uint32_t  offset);
extern void kdev_flash_read_flash_id(void);

static 
int _kdp_memxfer_flash_to_ddr(uint32_t dst, uint32_t src, size_t bytes)
{
    int32_t total_lens;
    int32_t access_byte;
    uint32_t write_addr;
    uint32_t read_data;
    int32_t rx_fifo_depth;
    
    if ((bytes & 0x3) > 0) return -1;
    
    total_lens = bytes;
    write_addr = dst;
    rx_fifo_depth = (int32_t)kdrv_spif_rxfifo_depth();
    //read from flash
    //write to ddr

#if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address(src))
    {
        kdev_flash_4Bytes_ctrl(1);
    #ifdef SPI_QUAD_MODE
        kdrv_spif_set_commands(src, SPI020_EC_CMD1, total_lens, SPI020_EC_CMD3);
    #else
        kdrv_spif_set_commands(src, SPI020_13_CMD1, total_lens, SPI020_13_CMD3);
    #endif
    }
    else
#endif
    {
    #ifdef SPI_QUAD_MODE
        kdrv_spif_set_commands(src, SPI020_EB_CMD1, total_lens, SPI020_EB_CMD3);
    #else
        #if defined(SPI_BUS_SPEED) && (SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ)
        kdrv_spif_set_commands(src, SPI020_0B_CMD1, total_lens, SPI020_0B_CMD3);
        #else
        kdrv_spif_set_commands(src, SPI020_03_CMD1, total_lens, SPI020_03_CMD3);
        #endif
    #endif
    }

    while (total_lens > 0)
    {        
        kdrv_spif_wait_rx_full();

        access_byte = _get_min(total_lens, rx_fifo_depth);
        total_lens -= access_byte;   

        while (access_byte > 0)
        {
            read_data = regSPIF_data->dw.kdrv_spif_dp;
            outw(write_addr, read_data);
            write_addr += 4;
            access_byte -= 4;
        }
    }    
#ifndef MIXING_MODE_OPEN_RENDERER
    kdrv_spif_wait_command_complete();/* wait for command complete */
#endif    

#if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address(src))
    kdev_flash_4Bytes_ctrl(0);
#endif    

    return 0;
}




static
int _kdp_memxfer_ddr_to_flash(uint32_t dst, uint32_t src, size_t bytes)
{
#define BLOCK_SIZE FLASH_PAGE_SIZE //256
#define SECTOR_ERASE_SIZE SPI020_SECTOR_SIZE //4096
#define SECTOR64_ERASE_SIZE  SPI020_BLOCK_64SIZE //0x10000

    int32_t total_lens;
    uint32_t erase_dst_addr;     
    uint32_t write_dst_addr;    
    int32_t write_len, write_len2;        
    int32_t access_byte;       
    uint32_t read_addr;
    uint32_t write_data;
    int32_t tx_fifo_depth;

    if ((dst & 0x00000FFF) > 0) return -1;
    if ((src & 0x3) > 0) return -1;
    if ((bytes & 0x3) > 0) return -1;

    //erase flash
    total_lens = bytes;    
    erase_dst_addr = dst;

    while (total_lens > 0) 
    {
        if ((total_lens >= SECTOR64_ERASE_SIZE) && ((erase_dst_addr & 0x0FFFF) == 0)) {  // use 64KB erase if possible
            kdev_flash_64kErase(erase_dst_addr);
            total_lens -= SECTOR64_ERASE_SIZE;
            erase_dst_addr += SECTOR64_ERASE_SIZE;
        }
        else {  // use 4K erase
            kdev_flash_write_control(1);
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address(erase_dst_addr))
            {
                kdev_flash_4Bytes_ctrl(1);
                kdrv_spif_set_commands(erase_dst_addr, SPI020_21_CMD1, SPI020_21_CMD2, SPI020_21_CMD3);
            }
            else
                kdrv_spif_set_commands(erase_dst_addr, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
            #else
            kdrv_spif_set_commands(erase_dst_addr, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
            #endif

            kdrv_spif_check_quad_status_till_ready();

            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address(erase_dst_addr))
            kdev_flash_4Bytes_ctrl(0);
            #endif
            total_lens -= SECTOR_ERASE_SIZE;
            erase_dst_addr += SECTOR_ERASE_SIZE;
        }
        if (total_lens <= 0)
            break;
    }
    
    total_lens = bytes;
    write_dst_addr = dst;
    read_addr = src;
    tx_fifo_depth = (int32_t)kdrv_spif_txfifo_depth();    

    while (total_lens > 0) {
        kdev_flash_write_control(1);
        
        write_len = _get_min(total_lens, BLOCK_SIZE);
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address(write_dst_addr))
        {
            kdev_flash_4Bytes_ctrl(1);
            kdrv_spif_set_commands(write_dst_addr, SPI020_12_CMD1, write_len, SPI020_12_CMD3);
        }
        else
            kdrv_spif_set_commands(write_dst_addr, SPI020_02_CMD1, write_len, SPI020_02_CMD3);
        #else
        kdrv_spif_set_commands(write_dst_addr, SPI020_02_CMD1, write_len, SPI020_02_CMD3);        
        #endif        
        
        write_dst_addr += write_len;
        write_len2 = write_len;
        
        while(write_len2 > 0)
        {
            kdrv_spif_wait_tx_empty();
            access_byte = _get_min(write_len2, tx_fifo_depth);
            write_len2 -= access_byte;
            while(access_byte > 0)
            {
                write_data = inw(read_addr);                
                regSPIF_data->dw.kdrv_spif_dp = write_data;
                read_addr += 4;
                access_byte -= 4;
            }
        }        
        
        kdrv_spif_check_quad_status_till_ready();
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address(write_dst_addr))
        kdev_flash_4Bytes_ctrl(0);
        #endif
        total_lens -= write_len;
    }

    return 0;
}

int kdp_memxfer_init(uint8_t flash_mode, uint8_t mem_mode)
{
    int ret = -1;

    _flash_mode = (_flash_mode & (~MEMXFER_OPS_MASK&0xFF)) | flash_mode;
    _mem_mode = (_mem_mode & (~MEMXFER_OPS_MASK&0xFF)) | mem_mode;
    kdrv_spif_memxfer_initialize(_flash_mode, _mem_mode);
    if (!(_flash_mode & MEMXFER_INITED)) {
        kdev_flash_read_flash_id();
        _flash_mode |= MEMXFER_INITED;
    }
    
    return ret;
}

int kdp_memxfer_flash_to_ddr(uint32_t dst, uint32_t src, size_t bytes)
{
    return _kdp_memxfer_flash_to_ddr(dst, src, bytes);
}

int kdp_memxfer_ddr_to_flash(uint32_t dst, uint32_t src, size_t bytes)
{
    return _kdp_memxfer_ddr_to_flash(dst, src, bytes);
}

/**
 * @brief flash 64k sector erase
 */ 
int kdp_memxfer_flash_sector_erase64k(uint32_t addr)
{
    kdev_flash_64kErase(addr);
    return 0;
}

/**  
 * @brief load ncpu firmware code from flash to niram 
 */
int kdp_memxfer_flash_to_niram(int part_idx)

{
    /* stop ncpu, then load code from flash to NiRAM */
    if (part_idx == 0) {
        kdp_memxfer_flash_to_ddr((uint32_t)NCPU_START_ADDRESS, 
                NCPU_PARTITION0_START_IN_FLASH, NCPU_IMAGE_SIZE);
    } else {
        kdp_memxfer_flash_to_ddr((uint32_t)NCPU_START_ADDRESS, 
                NCPU_PARTITION1_START_IN_FLASH, NCPU_IMAGE_SIZE);
    }
    return 0;
}

uint8_t kdp_memxfer_get_flash_device_id(void)
{
    return flash_device_id;
}

const struct s_kdp_memxfer kdp_memxfer_module = {
    kdp_memxfer_init,
    kdp_memxfer_flash_to_ddr,
    kdp_memxfer_ddr_to_flash,
    kdp_memxfer_flash_sector_erase64k,
    kdp_memxfer_flash_to_niram,
    kdp_memxfer_get_flash_device_id,
};

