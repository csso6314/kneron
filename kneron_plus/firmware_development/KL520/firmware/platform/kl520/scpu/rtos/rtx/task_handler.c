/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/
 //Include
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "task_handler.h"

 //Function
void task_initialize(void)
{
    uint32_t    index = 0;
    uint32_t    thread_err_flag = false;

    while (NULL != g_atKneronTaskPool[index].fpEntry)
    {
        osThreadAttr_t thread_attr;
        memset(&thread_attr, 0, sizeof(thread_attr));
        thread_attr.stack_size = g_atKneronTaskPool[index].dwStackSize;
        thread_attr.priority = g_atKneronTaskPool[index].dwPriority;
        *g_atKneronTaskPool[index].tTaskHandle = osThreadNew(g_atKneronTaskPool[index].fpEntry, NULL, &thread_attr);
        if (*g_atKneronTaskPool[index].tTaskHandle == NULL)
        {
            printf("[******** ERROR ********] Thread [%s] is not created.\n", g_atKneronTaskPool[index].caName);
            thread_err_flag = true;
            //return -1; // Ooops, thread creation gets failed !
        }

        if ( NULL != g_atKneronTaskPool[index].tQueueHandle) {
            osMessageQueueAttr_t msgq_attr;
            memset(&msgq_attr, 0, sizeof(msgq_attr));

            *g_atKneronTaskPool[index].tQueueHandle = osMessageQueueNew(g_atKneronTaskPool[index].tQmsg_count, g_atKneronTaskPool[index].tQmsg_size, NULL);
            if (*g_atKneronTaskPool[index].tQueueHandle == NULL)
            {
                printf("[******** ERROR ********] MessageQueue for thread [%s] is not created\n", g_atKneronTaskPool[index].caName);
                thread_err_flag = true;
            }
        }

        index ++;
    }

    if(true == thread_err_flag)
    {
        printf("\nmain() is terminated\n");
        __disable_irq();
        while(1);
    }
}
