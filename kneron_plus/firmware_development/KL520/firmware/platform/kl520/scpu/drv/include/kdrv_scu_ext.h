#ifndef __SCU_EXTREG_H__
#define __SCU_EXTREG_H__

#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "base.h"
#include "io.h"


/* I/O pinmux mode */
enum {
    PINMUX_MODE0 = 0,
    PINMUX_MODE1,
    PINMUX_MODE2,
    PINMUX_MODE3,
    PINMUX_MODE4,
    PINMUX_MODE5,
    PINMUX_MODE6,
    PINMUX_MODE7, 
};


#define PINMUX_SET(reg, mode)                       outw(reg, ((inw(reg) & ~(BIT(2) | BIT(1) | BIT(0))) | mode))
#define PINMUX_CLR(reg)                             outw(reg, (inw(reg) & ~(BIT(2) | BIT(1) | BIT(0))))


#define SCU_EXTREG_PLL0_SETTING                             (SCU_EXTREG_PA_BASE + 0x0000)
#define SCU_EXTREG_PLL1_SETTING                             (SCU_EXTREG_PA_BASE + 0x0004)
#define SCU_EXTREG_PLL2_SETTING                             (SCU_EXTREG_PA_BASE + 0x0008)
#define SCU_EXTREG_PLL3_SETTING                             (SCU_EXTREG_PA_BASE + 0x000C)
#define SCU_EXTREG_PLL4_SETTING                             (SCU_EXTREG_PA_BASE + 0x0010)
#define SCU_EXTREG_PLL5_SETTING                             (SCU_EXTREG_PA_BASE + 0x003C)
#define SCU_EXTREG_CLK_EN0                                  (SCU_EXTREG_PA_BASE + 0x0014)
#define SCU_EXTREG_CLK_EN1                                  (SCU_EXTREG_PA_BASE + 0x0018)
#define SCU_EXTREG_CLK_EN2                                  (SCU_EXTREG_PA_BASE + 0x001C)
#define SCU_EXTREG_CLK_MUX_SEL                              (SCU_EXTREG_PA_BASE + 0x0020)
#define SCU_EXTREG_CLK_DIV0                                 (SCU_EXTREG_PA_BASE + 0x0024)
#define SCU_EXTREG_CLK_DIV1                                 (SCU_EXTREG_PA_BASE + 0x0028)
#define SCU_EXTREG_CLK_DIV2                                 (SCU_EXTREG_PA_BASE + 0x002C)
#define SCU_EXTREG_CLK_DIV3                                 (SCU_EXTREG_PA_BASE + 0x0030)
#define SCU_EXTREG_CLK_DIV4                                 (SCU_EXTREG_PA_BASE + 0x0034)
#define SCU_EXTREG_CLK_DIV5                                 (SCU_EXTREG_PA_BASE + 0x0038)
#define SCU_EXTREG_SWRST_MASK0                              (SCU_EXTREG_PA_BASE + 0x0040)
#define SCU_EXTREG_SWRST_MASK1                              (SCU_EXTREG_PA_BASE + 0x0044)
#define SCU_EXTREG_SWRST_MASK2                              (SCU_EXTREG_PA_BASE + 0x0048)
#define SCU_EXTREG_SWRST                                    (SCU_EXTREG_PA_BASE + 0x004C)
#define SCU_EXTREG_CM4_NCPU_CTRL                            (SCU_EXTREG_PA_BASE + 0x0068)
#define SCU_EXTREG_DDR_CTRL                                 (SCU_EXTREG_PA_BASE + 0x0080)
#define SCU_EXTREG_USB_OTG_CTRL                             (SCU_EXTREG_PA_BASE + 0x008C)
#define SCU_EXTREG_CSIRX_CTRL0                              (SCU_EXTREG_PA_BASE + 0x0090)
#define SCU_EXTREG_CSIRX_CTRL1                              (SCU_EXTREG_PA_BASE + 0x0094)
#define SCU_EXTREG_DPI2AHB_CTRL                             (SCU_EXTREG_PA_BASE + 0x009C)
#define SCU_EXTREG_BONDING_OPTION                           (SCU_EXTREG_PA_BASE + 0x00A0)
#define SCU_EXTREG_SD_CTRL                                  (SCU_EXTREG_PA_BASE + 0x00A4)
#define SCU_EXTREG_ADC_CTRL                                 (SCU_EXTREG_PA_BASE + 0x00A8)
#define SCU_EXTREG_MISC                                     (SCU_EXTREG_PA_BASE + 0x00B0)
#define SCU_EXTREG_CLK_DIV6                                 (SCU_EXTREG_PA_BASE + 0x00D0)
#define SCU_EXTREG_CLK_DIV7                                 (SCU_EXTREG_PA_BASE + 0x00D4)
#define SCU_EXTREG_SPI_CS_N                                 (SCU_EXTREG_PA_BASE + 0x0100)
#define SCU_EXTREG_SPI_CLK                                  (SCU_EXTREG_PA_BASE + 0x0104)
#define SCU_EXTREG_SPI_DO                                   (SCU_EXTREG_PA_BASE + 0x0108)
#define SCU_EXTREG_SPI_DI                                   (SCU_EXTREG_PA_BASE + 0x010C)
#define SCU_EXTREG_SPI_WP_N_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0110)

#define SCU_EXTREG_SPI_HOLD_N_IOCTRL                        (SCU_EXTREG_PA_BASE + 0x0114)
#define SCU_EXTREG_SWJ_TRST_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0118)
#define SCU_EXTREG_SWJ_TDI_IOCTRL                           (SCU_EXTREG_PA_BASE + 0x011C)
#define SCU_EXTREG_SWJ_SWDITMS_IOCTRL                       (SCU_EXTREG_PA_BASE + 0x0120)
#define SCU_EXTREG_SWJ_SWCLKTCK_IOCTRL                      (SCU_EXTREG_PA_BASE + 0x0124)
#define SCU_EXTREG_SWJ_TDO_IOCTRL                           (SCU_EXTREG_PA_BASE + 0x0128)
#define SCU_EXTREG_LC_PCLK_IOCTRL                           (SCU_EXTREG_PA_BASE + 0x012C)
#define SCU_EXTREG_LC_VS_IOCTRL                             (SCU_EXTREG_PA_BASE + 0x0130)
#define SCU_EXTREG_LC_HS_IOCTRL                             (SCU_EXTREG_PA_BASE + 0x0134)
#define SCU_EXTREG_LC_DE_IOCTRL                             (SCU_EXTREG_PA_BASE + 0x0138)
#define SCU_EXTREG_LC_DATA0_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x013C)
#define SCU_EXTREG_LC_DATA1_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0140)
#define SCU_EXTREG_LC_DATA2_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0144)
#define SCU_EXTREG_LC_DATA3_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0148)
#define SCU_EXTREG_LC_DATA4_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x014C)
#define SCU_EXTREG_LC_DATA5_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0150)
#define SCU_EXTREG_LC_DATA6_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0154)
#define SCU_EXTREG_LC_DATA7_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0158)
#define SCU_EXTREG_LC_DATA8_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x015C)
#define SCU_EXTREG_LC_DATA9_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0160)
#define SCU_EXTREG_LC_DATA10_IOCTRL                         (SCU_EXTREG_PA_BASE + 0x0164)
#define SCU_EXTREG_LC_DATA11_IOCTRL                         (SCU_EXTREG_PA_BASE + 0x0168)
#define SCU_EXTREG_LC_DATA12_IOCTRL                         (SCU_EXTREG_PA_BASE + 0x016C)
#define SCU_EXTREG_LC_DATA13_IOCTRL                         (SCU_EXTREG_PA_BASE + 0x0170)
#define SCU_EXTREG_LC_DATA14_IOCTRL                         (SCU_EXTREG_PA_BASE + 0x0174)
#define SCU_EXTREG_LC_DATA15_IOCTRL                         (SCU_EXTREG_PA_BASE + 0x0178)
#define SCU_EXTREG_SD_CLK_IOCTRL                            (SCU_EXTREG_PA_BASE + 0x017C)
#define SCU_EXTREG_SD_CMD_IOCTRL                            (SCU_EXTREG_PA_BASE + 0x0180)
#define SCU_EXTREG_SD_DATA0_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0184)
#define SCU_EXTREG_SD_DATA1_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0188)
#define SCU_EXTREG_SD_DATA2_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x018C)
#define SCU_EXTREG_SD_DATA3_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0190)
#define SCU_EXTREG_UART0_RX_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0194)
#define SCU_EXTREG_UART0_TX_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x0198)
#define SCU_EXTREG_I2C0_CLK_IOCTRL                          (SCU_EXTREG_PA_BASE + 0x019C)
#define SCU_EXTREG_I2C0_DATA_IOCTRL                         (SCU_EXTREG_PA_BASE + 0x01A0)
#define SCU_EXTREG_PWM0_IOCTRL                              (SCU_EXTREG_PA_BASE + 0x01A4)
#define SCU_EXTREG_OTG_DRV_VBUS_IOCTRL                      (SCU_EXTREG_PA_BASE + 0x01A8)
#define SCU_EXTREG_SPARE0_IOCTRL                            (SCU_EXTREG_PA_BASE + 0x01B0)
#define SCU_EXTREG_SPARE1_IOCTRL                            (SCU_EXTREG_PA_BASE + 0x01B4)


/* PLL110HH0L */
/* PLL0 Setting Register (Offset: 0x0000) -- Default power domain */
/* PLL0 is the clock source of system CM4, system / peripheral bus */
#define SCU_EXTREG_PLL0_SETTING_GET_ns()                    GET_BITS(SCU_EXTREG_PLL0_SETTING, 24, 30)
#define SCU_EXTREG_PLL0_SETTING_GET_ms()                    GET_BITS(SCU_EXTREG_PLL0_SETTING, 16, 20)
#define SCU_EXTREG_PLL0_SETTING_GET_cc()                    GET_BITS(SCU_EXTREG_PLL0_SETTING, 12, 13)
#define SCU_EXTREG_PLL0_SETTING_GET_f()                     GET_BITS(SCU_EXTREG_PLL0_SETTING, 8, 9)
#define SCU_EXTREG_PLL0_SETTING_GET_en()                    GET_BIT(SCU_EXTREG_PLL0_SETTING, 0)

#define SCU_EXTREG_PLL0_SETTING_SET_ns()                    SET_MASKED_BITS(SCU_EXTREG_PLL0_SETTING, 24, 30)
#define SCU_EXTREG_PLL0_SETTING_SET_ms()                    SET_MASKED_BITS(SCU_EXTREG_PLL0_SETTING, 16, 20)
#define SCU_EXTREG_PLL0_SETTING_SET_cc()                    SET_MASKED_BITS(SCU_EXTREG_PLL0_SETTING, 12, 13)
#define SCU_EXTREG_PLL0_SETTING_SET_f()                     SET_MASKED_BITS(SCU_EXTREG_PLL0_SETTING, 8, 9)
#define SCU_EXTREG_PLL0_SETTING_SET_en()                    SET_MASKED_BIT(SCU_EXTREG_PLL0_SETTING, 0)

/* PLL110HH0L */
/* PLL1 Setting Register (Offset: 0x0004) -- NPU power domain */
/* PLL1 is the clock source of neural CM4, and nerual bus */
#define SCU_EXTREG_PLL1_SETTING_GET_ns()                    GET_BITS(SCU_EXTREG_PLL1_SETTING, 24, 30)
#define SCU_EXTREG_PLL1_SETTING_GET_ms()                    GET_BITS(SCU_EXTREG_PLL1_SETTING, 16, 20)
#define SCU_EXTREG_PLL1_SETTING_GET_cc()                    GET_BITS(SCU_EXTREG_PLL1_SETTING, 12, 13)
#define SCU_EXTREG_PLL1_SETTING_GET_f()                     GET_BITS(SCU_EXTREG_PLL1_SETTING, 8, 9)
#define SCU_EXTREG_PLL1_SETTING_GET_en()                    GET_BIT(SCU_EXTREG_PLL1_SETTING, 0)

#define SCU_EXTREG_PLL1_SETTING_SET_ns(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL1_SETTING, val, 24, 30)
#define SCU_EXTREG_PLL1_SETTING_SET_ms(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL1_SETTING, val, 16, 20)
#define SCU_EXTREG_PLL1_SETTING_SET_cc(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL1_SETTING, val, 12, 13)
#define SCU_EXTREG_PLL1_SETTING_SET_f(val)                  SET_MASKED_BITS(SCU_EXTREG_PLL1_SETTING, val, 8, 9)
#define SCU_EXTREG_PLL1_SETTING_SET_en(val)                 SET_MASKED_BIT(SCU_EXTREG_PLL1_SETTING, val, 0)

/* PLL110HH0L */
/* PLL2 Setting Register (Offset: 0x0008) -- NPU power domain */
/* PLL2 is the clock source of DDR controller */
#define SCU_EXTREG_PLL2_SETTING_GET_ns()                    GET_BITS(SCU_EXTREG_PLL2_SETTING, 24, 30)
#define SCU_EXTREG_PLL2_SETTING_GET_ms()                    GET_BITS(SCU_EXTREG_PLL2_SETTING, 16, 20)
#define SCU_EXTREG_PLL2_SETTING_GET_cc()                    GET_BITS(SCU_EXTREG_PLL2_SETTING, 12, 13)
#define SCU_EXTREG_PLL2_SETTING_GET_f()                     GET_BITS(SCU_EXTREG_PLL2_SETTING, 8, 9)
#define SCU_EXTREG_PLL2_SETTING_GET_en()                    GET_BIT(SCU_EXTREG_PLL2_SETTING, 0)

#define SCU_EXTREG_PLL2_SETTING_SET_ns(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL2_SETTING, val, 24, 30)
#define SCU_EXTREG_PLL2_SETTING_SET_ms(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL2_SETTING, val, 16, 20)
#define SCU_EXTREG_PLL2_SETTING_SET_cc(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL2_SETTING, val, 12, 13)
#define SCU_EXTREG_PLL2_SETTING_SET_f(val)                  SET_MASKED_BITS(SCU_EXTREG_PLL2_SETTING, val, 8, 9)
#define SCU_EXTREG_PLL2_SETTING_SET_en(val)                 SET_MASKED_BIT(SCU_EXTREG_PLL2_SETTING, val, 0)

/* PLL510HH0L */
/* PLL3 Setting Register (Offset: 0x000C) -- NPU power domain */
/* PLL3 is the clock source of MIPI CSI RX0 */
#define SCU_EXTREG_PLL3_SETTING_ps_MASK                     (BIT24|BIT25|BIT26|BIT27|BIT28)
#define SCU_EXTREG_PLL3_SETTING_ns_MASK                     (BIT12|BIT13|BIT14|BIT15|BIT16|BIT17|BIT18|BIT19|BIT20)
#define SCU_EXTREG_PLL3_SETTING_ms_MASK                     (BIT8|BIT9|BIT10)
#define SCU_EXTREG_PLL3_SETTING_is_MASK                     (BIT4|BIT5|BIT6)
#define SCU_EXTREG_PLL3_SETTING_rs_MASK                     (BIT2|BIT3)

#define SCU_EXTREG_PLL3_SETTING_ps_START                    24
#define SCU_EXTREG_PLL3_SETTING_ns_START                    12
#define SCU_EXTREG_PLL3_SETTING_ms_START                    8
#define SCU_EXTREG_PLL3_SETTING_is_START                    4
#define SCU_EXTREG_PLL3_SETTING_rs_START                    2


#define SCU_EXTREG_PLL3_SETTING_GET_ps()                    GET_BITS(SCU_EXTREG_PLL3_SETTING, 24, 28)
#define SCU_EXTREG_PLL3_SETTING_GET_ns()                    GET_BITS(SCU_EXTREG_PLL3_SETTING, 12, 20)
#define SCU_EXTREG_PLL3_SETTING_GET_ms()                    GET_BITS(SCU_EXTREG_PLL3_SETTING, 8, 10)
#define SCU_EXTREG_PLL3_SETTING_GET_is()                    GET_BITS(SCU_EXTREG_PLL3_SETTING, 4, 6)
#define SCU_EXTREG_PLL3_SETTING_GET_rs()                    GET_BITS(SCU_EXTREG_PLL3_SETTING, 2, 3)
#define SCU_EXTREG_PLL3_SETTING_GET_en()                    GET_BIT(SCU_EXTREG_PLL3_SETTING, 0)

#define SCU_EXTREG_PLL3_SETTING_SET_ps(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL3_SETTING, val, 24, 28)
#define SCU_EXTREG_PLL3_SETTING_SET_ns(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL3_SETTING, val, 12, 20)
#define SCU_EXTREG_PLL3_SETTING_SET_ms(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL3_SETTING, val, 8, 10)
#define SCU_EXTREG_PLL3_SETTING_SET_is(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL3_SETTING, val, 4, 6)
#define SCU_EXTREG_PLL3_SETTING_SET_rs(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL3_SETTING, val, 2, 3)
#define SCU_EXTREG_PLL3_SETTING_SET_en(val)                 SET_MASKED_BIT(SCU_EXTREG_PLL3_SETTING, val, 0)

/* PLL510HH0L */
/* PLL4 Setting Register (Offset: 0x0010) -- Default domain */
/* PLL4 is the clock source of NPU or audio master clock */
#define SCU_EXTREG_PLL4_SETTING_GET_ps()                    GET_BITS(SCU_EXTREG_PLL4_SETTING, 24, 28)
#define SCU_EXTREG_PLL4_SETTING_GET_ns()                    GET_BITS(SCU_EXTREG_PLL4_SETTING, 12, 20)
#define SCU_EXTREG_PLL4_SETTING_GET_ms()                    GET_BITS(SCU_EXTREG_PLL4_SETTING, 8, 10)
#define SCU_EXTREG_PLL4_SETTING_GET_is()                    GET_BITS(SCU_EXTREG_PLL4_SETTING, 4, 6)
#define SCU_EXTREG_PLL4_SETTING_GET_rs()                    GET_BITS(SCU_EXTREG_PLL4_SETTING, 2, 3)
#define SCU_EXTREG_PLL4_SETTING_GET_en()                    GET_BIT(SCU_EXTREG_PLL4_SETTING, 0)

#define SCU_EXTREG_PLL4_SETTING_SET_ps(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL4_SETTING, val, 24, 28)
#define SCU_EXTREG_PLL4_SETTING_SET_ns(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL4_SETTING, val, 12, 20)
#define SCU_EXTREG_PLL4_SETTING_SET_ms(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL4_SETTING, val, 8, 10)
#define SCU_EXTREG_PLL4_SETTING_SET_is(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL4_SETTING, val, 4, 6)
#define SCU_EXTREG_PLL4_SETTING_SET_rs(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL4_SETTING, val, 2, 3)
#define SCU_EXTREG_PLL4_SETTING_SET_en(val)                 SET_MASKED_BIT(SCU_EXTREG_PLL4_SETTING, val, 0)

/* Clock Enable Register 0 (Offset: 0x0014) */
#define SCU_EXTREG_CLK_EN0_GET_ncpu_traceclk()              GET_BIT(SCU_EXTREG_CLK_EN0, 24)
#define SCU_EXTREG_CLK_EN0_GET_scpu_traceclk()              GET_BIT(SCU_EXTREG_CLK_EN0, 23)
#define SCU_EXTREG_CLK_EN0_GET_ncpu_fclk_src()              GET_BIT(SCU_EXTREG_CLK_EN0, 22)
#define SCU_EXTREG_CLK_EN0_GET_pll4_fref_pll0()             GET_BIT(SCU_EXTREG_CLK_EN0, 12)
#define SCU_EXTREG_CLK_EN0_GET_pll5_out2()                  GET_BIT(SCU_EXTREG_CLK_EN0, 9)
#define SCU_EXTREG_CLK_EN0_GET_pll5_out1()                  GET_BIT(SCU_EXTREG_CLK_EN0, 8)
#define SCU_EXTREG_CLK_EN0_GET_pll4_out1()                  GET_BIT(SCU_EXTREG_CLK_EN0, 6)
#define SCU_EXTREG_CLK_EN0_GET_pll3_out2()                  GET_BIT(SCU_EXTREG_CLK_EN0, 5)
#define SCU_EXTREG_CLK_EN0_GET_pll3_out1()                  GET_BIT(SCU_EXTREG_CLK_EN0, 4)
#define SCU_EXTREG_CLK_EN0_GET_pll2_out()                   GET_BIT(SCU_EXTREG_CLK_EN0, 2)
#define SCU_EXTREG_CLK_EN0_GET_pll1_out()                   GET_BIT(SCU_EXTREG_CLK_EN0, 1)

#define SCU_EXTREG_CLK_EN0_SET_ncpu_traceclk(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 24)
#define SCU_EXTREG_CLK_EN0_SET_scpu_traceclk(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 23)
#define SCU_EXTREG_CLK_EN0_SET_ncpu_fclk_src(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 22)
#define SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(val)          SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 12)
#define SCU_EXTREG_CLK_EN0_SET_pll5_out2(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 9)
#define SCU_EXTREG_CLK_EN0_SET_pll5_out1(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 8)
#define SCU_EXTREG_CLK_EN0_SET_pll4_out1(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 6)
#define SCU_EXTREG_CLK_EN0_SET_pll3_out2(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 5)
#define SCU_EXTREG_CLK_EN0_SET_pll3_out1(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 4)
#define SCU_EXTREG_CLK_EN0_SET_pll2_out(val)                SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 2)
#define SCU_EXTREG_CLK_EN0_SET_pll1_out(val)                SET_MASKED_BIT(SCU_EXTREG_CLK_EN0, val, 1)

/* Clock Enable Register 1 (Offset: 0x0018) */
#define SCU_EXTREG_CLK_EN1_GET_spi_clk()                    GET_BIT(SCU_EXTREG_CLK_EN1, 24)
#define SCU_EXTREG_CLK_EN1_GET_npu()                        GET_BIT(SCU_EXTREG_CLK_EN1, 23)
#define SCU_EXTREG_CLK_EN1_GET_adcclk()                     GET_BIT(SCU_EXTREG_CLK_EN1, 22)
#define SCU_EXTREG_CLK_EN1_GET_wdt_extclk()                 GET_BIT(SCU_EXTREG_CLK_EN1, 21)
#define SCU_EXTREG_CLK_EN1_GET_sdclk()                      GET_BIT(SCU_EXTREG_CLK_EN1, 20)
#define SCU_EXTREG_CLK_EN1_GET_TxHsPllRefClk()              GET_BIT(SCU_EXTREG_CLK_EN1, 16)
#define SCU_EXTREG_CLK_EN1_GET_tx_EscClk()                  GET_BIT(SCU_EXTREG_CLK_EN1, 14)
#define SCU_EXTREG_CLK_EN1_GET_csitx_dsi()                  GET_BIT(SCU_EXTREG_CLK_EN1, 13)
#define SCU_EXTREG_CLK_EN1_GET_csitx_csi()                  GET_BIT(SCU_EXTREG_CLK_EN1, 12)
#define SCU_EXTREG_CLK_EN1_GET_csirx1_TxEscClk()            GET_BIT(SCU_EXTREG_CLK_EN1, 10)
#define SCU_EXTREG_CLK_EN1_GET_csirx1_csi()                 GET_BIT(SCU_EXTREG_CLK_EN1, 9)
#define SCU_EXTREG_CLK_EN1_GET_csirx1_vc0()                 GET_BIT(SCU_EXTREG_CLK_EN1, 8)
#define SCU_EXTREG_CLK_EN1_GET_csirx0_TxEscClk()            GET_BIT(SCU_EXTREG_CLK_EN1, 6)
#define SCU_EXTREG_CLK_EN1_GET_csirx0_csi()                 GET_BIT(SCU_EXTREG_CLK_EN1, 5)
#define SCU_EXTREG_CLK_EN1_GET_csirx0_vc0()                 GET_BIT(SCU_EXTREG_CLK_EN1, 4)
#define SCU_EXTREG_CLK_EN1_GET_LC_SCALER()                  GET_BIT(SCU_EXTREG_CLK_EN1, 1)
#define SCU_EXTREG_CLK_EN1_GET_LC_CLK()                     GET_BIT(SCU_EXTREG_CLK_EN1, 0)

#define SCU_EXTREG_CLK_EN1_SET_spi_clk(val)                 SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 24)
#define SCU_EXTREG_CLK_EN1_SET_npu(val)                     SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 23)
#define SCU_EXTREG_CLK_EN1_SET_adcclk(val)                  SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 22)
#define SCU_EXTREG_CLK_EN1_SET_wdt_extclk(val)              SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 21)
#define SCU_EXTREG_CLK_EN1_SET_sdclk(val)                   SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 20)
#define SCU_EXTREG_CLK_EN1_SET_TxHsPllRefClk(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 16)
#define SCU_EXTREG_CLK_EN1_SET_tx_EscClk(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 14)
#define SCU_EXTREG_CLK_EN1_SET_csitx_dsi(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 13)
#define SCU_EXTREG_CLK_EN1_SET_csitx_csi(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 12)
#define SCU_EXTREG_CLK_EN1_SET_csirx1_TxEscClk(val)         SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 10)
#define SCU_EXTREG_CLK_EN1_SET_csirx1_csi(val)              SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 9)
#define SCU_EXTREG_CLK_EN1_SET_csirx1_vc0(val)              SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 8)
#define SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(val)         SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 6)
#define SCU_EXTREG_CLK_EN1_SET_csirx0_csi(val)              SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 5)
#define SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(val)              SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 4)
#define SCU_EXTREG_CLK_EN1_SET_LC_SCALER(val)               SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 1)
#define SCU_EXTREG_CLK_EN1_SET_LC_CLK(val)                  SET_MASKED_BIT(SCU_EXTREG_CLK_EN1, val, 0)

/* Clock Enable Register 2 (Offset: 0x001C) */
#define SCU_EXTREG_CLK_EN2_GET_tmr1_extclk3()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 29)
#define SCU_EXTREG_CLK_EN2_GET_tmr1_extclk2()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 28)
#define SCU_EXTREG_CLK_EN2_GET_tmr1_extclk1()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 27)
#define SCU_EXTREG_CLK_EN2_GET_tmr0_extclk3()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 26)
#define SCU_EXTREG_CLK_EN2_GET_tmr0_extclk2()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 25)
#define SCU_EXTREG_CLK_EN2_GET_tmr0_extclk1()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 24)
#define SCU_EXTREG_CLK_EN2_GET_pwm_extclk6()                GET_BIT(SCU_EXTREG_PLL2_SETTING, 21)
#define SCU_EXTREG_CLK_EN2_GET_pwm_extclk5()                GET_BIT(SCU_EXTREG_PLL2_SETTING, 20)
#define SCU_EXTREG_CLK_EN2_GET_pwm_extclk4()                GET_BIT(SCU_EXTREG_PLL2_SETTING, 19)
#define SCU_EXTREG_CLK_EN2_GET_pwm_extclk3()                GET_BIT(SCU_EXTREG_PLL2_SETTING, 18)
#define SCU_EXTREG_CLK_EN2_GET_pwm_extclk2()                GET_BIT(SCU_EXTREG_PLL2_SETTING, 17)
#define SCU_EXTREG_CLK_EN2_GET_pwm_extclk1()                GET_BIT(SCU_EXTREG_PLL2_SETTING, 16)
#define SCU_EXTREG_CLK_EN2_GET_uart1_3_fref()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 15)
#define SCU_EXTREG_CLK_EN2_GET_uart1_2_fref()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 14)
#define SCU_EXTREG_CLK_EN2_GET_uart1_1_fref()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 13)
#define SCU_EXTREG_CLK_EN2_GET_uart1_0_fref()               GET_BIT(SCU_EXTREG_PLL2_SETTING, 12)
#define SCU_EXTREG_CLK_EN2_GET_uart0_fref()                 GET_BIT(SCU_EXTREG_PLL2_SETTING, 8)
#define SCU_EXTREG_CLK_EN2_GET_ssp1_1_sspclk()              GET_BIT(SCU_EXTREG_PLL2_SETTING, 6)
#define SCU_EXTREG_CLK_EN2_GET_ssp1_0_sspclk()              GET_BIT(SCU_EXTREG_PLL2_SETTING, 4)
#define SCU_EXTREG_CLK_EN2_GET_ssp0_1_sspclk()              GET_BIT(SCU_EXTREG_PLL2_SETTING, 2)
#define SCU_EXTREG_CLK_EN2_GET_ssp0_0_sspclk()              GET_BIT(SCU_EXTREG_PLL2_SETTING, 0)

#define SCU_EXTREG_CLK_EN2_SET_tmr1_extclk3(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 29)
#define SCU_EXTREG_CLK_EN2_SET_tmr1_extclk2(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 28)
#define SCU_EXTREG_CLK_EN2_SET_tmr1_extclk1(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 27)
#define SCU_EXTREG_CLK_EN2_SET_tmr0_extclk3(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 26)
#define SCU_EXTREG_CLK_EN2_SET_tmr0_extclk2(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 25)
#define SCU_EXTREG_CLK_EN2_SET_tmr0_extclk1(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 24)
#define SCU_EXTREG_CLK_EN2_SET_pwm_extclk6(val)             SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 21)
#define SCU_EXTREG_CLK_EN2_SET_pwm_extclk5(val)             SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 20)
#define SCU_EXTREG_CLK_EN2_SET_pwm_extclk4(val)             SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 19)
#define SCU_EXTREG_CLK_EN2_SET_pwm_extclk3(val)             SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 18)
#define SCU_EXTREG_CLK_EN2_SET_pwm_extclk2(val)             SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 17)
#define SCU_EXTREG_CLK_EN2_SET_pwm_extclk1(val)             SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 16)
#define SCU_EXTREG_CLK_EN2_SET_uart1_3_fref(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 15)
#define SCU_EXTREG_CLK_EN2_SET_uart1_2_fref(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 14)
#define SCU_EXTREG_CLK_EN2_SET_uart1_1_fref(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 13)
#define SCU_EXTREG_CLK_EN2_SET_uart1_0_fref(val)            SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 12)
#define SCU_EXTREG_CLK_EN2_SET_uart0_fref(val)              SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 8)
#define SCU_EXTREG_CLK_EN2_SET_ssp1_1_sspclk(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 6)
#define SCU_EXTREG_CLK_EN2_SET_ssp1_0_sspclk(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 4)
#define SCU_EXTREG_CLK_EN2_SET_ssp0_1_sspclk(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 2)
#define SCU_EXTREG_CLK_EN2_SET_ssp0_0_sspclk(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_EN2, val, 0)

/* Clock Mux Selection Register (Offset: 0x0020) */
#define SCU_EXTREG_CLK_MUX_SEL_GET_ncpu_traceclk()          GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 14)
#define SCU_EXTREG_CLK_MUX_SEL_GET_scpu_traceclk_src()      GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 13)
#define SCU_EXTREG_CLK_MUX_SEL_GET_csirx1_clk()             GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 12)
#define SCU_EXTREG_CLK_MUX_SEL_GET_npu_clk()                GET_BITS(SCU_EXTREG_CLK_MUX_SEL, 8, 9)
#define SCU_EXTREG_CLK_MUX_SEL_GET_pll4_fref()              GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 6)
#define SCU_EXTREG_CLK_MUX_SEL_GET_uart_0_irda_uclk()       GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 4)
#define SCU_EXTREG_CLK_MUX_SEL_GET_ssp1_1_sspclk()          GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 3)
#define SCU_EXTREG_CLK_MUX_SEL_GET_ssp1_0_sspclk()          GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 2)
#define SCU_EXTREG_CLK_MUX_SEL_GET_ssp0_1_sspclk()          GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 1)
#define SCU_EXTREG_CLK_MUX_SEL_GET_ssp0_0_sspclk()          GET_BIT(SCU_EXTREG_CLK_MUX_SEL, 0)

#define SCU_EXTREG_CLK_MUX_SEL_SET_ncpu_traceclk(val)       SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 14)
#define SCU_EXTREG_CLK_MUX_SEL_SET_scpu_traceclk_src(val)   SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 13)
#define SCU_EXTREG_CLK_MUX_SEL_SET_csirx1_clk(val)          SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 12)
#define SCU_EXTREG_CLK_MUX_SEL_SET_npu_clk(val)             SET_MASKED_BITS(SCU_EXTREG_CLK_MUX_SEL, val, 8, 9)
#define SCU_EXTREG_CLK_MUX_SEL_SET_pll4_fref(val)           SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 6)
#define SCU_EXTREG_CLK_MUX_SEL_SET_uart_0_irda_uclk(val)    SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 4)
#define SCU_EXTREG_CLK_MUX_SEL_SET_ssp1_1_sspclk(val)       SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 3)
#define SCU_EXTREG_CLK_MUX_SEL_SET_ssp1_0_sspclk(val)       SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 2)
#define SCU_EXTREG_CLK_MUX_SEL_SET_ssp0_1_sspclk(val)       SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 1)
#define SCU_EXTREG_CLK_MUX_SEL_SET_ssp0_0_sspclk(val)       SET_MASKED_BIT(SCU_EXTREG_CLK_MUX_SEL, val, 0)

/* Clock Divider Register 0 (Offset: 0x0024) */
#define SCU_EXTREG_CLK_DIV0_GET_ncpu_fclk()                 GET_BITS(SCU_EXTREG_CLK_DIV0, 28, 31)
#define SCU_EXTREG_CLK_DIV0_GET_sdclk2x()                   GET_BITS(SCU_EXTREG_CLK_DIV0, 24, 27)
#define SCU_EXTREG_CLK_DIV0_GET_spi_clk()                   GET_BITS(SCU_EXTREG_CLK_DIV0, 20, 23)
#define SCU_EXTREG_CLK_DIV0_GET_pll4_fref_pll0()            GET_BITS(SCU_EXTREG_CLK_DIV0, 8, 12)

#define SCU_EXTREG_CLK_DIV0_SET_ncpu_fclk(val)              SET_MASKED_BITS(SCU_EXTREG_CLK_DIV0, val, 28, 31)
#define SCU_EXTREG_CLK_DIV0_SET_sdclk2x(val)                SET_MASKED_BITS(SCU_EXTREG_CLK_DIV0, val, 24, 27)
#define SCU_EXTREG_CLK_DIV0_SET_spi_clk(val)                SET_MASKED_BITS(SCU_EXTREG_CLK_DIV0, val, 20, 23)
#define SCU_EXTREG_CLK_DIV0_SET_pll4_fref_pll0(val)         SET_MASKED_BITS(SCU_EXTREG_CLK_DIV0, val, 8, 12)
/* Clock Divider Register 1 (Offset: 0x0028) */
#define SCU_EXTREG_CLK_DIV1_GET_csirx0_TxEscClk()           GET_BITS(SCU_EXTREG_CLK_DIV1, 28, 31)
#define SCU_EXTREG_CLK_DIV1_GET_csirx0_csi()                GET_BITS(SCU_EXTREG_CLK_DIV1, 20, 24)
#define SCU_EXTREG_CLK_DIV1_GET_csirx0_vc0()                GET_BITS(SCU_EXTREG_CLK_DIV1, 12, 16)
#define SCU_EXTREG_CLK_DIV1_GET_LC_CLK()                    GET_BITS(SCU_EXTREG_CLK_DIV1, 4, 8)
#define SCU_EXTREG_CLK_DIV1_GET_LC_SCALER_CLK()             GET_BITS(SCU_EXTREG_CLK_DIV1, 0, 3)

#define SCU_EXTREG_CLK_DIV1_SET_csirx0_TxEscClk(val)        SET_MASKED_BITS(SCU_EXTREG_CLK_DIV1, val, 28, 31)
#define SCU_EXTREG_CLK_DIV1_SET_csirx0_csi(val)             SET_MASKED_BITS(SCU_EXTREG_CLK_DIV1, val, 20, 24)
#define SCU_EXTREG_CLK_DIV1_SET_csirx0_vc0(val)             SET_MASKED_BITS(SCU_EXTREG_CLK_DIV1, val, 12, 16)
#define SCU_EXTREG_CLK_DIV1_SET_LC_CLK(val)                 SET_MASKED_BITS(SCU_EXTREG_CLK_DIV1, val, 4, 8)
#define SCU_EXTREG_CLK_DIV1_SET_LC_SCALER_CLK(val)          SET_MASKED_BITS(SCU_EXTREG_CLK_DIV1, val, 0, 3)
/* Clock Divider Register 2 (Offset: 0x002C) */
#define SCU_EXTREG_CLK_DIV2_GET_npu_clk_pll5()              GET_BITS(SCU_EXTREG_CLK_DIV2, 16, 18)
#define SCU_EXTREG_CLK_DIV2_GET_npu_clk_pll4()              GET_BITS(SCU_EXTREG_CLK_DIV2, 12, 14)
#define SCU_EXTREG_CLK_DIV2_GET_npu_clk_pll0()              GET_BITS(SCU_EXTREG_CLK_DIV2, 8, 10)

#define SCU_EXTREG_CLK_DIV2_SET_npu_clk_pll5(val)           SET_MASKED_BITS(SCU_EXTREG_CLK_DIV2, val, 16, 18)
#define SCU_EXTREG_CLK_DIV2_SET_npu_clk_pll4(val)           SET_MASKED_BITS(SCU_EXTREG_CLK_DIV2, val, 12, 14)
#define SCU_EXTREG_CLK_DIV2_SET_npu_clk_pll0(val)           SET_MASKED_BITS(SCU_EXTREG_CLK_DIV2, val, 8, 10)
/* Clock Divider Register 3 (Offset: 0x0030) */
#define SCU_EXTREG_CLK_DIV3_GET_ssp0_1_sspclk_slv()         GET_BITS(SCU_EXTREG_CLK_DIV3, 24, 26) 
#define SCU_EXTREG_CLK_DIV3_GET_ssp0_1_sspclk_mst()         GET_BITS(SCU_EXTREG_CLK_DIV3, 16, 21)
#define SCU_EXTREG_CLK_DIV3_GET_ssp0_0_sspclk_slv()         GET_BITS(SCU_EXTREG_CLK_DIV3, 8, 10)
#define SCU_EXTREG_CLK_DIV3_GET_ssp0_0_sspclk_mst()         GET_BITS(SCU_EXTREG_CLK_DIV3, 0, 5)

#define SCU_EXTREG_CLK_DIV3_SET_ssp0_1_sspclk_slv(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV3, val, 24, 26) 
#define SCU_EXTREG_CLK_DIV3_SET_ssp0_1_sspclk_mst(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV3, val, 16, 21)
#define SCU_EXTREG_CLK_DIV3_SET_ssp0_0_sspclk_slv(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV3, val, 8, 10)
#define SCU_EXTREG_CLK_DIV3_SET_ssp0_0_sspclk_mst(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV3, val, 0, 5)
/* Clock Divider Register 4 (Offset: 0x0034) */
#define SCU_EXTREG_CLK_DIV4_GET_ssp1_1_sspclk_slv()         GET_BITS(SCU_EXTREG_CLK_DIV4, 24, 26)
#define SCU_EXTREG_CLK_DIV4_GET_ssp1_1_sspclk_mst()         GET_BITS(SCU_EXTREG_CLK_DIV4, 16, 21)
#define SCU_EXTREG_CLK_DIV4_GET_ssp1_0_sspclk_slv()         GET_BITS(SCU_EXTREG_CLK_DIV4, 8, 10)
#define SCU_EXTREG_CLK_DIV4_GET_ssp1_0_sspclk_mst()         GET_BITS(SCU_EXTREG_CLK_DIV4, 0, 5)

#define SCU_EXTREG_CLK_DIV4_SET_ssp1_1_sspclk_slv(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV4, val, 24, 26)
#define SCU_EXTREG_CLK_DIV4_SET_ssp1_1_sspclk_mst(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV4, val, 16, 21)
#define SCU_EXTREG_CLK_DIV4_SET_ssp1_0_sspclk_slv(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV4, val, 8, 10)
#define SCU_EXTREG_CLK_DIV4_SET_ssp1_0_sspclk_mst(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV4, val, 0, 5)

/* PLL510HH0L */
/* PLL5 Setting Register (Offset: 0x003C) -- Disp -- NPU Domain */
/* PLL5 is the clock source of pixel clock, MIPI CSI/DPI TX clock, or MIPI CSI RX1 clock*/
#define SCU_EXTREG_PLL5_SETTING_ps_MASK                     (BIT24|BIT25|BIT26|BIT27|BIT28)
#define SCU_EXTREG_PLL5_SETTING_ns_MASK                     (BIT12|BIT13|BIT14|BIT15|BIT16|BIT17|BIT18|BIT19|BIT20)
#define SCU_EXTREG_PLL5_SETTING_ms_MASK                     (BIT8|BIT9|BIT10)
#define SCU_EXTREG_PLL5_SETTING_is_MASK                     (BIT4|BIT5|BIT6)
#define SCU_EXTREG_PLL5_SETTING_rs_MASK                     (BIT2|BIT3)

#define SCU_EXTREG_PLL5_SETTING_ps_START                    24
#define SCU_EXTREG_PLL5_SETTING_ns_START                    12
#define SCU_EXTREG_PLL5_SETTING_ms_START                    8
#define SCU_EXTREG_PLL5_SETTING_is_START                    4
#define SCU_EXTREG_PLL5_SETTING_rs_START                    2

#define SCU_EXTREG_PLL5_SETTING_GET_ps()                    GET_BITS(SCU_EXTREG_PLL5_SETTING, 24, 28)
#define SCU_EXTREG_PLL5_SETTING_GET_ns()                    GET_BITS(SCU_EXTREG_PLL5_SETTING, 12, 20)
#define SCU_EXTREG_PLL5_SETTING_GET_ms()                    GET_BITS(SCU_EXTREG_PLL5_SETTING, 8, 10)
#define SCU_EXTREG_PLL5_SETTING_GET_is()                    GET_BITS(SCU_EXTREG_PLL5_SETTING, 4, 6)
#define SCU_EXTREG_PLL5_SETTING_GET_rs()                    GET_BITS(SCU_EXTREG_PLL5_SETTING, 2, 3)
#define SCU_EXTREG_PLL5_SETTING_GET_en()                    GET_BIT(SCU_EXTREG_PLL5_SETTING, 0)

#define SCU_EXTREG_PLL5_SETTING_SET_ps(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL5_SETTING, val, 24, 28)
#define SCU_EXTREG_PLL5_SETTING_SET_ns(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL5_SETTING, val, 12, 20)
#define SCU_EXTREG_PLL5_SETTING_SET_ms(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL5_SETTING, val, 8, 10)
#define SCU_EXTREG_PLL5_SETTING_SET_is(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL5_SETTING, val, 4, 6)
#define SCU_EXTREG_PLL5_SETTING_SET_rs(val)                 SET_MASKED_BITS(SCU_EXTREG_PLL5_SETTING, val, 2, 3)
#define SCU_EXTREG_PLL5_SETTING_SET_en(val)                 SET_MASKED_BIT(SCU_EXTREG_PLL5_SETTING, val, 0)

/* Software Reset mask Register 0 (Offset: 0x0040) */
#define SCU_EXTREG_SWRST_MASK0_GET_cpu_resetreq_n()         GET_BIT(SCU_EXTREG_SWRST_MASK0, 0)

#define SCU_EXTREG_SWRST_MASK0_SET_cpu_resetreq_n(val)      SET_MASKED_BIT(SCU_EXTREG_SWRST_MASK0, val, 0)

/* Software Reset mask Register 1 (Offset: 0x0044) */
#define SCU_EXTREG_SWRST_MASK1_lcm_reset_n                  BIT23
#define SCU_EXTREG_SWRST_MASK1_AResetn_u_FTLCDC210          BIT20
#define SCU_EXTREG_SWRST_MASK1_PRESETn_u_FTLCDC210          BIT19
#define SCU_EXTREG_SWRST_MASK1_TV_RSTn_FTLCDC210            BIT18
#define SCU_EXTREG_SWRST_MASK1_LC_SCALER_RSTn_FTLCDC210     BIT17
#define SCU_EXTREG_SWRST_MASK1_LC_RSTn_FTLCDC210            BIT16

#define SCU_EXTREG_SWRST_MASK1_GET_lcm_reset_n()            GET_BIT(SCU_EXTREG_PLL5_SETTING, 23)
#define SCU_EXTREG_SWRST_MASK1_SET_lcm_reset_n(val)         SET_MASKED_BIT(SCU_EXTREG_PLL5_SETTING, val, 23)

/* Software Reset mask Register 2 (Offset: 0x0048) */


/* Software Reset Register (Offset: 0x004C) */
#define SCU_EXTREG_SWRST_GET_NPU_resetn()                   GET_BIT(SCU_EXTREG_SWRST, 2)
#define SCU_EXTREG_SWRST_GET_PD_NPU_resetn()                GET_BIT(SCU_EXTREG_SWRST, 1)
#define SCU_EXTREG_SWRST_GET_LCDC_resetn()                  GET_BIT(SCU_EXTREG_SWRST, 0)

#define SCU_EXTREG_SWRST_SET_NPU_resetn(val)                SET_MASKED_BIT(SCU_EXTREG_SWRST, val, 2)
#define SCU_EXTREG_SWRST_SET_PD_NPU_resetn(val)             SET_MASKED_BIT(SCU_EXTREG_SWRST, val, 1)
#define SCU_EXTREG_SWRST_SET_LCDC_resetn(val)               SET_MASKED_BIT(SCU_EXTREG_SWRST, val, 0)

/* CM4 NCPU Control Register 0 (Offset: 0x0068) */
#define SCU_EXTREG_CM4_NCPU_CTRL_GET_wakeup()               GET_BIT(SCU_EXTREG_CM4_NCPU_CTRL, 12)
#define SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(val)            SET_MASKED_BIT(SCU_EXTREG_CM4_NCPU_CTRL, val, 12)

/* DDR CTRL Register 0 (Offset: 0x0080) */
#define SCU_EXTREG_DDR_CTRL_Dphy_resetn                     BIT29
#define SCU_EXTREG_DDR_CTRL_wakeup                          BIT28
#define SCU_EXTREG_DDR_CTRL_SELFBIAS                        BIT15

#define SCU_EXTREG_DDR_CTRL_SET_Dphy_resetn(val)            SET_MASKED_BIT(SCU_EXTREG_DDR_CTRL, val, 29)
#define SCU_EXTREG_DDR_CTRL_SET_wakeup(val)                 SET_MASKED_BIT(SCU_EXTREG_DDR_CTRL, val, 28)
#define SCU_EXTREG_DDR_CTRL_SET_SELFBIAS(val)               SET_MASKED_BIT(SCU_EXTREG_DDR_CTRL, val, 15)

/* USB OTG CTRL Register (Offset: 0x008C) */
#define SCU_EXTREG_USB_OTG_CTRL_GET_EXTCTRL_SUSPENDM()      GET_BIT(SCU_EXTREG_USB_OTG_CTRL, 8)
#define SCU_EXTREG_USB_OTG_CTRL_GET_u_iddig()               GET_BIT(SCU_EXTREG_USB_OTG_CTRL, 7)
#define SCU_EXTREG_USB_OTG_CTRL_GET_wakeup()                GET_BIT(SCU_EXTREG_USB_OTG_CTRL, 6)
#define SCU_EXTREG_USB_OTG_CTRL_GET_l1_wakeup()             GET_BIT(SCU_EXTREG_USB_OTG_CTRL, 5)
#define SCU_EXTREG_USB_OTG_CTRL_GET_OSCOUTEN()              GET_BIT(SCU_EXTREG_USB_OTG_CTRL, 4)
#define SCU_EXTREG_USB_OTG_CTRL_GET_PLLALIV()               GET_BIT(SCU_EXTREG_USB_OTG_CTRL, 3)
#define SCU_EXTREG_USB_OTG_CTRL_GET_XTLSEL()                GET_BITS(SCU_EXTREG_USB_OTG_CTRL, 1, 2)
#define SCU_EXTREG_USB_OTG_CTRL_GET_OUTCLKSEL()             GET_BIT(SCU_EXTREG_USB_OTG_CTRL, 0)

#define SCU_EXTREG_USB_OTG_CTRL_SET_EXTCTRL_SUSPENDM(val)   SET_MASKED_BIT(SCU_EXTREG_USB_OTG_CTRL, val, 8)
#define SCU_EXTREG_USB_OTG_CTRL_SET_u_iddig(val)            SET_MASKED_BIT(SCU_EXTREG_USB_OTG_CTRL, val, 7)
#define SCU_EXTREG_USB_OTG_CTRL_SET_wakeup(val)             SET_MASKED_BIT(SCU_EXTREG_USB_OTG_CTRL, val, 6)
#define SCU_EXTREG_USB_OTG_CTRL_SET_l1_wakeup(val)          SET_MASKED_BIT(SCU_EXTREG_USB_OTG_CTRL, val, 5)
#define SCU_EXTREG_USB_OTG_CTRL_SET_OSCOUTEN(val)           SET_MASKED_BIT(SCU_EXTREG_USB_OTG_CTRL, val, 4)
#define SCU_EXTREG_USB_OTG_CTRL_SET_PLLALIV(val)            SET_MASKED_BIT(SCU_EXTREG_USB_OTG_CTRL, val, 3)
#define SCU_EXTREG_USB_OTG_CTRL_SET_XTLSEL(val)             SET_MASKED_BITS(SCU_EXTREG_USB_OTG_CTRL, val, 1, 2)
#define SCU_EXTREG_USB_OTG_CTRL_SET_OUTCLKSEL(val)          SET_MASKED_BIT(SCU_EXTREG_USB_OTG_CTRL, val, 0)

/* CSIRX CTRL Register 0 (Offset: 0x0090) */
#define SCU_EXTREG_CSIRX_CTRL0_apb_rst_n                    BIT28
#define SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n                    BIT25
#define SCU_EXTREG_CSIRX_CTRL0_sys_rst_n                    BIT24
#define SCU_EXTREG_CSIRX_CTRL0_ClkLnEn                      BIT1
#define SCU_EXTREG_CSIRX_CTRL0_Enable                       BIT0

#define SCU_EXTREG_CSIRX_CTRL0_GET_apb_rst_n()              GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 28)
#define SCU_EXTREG_CSIRX_CTRL0_GET_pwr_rst_n()              GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 25)
#define SCU_EXTREG_CSIRX_CTRL0_GET_sys_rst_n()              GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 24)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DP1()            GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 17)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DN1()            GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 16)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DP0()            GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 15)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DN0()            GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 14)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_CKP()            GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 13)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_CKN()            GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 12)
#define SCU_EXTREG_CSIRX_CTRL0_GET_ClkLnEn()                GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 1)
#define SCU_EXTREG_CSIRX_CTRL0_GET_Enable()                 GET_BIT(SCU_EXTREG_CSIRX_CTRL0, 0)

#define SCU_EXTREG_CSIRX_CTRL0_SET_apb_rst_n(val)           SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 28)
#define SCU_EXTREG_CSIRX_CTRL0_SET_pwr_rst_n(val)           SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 25)
#define SCU_EXTREG_CSIRX_CTRL0_SET_sys_rst_n(val)           SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 24)
#define SCU_EXTREG_CSIRX_CTRL0_SET_CMOS_IE_DP1(val)         SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 17)
#define SCU_EXTREG_CSIRX_CTRL0_SET_CMOS_IE_DN1(val)         SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 16)
#define SCU_EXTREG_CSIRX_CTRL0_SET_CMOS_IE_DP0(val)         SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 15)
#define SCU_EXTREG_CSIRX_CTRL0_SET_CMOS_IE_DN0(val)         SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 14)
#define SCU_EXTREG_CSIRX_CTRL0_SET_CMOS_IE_CKP(val)         SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 13)
#define SCU_EXTREG_CSIRX_CTRL0_SET_CMOS_IE_CKN(val)         SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 12)
#define SCU_EXTREG_CSIRX_CTRL0_SET_ClkLnEn(val)             SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 1)
#define SCU_EXTREG_CSIRX_CTRL0_SET_Enable(val)              SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL0, val, 0)

/* CSIRX CTRL Register 1 (Offset: 0x0094) */
#define SCU_EXTREG_CSIRX_CTRL1_apb_rst_n                    BIT28
#define SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n                    BIT25
#define SCU_EXTREG_CSIRX_CTRL1_sys_rst_n                    BIT24
#define SCU_EXTREG_CSIRX_CTRL1_ClkLnEn                      BIT1
#define SCU_EXTREG_CSIRX_CTRL1_Enable                       BIT0

#define SCU_EXTREG_CSIRX_CTRL1_GET_apb_rst_n()              GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 28)
#define SCU_EXTREG_CSIRX_CTRL1_GET_pwr_rst_n()              GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 25)
#define SCU_EXTREG_CSIRX_CTRL1_GET_sys_rst_n()              GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 24)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DP1_1()          GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 17)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DN1_1()          GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 16)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DP0_1()          GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 15)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_DN0_1()          GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 14)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_CKP_1()          GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 13)
#define SCU_EXTREG_CSIRX_CTRL0_GET_CMOS_IE_CKN_1()          GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 12)
#define SCU_EXTREG_CSIRX_CTRL1_GET_ClkLnEn()                GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 1)
#define SCU_EXTREG_CSIRX_CTRL1_GET_Enable()                 GET_BIT(SCU_EXTREG_CSIRX_CTRL1, 0)

#define SCU_EXTREG_CSIRX_CTRL1_SET_apb_rst_n(val)           SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 28)
#define SCU_EXTREG_CSIRX_CTRL1_SET_pwr_rst_n(val)           SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 25)
#define SCU_EXTREG_CSIRX_CTRL1_SET_sys_rst_n(val)           SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 24)
#define SCU_EXTREG_CSIRX_CTRL1_SET_CMOS_IE_DP1_1(val)       SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 17)
#define SCU_EXTREG_CSIRX_CTRL1_SET_CMOS_IE_DN1_1(val)       SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 16)
#define SCU_EXTREG_CSIRX_CTRL1_SET_CMOS_IE_DP0_1(val)       SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 15)
#define SCU_EXTREG_CSIRX_CTRL1_SET_CMOS_IE_DN0_1(val)       SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 14)
#define SCU_EXTREG_CSIRX_CTRL1_SET_CMOS_IE_CKP_1(val)       SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 13)
#define SCU_EXTREG_CSIRX_CTRL1_SET_CMOS_IE_CKN_1(val)       SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 12)
#define SCU_EXTREG_CSIRX_CTRL1_SET_ClkLnEn(val)             SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 1)
#define SCU_EXTREG_CSIRX_CTRL1_SET_Enable(val)              SET_MASKED_BIT(SCU_EXTREG_CSIRX_CTRL1, val, 0)

/* DPIAHB Control Register (Offset: 0x009C) */
#define SCU_EXTREG_DPI2AHB_CTRL_rst_n_1                     BIT7
#define SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1                 BIT6
#define SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1                 BIT5
#define SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1                 BIT4
#define SCU_EXTREG_DPI2AHB_CTRL_rst_n                       BIT3
#define SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n                   BIT2
#define SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n                   BIT1
#define SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n                   BIT0

#define SCU_EXTREG_DPI2AHB_CTRL_GET_rst_n_1()               GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 7)
#define SCU_EXTREG_DPI2AHB_CTRL_GET_apb_rst_n_1()           GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 6)
#define SCU_EXTREG_DPI2AHB_CTRL_GET_sys_rst_n_1()           GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 5)
#define SCU_EXTREG_DPI2AHB_CTRL_GET_pwr_rst_n_1()           GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 4)
#define SCU_EXTREG_DPI2AHB_CTRL_GET_rst_n()                 GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 3)
#define SCU_EXTREG_DPI2AHB_CTRL_GET_apb_rst_n()             GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 2)
#define SCU_EXTREG_DPI2AHB_CTRL_GET_sys_rst_n()             GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 1)
#define SCU_EXTREG_DPI2AHB_CTRL_GET_pwr_rst_n()             GET_BIT(SCU_EXTREG_DPI2AHB_CTRL, 0)

#define SCU_EXTREG_DPI2AHB_CTRL_SET_rst_n_1(val)            SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 7)
#define SCU_EXTREG_DPI2AHB_CTRL_SET_apb_rst_n_1(val)        SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 6)
#define SCU_EXTREG_DPI2AHB_CTRL_SET_sys_rst_n_1(val)        SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 5)
#define SCU_EXTREG_DPI2AHB_CTRL_SET_pwr_rst_n_1(val)        SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 4)
#define SCU_EXTREG_DPI2AHB_CTRL_SET_rst_n(val)              SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 3)
#define SCU_EXTREG_DPI2AHB_CTRL_SET_apb_rst_n(val)          SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 2)
#define SCU_EXTREG_DPI2AHB_CTRL_SET_sys_rst_n(val)          SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 1)
#define SCU_EXTREG_DPI2AHB_CTRL_SET_pwr_rst_n(val)          SET_MASKED_BIT(SCU_EXTREG_DPI2AHB_CTRL, val, 0)

/* Bonding Option Register (Offset: 0x00A0) */
#define SCU_EXTREG_BONDING_OPTION_GET_pkg_214()             GET_BIT(SCU_EXTREG_BONDING_OPTION, 0)

/* SD Ctrl Register (Offset: 0x00A4) */
#define SCU_EXTREG_SD_CTRL_GET_io_sd_cd(val)                GET_BIT(SCU_EXTREG_SD_CTRL, 0)
#define SCU_EXTREG_SD_CTRL_GET_io_sd_wp(val)                GET_BIT(SCU_EXTREG_SD_CTRL, 1)
#define SCU_EXTREG_SD_CTRL_SET_io_sd_cd(val)                SET_MASKED_BIT(SCU_EXTREG_SD_CTRL, 0)
#define SCU_EXTREG_SD_CTRL_SET_io_sd_wp(val)                SET_MASKED_BIT(SCU_EXTREG_SD_CTRL, 1)


/* Misc Register (Offset: 0x00B0) */
#define SCU_EXTREG_MISC_GET_smr_por_n()                     GET_BITS(SCU_EXTREG_MISC, 4, 6)
#define SCU_EXTREG_MISC_GET_lcm_cken()                      GET_BIT(SCU_EXTREG_MISC, 12)
#define SCU_EXTREG_MISC_SET_lcm_cken(val)                   SET_MASKED_BIT(SCU_EXTREG_MISC, val, 12)
#define SCU_EXTREG_MISC_SET_DPI_MUX_0_sel(val)             SET_MASKED_BIT(SCU_EXTREG_MISC, val, 8)
#define SCU_EXTREG_MISC_SET_DPI_MUX_0_get()                 GET_BIT(SCU_EXTREG_MISC, 8)
#define SCU_EXTREG_MISC_SET_DPI_MUX_1_sel(val)             SET_MASKED_BIT(SCU_EXTREG_MISC, val, 9)
#define SCU_EXTREG_MISC_SET_DPI_MUX_1_get()                 GET_BIT(SCU_EXTREG_MISC, 9)


#define SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DDRCK      BIT6
#define SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_NPU        BIT5
#define SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DEFAULT    BIT4
#define SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_MASK       (SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DDRCK | \
                                                             SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_NPU | \
                                                             SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DEFAULT)

/* Clock Divider Register 6 (Offset: 0x00D0) */
#define SCU_EXTREG_CLK_DIV6_GET_uart1_3_fref()              GET_BITS(SCU_EXTREG_CLK_DIV6, 24, 26)
#define SCU_EXTREG_CLK_DIV6_GET_uart1_2_fref()              GET_BITS(SCU_EXTREG_CLK_DIV6, 20, 22)
#define SCU_EXTREG_CLK_DIV6_GET_uart1_1_fref()              GET_BITS(SCU_EXTREG_CLK_DIV6, 16, 18)
#define SCU_EXTREG_CLK_DIV6_GET_uart1_0_fref()              GET_BITS(SCU_EXTREG_CLK_DIV6, 12, 14)
#define SCU_EXTREG_CLK_DIV6_GET_uart0_fref()                GET_BITS(SCU_EXTREG_CLK_DIV6, 8, 10)
#define SCU_EXTREG_CLK_DIV6_GET_uart0_fir_fref()            GET_BITS(SCU_EXTREG_CLK_DIV6, 0, 4)

#define SCU_EXTREG_CLK_DIV6_SET_uart1_3_fref(val)           SET_MASKED_BITS(SCU_EXTREG_CLK_DIV6, val, 24, 26)
#define SCU_EXTREG_CLK_DIV6_SET_uart1_2_fref(val)           SET_MASKED_BITS(SCU_EXTREG_CLK_DIV6, val, 20, 22)
#define SCU_EXTREG_CLK_DIV6_SET_uart1_1_fref(val)           SET_MASKED_BITS(SCU_EXTREG_CLK_DIV6, val, 16, 18)
#define SCU_EXTREG_CLK_DIV6_SET_uart1_0_fref(val)           SET_MASKED_BITS(SCU_EXTREG_CLK_DIV6, val, 12, 14)
#define SCU_EXTREG_CLK_DIV6_SET_uart0_fref(val)             SET_MASKED_BITS(SCU_EXTREG_CLK_DIV6, val, 8, 10)
#define SCU_EXTREG_CLK_DIV6_SET_uart0_fir_fref(val)         SET_MASKED_BITS(SCU_EXTREG_CLK_DIV6, val, 0, 4)
/* Clock Divider Register 7 (Offset: 0x00D4) */
#define SCU_EXTREG_CLK_DIV7_csirx1_TxEscClk_pll3_MASK       (BIT16|BIT17|BIT18|BIT19)
#define SCU_EXTREG_CLK_DIV7_csi1_csi_pll3_MASK              (BIT8|BIT9|BIT10|BIT11|BIT12)
#define SCU_EXTREG_CLK_DIV7_csi1_vc0_pll3_MASK              (BIT0|BIT1|BIT2|BIT3|BIT4)

#define SCU_EXTREG_CLK_DIV7_csirx1_TxEscClk_pll3_START      16
#define SCU_EXTREG_CLK_DIV7_csi1_csi_pll3_MASK_START        8
#define SCU_EXTREG_CLK_DIV7_csi1_vc0_pll3_MASK_START        0

#define SCU_EXTREG_CLK_DIV7_GET_ncpu_traceclk_div()         GET_BITS(SCU_EXTREG_CLK_DIV7, 23, 25)
#define SCU_EXTREG_CLK_DIV7_GET_scpu_traceclk_div()         GET_BITS(SCU_EXTREG_CLK_DIV7, 20, 22)
//csirx1_TxEscClk_pll5 / tx_TxEscClk / 
#define SCU_EXTREG_CLK_DIV7_GET_csirx1_TxEscClk_pll3()      GET_BITS(SCU_EXTREG_CLK_DIV7, 16, 19)
//csirx1_csi_pll3 / csitx_csi / dsitx_dsi / csirx1_csi_clk_pll5
#define SCU_EXTREG_CLK_DIV7_GET_csi1_csi_pll3()             GET_BITS(SCU_EXTREG_CLK_DIV7, 8, 12)
//csirx1_csi_pll3 / csitx_csi / dsitx_dsi / csirx1_csi_clk_pll5
#define SCU_EXTREG_CLK_DIV7_GET_csi1_vc0_pll3()             GET_BITS(SCU_EXTREG_CLK_DIV7, 0, 4)

#define SCU_EXTREG_CLK_DIV7_SET_ncpu_traceclk_div(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV7, val, 23, 25)
#define SCU_EXTREG_CLK_DIV7_SET_scpu_traceclk_div(val)      SET_MASKED_BITS(SCU_EXTREG_CLK_DIV7, val, 20, 22)
#define SCU_EXTREG_CLK_DIV7_SET_csirx1_TxEscClk_pll3(val)   SET_MASKED_BITS(SCU_EXTREG_CLK_DIV7, val, 16, 19)
#define SCU_EXTREG_CLK_DIV7_SET_csi1_csi_pll3(val)          SET_MASKED_BITS(SCU_EXTREG_CLK_DIV7, val, 8, 12)
#define SCU_EXTREG_CLK_DIV7_SET_csi1_vc0_pll3(val)          SET_MASKED_BITS(SCU_EXTREG_CLK_DIV7, val, 0, 4)


/* SPI_CS_N IO control register (Offset 0x0100) */
#define SCU_EXTREG_SPI_CS_N_GET_dcsr()                      GET_BITS(SCU_EXTREG_SPI_CS_N, 5, 8)
#define SCU_EXTREG_SPI_CS_N_GET_pd()                        GET_BIT(SCU_EXTREG_SPI_CS_N, 4)
#define SCU_EXTREG_SPI_CS_N_GET_pu()                        GET_BIT(SCU_EXTREG_SPI_CS_N, 3)

#define SCU_EXTREG_SPI_CS_N_SET_dcsr(val)                   SET_MASKED_BITS(SCU_EXTREG_SPI_CS_N, val, 5, 8)
#define SCU_EXTREG_SPI_CS_N_SET_pd(val)                     SET_MASKED_BIT(SCU_EXTREG_SPI_CS_N, 4)
#define SCU_EXTREG_SPI_CS_N_SET_pu(val)                     SET_MASKED_BIT(SCU_EXTREG_SPI_CS_N, 3)

/* SPI_CLK IO control register (Offset 0x0104) */
#define SCU_EXTREG_SPI_CLK_GET_dcsr()                       GET_BITS(SCU_EXTREG_SPI_CLK, 5, 8)
#define SCU_EXTREG_SPI_CLK_GET_pd()                         GET_BIT(SCU_EXTREG_SPI_CLK, 4)
#define SCU_EXTREG_SPI_CLK_GET_pu()                         GET_BIT(SCU_EXTREG_SPI_CLK, 3)
#define SCU_EXTREG_SPI_CLK_SET_dcsr(val)                    SET_MASKED_BITS(SCU_EXTREG_SPI_CLK, val, 5, 8)
#define SCU_EXTREG_SPI_CLK_SET_pd(val)                      SET_MASKED_BIT(SCU_EXTREG_SPI_CLK, 4)
#define SCU_EXTREG_SPI_CLK_SET_pu(val)                      SET_MASKED_BIT(SCU_EXTREG_SPI_CLK, 3)

/* SPI_CLK IO control register (Offset 0x0108) */
#define SCU_EXTREG_SPI_DO_GET_dcsr()                        GET_BITS(SCU_EXTREG_SPI_DO, 5, 8)
#define SCU_EXTREG_SPI_DO_GET_pd()                          GET_BIT(SCU_EXTREG_SPI_DO, 4)
#define SCU_EXTREG_SPI_DO_GET_pu()                          GET_BIT(SCU_EXTREG_SPI_DO, 3)
#define SCU_EXTREG_SPI_DO_SET_dcsr(val)                     SET_MASKED_BITS(SCU_EXTREG_SPI_DO, val, 5, 8)
#define SCU_EXTREG_SPI_DO_SET_pd(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_DO, 4)
#define SCU_EXTREG_SPI_DO_SET_pu(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_DO, 3)

/* SPI_CLK IO control register (Offset 0x010C) */
#define SCU_EXTREG_SPI_DI_GET_dcsr()                        GET_BITS(SCU_EXTREG_SPI_DI, 5, 8)
#define SCU_EXTREG_SPI_DI_GET_pd()                          GET_BIT(SCU_EXTREG_SPI_DI, 4)
#define SCU_EXTREG_SPI_DI_GET_pu()                          GET_BIT(SCU_EXTREG_SPI_DI, 3)
#define SCU_EXTREG_SPI_DI_SET_dcsr(val)                     SET_MASKED_BITS(SCU_EXTREG_SPI_DI, val, 5, 8)
#define SCU_EXTREG_SPI_DI_SET_pd(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_DI, 4)
#define SCU_EXTREG_SPI_DI_SET_pu(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_DI, 3)


/* SPI_CLK IO control register (Offset 0x0110) */
#define SCU_EXTREG_SPI_WP_N_IOCTRL_GET_dcsr()                        GET_BITS(SCU_EXTREG_SPI_WP_N_IOCTRL, 5, 8)
#define SCU_EXTREG_SPI_WP_N_IOCTRL_GET_pd()                          GET_BIT(SCU_EXTREG_SPI_WP_N_IOCTRL, 4)
#define SCU_EXTREG_SPI_WP_N_IOCTRL_GET_pu()                          GET_BIT(SCU_EXTREG_SPI_WP_N_IOCTRL, 3)
#define SCU_EXTREG_SPI_WP_N_IOCTRL_SET_dcsr(val)                     SET_MASKED_BITS(SCU_EXTREG_SPI_WP_N_IOCTRL, val, 5, 8)
#define SCU_EXTREG_SPI_WP_N_IOCTRL_SET_pd(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_WP_N_IOCTRL, 4)
#define SCU_EXTREG_SPI_WP_N_IOCTRL_SET_pu(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_WP_N_IOCTRL, 3)

/* SPI_CLK IO control register (Offset 0x0114) */
#define SCU_EXTREG_SPI_HOLD_N_IOCTRL_GET_pd()                          GET_BIT(SCU_EXTREG_SPI_HOLD_N_IOCTRL, 4)
#define SCU_EXTREG_SPI_HOLD_N_IOCTRL_GET_dcsr()                        GET_BITS(SCU_EXTREG_SPI_HOLD_N_IOCTRL, 5, 8)
#define SCU_EXTREG_SPI_HOLD_N_IOCTRL_GET_pu()                          GET_BIT(SCU_EXTREG_SPI_HOLD_N_IOCTRL, 3)
#define SCU_EXTREG_SPI_HOLD_N_IOCTRL_SET_dcsr(val)                     SET_MASKED_BITS(SCU_EXTREG_SPI_HOLD_N_IOCTRL, val, 5, 8)
#define SCU_EXTREG_SPI_HOLD_N_IOCTRL_SET_pd(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_HOLD_N_IOCTRL, 4)
#define SCU_EXTREG_SPI_HOLD_N_IOCTRL_SET_pu(val)                       SET_MASKED_BIT(SCU_EXTREG_SPI_HOLD_N_IOCTRL, 3)

/* LC_DATA6 IO control register (Offset 0x0154) */
#define SCU_EXTREG_LC_DATA6_GET_dcsr()                      GET_BITS(SCU_EXTREG_LC_DATA6_IOCTRL, 5, 8)
#define SCU_EXTREG_LC_DATA6_GET_pd()                        GET_BIT(SCU_EXTREG_LC_DATA6_IOCTRL, 4)
#define SCU_EXTREG_LC_DATA6_GET_pu()                        GET_BIT(SCU_EXTREG_LC_DATA6_IOCTRL, 3)
#define SCU_EXTREG_LC_DATA6_SET_dcsr(val)                   SET_MASKED_BITS(SCU_EXTREG_LC_DATA6_IOCTRL, val, 5, 8)
#define SCU_EXTREG_LC_DATA6_SET_pd(val)                     SET_MASKED_BIT(SCU_EXTREG_LC_DATA6_IOCTRL, 4)
#define SCU_EXTREG_LC_DATA6_SET_pu(val)                     SET_MASKED_BIT(SCU_EXTREG_LC_DATA6_IOCTRL, 3)

/* LC_DATA7 IO control register (Offset 0x0158) */
#define SCU_EXTREG_LC_DATA7_GET_dcsr()                      GET_BITS(SCU_EXTREG_LC_DATA7_IOCTRL, 5, 8)
#define SCU_EXTREG_LC_DATA7_GET_pd()                        GET_BIT(SCU_EXTREG_LC_DATA7_IOCTRL, 4)
#define SCU_EXTREG_LC_DATA7_GET_pu()                        GET_BIT(SCU_EXTREG_LC_DATA7_IOCTRL, 3)
#define SCU_EXTREG_LC_DATA7_SET_dcsr(val)                   SET_MASKED_BITS(SCU_EXTREG_LC_DATA7_IOCTRL, val, 5, 8)
#define SCU_EXTREG_LC_DATA7_SET_pd(val)                     SET_MASKED_BIT(SCU_EXTREG_LC_DATA7_IOCTRL, 4)
#define SCU_EXTREG_LC_DATA7_SET_pu(val)                     SET_MASKED_BIT(SCU_EXTREG_LC_DATA7_IOCTRL, 3)

#define SCU_EXTREG_PRINT(__desc, __regtype, __symbol) \
    DSG("%s %s %x", __desc, #__symbol, SCU_EXTREG_##__regtype##_GET##_##__symbol())

#define SCU_EXTREG_PRINT2(__label) \
    {DSG("%s=%x", #__label, inw(__label))};
    
#define SCU_EXTREG_PRINT_ALL() \
    SCU_EXTREG_PRINT2(SCU_EXTREG_PLL0_SETTING) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_PLL1_SETTING) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_PLL2_SETTING) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_PLL3_SETTING) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_PLL4_SETTING) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_PLL5_SETTING) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_EN0) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_EN1) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_EN2) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_MUX_SEL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV0) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV1) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV2) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV3) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV4) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV5) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SWRST_MASK0) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SWRST_MASK1) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SWRST_MASK2) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SWRST) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CM4_NCPU_CTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_DDR_CTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_USB_OTG_CTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CSIRX_CTRL0) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CSIRX_CTRL1) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_DPI2AHB_CTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_MISC) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV6) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CLK_DIV7) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CSIRX_CTRL0) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_CSIRX_CTRL1) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_DPI2AHB_CTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_PCLK_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_VS_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_HS_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DE_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA0_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA1_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA2_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA3_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA4_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA5_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA6_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA7_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA8_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA9_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA10_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA11_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA12_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA13_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA14_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_LC_DATA15_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SD_CLK_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SD_CMD_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SD_DATA0_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SD_DATA1_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SD_DATA2_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SD_DATA3_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_I2C0_DATA_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_I2C0_DATA_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_PWM0_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_OTG_DRV_VBUS_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE0_IOCTRL) \
    SCU_EXTREG_PRINT2(SCU_EXTREG_SPARE1_IOCTRL) 



typedef struct {
    volatile unsigned int pll0_setting;
    volatile unsigned int pll1_setting;
    volatile unsigned int pll2_setting;
    volatile unsigned int pll3_setting;
    volatile unsigned int pll4_setting;
    volatile unsigned int clk_en0;
    volatile unsigned int clk_en1;
    volatile unsigned int clk_en2;
    volatile unsigned int clk_muxsel;
    volatile unsigned int clk_div0;
    volatile unsigned int clk_div1;
    volatile unsigned int clk_div2;
    volatile unsigned int clk_div3;
    volatile unsigned int clk_div4;
    volatile unsigned int clk_div5;
    volatile unsigned int reserved01;
    volatile unsigned int pll5_setting;    
    volatile unsigned int clk_div6;
    volatile unsigned int clk_div7;    
    volatile unsigned int reserved02[10];
    volatile unsigned int spi_cs_n_io;
    volatile unsigned int spi_clk_io;
    volatile unsigned int spi_do_io;
    volatile unsigned int spi_di_io;
    volatile unsigned int spi_wp_n_io;
    volatile unsigned int spi_hold_n_io;
    volatile unsigned int swj_trst_io;
    volatile unsigned int swj_tdi_io;
    volatile unsigned int swj_swditms_io;
    volatile unsigned int swj_swclktck_io;
    volatile unsigned int swj_swj_tdo_io;
    volatile unsigned int lc_pclk_io;
    /* To Do */
    
} kdp520_scu_extreg;


#endif
