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

/**@addtogroup  KDRV_POWER  KDRV_POWER
 * @{
 * @brief       Kneron power driver
 *
 * @details     kl720 is a system with one NPU, two CPUs, and peripherals.\n
 *              One CPU handles system requests such as host communication, camera video and display, and peripherals.\n
 *              Another CPU assists NPU to do works like input image preprocessing and postprocessing.\n
 *              Two CPUs use shared memory and interrupt for their communication (IPC).\n\n
 *
 *              Upon power-on, default power domain will be turned on and the initial bootloader code (IPL) in ROM starts to run on SCPU.\n
 *              Once the secondary bootloader (SPL) is loaded to system internal SRAM by IPL, SPL will load SCPU OS and NCPU OS in SRAM and\n
 *              pass over the execution SCPU OS.\n
 *              Once NCPU OS is started (by SCPU OS), it will stay in idle thread and listen to commands from SCPU.\n
 *              SCPU will also stay in idle thread and listen to host commands in companion mode or listen to user commands in standalone mode.
 *
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KDRV_POWER_H__
#define __KDRV_POWER_H__

#include <base.h>
#include "kdrv_status.h"

/** @brief Enumerations of kl720 power domains */
typedef enum {
    POWER_DOMAIN_BAS = 1,       /**< Enum 1, Power to BAS power domain triggered by wake-up events */
    POWER_DOMAIN_NOM,           /**< Enum 2, Power to NOR power domain controlled by software */
    POWER_DOMAIN_MRX,           /**< Enum 3, Power to MRX power domain controlled by software */
    POWER_DOMAIN_UHO,           /**< Enum 4, Power to UHO power domain controlled by software */
    POWER_DOMAIN_NPU,           /**< Enum 5, Power to NPU power domain controlled by software */
    POWER_DOMAIN_UDR,           /**< Enum 6, Power to UDR power domain controlled by software */
} kdrv_power_domain_t;

/** @brief Enumerations of kl720 power operations */
typedef enum {
    POWER_OPS_FCS = 0,           /**< Enum 0 */
    POWER_OPS_CHANGE_BUS_SPEED, /**< Enum 1 */
} kdrv_power_ops_t;

/** @brief Enumerations of kl720 power modes */
typedef enum {
    POWER_MODE_FULLOFF = 0,     /**< Enum 0 */
    POWER_MODE_AUX_PWR_ON,      /**< Enum 1 */
    POWER_MODE_BASE,            /**< Enum 2 */
    POWER_MODE_NOR,             /**< Enum 3 */
    POWER_MODE_IMG_DETECT,      /**< Enum 4 */
    POWER_MODE_UVC,             /**< Enum 5 */
    POWER_MODE_AI_TEST,         /**< Enum 6 */
    POWER_MODE_USB_DEVICE,      /**< Enum 7 */
    POWER_MODE_AI_RUNING,       /**< Enum 8 */
    POWER_MODE_USB_AI,          /**< Enum 9 */
    POWER_MODE_UVC_AI,          /**< Enum 10 */
    POWER_MODE_UVC_AI_PASS,     /**< Enum 11 */
    POWER_MODE_ALL_AXI_ON,      /**< Enum 12 */
    POWER_MODE_DORM_USB_S,      /**< Enum 13 */
    POWER_MODE_SNOZ_USB_S,      /**< Enum 14 */
    POWER_MODE_DORM,            /**< Enum 15 */
    POWER_MODE_SNOZ,            /**< Enum 16 */
    POWER_MODE_MAX              /**< Enum 17 */
} kdrv_power_mode_t;

/**
 * @brief       Watchdog reset
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_power_sw_reset(void);

/**
 * @brief       set wake-up source
 *
 * @param[in]   wakeup_src_
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_set_wakeup_src(uint32_t wakeup_src_);
/**
 * @brief       Power operation
 *
 * @param[in]   ops             see @ref kdrv_power_ops_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_ops(kdrv_power_ops_t ops);

/**
 * @brief       Set power domain
 * @details     There are three powe domain in Kneron kl720 chip, see @ref kdrv_power_domain_t
 *
 * @param[in]   domain          see @ref kdrv_power_domain_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_set_domain(uint32_t domain);

/**
 * @brief       Set power mode
 *
 * @param[in]   next_pm             see @ref kdrv_power_mode_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_set_powermode(kdrv_power_mode_t next_pm);
/**
 * @brief       Shutdown the power supply to all blocks, except the logic in the RTC domain and DDR memory is in self-refresh state.
 *
 * @param[in]   mode            see @ref kdrv_power_mode_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_softoff(kdrv_power_mode_t pm);

/**
 * @brief       Set power mode into sleep
 *
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_power_sleep(void);
#endif
/** @}*/
