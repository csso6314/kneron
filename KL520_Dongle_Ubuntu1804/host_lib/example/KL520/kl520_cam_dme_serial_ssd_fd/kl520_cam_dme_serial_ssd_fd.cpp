/**
 * @file      kl520_cam_dme_serial_ssd_fd.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-7-7
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include "user_util.h"
#include "ipc.h"
#include "base.h"
#include "model_res.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
using namespace std;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define DME_MODEL_FILE      ("../../input_models/KL520/ssd_fd_lm/models_520.nef")
#define DME_MODEL_SIZE (20 * 1024 * 1024)

#define IMG_SOURCE_W 640
#define IMG_SOURCE_H 480
#define FD_IMG_SIZE (IMG_SOURCE_W * IMG_SOURCE_H * 2)
#define FD_MIN_SCORE        (0.5f)

// To print the dection results
//#define LOG
std::vector<cv::Mat> frames;

enum {
    no_face,
    face,
    with_mask,
};

int display_detection(int dev_idx, char *inf_res) {
    dme_res *detection_res = (dme_res *)inf_res;

    cvtColor(frames.front(), frames.front(), CV_BGR5652BGR);
    for (uint32_t i = 0; i < detection_res->box_count; i++) {
        if (check_ctl_break())
            return 0;
        if (detection_res->boxes[i].class_num == face)
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(255,255,0),2);
        else if (detection_res->boxes[i].class_num == with_mask)
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(0,255,0),2);
        else
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(255,0,0),2);

#ifdef LOG
        printf("[INFO] Box[%d] (x1, y1, x2, y2, score, class) = %f, %f, %f, %f, %f, %d\n", \
                i, detection_res->boxes[i].x1, detection_res->boxes[i].y1, \
                detection_res->boxes[i].x2, detection_res->boxes[i].y2, \
                detection_res->boxes[i].score, detection_res->boxes[i].class_num);
#endif
    }

    imshow("Camera output", frames.front());
    vector<cv::Mat>::iterator first_frame = frames.begin();
    frames.erase(first_frame);
    if (cv::waitKey(1) >= 0) {
        kdp_end_dme(dev_idx);
        return 1;
    }
    return 0;
}

static IplImage capture_frame(cv::VideoCapture cap)
{
    cv::Mat frame_input;

    cap >> frame_input;
    cvtColor(frame_input, frame_input, CV_BGR2BGR565);
    cv::flip(frame_input, frame_input, 1);
    frames.push_back(frame_input);

#if CV_MAJOR_VERSION > 3 || (CV_MAJOR_VERSION == 3 && CV_SUBMINOR_VERSION >= 9)
    return cvIplImage(frame_input);
#else
    return (IplImage)frame_input;
#endif
}

int user_test_dme_fd(int dev_idx, int user_id, int mode, struct kdp_dme_cfg_s dme_cfg, uint32_t test_loop)
{
    int ret = 0;
    uint32_t ret_model_id = 0;

    if (1) {
        char* p_buf = NULL;
        uint32_t buf_len = 0;
        uint32_t ret_size = 0;

        // read model data
        p_buf = new char[DME_MODEL_SIZE];
        memset(p_buf, 0, DME_MODEL_SIZE);
        int n_len = read_file_to_buf(p_buf, DME_MODEL_FILE, DME_MODEL_SIZE);
        if (n_len <= 0) {
            printf("reading model file failed:%d...\n", n_len);
            delete[] p_buf;
            return -1;
        }
        buf_len = n_len;

        printf("starting DME inference in [serial mode] ...\n");
        int ret = kdp_start_dme_ext(dev_idx, p_buf, buf_len, &ret_size);
        if (ret != 0) {
            printf("could not set to DME mode:%d..\n", ret_size);
            delete[] p_buf;
            return -1;
        }

        delete[] p_buf;
        printf("DME mode succeeded...\n");
        usleep(SLEEP_TIME);
    }

    if (1) {
        int dat_size = 0;

        dat_size = sizeof(struct kdp_dme_cfg_s);
        printf("starting DME configure ...\n");
        ret = kdp_dme_configure(dev_idx, (char *)&dme_cfg, dat_size, &ret_model_id);
        if (ret != 0) {
            printf("could not set to DME configure mode..\n");
            return -1;
        }
        printf("DME configure model[%d] succeeded...\n", ret_model_id);
        usleep(SLEEP_TIME);
    }

    if (1) {
        uint32_t inf_size = 0;
        bool res_flag = true;

        uint32_t buf_len = FD_IMG_SIZE;
        char inf_res[2560];

        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            printf("Unable to connect to camera.\n");
            return 1;
        }
        cap.set(CV_CAP_PROP_FRAME_WIDTH,IMG_SOURCE_W);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT,IMG_SOURCE_H);
        IplImage ipl_img;

        double start;
        float fps = 0;
        int loop = 0;

        printf("starting DME inference ...\n");
        start = what_time_is_it_now();
        while (test_loop && !check_ctl_break()) {
            ipl_img = capture_frame(cap);
            ret = kdp_dme_inference(dev_idx, (char *)ipl_img.imageData, buf_len, &inf_size, &res_flag, (char*) &inf_res, 0, 0);
            if (ret != 0) {
                printf("Inference failed..\n");
                return -1;
            }

            kdp_dme_retrieve_res(dev_idx, 0, inf_size, (char*) &inf_res);

            ret = display_detection(dev_idx, inf_res);
            if(ret) return 0;

            loop++;
            test_loop--;
            if (loop == 100) {
                fps = 1./((what_time_is_it_now() - start)/loop);
                printf("[INFO] average time on 100 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);
            }
        }
        printf("DME inference succeeded...\n");
        kdp_end_dme(dev_idx);
        usleep(SLEEP_TIME);
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    struct kdp_dme_cfg_s dme_cfg = create_dme_cfg_struct();

    // dme configuration
    dme_cfg.model_id     = KNERON_FD_MASK_MBSSD_200_200_3;      // the first model id when compiling in toolchain
    dme_cfg.output_num   = 8;                 // number of output node for the last model
    dme_cfg.image_col    = IMG_SOURCE_W;
    dme_cfg.image_row    = IMG_SOURCE_H;
    dme_cfg.image_ch     = 3;
    dme_cfg.image_format = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565;

    dme_cfg.ext_param[0] = FD_MIN_SCORE;      // if not set, use the default threshold in Kneron device
    // Serial mode (0)
    uint16_t mode = 0;

    //dme test
    user_test_dme_fd(dev_idx, user_id, mode, dme_cfg, 200);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
