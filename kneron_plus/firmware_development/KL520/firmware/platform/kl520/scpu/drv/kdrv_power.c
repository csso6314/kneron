/*
 * Kneron Power Base driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "kdrv_power.h"
#include "kdrv_scu.h"
#include "kdrv_scu_ext.h"
#include "kdrv_system.h"
#include "kdrv_clock.h" // for kdrv_delay_us()
#include "kdrv_ddr.h"
//#include "kmdw_console.h"

/************************************************************************
*                 	 Power Register Releated Macros
************************************************************************/
#define POWER_DOMAIN_WORKING_NONE       0x00000000
#define POWER_DOMAIN_WORKING_DEFAULT    0x00000001
#define POWER_DOMAIN_WORKING_NPU        0x00000002
#define POWER_DOMAIN_WORKING_DDR        0x00000004
#define POWER_DOMAIN_WORKING_ALL        (POWER_DOMAIN_WORKING_DEFAULT | POWER_DOMAIN_WORKING_NPU | POWER_DOMAIN_WORKING_DDR)

#define POWER_DOMAIN_SOFTOFF_DEFAULT    0x00000010
#define POWER_DOMAIN_SOFTOFF_NPU        0x00000020
#define POWER_DOMAIN_SOFTOFF_DDR        0x00000040

#define POWER_DOMAIN_SOFTOFF_MASK       (POWER_DOMAIN_SOFTOFF_DEFAULT | \
                                         POWER_DOMAIN_SOFTOFF_NPU | \
                                         POWER_DOMAIN_SOFTOFF_DDR)

#define PWR_CTRL_SOFTOFF_MASK   (SCU_REG_PWR_CTRL_PWRUP_UPDATE | \
                                SCU_REG_PWR_CTRL_PWRUP_CTRL_MASK | \
                                SCU_REG_PWR_CTRL_PWRDN_CTRL_MASK)

#define PWR_CTRL_SOFTOFF_DEEP_RETENTION     (SCU_REG_PWR_CTRL_PWRUP_UPDATE | \
                                            SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DDRCK | \
                                            SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT | \
                                            SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DDRCK)

#define PWR_CTRL_SOFTOFF_RTC_MODE           (SCU_REG_PWR_CTRL_PWRUP_UPDATE | \
                                            SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT)

kdrv_power_mode_t __power_mgr_mode = POWER_MODE_RTC;

/************************************************************************
*                 	           Private API
************************************************************************/
static void _power_mgr_ops_fcs()
{
    masked_outw(SCU_REG_PWR_MOD, 
                SCU_REG_PWR_MOD_SELFR_CMD_OFF | //scu will NOT issue the self-refresh command to DDR/SDR (in FCS)
                SCU_REG_PWR_MOD_FCS_PLL_RSTn |  //Keep PLL active in FCS
                SCU_REG_PWR_MOD_FCS,            //enter FCS
    
                SCU_REG_PWR_MOD_SELFR_CMD_OFF | SCU_REG_PWR_MOD_FCS_PLL2_RSTn |
                SCU_REG_PWR_MOD_FCS_DLL_RSTn | SCU_REG_PWR_MOD_FCS_PLL_RSTn |
                SCU_REG_PWR_MOD_FCS | SCU_REG_PWR_MOD_BUS_SPEED);
}

/************************************************************************
*                 	           Public API
************************************************************************/
void kdrv_power_sw_reset(void)
{
    //err_msg("Set watchdog reset\n");

    outw(WDT_FTWDT010_PA_BASE+0x0C, 0);
    outw(WDT_FTWDT010_PA_BASE+0x04, 1000);
    outw(WDT_FTWDT010_PA_BASE+0x0C, 0x03); // system reset
    outw(WDT_FTWDT010_PA_BASE+0x08, 0x5AB9);

    __WFI();
}

kdrv_status_t kdrv_power_set_domain(kdrv_power_domain_t domain, int enable)
{
    uint32_t val, mask, vcc_ready, misc_pwr, wait_state;

    switch (domain) {
        case POWER_DOMAIN_DDRCK:
            mask = SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DDRCK;
            vcc_ready = SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DDRCK;
            misc_pwr = SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DDRCK;
            break;
        case POWER_DOMAIN_NPU:
            mask = SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU;
            vcc_ready = SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_NPU;
            misc_pwr = SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_NPU;
            break;
        case POWER_DOMAIN_DEFAULT:
            mask = SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT;
            vcc_ready = SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DEFAULT;
            misc_pwr = SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DEFAULT;
            break;
        default:
            return KDRV_STATUS_INVALID_PARAM;
    }

    val = enable? mask : 0;
    val |= SCU_REG_PWR_CTRL_PWRUP_UPDATE;
    mask |= SCU_REG_PWR_CTRL_PWRUP_UPDATE;

    masked_outw(SCU_REG_PWR_CTRL, val, mask);
    kdrv_delay_us(500);

    /* Wait for VCC change */
    wait_state = enable? vcc_ready : 0;
    do {
        val = inw(SCU_REG_PWR_VCCSTS);
    } while((val & vcc_ready) != wait_state);

    /* Wait for power reset change */
    wait_state = enable? misc_pwr : 0;
    do {
        val = inw(SCU_EXTREG_MISC);
    } while((val & misc_pwr) != wait_state);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_power_softoff(kdrv_power_mode_t mode)
{
    uint32_t val;

    switch (mode) {
        case POWER_MODE_DEEP_RETENTION:
            val = PWR_CTRL_SOFTOFF_DEEP_RETENTION;
            break;
        case POWER_MODE_RTC:
            val = PWR_CTRL_SOFTOFF_RTC_MODE;
            break;
        default:
            return KDRV_STATUS_INVALID_PARAM;
    }
    masked_outw(SCU_REG_PWR_CTRL, val, PWR_CTRL_SOFTOFF_MASK);
    SCU_REG_PWR_MOD_SET_SOFTOFF(1);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_power_ops(kdrv_power_ops_t ops)
{   
    switch (ops) {    
    case POWER_OPS_FCS:
        _power_mgr_ops_fcs();
        break;
    case POWER_OPS_CHANGE_BUS_SPEED:
        break;
    case POWER_OPS_PLL_UPDATE:
        break;    
    case POWER_OPS_SLEEPING:
        //stop cpu 
        //stop ddr
        //send self-refresh command to ddr
        //wait ack
        break;
    default:
        return KDRV_STATUS_INVALID_PARAM;
    }
    return KDRV_STATUS_OK;
}
