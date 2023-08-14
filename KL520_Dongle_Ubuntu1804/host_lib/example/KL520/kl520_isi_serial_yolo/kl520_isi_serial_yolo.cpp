/**
 * @file      kl520_isi_serial_yolo.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-05-01
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "user_util.h"
#include "ipc.h"
#include "base.h"
#include "kapp_id.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define ISI_IMAGE_FILE      ("../../input_images/cars_street_at_night_608x608_rgb565.bin")
#define ISI_IMAGE_FILE_T    ("../../input_images/car_park_barrier_608x608_rgb565.bin")
#define IMG_SOURCE_W        608
#define IMG_SOURCE_H        608
#define ISI_IMG_SIZE        (IMG_SOURCE_W * IMG_SOURCE_H * 2)
#define ISI_APP_ID          APP_TINY_YOLO3
// isi initial result memory size in bytes
#define ISI_RESULT_SIZE     2048

//#define TEST_CHECK_FRAME_TIME

/**
 * do inference for the input
 * dev_idx: connected device ID. A host can connect several devices
 * img_buf, buf_len: the image buffer and file size
 * img_id: the sequence id of the image
 * window_left: the number of image buffer still available for input
 * return 0 on succeed, error code on failure
 */
static int do_inference(int dev_idx, char *img_buf, int buf_len, uint32_t img_id, uint32_t *rsp_code, uint32_t *window_left)
{
    int ret;
    // call kdp_isi_inference to do inference for each input
    ret = kdp_isi_inference(dev_idx, img_buf, buf_len, img_id, rsp_code, window_left);
    if (ret) {
        printf("ISI inference failed : %d\n", ret);
        return -1;
    }
    if (*rsp_code != 0) {
        printf("ISI inference error_code: [%d] [%d]\n", *rsp_code, *window_left);
        return -1;
    }

    return 0;
}

#ifdef TEST_CHECK_FRAME_TIME
static void do_check_frame_time(void)
{
    static double last_time = 0;
    double cur_time, elapsed_time;

    if (last_time == 0) {
        // skip first one
        last_time = what_time_is_it_now();
        return;
    }

    cur_time = what_time_is_it_now();
    elapsed_time = (cur_time - last_time) * 1000;

    printf("--> frame time %.2f ms\n", (float)elapsed_time);
    last_time = what_time_is_it_now();
}
#endif

/**
 * get the detection results for each input
 * dev_idx: connected device ID. A host can connect several devices
 * img_id: sequence id to get inference results of an image with the specified id
 * r_size: inference data size
 * r_data: inference result data
 * return 0 on succeed, error code on failure
 */
static int do_get_result(int dev_idx, uint32_t img_id, uint32_t *rsp_code, uint32_t *r_size, char *r_data)
{
    int ret;

    memset(r_data, 0, 8);  // initialize results data
    // call kdp_isi_retrieve_res to retrieve the detection results for each input
    ret = kdp_isi_retrieve_res(dev_idx, img_id, rsp_code, r_size, r_data);
    if (ret) {
        printf("ISI get [%d] result failed : %d\n", img_id, ret);
        return -1;
    }

    if (*rsp_code != 0) {
        printf("ISI get [%d] result error_code: [%d] [%d]\n", img_id, *rsp_code, *r_size);
        return -1;
    }

    if (*r_size >= sizeof(uint32_t)) {
#ifdef TEST_CHECK_FRAME_TIME
        do_check_frame_time();
#endif

        uint32_t box_count = *(uint32_t*) &r_data[4];
        printf("image %d -> %d object(s)\n", img_id, box_count);
        return 0;
    } else {
        printf("Img [%d] : result_size %d too small\n", img_id, *r_size);
        return -1;
    }
}

/**
 * example for isi serial yolo with binary inputs
 * isi: image streaming interface; model is in flash
 * async: pipeline between data transfer of host/scpu and inference (pre/npu/post) in ncpu/npu
 */
int user_test_yolo(int dev_idx, int user_id, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    uint32_t image_buf_size = 0;
    char img_buf1[ISI_IMG_SIZE];
    char img_buf2[ISI_IMG_SIZE];

    if (1) {
        uint16_t width  = IMG_SOURCE_W;
        uint16_t height = IMG_SOURCE_H;
        // image format flags
        // IMAGE_FORMAT_SUB128: subtract 128 for R/G/B value in Kneron device
        // NPU_FORMAT_RGB565: image input is RGB565
        // IMAGE_FORMAT_PARALLEL_PROC: pipeline between ncpu and npu
        uint32_t format = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565;

        // Flash the firmware code with companion mode for tiny_yolo_v3 !!!
        printf("starting ISI mode ...\n");
        // call kdp_start_isi_mode to start isi mode        
        int ret = kdp_start_isi_mode(
            dev_idx, ISI_APP_ID, ISI_RESULT_SIZE, width, height, format, &error_code, &image_buf_size);
        if (ret != 0) {
            printf("could not set to ISI mode: %d ..\n", ret);
            return -1;
        }

        if (image_buf_size < 3) {
            printf("ISI mode window %d too small...\n", image_buf_size);
            return -1;
        }

        printf("ISI mode succeeded (window = %d)...\n", image_buf_size);
        usleep(SLEEP_TIME);
    }

    if (1) {
        int n_len = read_file_to_buf(img_buf1, ISI_IMAGE_FILE, ISI_IMG_SIZE);
        if (n_len <= 0) {
            printf("reading image file 1 failed:%d...\n", n_len);
            return -1;
        }
        
        n_len = read_file_to_buf(img_buf2, ISI_IMAGE_FILE_T, ISI_IMG_SIZE);
        if (n_len <= 0) {
            printf("reading image file 2 failed:%d...\n", n_len);
            return -1;
        }
    }

    /* call kdp_isi_inference to do inference with the inputs
       call kdp_isi_retrieve_res to retrieve the detection results
       call kdp_end_isi to end isi mode */
    if (1) {
        printf("starting ISI inference ...\n");
        uint32_t img_id = 1234;
        uint32_t img_left = 12;
        uint32_t result_size = 0;
        uint32_t buf_len = ISI_IMG_SIZE;
        char inf_res[ISI_RESULT_SIZE];

        // start time for the first frame
        double start_time = what_time_is_it_now();

        uint32_t loop = test_loop;
        char *buf_in;

        while (loop && !check_ctl_break()) {
            if (loop & 1) {
                buf_in = img_buf2;
            } else {
                buf_in = img_buf1;
            }
            // do inference for each input
            ret = do_inference(dev_idx, buf_in, buf_len, img_id, &error_code, &img_left);
            if (ret)
                return ret;

            // retrieve the detection results for each input
            ret = do_get_result(dev_idx, img_id, &error_code, &result_size, inf_res);
            if (ret)
                return ret;

            img_id++;
            loop--;
        }

        // calculate the FPS
        if (1) {
            double end_time = what_time_is_it_now();
            double elapsed_time, avg_elapsed_time, avg_fps;

            elapsed_time = (end_time - start_time) * 1000;
            avg_elapsed_time = elapsed_time / test_loop;
            avg_fps = 1000.0f / avg_elapsed_time;

            printf("\n=> Avg %.2f FPS (%.2f ms = %.2f/%d)\n\n",
                avg_fps, avg_elapsed_time, elapsed_time, test_loop);
        }
        // call kdp_end_isi to end isi mode after finishing the inference for all frames
        kdp_end_isi(dev_idx);
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    user_test_yolo(dev_idx, user_id, 300);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
