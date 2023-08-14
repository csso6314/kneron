/**
 * @file      user_test_dme.cpp for KL720
 * @brief     kdp host lib user test examples 
 * @version   0.2 - 2020-10-01
 * @copyright (c) 2019-2020 Kneron Inc. All right reserved.
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

extern "C" {
uint32_t round_up(uint32_t num);
int post_yolo_v3(int model_id, struct kdp_image_s *image_p);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define DME_IMAGE_FILE      ("../../input_images/a_dog_640x480_rgb565.bin")
#define DME_MODEL_FILE      ("../../input_models/KL720/yolov3_tiny_416/models_720.nef")

/* Model input dimension : Tiny Yolo 416 */
#define IMG_FORMAT          (NPU_FORMAT_RGB565 | IMAGE_FORMAT_SUB128)

#define DME_IMG_SIZE        (640*480*2)  // RGB565
#define DME_MODEL_SIZE      (12 * 1024 * 1024)
#define DME_FWINFO_SIZE     128

char inf_res_raw[768000];

void get_detection_res_raw(int dev_idx, uint32_t inf_size, struct post_parameter_s post_par)
{
    // Get the data for all output nodes: TOTAL_OUT_NUMBER + (H/C/W/RADIX/SCALE) + (H/C/W/RADIX/SCALE) + ...
    // + FP_DATA + FP_DATA + ...
    raw_cnn_res_t *pRes;
    raw_onode_t *pNode;
    kdp_dme_retrieve_res(dev_idx, 0, inf_size, inf_res_raw);

    // Prepare for postprocessing
    pRes = (raw_cnn_res_t *)inf_res_raw;
    int output_num = pRes->total_nodes;

    uint32_t r_len, offset;
    struct yolo_result_s *det_res = (struct yolo_result_s *)calloc(1, sizeof(dme_yolo_res));
    struct kdp_image_s *image_p = (struct kdp_image_s *)calloc(1, sizeof(struct kdp_image_s));
    offset = sizeof(raw_cnn_res_t);

    // Struct to pass the parameters
    RAW_INPUT_COL(image_p) = post_par.raw_input_col;
    RAW_INPUT_ROW(image_p) = post_par.raw_input_row;
    DIM_INPUT_COL(image_p) = post_par.model_input_row;
    DIM_INPUT_ROW(image_p) = post_par.model_input_row;
    RAW_FORMAT(image_p) = post_par.image_format;
    POSTPROC_RESULT_MEM_ADDR(image_p) = (uint32_t *)det_res;
    POSTPROC_OUTPUT_NUM(image_p) = output_num;

    for (int i = 0; i < output_num; i++) {
        if (check_ctl_break())
            return;
        pNode = &pRes->onode_a[i];
        r_len = pNode->ch_length * pNode->row_length  * round_up(pNode->col_length);
        POSTPROC_OUT_NODE_ADDR(image_p, i) = inf_res_raw + offset;
        POSTPROC_OUT_NODE_ROW(image_p, i) = pNode->row_length;
        POSTPROC_OUT_NODE_CH(image_p, i) = pNode->ch_length;
        POSTPROC_OUT_NODE_COL(image_p, i) = pNode->col_length;
        POSTPROC_OUT_NODE_RADIX(image_p, i) = pNode->output_radix;
        POSTPROC_OUT_NODE_SCALE(image_p, i) = pNode->output_scale;
        offset = offset + r_len;
    }

    // Do postprocessing
    post_yolo_v3(0, image_p);

    free(image_p);
    free(det_res);
}

int user_test_dme(int dev_idx, int user_id, int mode, struct post_parameter_s post_par, struct kdp_dme_cfg_s *dme_cfg)
{
    uint32_t model_id, ret_size;
    int ret;

    if (1) {
        uint32_t model_size = 0;
        int ret;
        char* p_buf = NULL;

        // read model file
        p_buf = new char[DME_MODEL_SIZE];
        memset(p_buf, 0, DME_MODEL_SIZE);
        ret = read_file_to_buf(p_buf, DME_MODEL_FILE, DME_MODEL_SIZE);
        if (ret <= 0) {
            printf("reading model file failed:%d...\n", ret);
            delete[] p_buf;
            return -1;
        }
        model_size = ret;

        printf("starting DME inference in [serial mode] ...\n");
        ret = kdp_start_dme_ext(dev_idx, p_buf, model_size, &ret_size);
        if (ret != 0) {
            printf("could not set to DME mode:%d..\n", model_size);
            delete[] p_buf;
            return -1;
        }

        delete[] p_buf;
        printf("DME mode succeeded...\n");
        usleep(SLEEP_TIME);
    }

    if (1) {
        uint32_t dat_size;
        dat_size = sizeof(struct kdp_dme_cfg_s);
        printf("starting DME configure ...\n");
        if (user_id == 1) {
            ret = kdp_dme_configure(dev_idx, (char *)&(dme_cfg[1]), dat_size, &model_id);
        } else {
            ret = kdp_dme_configure(dev_idx, (char *)&(dme_cfg[0]), dat_size, &model_id);
        }

        if (ret != 0) {
            printf("could not set to DME configure mode..\n");
            return -1;
        }
        printf("DME configure model[%d] succeeded...\n", model_id);
        usleep(SLEEP_TIME);
    }

    if (1) {
        uint32_t inf_size = 0;
        char inf_res[256000];
        bool res_flag = true;

        char img_buf[DME_IMG_SIZE];
        uint32_t buf_len = 0;

        // TODO need set image name here
        memset(img_buf, 0, sizeof(img_buf));
        int n_len = read_file_to_buf(img_buf, DME_IMAGE_FILE, DME_IMG_SIZE);
        if (n_len <= 0) {
            printf("reading dme image file failed:%d...\n", n_len);
            return -1;
        }

        buf_len = n_len;

        double start;
        float fps = 0;
        int loop = 100;
        
        printf("starting DME inference for [%d] times ...\n", loop);

        start = what_time_is_it_now();
        for (int i = 0; i < loop; i++) {
            if (check_ctl_break())
                return 0;
            int ret = kdp_dme_inference(dev_idx, img_buf, buf_len, &inf_size, &res_flag, inf_res, mode, model_id);
            if (ret != 0) {
                printf("could not set to DME inference mode..\n");
                return -1;
            }
            if (user_id == 1) {
                printf("Image %d:\n", i + 1);
                kdp_dme_retrieve_res(dev_idx, 0, inf_size, inf_res);
                dme_res *detection_res = (dme_res *)&inf_res;
                printf("Number of class     = %d\n", detection_res->class_count);
                printf("Number of detection = %d\n", detection_res->box_count);
                for (uint32_t i = 0; i < detection_res->box_count; i++) {
                    printf("Box[%d] (x1, y1, x2, y2, score, class) = %3d, %3d, %3d, %3d, %f, %d\n",
i+1, (int)detection_res->boxes[i].x1, (int)detection_res->boxes[i].y1, (int)detection_res->boxes[i].x2, (int)detection_res->boxes[i].y2, detection_res->boxes[i].score, detection_res->boxes[i].class_num);
                }
            } else {
                printf("Image %d: Postprocess on host\n", i + 1);
                get_detection_res_raw(dev_idx, inf_size, post_par);
            }
            if (check_ctl_break())
                return 0;
        }
        fps = 1./((what_time_is_it_now() - start)/loop);
        printf("\naverage time on 100 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);

        printf("DME inference succeeded...\n");
        // kdp_end_dme(dev_idx); // TODO: make reset in kdp_end_dme work on 720
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    struct post_parameter_s post_par;
    struct kdp_dme_cfg_s dme_cfg[2];
    memset(dme_cfg, 0, 2*sizeof(kdp_dme_cfg_s));
    
    // parameters for postprocessing: raw FP results and postprocess in host
    post_par.raw_input_row   = 480;
    post_par.raw_input_col   = 640;
    post_par.model_input_row = 416;
    post_par.model_input_col = 416;
    post_par.image_format    = IMG_FORMAT | IMAGE_FORMAT_RAW_OUTPUT;

    // dme configuration: raw FP results
    dme_cfg[0].model_id     = TINY_YOLO_V3_416_416_3;  // model id when compiling in toolchain
    dme_cfg[0].output_num   = 2;             // number of output node for the model
    dme_cfg[0].image_col    = 640;
    dme_cfg[0].image_row    = 480;
    dme_cfg[0].image_ch     = 3;
    // 0x10080000 with add_norm option false when compiling in toolchain
    // Provide the input of preprocessed rgba image with yolo preprocessing method
    // For example: rgb565 -> rgba -> rgba (right_shift 1 bit)
    dme_cfg[0].image_format = IMG_FORMAT | IMAGE_FORMAT_RAW_OUTPUT;

    // dme configuration: detection results
    dme_cfg[1].model_id     = TINY_YOLO_V3_416_416_3;  // model id when compiling in toolchain
    dme_cfg[1].output_num   = 2;             // number of output node for the model
    dme_cfg[1].image_col    = 640;
    dme_cfg[1].image_row    = 480;
    dme_cfg[1].image_ch     = 3;
    // 0x00080000 with add_norm option false when compiling in toolchain
    dme_cfg[1].image_format = IMG_FORMAT;

    // Serial mode (0)
    // Compared to serial mode, async mode has better performance.
    // Refer to user_test_dme_async_yolo.cpp for example in async mode.
    uint16_t mode = 0;

    //dme test
    user_test_dme(dev_idx, user_id, mode, post_par, dme_cfg);
    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
