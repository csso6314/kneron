#include "kmdw_memxfer.h"
#include "kmdw_console.h"
#include "kdev_flash.h"

#define FLASH_WB_DEV            0xEF
#define FLASH_MXIC_DEV          0xC2
#define FLASH_Micron_DEV        0x20
#define FLASH_SIZE_1MB_ID       0x14

#define MEMXFER_INITED          0x10
#define MEMXFER_OPS_MASK MEMXFER_OPS_CPU | MEMXFER_OPS_DMA

#define SPI_QUAD_MODE
#define SPI_BUS_SPEED_100MHZ    0x01
#define SPI_BUS_SPEED_50MHZ     0x02
#define SPI_BUS_SPEED_25MHZ     0x04
#define SPI_BUS_SPEED           SPI_BUS_SPEED_100MHZ


#if FLASH_4BYTES_CMD_EN
extern void kdev_flash_4Bytes_ctrl(uint8_t enable);
#endif

#define _get_min(x,y) ( x < y ? x: y )

#if !defined(FLASH_TYPE)
static uint8_t _flash_mode = MEMXFER_OPS_NONE;
static uint8_t _mem_mode = MEMXFER_OPS_NONE;
#endif


static uint8_t flash_device_id = 0;

extern void kdrv_spif_set_commands(uint32_t cmd0, uint32_t cmd1, uint32_t cmd2, uint32_t cmd3);
extern void kdrv_spif_wait_command_complete(void);
extern void kdev_flash_write_control(uint8_t  enable);
extern void kdrv_spif_check_quad_status_till_ready(void);
extern void kdev_flash_64kErase(uint32_t  offset);
extern kdev_status_t kdev_flash_read_flash_id(void);

#define VERIFY_BLK_SZ FLASH_MINI_BLOCK_SIZE

#if defined(SPIF_CONTINUOUS_READ_EN) && (FLASH_TYPE == FLASH_TYPE_WINBOND_NAND)
static int _kdp_memxfer_flash_to_ddr_auto(uint32_t dst, uint32_t src, size_t bytes, bool continuous)
{
    uint32_t remainder, loop, i, len;

    loop = bytes / VERIFY_BLK_SZ;
    if(bytes % VERIFY_BLK_SZ)
        loop += 1;
    remainder = bytes;

    for (i = 0; i < loop; i++) {
        len = (remainder > VERIFY_BLK_SZ) ? VERIFY_BLK_SZ : remainder;

#if defined(FLASH_TYPE) && (FLASH_TYPE == FLASH_TYPE_WINBOND_NAND)
        if(continuous)
            kdev_flash_continuous_read((src + i * VERIFY_BLK_SZ),
                             (void*)(dst + i * VERIFY_BLK_SZ), len);  // read the new sector
        else
            kdev_flash_readdata((src + i * VERIFY_BLK_SZ),
                            (void*)(dst + i * VERIFY_BLK_SZ), len);  // read the new sector
      
#else
        kdev_flash_readdata((src + i * VERIFY_BLK_SZ),
                            (void*)(dst + i * VERIFY_BLK_SZ), len);  // read the new sector
#endif
        remainder -= len;
    }
    return 0;
}
#endif

static 
int _kdp_memxfer_flash_to_ddr(uint32_t dst, uint32_t src, size_t bytes)
{
#if defined(SPIF_CONTINUOUS_READ_EN) && (FLASH_TYPE == FLASH_TYPE_WINBOND_NAND)
    uint32_t total_len = bytes;
    uint32_t read_len = 0;
    uint32_t offset = (src % spi_nand_data_buf_size);
    
    if(offset)
    {
        if((spi_nand_data_buf_size - offset) > bytes)
            read_len = bytes;
        else
            read_len = (spi_nand_data_buf_size - offset);
        _kdp_memxfer_flash_to_ddr_auto(dst, src, read_len, false);
    }
  
    total_len -= read_len;
    if(total_len)
    {
        _kdp_memxfer_flash_to_ddr_auto(dst+read_len, (src+read_len), total_len, true);
    }
#else
    uint32_t remainder, loop, i, len;

    loop = bytes / VERIFY_BLK_SZ;
    if(bytes % VERIFY_BLK_SZ)
        loop += 1;
    remainder = bytes;

    for (i = 0; i < loop; i++) {
        len = (remainder > VERIFY_BLK_SZ) ? VERIFY_BLK_SZ : remainder;
        kdev_flash_readdata((src + i * VERIFY_BLK_SZ),
                            (void*)(dst + i * VERIFY_BLK_SZ), len);  // read the new sector

        remainder -= len;
    }
#endif

    return 0;
}




static
int _kdp_memxfer_ddr_to_flash(uint32_t dst, uint32_t src, size_t bytes)
{
    /* please be noted this function is for program size < 128KBytes */
    kdev_flash_programdata_memxfer(dst, (void*)src, bytes);

    return 0;
}

int kdp_memxfer_init(uint8_t flash_mode, uint8_t mem_mode)
{
	#if defined(FLASH_TYPE)
    static int flash_inited = 0;

    if (!flash_inited) {
        flash_inited = 1;
        return(kdev_flash_initialize());
    } else {
        return 0;
    }

	#else
    int ret = -1;

    _flash_mode = (_flash_mode & (~MEMXFER_OPS_MASK&0xFF)) | flash_mode;
    _mem_mode = (_mem_mode & (~MEMXFER_OPS_MASK&0xFF)) | mem_mode;
    kdrv_spif_memxfer_initialize(_flash_mode, _mem_mode);
    if (!(_flash_mode & MEMXFER_INITED)) {
        kdev_flash_read_flash_id();

        #if FLASH_4BYTES_CMD_EN
        kdev_flash_4Bytes_ctrl(1);
        #else
        kdev_flash_4Bytes_ctrl(0);
        #endif    

        _flash_mode |= MEMXFER_INITED;
    }
    
    return ret;
    #endif
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
    //must consider for NAND
    
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
    kdp_memxfer_get_flash_device_id,
};

