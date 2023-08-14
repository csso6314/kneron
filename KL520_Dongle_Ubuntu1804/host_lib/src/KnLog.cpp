/**
 * @file      KnLog.cpp
 * @brief     Implementation file for LOG handler
 * @version   0.1 - 2019-04-12
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "KnLog.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "KdpHostApi.h"
mutex log_mtx;

#define GF_LOG_MAX_SIZE   (10 * 1024 * 1024)

bool g_log_to_tty = false;

static KN_LOG_LEVEL    g_log_running_level;
static FILE*           g_log_fd = NULL;   // file pointer to opened log file
static string file_name;

void KnLogInit(const string dir, const string fn)
{
    g_log_running_level = KN_LOG_ALL;

    // create log directory if it's not existed
    if (0 != access(dir.c_str(), F_OK)) {
#ifdef __linux__
        int flag = mkdir(dir.c_str(), 0777);
#elif _WIN32
        int flag = mkdir(dir.c_str());
#endif
        if (flag < 0)   {
            printf("create log directory: %s failed!\n", dir.c_str());
            return;
        }
    }

    // open log file & set buffer size
    file_name = dir;
    file_name += fn;
    g_log_fd = fopen (file_name.c_str(), "a+");
    if (g_log_fd == NULL) {
        printf ("fopen failed when opening log file:%s.\n", file_name.c_str());
        return;
    }
    setvbuf (g_log_fd, NULL, _IOLBF, 0);
}


void FaBackUpLogFile(const string dir) {
    if (0 != access(dir.c_str(), W_OK))
        return;

    struct stat statbuf;
    stat(dir.c_str(), &statbuf);
    if (statbuf.st_size > GF_LOG_MAX_SIZE) {
        fclose(g_log_fd);
        string new_dir = dir + ".bak";
        rename(dir.c_str(), new_dir.c_str());
        g_log_fd = fopen(dir.c_str(), "a+");
        if (g_log_fd == NULL) {
            printf("fopen failed when opening log file:%s.\n", dir.c_str());
            return;
        }
        setvbuf(g_log_fd, NULL, _IOLBF, 0);
    }
}

void KnLogClose() {
    if (g_log_fd == NULL) return;
    fclose(g_log_fd);
}

void KnSetLogLevel (KN_LOG_LEVEL loglevel)
{
    g_log_running_level = loglevel;
}

void KnPrint (KN_LOG_LEVEL loglevel, const char *format, ...)
{    
    if (g_log_fd == NULL) return;
    if (loglevel > g_log_running_level) return;
    log_mtx.lock();
    FaBackUpLogFile(file_name.c_str());

    // get input text
    char buf[MAX_LOG_SIZE];    
    memset(buf, 0, sizeof(buf));
    va_list args;    
    va_start(args, format);    
    vsnprintf(buf, sizeof(buf) - 1, format, args);    
    va_end(args);    

    fputs(buf, g_log_fd);

    if (g_log_to_tty) {
        fputs(buf, stdout);
    }
    log_mtx.unlock();
}

void KnTsPrint (KN_LOG_LEVEL loglevel, const char *format, ...)
{
    if (g_log_fd == NULL) return;
    if (loglevel > g_log_running_level) return;
    log_mtx.lock();
    FaBackUpLogFile(file_name.c_str());

    // get input text
    char buf[MAX_LOG_SIZE];    
    memset(buf, 0, sizeof(buf));
    va_list args;    
    va_start(args, format);    
    vsnprintf(buf, sizeof(buf) - 1, format, args);    
    va_end(args);    

    // get current time
    char tbuf[128];
    memset(tbuf, 0, sizeof(tbuf));
    struct timeval time;
    gettimeofday(&time, NULL);

    // make timestamp
    struct tm t_tm;
    memset(&t_tm, 0, sizeof(struct tm));
    const time_t sec=time.tv_sec;

#ifdef _WIN32
    gmtime_s(&t_tm, &sec);
#elif __linux__
    gmtime_r(&sec, &t_tm);
#endif

    snprintf(tbuf, sizeof(tbuf)-1, "%d-%d-%d::%d-%d-%d::%ld -  ", 
        t_tm.tm_year+1900, t_tm.tm_mon+1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec, time.tv_usec);

    // make output text
    char mbuf[MAX_LOG_SIZE];
    memset(mbuf, 0, sizeof(mbuf)-1);
    strncat(mbuf, tbuf, sizeof(mbuf)-1);
    strncat(mbuf, buf, sizeof(mbuf)-1);

    fputs(mbuf, g_log_fd);

    if (g_log_to_tty) {
        fputs(mbuf, stdout);
    }
    log_mtx.unlock();
}
