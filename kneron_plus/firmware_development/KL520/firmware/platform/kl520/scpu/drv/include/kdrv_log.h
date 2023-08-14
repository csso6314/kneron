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
#ifndef __KDRV_LOG_H__
#define __KDRV_LOG_H__

#include "kdrv_status.h"
#include "kdrv_uart.h"

/* The size of the buffers in pool and the number of buffers in buffer pool. */
#define LOG_BUF_SIZE                    256
#define LOG_BUF_NUM                     20

#define FLAGS_KDRV_PRINT_LOG_EVENTS     BIT0

/**
 * @brief       Push log message into log buffer 
 *
 * @param[in]   str             String of log message
 * @return      N/A
 *
 * @note        If bufferable logger thread doesn't be created,\n
 *              this log message will be directly wrote to UART via caller thread
 */
void kdrv_log_push(char *str);

/**
 * @brief       Initialize bufferable logger thread to asynchronize log print 
 *
 * @param[in]   handle          UART device handle which is used to print out log on console
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_logger_thread_create(kdrv_uart_handle_t handle);

#endif //__KDRV_LOG_H__
