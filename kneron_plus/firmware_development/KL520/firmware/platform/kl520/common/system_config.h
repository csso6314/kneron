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

/**@addtogroup  SYSTEM_CONFIG
 * @{
 * @brief       Kneron System config
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

//clock

#define APB_CLOCK 100000000     //FPGA:50000000
//#define LCD_PATGEN
// total record how much size, then play it
#define I2S_TOTAL_SIZE          0x2000          
#define UART_CLOCK              (30000000UL) //kneron
#define UART_CLOCK_2            (30000000UL) //kneron

//#define MIXING_MODE_OPEN_RENDERER

#endif

