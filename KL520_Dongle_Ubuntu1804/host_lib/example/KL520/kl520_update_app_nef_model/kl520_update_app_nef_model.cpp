/**
 * @file      kl520_update_app_nef_model.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2019-08-01
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"

#include <string.h>
#include <unistd.h>
#include "user_util.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define FW_SCPU_FILE (TEST_OTA_DIR "/ready_to_load/fw_scpu.bin")
#define FW_NCPU_FILE (TEST_OTA_DIR "/ready_to_load/fw_ncpu.bin")
#define FW_FILE_SIZE (128 * 1024)
#define MODEL_FILE (TEST_OTA_DIR "/ready_to_load/models_520.nef")
#define MD_FILE_SIZE (28 * 1024 * 1024 + 4 * 1024)


//user test update firmware
int user_test_app(int dev_idx, int user_id)
{
    //udt firmware
    uint32_t module_id = user_id;
    char img_buf[FW_FILE_SIZE];
    uint32_t buf_len = 0;
    int buf_len_ret = 0;

    if(module_id != 0 && module_id != 1 && module_id != 2) {
        printf("invalid module id:%d ...\n", user_id);
        return -1;
    }

    printf("starting update fw ...\n");

    //update scpu
    module_id = 1;
    buf_len_ret = read_file_to_buf(img_buf, FW_SCPU_FILE, FW_FILE_SIZE);

    if (buf_len_ret  <= 0) {
        printf("reading file to buf failed:%d...\n", buf_len_ret);
        return -1;
    }
    else {
        buf_len = buf_len_ret;
    }

    int ret = kdp_update_fw(dev_idx, &module_id, img_buf, buf_len);
    if (ret != 0) {
        printf("could not update fw..\n");
        return -1;
    }

    printf("update SCPU firmware succeeded...\n");

    //update ncpu
    buf_len_ret = read_file_to_buf(img_buf, FW_NCPU_FILE, FW_FILE_SIZE);
    module_id = 2;
    if (buf_len_ret  <= 0) {
        printf("reading file to buf failed:%d...\n", buf_len_ret);
        return -1;
    }
    else {
        buf_len = buf_len_ret;
    }

    ret = kdp_update_fw(dev_idx, &module_id, img_buf, buf_len);
    if (ret != 0) {
        printf("could not update fw..\n");
        return -1;
    }

    printf("update NCPU firmware succeeded...\n");

    //update model
    char* p_buf = new char[MD_FILE_SIZE];
    if(!p_buf) {
        printf("memory fail..\n");
        return -1;
    }

    memset(p_buf, 0, MD_FILE_SIZE);
    printf("starting update nef model ...\n");

    buf_len_ret = read_file_to_buf(p_buf, MODEL_FILE, MD_FILE_SIZE);
    if (buf_len_ret <= 0) {
        printf("reading file to buf failed:%d...\n", buf_len_ret);
        delete[] p_buf;
        return -1;
    } else {
        buf_len = buf_len_ret;
    }

    ret = kdp_update_nef_model(dev_idx, p_buf, buf_len);
    if (ret != 0) {
        printf("could not update model..\n");
        delete[] p_buf;
        return -1;
    }
    printf("update model succeeded...\n");

    delete[] p_buf;
    
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
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");
    printf("\nFW firmware_id %08x build_id %08x\n", sfirmware_id, sbuild_id);
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    //udt application test
    user_test_app(dev_idx, user_id);
    //udt application id test
    user_fw_id(dev_idx);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
