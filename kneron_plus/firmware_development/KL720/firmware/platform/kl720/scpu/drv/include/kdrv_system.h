/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/

/**@addtogroup  KDRV_SYSTEM     KDRV_SYSTEM
 * @{
 * @brief       Kneron system driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KDRV_SYSTEM_H__
#define __KDRV_SYSTEM_H__

#include <stdint.h>
#include "base.h"
#include "kdrv_power.h"
#include "kdrv_clock.h"


#define FLAGS_SOURCE_READY_EVT     0x91ad

#define SCPU_FW     1
#define NCPU_FW     2

/** @brief Enumeration of system reset*/
enum {
    SUBSYS_NPU      = 1,    /**< Enum 1, Software reset for NPU */
    SUBSYS_PD_NPU,          /**< Enum 2, Software reset for whole NPU domain */
    SUBSYS_LCDC,            /**< Enum 3, Software reset for LCDC */
    SUBSYS_NCPU,            /**< Enum 4, The signal controls SYSRESETn of NCPU */
    GLOBAL_RESET,           /**< Enum 5 */
};

/**
 * @brief           System initialize
 * @details         Turn on NPU/DDR power domain and enable some main clock PLL .
 *
 * @return          N/A
 */
void kdrv_system_initialize(kdrv_power_mode_t power_mode, uint32_t wakeup_src, sysclockopt *sysclk_opt);

/**
 * @brief           NCPU system initialize
 * @details         Enable NCPU/NPU and some main PLL clock .\n
 *
 * @return          N/A
 * @note            This API should be called after @ref kdrv_system_initialize() to make sure NPU/DDR power domain is powered on.
 */
void kdrv_system_init_ncpu(void);

/**
 * @brief           System reset
 *
 * @param[in]       subsystem           subsystem reset id
 *
 * @note            SUBSYS_NPU:         reset NPU\n
 *                  SUBSYS_PD_NPU:      reset whole NPU domain(clk+ddr phy)\n
 *                  SUBSYS_LCDC:        reset LCDC\n
 *                  SUBSYS_NCPU:        reset NCPU
 * @return          N/A
 */
void kdrv_system_reset(int32_t subsystem);

#endif
/** @}*/

