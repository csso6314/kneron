#ifndef _DUAL_QUEUE_H_
#define _DUAL_QUEUE_H_

#include <stdbool.h>
#include "cmsis_os2.h"

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

// handle to a dual_fifo_t
typedef void *dual_fifo_t;

// create a new dual fifo
dual_fifo_t dual_fifo__create(uint32_t queue_count, uint32_t buf_size);

// producer: acquire a new free buffer
// NOTE: timeout should be 0 for force_grab = true
osStatus_t dual_fifo__get_free_buffer(dual_fifo_t df, void *data_ptr, uint32_t timeout, bool force_grab);

// producer: put/enqueue data buffer
osStatus_t dual_fifo__enqueue_data(dual_fifo_t df, const void *data_ptr, uint32_t timeout);

// consumer get/dequeue data buffer
osStatus_t dual_fifo__dequeue_data(dual_fifo_t df, void *data_ptr, uint32_t timeout);

// consumer: return used data buffer
osStatus_t dual_fifo__put_free_buffer(dual_fifo_t df, const void *data_ptr, uint32_t timeout);

// check the number of un-consumed data buffer
uint32_t dual_fifo__unconsumed_data(dual_fifo_t df);

// return the actual buffer size of this dual fifo
uint32_t dual_fifo__buffer_size(dual_fifo_t df);

// destroy a dual fifo
void dual_fifo__destroy(dual_fifo_t df);

#endif
