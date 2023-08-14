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
#include "kneron_kl720.h"
#include "kdrv_gpio.h"
#include "kdrv_scu.h"
#include "kdrv_cmsis_core.h"

/* GPIO registers offset */

typedef volatile union {
    struct
    {
        uint32_t DOUT_OFFSET;      // 0x00  GPIO data output register
        uint32_t DIN_OFFSET;       // 0x04  GPIO data input register
        uint32_t PINOUT_OFFSET;    // 0x08  GPIO direction register 0: input    1: output
        uint32_t PIN_BYPASS;       // 0x0C  GPIO bypass register 0: no bypass   1: bypass
        uint32_t DATASET;          // 0x10  GPIO data bit set register
        uint32_t DATACLR;          // 0x14  GPIO data bit clear register
        uint32_t PULLENABLE;       // 0x18  GPIO pull up register (ref pinmux)
        uint32_t PULLTYPE;         // 0x1C  GPIO pull-high/pull-low register (ref pinmux)
        uint32_t INT_ENABLE;       // 0x20  GPIO interupt enable register
        uint32_t INT_RAWSTATE;     // 0x24  GPIO interupt raw status register
        uint32_t INT_MASKSTATE;    // 0x28  GPIO interrupt masked status register
        uint32_t INT_MASK;         // 0x2C  GPIO interupt mask register 0: disabled 1:enabled
        uint32_t INT_CLEAR;        // 0x30  GPIO interrupt clear 0: no effect 1: clear interrupt
        uint32_t INT_TRIGGER;      // 0x34  GPIO interrupt trigger method register 0: edge 1: level
        uint32_t INT_BOTH;         // 0x38  GPIO edge-trigger interrupt by 0: single edge 1: both edges
        uint32_t INT_RISENEG;      // 0x3C  GPIO interrupt triggerd at the 0: rising edge/High level, 1: Flling edge/Low level
        uint32_t INT_BOUNCEENABLE; // 0x40  GPIO pre-scale clock enable
        uint32_t INT_PRESCALE;     // 0x44  GPIO pre-scale
    } dw;                          //double word
} U_regGPIO;
// GPIO register base pointer
#define regGPIO ((U_regGPIO *)GPIO_REG_BASE)

// to record user callback and argument
static gpio_interrupt_callback_t _usr_isr_cb = 0;
static void *_usr_arg = 0;

// GPIO controller to NVIC ISR
void GPIO_IRQ_Handler(void)
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

kdrv_status_t kdrv_gpio_initialize(uint32_t num ,gpio_attr_context *gpio_attr_ctx)
{
    regSCU->bf.gpio_pclk = 1;
    NVIC_ClearPendingIRQ(GPIO_IRQn);
    NVIC_EnableIRQ(GPIO_IRQn);
    for(uint32_t i = 0 ; i < num ; i++)
    {
        kdrv_gpio_set_attribute((kdrv_gpio_pin_t) gpio_attr_ctx[i].gpio_pin, gpio_attr_ctx[i].gpio_attr);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gpio_uninitialize()
{
    NVIC_DisableIRQ(GPIO_IRQn);
    regSCU->bf.gpio_pclk = 0;
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

    //set output pin default level.
    if (attributes & GPIO_DIR_OUTPUT)
    {
        if ((attributes & GPIO_OUT_HIGH) == GPIO_OUT_HIGH)
        {
            regGPIO->dw.DATASET = pin_val;
        }
        else if ((attributes & GPIO_OUT_LOW) == GPIO_OUT_LOW)
        {
            regGPIO->dw.DATACLR = pin_val;
        }
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

