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
//#include "project.h"
#include "kdrv_system.h"


 //Function
void sys_initialize(void)
{
    /* SDK main init for companion mode */
    kdrv_system_init();
    kdrv_system_init_ncpu();
}
