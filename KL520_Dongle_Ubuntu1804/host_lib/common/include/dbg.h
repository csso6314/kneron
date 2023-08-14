/**
 * @file      dbg.h
 * @brief     debug macro 
 * @copyright (c) 2018 Kneron Inc. All right reserved.
 */

#ifndef __DBG_H__
#define __DBG_H__

#include <stdio.h>
#ifdef TARGET_NCPU
#include <fdebug.h>
#endif
#include "ipc.h"

#define LOG_NONE        0
#define LOG_CRITICAL    1
#define LOG_ERROR       2
#define LOG_USER        3

#define LOG_INFO        4
#define LOG_TRACE       5
#define LOG_DBG         6

#define LOG_PROFILE     9

#ifdef LOG_ENABLE

#ifdef TARGET_NCPU

extern struct scpu_to_ncpu_s *in_comm_p;

#define MSG(level, format, ...) \
    do { \
        int lvl = in_comm_p->debug_flags & 0x0000000F; \
        if (lvl == LOG_PROFILE) { \
            if (level == lvl)      \
                fLib_printf(format, ##__VA_ARGS__); \
        } else if (level > 0 && level <= lvl)     \
            fLib_printf(format, ##__VA_ARGS__); \
    } while (0)

#define log_get_level_ncpu()    (in_comm_p->debug_flags & 0x0000000F)

#endif

#define LOG(level, format, ...) \
    do { \
        fLib_printf("[%d|%s@%s,%d] " format, \
            level, __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
    } while (0)

#define _DSG(__flag__, __format__, ...) { if (__flag__ == LOG_INFO) fLib_printf(__format__"\n", ##__VA_ARGS__); }
#define DSG(__format__, ...) { fLib_printf(__format__"\n", ##__VA_ARGS__); }
#define DSG_NOLF(__format__, ...) { fLib_printf(__format__, ##__VA_ARGS__); }

#define dbg_msg(fmt, ...) MSG(LOG_DBG, fmt, ##__VA_ARGS__)
#define trace_msg(fmt, ...) MSG(LOG_TRACE, fmt, ##__VA_ARGS__)
#define info_msg(fmt, ...) MSG(LOG_INFO, fmt, ##__VA_ARGS__)
#define err_msg(fmt, ...) MSG(LOG_ERROR, fmt, ##__VA_ARGS__)
#define critical_msg(fmt, ...) MSG(LOG_CRITICAL, fmt, ##__VA_ARGS__)
#define profile_msg(fmt, ...) MSG(LOG_PROFILE, fmt, ##__VA_ARGS__)

#else

#define LOG(level, format, ...)
#define _DSG(__flag__, __format__, ...)
#define DSG(__format__, ...)
#define MSG(level, format, ...)
#define dbg_msg(fmt, ...)
#define trace_msg(fmt, ...)
#define info_msg(fmt, ...)
#define err_msg(fmt, ...)
#define critical_msg(fmt, ...)

#endif // LOG_ENABLE

#define dbg_msg_api(fmt, ...)       dbg_msg(fmt, ##__VA_ARGS__)
#define dbg_msg_app(fmt, ...)       dbg_msg(fmt, ##__VA_ARGS__)
#define dbg_msg_algo(fmt, ...)      critical_msg(fmt, ##__VA_ARGS__)
#define dbg_msg_console(fmt, ...)   critical_msg(fmt "\n", ##__VA_ARGS__)
#define dbg_msg_user(fmt, ...)      MSG(LOG_USER, fmt, ##__VA_ARGS__)

#endif // __DBG_H__
