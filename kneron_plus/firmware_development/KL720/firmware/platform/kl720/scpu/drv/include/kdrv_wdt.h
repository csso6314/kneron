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

/**@addtogroup  KDRV_WDT  KDRV_WDT
 * @{
 * @brief       Kneron WDT device mode driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_WDT_H__
#define __KDRV_WDT_H__

#include "base.h"
#include "kdrv_status.h"


/** 
 * @brief WDT user callback function. Note that this is callback form ISR context. 
 */
typedef void (*wdt_interrupt_callback_t)(void *arg);

/**
 * @brief       watchdog initialize for WDT IRQ setting.
 *
 * @return      N/A
 */
void kdrv_wdt_initialize(void);

/**
 * @brief       watchdog uninitialize for WDT IRQ setting.
 *
 * @return      N/A
 */
void kdrv_wdt_uninitialize(void);

/**
 * @brief       set watchdog callback function.
 *
 * @param[out]  wdt_isr_cb      callback function pointer
 * @param[in]   usr_arg         parameter
 */
void kdrv_wdt_register_callback(wdt_interrupt_callback_t wdt_isr_cb, void *usr_arg);

/**
 * @brief       watchdog enable
 *
 * @return      N/A
 */
void kdrv_wdt_enable(void);

/**
 * @brief       watchdog disable
 *
 * @return      N/A
 */
void kdrv_wdt_disable(void);

/**
 * @brief       watchdog reset, It will set protect key 0x5AB9 to trigger WDT system reset
 *
 * @return      N/A
 */
void kdrv_wdt_reset(void);

/**
 * @brief       watchdog reload
 *
 * @param[in]   value watchdog reload value
 * @return      N/A
 */
void kdrv_wdt_set_auto_reload(uint32_t value);

/**
 * @brief       watchdog interrupt enable
 *
 * @return      N/A
 */
void kdrv_wdt_sys_int_enable(void);


/**
 * @brief       watchdog interrupt disable 
 *
 * @return      N/A
 */
void kdrv_wdt_sys_int_disable(void);


/**
 * @brief       watchdog reset enable
 *
 * @return      N/A
 */
void kdrv_wdt_sys_reset_enable(void);

/**
 * @brief       watchdog reset disable
 *
 * @return      N/A
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
 *
 * @return      N/A
 */
void kdrv_wdt_set_clear_status(void);

/**
 * @brief       watchdog set interrupt counter
 *
 * @param[in[   counter     set the duration of assertion of wd_intr, the default value is 0xFF.
 *                          which means that the default assertion duration is 256 clock cycles(PCLK)
 * @return      N/A
 */
void kdrv_wdt_set_int_counter(uint8_t counter);

/**
 * @brief       watchdog, is counter zero
 *
 * @return      bool
 */
bool kdrv_wdt_is_counter_zero(void);

/**
 * @brief       set watchdog source clock
 *
 * @param[in]   src_clk 0: PCLK, 1:EXTCLK
 * @return      N/A
 */
void kdrv_wdt_set_src_clock(uint8_t src_clk);

/**
 * @brief       set watchdog external clock divider
 *
 * @param[in]   val         external divider value, default:0x1D (max:0x1F) 
 * @return      N/A
 */
void kdrv_wdt_set_extclk_div(uint8_t val);

/**
 * @brief      set watchdog irq enable 
 *
 * @return     N/A
 */
void kdrv_wdt_irq_enable(void);


/**
 * @brief      set watchdog irq disable 
 *
 * @return     N/A
 */
void kdrv_wdt_irq_disable(void);


/**
 * @brief kdrv_wdt_board_reset, wdt board reset immediately.
 *
 * @param[in]   rst_time        reset delay time (us)
 * @return      N/A
 */
void kdrv_wdt_board_reset(uint32_t rst_cnt);

#endif // __KDRV_WDT_H__
/** @}*/

