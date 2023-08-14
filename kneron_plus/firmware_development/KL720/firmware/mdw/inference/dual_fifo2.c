#include <stdlib.h>
#include <stdint.h>
#include "dual_fifo2.h"
#include "kmdw_memory.h"
#include "kmdw_console.h"

typedef struct _Dual_FIFO2_s
{
    osMessageQueueId_t free_msgq; // free buf queue
    osMessageQueueId_t data_msgq; // data buf queue
    uint32_t queue_count;
} _Dual_FIFO2_t;

dual_fifo2_t dual_fifo2_create(uint32_t queue_count)
{
    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)malloc(sizeof(_Dual_FIFO2_t));
    if (df_ptr == NULL)
        return (void *)DUAL_FIFO_MALLOC_FAILED;

    // fifo queues save only the pointer address
    df_ptr->free_msgq = osMessageQueueNew(queue_count, sizeof(buffer_object_t), NULL);
    if (df_ptr->free_msgq == NULL) {
        free(df_ptr);
        return (void *)DUAL_FIFO_MSGQ_NEW_FAILED;
    }

    df_ptr->data_msgq = osMessageQueueNew(queue_count, sizeof(buffer_object_t), NULL);
    if (df_ptr->data_msgq == NULL) {
        osMessageQueueDelete(df_ptr->free_msgq);
        free(df_ptr);
        return (void *)DUAL_FIFO_MSGQ_NEW_FAILED;
    }

    df_ptr->queue_count = queue_count;

    return (dual_fifo2_t)df_ptr;
}

osStatus_t dual_fifo2_get_free_buffer(dual_fifo2_t df, buffer_object_t *bobj, uint32_t timeout, bool force_grab)
{
    // NOTE: timeout should be 0 for force_grab = true

    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)df;

    if (force_grab)
        timeout = 0;

    osStatus_t sts = osMessageQueueGet(df_ptr->free_msgq, (void *)bobj, NULL, timeout);

    if (force_grab && sts == osErrorResource)
        sts = osMessageQueueGet(df_ptr->data_msgq, (void *)bobj, NULL, 0);

    return sts;
}

osStatus_t dual_fifo2_put_free_buffer(dual_fifo2_t df, buffer_object_t bobj, uint32_t timeout)
{
    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)df;
    osStatus_t sts = osMessageQueuePut(df_ptr->free_msgq, (const void *)&bobj, 0U, timeout);
    return sts;
}

osStatus_t dual_fifo2_enqueue_data(dual_fifo2_t df, buffer_object_t bobj, uint32_t timeout, bool preempt)
{
    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)df;
    osStatus_t sts = osMessageQueuePut(df_ptr->data_msgq, (const void *)&bobj, (preempt) ? (1U) : (0U), timeout);
    return sts;
}

osStatus_t dual_fifo2_dequeue_data(dual_fifo2_t df, buffer_object_t *bobj, uint32_t timeout)
{
    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)df;
    osStatus_t sts = osMessageQueueGet(df_ptr->data_msgq, (void *)bobj, NULL, timeout);
    return sts;
}

uint32_t dual_fifo2_num_unconsumed_data(dual_fifo2_t df)
{
    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)df;
    return osMessageQueueGetCount(df_ptr->data_msgq);
}

uint32_t dual_fifo2_num_free_buffer(dual_fifo2_t df)
{
    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)df;
    return osMessageQueueGetCount(df_ptr->free_msgq);
}

void dual_fifo2_destroy(dual_fifo2_t df)
{
    _Dual_FIFO2_t *df_ptr = (_Dual_FIFO2_t *)df;
    osMessageQueueDelete(df_ptr->free_msgq);
    osMessageQueueDelete(df_ptr->data_msgq);
    free(df_ptr);
}
