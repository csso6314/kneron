/*
 * Kneron RTC (Real Time Clock) driver using SCU
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
 
#include <string.h>
#include "cmsis_os2.h"
#include "kmdw_power_manager.h"
#include "kdrv_power.h"
#include "rtc.h"
#include "kmdw_console.h"
#include "io.h"

#define RTC_PA_BASE         (SCU_FTSCU100_PA_BASE + 0x200)

/* SCU RTC Registers */
#define RTC_REG_TIME1       (RTC_PA_BASE + 0x00)
#define RTC_REG_TIME2       (RTC_PA_BASE + 0x04)
#define RTC_REG_ALM1        (RTC_PA_BASE + 0x08)
#define RTC_REG_ALM2        (RTC_PA_BASE + 0x0C)
#define RTC_REG_CTRL        (RTC_PA_BASE + 0x10)

/* RTC_REG_TIME1 & RTC_REG_ALM1 */
#define RTC_TIME1_WEEKDAY   0x07000000
#define RTC_TIME1_HOUR      0x003F0000
#define RTC_TIME1_MIN       0x00007F00
#define RTC_TIME1_SEC       0x0000007F

#define RTC_TIME1_WEEKDAY_SHIFT   24
#define RTC_TIME1_HOUR_SHIFT      16
#define RTC_TIME1_MIN_SHIFT       8
#define RTC_TIME1_SEC_SHIFT       0

/* RTC_REG_TIME2 & RTC_REG_ALM2 */
#define RTC_TIME2_CENTURY   0xFF000000
#define RTC_TIME2_YEAR      0x00FF0000
#define RTC_TIME2_MONTH     0x00001F00
#define RTC_TIME2_DATE      0x0000003F

#define RTC_TIME2_CENTURY_SHIFT   24
#define RTC_TIME2_YEAR_SHIFT      16
#define RTC_TIME2_MONTH_SHIFT     8
#define RTC_TIME2_DATE_SHIFT      0

/* RTC_REG_CTRL */
#define RTC_CTRL_PWREN_IE_OUT   BIT30
#define RTC_CTRL_CLK_READY      BIT15
#define RTC_CTRL_ALMEN_STS      BIT9
#define RTC_CTRL_RTCEN_STS      BIT8
#define RTC_CTRL_SECOUT_EN      BIT7
#define RTC_CTRL_PERIODIC_SEL   (BIT6 | BIT5 | BIT4)
#define RTC_CTRL_LOCK_EN        BIT2
#define RTC_CTRL_ALM_EN         BIT1
#define RTC_CTRL_EN             BIT0

#define RTC_CTRL_PERIODIC_SEL_SHIFT     4

#define RTC_CTRL_PERIODIC_SEC   0x70
#define RTC_CTRL_PERIODIC_MIN   0x60
#define RTC_CTRL_PERIODIC_HOUR  0x50
#define RTC_CTRL_PERIODIC_DAY   0x40
#define RTC_CTRL_PERIODIC_MONTH 0x30

union rtc_date_u {
    struct rtc_date_s       date_s;
    uint32_t                date_raw;
};

union rtc_time_u {
    struct rtc_time_s       time_s;
    uint32_t                time_raw;
};

static const uint32_t per_int_table[] = {
    [PERIODIC_MONTH_INT] = RTC_CTRL_PERIODIC_MONTH,
    [PERIODIC_DAY_INT] = RTC_CTRL_PERIODIC_DAY,
    [PERIODIC_HOUR_INT] = RTC_CTRL_PERIODIC_HOUR,
    [PERIODIC_MIN_INT] = RTC_CTRL_PERIODIC_MIN,
    [PERIODIC_SEC_INT] = RTC_CTRL_PERIODIC_SEC,
};

static const int days_of_month[] = {
    0,      // invalid
    31,     // Jan
    28,     // Feb
    31,     // Mar
    30,     // Apr
    31,     // May
    30,     // Jun
    31,     // Jul
    31,     // Aug
    30,     // Spt
    31,     // Oct
    30,     // Nov
    31,     // Dec
};

static const int days_to_month[] = {
    0,   // Jan
    31,     // Feb
    28+31,     // Mar
    31+28+31,     // Apr
    30+31+28+31,     // May
    31+30+31+28+31,     // Jun
    30+31+30+31+28+31,     // Jul
    31+30+31+30+31+28+31,     // Aug
    31+31+30+31+30+31+28+31,     // Spt
    30+31+31+30+31+30+31+28+31,     // Oct
    31+30+31+31+30+31+30+31+28+31,     // Nov
    30+31+30+31+31+30+31+30+31+28+31,     // Dec
    31+30+31+30+31+31+30+31+30+31+28+31,     // Year 365
};

static const char *weekdays[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

static struct rtc_date_s init_date = {
    /* 11/07/2019 */
    11,
    7,
    19,
    20,
};

static struct rtc_time_s init_time = {
    /* 07:11:00 Thu(4) */
    0,
    11,
    7,
    4,
};

static void rtc_rectify_date(struct rtc_date_s *date)
{
    date->century %= CENTURY_PER_100;
    date->year %= YEARS_PER_CENTURY;

    if (date->month)    // RTC valid month: 1-12
        date->month--;
    date->month %= MONTH_PER_YEAR;
    date->month++;

    if (date->date)     // RTC valid date: 1-31
        date->date--;
    date->date %= MAX_DAYS_PER_MONTH;
    date->date++;
}

static void rtc_rectify_time(struct rtc_time_s *time)
{
    time->weekday %= DAYS_PER_WEEK;
    time->hour %= HOURS_PER_DAY;
    time->min %= MINS_PER_HOUR;
    time->sec %= SECS_PER_MIN;
}

static void rtc_set_date(struct rtc_date_s *date)
{
    union rtc_date_u    *date_p;
    
    rtc_rectify_date(date);

    date_p = (union rtc_date_u *)date;
	outw(RTC_REG_TIME2, date_p->date_raw);
    dbg_msg("rtc_set_date: 0x%8.8x\n", date_p->date_raw);
}

static void rtc_get_date(struct rtc_date_s *date)
{
    union rtc_date_u    *date_p;
    
    date_p = (union rtc_date_u *)date;
	date_p->date_raw = inw(RTC_REG_TIME2);
}

static void rtc_set_time(struct rtc_time_s *time)
{
    union rtc_time_u    *time_p;

    rtc_rectify_time(time);

    time_p = (union rtc_time_u *)time;
	outw(RTC_REG_TIME1, time_p->time_raw);
    dbg_msg("rtc_set_time: 0x%8.8x\n", time_p->time_raw);
}

static void rtc_get_time(struct rtc_time_s *time)
{
    union rtc_time_u    *time_p;
    uint32_t first_read, second_read;

    time_p = (union rtc_time_u *)time;

    /* Read twice to get a good/same reading */
    do {
        first_read = inw(RTC_REG_TIME1);
        second_read = inw(RTC_REG_TIME1);
    } while (first_read != second_read);

    time_p->time_raw = second_read;
}

static void rtc_enable(void)
{
	masked_outw(RTC_REG_CTRL, RTC_CTRL_EN, RTC_CTRL_EN);
    do {
    } while (!(inw(RTC_REG_CTRL) & RTC_CTRL_RTCEN_STS));
}

static void rtc_disable(void)
{
    outw(RTC_REG_CTRL, 0);      // clear all
    do {
    } while (inw(RTC_REG_CTRL) & RTC_CTRL_RTCEN_STS);
}

static void rtc_alm_enable(void)
{
    masked_outw(RTC_REG_CTRL, RTC_CTRL_ALM_EN, RTC_CTRL_ALM_EN);
    do {
    } while (!(inw(RTC_REG_CTRL) & RTC_CTRL_ALMEN_STS));
}

static void rtc_alm_disable(void)
{
    masked_outw(RTC_REG_CTRL, 0, RTC_CTRL_ALM_EN);
    do {
    } while (inw(RTC_REG_CTRL) & RTC_CTRL_ALMEN_STS);
}

void rtc_alarm_enable(enum alarm_type alm_type, void *param1, void *param2)
{	
    union rtc_time_u    *time_p;
    union rtc_date_u    *date_p;
    uint32_t            tmp;

    // disable hw first
    rtc_alm_disable();

    if (alm_type == ALARM_IN_SECS) {
        struct rtc_time_s time;
        struct rtc_date_s date;
        uint32_t time_in_secs, carry_on;

        rtc_get_date(&date);
        rtc_get_time(&time);
        time_in_secs = *(uint32_t *)param1;

        // update seconds
        tmp = time.sec + time_in_secs;  // use u32 tmp to avoid overflow
        time.sec = tmp % SECS_PER_MIN;
        carry_on = tmp / SECS_PER_MIN;
        if (carry_on) {
            // update minutes
            tmp = time.min + carry_on;
            time.min = tmp % MINS_PER_HOUR;
            carry_on = tmp / MINS_PER_HOUR;
            if (carry_on) {
                // update hours
                tmp = time.hour + carry_on;
                time.hour = tmp % HOURS_PER_DAY;
                carry_on = tmp / HOURS_PER_DAY;
                if (carry_on) {
                    // update weekday
                    tmp = time.weekday + carry_on;
                    time.weekday = tmp % DAYS_PER_WEEK;

                    // Now update date
                    tmp = date.date + carry_on;
                    date.date = ((tmp - 1) % days_of_month[date.month]) + 1;
                    if (tmp > days_of_month[date.month]) {
                        // update month: no more than 1 month in future
                        date.month = (date.month % MONTH_PER_YEAR) + 1;
                        if (date.month == 1)
                            // update year
                            date.year++;
                    }
                }
            }
        }
        date_p = (union rtc_date_u *)&date;
        outw(RTC_REG_ALM2, date_p->date_raw);

        time_p = (union rtc_time_u *)&time;
        outw(RTC_REG_ALM1, time_p->time_raw);

        // enable now
        rtc_alm_enable();
    } else if (alm_type == ALARM_IN_DATE_TIME) {
        struct rtc_time_s *time;
        struct rtc_date_s *date;

        date = (struct rtc_date_s *)param1;
        rtc_rectify_date(date);
        date_p = (union rtc_date_u *)date;
        outw(RTC_REG_ALM2, date_p->date_raw);

        time = (struct rtc_time_s *)param2;
        rtc_rectify_time(time);
        time_p = (union rtc_time_u *)time;
        outw(RTC_REG_ALM1, time_p->time_raw);

        // enable now
        rtc_alm_enable();
    }
}

void rtc_alarm_disable(enum alarm_type alm_type)
{	
    rtc_alm_disable();
}

void rtc_periodic_enable(enum periodic_interrupt per_int_type)
{
    uint32_t    ctrl;

    ctrl = per_int_table[per_int_type];
    masked_outw(RTC_REG_CTRL, ctrl, RTC_CTRL_PERIODIC_SEL);
}

void rtc_current_time_info(void)
{
    struct rtc_time_s time;
    struct rtc_date_s date;

    rtc_get_date(&date);
    rtc_get_time(&time);

    info_msg("RTC: (%s) %2.2d%2.2d/%2.2d/%2.2d - %2.2d:%2.2d:%2.2d\n",
            weekdays[time.weekday],
            date.century, date.year, date.month, date.date,
            time.hour, time.min, time.sec);
}

void rtc_get_date_time_in_secs(uint32_t *date_time_in_secs)
{
    struct rtc_time_s time;
    struct rtc_date_s date;
    uint32_t long_time;

    rtc_get_date(&date);
    rtc_get_time(&time);

    if (0) {
        // simple test
        long_time = time.sec + time.min * SECS_PER_MIN + time.hour * SECS_PER_HOUR;
    } else {
        // TODO: leap year
        long_time = time.sec + time.min * SECS_PER_MIN + time.hour * SECS_PER_HOUR
            + (date.date - 1) * SECS_PER_DAY + days_to_month[date.month - 1] * SECS_PER_DAY
            + date.year * (DAYS_PER_YEAR * SECS_PER_DAY);
    }
    
    if (date_time_in_secs != NULL)
        *date_time_in_secs = long_time;
    else
        info_msg("Flat time: %d\n", long_time);
}

void rtc_get_date_time(struct rtc_date_s *date, struct rtc_time_s *time)
{
    if (date != NULL)
        rtc_get_date(date);
    if (time != NULL)
        rtc_get_time(time);
}

void rtc_init(struct rtc_time_s *time, struct rtc_date_s *date)
{
    rtc_disable();

    if (time == NULL)
        rtc_set_time(&init_time);
    else
        rtc_set_time(time);

    if (date == NULL)
        rtc_set_date(&init_date);
    else
        rtc_set_date(date);

    rtc_enable();
}
