/**
 * @file      user_util.h
 * @brief     kdp host lib user test header file
 * @version   0.1 - 2019-08-01 
 * @copyright (c) 2019 Kneron Inc.All right reserved.
 */

#ifndef __USER_UTIL__H__
#define __USER_UTIL__H__
#include <signal.h>
#include <stdint.h>
#if defined (__cplusplus) || defined (c_plusplus)
extern "C"{
#endif

#define TEST_OTA_DIR                  "../../app_binaries/KL520/ota/"

#define IMAGE_DATA_FILE (TEST_IMG_DIR "u1_f1_rgb.bin")
#define IMAGE_DATA_FILE_2 (TEST_IMG_DIR "u16_f1_rgb.bin")

#define IMAGE_SIZE (640*480*2)
#define SLEEP_TIME 1000
#define HOSTLIB_SIG  SIGINT | SIGTERM 

bool check_ctl_break(void);
int register_hostlib_signal(void);
int write_buf_to_file(char* buf, const char* fn, uint32_t size);
int read_file_to_buf_2(char** buf, const char* fn);
int read_file_to_buf(char* buf, const char* fn, int nlen);
char *bmp_to_rgb565_auto_malloc(const char *file_path, int *width, int *height);

// return : auto-allocated buffer of file content
// buffer_size : as output to report the size of file content
char *read_file_to_buffer_auto_malloc(const char *file_path, long *buffer_size);
double what_time_is_it_now();
struct kdp_dme_cfg_s create_dme_cfg_struct();
struct kdp_isi_cfg_s create_isi_cfg_struct();
struct kdp_metadata_s create_metadata_struct();

/// @brief (CI only) prepare ci_logfile_name
/// @details [executable_name].log will be generated
///
/// @param argc main's argc
/// @param argv main's argv
/// @returen ci_log_file_name
///
const char* ci_prepare_logfile_name(int argc, char*argv[]);

/// @brief (CI only) print data to log file 
/// @details [executable_name].log will be generated
///
/// @param fmt printf format
/// @param ... printf arguments list 
/// @returen true if no error
///
bool print2log(const char* fmt, ...);
#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

#endif
