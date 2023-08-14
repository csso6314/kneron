/**
 * @file        kdrv_usbd.h
 * @brief       Kneron WDT driver
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_WDT_H__
#define __KDRV_WDT_H__

#include "base.h"
#include "regbase.h"
#include "kdrv_status.h"

/** 
 * @brief wdt registers definition
 */
#define KDRV_WDT_BASE                   WDT_FTWDT010_PA_BASE    /**<  wdt timer base address*/        
#define REG_WDT_CNT                     (KDRV_WDT_BASE + 0x00)  /**<  wdt timer counter */
#define REG_WDT_LOAD                    (KDRV_WDT_BASE + 0x04)  /**<  auto reload register */
#define REG_WDT_RST                     (KDRV_WDT_BASE + 0x08)  /**<  restart register */
#define REG_WDT_CR                      (KDRV_WDT_BASE + 0x0C)  /**<  control register */
#define REG_WDT_STS                     (KDRV_WDT_BASE + 0x10)  /**<  wdt status register */
#define REG_WDT_CLR                     (KDRV_WDT_BASE + 0x14)  /**<  wdt time cleared register */
#define REG_WDT_INTR_LEN                (KDRV_WDT_BASE + 0x18)  /**<  wdt intr length register */
#define REG_WDT_REV                     (KDRV_WDT_BASE + 0x1C)  /**<  wdt revision */

/** 
 * @brief wdt control register
 */
#define WDT_CR_EN                        BIT(0)  /**< WDT enable bit, 0: disable, 1: enable */
#define WDT_CR_RST_EN                    BIT(1)  /**< WDT reset bit, 0: disable, 1: enable */
#define WDT_CR_INT_EN                    BIT(2)  /**< WDT int enable bit, 0: disable, 1: enable */
#define WDT_CR_EXT_EN                    BIT(3)  /**< WDT extclk enable bit, 0:disable, 1:enable */
#define WDT_CR_EXTCLK                    BIT(4)  /**< WDT clock source bit, 0:PCLK, 1:EXTCLK */


/**
 * @brief       watchdog enable
 */
void kdrv_wdt_enable(void);

/**
 * @brief       watchdog disable
 */
void kdrv_wdt_disable(void);

/**
 * @brief       watchdog reset
 */
void kdrv_wdt_reset(void);

/**
 * @brief       watchdog reload
 *
 * @param[in]   value watchdog reload value
 */
void kdrv_wdt_set_auto_reload(uint32_t value);

/**
 * @brief       watchdog interrupt enable
 */
void kdrv_wdt_sys_int_enable(void);


/**
 * @brief       watchdog interrupt disable
 */
void kdrv_wdt_sys_int_disable(void);

/**
 * @brief       watchdog reset enable
 */
void kdrv_wdt_sys_reset_enable(void);

/**
 * @brief       watchdog reset disable
 */
void kdrv_wdt_sys_reset_disable(void);


/**
 * @brief       watchdog read counter
 *
 * @return      counter value
 */
uint32_t kdrv_wdt_read_counter(void);

/**
 * @brief       watchdog status clear
 */
void kdrv_wdt_set_clear_status(void);

/**
 * @brief       watchdog set interrupt counter
 *
 * @param[in[   counter set the duration of assertion of wd_intr, the default value is 0xFF.\n
 *              which means that the default assertion duration is 256 clock cycles(PCLK)
 */
void kdrv_wdt_set_int_counter(uint8_t counter);

/**
 * @brief       watchdog, is counter zero
 *
 * @return      bool
 */
bool kdrv_wdt_is_counter_zero(void);

#endif // __KDRV_WDT_H__
