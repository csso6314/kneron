/**
 * @file      kl520_isi_async_bypass_pre_post_host_yolo.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-10-21
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "user_util.h"
#include "post_processing_ex.h"
#include "kdpio.h"
#include "ipc.h"
#include "base.h"
#include "kapp_id.h"

extern "C" {
uint32_t round_up(uint32_t num);
int post_yolo_v3(int model_id, struct kdp_image_s *image_p);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define ISI_IMAGE_FILE      ("../../input_images/cars_street_at_night_224x224_rgba8888.bin")
#define ISI_IMAGE_FILE_T    ("../../input_images/car_park_barrier_224x224_rgba8888.bin")
#define IMG_SOURCE_W        224
#define IMG_SOURCE_H        224
#define ISI_IMG_SIZE        (IMG_SOURCE_W * IMG_SOURCE_H * 4)
#define ISI_APP_ID          APP_TINY_YOLO3
// isi initial result memory size in bytes (equal or larger than the real size)
#define ISI_RESULT_SIZE     102400

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
 * inf_res: inference result data
 * post_par: parameters for postprocessing in host side
 * return 0 on succeed, error code on failure
 */
static int do_get_result(int dev_idx, uint32_t img_id, uint32_t *rsp_code, uint32_t *r_size,
                        char *inf_res, struct post_parameter_s post_par)
{
    int ret;

    memset(inf_res, 0, 8);  // initialize results data
    // call kdp_isi_retrieve_res to retrieve the data for all output nodes of each input:
    // TOTAL_OUT_NUMBER + (H/C/W/RADIX/SCALE) + (H/C/W/RADIX/SCALE) + ...
    // + FP_DATA (size: H*C*16*CEIL(W/16) ) + FP_DATA + ...
    ret = kdp_isi_retrieve_res(dev_idx, img_id, rsp_code, r_size, inf_res);
    if (ret) {
        printf("ISI get [%d] result failed : %d\n", img_id, ret);
        return -1;
    }

    if (*rsp_code != 0) {
        printf("ISI get [%d] result error_code: [%d] [%d]\n", img_id, *rsp_code, *r_size);
        return -1;
    }

    // Prepare for postprocessing      
    int output_num = inf_res[0];
    struct output_node_params *p_node_info;
    int r_len, offset;
    struct yolo_result_s *det_res = (struct yolo_result_s *)calloc(1, sizeof(dme_yolo_res));
    struct kdp_image_s *image_p = (struct kdp_image_s *)calloc(1, sizeof(struct kdp_image_s));
    offset = sizeof(int) + output_num * sizeof(output_node_params);

    // save the parameters to struct kdp_image_s
    RAW_INPUT_COL(image_p) = post_par.raw_input_col;
    RAW_INPUT_ROW(image_p) = post_par.raw_input_row;
    DIM_INPUT_COL(image_p) = post_par.model_input_row;
    DIM_INPUT_ROW(image_p) = post_par.model_input_row;
    RAW_FORMAT(image_p) = post_par.image_format;
    POSTPROC_RESULT_MEM_ADDR(image_p) = (uint32_t *)det_res;
    POSTPROC_OUTPUT_NUM(image_p) = output_num;

    for (int i = 0; i < output_num; i++) {
        // parse the H/C/W/Radix/Scale info
        p_node_info = (struct output_node_params *)(inf_res + sizeof(int) + i * sizeof(output_node_params));
        r_len = p_node_info->channel * p_node_info->height * round_up(p_node_info->width);
        // save the parameters to struct kdp_image_s
        POSTPROC_OUT_NODE_ADDR(image_p, i) = inf_res + offset;
        POSTPROC_OUT_NODE_ROW(image_p, i) = p_node_info->height;
        POSTPROC_OUT_NODE_CH(image_p, i) = p_node_info->channel;
        POSTPROC_OUT_NODE_COL(image_p, i) = p_node_info->width;
        POSTPROC_OUT_NODE_RADIX(image_p, i) = p_node_info->radix;
        POSTPROC_OUT_NODE_SCALE(image_p, i) = p_node_info->scale;

        // offset for next H/C/W/Radix/Scale info
        offset = offset + r_len;
    }            

    // Do postprocessing
    post_yolo_v3(0, image_p);

    free(image_p);
    free(det_res);

    if (*r_size >= sizeof(uint32_t)) {
#ifdef TEST_CHECK_FRAME_TIME
        do_check_frame_time();
#endif
        return 0;
    } else {
        printf("Img [%d] : result_size %d too small\n", img_id, *r_size);
        return -1;
    }
}

/**
 * example for isi async yolo with binary inputs
 * isi: image streaming interface; model is in flash
 * async: pipeline between data transfer of host/scpu and inference (pre/npu/post) in ncpu/npu
 */
int user_test_yolo(int dev_idx, int user_id, struct post_parameter_s post_par, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    uint32_t image_buf_size = 0;
    char img_buf1[ISI_IMG_SIZE];
    char img_buf2[ISI_IMG_SIZE];

    if (1) {
        uint16_t width  = IMG_SOURCE_W;
        uint16_t height = IMG_SOURCE_H;
        uint32_t format = post_par.image_format;

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
        uint32_t img_id_tx = 1234;
        uint32_t img_id_rx = img_id_tx;
        uint32_t img_left = 12;
        uint32_t result_size = 0;
        uint32_t buf_len = ISI_IMG_SIZE;
        char inf_res[ISI_RESULT_SIZE];

        // start time for the first frame
        double start_time = what_time_is_it_now();

        // Send 2 images first
        // do inference for each input
        ret = do_inference(dev_idx, img_buf1, buf_len, img_id_tx, &error_code, &img_left);
        if (ret)
            return ret;
        img_id_tx++;
        // do inference for each input
        ret = do_inference(dev_idx, img_buf2, buf_len, img_id_tx, &error_code, &img_left);
        if (ret)
            return ret;
        img_id_tx++;

        // Send the rest and get result in loop, with 2 images alternatively
        uint32_t loop = 0;
        if (test_loop > 3)
            loop = test_loop - 2;

        while (loop && !check_ctl_break()) {
            // do inference for each input
            ret = do_inference(dev_idx, img_buf1, buf_len, img_id_tx, &error_code, &img_left);
            if (ret)
                return ret;
            img_id_tx++;

            ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res, post_par);
            if (ret)
                return ret;
            img_id_rx++;

            loop--;
            // Odd loop case
            if (loop == 0)
                break;
            // do inference for each input
            ret = do_inference(dev_idx, img_buf2, buf_len, img_id_tx, &error_code, &img_left);
            if (ret)
                return ret;
            img_id_tx++;

            // retrieve the detection results for each input
            ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res, post_par);
            if (ret)
                return ret;
            img_id_rx++;

            loop--;
        }

        // Get last 2 results
        // retrieve the detection results for each input
        ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res, post_par);
        if (ret)
            return ret;
        img_id_rx++;
        // retrieve the detection results for each input
        ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res, post_par);
        if (ret)
            return ret;
        img_id_rx++;
        // calculate the FPS based on the time for 1000 frames
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
    struct post_parameter_s post_par;
    // parameters for postprocessing: raw FP results and postprocess in host
    post_par.raw_input_row   = IMG_SOURCE_W;
    post_par.raw_input_col   = IMG_SOURCE_W;
    post_par.model_input_row = 224;
    post_par.model_input_col = 224;
    post_par.image_format    = IMAGE_FORMAT_RAW_OUTPUT | IMAGE_FORMAT_BYPASS_PRE;
    user_test_yolo(dev_idx, user_id, post_par, 1000);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
