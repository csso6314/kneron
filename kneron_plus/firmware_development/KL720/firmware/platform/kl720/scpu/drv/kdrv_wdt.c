/*
 * Kneron KL720 WDT driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */


#include <stdio.h>
#include "kdrv_wdt.h"
#include "kdrv_scu_ext.h"
#include "regbase.h"
#include "kdrv_cmsis_core.h"
#include "system_config.h"


typedef volatile struct kdrv_wdt_s {
    uint32_t    counter;    // 0x00 wdt timer counter 
    uint32_t    load;       // 0x04 wdt auto reload register
    uint32_t    restart;    // 0x08 wdt restart register
    uint32_t    cr;         // 0x0C wdt control register
    uint32_t    status;     // 0x10 wdt status register
    uint32_t    clear;      // 0x14 wdt time cleared register
    uint32_t    intrlen;    // 0x18 wdt intr length register
    uint32_t    revision;   // 0x1C revision
} kdrv_wdt_reg_t;

kdrv_wdt_reg_t volatile *wdt_reg = (kdrv_wdt_reg_t*) (WDT_REG_BASE);

/* kdrv_wdt_reg_t.clear, wdt status clear register */
#define WDT_CLEAR_EN                    BIT(0)  /* WDT clear bit, when write 1, WDT status will be cleared */
/* kdrv_wdt_reg_t.cr, wdt control register */
#define WDT_CR_EN                       BIT(0)  /* WDT enable bit, 0: disable, 1: enable */
#define WDT_CR_RST_EN                   BIT(1)  /* WDT reset bit, 0: disable, 1: enable */
#define WDT_CR_INT_EN                   BIT(2)  /* WDT int enable bit, 0: disable, 1: enable */
#define WDT_CR_EXT_EN                   BIT(3)  /* WDT extclk enable bit, 0:disable, 1:enable */
#define WDT_CR_EXTCLK                   BIT(4)  /* WDT clock source bit, 0:PCLK, 1:EXTCLK */

/* kdrv_wdt_reg_t.restart, wdt restart register */
#define WDT_RST_AUTO_RELOAD_KEY         0x5AB9


/* WDT_EXTCLK control */
#define SCU_EXT_WDT_EXTCLK_EN


/* to register user callback and argument */
static wdt_interrupt_callback_t _usr_wdt_isr_cb = 0;
static void *_usr_arg = 0;

/*
 * @brief register user callback and parameter.
 */
void kdrv_wdt_register_callback(wdt_interrupt_callback_t wdt_isr_cb, void *usr_arg)
{
    _usr_wdt_isr_cb = wdt_isr_cb;
    _usr_arg = usr_arg;
}

static void WDT_IRQ_Handler(void)
{
    if (_usr_wdt_isr_cb)
        _usr_wdt_isr_cb(_usr_arg);
    kdrv_wdt_set_clear_status();
    return;
}

/**
 * @brief kdr_wdt_initialize, initialize for WDT IRQ setting.
 */
void kdrv_wdt_initialize(void)
{
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_EnableIRQ(WDT_IRQn);
    NVIC_SetVector((IRQn_Type)WDT_IRQn, (uint32_t)WDT_IRQ_Handler);
}


/**
 * @brief kdr_wdt_uninitialize, uninitialize for WDT IRQ setting.
 */
void kdrv_wdt_uninitialize(void)
{
    NVIC_DisableIRQ(WDT_IRQn);
}

/* watchdog enable function */
void kdrv_wdt_enable(void)
{
    wdt_reg->cr |= WDT_CR_EN;
}


/* watchdog disable */
void kdrv_wdt_disable(void)
{
    wdt_reg->cr &= ~WDT_CR_EN;
}


/* force to load the WdLoad into WdCounter */
void kdrv_wdt_reset(void)
{
    wdt_reg->restart = WDT_RST_AUTO_RELOAD_KEY;
}

/* set auto reload value */
void kdrv_wdt_set_auto_reload(uint32_t value)
{
    wdt_reg->load = value;
}

/* clear watchdog status */
void kdrv_wdt_set_clear_status(void)
{
    wdt_reg->clear = 1;
}

/* set watchdog interrupt counter */
void kdrv_wdt_set_int_counter(uint8_t counter)
{
    wdt_reg->intrlen = counter;
}

/* set watchdog interrupt enable */
void kdrv_wdt_sys_int_enable(void)
{
    wdt_reg->cr |= (WDT_CR_INT_EN | WDT_CR_EN);
}

/* set watchdog interrupt disable */
void kdrv_wdt_sys_int_disable(void)
{
    wdt_reg->cr &= ~WDT_CR_INT_EN;

}

/* set watchdog reset enable */
void kdrv_wdt_sys_reset_enable(void)
{
    wdt_reg->cr |= WDT_CR_RST_EN;
}

/* set watchdog reset disable */
void kdrv_wdt_sys_reset_disable(void)
{
    wdt_reg->cr &= ~WDT_CR_RST_EN;
}

/* is watchdog zero? */
bool kdrv_wdt_is_counter_zero()
{
    if (wdt_reg->status == 1)
        return true;
    else
        return false;
}

/* read watchdog counter */
uint32_t kdrv_wdt_read_counter()
{
    return wdt_reg->counter;
}

/* set watchdog source clock */
void kdrv_wdt_set_src_clock(uint8_t src_clk)
{
    if (src_clk) {
        wdt_reg->cr |= WDT_CR_EXTCLK;
        regSCUEXT->clock_enable_reg0 |= CLK_REG0_WDT_EXT_CLK_EN;
    } else {
        wdt_reg->cr &= ~WDT_CR_EXTCLK;
        regSCUEXT->clock_enable_reg0 &= ~CLK_REG0_WDT_EXT_CLK_EN;
    }
}

void kdrv_wdt_set_extclk_div(uint8_t val)
{
    uint32_t tval;
    tval = regSCUEXT->clock_div4 & (~CLK_DIV4_WDT_DIV_FACTOR_MASK);
    regSCUEXT->clock_div4 = tval | ((val << CLK_DIV4_WDT_DIV_FACTOR_SHIFT) & CLK_DIV4_WDT_DIV_FACTOR_MASK);
}

void kdrv_wdt_irq_enable(void)
{ 
    NVIC_EnableIRQ(WDT_IRQn);
}

void kdrv_wdt_irq_disable(void)
{ 
    NVIC_DisableIRQ(WDT_IRQn);
}


/*
 * @brief kdrv_wdt_board_reset, wdt board reset immediately.
 */
void kdrv_wdt_board_reset(uint32_t rst_cnt)
{
    kdrv_wdt_disable();
    kdrv_wdt_set_int_counter(0xFF);
    kdrv_wdt_set_auto_reload(APB_CLOCK / 1000000 * rst_cnt);
    kdrv_wdt_reset();
    kdrv_wdt_enable();               //Enable WDT
    kdrv_wdt_sys_reset_enable();
}
