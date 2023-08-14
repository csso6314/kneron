/**
 * @file        kl520_demo_cam_user_define_api_drop_frame.cpp
 * @brief       main code of user define api with camera application
 * @version     1.0
 * @date        2021-06-13
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

extern "C"
{
#include "kp_core.h"
#include "kp_inference.h"
#include "helper_functions.h"
#include "postprocess.h"
}

#include <opencv2/opencv.hpp>
#include <mutex>

static char _scpu_fw_path[128] = "../../res/firmware/KL520/fw_scpu.bin";
static char _ncpu_fw_path[128] = "../../res/firmware/KL520/fw_ncpu.bin";
static char _model_file_path[128] = "../../res/models/KL520/tiny_yolo_v3/models_520.nef";

static bool _receive_running = true;
static std::mutex _mutex_result;
static kp_device_group_t _device;
static kp_model_nef_descriptor_t _model_desc;
static kp_yolo_result_t _yolo_result_latest = {0};
static kp_generic_image_inference_desc_t _input_data;
static kp_generic_image_inference_result_header_t _output_desc;
static int _image_width;
static int _image_height;
static int _cur_result_index = 0;

void *image_send_function(void *data)
{
    static cv::VideoCapture _cv_camera_cap;
    static cv::Mat _cv_img_show;

    double time_spent_sum = 0.0;
    char imgFPS[60] = "image FPS: ";
    char infFPS[60] = "inference FPS: ";
    char strImgRes[60];
    char strModelRes[60];
    char box_info[128];

    // record last result
    static kp_yolo_result_t _yolo_result_last = {0};

    // record stabilized bounding boxes result
    static uint32_t box_count_stabilized;
    static kp_bounding_box_t boxes_stabilized[YOLO_GOOD_BOX_MAX];

    cv::Mat _cv_img_cam;
    cv::Mat _cv_img_rgb565;
    int img_count = 0;
    int result_count = 0;

    /* Open camera on index 0 */
    bool opened = _cv_camera_cap.open(0);
    printf("open camera ... %s\n", (opened) ? "OK" : "failed");

    /* Setting frame size may failed in OpenCV */
    _image_width = static_cast<int>(_cv_camera_cap.get(CV_CAP_PROP_FRAME_WIDTH));
    _image_height = static_cast<int>(_cv_camera_cap.get(CV_CAP_PROP_FRAME_HEIGHT));

    // to print image and model resolution
    sprintf(strImgRes, "image: %d x %d", _image_width, _image_height);
    sprintf(strModelRes, "model: %d x %d", _model_desc.models[0].input_nodes[0].shape_npu[3], _model_desc.models[0].input_nodes[0].shape_npu[2]);

    /******* set up the input descriptor *******/
    _input_data.model_id = _model_desc.models[0].id;    // first model ID
    _input_data.inference_number = 0;                   // inference number, used to verify with output result
    _input_data.num_input_node_image = 1;               // number of image

    _input_data.input_node_image_list[0].resize_mode = KP_RESIZE_ENABLE;        // enable resize in pre-process
    _input_data.input_node_image_list[0].padding_mode = KP_PADDING_CORNER;      // enable corner padding in pre-process
    _input_data.input_node_image_list[0].normalize_mode = KP_NORMALIZE_KNERON;  // this depends on models
    _input_data.input_node_image_list[0].image_format = KP_IMAGE_FORMAT_RGB565; // image format
    _input_data.input_node_image_list[0].width = _image_width;                  // image width
    _input_data.input_node_image_list[0].height = _image_height;                // image height
    _input_data.input_node_image_list[0].crop_count = 0;                        // number of crop area, 0 means no cropping

    /* Prepare display window */
    cv::namedWindow("Generic Inference", CV_WINDOW_AUTOSIZE);

    while (true)
    {
        /* Get one frame from camera */
        _cv_camera_cap.read(_cv_img_cam);

        cv::cvtColor(_cv_img_cam, _cv_img_rgb565, CV_BGR2BGR565); // convert image color fomart to Kneron-specified

        /* Send buffer to generic inference */
        _input_data.input_node_image_list[0].image_buffer = (uint8_t *)_cv_img_rgb565.data;    // buffer of image data
        int ret = kp_generic_image_inference_send(_device, &_input_data);
        if (ret != KP_SUCCESS)
        {
            printf("kp_generic_raw_inference_send() error = %d (%s)\n", ret, kp_error_string(ret));
            break;
        }

        img_count++;

        _mutex_result.lock();

        /* This function is optional to avoid box "jumping" */
        helper_bounding_box_stabilization(_yolo_result_last.box_count, _yolo_result_last.boxes, _yolo_result_latest.box_count, _yolo_result_latest.boxes,
                                          &box_count_stabilized, boxes_stabilized, 20, 0.3);

        /* Draw all bounding boxes */
        for (uint32_t i = 0; i < box_count_stabilized; i++)
        {
            int x1 = boxes_stabilized[i].x1;
            int y1 = boxes_stabilized[i].y1;
            int x2 = boxes_stabilized[i].x2;
            int y2 = boxes_stabilized[i].y2;

            int textX = x1;
            int textY = y1 - 10;
            if (textY < 0)
                textY = y1 + 5;

            sprintf(box_info, "class %d (%.2f)", boxes_stabilized[i].class_num, boxes_stabilized[i].score);

            cv::rectangle(_cv_img_cam, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(50, 255, 50), 2);
            cv::putText(_cv_img_cam, box_info, cv::Point(textX, textY), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(50, 50, 255), 1);
        }

        memcpy((void *)&_yolo_result_last, (void *)&_yolo_result_latest, sizeof(kp_yolo_result_t));
        _mutex_result.unlock();

        /* calculate FPS every 60 frames */
        if (img_count == 1)
        {
            result_count = _cur_result_index;
            helper_measure_time_begin();
        }
        else if (img_count == 61)
        {
            double time_spent;
            helper_measure_time_end(&time_spent);
            sprintf(imgFPS, "image FPS: %.2lf", 60 / time_spent);
            sprintf(infFPS, "inference FPS: %.2lf", (_cur_result_index - result_count) / time_spent);
            img_count = 0;
        }

        cv::putText(_cv_img_cam, imgFPS, cv::Point(5, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(50, 50, 255), 1);
        cv::putText(_cv_img_cam, infFPS, cv::Point(5, 40), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(50, 50, 255), 1);
        cv::putText(_cv_img_cam, strImgRes, cv::Point(5, 60), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(50, 50, 255), 1);
        cv::putText(_cv_img_cam, strModelRes, cv::Point(5, 80), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(50, 50, 255), 1);
        cv::putText(_cv_img_cam, "Press 'ESC' to exit", cv::Point(10, _cv_img_cam.rows - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(255, 255, 255), 2);

        cv::imshow("Generic Inference", _cv_img_cam);

        /* Press 'ESC' to exit */
        if (27 == cv::waitKey(10))
        {
            break;
        }
    }

    _receive_running = false;

    return NULL;
}

void *result_receive_function(void *data)
{
    // tiny yolo v3 outputs only two nodes, described by _output_desc.num_output_node
    kp_inf_float_node_output_t *output_nodes[2] = {NULL};

    /******* allocate memory for raw output *******/
    // by default here select first model
    uint32_t raw_buf_size = _model_desc.models[0].max_raw_out_size;
    uint8_t *raw_output_buf = (uint8_t *)malloc(raw_buf_size);

    while (_receive_running)
    {
        /* Receive one result of generic inference */
        int ret = kp_generic_image_inference_receive(_device, &_output_desc, raw_output_buf, raw_buf_size);

        if (KP_ERROR_USB_TIMEOUT_N7 == ret) {
            continue;
        } else if (KP_SUCCESS != ret) {
            printf("kp_generic_raw_inference_receive() error = %d (%s)\n", ret, kp_error_string(ret));
            break;
        }

        // retrieve output nodes in floating point format in first crop box
        output_nodes[0] = kp_generic_inference_retrieve_float_node(0, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
        output_nodes[1] = kp_generic_inference_retrieve_float_node(1, raw_output_buf, KP_CHANNEL_ORDERING_HCW);

        _mutex_result.lock();

        // post-process yolo v3 output nodes to class/bounding boxes
        post_process_yolo_v3(output_nodes, _output_desc.num_output_node, &_output_desc.pre_proc_info[0], 0.2, &_yolo_result_latest);
        _mutex_result.unlock();

        free(output_nodes[0]);
        free(output_nodes[1]);

        ++_cur_result_index;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    // each device has a unique port ID, 0 for auto-search
    int port_id = (argc > 1) ? atoi(argv[1]) : 0;
    int ret;

    /******* reboot the device *******/
    _device = kp_connect_devices(1, &port_id, NULL);
    printf("connect device ... %s\n", (_device) ? "OK" : "failed");

    kp_set_timeout(_device, 5000); // 5 secs timeout

    /******* upload firmware to device *******/
    ret = kp_load_firmware_from_file(_device, _scpu_fw_path, _ncpu_fw_path);
    printf("upload firmware ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* upload model to device *******/
    ret = kp_load_model_from_file(_device, _model_file_path, &_model_desc);
    printf("upload model ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* configure inference settings (make it frame-droppabe for real-time purpose) *******/
    kp_inf_configuration_t infConf = {.enable_frame_drop = true};
    ret = kp_inference_configure(_device, &infConf);
    printf("configure inference frame-droppable ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    pthread_t image_send_thd, result_recv_thd;

    /* Create send image thread and receive result thread */
    pthread_create(&image_send_thd, NULL, image_send_function, NULL);
    pthread_create(&result_recv_thd, NULL, result_receive_function, NULL);

    pthread_join(image_send_thd, NULL);
    pthread_join(result_recv_thd, NULL);

    printf("\ndisconnecting device ...\n");

    kp_release_model_nef_descriptor(&_model_desc);
    kp_disconnect_devices(_device);

    return 0;
}
