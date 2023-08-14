/**
 * @file      isi_centernet.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-08-25
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
#include "post_processing_ex.h"
#include "kdpio.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

uint32_t round_up(uint32_t num);
int post_yolo_v3(int model_id, struct kdp_image_s* image_p);
int post_yolo_custom(int model_id, struct kdp_image_s* image_p);
int post_imgnet_classification(int model_id, struct kdp_image_s* image_p);

#define ISI_IMAGE_FILE ("../../input_images/cars_street_at_night_608x608_rgb565.bin")
#define ISI_IMAGE_FILE_T ("../../input_images/car_park_barrier_608x608_rgb565.bin")
#define ISI_IMG_W       608
#define ISI_IMG_H       608
#define ISI_IMG_BPP     2       // Byte Per Pixel = 2 for RGB565
#define ISI_IMG_SIZE    (ISI_IMG_W * ISI_IMG_H * ISI_IMG_BPP)
#define ISI_IMG_FORMAT  (IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565)

#define ISI_APP_ID      APP_CENTER_APP
#define ISI_RESULT_SIZE 0x100000

#define MIN_SCORE_DEFAULT      (0.35f)

#define MIN_SCORE_YOLOv3        (0.60f)
#define MIN_SCORE_YOLOv5        (0.15f)

//#define TEST_CHECK_FRAME_TIME

static uint32_t model_id = KNERON_OBJECTDETECTION_CENTERNET_512_512_3;
static int do_postproc = 0;
static int use_dme_model = 0;

static char inf_res[ISI_RESULT_SIZE];
static char *inf_res_raw = inf_res;
static struct post_parameter_s post_par;

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

static char *s_result_hex[2] = {NULL};
static uint32_t s_result_size[2] = {0};
static bool check_result(uint32_t img_id, uint32_t *r_size, char *r_data)
{
    // cast data to yolo_result_s
    yolo_result_s *yres = (yolo_result_s *)r_data;

    /**
     * img_id = 1:
     *     save box count to s_result_size[0]
     *     save box content to s_result_hex[0]
     * img_id = 2:
     *     save box count to s_result_size[1]
     *     save box content to s_result_hex[1]
     * img_id > 2:
     *     compare box count and content to s_result_size and s_result_hex
     */
    if (img_id <= 2)
    {
        printf("image %d result details:\n", img_id);
        printf("- class count: %d\n", yres->class_count);
        printf("- object count: %d\n", yres->box_count);

        for (uint32_t i = 0; i < yres->box_count; i++)
        {
            printf("- box %d : (%.0f, %.0f) (%.0f, %.0f) score %.3f classnum %d\n", i,
                   yres->boxes[i].x1, yres->boxes[i].y1, yres->boxes[i].x2, yres->boxes[i].y2,
                   yres->boxes[i].score, yres->boxes[i].class_num);
        }
        printf("\n");

        s_result_size[img_id - 1] = *r_size;
        s_result_hex[img_id - 1] = (char *)malloc(*r_size);
        memcpy(s_result_hex[img_id - 1], yres, *r_size);

        return true;
    }
    else
    {
        uint32_t box_count = yres->box_count;
        printf("image %d -> %d object(s)", img_id, box_count);

        bool result_compare_count = false;
        bool result_compare_content = false;
        if ((img_id % 2) == 1)
        {
            if (s_result_size[0] == *r_size)
                result_compare_count = true;
            if (0 == memcmp(s_result_hex[0], yres, *r_size))
                result_compare_content = true;
        }
        else
        {
            if (s_result_size[1] == *r_size)
                result_compare_count = true;
            if (0 == memcmp(s_result_hex[1], yres, *r_size))
                result_compare_content = true;
        }

        if (result_compare_count)
        {
            printf(" ... box_count correct, ");
        }
        else
        {
            printf(" ... box_count failed, ");
        }

        if (result_compare_content)
        {
            printf("content correct\n");
        }
        else
        {
            printf("content failed\n");
        }

        return result_compare_count && result_compare_content;
    }
}

struct kdp_image_s  image_s = { 0 };
struct kdp_image_s* image_p = &image_s;

dme_yolo_res  det_res_s = { 0 };
dme_yolo_res* det_res = (dme_yolo_res *)&det_res_s;

static int get_detection_res_raw(uint32_t img_id, uint32_t inf_size, struct post_parameter_s post_par)
{
    struct imagenet_result_s* imgnet_res;
    raw_cnn_res_t* pRes;
    raw_onode_t* pNode;
    int len, rc = 0;

    // Prepare for postprocessing
    pRes = (raw_cnn_res_t*)inf_res_raw;
    int output_num = pRes->total_nodes;

    uint32_t r_len, offset;
    offset = sizeof(raw_cnn_res_t);

    // Struct to pass the parameters
    RAW_INPUT_COL(image_p) = post_par.raw_input_col;
    RAW_INPUT_ROW(image_p) = post_par.raw_input_row;
    DIM_INPUT_COL(image_p) = post_par.model_input_row;
    DIM_INPUT_ROW(image_p) = post_par.model_input_row;
    RAW_FORMAT(image_p) = post_par.image_format;
    POSTPROC_RESULT_MEM_ADDR(image_p) = (uint32_t*)det_res;
    POSTPROC_OUTPUT_NUM(image_p) = output_num;

    for (int i = 0; i < output_num; i++) {
        pNode = &pRes->onode_a[i];
        r_len = pNode->ch_length * pNode->row_length * round_up(pNode->col_length);
        POSTPROC_OUT_NODE_ADDR(image_p, i) = inf_res_raw + offset;
        POSTPROC_OUT_NODE_ROW(image_p, i) = pNode->row_length;
        POSTPROC_OUT_NODE_CH(image_p, i) = pNode->ch_length;
        POSTPROC_OUT_NODE_COL(image_p, i) = pNode->col_length;
        POSTPROC_OUT_NODE_RADIX(image_p, i) = pNode->output_radix;
        POSTPROC_OUT_NODE_SCALE(image_p, i) = pNode->output_scale;
        offset = offset + r_len;
    }

    // Do postprocessing
    switch (model_id) {
    case TINY_YOLO_V3_416_416_3:
    case TINY_YOLO_V3_608_608_3:
    case YOLO_V3_416_416_3:
    case YOLO_V3_608_608_3:
        len = post_yolo_v3(model_id, image_p);
        if (false == check_result(img_id, (uint32_t *)&len, (char *)det_res))
            rc = -1;
        break;
    case CUSTOMER_MODEL_1:
    case CUSTOMER_MODEL_2:
        len = post_yolo_custom(model_id, image_p);
        if (false == check_result(img_id, (uint32_t *)&len, (char*)det_res))
            rc = -1;
        break;
    case IMAGENET_CLASSIFICATION_MOBILENET_V2_224_224_3:
        len = post_imgnet_classification(model_id, image_p);
        printf("ImageNet: size %d\n", len);
        imgnet_res = (struct imagenet_result_s*)det_res;
        for (int i = 0; i < IMAGENET_TOP_MAX; i++) {
            printf("    [%d] : %d, %f\n", i, imgnet_res->index, imgnet_res->score);
            imgnet_res++;
        }
        break;
    default:
        rc = -1;
        break;
    }

    return rc;
}

static int get_detection_res(uint32_t img_id, uint32_t* r_size, char* r_data)
{
#ifdef TEST_CHECK_FRAME_TIME
    do_check_frame_time();
#endif
    struct imagenet_result_s* imgnet_res;
    struct landmark_result_s* lm_res;
    struct fr_result_s* fr_res;
    int ret = 0;

    switch (model_id) {
    case KNERON_OBJECTDETECTION_CENTERNET_512_512_3:
    case YOLO_V4_416_416_3:
    case KNERON_FD_MBSSD_200_200_3:
    case TINY_YOLO_V3_416_416_3:
    case TINY_YOLO_V3_608_608_3:
    case YOLO_V3_416_416_3:
    case YOLO_V3_608_608_3:
    case KNERON_YOLOV5S_640_640_3:
    case CUSTOMER_MODEL_1:
    case CUSTOMER_MODEL_2:
        if (false == check_result(img_id, r_size, r_data))
        {
            ret = -1;
        }
        break;

    case IMAGENET_CLASSIFICATION_MOBILENET_V2_224_224_3:
        printf("ImageNet: size %d\n", *r_size);
        imgnet_res = (struct imagenet_result_s*)r_data;
        for (int i = 0; i < IMAGENET_TOP_MAX; i++) {
            printf("    [%d] : %d, %f\n", i, imgnet_res->index, imgnet_res->score);
            imgnet_res++;
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
        printf("Wrong model ID %d with size %d\n", model_id, *r_size);
        break;
    }

    return ret;
}

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

    if (*r_size < sizeof(uint32_t)) {
        printf("Img [%d] : result_size %d too small\n", img_id, *r_size);
        return -1;
    }

    if (do_postproc) {
        ret = get_detection_res_raw(img_id, *r_size, post_par);
    } else {
        ret = get_detection_res(img_id, r_size, r_data);
    }

    return ret;
}

int user_test_centernet(int dev_idx, int use_parallel, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    uint32_t image_buf_size = 0;
    char img_buf1[ISI_IMG_SIZE];
    char img_buf2[ISI_IMG_SIZE];
    struct kdp_isi_cfg_s isi_cfg = create_isi_cfg_struct();
    int cfg_size = sizeof(struct kdp_isi_cfg_s);

    if (1)
    {
        // isi configuration
        isi_cfg.app_id = ISI_APP_ID;
        isi_cfg.res_buf_size = ISI_RESULT_SIZE;
        isi_cfg.image_col = ISI_IMG_W;
        isi_cfg.image_row = ISI_IMG_H;

        switch (model_id) {
        case TINY_YOLO_V3_416_416_3:
        case TINY_YOLO_V3_608_608_3:
        case YOLO_V3_416_416_3:
        case YOLO_V3_608_608_3:
            isi_cfg.ext_param[0] = MIN_SCORE_YOLOv3;
            break;
        case KNERON_YOLOV5S_640_640_3:
            isi_cfg.ext_param[0] = MIN_SCORE_YOLOv5;
            break;
        default:
            isi_cfg.ext_param[0] = MIN_SCORE_DEFAULT;
            break;
        }

        if (use_parallel == 1)
            isi_cfg.image_format = ISI_IMG_FORMAT | IMAGE_FORMAT_PARALLEL_PROC;
        else
            isi_cfg.image_format = ISI_IMG_FORMAT;

        if (do_postproc) {
            isi_cfg.image_format |= IMAGE_FORMAT_RAW_OUTPUT;

            post_par.raw_input_row = isi_cfg.image_row;
            post_par.raw_input_col = isi_cfg.image_col;

            switch (model_id) {
            case TINY_YOLO_V3_416_416_3:
            case YOLO_V3_416_416_3:
            case CUSTOMER_MODEL_2:
                post_par.model_input_row = 416;
                post_par.model_input_col = 416;
                break;
            case TINY_YOLO_V3_608_608_3:
            case YOLO_V3_608_608_3:
            case CUSTOMER_MODEL_1:
                post_par.model_input_row = 608;
                post_par.model_input_col = 608;
                break;
            case IMAGENET_CLASSIFICATION_MOBILENET_V2_224_224_3:
                post_par.model_input_row = 224;
                post_par.model_input_col = 224;
                break;
            default:
                printf("ISI: model_id %d host post processing not supported.\n", model_id);
                return -1;
            }
        }

        printf("starting ISI mode ...\n");
        int ret = kdp_start_isi_mode_ext(dev_idx, (char*)&isi_cfg, cfg_size, &error_code, &image_buf_size);
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

        int param;
        if (use_dme_model)
            param = 0;
        else
            param = CONFIG_USE_FLASH_MODEL;
        printf("Config model %d (param 0x%x)\n", model_id, param);
        ret = kdp_isi_config(dev_idx, model_id, param, &error_code);
        if (ret != 0 || error_code)
        {
            printf("could not configure to model %d: %d, %d\n", model_id, ret, error_code);
            model_id = KNERON_OBJECTDETECTION_CENTERNET_512_512_3;
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
            if (test_loop > 2)
                loop = test_loop - 2;
            else {
                test_loop = 2;
                loop = 0;
            }
        }
        else
        {
            loop = test_loop;
        }

        int forever = (test_loop == 0) ? 1 : 0;

        while (loop)
        {
            if (check_ctl_break()) {
                printf("Ctrl-C received. Exit.\n");
                break;
            }

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

        // Release s_result_hex
        if (s_result_hex[0])
        {
            free(s_result_hex[0]);
        }

        if (s_result_hex[1])
        {
            free(s_result_hex[1]);
        }

        if (1)
        {
            double end_time = what_time_is_it_now();
            double elapsed_time, avg_elapsed_time, avg_fps;
            uint32_t actual_loop = test_loop - loop;

            elapsed_time = (end_time - start_time) * 1000;
            avg_elapsed_time = elapsed_time / actual_loop;
            avg_fps = 1000.0f / avg_elapsed_time;

            printf("\n=> Avg %.2f FPS (%.2f ms = %.2f/%d)\n\n",
                    avg_fps, avg_elapsed_time, elapsed_time, actual_loop);
        }
    }
    return 0;
}

static void usage(char *name)
{
    printf("\n");
    printf("usage: ./%s [parallel_mode] [run_frames] [model_id] [host_postproc] [use_dme_model]\n\n", name);
    printf("[parallel_mode] 0: non parallel mode, 1: parallel mode\n");
    printf("[run_frames] number of frames to run, 0 = forever\n");
    printf("[model_id] model ID (optional. Default is 200 for CenterNet.)\n");
    printf("[host_postproc] 0: postprocess at ncpu, 1: postprocess at host\n");
    printf("[use_dme_model] 0: use models in flash, 1: use models in memory (DME)\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 6) {
        usage(argv[0]);
        return -1;
    }

    int use_parallel = atoi(argv[1]);
    int run_frames = atoi(argv[2]);

    if (argc >= 4)
        model_id = atoi(argv[3]);

    if (argc >= 5)
        do_postproc = atoi(argv[4]);

    if (argc >= 6)
        use_dme_model = atoi(argv[5]);

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

    register_hostlib_signal();
    user_test_centernet(dev_idx, use_parallel, run_frames);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
