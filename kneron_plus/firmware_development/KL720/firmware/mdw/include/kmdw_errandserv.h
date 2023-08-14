
/**
 * @file        kmdw_errandserv.h
 * @brief       errand service handler
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_ERRANDSERV_H__
#define __KMDW_ERRANDSERV_H__

#include "cmsis_os2.h"

typedef void (*errand_function_t)(void *arg);

// start errand service and hence create a thread for this
// return 0 means OK
int kmdw_errandserv_start(void);

// stop errand service
// return 0 means OK
int kmdw_errandserv_finish(void);

// enqueue an errand task (like bottom-half in Linux), can be executed from whatever thread/ISR context
// return 0 means OK
// errand_func : must have
// arg : optional
// execute_time : 0 = immediately, unit in milliseconds
int kmdw_errandserv_run_task(errand_function_t func, void *arg, uint32_t execute_time);

#endif
