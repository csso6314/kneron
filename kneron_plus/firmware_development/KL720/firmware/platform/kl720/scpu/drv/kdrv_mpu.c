/* -------------------------------------------------------------------------- 
 * Copyright (c) 2018-2019 Kneron Inc. All rights reserved.
 *
 *      Name:    kdrv_mpu.c
 *      Purpose: Kneron SCPU memory configuration and protection
 *
 *---------------------------------------------------------------------------*/



#include <stdio.h>
#include <string.h>
#include "io.h"
#include "regbase.h"
#include "membase.h"
#include "kdrv_cmsis_core.h"


#define ROM_MEM_BASE 0x10000000
#define SiRAM_MEM_VECTOR_OFFSET 0x2000
#define MPU_DEFAULT_MAP 0x04
#define MPU_XN 1U << 28
#define MPU_1KB (ARM_MPU_REGION_SIZE_1KB << 1)
#define MPU_32KB (ARM_MPU_REGION_SIZE_32KB << 1)
#define MPU_64KB (ARM_MPU_REGION_SIZE_64KB << 1)
#define MPU_128KB (ARM_MPU_REGION_SIZE_128KB << 1)
#define MPU_96KB_SRD (0xC0 << 8) // disable sub region 6 and 7
#define MPU_64MB (ARM_MPU_REGION_SIZE_64MB << 1)
#define MPU_4GB (ARM_MPU_REGION_SIZE_4GB << 1)
#define MPU_2pGB_SRD (0xE0 << 8)  // exclude sub-region 5, 6 and 7 (0xA000-0xFFFF uses default)
#define MPU_RW (ARM_MPU_AP_FULL << 24)
#define MPU_RO (ARM_MPU_AP_RO << 24)
#define MPU_NA (ARM_MPU_AP_NONE << 24)
#define MPU_CACHE  (0x0F << 16) // TEX = 001, S = 1, C = 1, B = 1, -> full cache WBWA shared
#define MPU_DEV_S  (0x05 << 16) // TEX = 000, S = 1, C = 0, B = 1, -> shared device
#define MPU_EN 1


void kdrv_mpu_config(void)
{
    ARM_MPU_SetRegionEx(0, 0, MPU_NA | MPU_DEV_S | MPU_4GB | MPU_2pGB_SRD | MPU_XN | MPU_EN);
    ARM_MPU_SetRegionEx(1, ROM_MEM_BASE, MPU_RO | MPU_CACHE | MPU_32KB | MPU_EN);
    ARM_MPU_SetRegionEx(2, SiRAM_MEM_BASE, MPU_RO | MPU_CACHE | MPU_128KB | MPU_96KB_SRD | MPU_EN);
    ARM_MPU_SetRegionEx(3, SiRAM_MEM_BASE + SiRAM_MEM_VECTOR_OFFSET, MPU_RW | MPU_CACHE | MPU_1KB | MPU_EN);
    ARM_MPU_SetRegionEx(4, SdRAM_MEM_BASE, MPU_RW | MPU_CACHE | MPU_128KB | MPU_96KB_SRD | MPU_XN | MPU_EN);
    ARM_MPU_SetRegionEx(5, DDR_MEM_BASE, MPU_RW | MPU_CACHE | MPU_64MB | MPU_XN | MPU_EN);
    ARM_MPU_Enable(MPU_DEFAULT_MAP);  // enable the background mem map
}

void kdrv_mpu_niram_enable(void)
{
    ARM_MPU_Disable();  // turn off mpu
    ARM_MPU_SetRegionEx(6, NiRAM_MEM_BASE, MPU_RW | MPU_CACHE | MPU_64KB | MPU_EN);
    ARM_MPU_Enable(MPU_DEFAULT_MAP);  // re-enable mpu
}

void kdrv_mpu_niram_disable(void)
{
    ARM_MPU_Disable();  // turn off mpu
    ARM_MPU_SetRegionEx(6, NiRAM_MEM_BASE, MPU_RW | MPU_CACHE | MPU_64KB);
    ARM_MPU_Enable(MPU_DEFAULT_MAP);  // re-enable mpu
}

