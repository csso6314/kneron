/**
 * @file        kmdw_led.h
 * @brief       LED APIs
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_LED_H_
#define __KMDW_LED_H_

#include <stdbool.h>
#include "kdrv_gpio.h"
#include "kmdw_status.h"

/**
 * @brief enum for LED status
 */
typedef enum {
    KMDW_LED_OFF = 0,     /**< LED off */
    KMDW_LED_ON           /**< LED on */
} kmdw_led_status_e;

/**
 * @brief       Get the initialized GPIO bits mask
 *
 * @param[out]  uint32_t initialized GPIO bits mask
 */
uint32_t kmdw_led_get_init_mask(void);

/**
 * @brief       Via test the specific GPIO output level to check whether LED is ON of OFF
 *
 * @param[in]   gpio            see @ref kdrv_gpio_pin_t
 * @param[in]   on              boolean value to LED status
 * @param[out]  kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_led_test(kdrv_gpio_pin_t gpio, kmdw_led_status_e *on);

/**
 * @brief       Turn on/off the LED via set the corresponding GPIO output level to LOW
 *
 * @param[in]   gpio            see @ref kdrv_gpio_pin_t
 * @param[in]   on              boolean value for on/off LED
 * @param[out]  kmdw_status_t   see @ref kmdw_status_t
 */
kmdw_status_t kmdw_led_set(kdrv_gpio_pin_t gpio, kmdw_led_status_e on);

/**
 * @brief       Initialize LED
 * @details     The dafault GPIO output level is HIGH for the initialized LED
 * @param[in]   led_mask        led bits mask
 * @Example:    Initialize GPIO_10 and GPIO_11 for 2 LED bulbs\n
                kmdw_led_init((1UL << GPIO_PIN_10) | )(1UL << GPIO_PIN_11);
 */
void kmdw_led_init(uint32_t led_mask);

#endif
