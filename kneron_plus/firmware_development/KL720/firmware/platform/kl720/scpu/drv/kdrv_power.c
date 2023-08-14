/*
 * Kneron Power Base driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "kdrv_power.h"
#include "kdrv_scu.h"
#include "kdrv_scu_ext.h"
#include "kdrv_clock.h" // for kdrv_delay_us()
#include "kdrv_io.h"
#include "regbase.h"
#include "kdrv_cmsis_core.h"

/************************************************************************
*                     Power Register Releated Macros
************************************************************************/
#define POWER_DOMAIN_STANDBY    (SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM)
#define POWER_DOMAIN_FUNCTIONAL (SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_MRX | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UHO)

uint32_t current_power_mode = POWER_MODE_USB_DEVICE;
uint32_t power_status[POWER_MODE_MAX]=
{
    /*FULLOFF       */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NONE,
    /*AUX_PWR_ON    */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NONE,
    /*BASE          */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS,
    /*NOR           */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM,
    /*IMG_DETECT    */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_MRX,

    /*UVC           */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UHO,
    /*AI_TEST       */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU,
    /*USB_DEVICE    */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR,
    /*AI_RUNING     */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_MRX | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU,
    /*USB_AI        */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR,

    /*UVC_AI        */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UHO,
    /*UVC_AI_PASS   */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UHO,
    /*ALL_AXI_ON    */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_MRX | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UHO,
    /*DORM_USB_S    */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DCK | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR,
    /*SNOZ_USB_S    */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DCK | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_UDR,

    /*DORM          */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DCK,
    /*SNOZ          */SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DCK | SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS,
};
static uint32_t wakeup_src = 0;
/************************************************************************
*                               Private API
************************************************************************/
static void _power_mgr_ops_fcs()
{
    regSCU->bf.fcs = 1;
}
static void _power_mgr_ops_busspped()
{
    //regSCU->bf.bus_speed = 1;
    regSCU->dw.pwr_mod = 0x90300020;
}

/************************************************************************
*                               Public API
************************************************************************/
void kdrv_power_sw_reset(void)
{
    //err_msg("Set watchdog reset\n");

#if !defined(_BOARD_SN720HAPS_H_)
    outw(WDT_REG_BASE+0x0C, 0);
    outw(WDT_REG_BASE+0x04, 1000);
    outw(WDT_REG_BASE+0x0C, 0x03); // system reset
    outw(WDT_REG_BASE+0x08, 0x5AB9);

    __WFI();
#endif
}

kdrv_status_t kdrv_power_set_wakeup_src(uint32_t wakeup_src_)
{
    wakeup_src = wakeup_src_;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_power_set_domain(uint32_t domain)
{
    uint32_t val,vcc_ready, por_flag;
    regSCU->bf.pwrup_ctrl = domain>>8;
    regSCU->bf.pwrctrl_update = 1;
    vcc_ready = domain>>8;
    por_flag = domain<<8;

    /* Wait for VCC ready flag */
    do {
        val = regSCU->dw.pwr_vccsts;//(0x048)Power domain voltage supplied status register
    } while((val & vcc_ready) != vcc_ready);

    /* Wait for power on reset */
    do {
        val = regSCUEXT->scu_power_ext;//(0x01F0)
    } while((val & por_flag) != por_flag);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_power_set_powermode(kdrv_power_mode_t next_pm)
{
    uint32_t n_powerdomains;

    if(next_pm == POWER_MODE_DORM_USB_S && next_pm == POWER_MODE_DORM)
        return KDRV_STATUS_ERROR;
    if(next_pm == current_power_mode)
        return KDRV_STATUS_ERROR;
    if(current_power_mode < POWER_MODE_IMG_DETECT && next_pm >= POWER_MODE_IMG_DETECT)       //Low to Highs
    {
        //STEP1 check if need to ture on BAS
        if((power_status[current_power_mode] & SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS) == 0)
            kdrv_power_set_domain(SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS);
        //STEP2 check if need to ture on NOM
        if((power_status[current_power_mode] & SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM) == 0)
            kdrv_power_set_domain(POWER_DOMAIN_STANDBY);
        //STEP3 turn whole active domains as request ;
        n_powerdomains = power_status[next_pm];
        kdrv_power_set_domain(n_powerdomains);
    }
    else if(current_power_mode >= POWER_MODE_IMG_DETECT && next_pm < POWER_MODE_IMG_DETECT)   //High to Low, default BAS and NOR should be enable.
    {
        //STEP1 turn off whole active power domain (MRX/UHO/UDR/NPU)
        n_powerdomains = POWER_DOMAIN_STANDBY;  //turn off active domain. MRX/NPU/UDR/UHO
        kdrv_power_set_domain(n_powerdomains);
        //STEP2 check if need turn off NOR
        if((power_status[next_pm] & SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM) == 0)
            kdrv_power_set_domain(SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NOM);
        //STEP3 Compare if need turn off BAS
        if((power_status[next_pm] & SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS) == 0)
            kdrv_power_set_domain(SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS);
    }
    else
    {
        n_powerdomains = power_status[next_pm];
        kdrv_power_set_domain(n_powerdomains);
    }
    current_power_mode = next_pm;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_power_softoff(kdrv_power_mode_t next_pm)
{
    if(next_pm != POWER_MODE_DORM_USB_S && next_pm != POWER_MODE_DORM)
        return KDRV_STATUS_ERROR;

    regSCU->dw.btup_ctrl = (regSCU->dw.btup_ctrl & ~(SCU_REG_BTUP_CTRL_RTC_BU_EN | SCU_REG_BTUP_CTRL_EXT_WAK_BTN_BU_EN | SCU_REG_BTUP_CTRL_EXT_U3_BU_EN | SCU_REG_BTUP_CTRL_EXT_U2_BU_EN)) | wakeup_src;
    regSCU->bf.pwrdn_ctrl = (power_status[next_pm] & 0xFF00)>>8;
    regSCU->bf.pwrup_ctrl = SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_BAS>>8;  //recommand to back to BAS domain if CM4 wake up via Buttom/RTC/USB.

    regSCU->dw.int_en = SCU_INT_WAKEUP;
    regSCU->bf.perint_sel = 0;

    current_power_mode = POWER_MODE_BASE;
    regSCU->bf.pwrbtn_en = 0;
    regSCU->bf.pwrbtn = 0;

    regSCU->bf.softoff = 1;                                             //SCU_REG_PWR_MOD_SET_SOFTOFF(1);

    __WFI();

    return KDRV_STATUS_OK;
}
kdrv_status_t kdrv_power_sleep(void)
{
    uint32_t int_en_temp,peri_tmp;
    regSCU->dw.btup_ctrl = (regSCU->dw.btup_ctrl & ~(SCU_REG_BTUP_CTRL_RTC_BU_EN | SCU_REG_BTUP_CTRL_EXT_WAK_BTN_BU_EN | SCU_REG_BTUP_CTRL_EXT_U3_BU_EN | SCU_REG_BTUP_CTRL_EXT_U2_BU_EN)) | wakeup_src;
    regSCU->dw.slp_wakup_st = 0xFFFFFFFF;
    regSCU->dw.int_sts = 0xFFFFFFFF;

    regSCU->bf.slp_wakup_en0 = 1;
    regSCU->bf.slp_wakup_enx = 0;
    regSCU->bf.pwrbtn_en = 0;
    regSCU->bf.pwrbtn = 0;

    int_en_temp = regSCU->dw.int_en;
    regSCU->dw.int_en = SCU_INT_WAKEUP;
    peri_tmp = regSCU->bf.perint_sel;
    regSCU->bf.perint_sel = 0;

    regSCU->dw.slp_ahbclkg = 0;
    regSCU->dw.slp_apbclkg[0] = 0;
    regSCU->dw.slp_apbclkg[1] = 0;
    regSCU->dw.slp_axiclkg = 0;
    if((regSCU->dw.btup_ctrl & (SCU_REG_BTUP_CTRL_EXT_U3_BU_EN|SCU_REG_BTUP_CTRL_EXT_U2_BU_EN)) != 0)
    {
        regSCU->bf.axi_s_pclk = 1;
        regSCU->bf.ddr_s_pclk = 1;
        regSCU->bf.usb3_apb_s_clk = 1;
        regSCU->bf.usb2_apb_s_clk = 1;
        regSCU->bf.slp_aclk_en_0 = 1;
        regSCU->bf.slp_aclk_en_2 = 1;
        regSCU->bf.slp_aclk_en_3 = 1;
        regSCU->bf.slp_aclk_en_4 = 1;
    }

    regSCU->bf.sleep_ = 1;

    __WFI();

    regSCU->dw.int_en = int_en_temp;
    regSCU->bf.perint_sel = peri_tmp;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_power_ops(kdrv_power_ops_t ops)
{
    switch (ops) {
    case POWER_OPS_FCS:
        _power_mgr_ops_fcs();
        break;
    case POWER_OPS_CHANGE_BUS_SPEED:
        _power_mgr_ops_busspped();
        break;
    default:
        return KDRV_STATUS_INVALID_PARAM;
    }
    return KDRV_STATUS_OK;
}

