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
#ifndef Kneron_Beethoven
#define Kneron_Beethoven

/*  ----------------------------   APB1 Peripheral Device    --------------------------------*/

//clock

#define APB_CLOCK 50000000     //FPGA:50000000
//#define LCD_PATGEN
// total record how much size, then play it
#define I2S_TOTAL_SIZE          0x2000

#if _BOARD_SN720HAPS_H_ == 1
#define UART_CLOCK              (52000000UL) //20191024 bit file
#else
#define UART_CLOCK              (30000000UL) //Chip
#endif

#endif

