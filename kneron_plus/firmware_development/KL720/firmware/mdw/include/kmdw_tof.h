/**
 * @file        kmdw_tof.h
 * @brief       structure for the tof related functions
 *
 * @copyright   Copyright (c) 2019-2022 Kneron Inc. All rights reserved.
 */

#include "base.h"
#include "kmdw_ipc.h"

typedef struct model_tof_dec_ctx_s {
    scpu_to_ncpu_t*         p_ipc_out;
    ncpu_to_scpu_result_t*  p_ipc_in;
    osEventFlagsId_t        tof_dec_evt_id;
    uint32_t                tof_dec_evt_flag;
    tof_decode_result_t     npu_rslt;
} tof_dec_kmdw_ctx_t;

typedef struct {
    scpu_to_ncpu_t*         p_ipc_out;
    ncpu_to_scpu_result_t*  p_ipc_in;
    osEventFlagsId_t        ir_bright_evt_id;
    uint32_t                ir_bright_evt_flag;
    ir_bright_result_t      npu_rslt;
} ir_bright_kmdw_ctx_t;

void calculate_gamma_table(int16_t *gamma_table);

void kmdw_tof_config_init(void);

int kmdw_tof_decode(uint32_t tof_in_buf_addr, uint32_t tof_in_len, uint32_t out_depth_buf_addr, uint32_t out_ir_buf_addr);

tof_decode_result_t* model_tof_dec_get_rslt(void);

void kmdw_tof_ir_bright_init(uint32_t src_addr, uint32_t src_w, uint32_t src_h);
int32_t kmdw_tof_ir_bright_aec(uint32_t nir_output_buf, int32_t src_width, uint32_t src_height, bounding_box_t *single_tof_box, bool calc_full);

ir_bright_result_t* model_ir_bright_get_rslt(void);
