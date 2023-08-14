/*
 * Copyright (c) 2019 by Cadence Design Systems. ALL RIGHTS RESERVED.
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

#ifndef IDMA_OS_H_
#define IDMA_OS_H_

// IDMA OS interface API

#include <stdint.h>
#if defined(IDMA_USE_XTOS) || defined(IDMA_APP_USE_XTOS)
#include <xtensa/xtruntime.h>
#if XCHAL_HAVE_INTERRUPTS
#include <xtensa/tie/xt_interrupt.h>
#endif
#endif

// Typedef for OS interrupt handler function pointer. Hopefully this
// will work for most of not all RTOS that are relevant.
typedef void (*os_handler)(void * arg);

// Backward compatibility.
typedef os_handler os_int_handler_t;


#if defined(IDMA_USE_XTOS) || defined(IDMA_APP_USE_XTOS)

// API for XTOS. Mostly inlined for performance.
#error "dfd"
#ifdef __cplusplus
extern "C" {
#endif

__attribute__((always_inline)) static inline uint32_t
idma_disable_interrupts(void)
{
    return XTOS_SET_INTLEVEL(XCHAL_NUM_INTLEVELS);
}

__attribute__((always_inline)) static inline void
idma_enable_interrupts(uint32_t level)
{
    XTOS_RESTORE_INTLEVEL(level);
}

// Returns 0 on success, < 0 on error.
int32_t
idma_register_interrupts(int32_t    ch,
                         os_handler done_handler,
                         os_handler err_handler);

__attribute__((always_inline)) static inline void *
idma_thread_id(void)
{
    return NULL;
}

__attribute__((always_inline)) static inline void
idma_thread_block(void * thread) // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const in generic API"
{
    (void) thread;    // Unused

#if XCHAL_HAVE_XEA3
    XTOS_SET_INTLEVEL(0);
    XTOS_DISABLE_INTERRUPTS();
#endif
    XT_WAITI(0);
}

__attribute__((always_inline)) static inline void
idma_thread_unblock(void * thread) // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const in generic API"
{
    (void) thread;    // Unused
}

__attribute__((always_inline)) static inline void
idma_chan_buf_set(int32_t ch, idma_buf_t * buf)
{
    if (ch < XCHAL_IDMA_NUM_CHANNELS) {
        g_idma_buf_ptr[ch] = buf;
    }
}

__attribute__((always_inline)) static inline idma_buf_t *
idma_chan_buf_get(int32_t ch)
{
    if (ch < XCHAL_IDMA_NUM_CHANNELS) {
        return g_idma_buf_ptr[ch];
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif

#else

// Generic API for RTOS interfacing.

#ifdef __cplusplus
extern "C" {
#endif

// The opaque value returned from the disable function must be passed
// unchanged to the enable function.
uint32_t        idma_disable_interrupts(void);
void            idma_enable_interrupts(uint32_t level);
// Returns 0 on success, < 0 on error.
int32_t         idma_register_interrupts(int32_t    ch,
        os_handler done_handler,
        os_handler err_handler);
// Returns opaque value that must be passed unchanged to block/unblock.
void *          idma_thread_id(void);
void            idma_thread_block(void * thread);
void            idma_thread_unblock(void * thread);
void            idma_chan_buf_set(int32_t ch, idma_buf_t * buf);
// Returns previously set buffer ptr, or NULL.
idma_buf_t *    idma_chan_buf_get(int32_t ch);

#ifdef __cplusplus
}
#endif

#endif /* defined(IDMA_USE_XTOS) || defined(IDMA_APP_USE_XTOS) */

#endif /* IDMA_OS_H_ */

