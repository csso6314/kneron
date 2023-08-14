/*
 * Kneron System driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <stdlib.h>
#include "cmsis_os2.h"
#include "base.h"
#include "kdrv_ncpu.h"
#include "kmdw_dfu.h"
#include "kdrv_mpu.h"
#include "kmdw_memxfer.h"
#include "kmdw_console.h"
#include "kdrv_clock.h"
#include "kdrv_io.h"
#include "kmdw_sbt.h"
#include "kdev_flash.h"

typedef enum {
    skip_ncpu_load = 0,
    make_ncpu_load = 1,
}ncpu_load;

#ifndef _BOARD_SN720HAPS_H_
static kdrv_status_t _kmdw_system_flash_to_niram(int part_idx)
{
    uint32_t flash_ncpu_offset, flash_ncpu_ddr_offset;
    flash_ncpu_offset = part_idx == 0? FLASH_FW_NCPU0_ADDR : FLASH_FW_NCPU1_ADDR;
    flash_ncpu_ddr_offset = flash_ncpu_offset + NCPU_IMAGE_IRAM_SIZE;
    /*
     *  swap buffer: 16KB sram, NCPU load time : 223738us
     *  swap buffer: 3MB ddr,   NCPU load time : 627997us
     */
    if (kmdw_sbt_get_boot_flag()) {
        /* secure_boot flag enable */

        #define SBT_SWAP_SIZE  (16 * 1024)
        uint32_t *buf = (uint32_t*)malloc((SBT_SWAP_SIZE/4) * sizeof(uint32_t));
        if(NULL == buf) {
            err_msg("Error! Insufficent memory for NCPU FW loading\n");
            return KDRV_STATUS_ERROR;
        }

        kmdw_sbt_flash_fw_loader(flash_ncpu_offset, FW_NCPU, (void*) buf, SBT_SWAP_SIZE);

        free(buf);
    } else {
        /* secure boot flag disable */
        /* stop ncpu, then load ncpu firmware from flash to NiRAM & DDR */
        kdp_memxfer_module.flash_to_ddr((uint32_t)NiRAM_MEM_BASE, flash_ncpu_offset, NCPU_IMAGE_IRAM_SIZE);
        kdp_memxfer_module.flash_to_ddr((uint32_t)NCPU_FW_DDR_BASE, flash_ncpu_ddr_offset, NCPU_IMAGE_DDR_SIZE);
    }

    return KDRV_STATUS_OK;
}

#endif

/**
*   flag = 0, just reset ncpu
*   flag <> 0, load and launch ncpu
*   flag < 0, not using mpu
*
*   @retVal: 0, new sum32 value matches with another one in FLASH
*           >0, return wrong new sum32 value
*/

kdrv_status_t load_ncpu_fw(int32_t reset_flag)
{
    kdrv_status_t sts = KDRV_STATUS_OK;
  #ifndef _BOARD_SN720HAPS_H_
    //check magic number DDR_MAGIC_BASE
    uint32_t dfu_active_sts;
    int32_t skip_flag = make_ncpu_load;
    uint32_t magic_lb, magic_hb;
    magic_lb = inw(DDR_MAGIC_BASE);
    magic_hb = inw(DDR_MAGIC_BASE+0x04);
    //kmdw_printf("========Magic number is 0x%04x%04x========\n", magic_hb, magic_lb);
    if((magic_lb == 0x11223344) && (magic_hb == 0xaabbccdd))
    {
        //kmdw_printf("========Magic number is correct 0x%04x%04x========\n", magic_hb, magic_lb);
        kmdw_printf("==boot from USB_DFW==\n");
        skip_flag = skip_ncpu_load;
    }
    else
    {
    #if defined(FLASH_TYPE) && (FLASH_TYPE == FLASH_TYPE_NULL)
        skip_flag = skip_ncpu_load;                      // there's no flash
    #else
        kmdw_printf("==boot from External Flash==\n");
    #endif
    }
  #endif
    kdrv_ncpu_boot_initialize();
    kdrv_ncpu_set_stall(1);
  #ifndef _BOARD_SN720HAPS_H_
    if (reset_flag && skip_flag) {
        if (reset_flag > 0)
            kdrv_mpu_niram_enable();
        /* flash sataus check and update */
        kmdw_dfu_cfg_sts_check();

        /* get active ncpu partition id */
        dfu_active_sts = kmdw_dfu_get_active_ncpu_partition();
        sts = _kmdw_system_flash_to_niram(dfu_active_sts);
        if (sts == KDRV_STATUS_OK)
            kmdw_printf("Load NCPU from flash image%d: 0x%08x\n", dfu_active_sts, (dfu_active_sts?FLASH_FW_NCPU1_ADDR:FLASH_FW_NCPU0_ADDR));
        else
            kmdw_printf("Load NCPU from flash failed\n");
        
        /* ncpu firmware include NiRAM + DDR, it includes the NCPU vector on the DDR.
        So ncpu may check fail on the CRC32 */
        //ret = system_check_fw_image(NCPU_FW);
        if (reset_flag > 0)
            kdrv_mpu_niram_disable();
    }
  #endif
    kdrv_ncpu_set_stall(1);
    kdrv_ncpu_reset();
    kdrv_ncpu_set_stall(0);
    return sts;
}

#if 0
/**
 * @brief Calculate sum32 of scpu/ncpu fw and compare to the value which resides at the end of flash
 * N-iRAM(64KB) 0x28000000 ~ 0x2800FFFF
 * S-iRAM(88KB) 0x10102000 ~ 0x10117FFF
 * 
 * If sums are the same, return 0  (Note: check value could be zero)
 * Else return wrong calculated value, if value is zero, return 0xffffffff
 */
uint32_t system_check_fw_image(int32_t fw_type)
{
    uint8_t *pBase;
    uint32_t sum32_cal, sum32_in_flash;
    uint32_t size;

    if(fw_type == SCPU_FW) {
        pBase = (uint8_t *)SCPU_START_ADDRESS;
        size = (SCPU_IMAGE_SIZE - 4);
    }
    else if(fw_type == NCPU_FW){
        pBase = (uint8_t *)NCPU_START_ADDRESS;
        size = (NCPU_IMAGE_SIZE - 4);
    }
    
    sum32_cal = kdp_gen_sum32(pBase, size);
    sum32_in_flash = *(uint32_t *)(pBase + size);
    if (sum32_cal == sum32_in_flash)
            return 0;
    if (sum32_cal == 0)
        sum32_cal--;
    return sum32_cal;
}
#endif
