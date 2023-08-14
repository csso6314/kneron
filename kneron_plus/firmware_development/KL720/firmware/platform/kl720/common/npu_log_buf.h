/*
 * Kneron NPU log buffer
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __NPU_LOG_BUF_H__
#define __NPU_LOG_BUF_H__

#include "base.h"

typedef struct _npu_log_msg
{
    uint8_t len;
    uint8_t *pMsg;
} npu_log_msg_node_t;

typedef struct _npu_log_msg_pool
{
    uint8_t * pStart;
    uint8_t * pEnd;
    uint32_t wptr;    //this pointer is updated by NCPU only
    uint32_t rptr;    //this pointer is updated by SCPU only
} npu_log_buf_pool_t;


/* global control variable for log */
typedef struct _npu_log_ctl
{
    uint16_t  nMaxNodes;
    uint16_t  wNodeIdx;    // this index is updated by NPU only
    uint16_t  rNodeIdx;    // this index is updated by SCPU only
    uint8_t   *pNodeBase;  // place npu_log_msg_node_t[]
    int16_t  node_overfill;   // -1  Full, 0 normal
    int16_t  pool_overfill;   // -1 Full, - normal

    npu_log_buf_pool_t  buf_pool;

} npu_log_ctl_t;

#define NPU_LOG_CTL_SPACE    64

#define NPU_LOG_SHARE_NODE_MEM_LEN_IN_BYTES    (1024-NPU_LOG_CTL_SPACE)
#define NPU_LOG_SHARE_MSG_BUF_LEN_IN_BYTES     1024*4

#define NPU_LOG_MSG_TOTAL_BYTES (NPU_LOG_CTL_SPACE + NPU_LOG_SHARE_NODE_MEM_LEN_IN_BYTES + NPU_LOG_SHARE_MSG_BUF_LEN_IN_BYTES)

#define FLAG_LOG_BUF_START_LOG         BIT0

#endif  /* __NPU_LOG_BUF_H__ */

