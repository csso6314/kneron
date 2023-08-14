/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */

/******************************************************************************
*  Filename:
*  ---------
*  main.c
*
*  Description:
*  ------------
*
*
******************************************************************************/
#include <stdio.h>
#include "cmsis_os2.h"

#include "project.h"
#include "version.h"

// Customized configration and implimentation
#include "system_init.h"
#include "driver_init.h"
#include "device_init.h"
#include "middleware_init.h"
#include "application_init.h"

/* declare inference code implementation here */
extern void task_initialize(void);

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/


int main(void)
{
    sys_initialize();
    drv_initialize();       /* customize driver     initialization, see driver_init.c */
    dev_initialize();       /* customize device     initialization, see device_init.c */
    mdw_initialize();       /* customize middleware initialization, see middlewre_init.c */

    osKernelInitialize();

    printf("SDK v%u.%u.%u-:build.%03u\n",
            (uint8_t)(IMG_FW_VERSION >> 24),
            (uint8_t)(IMG_FW_VERSION >> 16),
            (uint8_t)(IMG_FW_VERSION >> 8),
            (uint8_t)(IMG_FW_VERSION));

    app_initialize();       /* customize application initialization, see application_init.c */

    /* New task threads */
    task_initialize();

    /* Start RTOS Kernel */
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    printf("fatal error: programmer counter should not be here\n");
    while (1);
}
