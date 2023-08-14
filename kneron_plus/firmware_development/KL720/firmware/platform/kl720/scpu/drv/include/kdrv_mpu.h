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

/**@addtogroup  KDRV_MPU KDRV_MPU
 * @{
 * @brief       Kneron MPU driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_MPU_H__
#define __KDRV_MPU_H__

/**
 * @brief           config memorty protect space, siram + niram
 *
 * @return          N/A
 */
void kdrv_mpu_config(void);

/**
 * @brief           mpu protect enable for niram memory space
 *
 * @return          N/A
 */
void kdrv_mpu_niram_enable(void);

/**
 * @brief           mpu protect disable for niram memory space
 *
 * @return          N/A
 */
void kdrv_mpu_niram_disable(void);
#endif
