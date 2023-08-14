/**
 * @file      err_code.h
 * @brief     Definition for rsp error code
 * @version   0.1 - 2019-05-05
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

#if defined (__cplusplus) || defined (c_plusplus)
extern "C"{
#endif

#include "com_err.h"

void error_code_2_str(int code, char* buf, int len);

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

#endif
