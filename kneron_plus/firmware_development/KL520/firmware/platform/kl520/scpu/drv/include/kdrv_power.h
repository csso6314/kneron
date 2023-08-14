/**
 * @file        kdrv_power.h
 * @brief       Kneron power driver
 * @details     KL520 is a system with one NPU, two CPUs, and peripherals.\n
 *              One CPU handles system requests such as host communication, camera video and display, and peripherals.\n
 *              Another CPU assists NPU to do works like input image preprocessing and postprocessing.\n
 *              Two CPUs use shared memory and interrupt for their communication (IPC).\n\n
 *              Upon power-on, default power domain will be turned on and the initial bootloader code (IPL) in ROM starts to run on SCPU.\n
 *              Once the secondary bootloader (SPL) is loaded to system internal SRAM by IPL, SPL will load SCPU OS and NCPU OS in SRAM and\n
 *              pass over the execution SCPU OS.\n
 *              Once NCPU OS is started (by SCPU OS), it will stay in idle thread and listen to commands from SCPU.\n
 *              SCPU will also stay in idle thread and listen to host commands in companion mode or listen to user commands in standalone mode.
 *
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_POWER_H__
#define __KDRV_POWER_H__

#include <base.h>
#include "kdrv_status.h"

/**
 * @brief Enumerations of kl520 power domains
 */ 
typedef enum {
    POWER_DOMAIN_DEFAULT = 1,   /**< Enum 1, Power to Default power domain triggered by wake-up events */
    POWER_DOMAIN_NPU,           /**< Enum 2, Power to NPU power domain controlled by software */
    POWER_DOMAIN_DDRCK          /**< Enum 3, Power to DDRCK power domain controlled by software */
} kdrv_power_domain_t;

/**
 * @brief Enumerations of kl520 power operations
 */ 
typedef enum {
    POWER_OPS_FCS = 0,          /**< Enum 0 */
    POWER_OPS_CHANGE_BUS_SPEED, /**< Enum 1 */
    POWER_OPS_PLL_UPDATE,       /**< Enum 2 */
    POWER_OPS_SLEEPING          /**< Enum 3 */    
} kdrv_power_ops_t;

/**
 * @brief Enumerations of kl520 power modes
 */ 
typedef enum {
    POWER_MODE_RTC = 0,          /**< Enum 0, RTC */
    POWER_MODE_ALWAYSON,         /**< Enum 1, RTC + Default */
    POWER_MODE_FULL,             /**< Enum 2, RTC + Default + DDR + NPU */
    POWER_MODE_RETENTION,        /**< Enum 3, RTC + Default + DDR(Self-refresh) */
    POWER_MODE_DEEP_RETENTION    /**< Enum 4, RTC + DDR(Self-refresh) */
} kdrv_power_mode_t;

/**
 * @brief       Watchdog reset
 */
void kdrv_power_sw_reset(void);

/**
 * @brief       Power operation
 *
 * @param[in]   ops             see @ref kdrv_power_ops_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_ops(kdrv_power_ops_t ops);

/**
 * @brief       Set power domain
 * @details     There are three powe domain in Kneron kl520 chip, see @ref kdrv_power_domain_t
 *
 * @param[in]   domain          see @ref kdrv_power_domain_t
 * @param[in]   enable          Enable the power domain
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_set_domain(kdrv_power_domain_t domain, int enable);

/**
 * @brief       Shutdown the power supply to all blocks, except the logic in the RTC domain and DDR memory is in self-refresh state.
 *
 * @param[in]   mode            see @ref kdrv_power_mode_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_softoff(kdrv_power_mode_t mode);

#endif

