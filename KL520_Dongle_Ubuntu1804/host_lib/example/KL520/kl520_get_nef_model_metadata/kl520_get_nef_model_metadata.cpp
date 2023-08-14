/**
 * @file      kl520_get_nef_model_metadata.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-11-11
 * @copyright (c) 2020 Kneron Inc. All right reserved.
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

#define MODEL_FILE (TEST_OTA_DIR "/ready_to_load/models_520.nef")
#define MD_FILE_SIZE (28 * 1024 * 1024 + 4 * 1024)

//user test get metadata
int user_test_get_metadata(int dev_idx, int user_id)
{
    uint32_t buf_len = 0;
    int buf_len_ret = 0;
    char* p_buf = new char[MD_FILE_SIZE];
    if(!p_buf) {
        printf("memory fail..\n");
        return -1;
    }

    memset(p_buf, 0, MD_FILE_SIZE);
    printf("starting get metadata from nef model ...\n");

    buf_len_ret = read_file_to_buf(p_buf, MODEL_FILE, MD_FILE_SIZE);
    if (buf_len_ret <= 0) {
        printf("reading file to buf failed:%d...\n", buf_len_ret);
        return -1;
    }
    else {
        buf_len = buf_len_ret;
    }

    //uint32_t model_size = buf_len;
    //kdp_metadata_s target = 0;
    struct kdp_metadata_s metadata = create_metadata_struct();

    int ret = kdp_get_nef_model_metadata(p_buf, buf_len, &metadata);
    if (ret != 0) {
        printf("could not get metadata from nef model..\n");
        delete[] p_buf;
        return -1;
    }

    printf("\n");
    printf("platform:%s\n", metadata.platform);
    printf("target:%d\n", metadata.target); // 0: KL520, 1: KL720, etc.
    printf("crc:0x%08x\n", metadata.crc);
    printf("kn number:%d\n", metadata.kn_num);
    printf("encrypt type:%d\n", metadata.enc_type);
    printf("toolchain version:%s\n", metadata.tc_ver);
    printf("compiler version:%s\n", metadata.compiler_ver);
    printf("\n");

    print2log("platform:%s\n", metadata.platform);
    print2log("target:%d\n", metadata.target); // 0: KL520, 1: KL720, etc.
    print2log("crc:0x%08x\n", metadata.crc);
    print2log("kn number:%d\n", metadata.kn_num);
    print2log("encrypt type:%d\n", metadata.enc_type);
    print2log("toolchain version:%s\n", metadata.tc_ver);
    print2log("compiler version:%s\n", metadata.compiler_ver);

    printf("get metadata from nef model succeeded...\n");

    delete[] p_buf;
    return 0;
}


int user_test(int dev_idx, int user_id)
{
    user_test_get_metadata(dev_idx, user_id);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
