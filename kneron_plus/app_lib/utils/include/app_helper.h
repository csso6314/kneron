/**
 * @file        app_helper.h
 * @brief       Kneron PLUS APP library helper functions
 *
 * Helper functions for application examples are provided
 *
 * @version     0.1
 * @date        2021-08-05
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#include "kp_inference.h"
#include "kp_struct.h"

int get_image_size(kp_image_format_t format, int width, int height, uint32_t *image_size);
bool check_model_id_is_exist_in_nef(kp_device_group_t devices_grp, uint32_t model_id);
