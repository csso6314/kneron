/*
 * Kneron NPU driver for KDP520
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */

#include <string.h>
#include "base.h"
#include "kdrv_cmsis_core.h"
#include "scu_ipc.h"
#include "kdrv_ipc.h"
#include "kmdw_ipc.h"
#include "kmdw_console.h"

#ifdef _BOARD_SN720HAPS_H_
#define NCPU_IRQ              9
#else
#define NCPU_IRQ              SGI_IRQn
#endif

#define CPU_NODE_WORKING_BUFF_SIZE  5 * 1024 * 1024

ipc_handler_t   ipc_handler_cb;

scpu_to_ncpu_t *out_comm_p;
ncpu_to_scpu_result_t *in_comm_p;
extern osThreadId_t logger_tid;
extern logger_mgt_t logger_mgt;

static boolean bDSPBusy;


/**
 * @brief Weak function for user to implement the specific functions\n
 * Ex: Start to flash LED or start to evaluate NCPU running time
 * @return IPC struct
 */
__WEAK void hook_ncpu_start(void)
{

}

/**
 * @brief Weak function for user to implement the specific functions\n
 * Ex: Stop to flash LED or stop to evaluate NCPU running time
 * @return IPC struct
 */
__WEAK void hook_ncpu_stop(void)
{

}

void kmdw_ipc_trigger_int(int ipc_cmd)
{
    while(bDSPBusy == true) {
        osDelay(10);
    }
    hook_ncpu_start();

    if(ipc_cmd == CMD_RUN_NPU) {
        ncpu_to_scpu_req_img_t *req_img_p = &in_comm_p->req_img;

        dbg_msg("<IPC->DSP> %d, %d (sts %d)\n", in_comm_p->bHandledByScpu, req_img_p->bHandledByScpu,
                in_comm_p->sts);
    } else {
        // standalone cases will not require parallel processing
        ncpu_to_scpu_req_img_t *req_img_p = &in_comm_p->req_img;
        req_img_p->bHandledByScpu = true;
    }

    out_comm_p->cmd = ipc_cmd;

    out_comm_p->bNcpuReceived = 0;
    kdrv_ipc_trigger_to_ncpu_int();
}

static void NCPU_IRQHandler(void)
{
    ncpu_to_scpu_req_img_t *req_img_p = &in_comm_p->req_img;
    struct kdp_img_raw_s *p_raw_image;

    // kp inference debug code
    if(in_comm_p->kp_dbg_status == 0xAA) // FIXME: workaround for dbg flow
    {
        ipc_handler_cb(p_raw_image, 0x999);
        goto irq_out;
    }

    if(in_comm_p->print_log) {
        osThreadFlagsSet(logger_tid, FLAG_LOGGER_NCPU_IN);
        in_comm_p->print_log = false;
        goto irq_out;
    }

    if (in_comm_p->sts != NCPU_STS_READY) {
        ipc_err_msg("Note: DSP IRQ (%d, %d) %d %d %d\n", in_comm_p->bHandledByScpu, req_img_p->bHandledByScpu,
            in_comm_p->sts, in_comm_p->ipc_type, in_comm_p->out_type);
		bDSPBusy = true;
		goto irq_out;
    }
    bDSPBusy = false;

handle_irqs:
    if (!in_comm_p->bHandledByScpu) {

        if((in_comm_p->out_type < NCPU_POSTPROC_RESULT) || (in_comm_p->out_type >= NCPU_RESULT_TYPE_MAX)){
            ipc_err_msg("[DSP IRQ: wrong out_type] (%d, %d) %d %d %d\n", in_comm_p->bHandledByScpu, req_img_p->bHandledByScpu,
                in_comm_p->sts, in_comm_p->ipc_type, in_comm_p->out_type);
            goto irq_out;
        }

        if(in_comm_p->out_type == NCPU_POSTPROC_RESULT) {
            p_raw_image = (struct kdp_img_raw_s *)in_comm_p->result.postproc.OrigRawImgAddr;
            ipc_handler_cb(p_raw_image, IMAGE_STATE_RECEIVING);
        }

        if(in_comm_p->out_type == NCPU_JPEG_ENC_RESULT) {
            ipc_handler_cb(p_raw_image, IMAGE_STATE_JPEG_ENC_DONE);
        }

        if(in_comm_p->out_type == NCPU_JPEG_DEC_RESULT) {
            ipc_handler_cb(p_raw_image, IMAGE_STATE_JPEG_DEC_DONE);
        }

		if(in_comm_p->out_type == NCPU_TOF_DEC_RESULT) {
            ipc_handler_cb(p_raw_image, IMAGE_STATE_TOF_DEC_DONE);
        }

        if(in_comm_p->out_type == NCPU_IR_BRIGHT_RESULT) {
            ipc_handler_cb(p_raw_image, IMAGE_STATE_TOF_CALC_IR_BRIGHT_DONE);
        }

        in_comm_p->bHandledByScpu = true;
    }

    if (!req_img_p->bHandledByScpu) {
        req_img_p->bHandledByScpu = true;
        ipc_handler_cb(NULL, IMAGE_STATE_ACTIVE);
    }

irq_out:
    hook_ncpu_stop();
    kdrv_ipc_clear_from_ncpu_int();
    NVIC_ClearPendingIRQ((IRQn_Type)NCPU_IRQ);

    if((!in_comm_p->bHandledByScpu || !req_img_p->bHandledByScpu) && (in_comm_p->out_type == NCPU_POSTPROC_RESULT)){
        ipc_err_msg("[dsp irq] (%d, %d) %d %d %d\n", in_comm_p->bHandledByScpu, req_img_p->bHandledByScpu,
            in_comm_p->sts, in_comm_p->ipc_type, in_comm_p->out_type);
        goto handle_irqs;
    }
}

/* ############################
 * ##    Public Functions    ##
 * ############################ */

void kmdw_ipc_set_image_active(uint32_t n_index)
{
    out_comm_p->pRawimg = &out_comm_p->raw_images[n_index];
}

void kmdw_ipc_set_model_active(uint32_t n_index)
{
    out_comm_p->model_slot_index = n_index;
}

void kmdw_ipc_set_model(struct kdp_model_s *model_info_addr, uint32_t info_idx, int32_t slot_idx)
{
    if (slot_idx >= MULTI_MODEL_MAX) {
        dbg_msg("[ERR] too many active model is set\n");
        return;
    }

    struct kdp_model_s * info = model_info_addr + info_idx;
    out_comm_p->models[slot_idx] = *(info);
    out_comm_p->models_type[slot_idx] = info->model_type;

    dbg_msg("[%s] idx [%u], slot [%d]\n"
            "in addr [0x%x], len [%u], out addr [0x%x], len [%u]\n",
            __func__, info_idx, slot_idx,
            info->input_mem_addr, info->input_mem_len, info->output_mem_addr, info->output_mem_len);

    dbg_msg("buf addr [0x%x], len [%u], cmd addr [0x%x], len [%u]\n"
            "weight addr [0x%x], len [%u], setup addr [0x%x], len [%u]\n",
            info->buf_addr, info->buf_len, info->cmd_mem_addr, info->cmd_mem_len,
            info->weight_mem_addr, info->weight_mem_len, info->setup_mem_addr, info->setup_mem_len);

    if (slot_idx + 1 > out_comm_p->num_models) {
        out_comm_p->num_models = slot_idx + 1;
    }
}

void kmdw_ipc_initialize(ipc_handler_t ipc_handler)
{
    uint32_t nExtBufInSize, nExtBufOutSize;

    out_comm_p = (scpu_to_ncpu_t *)SCPU_IPC_MEM_ADDR;
    in_comm_p = (ncpu_to_scpu_result_t *)SCPU_IPC_MEM_ADDR2;

    memset(out_comm_p, 0, sizeof(scpu_to_ncpu_t));
    memset(in_comm_p, 0, sizeof(ncpu_to_scpu_result_t));

    out_comm_p->id = SCPU2NCPU_ID;
    in_comm_p->id  = NCPU2SCPU_ID;

    kmdw_console_set_log_level_scpu(LOG_ERROR);
    kmdw_console_set_log_level_ncpu(LOG_ERROR);

    NVIC_SetVector((IRQn_Type)NCPU_IRQ, (uint32_t)NCPU_IRQHandler);
    NVIC_EnableIRQ((IRQn_Type)NCPU_IRQ);

    uint32_t mem_len2 = 0x100000;
    uint8_t *mem_addr2 = (uint8_t*)kmdw_ddr_reserve(sizeof(uint8_t)*mem_len2);
    if(mem_addr2 == 0) {
        critical_msg("Error: failed to malloc IPC mem_addr2\n");
        return;
    }
    out_comm_p->input_mem_addr2 = (uint32_t)mem_addr2;
    out_comm_p->input_mem_len2 = mem_len2;
    out_comm_p->ncpu_img_req_msg_addr = (uint32_t)&in_comm_p->req_img;
    out_comm_p->kp_dbg_checkpoinots = 0x0;

    /* for jpeg enc/dec or tof dec use */
    nExtBufInSize = MAX(sizeof(tof_decode_params_t), MAX(sizeof(jpeg_encode_params_t), sizeof(jpeg_decode_params_t)));
    nExtBufOutSize = MAX(sizeof(tof_decode_result_t), MAX(sizeof(jpeg_encode_result_t), sizeof(jpeg_decode_result_t)));
    out_comm_p->pExtInParam = (void *)kmdw_ddr_reserve(nExtBufInSize);
    if(out_comm_p->pExtInParam == 0) {
        critical_msg("Error: failed to malloc IPC out_comm_p->pExtInParam\n");
        return;
    }
    out_comm_p->pExtOutRslt = (void *)kmdw_ddr_reserve(nExtBufOutSize);
    if(out_comm_p->pExtOutRslt == 0) {
        critical_msg("Error: failed to malloc IPC out_comm_p->pExtOutRslt\n");
        return;
    }

    out_comm_p->nLenCpuNodeBuffer = CPU_NODE_WORKING_BUFF_SIZE;
    out_comm_p->pCpuNodeBuffer = (void *)kmdw_ddr_reserve(out_comm_p->nLenCpuNodeBuffer);
    if (NULL == out_comm_p->pCpuNodeBuffer) {
        critical_msg("Error: failed to malloc IPC out_comm_p->pCpuNodeBuffer\n");
        out_comm_p->nLenCpuNodeBuffer = 0;
        return;
    }

    ipc_handler_cb = ipc_handler;
    bDSPBusy = false;

    in_comm_p->p_log_buf_base = (uint8_t *)&logger_mgt;

    kdrv_ipc_enable_to_ncpu_int();
}

struct scpu_to_ncpu_s* kmdw_ipc_get_output(void)
{
    return out_comm_p;
}

ncpu_to_scpu_result_t* kmdw_ipc_get_input(void)
{
    return in_comm_p;
}

void kdrv_ncpu_set_scpu_debug_lvl(uint32_t lvl)
{
    out_comm_p->debug_flags = (out_comm_p->debug_flags & ~0x000F0000) | (((lvl) << 16) & 0x000F0000);
}

void kdrv_ncpu_set_ncpu_debug_lvl(uint32_t lvl)
{
    out_comm_p->debug_flags = (out_comm_p->debug_flags & ~0x0000000F) | ((lvl)&0x0000000F);
}
