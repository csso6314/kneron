/**
 * @file        kmdw_sbt.h
 * @brief       Kneron Secure boot APIs
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_SBT_H__
#define __KMDW_SBT_H__

#include <stdbool.h>
#include <stdint.h>
#include "kdrv_status.h"

/**
 * @brief firmware type
 */
typedef enum {
    FW_SCPU = 0,   /**< fw_scpu */
    FW_NCPU,       /**< fw_ncpu */
} fw_type_e;

/**
 * @brief kmdw_sbt_get_boot_flag get efuse secure boot flag
 *
 * @return bool
 */
bool kmdw_sbt_get_boot_flag(void);

/** 
 * @brief kmdw_sbt_flash_fw_loader secure firmware loader
 *
 * @param[in]   flash_addr          flash address of firmware
 * @param[in]   cpu                 cpu type, ref @fw_type_e
 * @param[in]   swap_buf            swap buffer for decrypt process
 * @param[in]   swap_buf_size       swap buffer size
 * @return kdrv_status_t
 */
kdrv_status_t kmdw_sbt_flash_fw_loader(uint32_t flash_addr, fw_type_e cpu, uint32_t *swap_buf, uint32_t swap_buf_size);

#endif
