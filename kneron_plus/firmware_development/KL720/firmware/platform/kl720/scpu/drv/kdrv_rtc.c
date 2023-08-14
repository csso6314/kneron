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
*  kdrv_rtc.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  This is RTC driver
*
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

// Sec 0: Comment block of the file

// Sec 1: Include File
#include "kdrv_rtc.h"
#include "regbase.h"
#include "kdrv_cmsis_core.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define RTC_PA_BASE             (SCU_REG_BASE + 0x200)

#define RTC_CTRL_PERIODIC_SEC   0x7
#define RTC_CTRL_PERIODIC_MIN   0x6
#define RTC_CTRL_PERIODIC_HOUR  0x5
#define RTC_CTRL_PERIODIC_DAY   0x4
#define RTC_CTRL_PERIODIC_MONTH 0x3

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
typedef union{
    rtc_date_s      date_s;
    uint32_t        date_raw;
}rtc_date_u;

typedef union{
    rtc_time_s      time_s;
    uint32_t        time_raw;
}rtc_time_u;

static const uint32_t per_int_table[] = {
    [PERIODIC_MONTH_INT]    = RTC_CTRL_PERIODIC_MONTH,
    [PERIODIC_DAY_INT]      = RTC_CTRL_PERIODIC_DAY,
    [PERIODIC_HOUR_INT]     = RTC_CTRL_PERIODIC_HOUR,
    [PERIODIC_MIN_INT]      = RTC_CTRL_PERIODIC_MIN,
    [PERIODIC_SEC_INT]      = RTC_CTRL_PERIODIC_SEC,
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

//static const char *weekdays[] = {
//    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
//};

static rtc_date_s init_date = {
    /* 01/06/2020 */
    01,
    06,
    20,
    20,
};

static rtc_time_s init_time = {
    /* 07:11:00 Mon(1) */
    0,
    0,
    0,
    1,
};
typedef volatile union U_regRTC{
    struct{
        uint32_t rtc_time1;             //(0x200)   RTC timer register 1
        uint32_t rtc_time2;             //(0x204)   RTC timer register 2
        uint32_t rtc_alm1;              //(0x208)   RTC alarm time register 1
        uint32_t rtc_alm2;              //(0x20C)   RTC alarm time register 2
        uint32_t rtc_ctrl;              //(0x210)   RTC control register
    }dw;
    struct{
        //(0x200)  //RTC timer register 1
        uint32_t rtc_sec        : 7;    //Current second, The valid value is ranging from 0 to 59.
        uint32_t rsvd01         : 1;    //Reserved
        uint32_t rtc_min        : 7;    //Current minute, The valid value is ranging from 0 to 59.
        uint32_t rsvd02         : 1;    //Reserved
        uint32_t rtc_hour       : 6;    //Current hour,   The valid value is ranging from 0 to 23.
        uint32_t rsvd03         : 2;    //Reserved
        uint32_t rtc_weekday    : 3;    //Current weekday The valid value is ranging from 0 to 6. 0: Sunday 1: Monday 2: Tuesday 3: Wednesday 4: Thursday 5: Friday 6: Saturday
        uint32_t rsvd04         : 5;    //Reserved

        //(0x204)  //RTC timer register 2
        uint32_t rtc_date       : 6;    //Current day The valid value is ranging from 1 to 31.
        uint32_t rsvd05         : 2;    //Reserved
        uint32_t rtc_month      : 5;    //Current month The valid value is ranging from 1 to 12.
        uint32_t rsvd06         : 3;    //Reserved
        uint32_t rtc_year       : 8;    //Current year The valid value is ranging from 0 to 99.
        uint32_t rtc_century    : 8;    //Current century The valid value is ranging from 0 to 99.

        //(0x208)  //RTC alarm time register 1
        uint32_t alm_sec        : 7;    //Alarm second The valid value is ranging from 0 to 59. When all bits are set to ��1��, this field will be ��don��t care�� in the alarm matching.
        uint32_t rsvd07         : 1;    //Reserved
        uint32_t alm_min        : 7;    //Alarm minute The valid value is ranging from 0 to 59. When all bits are set to ��1��, this field will be ��don��t care�� in the alarm matching.
        uint32_t rsvd08         : 1;    //Reserved
        uint32_t alm_hour       : 6;    //Alarm hour The valid value is ranging from 0 to 23. When all bits are set to ��1��, this field will be ��don��t care�� in the alarm matching.
        uint32_t rsvd09         : 2;    //Reserved
        uint32_t alm_weekday    : 3;    //Alarm weekday The valid value is ranging from 0 to 6. When all bits are set to ��1��, this field will be ��don��t care�� in the alarm matching.
        uint32_t rsvd10         : 5;    //Reserved

        //(0x20C)  //RTC alarm time register 2
        uint32_t alm_date_      : 6;    //Alarm Date, the valid value is ranging from 1 to 31. When all bits are set to ��0��, the field is ��don��t care�� in alarm matching.
        uint32_t rsvd11         : 2;    //Reserved
        uint32_t alm_month      : 5;    //Alarm Month, the valid value is ranging from 1 to 12. When all bits are set to ��0��, the field is ��don��t care�� in alarm matching.
        uint32_t rsvd12         :19;    //Reserved

        //(0x210)  //RTC control register
        uint32_t rtc_en         : 1;    //RTC enable The RTC clock should be confirmed stable through monitoring bit 2 of the OSC control register (0x1C) before turning on RTC.
        uint32_t rtc_alarm_en   : 1;    //RTC alarm enable
        uint32_t lock_en        : 1;    //Lock the Time2 register after reading the Time1 register, the lock will be released by the following conditions: 1. Time2 is read. 2. Time1 or Time2 is written, 3. RTC is disabled.
        uint32_t rsvd13         : 1;    //Reserved
        uint32_t perint_sel     : 3;    //Periodic interrupt output signal select 3'b111: Each second 3'b110: Each minute and triggered at 0 second of every minute 3'b101: Each hour and triggered at 0 second of every hour. 3'b100: Each day and triggered at 0 second of every day 3'b011: Each month and triggered at 0 second of every month Others: Disable  
        uint32_t secout_en      : 1;    //Enable an event in each second
        uint32_t rtcen_sts      : 1;    //RTC enable status The bit will be set when RTC accepts the command and is cleared when RTC_EN is disabled
        uint32_t rtc_almen_sts  : 1;    //RTC alarm enable status The bit will be set when RTC accepts the command and is cleared when RTC_ALARM_EN is disabled.
        uint32_t rsvd14         : 1;    //Reserved
        uint32_t pwuten_sts     : 1;    //Periodic wakeup timer enable status
        uint32_t rsvd15         : 3;    //Reserved
        uint32_t rtc_clk_ready  : 1;    //RTC clock stable status
        uint32_t rsvd16         :14;    //Reserved
        uint32_t pwren_ie_out   : 1;    //T_pwren_ie control. This bit is valid when FTSCU100_ALM_PWRON is defined.
        uint32_t rsvd17         : 1;    //Reserved
    }bf;
}U_regRTC;

#define RTCREG  ((U_regRTC*) RTC_PA_BASE)
/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable


// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
static void kdrv_rtc_rectify_date(rtc_date_s *date)
{
    date->century   %= CENTURY_PER_100;
    date->year      %= YEARS_PER_CENTURY;

    if (date->month)    // RTC valid month: 1-12
        date->month--;
    date->month     %= MONTH_PER_YEAR;
    date->month++;

    if (date->date)     // RTC valid date: 1-31
        date->date--;
    date->date      %= MAX_DAYS_PER_MONTH;
    date->date++;
}

static void kdrv_rtc_rectify_time(rtc_time_s *time)
{
    time->weekday   %= DAYS_PER_WEEK;
    time->hour      %= HOURS_PER_DAY;
    time->min       %= MINS_PER_HOUR;
    time->sec       %= SECS_PER_MIN;
}

static void kdrv_rtc_set_date(rtc_date_s *date)
{
    rtc_date_u    *date_p;

    kdrv_rtc_rectify_date(date);

    date_p = (rtc_date_u *)date;
    RTCREG->dw.rtc_time2 = date_p->date_raw;
}

static void kdrv_rtc_get_date(rtc_date_s *date)
{
    rtc_date_u    *date_p;

    date_p = (rtc_date_u *)date;
    date_p->date_raw = RTCREG->dw.rtc_time2;
}

static void kdrv_rtc_set_time(rtc_time_s *time)
{
    rtc_time_u    *time_p;

    kdrv_rtc_rectify_time(time);

    time_p = (rtc_time_u *)time;
    RTCREG->dw.rtc_time1 = time_p->time_raw;
}

static void kdrv_rtc_get_time(rtc_time_s *time)
{
    rtc_time_u    *time_p;
    uint32_t first_read, second_read;

    time_p = (rtc_time_u *)time;

    /* Read twice to get a good/same reading */
    do {
        first_read  = RTCREG->dw.rtc_time1;
        second_read = RTCREG->dw.rtc_time1;
    } while (first_read != second_read);

    time_p->time_raw = second_read;
}

static void kdrv_rtc_enable(void)
{
    RTCREG->bf.rtc_en = 1;
    do {
    } while (!RTCREG->bf.rtcen_sts);
}

static void kdrv_rtc_disable(void)
{
    RTCREG->dw.rtc_ctrl = 0;
    do {
    } while (RTCREG->bf.rtcen_sts);
}

static void kdrv_rtc_alm_enable(void)
{
    RTCREG->bf.rtc_alarm_en = 1;
    do {
    } while (!RTCREG->bf.rtc_almen_sts);
}

static void kdrv_rtc_alm_disable(void)
{
    RTCREG->bf.rtc_alarm_en = 0;
    do {
    } while (RTCREG->bf.rtc_almen_sts);
}

void kdrv_rtc_alarm_enable(alarm_type alm_type, void *param1, void *param2)
{
    rtc_time_u    *time_p;
    rtc_date_u    *date_p;
    uint32_t            tmp;

    // disable hw first
    kdrv_rtc_alm_disable();

    if (alm_type == ALARM_IN_SECS) {
        rtc_time_s time;
        rtc_date_s date;
        uint32_t time_in_secs, carry_on;

        kdrv_rtc_get_date(&date);
        kdrv_rtc_get_time(&time);
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
        date_p = (rtc_date_u *)&date;
        RTCREG->dw.rtc_alm2 = date_p->date_raw;

        time_p = (rtc_time_u *)&time;
        RTCREG->dw.rtc_alm1 = time_p->time_raw;
        

        // enable now
        kdrv_rtc_alm_enable();
    } 
    else if (alm_type == ALARM_IN_DATE_TIME) 
    {
        rtc_time_s *time;
        rtc_date_s *date;

        date = (rtc_date_s *)param1;
        kdrv_rtc_rectify_date(date);
        date_p = (rtc_date_u *)date;
        RTCREG->dw.rtc_alm2 = date_p->date_raw;

        time = (rtc_time_s *)param2;
        kdrv_rtc_rectify_time(time);
        time_p = (rtc_time_u *)time;
        RTCREG->dw.rtc_alm1 = time_p->time_raw;

        // enable now
        kdrv_rtc_alm_enable();
    }
}

void kdrv_rtc_alarm_disable(void)
{
    kdrv_rtc_alm_disable();
}

void kdrv_rtc_periodic_enable(periodic_interrupt per_int_type)
{
    uint32_t    ctrl;

    ctrl = per_int_table[per_int_type];
    RTCREG->bf.perint_sel = ctrl;
}

void kdrv_rtc_sec_enable(void)
{
    RTCREG->bf.secout_en = 1;
}
void kdrv_rtc_sec_disable(void)
{
    RTCREG->bf.secout_en = 0;
}

void kdrv_rtc_get_date_time_in_secs(uint32_t *date_time_in_secs)
{
    rtc_time_s time;
    rtc_date_s date;
    uint32_t long_time;

    kdrv_rtc_get_date(&date);
    kdrv_rtc_get_time(&time);

    {
        // TODO: leap year
        long_time = time.sec + time.min * SECS_PER_MIN + time.hour * SECS_PER_HOUR
            + (date.date - 1) * SECS_PER_DAY + days_to_month[date.month - 1] * SECS_PER_DAY
            + date.year * (DAYS_PER_YEAR * SECS_PER_DAY);
    }

    if (date_time_in_secs != NULL)
        *date_time_in_secs = long_time;
}

void kdrv_rtc_get_date_time(rtc_date_s *date, rtc_time_s *time)
{
    if (date != NULL)
        kdrv_rtc_get_date(date);
    if (time != NULL)
        kdrv_rtc_get_time(time);
}

void kdrv_rtc_set_attribute(rtc_time_s *time, rtc_date_s *date)
{
    kdrv_rtc_disable();

    if (time == NULL)
        kdrv_rtc_set_time(&init_time);
    else
        kdrv_rtc_set_time(time);

    if (date == NULL)
        kdrv_rtc_set_date(&init_date);
    else
        kdrv_rtc_set_date(date);

    kdrv_rtc_enable();
}
void kdrv_rtc_initialize(void)
{
    kdrv_rtc_set_attribute(NULL, NULL);
}

