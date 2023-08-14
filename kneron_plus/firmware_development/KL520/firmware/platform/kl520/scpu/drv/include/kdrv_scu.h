#ifndef __SCU_REG_H__
#define __SCU_REG_H__

#include "regbase.h"
#include "base.h"
#include "io.h"


#define SCU_REG_BTUP_STS                                (SCU_FTSCU100_PA_BASE + 0x0000)
#define SCU_REG_BTUP_CTRL                               (SCU_FTSCU100_PA_BASE + 0x0004)
#define SCU_REG_PWR_CTRL                                (SCU_FTSCU100_PA_BASE + 0x0008)
#define SCU_REG_PWR_MOD                                 (SCU_FTSCU100_PA_BASE + 0x0020)
#define SCU_REG_INT_STS                                 (SCU_FTSCU100_PA_BASE + 0x0024)
#define SCU_REG_INT_EN                                  (SCU_FTSCU100_PA_BASE + 0x0028)
#define SCU_REG_PLL_CTRL                                (SCU_FTSCU100_PA_BASE + 0x0030)
#define SCU_REG_PLL2_CTRL                               (SCU_FTSCU100_PA_BASE + 0x0040)
#define SCU_REG_DLL_CTRL                                (SCU_FTSCU100_PA_BASE + 0x0044)
#define SCU_REG_PWR_VCCSTS                              (SCU_FTSCU100_PA_BASE + 0x0048)
#define SCU_REG_AHBCLKG                                 (SCU_FTSCU100_PA_BASE + 0x0050)
#define SCU_REG_SLP_AHBCLKG                             (SCU_FTSCU100_PA_BASE + 0x0058)
#define SCU_REG_APBCLKG                                 (SCU_FTSCU100_PA_BASE + 0x0060)
#define SCU_REG_APBCLKG2                                (SCU_FTSCU100_PA_BASE + 0x0064)
#define SCU_REG_SLP_APBCLKG                             (SCU_FTSCU100_PA_BASE + 0x0068)
#define SCU_REG_SLP_WAKUP_ST                            (SCU_FTSCU100_PA_BASE + 0x00C0)
#define SCU_REG_SLP_WAKUP_EN                            (SCU_FTSCU100_PA_BASE + 0x00C4)
#define SCU_REG_RTC_TIME1                               (SCU_FTSCU100_PA_BASE + 0x0200)
#define SCU_REG_RTC_TIME2                               (SCU_FTSCU100_PA_BASE + 0x0204)
#define SCU_REG_RTC_ALM1                                (SCU_FTSCU100_PA_BASE + 0x0208)
#define SCU_REG_RTC_ALM2                                (SCU_FTSCU100_PA_BASE + 0x020C)
#define SCU_REG_RTC_CTRL                                (SCU_FTSCU100_PA_BASE + 0x0210)


/* Boot-up Status Register (Offset: 0x0000) */
#define SCU_REG_BTUP_STS_RTC_BTUPTS                     BIT17   // RTC Alarm
#define SCU_REG_BTUP_STS_PWRBTN_STS                     BIT16   // PWR Button
#define SCU_REG_BTUP_STS_PMR                            BIT11   // Power Off mode
#define SCU_REG_BTUP_STS_SMR                            BIT10   // Dormant mode
#define SCU_REG_BTUP_STS_WDR                            BIT9    // Watchdog reset
#define SCU_REG_BTUP_STS_HWR                            BIT8    // Hardware reset
#define SCU_REG_BTUP_STS_PMR1                           BIT7    // Power mode 1
#define SCU_REG_BTUP_STS_PMR2                           BIT6    // Power mode 2
/* Boot-up Control Register (Offset: 0x0004) */
#define SCU_REG_BTUP_CTRL_RTC_BU_EN                     BIT17
#define SCU_REG_BTUP_CTRL_PWRBTN_EN                     BIT16
#define SCU_REG_BTUP_CTRL_GPO_1_OUT                     BIT1
#define SCU_REG_BTUP_CTRL_GPO_OUT                       BIT0
/* Power Control Register (Offset: 0x0008) */
#define SCU_REG_PWR_CTRL_PWRUP_UPDATE                   BIT24
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DDRCK        BIT10 // DDRCK domain
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU          BIT9 // NPU domain
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT      BIT8 // Default domain
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_MASK                (SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DDRCK | \
                                                         SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU | \
                                                         SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT)

#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DDRCK        BIT2 // DDRCK domain
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NPU          BIT1 // NPU domain
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DEFAULT      BIT0 // Default domain
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_MASK                (SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DDRCK | \
                                                         SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NPU | \
                                                         SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DEFAULT)


/* Power Mode Register (Offset: 0x0020) */
#define SCU_REG_PWR_MOD_SELFR_CMD_OFF                   BIT31 // DDR/SDR self-refresh command off
#define SCU_REG_PWR_MOD_FCS_PLL2_RSTn                   BIT30 // PLL2 resets in the frequency change sequence
#define SCU_REG_PWR_MOD_FCS_DLL_RSTn                    BIT29 // DLL resets in the frequency change sequence
#define SCU_REG_PWR_MOD_FCS_PLL_RSTn                    BIT28 // PLL resets in the frequency change sequence
#define SCU_REG_PWR_MOD_SW_RST                          BIT10 // Softwar Reset
#define SCU_REG_PWR_MOD_FCS                             BIT6  // Frequency Change Sequence (FCS)
#define SCU_REG_PWR_MOD_BUS_SPEED                       BIT5  // BUS speed change command
#define SCU_REG_PWR_MOD_SOFTOFF                         BIT1
#define SCU_REG_PWR_MOD_SET_SOFTOFF(val)                SET_MASKED_BIT(SCU_REG_PWR_MOD, val, 1)

/* Interrupt Status Register (Offset : 0x024) */
#define SCU_REG_INT_STS_PWRSTATE_CHG                    BIT28 //Power domain state change status
#define SCU_REG_INT_STS_RTC_SEC                         BIT18 //RTC second out interrupt status
#define SCU_REG_INT_STS_RTC_PER                         BIT17 //RTC periodic interrupt status
#define SCU_REG_INT_STS_RTC_ALARM                       BIT16 //RTC alarm interrupt status
#define SCU_REG_INT_STS_PLL_UPDATE                      BIT8 //PLL update finish interrupt status
#define SCU_REG_INT_STS_FCS                             BIT6 //FCS command finish interrupt status
#define SCU_REG_INT_STS_BUSSPEED                        BIT5 //BUS speed finish interrupt status
#define SCU_REG_INT_STS_WAKEUP                          BIT3 //Wake up event status in the sleep mode
#define SCU_REG_INT_STS_PWRBTN_RISE                     BIT1 //Power button rising interrupt status at the working state
#define SCU_REG_INT_STS_PWRBTN_FALL                     BIT0 //Power button falling interrupt status at the working state

/* PLL Control Register (Offset: 0x0030) */
#define SCU_REG_PLL_CTRL_GET_CLKIN_MUX()                GET_BITS(SCU_EXTREG_USB_OTG_CTRL, 4, 5)
#define SCU_REG_PLL_CTRL_SET_CLKIN_MUX(val)             SET_MASKED_BITS(SCU_EXTREG_USB_OTG_CTRL, val, 4, 5)
#define SCU_REG_PLL_CTRL_CLKIN_MUX_BIT_START            4
#define SCU_REG_PLL_CTRL_CLKIN_MUX_MASK                 BIT4 | BIT5 | BIT6 | BIT7
#define SCU_REG_PLL_CTRL_PLLEN                          BIT0

#define SCU_REG_PLL_CTRL_SET_PLLEN(val)                 SET_MASKED_BIT(SCU_REG_DLL_CTRL, val, 0)

/* Software Reset Mask Register (Offset: 0x0040) */
#define SCU_REG_PLL2_CTRL_PLL2EN                        BIT0
#define SCU_REG_PLL2_CTRL_SET_PLL2EN(val)               SET_MASKED_BIT(SCU_REG_PLL2_CTRL, val, 0)
/* Software Reset Mask Register (Offset: 0x0044) */
#define SCU_REG_DLL_CTRL_DLLEN                          BIT0
#define SCU_REG_DLL_CTRL_SET_DLLEN(val)                 SET_MASKED_BIT(SCU_REG_DLL_CTRL, val, 0)
/* Power Domain Voltage Supplied Status Register (Offset: 0x0048) */
#define SCU_REG_PWR_VCCSTS_GET_PWR_READY()              GET_BITS(SCU_REG_PWR_VCCSTS, 0, 2)
#define SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DDRCK       BIT2
#define SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_NPU         BIT1
#define SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DEFAULT     BIT0
#define SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_MASK        (SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DDRCK | \
                                                         SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_NPU | \
                                                         SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DEFAULT)

/* AHB Clock Control Register (AHBCLKG, 0x050~0x054) in working state */
#define SCU_REG_AHBCLKG_HCLK_EN_DPI1_HCLK               BIT21
#define SCU_REG_AHBCLKG_HCLK_EN_DPI0_HCLK               BIT20

/* APB Clock Control Register (0x060, 0x64) in working state */
#define SCU_REG_APBCLKG2_PCLK_EN_MIPI_RX1_PHY_PCLK      BIT1
#define SCU_REG_APBCLKG2_PCLK_EN_MIPI_RX0_PHY_PCLK      BIT0

#define SCU_REG_APBCLKG_PCLK_EN_CSIRX1_PCLK             BIT29
#define SCU_REG_APBCLKG_PCLK_EN_CSIRX0_PCLK             BIT28
#define SCU_REG_APBCLKG_PCLK_EN_GPIO_PCLK               BIT20
#define SCU_REG_APBCLKG_PCLK_EN_PWM_PCLK                BIT17
#define SCU_REG_APBCLKG_PCLK_EN_I2C3_PCLK               BIT5
#define SCU_REG_APBCLKG_PCLK_EN_I2C2_PCLK               BIT4
#define SCU_REG_APBCLKG_PCLK_EN_I2C1_PCLK               BIT3
#define SCU_REG_APBCLKG_PCLK_EN_I2C0_PCLK               BIT2

#define SCU_REG_APBCLKG2                                (SCU_FTSCU100_PA_BASE + 0x0064)

/* Wakeup Event Status Register (Offset : 0x0C0) */
#define SCU_REG_SLP_WAUP_EN_GET_SLP_WAKUP_ST0()         GET_BIT(SCU_REG_SLP_WAKUP_ST, 0)
#define SCU_REG_SLP_WAUP_EN_SET_SLP_WAKUP_ST0(val)      SET_MASKED_BIT(SCU_REG_SLP_WAKUP_ST, val, 0)

/* Wakeup Event Enable Register (Offset : 0x0C4) */
#define SCU_REG_SLP_WAUP_EN_GET_SLP_WAKUP_EN0()         GET_BIT(SCU_REG_SLP_WAKUP_EN, 0)
#define SCU_REG_SLP_WAUP_EN_SET_SLP_WAKUP_EN0(val)      SET_MASKED_BIT(SCU_REG_SLP_WAKUP_EN, val, 0)


#endif
