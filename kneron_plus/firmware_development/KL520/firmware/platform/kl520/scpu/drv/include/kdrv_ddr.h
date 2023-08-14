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

/**@addtogroup  KDRV_DDR  KDRV_DDR
 * @{
 * @brief       Kneron generic DDR driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_DDR_H__
#define __KDRV_DDR_H__

/** @brief Enumeration of ddr init mode */
enum kdrv_ddr_init_mode {
    DDR_INIT_WAKEUP_ONLY = 0,           /**<Wake up DDR controller only. */
    DDR_INIT_ALL,                       /**<DDRx SDRAM enters into normal mode, \n
                                            In the normal mode, all operations run at full speed.*/
    DDR_INIT_ALL_EXIT_SELF_REFRESH,     /**<To exit from the self refresh mode by software,\n
                                            users need to ensure that DDRx SDRAM is in the self refresh mode before issuing\n
                                            an existing self refresh command by software(kdrv_ddr_system_init()). */
};

/**
 * @brief           DDR wakeup and de-assert reset of DDR controller.
 *
 * @return          N/A
 */
void kdrv_ddr_wakeup(void);

/**
 * @brief           DDR initialize.
 *
 * @return          N/A
 */
void kdrv_ddr_system_init(enum kdrv_ddr_init_mode mode);

/**
 * @brief           DDR enter self-refresh mode to save power. 
 *
 * @return          N/A
 */
void kdrv_ddr_self_refresh_enter(void);

/**
 * @brief           DDR exit self-refresh mode to normal mode.
 *
 * @return          N/A
 */
void kdrv_ddr_self_refresh_exit(void);


#endif
