/*
 * Kneron System driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */


#include "kdrv_system.h"
#ifndef NON_OS
#include "cmsis_os2.h"
#endif
#include "kdrv_scu.h"
#include "kdrv_scu_ext.h"
#include "kdrv_power.h"
#include "kdrv_clock.h"
#include "kdrv_ddr.h"

uint32_t bootup_status;
uint32_t warm_boot;
uint32_t __sys_int_flag;

#define BOOTUP_STATUS_WARM      (SCU_REG_BTUP_STS_SMR | SCU_REG_BTUP_STS_PMR2)

void kdrv_system_reset(int32_t subsystem)
{
    switch (subsystem) {
        case SUBSYS_NPU:
            SCU_EXTREG_SWRST_SET_NPU_resetn(1);
            break;
        case SUBSYS_PD_NPU:
            SCU_EXTREG_SWRST_SET_PD_NPU_resetn(1);
            break;
        case SUBSYS_LCDC:
            SCU_EXTREG_SWRST_SET_LCDC_resetn(1);
            break;
        case SUBSYS_NCPU:
            SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(0);
            SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1);    
    }
}

void system_isr(void)
{
    outw(SCU_REG_INT_STS, 0xffffffff); //clear sleep wakeup interrupt   

    __sys_int_flag = 0x1;
}

static void reset_handler(void)
{
    bootup_status = inw(SCU_REG_BTUP_STS);
    outw(SCU_REG_BTUP_STS, 0xffffffff);  // clear boot-up status
    outw(SCU_REG_BTUP_CTRL, SCU_REG_BTUP_CTRL_RTC_BU_EN | // RTC wakeup allowed
                            SCU_REG_BTUP_CTRL_PWRBTN_EN | // send power button output signal
                            SCU_REG_BTUP_CTRL_GPO_1_OUT |
                            SCU_REG_BTUP_CTRL_GPO_OUT);

    NVIC_ClearPendingIRQ((IRQn_Type)SYS_SYSTEM_IRQ);
    NVIC_EnableIRQ((IRQn_Type)SYS_SYSTEM_IRQ);

    outw(SCU_REG_INT_STS, 0xffffffff); // clear all interrupt status
    outw(SCU_REG_INT_EN, 0xffffffff); // enable all interrupts during boot

    //can't directly write to the PLL control pins, it needs to use the 
    //FCS or PLL_UPDATE command that contained he power-ode register
    kdrv_clock_mgr_set_scuclkin(scuclkin_pll0div3, true);
    kdrv_delay_us(2000);

    kdrv_power_ops(POWER_OPS_FCS);
    __WFI();
    do{
    }while((__sys_int_flag)!= 0x1);

//    kdrv_clock_mgr_set_muxsel(CLOCK_MUXSEL_NCPU_TRACECLK_DEFAULT |
//                         CLOCK_MUXSEL_SCPU_TRACECLK_SRC_PLL0DIV3 |
//                         CLOCK_MUXSEL_CSIRX1_CLK_PLL5 |
//                         CLOCK_MUXSEL_NPU_CLK_PLL0 | /* CLOCK_MUXSEL_NPU_CLK_PLL4 | */ 
//                         CLOCK_MUXSEL_PLL4_FREF_PLL0DIV | 
//                         CLOCK_MUXSEL_UART_0_IRDA_UCLK_UART);
    kdrv_delay_us(1000);
}

void kdrv_system_init(void)
{
    NVIC_SetVector((IRQn_Type)SYS_SYSTEM_IRQ, (uint32_t)system_isr);

    reset_handler();

    if ((bootup_status & BOOTUP_STATUS_WARM) == BOOTUP_STATUS_WARM)
        warm_boot = 1;
    else
        warm_boot = 0;

    kdrv_clock_mgr_init();

    /* Default power domain is already on */
    {
        //kdrv_clock_mgr_open_pll4(); // npu
        kdrv_delay_us(10 * 10);
        kdrv_clock_enable(CLK_PLL4_FREF_PLL0);
        kdrv_delay_us(30 * 10);
        kdrv_clock_enable(CLK_PLL4);
        kdrv_delay_us(30 * 10);
        kdrv_clock_enable(CLK_PLL4_OUT);
    }
                
    /* Turn on NPU power domain */
    kdrv_power_set_domain(POWER_DOMAIN_NPU, 1);
    if (warm_boot) {
        /* TODO: reload ncpu fw to NiRAM */
    }

    {
        /* Turn on DDR power domain for cold boot */
        kdrv_power_set_domain(POWER_DOMAIN_DDRCK, 1);

        kdrv_clock_enable(CLK_PLL1);
        kdrv_delay_us(10 * 10);
        kdrv_clock_enable(CLK_PLL1_OUT);
        kdrv_delay_us(30 * 10);

        kdrv_clock_enable(CLK_PLL2);
        kdrv_delay_us(10 * 10);
        kdrv_clock_enable(CLK_PLL2_OUT);
    }

    {   // PLL5
        kdrv_clock_mgr_change_pll5_clock(1, 0x63, 2);

        // Set I2C PCLKs disabled by default
        uint32_t mask = SCU_REG_APBCLKG_PCLK_EN_I2C0_PCLK |
                        SCU_REG_APBCLKG_PCLK_EN_I2C1_PCLK |
                        SCU_REG_APBCLKG_PCLK_EN_I2C2_PCLK |
                        SCU_REG_APBCLKG_PCLK_EN_I2C3_PCLK;
        masked_outw(SCU_REG_APBCLKG, 0, mask);
    }
    
    //fcs
    {
        kdrv_delay_us(10 * 10);
        kdrv_clock_enable(CLK_PLL4_FREF_PLL0);
        kdrv_delay_us(30 * 10); 
        kdrv_clock_enable(CLK_PLL4);
        kdrv_delay_us(30 * 10);
        kdrv_clock_enable(CLK_PLL4_OUT);

        NVIC_ClearPendingIRQ((IRQn_Type)SYS_SYSTEM_IRQ);
        NVIC_EnableIRQ((IRQn_Type)SYS_SYSTEM_IRQ);
        
        outw(SCU_REG_INT_STS, 0xffffffff); // clear all interrupt status

        kdrv_clock_enable(CLK_FCS_PLL2);
        kdrv_clock_enable(CLK_FCS_DLL);
        kdrv_clock_mgr_set_scuclkin(scuclkin_pll0div3, false);
        kdrv_power_ops(POWER_OPS_FCS);

        __WFI();
        do {
        } while((__sys_int_flag)!= 0x1);
    }
}

void kdrv_system_init_ncpu(void)
{
    kdrv_clock_enable(CLK_SCPU_TRACE);
    kdrv_clock_enable(CLK_NCPU);
    kdrv_clock_enable(CLK_NPU);
    kdrv_system_reset(SUBSYS_PD_NPU);
    kdrv_system_reset(SUBSYS_NPU);
}
