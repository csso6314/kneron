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
*  KL520
*
*  Description:
*  ------------
*  This is PWMTIMER driver
*
*  Author:
*  -------
*  Albert Chen
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

// Sec 0: Comment block of the file

// Sec 1: Include File
#include "base.h"
#include "io.h"
#include "Driver_Common.h"
#include "regbase.h"
#include "kdrv_pwm.h"
#include "kdrv_pinmux.h"
#include "kdrv_cmsis_core.h"
//#include "kmdw_console.h"

//#define PWM_DBG
#ifdef PWM_DBG
#define pwm_msg(fmt, ...) info_msg("[KDRV_PWM] " fmt, ##__VA_ARGS__)
#else
#define pwm_msg(fmt, ...)
#endif

#define MAX_PWM_TIMER               6
#define HZ                          100 /// how many tick each sec
#define MAX_TIMER                   7
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
    uint32_t TmSrc:     1;  /* bit 0 */
    uint32_t TmStart:   1;
    uint32_t TmUpdate:  1;
    uint32_t TmOutInv:  1;
    uint32_t TmAutoLoad:1;
    uint32_t TmIntEn:   1;
    uint32_t TmIntMode: 1;
    uint32_t TmDmaEn:   1;
    uint32_t TmPwmEn:   1;  /* bit 8 */
    uint32_t Reserved:  15; /* bit 9~23 */
    uint32_t TmDeadZone:8;  /* bit 24~31 */
}kdrv_pwmtimer_control;

typedef struct
{
    uint32_t IntNum;        /* interrupt number */
    uint32_t Tick;          /* Tick Per Second */
    uint32_t Running;       /* Is timer running */
}kdrv_pwmtimer_struct;



typedef void (*timer_isr)(void);

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable
kdrv_pwmtimer_control *timer_control[MAX_TIMER+1];
uint32_t *CNTBBase_tmp;
uint32_t *CMPBBase_tmp;

static kdrv_pwmtimer_struct ftimer[MAX_TIMER+1];
static uint32_t TimerBase[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x10, PWM_FTPWMTMR010_PA_BASE+0x20,PWM_FTPWMTMR010_PA_BASE+0x30,PWM_FTPWMTMR010_PA_BASE+0x40,PWM_FTPWMTMR010_PA_BASE+0x50,PWM_FTPWMTMR010_PA_BASE+0x60,PWM_FTPWMTMR010_PA_BASE+0x70};
static uint32_t CNTBBase[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x14, PWM_FTPWMTMR010_PA_BASE+0x24,PWM_FTPWMTMR010_PA_BASE+0x34,PWM_FTPWMTMR010_PA_BASE+0x44,PWM_FTPWMTMR010_PA_BASE+0x54,PWM_FTPWMTMR010_PA_BASE+0x64,PWM_FTPWMTMR010_PA_BASE+0x74};
static uint32_t CMPBBase[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x18, PWM_FTPWMTMR010_PA_BASE+0x28,PWM_FTPWMTMR010_PA_BASE+0x38,PWM_FTPWMTMR010_PA_BASE+0x48,PWM_FTPWMTMR010_PA_BASE+0x58,PWM_FTPWMTMR010_PA_BASE+0x68,PWM_FTPWMTMR010_PA_BASE+0x78};
uint32_t t1_tick = 0, t2_tick = 0, t3_tick = 0, t4_tick = 0, t5_tick = 0, t6_tick = 0;

// Sec 9: declaration of static function prototype
static void kdrv_pwmtimer_resetall(void);
static void kdrv_pwmtimer_autoreloadvalue(pwmtimer timer, uint32_t value);
static kdrv_status_t kdrv_pwmtimer_int_clear(pwmtimer timer);

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
static kdrv_status_t _set_timer_clk_source(pwmtimer timer,uint32_t clk)
{
    if ((timer == 0) || (timer > MAX_TIMER))
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;

    timer_control[timer]->TmSrc = clk;

    return KDRV_STATUS_OK;
}

static kdrv_status_t _set_timer_tick(uint32_t timer,uint32_t clk_tick)
{
    volatile kdrv_pwmtimer_struct *ctimer = &ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;

    ctimer->Tick = clk_tick;

    return KDRV_STATUS_OK;
}

static void _print_pwm_reg(pwmtimer timer)
{
#ifdef PWM_DBG
    uint32_t val = 0;

    val = inw(TimerBase[timer]);
    pwm_msg("T%d_CTRL = 0x%4x", timer, val);
    val = inw(CNTBBase[timer]);
    pwm_msg("T%d_CNTB = 0x%4x", timer, val);
    val = inw(CMPBBase[timer]);
    pwm_msg("T%d_CMPB = 0x%4x", timer, val);
    val = inw(CMPBBase[timer] + 0x4);
    pwm_msg("T%d_CNTO = 0x%4x", timer, val);
#endif
}


static void PWMTimer1_IRQHandler(void)
{
    kdrv_pwmtimer_int_clear(PWMTIMER1);
    ++t1_tick;
}

static void PWMTimer2_IRQHandler(void)
{
    kdrv_pwmtimer_int_clear(PWMTIMER2);
    ++t2_tick;
}

static void PWMTimer3_IRQHandler(void)
{
    kdrv_pwmtimer_int_clear(PWMTIMER3);
    ++t3_tick;
}

static void PWMTimer4_IRQHandler(void)
{
    kdrv_pwmtimer_int_clear(PWMTIMER4);
    ++t4_tick;
}

static void PWMTimer5_IRQHandler(void)
{
    kdrv_pwmtimer_int_clear(PWMTIMER5);
    ++t5_tick;
}

static void PWMTimer6_IRQHandler(void)
{
    kdrv_pwmtimer_int_clear(PWMTIMER6);
    ++t6_tick;
}

static kdrv_status_t kdrv_pwmtimer_disable(pwmtimer timer)
{
    kdrv_pwmtimer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;

    timer_control[timer]->TmStart   = 0;
    timer_control[timer]->TmUpdate  = 0;
    timer_control[timer]->TmOutInv  = 0;
    timer_control[timer]->TmDmaEn   = 0;
    timer_control[timer]->TmIntEn   = 0;
    timer_control[timer]->TmDeadZone= 0;
    timer_control[timer]->TmSrc     = 0;
    timer_control[timer]->TmAutoLoad= 0;

    ctimer->Running = false;

    return KDRV_STATUS_OK;
}

static void kdrv_pwmtimer_resetall(void)
{
    uint32_t i;

    for (i = 1; i <= MAX_TIMER; i++)
        kdrv_pwmtimer_disable((pwmtimer)i);
}

static void kdrv_pwmtimer_autoreloadvalue(pwmtimer timer, uint32_t value)
{
    outw(TimerBase[timer] + TIMER_LOAD, value);
}

static kdrv_status_t kdrv_pwmtimer_autoreloadenable(pwmtimer timer)
{
    if ((timer == 0) || (timer > MAX_TIMER))
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;

    timer_control[timer]->TmAutoLoad = 1;
    return KDRV_STATUS_OK;
}

static kdrv_status_t kdrv_pwmtimer_int_enable(pwmtimer timer)
{
    if ((timer == 0) || (timer > MAX_TIMER))
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;

    timer_control[timer]->TmIntEn = 1;
    return KDRV_STATUS_OK;
}


static kdrv_status_t kdrv_pwmtimer_ioctrl(timeriotype iotype, pwmtimer timer, uint32_t tick)
{
    switch(iotype)
    {
        case IO_TIMER_RESETALL:
            kdrv_pwmtimer_resetall();
            break;
        case IO_TIMER_SETTICK:
            return _set_timer_tick(timer,tick);
        case IO_TIMER_SETCLKSRC:
            return _set_timer_clk_source(timer,tick);
        default:
            return KDRV_STATUS_ERROR;
    }

    return KDRV_STATUS_OK;
}

static kdrv_status_t kdrv_pwmtimer_int_clear(pwmtimer timer)
{
    int value;

    if ((timer == 0) || (timer > MAX_TIMER))
       return KDRV_STATUS_TIMER_INVALID_TIMER_ID;

    value = 1<<(timer-1);
    outw(PWM_FTPWMTMR010_PA_BASE + TIMER_INTSTAT, value);

    return KDRV_STATUS_OK;
}

static kdrv_status_t kdrv_pwmtimer_enable(pwmtimer timer)
{
    kdrv_pwmtimer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;

    if(ctimer->Running == true)
        return KDRV_STATUS_TIMER_ID_IN_USED;
    timer_control[timer]->TmUpdate  = 1;
    timer_control[timer]->TmStart   = 1;

    ctimer->Running = true;

    return KDRV_STATUS_OK;
}

uint32_t kdrv_current_t1_tick(void)
{
    return t1_tick;
}

uint32_t kdrv_current_t2_tick(void)
{
    return t2_tick;
}

uint32_t kdrv_current_t3_tick(void)
{
    return t3_tick;
}

uint32_t kdrv_current_t4_tick(void)
{
    return t4_tick;
}

uint32_t kdrv_current_t5_tick(void)
{
    return t5_tick;
}

uint32_t kdrv_current_t6_tick(void)
{
    return t6_tick;
}

kdrv_status_t kdrv_pwmtimer_initialize(pwmtimer timer, uint32_t tick)
{
    kdrv_pwmtimer_struct *ctimer = &ftimer[timer];
    uint32_t timer_irq;
    timer_isr isr;

    if (timer == 0 || timer > MAX_TIMER)
        return KDRV_STATUS_TIMER_INVALID_TIMER_ID;

    switch(timer)
    {
        case PWMTIMER1:
            t1_tick = 0;
            timer_irq = PWM_FTPWMTMR010_1_IRQ;
            isr = PWMTimer1_IRQHandler;
        break;
        case PWMTIMER2:
            t2_tick = 0;
            timer_irq = PWM_FTPWMTMR010_2_IRQ;
            isr = PWMTimer2_IRQHandler;
        break;
        case PWMTIMER3:
            t3_tick = 0;
            timer_irq = PWM_FTPWMTMR010_3_IRQ;
            isr = PWMTimer3_IRQHandler;
        break;
        case PWMTIMER4:
            t4_tick = 0;
            timer_irq = PWM_FTPWMTMR010_4_IRQ;
            isr = PWMTimer4_IRQHandler;
        break;
        case PWMTIMER5:
            t5_tick = 0;
            timer_irq = PWM_FTPWMTMR010_5_IRQ;
            isr = PWMTimer5_IRQHandler;
        break;
        case PWMTIMER6:
            t6_tick = 0;
            timer_irq = PWM_FTPWMTMR010_6_IRQ;
            isr = PWMTimer6_IRQHandler;
        break;
        default:
            return KDRV_STATUS_TIMER_INVALID_TIMER_ID;
    }

    timer_control[timer]=(kdrv_pwmtimer_control *)(TimerBase[timer]);
    kdrv_pwmtimer_close(timer);

    if(kdrv_pwmtimer_ioctrl(IO_TIMER_SETTICK,timer,tick))
        return KDRV_STATUS_ERROR;

    kdrv_pwmtimer_autoreloadvalue(timer,ctimer->Tick);
    kdrv_pwmtimer_autoreloadenable(timer);

    kdrv_pwmtimer_int_clear(timer);
    if (kdrv_pwmtimer_int_enable(timer))
        return KDRV_STATUS_ERROR;

    NVIC_SetVector((IRQn_Type)timer_irq, (uint32_t)isr);
    NVIC_EnableIRQ((IRQn_Type)timer_irq);

    if(kdrv_pwmtimer_enable(timer))
        return KDRV_STATUS_ERROR;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_pwmtimer_close(pwmtimer timer)
{
    uint32_t timer_irq;

    if (timer == 0 || timer > MAX_TIMER)
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;

    if(kdrv_pwmtimer_disable(timer))
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;   /* Stop the timer first */

    switch(timer)
    {
        case PWMTIMER1:
            timer_irq = PWM_FTPWMTMR010_1_IRQ;
        break;
        case PWMTIMER2:
            timer_irq = PWM_FTPWMTMR010_2_IRQ;
        break;
        case PWMTIMER3:
            timer_irq = PWM_FTPWMTMR010_3_IRQ;
        break;
        case PWMTIMER4:
            timer_irq = PWM_FTPWMTMR010_4_IRQ;
        break;
        case PWMTIMER5:
            timer_irq = PWM_FTPWMTMR010_5_IRQ;
        break;
        case PWMTIMER6:
            timer_irq = PWM_FTPWMTMR010_6_IRQ;
        break;
        default:
            return KDRV_STATUS_ERROR;
    }

    NVIC_DisableIRQ((IRQn_Type)timer_irq);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_pwmtimer_tick_reset(pwmtimer timer)
{
    if ((timer == 0) || (timer > MAX_PWM_TIMER))
    {
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;
    }

    switch(timer)
    {
        case PWMTIMER1:
            t1_tick = 0;
        break;
        case PWMTIMER2:
            t2_tick = 0;
        break;
        case PWMTIMER3:
            t3_tick = 0;
        break;
        case PWMTIMER4:
            t4_tick = 0;
        break;
        case PWMTIMER5:
            t5_tick = 0;
        break;
        case PWMTIMER6:
            t6_tick = 0;
        break;
        default:
            return KDRV_STATUS_ERROR;
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_pwmtimer_delay_ms(uint32_t msec)
{
    uint32_t tick_end;
    kdrv_status_t ret;
    ret = kdrv_pwmtimer_initialize(PWMTIMER1, PWMTMR_1MSEC_PERIOD);
    if(ret != KDRV_STATUS_OK)
        return ret;
    tick_end = kdrv_current_t1_tick() + msec;
    do {
        __WFI();
    } while (tick_end > kdrv_current_t1_tick());

    ret = kdrv_pwmtimer_close(PWMTIMER1);

    return ret;
}

// Start of PWM function
kdrv_status_t kdrv_pwm_config(pwmtimer timer, pwmpolarity polarity, uint32_t duty, uint32_t period, bool ns2clkcnt)
{
    uint32_t ClkCnt;
    uint32_t ratio_period = 0;
    uint32_t ratio_duty = 0;
    CNTBBase_tmp = (uint32_t *)CNTBBase[timer];
    CMPBBase_tmp = (uint32_t *)CMPBBase[timer];

    if (timer == 0 || timer > MAX_TIMER)
        return KDRV_STATUS_TIMER_INVALID_TIMER_ID;
    if(duty == 0 || period == 0)
        return KDRV_STATUS_ERROR;

    _print_pwm_reg(timer);

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
    timer_control[timer]=(kdrv_pwmtimer_control *)(TimerBase[timer]);
    timer_control[timer]->TmUpdate = 1;
    timer_control[timer]->TmOutInv = polarity;
    timer_control[timer]->TmAutoLoad = 1;

    timer_control[timer]->TmStart = 1;
    _print_pwm_reg(timer);

    return KDRV_STATUS_OK;
}
kdrv_status_t kdrv_pwm_enable(pwmtimer timer)
{
    timer_control[timer]->TmPwmEn = 1;
    _print_pwm_reg(timer);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_pwm_disable(pwmtimer timer)
{

    if(1 != timer_control[timer]->TmPwmEn)
    {
        pwm_msg("pwm%d is not enable!", timer);
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    }

    timer_control[timer]->TmPwmEn = 0;

    _print_pwm_reg(timer);

    return KDRV_STATUS_OK;
}
// End of PWM function

