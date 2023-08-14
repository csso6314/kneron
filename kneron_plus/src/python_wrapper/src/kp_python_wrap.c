/**
 * @file        kp_python_wrap.c
 * @brief       python wrapper
 * @version     0.1
 * @date        2021-04-28
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include "kp_python_wrap.h"
#include <stdlib.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C"{
#endif

void py_c_free(void* free_ptr) {
    if (NULL != free_ptr) {
        free(free_ptr);
    }
}

#ifdef __cplusplus
}
#endif
