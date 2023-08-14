/*
 * Kneron Main Entry driver
 *
 * Copyright (C) 2020 Kneron, Inc. All rights reserved.
 *
 */

#include <stdio.h>
#include "cmsis_os2.h" // ARM::CMSIS:RTOS2:Keil RTX5

#include "project.h"
#include "version.h"

// Customized configration and implimentation
#include "system_init.h"
#include "driver_init.h"
#include "device_init.h"
#include "middleware_init.h"
#include "application_init.h"

#include "kmdw_console.h"

extern void task_initialize(void);

/**
 * @brief main, main function
 */
int main(void)
{
    sys_initialize();
    drv_initialize();       /* customize driver     initialization, see driver_init.c */
    dev_initialize();       /* customize device     initialization, see device_init.c */
    mdw_initialize();       /* customize middleware initialization, see middlewre_init.c */

    osKernelInitialize();    // Initialize CMSIS-RTOS

    printf("SDK v%u.%u.%u-:build.%03u\n",
            (uint8_t)(IMG_FW_MAJOR),
            (uint8_t)(IMG_FW_MINOR),
            (uint8_t)(IMG_FW_UPDATE),
            (uint32_t)(IMG_FW_BUILD));

    app_initialize();       /* customize application initialization, see application_init.c */

    /* New task threads */
    task_initialize();

    /* Start RTOS Kernel */
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    while (1)
    {
    }
}
