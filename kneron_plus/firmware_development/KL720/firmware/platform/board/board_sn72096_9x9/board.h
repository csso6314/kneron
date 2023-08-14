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

/******************************************************************************
*  Filename:
*  ---------
*  board_sn72096_9x9.h
*
*  Description:
*  ------------
*
*
******************************************************************************/

#ifndef _BOARD_SN72096_9X9_H_
#define _BOARD_SN72096_9X9_H_

/******************************************************************************
Head Block of The File
******************************************************************************/
// Sec 0: Comment block of the file

// Sec 1: Include File

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define YES                             1
#define NO                              0

//System clock
//SCPU
#define SCPU_400                        0
//AXI_DDR
#define AXI_DDR_200                     0
#define AXI_DDR_333                     1
#define AXI_DDR_400                     2
#define AXI_DDR_466                     3
#define AXI_DDR_533                     4
//MRX1
#define MRX1_720                        0
//MRX0
#define MRX0_720                        0
#define MRX0_1320                       1
//NPU
#define NPU_200                         0
#define NPU_250                         1
#define NPU_300                         2
#define NPU_350                         3
#define NPU_400                         4
#define NPU_500                         5
#define NPU_600                         6
#define NPU_650                         7
#define NPU_700                         8
//DSP
#define DSP_200                         0
#define DSP_300                         1
#define DSP_400                         2
#define DSP_500                         3
//Audio
#define ADO_12p288                      0

//Display
#define DISPLAY_DEVICE_LCDC             0
#define DISPLAY_DEVICE_LCM              1
#define DISPLAY_DEVICE_NONE             2

#define DISPLAY_PANEL_MZT_480X272       0
#define DISPLAY_PANEL_ST7789_240X320    1
#define DISPLAY_PANEL_NONE              2


#define CFG_AI_3D_LIVENESS_IN_NONE      0
#define CFG_AI_3D_LIVENESS_IN_SCPU      1
#define CFG_AI_3D_LIVENESS_IN_NCPU      2

//Protocal
#define COM_NOT_USED                    -1
// UART
#define COMM_UART_0                     0
#define COMM_UART_1                     1

#define COMM_UART_BAUDRATE_1200         0
#define COMM_UART_BAUDRATE_2400         1
#define COMM_UART_BAUDRATE_4800         2
#define COMM_UART_BAUDRATE_9600         3
#define COMM_UART_BAUDRATE_14400        4
#define COMM_UART_BAUDRATE_19200        5
#define COMM_UART_BAUDRATE_38400        6
#define COMM_UART_BAUDRATE_57600        7
#define COMM_UART_BAUDRATE_115200       8
#define COMM_UART_BAUDRATE_460800       9
#define COMM_UART_BAUDRATE_921600       10

// I2C
#define COMM_I2C_0                      0
#define COMM_I2C_1                      1

#define COMM_I2C_SPEED_100K             0
#define COMM_I2C_SPEED_200K             1
#define COMM_I2C_SPEED_400K             2
#define COMM_I2C_SPEED_1000K            3

#define COMM_I2C_MODE_SLAVE             0
#define COMM_I2C_MODE_MASTER            1

// SPI
#define COMM_SPI_0                      0
#define COMM_SPI_1                      1

#define COMM_SPI_MODE_MODE_0            0
#define COMM_SPI_MODE_MODE_1            1
#define COMM_SPI_MODE_MODE_2            2
#define COMM_SPI_MODE_MODE_3            3

// I2S
#define COMM_I2S_0                      0
#define COMM_I2S_1                      1

#define COMM_I2S_MODE_SLAVE_MONO        0
#define COMM_I2S_MODE_SLAVE_STEREO      1
#define COMM_I2S_MODE_MASTER_MONO       2
#define COMM_I2S_MODE_MASTER_STEREO     3

#define COMM_I2S_MSB_FIRST              0
#define COMM_I2S_LSB_FIRST              1

#define COMM_I2S_PAD_BACK               0
#define COMM_I2S_PAD_FRONT              1

#define COMM_I2S_AUDIO_FREQ_192K        (192000U)
#define COMM_I2S_AUDIO_FREQ_96K         (96000U)
#define COMM_I2S_AUDIO_FREQ_48K         (48000U)
#define COMM_I2S_AUDIO_FREQ_44K         (44100U)
#define COMM_I2S_AUDIO_FREQ_32K         (32000U)
#define COMM_I2S_AUDIO_FREQ_22K         (22050U)
#define COMM_I2S_AUDIO_FREQ_16K         (16000U)
#define COMM_I2S_AUDIO_FREQ_11K         (11025U)
#define COMM_I2S_AUDIO_FREQ_8K          (8000U)

//flash
//flash manufacturer
#define FLASH_TYPE_NULL                 0X00
#define FLASH_TYPE_WINBOND_NOR          0X01
#define FLASH_TYPE_WINBOND_NAND         0X02
#define FLASH_TYPE_MXIC_NOR             0X11
#define FLASH_TYPE_MXIC_NAND            0X12
#define FLASH_TYPE_GIGADEVICE_NOR       0X21
#define FLASH_TYPE_GIGADEVICE_NAND      0X22

//flash SIZE
#define FLASH_SIZE_64MBIT               1   //8MBYTES
#define FLASH_SIZE_128MBIT              2   //16MBYTES
#define FLASH_SIZE_256MBIT              3   //32MBYTES
#define FLASH_SIZE_512MBIT              4   //64MBYTES
#define FLASH_SIZE_1GBIT                5   //128MBYTES
#define FLASH_SIZE_2GBIT                6   //256MBYTES
#define FLASH_SIZE_4GBIT                7   //512MBYTES

//speed(25MHZ/50MHZ/100MHZ)
#define FLASH_COMM_SPEED_25MHZ          1
#define FLASH_COMM_SPEED_50MHZ          2
#define FLASH_COMM_SPEED_100MHZ         3

//IO pin DRIVING STRENGTH
#define FLASH_DRV_NORMAL_MODE           1
#define FLASH_DRV_DUAL_IO_MODE          2
#define FLASH_DRV_DUAL_OUTPUT_MODE      3
#define FLASH_DRV_QUAD_IO_MODE          4
#define FLASH_DRV_QUAD_OUTPUT_MODE      5

//others
#define SUPPORT_ADC_DISABLE             0
#define SUPPORT_ADC_ENABLE              1
#define SUPPORT_DDR_DISABLE             0
#define SUPPORT_DDR_ENABLE              1
#define SUPPORT_GDMA_DISABLE            0
#define SUPPORT_GDMA_ENABLE             1
#define SUPPORT_GPIO_DISABLE            0
#define SUPPORT_GPIO_ENABLE             1
#define SUPPORT_PWM_DISABLE             0
#define SUPPORT_PWM_ENABLE              1
#define SUPPORT_SDC_DISABLE             0
#define SUPPORT_SDC_ENABLE              1
#define SUPPORT_TIMER_DISABLE           0
#define SUPPORT_TIMER_ENABLE            1
#define SUPPORT_USB_DISABLE             0
#define SUPPORT_USB_ENABLE              1
#define SUPPORT_WDT_DISABLE             0
#define SUPPORT_WDT_ENABLE              1
#define SUPPORT_UART_CONSOLE_DISABLE    0
#define SUPPORT_UART_CONSOLE_ENABLE     1
#define SUPPORT_I2S_ENABLE              1
#define SUPPORT_I2S_DISABLE             0
#define SUPPORT_I2C_ENABLE              1
#define SUPPORT_I2C_DISABLE             0
#define SUPPORT_SPI_ENABLE              1
#define SUPPORT_SPI_DISABLE             0
#define SUPPORT_UART_ENABLE             1
#define SUPPORT_UART_DISABLE            0
/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/


#endif //_BOARD_SN72096_9X9_H_

