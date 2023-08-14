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

#include "project.h"

#include "kmdw_memory.h"
#include "kmdw_model.h"
#include "kmdw_dfu.h"

void mdw_initialize(void)
{
    kmdw_ddr_init(DDR_HEAP_BEGIN, DDR_HEAP_END);
    kmdw_ddr_store_system_reserve(DDR_SYSTEM_RESERVED_BEGIN, DDR_SYSTEM_RESERVED_END);
    kmdw_dfu_init(NULL, NULL);
    kmdw_model_init();

    // FW is loaded by fw_loader
    //load_ncpu_fw(1/*reset_flag*/);              // (kmdw_system.h) load ncpu fw from flash
}

