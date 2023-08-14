/**
 * @file        kl520_demo_app_yolo_inference.c
 * @brief       main code of yolo inference application - easy version
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kp_core.h"
#include "kp_inference.h"
#include "helper_functions.h"

#include "kl520_demo_customize_inf_single_model_profile.h"

static char _scpu_fw_path[128] = "../../res/firmware/KL520/fw_scpu.bin";
static char _ncpu_fw_path[128] = "../../res/firmware/KL520/fw_ncpu.bin";
static char _model_file_path[128] = "../../res/models/KL520/tiny_yolo_v3/models_520.nef";
static char _image_file_path[128] = "../../res/images/bike_cars_street_224x224.bmp";
static int _loop = 100;

static kp_devices_list_t *_device_list;
static kp_device_group_t _device;
static char *_img_buf;
static int _img_width, _img_height;

int main(int argc, char *argv[])
{

    // each device has a unique port ID, 0 for auto-search
    int port_id = (argc > 1) ? atoi(argv[1]) : 0;

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
    int ret = kp_load_firmware_from_file(_device, _scpu_fw_path, _ncpu_fw_path);
    printf("upload firmware ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* upload model to device *******/
    ret = kp_load_model_from_file(_device, _model_file_path, NULL);
    printf("upload model ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* prepare the image buffer read from file *******/
    // here convert a bmp file to RGB565 format buffer
    _img_buf = helper_bmp_file_to_raw_buffer(_image_file_path, &_img_width, &_img_height, KP_IMAGE_FORMAT_RGB565);
    printf("read image ... %s\n", (_img_buf) ? "OK" : "failed");

    /******* starting inference work *******/

    // turn on profiling
    ret = kp_profile_set_enable(_device, true);
    printf("enable profile ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* prepare input and output header/buffers *******/
    demo_customize_inf_single_model_header_t input_header;
    demo_customize_inf_single_model_result_t output_result;

    input_header.header_stamp.job_id = DEMO_KL520_CUSTOMIZE_INF_SINGLE_MODEL_JOB_ID;
    input_header.header_stamp.total_image = 1;
    input_header.header_stamp.image_index = 0;
    input_header.width = _img_width;
    input_header.height = _img_height;

    int header_size = sizeof(demo_customize_inf_single_model_header_t);
    int image_size = _img_width * _img_height * 2; // RGB565
    int result_size = sizeof(demo_customize_inf_single_model_result_t);
    int recv_size = 0;

    printf("\nstarting inference loop %d times:\n", _loop);

    for (int i = 0; i < _loop; i++)
    {
        ret = kp_customized_inference_send(_device, (void *)&input_header, header_size, (uint8_t *)_img_buf, image_size);
        if (ret != KP_SUCCESS)
            break;

        ret = kp_customized_inference_receive(_device, (void *)&output_result, result_size, &recv_size);
        if (ret != KP_SUCCESS)
            break;

        printf(".");
        fflush(stdout);
    }
    printf("\n");

    if (ret != KP_SUCCESS)
    {
        printf("\ninference failed, error = %d (%s)\n", ret, kp_error_string(ret));
        return -1;
    }

    printf("inference loop completed.\n");

    // collect profiling statistics
    kp_profile_data_t profile_data;
    kp_profile_get_statistics(_device, &profile_data);

    printf("\n[profile]\n");
    printf("number of models: %d\n", profile_data.num_model_profiled);
    for (int i = 0; i < profile_data.num_model_profiled; i++)
    {
        printf("    - model_id = %d\n", profile_data.model_st[i].model_id);
        printf("    - inf_count = %d\n", profile_data.model_st[i].inf_count);
        printf("    - cpu_op_count = %d\n", profile_data.model_st[i].cpu_op_count);
        printf("    - avg_pre_process_ms = %.2f\n", profile_data.model_st[i].avg_pre_process_ms);
        printf("    - avg_inference_ms = %.2f\n", profile_data.model_st[i].avg_inference_ms);
        printf("    - avg_cpu_op_ms = %.2f\n", profile_data.model_st[i].avg_cpu_op_ms);
        printf("    - avg_cpu_op_per_cpu_node_ms = %.2f\n", profile_data.model_st[i].avg_cpu_op_per_cpu_node_ms);
        printf("    - avg_post_process_ms = %.2f\n", profile_data.model_st[i].avg_post_process_ms);
        printf("\n");
    }

    // turn off profiling
    kp_profile_set_enable(_device, false);

    free(_img_buf);
    kp_disconnect_devices(_device);

    // borrow kp_yolo_result_t to print output
    helper_print_yolo_box_on_bmp((kp_yolo_result_t *)&output_result.yolo_result, _image_file_path);

    return 0;
}
