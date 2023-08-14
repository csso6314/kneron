/*
 * Kneron NPU driver for KDP520
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */

#include <string.h>
#include "io.h"
#include "kmdw_memory.h"
#include "ipc.h"
#include "scu_ipc.h"
#include "kdrv_ncpu.h"
#include "kmdw_console.h"
#include "base.h"
#include "power_manager.h"
#include "kdrv_cmsis_core.h"

#define IRQ_QUEUE_SIZE (5*2)

#define NCPU_IRQ              51

#define INPROC_ARRAY_MAX_SIZE 1000   /* 1000 uint32*/
#define IMAGE_PREPROCESS_BUF_SIZE 0x200000

ipc_handler_t   ipc_handler_cb;

struct scpu_to_ncpu_s *s_out_comm;
struct ncpu_to_scpu_s *s_in_comm;

#define CMD_FLAGS(i)                (s_out_comm->cmd_flags[i])
#define CMD_FLAGS_IS_ACTIVE(i)      (s_out_comm->cmd_flags[i] == IMAGE_STATE_ACTIVE)
#define CMD_FLAGS_IS_INACTIVE(i)    (s_out_comm->cmd_flags[i] == IMAGE_STATE_INACTIVE)
#define CMD_STATUS(i)               (s_in_comm->cmd_status[i])
#define CMD_STATUS_IS_NPU_BUSY(i)   (s_in_comm->cmd_status[i] == IMAGE_STATE_NPU_BUSY)
#define CMD_STATUS_IS_NPU_DONE(i)   (s_in_comm->cmd_status[i] == IMAGE_STATE_NPU_DONE)
#define CMD_STATUS_IS_PROC_DONE(i)  (s_in_comm->cmd_status[i] == IMAGE_STATE_POST_PROCESSING_DONE)

void kdrv_ncpu_trigger_int(int to_trigger)
{
    CMD_FLAGS(to_trigger) = IMAGE_STATE_ACTIVE;

check_status:
    // check if npu is busy
    if (CMD_STATUS_IS_NPU_BUSY(to_trigger)) {
        err_msg("kdrv_ncpu: npu busy: [%d] %d/%d ?\n", to_trigger,
            CMD_FLAGS(to_trigger), CMD_STATUS(to_trigger));

        osDelay(5);
        goto check_status;
    }

    s_out_comm->cmd = CMD_RUN_NPU + to_trigger;

    scu_ipc_trigger_to_ncpu_int();
}

static void NCPU_IRQHandler(void)
{
    int en_q, de_q, irq, ipc_idx;

    // kp inference debug code
    if(s_in_comm->kp_dbg_status == 0xAA)
    {
        scu_ipc_clear_from_ncpu_int();
        NVIC_ClearPendingIRQ((IRQn_Type)NCPU_IRQ);

        ipc_handler_cb(0x999, 0x0);
        return;
    }

handler_entry:
    en_q = s_in_comm->irq_enqueued;
    de_q = s_in_comm->irq_dequeued;
    while (de_q != en_q) {
        irq = s_in_comm->irq_queue[de_q];
        if (irq < IRQ_BASE) {
            err_msg("SCPU IRQ queue[%d] %d ?\n", de_q, irq);
            s_in_comm->irq_queue[de_q] = -1;
            de_q++;
            break;
        }

        irq -= IRQ_BASE;
        ipc_idx = irq / IRQ_PER_IMG;
        irq %= IRQ_PER_IMG;

        if (ipc_idx >= IPC_IMAGE_ACTIVE_MAX) {
            err_msg("SCPU IRQ queue[%d] ipc_idx %d? irq %d\n", de_q, ipc_idx, irq);
            s_in_comm->irq_queue[de_q] = -2;
            de_q++;
            break;
        }

        if (CMD_FLAGS_IS_ACTIVE(ipc_idx)) {
            if (irq == IRQ_POST_PROC_DONE) {
                CMD_FLAGS(ipc_idx) = IMAGE_STATE_RECEIVING;
                ipc_handler_cb(ipc_idx, CMD_FLAGS(ipc_idx));
                CMD_FLAGS(ipc_idx) = IMAGE_STATE_INACTIVE;
            } else if (irq == IRQ_NPU_DONE) {
                ipc_handler_cb(ipc_idx, CMD_FLAGS(ipc_idx));
            } else {
                err_msg("SCPU IRQ queue[%d] ipc_idx %d irq %d?\n", de_q, ipc_idx, irq);
                s_in_comm->irq_queue[de_q] = -3;
                de_q++;
                break;
            }

            s_in_comm->irq_queue[de_q] = 0;
            de_q++;
            de_q %= IRQ_QUEUE_SIZE;
        } else {
            err_msg("SCPU: wrong status: %d: [0] %d/%d, [1] %d/%d\n",
                s_in_comm->status,
                CMD_FLAGS(0), CMD_STATUS(0),
                CMD_FLAGS(1), CMD_STATUS(1));
            err_msg("SCPU IRQ queue[%d] ipc_idx %d? irq %d\n", de_q, ipc_idx, irq);

            s_in_comm->irq_queue[de_q] = -4;
            de_q++;
            break;
        }
    }

    s_in_comm->irq_dequeued = de_q % IRQ_QUEUE_SIZE;

    scu_ipc_clear_from_ncpu_int();
    NVIC_ClearPendingIRQ((IRQn_Type)NCPU_IRQ);

    // Re-check for new IRQ
    if (en_q != s_in_comm->irq_enqueued)
        goto handler_entry;

    if (s_in_comm->status == STATUS_ERR) {
        power_manager_reset();
    }
}

/* ############################
 * ##    Public Functions    ##
 * ############################ */

int kdrv_ncpu_get_avail_com(void)
{
    if (CMD_FLAGS_IS_INACTIVE(0))
        return 0;
    else if (CMD_FLAGS_IS_INACTIVE(1))
        return 1;
    else
        return -1;
}

int kdrv_ncpu_set_image_active(uint32_t n_index)
{
    int rc;

    rc = kdrv_ncpu_get_avail_com();
    if (rc != -1) {
        s_out_comm->active_img_index[rc] = n_index;
    } else {
        dbg_msg("[NO AVAIL] set_image_active: [0] %d/%d, [1] %d/%d\n",
                CMD_FLAGS(0), CMD_STATUS(0),
                CMD_FLAGS(1), CMD_STATUS(1));
    }

    return rc;
}

void kdrv_ncpu_set_model_active(int ipc_idx, uint32_t n_index)
{
    s_out_comm->model_slot_index[ipc_idx] = n_index;
}

void kdrv_ncpu_set_model(struct kdp_model_s *model_info_addr, uint32_t info_idx, int32_t slot_idx)
{
    if (slot_idx >= MULTI_MODEL_MAX) {
        dbg_msg("[ERR] too many active model is set\n");
        return;
    }

    struct kdp_model_s * info = model_info_addr + info_idx;
    s_out_comm->models[slot_idx] = *(info);
    s_out_comm->models_type[slot_idx] = info->model_type;

    dbg_msg("[%s] idx [%u], slot [%d]\n"
            "in addr [0x%x], len [%u], out addr [0x%x], len [%u]\n",
            __func__, info_idx, slot_idx,
            info->input_mem_addr, info->input_mem_len, info->output_mem_addr, info->output_mem_len);

    dbg_msg("buf addr [0x%x], len [%u], cmd addr [0x%x], len [%u]\n"
            "weight addr [0x%x], len [%u], setup addr [0x%x], len [%u]\n",
            info->buf_addr, info->buf_len, info->cmd_mem_addr, info->cmd_mem_len,
            info->weight_mem_addr, info->weight_mem_len, info->setup_mem_addr, info->setup_mem_len);

    if (slot_idx + 1 > s_out_comm->num_models) {
        s_out_comm->num_models = slot_idx + 1;
    }
}

void kdrv_ncpu_initialize(ipc_handler_t ipc_handler)
{
    s_out_comm = (struct scpu_to_ncpu_s *)IPC_MEM_ADDR;
    s_in_comm = (struct ncpu_to_scpu_s *)IPC_MEM_ADDR2;

    memset(s_out_comm, 0, sizeof(struct scpu_to_ncpu_s));
    memset(s_in_comm, 0, sizeof(struct ncpu_to_scpu_s));

    s_out_comm->id = SCPU2NCPU_ID;
    s_out_comm->input_count = 0;
    s_out_comm->kp_dbg_checkpoinots = 0x0; // kp inference debug code

    kmdw_console_set_log_level_scpu(LOG_INFO);
    kmdw_console_set_log_level_ncpu(LOG_ERROR);

    NVIC_SetVector((IRQn_Type)NCPU_IRQ, (uint32_t)NCPU_IRQHandler);
    NVIC_EnableIRQ((IRQn_Type)NCPU_IRQ);

    uint32_t mem_len2 = IMAGE_PREPROCESS_BUF_SIZE;
    uint8_t *mem_addr2 = (uint8_t*)kmdw_ddr_reserve(sizeof(uint8_t)*mem_len2);
    kdrv_ncpu_get_output()->input_mem_addr2 = (uint32_t)mem_addr2;
    kdrv_ncpu_get_output()->input_mem_len2 = mem_len2;

    //allocate the space for inproc array
    uint8_t *mem_addr3 = (uint8_t*)kmdw_ddr_reserve(sizeof(uint32_t)*INPROC_ARRAY_MAX_SIZE);
    kdrv_ncpu_get_output()->inproc_mem_addr = (uint32_t)mem_addr3;

    ipc_handler_cb = ipc_handler;

    scu_ipc_enable_to_ncpu_int();
}

struct ncpu_to_scpu_s* kdrv_ncpu_get_input(void)
{
    return s_in_comm;
}

struct scpu_to_ncpu_s* kdrv_ncpu_get_output(void)
{
    return s_out_comm;
}

void kdrv_ncpu_set_scpu_debug_lvl(uint32_t lvl)
{
    s_out_comm->debug_flags = (s_out_comm->debug_flags & ~0x000F0000) | (((lvl) << 16) & 0x000F0000);
}

void kdrv_ncpu_set_ncpu_debug_lvl(uint32_t lvl)
{
    s_out_comm->debug_flags = (s_out_comm->debug_flags & ~0x0000000F) | ((lvl)&0x0000000F);
}

