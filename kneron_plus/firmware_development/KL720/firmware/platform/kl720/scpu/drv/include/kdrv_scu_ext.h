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
/**@addtogroup  KDRV_SCU_EXT    KDRV_SCU_EXT
 * @{
 * @brief       Kneron SCU driver (extension register)
  * @version    v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
*/

#ifndef __KDRV_SCU_EXTREG_H__
#define __KDRV_SCU_EXTREG_H__

#include <stdint.h>
#include "regbase.h"

//#define    SCU_EXTREG_PLL0                      (SCU_EXT_REG_BASE + 0x0000)
#define    SCU_EXTREG_PLL1                      (SCU_EXT_REG_BASE + 0x0008)
#define    SCU_EXTREG_PLL1_LOCK_TIMER           (SCU_EXT_REG_BASE + 0x000C)
#define    SCU_EXTREG_PLL2                      (SCU_EXT_REG_BASE + 0x0010)
#define    SCU_EXTREG_PLL2_LOCK_TIMER           (SCU_EXT_REG_BASE + 0x0014)
#define    SCU_EXTREG_PLL3                      (SCU_EXT_REG_BASE + 0x0018)
#define    SCU_EXTREG_PLL3_LOCK_TIMER           (SCU_EXT_REG_BASE + 0x001C)
#define    SCU_EXTREG_PLL4                      (SCU_EXT_REG_BASE + 0x0020)
#define    SCU_EXTREG_PLL4_LOCK_TIMER           (SCU_EXT_REG_BASE + 0x0024)
#define    SCU_EXTREG_PLL5                      (SCU_EXT_REG_BASE + 0x0028)
#define    SCU_EXTREG_PLL5_LOCK_TIMER           (SCU_EXT_REG_BASE + 0x002C)
#define    SCU_EXTREG_PLL6                      (SCU_EXT_REG_BASE + 0x0030)
#define    SCU_EXTREG_PLL6_LOCK_TIMER           (SCU_EXT_REG_BASE + 0x0034)
//#define    SCU_EXTREG_DDR_PLL                   (SCU_EXT_REG_BASE + 0x0038)
//#define    SCU_EXTREG_DDR_PLL_LOCK_TIMER        (SCU_EXT_REG_BASE + 0x003C)
#define    SCU_EXTREG_DSYS_CTRL                 (SCU_EXT_REG_BASE + 0x0040)
#define    SCU_EXTREG_CLOCK_ENABLE_REG0         (SCU_EXT_REG_BASE + 0x0044)
#define    SCU_EXTREG_CLOCK_ENABLE_REG1         (SCU_EXT_REG_BASE + 0x0048)
//#define    SCU_EXTREG_AHB                       (SCU_EXT_REG_BASE + 0x004C)
//#define    SCU_EXTREG_H2PS_AHB                  (SCU_EXT_REG_BASE + 0x0050)
#define    SCU_EXTREG_CLOCK_MUX                 (SCU_EXT_REG_BASE + 0x0054)
#define    SCU_EXTREG_CLOCK_DIV0                (SCU_EXT_REG_BASE + 0x0058)
#define    SCU_EXTREG_CLOCK_DIV1                (SCU_EXT_REG_BASE + 0x005C)
#define    SCU_EXTREG_CLOCK_DIV2                (SCU_EXT_REG_BASE + 0x0060)
#define    SCU_EXTREG_CLOCK_DIV3                (SCU_EXT_REG_BASE + 0x0064)
#define    SCU_EXTREG_CLOCK_DIV4                (SCU_EXT_REG_BASE + 0x0068)
#define    SCU_EXTREG_CLOCK_DIV5                (SCU_EXT_REG_BASE + 0x006C)
#define    SCU_EXTREG_CLOCK_DIV6                (SCU_EXT_REG_BASE + 0x0070)
#define    SCU_EXTREG_SOFTWARE_RM3              (SCU_EXT_REG_BASE + 0x0080)
#define    SCU_EXTREG_SOFTWARE_RM2              (SCU_EXT_REG_BASE + 0x0084)
#define    SCU_EXTREG_SOFTWARE_RM1              (SCU_EXT_REG_BASE + 0x0088)
#define    SCU_EXTREG_SOFTWARE_RM0              (SCU_EXT_REG_BASE + 0x008C)
#define    SCU_EXTREG_SWRST                     (SCU_EXT_REG_BASE + 0x00A0)
#define    SCU_EXTREG_IPRST                     (SCU_EXT_REG_BASE + 0x00A4)
#define    SCU_EXTREG_CM4_CTRL_REG0             (SCU_EXT_REG_BASE + 0x0100)
#define    SCU_EXTREG_CM4_CTRL_REG1             (SCU_EXT_REG_BASE + 0x0104)
#define    SCU_EXTREG_VP6_CTRL_REG0             (SCU_EXT_REG_BASE + 0x0108)
#define    SCU_EXTREG_VP6_CTRL_REG1             (SCU_EXT_REG_BASE + 0x010C)
#define    SCU_EXTREG_DDR_RST                   (SCU_EXT_REG_BASE + 0x0110)
#define    SCU_EXTREG_CSIRX_CTRL_REG0           (SCU_EXT_REG_BASE + 0x0114)
#define    SCU_EXTREG_CSIRX_CTRL_REG1           (SCU_EXT_REG_BASE + 0x0118)
#define    SCU_EXTREG_DPI2AHB_CTRL_REG          (SCU_EXT_REG_BASE + 0x011C)
#define    SCU_EXTREG_USB3_PHY_CTRL_REG         (SCU_EXT_REG_BASE + 0x0120)
#define    SCU_EXTREG_USB2_PHY_CTRL_REG         (SCU_EXT_REG_BASE + 0x0124)
#define    SCU_EXTREG_CPU_IPC                   (SCU_EXT_REG_BASE + 0x0128)
#define    SCU_EXTREG_DSP_IPC                   (SCU_EXT_REG_BASE + 0x012C)
#define    SCU_EXTREG_AHB2RAM_CTRL_REG          (SCU_EXT_REG_BASE + 0x0140)
#define    SCU_EXTREG_MISC                      (SCU_EXT_REG_BASE + 0x0144)
#define    SCU_EXTREG_EFUSE_128                 (SCU_EXT_REG_BASE + 0x0148)
#define    SCU_EXTREG_EFUSE_512                 (SCU_EXT_REG_BASE + 0x014C)
#define    SCU_EXTREG_FTDMAC020_CTRL_REG        (SCU_EXT_REG_BASE + 0x0150)
#define    SCU_EXTREG_FTDMAC020_CONST_REG       (SCU_EXT_REG_BASE + 0x0154)
#define    SCU_EXTREG_GAL_SW_RESET_PROTECT_REG  (SCU_EXT_REG_BASE + 0x0158)
#define    SCU_EXTREG_DDR_AXI_REG               (SCU_EXT_REG_BASE + 0x015C)
#define    SCU_EXTREG_MIPI_CSIRX_TRIMMING_REG   (SCU_EXT_REG_BASE + 0x0160)
#define    SCU_EXTREG_SPARE_DEFAULT_0R0         (SCU_EXT_REG_BASE + 0x01D0)
#define    SCU_EXTREG_SPARE_DEFAULT_0R1         (SCU_EXT_REG_BASE + 0x01D4)
#define    SCU_EXTREG_SPARE_DEFAULT_1R0         (SCU_EXT_REG_BASE + 0x01D8)
#define    SCU_EXTREG_SPARE_DEFAULT_1R1         (SCU_EXT_REG_BASE + 0x01DC)
#define    SCU_EXTREG_SPARE_RO_0                (SCU_EXT_REG_BASE + 0x01E0)
#define    SCU_EXTREG_SPARE_RO_1                (SCU_EXT_REG_BASE + 0x01E4)
#define    SCU_EXTREG_SCU_POWER_EXT             (SCU_EXT_REG_BASE + 0x01F0)
#define    SCU_EXTREG_IOCTL_SPI_CS_N            (SCU_EXT_REG_BASE + 0x0200)
#define    SCU_EXTREG_IOCTL_SPI_CLK             (SCU_EXT_REG_BASE + 0x0204)
#define    SCU_EXTREG_IOCTL_SPI_DO              (SCU_EXT_REG_BASE + 0x0208)
#define    SCU_EXTREG_IOCTL_SPI_DI              (SCU_EXT_REG_BASE + 0x020C)
#define    SCU_EXTREG_IOCTL_SPI_WP_N            (SCU_EXT_REG_BASE + 0x0210)
#define    SCU_EXTREG_IOCTL_SPI_HOLD_N          (SCU_EXT_REG_BASE + 0x0214)
#define    SCU_EXTREG_IOCTL_I2C0_CLK            (SCU_EXT_REG_BASE + 0x0218)
#define    SCU_EXTREG_IOCTL_I2C0_DATA           (SCU_EXT_REG_BASE + 0x021C)
#define    SCU_EXTREG_IOCTL_I2C1_CLK            (SCU_EXT_REG_BASE + 0x0220)
#define    SCU_EXTREG_IOCTL_I2C1_DATA           (SCU_EXT_REG_BASE + 0x0224)
#define    SCU_EXTREG_IOCTL_I2C2_CLK            (SCU_EXT_REG_BASE + 0x0228)
#define    SCU_EXTREG_IOCTL_I2C2_DATA           (SCU_EXT_REG_BASE + 0x022C)
#define    SCU_EXTREG_IOCTL_SSP0_CLK            (SCU_EXT_REG_BASE + 0x0230)
#define    SCU_EXTREG_IOCTL_SSP0_CS0            (SCU_EXT_REG_BASE + 0x0234)
#define    SCU_EXTREG_IOCTL_SSP0_CS1            (SCU_EXT_REG_BASE + 0x0238)
#define    SCU_EXTREG_IOCTL_SSP0_DI             (SCU_EXT_REG_BASE + 0x023C)
#define    SCU_EXTREG_IOCTL_SSP0_DO             (SCU_EXT_REG_BASE + 0x0240)
#define    SCU_EXTREG_IOCTL_SSP1_CLK            (SCU_EXT_REG_BASE + 0x0244)
#define    SCU_EXTREG_IOCTL_SSP1_CS             (SCU_EXT_REG_BASE + 0x0248)
#define    SCU_EXTREG_IOCTL_SSP1_DI             (SCU_EXT_REG_BASE + 0x024C)
#define    SCU_EXTREG_IOCTL_SSP1_DO             (SCU_EXT_REG_BASE + 0x0250)
#define    SCU_EXTREG_IOCTL_SSP1_DCX            (SCU_EXT_REG_BASE + 0x0254)
#define    SCU_EXTREG_IOCTL_MCU_JTAG_TRST_N     (SCU_EXT_REG_BASE + 0x0258)
#define    SCU_EXTREG_IOCTL_MCU_JTAG_TDI        (SCU_EXT_REG_BASE + 0x025C)
#define    SCU_EXTREG_IOCTL_MCU_JTAG_TMS        (SCU_EXT_REG_BASE + 0x0260)
#define    SCU_EXTREG_IOCTL_MCU_JTAG_TCK        (SCU_EXT_REG_BASE + 0x0264)
#define    SCU_EXTREG_IOCTL_UART0_TX            (SCU_EXT_REG_BASE + 0x0268)
#define    SCU_EXTREG_IOCTL_UART0_RX            (SCU_EXT_REG_BASE + 0x026C)
#define    SCU_EXTREG_IOCTL_DSP_JTAG_TRST_N     (SCU_EXT_REG_BASE + 0x0270)
#define    SCU_EXTREG_IOCTL_DSP_JTAG_TDI        (SCU_EXT_REG_BASE + 0x0274)
#define    SCU_EXTREG_IOCTL_DSP_JTAG_TDO        (SCU_EXT_REG_BASE + 0x0278)
#define    SCU_EXTREG_IOCTL_DSP_JTAG_TMS        (SCU_EXT_REG_BASE + 0x027C)
#define    SCU_EXTREG_IOCTL_DSP_JTAG_TCK        (SCU_EXT_REG_BASE + 0x0280)
#define    SCU_EXTREG_IOCTL_MCU_TRACE_CLK       (SCU_EXT_REG_BASE + 0x0284)
#define    SCU_EXTREG_IOCTL_MCU_TRACE_DATA0     (SCU_EXT_REG_BASE + 0x0288)
#define    SCU_EXTREG_IOCTL_MCU_TRACE_DATA1     (SCU_EXT_REG_BASE + 0x028C)
#define    SCU_EXTREG_IOCTL_MCU_TRACE_DATA2     (SCU_EXT_REG_BASE + 0x0290)
#define    SCU_EXTREG_IOCTL_MCU_TRACE_DATA3     (SCU_EXT_REG_BASE + 0x0294)
#define    SCU_EXTREG_IOCTL_UART1_RI            (SCU_EXT_REG_BASE + 0x0298)
#define    SCU_EXTREG_IOCTL_SD1_D3              (SCU_EXT_REG_BASE + 0x029C)
#define    SCU_EXTREG_IOCTL_SD1_D2              (SCU_EXT_REG_BASE + 0x02A0)
#define    SCU_EXTREG_IOCTL_SD1_D1              (SCU_EXT_REG_BASE + 0x02A4)
#define    SCU_EXTREG_IOCTL_SD1_D0              (SCU_EXT_REG_BASE + 0x02A8)
#define    SCU_EXTREG_IOCTL_SD1_CMD             (SCU_EXT_REG_BASE + 0x02AC)
#define    SCU_EXTREG_IOCTL_SD1_CLK             (SCU_EXT_REG_BASE + 0x02B0)
#define    SCU_EXTREG_IOCTL_SD0_D3              (SCU_EXT_REG_BASE + 0x02B4)
#define    SCU_EXTREG_IOCTL_SD0_D2              (SCU_EXT_REG_BASE + 0x02B8)
#define    SCU_EXTREG_IOCTL_SD0_D1              (SCU_EXT_REG_BASE + 0x02BC)
#define    SCU_EXTREG_IOCTL_SD0_D0              (SCU_EXT_REG_BASE + 0x02C0)
#define    SCU_EXTREG_IOCTL_SD0_CMD             (SCU_EXT_REG_BASE + 0x02C4)
#define    SCU_EXTREG_IOCTL_SD0_CLK             (SCU_EXT_REG_BASE + 0x02C8)
#define    SCU_EXTREG_IOCTL_SD0_CARD_PWN        (SCU_EXT_REG_BASE + 0x02CC)
#define    SCU_EXTREG_IOCTL_SD0_CARD_DET        (SCU_EXT_REG_BASE + 0x02D0)
#define    SCU_EXTREG_IOCTL_MCU_JTAG_TDO        (SCU_EXT_REG_BASE + 0x02D4)
#define    SCU_EXTREG_IOCTL_PWM0                (SCU_EXT_REG_BASE + 0x02D8)
#define    SCU_EXTREG_IOCTL_PWM1                (SCU_EXT_REG_BASE + 0x02DC)
#define    SCU_EXTREG_IOCTL_DPI_PCLKI           (SCU_EXT_REG_BASE + 0x02E0)
#define    SCU_EXTREG_IOCTL_DPI_VSI             (SCU_EXT_REG_BASE + 0x02E4)
#define    SCU_EXTREG_IOCTL_DPI_HSI             (SCU_EXT_REG_BASE + 0x02E8)
#define    SCU_EXTREG_IOCTL_DPI_DEI             (SCU_EXT_REG_BASE + 0x02EC)
#define    SCU_EXTREG_IOCTL_DPI_DATAI0          (SCU_EXT_REG_BASE + 0x02F0)
#define    SCU_EXTREG_IOCTL_DPI_DATAI1          (SCU_EXT_REG_BASE + 0x02F4)
#define    SCU_EXTREG_IOCTL_DPI_DATAI2          (SCU_EXT_REG_BASE + 0x02F8)
#define    SCU_EXTREG_IOCTL_DPI_DATAI3          (SCU_EXT_REG_BASE + 0x02FC)
#define    SCU_EXTREG_IOCTL_DPI_DATAI4          (SCU_EXT_REG_BASE + 0x0300)
#define    SCU_EXTREG_IOCTL_DPI_DATAI5          (SCU_EXT_REG_BASE + 0x0304)
#define    SCU_EXTREG_IOCTL_DPI_DATAI6          (SCU_EXT_REG_BASE + 0x0308)
#define    SCU_EXTREG_IOCTL_DPI_DATAI7          (SCU_EXT_REG_BASE + 0x030C)
#define    SCU_EXTREG_IOCTL_DPI_DATAI8          (SCU_EXT_REG_BASE + 0x0310)
#define    SCU_EXTREG_IOCTL_DPI_DATAI9          (SCU_EXT_REG_BASE + 0x0314)
#define    SCU_EXTREG_IOCTL_DPI_DATAI10         (SCU_EXT_REG_BASE + 0x0318)
#define    SCU_EXTREG_IOCTL_DPI_DATAI11         (SCU_EXT_REG_BASE + 0x031C)
#define    SCU_EXTREG_IOCTL_DPI_DATAI12         (SCU_EXT_REG_BASE + 0x0320)
#define    SCU_EXTREG_IOCTL_DPI_DATAI13         (SCU_EXT_REG_BASE + 0x0324)
#define    SCU_EXTREG_IOCTL_DPI_DATAI14         (SCU_EXT_REG_BASE + 0x0328)
#define    SCU_EXTREG_IOCTL_DPI_DATAI15         (SCU_EXT_REG_BASE + 0x032C)
#define    SCU_EXTREG_IOCTL_DPI_PCLKO           (SCU_EXT_REG_BASE + 0x0330)
#define    SCU_EXTREG_IOCTL_DPI_VSO             (SCU_EXT_REG_BASE + 0x0334)
#define    SCU_EXTREG_IOCTL_DPI_HSO             (SCU_EXT_REG_BASE + 0x0338)
#define    SCU_EXTREG_IOCTL_DPI_DEO             (SCU_EXT_REG_BASE + 0x033C)
#define    SCU_EXTREG_IOCTL_DPI_DATAO0          (SCU_EXT_REG_BASE + 0x0340)
#define    SCU_EXTREG_IOCTL_DPI_DATAO1          (SCU_EXT_REG_BASE + 0x0344)
#define    SCU_EXTREG_IOCTL_DPI_DATAO2          (SCU_EXT_REG_BASE + 0x0348)
#define    SCU_EXTREG_IOCTL_DPI_DATAO3          (SCU_EXT_REG_BASE + 0x034C)
#define    SCU_EXTREG_IOCTL_DPI_DATAO4          (SCU_EXT_REG_BASE + 0x0350)
#define    SCU_EXTREG_IOCTL_DPI_DATAO5          (SCU_EXT_REG_BASE + 0x0354)
#define    SCU_EXTREG_IOCTL_DPI_DATAO6          (SCU_EXT_REG_BASE + 0x0358)
#define    SCU_EXTREG_IOCTL_DPI_DATAO7          (SCU_EXT_REG_BASE + 0x035C)
#define    SCU_EXTREG_IOCTL_DPI_DATAO8          (SCU_EXT_REG_BASE + 0x0360)
#define    SCU_EXTREG_IOCTL_DPI_DATAO9          (SCU_EXT_REG_BASE + 0x0364)
#define    SCU_EXTREG_IOCTL_DPI_DATAO10         (SCU_EXT_REG_BASE + 0x0368)
#define    SCU_EXTREG_IOCTL_DPI_DATAO11         (SCU_EXT_REG_BASE + 0x036C)
#define    SCU_EXTREG_IOCTL_USB3_VBUS           (SCU_EXT_REG_BASE + 0x0370)
#define    SCU_EXTREG_IOCTL_USB3_DRV_VBUS       (SCU_EXT_REG_BASE + 0x0374)
#define    SCU_EXTREG_IOCTL_USB2_VBUS           (SCU_EXT_REG_BASE + 0x037C)
#define    SCU_EXTREG_IOCTL_USB2_DRV_VBUS       (SCU_EXT_REG_BASE + 0x0380)
#define    SCU_EXTREG_IOCTL_MCLK                (SCU_EXT_REG_BASE + 0x0384)

#define SCU_EXTREG_PRINT(__desc, __regtype, __symbol) \
    DSG("%s %s %x", __desc, #__symbol, SCU_EXTREG_##__regtype##_GET##_##__symbol())

#define SCU_EXTREG_PRINT2(__label) \
    {DSG("%s=%x", #__label, inw(__label))};

#define SCU_EXTREG_PRINT_ALL() \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL1_LOCK_TIMER)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL2_LOCK_TIMER)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL3_LOCK_TIMER)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL4)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL4_LOCK_TIMER)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL5)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL5_LOCK_TIMER)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL6)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_PLL6_LOCK_TIMER)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_DSYS_CTRL)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_ENABLE_REG0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_ENABLE_REG1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_MUX)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_DIV0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_DIV1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_DIV2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_DIV3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_DIV4)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_DIV5)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CLOCK_DIV6)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SOFTWARE_RM3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SOFTWARE_RM2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SOFTWARE_RM1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SOFTWARE_RM0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SWRST)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IPRST)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CM4_CTRL_REG0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CM4_CTRL_REG1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_VP6_CTRL_REG0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_VP6_CTRL_REG1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_DDR_RST)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CSIRX_CTRL_REG0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CSIRX_CTRL_REG1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_DPI2AHB_CTRL_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_USB3_PHY_CTRL_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_USB2_PHY_CTRL_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_CPU_IPC)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_DSP_IPC)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_AHB2RAM_CTRL_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_MISC)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_EFUSE_128)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_EFUSE_512)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_FTDMAC020_CTRL_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_FTDMAC020_CONST_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_GAL_SW_RESET_PROTECT_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_DDR_AXI_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_MIPI_CSIRX_TRIMMING_REG)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE_DEFAULT_0R0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE_DEFAULT_0R1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE_DEFAULT_1R0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE_DEFAULT_1R1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE_RO_0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE_RO_1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_SCU_POWER_EXT)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SPI_CS_N)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SPI_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SPI_DO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SPI_DI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SPI_WP_N)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SPI_HOLD_N)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_I2C0_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_I2C0_DATA)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_I2C1_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_I2C1_DATA)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_I2C2_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_I2C2_DATA)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP0_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP0_CS0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP0_CS1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP0_DI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP0_DO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP1_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP1_CS)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP1_DI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP1_DO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SSP1_DCX)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_JTAG_TRST_N)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_JTAG_TDI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_JTAG_TMS)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_JTAG_TCK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_UART0_TX)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_UART0_RX)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DSP_JTAG_TRST_N)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DSP_JTAG_TDI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DSP_JTAG_TDO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DSP_JTAG_TMS)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DSP_JTAG_TCK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_TRACE_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_TRACE_DATA0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_TRACE_DATA1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_TRACE_DATA2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_TRACE_DATA3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_UART1_RI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD1_D3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD1_D2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD1_D1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD1_D0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD1_CMD)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD1_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_D3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_D2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_D1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_D0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_CMD)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_CLK)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_CARD_PWN)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_SD0_CARD_DET)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCU_JTAG_TDO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_PWM0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_PWM1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_PCLKI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_VSI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_HSI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DEI)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI4)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI5)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI6)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI7)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI8)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI9)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI10)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI11)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI12)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI13)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI14)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAI15)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_PCLKO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_VSO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_HSO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DEO)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO0)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO1)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO2)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO3)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO4)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO5)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO6)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO7)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO8)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO9)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO10)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_DPI_DATAO11)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_USB3_VBUS)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_USB3_DRV_VBUS)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_USB2_VBUS)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_USB2_DRV_VBUS)    \
        SCU_EXTREG_PRINT2(SCU_EXTREG_IOCTL_MCLK)

/*---------------------- Power Domain Software Reset Register (Offset: 0x00A0) -------------------------*/
/* Power Domain Software Reset Register (Offset: 0x00A0)
 */
typedef volatile struct {
    uint32_t PD_UDR: 1;                 /**< [0] 0: Assert power domain software reset. */
    uint32_t PD_UHO: 1;                 /**< [1]  */
    uint32_t PD_MRX: 1;                 /**< [2]  */
    uint32_t PD_NOM: 1;                 /**< [3]  */
    uint32_t PD_NPU: 1;                 /**< [4]  */
    uint32_t RSV2: 3;                   /**< [5:7] Reserved 2 */
    uint32_t BUS_NOM_AXI: 1;            /**< [8] 0: Assert NOM domain bus software reset. */
    uint32_t BUS_NOM_APB_0: 1;          /**< [9]  */
    uint32_t BUS_NOM_APB_1: 1;          /**< [10]  */
    uint32_t RSV1: 5;                   /**< [11:15] Reserved 1 */
    uint32_t GLOBAL: 1;                 /**< [16] Global software reset.
                                            To enable this function, please set offset 0x0158 first.
                                            1: Assert global software reset. */
    uint32_t RSV0: 15;                  /**< [17:31] Reserved 0 */
} pwr_sw_rst_t;

/* KDRV_PWR_SW_RST bit field definition
 * Constant definitions for KDRV_PWR_DOMAIN_SW_RST
 */
#define KDRV_PWR_SW_RST_PD_UDR_BIT           (0)                                          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_udr Position              */
#define KDRV_PWR_SW_RST_PD_UDR_MASK          (1UL << KDRV_PWR_SW_RST_PD_UDR_BIT)          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_udr Mask, VAR: 0x0        */
#define KDRV_PWR_SW_RST_PD_UHO_BIT           (1)                                          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_uho Position              */
#define KDRV_PWR_SW_RST_PD_UHO_MASK          (1UL << KDRV_PWR_SW_RST_PD_UHO_BIT)          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_uho Mask, VAR: 0x0        */
#define KDRV_PWR_SW_RST_PD_MRX_BIT           (2)                                          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_mrx Position              */
#define KDRV_PWR_SW_RST_PD_MRX_MASK          (1UL << KDRV_PWR_SW_RST_PD_MRX_BIT)          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_mrx Mask, VAR: 0x0        */
#define KDRV_PWR_SW_RST_PD_NOM_BIT           (3)                                          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_nom Position              */
#define KDRV_PWR_SW_RST_PD_NOM_MASK          (1UL << KDRV_PWR_SW_RST_PD_NOM_BIT)          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_nom Mask, VAR: 0x0        */
#define KDRV_PWR_SW_RST_PD_NPU_BIT           (4)                                          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_npu Position              */
#define KDRV_PWR_SW_RST_PD_NPU_MASK          (1UL << KDRV_PWR_SW_RST_PD_NPU_BIT)          /**< kdp720_scu_extreg:pwr_sw_rst_t: pd_npu Mask, VAR: 0x0        */
#define KDRV_PWR_SW_RST_BUS_NOM_AXI_BIT      (8)                                          /**< kdp720_scu_extreg:pwr_sw_rst_t: bus_nom_axi Position         */
#define KDRV_PWR_SW_RST_BUS_NOM_AXI_MASK     (1UL << KDRV_PWR_SW_RST_BUS_NOM_AXI_BIT)     /**< kdp720_scu_extreg:pwr_sw_rst_t: bus_nom_axi Mask, VAR: 0x0   */
#define KDRV_PWR_SW_RST_BUS_NOM_APB_0_BIT    (9)                                          /**< kdp720_scu_extreg:pwr_sw_rst_t: bus_nom_apb_0 Position       */
#define KDRV_PWR_SW_RST_BUS_NOM_APB_0_MASK   (1UL << KDRV_PWR_SW_RST_BUS_NOM_APB_0_BIT)   /**< kdp720_scu_extreg:pwr_sw_rst_t: bus_nom_apb_0 Mask, VAR: 0x1 */
#define KDRV_PWR_SW_RST_BUS_NOM_APB_1_BIT    (10)                                         /**< kdp720_scu_extreg:pwr_sw_rst_t: bus_nom_apb_1 Position       */
#define KDRV_PWR_SW_RST_BUS_NOM_APB_1_MASK   (1UL << KDRV_PWR_SW_RST_BUS_NOM_APB_1_BIT)   /**< kdp720_scu_extreg:pwr_sw_rst_t: bus_nom_apb_1 Mask, VAR: 0x1 */
#define KDRV_PWR_SW_RST_GLOBAL_BIT           (16)                                         /**< kdp720_scu_extreg:pwr_sw_rst_t: global Position              */
#define KDRV_PWR_SW_RST_GLOBAL_MASK          (1UL << KDRV_PWR_SW_RST_GLOBAL_BIT)          /**< kdp720_scu_extreg:pwr_sw_rst_t: global Mask, VAR: 0x0        */


/*---------------------- DDR Reset Register (Offset: 0x0110)-------------------------*/
/*  DDR Reset Register (Offset: 0x0110) */
typedef volatile struct {
    uint32_t ARESETN_DDR: 1;            /**< [0] AXI Asynchronous Reset. Active-low pin that asynchronously resets the AXI logic to its default state. */
    uint32_t CORE_DDRC_RSTN: 1;         /**< [1] Active low reset signal. The controller must be taken out of reset only after all registers have been programmed. */
    uint32_t PRESETN_DDR: 1;            /**< [2] APB reset.*/
    uint32_t PHY_SCAN_RESETN: 1;        /**< [3] */
    uint32_t PHY_CTL_SDR_RST_N: 1;      /**< [4] */
    uint32_t RSV0: 27;                  /**< Reserved 0 */
} ddr_rst_t;

/* KDRV_DDR_RST bit field definition
 * Constant definitions for KDRV_DDR_RST
 */
#define KDRV_DDR_RST_ARESETN_BIT            (0)                                         /**< kdp720_scu_extreg:ddr_rst_t: aresetn_ddr Position              */
#define KDRV_DDR_RST_ARESETN_MASK           (1UL << KDRV_DDR_RST_ARESETN_BIT)           /**< kdp720_scu_extreg:ddr_rst_t: aresetn_ddr Mask, VAR: 0x0        */
#define KDRV_DDR_RST_CORE_DDRC_RSTN_BIT     (1)                                         /**< kdp720_scu_extreg:ddr_rst_t: core_ddrc_rstn Position           */
#define KDRV_DDR_RST_CORE_DDRC_RSTN_MASK    (1UL << KDRV_DDR_RST_CORE_DDRC_RSTN_BIT)    /**< kdp720_scu_extreg:ddr_rst_t: core_ddrc_rstn Mask, VAR: 0x0     */
#define KDRV_DDR_RST_PRESETN_BIT            (2)                                         /**< kdp720_scu_extreg:ddr_rst_t: presetn Position                  */
#define KDRV_DDR_RST_PRESETN_MASK           (1UL << KDRV_DDR_RST_PRESETN_BIT)           /**< kdp720_scu_extreg:ddr_rst_t: presetn Mask, VAR: 0x0            */
#define KDRV_DDR_RST_PHY_SCAN_RESETN_BIT    (3)                                         /**< kdp720_scu_extreg:ddr_rst_t: PHY_scan_resetn Position          */
#define KDRV_DDR_RST_PHY_SCAN_RESETN_MASK   (1UL << KDRV_DDR_RST_PHY_SCAN_RESETN_BIT)   /**< kdp720_scu_extreg:ddr_rst_t: PHY_scan_resetn Mask, VAR: 0x0    */
#define KDRV_DDR_RST_PHY_CTL_SDR_RSTN_BIT   (4)                                         /**< kdp720_scu_extreg:ddr_rst_t: PHY_ctl_sdr_rst_n Position        */
#define KDRV_DDR_RST_PHY_CTL_SDR_RSTN_MASK  (1UL << KDRV_DDR_RST_PHY_CTL_SDR_RSTN_BIT)  /**< kdp720_scu_extreg:ddr_rst_t: PHY_ctl_sdr_rst_n Mask, VAR: 0x0  */

typedef struct {
    volatile uint32_t         rsvd0[2];                     //0x0000
    volatile uint32_t         pll1;                         //0x0008
    volatile uint32_t         pll1_lock_timer;              //0x000C
    volatile uint32_t         pll2;                         //0x0010
    volatile uint32_t         pll2_lock_timer;              //0x0014
    volatile uint32_t         pll3;                         //0x0018
    volatile uint32_t         pll3_lock_timer;              //0x001C
    volatile uint32_t         pll4;                         //0x0020
    volatile uint32_t         pll4_lock_timer;              //0x0024
    volatile uint32_t         pll5;                         //0x0028
    volatile uint32_t         pll5_lock_timer;              //0x002C
    volatile uint32_t         pll6;                         //0x0030
    volatile uint32_t         pll6_lock_timer;              //0x0034
    volatile uint32_t         rsvd9[2];                     //0x0038  0x003C
    volatile uint32_t         dsys_ctrl;                    //0x0040
    volatile uint32_t         clock_enable_reg0;            //0x0044
    volatile uint32_t         clock_enable_reg1;            //0x0048
    volatile uint32_t         rsvda[2];                     //0x004C  0x0050
    volatile uint32_t         clock_mux;                    //0x0054
    volatile uint32_t         clock_div0;                   //0x0058
    volatile uint32_t         clock_div1;                   //0x005C
    volatile uint32_t         clock_div2;                   //0x0060
    volatile uint32_t         clock_div3;                   //0x0064
    volatile uint32_t         clock_div4;                   //0x0068
    volatile uint32_t         clock_div5;                   //0x006C
    volatile uint32_t         clock_div6;                   //0x0070
    volatile uint32_t         rsvd1[3];                     //0x0074  0x0078 0x007C
    volatile uint32_t         software_rm3;                 //0x0080
    volatile uint32_t         software_rm2;                 //0x0084
    volatile uint32_t         software_rm1;                 //0x0088
    volatile uint32_t         software_rm0;                 //0x008C
    volatile uint32_t         rsvd2[4];                     //0x0090  0x0094  0x0098  0x009C
    volatile uint32_t         pwr_sw_rst;                   //0x00A0
    volatile uint32_t         iprst;                        //0x00A4
    volatile uint32_t         rsvd3[22];                    //0x00A8~ 0x00FC
    volatile uint32_t         cm4_ctrl_reg0;                //0x0100
    volatile uint32_t         cm4_ctrl_reg1;                //0x0104
    volatile uint32_t         vp6_ctrl_reg0;                //0x0108
    volatile uint32_t         vp6_ctrl_reg1;                //0x010C
    volatile uint32_t         ddr_rst;                      //0x0110
    volatile uint32_t         csirx_ctrl_reg0;              //0x0114
    volatile uint32_t         csirx_ctrl_reg1;              //0x0118
    volatile uint32_t         dpi2ahb_ctrl_reg;             //0x011C
    volatile uint32_t         usb3_phy_ctrl_reg;            //0x0120
    volatile uint32_t         usb2_phy_ctrl_reg;            //0x0124
    volatile uint32_t         cpu_ipc;                      //0x0128
    volatile uint32_t         dsp_ipc;                      //0x012C
    volatile uint32_t         rsvd4[4];                     //0x0130  0x0134  0x0138 0x013C
    volatile uint32_t         ahb2ram_ctrl_reg;             //0x0140
    volatile uint32_t         misc;                         //0x0144
    volatile uint32_t         efuse_128;                    //0x0148
    volatile uint32_t         efuse_512;                    //0x014C
    volatile uint32_t         ftdmac020_ctrl_reg;           //0x0150
    volatile uint32_t         ftdmac020_const_reg;          //0x0154
    volatile uint32_t         global_sw_reset_protect_reg;  //0x0158
    volatile uint32_t         ddr_axi_qos_reg;              //0x015C
    volatile uint32_t         mipi_csirx_phy_trimming_reg;  //0x0160
    volatile uint32_t         rsvd5[27];                    //0x0164~ 0x01CC
    volatile uint32_t         spare_default_0r0;            //0x01D0
    volatile uint32_t         spare_default_0r1;            //0x01D4
    volatile uint32_t         spare_default_1r0;            //0x01D8
    volatile uint32_t         spare_default_1r1;            //0x01DC
    volatile uint32_t         spare_ro_0;                   //0x01E0
    volatile uint32_t         spare_ro_1;                   //0x01E4
    volatile uint32_t         rsvd6[2];                     //0x01E8  0x01EC
    volatile uint32_t         scu_power_ext;                //0x01F0
    volatile uint32_t         rsvd7[3];                     //0x01F4  0x01F8  0x01FC
    volatile uint32_t         ioctl_spi_cs_n;               //0x0200
    volatile uint32_t         ioctl_spi_clk;                //0x0204
    volatile uint32_t         ioctl_spi_do;                 //0x0208
    volatile uint32_t         ioctl_spi_di;                 //0x020C
    volatile uint32_t         ioctl_spi_wp_n;               //0x0210
    volatile uint32_t         ioctl_spi_hold_n;             //0x0214
    volatile uint32_t         ioctl_i2c0_clk;               //0x0218
    volatile uint32_t         ioctl_i2c0_data;              //0x021C
    volatile uint32_t         ioctl_i2c1_clk;               //0x0220
    volatile uint32_t         ioctl_i2c1_data;              //0x0224
    volatile uint32_t         ioctl_i2c2_clk;               //0x0228
    volatile uint32_t         ioctl_i2c2_data;              //0x022C
    volatile uint32_t         ioctl_ssp0_clk;               //0x0230
    volatile uint32_t         ioctl_ssp0_cs0;               //0x0234
    volatile uint32_t         ioctl_ssp0_cs1;               //0x0238
    volatile uint32_t         ioctl_ssp0_di;                //0x023C
    volatile uint32_t         ioctl_ssp0_do;                //0x0240
    volatile uint32_t         ioctl_ssp1_clk;               //0x0244
    volatile uint32_t         ioctl_ssp1_cs;                //0x0248
    volatile uint32_t         ioctl_ssp1_di;                //0x024C
    volatile uint32_t         ioctl_ssp1_do;                //0x0250
    volatile uint32_t         ioctl_ssp1_dcx;               //0x0254
    volatile uint32_t         ioctl_mcu_jtag_trst_n;        //0x0258
    volatile uint32_t         ioctl_mcu_jtag_tdi;           //0x025C
    volatile uint32_t         ioctl_mcu_jtag_tms;           //0x0260
    volatile uint32_t         ioctl_mcu_jtag_tck;           //0x0264
    volatile uint32_t         ioctl_uart0_tx;               //0x0268
    volatile uint32_t         ioctl_uart0_rx;               //0x026C
    volatile uint32_t         ioctl_dsp_jtag_trst_n;        //0x0270
    volatile uint32_t         ioctl_dsp_jtag_tdi;           //0x0274
    volatile uint32_t         ioctl_dsp_jtag_tdo;           //0x0278
    volatile uint32_t         ioctl_dsp_jtag_tms;           //0x027C
    volatile uint32_t         ioctl_dsp_jtag_tck;           //0x0280
    volatile uint32_t         ioctl_mcu_trace_clk;          //0x0284
    volatile uint32_t         ioctl_mcu_trace_data0;        //0x0288
    volatile uint32_t         ioctl_mcu_trace_data1;        //0x028C
    volatile uint32_t         ioctl_mcu_trace_data2;        //0x0290
    volatile uint32_t         ioctl_mcu_trace_data3;        //0x0294
    volatile uint32_t         ioctl_uart1_ri;               //0x0298
    volatile uint32_t         ioctl_sd1_d3;                 //0x029C
    volatile uint32_t         ioctl_sd1_d2;                 //0x02A0
    volatile uint32_t         ioctl_sd1_d1;                 //0x02A4
    volatile uint32_t         ioctl_sd1_d0;                 //0x02A8
    volatile uint32_t         ioctl_sd1_cmd;                //0x02AC
    volatile uint32_t         ioctl_sd1_clk;                //0x02B0
    volatile uint32_t         ioctl_sd0_d3;                 //0x02B4
    volatile uint32_t         ioctl_sd0_d2;                 //0x02B8
    volatile uint32_t         ioctl_sd0_d1;                 //0x02BC
    volatile uint32_t         ioctl_sd0_d0;                 //0x02C0
    volatile uint32_t         ioctl_sd0_cmd;                //0x02C4
    volatile uint32_t         ioctl_sd0_clk;                //0x02C8
    volatile uint32_t         ioctl_sd0_card_pwn;           //0x02CC
    volatile uint32_t         ioctl_sd0_card_det;           //0x02D0
    volatile uint32_t         ioctl_mcu_jtag_tdo;           //0x02D4
    volatile uint32_t         ioctl_pwm0;                   //0x02D8
    volatile uint32_t         ioctl_pwm1;                   //0x02DC
    volatile uint32_t         ioctl_dpi_pclki;              //0x02E0
    volatile uint32_t         ioctl_dpi_vsi;                //0x02E4
    volatile uint32_t         ioctl_dpi_hsi;                //0x02E8
    volatile uint32_t         ioctl_dpi_dei;                //0x02EC
    volatile uint32_t         ioctl_dpi_datai0;             //0x02F0
    volatile uint32_t         ioctl_dpi_datai1;             //0x02F4
    volatile uint32_t         ioctl_dpi_datai2;             //0x02F8
    volatile uint32_t         ioctl_dpi_datai3;             //0x02FC
    volatile uint32_t         ioctl_dpi_datai4;             //0x0300
    volatile uint32_t         ioctl_dpi_datai5;             //0x0304
    volatile uint32_t         ioctl_dpi_datai6;             //0x0308
    volatile uint32_t         ioctl_dpi_datai7;             //0x030C
    volatile uint32_t         ioctl_dpi_datai8;             //0x0310
    volatile uint32_t         ioctl_dpi_datai9;             //0x0314
    volatile uint32_t         ioctl_dpi_datai10;            //0x0318
    volatile uint32_t         ioctl_dpi_datai11;            //0x031C
    volatile uint32_t         ioctl_dpi_datai12;            //0x0320
    volatile uint32_t         ioctl_dpi_datai13;            //0x0324
    volatile uint32_t         ioctl_dpi_datai14;            //0x0328
    volatile uint32_t         ioctl_dpi_datai15;            //0x032C
    volatile uint32_t         ioctl_dpi_pclko;              //0x0330
    volatile uint32_t         ioctl_dpi_vso;                //0x0334
    volatile uint32_t         ioctl_dpi_hso;                //0x0338
    volatile uint32_t         ioctl_dpi_deo;                //0x033C
    volatile uint32_t         ioctl_dpi_datao0;             //0x0340
    volatile uint32_t         ioctl_dpi_datao1;             //0x0344
    volatile uint32_t         ioctl_dpi_datao2;             //0x0348
    volatile uint32_t         ioctl_dpi_datao3;             //0x034C
    volatile uint32_t         ioctl_dpi_datao4;             //0x0350
    volatile uint32_t         ioctl_dpi_datao5;             //0x0354
    volatile uint32_t         ioctl_dpi_datao6;             //0x0358
    volatile uint32_t         ioctl_dpi_datao7;             //0x035C
    volatile uint32_t         ioctl_dpi_datao8;             //0x0360
    volatile uint32_t         ioctl_dpi_datao9;             //0x0364
    volatile uint32_t         ioctl_dpi_datao10;            //0x0368
    volatile uint32_t         ioctl_dpi_datao11;            //0x036C
    volatile uint32_t         ioctl_usb3_vbus;              //0x0370
    volatile uint32_t         ioctl_usb3_drv_vbus;          //0x0374
    volatile uint32_t         rsvd8;                        //0x0378
    volatile uint32_t         ioctl_usb2_vbus;              //0x037C
    volatile uint32_t         ioctl_usb2_drv_vbus;          //0x0380
    volatile uint32_t         ioctl_mclk;                   //0x0384
    /* To Do */

} kdp720_scu_extreg;

#define regSCUEXT  ((kdp720_scu_extreg*) SCU_EXT_REG_BASE)


//PLL setting and lock time register.
typedef volatile union U_regPLLnSetting
{
    struct
    {
        uint32_t PLL1_SET;              //PLL1 Setting Register   (Offset: 0x0008)
        uint32_t PLL1_TIMELOCK;         //PLL1 Lock Time Register (Offset: 0x000C)
        uint32_t PLL2_SET;              //PLL2 Setting Register   (Offset: 0x0010)
        uint32_t PLL2_TIMELOCK;         //PLL2 Lock Time Register (Offset: 0x0014)
        uint32_t PLL3_SET;              //PLL3 Setting Register   (Offset: 0x0018)
        uint32_t PLL3_TIMELOCK;         //PLL3 Lock Time Register (Offset: 0x001C)
        uint32_t PLL4_SET;              //PLL4 Setting Register   (Offset: 0x0020)
        uint32_t PLL4_TIMELOCK;         //PLL4 Lock Time Register (Offset: 0x0024)
        uint32_t PLL5_SET;              //PLL5 Setting Register   (Offset: 0x0028)
        uint32_t PLL5_TIMELOCK;         //PLL5 Lock Time Register (Offset: 0x002C)
        uint32_t PLL6_SET;              //PLL6 Setting Register   (Offset: 0x0030)
        uint32_t PLL6_TIMELOCK;         //PLL6 Lock Time Register (Offset: 0x0034)
    }dw;    //double word

    struct
    {
        //PLL1 Setting Register   (Offset: 0x0008)
        uint32_t PLL1_EN            : 1;
        uint32_t PLL1_RSVD_0        : 7;
        uint32_t PLL1_IS            : 2;
        uint32_t PLL1_RSVD_1        : 2;
        uint32_t PLL1_PS            : 4;
        uint32_t PLL1_MS            : 3;
        uint32_t PLL1_RSVD_2        : 1;
        uint32_t PLL1_NS            : 9;
        uint32_t PLL1_RSVD_3        : 3;
        //PLL1 Lock Time Register (Offset: 0x000C)
        uint32_t PLL1_TMR           :31;
        uint32_t PLL1_LOCK          : 1;

        //PLL2 Setting Register   (Offset: 0x0010)
        uint32_t PLL2_EN            : 1;
        uint32_t PLL2_RSVD_0        : 7;
        uint32_t PLL2_IS            : 2;
        uint32_t PLL2_RSVD_1        : 2;
        uint32_t PLL2_PS            : 4;
        uint32_t PLL2_MS            : 3;
        uint32_t PLL2_RSVD_2        : 1;
        uint32_t PLL2_NS            : 9;
        uint32_t PLL2_RSVD_3        : 3;
        //PLL2 Lock Time Register (Offset: 0x0014)
        uint32_t PLL2_TMR           :31;
        uint32_t PLL2_LOCK          : 1;

        //PLL3 Setting Register   (Offset: 0x0018)
        uint32_t PLL3_EN            : 1;
        uint32_t PLL3_RSVD_0        : 7;
        uint32_t PLL3_IS            : 2;
        uint32_t PLL3_RSVD_1        : 2;
        uint32_t PLL3_PS            : 4;
        uint32_t PLL3_MS            : 3;
        uint32_t PLL3_RSVD_2        : 1;
        uint32_t PLL3_NS            : 9;
        uint32_t PLL3_RSVD_3        : 3;
        //PLL3 Lock Time Register (Offset: 0x001C)
        uint32_t PLL3_TMR           :31;
        uint32_t PLL3_LOCK          : 1;

        //PLL4 Setting Register   (Offset: 0x0020)
        uint32_t PLL4_EN            : 1;
        uint32_t PLL4_RSVD_0        : 7;
        uint32_t PLL4_IS            : 2;
        uint32_t PLL4_RSVD_1        : 2;
        uint32_t PLL4_PS            : 4;
        uint32_t PLL4_MS            : 3;
        uint32_t PLL4_RSVD_2        : 1;
        uint32_t PLL4_NS            : 9;
        uint32_t PLL4_RSVD_3        : 3;
        //PLL4 Lock Time Register (Offset: 0x0024)
        uint32_t PLL4_TMR           :31;
        uint32_t PLL4_LOCK          : 1;

        //PLL5 Setting Register   (Offset: 0x0028)
        uint32_t PLL5_EN            : 1;
        uint32_t PLL5_RSVD_0        : 7;
        uint32_t PLL5_IS            : 2;
        uint32_t PLL5_RSVD_1        : 2;
        uint32_t PLL5_PS            : 4;
        uint32_t PLL5_MS            : 3;
        uint32_t PLL5_RSVD_2        : 1;
        uint32_t PLL5_NS            : 9;
        uint32_t PLL5_RSVD_3        : 3;
        //PLL5 Lock Time Register (Offset: 0x002C)
        uint32_t PLL5_TMR           :31;
        uint32_t PLL5_LOCK          : 1;

        //PLL6 Setting Register   (Offset: 0x0030)
        uint32_t PLL6_EN            : 1;
        uint32_t PLL6_RSVD_0        : 7;
        uint32_t PLL6_IS            : 2;
        uint32_t PLL6_RSVD_1        : 2;
        uint32_t PLL6_PS            : 4;
        uint32_t PLL6_MS            : 3;
        uint32_t PLL6_RSVD_2        : 1;
        uint32_t PLL6_NS            : 9;
        uint32_t PLL6_RSVD_3        : 3;
        //PLL6 Lock Time Register (Offset: 0x0034)
        uint32_t PLL6_TMR           :31;
        uint32_t PLL6_LOCK          : 1;
    }bf;    //bit-field
}U_regPLLnSetting;

#define REG_SCUEXT_PLLNSET      ((U_regPLLnSetting*) (SCU_EXT_REG_BASE + 0x0008))
#define PLL_(a, b)              REG_SCUEXT_PLLNSET->dw.PLL##a##_SET = b
#define SET_PLL(a, b)           PLL_(a,b)
#define SET_PLL_TIMER_(a, b)    REG_SCUEXT_PLLNSET->bf.PLL##a##_TMR = b
#define SET_PLL_TIMER(a, b)     SET_PLL_TIMER_(a,b)

#define SET_PLL_EN(a, b)        REG_SCUEXT_PLLNSET->bf.PLL##a##_EN = b
#define SET_PLL_DDR_EN(b)       REG_SCUEXT_PLLNSET->bf.PLL1_EN = b
#define SET_PLL_MRX1_EN(b)      REG_SCUEXT_PLLNSET->bf.PLL2_EN = b
#define SET_PLL_MRX0_EN(b)      REG_SCUEXT_PLLNSET->bf.PLL3_EN = b
#define SET_PLL_NPU_EN(b)       REG_SCUEXT_PLLNSET->bf.PLL4_EN = b
#define SET_PLL_DSP_EN(b)       REG_SCUEXT_PLLNSET->bf.PLL5_EN = b
#define SET_PLL_ADO_EN(b)       REG_SCUEXT_PLLNSET->bf.PLL6_EN = b

#define PLL_LOCK_STAT(a)        REG_SCUEXT_PLLNSET->bf.PLL##a##_LOCK
#define PLL_DDR_LOCK_STAT       REG_SCUEXT_PLLNSET->bf.PLL1_LOCK
#define PLL_MRX1_LOCK_STAT      REG_SCUEXT_PLLNSET->bf.PLL2_LOCK
#define PLL_MRX0_LOCK_STAT      REG_SCUEXT_PLLNSET->bf.PLL3_LOCK
#define PLL_NPU_LOCK_STAT       REG_SCUEXT_PLLNSET->bf.PLL4_LOCK
#define PLL_DSP_LOCK_STAT       REG_SCUEXT_PLLNSET->bf.PLL5_LOCK
#define PLL_ADO_LOCK_STAT       REG_SCUEXT_PLLNSET->bf.PLL6_LOCK

#define PLL_TIME_STAT(a, b)     REG_SCUEXT_PLLNSET->bf.PLL##a##_TIMER = b

//Clock enable
typedef volatile union U_regCLKEN
{
    struct
    {
        uint32_t CLK_EN0;       //Clock Enable Register 0 (Offset: 0x0044)
        uint32_t CLK_EN1;       //Clock Enable Register 1 (Offset: 0x0048)
    }dw;    //double word

    struct
    {
        uint32_t UART0_CLK_EN           :1;                     //uart0_uclk clock enable control.
        uint32_t UART1_CLK_EN           :1;                     //uart1_uclk clock enable control.
        uint32_t EFUSE_CLK_EN           :1;                     //efuse_clk clock enable control.
        uint32_t EFUSE_128_CLK_EN       :1;                     //efuse_128_clk clock enable control.
        uint32_t DSP_GATE_EN            :1;                     //dsp_clk_mux_out clock enable control.
        uint32_t NPU_CLK_EN             :1;                     //npu_clk_mux_out clock enable control.
        uint32_t CSIRX1_ESC_CLK_EN      :1;                     //csirx1_EscClk clock enable control.
        uint32_t CSIRX1_CSI_CLK_EN      :1;                     //csirx1_csi_clk clock enable control.
        uint32_t CSIRX1_VC_PCLK_EN      :1;                     //csirx1_vc_pclk clock enable control.
        uint32_t CSIRX0_ESC_CLK_EN      :1;                     //csirx0_EscClk clock enable control.
        uint32_t CSIRX0_CSI_CLK_EN      :1;                     //csirx0_csi_clk clock enable control.
        uint32_t CSIRX0_VC_PCLK_EN      :1;                     //csirx0_vc_pclk clock enable control.
        uint32_t WDT_EXT_CLK_EN         :1;                     //wdt_extclk clock enable control.
        uint32_t TMR_EXT1_CLK_EN        :1;                     //tmr_extclk1 clock enable control.
        uint32_t TMR_EXT2_CLK_EN        :1;                     //tmr_extclk2 clock enable control.
        uint32_t TMR_EXT3_CLK_EN        :1;                     //tmr_extclk3 clock enable control.
        uint32_t SPI_CLK_EN             :1;                     //spi_clk clock enable control.
        uint32_t SSP0_CLK_EN            :1;                     //ssp0_sspclk clock enable control.
        uint32_t SSP1_CLK_EN            :1;                     //ssp1_sspclk clock enable control.
        uint32_t PLL4_NPU_MUX_EN        :1;                     //npu_clk clock enable control.
        uint32_t PLL5_DSP_MUX_EN        :1;                     //dsp_clk clock enable control.
        uint32_t PWM_EXT1_CLK_EN        :1;                     //pwm_extclk2 clock enable control.
        uint32_t PWM_EXT2_CLK_EN        :1;                     //pwm_extclk2 clock enable control.
        uint32_t SDC_SDCLK1X_CLK_EN     :1;                     //sdc_sdclk1x_g clock enable control.
        uint32_t SDC_SDCLK2X_CLK_EN     :1;                     //sdc_sdclk2x_g clock enable control.
        uint32_t SDC_U1_SDCLK1X_CLK_EN  :1;                     //sdc_u1_sdclk1x_g clock enable control.
        uint32_t SDC_U1_SDCLK2X_CLK_EN  :1;                     //sdc_u1_sdclk2x_g clock enable control.
        uint32_t I2S_MCLK_EN            :1;                     //i2s_mclk clock enable control.
        uint32_t TDC_XCLK_EN            :1;                     //tdc_xclk clock enable control.
        uint32_t LC_SCALER_CLK_EN       :1;                     //lc_scale_clk clock enable control.
        uint32_t LC_CLK_EN              :1;                     //lc_clk clock enable control.
        uint32_t U3_CLK60_EN            :1;                     //u3_clk60_u0 clock enable control.

        uint32_t AXI_SRC_CLK_EN         :1;                     //axi_src_clk clock enable control.
        uint32_t RSVD31                 :31;

    }bf;    //bit-field
}U_regCLKEN;
#define REG_BUS_CLKEN  ((U_regCLKEN*) (SCU_EXT_REG_BASE + 0x0044))
#define SET_CLOCK_EN(a, b) REG_BUS_CLKEN->bf.a = b
#define SET_CLOCK_EN0(a) REG_BUS_CLKEN->dw.CLK_EN0 = a
#define SET_CLOCK_EN1(a) REG_BUS_CLKEN->dw.CLK_EN1 = a

//Clock mux select
typedef volatile union U_regCLKMUX
{
    struct
    {
        uint32_t CLK_MUX_SEL;       //Clock Mux Selection Register      (Offset: 0x0054)
    }dw;    //double word

    struct
    {
        uint32_t UART0_CLK_SEL          : 1;
        uint32_t UART1_CLK_SEL          : 1;
        uint32_t DSP_CLK_SEL            : 1;
        uint32_t NPU_CLK_SEL            : 1;
        uint32_t SSP_CLK_SEL            : 1;
        uint32_t RSVD                   :27;
    }bf;    //bit-field
}U_regCLKMUX;
#define REG_CLK_MUX_SEL     ((U_regCLKMUX*) (SCU_EXT_REG_BASE + 0x0054))
#define SET_CLOCK_MUX(a)    REG_CLK_MUX_SEL->dw.CLK_MUX_SEL = a
#define SET_CLOCK_MUX_(a,b) REG_CLK_MUX_SEL->bf.a = b

// Divider
typedef volatile union U_regCLKDVIDER
{
    struct
    {
        uint32_t CLK_DIVIDER0;      //Clock Divider Register 0 (Offset: 0x0058)
        uint32_t CLK_DIVIDER1;      //Clock Divider Register 1 (Offset: 0x005C)
        uint32_t CLK_DIVIDER2;      //Clock Divider Register 2 (Offset: 0x0060)
        uint32_t CLK_DIVIDER3;      //Clock Divider Register 3 (Offset: 0x0064)
        uint32_t CLK_DIVIDER4;      //Clock Divider Register 4 (Offset: 0x0068)
        uint32_t CLK_DIVIDER5;      //Clock Divider Register 5 (Offset: 0x006C)
        uint32_t CLK_DIVIDER6;      //Clock Divider Register 6 (Offset: 0x0070)
    }dw;    //double word

    struct
    {
        //Clock Divider Register 0 (Offset: 0x0058)
        uint32_t PLL2_CSI1_DIV_FACTOR               : 5;
        uint32_t RSVD_0                             : 3;
        uint32_t PLL2_CSI1_VC_DIV_FACTOR            : 5;
        uint32_t RSVD_1                             : 7;
        uint32_t PLL2_ESC1_DIV_FACTOR               : 6;
        uint32_t RSVD_2                             : 6;

        //Clock Divider Register 1 (Offset: 0x005C)
        uint32_t PLL3_CSI0_DIV_FACTOR               : 5;
        uint32_t RSVD_3                             : 3;
        uint32_t PLL3_CSI0_VC_DIV_FACTOR            : 5;
        uint32_t RSVD_4                             : 7;
        uint32_t PLL3_ESC0_DIV_FACTOR               : 6;
        uint32_t RSVD_6                             : 6;

        //Clock Divider Register 2 (Offset: 0x0060)
        uint32_t PLL5_DSP_DIV_FACTOR                : 4;
        uint32_t DSP_DIV_FACTOR                     : 5;
        uint32_t RSVD_7                             : 7;
        uint32_t PLL4_NPU_DIV_FACTOR                : 4;
        uint32_t NPU_DIV_FACTOR                     : 5;
        uint32_t RSVD_8                             : 7;

        //Clock Divider Register 3 (Offset: 0x0064)
        uint32_t PRE_TMR_WDT_UART_DIV_FACTOR        : 5;
        uint32_t RSVD_9                             : 3;
        uint32_t SPI_DIV_FACTOR                     : 5;
        uint32_t RSVD_10                            : 3;
        uint32_t SSP0_DIV_FACTOR                    : 5;
        uint32_t RSVD_11                            : 3;
        uint32_t SSP1_DIV_FACTOR                    : 5;
        uint32_t RSVD_12                            : 3;

        //Clock Divider Register 4 (Offset: 0x0068)
        uint32_t PCLK_DIV_FACTOR                    : 5;
        uint32_t RSVD_13                            : 3;
        uint32_t LC_SCALER_DIV_FACTOR               : 4;
        uint32_t RSVD_14                            : 4;
        uint32_t TDC_XCLK_DIV_FACTOR                : 5;
        uint32_t RSVD_15                            : 3;
        uint32_t WDT_DIV_FACTOR                     : 5;
        uint32_t RSVD_16                            : 3;

        //Clock Divider Register 5 (Offset: 0x006C)
        uint32_t SDC_U1_DIV_FACTOR                  : 4;
        uint32_t SDC_DIV_FACTOR                     : 4;
        uint32_t I2S_MCLK_DIV_FACTOR                : 7;
        uint32_t RSVD_17                            : 1;
        uint32_t U3_CLK60_DIV_FACTOR                : 5;
        uint32_t RSVD_18                            : 3;
        uint32_t LC_CLK_DIV_FACTOR                  : 7;
        uint32_t RSVD_19                            : 1;

        //Clock Divider Register 6 (Offset: 0x0070)
        uint32_t PLL6_DIV_FACTOR                    : 7;
        uint32_t RSVD_20                            : 1;
        uint32_t UART0_DIV_FACTOR                   : 5;
        uint32_t RSVD_21                            :19;
    }bf;    //bit-field
}U_regCLKDVIDER;
#define REG_CLK_DIVIDER             ((U_regCLKDVIDER*) (SCU_EXT_REG_BASE + 0x0058))
#define SET_CLOCK_DIVIDER(a, b)     REG_CLK_DIVIDER->bf.a = b
#define SET_MRX1_CLOCK_DIVIDER(b)   REG_CLK_DIVIDER->dw.CLK_DIVIDER0 = b
#define SET_MRX0_CLOCK_DIVIDER(b)   REG_CLK_DIVIDER->dw.CLK_DIVIDER1 = b

#define SET_MRX1_VC_CLK_DIVIDER(b)  REG_CLK_DIVIDER->bf.PLL2_CSI1_VC_DIV_FACTOR = b
#define SET_MRX1_CSI_CLK_DIVIDER(b) REG_CLK_DIVIDER->bf.PLL2_CSI1_DIV_FACTOR    = b
#define SET_MRX1_EXC_CLK_DIVIDER(b) REG_CLK_DIVIDER->bf.PLL2_ESC1_DIV_FACTOR    = b

#define SET_MRX0_VC_CLK_DIVIDER(b)  REG_CLK_DIVIDER->bf.PLL3_CSI0_VC_DIV_FACTOR = b
#define SET_MRX0_CSI_CLK_DIVIDER(b) REG_CLK_DIVIDER->bf.PLL3_CSI0_DIV_FACTOR    = b
#define SET_MRX0_EXC_CLK_DIVIDER(b) REG_CLK_DIVIDER->bf.PLL3_ESC0_DIV_FACTOR    = b

#define SET_NPU_CLOCK_DIVIDER(b)    REG_CLK_DIVIDER->bf.PLL4_NPU_DIV_FACTOR = b
#define SET_DSP_CLOCK_DIVIDER(b)    REG_CLK_DIVIDER->bf.PLL5_DSP_DIV_FACTOR = b
#define SET_ADO_CLOCK_DIVIDER(b)    REG_CLK_DIVIDER->bf.I2S_MCLK_DIV_FACTOR = b


//SW Resets
typedef volatile union U_regRST
{
    struct
    {
        uint32_t PDBUS_RST;    //Power Domain Software Reset Register   (Offset: 0x00A0)
        uint32_t IP_RST;       //IP Software Reset Register             (Offset: 0x00A4)
    }dw;    //double word

    struct
    {
        uint32_t PD_UDR                 : 1;
        uint32_t PD_UHO                 : 1;
        uint32_t PD_MRX                 : 1;
        uint32_t PD_NOM                 : 1;
        uint32_t PD_NPU                 : 1;
        uint32_t RSVD_0                 : 3;
        uint32_t BUS_NOM_AXI            : 1;
        uint32_t BUS_NOM_APB_0          : 1;
        uint32_t BUS_NOM_ABP_1          : 1;
        uint32_t RSVD_1                 : 5;
        uint32_t GLOBAL                 : 1;
        uint32_t RSVD_2                 :15;

        uint32_t BP010_12X14            : 1;
        uint32_t CRYPTOMASTER_AHB_BEE   : 1;
        uint32_t FAHB2ERAM_RET_16B      : 1;
        uint32_t IFTAHB2RAM_128KB       : 1;
        uint32_t DFTAHB2RAM_128KB       : 1;
        uint32_t FTAHB2ROM              : 1;
        uint32_t FTAXIC030              : 1;
        uint32_t FTDMAC020              : 1;
        uint32_t FTDMAC030              : 1;
        uint32_t FTGPIO010              : 1;
        uint32_t FTIIC010_U0            : 1;
        uint32_t FTIIC010_U1            : 1;
        uint32_t FTIIC010_U2            : 1;
        uint32_t FTLCDC210              : 1;
        uint32_t FTPWMTMR010            : 1;
        uint32_t FTSDC021               : 1;
        uint32_t FTSDC021_U1            : 1;
        uint32_t FTSPI020               : 1;
        uint32_t FTSPI2AHBL             : 1;
        uint32_t FTTDCC010              : 1;
        uint32_t FTSSP010_U0            : 1;
        uint32_t FTSSP010_U1            : 1;
        uint32_t FTUART010_U0           : 1;
        uint32_t FTUART010_U1           : 1;
        uint32_t FTWDT010               : 1;
        uint32_t FTTMR010               : 1;
        uint32_t NPU                    : 1;
        uint32_t VP6                    : 1;
        uint32_t FTRSC030_U0            : 1;
        uint32_t FTRSC030_U1            : 1;
        uint32_t FTRSC030_U2            : 1;
        uint32_t FTRSC030_U3            : 1;
    }bf;    //bit-field
}U_regRST;
#define REG_RST             ((U_regRST*) (SCU_EXT_REG_BASE + 0x00A0))
#define SET_SWRST(a, b)     REG_RST->bf.a = b

//MIPI CSIRX CTRL Reg
typedef volatile union U_regCSIRX_CTRL
{
    struct
    {
        uint32_t CSIRX_CTL_0;           //MIPI CSIRX CTRL Register 0 (Offset: 0x0114)
        uint32_t CSIRX_CTL_1;           //MIPI CSIRX CTRL Register 0 (Offset: 0x0118)
    }dw;    //double word

    struct
    {
        uint32_t CSIRX_ENABLE_0         : 1;
        uint32_t RSVD_0                 : 1;
        uint32_t PPI_ENABLE_0           : 2;
        uint32_t SYS_RST_N_0            : 1;
        uint32_t PWR_RST_N_0            : 1;
        uint32_t APB_RST_N_0            : 1;
        uint32_t RSVD_1                 : 1;
        uint32_t PHY_SYS_RST_N_0        : 1;
        uint32_t PHY_PWR_RST_N_0        : 1;
        uint32_t PHY_APB_RST_N_0        : 1;
        uint32_t RSVD_2                 :21;
        
        uint32_t CSIRX_ENABLE_1         : 1;
        uint32_t RSVD_3                 : 1;
        uint32_t PPI_ENABLE_1           : 2;
        uint32_t SYS_RST_N_1            : 1;
        uint32_t PWR_RST_N_1            : 1;
        uint32_t APB_RST_N_1            : 1;
        uint32_t RSVD_4                 : 1;
        uint32_t PHY_SYS_RST_N_1        : 1;
        uint32_t PHY_PWR_RST_N_1        : 1;
        uint32_t PHY_APB_RST_N_1        : 1;
        uint32_t RSVD_5                 :21; 
    }bf;    //bit-field
}U_regCSIRX_CTRL;
#define REG_CSIRX_CTRL          ((U_regCSIRX_CTRL*) (SCU_EXT_REG_BASE + 0x0114))
#define SET_CSIRX_CTRL(a, b)    REG_CSIRX_CTRL->bf.a = b


//MISC Resets
typedef volatile union U_regMISC
{
    struct
    {
        uint32_t MISC;              //MISC Register (Offset: 0x0144)
    }dw;    //double word

    struct
    {
        uint32_t EFUSE_128B_MR          : 1;
        uint32_t EFUSE_512B_MR          : 1;
        uint32_t RSVD_0                 : 6;
        uint32_t DPI_MUX_SEL_0          : 1;
        uint32_t DPI_MUX_SEL_1          : 1;
        uint32_t RSVD_1                 : 6;
        uint32_t LS3V_BK_0              : 1;
        uint32_t RSVD_2                 : 2;
        uint32_t LS3V_BK_3              : 1;
        uint32_t RSVD_3                 : 12;
    }bf;    //bit-field
}U_regMISC;
#define REG_MISC            ((U_regMISC*) (SCU_EXT_REG_BASE + 0x0144))
#define SET_MISC(a, b)      REG_MISC->bf.a = b

/* kdp720_scu_extreg.clock_enable_reg0, clock enable register 0, 0x0044 */
#define CLK_REG0_UART0_CLK_EN           BIT(0)
#define CLK_REG0_UART1_CLK_EN           BIT(1)
#define CLK_REG0_EFUSE_CLK_EN           BIT(2)
#define CLK_REG0_EFUSE_128_CLK_EN       BIT(3)
#define CLK_REG0_DSP_GATE_EN            BIT(4)
#define CLK_REG0_NPU_CLK_EN             BIT(5)
#define CLK_REG0_CSIRX1_ESC_CLK_EN      BIT(6)
#define CLK_REG0_CSIRX1_CSI_CLK_EN      BIT(7)
#define CLK_REG0_CSIRX1_VC_PCLK_EN      BIT(8)
#define CLK_REG0_CSIRX0_ESC_CLK_EN      BIT(9)
#define CLK_REG0_CSIRX0_CSI_CLK_EN      BIT(10)
#define CLK_REG0_CSIRX0_VC_PCLK_EN      BIT(11)
#define CLK_REG0_WDT_EXT_CLK_EN         BIT(12)
#define CLK_REG0_TMR_EXT1_CLK_EN        BIT(13)
#define CLK_REG0_TMR_EXT2_CLK_EN        BIT(14)
#define CLK_REG0_TMR_EXT3_CLK_EN        BIT(15)
#define CLK_REG0_SPI_CLK_EN             BIT(16)
#define CLK_REG0_SSP0_CLK_EN            BIT(17)
#define CLK_REG0_SSP1_CLK_EN            BIT(18)
#define CLK_REG0_PLL4_NPU_MUX_EN        BIT(19)
#define CLK_REG0_PLL5_DSP_MUX_EN        BIT(20)
#define CLK_REG0_PWM_EXT1_CLK_EN        BIT(21)
#define CLK_REG0_PWM_EXT2_CLK_EN        BIT(22)
#define CLK_REG0_SDC_SDCLK1X_CLK_EN     BIT(23)
#define CLK_REG0_SDC_SDCLK2X_CLK_EN     BIT(24)
#define CLK_REG0_SDC_U1_SDCLK1X_CLK_EN  BIT(25)
#define CLK_REG0_SDC_U1_SDCLK2X_CLK_EN  BIT(26)
#define CLK_REG0_I2S_MCLK_EN            BIT(27)
#define CLK_REG0_TDC_XCLK_EN            BIT(28)
#define CLK_REG0_LC_SCALER_CLK_EN       BIT(29)
#define CLK_REG0_LC_CLK_EN              BIT(30)
#define CLK_REG0_U3_CLK60_EN            BIT(31)


/* kdp720_scu_extreg.clock_div4, clock divider 4, 0x0068 */
#define CLK_DIV4_PCKL_DIV_FACTOR_SHIFT      0
#define CLK_DIV4_PCKL_DIV_FACTOR_MASK       ((0x1F) << CLK_DIV4_PCKL_DIV_FACTOR_SHIFT)
#define CLK_DIV4_LC_SCALER_DIV_FACTOR_SHIFT 8
#define CLK_DIV4_LC_SCALER_DIV_FACTOR_MASK  ((0x0F) << CLK_DIV4_LC_SCALER_DIV_FACTOR_SHIFT)
#define CLK_DIV4_TDC_XCLK_DIV_FACTOR_SHIFT  16
#define CLK_DIV4_TDC_XCLK_DIV_FACTOR_MASK   ((0x1F) << CLK_DIV4_TDC_XCLK_DIV_FACTOR_SHIFT)
#define CLK_DIV4_WDT_DIV_FACTOR_SHIFT       24
#define CLK_DIV4_WDT_DIV_FACTOR_MASK        ((0x1F) << CLK_DIV4_WDT_DIV_FACTOR_SHIFT)

#endif //__KDRV_SCU_EXTREG_H__
/** @}*/

