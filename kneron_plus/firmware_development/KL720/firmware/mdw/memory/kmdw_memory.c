/*
 * This software was ported from freeRTOS and was modified for Kneron use.
 * Copyright(C) 2021 Kneron, Inc.
 *
 * The below statement is FreeRTOS copyright statement for legal compliance.
 *
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 */

#include "cmsis_os2.h"
#include "base.h"
#include "kmdw_console.h"
#include "kmdw_memory.h"

/* ddr malloc direction : from tail(bigger address) to head (smaller) */

static uint32_t s_ddr_addr_tail = 0;
static uint32_t s_ddr_addr_boundary = 0;

static uint32_t s_ddr_system_reserve_addr = 0;
static uint32_t s_ddr_system_reserve_size = 0;

void kmdw_ddr_init(uint32_t start_addr, uint32_t end_addr)
{
    s_ddr_addr_tail = start_addr; //KDP_DDR_HEAP_HEAD_FOR_MALLOC+1 (address to the last used tail)
    s_ddr_addr_boundary = end_addr; //KDP_DDR_MODEL_RESERVED_END
}

int kmdw_ddr_set_ddr_boundary(uint32_t boundary)
{
    if (boundary >= s_ddr_addr_tail)
        return -1;
    else {
        s_ddr_addr_boundary = boundary;
        return 0;
    }
}

uint32_t kmdw_ddr_reserve(uint32_t numbyte)
{
    uint32_t aligned_numbyte;
    uint32_t tail_tmp;

    if(numbyte == 0)
        return 0;

    if(s_ddr_addr_boundary == 0)
        return 0; //not initialized yet

    aligned_numbyte = ALIGN64(numbyte);
    tail_tmp = s_ddr_addr_tail;
    tail_tmp = ALIGN64_FLOOR(tail_tmp - aligned_numbyte);

    if(tail_tmp <= s_ddr_addr_boundary)
    {
        err_msg("[ERR] ddr malloc64 %x over (<) available bondary %x\n", tail_tmp, s_ddr_addr_boundary);
        return 0;
    }
    s_ddr_addr_tail = tail_tmp;
    return s_ddr_addr_tail;
}

uint32_t kmdw_ddr_get_heap_tail()
{
    return s_ddr_addr_tail;
}

void kmdw_ddr_store_system_reserve(uint32_t start_addr, uint32_t end_addr)
{
    s_ddr_system_reserve_addr = start_addr;
    s_ddr_system_reserve_size = (end_addr - start_addr);
}

void kmdw_ddr_get_system_reserve(uint32_t *start_addr, uint32_t *ddr_size)
{
    *start_addr = s_ddr_system_reserve_addr;
    *ddr_size = s_ddr_system_reserve_size;
}
