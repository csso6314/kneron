/*
 * Kneron Header for KDP on KL520
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */

#ifndef POST_PROCESSING_H
#define POST_PROCESSING_H

#include "kdpio.h"

int post_processing(struct kdp_image_s *image_p, int model_id);

typedef struct host_od_post_params_s {
    float       prob_thresh;
    float       nms_thresh;
    uint32_t    max_detection_per_class;
    uint16_t    anchor_row;
    uint16_t    anchor_col;
    uint16_t    stride_size;
    uint16_t    reserved_size;
    uint32_t    data[];
} host_od_post_params_t;

#endif
