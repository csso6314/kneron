/*
 * Kneron Example Pre-Processing driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "base.h"
#include "kdpio.h"
#include "ipc.h"

inline static int pad_up_16(int a)
{
    return ceil((float)a / 16) * 16;
}

static void pre_proc_unsign_right_shift(uint8_t *src_p, uint8_t *dst_p, int row, int col, int bit_shift)
{
    int unit = 4;
    unsigned int r;
    int pad_col = pad_up_16(col);

    int len = pad_col * row * unit;
    for (r = 0; r < len; r++) {
        *(dst_p + r) = (*(src_p + r)) >> bit_shift;
    }
    return;
}

// This function is to right-shift the input RGBA image (HeightxWidthxChannel: 224x224x4) for 1 bit
int user_pre_yolo(struct kdp_image_s *image_p)
{
    int out_row, out_col;
    int input_radix, bit_shift;
    uint8_t *src_p, *dst_p;

    out_row = DIM_INPUT_ROW(image_p);
    out_col = DIM_INPUT_COL(image_p);

    input_radix = PREPROC_INPUT_RADIX(image_p);
    bit_shift = 8 - input_radix; // 1 byte (8 bits) for every R/G/B/A data

    src_p = (uint8_t *)RAW_IMAGE_MEM_ADDR(image_p);
    dst_p = (uint8_t *)PREPROC_INPUT_MEM_ADDR(image_p);
    pre_proc_unsign_right_shift(src_p, dst_p, out_row, out_col, bit_shift);

    return 0;
}

