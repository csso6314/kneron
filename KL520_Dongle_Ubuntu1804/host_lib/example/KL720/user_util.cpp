/**
 * @file      user_util.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2019-07-31
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"
#include "user_util.h"
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#if defined (__cplusplus) || defined (c_plusplus)
extern "C"{
#endif

volatile uint32_t term_sig = 0;

bool check_ctl_break(void)
{

    return  (1 == term_sig);

}

static void sig_term_handler(int signo)
{
    if (0 != ((HOSTLIB_SIG) & signo)) {
        term_sig = 1;
    }
}

int register_hostlib_signal(void) 
{
    if (signal(SIGINT, sig_term_handler) == SIG_IGN)
        signal(SIGINT, SIG_IGN);

    if (signal(SIGTERM, sig_term_handler) == SIG_IGN)
        signal(SIGTERM, SIG_IGN);
    return 0;
}


int write_buf_to_file(char* buf, const char* fn, uint32_t size) {
    if ((NULL == buf) || (NULL == fn)) {
        printf("file name or buffer is null!\n");
        return -1;
    }

    FILE* out = fopen(fn, "wb");
    if (NULL == out) {
        printf("opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }
 
    fwrite (buf , sizeof(char), size, out);
    fclose(out);
	
    return 0;
}

int read_file_to_buf_2(char** buf, const char* fn) {
    if (NULL == fn) {
        printf("file name is null!\n");
        return -1;
    }

    FILE* in = fopen(fn, "rb");
    if (!in) {
        printf("opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }

    fseek(in, 0, SEEK_END);
    long size = ftell(in);  //get the size
 
    *buf = (char *) malloc(size);
    memset(*buf, 0, size);
    fseek(in, 0, SEEK_SET);  //move to begining

    int res = 0;
    while (1) {
        int len = fread(*buf + res, 1, 1024, in);
        if (len == 0) {
            if (!feof(in)) {
                printf("reading from img file failed:%s.!\n", fn);
                fclose(in);
                return -1;
            }
            break;
        }
        res += len;  //calc size
    }
    fclose(in);
    return res;
}

int read_file_to_buf(char* buf, const char* fn, int nlen) {
    if (buf == NULL || fn == NULL) {
        printf("file name or buffer is null!\n");
        return -1;
    }

    FILE* in = fopen(fn, "rb");
    if (!in) {
        printf("opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }

    fseek(in, 0, SEEK_END);
    long f_n = ftell(in);  //get the size
    if (f_n > nlen) {
        printf("buffer is not enough.!\n");
        fclose(in);
        return -1;
    }

    fseek(in, 0, SEEK_SET);  //move to begining

    int res = 0;
    while (1) {
        int len = fread(buf + res, 1, 1024, in);
        if (len == 0) {
            if (!feof(in)) {
                printf("reading from img file failed:%s.!\n", fn);
                fclose(in);
                return -1;
            }
            break;
        }
        res += len;  //calc size
    }
    fclose(in);
    return res;
}

double what_time_is_it_now() {
    struct timeval time;
    if (gettimeofday(&time, NULL)) {
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

struct kdp_dme_cfg_s create_dme_cfg_struct() {
    struct kdp_dme_cfg_s dme_cfg;
    memset(&dme_cfg, 0, sizeof(kdp_dme_cfg_s));
    return dme_cfg;
}

struct kdp_isi_cfg_s create_isi_cfg_struct() {
    struct kdp_isi_cfg_s isi_cfg;
    memset(&isi_cfg, 0, sizeof(kdp_isi_cfg_s));
    return isi_cfg;
}

struct kdp_metadata_s create_metadata_struct() {
    struct kdp_metadata_s metadata;
    memset(&metadata, 0, sizeof(kdp_metadata_s));
    return metadata;
}

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif
