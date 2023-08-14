/**
 * @file      main.cpp
 * @brief     kdp host lib demo file 
 * @version   0.1 - 2019-04-19
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "stdio.h"
#include "kdp_host.h"
#include "user_test.h"
#include "user_util.h"
#include <unistd.h>
#include <stdlib.h>


#if defined (__cplusplus) || defined (c_plusplus)
extern "C"{
#endif

#define MAX_TIMES 1
const char* p_ci_logfile_name_g = NULL;

int main(int argc, char* argv[])
{
    int user_id = 0;
    p_ci_logfile_name_g = ci_prepare_logfile_name(argc, argv);

    if(argc == 2) {
        user_id = atoi(argv[1]);
    }

    printf("init kdp host lib log....\n");
    kdp_init_log("/tmp/", "mzt.log");

    if(kdp_lib_init() < 0) {
        printf("init for kdp host lib failed.\n");
        return -1;
    }

    printf("adding devices....\n");

    int dev_idx = -1;
    kdp_device_info_list_t *list = NULL;
    kdp_scan_usb_devices(&list);

    for (int i = 0; i < list->num_dev; i++)
    {
        if (list->kdevice[i].product_id == KDP_DEVICE_KL520)
        {
            // found it, now try to connect
            dev_idx = kdp_connect_usb_device(list->kdevice[i].scan_index);
            break;
        }
    }
    free(list);

    if(dev_idx < 0 ) {
        printf("add device failed.\n");
        return -1;
    }

    printf("start kdp host lib....\n");
    if(kdp_lib_start() < 0) {
        printf("start kdp host lib failed.\n");
        return -1;
    }
    if (0 > register_hostlib_signal()) {
        return -1;
    }

    for(int i = 0; i < MAX_TIMES; i++) {
        if (check_ctl_break())
            return 0;
        printf("doing test :%d....\n", i);
        user_test(dev_idx, user_id);
        usleep(10000);
    }

    printf("de init kdp host lib....\n");
    kdp_lib_de_init();
   
    return 0;
}

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif
