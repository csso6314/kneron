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

/**@addtogroup  KDRV_NCPU  KDRV_NCPU
 * @{
 * @brief       Kneron NCPU driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */


#ifndef __KDRV_NCPU_H__
#define __KDRV_NCPU_H__

#include "base.h"

/**
 * @brief ncpu_set_stall() set ncpu into stall mode
 *
 * @param[in] is_stall 0: none, 1: stall
 * @return  N/A
 */
void kdrv_ncpu_set_stall(uint8_t is_stall);

/**
 * @brief ncpu boot initialize
 *
 * @param[in] N/A
 * @return  N/A
 */
void kdrv_ncpu_boot_initialize(void);

/**
 * @brief ncpu reset
 *
 * @param[in] N/A
 * @return  N/A
 */
void kdrv_ncpu_reset(void);

/**
 * @brief Get stall status
 *
 * @param[in] N/A
 * @return  N/A
 */
uint8_t kdrv_get_stall_status(void);

#endif // __KDRV_NCPU_H__
/** @}*/

