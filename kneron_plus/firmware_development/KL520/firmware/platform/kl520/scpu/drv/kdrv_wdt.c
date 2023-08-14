/*
 * Kneron KL520 WDT driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include "io.h"
#include "kdrv_wdt.h"

/* REG_WDT_RST, wdt restart register */
#define WDT_RST_AUTO_RELOAD_KEY        0x5AB9


/* watchdog enable function */
void kdrv_wdt_enable(void)
{
    outw(REG_WDT_CR, inw(REG_WDT_CR) | WDT_CR_EN);
}


/* watchdog disable */
void kdrv_wdt_disable(void)
{
    outw(REG_WDT_CR, inw(REG_WDT_CR) & ~WDT_CR_EN);
}


/* force to load the WdLoad into WdCounter */
void kdrv_wdt_reset(void)
{
    outw(REG_WDT_RST, WDT_RST_AUTO_RELOAD_KEY);
}

/* set auto reload value */
void kdrv_wdt_set_auto_reload(uint32_t value)
{
    outw(REG_WDT_LOAD, value);
}

/* clear watchdog status */
void kdrv_wdt_set_clear_status(void)
{
    outw(REG_WDT_CLR, 1);
}

/* set watchdog interrupt counter */
void kdrv_wdt_set_int_counter(uint8_t counter)
{
    outw(REG_WDT_INTR_LEN, counter);
}

/* set watchdog interrupt enable */
void kdrv_wdt_sys_int_enable(void)
{
    //outw(REG_WDT_CR, WDT_CR_INT_EN | WDT_CR_EN);
    outw(REG_WDT_CR, inw(REG_WDT_CR) | WDT_CR_INT_EN);
}

/* set watchdog interrupt disable */
void kdrv_wdt_sys_int_disable(void)
{

    outw(REG_WDT_CR, inw(REG_WDT_CR) & ~WDT_CR_INT_EN);
}

/* set watchdog reset enable */
void kdrv_wdt_sys_reset_enable(void)
{
    outw(REG_WDT_CR, inw(REG_WDT_CR) | WDT_CR_RST_EN);
}

/* set watchdog reset disable */
void kdrv_wdt_sys_reset_disable(void)
{
    outw(REG_WDT_CR, inw(REG_WDT_CR) & ~WDT_CR_RST_EN);

}

/* is watchdog zero? */
bool kdrv_wdt_is_counter_zero()
{
    uint32_t tmp;

    tmp = inw(REG_WDT_STS);
    if (tmp == 1)
        return true;
    else
        return false;
}

/* read watchdog counter */
uint32_t kdrv_wdt_read_counter()
{
    return inw(REG_WDT_CNT);
}

