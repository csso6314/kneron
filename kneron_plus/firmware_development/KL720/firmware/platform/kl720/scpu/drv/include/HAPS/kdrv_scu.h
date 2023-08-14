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

/**@addtogroup  KDRV_SCU    KDRV_SCU
 * @{
 * @brief       Kneron SCU driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
*/

/* This file is for 9x9 package which has no MRX PMIC */

#ifndef _KDRV_SCU_H
#define _KDRV_SCU_H

#define SCU_REG_BTUP_STS            (SCU_REG_BASE + 0x000)  //Boot-up status register
#define SCU_REG_BTUP_CTRL           (SCU_REG_BASE + 0x004)  //Boot-up control register
#define SCU_REG_PWR_CTRL            (SCU_REG_BASE + 0x008)  //Power control register
#define SCU_REG_PWRUP_SEQ           (SCU_REG_BASE + 0x00C)  //Power-up sequence control register
#define SCU_REG_CHIPID              (SCU_REG_BASE + 0x010)  //Chip ID register
#define SCU_REG_VERID               (SCU_REG_BASE + 0x014)  //Status control unit version register
#define SCU_REG_STRAP               (SCU_REG_BASE + 0x018)  //Strap value register
#define SCU_REG_PWR_MOD             (SCU_REG_BASE + 0x020)  //Power mode register
#define SCU_REG_INT_STS             (SCU_REG_BASE + 0x024)  //Interrupt status register
#define SCU_REG_INT_EN              (SCU_REG_BASE + 0x028)  //Interrupt enable register
#define SCU_REG_SWRST_CTRL          (SCU_REG_BASE + 0x02C)  //Software reset control register
#define SCU_REG_PLL_CTRL            (SCU_REG_BASE + 0x030)  //Traditional PLL control register
#define SCU_REG_PWR_VCCSTS          (SCU_REG_BASE + 0x048)  //Power domain voltage supplied status register
#define SCU_REG_AHBCLKG             (SCU_REG_BASE + 0x050)  //AHB clock control register
#define SCU_REG_SLP_AHBCLKG         (SCU_REG_BASE + 0x058)  //AHB clock sleep control register
#define SCU_REG_APBCLKG             (SCU_REG_BASE + 0x060)  //APB clock control register
#define SCU_REG_SLP_APBCLKG         (SCU_REG_BASE + 0x068)  //APB clock sleep control register
#define SCU_REG_AXICLKG             (SCU_REG_BASE + 0x080)  //AXI clock gated register
#define SCU_REG_SLP_AXICLKG         (SCU_REG_BASE + 0x088)  //AXI clock sleep control register
#define SCU_REG_SLP_WAKUP_ST        (SCU_REG_BASE + 0x0C0)  //Sleep wakeup events status
#define SCU_REG_SLP_WAKUP_EN        (SCU_REG_BASE + 0x0C4)  //Sleep wakeup events enable
#define SCU_REG_RTC_TIME1           (SCU_REG_BASE + 0x200)  //RTC timer register 1
#define SCU_REG_RTC_TIME2           (SCU_REG_BASE + 0x204)  //RTC timer register 2
#define SCU_REG_RTC_ALM1            (SCU_REG_BASE + 0x208)  //RTC alarm time register 1
#define SCU_REG_RTC_ALM2            (SCU_REG_BASE + 0x20C)  //RTC alarm time register 2
#define SCU_REG_RTC_CTRL            (SCU_REG_BASE + 0x210)  //RTC control register
#define SCU_REG_RTC_TRIM            (SCU_REG_BASE + 0x214)  //RTC tick trim control register
#define SCU_REG_BCD_RTC_TIME1       (SCU_REG_BASE + 0x220)  //RTC timer register 1 for BCD
#define SCU_REG_BCD_RTC_TIME2       (SCU_REG_BASE + 0x224)  //RTC timer register 2 for BCD
#define SCU_REG_BCD_RTC_ALM1        (SCU_REG_BASE + 0x228)  //RTC alarm time register 1 for BCD
#define SCU_REG_BCD_RTC_ALM2        (SCU_REG_BASE + 0x22C)  //RTC alarm time register 2 for BCD

/* Boot-up Status Register (Offset: 0x0000) */

#define SCU_REG_BTUP_STS_RTC_BTUPTS                     BIT17   // RTC Alarm
#define SCU_REG_BTUP_STS_PWRBTN_STS                     BIT16   // PWR Button
#define SCU_REG_BTUP_STS_PMR                            BIT11   // Power Off mode
#define SCU_REG_BTUP_STS_SMR                            BIT10   // Dormant mode
#define SCU_REG_BTUP_STS_WDR                            BIT9    // Watchdog reset
#define SCU_REG_BTUP_STS_PMR1                           BIT7    // Power mode 1
/* Boot-up Control Register (Offset: 0x0004) */
#define SCU_REG_BTUP_CTRL_EXT_WAK_BTN_BU_EN             BIT20
#define SCU_REG_BTUP_CTRL_EXT_U3_BU_EN                  BIT19   // U3_BU_EN denote USB3 in super-speed mode.
#define SCU_REG_BTUP_CTRL_EXT_U2_BU_EN                  BIT18   // U2_BU_EN denote USB3 in high-speed mode.
#define SCU_REG_BTUP_CTRL_RTC_BU_EN                     BIT17
#define SCU_REG_BTUP_CTRL_PWRBTN_EN                     BIT16
//#define SCU_REG_BTUP_CTRL_GPO_OUT                       BIT0
/* Power Control Register (Offset: 0x0008) */
#define SCU_REG_PWR_CTRL_PWRUP_UPDATE                   BIT24
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DCK          BIT14
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM          BIT13
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU          BIT12

#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_MRX          0       //there is no MRX PMIX for 9x9 package

#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UHO          BIT10
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR          BIT9
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS          BIT8
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NONE         0
#define SCU_REG_PWR_CTRL_PWRUP_CTRL_MASK                (SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | \
                                                         SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU | \
                                                         SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_MRX | \
                                                         SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UHO | \
                                                         SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR | \
                                                         SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS)

#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DCK          BIT6
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NOM          BIT5
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NPU          BIT4
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_MRX          BIT3
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_UHO          BIT2
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_UDR          BIT1
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_BAS          BIT0
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NONE         0
#define SCU_REG_PWR_CTRL_PWRDN_CTRL_MASK                (SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NOM | \
                                                         SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NPU | \
                                                         SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_MRX | \
                                                         SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_UHO | \
                                                         SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_UDR | \
                                                         SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_BAS)

/* SCU_REG_INT_EN & SCU_REG_INT_STS */
#define SCU_INT_PWRSTATE_CHG        BIT28
#define SCU_INT_RTC_SEC             BIT18
#define SCU_INT_RTC_PERIODIC        BIT17
#define SCU_INT_RTC_ALARM           BIT16
#define SCU_INT_PLL_UPDATE          BIT8
#define SCU_INT_FCS                 BIT6
#define SCU_INT_BUSSPEED            BIT5
#define SCU_INT_WAKEUP              BIT3
#define SCU_INT_PWRBTN_RISE         BIT1    //Beethoven not support
#define SCU_INT_PWRBTN_FALL         BIT0    //Beethoven not support

/** @brief Union of all SCU registers */
typedef volatile union U_regSCU{
    struct
    {
        uint32_t btup_sts;          //(0x000)       Boot-up status register
        uint32_t btup_ctrl;         //(0x004)       Boot-up control register
        uint32_t pwr_ctrl;          //(0x008)       Power control register
        uint32_t pwrup_seq;         //(0x00C)       Power-up sequence control register
        uint32_t chipid;            //(0x010)       Chip ID register
        uint32_t verid;             //(0x014)       Status control unit version register
        uint32_t strap;             //(0x018)       Strap value register
        uint32_t rsvd_0;            //(0x01C)       rsvd_0
        uint32_t pwr_mod;           //(0x020)       Power mode register
        uint32_t int_sts;           //(0x024)       Interrupt status register
        uint32_t int_en;            //(0x028)       Interrupt enable register
        uint32_t swrst_ctrl;        //(0x02C)       Software reset control register
        uint32_t pll_ctrl;          //(0x030)       Traditional PLL control register
        uint32_t rsvd_1[5];         //(0x034~0x044) rsvd_1
        uint32_t pwr_vccsts;        //(0x048)       Power domain voltage supplied status register
        uint32_t rsvd_2;            //(0x04C)       rsvd_2
        uint32_t ahbclkg;           //(0x050)       AHB clock control register
        uint32_t rsvd_3;            //(0x054)       rsvd_3
        uint32_t slp_ahbclkg;       //(0x058)       AHB clock sleep control register
        uint32_t rsvd_4;            //(0x05C)       rsvd_4
        uint32_t apbclkg[2];        //(0x060)       APB clock control register
        uint32_t slp_apbclkg[2];    //(0x068)       APB clock sleep control register
        uint32_t rsvd_5[4];         //(0x070~0x07C) rsvd_5
        uint32_t axiclkg;           //(0x080)       AXI clock gated register
        uint32_t rsvd_6;            //(0x084)       rsvd_6
        uint32_t slp_axiclkg;       //(0x088)       AXI clock sleep control register
        uint32_t rsvd_7[13];        //(0x08C~0x0BC) rsvd_7
        uint32_t slp_wakup_st;      //(0x0C0)       Sleep wakeup events status
        uint32_t slp_wakup_en;      //(0x0C4)       Sleep wakeup events enable
        uint32_t rsvd_8[78];        //(0x0C8~0x1FC) rsvd_8
        uint32_t rtc_time1;         //(0x200)       RTC timer register 1
        uint32_t rtc_time2;         //(0x204)       RTC timer register 2
        uint32_t rtc_alm1;          //(0x208)       RTC alarm time register 1
        uint32_t rtc_alm2;          //(0x20C)       RTC alarm time register 2
        uint32_t rtc_ctrl;          //(0x210)       RTC control register
        uint32_t rtc_trim;          //(0x214)       RTC tick trim control register
        uint32_t rsvd_9[2];         //(0x218~0x21C) rsvd_9
        uint32_t bcd_rtc_time1;     //(0x220)       RTC timer register 1 for BCD
        uint32_t bcd_rtc_time2;     //(0x224)       RTC timer register 2 for BCD
        uint32_t bcd_rtc_alm1;      //(0x228)       RTC alarm time register 1 for BCD
        uint32_t bcd_rtc_alm2;      //(0x22C)       RTC alarm time register 2 for BCD
    }dw;
    struct{
        //(0x000)  //Boot-up status register
        uint32_t rsvd1              : 7;
        uint32_t pmr1               : 1;
        uint32_t rsvd2              : 1;
        uint32_t wdr                : 1;
        uint32_t smr                : 1;
        uint32_t pmr                : 1;
        uint32_t emi_reboot         : 1;
        uint32_t pwrbtn             : 1;
        uint32_t rsvd3              : 3;
        uint32_t rtc_btup_sys       : 1;
        uint32_t ext_bootup_sys     : 3;
        uint32_t rsvd4              :11;

        //(0x004)  //Boot-up control register
        uint32_t gpo_out            :16;
        uint32_t pwrbtn_en          : 1;
        uint32_t rtc_btup_en        : 1;
        uint32_t ext_boot_en        : 3;
        uint32_t rsvd5              :11;

        //(0x008)  //Power control register
        uint32_t pwrdn_ctrl         : 8;
        uint32_t pwrup_ctrl         : 8;
        uint32_t debounce_mode      : 8;
        uint32_t pwrctrl_update     : 1;
        uint32_t rsvd6              : 3;
        uint32_t pwrctrl_dcsr       : 4;

        //(0x00C)  //Power-up sequence control register
        uint32_t pwrup_enslot1      : 8;
        uint32_t pwrup_enslot2      : 8;
        uint32_t pwrup_enslot3      : 8;
        uint32_t pwrup_enslot4      : 8;

        //(0x010)  //Chip ID register
        uint32_t chip_id            :32;

        //(0x014)  //Status control unit version register
        uint32_t ip_version         :32;

        //(0x018)  //Strap value register
        uint32_t rsvd7              : 8;
        uint32_t user_strap         :24;

        //(0x01C)
        uint32_t rsvd7_             :32;

        //(0x020)  //Power mode register
        uint32_t rsvd8              : 1;
        uint32_t softoff            : 1;
        uint32_t rsvd9              : 1;
        uint32_t sleep_             : 1;
        uint32_t rsvd10             : 1;
        uint32_t bus_speed          : 1;
        uint32_t fcs                : 1;
        uint32_t rsvd11             : 1;
        uint32_t pll_update         : 1;
        uint32_t curr_restore       : 1;
        uint32_t sw_rst             : 1;
        uint32_t rsvd12             : 9;
        uint32_t bus_mux            : 4;
        uint32_t rsvd13             : 8;

        //(0x024)  //Interrupt status register
        uint32_t rsvd14             : 3;
        uint32_t int_wakeup         : 1;
        uint32_t rsvd15             : 1;
        uint32_t int_busspeed       : 1;
        uint32_t int_fcs            : 1;
        uint32_t rsvd16             : 1;
        uint32_t int_pll_update     : 1;
        uint32_t rsvd17             : 7;
        uint32_t int_rtc_alarm      : 1;
        uint32_t int_rtc_per        : 1;
        uint32_t int_rtc_sec        : 1;
        uint32_t rsvd18             : 9;
        uint32_t int_pwrsta_chg     : 1;
        uint32_t rsvd20             : 3;

        //(0x028)  //Interrupt enable register
        uint32_t rsvd21             : 3;
        uint32_t wakeup_eint        : 1;
        uint32_t rsvd22             : 1;
        uint32_t busspeed_eint      : 1;
        uint32_t fcs_eint           : 1;
        uint32_t rsvd23             : 1;
        uint32_t pll_update_eint    : 1;
        uint32_t rsvd24             : 7;
        uint32_t rtc_alarm_eint     : 1;
        uint32_t rtc_per_eint       : 1;
        uint32_t rtc_sec_eint       : 1;
        uint32_t rsvd25             : 9;
        uint32_t pwrsta_chg_eint    : 1;
        uint32_t rsvd26             : 3;

        //(0x02C)  //Software reset control register
        uint32_t swrst_active       : 5;
        uint32_t rsvd27             :11;
        uint32_t swrst_wait         : 5;
        uint32_t rsvd28             :11;

        //(0x030)  //Traditional PLL control register
        uint32_t pllen              : 1;
        uint32_t pllstable          : 1;
        uint32_t rsvd29             : 2;
        uint32_t clkin_mux          : 4;
        uint32_t pllps              : 3;
        uint32_t rsvd30             : 5;
        uint32_t pllms              : 3;
        uint32_t pllns_8            : 1;
        uint32_t pllis              : 2;
        uint32_t rsvd31             : 2;
        uint32_t pllns_0_7          : 8;

        //(0x034~0x044)
        uint32_t rsvd32[5];

        //(0x048)  //Power domain voltage supplied status register
        uint32_t pwr_ready          : 8;
        uint32_t pwr_enable_sts     : 8;
        uint32_t rsvd33             :16;

        //(0x04C)
        uint32_t rsvd33_            :32;

        //(0x050)  //AHB clock control register
        uint32_t dpi2ahb_0_hclk     : 1;
        uint32_t dpi2ahb_1_hclk     : 1;
        uint32_t spi2ahb_hclk       : 1;
        uint32_t dmac_hclk          : 1;
        uint32_t aes_hclk           : 1;
        uint32_t sdc_hclk           : 1;
        uint32_t sdc_u1_hclk        : 1;
        uint32_t cpu_clk            : 1;
        uint32_t ahb_hclk           : 1;
        uint32_t ahb2rom_hclk       : 1;
        uint32_t spi_hclk           : 1;
        uint32_t h2p_hclk           : 1;
        uint32_t h2x_0_hclk         : 1;
        uint32_t h2x_1_hclk         : 1;
        uint32_t h2x_2_hclk         : 1;
        uint32_t x2h_hclk           : 1;
        uint32_t ahb2ram_hclk       : 1;
        uint32_t rsvd34             :15;

        //(0x054)
        uint32_t rsvd34_            :32;

        //(0x058)  //AHB clock sleep control register
        uint32_t dpi2ahb_0_s_hclk   : 1;
        uint32_t dpi2ahb_1_s_hclk   : 1;
        uint32_t spi2ahb_s_hclk     : 1;
        uint32_t dmac_s_hclk        : 1;
        uint32_t aes_s_hclk         : 1;
        uint32_t sdc_s_hclk         : 1;
        uint32_t sdc_u1_s_hclk      : 1;
        uint32_t cpu_s_clk          : 1;
        uint32_t ahb_s_hclk         : 1;
        uint32_t ahb2rom_s_hclk     : 1;
        uint32_t spi_s_hclk         : 1;
        uint32_t h2p_s_hclk         : 1;
        uint32_t h2x_0_s_hclk       : 1;
        uint32_t h2x_1_s_hclk       : 1;
        uint32_t h2x_2_s_hclk       : 1;
        uint32_t x2h_s_hclk         : 1;
        uint32_t ahb2ram_s_hclk     : 1;
        uint32_t rsvd35             :15;

        //(0x05C)
        uint32_t rsvd35_            :32;

        //(0x060)  //APB clock control register
        uint32_t axi_pclk           : 1;
        uint32_t ddr_pclk           : 1;
        uint32_t dmac_pclk          : 1;
        uint32_t uart0_pclk         : 1;
        uint32_t uart1_pclk         : 1;
        uint32_t gpio_pclk          : 1;
        uint32_t i2c0_pclk          : 1;
        uint32_t i2c1_pclk          : 1;
        uint32_t i2c2_pclk          : 1;
        uint32_t tmr_pclk           : 1;
        uint32_t ssp0_pclk          : 1;
        uint32_t ssp1_pclk          : 1;
        uint32_t efuse_pclk         : 1;
        uint32_t mipi_rx0_pclk      : 1;
        uint32_t mipi_rx1_pclk      : 1;
        uint32_t dpi2ahb_0_pclk     : 1;
        uint32_t dpi2ahb_1_pclk     : 1;
        uint32_t rsvd36             : 4;
        uint32_t wdt_pclk           : 1;
        uint32_t x2h_pclk           : 1;
        uint32_t npu_pclk           : 1;
        uint32_t dsp_pclk           : 1;
        uint32_t rsvd37             : 1;
        uint32_t usb3_apb_clk       : 1;
        uint32_t usb2_apb_clk       : 1;
        uint32_t lc_pclk            : 1;
        uint32_t tdc_pclk           : 1;
        uint32_t pwm_pclk           : 1;
        uint32_t h2x_0_pclk         : 1;

        //(0x064)  //APB clock control register
        uint32_t h2x_1_pclk         : 1;
        uint32_t h2x_2_pclk         : 1;
        uint32_t x2p_pclk           : 1;
        uint32_t x2x_u0_pclk        : 1;
        uint32_t x2x_u1_pclk        : 1;
        uint32_t x2x_u1_1_pclk      : 1;
        uint32_t x2x_u2_pclk        : 1;
        uint32_t x2x_u3_pclk        : 1;
        uint32_t x2x_u3_1_pclk      : 1;
        uint32_t x2x_u4_pclk        : 1;
        uint32_t x2x_u5_pclk        : 1;
        uint32_t x2x_u7_1_pclk      : 1;
        uint32_t x2x_u7_pclk        : 1;
        uint32_t x2x_u6_pclk        : 1;
        uint32_t x2x_u8_pclk        : 1;
        uint32_t x2x_u9_pclk        : 1;
        uint32_t efuse_128_pclk     : 1;
        uint32_t rsvd38             :15;

        //(0x068)  //APB clock sleep control register
        uint32_t axi_s_pclk         : 1;
        uint32_t ddr_s_pclk         : 1;
        uint32_t dmac_s_pclk        : 1;
        uint32_t uart0_s_pclk       : 1;
        uint32_t uart1_s_pclk       : 1;
        uint32_t gpio_s_pclk        : 1;
        uint32_t i2c0_s_pclk        : 1;
        uint32_t i2c1_s_pclk        : 1;
        uint32_t i2c2_s_pclk        : 1;
        uint32_t tmr_s_pclk         : 1;
        uint32_t ssp0_s_pclk        : 1;
        uint32_t ssp1_s_pclk        : 1;
        uint32_t efuse_s_pclk       : 1;
        uint32_t mipi_rx0_s_pclk    : 1;
        uint32_t mipi_rx1_s_pclk    : 1;
        uint32_t dpi2ahb_0_s_pclk   : 1;
        uint32_t dpi2ahb_1_s_pclk   : 1;
        uint32_t rsvd39             : 4;
        uint32_t wdt_s_pclk         : 1;
        uint32_t x2h_s_pclk         : 1;
        uint32_t npu_s_pclk         : 1;
        uint32_t dsp_s_pclk         : 1;
        uint32_t rsvd40             : 1;
        uint32_t usb3_apb_s_clk     : 1;
        uint32_t usb2_apb_s_clk     : 1;
        uint32_t lc_s_pclk          : 1;
        uint32_t tdc_s_pclk         : 1;
        uint32_t pwm_s_pclk         : 1;
        uint32_t h2x_0_s_pclk       : 1;

        //(0x06C)  //APB clock sleep control register
        uint32_t h2x_1_s_pclk       : 1;
        uint32_t h2x_2_s_pclk       : 1;
        uint32_t x2p_s_pclk         : 1;
        uint32_t x2x_u0_s_pclk      : 1;
        uint32_t x2x_u1_s_pclk      : 1;
        uint32_t x2x_u1_1_s_pclk    : 1;
        uint32_t x2x_u2_s_pclk      : 1;
        uint32_t x2x_u3_s_pclk      : 1;
        uint32_t x2x_u3_1_s_pclk    : 1;
        uint32_t x2x_u4_s_pclk      : 1;
        uint32_t x2x_u5_s_pclk      : 1;
        uint32_t x2x_u7_1_s_pclk    : 1;
        uint32_t x2x_u7_s_pclk      : 1;
        uint32_t x2x_u6_s_pclk      : 1;
        uint32_t x2x_u8_s_pclk      : 1;
        uint32_t x2x_u9_s_pclk      : 1;
        uint32_t efuse_128_s_pclk   : 1;
        uint32_t rsvd41             :15;

        //(0x70~0x7C)
        uint32_t rsvd42[4];

        //(0x080)  //AXI clock gated register
        uint32_t axiclk_en_0        : 1;    //axic_aclk / x2h_aclk / h2x_0_aclk /h2x_1_aclk / h2x_2_aclk / x2x_u0_aclkm /x2x_u1_aclks / x2x_u1_1_aclks / x2x_u2_aclks /x2x_u3_aclkm / x2x_u3_1_aclkm / x2x_u4_aclks /x2x_u5_aclkm / x2x_u7_1_aclkm /x2x_u7_aclkm / x2x_u6_aclks / x2x_u8_aclks /x2x_u9_aclkm
        uint32_t axiclk_en_1        : 1;    //dmac_aclk / x2x_u0_aclks /x2x_u1_aclkm
        uint32_t axiclk_en_2        : 1;    //ddr_aclk
        uint32_t axiclk_en_3        : 1;    //usb3_0_aclk_g / x2x_u2_aclkm /
        uint32_t axiclk_en_4        : 1;    //usb2_aclk_g / x2x_u4_aclkm / x2x_u3_1_aclks
        uint32_t axiclk_en_5        : 1;    //x2p_aclk / x2x_u1_1_aclkm
        uint32_t axiclk_en_6        : 1;    //lcdc_aclk_g / x2x_u9_aclks
        uint32_t rsvd43             :25;

        //(0x084)       rsvd
        uint32_t rsvd43_            :32;       //

        //(0x088)  //AXI clock sleep control register
        uint32_t slp_aclk_en_0      : 1;    //axic_aclk / x2h_aclk / h2x_0_aclk /h2x_1_aclk / h2x_2_aclk / x2x_u0_aclkm /x2x_u1_aclks / x2x_u1_1_aclks / x2x_u2_aclks /x2x_u3_aclkm / x2x_u3_1_aclkm / x2x_u4_aclks /x2x_u5_aclkm / x2x_u7_1_aclkm /x2x_u7_aclkm / x2x_u6_aclks / x2x_u8_aclks /x2x_u9_aclkm
        uint32_t slp_aclk_en_1      : 1;    //dmac_aclk / x2x_u0_aclks /x2x_u1_aclkm
        uint32_t slp_aclk_en_2      : 1;    //ddr_aclk
        uint32_t slp_aclk_en_3      : 1;    //usb3_0_aclk_g / x2x_u2_aclkm /x2x_u3_aclks
        uint32_t slp_aclk_en_4      : 1;    //usb2_aclk_g / x2x_u4_aclkm /x2x_u3_1_aclks
        uint32_t slp_aclk_en_5      : 1;    //x2p_aclk / x2x_u1_1_aclkm
        uint32_t slp_aclk_en_6      : 1;    //lcdc_aclk_g / x2x_u9_aclks
        uint32_t rsvd44             :25;

        //(0x8C~0xBC)
        uint32_t rsvd45[13];

        //(0x0C0)  //Sleep wakeup events status
        uint32_t slp_wakup_st0      : 1;    //Wakeup from sleep by the boot-up events
        uint32_t slp_wakup_stx      : 1;    //Wakeup from sleep_wakup_in pin x [1] RTC alarm.
        uint32_t rsvd46             :30;

        //(0x0C4)  //Sleep wakeup events enable
        uint32_t slp_wakup_en0      : 1;    //Wakeup enable for the boot-up events
        uint32_t slp_wakup_enx      : 1;    //Wakeup enable for boot-up events for pin x [1] RTC alarm.
        uint32_t rsvd47             :30;

        //(0xC8~0x1FC)
        uint32_t rsvd48[78];

        //(0x200)  //RTC timer register 1
        uint32_t rtc_sec            : 7;
        uint32_t rsvd49             : 1;
        uint32_t rtc_min            : 7;
        uint32_t rsvd50             : 1;
        uint32_t rtc_hour           : 6;
        uint32_t rsvd51             : 2;
        uint32_t rtc_weekday        : 3;
        uint32_t rsvd52             : 5;

        //(0x204)  //RTC timer register 2
        uint32_t rtc_date           : 6;
        uint32_t rsvd53             : 2;
        uint32_t rtc_month          : 5;
        uint32_t rsvd54             : 3;
        uint32_t rtc_year           : 8;
        uint32_t rtc_century        : 8;

        //(0x208)  //RTC alarm time register 1
        uint32_t alm_sec            : 7;
        uint32_t rsvd55             : 1;
        uint32_t alm_min            : 7;
        uint32_t rsvd56             : 1;
        uint32_t alm_hour           : 6;
        uint32_t rsvd57             : 2;
        uint32_t alm_weekday        : 3;
        uint32_t rsvd58             : 5;

        //(0x20C)  //RTC alarm time register 2
        uint32_t alm_date_          : 6;
        uint32_t rsvd59             : 2;
        uint32_t alm_month          : 5;
        uint32_t rsvd60             :19;

        //(0x210)  //RTC control register
        uint32_t rtc_en             : 1;
        uint32_t rtc_alarm_en       : 1;
        uint32_t lock_en            : 1;
        uint32_t rsvd61             : 1;
        uint32_t perint_sel         : 3;
        uint32_t secout_en          : 1;
        uint32_t rtcen_sts          : 1;
        uint32_t rtc_almen_sts      : 1;
        uint32_t rsvd62             : 1;
        uint32_t pwuten_sts         : 1;
        uint32_t rsvd63             : 3;
        uint32_t rtc_clk_ready      : 1;
        uint32_t rtc_gpo            : 8;
        uint32_t rsvd64             : 8;

        //(0x214)  //RTC tick trim control register
        uint32_t trim_f             :16;
        uint32_t trim_p             :16;

        //(0x218~0x21C) rsvd
        uint32_t rsvd64_[2];

        //(0x220)  //RTC timer register 1 for BCD
        uint32_t bcd_rtc_sec        : 4;
        uint32_t bcd_rtc_sec_dec    : 4;
        uint32_t bcd_rtc_min        : 4;
        uint32_t bcd_rtc_min_dec    : 4;
        uint32_t bcd_rtc_hour       : 4;
        uint32_t bcd_rtc_hour_dec   : 4;
        uint32_t bcd_rtc_weekday    : 3;
        uint32_t rsvd65             : 5;

        //(0x224)  //RTC timer register 2 for BCD
        uint32_t bcd_rtc_date       : 4;
        uint32_t bcd_rtc_date_dec   : 4;
        uint32_t bcd_rtc_month      : 4;
        uint32_t bcd_rtc_month_dec  : 4;
        uint32_t bcd_rtc_year       : 4;
        uint32_t bcd_rtc_year_dec   : 4;
        uint32_t bcd_rtc_century    : 4;
        uint32_t bcd_rtc_century_dec: 4;

        //(0x228)  //RTC alarm time register 1 for BCD
        uint32_t bcd_alm_sec        : 4;
        uint32_t bcd_alm_sec_dec    : 4;
        uint32_t bcd_alm_min        : 4;
        uint32_t bcd_alm_min_dec    : 4;
        uint32_t bcd_alm_hour       : 4;
        uint32_t bcd_alm_hour_dec   : 4;
        uint32_t bcd_alm_weekday    : 3;
        uint32_t rsvd66             : 5;

        //(0x22C)  //RTC alarm time register 2 for BCD
        uint32_t alm_date           : 4;
        uint32_t bcd_alm_date_dev   : 4;
        uint32_t bcd_alm_month      : 4;
        uint32_t bck_alm_month_dev  : 4;
        uint32_t rsvd67             :16;
    }bf;
}U_regSCU;
#define regSCU  ((U_regSCU*) SCU_REG_BASE)

/** @brief Union of all SCU clocks */
typedef volatile union U_regSCU_clk_en{
    struct
    {
        uint32_t AHBCLKG;           //(0x050)       AHB clock control register
        uint32_t RSVD_3;            //(0x054)       rsvd_3
        uint32_t SLP_AHBCLKG;       //(0x058)       AHB clock sleep control register
        uint32_t APBCLKG[2];        //(0x060)       APB clock control register
        uint32_t SLP_APBCLKG[2];    //(0x068)       APB clock sleep control register
        uint32_t RSVD_4[4];         //(0x070~0x07C) rsvd_4
        uint32_t AXICLKG;           //(0x080)       AXI clock gated register
    }dw;
    struct
    {
        //(0x050)  //AHB clock ontrol register
        uint32_t DPI2AHB_0_HCLK     : 1;
        uint32_t DPI2AHB_1_HCLK     : 1;
        uint32_t SPI2AHB_HCLK       : 1;
        uint32_t DMAC_HCLK          : 1;
        uint32_t AES_HCLK           : 1;
        uint32_t SDC_HCLK           : 1;
        uint32_t SDC_U1_HCLK        : 1;
        uint32_t CPU_CLK            : 1;
        uint32_t AHB_HCLK           : 1;
        uint32_t AHB2ROM_HCLK       : 1;
        uint32_t SPI_HCLK           : 1;
        uint32_t H2P_HCLK           : 1;
        uint32_t H2X_0_HCLK         : 1;
        uint32_t H2X_1_HCLK         : 1;
        uint32_t H2X_2_HCLK         : 1;
        uint32_t X2H_HCLK           : 1;
        uint32_t AHB2RAM_HCLK       : 1;
        uint32_t RSVD34             :15;

        //(0x054)
        uint32_t RSVD34_            :32;

        //(0x058)  //AHB clock slee control register
        uint32_t DPI2AHB_0_S_HCLK   : 1;
        uint32_t DPI2AHB_1_S_HCLK   : 1;
        uint32_t SPI2AHB_S_HCLK     : 1;
        uint32_t DMAC_S_HCLK        : 1;
        uint32_t AES_S_HCLK         : 1;
        uint32_t SDC_S_HCLK         : 1;
        uint32_t SDC_U1_S_HCLK      : 1;
        uint32_t CPU_S_CLK          : 1;
        uint32_t AHB_S_HCLK         : 1;
        uint32_t AHB2ROM_S_HCLK     : 1;
        uint32_t SPI_S_HCLK         : 1;
        uint32_t H2P_S_HCLK         : 1;
        uint32_t H2X_0_S_HCLK       : 1;
        uint32_t H2X_1_S_HCLK       : 1;
        uint32_t H2X_2_S_HCLK       : 1;
        uint32_t X2H_S_HCLK         : 1;
        uint32_t AHB2RAM_S_HCLK     : 1;
        uint32_t RSVD35             :15;

        //(0x05C)
        uint32_t RSVD35_            :32;

        //(0x060)  //APB clock contol register
        uint32_t AXI_PCLK           : 1;
        uint32_t DDR_PCLK           : 1;
        uint32_t DMAC_PCLK          : 1;
        uint32_t UART0_PCLK         : 1;
        uint32_t UART1_PCLK         : 1;
        uint32_t GPIO_PCLK          : 1;
        uint32_t I2C0_PCLK          : 1;
        uint32_t I2C1_PCLK          : 1;
        uint32_t I2C2_PCLK          : 1;
        uint32_t TMR_PCLK           : 1;
        uint32_t SSP0_PCLK          : 1;
        uint32_t SSP1_PCLK          : 1;
        uint32_t EFUSE_PCLK         : 1;
        uint32_t MIPI_RX0_PCLK      : 1;
        uint32_t MIPI_RX1_PCLK      : 1;
        uint32_t DPI2AHB_0_PCLK     : 1;
        uint32_t DPI2AHB_1_PCLK     : 1;
        uint32_t RSVD36             : 4;
        uint32_t WDT_PCLK           : 1;
        uint32_t X2H_PCLK           : 1;
        uint32_t NPU_PCLK           : 1;
        uint32_t DSP_PCLK           : 1;
        uint32_t RSVD37             : 1;
        uint32_t USB3_APB_CLK       : 1;
        uint32_t USB2_APB_CLK       : 1;
        uint32_t LC_PCLK            : 1;
        uint32_t TDC_PCLK           : 1;
        uint32_t PWM_PCLK           : 1;
        uint32_t H2X_0_PCLK         : 1;

        //(0x064)  //APB clock contol register
        uint32_t H2X_1_PCLK         : 1;
        uint32_t H2X_2_PCLK         : 1;
        uint32_t X2P_PCLK           : 1;
        uint32_t X2X_U0_PCLK        : 1;
        uint32_t X2X_U1_PCLK        : 1;
        uint32_t X2X_U1_1_PCLK      : 1;
        uint32_t X2X_U2_PCLK        : 1;
        uint32_t X2X_U3_PCLK        : 1;
        uint32_t X2X_U3_1_PCLK      : 1;
        uint32_t X2X_U4_PCLK        : 1;
        uint32_t X2X_U5_PCLK        : 1;
        uint32_t X2X_U7_1_PCLK      : 1;
        uint32_t X2X_U7_PCLK        : 1;
        uint32_t X2X_U6_PCLK        : 1;
        uint32_t X2X_U8_PCLK        : 1;
        uint32_t X2X_U9_PCLK        : 1;
        uint32_t EFUSE_128_PCLK     : 1;
        uint32_t RSVD38             :15;

        //(0x068)  //APB clock slee control register
        uint32_t AXI_S_PCLK         : 1;
        uint32_t DDR_S_PCLK         : 1;
        uint32_t DMAC_S_PCLK        : 1;
        uint32_t UART0_S_PCLK       : 1;
        uint32_t UART1_S_PCLK       : 1;
        uint32_t GPIO_S_PCLK        : 1;
        uint32_t I2C0_S_PCLK        : 1;
        uint32_t I2C1_S_PCLK        : 1;
        uint32_t I2C2_S_PCLK        : 1;
        uint32_t TMR_S_PCLK         : 1;
        uint32_t SSP0_S_PCLK        : 1;
        uint32_t SSP1_S_PCLK        : 1;
        uint32_t EFUSE_S_PCLK       : 1;
        uint32_t MIPI_RX0_S_PCLK    : 1;
        uint32_t MIPI_RX1_S_PCLK    : 1;
        uint32_t DPI2AHB_0_S_PCLK   : 1;
        uint32_t DPI2AHB_1_S_PCLK   : 1;
        uint32_t RSVD39             : 4;
        uint32_t WDT_S_PCLK         : 1;
        uint32_t X2H_S_PCLK         : 1;
        uint32_t NPU_S_PCLK         : 1;
        uint32_t DSP_S_PCLK         : 1;
        uint32_t RSVD40             : 1;
        uint32_t USB3_APB_S_CLK     : 1;
        uint32_t USB2_APB_S_CLK     : 1;
        uint32_t LC_S_PCLK          : 1;
        uint32_t TDC_S_PCLK         : 1;
        uint32_t PWM_S_PCLK         : 1;
        uint32_t H2X_0_S_PCLK       : 1;

        //(0x06C)  //APB clock slee control register
        uint32_t H2X_1_S_PCLK       : 1;
        uint32_t H2X_2_S_PCLK       : 1;
        uint32_t X2P_S_PCLK         : 1;
        uint32_t X2X_U0_S_PCLK      : 1;
        uint32_t X2X_U1_S_PCLK      : 1;
        uint32_t X2X_U1_1_S_PCLK    : 1;
        uint32_t X2X_U2_S_PCLK      : 1;
        uint32_t X2X_U3_S_PCLK      : 1;
        uint32_t X2X_U3_1_S_PCLK    : 1;
        uint32_t X2X_U4_S_PCLK      : 1;
        uint32_t X2X_U5_S_PCLK      : 1;
        uint32_t X2X_U7_1_S_PCLK    : 1;
        uint32_t X2X_U7_S_PCLK      : 1;
        uint32_t X2X_U6_S_PCLK      : 1;
        uint32_t X2X_U8_S_PCLK      : 1;
        uint32_t X2X_U9_S_PCLK      : 1;
        uint32_t EFUSE_128_S_PCLK   : 1;
        uint32_t RSVD41             :15;
    }bf;
}U_regSCU_clk_en;
#define REG_SCU_CLK_EN  ((U_regSCU_clk_en*) (SCU_REG_BASE + 0x0050))
#define SET_SCU_BUSCLOCK_EN(a, b) REG_SCU_CLK_EN->bf.a = b
#endif
/** @}*/

