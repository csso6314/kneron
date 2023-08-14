/**
 * @file        kmdw_fifoq_manager.h
 * @brief       for kdp2 fw only - inference structures and functions
 *
 * @copyright   Copyright (c) 2022 Kneron Inc. All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "kp_struct.h"
#include "dual_fifo2.h"

/**
 * @brief Init the fifo queue manager
 *
 * @param image_count[in] the capacity of buffers in image queue
 * @param result_count[in] the capacity of buffers in result queue
 * @return int 0: success, -1: failed
 */
int kmdw_fifoq_manager_init(uint32_t image_count, uint32_t result_count);

/**
 * @brief enqueue one inference object containing one or more images to the "inference-waiting buffer queue"
 *
 * return immediately if queue is not full
 * blocking wait (unless timeout) if queue if full
 * If 1 < total_num_buf, the image buffers will be enqueued after index 0 ~ (total_num_buf - 1) has been stored.
 *
 * @param total_num_buf[in] the total number of buffers should be contain in the list
 * @param index[in] index of the buffer in the list
 * @param buf_addr[in] address of the buffer
 * @param buf_size[in] size of the buffer
 * @param timeout[in] CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @param preempt[in] preempt this result data
 * @return osStatus_t Status code values returned by CMSIS-RTOS functions.
 */
osStatus_t kmdw_fifoq_manager_image_enqueue(uint32_t total_num_buf, uint32_t index, uint32_t buf_addr, int buf_size, uint32_t timeout, bool preempt);

/**
 * @brief request one inference object from the "inference-waiting buffer queue"
 *
 * return immediately if queue has inference objects
 * blocking wait (unless timeout) if no inference object is available yet
 *
 * @param bobj[out] buffer object containing one or more buffers to be inference
 * @param timeout[int] CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return osStatus_t Status code values returned by CMSIS-RTOS functions.
 */
osStatus_t kmdw_fifoq_manager_image_dequeue(buffer_object_t *bobj, uint32_t timeout);

/**
 * @brief retrieve one free-to-use image buffer from the "inference-done image queue"
 *
 * return immediately if queue has free buffers
 * blocking wait (unless timeout) if no free buffer is available
 * if force_grab, and no free buffer is available, it will force grab the earliest-queued buffer from the "inference-waiting buffer queue"
 *
 * @param buf_addr[in] address of the buffer
 * @param buf_size[in] size of the buffer
 * @param timeout[in] CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @param force_grab[int] whether force grab one buffer from data queue when no free buffer is available
 * @return osStatus_t Status code values returned by CMSIS-RTOS functions.
 */
osStatus_t kmdw_fifoq_manager_image_get_free_buffer(uint32_t *buf_addr, int *buf_size, uint32_t timeout, bool force_grab);

/**
 * @brief put one free buffer to the "inference-done image queue"
 *
 * return immediately if queue is not full
 * blocking wait (unless timeout) if no free buffer is available
 *
 * @param buf_addr[in] address of the buffer
 * @param buf_size[in] size of the buffer
 * @param timeout[in] CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return osStatus_t Status code values returned by CMSIS-RTOS functions.
 */
osStatus_t kmdw_fifoq_manager_image_put_free_buffer(uint32_t buf_addr, int buf_size, uint32_t timeout);

/**
 * @brief enqueue one result data to the "inference-complete result queue"
 *
 * return immediately if queue is not full
 * blocking wait (unless timeout) if queue if full
 *
 * @param result_buf[in] address of the buffer
 * @param result_buf_size[in] size of the buffer
 * @param preempt[in] preempt this result data
 * @return osStatus_t Status code values returned by CMSIS-RTOS functions.
 */
osStatus_t kmdw_fifoq_manager_result_enqueue(void *result_buf, int result_buf_size, bool preempt);

/**
 * @brief request one inference result from the "inference-complete result queue"
 *
 * return immediately if queue has resutls
 * blocking wait (unless timeout) if no result is available yet
 *
 * @param buf_addr[in] address of the buffer
 * @param buf_size[in] size of the buffer
 * @param timeout[in] CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return osStatus_t Status code values returned by CMSIS-RTOS functions.
 */
osStatus_t kmdw_fifoq_manager_result_dequeue(uint32_t *buf_addr, int *buf_size, uint32_t timeout);

/**
 * @brief retrieve one free-to-use result buffer from the "free result queue"
 *
 * return immediately if queue has free buffers
 * blocking wait (unless timeout) if no free buffer is available
 * if force_grab, and no free buffer is available, it will force grab the earliest-queued buffer from the "inference-complete result queue"
 *
 * @param buf_size[in] size of the buffer
 * @return void* address of the buffer
 */
void *kmdw_fifoq_manager_result_get_free_buffer(int *buf_size);

/**
 * @brief put one free buffer to the "free result queue" (which will be used by inference APP)
 *
 * return immediately if queue is not full
 * blocking wait (unless timeout) if no free buffer is available
 *
 * @param buf_addr[in] address of the buffer
 * @param buf_size[in] size of the buffer
 * @param timeout[in] CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return osStatus_t Status code values returned by CMSIS-RTOS functions.
 */
osStatus_t kmdw_fifoq_manager_result_put_free_buffer(uint32_t buf_addr, int buf_size, uint32_t timeout);

/**
 * @brief Clear the data queues and put buffer back to free queues
 */
void kmdw_fifoq_manager_clean_queues(void);

/**
 * @brief Set the status of the fifo queue buffer has been allocated
 *
 * @param input_buf_countp[in] Input buffer count for FIFO queue
 * @param input_buf_size[in] Input buffer size for FIFO queue
 * @param result_buf_count[in] Result buffer count for FIFO queue
 * @param result_buf_size[in] Result buffer size for FIFO queue
 */
void kmdw_fifoq_manager_store_fifoq_config(uint32_t input_buf_count, uint32_t input_buf_size, uint32_t result_buf_count, uint32_t result_buf_size);

/**
 * @brief Get the status of whether the fifo queue buffer has been allocated
 *
 * @return true the fifo queue buffer has been allocated
 * @return false the fifo queue buffer has NOT been allocated
 */
bool kmdw_fifoq_manager_get_fifoq_allocated(void);

/**
 * @brief Get the configuration of FIFO queue buffer
 *
 * @param input_buf_count[out] Input buffer count for FIFO queue
 * @param input_buf_size[out] Input buffer size for FIFO queue
 * @param result_buf_count[out] Result buffer count for FIFO queue
 * @param result_buf_size[out] Result buffer size for FIFO queue
 */
void kmdw_fifoq_manager_get_fifoq_config(uint32_t *input_buf_count, uint32_t *input_buf_size, uint32_t *result_buf_count, uint32_t *result_buf_size);
