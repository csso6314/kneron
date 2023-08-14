/**
 * @file        kl520_demo_user_define_api.c
 * @brief       main code of user define api with post processing config
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
#include "kp_app_yolo.h"
#include "helper_functions.h"

static char _scpu_fw_path[128] = "../../res/firmware/KL520/fw_scpu.bin";
static char _ncpu_fw_path[128] = "../../res/firmware/KL520/fw_ncpu.bin";

/* This example must use this model. Please do not change the model file. */
/* If you wish to inference with other models, please use Generic Inference or Customized Inference instead */
/* For Generic Inference, please refer http://doc.kneron.com/docs/#plus/introduction/run_examples/#21-generic-inference-example */
/* For Customized Inference, please refer http://doc.kneron.com/docs/#plus/customized_api/introduction/ */
static char _model_file_path[128] = "../../res/models/KL520/tiny_yolo_v3/models_520.nef";

static char _image_file_path[128] = "../../res/images/bike_cars_street_224x224.bmp";
static int _loop = 100;

static kp_devices_list_t *_device_list;
static kp_device_group_t _device;
static kp_app_yolo_result_t _yolo_result;
static char *_img_buf;
static int _img_width, _img_height;

int main(int argc, char *argv[])
{
    // each device has a unique port ID, 0 for auto-search
    int port_id = (argc > 1) ? atoi(argv[1]) : 0;
    int ret;
    kp_model_nef_descriptor_t model_desc;
    kp_app_yolo_post_proc_config_t org_pp_params;
    kp_app_yolo_post_proc_config_t new_pp_params;

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
    ret = kp_load_firmware_from_file(_device, _scpu_fw_path, _ncpu_fw_path);
    printf("upload firmware ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* upload model to device *******/
    ret = kp_load_model_from_file(_device, _model_file_path, &model_desc);
    printf("upload model ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* modify post-processing parameters *******/
    int model_id = model_desc.models[0].id; // assume that first model in the NEF is target model
    ret = kp_app_yolo_get_post_proc_parameters(_device, model_id, &org_pp_params);
    printf("get post-process parameters ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    memcpy((void *)&new_pp_params, (void *)&org_pp_params, sizeof(org_pp_params)); // make a copy of post-process parameters

    /* If parameters is not set, firmware will use the default values */
    new_pp_params.prob_thresh = 0.5; // set higher thresh value
    ret = kp_app_yolo_set_post_proc_parameters(_device, model_id, &new_pp_params);
    printf("set post-process parameters ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* prepare the image buffer read from file *******/
    // here convert a bmp file to RGB565 format buffer
    _img_buf = helper_bmp_file_to_raw_buffer(_image_file_path, &_img_width, &_img_height, KP_IMAGE_FORMAT_RGB565);
    printf("read image ... %s\n", (_img_buf) ? "OK" : "failed");

    /******* starting inference work *******/

    printf("\nstarting inference loop %d times:\n", _loop);

    for (int i = 0; i < _loop; i++)
    {
        uint32_t inference_number;

        ret = kp_app_yolo_inference_send(_device, i, (uint8_t *)_img_buf, _img_width, _img_height, KP_IMAGE_FORMAT_RGB565, NULL);
        if (ret != KP_SUCCESS)
            break;

        ret = kp_app_yolo_inference_receive(_device, &inference_number, &_yolo_result);
        if (ret != KP_SUCCESS)
            break;

        printf(".");
        fflush(stdout);
    }
    printf("\n");

    ret = kp_app_yolo_set_post_proc_parameters(_device, model_id, &org_pp_params);
    printf("recover post-process parameters ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    free(_img_buf);
    kp_release_model_nef_descriptor(&model_desc);
    kp_disconnect_devices(_device);

    if (ret != KP_SUCCESS)
    {
        printf("\ninference failed, error = %d (%s)\n", ret, kp_error_string(ret));
        return -1;
    }

    // borrow kp_yolo_result_t to print output
    kp_yolo_result_t *y = (kp_yolo_result_t *)((uintptr_t)&_yolo_result + APP_PADDING_BYTES);
    helper_print_yolo_box_on_bmp(y, _image_file_path);

    return 0;
}
