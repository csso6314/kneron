#include "dual_fifo.h"

#include <stdlib.h>

typedef struct
{
    osMessageQueueId_t free_msgq; // free buf queue
    osMessageQueueId_t data_msgq; // data buf queue
    uint32_t buf_size;
} _Dual_FIFO_t;

dual_fifo_t dual_fifo__create(uint32_t queue_count, uint32_t buf_size)
{
    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)malloc(sizeof(_Dual_FIFO_t));
    // fifo queues save only the pointer address
    df_ptr->free_msgq = osMessageQueueNew(queue_count, sizeof(void *), NULL);
    df_ptr->data_msgq = osMessageQueueNew(queue_count, sizeof(void *), NULL);
    df_ptr->buf_size = buf_size;

    return (dual_fifo_t)df_ptr;
}

osStatus_t dual_fifo__get_free_buffer(dual_fifo_t df, void *data_ptr, uint32_t timeout, bool force_grab)
{
    // NOTE: timeout should be 0 for force_grab = true

    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)df;

    osStatus_t sts = osMessageQueueGet(df_ptr->free_msgq, data_ptr, NULL, timeout);

    if (force_grab && sts == osErrorResource)
    {
        sts = osMessageQueueGet(df_ptr->data_msgq, data_ptr, NULL, 0);
    }

    return sts;
}

osStatus_t dual_fifo__put_free_buffer(dual_fifo_t df, const void *data_ptr, uint32_t timeout)
{
    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)df;
    return osMessageQueuePut(df_ptr->free_msgq, data_ptr, 0U, timeout);
}

osStatus_t dual_fifo__enqueue_data(dual_fifo_t df, const void *data_ptr, uint32_t timeout)
{
    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)df;
    return osMessageQueuePut(df_ptr->data_msgq, data_ptr, 0U, timeout);
}

osStatus_t dual_fifo__dequeue_data(dual_fifo_t df, void *data_ptr, uint32_t timeout)
{
    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)df;
    return osMessageQueueGet(df_ptr->data_msgq, data_ptr, NULL, timeout);
}

uint32_t dual_fifo__unprocessed_data(dual_fifo_t df)
{
    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)df;
    return osMessageQueueGetCount(df_ptr->data_msgq);
}

uint32_t dual_fifo__buffer_size(dual_fifo_t df)
{
    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)df;
    return df_ptr->buf_size;
}

void dual_fifo__destroy(dual_fifo_t df)
{
    _Dual_FIFO_t *df_ptr = (_Dual_FIFO_t *)df;
    osMessageQueueDelete(df_ptr->free_msgq);
    osMessageQueueDelete(df_ptr->data_msgq);
    free(df_ptr);
}
