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
*  KL520
*
*  Description:
*  ------------
*  This is TIMER driver
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
#include "kdrv_timer.h"
#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "io.h"
#include <string.h>

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define TIMER_DGB
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
    TIMER_ID_3,
    TIMER_ID_4,
    TIMER_ID_5,
    TIMER_ID_MAX,
    TIMER_ID_INIT=0xFF
}TIMER_ID;

typedef struct  {
    uint32_t            in_use;
    timer_cb_fr_isr_t   cb;
    void *              user_arg;
    osThreadId_t        tid;
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
#define REGTIMER_0  ((U_regTIMER*) TMR_FTPWMTMR010_0_PA_BASE)
#define REGTIMER_1  ((U_regTIMER*) TMR_FTPWMTMR010_1_PA_BASE)

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
uint32_t ErrCnt[2];
#endif
uint32_t TCnt[2];
uint32_t perftimerid;
uint32_t timer_inited = 0;
// Sec 9: declaration of static function prototype
static kdrv_status_t kdrv_timer_closelight(uint32_t* TimerId);
/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
static void read_set_bit(volatile uint32_t addr, uint32_t bit)
{
    uint32_t tmp;
    tmp = inw(addr);
    tmp |= (1<<bit);
    outw(addr, tmp);
}

static void read_clear_bit(volatile uint32_t addr, uint32_t bit)
{
    uint32_t tmp;
    tmp = inw(addr);
    tmp &= ~(1<<bit);
    outw(addr, tmp);
}
static void kdrv_timer_ip_init()
{
    U_regTIMER* Timer_Reg;
    int i;
    for(i = 0 ; i < 2 ; i++)
    {
        Timer_Reg = (i == 0)? REGTIMER_0: REGTIMER_1;
        Timer_Reg->dw.Tm1Counter = 0;
        Timer_Reg->dw.Tm1Load    = 0;
        Timer_Reg->dw.Tm1Match1  = 0;
        Timer_Reg->dw.Tm1Match2  = 0;
        Timer_Reg->dw.Tm2Counter = 0;
        Timer_Reg->dw.Tm2Load    = 0;
        Timer_Reg->dw.Tm2Match1  = 0;
        Timer_Reg->dw.Tm2Match2  = 0;
        Timer_Reg->dw.Tm3Counter = 0;
        Timer_Reg->dw.Tm3Load    = 0;
        Timer_Reg->dw.Tm3Match1  = 0;
        Timer_Reg->dw.Tm3Match2  = 0;
        Timer_Reg->dw.TmCR       = 0;
        Timer_Reg->dw.IntrState  = 0;
        Timer_Reg->dw.IntrMask   = 0;
    }
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
    U_regTIMER* Timer_Reg;
    uint32_t value, ofs;

    ofs = (TM_REG_OFS * (TimerId % TM_PER_IP));  //bit field offset 0/3/6   for TMCR/INTRSTATE/INTRMASK
    value = 1<<(IntId + ofs);

    Timer_Reg = (TimerId < TM_PER_IP)? REGTIMER_0: REGTIMER_1;
    Timer_Reg->dw.IntrState = value;

    return KDRV_STATUS_OK;
}

void kdrv_timer_irqhandler0(void)
{
    uint32_t mask, n_bit, timerid, intid,timer_isr_stat;
    timer_isr_stat = REGTIMER_0->dw.IntrState;
    timer_isr_stat &= ~(REGTIMER_0->dw.IntrMask);
    #ifdef TIMER_DGB
    if(timer_isr_stat == 0 )
    {
        ErrCnt[0]++;
    }
    #endif
    while(timer_isr_stat)
    {
        mask = (timer_isr_stat) & (((timer_isr_stat) - 1)^(timer_isr_stat));
        n_bit = kdrv_timer_get_bit(mask);
        timerid = (n_bit / 3);
        intid = (n_bit % 3);
        kdrv_timer_intclear(timerid, intid);
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
            TCnt[0]++;
        }
        else
        {
            kdrv_timer_closelight(timer[timerid].ptmid);
        }
        timer_isr_stat &= ~mask;
    }
    timer_wakeup = 1;
}
void kdrv_timer_irqhandler1(void)
{
    uint32_t mask, n_bit, timerid, intid,timer_isr_stat;
    timer_isr_stat = REGTIMER_1->dw.IntrState;
    timer_isr_stat &= ~(REGTIMER_1->dw.IntrMask);
    #ifdef TIMER_DGB
    if( timer_isr_stat == 0)
    {
        ErrCnt[1]++;
    }
    #endif
    while(timer_isr_stat)
    {
        mask = (timer_isr_stat) & (((timer_isr_stat) - 1)^(timer_isr_stat));
        n_bit = kdrv_timer_get_bit(mask);
        timerid = (n_bit / 3)+ TM_PER_IP;
        intid = (n_bit % 3);
        kdrv_timer_intclear(timerid, intid);
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
            TCnt[1]++;
        }
        else
        {
            kdrv_timer_closelight(timer[timerid].ptmid);
        }
        timer_isr_stat &= ~mask;
    }
    timer_wakeup = 1;
}
kdrv_status_t kdrv_timer_register(uint32_t* TimerId, timer_cb_fr_isr_t cb_event,  void *arg)
{
    if (timer[*TimerId].in_use)
    {
        return KDRV_STATUS_TIMER_ID_IN_USED;
    }
    timer[*TimerId].in_use = 1;
    timer[*TimerId].cb = cb_event;
    timer[*TimerId].user_arg = arg;
    timer[*TimerId].ptmid = TimerId;

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

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_open(uint32_t* TimerId, timer_cb_fr_isr_t cb_event, void *arg)
{
    uint32_t free_mask=0;
    kdrv_status_t ret = KDRV_STATUS_OK;
    free_mask = (~timer_used_mask) &(((~timer_used_mask) - 1)^(~timer_used_mask));

    if(free_mask > 0x3F)
    {
        *TimerId = TIMER_ID_INIT;
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;
    }
    *TimerId = kdrv_timer_get_bit(free_mask);
    ret = kdrv_timer_register(TimerId, cb_event, arg);

    if(ret == KDRV_STATUS_OK)
        timer_used_mask |= free_mask;

    return ret;
}

static kdrv_status_t kdrv_timer_closelight(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;
    U_regTIMER* Timer_Reg;
    if(timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    Timer_Reg = (TimerID < TM_PER_IP)? REGTIMER_0: REGTIMER_1;

    switch(TimerID%TM_PER_IP)
    {
        case TM_ID_1:
            Timer_Reg->bf.Tm1En        = 0;
            Timer_Reg->bf.Tm1UpDown    = 0;
            Timer_Reg->bf.Tm1OfEn      = 0;
            Timer_Reg->bf.MTm1Match1   = 1;
            Timer_Reg->bf.MTm1Match2   = 1;
            Timer_Reg->bf.MTm1Overflow = 1;
            break;
        case TM_ID_2:
            Timer_Reg->bf.Tm2En        = 0;
            Timer_Reg->bf.Tm2UpDown    = 0;
            Timer_Reg->bf.Tm2OfEn      = 0;
            Timer_Reg->bf.MTm2Match1   = 1;
            Timer_Reg->bf.MTm2Match2   = 1;
            Timer_Reg->bf.MTm2Overflow = 1;
            break;
        case TM_ID_3:
            Timer_Reg->bf.Tm3En        = 0;
            Timer_Reg->bf.Tm3UpDown    = 0;
            Timer_Reg->bf.Tm3OfEn      = 0;
            Timer_Reg->bf.MTm3Match1   = 1;
            Timer_Reg->bf.MTm3Match2   = 1;
            Timer_Reg->bf.MTm3Overflow = 1;
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
    U_regTIMER* Timer_Reg;

    if (TimerID >= TIMER_ID_MAX)
        return KDRV_STATUS_TIMER_INVALID_TIMER_ID;

    if (timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;

    Timer_Reg = (TimerID < TM_PER_IP)? REGTIMER_0: REGTIMER_1;

    switch(TimerID%TM_PER_IP)
    {
        case TM_ID_1:
            Timer_Reg->bf.Tm1En         = 0;
            Timer_Reg->bf.Tm1UpDown     = 0;
            Timer_Reg->bf.Tm1OfEn       = 0;
            Timer_Reg->bf.MTm1Match1    = 1;
            Timer_Reg->bf.MTm1Match2    = 1;
            Timer_Reg->bf.MTm1Overflow  = 1;
            break;
        case TM_ID_2:
            Timer_Reg->bf.Tm2En         = 0;
            Timer_Reg->bf.Tm2UpDown     = 0;
            Timer_Reg->bf.Tm2OfEn       = 0;
            Timer_Reg->bf.MTm2Match1    = 1;
            Timer_Reg->bf.MTm2Match2    = 1;
            Timer_Reg->bf.MTm2Overflow  = 1;
            break;
        case TM_ID_3:
            Timer_Reg->bf.Tm3En         = 0;
            Timer_Reg->bf.Tm3UpDown     = 0;
            Timer_Reg->bf.Tm3OfEn       = 0;
            Timer_Reg->bf.MTm3Match1    = 1;
            Timer_Reg->bf.MTm3Match2    = 1;
            Timer_Reg->bf.MTm3Overflow  = 1;
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
    uint32_t load = Intval*100;
    uint32_t TimerID = *TimerId;
    uint32_t bit;
    uint32_t ofs;
    U_regTIMER* Timer_Reg;
    if(timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    ofs = (TM_REG_OFS * (TimerID % TM_PER_IP));  //bit field offset 0/3/6   for TMCR/INTRSTATE/INTRMASK
    Timer_Reg = (TimerID < TM_PER_IP)? REGTIMER_0: REGTIMER_1;
    if(State == TIMER_PAUSE)
    {
        bit = kdrv_timer_get_bit((TM_ENABLE) << ofs);
        read_clear_bit((uint32_t)&Timer_Reg->dw.TmCR, bit);
        return KDRV_STATUS_OK;
    }
    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            Timer_Reg->dw.Tm1Counter    = load;
            Timer_Reg->dw.Tm1Load       = 0;
            Timer_Reg->dw.Tm1Match1     = 0xFFFFFFFF;
            Timer_Reg->dw.Tm1Match2     = 0xFFFFFFFF;
            Timer_Reg->bf.Tm1OfEn       = 1;
            Timer_Reg->bf.Tm1Clock      = 0;
            Timer_Reg->bf.Tm1UpDown     = 0;
            Timer_Reg->bf.MTm1Match1    = 1;
            Timer_Reg->bf.MTm1Match2    = 1;
            Timer_Reg->bf.MTm1Overflow  = 0;
            break;
        case TM_ID_2:
            Timer_Reg->dw.Tm2Counter    = load;
            Timer_Reg->dw.Tm2Load       = 0;
            Timer_Reg->dw.Tm2Match1     = 0xFFFFFFFF;
            Timer_Reg->dw.Tm2Match2     = 0xFFFFFFFF;
            Timer_Reg->bf.Tm2OfEn       = 1;
            Timer_Reg->bf.Tm2Clock      = 0;
            Timer_Reg->bf.Tm2UpDown     = 0;
            Timer_Reg->bf.MTm2Match1    = 1;
            Timer_Reg->bf.MTm2Match2    = 1;
            Timer_Reg->bf.MTm2Overflow  = 0;
            break;
        case TM_ID_3:
            Timer_Reg->dw.Tm3Counter    = load;
            Timer_Reg->dw.Tm3Load       = 0;
            Timer_Reg->dw.Tm3Match1     = 0xFFFFFFFF;
            Timer_Reg->dw.Tm3Match2     = 0xFFFFFFFF;
            Timer_Reg->bf.Tm3OfEn       = 1;
            Timer_Reg->bf.Tm3Clock      = 0;
            Timer_Reg->bf.Tm3UpDown     = 0;
            Timer_Reg->bf.MTm3Match1    = 1;
            Timer_Reg->bf.MTm3Match2    = 1;
            Timer_Reg->bf.MTm3Overflow  = 0;
            break;
        default:
            break;
    }

    if(State == TIMER_START)
    {
        Timer_Reg->dw.TmCR |= ((TM_ENABLE) << ofs);
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
    free_mask = (~Used_mask) &(((~Used_mask) - 1)^(~Used_mask));
    if(free_mask == BIT0 || free_mask == BIT3)
    {
        Used_mask |= free_mask;
        free_mask = (~Used_mask) &(((~Used_mask) - 1)^(~Used_mask));
    }
    if(free_mask > 0x3F)
    {
        *TimerId = TIMER_ID_INIT;
        return KDRV_STATUS_TIMER_ID_NOT_AVAILABLE;
    }

    *TimerId = kdrv_timer_get_bit(free_mask);
    ret = kdrv_timer_register(TimerId, NULL, NULL);

    if(ret == KDRV_STATUS_OK)
        timer_used_mask |= free_mask;

    return ret;
}

kdrv_status_t kdrv_timer_perf_set(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;
    U_regTIMER* Timer_Reg;
    if(timer[TimerID].in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    if(timer[TimerID].perf_in_use == 1)
        return KDRV_STATUS_ERROR;
    timer[TimerID].perf_in_use = 1;
    timer[TimerID].perf_cnt = 0;
    Timer_Reg = (TimerID < TM_PER_IP)? REGTIMER_0: REGTIMER_1;

    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            Timer_Reg->dw.Tm1Counter    = 0;
            Timer_Reg->dw.Tm1Load       = 0;
            Timer_Reg->dw.Tm1Match1     = 0xFFFFFFFF;
            Timer_Reg->dw.Tm1Match2     = 0xFFFFFFFF;
            Timer_Reg->bf.Tm1OfEn       = 1;
            Timer_Reg->bf.Tm1Clock      = 0;
            Timer_Reg->bf.Tm1UpDown     = TM_UP;
            Timer_Reg->bf.MTm1Match1    = 1;
            Timer_Reg->bf.MTm1Match2    = 1;
            Timer_Reg->bf.MTm1Overflow  = 0;
            Timer_Reg->bf.Tm1En = 1;
            break;
        case TM_ID_2:
            Timer_Reg->dw.Tm2Counter    = 0;
            Timer_Reg->dw.Tm2Load       = 0;
            Timer_Reg->dw.Tm2Match1     = 0xFFFFFFFF;
            Timer_Reg->dw.Tm2Match2     = 0xFFFFFFFF;
            Timer_Reg->bf.Tm2OfEn       = 1;
            Timer_Reg->bf.Tm2Clock      = 0;
            Timer_Reg->bf.Tm2UpDown     = TM_UP;
            Timer_Reg->bf.MTm2Match1    = 1;
            Timer_Reg->bf.MTm2Match2    = 1;
            Timer_Reg->bf.MTm2Overflow  = 0;
            Timer_Reg->bf.Tm2En         = 1;
            break;
        case TM_ID_3:
            Timer_Reg->dw.Tm3Counter    = 0;
            Timer_Reg->dw.Tm3Load       = 0;
            Timer_Reg->dw.Tm3Match1     = 0xFFFFFFFF;
            Timer_Reg->dw.Tm3Match2     = 0xFFFFFFFF;
            Timer_Reg->bf.Tm3OfEn       = 1;
            Timer_Reg->bf.Tm3Clock      = 0;
            Timer_Reg->bf.Tm3UpDown     = TM_UP;
            Timer_Reg->bf.MTm3Match1    = 1;
            Timer_Reg->bf.MTm3Match2    = 1;
            Timer_Reg->bf.MTm3Overflow  = 0;
            Timer_Reg->bf.Tm3En         = 1;
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
    U_regTIMER* Timer_Reg;
    Timer_Reg = (*TimerId < TM_PER_IP)? REGTIMER_0: REGTIMER_1;
    if(timer[*TimerId].perf_in_use == 0)
        return KDRV_STATUS_TIMER_ID_NOT_IN_USED;
    if(timer[*TimerId].perf_cnt >= PERF_ISR_CNT)
        timer[*TimerId].perf_cnt = PERF_ISR_CNT;// avoid overflow;

    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            ctime = Timer_Reg->dw.Tm1Counter/100;
            break;
        case TM_ID_2:
            ctime = Timer_Reg->dw.Tm2Counter/100;
            break;
        case TM_ID_3:
            ctime = Timer_Reg->dw.Tm3Counter/100;
            break;
        default:
            break;
    }
    if(ctime < timer[*TimerId].perf_last_instant)
        *instant = ctime + (42949672 - timer[*TimerId].perf_last_instant);
    else
        *instant = ctime - timer[*TimerId].perf_last_instant;
    timer[*TimerId].perf_last_instant = ctime;
    *time = ctime;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_perf_reset(uint32_t* TimerId)
{
    uint32_t TimerID = *TimerId;
    U_regTIMER* Timer_Reg;
    Timer_Reg = (*TimerId < TM_PER_IP)? REGTIMER_0: REGTIMER_1;
    if(timer[*TimerId].perf_in_use == 0)
        return KDRV_STATUS_ERROR;
    timer[*TimerId].perf_cnt = 0;
    switch(TimerID % TM_PER_IP)
    {
        case TM_ID_1:
            Timer_Reg->dw.Tm1Counter = 0;
            break;
        case TM_ID_2:
            Timer_Reg->dw.Tm2Counter = 0;
            break;
        case TM_ID_3:
            Timer_Reg->dw.Tm3Counter = 0;
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
    read_set_bit(SCU_FTSCU100_PA_BASE + 0x60, 15);
    read_set_bit(SCU_FTSCU100_PA_BASE + 0x60, 16);
    kdrv_timer_ip_init();

    NVIC_SetVector((IRQn_Type)TMR_FTTMR010_0_1_IRQ, (uint32_t)kdrv_timer_irqhandler0);
    NVIC_ClearPendingIRQ((IRQn_Type)TMR_FTTMR010_0_1_IRQ);
    NVIC_EnableIRQ((IRQn_Type)TMR_FTTMR010_0_1_IRQ);
    NVIC_SetVector((IRQn_Type)TMR_FTTMR010_0_2_IRQ, (uint32_t)kdrv_timer_irqhandler0);
    NVIC_ClearPendingIRQ((IRQn_Type)TMR_FTTMR010_0_2_IRQ);
    NVIC_EnableIRQ((IRQn_Type)TMR_FTTMR010_0_2_IRQ);
    NVIC_SetVector((IRQn_Type)TMR_FTTMR010_0_3_IRQ, (uint32_t)kdrv_timer_irqhandler0);
    NVIC_ClearPendingIRQ((IRQn_Type)TMR_FTTMR010_0_3_IRQ);
    NVIC_EnableIRQ((IRQn_Type)TMR_FTTMR010_0_3_IRQ);

    NVIC_SetVector((IRQn_Type)TMR_FTTMR010_1_1_IRQ, (uint32_t)kdrv_timer_irqhandler1);
    NVIC_ClearPendingIRQ((IRQn_Type)TMR_FTTMR010_1_1_IRQ);
    NVIC_EnableIRQ((IRQn_Type)TMR_FTTMR010_1_1_IRQ);
    NVIC_SetVector((IRQn_Type)TMR_FTTMR010_1_2_IRQ, (uint32_t)kdrv_timer_irqhandler1);
    NVIC_ClearPendingIRQ((IRQn_Type)TMR_FTTMR010_1_2_IRQ);
    NVIC_EnableIRQ((IRQn_Type)TMR_FTTMR010_1_2_IRQ);
    NVIC_SetVector((IRQn_Type)TMR_FTTMR010_1_3_IRQ, (uint32_t)kdrv_timer_irqhandler1);
    NVIC_ClearPendingIRQ((IRQn_Type)TMR_FTTMR010_1_3_IRQ);
    NVIC_EnableIRQ((IRQn_Type)TMR_FTTMR010_1_3_IRQ);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_uninitialize(void)
{
    read_clear_bit(SCU_FTSCU100_PA_BASE + 0x60, 15);
    read_clear_bit(SCU_FTSCU100_PA_BASE + 0x60, 16);
    NVIC_DisableIRQ((IRQn_Type)TMR_FTTMR010_0_1_IRQ);
    NVIC_DisableIRQ((IRQn_Type)TMR_FTTMR010_0_2_IRQ);
    NVIC_DisableIRQ((IRQn_Type)TMR_FTTMR010_0_3_IRQ);

    NVIC_DisableIRQ((IRQn_Type)TMR_FTTMR010_1_1_IRQ);
    NVIC_DisableIRQ((IRQn_Type)TMR_FTTMR010_1_2_IRQ);
    NVIC_DisableIRQ((IRQn_Type)TMR_FTTMR010_1_3_IRQ);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_timer_delay_ms(uint32_t msec)
{
    uint32_t timerid;
    kdrv_timer_initialize();
    kdrv_timer_open(&timerid, NULL, NULL);
    return kdrv_timer_set(&timerid, (msec*1000), TIMER_START);
}

kdrv_status_t kdrv_timer_delay_us(uint32_t usec)
{
    uint32_t timerid;
    kdrv_timer_initialize();
    kdrv_timer_open(&timerid, NULL, NULL);
    return kdrv_timer_set(&timerid, (usec), TIMER_START);
}
kdrv_status_t kdrv_timer_perf_measure_start(void)
{
    kdrv_timer_initialize();
    kdrv_timer_perf_open(&perftimerid);
    kdrv_timer_perf_reset(&perftimerid);
    return kdrv_timer_perf_set(&perftimerid);
}

kdrv_status_t kdrv_timer_perf_measure_get(uint32_t *instant, uint32_t *time)
{
    return kdrv_timer_perf_get_instant(&perftimerid, instant, time);
}

