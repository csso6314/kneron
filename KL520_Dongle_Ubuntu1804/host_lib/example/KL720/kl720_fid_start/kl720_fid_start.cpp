/**
 * @file      user_test_all.cpp
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

#define IMAGE_DATA_FILE ("../../input_images/u1_f1_rgb.bin")

//user test for removing user
int user_test_rm_user(int dev_idx, int user_id)
{
    //remove user id
    uint32_t _id = user_id;
    uint32_t img_size = 0;
    printf("starting verify mode ...\n");
    int ret = kdp_start_sfid_mode(dev_idx, &img_size, DEF_FR_THRESH, 640, 480, IMG_FORMAT_RGB565);
    if (ret != 0 || img_size == 0) {
        printf("start verify mode failed :%d, buf size:%d.\n", ret, img_size);
        return -1;
    }
    printf("starting verify mode successfuly, img size:%d...\n", img_size);

    usleep(SLEEP_TIME);
    printf("removing user ...\n");
    ret = kdp_remove_user(dev_idx, _id);
    if (ret != 0) {
        printf("remove user :%d failed.\n", user_id);
        return -1;
    }
    printf("removing user succeeded ...\n");

    return 0;
}

//user test for verifying user
int user_test_ver_user(int dev_idx, int user_id)
{
    //ver user mode
    uint32_t img_size = IMAGE_SIZE;
    printf("starting verify mode ...\n");
    int ret = kdp_start_sfid_mode(dev_idx, &img_size, DEF_FR_THRESH, 640, 480, IMG_FORMAT_RGB565);
    if (ret != 0 || img_size == 0) {
        printf("start verify mode failed :%d, buf size:%d.\n", ret, img_size);
        return -1;
    }
    printf("starting verify mode successfuly, img size:%d...\n", img_size);

    usleep(SLEEP_TIME);

    uint16_t u_id = 0;
    char img_buf[IMAGE_SIZE];
    uint16_t mask = 0;
    uint32_t res_size = kdp_get_res_size(true, true, true, true, true);
    char res[res_size] = {0};

    uint32_t buf_len = 0;
    uint16_t uu_id = 0;

    for (int i = 0; i < 1; i++) { //do 10 times
        memset(img_buf, 0, sizeof(img_buf));

        printf("verifying for image ...\n");
        int n_len = read_file_to_buf(img_buf, IMAGE_DATA_FILE, IMAGE_SIZE);
        if (n_len <= 0) {
            printf("reading file to buf failed:%d...\n", n_len);
            return -1;
        }

        buf_len = n_len;
        // mask = null
        u_id = 0;
        ret = kdp_verify_user_id_generic(dev_idx, &u_id, img_buf, n_len, NULL, NULL);
        if (ret != 0) {
            printf("could not find matched user id.\n");
        } else {
            user_id = u_id;
            printf("find matched user id:%d.\n", user_id);
        }

        usleep(SLEEP_TIME);

        // mask= FD
        mask = kdp_get_res_mask(true, false, false, false, false);
        ret = kdp_verify_user_id_generic(dev_idx, &u_id, img_buf, n_len, &mask, res);
        if (ret != 0) {
            printf("could not find matched user id.\n");
        } else {
            user_id = u_id;
            printf("find matched user id:%d.\n", user_id);
        }

        usleep(SLEEP_TIME);

        // mask= FD | FR
        mask = kdp_get_res_mask(true, false, true, false, false);
        ret = kdp_verify_user_id_generic(dev_idx, &u_id, img_buf, n_len, &mask, res);
        if (ret != 0) {
            printf("could not find matched user id.\n");
        } else {
            user_id = u_id;
            printf("find matched user id:%d.\n", user_id);
        }
        usleep(SLEEP_TIME);

        // mask= FD
        printf("starting verify user id all res ...\n");
        mask = kdp_get_res_mask(true, false, false, false, false);
        ret = kdp_verify_user_id_generic(dev_idx, &uu_id, img_buf, buf_len, &mask, res);
        if (ret != 0) {
            printf("could not verify user id all res..\n");
            return -1;
        } else {
            user_id = uu_id;
            printf("find matched user id:%d.\n", user_id);
        }

        // mask= FD | FR
        usleep(SLEEP_TIME);
        printf("starting verify user id all res ...\n");
        mask = kdp_get_res_mask(true, false, true, false, false);
        ret = kdp_verify_user_id_generic(dev_idx, &uu_id, img_buf, buf_len, &mask, res);
        if (ret != 0) {
            printf("could not verify user id all res..\n");
            return -1;
        } else {
            user_id = uu_id;
            printf("find matched user id:%d.\n", user_id);
        }

        // mask= FD | FR | LV
        usleep(SLEEP_TIME);
        printf("starting verify user id all res ...\n");
        mask = kdp_get_res_mask(true, false, true, true, false);
        ret = kdp_verify_user_id_generic(dev_idx, &uu_id, img_buf, buf_len, &mask, res);
        if (ret != 0) {
            printf("could not verify user id all res..\n");
            return -1;
        } else {
            user_id = uu_id;
            printf("find matched user id:%d.\n", user_id);
        }
    }

    usleep(SLEEP_TIME);

    printf("verify_user id all res succeeded...\n");

    usleep(SLEEP_TIME);

    printf("starting verify_user_id_generic ...\n");
    ret = kdp_verify_user_id_generic(dev_idx, &uu_id, img_buf, buf_len, NULL, NULL);
    if (ret != 0) {
        printf("could not verify_user_id_generic..\n");
        return -1;
    }
    printf("starting verify user id generic succeeded...\n");
    usleep(SLEEP_TIME);

    printf("starting verify_user_id_generic ...\n");
    mask = kdp_get_res_mask(true, true, false, false, false);
    ret = kdp_verify_user_id_generic(dev_idx, &uu_id, img_buf, buf_len, &mask, res);
    if (ret != 0) {
        printf("could not verify_user_id_generic..\n");
        return -1;
    }
    printf("starting verify user id generic succeeded...\n");
    usleep(SLEEP_TIME);

    printf("starting verify_user_id_generic ...\n");
    mask = kdp_get_res_mask(true, true, true, true, false);
    ret = kdp_verify_user_id_generic(dev_idx, &uu_id, img_buf, buf_len, &mask, res);
    if (ret != 0) {
        printf("could not verify_user_id_generic..\n");
        return -1;
    }
    printf("starting verify user id generic succeeded...\n");
    usleep(SLEEP_TIME);

    return 0;
}

//user test for registering user
int user_test_reg_user(int dev_idx, int user_id)
{
    uint32_t img_size = IMAGE_SIZE;
    printf("starting verify mode ...\n");
    int ret = kdp_start_sfid_mode(dev_idx, &img_size, DEF_FR_THRESH, 640, 480, IMG_FORMAT_RGB565);
    if (ret != 0 || img_size == 0) {
        printf("start verify mode failed :%d, buf size:%d.\n", ret, img_size);
        return -1;
    }
    printf("starting verify mode successfuly, img size:%d...\n", img_size);

    //register
    uint16_t u_id = user_id;
    if (u_id == 0)
        u_id = 2;

    uint16_t img_idx = 1;
    printf("starting register mode 1...\n");
    ret = kdp_start_reg_user_mode(dev_idx, u_id, img_idx);
    if (ret != 0) {
        printf("could not set to register user mode..\n");
        return -1;
    }
    printf("register mode 1 succeeded...\n");

    usleep(SLEEP_TIME);

    char img_buf[IMAGE_SIZE];
    int buf_len = 0;

    if (1) {
        //TODO need set image name here
        memset(img_buf, 0, sizeof(img_buf));

        printf("registering with image...\n");
        int n_len = read_file_to_buf(img_buf, IMAGE_DATA_FILE, IMAGE_SIZE);
        if (n_len <= 0) {
            printf("reading file to buf failed:%d...\n", n_len);
            n_len = IMAGE_SIZE;
        }

        buf_len = n_len;
        ret = kdp_extract_feature_generic(dev_idx, img_buf, buf_len, NULL, NULL);
        if (ret != 0) {
            printf("could not extract feature for.\n");
        } else {
            printf("feature extracted successfully for img index.\n");
        }
    }

    usleep(SLEEP_TIME);
    img_idx = 2;
    printf("starting register mode 2 ...\n");
    ret = kdp_start_reg_user_mode(dev_idx, u_id, img_idx);
    if (ret != 0) {
        printf("could not set to register user mode..\n");
        return -1;
    }
    printf("register mode 2 succeeded...\n");

    usleep(SLEEP_TIME);

    uint16_t mask = 0;
    uint32_t res_size = kdp_get_res_size(true, true, true, true, true);
    char res[res_size] = {0};

    if (1) {
        //TODO need set image name here
        memset(img_buf, 0, sizeof(img_buf));

        printf("registering with image...\n");
        int n_len = read_file_to_buf(img_buf, IMAGE_DATA_FILE, IMAGE_SIZE);
        if (n_len <= 0) {
            printf("reading file to buf failed:%d...\n", n_len);
            n_len = IMAGE_SIZE;
        }

        buf_len = n_len;

        printf("starting extract feature all res ...\n");

        ret = kdp_extract_feature_generic(dev_idx, img_buf, buf_len, NULL, NULL);
        if (ret != 0) {
            printf("could not extract feature all res..\n");
            return -1;
        }
        usleep(SLEEP_TIME);
        printf("starting extract feature all res ...\n");
        mask = kdp_get_res_mask(true, false, false, false, false);
        ret = kdp_extract_feature_generic(dev_idx, img_buf, buf_len, &mask, res);
        if (ret != 0) {
            printf("could not extract feature all res..\n");
            return -1;
        }
        usleep(SLEEP_TIME);
        printf("starting extract feature all res ...\n");
        mask = kdp_get_res_mask(true, false, true, false, false);
        ret = kdp_extract_feature_generic(dev_idx, img_buf, buf_len, &mask, res);
        if (ret != 0) {
            printf("could not extract_feature_all_res..\n");
            return -1;
        }
        usleep(SLEEP_TIME);

        printf("starting extract feature all res ...\n");
        mask = kdp_get_res_mask(true, true, true, false, false);
        ret = kdp_extract_feature_generic(dev_idx, img_buf, buf_len, &mask, res);
        if (ret != 0) {
            printf("could not extract_feature_all_res..\n");
            return -1;
        }

        printf("extract feature all res succeeded...\n");
    }

    usleep(SLEEP_TIME);

    img_idx = 3;
    printf("starting register mode 3 ...\n");
    ret = kdp_start_reg_user_mode(dev_idx, u_id, img_idx);
    if (ret != 0) {
        printf("could not set to register user mode..\n");
        return -1;
    }
    printf("register mode 3 succeeded...\n");

    usleep(SLEEP_TIME);

    if (1) {
        //TODO need set image name here
        memset(img_buf, 0, sizeof(img_buf));

        printf("registering with image...\n");
        int n_len = read_file_to_buf(img_buf, IMAGE_DATA_FILE, IMAGE_SIZE);
        if (n_len <= 0) {
            printf("reading file to buf failed:%d...\n", n_len);
            n_len = IMAGE_SIZE;
        }

        printf("starting extract_feature_generic ...\n");
        mask = kdp_get_res_mask(true, true, true, true, true);
        ret = kdp_extract_feature_generic(dev_idx, img_buf, buf_len, &mask, res);
        if (ret != 0) {
            printf("could not extract_feature_generic..\n");
            return -1;
        }
        usleep(SLEEP_TIME);

        printf("extract feature generic succeeded...\n");
    }

    usleep(SLEEP_TIME);

    uint32_t _id = u_id;
    printf("registering user :%d ...\n", _id);
    ret = kdp_register_user(dev_idx, _id);
    if (ret != 0) {
        printf("register user failed.\n");
        return -1;
    }
    printf("registered with user id:%d.\n", _id);

    return 0;
}

//user test reset
int user_test_reset(int dev_idx, int user_id)
{

    //     reset mode control
    //     0 – no operation
    //     1 – reset message protocol
    //     3 – enter suspended mode
    //     4 – return active operation
    //   255 – reset system
    //   256 – system shut down (RTC)

    // 0x1000xxxx – reset debug output
    //                         level

    uint32_t reset_mode = 0;
    uint32_t sfw_id = 0;
    uint32_t sbuild_id = 0;
    uint16_t sys_status = 0;
    uint16_t app_status = 0;
    uint32_t nfw_id = 0;
    uint32_t nbuild_id = 0;
    printf("starting sys reset mode 0 ...\n");
    int ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded.:%d..\n", ret);

    usleep(SLEEP_TIME);

    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");

    usleep(SLEEP_TIME);

    reset_mode = 1;
    printf("starting sys reset mode 1 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded...\n");

    usleep(SLEEP_TIME);

    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");

    usleep(SLEEP_TIME);

    reset_mode = 3;
    printf("starting sys reset mode 3 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded...\n");

    usleep(SLEEP_TIME);

    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");

    usleep(SLEEP_TIME);

    reset_mode = 4;
    printf("starting sys reset mode 4 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded...\n");

    usleep(SLEEP_TIME);

    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");
#if 0
    reset_mode = 255;
    printf("starting sys reset mode 255 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded...\n");

    sleep(5);

    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");

    usleep(SLEEP_TIME);
#endif

#if 0
    //shutdown dev
    reset_mode = 256;
    printf("starting sys reset mode 256 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded...\n");

    sleep(5);
    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");

    usleep(SLEEP_TIME);

    //restart dev
    reset_mode = 0x22;
    printf("starting sys reset mode 0x22 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded...\n");

    usleep(SLEEP_TIME);
    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");
#endif


    reset_mode = 0x10000001;
    printf("starting sys reset mode 0x10000000 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    printf("sys reset mode succeeded...\n");

    usleep(SLEEP_TIME);

    printf("starting report sys status ...\n");
    ret = kdp_report_sys_status(dev_idx, &sfw_id, &sbuild_id,
        &sys_status, &app_status, &nfw_id, &nbuild_id);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("report sys status succeeded...\n");

    usleep(SLEEP_TIME);

    return 0;
}

int user_test_switch_fw(int dev_idx, int user_id)
{

    uint32_t reset_mode = 0;

    reset_mode = 0x20000001;
    printf("starting sys reset mode 0x20000001 ...\n");
    int ret = kdp_reset_sys(dev_idx, reset_mode);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("sys reset mode succeeded...\n");
    usleep(SLEEP_TIME);

    reset_mode = 0x20000002;
    printf("starting sys reset mode 0x20000002 ...\n");
    ret = kdp_reset_sys(dev_idx, reset_mode);
    if (ret != 0) {
        printf("could not report sys status..\n");
        return -1;
    }
    printf("sys reset mode succeeded...\n");
    usleep(SLEEP_TIME);

    return 0;
}

int user_test(int dev_idx, int user_id)
{
    //doing remove user test
    user_test_rm_user(dev_idx, user_id);

    //register user id
    user_test_reg_user(dev_idx, user_id);

    //verify user id
    user_test_ver_user(dev_idx, user_id);

    //reset test
    // TODO: reenable after system reset works ok
    // user_test_reset(dev_idx, user_id);

    // user_test_switch_fw(dev_idx, user_id);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
