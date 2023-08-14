#ifndef DUAL_FIFO2_H
#define DUAL_FIFO2_H

#include <stdbool.h>
#include "cmsis_os2.h"

#include "buffer_object.h"

/*
'osStatus_t' :
osOK: the message has been put into the queue.
osErrorTimeout: the message could not be put into the queue in the given time (wait-timed semantics).
osErrorResource: not enough space in the queue (try semantics).
osErrorParameter: parameter mq_id is NULL or invalid, non-zero timeout specified in an ISR.
*/

/*
'timeout' :
when timeout is 0, the function returns instantly (i.e. try semantics).
when timeout is set to osWaitForever the function will wait for an infinite time until the message is delivered (i.e. wait semantics).
all other values specify a time in kernel ticks for a timeout (i.e. timed-wait semantics).
*/

// handle to a dual_fifo2_t
typedef void *dual_fifo2_t;

#define DUAL_FIFO_VALID_ADDR 0x10 // value of 'dual_fifo2_t' should bigger than this
#define DUAL_FIFO_MALLOC_FAILED 0x1
#define DUAL_FIFO_MSGQ_NEW_FAILED 0x2

// create a new dual fifo
dual_fifo2_t dual_fifo2_create(uint32_t queue_count);

// producer: acquire a new free buffer
// NOTE: timeout should be 0 for force_grab = true
osStatus_t dual_fifo2_get_free_buffer(dual_fifo2_t df, buffer_object_t *bobj, uint32_t timeout, bool force_grab);

// producer: put/enqueue data buffer
osStatus_t dual_fifo2_enqueue_data(dual_fifo2_t df, buffer_object_t bobj, uint32_t timeout, bool preempt);

// consumer get/dequeue data buffer
osStatus_t dual_fifo2_dequeue_data(dual_fifo2_t df, buffer_object_t *bobj, uint32_t timeout);

// consumer: return used data buffer
osStatus_t dual_fifo2_put_free_buffer(dual_fifo2_t df, buffer_object_t bobj, uint32_t timeout);

// check the number of un-consumed data buffer
uint32_t dual_fifo2_num_unconsumed_data(dual_fifo2_t df);

// check the number of free data buffer
uint32_t dual_fifo2_num_free_buffer(dual_fifo2_t df);

// return the total contain buffer size of this dual fifo
uint32_t dual_fifo2_total_buffer_size(dual_fifo2_t df);

// destroy a dual fifo
void dual_fifo2_destroy(dual_fifo2_t df);

#endif

