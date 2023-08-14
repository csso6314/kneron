/*
 * Kneron Model API Manager
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

/***************** JPEG enc functions ***********/
#include <string.h>
#include "kmdw_jpeg.h"

jpeg_enc_kmdw_ctx_t jpeg_enc_kmdw_ctx = {0};
jpeg_dec_kmdw_ctx_t jpeg_dec_kmdw_ctx = {0};

int get_yuv_y_len(jpeg_fmt_t fmt, int w, int h)
{
    int len;
    switch(fmt) {
    case JPEG_YUV_420:
    case JPEG_YUV_422:
    case JPEG_YUV_444:
        len = w*h;
        break;

	default:
        len = w*h;
        break;

    }
    return len;
}

int get_yuv_u_len(jpeg_fmt_t fmt, int w, int h)
{
    int len;
    switch(fmt) {
    case JPEG_YUV_420:
    case JPEG_YUV_422:
        len = (w/2) * (h/2);
        break;

    case JPEG_YUV_444:
        len = w * h;
        break;

	default:
        len = (w/2) * (h/2);
        break;

    }
    return len;

}

int get_yuv_v_len(jpeg_fmt_t fmt, int w, int h)
{
    int len;
    switch(fmt) {
    case JPEG_YUV_420:
    case JPEG_YUV_422:
        len = (w/2) * (h/2);
        break;

    case JPEG_YUV_444:
        len = w * h;
        break;

	default:
        len = (w/2) * (h/2);
        break;

    }
    return len;

}

void model_jpeg_enc_cfg(uint32_t img_seq, uint32_t jpeg_in_buf_addr, uint32_t jpeg_in_size, uint32_t jpeg_out_buf_addr,
                        uint32_t exec_rlt_addr, uint32_t w, uint32_t h, uint32_t quality, uint32_t fmt, osEventFlagsId_t *pEvt, uint32_t evt_flag)
{
    jpeg_encode_params_t *pEnc;
    scpu_to_ncpu_t *p_out_ipc;

    jpeg_enc_kmdw_ctx.jpeg_enc_evt_id = *pEvt;
    jpeg_enc_kmdw_ctx.jpeg_enc_evt_flag = evt_flag;
    jpeg_enc_kmdw_ctx.p_ipc_out = kmdw_ipc_get_output();
    jpeg_enc_kmdw_ctx.p_ipc_in = kmdw_ipc_get_input();

    p_out_ipc = (scpu_to_ncpu_t *)jpeg_enc_kmdw_ctx.p_ipc_out;

    pEnc = (jpeg_encode_params_t *)p_out_ipc->pExtInParam;
    p_out_ipc->nLenExtInParam = sizeof (jpeg_encode_params_t);

    p_out_ipc->pExtOutRslt = (void *)exec_rlt_addr;
    p_out_ipc->nLenExtOutRslt = sizeof (jpeg_encode_result_t);

    memset((void *)pEnc, 0, sizeof (jpeg_encode_params_t));
    pEnc->frame_seq = img_seq;
    pEnc->rotation = 0;
    pEnc->quality = quality;
    pEnc->src_buf.fmt = (jpeg_fmt_t)fmt;
    pEnc->src_buf.width = w;
    pEnc->src_buf.height = h;
    pEnc->src_buf.yBuf = (uint8_t *)jpeg_in_buf_addr;
    pEnc->src_buf.uvBuf = pEnc->src_buf.yBuf +
                          get_yuv_y_len(pEnc->src_buf.fmt, pEnc->src_buf.width, pEnc->src_buf.height);
    pEnc->dst_buf = jpeg_out_buf_addr;
    pEnc->dst_buf_size = w * h * 2;  // estimated max size

    p_out_ipc->cmd = CMD_JPEG_ENCODE;

}

void model_jpeg_enc_start_run(void)
{
    //trigger ncpu/npu
    kmdw_ipc_trigger_int(CMD_JPEG_ENCODE);
}

jpeg_encode_result_t* model_jpeg_enc_get_rslt()
{
    return (jpeg_encode_result_t*)&jpeg_enc_kmdw_ctx.npu_rslt;
}

/***************** JPEG dec functions ***********/

void model_jpeg_dec_cfg(uint32_t img_seq, uint32_t jpeg_in_buf_addr, uint32_t jpeg_in_size, uint32_t yuv_out_buf_addr,
                        uint32_t exec_rlt_addr, uint32_t w, uint32_t h, uint32_t fmt, osEventFlagsId_t *pEvt, uint32_t evt_flag)
{
    jpeg_decode_params_t *pDec;
    scpu_to_ncpu_t *p_out_ipc;

    jpeg_dec_kmdw_ctx.jpeg_dec_evt_id = *pEvt;
    jpeg_dec_kmdw_ctx.jpeg_dec_evt_flag = evt_flag;
    jpeg_dec_kmdw_ctx.p_ipc_out = kmdw_ipc_get_output();
    jpeg_dec_kmdw_ctx.p_ipc_in = kmdw_ipc_get_input();

    p_out_ipc = (scpu_to_ncpu_t *)jpeg_dec_kmdw_ctx.p_ipc_out;

    pDec = (jpeg_decode_params_t *)p_out_ipc->pExtInParam;
    p_out_ipc->nLenExtInParam = sizeof (jpeg_decode_params_t);

    p_out_ipc->pExtOutRslt = (void *)exec_rlt_addr;
    p_out_ipc->nLenExtOutRslt = sizeof (jpeg_decode_result_t);

    memset((void *)pDec, 0, sizeof (jpeg_decode_params_t));

    pDec->src_buf.buf_addr = jpeg_in_buf_addr;
    pDec->src_buf.buf_filled_len = jpeg_in_size;
    pDec->out_fmt = (jpeg_fmt_t)fmt;
    pDec->width = w;
    pDec->height = h;

    pDec->dst_yuv_buf = yuv_out_buf_addr;
    pDec->dst_buf_size = w * h * 2;  // estimated max size, optional

    pDec->restart = 0;

    /*  gif buf for validation, not for product */
    pDec->ftr.gen_gif = false;
    pDec->gif_buf = NULL;

    pDec->frame_seq = img_seq;

    p_out_ipc->cmd = CMD_JPEG_DECODE;

}

void model_jpeg_dec_start_run(void)
{
    //trigger ncpu/npu
    kmdw_ipc_trigger_int(CMD_JPEG_DECODE);
}

jpeg_decode_result_t* model_jpeg_dec_get_rslt()
{
    return (jpeg_decode_result_t*)&jpeg_dec_kmdw_ctx.npu_rslt;
}
