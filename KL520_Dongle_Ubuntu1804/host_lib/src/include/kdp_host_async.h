/**
 * @file      kdp_host_async.h
 * @brief     kdp host lib header file for async transfer
 * @version   0.1 - 2019-04-25
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __KDP_HOST_ASYNC__H__
#define __KDP_HOST_ASYNC__H__

#include <stdint.h>

//msg hdr
typedef struct {
    uint16_t msg_type;
    uint16_t msg_len;
} KnMsgHdr;

//call back func type def
typedef int (*KdpCallBack)(int8_t* buf, int msg_len);

//----------------------------rsp msg definition -------------------
//structure for mem read rsp and mem echo rsp
typedef struct {
    KnMsgHdr hdr;
    uint32_t rsp_result;
    uint32_t num_bytes;
    int8_t*  p_data;
} MemReadRsp, MemEchoRsp;

//structure for mem write rsp and mem clear rsp
typedef struct {
    KnMsgHdr hdr;
    uint32_t rsp_result;
    uint32_t num_bytes;
} MemWriteRsp, MemClearRsp;

//structure for cmd reset response
typedef struct {
    KnMsgHdr hdr;
    uint32_t err_code;
    uint32_t reserved;
} CmdResetRsp;
//----------------------------rsp msg definition end-------------------


//mem test read request
int kdp_mem_test_read(int dev_idx, uint32_t start_addr, uint32_t num_bytes, KdpCallBack func);

//mem test write request
int kdp_mem_test_write(int dev_idx, uint32_t start_addr, uint32_t num_bytes, int8_t* p_data, int d_len, KdpCallBack func);

//mem test clear request
int kdp_mem_test_clear(int dev_idx, uint32_t start_addr, uint32_t num_bytes, KdpCallBack func);

//mem test echo request
int kdp_mem_test_echo(int dev_idx, uint32_t start_addr, uint32_t num_bytes, int8_t* p_data, int d_len, KdpCallBack func);

//cmd reset requet
int kdp_cmd_reset(int dev_idx, uint32_t rset_mode, KdpCallBack func);

//wait and handle rsp messages.
//it will call the callback funcs in the request api
void wait_n_handle_rsp_msg();

#endif
