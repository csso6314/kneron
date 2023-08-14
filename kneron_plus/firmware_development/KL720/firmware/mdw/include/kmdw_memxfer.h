/**
 * @file        kmdw_memxfer.h
 * @brief       memory operation between fash/ddr
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */
#ifndef __MEMXFER_H__
#define __MEMXFER_H__


#include "base.h"
#include "cmsis_os2.h"

#define MEMXFER_OPS_NONE 	0x00    /**< default op */
#define MEMXFER_OPS_CPU 	0x01    /**< transfer by CPU */
#define MEMXFER_OPS_DMA 	0x02    /**< transfer by DMA */

extern const struct s_kdp_memxfer kdp_memxfer_module;

struct s_kdp_memxfer {
    int (*init)(uint8_t flash_mode, uint8_t mem_mode);
    int (*flash_to_ddr)(uint32_t dst, uint32_t src, size_t bytes);
    int (*ddr_to_flash)(uint32_t dst, uint32_t src, size_t bytes);
    int (*flash_sector_erase64k)(uint32_t addr);
    uint8_t  (*flash_get_device_id)(void);
} ;

#endif
