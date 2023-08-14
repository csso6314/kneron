/**
 * @file      isi_fid.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-09-25
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "user_util.h"
#include "kapp_id.h"
#include "model_res.h"
#include "model_type.h"
#include "base.h"
#include "ipc.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

#define ISI_IMAGE_FILE      ("../../input_images/a_man_640x480_rgb565.bin")
#define ISI_IMAGE_FILE_T    ("../../input_images/a_man_640x480_rgb565.bin")
#define ISI_IMG_WIDTH       640
#define ISI_IMG_HEIGHT      480
#define ISI_IMG_SIZE        (ISI_IMG_WIDTH * ISI_IMG_HEIGHT * 2)
#define ISI_IMG_FORMAT      (IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565)

#define ISI_APP_ID          APP_CENTER_APP
#define ISI_RESULT_SIZE     2048

//#define TEST_CHECK_FRAME_TIME

static uint32_t model_id = KNERON_FD_MBSSD_200_200_3;

static int do_inference(int dev_idx, char *img_buf, int buf_len, uint32_t img_id, uint32_t *rsp_code, uint32_t *window_left)
{
    int ret;

    ret = kdp_isi_inference(dev_idx, img_buf, buf_len, img_id, rsp_code, window_left);
    if (ret)
    {
        printf("ISI inference failed : %d\n", ret);
        return -1;
    }
    if (*rsp_code != 0)
    {
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

    if (last_time == 0)
    {
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

static int do_get_result(int dev_idx, uint32_t img_id, uint32_t *rsp_code, uint32_t *r_size, char *r_data)
{
    int ret;

    memset(r_data, 0, 8); // initialize results data

    ret = kdp_isi_retrieve_res(dev_idx, img_id, rsp_code, r_size, r_data);
    if (ret)
    {
        printf("ISI get [%d] result failed : %d\n", img_id, ret);
        return -1;
    }

    if (*rsp_code != 0)
    {
        printf("ISI get [%d] result error_code: [%d] [%d]\n", img_id, *rsp_code, *r_size);
        return -1;
    }

    if (*r_size >= sizeof(uint32_t))
    {
#ifdef TEST_CHECK_FRAME_TIME
        do_check_frame_time();
#endif
        int box_count;
        struct yolo_result_s        *fd_res;
        struct landmark_result_s    *lm_res;
        struct fr_result_s          *fr_res;
        int i;

        switch (model_id) {
        case KNERON_FD_MBSSD_200_200_3:
            fd_res = (struct yolo_result_s*)r_data;
            box_count = fd_res->box_count;
            printf("image %d -> %d object(s)\n", img_id, box_count);
            for (i = 0; i < box_count; i++) {
                printf("    [%d] : class %d, score %f: [%f %f, %f %f]\n", i,
                    fd_res->boxes[i].class_num, fd_res->boxes[i].score,
                    fd_res->boxes[i].x1, fd_res->boxes[i].y1,
                    fd_res->boxes[i].x2, fd_res->boxes[i].y2);
            }
            break;

        case KNERON_LM_5PTS_ONET_56_56_3:
            lm_res = (struct landmark_result_s*)r_data;
            printf("Landmarks: size %d: score %f, blue %f\n", *r_size, lm_res->score, lm_res->blur);
            for (int i = 0; i < LAND_MARK_POINTS; i++) {
                printf("    [%d] : %d, %d\n", i, lm_res->marks[i].x, lm_res->marks[i].y);
            }
            break;

        case KNERON_FR_VGG10:
            fr_res = (struct fr_result_s*)r_data;
            printf("FR: size %d (first 5)\n", *r_size);
            for (int i = 0; i < 5; i++) {
                printf("    [%d] : %f\n", i, fr_res->feature_map[i]);
            }
            break;

        default:
            printf("Wrong model ID %d with size %d\n", model_id , *r_size);
            break;
        }
        return 0;
    }
    else
    {
        printf("Img [%d] : result_size %d too small\n", img_id, *r_size);
        return -1;
    }

    return 0;
}

int user_test_fid(int dev_idx, int use_parallel, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    uint32_t image_buf_size = 0;
    char img_buf1[ISI_IMG_SIZE];
    char img_buf2[ISI_IMG_SIZE];

    if (1)
    {
        uint16_t width = ISI_IMG_WIDTH;
        uint16_t height = ISI_IMG_HEIGHT;
        uint32_t format = 0;

        if (use_parallel == 1)
            format = ISI_IMG_FORMAT | IMAGE_FORMAT_PARALLEL_PROC;
        else
            format = ISI_IMG_FORMAT;

        printf("starting ISI mode ...\n");
        int ret = kdp_start_isi_mode(
            dev_idx, ISI_APP_ID, ISI_RESULT_SIZE, width, height, format, &error_code, &image_buf_size);
        if (ret != 0)
        {
            printf("could not set to ISI mode: %d ..\n", ret);
            return -1;
        }

        if (image_buf_size < 3)
        {
            printf("ISI mode window %d too small...\n", image_buf_size);
            return -1;
        }

        printf("Config model %d...\n", model_id);
        ret = kdp_isi_config(dev_idx, model_id, CONFIG_USE_FLASH_MODEL, &error_code);
        if (ret != 0 || error_code)
        {
            printf("could not configure to model %d: %d, %d\n", model_id, ret, error_code);
            model_id = KNERON_FD_MBSSD_200_200_3;
            return -1;
        }

        printf("ISI model %d succeeded (window = %d)...\n", model_id, image_buf_size);
        usleep(SLEEP_TIME);
    }

    if (1)
    {
        int n_len = read_file_to_buf(img_buf1, ISI_IMAGE_FILE, ISI_IMG_SIZE);
        if (n_len <= 0)
        {
            printf("reading image file 1 failed:%d...\n", n_len);
            return -1;
        }

        n_len = read_file_to_buf(img_buf2, ISI_IMAGE_FILE_T, ISI_IMG_SIZE);
        if (n_len <= 0)
        {
            printf("reading image file 2 failed:%d...\n", n_len);
            return -1;
        }
    }

    if (1)
    {
        printf("starting ISI inference ...\n");
        uint32_t img_id_tx = 1;
        uint32_t img_id_rx = img_id_tx;
        uint32_t img_left = 12;
        uint32_t result_size = 0;
        uint32_t buf_len = ISI_IMG_SIZE;
        char inf_res[ISI_RESULT_SIZE];

        double start_time = what_time_is_it_now();

        uint32_t loop = 0;

        if (use_parallel == 1)
        {
            // Send 2 images first
            ret = do_inference(dev_idx, img_buf1, buf_len, img_id_tx, &error_code, &img_left);
            if (ret)
                return ret;
            img_id_tx++;

            ret = do_inference(dev_idx, img_buf2, buf_len, img_id_tx, &error_code, &img_left);
            if (ret)
                return ret;
            img_id_tx++;

            // Send the rest and get result in loop, with 2 images alternatively
            if (test_loop > 3)
                loop = test_loop - 2;
        }
        else
        {
            loop = test_loop;
        }

        int forever = (test_loop == 0) ? 1 : 0;

        while (1)
        {
            ret = do_inference(dev_idx, img_buf1, buf_len, img_id_tx, &error_code, &img_left);
            if (ret)
                return ret;
            img_id_tx++;

            ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res);
            if (ret)
                return ret;
            img_id_rx++;

            loop--;
            // Odd loop case
            if (loop == 0)
                break;

            ret = do_inference(dev_idx, img_buf2, buf_len, img_id_tx, &error_code, &img_left);
            if (ret)
                return ret;
            img_id_tx++;

            ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res);
            if (ret)
                return ret;
            img_id_rx++;

            loop--;
            if(loop == 0 && forever == 0)
                break;
        }

        if (use_parallel == 1)
        {
            // Get last 2 results
            ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res);
            if (ret)
                return ret;
            img_id_rx++;

            ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res);
            if (ret)
                return ret;
            img_id_rx++;
        }

        if (1)
        {
            double end_time = what_time_is_it_now();
            double elapsed_time, avg_elapsed_time, avg_fps;

            elapsed_time = (end_time - start_time) * 1000;
            avg_elapsed_time = elapsed_time / test_loop;
            avg_fps = 1000.0f / avg_elapsed_time;

            printf("\n=> Avg %.2f FPS (%.2f ms = %.2f/%d)\n\n",
                    avg_fps, avg_elapsed_time, elapsed_time, test_loop);
        }
    }
    return 0;
}

static void usage(char* name)
{
    printf("\n");
    printf("usage: ./%s [parallel_mode] [run_frames] [model_id]\n", name);
    printf("\n[parallel_mode] 0: non parallel mode, 1: parallel mode\n");
    printf("[run_frames] number of frames to run, 0 = forever\n");
    printf("[model_id] model ID (optional. Default is 3 for FD.)\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4) {
        usage(argv[0]);
        return -1;
    }

    int use_parallel = atoi(argv[1]);
    int run_frames = atoi(argv[2]);

    if (argc == 4)
        model_id = atoi(argv[3]);

    printf("init kdp host lib log....\n");

    if (kdp_lib_init() < 0)
    {
        printf("init for kdp host lib failed.\n");
        return -1;
    }

    printf("adding devices....\n");
    int dev_idx = kdp_connect_usb_device(1);
    if (dev_idx < 0)
    {
        printf("add device failed.\n");
        return -1;
    }

    printf("start kdp host lib....\n");
    if (kdp_lib_start() < 0)
    {
        printf("start kdp host lib failed.\n");
        return -1;
    }

    user_test_fid(dev_idx, use_parallel, run_frames);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
