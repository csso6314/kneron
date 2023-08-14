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

/* History:
 *  Version 2.00
 *    Renamed driver NOR -> Flash (more generic)
 *    Non-blocking operation
 *    Added Events, Status and Capabilities
 *    Linked Flash information (GetInfo)
 *  Version 1.11
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */

/**@addtogroup  KDEV_FLASH  KDEV_FLASH
 * @{
 * @brief       Kneron flash device
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KDEV_FLASH_NOR_H
#define __KDEV_FLASH_NOR_H

#include "Driver_Common.h"
#include "kdrv_SPI020.h"
#include "kdev_status.h"
#include "project.h"
#if defined(FLASH_DRV) && (FLASH_DRV == FLASH_DRV_QUAD_OUTPUT_MODE)
#define FLASH_OP_MODE           FLASH_QUAD_RW
#elif defined(FLASH_DRV) && (FLASH_DRV == FLASH_DRV_QUAD_IO_MODE)
#define FLASH_OP_MODE           (FLASH_QUAD_RW|FLASH_IO_RW)
#elif defined(FLASH_DRV) && (FLASH_DRV == FLASH_DRV_DUAL_OUTPUT_MODE)
#define FLASH_OP_MODE           FLASH_DUAL_READ
#elif defined(FLASH_DRV) && (FLASH_DRV == FLASH_DRV_DUAL_IO_MODE)
#define FLASH_OP_MODE           (FLASH_DUAL_READ|FLASH_IO_RW)
#elif defined(FLASH_DRV) && (FLASH_DRV == FLASH_DRV_NORMAL_MODE)
#define FLASH_OP_MODE           FLASH_NORMAL
#else
#error "FLASH_DRV doesn't defined in project.h"
#endif

#define SPI020_SECTOR_SIZE		4096
#define SPI020_BLOCK_64SIZE		65536

#define FLASH_CODE_OPT              (YES)
#define	FLASH_CODING_GET_INFO_EN    (YES)
#define FLASH_QUAD_IO_EN        0x01

/**
* @brief Flash Sector index struct
*/
typedef struct {
  uint32_t start;                       /**< Sector Start address */
  uint32_t end;                         /**< Sector End address (start+size-1) */
}kdev_flash_sector_t;

/**
* @brief Flash information struct
*/
typedef struct {
  kdev_flash_sector_t *sector_info;        /**< Sector layout information (NULL=Uniform sectors) */
  uint32_t            sector_count;       /**< Number of sectors */
  uint32_t            sector_size;        /**< Uniform sector size in bytes (0=sector_info used) */
  uint32_t            page_size;          /**< Optimal programming page size in bytes */
  uint32_t            program_unit;       /**< Smallest programmable unit in bytes */
  uint8_t             erased_value;       /**< Contents of erased memory (usually 0xFF) */
  uint32_t            flash_size;
} kdev_flash_info_t;    

typedef struct _flash_paramter
{
    uint32_t  signature;							    //0x00
    uint8_t   PTP;								        //0x0C
    uint8_t   ID;									        //0x10
    uint8_t   erase_4K_support;				    //0x00 => 0x30[1:0];
    uint32_t  flash_size_KByte;				    //0x04~0x07 => 0x34~0x37
    uint16_t  page_size_Bytes;				    //0x28 => 0x58[7:4]=0x8
    uint16_t  sector_size_Bytes;
    uint32_t  block_size_Bytes;				    //how many sectors in one block
    uint16_t  total_sector_numbers;
} kdrv_spif_parameter_t;

/**
* @brief Flash Status struct
*/
typedef struct {
  uint32_t busy  : 1;                   /**< Flash busy flag */
  uint32_t error : 1;                   /**< Read/Program/Erase error flag (cleared on start of next operation) */
} kdev_flash_status_t;


extern uint32_t  kdev_flash_probe(spi_flash_t *flash);

// Function documentation
/**
* @fn          kdev_status_t kdev_flash_initialize (void)
* @brief       Initialize spi flash interface include hardware setting, get flash information and set to 4byte address 
*              if flash size is bigger than 16Mbytes
* @param[in]   N/A
* @return      @ref kdrv_status_t
*
* @note        This API MUST be called before using the Read/write APIs for spi flash.
*/
kdev_status_t kdev_flash_initialize(void);//ARM_Flash_SignalEvent_t cb_event);
/**
* @fn          kdrv_status_t kdev_flash_uninitialize(void)
* @brief       Uinitialize the spi flash interface.
* @return      @ref kdrv_status_t
*/
kdev_status_t kdev_flash_uninitialize(void);
/**
* @fn          kdev_status_t kdev_flash_power_control(ARM_POWER_STATE state)
* @brief       Power handling for spi flasg.
* @param[in]   state  Power state
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_power_control(ARM_POWER_STATE state);
/**
* @fn          kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt)
* @brief       Read data from specific index of spi flash.
* @param[in]   addr  Data address.
* @param[out]  data  Pointer to a buffer storing the data read from Flash.
* @param[in]   cnt   Number of data items to read.
* @return      number of data items read or @ref kdev_status_t
*/
kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt);
/**
* @fn          void kdev_flash_read(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf)
* @brief       Read data from specific index of spi flash.
* @param[in]   type  SPI operation type: standard/Dual/Quad mode.
* @param[in]   offset  Data address.
* @param[in]   len   Number of data items to read.
* @param[out]  data  Pointer to a buffer storing the data read from Flash.
* @return      N/A
*/
void kdev_flash_read(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf);
uint32_t kdev_flash_read_compare(void *buf, uint32_t  offset, uint32_t  len, uint8_t  type);
/**
* @fn          kdev_status_t kdev_flash_programdata (uint32_t addr, const void *data, uint32_t cnt)
* @brief       Program data to specific index in spi flash
* @param[in]   addr  Data address.
* @param[in]   data  Pointer to a buffer containing the data to be programmed to Flash.
* @param[in]   cnt   Number of data items to program.
* @return      number of data items programmed or @ref kdev_status_t
*/
kdev_status_t kdev_flash_programdata (uint32_t addr, const void *data, uint32_t cnt);
/**
* @fn          kdev_status_t kdev_flash_programdata (uint32_t addr, const void *data, uint32_t cnt)
* @brief       Program data to specific index in spi flash
* @param[in]   addr  Data address.
* @param[in]   data  Pointer to a buffer containing the data to be programmed to Flash.
* @param[in]   cnt   Number of data items to program.
* @return      number of data items programmed or @ref kdev_status_t
*/
kdev_status_t kdev_flash_programdata_memxfer(uint32_t addr, const void *data, uint32_t cnt);
/**
* @fn          kdev_status_t kdev_flash_erase_sector(uint32_t addr)
* @brief       Erase Flash by Sector(4k bytes).
* @param[in]   addr  Sector address
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_erase_sector(uint32_t addr);
/**
* @fn          kdev_status_t kdev_flash_erase_multi_sector(uint32_t start_addr, uint32_t end_addr)
* @brief       Erase multiple Flash Sectors(continuously).
* @param[in]   addr  Sector start address
* @param[in]   addr  Sector end address
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_erase_multi_sector(uint32_t start_addr, uint32_t end_addr);
/**
* @fn          kdev_status_t kdev_flash_erase_chip(void)
* @brief       Erase whole Flash at once.
               Optional function for faster full chip erase.
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_erase_chip(void);
/**
* @fn          kdev_flash_status_t kdev_flash_get_status(void)
* @brief       Get Flash status.
* @return      Flash status @ref kdev_flash_status_t
*/
kdev_flash_status_t kdev_flash_get_status(void);
/**
* @fn          kdev_status_t kdev_flash_get_info(void)
* @brief       Get Flash information.
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_get_info(void);
/**
* @fn          void kdev_flash_64kErase(uint32_t  offset)
* @brief       Block Erase to erase Flash 64K-bytes at once.
               The Block Erase instruction sets all memory within a specified block (64K-bytes) to the erased state of all FFh..
* @param[in]   offset  Sector start address
* @return      N/A
*/
void kdev_flash_64kErase(uint32_t  offset);
/**
* @fn          void kdev_flash_64kErase(uint32_t  offset)
* @brief       Sector Erase to erase Flash 4K-bytes at once.
               The Sector Erase instruction sets all memory within a specified sector (4K-bytes) to the erased state of all FFh..
* @param[in]   offset  Sector start address
* @return      N/A
*/
void kdev_flash_4kErase(uint32_t  offset);

#endif /* __KDEV_FLASH_H */
/** @}*/
