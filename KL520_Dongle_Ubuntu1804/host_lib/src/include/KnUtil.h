/**
 * @file      KnUtil.h
 * @brief     utility header file
 * @version   0.1 - 2019-04-29
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __KN_UTIL__H__
#define __KN_UTIL__H__

#define CRC16_CONSTANT 0x8005

#include <stdint.h>

enum FID_DB_OP_MODE {
    DB_IMPORT   = 1,
    DB_EXPORT   = 2,
    DB_QUERY    = 3,
    DB_ADD_FM   = 4,
    DB_DEL_FM   = 5,
    DB_QUERY_FM = 6,
    DB_OP_MAX   = 7
};

enum FID_DB_CONFIG_MODE {
    DB_SET_CONFIG = 1,
    DB_GET_CONFIG,
    DB_GET_DB_INDEX,
    DB_SWITCH_DB_INDEX,
    DB_SET_VERSION,
    DB_GET_VERSION,
    DB_GET_META_DATA_VERSION
};

int copy_file(const char* src, const char* dest);

void kn_dump_msg(int8_t* buf, int len, bool to_dev = true);

uint16_t gen_crc16(int8_t *data, uint16_t size);
extern "C" {
int read_file_to_buf(char* buf, const char* fn, int nlen);
}
#endif

