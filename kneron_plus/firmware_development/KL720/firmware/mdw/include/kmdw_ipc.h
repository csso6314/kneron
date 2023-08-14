/**
 * @file        kmdw_ipc.h
 * @brief       IPC APIs
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_IPC_H_
#define __KMDW_IPC_H_

#include "cmsis_os2.h"
#include "ipc.h"

typedef void (*ipc_handler_t)(struct kdp_img_raw_s *p_raw_image, int state);

/**
 * @brief Initialize NPU functionality
 * @param ipc_handler IPC callback
 */
void kmdw_ipc_initialize(ipc_handler_t ipc_handler);

/**
 * @brief Set model information
 * @param model_info_addr model information address
 * @param info_idx information index
 * @param slot_idx slot index
 */
void kmdw_ipc_set_model(struct kdp_model_s *model_info_addr, uint32_t info_idx, int32_t slot_idx);

/**
 * @brief Set active model index
 * @param index model slot index
 */
void kmdw_ipc_set_model_active(uint32_t index);

/**
 * @brief Set active image index
 * @param index image index
 */
void kmdw_ipc_set_image_active(uint32_t index);

/**
 * @brief Set SCPU debug level
 * @param lvl level
 */
void kdrv_ncpu_set_scpu_debug_lvl(uint32_t lvl);

/**
 * @brief Set NCPU debug level
 * @param lvl level
 */
void kdrv_ncpu_set_ncpu_debug_lvl(uint32_t lvl);

/**
 * @brief Trigger NCPU interrupt
 * @param ipc_idx IPC channel to trigger
 */
void kmdw_ipc_trigger_int(int ipc_cmd);

/**
 * @brief Get scpu_to_ncpu_t point
 * @return IPC struct
 */
scpu_to_ncpu_t* kmdw_ipc_get_output(void);

/**
 * @brief Get ncpu_to_scpu_result_t point
 * @return IPC struct
 */
ncpu_to_scpu_result_t* kmdw_ipc_get_input(void);


#endif
