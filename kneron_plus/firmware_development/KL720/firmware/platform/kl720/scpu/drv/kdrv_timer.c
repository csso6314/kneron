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
*  kdrv_timer.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  This is TIMER driver
*
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

// Sec 0: Comment block of the file

// Sec 1: Include File
#include "kdrv_timer.h"
#include "kneron_kl720.h"
#include "io.h"
#include "kdrv_scu.h"
#include "kdrv_scu_ext.h"
#include "kdrv_clock.h"
#include <string.h>
#include "kdrv_cmsis_core.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
//#define TIMER_DGB
#define PERF_ISR_CNT    99

#define FLAGS_TIMER_M1_DONE_EVENT BIT0
#define FLAGS_TIMER_M2_DONE_EVENT BIT1
#define FLAGS_TIMER_OF_DONE_EVENT BIT2
#define FLAGS_TIMER_ALL_EVENTS    ( FLAGS_TIMER_M1_DONE_EVENT | FLAGS_TIMER_M2_DONE_EVENT | FLAGS_TIMER_OF_DONE_EVENT)

#define TM_PER_IP       3
#define TM_REG_OFS      3
#define TM_ENABLE       0x01
#define TM_CLK          0x02
#define TM_OFENABLE     0x04

#define TM_INT_MASK_M1  0x01
#define TM_INT_MASK_M2  0x02
#define TM_INT_MASK_OF  0x04

#define TM_TMCR         0x30
#define TM_TMINTSTAT    0x34
#define TM_TMINTMASK    0x38

#define TM_ID_1         0
#define TM_ID_2         1
#define TM_ID_3         2

#define TM_DOWN         0
#define TM_UP           1

#define TIMER1_MASK     0x007
#define TIMER2_MASK     0x038
#define TIMER3_MASK     0x1C0

#define INTERVAL_MS(x)      (x*50000)
#define INTERVAL_US(x)      (x*50)
#define INTERVAL_MS_LONG(x) (x*6000)

#define TMR_EXT1_CLK_EN     BIT13
#define TMR_EXT2_CLK_EN     BIT14
#define TMR_EXT3_CLK_EN     BIT15
/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
typedef enum {
    TIMER_ID_0,
    TIMER_ID_1,
    TIMER_ID_2,
    TIMER_ID_MAX,
    TIMER_ID_INIT=0xFF
}TIMER_ID;

typedef struct  {
    uint32_t            in_use;
    timer_cb_fr_isr_t   cb;
    void *              user_arg;
    osThreadId_t        tid;
    timer_clksource_t   clksource;
    uint32_t*           ptmid;
    uint32_t            perf_in_use;
    uint32_t            perf_cnt;
    uint32_t            perf_last_instant;
}kdp_timer_t;

typedef volatile union U_regTIMER
{
    struct
    {
        uint32_t Tm1Counter;    //Timer1 counter
        uint32_t Tm1Load;       //Timer1 auto reload value
        uint32_t Tm1Match1;     //Timer1 match value 1
        uint32_t Tm1Match2;     //Timer1 match value 2
        uint32_t Tm2Counter;    //Timer2 counter
        uint32_t Tm2Load;       //Timer2 auto reload value
        uint32_t Tm2Match1;     //Timer2 match value 1
        uint32_t Tm2Match2;     //Timer2 match value 2
        uint32_t Tm3Counter;    //Timer3 counter
        uint32_t Tm3Load;       //Timer3 auto reload value
        uint32_t Tm3Match1;     //Timer3 match value 1
        uint32_t Tm3Match2;     //Timer3 match value 2
        uint32_t TmCR;          //Timer1, Timer2, Timer3 control registers
        uint32_t IntrState;     //Interrupt state of FTTMR010
        uint32_t IntrMask;      //Interrupt mask of FTTMR010
        uint32_t TmRevision;    //Revision number of FTTMR010
    }dw;    //double word

    struct
    {
        uint32_t Tm1Counter     :32;    //Timer 1 counter
        uint32_t Tm1Load        :32;    //Timer 1 load
        uint32_t Tm1Match1      :32;    //Timer 1 match level 1
        uint32_t Tm1Match2      :32;    //Timer 1 match level 2
        uint32_t Tm2Counter     :32;    //Timer 2 counter
        uint32_t Tm2Load        :32;    //Timer 2 load
        uint32_t Tm2Match1      :32;    //Timer 2 match level 1
        uint32_t Tm2Match2      :32;    //Timer 2 match level 2
        uint32_t Tm3Counter     :32;    //Timer 3 counter
        uint32_t Tm3Load        :32;    //Timer 3 load
        uint32_t Tm3Match1      :32;    //Timer 3 match level 1
        uint32_t Tm3Match2      :32;    //Timer 3 match level 2

        uint32_t Tm1En          :1;     //Timer 1 enable 0: disable / 1: enable
        uint32_t Tm1Clock       :1;     //Timer 1 clock source 0: PCLK / 1: EXT1CLK
        uint32_t Tm1OfEn        :1;     //Timer 1 overflow interrupt enable bit 0: disable / 1: enable
        uint32_t Tm2En          :1;     //Timer 2 enable 0: disable / 1: enable
        uint32_t Tm2Clock       :1;     //Timer 2 clock source 0: PCLK / 1: EXT2CLK
        uint32_t Tm2OfEn        :1;     //Timer 2 overflow interrupt enable bit 0: disable / 1: enable
        uint32_t Tm3En          :1;     //Timer 3 enable 0: disable / 1: enable
        uint32_t Tm3Clock       :1;     //Timer 3 clock source 0: PCLK / 1: EXT3CLK
        uint32_t Tm3OfEn        :1;     //Timer 3 overflow interrupt enable bit 0: disable / 1: enable
        uint32_t Tm1UpDown      :1;     //Timer 1 up or down count 0: down count / 1: up count
        uint32_t Tm2UpDown      :1;     //Timer 2 up or down count 0: down count / 1: up count
        uint32_t Tm3UpDown      :1;     //Timer 3 up or down count 0: down count / 1: up count
        uint32_t Reserve0       :20;

        uint32_t STm1Match1     :1;     //Timer 1 match 1 interrupt status
        uint32_t STm1Match2     :1;     //Timer 1 match 2 interrupt status
        uint32_t STm1Overflow   :1;     //Timer 1 overflow interrupt status
        uint32_t STm2Match1     :1;     //Timer 2 match 1 interrupt status
        uint32_t STm2Match2     :1;     //Timer 2 match 2 interrupt status
        uint32_t STm2Overflow   :1;     //Timer 2 overflow interrupt status
        uint32_t STm3Match1     :1;     //Timer 3 match 1 interrupt status
        uint32_t STm3Match2     :1;     //Timer 3 match 2 interrupt status
        uint32_t STm3Overflow   :1;     //Timer 3 overflow interrupt status
        uint32_t Reserve1       :23;


        uint32_t MTm1Match1     :1;     //Timer 1 match 1 interrupt mask 0: no effect/ 1: masked
        uint32_t MTm1Match2     :1;     //Timer 1 match 2 interrupt mask 0: no effect/ 1: masked
        uint32_t MTm1Overflow   :1;     //Timer 1 overflow interrupt mask 0: no effect/ 1: masked
        uint32_t MTm2Match1     :1;     //Timer 2 match 1 interrupt mask 0: no effect/ 1: masked
        uint32_t MTm2Match2     :1;     //Timer 2 match 2 interrupt mask 0: no effect/ 1: masked
        uint32_t MTm2Overflow   :1;     //Timer 2 overflow interrupt mask 0: no effect/ 1: masked
        uint32_t MTm3Match1     :1;     //Timer 3 match 1 interrupt mask 0: no effect/ 1: masked
        uint32_t MTm3Match2     :1;     //Timer 3 match 2 interrupt mask 0: no effect/ 1: masked
        uint32_t MTm3Overflow   :1;     //Timer 3 overflow interrupt mask 0: no effect/ 1: masked
        uint32_t Reserve2       :23;
    }bf;    //bit-field
}U_regTIMER;
#define REGTIMER_0  ((U_regTIMER*) TIMER_REG_BASE)

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable
volatile uint32_t timer_used_mask;
volatile uint32_t timer_wakeup;

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable
volatile kdp_timer_t timer[TIMER_ID_MAX];
#ifdef TIMER_DGB
uint32_t ErrCnt;
#endif
uint32_t perftimerid=TIMER_ID_INIT;
uint32_t timer_inited = 0;

// Sec 9: declaration of static function prototype
static kdrv_status_t kdrv_timer_closelight(uint32_t* TimerId);
/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
static void kdrv_timer_ip_init()
{
    REGTIMER_0->dw.Tm1Counter = 0;
    REGTIMER_0->dw.Tm1Load    = 0;
    REGTIMER_0->dw.Tm1Match1  = 0;
    REGTIMER_0->dw.Tm1Match2  = 0;
    REGTIMER_0->dw.Tm2Counter = 0;
    REGTIMER_0->dw.Tm2Load    = 0;
    REGTIMER_0->dw.Tm2Match1  = 0;
    REGTIMER_0->dw.Tm2Match2  = 0;
    REGTIMER_0->dw.Tm3Counter = 0;
    REGTIMER_0->dw.Tm3Load    = 0;
    REGTIMER_0->dw.Tm3Match1  = 0;
    REGTIMER_0->dw.Tm3Match2  = 0;
    REGTIMER_0->dw.TmCR       = 0;
    REGTIMER_0->dw.IntrState  = 0;
    REGTIMER_0->dw.IntrMask   = 0;
}

uint32_t kdrv_timer_get_bit(uint32_t mask)
{
    uint32_t Idtmp = 0;
    if(mask != 0)
    {
        do{
            mask = mask>>1;
            Idtmp++;
        }while(mask);
        return Idtmp-1;
    }
    return 0;
}

kdrv_status_t kdrv_timer_intclear(uint32_t TimerId, uint32_t IntId)
{
    uint32_t value, ofs;

    ofs = (TM_REG_OFS * (TimerId));  //bit field offset 0/3/6   for TMCR/INTRSTATE/INTRMASK
    value = 1<<(IntId + ofs);

    REGTIMER_0->dw.IntrState = value;

    return KDRV_STATUS_OK;
}

void kdrv_timer_irqhandler(uint32_t bit_mask)
{
    uint32_t mask, n_bit, timerid, intid,timer_isr_stat;
    timer_isr_stat = REGTIMER_0->dw.IntrState & bit_mask;
    REGTIMER_0->dw.IntrState = timer_isr_stat;
    timer_isr_stat &= ~(REGTIMER_0->dw.IntrMask);
    #ifdef TIMER_DGB
    if(timer_isr_stat == 0 )
    {
        ErrCnt++;
    }
    #endif
    while(timer_isr_stat)
    {
        mask = (timer_isr_stat) & (((timer_isr_stat) - 1)^(timer_isr_stat));
        n_bit = kdrv_timer_get_bit(mask);
        timerid = (n_bit / 3);
        intid = (n_bit % 3);
        if(timer[timerid].perf_in_use == 1)
        {
            timer[timerid].perf_cnt++;
        }
        else if(timer[timerid].cb != NULL)
        {
            timer[timerid].cb((cb_event_t)intid, timer[timerid].user_arg);
        }
        else if(timer[timerid].tid != 0)
        {
            kdrv_timer_closelight(timer[timerid].ptmid);
            osThreadFlagsSet(timer[timerid].tid, FLAGS_TIMER_OF_DONE_EVENT);
        }
        else
        {
            kdrv_timer_closelight(timer[timerid].ptmid);
        }
        timer_isr_stat &= ~mask;
    }
    timer_wakeup = 1;
}

void  HW_TIMER0_IRQ_Handler()
{
    kdrv_timer_irqhandler(TIMER1_MASK);
}

void  HW_TIMER1_IRQ_Handler()
{
    kdrv_timer_irqhandler(TIMER2_MASK);
}

void  HW_TIMER2_IRQ_Handler()
{
    kdrv_timer_irqhandler(TIMER3_MASK);
}
kdrv_status_t kdrv_timer_register(uint32_t* TimerId, timer_cb_fr_isr_t cb_event,  void *arg, timer_clksource_t clksource_opt)
{
    if (timer[*TimerId].in_use)
    {
        return KDRV_STATUS_TIMER_ID_IN_USED;
    }
    timer[*TimerId].in_use           = 1;
    timer[*TimerId].cb               = cb_event;
    timer[*TimerId].user_arg         = arg;
    timer[*TimerId].ptmid            = TimerId;
    timer[*TimerId].clksource        = clksource_opt;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_unregister(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;

    timer[TimerID].in_use            = 0;
    timer[TimerID].cb                = NULL;
    timer[TimerID].user_arg          = NULL;
    timer[TimerID].tid               = NULL;
    timer[TimerID].ptmid             = NULL;
    timer[TimerID].perf_in_use       = 0;
    timer[TimerID].perf_cnt          = 0;
    timer[TimerID].perf_last_instant = 0;
    timer[TimerID].clksource         = TIMER_CLKSOURCE_PCLK;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_open(uint32_t* TimerId, timer_cb_fr_isr_t cb_event, void *arg, timer_clksource_t clksource_opt)
{
    uint32_t free_mask=0;
    kdrv_status_t ret = KDRV_STATUS_OK;
    free_mask = (~timer_used_mask) &(((~timer_used_mask) - 1)^(~timer_used_mask));

    if(free_mask > 0x7)
    {
        *TimerId = TIMER_ID_INIT;
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;
    }
    *TimerId = kdrv_timer_get_bit(free_mask);
    ret = kdrv_timer_register(TimerId, cb_event, arg, clksource_opt);

    if(ret == KDRV_STATUS_OK)
        timer_used_mask |= free_mask;

    return ret;
}

static kdrv_status_t kdrv_timer_closelight(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;
    if(timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;

    switch(TimerID%TM_PER_IP)
    {
        case TM_ID_1:
            REGTIMER_0->bf.Tm1En        = 0;
            REGTIMER_0->bf.Tm1UpDown    = 0;
            REGTIMER_0->bf.Tm1OfEn      = 0;
            REGTIMER_0->bf.MTm1Match1   = 1;
            REGTIMER_0->bf.MTm1Match2   = 1;
            REGTIMER_0->bf.MTm1Overflow = 1;
            REGTIMER_0->bf.Tm1Clock     = 0;
            break;
        case TM_ID_2:
            REGTIMER_0->bf.Tm2En        = 0;
            REGTIMER_0->bf.Tm2UpDown    = 0;
            REGTIMER_0->bf.Tm2OfEn      = 0;
            REGTIMER_0->bf.MTm2Match1   = 1;
            REGTIMER_0->bf.MTm2Match2   = 1;
            REGTIMER_0->bf.MTm2Overflow = 1;
            REGTIMER_0->bf.Tm2Clock     = 0;
            break;
        case TM_ID_3:
            REGTIMER_0->bf.Tm3En        = 0;
            REGTIMER_0->bf.Tm3UpDown    = 0;
            REGTIMER_0->bf.Tm3OfEn      = 0;
            REGTIMER_0->bf.MTm3Match1   = 1;
            REGTIMER_0->bf.MTm3Match2   = 1;
            REGTIMER_0->bf.MTm3Overflow = 1;
            REGTIMER_0->bf.Tm3Clock     = 0;
            break;
        default:
            break;
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_close(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;
    uint32_t used_mask = (1<<*TimerId);

    if (TimerID >= TIMER_ID_MAX)
        return KDRV_STATUS_TIMER_INVALID_TIMER_ID;

    if (timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;

    switch(TimerID%TM_PER_IP)
    {
        case TM_ID_1:
            REGTIMER_0->bf.Tm1En         = 0;
            REGTIMER_0->bf.Tm1UpDown     = 0;
            REGTIMER_0->bf.Tm1OfEn       = 0;
            REGTIMER_0->bf.MTm1Match1    = 1;
            REGTIMER_0->bf.MTm1Match2    = 1;
            REGTIMER_0->bf.MTm1Overflow  = 1;
            REGTIMER_0->bf.Tm1Clock      = 0;
            break;
        case TM_ID_2:
            REGTIMER_0->bf.Tm2En         = 0;
            REGTIMER_0->bf.Tm2UpDown     = 0;
            REGTIMER_0->bf.Tm2OfEn       = 0;
            REGTIMER_0->bf.MTm2Match1    = 1;
            REGTIMER_0->bf.MTm2Match2    = 1;
            REGTIMER_0->bf.MTm2Overflow  = 1;
            REGTIMER_0->bf.Tm2Clock      = 0;
            break;
        case TM_ID_3:
            REGTIMER_0->bf.Tm3En         = 0;
            REGTIMER_0->bf.Tm3UpDown     = 0;
            REGTIMER_0->bf.Tm3OfEn       = 0;
            REGTIMER_0->bf.MTm3Match1    = 1;
            REGTIMER_0->bf.MTm3Match2    = 1;
            REGTIMER_0->bf.MTm3Overflow  = 1;
            REGTIMER_0->bf.Tm3Clock      = 0;
            break;
        default:
            break;

    }

    kdrv_timer_unregister(TimerId);

    timer_used_mask = timer_used_mask & ~(used_mask);

    *TimerId = TIMER_ID_INIT;
    return KDRV_STATUS_OK;
}
kdrv_status_t kdrv_timer_set(uint32_t* TimerId, uint32_t Intval, timer_stat_t State)
{
    uint32_t load = Intval;
    uint32_t TimerID = *TimerId;
    uint32_t ofs;
    if(timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    ofs = (TM_REG_OFS * (TimerID % TM_PER_IP));  //bit field offset 0/3/6   for TMCR/INTRSTATE/INTRMASK
    if(State == TIMER_PAUSE)
    {
        switch(TimerID % TM_PER_IP)
        {
            case TM_ID_1:
                REGTIMER_0->bf.Tm1OfEn   = 0;
                break;
            case TM_ID_2:
                REGTIMER_0->bf.Tm2OfEn   = 0;
                break;
            case TM_ID_3:
                REGTIMER_0->bf.Tm3OfEn   = 0;
                break;
            default:
                break;
        }
        return KDRV_STATUS_OK;
    }
    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            REGTIMER_0->dw.Tm1Counter    = load;
            REGTIMER_0->dw.Tm1Load       = 0;
            REGTIMER_0->dw.Tm1Match1     = 0xFFFFFFFF;
            REGTIMER_0->dw.Tm1Match2     = 0xFFFFFFFF;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm1OfEn       = 1;
            REGTIMER_0->bf.Tm1Clock      = timer[TimerID].clksource;
            REGTIMER_0->bf.Tm1UpDown     = 0;
            REGTIMER_0->bf.MTm1Match1    = 1;
            REGTIMER_0->bf.MTm1Match2    = 1;
            REGTIMER_0->bf.MTm1Overflow  = 0;
            break;
        case TM_ID_2:
            REGTIMER_0->dw.Tm2Counter    = load;
            REGTIMER_0->dw.Tm2Load       = 0;
            REGTIMER_0->dw.Tm2Match1     = 0xFFFFFFFF;
            REGTIMER_0->dw.Tm2Match2     = 0xFFFFFFFF;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm2OfEn       = 1;
            REGTIMER_0->bf.Tm2Clock      = timer[TimerID].clksource;
            REGTIMER_0->bf.Tm2UpDown     = 0;
            REGTIMER_0->bf.MTm2Match1    = 1;
            REGTIMER_0->bf.MTm2Match2    = 1;
            REGTIMER_0->bf.MTm2Overflow  = 0;
            break;
        case TM_ID_3:
            REGTIMER_0->dw.Tm3Counter    = load;
            REGTIMER_0->dw.Tm3Load       = 0;
            REGTIMER_0->dw.Tm3Match1     = 0xFFFFFFFF;
            REGTIMER_0->dw.Tm3Match2     = 0xFFFFFFFF;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm3OfEn       = 1;
            REGTIMER_0->bf.Tm3Clock      = timer[TimerID].clksource;
            REGTIMER_0->bf.Tm3UpDown     = 0;
            REGTIMER_0->bf.MTm3Match1    = 1;
            REGTIMER_0->bf.MTm3Match2    = 1;
            REGTIMER_0->bf.MTm3Overflow  = 0;
            break;
        default:
            break;
    }

    if(State == TIMER_START)
    {
        kdrv_delay_us(1);
        REGTIMER_0->dw.TmCR |= ((TM_ENABLE) << ofs);
        if(timer[TimerID].cb == NULL)
        {
            timer[TimerID].tid = osThreadGetId();
            if(timer[TimerID].tid != NULL)
                osThreadFlagsWait(FLAGS_TIMER_OF_DONE_EVENT, osFlagsWaitAny, osWaitForever);
            else
            {
                timer_wakeup = 0;
                do
                {
                    __WFE();
                } while (timer_wakeup == 0);
            }
            kdrv_timer_close(TimerId);
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_perf_open(uint32_t* TimerId)
{
    uint32_t free_mask=0;
    kdrv_status_t ret = KDRV_STATUS_OK;
    uint32_t Used_mask = timer_used_mask;
    if(*TimerId != TIMER_ID_INIT)
        return KDRV_STATUS_TIMER_ID_IN_USED;
    free_mask = (~Used_mask) &(((~Used_mask) - 1)^(~Used_mask));
    if(free_mask == BIT0)
    {
        Used_mask |= free_mask;
        free_mask = (~Used_mask) &(((~Used_mask) - 1)^(~Used_mask));
    }
    if(free_mask > 0x7)
    {
        *TimerId = TIMER_ID_INIT;
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;
    }

    *TimerId = kdrv_timer_get_bit(free_mask);
    ret = kdrv_timer_register(TimerId, NULL, NULL, TIMER_CLKSOURCE_PCLK);

    if(ret == KDRV_STATUS_OK)
        timer_used_mask |= free_mask;

    return ret;
}

kdrv_status_t kdrv_timer_perf_set(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;
    if(timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    if(timer[TimerID].perf_in_use == 1)
        return KDRV_STATUS_ERROR;
    timer[TimerID].perf_in_use = 1;
    timer[TimerID].perf_cnt = 0;

    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            REGTIMER_0->dw.Tm1Counter    = 0;
            REGTIMER_0->dw.Tm1Load       = 0;
            REGTIMER_0->dw.Tm1Match1     = 0xFFFFFFFF;
            REGTIMER_0->dw.Tm1Match2     = 0xFFFFFFFF;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm1OfEn       = 1;
            REGTIMER_0->bf.Tm1Clock      = timer[TimerID].clksource;
            REGTIMER_0->bf.Tm1UpDown     = TM_UP;
            REGTIMER_0->bf.MTm1Match1    = 1;
            REGTIMER_0->bf.MTm1Match2    = 1;
            REGTIMER_0->bf.MTm1Overflow  = 0;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm1En         = 1;
            break;
        case TM_ID_2:
            REGTIMER_0->dw.Tm2Counter    = 0;
            REGTIMER_0->dw.Tm2Load       = 0;
            REGTIMER_0->dw.Tm2Match1     = 0xFFFFFFFF;
            REGTIMER_0->dw.Tm2Match2     = 0xFFFFFFFF;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm2OfEn       = 1;
            REGTIMER_0->bf.Tm2Clock      = timer[TimerID].clksource;
            REGTIMER_0->bf.Tm2UpDown     = TM_UP;
            REGTIMER_0->bf.MTm2Match1    = 1;
            REGTIMER_0->bf.MTm2Match2    = 1;
            REGTIMER_0->bf.MTm2Overflow  = 0;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm2En         = 1;
            break;
        case TM_ID_3:
            REGTIMER_0->dw.Tm3Counter    = 0;
            REGTIMER_0->dw.Tm3Load       = 0;
            REGTIMER_0->dw.Tm3Match1     = 0xFFFFFFFF;
            REGTIMER_0->dw.Tm3Match2     = 0xFFFFFFFF;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm3OfEn       = 1;
            REGTIMER_0->bf.Tm3Clock      = timer[TimerID].clksource;
            REGTIMER_0->bf.Tm3UpDown     = TM_UP;
            REGTIMER_0->bf.MTm3Match1    = 1;
            REGTIMER_0->bf.MTm3Match2    = 1;
            REGTIMER_0->bf.MTm3Overflow  = 0;
            kdrv_delay_us(1);
            REGTIMER_0->bf.Tm3En         = 1;
            break;
        default:
            break;
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_perf_get_instant(uint32_t* TimerId, uint32_t* instant, uint32_t* time)
{
    uint32_t ctime;
    uint32_t TimerID = *TimerId;
    if(timer[*TimerId].perf_in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    if(timer[*TimerId].perf_cnt >= PERF_ISR_CNT)
        timer[*TimerId].perf_cnt = PERF_ISR_CNT;// avoid overflow;

    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            ctime = REGTIMER_0->dw.Tm1Counter/50;
            break;
        case TM_ID_2:
            ctime = REGTIMER_0->dw.Tm2Counter/50;
            break;
        case TM_ID_3:
            ctime = REGTIMER_0->dw.Tm3Counter/50;
            break;
        default:
            break;
    }
    if(ctime < timer[*TimerId].perf_last_instant)
        *instant = ctime + (0xFFFFFFFF/50 - timer[*TimerId].perf_last_instant);
    else
        *instant = ctime - timer[*TimerId].perf_last_instant;
    timer[*TimerId].perf_last_instant = ctime;
    *time = ctime;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_perf_reset(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;
    if(timer[*TimerId].perf_in_use == 0)
        return KDRV_STATUS_ERROR;
    timer[*TimerId].perf_cnt = 0;
    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            REGTIMER_0->dw.Tm1Counter = 0;
            break;
        case TM_ID_2:
            REGTIMER_0->dw.Tm2Counter = 0;
            break;
        case TM_ID_3:
            REGTIMER_0->dw.Tm3Counter = 0;
            break;
        default:
            break;
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_initialize(void)
{
    if(timer_inited == 1)
        return KDRV_STATUS_ERROR;
    timer_inited = 1;
    timer_used_mask = 0;
    regSCU->bf.tmr_pclk = 1;
    regSCUEXT->clock_enable_reg0 |= (TMR_EXT1_CLK_EN|TMR_EXT2_CLK_EN|TMR_EXT3_CLK_EN);
    kdrv_timer_ip_init();

    NVIC_ClearPendingIRQ((IRQn_Type)HW_TIMER0_IRQn);
    NVIC_EnableIRQ((IRQn_Type)HW_TIMER0_IRQn);
    NVIC_ClearPendingIRQ((IRQn_Type)HW_TIMER1_IRQn);
    NVIC_EnableIRQ((IRQn_Type)HW_TIMER1_IRQn);
    NVIC_ClearPendingIRQ((IRQn_Type)HW_TIMER2_IRQn);
    NVIC_EnableIRQ((IRQn_Type)HW_TIMER2_IRQn);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_uninitialize(void)
{
    NVIC_DisableIRQ((IRQn_Type)HW_TIMER0_IRQn);
    NVIC_DisableIRQ((IRQn_Type)HW_TIMER1_IRQn);
    NVIC_DisableIRQ((IRQn_Type)HW_TIMER2_IRQn);
    regSCU->bf.tmr_pclk = 0;
    regSCUEXT->clock_enable_reg0 &= ~(TMR_EXT1_CLK_EN|TMR_EXT2_CLK_EN|TMR_EXT3_CLK_EN);
    timer_inited = 0;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_delay_ms(uint32_t msec)
{
    uint32_t timerid;
    kdrv_timer_initialize();
    kdrv_timer_open(&timerid, NULL, NULL, TIMER_CLKSOURCE_PCLK);
    return kdrv_timer_set(&timerid, INTERVAL_MS(msec), TIMER_START);
}

kdrv_status_t kdrv_timer_delay_us(uint32_t usec)
{
    uint32_t timerid;
    kdrv_timer_initialize();
    kdrv_timer_open(&timerid, NULL, NULL, TIMER_CLKSOURCE_PCLK);
    return kdrv_timer_set(&timerid, INTERVAL_US(usec), TIMER_START);
}

kdrv_status_t kdrv_timer_delay_ms_long(uint32_t msec)
{
    uint32_t timerid;
    kdrv_timer_initialize();
    kdrv_timer_open(&timerid, NULL, NULL, TIMER_CLKSOURCE_EXTCLK);
    return kdrv_timer_set(&timerid, INTERVAL_MS_LONG(msec), TIMER_START);
}
kdrv_status_t kdrv_timer_perf_measure_start(void)
{
    kdrv_timer_initialize();
    kdrv_timer_perf_open(&perftimerid);
    kdrv_timer_perf_reset(&perftimerid);
    return kdrv_timer_perf_set(&perftimerid);
}

kdrv_status_t kdrv_timer_perf_measure_get_us(uint32_t *diff, uint32_t *currt_cnt)
{
    return kdrv_timer_perf_get_instant(&perftimerid, diff, currt_cnt);
}

