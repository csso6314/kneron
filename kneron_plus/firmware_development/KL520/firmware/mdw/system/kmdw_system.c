/*
 * Kneron System driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "cmsis_os2.h"
#include "board.h"
#include "project.h"

#include "kdrv_system.h"
#include "kdrv_scu.h"
#include "kdrv_scu_ext.h"
#include "kdrv_power.h"
#include "kdrv_clock.h"
#include "kdrv_ddr.h"
#include "kdrv_pwm.h"
#include "kdrv_mpu.h"

#include "kmdw_console.h"
#include "kmdw_memxfer.h"
#include "kmdw_dfu.h"
#include "kmdw_utils_crc.h"
#include "kmdw_system.h"

extern const struct s_kdp_memxfer kdp_memxfer_module;

void system_wakeup_ncpu(int32_t boot_loader_flag, uint8_t wakeup_all)
{
    if (boot_loader_flag)
        kdrv_pwmtimer_delay_ms(200);

    if (1 == wakeup_all) {    
        kdrv_clock_enable(CLK_SCPU_TRACE);
        kdrv_clock_enable(CLK_NCPU);
        kdrv_clock_enable(CLK_NPU);
        kdrv_system_reset(SUBSYS_PD_NPU);
        kdrv_system_reset(SUBSYS_NPU);
    }
    
    SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1);
}



/**
 * @brief RELOAD_NCPU_FW(), Reload NCPU firmware from flash
 */
void reload_ncpu_fw(void)
{
    kdrv_mpu_niram_enable();   
    int dfu_active_sts = kmdw_dfu_get_active_ncpu_partition();
    kdp_memxfer_module.init(MEMXFER_OPS_DMA, MEMXFER_OPS_DMA);
    kdp_memxfer_module.flash_to_niram(dfu_active_sts);
    kdrv_mpu_niram_disable();
    kdrv_system_reset(SUBSYS_NCPU);
}

/**
*   flag = 0, just launch ncpu
*   flag <> 0, load and launch ncpu
*   flag < 0, not using mpu
*
*   @retVal: 0, new sum32 value matches with another one in FLASH
*           >0, return wrong new sum32 value
*/
u32 load_ncpu_fw(int32_t reset_flag)
{
    uint32_t ret = 0;
    SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(0);  // stop ncpu first
    if (reset_flag) {
        if (reset_flag > 0)
            kdrv_mpu_niram_enable();
        /* initialize flash first for kmdw_dfu_get_active_ncpu_partition() */
        kdp_memxfer_module.init(MEMXFER_OPS_DMA, MEMXFER_OPS_DMA);
        int dfu_active_sts = kmdw_dfu_get_active_ncpu_partition();
        kdp_memxfer_module.flash_to_niram(dfu_active_sts);
        /*ncpu may change its interrupt vector in NiRAM, which may fail the image check.
        So ncpu check should be done before ncput is waken up and run*/
        ret = system_check_fw_image(NCPU_FW);
        if (reset_flag > 0)
            kdrv_mpu_niram_disable();
    }
    SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1);  // restart ncpu
    return ret;
}

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
        pBase = (u8 *)SCPU_START_ADDRESS;
        size = (SCPU_IMAGE_SIZE - 4);
    }
    else if(fw_type == NCPU_FW){
        pBase = (u8 *)NCPU_START_ADDRESS;
        size = (NCPU_IMAGE_SIZE - 4);
    }

    sum32_cal = kmdw_utils_crc_gen_sum32(pBase, size);
    sum32_in_flash = *(u32 *)(pBase + size);
    if (sum32_cal == sum32_in_flash)
            return 0;
    if (sum32_cal == 0)
        sum32_cal--;
    return sum32_cal;
}

