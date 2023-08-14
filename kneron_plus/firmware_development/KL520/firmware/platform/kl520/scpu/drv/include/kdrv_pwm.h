/**
 * @file        kdrv_owm.h
 * @brief       Kneron PWM timer driver
 * @note        If you don't want to use pwm timer, you can refer to @ref KDRV_TIMER
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef _KDRV_PWM_TIMER_H
#define _KDRV_PWM_TIMER_H

#include "base.h"
#include "system_config.h"
#include "kdrv_status.h"

/**
 * @brief  Definition of PWM timer period
 */
#define APB_CLK                     APB_CLOCK                       /**< APB bus clock grequency */
#define PWMTMR_1000MSEC_PERIOD      (uint32_t)(APB_CLK)             /**< PWM timer 1000 ms period */
#define PWMTMR_5000MSEC_PERIOD      (uint32_t)(APB_CLK*5)           /**< PWM timer 5000 ms period */
#define PWMTMR_100MSEC_PERIOD       (uint32_t)(APB_CLK/10)          /**< PWM timer 100 ms period */
#define PWMTMR_20MSEC_PERIOD        (uint32_t)(APB_CLK/50)          /**< PWM timer 20 ms period */
#define PWMTMR_15MSEC_PERIOD        (uint32_t)(((APB_CLK/100)*3)/2) /**< PWM timer 15 ms period */
#define PWMTMR_10MSEC_PERIOD        (uint32_t)(APB_CLK/100)         /**< PWM timer 10 ms period */
#define PWMTMR_1MSEC_PERIOD         (uint32_t)(APB_CLK/1000)        /**< PWM timer 1 ms period */
#define PWMTMR_01MSEC_PERIOD        (uint32_t)(APB_CLK/10000)       /**< PWM timer 100 us period */

/**
 * @brief  Enumerations of all timer callback event return status
 */
typedef enum {
    PWMTIMER1=1,    /**< Enum 1, PWM timer callback instance 1 */
    PWMTIMER2=2,    /**< Enum 2, PWM timer callback instance 2 */
    PWMTIMER3=3,    /**< Enum 3, PWM timer callback instance 3 */
    PWMTIMER4=4,    /**< Enum 4, PWM timer callback instance 4 */
    PWMTIMER5=5,    /**< Enum 5, PWM timer callback instance 5 */
    PWMTIMER6=6     /**< Enum 6, PWM timer callback instance 6 */
} pwmtimer;

/**
 * @brief Enumerations of kl520 power domains
 */
typedef enum Timer_IoType
{
    IO_TIMER_RESETALL,  /**< Enum 0 */
    IO_TIMER_GETTICK,   /**< Enum 1 */
    IO_TIMER_SETTICK,   /**< Enum 2 */
    IO_TIMER_SETCLKSRC  /**< Enum 3 */
} timeriotype;

/**
 * @brief Enumerations of polarity of a PWM signal
 */
typedef enum {
    PWM_POLARITY_NORMAL = 0,    /**< Enum 0, A high signal for the duration of the duty-cycle, followed by a low signal for the remainder of the pulse period*/
    PWM_POLARITY_INVERSED       /**< Enum 1, A low signal for the duration of the duty-cycle, followed by a high signal for the remainder of the pulse period*/
} pwmpolarity;

/**
* @brief        Get t1 tick
* @return       t1_tick
*/
uint32_t kdrv_current_t1_tick(void);

/**
* @brief        Get t2 tick
* @return       t2_tick
*/
uint32_t kdrv_current_t2_tick(void);

/**
* @brief        Get t3 tick
* @return       t3_tick
*/
uint32_t kdrv_current_t3_tick(void);

/**
* @brief        Get t4 tick
* @return       t4_tick
*/
uint32_t kdrv_current_t4_tick(void);

/**
* @brief        Get t5 tick
* @return       t5_tick
*/
uint32_t kdrv_current_t5_tick(void);

/**
* @brief        Get t6 tick
* @return       t6_tick
*/
uint32_t kdrv_current_t6_tick(void);

/**
* @brief        Initialize specific timer id and give tick.
*
* @param[in]    TimerId             pwm timer id, see @ref pwmtimer
* @param[in]    tick                tick number
* @return       kdrv_status_t       see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwmtimer_initialize(pwmtimer timer, uint32_t tick);

/**
* @brief        Close specifiec pwm timer id.
*
* @param[in]    TimerId             pwm timer id, see @ref pwmtimer
* @return       kdrv_status_t       see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwmtimer_close(pwmtimer timer);

/**
* @brief        Reset pwm timer tick
*
* @param[in]    TimerId             pwm timer id, see @ref pwmtimer
* @return       kdrv_status_t       see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwmtimer_tick_reset(pwmtimer timer);

/**
* @brief        Use pwm timer to delay for certain time interval.
*
* @param[in]    TimerId             timer id
* @return       kdrv_status_t       see @ref kdrv_status_t
*
* @note         This API uses @ref PWMTIMER1 to run pwm timer tick.\n
*               You should avoid to use the duplicated @ref pwmtimer.\n
*               If you don't want to occupy one PWM timer instance, please refer to use @ref kdrv_timer_delay_ms()
*/
kdrv_status_t kdrv_pwmtimer_delay_ms(uint32_t msec);

/**
* @brief        kdrv_pwm_config
*
* @details      After config pwm timer via this API, you should call kdrv_pwm_enable() to let pwm timer work well.
*
* @param[in]    timer           pwm timer id, see @ref pwmtimer 
* @param[in]    polarity        polarity, see @ref pwmpolarity
* @param[in]    duty_ms         duty cycle(ms)
* @param[in]    period_ms       period(ms)
* @return       kdrv_status_t   see @ref kdrv_status_t
*
* @note         Example:\n
*               kdrv_pwm_config(PWMTIMER1, PWM_POLARITY_NORMAL, duty, PWM0_FREQ_CNT, 0);\n
*               kdrv_pwm_enable(PWMTIMER1);
*/
kdrv_status_t kdrv_pwm_config(pwmtimer timer, pwmpolarity polarity, uint32_t duty, uint32_t period, bool ns2clkcnt);

/**
* @brief        kdrv_pwm_enable
*
* @param[in]    timer           pwm timer id, see @ref pwmtimer
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwm_enable(pwmtimer timer);

/**
* @brief       kdrv_pwm_disable
*
* @param[in]    timer           pwm timer id, see @ref pwmtimer
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwm_disable(pwmtimer timer);
#endif //_KDRV_PWM_TIMER_H

