/**
 * @file        kdrv_ncpu.h
 * @brief       Kneron NCPU driver
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_NPU_H_
#define __KDRV_NPU_H_

#include "cmsis_os2.h"
#include "ipc.h"

/**
 * @brief       Function pointer of IPC callback
 * @param[in]   ipc_idx   Index of IPC
 * @param[in]   state     State of IPC
 */
typedef void (*ipc_handler_t)(int ipc_idx, int state);

/**
 * @brief       Initialize NPU functionality
 * @param[in]   ipc_handler IPC callback
 */
void kdrv_ncpu_initialize(ipc_handler_t ipc_handler);

/**
 * @brief       Set model information
 * @param[in]   model_info_addr model information address
 * @param[in]   info_idx information index
 * @param[in]   slot_idx slot index
 */
void kdrv_ncpu_set_model(struct kdp_model_s *model_info_addr, uint32_t info_idx, int32_t slot_idx);

/**
 * @brief       Get available COM
 * @return      COM index
 */
int kdrv_ncpu_get_avail_com(void);

/**
 * @brief       Set active model index
 * @param[in]   ipc_idx IPC index
 * @param[in]   index model slot index
 */
void kdrv_ncpu_set_model_active(int ipc_idx, uint32_t index);

/**
 * @brief       Set active image index
 * @param[in]   index image index
 * @return      available COM
 */
int kdrv_ncpu_set_image_active(uint32_t index);

/**
 * @brief       Set SCPU debug level
 * @param[in]   lvl level
 */
void kdrv_ncpu_set_scpu_debug_lvl(uint32_t lvl);

/**
 * @brief       Set NCPU debug level
 * @param[in]   lvl level
 */
void kdrv_ncpu_set_ncpu_debug_lvl(uint32_t lvl);

/**
 * @brief       Trigger NCPU interrupt
 * @param[in]   ipc_idx IPC channel to trigger
 */
void kdrv_ncpu_trigger_int(int ipc_idx);

typedef struct ncpu_to_scpu_s ncpu_to_scpu_t;
/**
 * @brief       Get ncpu_to_scpu_s
 * @return      Pointer to struct struct ncpu_to_scpu_s
 */
ncpu_to_scpu_t* kdrv_ncpu_get_input(void);


typedef struct scpu_to_ncpu_s scpu_to_ncpu_t;
/**
 * @brief       Get scpu_to_ncpu_s
 * @return      Pointer to struct scpu_to_ncpu_s
 */
scpu_to_ncpu_t* kdrv_ncpu_get_output(void);

#endif

