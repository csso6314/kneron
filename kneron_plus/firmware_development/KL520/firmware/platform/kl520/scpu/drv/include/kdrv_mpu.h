/**
 * @file        kdrv_mpu.h
 * @brief       Kneron MPU driver
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
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
