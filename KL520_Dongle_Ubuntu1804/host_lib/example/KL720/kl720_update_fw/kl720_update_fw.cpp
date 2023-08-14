/**
 * @file      user_test_fw.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2019-08-01
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "user_util.h"
#include "com.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

#define FW_SCPU_FILE (TEST_DFU_DIR "/ready_to_load/fw_scpu.bin")
#define FW_NCPU_FILE (TEST_DFU_DIR "/ready_to_load/fw_ncpu.bin")
#define FW_FILE_SIZE_SCPU (128 * 1024)
#define FW_FILE_SIZE_NCPU (2 * 1024 * 1024)

    char img_buf[FW_FILE_SIZE_NCPU];
    /* Check the valid format of dfu firmware */
    uint32_t dfu_fmt_chk(uint8_t *data, uint32_t len)
    {
        for (uint32_t i = 0; i < len; i++) {
            if (*(data + i) != 0 && *(data + i) != 0xFF)
                return 0;
        }
        return -1;
    }

    /*
     * Calculate the 32-bit checksum of object using 32-bit size block
     */
    uint32_t gen_sum32(uint32_t *data, uint32_t size)
    {
        uint32_t sum = 0, i;
        for (i = 0; i < (size & 0xFFFFFFFC); i += 4) {
            sum += *data;
            data++;
        }
        i = size & 3;
        if (i)  // ending address misaligned ?
        {
            size = *data;
            size <<= (4 - i) * 8;
            size >>= (4 - i) * 8;
            sum += size;
        }
        return(sum);
    }


    //user test update firmware
    int user_test_fw(int dev_idx, int user_id)
    {
        //udt firmware
        uint32_t module_id = user_id;
        uint32_t buf_len = 0;
        uint32_t chk_len = 0;
        int buf_len_ret = 0;

        if (module_id != UPDATE_MODULE_SCPU && module_id != UPDATE_MODULE_NCPU)
        {
            printf("invalid module id: %d...\n", module_id);
            return -1;
        }

        printf("starting update fw ...\n");

        if (module_id == UPDATE_MODULE_SCPU)
        {
            buf_len_ret = read_file_to_buf(img_buf, FW_SCPU_FILE, FW_FILE_SIZE_SCPU);
            if (buf_len_ret <= 0)
            {
                printf("\n!!!! Failed to read '%s', please put the file in the same directory with the executable\n\n", FW_SCPU_FILE);
                return -1;
            }
        }
        else
        {
            buf_len_ret = read_file_to_buf(img_buf, FW_NCPU_FILE, FW_FILE_SIZE_NCPU);
            if (buf_len_ret <= 0)
            {
                printf("\n!!!! Failed to read '%s', please put the file in the same directory with the executable\n\n", FW_NCPU_FILE);
                return -1;
            }
        }
        buf_len = buf_len_ret;
        chk_len = buf_len < 512? buf_len : 512;
        if (dfu_fmt_chk((uint8_t*)img_buf, chk_len) != 0) {
            printf("Error, file format fail, Please check your firmware\n");
            return -1;
        }
        // check checksum
        uint32_t chksum32 = gen_sum32((uint32_t*)img_buf, buf_len - 4);
        if (chksum32 != *(u32 *)(img_buf + buf_len - 4)) {
            printf("Checksum fail, please check the firmware format\n");
            printf("fw size=%x, checksum=%x, get=%x\n",buf_len, *(u32 *)(img_buf + buf_len - 4), chksum32);
            return -1;
        } 
          
        int ret = kdp_update_fw(dev_idx, &module_id, img_buf, buf_len);
        if (ret != 0)
        {
            printf("could not update fw..\n");
            return -1;
        }

        printf("update firmware succeeded...\n");

        return 0;
    }

    //user test get version id
    int user_fw_id(int dev_idx)
    {
        int ret = 0;
        uint32_t sfirmware_id = 0;
        uint32_t sbuild_id = 0;
        uint32_t nfirmware_id = 0;
        uint32_t nbuild_id = 0;
        uint16_t sys_status = 0;
        uint16_t app_status = 0;
        printf("starting report sys status ...\n");
        ret = kdp_report_sys_status(dev_idx, &sfirmware_id, &sbuild_id,
                                    &sys_status, &app_status, &nfirmware_id, &nbuild_id);
        if (ret != 0)
        {
            printf("could not report sys status..\n");
            return -1;
        }
        printf("report sys status succeeded...\n");
        printf("\nSCPU firmware_id %08x build_id %08x\n", sfirmware_id, sbuild_id);
        printf("NCPU firmware_id %08x build_id %08x\n\n", nfirmware_id, nbuild_id);
        return 0;
    }

    int user_test(int dev_idx, int user_id)
    {
        //udt firwmare test
        int ret = user_test_fw(dev_idx, user_id);
        //udt firware id test
        if (ret == 0)
            ret = user_fw_id(dev_idx);

        return ret;
    }

    int main(int argc, char *argv[])
    {
        int ret = 0;
        int dev_idx = 0;
        int update_id = 0;

        if (argc < 2)
        {
            printf("\n");
            printf("usage: ./%s [update_id]]\n", argv[0]);
            printf("\n[update_id] 1: SCPU firmware, 2: NCPU firmware\n");
            ret = -1;
            goto FUNC_OUT;
        }

        update_id = atoi(argv[1]);

        if (update_id != 1 && update_id != 2)
        {
            printf("\nerror !! invalid update_id\n");
            ret = -1;
            goto FUNC_OUT;
        }

        printf("init kdp host lib log....\n");

        if (kdp_lib_init() < 0)
        {
            printf("init for kdp host lib failed.\n");
            ret = -1;
            goto FUNC_OUT;
        }

        printf("adding devices....\n");
        dev_idx = kdp_connect_usb_device(1);
        if (dev_idx < 0)
        {
            printf("add device failed.\n");
            ret = -1;
            goto FUNC_OUT;
        }

        printf("start kdp host lib....\n");
        if (kdp_lib_start() < 0)
        {
            printf("start kdp host lib failed.\n");
            ret = -1;
            goto FUNC_OUT;
        }

        ret = user_test(dev_idx, update_id);

FUNC_OUT:
        if (0 != ret) {
            exit(ret);
        }

        return 0;
    }

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
