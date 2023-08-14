/*
 * Kneron RTC driver header
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __RTC_H__
#define __RTC_H__

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

enum alarm_type {
    ALARM_IN_SECS = 1,
    ALARM_IN_DATE_TIME,
};

enum periodic_interrupt {
    PERIODIC_MONTH_INT = 0,
    PERIODIC_DAY_INT,
    PERIODIC_HOUR_INT,
    PERIODIC_MIN_INT,
    PERIODIC_SEC_INT,
};

struct rtc_time_s {
    uint32_t    sec: 8;         /* valid < SECS_PER_MIN */
    uint32_t    min: 8;         /* valid < MINS_PER_HOUR */
    uint32_t    hour: 8;        /* valid < HOURS_PER_DAY */
    uint32_t    weekday: 8;     /* valid < DAYS_PER_WEEK */
};

struct rtc_date_s {
    uint32_t    date: 8;        /* valid < MAX_DAYS_PER_MONTH */
    uint32_t    month: 8;       /* valid < MONTH_PER_YEAR */
    uint32_t    year: 8;        /* valid < YEARS_PER_CENTURY */
    uint32_t    century: 8;     /* valid < CENTURY_PER_100? */
};

void rtc_init(struct rtc_time_s *time, struct rtc_date_s *date);
void rtc_current_time_info(void);
void rtc_get_date_time(struct rtc_date_s *date, struct rtc_time_s *time);
void rtc_get_date_time_in_secs(uint32_t *date_time_in_secs);
void rtc_periodic_enable(enum periodic_interrupt per_int_type);
void rtc_alarm_enable(enum alarm_type alm_type, void *param1, void *param2);
void rtc_alarm_disable(enum alarm_type alm_type);

#endif
