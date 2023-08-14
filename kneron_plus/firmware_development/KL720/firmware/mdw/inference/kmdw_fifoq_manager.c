/*
 * Kneron Application general functions
 *
 * Copyright (C) 2022 Kneron, Inc. All rights reserved.
 *
 */

// #define DEBUG_PRINT

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kdrv_gdma3.h"
#include "kmdw_ipc.h"
#include "kmdw_memory.h"
#include "kmdw_model.h"
#include "kmdw_console.h"
#include "membase.h"

#include "kp_struct.h"
#include "kmdw_fifoq_manager.h"

#ifdef DEBUG_PRINT
#include "kmdw_console.h"
#define dbg_print(__format__, ...) kmdw_level_printf(LOG_CUSTOM, "[buf_mgr]"__format__, ##__VA_ARGS__)
#else
#define dbg_print(__format__, ...)
#endif

#define INF_TIMEOUT                 2000 // twice

static dual_fifo2_t _image_fifioq = 0;
static dual_fifo2_t _result_fifoq = 0;
static osMessageQueueId_t _temp_image_queue;
static bool _fifoq_mem_allocated = false;
static uint32_t _fifoq_input_buf_count = 0;
static uint32_t _fifoq_input_buf_size = 0;
static uint32_t _fifoq_result_buf_count = 0;
static uint32_t _fifoq_result_buf_size = 0;

typedef struct
{
    int total_num_buffer;
    uint32_t buffer_addr;
    int length;
    int buffer_index;
} special_buffer_object_t;

void kdp2_fifoq_manager_enqueue_image_thread(void *arg)
{
    special_buffer_object_t received_bobj;
    buffer_object_t stored_bobj;
    int num_received_img = 0;

    memset(&stored_bobj, 0, sizeof(buffer_object_t));

    while (true) {
        osStatus_t sts = osMessageQueueGet(_temp_image_queue, (void *)&received_bobj, NULL, osWaitForever);

        if (osOK != sts) {
            continue;
        }

        stored_bobj.num_of_buffer = received_bobj.total_num_buffer;

        if (NULL != stored_bobj.buffer_addr[received_bobj.buffer_index]) {
            kmdw_fifoq_manager_image_put_free_buffer(stored_bobj.buffer_addr[received_bobj.buffer_index],
                                                     stored_bobj.length[received_bobj.buffer_index], osWaitForever);
        } else {
            num_received_img++;
        }

        stored_bobj.buffer_addr[received_bobj.buffer_index] = received_bobj.buffer_addr;
        stored_bobj.length[received_bobj.buffer_index] = received_bobj.length;

        if (num_received_img == stored_bobj.num_of_buffer) {
            dual_fifo2_enqueue_data(_image_fifioq, stored_bobj, osWaitForever, false);

            num_received_img = 0;
            memset(&stored_bobj, 0, sizeof(buffer_object_t));
        }
    }
}

int kmdw_fifoq_manager_init(uint32_t image_count, uint32_t result_count)
{
    kmdw_printf("creating image queue with size %d\n", image_count);
    kmdw_printf("creating result queue with size %d\n", result_count);

    _image_fifioq = dual_fifo2_create(image_count);
    if ((uint32_t)_image_fifioq < DUAL_FIFO_VALID_ADDR)
    {
        kmdw_printf("image queue creating failed !!\n");
        return -1;
    }

    _result_fifoq = dual_fifo2_create(result_count);
    if ((uint32_t)_result_fifoq < DUAL_FIFO_VALID_ADDR)
    {
        kmdw_printf("result queue creating failed !!\n");
        return -1;
    }
    
    //the size of message queue is MAX at image count as one multiple-input
    _temp_image_queue = osMessageQueueNew(image_count, sizeof(special_buffer_object_t), NULL);

    return 0;
}

osStatus_t kmdw_fifoq_manager_image_enqueue(uint32_t total_num_buf, uint32_t index, uint32_t buf_addr, int buf_size, uint32_t timeout, bool preempt)
{
    if (0 == total_num_buf) {
        return osErrorParameter;
    }

    special_buffer_object_t special_obj;

    special_obj.total_num_buffer = total_num_buf;
    special_obj.buffer_index = (1 == special_obj.total_num_buffer) ? 0 : index;
    special_obj.buffer_addr = buf_addr;
    special_obj.length = buf_size;

    return osMessageQueuePut(_temp_image_queue, (const void *)&special_obj, (preempt) ? (1U) : (0U), timeout);
}

osStatus_t kmdw_fifoq_manager_image_dequeue(buffer_object_t *bobj, uint32_t timeout)
{
    return dual_fifo2_dequeue_data(_image_fifioq, bobj, timeout);
}

osStatus_t kmdw_fifoq_manager_image_get_free_buffer(uint32_t *buf_addr, int *buf_size, uint32_t timeout, bool force_grab)
{
    buffer_object_t bobj;
    osStatus_t sts = dual_fifo2_get_free_buffer(_image_fifioq, &bobj, timeout, force_grab);

    if (osOK != sts) {
        return sts;
    }

    //TODO: so far, just handle num_of_buffer > 1 in the future
    if (1 < bobj.num_of_buffer) {
        for (int i = 1; i < bobj.num_of_buffer; i++) {
            kmdw_fifoq_manager_image_put_free_buffer(bobj.buffer_addr[i], bobj.length[i], timeout);
            //TODO: need to handle return value 
        }
    }

    *buf_addr = bobj.buffer_addr[0];
    *buf_size = bobj.length[0];

    return sts;
}

osStatus_t kmdw_fifoq_manager_image_put_free_buffer(uint32_t buf_addr, int buf_size, uint32_t timeout)
{
    buffer_object_t bobj;

    bobj.num_of_buffer = 1;
    bobj.buffer_addr[0] = buf_addr;
    bobj.length[0] = buf_size;

    return dual_fifo2_put_free_buffer(_image_fifioq, bobj, timeout);
}

osStatus_t kmdw_fifoq_manager_result_enqueue(void *result_buf, int result_buf_size, bool preempt)
{
    buffer_object_t bobj;

    bobj.num_of_buffer = 1;
    bobj.buffer_addr[0] = (uint32_t)result_buf;
    bobj.length[0] = result_buf_size;

    return dual_fifo2_enqueue_data(_result_fifoq, bobj, 0, preempt);
}

osStatus_t kmdw_fifoq_manager_result_dequeue(uint32_t *buf_addr, int *buf_size, uint32_t timeout)
{
    buffer_object_t bobj;
    osStatus_t sts = dual_fifo2_dequeue_data(_result_fifoq, &bobj, timeout);

    if (osOK != sts) {
        return sts;
    }

    //TODO: so far, just handle num_of_buffer > 1 in the future
    if (1 < bobj.num_of_buffer) {
        for (int i = 1; i < bobj.num_of_buffer; i++) {
            kmdw_fifoq_manager_result_put_free_buffer(bobj.buffer_addr[i], bobj.length[i], timeout);
            //TODO: need to handle return value 
        }
    }

    *buf_addr = bobj.buffer_addr[0];
    *buf_size = bobj.length[0];

    return sts;
}

void *kmdw_fifoq_manager_result_get_free_buffer(int *buf_size)
{
    buffer_object_t bobj;
    osStatus_t sts = dual_fifo2_get_free_buffer(_result_fifoq, &bobj, osWaitForever, false);

    if (osOK != sts) {
        return NULL;
    }

    //TODO: so far, just handle num_of_buffer > 1 in the future
    if (1 < bobj.num_of_buffer) {
        for (int i = 1; i < bobj.num_of_buffer; i++) {
            kmdw_fifoq_manager_result_put_free_buffer(bobj.buffer_addr[i], bobj.length[i], 0);
            //TODO: need to handle return value
        }
    }

    *buf_size = bobj.length[0];

    return (void *)bobj.buffer_addr[0];
}

osStatus_t kmdw_fifoq_manager_result_put_free_buffer(uint32_t buf_addr, int buf_size, uint32_t timeout)
{
    buffer_object_t bobj;

    bobj.num_of_buffer = 1;
    bobj.buffer_addr[0] = buf_addr;
    bobj.length[0] = buf_size;

    return dual_fifo2_put_free_buffer(_result_fifoq, bobj, timeout);
}

void kmdw_fifoq_manager_clean_queues(void)
{
    buffer_object_t bobj;

    while (1)
    {
        if (dual_fifo2_dequeue_data(_image_fifioq, &bobj, 0) == osOK)
            dual_fifo2_put_free_buffer(_image_fifioq, bobj, 0);
        else
            break;
    }

    while (1)
    {
        if (dual_fifo2_dequeue_data(_result_fifoq, &bobj, 0) == osOK)
            dual_fifo2_put_free_buffer(_result_fifoq, bobj, 0);
        else
            break;
    }
}

void kmdw_fifoq_manager_store_fifoq_config(uint32_t input_buf_count, uint32_t input_buf_size, uint32_t result_buf_count, uint32_t result_buf_size)
{
    _fifoq_mem_allocated = true;
    _fifoq_input_buf_count = input_buf_count;
    _fifoq_input_buf_size = input_buf_size;
    _fifoq_result_buf_count = result_buf_count;
    _fifoq_result_buf_size = result_buf_size;
}

bool kmdw_fifoq_manager_get_fifoq_allocated()
{
    return _fifoq_mem_allocated;
}

void kmdw_fifoq_manager_get_fifoq_config(uint32_t *input_buf_count, uint32_t *input_buf_size, uint32_t *result_buf_count, uint32_t *result_buf_size)
{
    *input_buf_count = _fifoq_input_buf_count;
    *input_buf_size = _fifoq_input_buf_size;
    *result_buf_count = _fifoq_result_buf_count;
    *result_buf_size = _fifoq_result_buf_size;
}
