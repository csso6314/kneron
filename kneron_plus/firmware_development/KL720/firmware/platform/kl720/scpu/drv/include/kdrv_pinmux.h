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

/**@addtogroup  KDRV_PINMUX_CONFIG  KDRV_PINMUX_CONFIG
 * @{
 * @brief       Kneron pinmux config driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KDRV_PINMUX_H__
#define __KDRV_PINMUX_H__

/** @brief Enumerations of KDP520 all configurable pins */
typedef enum
{
    KDRV_PIN_X_SPI_CS_N = 0,        /**< Enum 0  */
    KDRV_PIN_X_SPI_CLK,             /**< Enum 1  */
    KDRV_PIN_X_SPI_DO,              /**< Enum 2  */
    KDRV_PIN_X_SPI_DI,              /**< Enum 3  */
    KDRV_PIN_X_SPI_WP_N,            /**< Enum 4  */
    KDRV_PIN_X_SPI_HOLD_N,          /**< Enum 5  */
    KDRV_PIN_X_I2C0_CLK,            /**< Enum 6  */
    KDRV_PIN_X_I2C0_DATA,           /**< Enum 7  */
    KDRV_PIN_X_I2C1_CLK,            /**< Enum 8 */
    KDRV_PIN_X_I2C1_DATA,           /**< Enum 9  */
    KDRV_PIN_X_I2C2_CLK,            /**< Enum 10  */
    KDRV_PIN_X_I2C2_DATA,           /**< Enum 11  */
    KDRV_PIN_X_SSP0_CLK,            /**< Enum 12  */
    KDRV_PIN_X_SSP0_CS0,            /**< Enum 13  */
    KDRV_PIN_X_SSP0_CS1,            /**< Enum 14  */
    KDRV_PIN_X_SSP0_DI,             /**< Enum 15 */
    KDRV_PIN_X_SSP0_DO,             /**< Enum 16  */
    KDRV_PIN_X_SSP1_CLK,            /**< Enum 17  */
    KDRV_PIN_X_SSP1_CS,             /**< Enum 18  */
    KDRV_PIN_X_SSP1_DI,             /**< Enum 19  */
    KDRV_PIN_X_SSP1_DO,             /**< Enum 20  */
    KDRV_PIN_X_SSP1_DCX,            /**< Enum 21  */
    KDRV_PIN_X_JTAG_TRSTN,          /**< Enum 22  */
    KDRV_PIN_X_JTAG_TDI,            /**< Enum 23  */
    KDRV_PIN_X_JTAG_TMS,            /**< Enum 24  */
    KDRV_PIN_X_JTAG_TCK,            /**< Enum 25  */
    KDRV_PIN_X_UART0_TX,            /**< Enum 26  */
    KDRV_PIN_X_UART0_RX,            /**< Enum 27  */
    KDRV_PIN_X_DSP_TRSTN,           /**< Enum 28  */
    KDRV_PIN_X_DSP_TDI,             /**< Enum 29  */
    KDRV_PIN_X_DSP_TDO,             /**< Enum 30  */
    KDRV_PIN_X_DSP_TMS,             /**< Enum 31  */
    KDRV_PIN_X_DSP_TCK,             /**< Enum 32  */
    KDRV_PIN_X_TRACE_CLK,           /**< Enum 33  */
    KDRV_PIN_X_TRACE_DATA0,     /**< Enum 34  */
    KDRV_PIN_X_TRACE_DATA1,     /**< Enum 35  */
    KDRV_PIN_X_TRACE_DATA2,     /**< Enum 36  */
    KDRV_PIN_X_TRACE_DATA3,     /**< Enum 37  */
    KDRV_PIN_X_UART1_RI,            /**< Enum 38  */
    KDRV_PIN_X_SD1_D3,              /**< Enum 39  */
    KDRV_PIN_X_SD1_D2,              /**< Enum 40  */
    KDRV_PIN_X_SD1_D1,              /**< Enum 41  */
    KDRV_PIN_X_SD1_D0,              /**< Enum 42  */
    KDRV_PIN_X_SD1_CMD,             /**< Enum 43  */
    KDRV_PIN_X_SD1_CLK,             /**< Enum 44  */
    KDRV_PIN_X_SD0_D3,              /**< Enum 45  */
    KDRV_PIN_X_SD0_D2,              /**< Enum 46  */
    KDRV_PIN_X_SD0_D1,              /**< Enum 47  */
    KDRV_PIN_X_SD0_D0,              /**< Enum 48  */
    KDRV_PIN_X_SD0_CMD,             /**< Enum 49  */
    KDRV_PIN_X_SD0_CLK,             /**< Enum 50  */
    KDRV_PIN_X_SD0_CARD_PWN,    /**< Enum 51  */
    KDRV_PIN_X_SD0_CARD_DET,    /**< Enum 52  */
    KDRV_PIN_X_JTAG_TDO,            /**< Enum 53  */
    KDRV_PIN_X_PWM0,                /**< Enum 54  */
    KDRV_PIN_X_PWM1,                /**< Enum 55  */
    KDRV_PIN_X_DPI_PCLKI,           /**< Enum 56  */
    KDRV_PIN_X_DPI_VSI,             /**< Enum 57  */
    KDRV_PIN_X_DPI_HSI,             /**< Enum 58  */
    KDRV_PIN_X_DPI_DEI,             /**< Enum 59  */
    KDRV_PIN_X_DPI_DATAI0,      /**< Enum 60  */
    KDRV_PIN_X_DPI_DATAI1,          /**< Enum 61  */
    KDRV_PIN_X_DPI_DATAI2,          /**< Enum 62  */
    KDRV_PIN_X_DPI_DATAI3,          /**< Enum 63  */
    KDRV_PIN_X_DPI_DATAI4,          /**< Enum 64  */
    KDRV_PIN_X_DPI_DATAI5,          /**< Enum 65  */
    KDRV_PIN_X_DPI_DATAI6,          /**< Enum 66  */
    KDRV_PIN_X_DPI_DATAI7,          /**< Enum 67  */
    KDRV_PIN_X_DPI_DATAI8,          /**< Enum 68  */
    KDRV_PIN_X_DPI_DATAI9,          /**< Enum 69  */
    KDRV_PIN_X_DPI_DATAI10,         /**< Enum 70  */
    KDRV_PIN_X_DPI_DATAI11,         /**< Enum 71  */
    KDRV_PIN_X_DPI_DATAI12,         /**< Enum 72  */
    KDRV_PIN_X_DPI_DATAI13,         /**< Enum 73  */
    KDRV_PIN_X_DPI_DATAI14,         /**< Enum 74  */
    KDRV_PIN_X_DPI_DATAI15,         /**< Enum 75  */
    KDRV_PIN_X_DPI_PCLKO,           /**< Enum 76  */
    KDRV_PIN_X_DPI_VSO,                 /**< Enum 77  */
    KDRV_PIN_X_DPI_HSO,                 /**< Enum 78  */
    KDRV_PIN_X_DPI_DEO,                 /**< Enum 79  */
    KDRV_PIN_X_DPI_DATAO0,          /**< Enum 80  */
    KDRV_PIN_X_DPI_DATAO1,          /**< Enum 81  */
    KDRV_PIN_X_DPI_DATAO2,          /**< Enum 82  */
    KDRV_PIN_X_DPI_DATAO3,          /**< Enum 83  */
    KDRV_PIN_X_DPI_DATAO4,          /**< Enum 84  */
    KDRV_PIN_X_DPI_DATAO5,          /**< Enum 85  */
    KDRV_PIN_X_DPI_DATAO6,          /**< Enum 86  */
    KDRV_PIN_X_DPI_DATAO7,          /**< Enum 87  */
    KDRV_PIN_X_DPI_DATAO8,          /**< Enum 88  */
    KDRV_PIN_X_DPI_DATAO9,          /**< Enum 89  */
    KDRV_PIN_X_DPI_DATAO10,         /**< Enum 90  */
    KDRV_PIN_X_DPI_DATAO11          /**< Enum 91  */
} kdrv_pin_name;

/** @brief Enumerations of KDP520 pinmux modes */
typedef enum{
    PIN_MODE_0 = 0,             /**< Enum 0, Pimux mode 0*/
    PIN_MODE_1,                 /**< Enum 1, Pimux mode 1*/
    PIN_MODE_2,                 /**< Enum 2, Pimux mode 2*/
    PIN_MODE_3,                 /**< Enum 3, Pimux mode 3, for GPIO mode only*/
    PIN_MODE_4,                 /**< Enum 4, Pimux mode 4*/
    PIN_MODE_5,                 /**< Enum 5, Pimux mode 5*/
    PIN_MODE_6,                 /**< Enum 6, Pimux mode 6*/
    PIN_MODE_7                  /**< Enum 7, Pimux mode 7*/
}kdrv_pinmux_mode;

/** @brief Enumerations of KDP520 pull status */
typedef enum{
    PIN_PULL_NONE,              /**< Enum 0, Pin none*/
    PIN_PULL_UP,                /**< Enum 1, Pin pull up*/
    PIN_PULL_DOWN,              /**< Enum 2, Pin pull down*/
}kdrv_pin_pull;

/** @brief Enumerations of KDP520 output driving capability */
typedef enum{
    PIN_DRIVING_4MA,            /**< Enum 0, 4mA  */
    PIN_DRIVING_8MA,            /**< Enum 1, 8mA  */
    PIN_DRIVING_12MA,           /**< Enum 2,  12mA */
    PIN_DRIVING_16MA,           /**< Enum 3, 16mA */
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
