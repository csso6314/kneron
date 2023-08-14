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

/**@addtogroup  KDRV_IPC  KDRV_IPC
 * @{
 * @brief       Kneron ipc driver
 * @version    v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_IPC_H__
#define __KDRV_IPC_H__

/**
 * @brief      Enable SCPU IPC interrupt to NCPU
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_ipc_enable_to_ncpu_int(void);

/**
 * @brief      Trigger SCPU IPC interrup[t to NCPU
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_ipc_trigger_to_ncpu_int(void);

/**
 * @brief      Clear SCPU IPC interrupt to NCPU
 *
 * @param[in]   N/A
 * @return      N/A
 */
void kdrv_ipc_clear_from_ncpu_int(void);


#endif/* __KDRV_IPC_H__ */
/** @}*/
