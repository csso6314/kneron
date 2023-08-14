/**
 * @file      kl520_get_model_info.cpp
 * @brief     kdp host lib user example
 * @version   0.1 - 2020-09-01
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "kdp_host.h"
#include "user_util.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int user_test_get_model_info(int dev_idx)
{
    int ret;
    char model_info[1024]; // total_number(4 bytes) + model_id_1(4 bytes) + model_id_2(4 bytes) + ...
    uint32_t *p_model_info;

    // get info for model in flash: 0
    printf("Info for model in flash \n");
    print2log("Info for model in flash \n");
    ret = kdp_get_model_info(dev_idx, 0, model_info);
    if (ret != 0) {
        printf("Could not get model IDs\n");
        return -1;
    }

    p_model_info = (uint32_t *)model_info;
    if (p_model_info[0] == 0xFFFF) {
        printf("Not supported by the version of the firmware\n");
    } else {
        printf("Total model: %d\n", p_model_info[0]);
        print2log("Total model: %d\n", p_model_info[0]);
        for (uint32_t k = 0; k < p_model_info[0]; k++) {
            printf("Model %d: ID %d\n", k, p_model_info[1+k]);
            print2log("Model %d: ID %d\n", k, p_model_info[1+k]);
        }
    }

    // get info for model in DDR: 1
    printf("Info for model in DDR \n");
    print2log("Info for model in DDR \n");
    ret = kdp_get_model_info(dev_idx, 1, model_info);
    if (ret != 0) {
        printf("Could not get model IDs\n");
        return -1;
    }

    p_model_info = (uint32_t *)model_info;
    if (p_model_info[0] == 0xFFFF) {
        printf("Not supported by the version of the firmware\n");
    } else {
        printf("Total model: %d\n", p_model_info[0]);
        print2log("Total model: %d\n", p_model_info[0]);
        for (uint32_t k = 0; k < p_model_info[0]; k++) {
            printf("Model %d: ID %d\n", k, p_model_info[1+k]);
            print2log("Model %d: ID %d\n", k, p_model_info[1+k]);
        }
    }
    
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    user_test_get_model_info(dev_idx);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
