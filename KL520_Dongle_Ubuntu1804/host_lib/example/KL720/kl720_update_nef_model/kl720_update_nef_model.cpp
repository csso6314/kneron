/**
 * @file      update_model.cpp
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

#define MODEL_FILE (TEST_DFU_DIR "/ready_to_load/models_720.nef")
#define MD_FILE_SIZE (100 * 1024 * 1024 + 4 * 1024)

//user test update model
int user_test_md(int dev_idx, int user_id)
{
    uint32_t buf_len = 0;
    int buf_len_ret = 0;
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
        return -1;
    }
    else {
        buf_len = buf_len_ret;
    }

    int ret = kdp_update_nef_model(dev_idx, p_buf, buf_len);
    if (ret != 0) {
        printf("could not update model..\n");
        delete[] p_buf;
        return -1;
    }
    printf("update model succeeded...\n");

    delete[] p_buf;
    return 0;
}


int user_test(int dev_idx, int user_id)
{
    //udt model test
    user_test_md(dev_idx, user_id);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
