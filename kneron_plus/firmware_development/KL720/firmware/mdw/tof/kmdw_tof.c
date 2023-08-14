/*
 * Kneron ToF API 
 *
 * Copyright (C) 2022 Kneron, Inc. All rights reserved.
 *
 */

//#include <stdlib.h>
#include <string.h>
#include "project.h"      /*for DDR_MEM_TOF_GAMMA_TABLE*/
#include "kmdw_tof.h"
#include "kmdw_console.h" /*for dbg_msg */
#include "kdev_sensor.h"
#include "kmdw_memory.h"
#include "math.h"

#define GAMMA 1.0
/***************** TOF dec functions ***********/

// TOF　Working buf = input params(p1, p2, non_H) + temp depth storage
// input params = (p1)640 * 240 + (p2)640 * 240 + (non_H)4096 + (cali_table)45 + (gamma_table)256
// temp depth storage = (depth)640 * 240 + (depth_prev)640 * 240 + (depth_resize)640 * 480
#define TOF_WORKING_BUF_SIZE (640 * 240 * 2 + 640 * 480 + 640 * 240 * 2 + 4096 + 256) * sizeof(int16_t) + 45 * sizeof(float)

tof_dec_kmdw_ctx_t tof_dec_kmdw_ctx = {0};

ir_bright_kmdw_ctx_t ir_bright_kmdw_ctx = {0};

void calculate_gamma_table(int16_t *gamma_table){
    int i;
    float f;
    float fProcompensation = 1/GAMMA;
    for(i = 0; i< 256; i++){
        f = (i + 0.5F) / 256;
        f = pow(f, fProcompensation);
        gamma_table[i] = (int16_t)(f*256 - 0.5F);
    }
}

static tof_decode_result_t *dec_result;
static ir_bright_result_t *ir_bright_result;

uint32_t tof_temp_depth_buf_addr,tof_depth_out_buf_addr, tof_ir_out_buf_addr, tof_table_gen_addr, tof_lut_addr;
uint32_t tof_ir_bright_buf_addr;
const unsigned char cal_data[384]={
0x05,0x00,0x00,0x00,0x48,0x65,0x61,0x64,0x01,0x00,0x00,0x80,0x3F,0xDB,0x6B,0xA7,0x76,0x17,0x00,0x00,0x00,0x44,0x65,0x76,0x69,0x05,0x0E,0x31,0x31,0x31,0x31,0x31,
0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x00,0x04,0x01,0x00,0x00,0xA0,0x42,0x19,0xD0,0x1B,0xF9,0x56,0x00,0x00,0x00,0x43,0x61,0x6D,0x70,0x02,0x00,0x36,0xC2,
0xF8,0x6B,0x16,0x6B,0x73,0x40,0x27,0x8B,0x79,0xA1,0x6A,0x88,0x6E,0x40,0x24,0xFF,0xFF,0x7F,0x6C,0xF6,0x7F,0x40,0x24,0xFF,0xFF,0x7F,0x6C,0xF6,0x7F,0x40,0x24,0x95,
0xCB,0xBD,0x3E,0xBE,0x7E,0xBF,0x37,0x2E,0x2F,0x7E,0x6B,0xE7,0xCD,0x3F,0xB1,0x75,0x56,0x37,0x91,0x53,0xDE,0xBF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x7F,
0x22,0xA6,0x6E,0x6D,0x44,0x3F,0x34,0xBC,0xCE,0x47,0x4F,0xD1,0xF5,0x3E,0x80,0x02,0xE0,0x01,0x1E,0xBD,0x05,0x51,0x1E,0x00,0x00,0x00,0x54,0x65,0x6D,0x70,0x00,0x00,
0x00,0x00,0x03,0x3C,0x7A,0x28,0xBC,0x4F,0x46,0xB1,0x39,0x94,0xE7,0x96,0xB6,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xB8,0x47,0x6E,0x11,
0xB4,0x00,0x00,0x00,0x44,0x69,0x73,0x74,0x25,0x6B,0x3D,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0xC0,0xCF,0x38,0x3B,0xBB,0x9B,0x27,0xBC,0x5E,0x63,0x97,0xBC,0xA8,0x57,0xCA,0xBD,
0xC3,0x64,0xAA,0x3A,0x2A,0x3A,0x92,0x3C,0x8E,0x09,0x56,0x3F,0x31,0x55,0x98,0xC3,0x19,0x77,0xE1,0x40,0x68,0x87,0x00,0x41,0x3E,0xFD,0x3F,0xBD,0x60,0x2C,0x83,0xBD,
0xFC,0xCE,0xC2,0xBD,0xB1,0x69,0x46,0x39,0xB6,0x6B,0x90,0x39,0x11,0x64,0x97,0x39,0xDE,0xEA,0x0B,0x3A,0x3D,0xAF,0xE4,0xB4,0x00,0x24,0x07,0xB5,0x0A,0x2A,0x3D,0xB5,
0x24,0xF3,0x5B,0xB5,0x9F,0xA0,0xD3,0xB5,0x12,0xFF,0x0C,0x30,0x5C,0x31,0x8B,0x2F,0x65,0x6B,0xA9,0x30,0xB7,0x73,0x1D,0x30,0x3E,0xFB,0xA3,0x30,0x1D,0x73,0x25,0x31,
0x69,0xCB,0x8F,0xAA,0x76,0xF5,0xF3,0x29,0xB5,0x48,0x8E,0xAB,0x38,0x2B,0xE6,0x2A,0x84,0x5B,0xA7,0xAB,0x54,0x3D,0x4B,0xAA,0xDE,0xF3,0xDB,0xAB,0x4D,0x0F,0xC8,0x29,};

void kmdw_tof_config_init()
{
    tof_decode_params_t *pDec;
    scpu_to_ncpu_t *p_out_ipc;

    dec_result = (void *)kmdw_ddr_reserve(sizeof(tof_decode_result_t));
    if (dec_result == 0)
        kmdw_printf("error !! dec_result DDR malloc failed\n");

    tof_temp_depth_buf_addr = kmdw_ddr_reserve(TOF_TEMP_DEPTH_SIZE);
    tof_depth_out_buf_addr = kmdw_ddr_reserve(TOF_DEPTH_IMG_SIZE);
    tof_ir_out_buf_addr = kmdw_ddr_reserve(TOF_IR_IMG_SIZE);
    tof_table_gen_addr = kmdw_ddr_reserve(TOF_INIT_TABLE_SIZE);
    tof_lut_addr = kmdw_ddr_reserve(TOF_INIT_TABLE_SIZE);

    int16_t *gamma_table = (int16_t *)(DDR_MEM_TOF_GAMMA_TABLE);
    // load sensor calibration data
    // kdp_memxfer_module.flash_to_ddr(DDR_MEM_TOF_RGB_CAM_PARAMS, FLASH_TOF_BUF_ADDR, DDR_MEM_TOF_RGB_CAM_PARAMS_SIZE);
    // kmdw_printf("DDR_MEM_TOF_RGB_CAM_PARAMS: %p\n", DDR_MEM_TOF_RGB_CAM_PARAMS);
    // kmdw_printf("DDR_MEM_TOF_RGB_DEPTH: %p\n", DDR_MEM_TOF_RGB_DEPTH);
    // kdp_memxfer_module.flash_to_ddr(DDR_MEM_TOF_GOLDEN_DATA, FLASH_TOF_BUF_ADDR + DDR_MEM_TOF_RGB_CAM_PARAMS_SIZE, DDR_MEM_TOF_GOLDEN_DATA_SIZE);
    // kmdw_printf("DDR_MEM_TOF_GOLDEN_DATA: %p\n", DDR_MEM_TOF_GOLDEN_DATA);
    unsigned char *cali_table = (unsigned char*)(DDR_MEM_TOF_CALI_TABLE);
    // unsigned char real_cali[DDR_MEM_TOF_CALI_TABLE_SIZE];

#if 0
    kdev_sensor_get_calibration_data(cali_table, DDR_MEM_TOF_CALI_TABLE_SIZE/4);//&real_cali[0]
#else 
    // for (int32_t i = 0; i < DDR_MEM_TOF_CALI_TABLE_SIZE; i++)
    // {
    //     *(real_cali + i) = cal_data[i];
    // }
#endif
    memcpy(cali_table, &cal_data, DDR_MEM_TOF_CALI_TABLE_SIZE);

    calculate_gamma_table(gamma_table);

    tof_dec_kmdw_ctx.tof_dec_evt_id = osEventFlagsNew(0);
    tof_dec_kmdw_ctx.tof_dec_evt_flag = 0x100;
    tof_dec_kmdw_ctx.p_ipc_out = kmdw_ipc_get_output();
    tof_dec_kmdw_ctx.p_ipc_in = kmdw_ipc_get_input();

    p_out_ipc = (scpu_to_ncpu_t *)tof_dec_kmdw_ctx.p_ipc_out;

    pDec = (tof_decode_params_t *)p_out_ipc->pExtInParam;
    p_out_ipc->nLenExtInParam = sizeof (tof_decode_params_t);

    p_out_ipc->pExtOutRslt = (void *)dec_result;
    p_out_ipc->nLenExtOutRslt = sizeof (tof_decode_result_t);

    memset((void *)pDec, 0, sizeof (tof_decode_params_t));

    pDec->p1_table = (int16_t *)(DDR_MEM_TOF_P1_TABLE);
    pDec->p2_table = (int16_t *)(DDR_MEM_TOF_P2_TABLE);
    pDec->shift_table = (int16_t *)(DDR_MEM_TOF_SHIFT_TABLE);
    pDec->table_gen = (int16_t *)(tof_table_gen_addr);
    pDec->gradient_lut = (float *)(tof_lut_addr);
    pDec->gamma_table = (int16_t *)(DDR_MEM_TOF_GAMMA_TABLE);
    pDec->non_linear_H = (int16_t *)(DDR_MEM_TOF_NON_LINEAR);
    pDec->CalibParameter = (unsigned char*)(DDR_MEM_TOF_CALI_TABLE);
    pDec->temperature_coeff = (float *)(DDR_MEM_TOF_TEMPERATURE_COEFF);
    pDec->ir_params = (ir_params *)(DDR_MEM_TOF_IR_CAM_PARAMS);

    pDec->temp_depth_buf = tof_temp_depth_buf_addr;
    pDec->temp_depth_buf_size = TOF_TEMP_DEPTH_SIZE;

    pDec->flag = 0x01;

    pDec->width = 640;
    pDec->height = 240;

    p_out_ipc->cmd = CMD_TOF_DECODE;
}

int kmdw_tof_decode(uint32_t tof_in_buf_addr, uint32_t tof_in_len, uint32_t out_depth_buf_addr, uint32_t out_ir_buf_addr)
{

    scpu_to_ncpu_t *p_out_ipc = (scpu_to_ncpu_t *)tof_dec_kmdw_ctx.p_ipc_out;
    tof_decode_params_t *pDec = (tof_decode_params_t *)p_out_ipc->pExtInParam;

    pDec->src_buf.buf_addr = tof_in_buf_addr;
    pDec->src_buf.buf_filled_len = tof_in_len;

    pDec->dst_depth_buf = out_depth_buf_addr;
    pDec->dst_depth_buf_size = 307200*2;  // estimated max size, optional

    pDec->dst_ir_buf = out_ir_buf_addr;
    pDec->dst_ir_buf_size = 307200;  // estimated max size, optional

    //trigger ncpu/npu
    kmdw_ipc_trigger_int(CMD_TOF_DECODE);

    uint32_t flags = osEventFlagsWait(tof_dec_kmdw_ctx.tof_dec_evt_id, tof_dec_kmdw_ctx.tof_dec_evt_flag, osFlagsWaitAny, 30000);
    if (flags == (uint32_t)osFlagsErrorTimeout)
    {
        kmdw_printf("error ! TOF decode timeout!!!!!!!!!!!!!!!\n");
        return -7; // FIXME: for now it is timeout
    }
    else
    {
        if(dec_result->sts == TOF_DECODE_OPERATION_SUCCESS)
            return 0;
        else{
            kmdw_printf("error ! TOF decode error code = %d\n, dec_result->sts");
            return dec_result->sts;
        }
    }
}

tof_decode_result_t* model_tof_dec_get_rslt(void)
{
    return (tof_decode_result_t*)&tof_dec_kmdw_ctx.npu_rslt;
}

void kmdw_tof_ir_bright_init(uint32_t src_addr, uint32_t src_w, uint32_t src_h)
{
    ir_bright_params_t *pDec;
    scpu_to_ncpu_t *p_out_ipc;

    ir_bright_result = (void *)kmdw_ddr_reserve(sizeof(ir_bright_result_t));
    if (ir_bright_result == 0)
        kmdw_printf("error !! ir_bright_result DDR malloc failed\n");
    
    // kmdw_printf("cala ir bright config\n");
    tof_ir_bright_buf_addr = kmdw_ddr_reserve(sizeof(bounding_box_t));

    ir_bright_kmdw_ctx.ir_bright_evt_id = osEventFlagsNew(0);
    ir_bright_kmdw_ctx.ir_bright_evt_flag = BIT8;//vt_flag;
    ir_bright_kmdw_ctx.p_ipc_out = kmdw_ipc_get_output();
    ir_bright_kmdw_ctx.p_ipc_in = kmdw_ipc_get_input();

    p_out_ipc = (scpu_to_ncpu_t *)ir_bright_kmdw_ctx.p_ipc_out;

    pDec = (ir_bright_params_t *)p_out_ipc->pExtInParam;
    p_out_ipc->nLenExtInParam = sizeof (ir_bright_params_t);

    p_out_ipc->pExtOutRslt = (void *)ir_bright_result;
    p_out_ipc->nLenExtOutRslt = sizeof (ir_bright_result_t);

    memset((void *)pDec, 0, sizeof (ir_bright_params_t));

    // change here to yolo result?
    bounding_box_t *p_aec_bbox = (bounding_box_t *)tof_ir_bright_buf_addr;
    p_aec_bbox->x1 = 0;
    p_aec_bbox->y1 = 0;
    p_aec_bbox->x2 = 64;//src_w-1;
    p_aec_bbox->y2 = 64;//src_h-1;
    
    pDec->ir_aec_bbox = (bounding_box_t *)p_aec_bbox;

    pDec->src_buf.buf = src_addr;
    pDec->src_buf.width = src_w;
    pDec->src_buf.height = src_h;

    p_out_ipc->cmd = CMD_TOF_CALC_IR_BRIGHT;
}

int32_t kmdw_tof_ir_bright_aec(uint32_t nir_output_buf, int32_t src_width, uint32_t src_height, bounding_box_t *single_tof_box, bool is_target_found)
{
    //trigger ncpu/npu
    // dbg_msg("[%.3f][%s] kmdw_tof_ir_bright_aec\n", (float)osKernelGetTickCount()/osKernelGetTickFreq(), __func__);
    /*  update nir output_buf, src_width, src_height */
    scpu_to_ncpu_t *p_out_ipc = (scpu_to_ncpu_t *)ir_bright_kmdw_ctx.p_ipc_out;
    ir_bright_params_t *pDec = (ir_bright_params_t *)p_out_ipc->pExtInParam;

    // critical_msg("ir_bright_kmdw_ctx.ir_bright_evt_id : %d\n",ir_bright_kmdw_ctx.ir_bright_evt_id);
    if (NULL == ir_bright_kmdw_ctx.ir_bright_evt_id)
    {
        ir_bright_kmdw_ctx.ir_bright_evt_id = osEventFlagsNew(0);
        ir_bright_kmdw_ctx.ir_bright_evt_flag = BIT8;
    }

    bounding_box_t *p_aec_bbox = (bounding_box_t *)tof_ir_bright_buf_addr;

    pDec->src_buf.buf = nir_output_buf;
    pDec->src_buf.width = src_width;
    pDec->src_buf.height = src_height;
    pDec->ir_aec_bbox = (bounding_box_t *)p_aec_bbox;
    pDec->is_target_found = is_target_found;

    /* --------------------------------------------- */
    // critical_msg("kmdw_tof_ir_bright_aec 1\n");
    kmdw_ipc_trigger_int(CMD_TOF_CALC_IR_BRIGHT);
    // critical_msg("kmdw_tof_ir_bright_aec 2\n");
    uint32_t flags = osEventFlagsWait(ir_bright_kmdw_ctx.ir_bright_evt_id, ir_bright_kmdw_ctx.ir_bright_evt_flag, osFlagsWaitAny, 5000);
    if (flags == (uint32_t)osFlagsErrorTimeout)
    {
        kmdw_printf("error ! kmdw_tof_ir_bright_aec timeout!!!!!!!!!!!!!!!\n");
        return -7; // FIXME: for now it is timeout
    }
    else
    {
        if(ir_bright_result->sts == TOF_IR_AEC_OPERATION_SUCCESS)
            return 0;
        else{
            kmdw_printf("error ! TOF ir aec error code = %d\n", ir_bright_result->sts);
            return ir_bright_result->sts;
        }
    }
}

ir_bright_result_t* model_ir_bright_get_rslt(void)
{
    return (ir_bright_result_t*)&ir_bright_kmdw_ctx.npu_rslt;
}
