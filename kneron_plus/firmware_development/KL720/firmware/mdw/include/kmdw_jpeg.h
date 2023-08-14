
/**
 * @file        kmdw_jpeg.h
 * @brief       structure for the tof related functions
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */

#include "base.h"
#include "kmdw_ipc.h"

typedef struct model_jpeg_enc_ctx_s {
    scpu_to_ncpu_t*         p_ipc_out;
    ncpu_to_scpu_result_t*  p_ipc_in;
    osEventFlagsId_t        jpeg_enc_evt_id;
    uint32_t                jpeg_enc_evt_flag;
    jpeg_encode_result_t    npu_rslt;
} jpeg_enc_kmdw_ctx_t;

typedef struct model_jpeg_dec_ctx_s {
    scpu_to_ncpu_t*         p_ipc_out;
    ncpu_to_scpu_result_t*  p_ipc_in;
    osEventFlagsId_t        jpeg_dec_evt_id;
    uint32_t                jpeg_dec_evt_flag;
    jpeg_decode_result_t    npu_rslt;
} jpeg_dec_kmdw_ctx_t;

void model_jpeg_enc_cfg(uint32_t img_seq, uint32_t jpeg_in_buf_addr, uint32_t jpeg_in_size, uint32_t jpeg_out_buf_addr,
                        uint32_t exec_rslt_addr, uint32_t w, uint32_t h, uint32_t quality, uint32_t fmt, osEventFlagsId_t *pEvt, uint32_t evt_flag);

void model_jpeg_enc_start_run(void);
jpeg_encode_result_t* model_jpeg_enc_get_rslt(void);

void model_jpeg_dec_cfg(uint32_t img_seq, uint32_t jpeg_in_buf_addr, uint32_t jpeg_in_size, uint32_t jpeg_out_buf_addr,
                        uint32_t exec_rslt_addr, uint32_t w, uint32_t h, uint32_t fmt, osEventFlagsId_t *pEvt, uint32_t evt_flag);

void model_jpeg_dec_start_run(void);
jpeg_decode_result_t* model_jpeg_dec_get_rslt(void);

int get_yuv_y_len(jpeg_fmt_t fmt, int w, int h);
int get_yuv_u_len(jpeg_fmt_t fmt, int w, int h);
int get_yuv_v_len(jpeg_fmt_t fmt, int w, int h);
