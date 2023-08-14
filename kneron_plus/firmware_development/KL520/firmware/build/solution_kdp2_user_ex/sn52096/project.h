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
*  project.h
*
*  Description:
*  ------------
*
*
******************************************************************************/

#ifndef _PROJECT_H_
#define _PROJECT_H_


/*=============================================================================
asic setting
=============================================================================*/
#include "membase.h"

/*=============================================================================
board setting
=============================================================================*/
#include "board.h"

#define FLASH_TYPE                              FLASH_TYPE_WINBOND_NOR
#define FLASH_SIZE                              FLASH_SIZE_256MBIT
#define FLASH_COMM                              FLASH_COMM_SPEED_25MHZ
#define FLASH_DRV                               FLASH_DRV_NORMAL_MODE

/*=============================================================================
COMM setting
=============================================================================*/
#define UART_NUM                                1
#define MSG_PORT                                COMM_PORT_ID_0
#define MSG_PORT_BAUDRATE                       COMM_UART_BAUDRATE_115200

/*=============================================================================
Pinmux setting
=============================================================================*/
#define PIN_NUM                                 38
#define KDRV_PIN_SPI_WP_N_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_SPI_HOLD_N_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_JTAG_TRST_N_REG                PIN_MODE_0 | (PIN_PULL_DOWN << 3) | (PIN_DRIVING_12MA << 6) //0x00000090
#define KDRV_PIN_JTAG_TDI_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_JTAG_SWDITMS_REG               PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_JTAG_SWCLKTCK_REG              PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_JTAG_TDO_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_LC_PCLK_REG                    PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_4MA  << 6) //0x00000000
#define KDRV_PIN_LC_VS_REG                      PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_HS_REG                      PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DE_REG                      PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_0_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_4MA  << 6) //0x00000000
#define KDRV_PIN_LC_DATA_1_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_4MA  << 6) //0x00000000
#define KDRV_PIN_LC_DATA_2_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_4MA  << 6) //0x00000000
#define KDRV_PIN_LC_DATA_3_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_4_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_5_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_6_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_7_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_8_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_9_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_10_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_11_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_12_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_13_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_14_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_LC_DATA_15_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_8MA  << 6) //0x00000040
#define KDRV_PIN_SD_CLK_REG                     PIN_MODE_1 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000081
#define KDRV_PIN_SD_CMD_REG                     PIN_MODE_1 | (PIN_PULL_UP   << 3) | (PIN_DRIVING_12MA << 6) //0x00000089
#define KDRV_PIN_SD_DAT_0_REG                   PIN_MODE_0 | (PIN_PULL_UP   << 3) | (PIN_DRIVING_4MA  << 6) //0x00000008
#define KDRV_PIN_SD_DAT_1_REG                   PIN_MODE_0 | (PIN_PULL_UP   << 3) | (PIN_DRIVING_4MA  << 6) //0x00000008
#define KDRV_PIN_SD_DAT_2_REG                   PIN_MODE_0 | (PIN_PULL_UP   << 3) | (PIN_DRIVING_4MA  << 6) //0x00000008
#define KDRV_PIN_SD_DAT_3_REG                   PIN_MODE_0 | (PIN_PULL_UP   << 3) | (PIN_DRIVING_4MA  << 6) //0x00000008
#define KDRV_PIN_UART0_RX_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_UART0_TX_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define KDRV_PIN_I2C0_SCL_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_4MA  << 6) //0x00000000
#define KDRV_PIN_I2C0_SDA_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_4MA  << 6) //0x00000000
#define KDRV_PIN_PWM0_REG                       PIN_MODE_0 | (PIN_PULL_NONE << 3) | (PIN_DRIVING_12MA << 6) //0x00000080
#define PINMUX_ARRAY                            {KDRV_PIN_SPI_WP_N_REG,        KDRV_PIN_SPI_HOLD_N_REG,        KDRV_PIN_JTAG_TRST_N_REG,         KDRV_PIN_JTAG_TDI_REG,          KDRV_PIN_JTAG_SWDITMS_REG,\
                                                 KDRV_PIN_JTAG_SWCLKTCK_REG,   KDRV_PIN_JTAG_TDO_REG,          KDRV_PIN_LC_PCLK_REG,             KDRV_PIN_LC_VS_REG,             KDRV_PIN_LC_HS_REG,\
                                                 KDRV_PIN_LC_DE_REG,           KDRV_PIN_LC_DATA_0_REG,         KDRV_PIN_LC_DATA_1_REG,           KDRV_PIN_LC_DATA_2_REG,         KDRV_PIN_LC_DATA_3_REG,\
                                                 KDRV_PIN_LC_DATA_4_REG,       KDRV_PIN_LC_DATA_5_REG,         KDRV_PIN_LC_DATA_6_REG,           KDRV_PIN_LC_DATA_7_REG,         KDRV_PIN_LC_DATA_8_REG,\
                                                 KDRV_PIN_LC_DATA_9_REG,       KDRV_PIN_LC_DATA_10_REG,        KDRV_PIN_LC_DATA_11_REG,          KDRV_PIN_LC_DATA_12_REG,        KDRV_PIN_LC_DATA_13_REG,\
                                                 KDRV_PIN_LC_DATA_14_REG,      KDRV_PIN_LC_DATA_15_REG,        KDRV_PIN_SD_CLK_REG,              KDRV_PIN_SD_CMD_REG,            KDRV_PIN_SD_DAT_0_REG,\
                                                 KDRV_PIN_SD_DAT_1_REG,        KDRV_PIN_SD_DAT_2_REG,          KDRV_PIN_SD_DAT_3_REG,            KDRV_PIN_UART0_RX_REG,          KDRV_PIN_UART0_TX_REG,\
                                                 KDRV_PIN_I2C0_SCL_REG,        KDRV_PIN_I2C0_SDA_REG,          KDRV_PIN_PWM0_REG};


/*=============================================================================
fw setting
=============================================================================*/
#define OS_DYNAMIC_MEM_SIZE                 (1024*32)      /**< available memory size in RTX*/

/*=============================================================================
DDR configuration
=============================================================================*/
/* DDR table */
#define DDR_BEGIN                           DDR_MEM_BASE   /**< = 0x60000000, definded in regbase.h*/
#define DDR_END                             (DDR_MEM_BASE + DDR_MEM_SIZE - 1) /**< DDR end address */

/** Reserve for all_models.bin */
#define DDR_MODEL_RESERVED_BEGIN            KDP_DDR_BASE   /**< space head for model data */
#define DDR_MODEL_RESERVED_END              0x613FFFFF     /**< space end for model data(initial boundary) */

/** Resseve for DDR heap. Allocation direction from END to BEGIN */
#define DDR_HEAP_BEGIN                      0x61400000     /**< space head for HEAP (initial boundary) */
#define DDR_HEAP_END                        0x63FCFFFF     /**< space end for HEAP */

/** Reserve for system information, 188KB */
#define DDR_SYSTEM_RESERVED_BEGIN           0x63FD0000     /**< space head for system info */
#define DDR_SYSTEM_RESERVED_END             0x63FFEFFF     /**< space end for system info */

/** Definition of snapshot image address and size, for kdrv_lcdc debug only*/
#define KDP_DDR_SNAPSHOT_RGB_IMG_SIZE       0x96000     /* 640x480x2(RGB565) */
#define KDP_DDR_SNAPSHOT_NIR_IMG_SIZE       0x4B000     /* 480x640x1(RAW8) */
#define KDP_DDR_SNAPSHOT_RGB_IMG_ADDR       DDR_MODEL_RESERVED_END
#define KDP_DDR_SNAPSHOT_NIR_IMG_ADDR       (DDR_MODEL_RESERVED_END + KDP_DDR_SNAPSHOT_RGB_IMG_SIZE )

/*=============================================================================
Flash configuration
=============================================================================*/
/* Flash table */
#define FLASH_FW_SCPU0_ADDR                 0x00002000  /**< fw_scpu.bin      */
#define FLASH_FW_NCPU0_ADDR                 0x00018000  /**< fw_ncpu.bin      */
#define FLASH_FW_CFG0_ADDR                  0x00028000  /**< boot_cfg0.bin    */
#define FLASH_FW_SCPU1_ADDR                 0x00041000  /**< fw_scpu1.bin     */
#define FLASH_FW_NCPU1_ADDR                 0x00057000  /**< fw_ncpu1.bin     */
#define FLASH_FW_CFG1_ADDR                  0x00067000  /**< boot_cfg1.bin    */
#define FLASH_MODEL_FW_INFO_ADDR            0x00300000  /**< fw_info.bin      */
#define FLASH_MODEL_ALL_ADDR                0x00301000  /**< all_models.bin   */
#define FLASH_END_ADDR                      0x01FFFFFF  /**< end addr of 32MB flash */

#define FLASH_MINI_BLOCK_SIZE               (4 * 1024)

/*=============================================================================
mdw setting
=============================================================================*/
/* scpu/ncpu image size */
#define SCPU_IMAGE_SIZE                     (SiRAM_MEM_SIZE - 0x2000)
#define NCPU_IMAGE_SIZE                     NiRAM_MEM_SIZE

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
-->critical setting<--
Below setting is for RD tuning or testing.
**Don't touch anything if you don't know what you are doing**
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/




#endif //_PROJECT_H_
