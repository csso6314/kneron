/**
 * @file        kmdw_dfu.h
 * @brief       APIs for device firmware update
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_DFU_H__
#define __KMDW_DFU_H__

#include "base.h"
#include "cmsis_os2.h"

#define SCPU_PARTITION0_START_IN_FLASH      FLASH_FW_SCPU0_ADDR     /**< 0x2000 */
#define NCPU_PARTITION0_START_IN_FLASH      FLASH_FW_NCPU0_ADDR     /**< 0x18000 */
#define PARTITION_0_CFG_START_IN_FLASH      FLASH_FW_CFG0_ADDR      /**< 0X28000 */
#define SCPU_PARTITION1_START_IN_FLASH      FLASH_FW_SCPU1_ADDR     /**< 0x41000 */
#define NCPU_PARTITION1_START_IN_FLASH      FLASH_FW_NCPU1_ADDR     /**< 0x57000 */
#define PARTITION_1_CFG_START_IN_FLASH      FLASH_FW_CFG1_ADDR      /**< 0X67000 */

#define SCPU_START_ADDRESS                  (SiRAM_MEM_BASE + 0x2000 ) /**< - */
#define NCPU_START_ADDRESS                  NiRAM_MEM_BASE          /**< - */

#define PARTITION_CFG_SIZE                  32  /**< size of cfg partition */

#define SUCCESS                             0   /**< return code: success */
#define MSG_AUTH_FAIL                       251 /**< return code: auth fail */
#define MSG_FLASH_FAIL                      252 /**< return code: falshing fail */
#define MSG_DATA_ERROR                      253 /**< return code: data check fail */
#define MSG_FLASH_NO_SPACE                  254 /**< return code: flash no space fail */

/* function used to read DFU content */
typedef uint32_t (*FnReadData)(uint32_t addr, uint32_t img_size);

/* @brief Init DFU function
 * @param tmp_buf temp buffer: size should be >= VERIFY_BLK_SZ
 * @param fn_read_data function to read data
 * @return 0 on success
 */
int kmdw_dfu_init(uint8_t* tmp_buf, FnReadData fn_read_data);

/**
 * @brief Update SCPU firmware
 * @return 0 on success
 */
int kmdw_dfu_update_scpu(void);

/**
 * @brief Update NCPU firmware
 * @return 0 on success
 */
int kmdw_dfu_update_ncpu(void);

/**
 * @brief Update model
 * @param[in] info_size fw_info.bin size
 * @param[in] model_size all_models.bin size
 * @return 0 on success
 */
int kmdw_dfu_update_model(uint32_t info_size, uint32_t model_size);

/**
 * @brief Update spl
 * @param[in] size spl size
 * @return 0 on success
 */
int kmdw_dfu_update_spl(uint32_t size);

/**
 * @brief Switch active partition
 * @param[in] partition active partition
 * @return 0 on success
 */
int kmdw_dfu_switch_active_partition(uint32_t partition);

/**
 * @brief get active SCPU partition ID
 * @return
 *   0 - partition 0
 *   1 - partition 1
 *  -1 - error condition (2 active partitions)
 */
int kmdw_dfu_get_active_scpu_partition(void);

/**
 * @brief get active NCPU partition ID
 * @return
 *     0 - partition 0
 *     1 - partition 1
 *    -1 - error condition (2 active partitions)
 */
int kmdw_dfu_get_active_ncpu_partition(void);

#endif
