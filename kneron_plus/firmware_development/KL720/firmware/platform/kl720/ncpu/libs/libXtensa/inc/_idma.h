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

#ifndef IDMA_H__
#define IDMA_H__

#include <xtensa/hal.h>
#include <xtensa/xtensa-types.h>
#include <xtensa/config/core-isa.h>
#if XCHAL_HAVE_EXTERN_REGS
#include <xtensa/tie/xt_externalregisters.h>
#endif


#ifndef XCHAL_UNIFIED_LOADSTORE
# define XCHAL_UNIFIED_LOADSTORE        0
#endif

#ifndef XCHAL_IDMA_ADDR_WIDTH
# define XCHAL_IDMA_ADDR_WIDTH          32
#endif

/* If HAL doesn't define # of channels macro, need to set to 1 */
#ifndef XCHAL_IDMA_NUM_CHANNELS
# define XCHAL_IDMA_NUM_CHANNELS        1
#endif

/* User enabled multichannel mode - use LIBIDMA_USE_MULTICHANNEL */
#if (defined IDMA_USE_MULTICHANNEL)
# define LIBIDMA_USE_MULTICHANNEL       1
#else
# define LIBIDMA_USE_MULTICHANNEL       0
#endif

/* Use multichannels only if user-enabled */
#if (LIBIDMA_USE_MULTICHANNEL > 0)
# define LIBIDMA_USE_MULTICHANNEL_API   1
#else
# define LIBIDMA_USE_MULTICHANNEL_API   0
#endif

/* Enable large (64-byte) descriptor API if the hardware supports it. */
#if (XCHAL_UNIFIED_LOADSTORE > 0)
# define IDMA_USE_64B_DESC              1
#else
# define IDMA_USE_64B_DESC              0
#endif

/* Enable wide address API if the hardware supports it. */
#if (XCHAL_IDMA_ADDR_WIDTH > 32) && (XCHAL_IDMA_ADDR_WIDTH <= 64)
# define IDMA_USE_WIDE_API              1
#else
# define IDMA_USE_WIDE_API              0
#endif

#if (IDMA_USE_64B_DESC == 0) && (IDMA_USE_WIDE_API > 0)
/* #error "IDMA_USE_WIDE_API requires a config that supports 64-byte descriptors" */
#endif

/* This is INTERNAL. Need to be here as headers use it */
#define PSO_SAVE_SIZE_PER_CHANNEL       6
#define IDMA_PSO_SAVE_SIZE              PSO_SAVE_SIZE_PER_CHANNEL*XCHAL_IDMA_NUM_CHANNELS


// For the old 2-channel implementation, ch0 and ch1 are at different
// base addresses. For the newer implementation, all the channels can
// be accessed at either of the two base addresses. We choose to use
// the first (device1) base address.
#if (XCHAL_IDMA_NUM_CHANNELS == 2) && (IDMA_USE_64B_DESC == 0)
# define IDMAREG_BASE_CH0               UINT32_C(0x00110000)
# define IDMAREG_BASE_CH1               UINT32_C(0x00130400)
# define IDMAREG_BASE(n)                ((n) == 0 ? IDMAREG_BASE_CH0 : IDMAREG_BASE_CH1)
#else
# define IDMAREG_BASE_SDEV1             UINT32_C(0x00110000)
# define IDMAREG_BASE(n)                (IDMAREG_BASE_SDEV1 + ((UINT32_C(0x400)) * (n)))
#endif

/* iDMA registers offsets */
#define IDMA_REG_SETTINGS               0x00
#define IDMA_REG_TIMEOUT                0x04
#define IDMA_REG_DESC_START             0x08
#define IDMA_REG_NUM_DESC               0x0C
#define IDMA_REG_DESC_INC               0x10
#define IDMA_REG_CONTROL                0x14
#define IDMA_REG_USERPRIV               0x18
#define IDMA_REG_STATUS                 0x40
#define IDMA_REG_CURR_DESC              0x44
#define IDMA_REG_DESC_TYPE              0x48
#define IDMA_REG_SRC_ADDR               0x4C
#define IDMA_REG_DST_ADDR               0x50

// PSO status, after core save/restore
// Value at IDMA_PSO_STATUS_OFFSET from at XtosSaveState.idmaregs
// will indicate if idma HW was idle prior to save, so save was
// skipped, or there was forced iDMA HW disable, or save was done.
#define IDMA_PSO_STATUS_OFFSET          5
#define IDMA_PSO_STATUS_SAVED           0x1
#define IDMA_PSO_STATUS_DISABLED        0x2
#define IDMA_PSO_STATUS_IDLE            0x3

#if !defined(_ASMLANGUAGE) && !defined(__ASSEMBLER__)

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* Typedef for internal use (required by idma_os.h) */
typedef struct idma_buf_struct idma_buf_t;

/* libidma internal buffer ptr, used in Fixed-Buffer mode */
extern idma_buf_t * g_idma_buf_ptr[XCHAL_IDMA_NUM_CHANNELS];

/**
 * Use inline XTOS interface if application defines IDMA_APP_USE_XTOS or
 * library build defines IDMA_USE_XTOS. Else use libidma OS interface.
 * Both are defined in idma_os.h.
 */
#include "idma_os.h"    /* parasoft-suppress MISRA2012-RULE-20_1-4 "This has to follow idma_buf_t" */

/**
 * Choose the memory - DRAM1 or DRAM0. Default is DRAM0
 */
#if !defined(IDMA_USE_DRAM1) && !defined(IDMA_USE_DRAM0)
#define IDMA_USE_DRAM0
#endif

/**
 * Use interrupts in APIs defined in the header. Default is to use interrupts.
 * Overhead of enabling/disabling interrupts to guard shared structs
 */
#if !defined(IDMA_USE_INTR)
#define IDMA_USE_INTR 1
#endif

/******************************************************/
/****     Initialization flags, types, defines     ****/
/******************************************************/

/**
 * iDMAlib and iDMA HW operation flags, for idma_init() 1st arg.
 */
#define IDMA_OCD_HALT_ON       0x001  /* Enable iDMA halt on OCD interrupt. */

/**
* Type of descriptor to add. NOTE: value reflects multiple of 16B.
*/
typedef enum {
    IDMA_1D_DESC   = 1,
    IDMA_2D_DESC   = 2,
    IDMA_64B_DESC  = 4
} idma_type_t;

/**
 * Maximum allowed PIF request block size.
 */
typedef enum {
    MAX_BLOCK_2  = 0,
    MAX_BLOCK_4  = 1,
    MAX_BLOCK_8  = 2,
    MAX_BLOCK_16 = 3
} idma_max_block_t;

/**
 * TICK_CYCLES_N: iDMA HW internal timer ticks every N cycles.
 */
typedef enum {
    TICK_CYCLES_1  =  0,   /* internal timer ticks every 1 cycle */
    TICK_CYCLES_2  =  1,   /* internal timer ticks every 2 cycles */
    TICK_CYCLES_4  =  2,
    TICK_CYCLES_8  =  3,
    TICK_CYCLES_16  = 4,
    TICK_CYCLES_32  = 5,
    TICK_CYCLES_64  = 6,
    TICK_CYCLES_128 = 7
} idma_ticks_cyc_t;

/**
 * "Descriptor Control Flags" for any type of descriptor
 * This sets the idma descriptor control field - see HW description.
 */
#define DESC_IDMA_NOPRIV_SRC      0x00200     /* Non-privileged source memory access */
#define DESC_IDMA_NOPRIV_DST      0x00800     /* Non-privileged dest. memory access */
#define DESC_IDMA_PRIOR_H         0x08000     /* QoS high */
#define DESC_IDMA_PRIOR_L         0x00000     /* QoS low */
#define DESC_IDMA_TRIG_WAIT       0x20000000  /* wait for external trigger to start */
#define DESC_IDMA_TRIG_OUT        0x40000000  /* send trigger out on desc. completion */
#define DESC_NOTIFY_W_INT         0x80000000  /* trigger interrupt on completion */
// "Descriptor Control Flags" for old 1-D and 2-D descriptors only
#define DESC_IDMA_TRANSPOSE       0x00010     /* do the 2D transpose (if HW supports it) */
#define DESC_IDMA_SIZE_1B         0x00000     /* Granularity - 1Byte to 8Bytes */
#define DESC_IDMA_SIZE_2B         0x00020
#define DESC_IDMA_SIZE_4B         0x00040
#define DESC_IDMA_SIZE_8B         0x00060
//* "Descriptor Control Flags" for new 64-Byte descriptors only.
#define DESC64_IDMA_OVERLAP       0x00000008  /* Overlap Bit (bit 3)        */
#define DESC64_IDMA_LONG_ADDR     0x00000400  /* LA – Long Address (bit 10) */

// "Descriptor Control Flags" AXI attribute bits for 64-byte descriptors.
// These go into bits 27:24 of the descriptor control word.
#define DESC64_IDMA_AXI_ATTRIBUTE(val)    (((val) & 0xF) << 24)

/**
 * iDMA API return values
 * For most of the iDMAlib API calls
 */
typedef enum {
    IDMA_ERR_NO_BUF = -40,          /* No valid ring buffer */
    IDMA_ERR_BAD_DESC = -20,        /* Descriptor not correct */
    IDMA_ERR_BAD_CHAN,              /* Invalid channel number */
    IDMA_ERR_NOT_INIT,              /* iDMAlib and HW not initialized  */
    IDMA_ERR_TASK_NOT_INIT,         /* Cannot scheduled uninitialized task  */
    IDMA_ERR_BAD_TASK,              /* Task not correct  */
    IDMA_ERR_BUSY,                  /* iDMA busy when not expected */
    IDMA_ERR_IN_SPEC_MODE,          /* iDMAlib in unexpected mode */
    IDMA_ERR_NOT_SPEC_MODE,         /* iDMAlib in unexpected mode */
    IDMA_ERR_TASK_EMPTY,            /* No descs in the task/buffer */
    IDMA_ERR_TASK_OUTSTAND_NEG,     /* Number of outstanding descs is a negative value  */
    IDMA_ERR_TASK_IN_ERROR,         /* Task in error */
    IDMA_ERR_BUFFER_IN_ERROR,       /* Buffer in error */
    IDMA_ERR_NO_NEXT_TASK,          /* Next task to process is missing  */
    IDMA_ERR_BUF_OVFL,              /* Attempt to schedule too many descriptors */
    IDMA_ERR_HW_ERROR,              /* HW error detected */
    IDMA_ERR_BAD_INIT,              /* Bad idma_init args */
    IDMA_OK = 0,                    /* No error */
    IDMA_CANT_SLEEP = 1,            /* Cannot sleep (no pending descriptors) */
} idma_status_t;

/**
 * iDMA task status API return values
 * NOTE: Valid only after task is scheduled
 *
 * N (>=0)            - Number of outstanding scheduled descriptors.
 * 0 (IDMA_TASK_DONE) - Whole task has completed the execution.
 * IDMA_TASK_ERROR    - iDMA HW error happened due to a descriptor executed
 *                      from this task. Error details are available (idma_hw_error_t).
 * IDMA_TASK_ABORTED  - Task forcefully aborted.
 */
typedef enum {
    IDMA_TASK_ABORTED = -3,
    IDMA_TASK_ERROR   = -2,
    IDMA_TASK_NOINIT  = -1,
    IDMA_TASK_DONE    =  0,
    IDMA_TASK_EMPTY   =  0
} task_status_t;

/**
 * iDMA HW error details API return values
 * This corresponds to the Error Codes field
 * of the HW Status register.
 */
#define IDMA_ERR_FETCH_ADDR      UINT32_C(0x2000)
#define IDMA_ERR_FETCH_DATA      UINT32_C(0x1000)
#define IDMA_ERR_READ_ADDR       UINT32_C(0x0800)
#define IDMA_ERR_READ_DATA       UINT32_C(0x0400)
#define IDMA_ERR_WRITE_ADDR      UINT32_C(0x0200)
#define IDMA_ERR_WRITE_DATA      UINT32_C(0x0100)
#define IDMA_ERR_REG_TIMEOUT     UINT32_C(0x0080)
#define IDMA_ERR_TRIG_OVFL       UINT32_C(0x0040)
#define IDMA_ERR_DESC_OVFL       UINT32_C(0x0020)
#define IDMA_ERR_DESC_UNKNW      UINT32_C(0x0010)
#define IDMA_ERR_DESC_UNSUP_DIR  UINT32_C(0x0008)
#define IDMA_ERR_DESC_BAD_PARAMS UINT32_C(0x0004)
#define IDMA_ERR_DESC_NULL_ADDR  UINT32_C(0x0002)
#define IDMA_ERR_DESC_PRIVILEGE  UINT32_C(0x0001)
#define IDMA_NO_ERR              UINT32_C(0x0000)

typedef uint32_t idma_hw_error_t;

/* Status reg fields */
#define   IDMA_STATE_IDLE        UINT32_C(0x0)  //idma done with all descriptors and disabled
#define   IDMA_STATE_STANDBY     UINT32_C(0x1)  //transient stat
#define   IDMA_STATE_BUSY        UINT32_C(0x2)  //idma busy copying
#define   IDMA_STATE_DONE        UINT32_C(0x3)  //idma done with all descriptors but enabled
#define   IDMA_STATE_HALT        UINT32_C(0x4)  //idma disabled during copy operation
#define   IDMA_STATE_ERROR       UINT32_C(0x5)  //idma in error
#define   IDMA_STATE_MASK        UINT32_C(0x7)

typedef uint32_t idma_state_t;

# ifdef IDMA_DEBUG
#   define INTERNAL_FUNC static
# else
#   define INTERNAL_FUNC static inline
# endif

/* No inlining for debug libs */
#ifdef IDMA_LIB_BUILD
# define IDMA_API static inline
#else
# define IDMA_API INTERNAL_FUNC
#endif

#define ALWAYS_INLINE   __attribute__((always_inline)) static inline

#ifndef IDMA_EXTERN
# define IDMA_EXTERN
#endif


#ifdef __cplusplus
extern "C" {
#endif

/**
 * iDMA error details structure - available from different API calls
 */
typedef struct idma_error_details_struct {
    idma_hw_error_t  err_type;       /* ErrorCodes field of the Status reg */
    uint32_t         currDesc;       /* Descriptor causing error           */
    uint32_t         srcAddr;        /* PIF source address causing error   */
    uint32_t         dstAddr;        /* PIF dest. address causing error    */
} idma_error_details_t;

/* callback typedef */
typedef void (*idma_callback_fn)( void* arg);
/* error callback typedef */
typedef void (*idma_err_callback_fn)( const idma_error_details_t* error);
/* log handler typedef */
typedef void (*idma_log_h)( const char* xlog);

/* Typedef for external API use */
typedef struct idma_buffer_struct {
    char buffer[4];
} idma_buffer_t;

/* allocate space for n descriptors of type idma_type_t. */
#define IDMA_BUFFER_SIZE(n,type)  \
                        (  ((n)* (((type) == IDMA_1D_DESC) ? IDMA_1D_DESC_SIZE : (((type) == IDMA_2D_DESC) ? IDMA_2D_DESC_SIZE : IDMA_64_DESC_SIZE))) + \
			sizeof(struct idma_buf_struct))

/* Define a buffer of containing n descriptors of type n */
#define IDMA_BUFFER_DEFINE(name, n, type)  \
                           IDMA_DRAM idma_buffer_t name[IDMA_BUFFER_SIZE((n), (type))/sizeof(idma_buffer_t)]

/* Use these macros when allocating copy buffers to avoid sharing cache lines: */
//Align to cache line
#define ALIGNDCACHE        __attribute__ ((aligned(XCHAL_DCACHE_LINESIZE)))
//Buffer's size will be rounded to the cache line size so the buffer occupies the whole cache line
#define IDMA_SIZE(N)       (((N) + XCHAL_DCACHE_LINESIZE-1) & -XCHAL_DCACHE_LINESIZE)

/* These macros are empty if no interrupts. Otherwise, they map to OS provided
 * functions that are defined in idma-xtos.h or idma-os.h. The XTOS versions
 * will get inlined for performance.
 */
#if defined XCHAL_HAVE_INTERRUPTS && (IDMA_USE_INTR > 0)
# define DECLARE_PS()           uint32_t ps
# define IDMA_ENABLE_INTS()     idma_enable_interrupts(ps)
# define IDMA_DISABLE_INTS()    ps = idma_disable_interrupts()
#else //no interrupts
# define DECLARE_PS()           do {} while (0)
# define IDMA_ENABLE_INTS()     do {} while (0)
# define IDMA_DISABLE_INTS()    do {} while (0)
#endif

/*************************************************/
/**  Forward declarations for internal stuff    **/
/*************************************************/
typedef struct idma_desc_struct idma_desc_t;
typedef struct idma_2d_desc_struct idma_2d_desc_t;
typedef struct idma_desc64_struct idma_desc64_t;

/************************************************/
/****        MISC HELPER FUNCTIONS           ****/
/************************************************/

ALWAYS_INLINE void *
cvt_uint32_to_voidp(uint32_t val)
{
    return (void *) val;                // parasoft-suppress MISRA2012-RULE-11_6-2 "Type conversion necessary."
}

ALWAYS_INLINE uint32_t
cvt_voidp_to_uint32(void * val)         // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const because of type conversion."
{
    return (uint32_t) val;              // parasoft-suppress MISRA2012-RULE-11_6-2 "Type conversion necessary."
}

ALWAYS_INLINE void *
cvt_uint32p_to_voidp(uint32_t * val)    // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const because of type conversion."
{
    return (void *) val;
}

ALWAYS_INLINE uint32_t *
cvt_voidp_to_uint32p(void * val)        // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const because of type conversion."
{
    return (uint32_t *) val;            // parasoft-suppress MISRA2012-RULE-11_5-4 "Type conversion necessary."
}

ALWAYS_INLINE void *
cvt_int32_to_voidp(int32_t val)
{
    return (void *) val;                // parasoft-suppress MISRA2012-RULE-11_6-2 "Type conversion necessary."
}

ALWAYS_INLINE int32_t
cvt_voidp_to_int32(void * val)          // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const because of type conversion."
{
    return (int32_t) val;               // parasoft-suppress MISRA2012-RULE-11_6-2 "Type conversion necessary."
}

/************************************************/
/****                  API                   ****/
/************************************************/

/* NOTE NOTE NOTE:
 * Have to define LIBIDMA_USE_MULTICHANNEL if want to use the multichannel API.
 * The new API basically adds the channel argument to the most of the old functions.
 * See below, #else part, for the old API. Also, each function descriptions mentions
 * if an argument is not present in the old API.
 */
#if (LIBIDMA_USE_MULTICHANNEL_API > 0)

/**
 * @name    Wait for all descriptors to finish, by polling only the HW
 * @param   ch    Selected iDMA HW channel.
 *                NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @retval  None
 */
IDMA_API void
idma_hw_wait_all(int32_t ch);

/**
 * @name   Schedule a number of descriptors, by accessing HW only.
 * @param  ch         Selected iDMA HW channel.
 *                    NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  count      Number of descriptors to schedule.
 * @retval None
 */
IDMA_API void
idma_hw_schedule(int32_t ch, uint32_t count);

/**
 * @name   Return number of outstanding descriptor, as indicated by HW
 * @param  ch  Selected iDMA HW channel.
 *             NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @retval number of outstanding descriptor
 */
IDMA_API uint32_t
idma_hw_num_outstanding(int32_t ch);

/**
 * @name   iDMAlib and iDMA HW initialization
 * @brief  Does iDMA HW soft reset, sets iDMAlib parameters and sets
 *         iDMA HW registers (Settings/Timeout) with given parameters.
 * @param  ch                   Selected. iDMA HW channel.
 *                              NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  flags                iDMAlib initialization flags, above.
 * @param  block_sz             idma_max_block_t above.
 * @param  pif_req              Maximum outstanding PIF requests (1-64).
 * @param  ticks_per_cyc        Ticks per cycle. See iDMA hardware details.
 * @param  timeout_ticks        Timeout ticks. See iDMA hardware details.
 * @param  err_cb_func          Register global error handler. Called on idma HW
 *                              error detection for which intr is always enabled.
 * @retval IDMA_OK              Successful
 * @retval !IDMA_OK             Error type
 */
IDMA_API idma_status_t
idma_init( int32_t ch,
           uint32_t         flags,
           idma_max_block_t block_sz,
           uint32_t         pif_req,
           idma_ticks_cyc_t ticks_per_cyc,
           uint32_t         timeout_ticks,
           idma_err_callback_fn err_cb_func);

/**
 * @name   Check the state of idma HW.
 * @param  ch    Selected iDMA HW channel.
 *               NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 */
IDMA_API idma_state_t
idma_get_state(int32_t ch);

/**
 * @name   Add log messages handler.
 * @brief  Log messages from iDMAlib are sent to this handler.
 *         NOTE: In debug library only !
 * @param  xlog         Function to call on iDMAlib log call
 */
void
idma_log_handler(idma_log_h xlog);

/**
 * @name   Sleep/block until current iDMA activity completes.
 * @brief  Wait in low-power mode until iDMA activity completes. Return immediately
 *         if iDMA is in error or there are no outstanding descriptors.
 * @param   ch   Selected iDMA HW channel.
 *               NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @retval IDMA_OK              Normal return.
 * @retval IDMA_CANT_SLEEP      Cannot sleep (no pending descriptors).
 * @retval (all other values)   Error code.
 */
IDMA_API idma_status_t
idma_sleep(int32_t ch);

/**
 * @name   Get the error details.
 * @brief  When task or fixed buffer status indicate error, use this
 *         function to obtain the error details.
 * @param   ch    Selected iDMA HW channel.
 *                NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  task             Pointer to iDMA task
 * @retval Pointer to the structure containing error details.
 *         NOTE: Pointer is valid only if idma task/buffer is in error.
 */
IDMA_API idma_error_details_t*   idma_error_details(int32_t ch);
IDMA_API idma_error_details_t*   idma_buffer_error_details(int32_t ch);

/**
 * @name   Disable iDMA HW when not in use.
 * @param  ch    Selected iDMA HW channel.
 *               NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @brief  Needs idma_init to resume.
 */
IDMA_API idma_status_t
idma_stop(int32_t ch);

/**
 * @name   Pause iDMA HW.
 * @param  ch    Selected iDMA HW channel.
 *               NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @brief  Can simply resume after
 */
IDMA_API void
idma_pause(int32_t ch);

/**
 * @name   Resume iDMA HW.
 * @param  ch    Selected iDMA HW channel.
 *               NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @brief  Can simply pause before
 */
IDMA_API void
idma_resume(int32_t ch);

/**
 * @name   Add 1D desc to the next location in the buffer/task
 * @brief  Descriptors are added in order. Used in both Task and
 *         and the fixed-buffer mode.
 *         NOTE: Number of added descriptors needs to match the
 *         size of the task (an argument to idma_init_task()).
 * @param  dst...     iDMA 1D transfer parameters
 * @param  flags      Descriptor options (Control field).
 *                    See "Descriptor Control Flags".
 * @retval IDMA_OK    Successful.
 * @retval !IDMA_OK   Error type.
 */
IDMA_API idma_status_t
idma_add_desc( idma_buffer_t *bufh,
               void *dst,
               void *src,
               size_t size,
               uint32_t flags);

/**
 * @name   Add 2D desc to the next location in the buffer/task
 * @brief  Descriptors are added in order. Used in both Task and
 *         and the fixed-buffer mode.
 *         NOTE: Number of added descriptors needs to match the
 *         size of the task (an argument to idma_init_task()).
 * @param  dst...    iDMA 1D transfer parameters
 * @param  flags      Descriptor options (Control field).
 *                    See "Descriptor Control Flags".
 * @retval IDMA_OK    Successful.
 * @retval !IDMA_OK   Error type.
 */
IDMA_API idma_status_t
idma_add_2d_desc( idma_buffer_t *bufh,
                  void *dst,
                  void *src,
                  size_t row_sz,
                  uint32_t flags,
                  uint32_t nrows,
                  uint32_t src_pitch,
                  uint32_t dst_pitch);

/************************************************/
/****               Task-mode API            ****/
/************************************************/

/**
 * @name   Initialize a task (a separate buffer/array of descriptors).
 * @brief  Needs to be called before adding descriptors to a task.
 * @param   ch       Selected iDMA HW channel.
 *                   NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  task      Pointer to the memory used for iDMA task.
 * @param  type      1D or 2D.
 * @param  num       Number of descriptors to be added. NOTE: must match
 *                   the actual number of added descs using  _add_ calls.
 *                   Also, IDMA_BUFFER_DEFINE must allocate sufficent memory.
 * @param  cb_func   Function to call on each descriptor completion.
 *                   (if enabled by descriptor).
 * @param  cb_data   Callback data.
 * @retval IDMA_OK   Successful.
 * @retval !IDMA_OK  Error type
 */
IDMA_API idma_status_t
idma_init_task (int32_t ch,
                idma_buffer_t *taskh,
                idma_type_t type,
                int32_t ndescs,
                idma_callback_fn cb_func,
                void *cb_data);

/**
 * @name   Get the task execution status.
 * @brief  Reads the number of outstanding descriptors from the
 *         scheduled task. When no iDMA interrupts, must be used
 *         together with idma_process_tasks() as something must
 *         trigger iDMAlib internal task completion processing.
 * @param  task   Pointer to iDMA task
 * @retval <  0   Error (task_status_t)
 * @retval >= 0   Number of outstanding descriptors in the task.
 */
IDMA_API int32_t
idma_task_status(idma_buffer_t *taskh);

/**
 * @name   Create and schedule 1D copy request.
 * @brief  Convenience functions that combine buffer initialization,
 *         adding one descriptor to it and scheduling the task.
 * @param  ch          Selected iDMA HW channel.
 *                     NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  task        Pointer to iDMA buffer
 * @param  cb_func     Function to call on each descriptor completion
 *                     (if enabled by descriptor)
 * @param  cb_data     Callback data.
 * @retval IDMA_OK     Successful.
 * @retval !IDMA_OK    Error type
 */
IDMA_API idma_status_t
idma_copy_task(int32_t ch,
               idma_buffer_t *taskh,
               void *dst,
               void *src,
               size_t size,
               uint32_t flags,
               void *cb_data,
               idma_callback_fn cb_func);

/**
 * @name   Create and schedule 2D copy request.
 * @brief  Convenience functions that combine buffer initialization,
 *         adding one descriptor to it and scheduling the task.
 * @param  ch          Selected iDMA HW channel.
 *                     NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  task        Pointer to iDMA buffer
 * @param  cb_func     Function to call on each descriptor completion
 *                     (if enabled by descriptor)
 * @param  cb_data     Callback data.
 * @retval IDMA_OK     Successful.
 * @retval !IDMA_OK    Error type
 */
IDMA_API idma_status_t
idma_copy_2d_task(int32_t ch,
                  idma_buffer_t *taskh,
                  void *dst,
                  void *src,
                  size_t row_sz,
                  uint32_t flags,
                  uint32_t nrows,
                  uint32_t src_pitch,
                  uint32_t dst_pitch,
                  void *cb_data,
                  idma_callback_fn cb_func);

/**
 * @name   Schedule a task for execution.
 * @brief  Schedule a task(separate buffer/array ofdescriptor)
 *         for execution in the task mode.  If IDMA is busy,
 *         last desc from the last scheduled buffer is linked
 *         with the 1st desc from this task using a JUMP command.
 *         If IDMA is not busy iDMA HW starts execution from this task.
 *         NOTE: TASK MODE ONLY.
 * @param  buf        Pointer to iDMA task to be scheduled.
 * @retval IDMA_OK    Successful.
 * @retval !IDMA_OK   Error type.
 */
idma_status_t
idma_schedule_task(idma_buffer_t *taskh);

/**
 * @name   Trigger iDMAlib internal processing of completed tasks.
 * @brief  Needed when no interrupts are available/setup.
 *         NOTE: Function will also invoke completion callback,
 *         if set with idma_init_task() function.
 * @param  ch         Selected iDMA HW channel.
 *                    NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @retval IDMA_OK    Successful.
 * @retval !IDMA_OK   Error type (when iDMA is in error).
 */
IDMA_API idma_status_t
idma_process_tasks(int32_t ch);

/**
 * @name   Abort all tasks.
 * @brief  Reset iDMA HW and set status of all "outstanding" tasks to
 *         aborted state. Note: "Outstanding" tasks can also include
 *         tasks that actually completed (their descriptors are all executed).
 *         To ensure all the completed tasks are not seen as "outstanding",
 *         enable iDMA Done interrupt for at least the last descriptor in
 *         a task, or call idma_process_tasks() before calling this function.
 * @param  ch          Selected iDMA HW channel.
 *                     NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @retval IDMA_OK     Successful.
 * @retval !IDMA_OK    Error type
 */
IDMA_API idma_status_t
idma_abort_tasks(int32_t ch);

/************************************************/
/****         Fixed-Buffer Mode API          ****/
/************************************************/

/**
 * @name   Initialize buffer (array of descs). Also enables the fixed-buffer
 *         mode which prevents using the buffer as a separate task.
 * @brief  Needs to be called after a buffer is created (e.g. using the
 *         IDMA_DEFINE_BUFFER) and before adding descriptors to it.
 * @param  ch         Selected iDMA HW channel.
 *                    NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  buffer     Pointer to the memory used for iDMA buffer
 * @param  type       1D or 2D
 * @param  ndescs     Number of descriptors. In the TASK mode, the number
 *                    of descs added to the task need to much this number.
 * @param  cb_func    Function to call on each descriptor completion.
 *                    (if enabled by each descriptor settings)
 * @param  cb_data    Callback data
 * @retval IDMA_OK    Successful.
 * @retval !IDMA_OK   Error type
 */
IDMA_API idma_status_t
idma_init_loop (int32_t ch,
                idma_buffer_t *bufh,
                idma_type_t type,
                int32_t ndescs,
                void *cb_data,
                idma_callback_fn cb_func);

/**
 * @name   Schedule consecutive descriptors for execution.
 * @brief  Schedule a number of next in line descriptors, with wrap-around
 *         NOTE: INCOMPATIBLE with idma_copy_desc/idma_copy_2d_desc
 * @param  ch       Selected iDMA HW channel.
 *                  NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  count    Number of descriptors to schedule.
 *                  NOTE: Cannot be larger than the number of descriptors
 *                  in the buffer initialized using idma_init_loop().
 * @retval <  0     Error code, from idma_status_t.
 * @retval >= 0     Unique index of the scheduled descriptor in the range from 0
 *                  to 0x7fffffff. Starts with value of 1 at iDMAlib initialization
 *		    time. It's expected the number of outstanding descriptors in
 *		    the system is always less then 0x7fffffff.
 *                  E.g. if the circular buffer contains 8 descs and retval=17,
 *                  the 2nd descriptor in the buffer is the last scheduled one and
 *                  by, the iDMAlib has scheduled  17 descriptors.
 *                  NOTE: This unique index is to be used in idma_desc_done() only.
 */
IDMA_API int32_t
idma_schedule_desc(int32_t ch,
                   uint32_t count);

/**
 * @name   Schedule consecutive descriptors for execution w/o ability to change descs.
 * @brief  Schedule a number of next in line descriptors, with wrap-around.
 *         The function is to be used when all the descriptors are already added
 *         so they are just scheduled or possibly updated. This is because the
 *         the function doesn't keep the track of the place where the next descriptor
 *         is to be added.
 *         NOTE: INCOMPATIBLE with idma_copy_desc/idma_copy_2d_desc and idma_update_desc_*
 * @param  ch      Selected iDMA HW channel.
 *                 NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  count   Number of descriptors to schedule.
 * @retval <  0    Error code, from idma_status_t.
 * @retval >= 0    See idma_schedule_desc()
 */
IDMA_API int32_t
idma_schedule_desc_fast(int32_t ch,
                        uint32_t count);

/**
 * @name   Add and schedule a 1D descriptor.
 * @brief  Descriptor added in order and scheduled in order.
 *         NOTE: INCOMPATIBLE with idma_add_desc/idma_add_2d_desc/idma_schedule_desc
 * @param  ch       Selected iDMA HW channel.
 *                  NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  dst      iDMA transfer destination
 * @param  src      iDMA transfer source
 * @param  size     iDMA transfer size
 * @param  flags    Descriptor options (Control field). See "Descriptor Control Flags".
 * @retval <  0     Error code, from idma_status_t.
 * @retval >= 0     See idma_schedule_desc()
 */
IDMA_API int32_t
idma_copy_desc(int32_t ch,
               void *dst,
               void *src,
               size_t size,
               uint32_t flags);

/**
 * @name   Add and schedule a 2D descriptor.
 * @brief  Descriptor added in order and scheduled in order.
 *         NOTE: INCOMPATIBLE with idma_add_desc/idma_add_2d_desc/idma_schedule_desc
 * @param  ch           Selected iDMA HW channel.
 *                      NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  dst          iDMA transfer destination
 * @param  src          iDMA transfer source
 * @param  size         iDMA transfer size
 * @param  flags        Descriptor options (Control field).
 *                      See "Descriptor Control Flags".
 * @param  nrows        Number of rows to transfer
 * @param  src_pitch    iDMA transfer source pitch
 * @param  dst_pitch    iDMA transfer destination pitch
 * @retval <  0         Error code, from idma_status_t.
 * @retval >= 0         See idma_schedule_desc()
 */
IDMA_API int32_t
idma_copy_2d_desc(int32_t ch,
                  void *dst,
                  void *src,
                  size_t size,
                  uint32_t flags,
                  uint32_t nrows,
                  uint32_t src_pitch,
                  uint32_t dst_pitch);

/**
 * @name   Check if a descriptor is done, using unique desc ID.
 * @brief  Unique ID is made available when descriptor was scheduled.
 *         The function does the internal processing so it can be
 *         called in an empty loop if want to wait for desc to complete.
 * @param  ch     Selected iDMA HW channel.
 *                NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  index  Unique ID of the descriptor.
 * @retval  1     Desc is done.
 * @retval  0     Desc is NOT done.
 * @retval -1     Error.
 */
IDMA_API int32_t
idma_desc_done(int32_t ch,
               int32_t index);


/**
 * @name   Update destination field of the "next" descriptor
 * @brief  Update  next to be schedule descriptor (means the next
 *         call to idma_desc_schedule() will schedule the desc.
 *         updated in this call.
 *         ONLY IN FIXED-BUFFER MODE.
 * @param  ch          Selected iDMA HW channel.
 *                     NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  dst         New destination address
 * @retval IDMA_OK     Successful
 * @retval !IDMA_OK    Error type
 */
IDMA_API idma_status_t
idma_update_desc_dst(int32_t ch,
                     void *dst);

/**
 * @name   Update source field of the "next" descriptor.
 * @brief  Update next to be schedule descriptor (means the next
 *         call to idma_desc_schedule() will schedule the desc.
 *         updated in this call.
 *         ONLY IN FIXED-BUFFER MODE.
 * @param  ch          Selected iDMA HW channel.
 *                     NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  src         New source address
 * @retval IDMA_OK     Successful
 * @retval !IDMA_OK    Error type
 */
IDMA_API idma_status_t
idma_update_desc_src(int32_t ch,
                     void *src);

/**
 * @name   Update size field of the "next" descriptor.
 * @brief  Update next to be schedule descriptor (means the next
 *         call to idma_desc_schedule() will schedule the desc.
 *         updated in this call.
 *         ONLY IN FIXED-BUFFER MODE.
 * @param  ch          Selected iDMA HW channel.
 *                     NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @param  size        New copy request size
 * @retval IDMA_OK     Successful
 * @retval !IDMA_OK    Error type
 */
IDMA_API idma_status_t
idma_update_desc_size(int32_t ch,
                      uint32_t size);

/**
 * @name   Get buffer status w/ internal processing of completed descs.
 * @brief  Check the number of outstanding descriptors. Will also
 *         trigger internal processing making this call needed when no
 *         interrupts are available/setup.
 *         ONLY IN FIXED-BUFFER MODE.
 * @brief  Returns error or the number of outstanding descs
 * @param  ch       Selected iDMA HW channel.
 *                  NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @retval >=0      num. of outstanding descriptors
 * @retval <0       Error type
 */
IDMA_API int32_t
idma_buffer_status(int32_t ch);

/**
 * @name   Check if the IDMA HW is in error. If yes the function also
 *         sets the error details, sets the buffer status to indicate
 *         error and returns the error code.
* @param   ch     Selected iDMA HW channel.
 *                NOTE: Argument present only if LIBIDMA_USE_MULTICHANNEL_API defined.
 * @retval IDMA_NO_ERR     No Error.
 * @retval !IDMA_NO_ERR    Error type
 */
IDMA_API idma_hw_error_t
idma_buffer_check_errors(int32_t ch);

/**
 * These two functions operate exactly like idma_schedule_desc() and
 * idma_schedule_desc_fast(), and in addition they also return the
 * current clock value (which normally is the CCOUNT register value)
 * in the location pointed to by the "ptime" argument.
 */
IDMA_API int32_t
idma_schedule_desc_clock(int32_t  ch,
                         uint32_t count,
                         uint32_t *ptime);

IDMA_API int32_t
idma_schedule_desc_fast_clock(int32_t  ch,
                              uint32_t count,
                              uint32_t *ptime);

#else // OLD API - no channels

IDMA_API void idma_hw_wait_all(void);
IDMA_API void idma_hw_schedule(uint32_t count);
IDMA_API uint32_t idma_hw_num_outstanding(void);

void idma_log_handler(idma_log_h xlog);

IDMA_API idma_status_t idma_init(uint32_t flags, idma_max_block_t block_sz, uint32_t pif_req, idma_ticks_cyc_t ticks_per_cyc, uint32_t timeout_ticks, idma_err_callback_fn  err_cb_func);
IDMA_API idma_status_t idma_add_desc(idma_buffer_t *bufh, void *dst, void *src, size_t size,  uint32_t flags);
IDMA_API idma_status_t idma_add_2d_desc(idma_buffer_t *bufh, void *dst, void *src, size_t row_sz, uint32_t flags, uint32_t nrows, uint32_t src_pitch,  uint32_t dst_pitch);
IDMA_API idma_status_t idma_init_loop (idma_buffer_t *bufh, idma_type_t type, int32_t ndescs, void *cb_data, idma_callback_fn cb_func);
IDMA_API idma_status_t idma_init_task(idma_buffer_t *taskh, idma_type_t type, int32_t ndescs, idma_callback_fn cb_func, void *cb_data);
IDMA_API idma_status_t idma_copy_task(idma_buffer_t *taskh, void *dst, void *src, size_t size, uint32_t flags, void *cb_data, idma_callback_fn cb_func);
IDMA_API idma_status_t idma_copy_2d_task(idma_buffer_t *taskh, void *dst, void *src, size_t row_sz, uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch, void *cb_data, idma_callback_fn cb_func);
IDMA_API idma_status_t idma_process_tasks(void);
IDMA_API idma_status_t idma_abort_tasks(void);
IDMA_API idma_status_t idma_stop(void);
IDMA_API idma_status_t idma_update_desc_dst(void *dst);
IDMA_API idma_status_t idma_update_desc_src(void *src);
IDMA_API idma_status_t idma_update_desc_size(uint32_t size);
idma_status_t idma_schedule_task( idma_buffer_t *taskh);

IDMA_API idma_state_t  idma_get_state(void);

IDMA_API void idma_pause(void);
IDMA_API void idma_resume(void);

IDMA_API int32_t idma_schedule_desc(uint32_t count);
IDMA_API int32_t idma_schedule_desc_fast(uint32_t count);
IDMA_API int32_t idma_copy_desc(void *dst, void *src, size_t size, uint32_t flags);
IDMA_API int32_t idma_copy_2d_desc(void *dst, void *src, size_t size, uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch);
IDMA_API int32_t idma_desc_done(int32_t index);
IDMA_API int32_t idma_buffer_status(void);
IDMA_API int32_t idma_task_status(idma_buffer_t *taskh);
IDMA_API idma_status_t idma_sleep(void);

IDMA_API idma_hw_error_t idma_buffer_check_errors(void);

IDMA_API idma_error_details_t*   idma_error_details(void);
IDMA_API idma_error_details_t*   idma_buffer_error_details(void);

IDMA_API int32_t idma_schedule_desc_clock(uint32_t count, uint32_t *ptime);
IDMA_API int32_t idma_schedule_desc_fast_clock(uint32_t count, uint32_t *ptime);

#endif  // not LIBIDMA_USE_MULTICHANNEL_API

/* For compatibility with macros that were used before */
#define IDMA_HW_NUM_OUTSTANDING idma_hw_num_outstanding
#define IDMA_HW_SCHEDULE idma_hw_schedule
#define IDMA_HW_WAIT_ALL idma_hw_wait_all

#if (IDMA_USE_64B_DESC > 0)

/**
 * @name   Add 1D descriptor in 64B format to the next location in the buffer/task
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_desc64( idma_buffer_t *bufh,
                 void *dst,
                 void *src,
                 size_t size,
                 uint32_t flags);

/**
 * @name   Add 2D descriptor in 64B format to the next location in the buffer/task
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_2d_desc64( idma_buffer_t *bufh,
                    void *dst,
                    void *src,
                    size_t row_sz,
                    uint32_t flags,
                    uint32_t nrows,
                    uint32_t src_pitch,
                    uint32_t dst_pitch);

/**
 * @name   Add predicated 2D descriptor in 64B format to the next location in the buffer/task
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_2d_pred_desc64( idma_buffer_t *bufh,
                         void *dst,
                         void *src,
                         size_t row_sz,
                         uint32_t flags,
                         void* pred_mask,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch);

/**
 * @name   Add 3D descriptor in 64B format to the next location in the buffer/task
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_3d_desc64( idma_buffer_t *bufh,
                    void *dst,
                    void *src,
                    uint32_t flags,
                    size_t   row_sz,
                    uint32_t nrows,
                    uint32_t ntiles,
                    uint32_t src_row_pitch,
                    uint32_t dst_row_pitch,
                    uint32_t src_tile_pitch,
                    uint32_t dst_tile_pitch);

/**
 * @name   Create & schedule 1D copy request in 64B format
 * @brief  See idma_copy_task for details
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_copy_task64(int32_t ch,
                 idma_buffer_t *taskh,
                 void *dst,
                 void *src,
                 size_t size,
                 uint32_t flags,
                 void *cb_data,
                 idma_callback_fn cb_func);

/**
 * @name   Create & schedule 2D copy request in 64B format
 * @brief  See idma_copy_2d_task for details.
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_copy_2d_task64(int32_t ch,
                    idma_buffer_t *taskh,
                    void *dst,
                    void *src,
                    size_t row_sz,
                    uint32_t flags,
                    uint32_t nrows,
                    uint32_t src_pitch,
                    uint32_t dst_pitch,
                    void *cb_data,
                    idma_callback_fn cb_func);

/**
 * @name   Create & schedule predicated 2D copy request in 64B format
 * @brief  See idma_copy_2d_task for details.
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_copy_2d_pred_task64(int32_t ch,
                         idma_buffer_t *taskh,
                         void *dst,
                         void *src,
                         size_t row_sz,
                         uint32_t flags,
                         void* pred_mask,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch,
                         void *cb_data,
                         idma_callback_fn cb_func);

/**
 * @name   Add and schedule a 1D descriptor in 64B format.
 * @brief  See idma_copy_desc for details.
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_desc64(int32_t ch,
                 void *dst,
                 void *src,
                 size_t size,
                 uint32_t flags);

/**
 * @name   Add and schedule a 2D descriptor in 64B format.
 * @brief  See idma_copy_2d_desc for details.
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_2d_desc64(int32_t ch,
                    void *dst,
                    void *src,
                    size_t size,
                    uint32_t flags,
                    uint32_t nrows,
                    uint32_t src_pitch,
                    uint32_t dst_pitch);

/**
 * @name   Add and schedule a predicated 2D descriptor in 64B format.
 * @brief  See idma_copy_2d_desc for details.
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_2d_pred_desc64(int32_t ch,
                         void *dst,
                         void *src,
                         size_t size,
                         uint32_t flags,
                         void* pred_mask,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch);

/**
 * @name   Add and schedule a 3D descriptor in 64B format.
 * @brief  See idma_copy_2d_desc for details.
 *         NOTE: This functions uses 32-bit src/dst addresses; it'll fetch
 *         one word behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_3d_desc64(int32_t ch,
                    void *dst,
                    void *src,
                    uint32_t flags,
                    size_t   row_sz,
                    uint32_t nrows,
                    uint32_t ntiles,
                    uint32_t src_row_pitch,
                    uint32_t dst_row_pitch,
                    uint32_t src_tile_pitch,
                    uint32_t dst_tile_pitch);

#if (IDMA_USE_WIDE_API > 0)

/**
 * @name   Add 1D descriptor in 64B format to the next location in the buffer/task.
 * @brief  See idma_add_2d_desc for details on arguments.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_desc64_wide( idma_buffer_t *bufh,
                      void *dst,
                      void *src,
                      size_t size,
                      uint32_t flags);

/**
 * @name   Add 2D descriptor in 64B format to the next location in the buffer/task
 * @brief  See idma_add_2d_desc for details on arguments.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_2d_desc64_wide( idma_buffer_t *bufh,
                         void *dst,
                         void *src,
                         size_t row_sz,
                         uint32_t flags,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch);

/**
 * @name   Add predicated 2D descriptor in 64B format to the next location in the buffer/task.
 * @brief  See idma_add_2d_desc for details on arguments.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_2d_pred_desc64_wide( idma_buffer_t *bufh,
                              void *dst,
                              void *src,
                              size_t row_sz,
                              uint32_t flags,
                              void* pred_mask,
                              uint32_t nrows,
                              uint32_t src_pitch,
                              uint32_t dst_pitch);

/**
 * @name   Add 3D descriptor in 64B format to the next location in the buffer/task.
 * @brief  See idma_add_3d_desc64 for details on arguments.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_add_3d_desc64_wide( idma_buffer_t *bufh,
                         void *dst,
                         void *src,
                         uint32_t flags,
                         size_t   row_sz,
                         uint32_t nrows,
                         uint32_t ntiles,
                         uint32_t src_row_pitch,
                         uint32_t dst_row_pitch,
                         uint32_t src_tile_pitch,
                         uint32_t dst_tile_pitch);

/**
 * @name   Create & schedule 1D copy request in 64B format.
 * @brief  See idma_copy_task for details
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_copy_task64_wide(int32_t ch,
                      idma_buffer_t *taskh,
                      void *dst,
                      void *src,
                      size_t size,
                      uint32_t flags,
                      void *cb_data,
                      idma_callback_fn cb_func);

/**
 * @name   Create & schedule 2D copy request in 64B format.
 * @brief  See idma_copy_task for details
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_copy_2d_task64_wide(int32_t ch,
                         idma_buffer_t *taskh,
                         void *dst,
                         void *src,
                         size_t row_sz,
                         uint32_t flags,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch,
                         void *cb_data,
                         idma_callback_fn cb_func);

/**
 * @name   Create & schedule predicated 2D copy request in 64B format.
 * @brief  See idma_copy_task for details
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API idma_status_t
idma_copy_2d_pred_task64_wide(int32_t ch,
                              idma_buffer_t *taskh,
                              void *dst,
                              void *src,
                              size_t row_sz,
                              uint32_t flags,
                              void* pred_mask,
                              uint32_t nrows,
                              uint32_t src_pitch,
                              uint32_t dst_pitch,
                              void *cb_data,
                              idma_callback_fn cb_func);

/**
 * @name   Add and schedule a 1D descriptor in 64B format.
 * @brief  See idma_copy_desc for details.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_desc64_wide(int32_t ch,
                      void *dst,
                      void *src,
                      size_t size,
                      uint32_t flags);

/**
 * @name   Add and schedule a 2D descriptor in 64B format.
 * @brief  See idma_copy_2d_desc for details.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two word behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_2d_desc64_wide(int32_t ch,
                         void *dst,
                         void *src,
                         size_t size,
                         uint32_t flags,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch);

/**
 * @name   Add and schedule a predicated 2D descriptor in 64B format.
 * @brief  See idma_copy_desc for details.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_2d_pred_desc64_wide(int32_t ch,
                              void *dst,
                              void *src,
                              size_t size,
                              uint32_t flags,
                              void* pred_mask,
                              uint32_t nrows,
                              uint32_t src_pitch,
                              uint32_t dst_pitch);

/**
 * @name   Add and schedule a 3D descriptor in 64B format.
 * @brief  See idma_copy_desc for details.
 *         NOTE: This functions uses 64-bit src/dst addresses; it'll fetch
 *         two words behind the src/dst pointers.
 */
IDMA_API int32_t
idma_copy_3d_desc64_wide(int32_t ch,
                         void *dst,
                         void *src,
                         uint32_t flags,
                         size_t   row_sz,
                         uint32_t nrows,
                         uint32_t ntiles,
                         uint32_t src_row_pitch,
                         uint32_t dst_row_pitch,
                         uint32_t src_tile_pitch,
                         uint32_t dst_tile_pitch);

#endif
#endif





/*************************************************/
/****            Internal Stuff               ****/
/**** DO NOT ACCESS OR RELY ON THE CODE BELOW ****/
/*************************************************/

#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE

# define SRC_WIDE_ADDR_SHIFT            16
# define DST_WIDE_ADDR_SHIFT            20
# define IDMA_WIDE_ADDR_MASK            UINT32_C(0x0000000F)

idma_status_t
idma_copy_task_wide_i(int32_t          ch,
                      idma_buffer_t *  taskh,
                      void *           dst,
                      void *           src,
                      size_t           size,
                      uint32_t         flags,
                      void *           cb_data,
                      idma_callback_fn cb_func);

idma_status_t
idma_copy_2d_task_wide_i(int32_t          ch,
                         idma_buffer_t *  taskh,
                         void *           dst,
                         void *           src,
                         size_t           row_sz,
                         uint32_t         flags,
                         uint32_t         nrows,
                         uint32_t         src_pitch,
                         uint32_t         dst_pitch,
                         void *           cb_data,
                         idma_callback_fn cb_func);

#endif // IDMA_USE_WIDE_ADDRESS_COMPILE

#define IDMA_CHANNEL_0                  0
#define IDMA_CHANNEL_1                  1
#define IDMA_CHANNEL_2                  2
#define IDMA_CHANNEL_3                  3

#if (LIBIDMA_USE_MULTICHANNEL_API > 0)
#  define IDMA_CH_PTR                   ch
#  define IDMA_CHAN_FUNC_ARG            int32_t ch,
#  define IDMA_CHAN_FUNC_ARG_SINGLE     int32_t ch
#else
#  define IDMA_CH_PTR                   IDMA_CHANNEL_0
#  define IDMA_CHAN_FUNC_ARG
#  define IDMA_CHAN_FUNC_ARG_SINGLE     void
#endif

#define IDMA_RER XT_RER
#define IDMA_WER XT_WER

#define IDMA_1D_DESC_CODE               3
#define IDMA_2D_DESC_CODE               7
#define IDMA_JMP_DESC_CODE              0
#define IDMA_64B_DESC_CODE              1

// Applicable to 64-byte descriptor only
#define IDMA_64B_SUBTYPE_MASK                   UINT32_C(0x00000007)
#define IDMA_64B_SUBTYPE_SHIFT                  4
#define IDMA_64B_1D_TYPE                        0
#define IDMA_64B_2D_TYPE                        1
#define IDMA_64B_2D_COMPRESS_TYPE               2
#define IDMA_64B_3D_TYPE                        4
#define IDMA_64B_LONG_ADDR_CTRL_BIT_MASK        UINT32_C(0x00000400)
#define SET_CONTROL_SUBTYPE(a)                  (((a) & IDMA_64B_SUBTYPE_MASK) << IDMA_64B_SUBTYPE_SHIFT)

// Inline selection for "wrapper" functions
#define WRAPPER_FUNC    static __attribute__((always_inline))

#define IDMA_1D_DESC_SIZE               16
#define IDMA_2D_DESC_SIZE               32
#define IDMA_64_DESC_SIZE               64

#if defined (IDMA_USE_DRAM1)
# define IDMA_DRAM    \
       __attribute__ ((section(".dram1.data")))
#endif
#if defined (IDMA_USE_DRAM0)
# define IDMA_DRAM    \
       __attribute__ ((section(".dram0.data")))
#endif

#ifdef IDMA_DEBUG
# define IDMA_CONTROL_STRUCT_SIZE_      52
#else
# define IDMA_CONTROL_STRUCT_SIZE_      48
#endif

# ifdef IDMA_DEBUG
#  define IDMA_ASSERT(expr)
void idma_print(int32_t ch, const char* fmt, ...);
#  define XLOG(ch, ...)  do { idma_print((ch), __VA_ARGS__);} while (0)              // parasoft-suppress MISRA2012-RULE-20_7 "used only in non-FuSa code"
# else
#  define IDMA_ASSERT(expr)
#  define XLOG(ch, ...)  (void)(ch); do {} while (0)
# endif

#if !defined (IDMA_BUILD) &&  defined (IDMA_USE_DRAM1)
#define IDMA_LOG_SIZE                   200
typedef struct {
    char tmp[IDMA_CONTROL_STRUCT_SIZE_];
} idma_ctrl_opaque_t;

idma_buf_t*        g_idma_buf_ptr[XCHAL_IDMA_NUM_CHANNELS]    IDMA_DRAM;
idma_ctrl_opaque_t g_idma_cntrl[XCHAL_IDMA_NUM_CHANNELS]      IDMA_DRAM;
# ifdef IDMA_DEBUG
char               g_idmalogbuf[IDMA_LOG_SIZE]  IDMA_DRAM;
# endif
#endif

/* 1D descriptor structure */
struct idma_desc_struct {
    uint32_t  control;
    void*     src;
    void*     dst;
    uint32_t  size;
};

/* 2D descriptor structure */
struct idma_2d_desc_struct {
    uint32_t  control;
    void*     src;
    void*     dst;
    uint32_t  size;
    uint32_t  src_pitch;
    uint32_t  dst_pitch;
    uint32_t  nrows;
    uint32_t  dummy;
};

/* 64B descriptor structure */
struct idma_desc64_struct {
    uint32_t  control;
    uint32_t  src;
    uint32_t  dst;
    int32_t   size;
    int32_t   src_pitch;
    int32_t   dst_pitch;
    int32_t   nrows;
    uint32_t  reserved1;
    int32_t   src_tile_pitch;
    int32_t   dst_tile_pitch;
    int32_t   ntiles;
    uint32_t  pred_mask;
    void*     reserved2;
    void*     reserved3;
    uint32_t  ext_src;
    uint32_t  ext_dst;
};

/* Real buffer struct - NOT TO BE USED BY APPLICATION */

struct idma_buf_struct {
    idma_desc_t *     next_desc;      // points past the last scheduled desc. Used by functions
    // needing to update the desc that is just to be scheduled.
    idma_desc_t *     next_add_desc;  // points past the last added desc. Used by functions that
    // populate a new task, or populate a buffer where later
    // schedule call will schedule them all at once.

    int32_t           num_descs;      // total num of descs, assigned on init. In fixed buffer mode
    // it sets the size of the circular buffer by setting JUMP
    // command between the last desc and the 1st one. In TASK mode
    // it is used to tell the lib how many descs to schedule.
    idma_desc_t *     last_desc;      // points past the last desc. Assigned to sped-up wrap-arround
    // calculation, not to use num_descs and multiplication.

    int32_t           type;           // descs type, assigned on init.

    idma_buf_t *      next_task;      // TASK: points to next task, assigned on schedule
    int32_t           cur_desc_i;     // FIXED-BUFFER: tracks next desc to execute.
    int32_t           status;         // TASK: # of remaining descs after schedule.
    // BOTH modes: error type, if in error.
    idma_callback_fn  cb_func;        // Callback on completion
    void *            cb_data;        // Completion callback argument
    int32_t           ch;
    void *            thread_id;      // (OS-specific) ID of owning thread
    int16_t           sleeping;       // Nonzero when thread is sleeping (blocked)
    int16_t           pending;        // Nonzero when buffer is queued
    int32_t           pending_desc_cnt;

    idma_desc_t       desc    __attribute__ ((aligned(16)));
};

/* Inline functions for iDMA register read/write */
ALWAYS_INLINE uint32_t
READ_IDMA_REG(int32_t ch, int32_t reg)
{
    return XT_RER(IDMAREG_BASE((uint32_t)ch) + (uint32_t)(reg));
}

ALWAYS_INLINE void
WRITE_IDMA_REG(int32_t ch, int32_t reg, uint32_t val)
{
    XT_WER((uint32_t)(val), (IDMAREG_BASE((uint32_t)ch)) + (uint32_t)(reg));
}

ALWAYS_INLINE void
set_desc_addr_fields(idma_desc_t * desc,
                     void *        dst,
                     void *        src)
{
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
    uint32_t * src36 = cvt_voidp_to_uint32p(src);
    uint32_t * dst36 = cvt_voidp_to_uint32p(dst);

    desc->src = cvt_uint32_to_voidp(src36[0]);
    desc->dst = cvt_uint32_to_voidp(dst36[0]);
    desc->control =
        (desc->control & ~(IDMA_WIDE_ADDR_MASK << SRC_WIDE_ADDR_SHIFT)) | ((src36[1] & IDMA_WIDE_ADDR_MASK) << SRC_WIDE_ADDR_SHIFT);
    desc->control =
        (desc->control & ~(IDMA_WIDE_ADDR_MASK << DST_WIDE_ADDR_SHIFT)) | ((dst36[1] & IDMA_WIDE_ADDR_MASK) << DST_WIDE_ADDR_SHIFT);
#else
    desc->src = src;
    desc->dst = dst;
#endif
}

ALWAYS_INLINE void
set_1d_fields( int32_t ch,
               idma_desc_t* desc,
               void *dst,
               void *src,
               size_t size)
{
    set_desc_addr_fields(desc, dst, src);
    desc->size = size;
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
# ifdef IDMA_DEBUG
    {
        uint32_t src64 = (desc->control >> SRC_WIDE_ADDR_SHIFT) & IDMA_WIDE_ADDR_MASK;
        uint32_t dst64 = (desc->control >> DST_WIDE_ADDR_SHIFT) & IDMA_WIDE_ADDR_MASK;
        XLOG(ch, "Add 1D desc wide @ %p, %d-bytes(%x-%p -> %x-%p), control:0x%x\n",
             desc, desc->size, src64, desc->src, dst64, desc->dst, desc->control);
    }
# endif
#else
    XLOG(ch, "Add 1D desc @ %p, %d-bytes(%p -> %p), control:0x%x\n", desc, desc->size, desc->src, desc->dst, desc->control);
#endif
    (void)(ch);
}

ALWAYS_INLINE void
set_2d_fields(int32_t ch,
              idma_desc_t* desc,
              void *dst,
              void *src,
              size_t size,
              uint32_t nrows,
              uint32_t src_pitch,
              uint32_t dst_pitch)
{
    idma_2d_desc_t* desc2d = (idma_2d_desc_t*) desc;                              // parasoft-suppress MISRA2012-RULE-11_3 "conversion checked"
    set_desc_addr_fields(desc, dst, src);
    desc2d->size       = size;
    desc2d->nrows      = nrows;
    desc2d->src_pitch  = src_pitch;
    desc2d->dst_pitch  = dst_pitch;
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
# ifdef IDMA_DEBUG
    {
        uint32_t src64 = (desc->control >> SRC_WIDE_ADDR_SHIFT) & IDMA_WIDE_ADDR_MASK;
        uint32_t dst64 = (desc->control >> DST_WIDE_ADDR_SHIFT) & IDMA_WIDE_ADDR_MASK;
        XLOG(ch, "Add 2D desc wide @ %p, %d-bytes (%x-%p -> %x-%p)\n",
             desc2d, desc2d->size, src64, desc->src, dst64, desc->dst);
        XLOG(ch, "           (#rows:%d, src_pitch:%d, dst_pitch:%d, control:0x%x)\n",
             desc2d->nrows, desc2d->src_pitch, desc2d->dst_pitch, desc2d->control);

    }
# endif
#else
    XLOG(ch, "Add 2D desc @ %p, %d-bytes (%p -> %p)\n", desc2d, desc2d->size, desc2d->src, desc2d->dst);
    XLOG(ch, "           (#rows:%d, src_pitch:%d, dst_pitch:%d, control:0x%x)\n",
         desc2d->nrows, desc2d->src_pitch, desc2d->dst_pitch, desc2d->control);
#endif
    (void)(ch);
}

#if (IDMA_USE_64B_DESC > 0)

INTERNAL_FUNC void
set_desc64_short_addr(idma_desc64_t* desc, void *dst, void *src)
{
    desc->src     = ((uint32_t*)src)[0];
    desc->dst     = ((uint32_t*)dst)[0];
}


INTERNAL_FUNC void
set_desc64_2d_pitch(idma_desc64_t* desc, uint32_t src_pitch, uint32_t dst_pitch)
{
    desc->src_pitch  = src_pitch;
    desc->dst_pitch  = dst_pitch;
}

INTERNAL_FUNC void
set_desc64_3d_pitch(idma_desc64_t* desc, uint32_t src_pitch, uint32_t dst_pitch, uint32_t src_tile_pitch, uint32_t dst_tile_pitch)
{
    set_desc64_2d_pitch(desc, src_pitch, dst_pitch);
    desc->src_tile_pitch  = src_tile_pitch;
    desc->dst_tile_pitch  = dst_tile_pitch;
}

INTERNAL_FUNC void
set_1d_desc64_fields(int32_t ch,
                     idma_desc_t* desch,
                     void *dst,
                     void *src,
                     size_t size)
{
    idma_desc64_t* desc = (idma_desc64_t*) desch;

    set_desc64_short_addr(desc, dst, src);
    desc->size = size;
    XLOG(ch, "Add 1D desc64 @ %p, %d-bytes(%lx -> %lx), control:0x%x\n", desc, desc->size, desc->src, desc->dst, desc->control);
}

INTERNAL_FUNC void
set_2d_desc64_fields(int32_t ch,
                     idma_desc_t* desch,
                     void *dst,
                     void *src,
                     size_t size,
                     uint32_t nrows,
                     uint32_t src_pitch,
                     uint32_t dst_pitch)
{
    idma_desc64_t* desc = (idma_desc64_t*) desch;

    set_desc64_short_addr(desc, dst, src);
    set_desc64_2d_pitch(desc, src_pitch, dst_pitch);
    desc->size       = size;
    desc->nrows      = nrows;

    XLOG(ch, "Add 2D desc @ %p, %d-bytes (%x -> %x)\n", desc, desc->size, desc->src, desc->dst);
    XLOG(ch, "           (#rows:%d, src_pitch:%d, dst_pitch:%d, control:0x%x)\n", desc->nrows, desc->src_pitch, desc->dst_pitch, desc->control);
}

INTERNAL_FUNC void
set_3d_desc64_fields(int32_t ch,
                     idma_desc_t* desch,
                     void     *dst,
                     void     *src,
                     size_t   row_sz,
                     uint32_t nrows,
                     uint32_t ntiles,
                     uint32_t src_pitch,
                     uint32_t dst_pitch,
                     uint32_t src_tile_pitch,
                     uint32_t dst_tile_pitch)
{
    idma_desc64_t* desc = (idma_desc64_t*) desch;

    set_desc64_short_addr(desc, dst, src);
    set_desc64_3d_pitch(desc, src_pitch, dst_pitch, src_tile_pitch, dst_tile_pitch);

    desc->size            = row_sz;
    desc->nrows           = nrows;
    desc->ntiles          = ntiles;

    XLOG(ch, "Add 3D desc @ %p, %d-bytes (%x -> %x)\n", desc, desc->size, desc->src, desc->dst);
    XLOG(ch, "           (#row_sz/rows/tiles:%d/%d/%d, row_pitch:%d/%d, tile_pitch:%d/%d, control:0x%x)\n",
         desc->size,desc->nrows, desc->ntiles, desc->src_pitch, desc->dst_pitch, src_tile_pitch, dst_tile_pitch, desc->control);
}

INTERNAL_FUNC void
set_desc64_ctrl(idma_desc_t* desch, uint32_t flags, uint32_t code)
{
    idma_desc64_t* desc = (idma_desc64_t*) desch;
    desc->control = (flags | code);
}

#if (IDMA_USE_WIDE_API > 0)

INTERNAL_FUNC void
set_desc64_long_addr(idma_desc64_t* desc, void *dst, void *src)
{
    set_desc64_short_addr(desc, dst, src);
    desc->ext_src = ((uint32_t*)src)[1];
    desc->ext_dst = ((uint32_t*)dst)[1];
}

INTERNAL_FUNC void
set_1d_desc64_fields_wide(int32_t ch,
                          idma_desc_t* desch,
                          void *dst,
                          void *src,
                          size_t size)
{
    idma_desc64_t* desc = (idma_desc64_t*) desch;

    set_desc64_long_addr(desc, dst, src);
    desc->size = size;
    XLOG(ch, "Add 1D desc64 wide @ %p, %d-bytes(%x-%x -> %x-%x), control:0x%x\n", desc, desc->size, desc->ext_src, desc->src, desc->ext_dst, desc->dst, desc->control);
}

INTERNAL_FUNC void
set_2d_desc64_fields_wide(int32_t ch,
                          idma_desc_t* desch,
                          void *dst,
                          void *src,
                          size_t size,
                          uint32_t nrows,
                          uint32_t src_pitch,
                          uint32_t dst_pitch)
{
    idma_desc64_t* desc = (idma_desc64_t*) desch;

    set_desc64_long_addr(desc, dst, src);
    set_desc64_2d_pitch(desc, src_pitch, dst_pitch);
    desc->size       = size;
    desc->nrows      = nrows;

    XLOG(ch, "Add 2D desc wide @ %p, %d-bytes (%x-%x -> %x-%x)\n", desc, desc->size,  desc->ext_src, desc->src, desc->ext_dst, desc->dst);
    XLOG(ch, "           (#rows:%d, src_pitch:%d, dst_pitch:%d, control:0x%x)\n", desc->nrows, desc->src_pitch, desc->dst_pitch, desc->control);
}

INTERNAL_FUNC void
set_3d_desc64_fields_wide(int32_t ch,
                          idma_desc_t* desch,
                          void    *dst,
                          void    *src,
                          size_t   row_sz,
                          uint32_t nrows,
                          uint32_t ntiles,
                          uint32_t src_pitch,
                          uint32_t dst_pitch,
                          uint32_t src_tile_pitch,
                          uint32_t dst_tile_pitch)
{
    idma_desc64_t* desc = (idma_desc64_t*) desch;

    set_desc64_long_addr(desc, dst, src);
    set_desc64_3d_pitch(desc, src_pitch, dst_pitch, src_tile_pitch, dst_tile_pitch);
    desc->size            = row_sz;
    desc->nrows           = nrows;
    desc->ntiles          = ntiles;

    XLOG(ch, "Add 3D desc wide @ %p, %d-bytes (%x-%x -> %x-%x)\n", desc, desc->size,  desc->ext_src, desc->src, desc->ext_dst, desc->dst);
    XLOG(ch, "           (#row_sz/rows/tiles:%d/%d/%d, row_pitch:%d/%d, tile_pitch:%d/%d, control:0x%x)\n",
         desc->size,desc->nrows, desc->ntiles, desc->src_pitch, desc->dst_pitch, src_tile_pitch, dst_tile_pitch, desc->control);
}

#endif
#endif

INTERNAL_FUNC void
set_desc_ctrl(idma_desc_t* desc, uint32_t flags, uint32_t code)
{
    desc->control = (flags | code);
}

INTERNAL_FUNC void
update_next_add_ptr(idma_buf_t* buf)
{
    idma_desc_t* next = &buf->next_add_desc[buf->type];
    XT_MOVGEZ((int32_t)next, (int32_t)&buf->desc, next - buf->last_desc);         // parasoft-suppress MISRA2012-RULE-11_4  MISRA2012-RULE-18_4 "conversion from ptr to int ok. ptr arithmetic ok."
    buf->next_add_desc = next;
}

#if defined(IDMA_USE_XTOS) || defined(IDMA_APP_USE_XTOS)
INTERNAL_FUNC void
update_next_desc(int32_t ch, uint32_t count)
{
    idma_buf_t*   buf;
    idma_desc_t*  next;
    int32_t       diff;

    buf  = idma_chan_buf_get(ch);
    next = &buf->next_desc[count*(uint32_t)buf->type];
    diff = next - buf->last_desc;

    /* next = (next >= buf->last_desc) ? &buf->desc + (&next - &buf->last_desc) : next; */
    XT_MOVGEZ((int32_t)next, (int32_t)(&buf->desc + diff), diff);                 // parasoft-suppress MISRA2012-RULE-11_4 MISRA2012-RULE-18_4 "conversion from ptr to int ok. ptr arithmetic ok."
    buf->next_desc = next;
}
#endif

INTERNAL_FUNC void
hw_schedule(int32_t ch, uint32_t count)
{
    XT_MEMW();
    WRITE_IDMA_REG(ch, IDMA_REG_CONTROL, 1);
    WRITE_IDMA_REG(ch, IDMA_REG_DESC_INC, count);
}

INTERNAL_FUNC uint32_t
hw_num_outstanding(int32_t ch)
{
    return READ_IDMA_REG(ch, IDMA_REG_NUM_DESC);
}

INTERNAL_FUNC int32_t
schedule_desc_fast(int32_t ch, uint32_t count)
{
    if (ch < XCHAL_IDMA_NUM_CHANNELS) {
        idma_buf_t* buf = idma_chan_buf_get(ch);

        buf->cur_desc_i += (int32_t)count;
        buf->cur_desc_i &= INT32_C(0x7fffffff);

        XLOG(ch, "Schedule %d desc from index:%d\n", count, buf->cur_desc_i);
        hw_schedule(ch, count);
        return buf->cur_desc_i;
    }

    return (int32_t) IDMA_ERR_BAD_CHAN;
}

#if defined(IDMA_USE_XTOS) || defined(IDMA_APP_USE_XTOS)
INTERNAL_FUNC int32_t
schedule_desc(int32_t ch, uint32_t count)
{
    update_next_desc(ch, count);
    return schedule_desc_fast(ch, count);
}
#else
int32_t schedule_thread_buffer(int32_t ch, uint32_t count);

INTERNAL_FUNC int32_t
schedule_desc(int32_t ch, uint32_t count)
{
    return schedule_thread_buffer(ch, count);
}
#endif

WRAPPER_FUNC idma_buf_t *
convert_buffer_to_buf(idma_buffer_t * buffer)   // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Pointer is converted, cannot use const here"
{
    return (idma_buf_t *) buffer;                 // parasoft-suppress MISRA2012-RULE-11_3-2 "Conversion to internal data type OK"
}


idma_status_t   idma_sleep_i(int32_t ch);
int32_t         idma_buffer_status_i(int32_t ch);
idma_state_t    idma_get_state_i(int32_t ch);
idma_status_t   idma_init_i(int32_t ch, uint32_t flags, idma_max_block_t block_sz, uint32_t pif_req, idma_ticks_cyc_t ticks_per_cyc, uint32_t timeout_ticks, idma_err_callback_fn err_cb_func);
void            idma_stop_i(int32_t ch);
void            idma_pause_i(int32_t ch);
void            idma_enable_i(int32_t ch);
void            idma_resume_i(int32_t ch);
idma_status_t   idma_process_tasks_i(int32_t ch);
idma_status_t   idma_abort_tasks_i(int32_t ch);
idma_status_t   idma_init_task_i (int32_t ch, idma_buffer_t *taskh, idma_type_t type, int32_t ndescs, idma_callback_fn cb_func, void *cb_data);
idma_status_t   idma_copy_task_i(int32_t ch, idma_buffer_t *taskh,  void *dst, void *src, size_t size, uint32_t flags, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_task64_i(int32_t ch, idma_buffer_t *taskh,  void *dst, void *src, size_t size, uint32_t flags, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_task64_wide_i(int32_t ch, idma_buffer_t *taskh,  void *dst, void *src, size_t size, uint32_t flags, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_2d_task64_i(int32_t ch, idma_buffer_t *taskh, void *dst, void *src, size_t row_sz, uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_2d_task64_wide_i(int32_t ch, idma_buffer_t *taskh, void *dst, void *src, size_t row_sz, uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_2d_pred_task64_i(int32_t ch, idma_buffer_t *taskh, void *dst, void *src, size_t row_sz, uint32_t flags, void* pred_mask, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_2d_pred_task64_wide_i(int32_t ch, idma_buffer_t *taskh, void *dst, void *src, size_t row_sz, uint32_t flags, void* pred_mask, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_3d_task64_i( int32_t ch, idma_buffer_t *taskh, void *dst, void *src, uint32_t flags, size_t row_sz, uint32_t nrows, uint32_t ntiles, uint32_t src_pitch, uint32_t dst_pitch, uint32_t src_tile_pitch, uint32_t dst_tile_pitch, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_3d_task64_wide_i( int32_t ch, idma_buffer_t *taskh, void *dst, void *src, uint32_t flags, size_t row_sz, uint32_t nrows, uint32_t ntiles, uint32_t src_pitch, uint32_t dst_pitch, uint32_t src_tile_pitch, uint32_t dst_tile_pitch, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_copy_2d_task_i(int32_t ch, idma_buffer_t *taskh, void *dst, void *src, size_t row_sz, uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch, void *cb_data, idma_callback_fn cb_func);
idma_status_t   idma_init_loop_i (int32_t ch, idma_buffer_t* bufh, idma_type_t type, int32_t ndescs, void* cb_data, idma_callback_fn cb_func);
idma_hw_error_t       idma_buffer_check_errors_i(int32_t ch);
idma_error_details_t* idma_error_details_i(int32_t ch);


/************************************************/
/****          API Implementation            ****/
/************************************************/

IDMA_API void
idma_hw_wait_all(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    uint32_t state;
    do {
        state = READ_IDMA_REG(IDMA_CH_PTR, IDMA_REG_STATUS) & IDMA_STATE_MASK;
        if ((state == (uint32_t)IDMA_STATE_DONE) || (state == (uint32_t)IDMA_STATE_IDLE) || (state == (uint32_t)IDMA_STATE_ERROR)) {
            break;
        }
    } while (1);
}

IDMA_API void
idma_hw_schedule(IDMA_CHAN_FUNC_ARG
                 uint32_t count)
{
#if defined(IDMA_USE_XTOS) || defined(IDMA_APP_USE_XTOS)
    hw_schedule(IDMA_CH_PTR, count);
#else
# if (LIBIDMA_USE_MULTICHANNEL_API > 0)
    (void) idma_schedule_desc(ch, count);
# else
    (void) idma_schedule_desc(count);
# endif
#endif
}

IDMA_API uint32_t
idma_hw_num_outstanding(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return hw_num_outstanding(IDMA_CH_PTR);
}

IDMA_API idma_state_t
idma_get_state(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_get_state_i(IDMA_CH_PTR);
}

IDMA_API idma_status_t
idma_init( IDMA_CHAN_FUNC_ARG
           uint32_t         flags,
           idma_max_block_t block_sz,
           uint32_t         pif_req,
           idma_ticks_cyc_t ticks_per_cyc,
           uint32_t         timeout_ticks,
           idma_err_callback_fn   err_cb_func)
{
    return idma_init_i(IDMA_CH_PTR, flags, block_sz, pif_req, ticks_per_cyc, timeout_ticks, err_cb_func);
}

IDMA_API idma_status_t
idma_stop(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    idma_stop_i(IDMA_CH_PTR);
    return IDMA_OK;
}

IDMA_API void
idma_pause(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    idma_pause_i(IDMA_CH_PTR);
}

IDMA_API void
idma_resume(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    idma_resume_i(IDMA_CH_PTR);
}

IDMA_API idma_status_t
idma_sleep(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_sleep_i(IDMA_CH_PTR);
}

IDMA_API idma_status_t
idma_init_loop (IDMA_CHAN_FUNC_ARG
                idma_buffer_t *bufh,
                idma_type_t type,
                int32_t ndescs,
                void *cb_data,
                idma_callback_fn cb_func)
{
    return idma_init_loop_i(IDMA_CH_PTR, bufh, type, ndescs, cb_data, cb_func);
}

IDMA_API int32_t
idma_buffer_status(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_buffer_status_i(IDMA_CH_PTR);
}

IDMA_API idma_status_t
idma_add_desc( idma_buffer_t *bufh,
               void *dst,
               void *src,
               size_t size,
               uint32_t flags)
{
    idma_buf_t *buf;
    DECLARE_PS();
    IDMA_DISABLE_INTS();
    buf = convert_buffer_to_buf(bufh);

    set_desc_ctrl(buf->next_add_desc, flags, IDMA_1D_DESC_CODE);
    set_1d_fields(buf->ch, buf->next_add_desc, dst, src, size);

    update_next_add_ptr(buf);
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}


static inline idma_status_t
idma_add_2d_desc( idma_buffer_t *bufh, void *dst, void *src,  size_t row_sz,  uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch)
{
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = convert_buffer_to_buf(bufh);

    set_desc_ctrl(buf->next_add_desc, flags, IDMA_2D_DESC_CODE);
    set_2d_fields(buf->ch, buf->next_add_desc, dst, src, row_sz, nrows, src_pitch, dst_pitch);

    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API int32_t
idma_copy_desc(IDMA_CHAN_FUNC_ARG
               void *dst,
               void *src,
               size_t size,
               uint32_t flags)
{
    int32_t     ret;
    idma_buf_t* buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(IDMA_CH_PTR);

    set_desc_ctrl(buf->next_desc, flags, IDMA_1D_DESC_CODE);
    set_1d_fields(IDMA_CH_PTR, buf->next_desc, dst, src, size);

    ret = schedule_desc(IDMA_CH_PTR, 1U);
    IDMA_ENABLE_INTS();

    return ret;
}

IDMA_API int32_t
idma_copy_2d_desc(IDMA_CHAN_FUNC_ARG void *dst, void *src,  size_t size, uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch)
{
    idma_buf_t* buf;
    DECLARE_PS();
    int32_t    ret = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(IDMA_CH_PTR);

    set_desc_ctrl(buf->next_desc, flags, IDMA_2D_DESC_CODE);
    set_2d_fields (IDMA_CH_PTR, buf->next_desc, dst, src, size, nrows, src_pitch, dst_pitch);

    ret = schedule_desc(IDMA_CH_PTR, 1U);
    IDMA_ENABLE_INTS();
    return ret;
}

IDMA_API int32_t
idma_schedule_desc(IDMA_CHAN_FUNC_ARG
                   uint32_t count)
{
    int32_t ret;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    ret = schedule_desc(IDMA_CH_PTR, count);
    IDMA_ENABLE_INTS();
    return ret;
}

IDMA_API int32_t
idma_schedule_desc_fast(IDMA_CHAN_FUNC_ARG
                        uint32_t count)
{
    int32_t ret;
    DECLARE_PS();

    IDMA_DISABLE_INTS();
#if defined(IDMA_USE_XTOS) || defined(IDMA_APP_USE_XTOS)
    ret = schedule_desc_fast(IDMA_CH_PTR, count);
#else
    ret = schedule_desc(IDMA_CH_PTR, count);
#endif
    IDMA_ENABLE_INTS();
    return ret;
}

IDMA_API int32_t
idma_desc_done(IDMA_CHAN_FUNC_ARG
               int32_t index)
{
    idma_buf_t* buf;
    int32_t diff ;
    DECLARE_PS();
    int32_t done = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(IDMA_CH_PTR);

    /* Descriptor is done if it's index is less then or equal the difference between
       the last scheduled descriptor, and the number of outstanding descriptors.
       We take the diff between the last scheduled descriptor and the index.
       This diff is (cur_desc_i - index)  or  0x7fffffff - (index - cur_desc_i)
       depending on which value is larger.
     */
    diff  = (buf->cur_desc_i - index) & INT32_C(0x7fffffff);
    if ((int32_t)hw_num_outstanding(IDMA_CH_PTR) <= diff) {
        done = 1;
    }

    XLOG(IDMA_CH_PTR, "Descriptor @%u: %s (remaining: %d, current:%d) \n",
         index, ((done != 0) ? "DONE" : "PENDING"), hw_num_outstanding(IDMA_CH_PTR), buf->cur_desc_i);
    IDMA_ENABLE_INTS();
    return done;
}

ALWAYS_INLINE idma_status_t
idma_update_desc_dst(IDMA_CHAN_FUNC_ARG
                     void *dst)
{
    idma_desc_t *desc;
    idma_buf_t *buf;
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
    uint32_t * dstw = cvt_voidp_to_uint32p(dst);
#endif

    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(IDMA_CH_PTR);
    desc = buf->next_desc;
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
    desc->dst = cvt_uint32_to_voidp(dstw[0]);
    desc->control =
        (desc->control & ~(IDMA_WIDE_ADDR_MASK << DST_WIDE_ADDR_SHIFT)) | ((dstw[1] & IDMA_WIDE_ADDR_MASK) << DST_WIDE_ADDR_SHIFT);
    XLOG(IDMA_CH_PTR, "Change dst field for descriptor @ %p to: %p-%p\n",
         desc, cvt_uint32_to_voidp(dstw[1]), cvt_uint32_to_voidp(dstw[0]));
#else
    desc->dst = dst;
    XLOG(IDMA_CH_PTR, "Change dst field for descriptor @ %p to: %p\n",
         desc, desc->dst);
#endif
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

ALWAYS_INLINE idma_status_t
idma_update_desc_src(IDMA_CHAN_FUNC_ARG
                     void *src)
{
    idma_desc_t *desc;
    idma_buf_t *buf;
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
    uint32_t * srcw = cvt_voidp_to_uint32p(src);
#endif

    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(IDMA_CH_PTR);
    desc = buf->next_desc;
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
    desc->src = cvt_uint32_to_voidp(srcw[0]);
    desc->control =
        (desc->control & ~(IDMA_WIDE_ADDR_MASK << SRC_WIDE_ADDR_SHIFT)) | ((srcw[1] & IDMA_WIDE_ADDR_MASK) << SRC_WIDE_ADDR_SHIFT);
    XLOG(IDMA_CH_PTR, "Change src field for descriptor @ %p to: %p-%p\n",
         desc, cvt_uint32_to_voidp(srcw[1]), cvt_uint32_to_voidp(srcw[0]));
#else
    desc->src = src;
    XLOG(IDMA_CH_PTR, "Change src field for descriptor @ %p to: %p\n",
         desc, desc->src);
#endif
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API idma_status_t
idma_update_desc_size(IDMA_CHAN_FUNC_ARG
                      uint32_t size)
{
    idma_desc_t *desc;
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(IDMA_CH_PTR);
    desc = buf->next_desc;
    desc->size = size;

    XLOG(IDMA_CH_PTR, "Change size field for descriptor @ %p to: %d\n",  desc, desc->size);
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

#if (IDMA_USE_64B_DESC > 0)

IDMA_API idma_status_t
idma_update_desc64_dst(int32_t ch, void *dst)
{
    idma_desc64_t *desc;
    idma_buf_t *buf;

    DECLARE_PS();
    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(ch);
    desc = (idma_desc64_t*)buf->next_desc;
    desc->dst = ((uint32_t*)dst)[0];

    XLOG(ch, "Change dst field for descriptor @ %p to: %p\n",   desc, desc->dst);
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API idma_status_t
idma_update_desc64_src(int32_t ch, void *src)
{
    idma_desc64_t *desc;
    idma_buf_t *buf;

    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(ch);
    desc = (idma_desc64_t*)buf->next_desc;
    desc->src = ((uint32_t*)src)[0];

    XLOG(ch, "Change src field for descriptor @ %p to: %p\n",  desc, desc->src);
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API idma_status_t
idma_update_desc64_size(int32_t ch, uint32_t size)
{
    idma_desc64_t *desc;
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(ch);
    desc = (idma_desc64_t*)buf->next_desc;
    desc->size = size;

    XLOG(ch, "Change size field for descriptor @ %p to: %d\n",  desc, desc->size);
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

#if (IDMA_USE_WIDE_API > 0)

IDMA_API idma_status_t
idma_update_desc64_dst_wide(int32_t ch, void *dst)
{
    idma_desc64_t *desc;
    idma_buf_t *buf;

    DECLARE_PS();
    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(ch);
    desc = (idma_desc64_t*)buf->next_desc;
    desc->dst = ((uint32_t*)dst)[0];
    desc->ext_dst = ((uint32_t*)dst)[1];

    XLOG(ch, "Change dst field for descriptor @ %p to: %p\n",   desc, desc->dst);
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API idma_status_t
idma_update_desc64_src_wide(int32_t ch, void *src)
{
    idma_desc64_t *desc;
    idma_buf_t *buf;

    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf  = idma_chan_buf_get(ch);
    desc = (idma_desc64_t*)buf->next_desc;
    desc->src = ((uint32_t*)src)[0];
    desc->ext_src = ((uint32_t*)src)[1];

    XLOG(ch, "Change src field for descriptor @ %p to: %p\n",  desc, desc->src);
    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

#endif
#endif

IDMA_API idma_status_t
idma_init_task (IDMA_CHAN_FUNC_ARG
                idma_buffer_t *taskh,
                idma_type_t type,
                int32_t ndescs,
                idma_callback_fn cb_func,
                void *cb_data)
{
    return idma_init_task_i(IDMA_CH_PTR, taskh, type, ndescs, cb_func, cb_data);
}

ALWAYS_INLINE idma_status_t
idma_copy_task(IDMA_CHAN_FUNC_ARG
               idma_buffer_t *taskh,
               void *dst,
               void *src,
               size_t size,
               uint32_t flags,
               void *cb_data,
               idma_callback_fn cb_func)
{
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
    return idma_copy_task_wide_i(IDMA_CH_PTR, taskh, dst, src, size, flags, cb_data, cb_func);
#else
    return idma_copy_task_i(IDMA_CH_PTR, taskh, dst, src, size, flags, cb_data, cb_func);
#endif
}

ALWAYS_INLINE idma_status_t
idma_copy_2d_task(IDMA_CHAN_FUNC_ARG
                  idma_buffer_t *taskh,
                  void *dst,
                  void *src,
                  size_t row_sz,
                  uint32_t flags,
                  uint32_t nrows,
                  uint32_t src_pitch,
                  uint32_t dst_pitch,
                  void *cb_data,
                  idma_callback_fn cb_func)
{
#ifdef IDMA_USE_WIDE_ADDRESS_COMPILE
    return idma_copy_2d_task_wide_i(IDMA_CH_PTR, taskh, dst, src, row_sz, flags, nrows, src_pitch, dst_pitch, cb_data, cb_func);
#else
    return idma_copy_2d_task_i(IDMA_CH_PTR, taskh, dst, src, row_sz, flags, nrows, src_pitch, dst_pitch, cb_data, cb_func);
#endif
}

#if (IDMA_USE_64B_DESC > 0)

IDMA_API idma_status_t
idma_copy_2d_pred_task64(int32_t ch,
                         idma_buffer_t *taskh,
                         void *dst,
                         void *src,
                         size_t row_sz,
                         uint32_t flags,
                         void* pred_mask,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch,
                         void *cb_data,
                         idma_callback_fn cb_func)
{
    return idma_copy_2d_pred_task64_i(ch, taskh, dst, src, row_sz, flags, pred_mask, nrows, src_pitch, dst_pitch, cb_data, cb_func);
}

IDMA_API idma_status_t
idma_copy_task64(int32_t ch,
                 idma_buffer_t *taskh,
                 void *dst,
                 void *src,
                 size_t size,
                 uint32_t flags,
                 void *cb_data,
                 idma_callback_fn cb_func)
{
    return idma_copy_task64_i(ch, taskh, dst, src, size, flags, cb_data, cb_func);
}

IDMA_API idma_status_t
idma_copy_2d_task64(int32_t ch,
                    idma_buffer_t *taskh,
                    void *dst,
                    void *src,
                    size_t row_sz,
                    uint32_t flags,
                    uint32_t nrows,
                    uint32_t src_pitch,
                    uint32_t dst_pitch,
                    void *cb_data,
                    idma_callback_fn cb_func)
{
    return idma_copy_2d_task64_i(ch, taskh, dst, src, row_sz, flags, nrows, src_pitch, dst_pitch, cb_data, cb_func);
}

IDMA_API idma_status_t
idma_copy_3d_task64(int32_t ch,
                    idma_buffer_t *taskh,
                    void *dst,
                    void *src,
                    uint32_t flags,
                    size_t   row_sz,
                    uint32_t nrows,
                    uint32_t ntiles,
                    uint32_t src_pitch,
                    uint32_t dst_pitch,
                    uint32_t src_tile_pitch,
                    uint32_t dst_tile_pitch,
                    void *cb_data,
                    idma_callback_fn cb_func)
{
    return idma_copy_3d_task64_i(ch, taskh, dst, src, flags, row_sz, nrows, ntiles, src_pitch, dst_pitch, src_tile_pitch, dst_tile_pitch, cb_data, cb_func);
}

IDMA_API idma_status_t
idma_add_desc64( idma_buffer_t *bufh,
                 void *dst,
                 void *src,
                 size_t size,
                 uint32_t flags)
{
    idma_buf_t *buf;
    DECLARE_PS();
    IDMA_DISABLE_INTS();
    buf = convert_buffer_to_buf(bufh);

    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | SET_CONTROL_SUBTYPE(IDMA_64B_1D_TYPE));
    set_1d_desc64_fields(buf->ch, buf->next_add_desc, dst, src, size);
    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

static inline idma_status_t
idma_add_2d_desc64( idma_buffer_t *bufh,
                    void *dst,
                    void *src,
                    size_t row_sz,
                    uint32_t flags,
                    uint32_t nrows,
                    uint32_t src_pitch,
                    uint32_t dst_pitch)
{
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = convert_buffer_to_buf(bufh);

    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | SET_CONTROL_SUBTYPE(IDMA_64B_2D_TYPE));
    set_2d_desc64_fields(buf->ch, buf->next_add_desc, dst, src, row_sz, nrows, src_pitch, dst_pitch);

    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}


static inline idma_status_t
idma_add_2d_pred_desc64( idma_buffer_t *bufh,
                         void *dst,
                         void *src,
                         size_t row_sz,
                         uint32_t flags,
                         void* pred_mask,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch)
{
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = convert_buffer_to_buf(bufh);

    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | SET_CONTROL_SUBTYPE(IDMA_64B_2D_COMPRESS_TYPE));
    set_2d_desc64_fields(buf->ch, buf->next_add_desc, dst, src, row_sz, nrows, src_pitch, dst_pitch);
    ((idma_desc64_t*)buf->next_add_desc)->pred_mask = (uint32_t)pred_mask;

    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API idma_status_t
idma_add_3d_desc64( idma_buffer_t *bufh,
                    void *dst,
                    void *src,
                    uint32_t flags,
                    size_t   row_sz,
                    uint32_t nrows,
                    uint32_t ntiles,
                    uint32_t src_row_pitch,
                    uint32_t dst_row_pitch,
                    uint32_t src_tile_pitch,
                    uint32_t dst_tile_pitch)
{
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = convert_buffer_to_buf(bufh);
    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | SET_CONTROL_SUBTYPE(IDMA_64B_3D_TYPE));
    set_3d_desc64_fields(buf->ch, buf->next_add_desc, dst, src, row_sz, nrows, ntiles, src_row_pitch, dst_row_pitch, src_tile_pitch, dst_tile_pitch);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API int32_t
idma_copy_desc64(int32_t ch, void *dst, void *src, size_t size, uint32_t flags)
{
    int32_t     ret;
    idma_buf_t* buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE | SET_CONTROL_SUBTYPE(IDMA_64B_1D_TYPE));
    set_1d_desc64_fields(ch, buf->next_desc, dst, src, size);
    ret = schedule_desc(ch, 1U);
    IDMA_ENABLE_INTS();

    return ret;
}

IDMA_API int32_t
idma_copy_2d_desc64(int32_t ch,
                    void *dst,
                    void *src,
                    size_t size,
                    uint32_t flags,
                    uint32_t nrows,
                    uint32_t src_pitch,
                    uint32_t dst_pitch)
{
    idma_buf_t* buf;
    DECLARE_PS();
    int32_t    ret = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE  | SET_CONTROL_SUBTYPE(IDMA_64B_2D_TYPE));
    set_2d_desc64_fields(ch, buf->next_desc, dst, src, size, nrows, src_pitch, dst_pitch);

    ret = schedule_desc(ch, 1U);
    IDMA_ENABLE_INTS();
    return ret;
}

IDMA_API int32_t
idma_copy_2d_pred_desc64(int32_t ch, void *dst, void *src,  size_t size, uint32_t flags, void* pred_mask, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch)
{
    idma_buf_t* buf;
    DECLARE_PS();
    int32_t    ret = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE | SET_CONTROL_SUBTYPE(IDMA_64B_2D_COMPRESS_TYPE));
    set_2d_desc64_fields(ch, buf->next_desc, dst, src, size, nrows, src_pitch, dst_pitch);
    ((idma_desc64_t*)buf->next_desc)->pred_mask = (uint32_t)pred_mask;

    ret = schedule_desc(ch, 1U);
    IDMA_ENABLE_INTS();
    return ret;
}


IDMA_API int32_t
idma_copy_3d_desc64(int32_t ch,
                    void *dst,
                    void *src,
                    uint32_t flags,
                    size_t   row_sz,
                    uint32_t nrows,
                    uint32_t ntiles,
                    uint32_t src_row_pitch,
                    uint32_t dst_row_pitch,
                    uint32_t src_tile_pitch,
                    uint32_t dst_tile_pitch)
{
    idma_buf_t* buf;
    DECLARE_PS();
    int32_t    ret = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE | SET_CONTROL_SUBTYPE(IDMA_64B_3D_TYPE));
    set_3d_desc64_fields(ch, buf->next_desc, dst, src, row_sz, nrows, ntiles, src_row_pitch, dst_row_pitch, src_tile_pitch, dst_tile_pitch);

    ret = schedule_desc(ch, 1U);

    IDMA_ENABLE_INTS();
    return ret;
}

#if (IDMA_USE_WIDE_API > 0)

IDMA_API idma_status_t
idma_copy_task64_wide(int32_t ch,
                      idma_buffer_t *taskh,
                      void *dst,
                      void *src,
                      size_t size,
                      uint32_t flags,
                      void *cb_data,
                      idma_callback_fn cb_func)
{
    return idma_copy_task64_wide_i(ch, taskh, dst, src, size, flags, cb_data, cb_func);
}

IDMA_API idma_status_t
idma_copy_2d_task64_wide(int32_t ch,
                         idma_buffer_t *taskh,
                         void *dst,
                         void *src,
                         size_t row_sz,
                         uint32_t flags,
                         uint32_t nrows,
                         uint32_t src_pitch,
                         uint32_t dst_pitch,
                         void *cb_data,
                         idma_callback_fn cb_func)
{
    return idma_copy_2d_task64_wide_i(ch, taskh, dst, src, row_sz, flags, nrows, src_pitch, dst_pitch, cb_data, cb_func);
}

IDMA_API idma_status_t
idma_copy_3d_task64_wide(int32_t ch,
                         idma_buffer_t *taskh,
                         void *dst,
                         void *src,
                         uint32_t flags,
                         size_t   row_sz,
                         uint32_t nrows,
                         uint32_t ntiles,
                         uint32_t src_pitch,
                         uint32_t dst_pitch,
                         uint32_t src_tile_pitch,
                         uint32_t dst_tile_pitch,
                         void *cb_data,
                         idma_callback_fn cb_func)
{
    return idma_copy_3d_task64_wide_i(ch, taskh, dst, src, flags, row_sz, nrows, ntiles, src_pitch, dst_pitch, src_tile_pitch, dst_tile_pitch, cb_data, cb_func);
}

IDMA_API idma_status_t
idma_add_desc64_wide( idma_buffer_t *bufh,
                      void *dst,
                      void *src,
                      size_t size,
                      uint32_t flags)
{
    idma_buf_t *buf;
    DECLARE_PS();
    IDMA_DISABLE_INTS();
    buf = convert_buffer_to_buf(bufh);

    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_1D_TYPE));
    set_1d_desc64_fields_wide(buf->ch, buf->next_add_desc, dst, src, size);
    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API int32_t
idma_copy_desc64_wide(int32_t ch, void *dst, void *src, size_t size, uint32_t flags)
{
    int32_t     ret;
    idma_buf_t* buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_1D_TYPE));
    set_1d_desc64_fields_wide(ch, buf->next_desc, dst, src, size);
    ret = schedule_desc(ch, 1U);
    IDMA_ENABLE_INTS();

    return ret;
}

IDMA_API idma_status_t
idma_copy_2d_pred_task64_wide(int32_t ch,
                              idma_buffer_t *taskh,
                              void *dst,
                              void *src,
                              size_t row_sz,
                              uint32_t flags,
                              void* pred_mask,
                              uint32_t nrows,
                              uint32_t src_pitch,
                              uint32_t dst_pitch,
                              void *cb_data,
                              idma_callback_fn cb_func)
{
    return idma_copy_2d_pred_task64_wide_i(ch, taskh, dst, src, row_sz, flags, pred_mask, nrows, src_pitch, dst_pitch, cb_data, cb_func);
}


static inline idma_status_t
idma_add_2d_pred_desc64_wide( idma_buffer_t *bufh, void *dst, void *src,  size_t row_sz,  uint32_t flags, void* pred_mask, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch)
{
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = convert_buffer_to_buf(bufh);

    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_2D_COMPRESS_TYPE));
    set_2d_desc64_fields_wide(buf->ch, buf->next_add_desc, dst, src, row_sz, nrows, src_pitch, dst_pitch);
    ((idma_desc64_t*)buf->next_add_desc)->pred_mask = (uint32_t)pred_mask;

    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

static inline idma_status_t
idma_add_2d_desc64_wide( idma_buffer_t *bufh, void *dst, void *src,  size_t row_sz,  uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch)
{
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();
    buf = convert_buffer_to_buf(bufh);

    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_2D_TYPE));
    set_2d_desc64_fields_wide(buf->ch, buf->next_add_desc, dst, src, row_sz, nrows, src_pitch, dst_pitch);
    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API idma_status_t
idma_add_3d_desc64_wide( idma_buffer_t *bufh,
                         void *dst,
                         void *src,
                         uint32_t flags,
                         size_t   row_sz,
                         uint32_t nrows,
                         uint32_t ntiles,
                         uint32_t src_row_pitch,
                         uint32_t dst_row_pitch,
                         uint32_t src_tile_pitch,
                         uint32_t dst_tile_pitch)
{
    idma_buf_t *buf;
    DECLARE_PS();

    IDMA_DISABLE_INTS();

    buf = convert_buffer_to_buf(bufh);
    set_desc64_ctrl(buf->next_add_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_3D_TYPE));
    set_3d_desc64_fields_wide(buf->ch, buf->next_add_desc, dst, src, row_sz, nrows, ntiles, src_row_pitch, dst_row_pitch, src_tile_pitch, dst_tile_pitch);
    update_next_add_ptr(buf);

    IDMA_ENABLE_INTS();
    return IDMA_OK;
}

IDMA_API int32_t
idma_copy_2d_desc64_wide(int32_t ch, void *dst, void *src,  size_t size, uint32_t flags, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch)
{
    idma_buf_t* buf;
    DECLARE_PS();
    int32_t    ret = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_2D_TYPE));
    set_2d_desc64_fields_wide(ch, buf->next_desc, dst, src, size, nrows, src_pitch, dst_pitch);

    ret = schedule_desc(ch, 1U);
    IDMA_ENABLE_INTS();
    return ret;
}

IDMA_API int32_t
idma_copy_2d_pred_desc64_wide(int32_t ch, void *dst, void *src,  size_t size, uint32_t flags, void* pred_mask, uint32_t nrows, uint32_t src_pitch, uint32_t dst_pitch)
{
    idma_buf_t* buf;
    DECLARE_PS();
    int32_t    ret = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_2D_COMPRESS_TYPE));
    set_2d_desc64_fields_wide(ch, buf->next_desc, dst, src, size, nrows, src_pitch, dst_pitch);
    ((idma_desc64_t*)buf->next_desc)->pred_mask = (uint32_t)pred_mask;

    ret = schedule_desc(ch, 1U);
    IDMA_ENABLE_INTS();
    return ret;
}

IDMA_API int32_t
idma_copy_3d_desc64_wide(int32_t ch,
                         void *dst,
                         void *src,
                         uint32_t flags,
                         size_t   row_sz,
                         uint32_t nrows,
                         uint32_t ntiles,
                         uint32_t src_row_pitch,
                         uint32_t dst_row_pitch,
                         uint32_t src_tile_pitch,
                         uint32_t dst_tile_pitch)
{
    idma_buf_t* buf;
    DECLARE_PS();
    int32_t    ret = 0;

    IDMA_DISABLE_INTS();

    buf = idma_chan_buf_get(ch);
    set_desc64_ctrl(buf->next_desc, flags, IDMA_64B_DESC_CODE | IDMA_64B_LONG_ADDR_CTRL_BIT_MASK | SET_CONTROL_SUBTYPE(IDMA_64B_3D_TYPE));
    set_3d_desc64_fields_wide(ch, buf->next_desc,  dst, src, row_sz, nrows, ntiles, src_row_pitch, dst_row_pitch, src_tile_pitch, dst_tile_pitch);
    ret = schedule_desc(ch, 1U);

    IDMA_ENABLE_INTS();
    return ret;
}
#endif
#endif //64B


IDMA_API idma_status_t
idma_process_tasks(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_process_tasks_i(IDMA_CH_PTR);
}

IDMA_API idma_error_details_t*
idma_error_details(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_error_details_i(IDMA_CH_PTR);
}

IDMA_API idma_error_details_t*
idma_buffer_error_details(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_error_details_i(IDMA_CH_PTR);
}

IDMA_API idma_hw_error_t
idma_buffer_check_errors(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_buffer_check_errors_i(IDMA_CH_PTR);
}

IDMA_API idma_status_t
idma_abort_tasks(IDMA_CHAN_FUNC_ARG_SINGLE)
{
    return idma_abort_tasks_i(IDMA_CH_PTR);
}


IDMA_API int32_t
idma_task_status(idma_buffer_t *taskh)
{
    idma_buf_t* task;
    task = convert_buffer_to_buf(taskh);

    XLOG(task->ch, "Task %p status '%s' (%d)\n", task,
         (task->status == (int32_t)IDMA_TASK_ERROR) ? "Error"     :
         (task->status == (int32_t)IDMA_TASK_DONE)  ? "Done" :
         (task->status >  (int32_t)IDMA_TASK_DONE)  ? "Pending"   : "UNKNOWN", task->status);

    return (task->status);
}


IDMA_API int32_t
idma_schedule_desc_clock(IDMA_CHAN_FUNC_ARG
                         uint32_t count,
                         uint32_t *ptime)
{
    int32_t ret;
    DECLARE_PS();

    IDMA_DISABLE_INTS();
    ret = schedule_desc(IDMA_CH_PTR, count);
    *ptime = (uint32_t) clock();          // parasoft-suppress MISRA2012-RULE-21_10-2 "This function is used for profiling only."
    IDMA_ENABLE_INTS();
    return ret;
}

IDMA_API int32_t
idma_schedule_desc_fast_clock(IDMA_CHAN_FUNC_ARG
                              uint32_t count,
                              uint32_t *ptime)
{
    int32_t ret;
    DECLARE_PS();

    IDMA_DISABLE_INTS();
    ret = schedule_desc_fast(IDMA_CH_PTR, count);
    *ptime = (uint32_t) clock();          // parasoft-suppress MISRA2012-RULE-21_10-2 "This function is used for profiling only."
    IDMA_ENABLE_INTS();
    return ret;
}

#ifdef __cplusplus
}
#endif

#endif

#endif /* __IDMA_H__ */
