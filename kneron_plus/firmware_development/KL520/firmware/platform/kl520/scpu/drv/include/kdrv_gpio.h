/**
  * @file        kdrv_gpio.h
  * @brief       Kneron GPIO driver
  * @version     1.0
  * @copyright   (c) 2020 Kneron Inc. All right reserved.
  */
#ifndef __KDRV_GPIO_H__
#define __KDRV_GPIO_H__

#include <cmsis_os2.h>
#include "kdrv_status.h"

/** 
 * @brief Enumerations of GPIO pin attributes, input or output, interrupt trigger settings
 */
typedef enum
{
    GPIO_DIR_INPUT = 0x1,        /**< pin direction digital input*/
    GPIO_DIR_OUTPUT = 0x2,       /**< pin direction digital output*/
    GPIO_INT_EDGE_RISING = 0x4,  /**< indicate pin interrupt triggered when at rising edge */
    GPIO_INT_EDGE_FALLING = 0x8, /**< indicate pin interrupt triggered when at falling edge */
    GPIO_INT_EDGE_BOTH = 0x10,   /**< indicate pin interrupt triggered when at both edge rsising or falling */
    GPIO_INT_LEVEL_HIGH = 0x20,  /**< indicate pin interrupt triggered when at high voltage level */
    GPIO_INT_LEVEL_LOW = 0x40,   /**< indicate pin interrupt triggered when at low voltage level */
} kdrv_gpio_attribute_t;

/** 
 * @brief Enumerations of GPIO pin ID, there 32 GPIO pins
 */
typedef enum
{
    GPIO_PIN_0 = 0, /**<  GPIO pin ID 0 */
    GPIO_PIN_1,     /**<  GPIO pin ID 1 */
    GPIO_PIN_2,     /**<  GPIO pin ID 2 */
    GPIO_PIN_3,     /**<  GPIO pin ID 3 */
    GPIO_PIN_4,     /**<  GPIO pin ID 4 */
    GPIO_PIN_5,     /**<  GPIO pin ID 5 */
    GPIO_PIN_6,     /**<  GPIO pin ID 6 */
    GPIO_PIN_7,     /**<  GPIO pin ID 7 */
    GPIO_PIN_8,     /**<  GPIO pin ID 8 */
    GPIO_PIN_9,     /**<  GPIO pin ID 9 */
    GPIO_PIN_10,    /**<  GPIO pin ID 10 */
    GPIO_PIN_11,    /**<  GPIO pin ID 11 */
    GPIO_PIN_12,    /**<  GPIO pin ID 12 */
    GPIO_PIN_13,    /**<  GPIO pin ID 13 */
    GPIO_PIN_14,    /**<  GPIO pin ID 14 */
    GPIO_PIN_15,    /**<  GPIO pin ID 15 */
    GPIO_PIN_16,    /**<  GPIO pin ID 16 */
    GPIO_PIN_17,    /**<  GPIO pin ID 17 */
    GPIO_PIN_18,    /**<  GPIO pin ID 18 */
    GPIO_PIN_19,    /**<  GPIO pin ID 19 */
    GPIO_PIN_20,    /**<  GPIO pin ID 20 */
    GPIO_PIN_21,    /**<  GPIO pin ID 21 */
    GPIO_PIN_22,    /**<  GPIO pin ID 22 */
    GPIO_PIN_23,    /**<  GPIO pin ID 23 */
    GPIO_PIN_24,    /**<  GPIO pin ID 24 */
    GPIO_PIN_25,    /**<  GPIO pin ID 25 */
    GPIO_PIN_26,    /**<  GPIO pin ID 26 */
    GPIO_PIN_27,    /**<  GPIO pin ID 27 */
    GPIO_PIN_28,    /**<  GPIO pin ID 28 */
    GPIO_PIN_29,    /**<  GPIO pin ID 29 */
    GPIO_PIN_30,    /**<  GPIO pin ID 30 */
    GPIO_PIN_31     /**<  GPIO pin ID 31 */
} kdrv_gpio_pin_t;

/** 
 * @brief GPIO user callback function with specified GPIO pin. 
 * @note  This is callback form ISR context. 
 */
typedef void (*gpio_interrupt_callback_t)(kdrv_gpio_pin_t pin, void *arg);

/**
 * @brief           GPIO driver initialization, this must be invoked once before any GPIO manipulations
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_initialize(void);

/**
 * @brief           GPIO driver uninitialization
 *
 * @details         This function disables the corresponding clock and frees resources allocated for GPIO operations. 
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_uninitialize(void);

/**
 * @brief           set pin attributes for a specified GPIO pin
 *
 * @details         it must be well set up before GPIO pin to be used.
 *
 * @param[in]       pin                   After configuring the desired pin as a GPIO pin, the corresponding GPIO pin name should be used as kdp_gpio_pin_e indicated
 * @param[in]       attributes            This is to specify the function of specified GPIO pin,\n
 *                                        for digital output, set only DIR_OUTPUT,\n
 *                                        for digital input for read, set only DIR_INPUT,\n
 *                                        for interrupt usage, set DIR_INPUT and one of EDGE or LEVEL trigger attributes, this implies pin is used as an interrupt input\n
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_set_attribute(
    kdrv_gpio_pin_t pin,
    uint32_t attributes);

/**
 * @brief           register user callback with user argument for GPIO interrupt in this callback can get interrupts for all GPIO pins
 *
 * @param[in]       gpio_isr_cb           user callback function for GPIO interrupts, see @ref gpio_interrupt_callback_t
 * @param[in]       usr_arg               user's argument
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_register_callback(gpio_interrupt_callback_t gpio_isr_cb, void *usr_arg);

/**
 * @brief           set interrupt enable/disable for a specified GPIO pin
 *
 * @param[in]       pin                   GPIO pin ID, see @ref kdrv_gpio_pin_t
 * @param[in]       isEnable              enable/disable
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_set_interrupt(
    kdrv_gpio_pin_t pin,
    bool isEnable);

/**
 * @brief           set debounce enable/disable with clock setting in Hz
 *
 * @details         This can enable internal debouncing hardware for interrupt mode to eliminate the switch bounce.\n
 *                  It is very useful for connecting devices like a switch button or a keypad thing. 
 *
 * @param[in]       pin                   GPIO pin ID, see @ref kdrv_gpio_pin_t
 * @param[in]       isEnable              enable/disable
 * @param[in]       debounce_clock        The debouncing clock frequency in Hz
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_set_debounce(
    kdrv_gpio_pin_t pin,
    bool isEnable,
    uint32_t debounce_clock /* in Hz */);

/**
 * @brief           write GPIO digitial pin value
 *
 * @details         This function writes a high or low value to a digital pin.\n
 *                  The specified pin must be configured as digital output.
 *
 * @param[in]       pin                   GPIO pin ID, see @ref kdrv_gpio_pin_t
 * @param[in]       value                 Output value as digital high or digital low
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_write_pin(
    kdrv_gpio_pin_t pin,
    bool value);

/**
 * @brief           read GPIO digitial pin value
 *
 * @details         This function read a high or low value from a digital pin.\n
 *                  The specified pin must be configured as digital input and not in interrupt mode.
 *
 * @param[in]       pin                   GPIO pin ID, see @ref kdrv_gpio_pin_t
 * @param[out]      pValue                pointer to a value to read out GPIO voltage level
 *
 * @return          KDRV_STATUS_OK only
 */
kdrv_status_t kdrv_gpio_read_pin(
    kdrv_gpio_pin_t pin,
    bool *pValue);

#endif
