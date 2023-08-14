/**
 * @file        kmdw_memory.h
 * @brief       ddr memory access APIs
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_SYSTEM_H__
#define __KMDW_SYSTEM_H__

#include "base.h"

/**
 * @brief           Wakeup ncpu
 * 
 * @param[in]       boot_loader_flag        bootloader flag, 0: none, 1: add 200ms delay
 * @param[in]       wakeup_all              wakeup_all, 0: none, 1: wake up all ncpu clock 
 * @return          N/A
 */
void system_wakeup_ncpu(int32_t boot_loader_flag, uint8_t wakeup_all);


/**
 * @brief           Reload ncpu firmware from flash
 *
 * @return          N/A
 */
void reload_ncpu_fw(void);


/**
 * @brief           System initialize
 *
 * @param[in]       reset_flag          = 0, just launch ncpu
 *                                      <>0, load and launch ncpu
 *                                      < 0, not using mpu
 * @return          kdrv_status_t       0: KDRV_STATUS_OK, otherwise: wrong checksum
 */
uint32_t load_ncpu_fw(int32_t reset_flag);


/**
 * @brief           check fw image via sum32 method
 * @details         Calculate sum32 of scpu/ncpu fw and compare to the value which
 *                  resides at the end of flash
 *
 * @param[in]       fw_type             SCPU_FW, NCPU_FW
 * @return          kdrv_status_t       0: KDRV_STATUS_OK, otherwise: wrong calculated value
 */
uint32_t system_check_fw_image(int32_t fw_type);

#endif
