/*
 *
 * Copyright (C) 2020 Kneron, Inc. All rights reserved.
 *
 */

//#define ERRAND_DBG_ENABLE

#include <stdlib.h>
#include <string.h>
#include "kmdw_errandserv.h"

#ifdef ERRAND_DBG_ENABLE
#include "kmdw_console.h"
#define errand_dbg(__format__, ...) kmdw_level_printf(LOG_CUSTOM, __format__, ##__VA_ARGS__)
#else
#define errand_dbg(__format__, ...)
#endif

#define MAX_ERRAND_TASK_IN_Q 10 // Guess it should be enough

static osThreadId_t errand_tid = NULL;
static osMessageQueueId_t errand_msgq = NULL;
extern osMessageQueueId_t qErrandMgrSched;

typedef struct
{
    errand_function_t errand_func; // must have
    void *arg;                     // optional
    uint32_t execute_time;         // 0 for immediate task, > 0 for postponed tasks, unit is milliseconds
    osTimerId_t tid;               // only for postponed tasks
} errand_task_t;

static void timer_oneshot_callback(void *arg)
{
    errand_task_t *task = (errand_task_t *)arg;

    osStatus_t oss = osTimerDelete(task->tid); // delete this one-shot timer
    if (oss != osOK)
    {
        errand_dbg("[errand] osTimerDelete failed, err %d\n", oss);
    }

    task->errand_func(task->arg);

    free(task);
}

void errand_thread(void *arg)
{
    errand_task_t task = {0};
    errand_tid = osThreadGetId();
    if (errand_tid == NULL)
    {
        errand_dbg("[******** ERROR ********] errand_thread not launched\n");
    }
    errand_msgq = qErrandMgrSched;

    while (1)
    {
        osStatus_t oss = osMessageQueueGet(errand_msgq, &task, NULL, osWaitForever);
        if (oss != osOK)
        {
            errand_dbg("[errand] osMessageQueueGet error %d\n", oss);
            break;
        }

        if (task.errand_func == 0)
        {
            errand_dbg("[errand] user task function is NULL, skip it\n");
            continue;
        }

        if (task.execute_time == 0)
        {
            // immediate tasks

            task.errand_func(task.arg);
        }
        else
        {
            // for postponeds tasks

            errand_task_t *copied_task = (errand_task_t *)malloc(sizeof(errand_task_t));
            memcpy(copied_task, &task, sizeof(errand_task_t));

            osTimerId_t tid = osTimerNew(timer_oneshot_callback, osTimerOnce, copied_task, NULL);
            if (tid == NULL)
            {
                errand_dbg("[errand] osTimerNew failed, skip it\n");
                continue;
            }

            copied_task->tid = tid;

            osStatus_t oss = osTimerStart(tid, copied_task->execute_time);
            if (oss != osOK)
            {
                errand_dbg("[errand] osTimerStart failed\n");
            }
        }
    }

    errand_dbg("[errand] service stopped\n");
}

int kmdw_errandserv_start(void)
{
    /*errand_tid = osThreadNew(errand_thread, NULL, NULL);
    if (errand_tid == NULL)
    {
        errand_dbg("[******** ERROR ********] errand_thread not launched\n");
        return -1; // Ooops, thread creation gets failed !
    }

    errand_msgq = osMessageQueueNew(MAX_ERRAND_TASK_IN_Q, sizeof(errand_task_t), NULL);
    if (errand_msgq == NULL)
    {
        errand_dbg("[errand] osMessageQueueNew failed\n");
        return -2; // Ooops, message queue creation gets failed !
    }*/

    return 0;
}

int kmdw_errandserv_finish(void)
{
    // FIXME
    return 0;
}

int kmdw_errandserv_run_task(errand_function_t func, void *arg, uint32_t execute_time)
{
    errand_task_t task;

    task.errand_func = func;
    task.arg = arg;
    task.execute_time = execute_time;

    osStatus_t oss = osMessageQueuePut(errand_msgq, (const void *)&task, NULL, 0);
    if (oss != osOK)
    {
        errand_dbg("[errand] osMessageQueuePut failed err %d\n", oss);
        return -1;
    }

    return 0;
}
