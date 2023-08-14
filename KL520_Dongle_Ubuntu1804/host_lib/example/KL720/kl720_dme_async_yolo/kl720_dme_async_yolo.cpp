/**
 * @file      dme_async_yolo.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.2 - 2020-11-01
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
#define DME_IMAGE_FILE_T    ("../../input_images/a_man_640x480_rgb565.bin")
#define DME_MODEL_FILE      ("../../input_models/KL720/yolov3_tiny_416/models_720.nef")

#define DME_IMG_SIZE (640 * 480 * 2)
// model size, initialize to a value larger than the real model size
#define DME_MODEL_SIZE (12 * 1024 * 1024)

#define DME_SEND_IMG_RETRY_TIME 2

char inf_res_raw[0x100000];

/**
 * get the data for all output nodes and do postprocessing for YOLO in host side
 * dev_idx: connected device ID. A host can connect several devices
 * inf_size: the size of inference result
 * post_par: parameters for postprocessing in host side
 * return 0 on succeed, 1 on exit
 */
void get_detection_res(int dev_idx, uint32_t inf_size, struct post_parameter_s post_par)
{
    char inf_res[256000];

    kdp_dme_retrieve_res(dev_idx, 0, inf_size, inf_res);
    dme_res *detection_res = (dme_res *)&inf_res;
    printf("Number of class     = %d\n", detection_res->class_count);
    printf("Number of detection = %d\n", detection_res->box_count);
    for (uint32_t i = 0; i < detection_res->box_count; i++) {
        printf("Box[%d] (x1, y1, x2, y2, score, class) = %3d, %3d, %3d, %3d, %f, %d\n", \
             i, (int)detection_res->boxes[i].x1, (int)detection_res->boxes[i].y1, \
             (int)detection_res->boxes[i].x2, (int)detection_res->boxes[i].y2, \
             detection_res->boxes[i].score, detection_res->boxes[i].class_num);
    }
}

void get_detection_res_raw(int dev_idx, uint32_t inf_size, struct post_parameter_s post_par)
{
    raw_cnn_res_t *pRes;
    raw_onode_t *pNode;

    // Get the data for all output nodes: TOTAL_OUT_NUMBER + (H/C/W/RADIX/SCALE) + (H/C/W/RADIX/SCALE) + ...

    // Skip post processing if raw data size is too large
    if (inf_size >= 1000000) {
        printf("*******************   BAD INFERENCE SIZE [%d]    ***********************\n", inf_size);
        return;
    }

    // + FP_DATA (size: H*C*16*CEIL(W/16) ) + FP_DATA + ...
    kdp_dme_retrieve_res(dev_idx, 0, inf_size, inf_res_raw);

    // Prepare for postprocessing
    pRes = (raw_cnn_res_t *)inf_res_raw;
    int output_num = pRes->total_nodes;

    uint32_t r_len, offset;
    struct yolo_result_s *det_res = (struct yolo_result_s *)calloc(1, sizeof(dme_yolo_res));
    struct kdp_image_s *image_p = (struct kdp_image_s *)calloc(1, sizeof(struct kdp_image_s));
    offset = sizeof(raw_cnn_res_t);

    // save the parameters to struct kdp_image_s
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

int user_test_dme(int dev_idx, int user_id, uint16_t mode, struct post_parameter_s post_par, \
                  struct kdp_dme_cfg_s dme_cfg)
{
    uint32_t model_id, ret_size;
    int ret;

    if (1) {
        int model_size = 0;
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

        printf("starting DME mode ...\n");
        ret = kdp_start_dme_ext(dev_idx, p_buf, model_size, &ret_size);
        if (ret != 0) {
            printf("could not set to DME mode:%d..\n", ret_size);
            delete[] p_buf;
            return -1;
        }

        delete[] p_buf;
        printf("DME mode succeeded...\n");
        usleep(SLEEP_TIME);
    }

    /* start of calling kdp_dme_configure to set dme configuration */
    if (1) {
        int dat_size = 0;

        dat_size = sizeof(struct kdp_dme_cfg_s);
        printf("starting DME configure ...\n");
        int ret = kdp_dme_configure(dev_idx, (char *)&dme_cfg, dat_size, &model_id);
        if (ret != 0) {
            printf("could not set to DME configure mode..\n");
            return -1;
        }
        printf("DME configure model [%d] succeeded...\n", model_id);
        usleep(SLEEP_TIME);
    }
    /* end of call kdp_dme_configure to set dme configuration */

    /* call kdp_dme_inference to do inference with the binary inputs in DME async mode
       call kdp_dme_get_status to get the dme inference status (0: not completed, 1: completed)
       call kdp_dme_retrieve_res to retrieve the detection results after getting completed status
       call kdp_end_dme to end dme mode */
    if (1) {
        uint32_t inf_size = 0;
        char inf_res[256000];
        bool res_flag = true;

        /* start of read image data for test images */
        char img_buf[DME_IMG_SIZE];
        char img_buf2[DME_IMG_SIZE];
        uint32_t buf_len = 0;

        // TODO need set image name here
        memset(img_buf, 0, sizeof(img_buf));
        int n_len = read_file_to_buf(img_buf, DME_IMAGE_FILE, DME_IMG_SIZE);
        if (n_len <= 0) {
            printf("reading dme image file failed:%d...\n", n_len);
            return -1;
        }
        
        n_len = read_file_to_buf(img_buf2, DME_IMAGE_FILE_T, DME_IMG_SIZE);
        if (n_len <= 0) {
            printf("reading dme image file 2 failed:%d...\n", n_len);
            return -1;
        }

        buf_len = n_len;
        /* end of read image data for test images */

        uint32_t ssid_1 = 0;
        uint32_t ssid_2 = 0;
        uint16_t status = 0;

        /*********************************************************************************************************/
        // To start async mode, MUST send 2 images firstly (kdp_dme_inference for image1 and image2), 
        // then start to get the status for the first image1!!!
        /***************************************************************************************/

        printf("starting DME inference in [async mode] ...\n");

        int loop = 0;
        // The total num of loops is (loop_end + 1)
        int loop_end = 99;  // minimum is 1
        int ret = 0;
        double start;
        float fps = 0;
        
        start = what_time_is_it_now();
        // send img session 1. Retry for 2 times if not succeed.
        for (int i = 0; i < 1 + DME_SEND_IMG_RETRY_TIME; i++) {
            if (check_ctl_break())
                return 0;
            // call kdp_dme_inference to do inference for each input
            ret = kdp_dme_inference(dev_idx, img_buf, buf_len, &ssid_1, &res_flag, inf_res, mode, model_id);
                       
            // Return if not succeed after retry for 2 times.
            if (ret == -1) {
                printf("could not set to DME inference mode..[error = %d]\n", ret);
                return -1;
            }

            if (ret == -2 && DME_SEND_IMG_RETRY_TIME == i) {
                printf("could not set to DME inference mode after retrying %d times..[error = %d]\n", DME_SEND_IMG_RETRY_TIME, ret);
                return -1;
            }
            
            if (ret == 0) {
                break;
            }
            printf("retry to set to DME inference mode..[time = %d]\n", i);
        }

        for(loop = 0; loop < loop_end; loop++) {
            if (check_ctl_break())
                return 0;
            // calculate the FPS based on the time for loop_end+1 frames
            if (loop & 1) {
                // send img session 1
                for (int i = 0; i < 1 + DME_SEND_IMG_RETRY_TIME; i++) {
                    // call kdp_dme_inference to do inference for each input
                    ret = kdp_dme_inference(dev_idx, img_buf, buf_len, &ssid_1, &res_flag, inf_res, mode, model_id);
                    
                    // Return if not succeed after retry for 2 times.
                    if (ret == -1) {
                        printf("could not set to DME inference mode..[error = %d]\n", ret);
                        return -1;
                    }

                    if (ret == -2 && DME_SEND_IMG_RETRY_TIME == i) {
                        printf("could not set to DME inference mode after retrying %d times..[error = %d]\n", DME_SEND_IMG_RETRY_TIME, ret);
                        return -1;
                    }
                    
                    if (ret == 0) {
                        break;
                    }
                    printf("retry to set to DME inference mode..[time = %d]\n", i);
                }
                
                // get status for session 2
                while (!check_ctl_break()) {
                    status = 0; // Must re-initialize status to 0
                    // call kdp_dme_get_status to get the dme inference status for session 2
                    kdp_dme_get_status(dev_idx, (uint16_t *)&ssid_2, &status, &inf_size, inf_res);
                    if (1 == status) {
                        // get the data for all output nodes and do postprocessing for YOLO in host side
                        printf("Image %d:\n", loop + 1);
                        if (user_id)
                            get_detection_res(dev_idx, inf_size, post_par);
                        else
                            get_detection_res_raw(dev_idx, inf_size, post_par);
                        break;
                    }
                }
                if (check_ctl_break())
                    return 0;
            } else {
                // send img session 2
                for (int i = 0; i < 1 + DME_SEND_IMG_RETRY_TIME; i++) {
                    if (check_ctl_break())
                        return 0;
                    // call kdp_dme_inference to do inference for each input
                    ret = kdp_dme_inference(dev_idx, img_buf2, buf_len, &ssid_2, &res_flag, inf_res, mode, model_id);
                    
                    // Return if not succeed after retry for 2 times.
                    if (ret == -1) {
                        printf("could not set to DME inference mode..[error = %d]\n", ret);
                        return -1;
                    }

                    if (ret == -2 && DME_SEND_IMG_RETRY_TIME == i) {
                        printf("could not set to DME inference mode after retrying %d times..[error = %d]\n", DME_SEND_IMG_RETRY_TIME, ret);
                        return -1;
                    }
                    
                    if (ret == 0) {
                        break;
                    }
                    printf("retry to set to DME inference mode..[time = %d]\n", i);
                }
                
                // get status for session 1
                while (!check_ctl_break()) {
                    status = 0; // Must re-initialize status to 0
                    // call kdp_dme_get_status to get the dme inference status for session 1
                    kdp_dme_get_status(dev_idx, (uint16_t *)&ssid_1, &status, &inf_size, inf_res);

                    if (1 == status) {
                        // get the data for all output nodes and do postprocessing for YOLO in host side
                        printf("Image %d:\n", loop + 1);
                        if (user_id)
                            get_detection_res(dev_idx, inf_size, post_par);
                        else
                            get_detection_res_raw(dev_idx, inf_size, post_par);
                        break;
                    }
                }
                if (check_ctl_break())
                    return 0;
            }
        }

        // loop++ in previous for loop
        if ((loop - 1) & 1) {           
            // get status for session 1 & 2
            while (!check_ctl_break()) {
                status = 0; // Must re-initialize status to 0
                // call kdp_dme_get_status to get the dme inference status for session 1
                kdp_dme_get_status(dev_idx, (uint16_t *)&ssid_1, &status, &inf_size, inf_res);
                if (1 == status) {
                    // get the data for all output nodes and do postprocessing in host side
                    printf("Image %d:\n", loop + 1);
                    if (user_id)
                        get_detection_res(dev_idx, inf_size, post_par);
                    else
                        get_detection_res_raw(dev_idx, inf_size, post_par);
                    break;
                }
            }
            if (check_ctl_break())
                return 0;
        } else {           
            // get status for session 2 & 1
            while (!check_ctl_break()) {
                status = 0; // Must re-initialize status to 0
                // get the dme inference status for session 2
                kdp_dme_get_status(dev_idx, (uint16_t *)&ssid_2, &status, &inf_size, inf_res);
                if (1 == status) {
                    // get the data for all output nodes and do postprocessing in host side
                    printf("Image %d:\n", loop + 1);
                    if (user_id)
                        get_detection_res(dev_idx, inf_size, post_par);
                    else
                        get_detection_res_raw(dev_idx, inf_size, post_par);
                    break;
                }
            }
            if (check_ctl_break())
                return 0;
        }

        fps = 1./((what_time_is_it_now() - start)/loop);
        printf("\naverage time on %d frames: %f ms/frame, fps: %f\n", loop_end+1, (1/fps)*1000, fps);
        printf("DME inference succeeded......\n");
        // call kdp_end_dme to end dme mode after finishing the inference for all inputs
        //kdp_end_dme(dev_idx);
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    struct post_parameter_s post_par;
    struct kdp_dme_cfg_s dme_cfg = create_dme_cfg_struct();
    
    // parameters for postprocessing in host side
    post_par.raw_input_row   = 480;
    post_par.raw_input_col   = 640;
    post_par.model_input_row = 416;
    post_par.model_input_col = 416;
    post_par.image_format    = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565 | IMAGE_FORMAT_RAW_OUTPUT;

    // dme configuration
    dme_cfg.model_id     = TINY_YOLO_V3_416_416_3; // model id when compiling in toolchain
    dme_cfg.output_num   = 2;            // number of output node for the model
    dme_cfg.image_col    = post_par.raw_input_col;
    dme_cfg.image_row    = post_par.raw_input_row;
    dme_cfg.image_ch     = 3;
    // 0x80000060 with add_norm option on when compiling in toolchain
    //dme_cfg.image_format = post_par.image_format;
    if (user_id)
        dme_cfg.image_format = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565;
    else
        dme_cfg.image_format = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565 | IMAGE_FORMAT_RAW_OUTPUT;
    
    // Async mode (1)
    uint16_t mode = 1;

    //dme test
    user_test_dme(dev_idx, user_id, mode, post_par, dme_cfg);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif    // 100 for yolo v3
