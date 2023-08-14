#pragma once

#include "ipc.h"

typedef struct
{
    int num_of_buffer;
    uint32_t buffer_addr[MAX_INPUT_NODE_COUNT];
    int length[MAX_INPUT_NODE_COUNT];
} buffer_object_t;
