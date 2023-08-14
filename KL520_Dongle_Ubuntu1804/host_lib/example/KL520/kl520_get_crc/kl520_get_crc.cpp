/**
 * @file      kl520_get_crc.cpp
 * @brief     kdp host lib user example
 * @version   0.1 - 2020-11-09
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "kdp_host.h"
#include "user_util.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int user_test_get_crc(int dev_idx)
{
    int ret;
    char crc[4]; // 4 bytes
    uint32_t *p_crc;

    // 0 - Flash, 1 - DDR
    printf("starting get model CRC in Flash\n");
    print2log("starting get model CRC in Flash\n");
    ret = kdp_get_crc(dev_idx, 0/*flash*/, crc);
    if (ret != 0) {
        printf("Could not get CRC\n");
        return -1;
    }

    p_crc = (uint32_t *)crc;
    if (p_crc[0] == 0xFFFF) {
        printf("Not supported by the version of the firmware\n");
    } else {
        if (p_crc[0] == 0xFFFFFFFF) {
            printf("No CRC info for the loaded models\n");
        } else {
            printf("CRC: 0x%x\n", p_crc[0]);
            print2log("CRC: 0x%x\n", p_crc[0]);
        }
    }
    
    printf("starting get model CRC in ddr\n");
    print2log("starting get model CRC in ddr\n");
    ret = kdp_get_crc(dev_idx, 1/*ddr*/, crc);
    if (ret != 0) {
        printf("Could not get CRC\n");
        return -1;
    }

    p_crc = (uint32_t *)crc;
    if (p_crc[0] == 0xFFFF) {
        printf("Not supported by the version of the firmware\n");
    } else {
        if (p_crc[0] == 0xFFFFFFFF) {
            printf("No CRC info for the loaded models\n");
        } else if (p_crc[0] == 0) {
            printf("Models have not been loaded into DDR\n");
        } else {
            printf("CRC: 0x%x\n", p_crc[0]);
            print2log("CRC: 0x%x\n", p_crc[0]);
        }
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    user_test_get_crc(dev_idx);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
