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
#include "project.h"
#if defined(FLASH_TYPE) && (FLASH_TYPE == FLASH_TYPE_NULL)
#include "kdev_flash_null.h"
#else
#include "kdev_flash.h"
#endif

//Function 
void dev_initialize(void)
{
#if !defined(_BOARD_SN720HAPS_H_)
	kdev_flash_initialize();
#endif
}

