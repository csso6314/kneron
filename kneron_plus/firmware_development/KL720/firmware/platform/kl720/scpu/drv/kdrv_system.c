/*
 * Kneron System driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "kdrv_system.h"
#include <cmsis_os2.h>
#include "kdrv_scu.h"
#include "kdrv_scu_ext.h"
#include "kdrv_power.h"
#include "kdrv_clock.h"
#include "kdrv_io.h"
#include "kdrv_pll.h"
#include "kdrv_ddr.h"
#include "kdrv_cmsis_core.h"

//#define SYS_DEBUG
#ifdef SYS_DEBUG
#define sys_msg(fmt, ...)   printf(LOG_CUSTOM, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define sys_msg(fmt, ...)
#endif
uint32_t bootup_status;
uint32_t warm_boot;
uint32_t __sys_int_flag;
uint32_t isrcnt[3];

void kdrv_system_globalreset(void)
{
    regSCUEXT->global_sw_reset_protect_reg = 0x5a5a5a5a;
    SET_SWRST(GLOBAL, 1);
}
void kdrv_system_reset(int32_t subsystem)
{
    switch (subsystem) {
        case SUBSYS_NPU:
            SET_SWRST(NPU, 1);
            break;
        case SUBSYS_PD_NPU:
            SET_SWRST(PD_NPU, 1);
            break;
        case SUBSYS_LCDC:
            SET_SWRST(FTLCDC210, 1);
            break;
        case SUBSYS_NCPU:
            SET_SWRST(VP6, 0);
            SET_SWRST(VP6, 1);
            break;
        case GLOBAL_RESET:
            kdrv_system_globalreset();
            break;
        default:
            break;
    }
}
void SYS_IRQ_Handler(void)
{
    uint32_t status;

    status = regSCU->dw.int_sts;
    regSCU->dw.int_sts = status;

    if (status & SCU_INT_RTC_ALARM)
    {
        isrcnt[0]++;
        sys_msg("isrcnt[0] = %d\n", isrcnt[0]);
    }
    if (status & SCU_INT_RTC_PERIODIC)
    {
        isrcnt[1]++;
        sys_msg("isrcnt[1] = %d\n", isrcnt[1]);
    }

    if (status & SCU_INT_RTC_SEC)
    {
        isrcnt[2]++;
        sys_msg("isrcnt[2] = %d\n", isrcnt[2]);
    }
    NVIC_ClearPendingIRQ(SYS_IRQn);
    __sys_int_flag = 0x1;
}

void kdrv_system_initialize(kdrv_power_mode_t power_mode, uint32_t wakeup_src, sysclockopt *sysclk_opt)
{
    bootup_status = regSCU->dw.btup_sts;
    regSCU->dw.btup_sts = 0xffffffff;
    NVIC_ClearPendingIRQ((IRQn_Type)SYS_IRQn);
    NVIC_EnableIRQ((IRQn_Type)SYS_IRQn);
    regSCU->dw.int_sts = 0xffffffff;        // clear all interrupt status
    __sys_int_flag = 0;
    kdrv_power_ops(POWER_OPS_CHANGE_BUS_SPEED);
    __WFI();
    do{
    }while((__sys_int_flag)!= 0x1);

    // ------------------------------------------------------------------------ //
    // turn on power domain
    // ------------------------------------------------------------------------ //
    kdrv_power_set_wakeup_src(wakeup_src);
    kdrv_power_set_powermode(power_mode);

    // ------------------------------------------------------------------------ //
    // turn on PLL
    // ------------------------------------------------------------------------ //
    kdrv_clock_inititialize(sysclk_opt);


    // ------------------------------------------------------------------------ //
    // Reset
    // ------------------------------------------------------------------------ //
    //PD reset deassert.
    REG_RST->dw.PDBUS_RST       |= 0x0000001F;
    //BUS reset deassert.
    REG_RST->dw.PDBUS_RST       |= 0x00000300;
    //IP reset deassert.
    REG_RST->dw.IP_RST          |= 0xFFFFFFFF;

}

