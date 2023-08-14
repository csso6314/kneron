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

#ifndef __KDRV_TDC_H__
#define __KDRV_TDC_H__
#include "base.h"
#include "kdrv_status.h"

/** tdc scan mode */
typedef enum  {
    TDC_SCAN_SINGLE = 0,
    TDC_SCAN_CONTINUE,
} kdrv_tdc_scan_mode_e;


/** tdc average scan count mode */
typedef enum  {
    TDC_AVG_1,
    TDC_AVG_2,
    TDC_AVG_4,
    TDC_AVG_8,
    TDC_AVG_16,
    TDC_AVG_32,
    TDC_AVG_64,
    TDC_AVG_128,
} kdrv_tdc_avg_mode_e;

/* kdrv_tdc_reg_t.intst interrupt status bit definition */
#define KDRV_TDC_INTST                  BIT(0)
#define KDRV_TDC_INTST_CH_DONE          BIT(1)
#define KDRV_TDC_INTST_THRD             BIT(2)
#define KDRV_TDC_INTST_CH0_DONE         BIT(8)
#define KDRV_TDC_INTST_THRD_CH0_UNDR    BIT(16)
#define KDRV_TDC_INTST_THRD_CH0_OVR     BIT(24)


/** TDC user callback function. Note that this is callback form ISR context. */
typedef void (*tdc_interrupt_callback_t)(void *arg);

void kdrv_tdc_initialize(int polling_mode);
void kdrv_tdc_uninitialize(void);
bool kdrv_tdc_register_callback(tdc_interrupt_callback_t tdc_isr_cb, void *usr_arg);
void kdrv_tdc_update(void);

/**
 * @brief tdc scan enable
 *
 * @param[in]   scan_mode       tdc scan mode 0:signle, 1:continue
 * @param[in]   avg_mode        scan average mode. @ref kdrv_tdc_avg_mode_e
 *
 * @return      N/A
 */
void kdrv_tdc_enable(uint8_t scan_mode, kdrv_tdc_avg_mode_e avg_mode);

/**
 * @brief tdc scan disable
 */
void kdrv_tdc_disable(void);



/**
 * @brief read interrupt status
 */
uint32_t kdrv_tdc_int_status_read(void);

/**
 * @brief tdc interrupt status clear
 */
void kdrv_tdc_int_status_clr(void);

/**
 * @brief get scan sensing data
 *
 * @return scan data
 */
uint8_t kdrv_tdc_get_data(void);

/**
 * @brief get averaged scan sensing temperature
 *
 * @return averaged scan data
 */
uint8_t kdrv_tdc_get_avg_temp(void);

/**
 * @brief get temperature value
 *
 * @return temperature data -40 ~ 125
 */
float kdrv_tdc_get_temp(void);

/**
 * @brief get fixed point temperature value
 *
 * @return temperature data -40 ~ 125
 */
int32_t kdrv_tdc_get_temp_fixed_point(void);

/**
 * @brief set high/low temperature threshold value
 *      -40 <-> 163
 *      125 <-> 823
 *
 * @param[in]   lthrd       low temperature threshold(-40~125, step 0.25)
 * @param[in]   hthrd       high temperature threshold(-40~125, step 0.25)
 * @return      void
 */
void kdrv_tdc_set_thrd(float lthrd, float hthrd);

/**
 * @brief set high/low temperature threshold enable flag
 *
 * @param[in]   lthrd_en    low threshold flag
 * @param[in]   hthrd_en    high threshold flag
 * @return      void
 */
void kdrv_tdc_set_thrd_enflag(uint8_t lthrd_en, uint8_t hthrd_en);

/**
 * @brief set high/low temperature threshold interrupt enable
 *
 * @param[in]   lthrd_en    low threshold interrupt enable
 * @param[in]   hthrd_en    high threshold interrupt enable
 * @return      void
 */
void kdrv_tdc_set_thrd_int_enable(uint8_t lthrd_en, uint8_t hthrd_en);

/**
 * @brief set channel conversion done interrupt enable
 *
 * @param[in]   en    enable value
 * @return      void
 */
void kdrv_tdc_set_conversion_int_enable(uint8_t en);

#endif
