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
/**@addtogroup  KDRV_RTC  KDRV_RTC
 * @{
 * @brief       Kneron rtc driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __RTC_H__
#define __RTC_H__

#include "base.h"
#include "kdrv_status.h"

#define SECS_PER_MIN            60
#define MINS_PER_HOUR           60
#define HOURS_PER_DAY           24
#define SECS_PER_HOUR           (MINS_PER_HOUR * SECS_PER_MIN)
#define SECS_PER_DAY            (HOURS_PER_DAY * SECS_PER_HOUR)

#define MAX_DAYS_PER_MONTH      31
#define MONTH_PER_YEAR          12
#define YEARS_PER_CENTURY       100
#define CENTURY_PER_100         100     // for the time being

#define DAYS_PER_WEEK           7
#define DAYS_PER_YEAR           365

/** @brief Enumerations of all alarm typefor kdrv_timer_set*/
typedef enum{
    ALARM_IN_SECS = 1,       /**<Enum 1 */
    ALARM_IN_DATE_TIME,      /**<Enum 2 */
}alarm_type;

/** @brief Enumerations of periodic interrupt setting*/
typedef enum{
    PERIODIC_MONTH_INT = 0,         /**<Enum 0, Periodic interrupt output signal each month */
    PERIODIC_DAY_INT,               /**<Enum 1, Periodic interrupt output signal each day */
    PERIODIC_HOUR_INT,              /**<Enum 2, Periodic interrupt output signal each hour */
    PERIODIC_MIN_INT,               /**<Enum 3, Periodic interrupt output signal each minute */
    PERIODIC_SEC_INT,               /**<Enum 4, Periodic interrupt output signal each second */
}periodic_interrupt;

/** @brief Structure of RTC time setting*/
typedef struct{
    uint32_t    sec     : 8;    /* valid < SECS_PER_MIN */
    uint32_t    min     : 8;    /* valid < MINS_PER_HOUR */
    uint32_t    hour    : 8;    /* valid < HOURS_PER_DAY */
    uint32_t    weekday : 8;    /* valid < DAYS_PER_WEEK */
}rtc_time_s;

/** @brief Structure of RTC date setting*/
typedef struct{
    uint32_t    date    : 8;    /* valid < MAX_DAYS_PER_MONTH */
    uint32_t    month   : 8;    /* valid < MONTH_PER_YEAR */
    uint32_t    year    : 8;    /* valid < YEARS_PER_CENTURY */
    uint32_t    century : 8;    /* valid < CENTURY_PER_100? */
}rtc_date_s;

/**
 * @brief       Set RTC attribute - date and time
 *
 * @param[in]   *time   Pointer to time, see @ref rtc_time_s
 * @param[in]   *date   Pointer to date, see @ref rtc_date_s
 * @note            If date is NULL, RTC driver would use default date value(01/06/2020)\n
 *                      If time is NULL, RTC driver would use default time value(07:11:00 Mon(1))
 * @return      N/A
 */
void kdrv_rtc_set_attribute(rtc_time_s *time, rtc_date_s *date);

/**
 * @brief       Get current date and time configuration on RTC driver
 *
 * @param[in]   *time   Pointer to time, see @ref rtc_time_s
 * @param[in]   *date   Pointer to date, see @ref rtc_date_s
 * @return      N/A
 */
void kdrv_rtc_get_date_time(rtc_date_s *date, rtc_time_s *time);

/**
 * @brief       Get current date and time configuration in seconds
 *
 * @param[in]   *date_time_in_secs   Pointer to date and time in seconds
 * @return      N/A
 */
void kdrv_rtc_get_date_time_in_secs(uint32_t *date_time_in_secs);

/**
 * @brief       Enable an period RTC interrupt
 *
 * @param[in]   per_int_type    see @ref periodic_interrupt
 * @return      N/A
 */
void kdrv_rtc_periodic_enable(periodic_interrupt per_int_type);

/**
 * @brief       Enable an event in each second
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_rtc_sec_enable(void);

/**
 * @brief       Disable an event in each second
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_rtc_sec_disable(void);

/**
 * @brief       Enable  RTC alarm
 *
 * @param[in]   alm_type        see @ref alarm_type
 * @param[in]   *param1         Pointer to the first input parameter
 * @param[in]   *param2         Pointer to the first input parameter
 * @note            If alm_type is @ref ALARM_IN_SECS, param1 standards for time in seconds\n
 *                      If alm_type is @ref ALARM_IN_DATE_TIME,  param1 standards for @ref rtc_date_s, and param2 standards for @ref rtc_time_s
 * @return      N/A
 */
void kdrv_rtc_alarm_enable(alarm_type alm_type, void *param1, void *param2);

/**
 * @brief       Disable RTC alarm
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_rtc_alarm_disable(void);

/**
 * @brief       Initialize RTC with default value, date: 01/06/2020 and time: 07:11:00 Mon(1)
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_rtc_initialize(void);

#endif
/** @}*/
