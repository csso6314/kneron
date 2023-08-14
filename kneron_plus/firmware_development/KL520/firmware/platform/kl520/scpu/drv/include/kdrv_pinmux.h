/**
 * @file        kdrv_pinmux.h
 * @brief       Kneron pinmux config driver
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_PINMUX_H__
#define __KDRV_PINMUX_H__

/**
 * @brief Enumerations of KDP520 all configurable pins
 */
typedef enum
{
    KDRV_PIN_SPI_WP_N = 0,      /**< PAD name X_SPI_WP_N, default PIN_MODE_0*/
    KDRV_PIN_SPI_HOLD_N,        /**< PAD name X_SPI_HOLD_N, default PIN_MODE_0*/
    KDRV_PIN_JTAG_TRST_N,       /**< PAD name X_JTAG_TRST_N, default PIN_MODE_0*/
    KDRV_PIN_JTAG_TDI,          /**< PAD name X_JTAG_TDI, default PIN_MODE_0*/
    KDRV_PIN_JTAG_SWDITMS,      /**< PAD name X_JTAG_SWDITMS, default PIN_MODE_0*/
    KDRV_PIN_JTAG_SWCLKTCK,     /**< PAD name X_JTAG_SWCLKTCK, default PIN_MODE_0*/
    KDRV_PIN_JTAG_TDO,          /**< PAD name X_JTAG_TDO, default PIN_MODE_0*/
    KDRV_PIN_LC_PCLK,           /**< PAD name X_LC_PCLK, default PIN_MODE_0*/
    KDRV_PIN_LC_VS,             /**< PAD name X_LC_VS, default PIN_MODE_0*/
    KDRV_PIN_LC_HS,             /**< PAD name X_LC_HS, default PIN_MODE_0*/
    KDRV_PIN_LC_DE,             /**< PAD name X_LC_DE, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_0,         /**< PAD name X_LC_DATA_0, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_1,         /**< PAD name X_LC_DATA_1, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_2,         /**< PAD name X_LC_DATA_2, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_3,         /**< PAD name X_LC_DATA_3, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_4,         /**< PAD name X_LC_DATA_4, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_5,         /**< PAD name X_LC_DATA_5, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_6,         /**< PAD name X_LC_DATA_6, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_7,         /**< PAD name X_LC_DATA_7, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_8,         /**< PAD name X_LC_DATA_8, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_9,         /**< PAD name X_LC_DATA_9, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_10,        /**< PAD name X_LC_DATA_10, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_11,        /**< PAD name X_LC_DATA_11, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_12,        /**< PAD name X_LC_DATA_12, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_13,        /**< PAD name X_LC_DATA_13, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_14,        /**< PAD name X_LC_DATA_14, default PIN_MODE_0*/
    KDRV_PIN_LC_DATA_15,        /**< PAD name X_LC_DATA_15, default PIN_MODE_0*/
    KDRV_PIN_SD_CLK,            /**< PAD name X_SD_CLK, default PIN_MODE_0*/
    KDRV_PIN_SD_CMD,            /**< PAD name X_SD_CMD, default PIN_MODE_0*/
    KDRV_PIN_SD_DAT_0,          /**< PAD name X_SD_DAT_0, default PIN_MODE_0*/
    KDRV_PIN_SD_DAT_1,          /**< PAD name X_SD_DAT_1, default PIN_MODE_0*/
    KDRV_PIN_SD_DAT_2,          /**< PAD name X_SD_DAT_2, default PIN_MODE_0*/
    KDRV_PIN_SD_DAT_3,          /**< PAD name X_SD_DAT_3, default PIN_MODE_0*/
    KDRV_PIN_UART0_RX,          /**< PAD name X_UART0_RX, default PIN_MODE_0*/
    KDRV_PIN_UART0_TX,          /**< PAD name X_UART0_TX, default PIN_MODE_0*/
    KDRV_PIN_I2C0_SCL,          /**< PAD name X_I2C0_SCL, default PIN_MODE_0*/
    KDRV_PIN_I2C0_SDA,          /**< PAD name X_I2C0_SDA, default PIN_MODE_0*/
    KDRV_PIN_PWM0               /**< PAD name X_PWM0, default PIN_MODE_0*/
} kdrv_pin_name;

/**
 * @brief Enumerations of KDP520 pinmux modes
 */
typedef enum{
    PIN_MODE_0 = 0,             /**< Pimux mode 0, Enum 0*/
    PIN_MODE_1,                 /**< Pimux mode 1, Enum 1*/
    PIN_MODE_2,                 /**< Pimux mode 2, Enum 2*/
    PIN_MODE_3,                 /**< Pimux mode 3, for GPIO mode only, Enum 3*/
    PIN_MODE_4,                 /**< Pimux mode 4, Enum 4*/
    PIN_MODE_5,                 /**< Pimux mode 5, Enum 5*/
    PIN_MODE_6,                 /**< Pimux mode 6, Enum 6*/
    PIN_MODE_7                  /**< Pimux mode 7, Enum 7*/
}kdrv_pinmux_mode;

/**
 * @brief Enumerations of KDP520 pull status
 */
typedef enum{
    PIN_PULL_NONE,              /**< Pin none,      Enum 0 */
    PIN_PULL_UP,                /**< Pin pull up,   Enum 1 */
    PIN_PULL_DOWN,              /**< Pin pull down, Enum 2*/
}kdrv_pin_pull;

/**
 * @brief Enumerations of KDP520 output driving capability
 */
typedef enum{
    PIN_DRIVING_4MA,            /**< 4mA  - 00, Enum 0*/
    PIN_DRIVING_8MA,            /**< 8mA  - 01, Enum 1*/
    PIN_DRIVING_12MA,           /**< 12mA - 10, Enum 2*/
    PIN_DRIVING_16MA,           /**< 16mA - 11, Enum 3*/
}kdrv_pin_driving;


/**
 * @brief       Pinmux init
 *
 * @return      N/A
 */
extern void kdrv_pinmux_initialize(uint32_t num, uint32_t *p_array);

/**
 * @brief       Pinmux configure
 *
 * @param[in]   pin          see @ref kdrv_pin_name
 * @param[in]   mode         see @ref kdrv_pinmux_mode
 * @param[in]   pull_type    see @ref kdrv_pin_pull
 * @param[in]   driving      see @ref kdrv_pin_driving
 * @return      N/A
 */
extern void kdrv_pinmux_config(kdrv_pin_name pin, kdrv_pinmux_mode mode, kdrv_pin_pull pull_type, kdrv_pin_driving driving);

#endif
/** @}*/
