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
*  kdrv_pwm.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  This is PWM driver
*
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

// Sec 0: Comment block of the file

// Sec 1: Include File

#include "base.h"
#include "Driver_Common.h"
#include "regbase.h"
#include "kdrv_pwm.h"
#include "kdrv_scu_ext.h"
//#define PWM_DBG
#ifdef PWM_DBG
#define pwm_msg(fmt, ...)   printf("[KDRV_PWM] " fmt, ##__VA_ARGS__)
#else
#define pwm_msg(fmt, ...)
#endif

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define MAX_PWM_TIMER               6
#define HZ                          100 /// how many tick each sec
#define MAX_TIMER                   2
#define TIMER_INTSTAT               0x0
#define TIMER_CR                    0x0
#define TIMER_LOAD                  0x4
#define TIMER_COMPARE               0x8
#define TIMER_CNTO                  0xc

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
typedef struct
{
    uint32_t clk_src        : 1;
    uint32_t start          : 1;
    uint32_t update         : 1;
    uint32_t out_inv        : 1;
    uint32_t autoload       : 1;
    uint32_t int_en         : 1;
    uint32_t int_mo         : 1;
    uint32_t tdma_en        : 1;
    uint32_t pwm_en         : 1;
    uint32_t rsvd           :15;
    uint32_t dz             : 8;
}kdrv_pwm_ctrl;

typedef volatile union pwm{
    struct{
        uint32_t int_cstat;                 //(0x00)        Interrupt status and control register of FTPWMTMR010
        uint32_t rsvd[3];                   //(0x04~0x0C)   reserved
        uint32_t t1_ctrl;                   //(0x10)        Timer 1 control register
        uint32_t t1_cntb;                   //(0x14)        Timer 1 count buffer register
        uint32_t t1_cmpb;                   //(0x18)        Timer 1 compare buffer register
        uint32_t t1_cnto;                   //(0x1C)        Timer 1 observation register
        uint32_t t2_ctrl;                   //(0x20)        Timer 2 control register
        uint32_t t2_cntb;                   //(0x24)        Timer 2 count buffer register
        uint32_t t2_cmpb;                   //(0x28)        Timer 2 compare buffer register
        uint32_t t2_cnto;                   //(0x2C)        Timer 2 observation register
    }dw;
    struct{
        //(0x00)        Interrupt status and control register of FTPWMTMR010
        uint32_t tm1_int_stat       : 1;    //Timer 1 interrupt status Cleared by writing ．1・ to this bit 0: No effect    1: Counter value of Timer 1 reaches zero.
        uint32_t tm2_int_stat       : 1;    //Timer 2 interrupt status Cleared by writing ．1・ to this bit 0: No effect    1: Counter value of Timer 1 reaches zero.
        uint32_t rsvd1              :30;    //rsvd

        //(0x04~0x0C)   reserved
        uint32_t rsvd2[3];

        //(0x10)        Timer 1 control register
        uint32_t tmx_1_clk_src      : 1;    //Timer1 clock source 0: PCLK 1: ext_clk
        uint32_t tmx_1_start        : 1;    //Timer1 start/stop   0: Stop 1: Start
        uint32_t tmx_1_update       : 1;    //Timer1 updates TMX_CNTB and TMX_CMPB. Write ．1・ to this register will trigger a manual update. This bit will be auto-cleared after the operation.0: No operation 1: Manual update
        uint32_t tmx_1_out_inv      : 1;    //Timer1 output inverter on/off 0: Inverter off 1: Inverter on
        uint32_t tmx_1_auto         : 1;    //Timer1 auto-reload on/off     0: One-shot 1: Interval mode (Auto-reload)
        uint32_t tmx_1_int_en       : 1;    //Timer1 interrupt mode enable/disable      0: Disable 1: Enable
        uint32_t tmx_1_int_mo       : 1;    //Timer1 interrupt mode 0: Level (Clear an interrupt by writing ．1・ to TMX_INT_STAT) 1: Pulse
        uint32_t tmx_1_dma_en       : 1;    //Timer1 DMA request mode enable/disable 0: Disable 1: Enable
        uint32_t tmx_1_pwm_en       : 1;    //Timer1 PWM function enable/disable 0: Disable 1: Enable
        uint32_t rsvd3              :15;    //rsvd
        uint32_t tmx_1_dz           : 8;    //Timer1 dead-zone length = TMX_CTRL[31:24] - 1

        //(0x14)        Timer 1 count buffer register
        uint32_t tmx_1_cntb         :32;

        //(0x18)        Timer 1 compare buffer register
        uint32_t tmx_1_cmpb         :32;

        //(0x1C)        Timer 1 observation register
        uint32_t tmx_1_cnto         :32;

        //(0x20)        Timer 2 control register
        uint32_t tmx_2_clk_src      : 1;    //Timer2 clock source 0: PCLK 1: ext_clk
        uint32_t tmx_2_start        : 1;    //Timer2 start/stop   0: Stop 1: Start
        uint32_t tmx_2_update       : 1;    //Timer2 updates TMX_CNTB and TMX_CMPB. Write ．1・ to this register will trigger a manual update. This bit will be auto-cleared after the operation.0: No operation 1: Manual update
        uint32_t tmx_2_out_inv      : 1;    //Timer2 output inverter on/off 0: Inverter off 1: Inverter on
        uint32_t tmx_2_auto         : 1;    //Timer2 auto-reload on/off     0: One-shot 1: Interval mode (Auto-reload)
        uint32_t tmx_2_int_en       : 1;    //Timer2 interrupt mode enable/disable      0: Disable 1: Enable
        uint32_t tmx_2_int_mo       : 1;    //Timer2 interrupt mode 0: Level (Clear an interrupt by writing ．1・ to TMX_INT_STAT) 1: Pulse
        uint32_t tmx_2_dma_en       : 1;    //Timer2 DMA request mode enable/disable 0: Disable 1: Enable
        uint32_t tmx_2_pwm_en       : 1;    //Timer2 PWM function enable/disable 0: Disable 1: Enable
        uint32_t rsvd4              :15;    //rsvd
        uint32_t tmx_2_dz           : 8;    //Timer2 dead-zone length = TMX_CTRL[31:24] - 1

        //(0x24)        Timer 2 count buffer register
        uint32_t tmx_2_cntb         :32;

        //(0x28)        Timer 2 compare buffer register
        uint32_t tmx_2_cmpb         :32;

        //(0x2C)        Timer 2 observation register
        uint32_t tmx_2_cnto         :32;
    }bf;
}U_regPWM;
#define REGPWM  ((U_regPWM*) PWM_REG_BASE)

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable
kdrv_pwm_ctrl *pwm_ctrl[MAX_TIMER+1];
uint32_t *CNTBBase_tmp;
uint32_t *CMPBBase_tmp;

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/

kdrv_status_t kdrv_pwm_config(pwm_id pwmid, pwmpolarity polarity, uint32_t duty, uint32_t period, bool ns2clkcnt)
{
    uint32_t ClkCnt;
    uint32_t ratio_period = 0;
    uint32_t ratio_duty = 0;
    CNTBBase_tmp = (uint32_t *)(&(REGPWM->dw.t1_cntb) + ((pwmid-1) * 4));
    CMPBBase_tmp = (uint32_t *)(&(REGPWM->dw.t1_cmpb) + ((pwmid-1) * 4));

    if (pwmid == 0 || pwmid > MAX_TIMER)
        return KDRV_STATUS_TIMER_INVALID_TIMER_ID;
    if(duty == 0 || period == 0)
        return KDRV_STATUS_ERROR;

    if(ns2clkcnt)
    {
        ratio_period = 1000000000/period;
        ClkCnt = APB_CLK / ratio_period;
        *CNTBBase_tmp = ClkCnt ;

        ratio_duty = 1000000000/duty;
        ClkCnt = APB_CLK / ratio_duty;
        *CMPBBase_tmp = ClkCnt ;
    }
    else
    {
        *CNTBBase_tmp = period;
        *CMPBBase_tmp = duty;
    }
    pwm_ctrl[pwmid]             = (kdrv_pwm_ctrl *)(&(REGPWM->dw.t1_ctrl) + ((pwmid-1) * 4));
    pwm_ctrl[pwmid]->out_inv    = polarity;
    pwm_ctrl[pwmid]->update     = 1;
    pwm_ctrl[pwmid]->autoload   = 1;
    pwm_ctrl[pwmid]->start      = 1;

    return KDRV_STATUS_OK;
}
kdrv_status_t kdrv_pwm_enable(pwm_id pwmid)
{
    pwm_ctrl[pwmid]->pwm_en = 1;

    if(pwmid == PWM_ID_1)
    {
        regSCUEXT->spare_default_0r0 |= 0x100;
    }
    else
    {
        regSCUEXT->spare_default_0r0 |= 0x200;
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_pwm_disable(pwm_id pwmid)
{

    if(1 != pwm_ctrl[pwmid]->pwm_en)
    {
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    }

    pwm_ctrl[pwmid]->pwm_en = 0;
    if(pwmid == PWM_ID_1)
    {
        regSCUEXT->spare_default_0r0 &= ~(0x100);
    }
    else
    {
        regSCUEXT->spare_default_0r0 &= ~(0x200);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_pwm_initialize(void)
{
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_pwm_uninitialize(void)
{
    return KDRV_STATUS_OK;
}
// End of PWM function

