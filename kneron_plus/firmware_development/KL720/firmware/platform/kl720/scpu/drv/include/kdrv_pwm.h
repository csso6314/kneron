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
/**@addtogroup  KDRV_PWM    KDRV_PWM
 * @{
 * @brief       Kneron PWM timer driver - Pulse-width modulation API.
 * @note        If you don't want to use pwm timer, you can refer to @ref KDRV_TIMER
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef _KDRV_PWM_TIMER_H
#define _KDRV_PWM_TIMER_H

/******************************************************************************
Head Block of The File
******************************************************************************/
#include "base.h"
#include "kdrv_status.h"
#include "system_config.h"

#define APB_CLK                     APB_CLOCK

/**@brief  Enumerations of all timer callback event return status */
typedef enum {
    PWM_ID_1=1,    /**< Enum 1, PWM timer callback instance 1 */
    PWM_ID_2=2,    /**< Enum 2, PWM timer callback instance 2 */
} pwm_id;

/** @brief Enumerations of polarity of a PWM signal */
typedef enum {
    PWM_POLARITY_NORMAL = 0,    /**< Enum 0, A high signal for the duration of the duty-cycle, followed by a low signal for the remainder of the pulse period*/
    PWM_POLARITY_INVERSED,      /**< Enum 1,  A low signal for the duration of the duty-cycle, followed by a high signal for the remainder of the pulse period*/
} pwmpolarity;

/**
* @brief        kdrv_pwm_config
*
* @details      After config pwm timer via this API, you should call kdrv_pwm_enable() to let pwm timer work well.
*
* @param[in]    timer           pwm timer id, see @ref pwm_id 
* @param[in]    polarity        polarity, see @ref pwmpolarity
* @param[in]    duty_ms         duty cycle(ms)
* @param[in]    period_ms       period(ms)
* @return       kdrv_status_t   see @ref kdrv_status_t
*
* @note         Example:\n
*               kdrv_pwm_config(PWM_ID_1, PWM_POLARITY_NORMAL, duty, PWM0_FREQ_CNT, 0);\n
*               kdrv_pwm_enable(PWM_ID_1);
*/
kdrv_status_t kdrv_pwm_config(pwm_id pwmid, pwmpolarity polarity, uint32_t duty, uint32_t period, bool ns2clkcnt);

/**
* @brief        kdrv_pwm_enable
* 
* @param[in]    timer           pwm timer id, see @ref pwm_id 
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwm_enable(pwm_id pwmid);

/**
* @brief       kdrv_pwm_disable
* 
* @param[in]    timer           pwm timer id, see @ref pwm_id 
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwm_disable(pwm_id pwmid);

/**
* @brief       kdrv_pwm_initialize
* 
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwm_initialize(void);

/**
* @brief       kdrv_pwm_uninitialize
* 
* @return       kdrv_status_t   see @ref kdrv_status_t
*/
kdrv_status_t kdrv_pwm_uninitialize(void);

#endif //_KDRV_PWM_TIMER_H
/** @}*/

