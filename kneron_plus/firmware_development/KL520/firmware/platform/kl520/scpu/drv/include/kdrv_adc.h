/**
 * @file        kdrv_adc.h
 * @brief       Kneron generic adc driver
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_ADC_H
#define __KDRV_ADC_H

#include "cmsis_os2.h"
#include "kdrv_status.h"

/**
 * @brief ADC Resource Configuration 
 */
typedef struct {
  int               io_base;        /**< ADC register interface */
  int               irq;            /**< ADC Event IRQ Number */
} kdrv_adc_resource_t;


/**
 * @brief           ADC driver initialization
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_adc_initialize(void);
/**
 * @brief           ADC driver uninitialization
 *
 * @param[in]       *res                  a handle of a ADC resource
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_adc_uninitialize(kdrv_adc_resource_t *res);
/**
 * @brief           ADC reset control
 *
 * @param[in]       *res                  a handle of a ADC resource
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_adc_rest(kdrv_adc_resource_t *res);
/**
 * @brief           ADC enable control
 *
 * @param[in]       *res                  a handle of a ADC resource
 *                  mode                  which mode to be enabled
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_adc_enable(kdrv_adc_resource_t *res, int mode);
/**
 * @brief           ADC data read control
 *
 * @param[in]       id                    which adc channel to be enabled
 * @return          int                   return ADC data
 */
int kdrv_adc_read(int id);
#endif
