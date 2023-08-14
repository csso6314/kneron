
/**
 * @file        kmdw_tdc.h
 * @brief       temporature difference controller APIs
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_TDC_H__
#define __KMDW_TDC_H__

#include "base.h"

#define TDC_UPDATE_PERIOD_MS    (500)   /**< tdc update period in ms*/

/**
 * @brief       Check if SW protection for dangerous temperature degree celsius is enabled
 *
 * @return      bool
 */
bool kmdw_tdc_sw_protection_en(void);

/**
 * @brief       Check whether the temperature is too higher to run NCPU
 *
 * @return      bool
 */
bool kmdw_tdc_is_ncpu_overheating(void);

/**
 * @brief       Get the current temperature
 *
 * @return      int32_t   the current temperature in integer format
 */
int32_t kmdw_tdc_get_temperature(void);

/**
 * @brief       Update TDC temperature in a period of TDC_UPDATE_PERIOD_MS
 */
void kmdw_tdc_update(void);

/**
 * @brief       Create a new thread to monitor the tempurature every 1 seconds.
 * @note        If the tempurature is over 70 degree celsius or under 20 degree celsius, whole system sould automatically shutdown\n
 *              If the tempurature is over 68 degree, NCPU would reject any operation
 */
void kmdw_tdc_start(void);

#endif
