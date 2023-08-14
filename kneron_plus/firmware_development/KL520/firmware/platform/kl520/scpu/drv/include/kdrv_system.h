/**
 * @file        kdrv_system.h
 * @brief       Kneron system driver
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_SYSTEM_H__
#define __KDRV_SYSTEM_H__

#include <stdint.h>
#include "base.h"

/**
 * @brief Definition of index of FW
 */
#define SCPU_FW     1   /**< Index of SCPU FW */
#define NCPU_FW     2   /**< Index of NCPU FW */

/**
 * @brief Enumeration of system reset
 */
enum {
    SUBSYS_NPU      = 1,    /**< Software reset for NPU */
    SUBSYS_PD_NPU,          /**< Software reset for whole NPU domain */
    SUBSYS_LCDC,            /**< Software reset for LCDC */
    SUBSYS_NCPU,            /**< The signal controls SYSRESETn of NCPU */
};

/**
 * @brief           System initialize
 * @details         Turn on NPU/DDR power domain and enable some main clock PLL .\n
 */
void kdrv_system_init(void);

/**
 * @brief           NCPU system initialize
 * @details         Enable NCPU/NPU and some main PLL clock .\n
 * @note            This API should be called after @ref kdrv_system_init() to make sure NPU/DDR power domain is powered on.
 */
void kdrv_system_init_ncpu(void);

/**
 * @brief           System reset
 *
 * @param[in]       subsystem           subsystem reset id
 *
 * @note            SUBSYS_NPU:         reset NPU
 *                  SUBSYS_PD_NPU:      reset whole NPU domain(clk+ddr phy)
 *                  SUBSYS_LCDC:        reset LCDC
 *                  SUBSYS_NCPU:        reset NCPU
 */
void kdrv_system_reset(int32_t subsystem);

#endif
