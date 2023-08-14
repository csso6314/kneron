/**
 * @file      kl520_update_nef_model.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2019-08-01
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"
#include "stdlib.h"

#include <string.h>
#include <unistd.h>
#include "user_util.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MODEL_FILE (TEST_OTA_DIR "/ready_to_load/models_520.nef")

//user test update model
int user_test_md(int dev_idx, int user_id)
{
    printf("starting update nef model ...\n");

    long buf_len = 0;
    char* p_buf = read_file_to_buffer_auto_malloc(MODEL_FILE, &buf_len);

    if (!p_buf) {
        printf("reading file to buf failed...\n");
        return -1;
    }

    int ret = kdp_update_nef_model(dev_idx, p_buf, (int)buf_len);
    if (ret != 0) {
        printf("could not update model..\n");
        free(p_buf);
        return -1;
    }
    printf("update model succeeded...\n");

    free(p_buf);
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
