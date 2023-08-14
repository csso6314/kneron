#pragma once

#include "kdp2_ll.h"

typedef struct
{
    kdp2_ll_device_t *ll_device;
    int timeout;
} _kdp2_device_t;

/* Structure of output_node_params */
struct _fw_output_node_descriptor
{
    int height;
    int channel;
    int width;
    int radix;
    float scale;
};

#define ACK_RESPONSE_LEN 16

#define MAX_RAW_OUTPUT_NODE 50 // guess this number is enough !!!???

#define IMAGE_FORMAT_RAW_OUTPUT 0x10000000
#define IMAGE_FORMAT_BYPASS_POST 0x00010000 // not working

#define IMAGE_FORMAT_PARALLEL_PROC 0x08000000
#define IMAGE_FORMAT_SUB128 0x80000000
#define IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT 0x00400000

#define NPU_FORMAT_RGB565 0x60