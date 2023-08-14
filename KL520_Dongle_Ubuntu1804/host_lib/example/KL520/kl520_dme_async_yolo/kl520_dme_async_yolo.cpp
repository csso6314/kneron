/**
 * @file      kl520_dme_async_yolo.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2019-08-01
 * @copyright (c) 2019 Kneron Inc. All right reserved.
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

#define DME_IMAGE_FILE      ("../../input_images/intersection_608x608_rgb565.bin")
#define DME_IMAGE_FILE_T    ("../../input_images/street_608x608_rgb565.bin")
#define DME_MODEL_FILE      ("../../input_models/KL520/tiny_yolo_v3_608/models_520.nef")

#define DME_IMG_SIZE (608 * 608 * 2)
// model size, initialize to a value larger than the real model size
#define DME_MODEL_SIZE (20 * 1024 * 1024)
// firmware information size, initialize to a value larger than the real fw info size
#define DME_FWINFO_SIZE 128

#define DME_SEND_IMG_RETRY_TIME 2

/**
 * get the data for all output nodes and do postprocessing for YOLO in host side
 * dev_idx: connected device ID. A host can connect several devices
 * inf_size: the size of inference result
 * post_par: parameters for postprocessing in host side
 * return 0 on succeed, 1 on exit
 */
void get_detection_res(int dev_idx, uint32_t inf_size, struct post_parameter_s post_par)
{
    // array byte size larger than the size of the data for all output nodes
    char inf_res[768000];
    // Get the data for all output nodes: TOTAL_OUT_NUMBER + (H/C/W/RADIX/SCALE) + (H/C/W/RADIX/SCALE) + ...
    // + FP_DATA (size: H*C*16*CEIL(W/16) ) + FP_DATA + ...
    kdp_dme_retrieve_res(dev_idx, 0, inf_size, inf_res);

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
        if (check_ctl_break())
            return;
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
}

int user_test_dme(int dev_idx, uint16_t mode, struct post_parameter_s post_par, \
                  struct kdp_dme_cfg_s dme_cfg)
{
    // 100 for yolo608
    uint32_t model_id_608 = 0;

    /* start of reading fw info and model data, calling kdp_start_dme_ext API to send the data to Kneron device
       via USB and start DME mode */
    if (1) {
        printf("reading model NEF file from '%s'\n", DME_MODEL_FILE);

        long model_size;
        char *model_buf = read_file_to_buffer_auto_malloc(DME_MODEL_FILE, &model_size);
        if(model_buf == NULL)
            return -1;

        printf("starting DME inference ...\n");
        uint32_t ret_size = 0;
        int ret = kdp_start_dme_ext(dev_idx, model_buf, model_size, &ret_size);
        if (ret != 0) {
            printf("could not set to DME mode:%d..\n", ret_size);
            free(model_buf);
            return -1;
        }

        free(model_buf);
        
        printf("DME mode succeeded...\n");
        usleep(SLEEP_TIME);
    }
    /* end of reading fw info and model data, calling API to send the data to Kneron device and start DME mode */

    /* start of calling kdp_dme_configure to set dme configuration */
    if (1) {
        int dat_size = 0;

        dat_size = sizeof(struct kdp_dme_cfg_s);
        printf("starting DME configure ...\n");
        int ret = kdp_dme_configure(dev_idx, (char *)&dme_cfg, dat_size, &model_id_608);
        if (ret != 0) {
            printf("could not set to DME configure mode..\n");
            return -1;
        }
        printf("DME configure model [%d] succeeded...\n", model_id_608);
        usleep(SLEEP_TIME);
    }
    /* end of call kdp_dme_configure to set dme configuration */

    /* call kdp_dme_inference to do inference with the binary inputs in DME async mode
       call kdp_dme_get_status to get the dme inference status (0: not completed, 1: completed)
       call kdp_dme_retrieve_res to retrieve the detection results after getting completed status
       call kdp_end_dme to end dme mode */
    if (1) {
        uint32_t inf_size = 0;
        char *inf_res = (char *)malloc(256*1024);
        bool res_flag = true;

        /* start of read image data for test images */
        char *img_buf = (char *)malloc(DME_IMG_SIZE);
        char *img_buf2 = (char *)malloc(DME_IMG_SIZE);
        uint32_t buf_len = 0;

        // TODO need set image name here
        memset(img_buf, 0, sizeof(DME_IMG_SIZE));
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
        int loop_end = 101;
        int ret = 0;
        double start;
        float fps = 0;
        
        start = what_time_is_it_now();
        // send img session 1. Retry for 2 times if not succeed.
        for (int i = 0; i < 1 + DME_SEND_IMG_RETRY_TIME; i++) {
            if (check_ctl_break())
                return 0;
            // call kdp_dme_inference to do inference for each input
            ret = kdp_dme_inference(dev_idx, img_buf, buf_len, &ssid_1, &res_flag, inf_res, mode, model_id_608);
                       
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
            // calculate the FPS based on the time for "loop_end" frames       
            if (loop == loop_end-1) {
                fps = 1./((what_time_is_it_now() - start)/loop);
                printf("[INFO] average time on 100 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);
            }
            if (loop & 1) {
                // send img session 1
                for (int i = 0; i < 1 + DME_SEND_IMG_RETRY_TIME; i++) {
                    // call kdp_dme_inference to do inference for each input
                    ret = kdp_dme_inference(dev_idx, img_buf, buf_len, &ssid_1, &res_flag, inf_res, mode, model_id_608);
                    
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
                        get_detection_res(dev_idx, inf_size, post_par);
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
                    ret = kdp_dme_inference(dev_idx, img_buf2, buf_len, &ssid_2, &res_flag, inf_res, mode, model_id_608);
                    
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
                        get_detection_res(dev_idx, inf_size, post_par);
                        break;
                    }
                }
                if (check_ctl_break())
                    return 0;
            }
        }

        // loop++ in previous for loop
        if ((loop - 1) & 1) {           
            // get status for session 1
            while (!check_ctl_break()) {
                status = 0; // Must re-initialize status to 0
                // call kdp_dme_get_status to get the dme inference status for session 1
                kdp_dme_get_status(dev_idx, (uint16_t *)&ssid_1, &status, &inf_size, inf_res);
                if (1 == status) {
                    // get the data for all output nodes and do postprocessing in host side
                    get_detection_res(dev_idx, inf_size, post_par);
                    break;
                }
            }
            if (check_ctl_break())
                return 0;
        } else {           
            // get status for session 2
            while (!check_ctl_break()) {
                status = 0; // Must re-initialize status to 0
                // get the dme inference status for session 2
                kdp_dme_get_status(dev_idx, (uint16_t *)&ssid_2, &status, &inf_size, inf_res);
                if (1 == status) {
                    // get the data for all output nodes and do postprocessing in host side
                    get_detection_res(dev_idx, inf_size, post_par);
                    break;
                }
            }
            if (check_ctl_break())
                return 0;
        }

        printf("DME inference succeeded......\n");
        // call kdp_end_dme to end dme mode after finishing the inference for all inputs
        kdp_end_dme(dev_idx);

        free(inf_res);
        free(img_buf);
        free(img_buf2);

    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    struct post_parameter_s post_par;
    struct kdp_dme_cfg_s dme_cfg = create_dme_cfg_struct();
    
    // parameters for postprocessing in host side
    post_par.raw_input_row   = 608;
    post_par.raw_input_col   = 608;
    post_par.model_input_row = 608;
    post_par.model_input_col = 608;
    post_par.image_format    = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565 | IMAGE_FORMAT_RAW_OUTPUT;

    // dme configuration
    dme_cfg.model_id     = TINY_YOLO_V3_608_608_3; // model id when compiling in toolchain
    dme_cfg.output_num   = 2;                      // number of output node for the model
    dme_cfg.image_col    = post_par.raw_input_col;
    dme_cfg.image_row    = post_par.raw_input_row;
    dme_cfg.image_ch     = 3;
    // 0x80000060 with add_norm option on when compiling in toolchain
    dme_cfg.image_format = post_par.image_format;
    
    // Async mode (1)
    uint16_t mode = 1;

    //dme test
    user_test_dme(dev_idx, mode, post_par, dme_cfg);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
