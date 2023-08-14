/**
 * @file      KnLog.h
 * @brief     Header file for LOG handler
 * @version   0.1 - 2019-04-12
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __KN_LOG_H__
#define __KN_LOG_H__

#define KN_LOG_DIR  "/tmp/log/"
#define KN_LOG_FILE "mzt.log"
#define MAX_LOG_SIZE 2048

#include <string>

using namespace std;

typedef enum {
    KN_LOG_STOP = 0,
    KN_LOG_FATAL,
    KN_LOG_ERR,
    KN_LOG_MSG,
    KN_LOG_DBG,
    KN_LOG_TRACE,
    KN_LOG_ALL
} KN_LOG_LEVEL;


void KnLogInit(const string dir = KN_LOG_DIR, const string fn = KN_LOG_FILE);
void KnPrint(KN_LOG_LEVEL log_level, const char *format, ...);      // print text to log file
void KnTsPrint(KN_LOG_LEVEL log_level, const char *format, ...);    // print text to log file with timestamp
void KnSetLogLevel(KN_LOG_LEVEL loglevel);
void KnLogClose();

#endif
