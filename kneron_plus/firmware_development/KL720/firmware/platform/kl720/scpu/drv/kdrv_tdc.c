/* --------------------------------------------------------------------------
 * Copyright (c) 2018-2019 Kneron Inc. All rights reserved.
 *
 *      Name:    kdrv_tdc.c
 *      Purpose: Kneron Temperature Digital Converter
 *
 *---------------------------------------------------------------------------*/

#include <stdbool.h>
#include "kdrv_status.h"
#include "kdrv_tdc.h"
#include "regbase.h"
#include "kdrv_cmsis_core.h"

static volatile uint32_t tdc_data;
static volatile uint32_t tdc_avg_data;
static volatile uint8_t tdc_avg_divisor;
static volatile uint8_t tdc_scan_cnt;
static volatile uint8_t tdc_scan_max;
static volatile uint32_t tdc_data_cnt;
static volatile bool tdc_updated;

extern uint8_t kdrv_efuse_get_tdc_trim_code(void);

typedef volatile union kdrv_tdc_thrd_s {
    struct {
        uint32_t    thrd;
    } dw;
    struct {
        uint32_t    lthrd           :12;
        uint32_t    rsvd0801        :3;
        uint32_t    lthrd_en        :1;
        uint32_t    hthrd           :12;
        uint32_t    rsvd0802        :3;
        uint32_t    hthrd_en        :1;
    } bf;
} kdrv_tdc_thrd_t;


typedef volatile union kdrv_tdc_s {
    struct {
        uint32_t    data[8];    // 0x000 ~ 0x01C sensing data register
        uint8_t     rsvd20[0x60];// 0x020 ~ 0x07F reserved
        kdrv_tdc_thrd_t    thrd[8];    // 0x080 ~ 0x09C threshold detect register
        uint8_t     rsvda0[0x60];// 0x0A0 ~ 0x0FF reserved
        uint32_t    ctrl;       // 0x100 tdc control register
        uint32_t    trim;       // 0x104 trim register
        uint32_t    inten;      // 0x108 interrupt enable register
        uint32_t    intst;      // 0x10C interrupt status register
        uint32_t    tparam0;    // 0x110 timing parameter 0 register
        uint32_t    tparam1;    // 0x114 timing parameter 1 register
        uint32_t    reserved;   // 0x118 reserved
        uint32_t    prescal;    // 0x11C tdc clock re-scaler register
    } dw;
    struct {
        /* 0x000 ~ 0x01C sensing data register */
        uint32_t    data[8];

        /* 0x020 ~ 0x07F reserved */
        uint8_t     rsvd20[0x60];

        /* 0x080 ~ 0x09C threshold detect register */
        kdrv_tdc_thrd_t   thrd[8];

        /* 0x0A0 ~ 0x0FF reserved */
        uint8_t     rsvda0[0x60];

        /* 0x100 ctrl register */
        uint32_t    tdcc_en         :1;     // rw, tdc enable
        uint32_t    scanmode        :1;     // rw, scan mode 0:signle, 1:continue
        uint32_t    rsvd0           :22;    // reserved
        uint32_t    chopctrl        :8;     // rw, fixed to 0 in normal mode

        /* 0x104 trim register */
        uint32_t    trim            :8;     // rw, tdc trim value
        uint32_t    rsvd1           :24;    // reserved

        /* 0x108 inten register */
        uint32_t    resvd2          :8;     // reserved
        uint32_t    chdone_inten    :8;     // rw, tdc channel done interrupt enable
        uint32_t    undr_inten      :8;     // rw, under threshold int enable
        uint32_t    ovr_inten       :8;     // rw, over threshold int enable

        /* 0x10C intst register */
        uint32_t    inst            :1;     // ro, global tdc interrupt status
        uint32_t    done_inst       :1;     // ro, tdc done interrupt status
        uint32_t    th_inst         :1;     // ro, threshold status
        uint32_t    rsvd0c          :5;     // reserved
        uint32_t    chdone_inst     :8;     // rwc, tdc channel done status
        uint32_t    undr_inst       :8;     // rwc, under threshold int status
        uint32_t    ovr_inst        :8;     // rwc, over threshold int status

        /* 0x110 tparam0 register */
        uint32_t    tpu             :4;     // rw, tdc power up delay time
        uint32_t    ten             :4;     // rw, tdc enable delay time
        uint32_t    treset          :4;     // rw, reset time
        uint32_t    rsvd10          :4;     // reserved
        uint32_t    twu             :16;    // rw, wakeup delay time

        /* 0x114 tparam1 register */
        uint32_t    tsample         :10;    // rw, sample time
        uint32_t    rsvd14          :6;     // ro, reserved
        uint32_t    thold           :10;    // rw, hold time
        uint32_t    rsvd141         :6;     // ro, reserved

        /* 0x118 reserved register */
        uint32_t    rsvd118         :32;    // reserved

        /* 0x11C prescal register */
        uint32_t    xclkdiv         :8;     // rw, xclk divider ratio
        uint32_t    rsvd11c         :24;    // reserved
    } bf;
} kdrv_tdc_reg_t;

kdrv_tdc_reg_t *tdc_reg = (kdrv_tdc_reg_t*) (TDC_REG_BASE);


/* to register user callback and argument */
static tdc_interrupt_callback_t _usr_tdc_isr_cb = 0;
static void *_usr_arg = 0;

/*
 * @brief register user callback and parameter.
 */
bool kdrv_tdc_register_callback(tdc_interrupt_callback_t tdc_isr_cb, void *usr_arg)
{
    _usr_tdc_isr_cb = tdc_isr_cb;
    _usr_arg = usr_arg;
    return true;
}


/*
 * @brief tdc average scan value update
 */
static void kdrv_tdc_scan_update(void)
{
    tdc_data = tdc_reg->dw.data[0];
    if (tdc_scan_cnt == tdc_scan_max) {
        tdc_data_cnt += tdc_data;
        tdc_avg_data = tdc_data_cnt >> tdc_avg_divisor;
        tdc_scan_cnt = 0;
        tdc_data_cnt = 0;
        tdc_updated = true;
    }
    else if (tdc_scan_cnt < tdc_scan_max) {
        tdc_data_cnt += tdc_data;
    }
    if (tdc_reg->bf.ovr_inst || tdc_reg->bf.undr_inst) {
        /* over/under thrd status will bee high priority and it will overwrite the average value and set cb trigger */
        tdc_updated = true;
    }
}

void kdrv_tdc_update(void)
{
    __disable_irq();
    tdc_scan_cnt++;
    kdrv_tdc_scan_update();
    __enable_irq();
}

/*
 * @brief tdc isr and callback function
 */
static void kdrv_tdc_isr(void)
{
    if (tdc_reg->bf.done_inst) {
        tdc_scan_cnt++;
        kdrv_tdc_scan_update();
    }
    if (_usr_tdc_isr_cb && tdc_updated) {
        tdc_updated = false;
        _usr_tdc_isr_cb(_usr_arg);
    }

    kdrv_tdc_int_status_clr();
}

/*
 * @brief tdc average mode
 */
static uint8_t kdrv_tdc_avg_mode2cnt(kdrv_tdc_avg_mode_e avg_mode)
{
    switch(avg_mode) {
    case TDC_AVG_1:
        return 1;
    case TDC_AVG_2:
        return 2;
    case TDC_AVG_4:
        return 4;
    case TDC_AVG_8:
        return 8;
    case TDC_AVG_16:
        return 16;
    case TDC_AVG_32:
        return 32;
    case TDC_AVG_64:
        return 64;
    case TDC_AVG_128:
        return 128;
    default:
        return 1;
    }
}

/*
 * @brief tdc digital to temperature
 *      -40 <-> 163
 *      125 <-> 823
 */
static float kdrv_tdc_d2t(uint32_t data)
{
    int32_t val;
    if (data < 163)
        val = 163;
    else if(data > 823)
        val = 823;
    else
        val = data;
    return (-40 + ((val - 163) >> 2));
}

/*
 * @brief tdc digital to temperature
 *      -40 <-> 163
 *      125 <-> 823
 */
static int32_t kdrv_tdc_d2t_fixed_point(uint32_t data)
{
    int32_t val = data;
    return (-40 + ((val - 163) >> 2));
}

/*
 * @brief tdc temperature to digital
 *      -40 <-> 163
 *      125 <-> 823
 */
static uint32_t kdrv_tdc_t2d(float temp)
{
    float tval;
    if (temp < -40)
        tval = -40;
    else if(temp > 125)
        tval = 125;
    else
        tval = temp;
    return (uint32_t)((tval - (-40))/(float)(0.25) + 163);
}

/* ============================================================================*/
/* Export function */
/* intialize tdc module irq setting */
void kdrv_tdc_initialize(int polling_mode)
{
    tdc_reg->bf.tdcc_en = 0;
    tdc_reg->bf.trim = kdrv_efuse_get_tdc_trim_code();

    NVIC_SetVector((IRQn_Type)TDC_IRQn, (uint32_t)kdrv_tdc_isr);
    NVIC_ClearPendingIRQ(TDC_IRQn);
    NVIC_EnableIRQ(TDC_IRQn);

    if(!polling_mode){
        tdc_reg->bf.chdone_inten = 1;
    }
    else{
        tdc_reg->bf.chdone_inten = 0;
    }
}

/* unintialize tdc module irq */
void kdrv_tdc_uninitialize(void)
{
    NVIC_DisableIRQ(TDC_IRQn);
}

uint32_t kdrv_tdc_int_status_read(void)
{
    return(tdc_reg->dw.intst);
}

void kdrv_tdc_int_status_clr(void)
{
    //tdc_reg->dw.intst = tdc_reg->dw.intst;
    tdc_reg->bf.chdone_inst = 0x01;
    tdc_reg->bf.ovr_inst = 0x01;
    tdc_reg->bf.undr_inst = 0x01;
}

/* tdc enable to start scan */
void kdrv_tdc_enable(uint8_t scan_mode, kdrv_tdc_avg_mode_e avg_mode)
{
    tdc_data = 0;
    tdc_avg_data = 0;
    tdc_data_cnt = 0;
    tdc_scan_cnt = 0;
    tdc_scan_max = kdrv_tdc_avg_mode2cnt(avg_mode);
    tdc_avg_divisor = avg_mode;
    tdc_reg->bf.scanmode = scan_mode;
    tdc_reg->bf.tdcc_en = 1;
    //tdc_reg->bf.chdone_inten = 1;
}

/* tdc module disable */
void kdrv_tdc_disable(void)
{
    tdc_reg->bf.chdone_inten = 0;
    tdc_reg->bf.tdcc_en = 0;
}

uint8_t kdrv_tdc_get_data(void)
{
    return tdc_data;
}

uint8_t kdrv_tdc_get_avg_temp(void)
{
    if(tdc_updated == true){
        return kdrv_tdc_d2t_fixed_point(tdc_avg_data);
    }
    return 0;
}


/**
 * get temperture data
 */
float kdrv_tdc_get_temp(void)
{
    if(tdc_updated == true){
        return(kdrv_tdc_d2t(tdc_data));
    }
    return 0;
}

/**
 * get fixed point temperture data
 */
int32_t kdrv_tdc_get_temp_fixed_point(void)
{
    return kdrv_tdc_d2t_fixed_point(tdc_data);
}


void kdrv_tdc_set_thrd(float lthrd, float hthrd)
{
    tdc_reg->dw.thrd[0].bf.hthrd = kdrv_tdc_t2d(hthrd);
    tdc_reg->dw.thrd[0].bf.lthrd = kdrv_tdc_t2d(lthrd);
}

void kdrv_tdc_set_thrd_enflag(uint8_t lthrd_en, uint8_t hthrd_en)
{
    tdc_reg->dw.thrd[0].bf.lthrd_en = lthrd_en;
    tdc_reg->dw.thrd[0].bf.hthrd_en = hthrd_en;
}

void kdrv_tdc_set_conversion_int_enable(uint8_t en){
    tdc_reg->bf.chdone_inten = en;
}

void kdrv_tdc_set_thrd_int_enable(uint8_t lthrd_en, uint8_t hthrd_en){
    tdc_reg->bf.undr_inten = lthrd_en;
    tdc_reg->bf.ovr_inten = hthrd_en;
}
