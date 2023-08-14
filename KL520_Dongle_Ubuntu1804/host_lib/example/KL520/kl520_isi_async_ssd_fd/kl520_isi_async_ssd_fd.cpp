/**
 * @file      kl520_isi_async_ssd_fd.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-05-01
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"
#include "ipc.h"
#include "base.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "user_util.h"
#include "kapp_id.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define ODTY_IMAGE_FILE      ("../../input_images/male_without_mask_200x200_rgb565.bin")
#define ODTY_IMAGE_FILE_T    ("../../input_images/male_without_mask_200x200_rgb565.bin")
#define IMAGE_FILE           ("../../input_images/male_without_mask_200x200_rgb565.png")
#define IMAGE_FILE_T         ("../../input_images/male_without_mask_200x200_rgb565.png")
#define IMAGE_FILE_RES       ("without_mask_res.png")
#define IMAGE_FILE_RES_T     ("without_mask_res.png")
#define ODTY_IMG_SIZE (300 * 300 * 2)

#define ODTY_APP_ID     APP_FD_LM
#define ODTY_RESULT_SIZE 2048

int user_test_ssd_fd(int dev_idx, int user_id, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    uint32_t image_buf_size = 0;
    char img_buf1[ODTY_IMG_SIZE];
    char img_buf2[ODTY_IMG_SIZE];
    uint16_t width = 200;
    uint16_t height = 200;

    if (1) {
        uint32_t format = 0x80000060;
        uint32_t reset_mode = 0x10000202; //4 for info

        int ret = kdp_reset_sys(dev_idx, reset_mode);
        printf("starting ODTY mode ...\n");
        ret = kdp_start_isi_mode(
            dev_idx, ODTY_APP_ID, ODTY_RESULT_SIZE, width, height, format, &error_code, &image_buf_size);
        if (ret != 0) {
            printf("could not set to ODTY mode: %d ..\n", ret);
            return -1;
        }

        printf("ODTA mode succeeded...\n");
        usleep(SLEEP_TIME);
    }

    if (1) {
        memset(img_buf1, 0, ODTY_IMG_SIZE);
        memset(img_buf2, 0, ODTY_IMG_SIZE);
        int n_len = read_file_to_buf(img_buf1, ODTY_IMAGE_FILE, ODTY_IMG_SIZE);
        if (n_len <= 0) {
            printf("reading odta image file 1 failed:%d...\n", n_len);
            return -1;
        }
        
        n_len = read_file_to_buf(img_buf2, ODTY_IMAGE_FILE_T, ODTY_IMG_SIZE);
        if (n_len <= 0) {
            printf("reading odta image file 2 failed:%d...\n", n_len);
            return -1;
        }
    }

    if (1) {
        printf("starting ODTY inference ...\n");
        uint32_t img_id = 1234;
        uint32_t img_left = 12;
        uint32_t result_size = 0;
        uint32_t buf_len = ODTY_IMG_SIZE;
        uint32_t buf_depth = 0;
        char* img_buf;
        char inf_res[ODTY_RESULT_SIZE];

        int loop1 = 0;
        double start;
        float fps = 0;
        
        start = what_time_is_it_now();
        // fill up the image buffers
        while (!check_ctl_break()) {
            if (img_id & 1)
                img_buf = img_buf1;
            else
                img_buf = img_buf2;
            ret = kdp_isi_inference(dev_idx, img_buf, buf_len, img_id, &error_code, &img_left);
                if (ret) {
                    printf("ODTY inference failed : %d\n", ret);
                    return -1;
                }
                else {
                //printf("ODTY inference : [%d] [%d]\n", error_code, img_left);
                if (error_code == 0) {
                    img_id++;
                    buf_depth++;
                    if (img_left == 0)
                        break;
                    if (test_loop <= buf_depth)
                        break;
                }
            }
        }
        if (check_ctl_break())
            return 0;

        printf("ODTY image buffer depth = %d\n", image_buf_size);

        printf("mask 1: no mask, mask2: with mask\n");

        // start the inference loop
        uint32_t loop = 0;
        uint32_t expected;
        int32_t mask;
        if (test_loop > buf_depth)
            loop = test_loop - buf_depth;
        while(loop && !check_ctl_break()) {  // get previous inference results and send in one more image
            memset(inf_res, 0, 8);  // initialize results data
            ret = kdp_isi_retrieve_res(dev_idx, img_id-buf_depth, &error_code, &result_size, inf_res);
            loop1++;
            
            if (loop1 == 100) {
                fps = 1./((what_time_is_it_now() - start)/loop1);
                printf("[INFO] average time on 100 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);
            }
            if (result_size) {
               
                dme_res *detection_res = (dme_res *)inf_res;
                if ((img_id-buf_depth) & 1){
                    expected = 1;
                    mask = 1;
                }else{
                    expected = 1;
                    mask = 1;
                }
                if (detection_res ->box_count != expected)
                    printf("image %d error results\n", img_id-buf_depth);
                printf("image %d -> %d face(s)\n", img_id-image_buf_size, detection_res ->box_count);
                if (detection_res ->boxes[0].class_num != mask)
                    printf("image %d error results\n", img_id-buf_depth);
                printf("image %d -> mask %d, score: %f\n", img_id-image_buf_size, detection_res ->boxes[0].class_num, detection_res->boxes[0].score);
                float x1 = detection_res->boxes[0].x1;
                float y1 = detection_res->boxes[0].y1;
                float x2 = detection_res->boxes[0].x2;
                float y2 = detection_res->boxes[0].y2;
                printf("x1: %f\n", x1);
                printf("y1: %f\n", y1);
                printf("x2: %f\n", x2);
                printf("y2: %f\n", y2);
                Mat image;
                if( mask == 1){
                    image = imread(IMAGE_FILE, CV_LOAD_IMAGE_COLOR); 
                }else{
                    image = imread(IMAGE_FILE_T, CV_LOAD_IMAGE_COLOR);
                }

                int img_width = image.cols;
                int img_height = image.rows;
                printf("img_width: %d, img_height: %d\n", img_width, img_height);

                float ratio_width = (float) img_width / (float) width;
                float ratio_height = (float) img_height / (float) height;

                printf("ratio_width: %f, ratio_height: %f\n", ratio_width, ratio_height);

                float img_x1 = x1 * ratio_width;
                float img_y1 = y1 * ratio_height;
                float img_x2 = x2 * ratio_width;
                float img_y2 = y2 * ratio_height;

                Rect rect((int)img_x1, (int)img_y1, (int)(img_x2 - img_x1), (int)(img_y2 - img_y1));
                if( mask == 1){
                    rectangle(image, rect, Scalar(0, 255, 0));
                    imwrite(IMAGE_FILE_RES, image);
                }else{
                    rectangle(image, rect, Scalar(0, 0, 255));
                    imwrite(IMAGE_FILE_RES_T, image);
                }

            }
            if (img_id & 1)
                img_buf = img_buf1;
            else
                img_buf = img_buf2;
            ret = kdp_isi_inference(dev_idx, img_buf, buf_len, img_id, &error_code, &img_left);
            img_id++;
            loop--;
        }
        if (check_ctl_break())
            return 0;
        // get the results from images remaining in the buffers
        while (buf_depth && !check_ctl_break()) {
            memset(inf_res, 0, 8);  // initialize results data
            //printf("get results %d\n", img_id-buf_depth);
            ret = kdp_isi_retrieve_res(dev_idx, img_id-buf_depth, &error_code, &result_size, inf_res);
            if (result_size) {
                dme_res *detection_res = (dme_res *)inf_res;
                if ((img_id-buf_depth) & 1){
                    expected = 1;
                    mask = 1;
                }else{
                    expected = 1;
                    mask = 1;
                }
                if (detection_res ->box_count != expected)
                    printf("image %d error results\n", img_id-buf_depth);
                printf("image %d -> %d face(s)\n", img_id-image_buf_size, detection_res ->box_count);
                if (detection_res ->boxes[0].class_num != mask)
                    printf("image %d error results\n", img_id-buf_depth);
                printf("image %d -> mask %d\n", img_id-image_buf_size, detection_res ->boxes[0].class_num);
                
            }
            buf_depth--;
        }
        // call kdp_end_isi to end isi mode after finishing the inference for all frames
        kdp_end_isi(dev_idx);
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    //ssd_fd test
    user_test_ssd_fd(dev_idx, user_id, 105);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
