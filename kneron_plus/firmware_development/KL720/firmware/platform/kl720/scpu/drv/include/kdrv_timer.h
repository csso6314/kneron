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
/**@addtogroup  KDRV_TIMER  KDRV_TIMER
 * @{
 * @brief       Kneron timer driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef _KDRV_TIMER_H_
#define _KDRV_TIMER_H_

#include "base.h"
#include <cmsis_os2.h>
#include "kdrv_status.h"

/** @brief Enumerations of all timer call back even return status */
typedef enum {
    TIMER_M1_TIMEOUT,   /**< Enum 0, reach timer M1 level */
    TIMER_M2_TIMEOUT,   /**< Enum 1, reach timer M2 level*/
    TIMER_OF_TIMEOUT    /**< Enum 2, timer overfloor*/
}cb_event_t;

/** @brief Enumerations of all timer status for kdrv_timer_set*/
typedef enum {
    TIMER_PAUSE,            /**< Enum 0, Pause timer */
    TIMER_START,            /**< Enum 1, Start timer */
    TIMER_STAT_DEFAULT      /**< Enum 2, Start timer with default setting*/
}timer_stat_t;

/** @brief Enumerations of all clock source option for timer_clksource_t*/
typedef enum {
    TIMER_CLKSOURCE_PCLK,       /**< Enum 0, clock source from pclk*/
    TIMER_CLKSOURCE_EXTCLK      /**< Enum 1, clock source from ext clock*/
}timer_clksource_t;

/** @brief  Timer user callback function with specified timer event.
* @note that this is callback form ISR context.
*/
typedef void (*timer_cb_fr_isr_t) (cb_event_t argument, void* arg);

/**
* @brief        Enable clock, init timer ip, register IRQ/ISR function
*              
* @param[in]    N/A
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_initialize(void);

/**
* @brief        Disable clock, and timer IRQ
*              
* @param[in]    N/A
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_uninitialize(void);

/**
* @brief        Request one timer id for further usage.
*              
* @param[out]   TimerId         pointer of timer id.
* @param[in]    event_cb        timer_cb_fr_isr_t, see @ref timer_cb_fr_isr_t
* @param[in]    arg             user define argument
* @param[in]    clksource_opt   clock source option, see @ref timer_clksource_t
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_open(uint32_t* TimerId, timer_cb_fr_isr_t cb_event, void *arg, timer_clksource_t clksource_opt);

/**
* @brief        Close specific timer id.
*              
* @param[in]    TimerId         pointer of timer id
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_close(uint32_t* TimerId);

/**
* @brief        Set specific timer with interval and status.
*              
* @param[in]    TimerId         pointer of timer id
* @param[in]    Interval        set timer interval
* @param[in]    timer_stat      see timer_stat, see @ref timer_stat_t
* @return       kdrv_status_t   see @ref kdrv_status_t
*
* @note         This API should be called after kdrv_timer_open()\n
*               Example:\n
*               uint32_t timerid;\n
*               kdrv_timer_open(&timerid, NULL, NULL);\n
*               kdrv_timer_set(&timerid, 5000000, TIMER_START);
*/
kdrv_status_t kdrv_timer_set(uint32_t* TimerId, uint32_t Intval, timer_stat_t State);

/**
* @brief        Open a timer with specific timer id for performance measurement.
* @note         Need use @ref kdrv_timer_perf_set() to start timing measurement.
*
* @param[out]   TimerId         pointer of timer id
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_perf_open(uint32_t* TimerId);

/**
* @brief        Set specific timer for performance measurment usage.
* 
* @param[in]    TimerId         pointer of timer id
* @return       kdrv_status_t   see @ref kdrv_status_t
*
* @note         You should call @ref kdrv_timer_perf_open() and @ref kdrv_timer_perf_reset() firstly before call this API.\n
*               Example:\n
*               uint32_t perftimerid;\n
*               kdrv_timer_perf_open(&pftimerid);\n
*               kdrv_timer_perf_reset(&pftimerid);\n
*               kdrv_timer_perf_set(&perftimerid);
*/
kdrv_status_t kdrv_timer_perf_set(uint32_t* TimerId);

/**
* @brief        Get time consumption.
* 
* @param[in]    TimerId         pointer of timer id
* @param[out]   instant         pointer of time instant register
* @return       Time cunsumption
*/
kdrv_status_t kdrv_timer_perf_get_instant(uint32_t* TimerId, uint32_t* instant, uint32_t* time);

/**
* @brief        Reset performance timer.
* 
* @param[in]    TimerId         pointer of timer id
* @return       kdrv_status_t   see @ref kdrv_status_t
*
* @note         After call @ref kdrv_timer_perf_open(), you should reset this timer first.\n
*               Example:\n
*               uint32_t perftimerid;
*               kdrv_timer_perf_open(&pftimerid);\n
*               kdrv_timer_perf_reset(&pftimerid);
*/
kdrv_status_t kdrv_timer_perf_reset(uint32_t* TimerId);

/**
* @brief        Let system delay ms
*              
* @param[in]    usec            time interval(ms).
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_delay_ms(uint32_t msec);

/**
* @brief        Let system delay us
*
* @param[in]    usec            time interval(us).
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_delay_us(uint32_t usec);

/**
* @brief        Let system delay ms
*
* @param[in]    usec            time interval(ms).
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_delay_ms_long(uint32_t msec);

/**
* @brief        Start to use performance measurement function.
*
* @param[in]    N/A
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_perf_measure_start(void);

/**
* @brief        Get time interval
* 
* @param[in]    diff            Difference time interval compare to last time instant.
* @param[in]    currt_cnt       Current counter.
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_timer_perf_measure_get_us(uint32_t *diff, uint32_t *currt_cnt);
#endif // _KDRV_TIMER_H_
/** @}*/

