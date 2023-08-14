/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */
#include <stdbool.h>
#include "kmdw_led.h"

static uint32_t init_led_mask = 0;

#define LED_GPIO_OUTPUT_LOW      false
#define LED_GPIO_OUTPUT_HIGH     true

uint32_t kmdw_led_get_init_mask(void)
{
    return init_led_mask;
}

kmdw_status_t kmdw_led_test(kdrv_gpio_pin_t gpio, kmdw_led_status_e *on)
{
    if(!(init_led_mask & (1UL << gpio)))
        return KMDW_STATUS_ERROR;

    kdrv_gpio_read_pin(gpio, (bool *)on);
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_led_set(kdrv_gpio_pin_t gpio, kmdw_led_status_e on)
{
    if(!(init_led_mask & (1UL << gpio)))
        return KMDW_STATUS_ERROR;

    if(on)
        kdrv_gpio_write_pin(gpio, LED_GPIO_OUTPUT_LOW);
    else
        kdrv_gpio_write_pin(gpio, LED_GPIO_OUTPUT_HIGH);
		
    return KMDW_STATUS_OK;
}

void kmdw_led_init(uint32_t led_mask)
{
    init_led_mask = led_mask;
    for(kdrv_gpio_pin_t i = GPIO_PIN_0; i <= GPIO_PIN_31; i++) {
        if((1UL << i) & led_mask) {
            kdrv_gpio_set_attribute(i, GPIO_DIR_OUTPUT);
            kdrv_gpio_write_pin(i, LED_GPIO_OUTPUT_HIGH);
        }
    }
}

