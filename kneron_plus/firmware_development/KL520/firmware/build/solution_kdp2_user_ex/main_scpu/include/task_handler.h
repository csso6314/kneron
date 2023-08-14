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

/**@addtogroup  TASK_HANDLER
 * @{
 * @brief       Kneron System init
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef _TASK_HANDLER_H
#define _TASK_HANDLER_H
#include "cmsis_os2.h"
// #include "project.h"
#define USB_HOST
/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
typedef struct
{
    //parameters for creating tasks
    const char caName[8]; //now, len=8

    osThreadId_t        *tTaskHandle;
    osThreadFunc_t      fpEntry;
    const uint32_t      dwStackSize;
    osPriority_t        dwPriority;

    //parameters for creating queue
    osMessageQueueId_t  *tQueueHandle;
    const uint32_t      tQmsg_count;
    const uint32_t      tQmsg_size;
}T_S_KneronTask;

osThreadId_t task_log_handle;
osThreadId_t task_infdata_handle;
osThreadId_t task_infcb_handle;
osThreadId_t task_usb_recv_handle;
osThreadId_t task_usb_send_handle;
osThreadId_t task_buf_mgr_handle;

// put osMessageQueueId_t objects here for setting tQueueHandle

extern void logger_thread(void *arg);
extern void kmdw_inference_image_dispatcher_thread(void *argument);
extern void kmdw_inference_result_handler_callback_thread(void *argument);
extern void kdp2_usb_companion_image_thread(void *arg);
extern void kdp2_usb_companion_result_thread(void *arg);
extern void kdp2_fifoq_manager_enqueue_image_thread(void *arg);

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable
T_S_KneronTask g_atKneronTaskPool[]=
{
// TaskName     TaskHandle                   TaskFuncEntry                                   TaskStack   TaskPriority            QueueHandle         QueueMsgCount   QueueMsgSize
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  {"LogTask",   &task_log_handle,            logger_thread,                                  1024,       osPriorityBelowNormal,  NULL,                0,              0             },
  {"Infdata",   &task_infdata_handle,        kmdw_inference_image_dispatcher_thread,         2048,       osPriorityNormal,       NULL,                0,              0             },
  {"Infcb",     &task_infcb_handle,          kmdw_inference_result_handler_callback_thread,  1024,       osPriorityNormal,       NULL,                0,              0             },
  {"usbrecv",   &task_usb_recv_handle,       kdp2_usb_companion_image_thread,                1024,       osPriorityNormal,       NULL,                0,              0             },
  {"usbsend",   &task_usb_send_handle,       kdp2_usb_companion_result_thread,               1024,       osPriorityNormal,       NULL,                0,              0             },
  {"buf_mgr",   &task_buf_mgr_handle,        kdp2_fifoq_manager_enqueue_image_thread,        1024,       osPriorityNormal,       NULL,                0,              0             },

//
//Follow above format to add your TASK here
//


//end of table, don't remove it
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  {NULL,NULL,NULL,0,0,NULL,0,0}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
};

#endif

