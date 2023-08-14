/*
 * Kneron Header for KDP on KL520
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */

#pragma once

#include "kdpio.h"

typedef enum
{
    KP_DBG_PROFILE_BEFORE_PREPROCESS_STAMP = 1,
    KP_DBG_PROFILE_AFTER_PREPROCESS_STAMP = 2,
    KP_DBG_PROFILE_BEFORE_INFERENCE_STAMP = 3,
    KP_DBG_PROFILE_AFTER_INFERENCE_STAMP = 4,
    KP_DBG_PROFILE_BEFORE_POSTROCESS_STAMP = 5,
    KP_DBG_PROFILE_AFTER_POSTROCESS_STAMP = 6,
    KP_DBG_PROFILE_BEFORE_CPU_OP_STAMP = 7,
    KP_DBG_PROFILE_AFTER_CPU_OP_STAMP = 8,
} kp_dbg_profile_stamp_t;

void kdpio_dbg_checkpoint_pre_processing(uint32_t checkpoint_flag, struct kdp_image_s *image_p, int image_index /*unused param in KL520*/);
void kdpio_dbg_checkpoint_after_inference(uint32_t checkpoint_flag, struct kdp_image_s *image_p);
void kdpio_dbg_checkpoint_cpu_op(uint32_t checkpoint_flag, struct kdp_image_s *image_p, uint32_t cpu_node_idx /*unused param in KL520*/);
void kdpio_dbg_profile(kp_dbg_profile_stamp_t stamp, struct kdp_image_s *image_p);
