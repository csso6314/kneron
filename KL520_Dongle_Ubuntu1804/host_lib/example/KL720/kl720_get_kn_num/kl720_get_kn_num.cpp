/**
 * @file      get_kn_num.cpp
 * @brief     kdp host lib user example
 * @version   0.1 - 2020-08-17
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "kdp_host.h"
#include "user_util.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int user_test_get_kn_num(int dev_idx, int user_id)
{
    int ret;
    uint32_t kn_num;

    printf("starting get KN number: %d, %d\n", dev_idx, user_id);
    ret = kdp_get_kn_number(dev_idx, &kn_num);
    if (ret != 0) {
        printf("Could not get KN number\n");
        return -1;
    }

    if (kn_num == 0xFFFF)
        printf("Not supported by the version of the firmware\n");
    else{
        printf("KN number: %8.8x\n", kn_num);
    }
    
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    user_test_get_kn_num(dev_idx, user_id);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
