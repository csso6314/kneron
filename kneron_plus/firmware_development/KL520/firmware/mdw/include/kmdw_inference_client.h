/**
 * @file        kmdw_inference_client.h
 * @brief       for kdp2 fw only - inference structures and functions
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "kp_struct.h"

// enqueue one image to the "inference-waiting buffer queue"
// return immediately if queue is not full
// blocking wait (unless timeout) if queue if full
osStatus_t kmdw_inference_client_image_enqueue(uint32_t buf_addr, int buf_size, uint32_t timeout, bool preempt);

// retrieve one free-to-use image buffer from the "inference-done image queue"
// return immediately if queue has free buffers
// blocking wait (unless timeout) if no free buffer is available
// if force_grab, and no free buffer is available, it will forcely grab the earliest-queued buffer from the "inference-waiting buffer queue"
osStatus_t kmdw_inference_client_image_get_free_buffer(uint32_t *buf_addr, int *buf_size, uint32_t timeout, bool force_grab);

// put one free buffer to the "inference-done image queue"
// return immediately if queue is not full
// blocking wait (unless timeout) if no free buffer is available
osStatus_t kmdw_inference_client_image_put_free_buffer(uint32_t buf_addr, int buf_size, uint32_t timeout);

// reqeust one inference result from the "inference-complete result queue"
// return immediately if queue has resutls
// blocking wait (unless timeout) if no result is available yet
osStatus_t kmdw_inference_client_result_request(uint32_t *buf_addr, int *buf_size, uint32_t timeout);

// put one free buffer to the "free result queue" (which will be used by inference APP)
// return immediately if queue is not full
// blocking wait (unless timeout) if no free buffer is available
osStatus_t kmdw_inference_client_result_put_free_buffer(uint32_t buf_addr, int buf_size, uint32_t timeout);

void kmdw_inference_client_clean_queues(void);

void kmdw_inference_client_set_fifoq_allocated(bool blAllocated);

bool kmdw_inference_client_get_fifoq_allocated(void);
