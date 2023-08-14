/**
 * @file      kl520_cam_dme_serial_yolo.cpp
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

#define DME_MODEL_FILE      ("../../input_models/KL520/tiny_yolo_v3/models_520.nef")
// model size, initialize to a value larger than the real model size
#define DME_MODEL_SIZE      (20 * 1024 * 1024)
// firmware information size, initialize to a value larger than the real fw info size
#define DME_FWINFO_SIZE     512
#define IMG_SOURCE_W        640
#define IMG_SOURCE_H        480
#define ODTY_IMG_SIZE       (IMG_SOURCE_W * IMG_SOURCE_H * 2)
// the threshold for postprocess. If not set, use the default threshold in Kneron device
#define TY_MIN_SCORE        (0.2f)

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
 * return 0 on succeed, 1 on exit
 */
int display_detection(int dev_idx, char *inf_res) {
    // parse the detection result
    dme_res *detection_res = (dme_res *)inf_res;

#ifdef LOG
    printf("Total boxes %d, total class %d\n", detection_res->box_count, detection_res->class_count);
#endif
    // draw detection on the frame
    cvtColor(frames.front(), frames.front(), CV_BGR5652BGR);
    for (uint32_t i = 0; i < detection_res->box_count; i++) {
        if (check_ctl_break())
            return 1;
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
    // exit when pressing any key
    if (cv::waitKey(1) >= 0) {
        // call kdp_end_dme to end dme mode
        kdp_end_dme(dev_idx);
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
 * example for dme serial yolo with camera inputs and extra parameter of threshold to postprocess
 */
int user_test_dme_yolo(int dev_idx, int user_id, int mode, struct kdp_dme_cfg_s dme_cfg, uint32_t test_loop)
{
    int ret = 0;
    uint32_t ret_model_id = 0;

    /* start of reading fw info and model data, calling kdp_start_dme_ext API to send the data to Kneron device
       via USB and start DME mode */
    if (1) {
        printf("reading model NEF file from '%s'\n", DME_MODEL_FILE);

        long model_size;
        char *model_buf = read_file_to_buffer_auto_malloc(DME_MODEL_FILE, &model_size);
        if(model_buf == NULL)
            return -1;

        printf("starting DME inference in [serial mode] ...\n");
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
        ret = kdp_dme_configure(dev_idx, (char *)&dme_cfg, dat_size, &ret_model_id);
        if (ret != 0) {
            printf("could not set to DME configure mode..\n");
            return -1;
        }
        printf("DME configure model[%d] succeeded...\n", ret_model_id);
        usleep(SLEEP_TIME);
    }
    /* end of call kdp_dme_configure to set dme configuration */

    /* call kdp_dme_inference to do inference with the camera inputs in DME serial mode
       call kdp_dme_retrieve_res to retrieve the detection results
       call kdp_end_dme to end dme mode */
    if (1) {
        uint32_t inf_size = 0;
        bool res_flag = true;

        uint32_t buf_len = ODTY_IMG_SIZE;
        char inf_res[2560];

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
        IplImage ipl_img;

        double start;
        float fps = 0;
        int loop = 0;

        printf("starting DME inference ...\n");
        // start time for the first frame
        start = what_time_is_it_now();
        while (test_loop && !check_ctl_break()) {
            // call capture_frame to capture a new frame
            ipl_img = capture_frame(cap);
            // call kdp_dme_inference to do inference for each frame
            ret = kdp_dme_inference(dev_idx, (char *)ipl_img.imageData, buf_len, &inf_size, &res_flag, (char*) &inf_res, 0, 0);
            if (ret != 0) {
                printf("Inference failed..\n");
                return -1;
            }

            // call kdp_dme_retrieve_res to retrieve the detection results for each frame
            kdp_dme_retrieve_res(dev_idx, 0, inf_size, (char*) &inf_res);

            // display the detection in window
            ret = display_detection(dev_idx, inf_res);
            if(ret) return 0;

            loop++;
            test_loop--;

            // calculate the FPS based on the time for 100 frames
            if (loop == 100) {
                fps = 1./((what_time_is_it_now() - start)/loop);
                printf("[INFO] average time on 100 frames: %f ms/frame, fps: %f\n", (1/fps)*1000, fps);
            }
        }
        printf("DME inference succeeded...\n");

        // call kdp_end_dme to end dme mode after finishing the inference for all frames
        kdp_end_dme(dev_idx);
        usleep(SLEEP_TIME);
    }
    return 0;
}

int user_test(int dev_idx, int user_id)
{
    // create kdp_dme_cfg_s struct by calling the function create_dme_cfg_struct
    struct kdp_dme_cfg_s dme_cfg = create_dme_cfg_struct();

    /* start of dme configuration */
    dme_cfg.model_id     = TINY_YOLO_V3_224_224_3;      // the first model id when compiling in toolchain
    dme_cfg.output_num   = 2;                 // number of output node for the last model
    dme_cfg.image_col    = IMG_SOURCE_W;      // width of image inputs
    dme_cfg.image_row    = IMG_SOURCE_H;      // height of image inputs
    dme_cfg.image_ch     = 3;                 // channel of image inputs
    // image format flags
    // IMAGE_FORMAT_SUB128: subtract 128 for R/G/B value in Kneron device
    // NPU_FORMAT_RGB565: image input is RGB565
    dme_cfg.image_format = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565;
    dme_cfg.ext_param[0] = TY_MIN_SCORE;      // the threshold for postprocess in Kneron device
    /* end of dme configuration */

    // Serial mode (0)
    uint16_t mode = 0;

    // dme test
    user_test_dme_yolo(dev_idx, user_id, mode, dme_cfg, 200);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
