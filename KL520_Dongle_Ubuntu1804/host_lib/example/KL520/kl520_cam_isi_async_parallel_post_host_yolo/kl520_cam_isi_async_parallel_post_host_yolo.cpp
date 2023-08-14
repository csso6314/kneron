/**
 * @file      kl520_cam_isi_async_parallel_post_host_yolo.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-12-21
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
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
using namespace std;

extern "C" {
uint32_t round_up(uint32_t num);
int post_yolo_v3(int model_id, struct kdp_image_s *image_p);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define IMG_SOURCE_W        640
#define IMG_SOURCE_H        480
#define ISI_IMG_SIZE        (IMG_SOURCE_W * IMG_SOURCE_H * 2)
#define ISI_APP_ID          APP_TINY_YOLO3
// isi initial result memory size in bytes (equal or larger than the real size)
#define ISI_RESULT_SIZE     102400
// To print the dection results
//#define LOG
std::vector<cv::Mat> frames;

// class names
enum {
    PERSON,
    BICYCLE,
    CAR,
    MOTORBIKE,
    AEROPLANE,
    BUS,
    TRAIN,
    TRUCK,
    BOAT,
    TRAFFIC_LIGHT,
    FIRE_HYDRANT,
    STOP_SIGN,
    PARKING_METER,
    BENCH,
    BIRD,
    CAT,
    DOG,
    HORSE,
    SHEEP,
    COW,
    ELEPHANT,
    BEAR,
    ZEBRA,
    GIRAFFE,
    BACKPACK,
    UMBRELLA,
    HANDBAG,
    TIE,
    SUITCASE,
    FRISBEE,
    SKIS,
    SNOWBOARD,
    SPORTS_BALL,
    KITE,
    BASEBALL_BAT,
    BASEBALL_GLOVE,
    SKATEBOARD,
    SURFBOARD,
    TENNIS_RACKET,
    BOTTLE,
    WINE_GLASS,
    CUP,
    FORK,
    KNIFE,
    SPOON,
    BOWL,
    BANANA,
    APPLE,
    SANDWICH,
    ORANGE,
    BROCCOLI,
    CARROT,
    HOT_DOG,
    PIZZA,
    DONUT,
    CAKE,
    CHAIR,
    SOFA,
    POTTEDPLANT,
    BED,
    DININGTABLE,
    TOILET,
    TVMONITOR,
    LAPTOP,
    MOUSE,
    REMOTE,
    KEYBOARD,
    CELL_PHONE,
    MICROWAVE,
    OVEN,
    TOASTER,
    SINK,
    REFRIGERATOR,
    BOOK,
    CLOCK,
    VASE,
    SCISSORS,
    TEDDY_BEAR,
    HAIR_DRIER,
    TOOTHBRUSH,
};

/**
 * parse the inference results and display the detection in Window
 * dev_idx: connected device ID. A host can connect several devices
 * inf_res: contains the returned inference result
 * post_par: parameters for postprocessing in host side
 * return 0 on succeed, 1 on exit
 */
int display_detection(int dev_idx, char *inf_res, struct post_parameter_s post_par) {
    // Prepare for postprocessing      
    int output_num = inf_res[0];
    struct output_node_params *p_node_info;
    int r_len, offset;
    struct yolo_result_s *detection_res = (struct yolo_result_s *)calloc(1, sizeof(dme_yolo_res));
    struct kdp_image_s *image_p = (struct kdp_image_s *)calloc(1, sizeof(struct kdp_image_s));
    offset = sizeof(int) + output_num * sizeof(output_node_params);

    // save the parameters to struct kdp_image_s
    RAW_INPUT_COL(image_p) = post_par.raw_input_col;
    RAW_INPUT_ROW(image_p) = post_par.raw_input_row;
    DIM_INPUT_COL(image_p) = post_par.model_input_row;
    DIM_INPUT_ROW(image_p) = post_par.model_input_row;
    RAW_FORMAT(image_p) = post_par.image_format;
    POSTPROC_RESULT_MEM_ADDR(image_p) = (uint32_t *)detection_res;
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

#ifdef LOG
    printf("Total boxes %d, total class %d\n", detection_res->box_count, detection_res->class_count);
#endif
    // draw detection on the frame
    cvtColor(frames.front(), frames.front(), CV_BGR5652BGR);
    for (uint32_t i = 0; i < detection_res->box_count; i++) {
        if (check_ctl_break())
            return 0;
        if (detection_res->boxes[i].class_num == PERSON)
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(255,255,0),2);
        else
            cv::rectangle(frames.front(), cv::Point(detection_res->boxes[i].x1, detection_res->boxes[i].y1), cv::Point(detection_res->boxes[i].x2, detection_res->boxes[i].y2), cv::Scalar(255,0,0),2);

#ifdef LOG
        printf("[INFO] Box[%d] (x1, y1, x2, y2, score, class) = %f, %f, %f, %f, %f, %d\n", \
                i, detection_res->boxes[i].x1, detection_res->boxes[i].y1, \
                detection_res->boxes[i].x2, detection_res->boxes[i].y2, \
                detection_res->boxes[i].score, detection_res->boxes[i].class_num);
#endif
    }

    // show detection in window
    imshow("Camera output", frames.front());
    vector<cv::Mat>::iterator first_frame = frames.begin();
    // delete the first frame after showing detection
    frames.erase(first_frame);

    free(detection_res);

    // exit when pressing any key
    if (cv::waitKey(1) >= 0) {
        // call kdp_end_isi to end isi mode
        kdp_end_isi(dev_idx);
        return 1;
    }
    return 0;
}

/**
 * capture a new frame from camera
 * cap: video capturing device
 * return frame in IplImage format
 */
static IplImage capture_frame(cv::VideoCapture cap)
{
    cv::Mat frame_input;
    // capture a new frame from camera
    cap >> frame_input;
    // convert frame from BGR to RGB565
    cvtColor(frame_input, frame_input, CV_BGR2BGR565);
    // flip the frame
    cv::flip(frame_input, frame_input, 1);
    // save the frame into vector of frames for displaying
    frames.push_back(frame_input);
    // return frame in IplImage format
#if CV_MAJOR_VERSION > 3 || (CV_MAJOR_VERSION == 3 && CV_SUBMINOR_VERSION >= 9)
    return cvIplImage(frame_input);
#else
    return (IplImage)frame_input;
#endif
}

/**
 * example for isi async parallel yolo with camera inputs
 * isi: image streaming interface; model is in flash
 * async: pipeline between data transfer of host/scpu and inference (pre/npu/post) in ncpu/npu
 * parallel: pipeline between ncpu and npu
 */
int user_test_yolo(int dev_idx, struct post_parameter_s post_par, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    int ret_dis = 0;

    /* start of calling kdp_start_isi_mode API to start ISI mode */
    if (1) {
        uint16_t width  = IMG_SOURCE_W;
        uint16_t height = IMG_SOURCE_H;
        uint32_t format = post_par.image_format;
        uint32_t image_buf_size = 0;

        // Flash the firmware code with companion mode for tiny_yolo_v3 !!!
        printf("starting [ISI mode] for tiny_yolo_v3 ...\n");
        // call kdp_start_isi_mode to start isi mode
        int ret = kdp_start_isi_mode(dev_idx, ISI_APP_ID, ISI_RESULT_SIZE,
                width, height, format, &error_code, &image_buf_size);
        if (ret != 0) {
            printf("could not set to ISI mode: %d ..\n", ret);
            return -1;
        }

        printf("ISI mode succeeded...\n");
        usleep(SLEEP_TIME);
    }

    /* call kdp_isi_inference to do inference with the camera inputs
       call kdp_isi_retrieve_res to retrieve the raw results
       call kdp_end_isi to end isi mode */
    if (1) {
        printf("starting ISI inference ...\n");
        uint32_t img_id = 1234;
        uint32_t img_left = 12;
        uint32_t result_size = 0;
        uint32_t buf_len = ISI_IMG_SIZE;
        uint32_t buffer_depth = 0;
        char inf_res[ISI_RESULT_SIZE];
        // create video capturing device
        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            printf("Unable to connect to camera.\n");
            return 1;
        }
        // set camera frame width and height
        cap.set(CV_CAP_PROP_FRAME_WIDTH,IMG_SOURCE_W);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT,IMG_SOURCE_H);
        int loop1 = 0;
        double start;
        float fps = 0;
        IplImage ipl_img;
        
        // start time for the first frame
        start = what_time_is_it_now();
        // fill up the image buffers
        while (!check_ctl_break()) {
            // call capture_frame to capture a new frame
            ipl_img = capture_frame(cap);
            // call kdp_isi_inference to do inference for each frame
            ret = kdp_isi_inference(dev_idx, (char *)ipl_img.imageData, buf_len, img_id, &error_code, &img_left);

            if (ret) {
                printf("ISI inference failed : %d\n", ret);
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
        printf("ISI image buffer depth = %d\n", buffer_depth);

        // start the inference loop
        uint32_t loop = test_loop - buffer_depth;
        while(loop && !check_ctl_break()) {  // get previous inference results and send in one more image
            memset(inf_res, 0, 8);  // initialize results data
            // call kdp_isi_retrieve_res to retrieve the raw results for each frame
            ret = kdp_isi_retrieve_res(dev_idx, img_id-buffer_depth, &error_code, &result_size, inf_res);
            loop1++;
            // display the detection in window
            ret_dis = display_detection(dev_idx, inf_res, post_par);
            if(ret_dis) return 0;

            // calculate the FPS based on the time for 1000 frames
            if (loop1 == 1000) {
                fps = 1./((what_time_is_it_now() - start)/loop1);
                printf("[INFO] average time on 1000 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);
            }

            // call capture_frame to capture a new frame
            ipl_img = capture_frame(cap);
            // call kdp_isi_inference to do inference for each frame
            ret = kdp_isi_inference(dev_idx, ipl_img.imageData, buf_len, img_id, &error_code, &img_left);

            img_id++;
            loop--;
        }

        // get the results from images remaining in the buffers
        while (buffer_depth && !check_ctl_break()) {
            memset(inf_res, 0, 8);  // initialize results data
            // retrieve the raw results for each frame
            ret = kdp_isi_retrieve_res(dev_idx, img_id-buffer_depth, &error_code, &result_size, inf_res);
            // display the detection in window
            display_detection(dev_idx, inf_res, post_par);
            buffer_depth--;
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
    // image format flags
    // IMAGE_FORMAT_SUB128: subtract 128 for R/G/B value in Kneron device
    // NPU_FORMAT_RGB565: image input is RGB565
    // IMAGE_FORMAT_PARALLEL_PROC: pipeline between ncpu and npu
    // IMAGE_FORMAT_RAW_OUTPUT: get raw data
    post_par.image_format    = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565 | IMAGE_FORMAT_PARALLEL_PROC | IMAGE_FORMAT_RAW_OUTPUT;
    user_test_yolo(dev_idx, post_par, 2000);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
