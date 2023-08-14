/*
 * Copyright (c) 2018 by Cadence Design Systems. ALL RIGHTS RESERVED.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef IDMA_INTERNAL_H__
#define IDMA_INTERNAL_H__

#include "_idma.h"

#define IDMA_LOG_SIZE    200

#define IDMA_BUILD

#if XCHAL_NUM_DATARAM > 0
#define IDMA_DATARAM0_ADDR     XCHAL_DATARAM0_PADDR
#define IDMA_DATARAM0_SIZE     XCHAL_DATARAM0_SIZE
#else
#define IDMA_DATARAM0_ADDR     0
#define IDMA_DATARAM0_SIZE     0
#endif

#if XCHAL_NUM_DATARAM > 1
#define IDMA_DATARAM1_ADDR     XCHAL_DATARAM1_PADDR
#define IDMA_DATARAM1_SIZE     XCHAL_DATARAM1_SIZE
#else
#define IDMA_DATARAM1_ADDR     0
#define IDMA_DATARAM1_SIZE     0
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct idma_task_struct;

typedef struct idma_cntrl_struct {
    idma_buf_t*       oldest_task;
    idma_buf_t*       newest_task;
    uintptr_t         reg_base;
    uint32_t          timeout;
    uint32_t          settings;
    idma_err_callback_fn    err_cb_func;

    uint32_t          num_outstanding;
#ifdef IDMA_DEBUG
    idma_log_h        xlogh;
#endif
    uint8_t           initialized;
    idma_error_details_t  error;
} idma_cntrl_t;

#ifdef __cplusplus
}
#endif

/* Settings reg */
#define IDMA_MAX_BLOCK_SIZE_SHIFT	2
#define IDMA_MAX_BLOCK_SIZE_MASK	3U
#define IDMA_OUTSTANDING_REG_SHIFT	8U
#define IDMA_OUTSTANDING_REG_MASK	0x3FU
#define IDMA_HALT_ON_OCD_SHIFT		6U
#define IDMA_HALT_ON_OCD_MASK		1U
#define IDMA_FETCH_START		7U
#define IDMA_FETCH_START_MASK		1U

/* Timeout reg fields */
#define IDMA_TIMEOUT_CLOCK_SHIFT       0U
#define IDMA_TIMEOUT_CLOCK_MASK        7U
#define IDMA_TIMEOUT_THRESHOLD_SHIFT   3U
#define IDMA_TIMEOUT_THRESHOLD_MASK    0x1FFFFFFFU

#define IDMA_HAVE_TRIG_SHIFT          1
#define IDMA_HAVE_TRIG_MASK           0x2

#define IDMA_ERRCODES_SHIFT           18
#define IDMA_ERRORS_MASK              0xFFFC0000

#endif /* IDMA_INTERNAL_H__ */
