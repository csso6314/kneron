/*
 * Kneron Header for KDP on KL720
 *
 * Copyright (C) 2018-2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDPSYS_H__
#define __KDPSYS_H__

#include <stdint.h>
#include "ipc.h"

/**
 * @brief       set key information for NPU.
 *
 * @param[in]   mode            0: setup npu attribute without key for inpro, 1: setup npu attribute with key
 * @param[in]   model_p         pointer of model structure
 * @return      N/A
 */
void kdpio_set_key(uint32_t mode, kdp_model_t *model_p);

#endif
