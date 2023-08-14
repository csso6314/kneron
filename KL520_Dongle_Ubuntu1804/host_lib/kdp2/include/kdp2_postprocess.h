#pragma once

/**
 * @file      kdp2_postprocess.h
 * @brief     Post processing API
 *
 * A few post-processing functions are implemented here for specifc models.
 * 
 * @copyright (c) 2021 Kneron Inc. All right reserved.
 */

#include <stdint.h>
#include "kdp2_struct.h"

/**
 * @brief YOLO V3 post-processing function.
 *
 * @param node_output floating-point output node arrays, it should come from kdp2_raw_inference_retrieve_node().
 * @param num_output_node total number of output node.
 * @param img_width image width.
 * @param img_height image height.
 * @param yoloResult this is the yolo result output, users need to prepare a buffer of 'kdp2_yolo_result_t' for this.
 *
 * @return return 0 means sucessful, otherwise failed.
 * 
 */
int kdp2_post_process_yolo_v3(kdp2_node_output_t *node_output[], int num_output_node,
                              int img_width, int img_height, kdp2_yolo_result_t *yoloResult);
