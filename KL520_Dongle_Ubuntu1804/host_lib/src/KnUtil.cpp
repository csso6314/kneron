/**
 * @file      KnUtil.cpp
 * @brief     utility implementation file
 * @version   0.1 - 2019-04-29
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "KnLog.h"
#include "KnUtil.h"

#include <stdio.h>
#include <string.h>

//#if defined(THREAD_USE_CPP11)
#include <errno.h>
//#endif 

#define DUMP_DBG_MSG  0

int read_file_to_buf(char* buf, const char* fn, int nlen) {
    if(buf == NULL || fn == NULL) {
        KnPrint(KN_LOG_ERR, "file name or buffer is null!\n");
        return -1;
    }

    FILE* in = fopen(fn, "rb");
    if(!in) {
        KnPrint(KN_LOG_ERR, "opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }

    fseek(in, 0, SEEK_END);
    long f_n = ftell(in); //get the size
    if(f_n > nlen) {
        KnPrint(KN_LOG_ERR, "buffer is not enough.!\n");
        return -1;
    }

    fseek(in, 0, SEEK_SET); //move to begining

    int res = 0;
    while(1) {
        int len = fread(buf+res, 1, 1024, in);
        if(len == 0) {
            if(!feof(in)) {
                KnPrint(KN_LOG_ERR, "reading from img file failed:%s.!\n", fn);
                return -1;
            }
            break;
        }
        res += len; //calc size
    }
    return res;
}

int copy_file(const char* src, const char* dest) {
    if(src == NULL || dest == NULL) {
        KnPrint(KN_LOG_ERR, "copy from %s to %s failed.!\n", src, dest);
        return -1;
    }

    FILE* in = fopen(src, "r+");
    if(!in) {
        KnPrint(KN_LOG_ERR, "opening file %s failed, error:%s.!\n", src, strerror(errno));
        return -1;
    }

    FILE* out = fopen(dest, "w+");
    if(!out) {
        KnPrint(KN_LOG_ERR, "opening file %s failed, error:%s.!\n", dest, strerror(errno));
        return -1;
    }

    int len = 0;
    char buf[1024];

    while(1) {
        len = fread(buf, 1, sizeof(buf), in);

        if(len == 0) {
            if(!feof(in)) {
                KnPrint(KN_LOG_ERR, "reading from src file failed:%s.!\n", src);
                return -1;
            }
            break;
        }

        fwrite(buf, 1, len, out);
    }
    return 0;
}

//it is only used for debug. 
//must be disabled when release;
void kn_dump_msg(int8_t* buf, int len, bool to_dev) {
#if !DUMP_DBG_MSG
    return;
#endif

    printf("\n");
    if(to_dev) {
        printf("host is sending to dev ------");
    } else {
        printf("host is receiving from dev ------");
    }

    for(int i = 0; i < len; i++) {
        if(i % 16 == 0) {
            printf("\n");
        }

        printf("0x%02x ", (unsigned char)buf[i]);
    }

    printf("\n---------\n\n");
}

uint16_t gen_crc16(int8_t *data, uint16_t size) {
    uint16_t out = 0;
    int bits_read = 0, bit_flag, i;

    /* Sanity check: */
    if (data == NULL) return 0;

    while (size > 0) {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;

        int8_t tmp = *data;
        uint8_t t8 = (uint8_t)tmp;
        uint16_t t16 = t8;
        out |= (t16 >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if (bits_read > 7) {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if (bit_flag) out ^= CRC16_CONSTANT;
    }

    // push out the last 16 bits
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if (bit_flag) out ^= CRC16_CONSTANT;
    }

    // reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>= 1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}

