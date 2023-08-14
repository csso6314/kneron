/**
 * @file      kl520_soft_reset.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-06-10
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "kdp_host.h"
#include "stdio.h"
#include "user_util.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

int user_test_soft_reset(int dev_idx)
{
    int ret = 0;

    uint32_t reset_mode = 255;

    ret = kdp_reset_sys(dev_idx, reset_mode);
    if (ret != 0) {
        printf("could not reset sys..\n");
        return -1;
    }
    printf("sys reset mode succeeded...\n");
    print2log("sys reset mode succeeded...\n");

    return 0;
}

int user_test(int dev_idx, int user_id)
{
    user_test_soft_reset(dev_idx);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
