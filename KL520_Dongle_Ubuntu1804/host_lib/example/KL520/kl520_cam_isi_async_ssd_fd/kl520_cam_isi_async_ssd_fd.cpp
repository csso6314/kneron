/**
 * @file      kl520_cam_isi_async_ssd_fd.cpp
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
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
using namespace std;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define OD_MIN_PERSON_SCORE         (0.2f)
#define OD_MIN_OTHER_CLASS_SCORE    (0.4f)
#define OD_BBOX_RATIO               (0.93f)
#define IMG_SOURCE_W 640
#define IMG_SOURCE_H 480
#define OD8C_IMG_SIZE (IMG_SOURCE_W * IMG_SOURCE_H * 2)
#define OD8C_APP_ID   APP_FD_LM
#define OD8C_RESULT_SIZE 2048
// To print the dection results
//#define LOG
std::vector<cv::Mat> frames;

//#define ODTY_IMG_SIZE (300 * 300 * 2)
#define ODTY_IMG_SIZE (IMG_SOURCE_W * IMG_SOURCE_H * 2)

#define ODTY_APP_ID   APP_FD_LM
#define ODTY_RESULT_SIZE 2048

/*
enum {
    background,
    bicycle,
    bus,
    car,
    cat,
    dog,
    motorbike,
    person,
};
*/
enum {
    no_face,
    face,
    with_mask,
};

int display_detection(int dev_idx, char *inf_res) {
    float x, y, w, h;
    float box_ratio;
    float scale = 1. / (IMG_SOURCE_W * IMG_SOURCE_H);

    dme_res *detection_res = (dme_res *)inf_res;

//#ifdef LOG
    printf("boxes %d (show person > %f, other classes > %f) \n",
        detection_res->box_count, OD_MIN_PERSON_SCORE, OD_MIN_OTHER_CLASS_SCORE);
//#endif
    cvtColor(frames.front(), frames.front(), CV_BGR5652BGR);
    for (uint32_t i = 0; i < detection_res->box_count; i++) {
        if (check_ctl_break())
            return 0;
        // filter detections for different classes
        //if (detection_res->boxes[i].class_num == with_mask) {
        //    if (detection_res->boxes[i].score <= OD_MIN_PERSON_SCORE)
        //        continue;
        //} else {
        //    if (detection_res->boxes[i].score <= OD_MIN_OTHER_CLASS_SCORE)
        //        continue;
        //}

        x = detection_res->boxes[i].x1;
        y = detection_res->boxes[i].y1;
        w = detection_res->boxes[i].x2 - x;
        h = detection_res->boxes[i].y2 - y;

        box_ratio = (w * h) * scale;

        if (box_ratio == 0.0f) {
            continue;
        }

        //if (box_ratio > OD_BBOX_RATIO && detection_res->boxes[i].class_num == person)
        //    continue;

        if (detection_res->boxes[i].class_num == face)
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(255,255,0),2);
        else if (detection_res->boxes[i].class_num == with_mask)
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(0,255,0),2);
        else
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(255,0,0),2);

//#ifdef LOG
        printf("[INFO] Box[%d] (x1, y1, x2, y2, score, class) = %f, %f, %f, %f, %f, %d\n", \
                i, detection_res->boxes[i].x1, detection_res->boxes[i].y1, \
                detection_res->boxes[i].x2, detection_res->boxes[i].y2, \
                detection_res->boxes[i].score, detection_res->boxes[i].class_num);
//#endif
    }

    imshow("Camera output", frames.front());
    vector<cv::Mat>::iterator first_frame = frames.begin();
    frames.erase(first_frame);
    if (cv::waitKey(30) >= 0) {
        // call kdp_end_isi to end isi mode
        kdp_end_isi(dev_idx);
        return 1;
    }
    return 0;
}

int user_test_cam_ssd_fd(int dev_idx, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    int ret_dis = 0;

    if (1) {
        uint16_t width  = 640;
        uint16_t height = 480;
        uint32_t format = 0x80000060;
        // async (host/scpu) + parallel (scpu/ncpu)
        //uint32_t format = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565 | IMAGE_FORMAT_CHANGE_ASPECT_RATIO | IMAGE_FORMAT_PARALLEL_PROC;
        uint32_t image_buf_size = 0;

        // Flash the firmware code with companion mode for object detection of 8 classes !!!
        printf("starting [companion mode] for object detection of 8 classes ...\n");
        int ret = kdp_start_isi_mode(dev_idx, ODTY_APP_ID, ODTY_RESULT_SIZE,
                width, height, format, &error_code, &image_buf_size);
        if (ret != 0) {
            printf("could not set to companion mode: %d ..\n", ret);
            return -1;
        }

        printf("Companion mode succeeded...\n");
        usleep(SLEEP_TIME);
    }

    if (1) {
        printf("starting companion inference ...\n");
        uint32_t img_id = 1234;
        uint32_t img_left = 12;
        uint32_t result_size = 0;
        uint32_t buf_len = ODTY_IMG_SIZE;
        uint32_t buffer_depth = 0;
        char inf_res[ODTY_RESULT_SIZE];

        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            printf("Unable to connect to camera.\n");
            return 1;
        }
        cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
        cv::Mat frame_input;
        int loop1 = 0;
        double start;
        float fps = 0;
        
        start = what_time_is_it_now();
        // fill up the image buffers
        while (!check_ctl_break()) {
            if (img_id & 1) {
                cap >> frame_input;
                cvtColor(frame_input, frame_input, CV_BGR2BGR565);
                cv::flip(frame_input, frame_input, 1);
                frames.push_back(frame_input);
#if CV_MAJOR_VERSION > 3 || (CV_MAJOR_VERSION == 3 && CV_SUBMINOR_VERSION >= 9)
                IplImage ipl_img = cvIplImage(frame_input);
#else
                IplImage ipl_img = (IplImage)frame_input;
#endif
                ret = kdp_isi_inference(dev_idx, (char *)ipl_img.imageData, buf_len, img_id, &error_code, &img_left);
            } else {
                cap >> frame_input;
                cvtColor(frame_input, frame_input, CV_BGR2BGR565);
                cv::flip(frame_input, frame_input, 1);
#if CV_MAJOR_VERSION > 3 || (CV_MAJOR_VERSION == 3 && CV_SUBMINOR_VERSION >= 9)
                IplImage ipl_img = cvIplImage(frame_input);
#else
                IplImage ipl_img = (IplImage)frame_input;
#endif
                frames.push_back(frame_input);
                ret = kdp_isi_inference(dev_idx, (char *)ipl_img.imageData, buf_len, img_id, &error_code, &img_left);
            }
            if (ret) {
                printf("Companion inference failed : %d\n", ret);
                return -1;
            }
            else {
                //printf("Companion inference : [%d] [%d]\n", error_code, img_left);
                if (error_code == 0) {
                    img_id++;
                    buffer_depth++;
                    if (img_left == 0)
                        break;
                }
            }
        }
        if (check_ctl_break())
            return 0;
        printf("Companion image buffer depth = %d\n", buffer_depth);

        // start the inference loop
        uint32_t loop = test_loop - buffer_depth;
        while(loop && !check_ctl_break()) {  // get previous inference results and send in one more image
            memset(inf_res, 0, 8);  // initialize results data
            ret = kdp_isi_retrieve_res(dev_idx, img_id-buffer_depth, &error_code, &result_size, inf_res);
            loop1++;
            if (img_id & 1) {
                ret_dis = display_detection(dev_idx, inf_res);
                if(ret_dis) return 0;

                if (loop1 == 100) {
                    fps = 1./((what_time_is_it_now() - start)/loop1);
                    printf("[INFO] average time on 100 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);
                }
                
                cap >> frame_input;
                cvtColor(frame_input, frame_input, CV_BGR2BGR565);
                cv::flip(frame_input, frame_input, 1);
                frames.push_back(frame_input);
#if CV_MAJOR_VERSION > 3 || (CV_MAJOR_VERSION == 3 && CV_SUBMINOR_VERSION >= 9)
                IplImage ipl_img = cvIplImage(frame_input);
#else
                IplImage ipl_img = (IplImage)frame_input;
#endif
                ret = kdp_isi_inference(dev_idx, ipl_img.imageData, buf_len, img_id, &error_code, &img_left);
            } else {
                ret_dis = display_detection(dev_idx, inf_res);
                if(ret_dis) return 0;

                if (loop1 == 100) {
                    fps = 1./((what_time_is_it_now() - start)/loop1);
                    printf("[INFO] average time on 100 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);
                }

                cap >> frame_input;
                cvtColor(frame_input, frame_input, CV_BGR2BGR565);
                cv::flip(frame_input, frame_input, 1);
                frames.push_back(frame_input);
#if CV_MAJOR_VERSION > 3 || (CV_MAJOR_VERSION == 3 && CV_SUBMINOR_VERSION >= 9)
                IplImage ipl_img = cvIplImage(frame_input);
#else
                IplImage ipl_img = (IplImage)frame_input;
#endif
                ret = kdp_isi_inference(dev_idx, ipl_img.imageData, buf_len, img_id, &error_code, &img_left);
            }

            img_id++;
            loop--;
        }
        if (check_ctl_break())
            return 0;
        // get the results from images remaining in the buffers
        while (buffer_depth && !check_ctl_break()) {
            memset(inf_res, 0, 8);  // initialize results data
            ret = kdp_isi_retrieve_res(dev_idx, img_id-buffer_depth, &error_code, &result_size, inf_res);
            display_detection(dev_idx, inf_res);
            buffer_depth--;
        }
        if (check_ctl_break())
            return 0;
        // call kdp_end_isi to end isi mode after finishing the inference for all frames
        kdp_end_isi(dev_idx);
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    //object detection test
    user_test_cam_ssd_fd(dev_idx, 1000);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
