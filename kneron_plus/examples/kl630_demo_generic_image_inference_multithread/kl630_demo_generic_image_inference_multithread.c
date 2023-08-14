/**
 * @file        kl520_demo_generic_inference_multithread.c
 * @brief       main code of generic inference under multithread
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "kp_core.h"
#include "legacy/kp_inference_v1.h"
#include "helper_functions.h"

static char _scpu_fw_path[128] = "../../res/firmware/KL630/kp_firmware.tar";
static char _model_file_path[128] = "../../res/models/KL630/YoloV5s_640_640_3/models_630.nef";
static char _image_file_path[128] = "../../res/images/one_bike_many_cars_608x608.bmp";
static int _loop = 10;

static kp_devices_list_t *_device_list;
static kp_device_group_t _device;
static kp_model_nef_descriptor_t _model_desc;
static kp_generic_image_inference_desc_t _input_data;
static kp_generic_image_inference_result_header_t _output_desc;
static uint32_t _raw_buf_size = 0;
static uint8_t *_raw_output_buf = NULL;
static char *_img_buf;
static int _img_width, _img_height;

void *image_send_function(void *data)
{
    for (int i = 0; i < _loop; i++)
    {
        int ret = kp_generic_image_inference_send(_device, &_input_data);
        if (ret != KP_SUCCESS)
        {
            printf("kp_generic_raw_inference_send() error = %d (%s)\n", ret, kp_error_string(ret));
            exit(0);
        }
    }
    return NULL;
}

void *result_receive_function(void *data)
{
    // best effort to receive results
    for (int i = 0; i < _loop; i++)
    {
        int ret = kp_generic_image_inference_receive(_device, &_output_desc, _raw_output_buf, _raw_buf_size);
        if (ret != KP_SUCCESS)
        {
            printf("kp_generic_raw_inference_receive() error = %d (%s)\n", ret, kp_error_string(ret));
            exit(0);
        }
        printf(".");
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // each device has a unique port ID, 0 for auto-search
    int port_id = (argc > 1) ? atoi(argv[1]) : 0;
    int ret;

    /******* check the device USB speed *******/
    int link_speed;
    _device_list = kp_scan_devices();

    helper_get_device_usb_speed_by_port_id(_device_list, port_id, &link_speed);
    if (KP_USB_SPEED_HIGH != link_speed)
        printf("[warning] device is not run at high speed.\n");

    /******* connect the device *******/
    _device = kp_connect_devices(1, &port_id, NULL);
    printf("connect device ... %s\n", (_device) ? "OK" : "failed");

    kp_set_timeout(_device, 5000); // 5 secs timeout

    /******* upload firmware to device *******/
    ret = kp_load_firmware_from_file(_device, _scpu_fw_path, NULL);
    printf("upload firmware ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* upload model to device *******/
    ret = kp_load_model_from_file(_device, _model_file_path, &_model_desc);
    printf("upload model ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* allocate memory for raw output *******/
    // by default here select first model
    _raw_buf_size = _model_desc.models[0].max_raw_out_size;
    _raw_output_buf = (uint8_t *)malloc(_raw_buf_size);

    /******* prepare the image buffer read from file *******/
    // here convert a bmp file to RGB565 format buffer
    _img_buf = helper_bmp_file_to_raw_buffer(_image_file_path, &_img_width, &_img_height, KP_IMAGE_FORMAT_RGB565);
    printf("read image ... %s\n", (_img_buf) ? "OK" : "failed");

    /******* set up the input descriptor *******/
    _input_data.model_id = _model_desc.models[0].id;   // first model ID
    _input_data.inference_number = 0;                  // inference number, used to verify with output result
    _input_data.num_input_node_image = 1;

    _input_data.input_node_image_list[0].resize_mode = KP_RESIZE_ENABLE;        // enable resize in pre-process
    _input_data.input_node_image_list[0].padding_mode = KP_PADDING_CORNER;      // enable corner padding in pre-process
    _input_data.input_node_image_list[0].normalize_mode = KP_NORMALIZE_KNERON;  // this depends on models
    _input_data.input_node_image_list[0].image_format = KP_IMAGE_FORMAT_RGB565; // image format
    _input_data.input_node_image_list[0].width = _img_width;                    // image width
    _input_data.input_node_image_list[0].height = _img_height;                  // image height
    _input_data.input_node_image_list[0].crop_count = 0;                        // number of crop area, 0 means no cropping
    _input_data.input_node_image_list[0].image_buffer = (uint8_t *)_img_buf;    // buffer of image data

    /******* starting inference work *******/
    printf("\nstarting inference loop %d times:\n", _loop);

    pthread_t image_send_thd, result_recv_thd;
    double time_spent;

    helper_measure_time_begin();

    pthread_create(&image_send_thd, NULL, image_send_function, NULL);
    pthread_create(&result_recv_thd, NULL, result_receive_function, NULL);

    pthread_join(image_send_thd, NULL);
    pthread_join(result_recv_thd, NULL);

    helper_measure_time_end(&time_spent);

    free(_img_buf);
    kp_release_model_nef_descriptor(&_model_desc);
    kp_disconnect_devices(_device);

    kp_inf_float_node_output_t *output_nodes[3] = {NULL}; // tiny yolo v5 outputs 3 nodes, described by _output_desc.num_output_node

    // retrieve output nodes in floating point format
    output_nodes[0] = kp_generic_inference_retrieve_float_node(0, _raw_output_buf, KP_CHANNEL_ORDERING_CHW);
    output_nodes[1] = kp_generic_inference_retrieve_float_node(1, _raw_output_buf, KP_CHANNEL_ORDERING_CHW);
    output_nodes[2] = kp_generic_inference_retrieve_float_node(2, _raw_output_buf, KP_CHANNEL_ORDERING_CHW);

    // print and save the content of node data to files
    helper_dump_floating_node_data_to_files(output_nodes, _output_desc.num_output_node, _image_file_path);

    free(output_nodes[0]);
    free(output_nodes[1]);
    free(output_nodes[2]);

    free(_raw_output_buf);

    if (ret != KP_SUCCESS)
    {
        printf("\ninference failed, error = %d (%s)\n", ret, kp_error_string(ret));
        return -1;
    }

    printf("\n\ntotal inference %d images\n", _loop);
    printf("time spent: %.2f secs, FPS = %.1f\n", time_spent, _loop / time_spent);

    return 0;
}
