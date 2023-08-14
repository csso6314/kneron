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
#include <sys/time.h>
#include <sys/stat.h>
#include "user_util.h"
#include "kapp_id.h"
#include "model_res.h"
#include "model_type.h"
#include "base.h"
#include "ipc.h"

#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{

using namespace std;
using namespace cv;
#endif

//#define TEST_CHECK_FRAME_TIME

#define ISI_APP_ID          APP_CENTER_APP

#define ISI_RESULT_SIZE     2048

#define ISI_IMG_FORMAT      (IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565 | IMAGE_FORMAT_CHANGE_ASPECT_RATIO)
#define VIDEO_MODES         3

struct video_array {
    int     width;
    int     height;
    int     size;
} img_s[VIDEO_MODES] = {
    {640, 480, 640*480*2},  // VGA
    {800, 600, 800*600*2},   // SVGA
    {1280, 720, 1280*720*2}   // WXGA
};

VideoCapture cap;
vector<Mat> frames;

uint32_t model_id = KNERON_OBJECTDETECTION_CENTERNET_512_512_3;
char caption[80];

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

static int cam_open(int video_mode)
{
    if (!cap.open(0))
    {
        printf("Unable to connect to camera.\n");
        return -1;
    }

    cap.set(CV_CAP_PROP_FRAME_WIDTH, img_s[video_mode].width);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, img_s[video_mode].height);

    return 0;
}

/**
 * capture a new frame from camera
 * cap: video capturing device
 * return frame in IplImage format
 */
static IplImage cam_capture_frame(VideoCapture cap)
{
    Mat frame_input;
    // capture a new frame from camera
    cap >> frame_input;
    // convert frame from BGR to RGB565
    cvtColor(frame_input, frame_input, CV_BGR2BGR565);
    // flip the frame
    flip(frame_input, frame_input, 1);
    // save the frame into vector of frames for displaying
    frames.push_back(frame_input);
    // return frame in IplImage format
#if CV_MAJOR_VERSION > 3 || (CV_MAJOR_VERSION == 3 && CV_SUBMINOR_VERSION >= 9)
    return cvIplImage(frame_input);
#else
    return (IplImage)frame_input;
#endif
}

static string get_local_time(void)
{
    string default_time = "19700101_000000.000";
    try
    {
        struct timeval cur_time;
        gettimeofday(&cur_time, NULL);
        int milli = cur_time.tv_usec / 1000;

        char buffer[80] = {0};
        struct tm now_time;
        time_t tv_sec = cur_time.tv_sec;
        localtime_r(&tv_sec, &now_time);
        strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &now_time);

        char current_time[84] = {0};
        snprintf(current_time, sizeof(current_time), "%s.%03d", buffer, milli);

        return current_time;
    }
    catch(const std::exception& e)
    {
        return default_time;
    }
    catch (...)
    {
        return default_time;
    }
}

static void create_dir(const char* dir_name)
{
    if (0 != access(dir_name, F_OK))
    {
#ifdef __linux__
        int flag = mkdir(dir_name, 0777);
#elif _WIN32
        int flag = mkdir(dir_name);
#endif
        if (flag < 0)
        {
            printf("create directory: %s failed!\n", dir_name);
        }
    }
}

static int handle_key_pressed(char key, Mat front_frame_raw, Mat front_frame_bb)
{
    int ret = 0;
    char image[84] = {0}; // raw image
    char image_bb[84] = {0}; // image with bounding box
    const char SAVE_IMAGE_DIR[] = "kl720_cam_isi_center_app_imgs";

    switch (key)
    {
    case 's':
    case 'S':
    case 'b':
        create_dir(SAVE_IMAGE_DIR);
        string now = get_local_time();
        snprintf(image, sizeof(image), "%s/%s.jpg", SAVE_IMAGE_DIR, now.c_str());
        snprintf(image_bb, sizeof(image_bb), "%s/%s_bb.jpg", SAVE_IMAGE_DIR, now.c_str());
        break;
    }

    switch (key)
    {
    case 's': // save raw image
        cvtColor(front_frame_raw, front_frame_raw, CV_BGR5652BGR);
        imwrite(image, front_frame_raw);
        break;
    case 'S': // save image with bounding box
        imwrite(image_bb, front_frame_bb);
        break;
    case 'b': // save both raw image and image with bounding box
        cvtColor(front_frame_raw, front_frame_raw, CV_BGR5652BGR);
        imwrite(image, front_frame_raw);
        imwrite(image_bb, front_frame_bb);
        break;
    default: // exit
        ret = 1;
        break;
    }
    return ret;
}

static int cam_show_detection(int img_id, char* inf_res)
{
    // parse the detection result
    dme_res* detection_res = (dme_res*)inf_res;
    uint32_t box_count = detection_res->box_count;
    struct bounding_box_s* box;
    uint32_t i;
    Mat front_frame_raw = frames.front();
    Mat front_frame = frames.front();
    Scalar color;

    printf("image %d -> %d object(s)\n", img_id, box_count);

    // draw detection on the frame
    cvtColor(front_frame, front_frame, CV_BGR5652BGR);
    box = &detection_res->boxes[0];
    for (i = 0; i < box_count; i++) {
        if (box->class_num == 0 /*person*/)
            color = Scalar(255, 0, 0);
        else
            color = Scalar(255, 255, 0);
        rectangle(front_frame, Point(box->x1, box->y1), Point(box->x2, box->y2), color, 2);
        printf("  [%d] {%d: %f}: (%d, %d) (%d, %d)\n", i, box->class_num, box->score,
                    (int)box->x1, (int)box->y1, (int)box->x2, (int)box->y2);
        box++;
    }

    // show detection in window
    imshow(caption, front_frame);

    // delete the first frame after showing detection
    vector<Mat>::iterator first_frame = frames.begin();
    frames.erase(first_frame);
    // exit when pressing any key
    char key = (char) waitKey(1);
    if (key >= 0) {
        return handle_key_pressed(key, front_frame_raw, front_frame);
    }
    return 0;
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

    if (*r_size >= sizeof(uint32_t))
    {
#ifdef TEST_CHECK_FRAME_TIME
        do_check_frame_time();
#endif

        switch (model_id) {
        case KNERON_OBJECTDETECTION_CENTERNET_512_512_3:
        case TINY_YOLO_V3_416_416_3:
        case TINY_YOLO_V3_608_608_3:
        case YOLO_V3_416_416_3:
        case YOLO_V3_608_608_3:
        case YOLO_V4_416_416_3:
        case KNERON_YOLOV5S_640_640_3:
        case KNERON_FD_MBSSD_200_200_3:
        case CUSTOMER_MODEL_1:
        case CUSTOMER_MODEL_2:
            ret = cam_show_detection(img_id, r_data);
            break;

        default:
            printf("Wrong model ID %d with size %d\n", model_id , *r_size);
            break;
        }
    }
    else
    {
        printf("Img [%d] : result_size %d too small\n", img_id, *r_size);
        return -1;
    }

    return ret;
}

static int user_test_centernet(int dev_idx, int video_mode, uint32_t test_loop)
{
    int ret = 0;
    uint32_t error_code = 0;
    uint32_t image_buf_size = 0;

    if (cam_open(video_mode))
        return -1;

    sprintf(caption, "Center App (%d x %d)", img_s[video_mode].width, img_s[video_mode].height);

    if (1)
    {
        uint16_t width = img_s[video_mode].width;
        uint16_t height = img_s[video_mode].height;
        uint32_t format = ISI_IMG_FORMAT;

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
            model_id = KNERON_OBJECTDETECTION_CENTERNET_512_512_3;
            return -1;
        }

        printf("ISI model %d succeeded (window = %d)...\n", model_id, image_buf_size);
        usleep(SLEEP_TIME);
    }

    if (1)
    {
        printf("starting ISI inference ...\n");
        uint32_t img_id_tx = 1;
        uint32_t img_id_rx = img_id_tx;
        uint32_t img_left = 12;
        uint32_t result_size = 0;
        uint32_t buf_len = img_s[video_mode].size;
        char inf_res[ISI_RESULT_SIZE];
        IplImage ipl_img;

        double start_time = what_time_is_it_now();

        uint32_t loop = test_loop;

        while (1)
        {
            ipl_img = cam_capture_frame(cap);

            ret = do_inference(dev_idx, (char*)ipl_img.imageData, buf_len, img_id_tx, &error_code, &img_left);
            if (ret)
                return ret;
            img_id_tx++;

            ret = do_get_result(dev_idx, img_id_rx, &error_code, &result_size, inf_res);
            if (ret) {
                printf("Stopped.\n");
                break;
            }
            img_id_rx++;

            loop--;
            if (loop == 0)
                break;

            if (check_ctl_break()) {
                printf("Ctrl-C received. Exit.\n");
                break;
            }
        }

        if (1)
        {
            double end_time = what_time_is_it_now();
            double elapsed_time, avg_elapsed_time, avg_fps;
            uint32_t actual_loop;

            if (test_loop)
                actual_loop = test_loop - loop;
            else
                actual_loop = ((uint32_t)(-1) - loop) + 1;
            elapsed_time = (end_time - start_time) * 1000;
            avg_elapsed_time = elapsed_time / actual_loop;
            avg_fps = 1000.0f / avg_elapsed_time;

            printf("\n=> Avg %.2f FPS (%.2f ms = %.2f/%d)\n\n",
                    avg_fps, avg_elapsed_time, elapsed_time, test_loop);
        }
    }
    return 0;
}

static void usage(char *name)
{
    printf("\n");
    printf("usage: ./%s [video mode] [run_frames] [model_id]\n", name);
    printf("\n[video mode] 0: vga (640x480), 1: svga (800x600), 2: wxga (1280x720)\n");
    printf("[run_frames] number of frames to run, 0 = forever\n");
    printf("[model_id] model ID (optional. Default is 200 for CenterNet.)\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4) {
        usage(argv[0]);
        return -1;
    }

    int video_mode = atoi(argv[1]);
    if (video_mode >= VIDEO_MODES) {
        usage(argv[0]);
        return -1;
    }
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

    register_hostlib_signal();
    user_test_centernet(dev_idx, video_mode, run_frames);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
