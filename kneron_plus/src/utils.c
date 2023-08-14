/**
 * @file        utils.c
 * @brief       internal utils functions
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

// #define DEBUG_PRINT

#include "internal_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }
#else
#define dbg_print(format, ...)
#endif

#define err_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }

void* realloc_zero(void* memory, size_t new_size) {
    /**
     * realloc memory and memset data to '0'
     * 
     * note: if realloc size is zero, the memory will be free.
     * note: return realloc memory address. return NULL when realloc memory fail or realloc size is zero.
     */

    void* _NewMemory = NULL;

    if (0 < new_size) {
        _NewMemory = realloc(memory, new_size);

        if (NULL == _NewMemory) {
            err_print("[%s] realloc memory fail, line %d.\n", __FUNCTION__, __LINE__);
            return _NewMemory;
        }

        _NewMemory = memset(_NewMemory, 0, new_size);
    } else if (0 == new_size && NULL != memory) {
        free(memory);
    }

    return _NewMemory;
}

void* strcpy_dst_realloc(char* dst_buff, const char* src_buff) {
    if (NULL == src_buff) {
        err_print("[%s] src_buff is NULL, line %d.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    char* new_dst_buff = realloc_zero(dst_buff, strlen(src_buff) + 1);

    if (NULL == new_dst_buff) {
        err_print("[%s] realloc memory fail, line %d.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    strcpy(new_dst_buff, src_buff);

    return new_dst_buff;
}
