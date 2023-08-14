/**
 * @file      user_util.h
 * @brief     kdp host lib user test header file
 * @version   0.1 - 2019-08-01 
 * @copyright (c) 2019 Kneron Inc.All right reserved.
 */

#ifndef __USER_UTIL__H__
#define __USER_UTIL__H__
#include <stdint.h>
#include <signal.h>
#if defined (__cplusplus) || defined (c_plusplus)
extern "C"{
#endif

#define TEST_DFU_DIR                  "../../app_binaries/KL720/dfu/"

#define IMAGE_SIZE (640*480*2)
#define SLEEP_TIME 1000
#define HOSTLIB_SIG  SIGINT | SIGTERM 

bool check_ctl_break(void);
int register_hostlib_signal(void);
int write_buf_to_file(char* buf, const char* fn, uint32_t size);
int read_file_to_buf(char* buf, const char* fn, int nlen);
int read_file_to_buf_2(char** buf, const char* fn);
double what_time_is_it_now();
struct kdp_dme_cfg_s create_dme_cfg_struct();
struct kdp_isi_cfg_s create_isi_cfg_struct();
struct kdp_metadata_s create_metadata_struct();

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

#endif
