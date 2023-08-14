/* --------------------------------------------------------------------------
 * Copyright (c) 2018-2019 Kneron Inc. All rights reserved.
 *
 *      Name:    main.c
 *      Purpose: Kneron NCPU
 *
 *---------------------------------------------------------------------------*/


#include "cmsis_os2.h"
#include "kdpio.h"

extern void SystemCoreClockUpdate(void);

/*----------------------------------------------------------------------------
 *      Main: Initialize OS Kernel and NCPU SDK
 *---------------------------------------------------------------------------*/
int main(void)
{
    SystemCoreClockUpdate();
    osKernelInitialize();

    /* init NCPU */
    kdpio_sdk_init();

    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    while (1)
        ;
}
