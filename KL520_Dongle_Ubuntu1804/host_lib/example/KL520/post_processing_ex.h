/*
 * Kneron Example Post-Processing driver
 *
 * Copyright (C) 2018-2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef POST_PROCESSING_EX_H
#define POST_PROCESSING_EX_H
#include "ipc.h"

#define YOLO_GOOD_BOX_MAX       2000

// customized as number of detections changes for different prob_thresh_yolov3
typedef struct {
    uint32_t class_count; // total class count
    uint32_t box_count;   // boxes of all classes
    struct bounding_box_s boxes[YOLO_GOOD_BOX_MAX]; // box information
} dme_yolo_res;

/* IOU Methods */
enum IOU_TYPE {
    IOU_UNION = 0,
    IOU_MIN,
};

/* Structure of parameters for yolo */
struct post_parameter_s {
    unsigned int raw_input_row;
    unsigned int raw_input_col;
    unsigned int model_input_row;
    unsigned int model_input_col;
    unsigned int image_format;
};

/* Structure of output_node_info */
struct output_node_info {
    int addr;
    int height;
    int channel;
    int width;
    int radix;
    float scale;
};

/* Structure of output_node_params */
struct output_node_params {
    int height;
    int channel;
    int width;
    int radix;
    float scale;
};

#endif
