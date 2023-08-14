/*
 * Kneron Header for KDP on KL520
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef CPU_OPS_H
#define CPU_OPS_H

#include "kdpio.h"

int kdp_do_cpu_ops(struct kdp_image_s *image_p);

/* Supported CPU node functions */
int cpu_op_nearest_upsample(struct kdp_image_s *image_p);
int cpu_op_zero_upsample(struct kdp_image_s *image_p);
#endif
