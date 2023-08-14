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

#include <stdlib.h>
#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "system_config.h"
#include "kdrv_gpio.h"

/* GPIO registers offset */

typedef volatile union {
    struct
    {
        uint32_t DOUT_OFFSET;      // 0x0
        uint32_t DIN_OFFSET;       // 0x4
        uint32_t PINOUT_OFFSET;    // 0x8
        uint32_t PIN_BYPASS;       // 0xC
        uint32_t DATASET;          // 0x10
        uint32_t DATACLR;          // 0x14
        uint32_t PULLENABLE;       // 0x18
        uint32_t PULLTYPE;         // 0x1C
        uint32_t INT_ENABLE;       // 0x20
        uint32_t INT_RAWSTATE;     // 0x24
        uint32_t INT_MASKSTATE;    // 0x28
        uint32_t INT_MASK;         // 0x2C
        uint32_t INT_CLEAR;        // 0x30
        uint32_t INT_TRIGGER;      // 0x34
        uint32_t INT_BOTH;         // 0x38
        uint32_t INT_RISENEG;      // 0x3C
        uint32_t INT_BOUNCEENABLE; // 0x40
        uint32_t INT_PRESCALE;     // 0x44
    } dw;                          //double word
} U_regGPIO;

// GPIO register base pointer
#define regGPIO ((U_regGPIO *)GPIO_FTGPIO010_PA_BASE)

// to record user callback and argument
static gpio_interrupt_callback_t _usr_isr_cb = 0;
static void *_usr_arg = 0;

// GPIO controller to NVIC ISR
static void gpio_isr(void)
{
    // read GPIO interrupt pins with masked status
    uint32_t intr_status = regGPIO->dw.INT_MASKSTATE;

    // clear the interrupt source coming from GPIO peripheral
    regGPIO->dw.INT_CLEAR = intr_status;

    if (!_usr_isr_cb)
        return;

    for (int i = 0; i < 32; i++)
    {
        if (intr_status & (0x1 << i))
        {
            _usr_isr_cb((kdrv_gpio_pin_t)i, _usr_arg);
        }
    }
}

kdrv_status_t kdrv_gpio_initialize()
{
    // FIXME: enable gpio pclk (gpio_pclk[20])
    uint32_t apb_clock_reg = (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x60));
    apb_clock_reg |= (0x1 << 20);
    (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x60)) = apb_clock_reg;

    NVIC_SetVector(GPIO_FTGPIO010_IRQ, (uint32_t)gpio_isr);

    // Clear and Enable SAI IRQ
    NVIC_ClearPendingIRQ(GPIO_FTGPIO010_IRQ);
    NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_uninitialize()
{
    NVIC_DisableIRQ(GPIO_FTGPIO010_IRQ);

    // FIXME: disable gpio pclk (gpio_pclk[20])
    uint32_t apb_clock_reg = (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x60));
    apb_clock_reg &= ~(0x1 << 20);
    (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x60)) = apb_clock_reg;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_set_attribute(kdrv_gpio_pin_t pin, uint32_t attributes)
{
    uint32_t pin_val = (1 << pin);

    // disable interrup in case of wrong conditions
    regGPIO->dw.INT_ENABLE &= ~pin_val;
    regGPIO->dw.INT_MASK |= pin_val;

    // set pin direction
    if (attributes & GPIO_DIR_INPUT)
        regGPIO->dw.PINOUT_OFFSET &= ~pin_val;
    else // GPIO_DIR_OUTPUT
        regGPIO->dw.PINOUT_OFFSET |= pin_val;

    uint32_t edge_attributes =
        (GPIO_INT_EDGE_RISING | GPIO_INT_EDGE_FALLING | GPIO_INT_EDGE_BOTH);

    if (attributes & edge_attributes)
    {
        // edge trigger
        regGPIO->dw.INT_TRIGGER &= ~pin_val;

        // set both or single edge
        if (attributes & GPIO_INT_EDGE_BOTH)
            regGPIO->dw.INT_BOTH |= pin_val;
        else
            regGPIO->dw.INT_BOTH &= ~pin_val;

        // set rising or falling edge
        if (attributes & GPIO_INT_EDGE_RISING)
            regGPIO->dw.INT_RISENEG &= ~pin_val;
        else
            regGPIO->dw.INT_RISENEG |= pin_val;
    }

    uint32_t level_attributes =
        (GPIO_INT_LEVEL_HIGH | GPIO_INT_LEVEL_LOW);

    if (attributes & level_attributes)
    {
        // level trigger
        regGPIO->dw.INT_TRIGGER |= pin_val;

        // set high or low level
        if (attributes & GPIO_INT_LEVEL_HIGH)
            regGPIO->dw.INT_RISENEG &= ~pin_val;
        else
            regGPIO->dw.INT_RISENEG |= pin_val;
    }

    // clear pull-enable here, should use IO control register to set pull-up or pull-down
    regGPIO->dw.PULLENABLE &= ~pin_val;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_register_callback(gpio_interrupt_callback_t gpio_isr_cb, void *usr_arg)
{
    _usr_isr_cb = gpio_isr_cb;
    _usr_arg = usr_arg;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_set_interrupt(kdrv_gpio_pin_t pin, bool isEnable)
{
    uint32_t pin_val = (1 << pin);

    if (isEnable)
    {
        regGPIO->dw.INT_CLEAR |= pin_val;
        regGPIO->dw.INT_ENABLE |= pin_val;
        regGPIO->dw.INT_MASK &= ~pin_val;
    }
    else
    {
        regGPIO->dw.INT_ENABLE &= ~pin_val;
        regGPIO->dw.INT_MASK |= pin_val;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_set_debounce(kdrv_gpio_pin_t pin, bool isEnable, uint32_t debounce_clock)
{
    uint32_t pin_val = (1 << pin);

    if (isEnable)
    {
        regGPIO->dw.INT_BOUNCEENABLE |= pin_val;
        regGPIO->dw.INT_PRESCALE = (APB_CLOCK / debounce_clock);
    }
    else
        regGPIO->dw.INT_BOUNCEENABLE &= ~pin_val;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_write_pin(kdrv_gpio_pin_t pin, bool value)
{
    uint32_t pin_val = (1 << pin);

    if (value == true)
        regGPIO->dw.DATASET = pin_val;
    else
        regGPIO->dw.DATACLR = pin_val;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_read_pin(kdrv_gpio_pin_t pin, bool *pValue)
{
    uint32_t all_bits = regGPIO->dw.DIN_OFFSET;

    *pValue = (all_bits & (0x1 << pin)) ? true : false;

    return KDRV_STATUS_OK;
}
