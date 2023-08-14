/**
 * @file        kl520_demo_app_yolo_debug_inference.c
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

#include "kl520_demo_customize_inf_single_model_debug.h"

static char _scpu_fw_path[128] = "../../res/firmware/KL520/fw_scpu.bin";
static char _ncpu_fw_path[128] = "../../res/firmware/KL520/fw_ncpu.bin";
static char _model_file_path[128] = "../../res/models/KL520/tiny_yolo_v3/models_520.nef";
static char _image_file_path[128] = "../../res/images/one_bike_many_cars_800x800.bmp";

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
    if (_device == NULL)
        return -1;

    kp_set_timeout(_device, 5000); // 5 secs timeout

    /******* upload firmware to device *******/
    int ret = kp_load_firmware_from_file(_device, _scpu_fw_path, _ncpu_fw_path);
    printf("upload firmware ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");
    if (ret != KP_SUCCESS)
        return -1;

    /******* upload model to device *******/
    ret = kp_load_model_from_file(_device, _model_file_path, NULL);
    printf("upload model ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");
    if (ret != KP_SUCCESS)
        return -1;

    /******* prepare the image buffer read from file *******/
    // here convert a bmp file to RGB565 format buffer
    _img_buf = helper_bmp_file_to_raw_buffer(_image_file_path, &_img_width, &_img_height, KP_IMAGE_FORMAT_RGB565);
    printf("read image ... %s\n", (_img_buf) ? "OK" : "failed");
    if (_img_buf == NULL)
        return -1;

    /******* enable inference debug with specified checkpoint flags *******/
    uint32_t checkpoint_flags =
        (KP_DBG_CHECKPOINT_BEFORE_PREPROCESS |
         KP_DBG_CHECKPOINT_AFTER_PREPROCESS |
         KP_DBG_CHECKPOINT_BEFORE_CPU_OP |
         KP_DBG_CHECKPOINT_AFTER_CPU_OP |
         KP_DBG_CHECKPOINT_AFTER_INFERENCE);

    ret = kp_dbg_set_enable_checkpoints(_device, checkpoint_flags, true);
    printf("debug enable checkpoints ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");
    if (ret != KP_SUCCESS)
        return -1;

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

    /******* sned one image for inference *******/
    ret = kp_customized_inference_send(_device, (void *)&input_header, header_size, (uint8_t *)_img_buf, image_size);
    printf("send one image for inference ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");
    if (ret != KP_SUCCESS)
        return -1;

    /******* receive data for enabled checkpoints *******/
    void *checkpoint_buf;
    kp_dbg_checkpoint_data_before_preprocess_t *bf_pre;
    kp_dbg_checkpoint_data_after_preprocess_t *aft_pre;
    kp_dbg_checkpoint_data_before_cpu_op_t *bf_cpu_op;
    kp_dbg_checkpoint_data_after_cpu_op_t *aft_cpu_op;
    kp_dbg_checkpoint_data_after_inference_t *aft_inf;

    /******* receive checkpoint data at before-pre_process stage (original image) *******/
    ret = kp_dbg_receive_checkpoint_data(_device, &checkpoint_buf);
    bf_pre = (kp_dbg_checkpoint_data_before_preprocess_t *)checkpoint_buf;
    if (ret != KP_SUCCESS || bf_pre->checkpoint_tag != KP_DBG_CHECKPOINT_BEFORE_PREPROCESS)
    {
        printf("get before-pre_process data .. failed\n");
        return -1;
    }

    printf("\nbefore-pre_process data:\n");
    printf("image x, y = (%d, %d)\n", bf_pre->img_x, bf_pre->img_y);
    printf("image width, height = (%d, %d)\n", bf_pre->img_width, bf_pre->img_height);
    printf("img_format = 0x%x (kp_image_format_t)\n", bf_pre->img_format);
    printf("inference model ID = %d\n", bf_pre->target_inf_model);
    printf("image data (hex) = 0x %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X ...\n",
           bf_pre->image[0], bf_pre->image[1], bf_pre->image[2],
           bf_pre->image[3], bf_pre->image[4], bf_pre->image[5],
           bf_pre->image[6], bf_pre->image[7], bf_pre->image[8],
           bf_pre->image[9]);

    /******* receive checkpoint data at after-pre_process stage (preprocessed image) *******/
    ret = kp_dbg_receive_checkpoint_data(_device, &checkpoint_buf);
    aft_pre = (kp_dbg_checkpoint_data_after_preprocess_t *)checkpoint_buf;
    if (ret != KP_SUCCESS || aft_pre->checkpoint_tag != KP_DBG_CHECKPOINT_AFTER_PREPROCESS)
    {
        printf("get after-pre_process data .. failed\n");
        return -1;
    }

    printf("\nafter-pre_process data:\n");
    printf("image width, height = (%d, %d)\n", aft_pre->img_width, aft_pre->img_height);
    printf("img_format = 0x%x (kp_image_format_t)\n", aft_pre->img_format);
    printf("inference model ID = %d\n", aft_pre->target_inf_model);
    printf("image data (hex) = 0x %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X ...\n",
           aft_pre->image[0], aft_pre->image[1], aft_pre->image[2],
           aft_pre->image[3], aft_pre->image[4], aft_pre->image[5],
           aft_pre->image[6], aft_pre->image[7], aft_pre->image[8],
           aft_pre->image[9]);

    /******* receive checkpoint data at before-cpu stage (cpu operation) *******/
    ret = kp_dbg_receive_checkpoint_data(_device, &checkpoint_buf);
    bf_cpu_op = (kp_dbg_checkpoint_data_before_cpu_op_t *)checkpoint_buf;
    if (ret != KP_SUCCESS || aft_pre->checkpoint_tag != KP_DBG_CHECKPOINT_BEFORE_CPU_OP)
    {
        printf("get before-cpu data .. failed\n");
        return -1;
    }

    printf("\nbefore-cpu data:\n");
    printf("inference model ID = %d\n", bf_cpu_op->target_inf_model);
    printf("total raw output size = %d bytes\n", bf_cpu_op->total_output_size);
    printf("number of nodes = %d\n", bf_cpu_op->num_nodes);
    for (int i = 0; i < bf_cpu_op->num_nodes; i++)
    {
        printf("    node %d:\n", i);
        printf("    - width = %d\n", bf_cpu_op->node_metadata[i].width);
        printf("    - height = %d\n", bf_cpu_op->node_metadata[i].height);
        printf("    - channel = %d\n", bf_cpu_op->node_metadata[i].channel);
        printf("    - radix = %d\n", bf_cpu_op->node_metadata[i].radix);
        printf("    - scale = %.3f\n", bf_cpu_op->node_metadata[i].scale);
        printf("    - npu data layout = %s\n", helper_kp_model_tensor_data_layout_to_string(bf_cpu_op->node_metadata[i].data_layout));
    }

    printf("raw output (hex) = 0x %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X ...\n",
           bf_cpu_op->raw_output[0], bf_cpu_op->raw_output[1], bf_cpu_op->raw_output[2],
           bf_cpu_op->raw_output[3], bf_cpu_op->raw_output[4], bf_cpu_op->raw_output[5],
           bf_cpu_op->raw_output[6], bf_cpu_op->raw_output[7], bf_cpu_op->raw_output[8],
           bf_cpu_op->raw_output[9]);

    /******* receive checkpoint data at after-cpu stage (cpu operation) *******/
    ret = kp_dbg_receive_checkpoint_data(_device, &checkpoint_buf);
    aft_cpu_op = (kp_dbg_checkpoint_data_after_cpu_op_t *)checkpoint_buf;
    if (ret != KP_SUCCESS || aft_cpu_op->checkpoint_tag != KP_DBG_CHECKPOINT_AFTER_CPU_OP)
    {
        printf("get after-cpu data .. failed\n");
        return -1;
    }

    printf("\nafter-cpu data:\n");
    printf("inference model ID = %d\n", aft_cpu_op->target_inf_model);
    printf("total raw output size = %d bytes\n", aft_cpu_op->total_output_size);
    printf("number of nodes = %d\n", aft_cpu_op->num_nodes);
    for (int i = 0; i < aft_cpu_op->num_nodes; i++)
    {
        printf("    node %d:\n", i);
        printf("    - width = %d\n", aft_cpu_op->node_metadata[i].width);
        printf("    - height = %d\n", aft_cpu_op->node_metadata[i].height);
        printf("    - channel = %d\n", aft_cpu_op->node_metadata[i].channel);
        printf("    - radix = %d\n", aft_cpu_op->node_metadata[i].radix);
        printf("    - scale = %.3f\n", aft_cpu_op->node_metadata[i].scale);
        printf("    - npu data layout = %s\n", helper_kp_model_tensor_data_layout_to_string(aft_cpu_op->node_metadata[i].data_layout));
    }

    printf("raw output (hex) = 0x %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X ...\n",
           aft_cpu_op->raw_output[0], aft_cpu_op->raw_output[1], aft_cpu_op->raw_output[2],
           aft_cpu_op->raw_output[3], aft_cpu_op->raw_output[4], aft_cpu_op->raw_output[5],
           aft_cpu_op->raw_output[6], aft_cpu_op->raw_output[7], aft_cpu_op->raw_output[8],
           aft_cpu_op->raw_output[9]);

    /******* receive checkpoint data at after-inference stage (inference result raw output) *******/
    ret = kp_dbg_receive_checkpoint_data(_device, &checkpoint_buf);
    aft_inf = (kp_dbg_checkpoint_data_after_inference_t *)checkpoint_buf;
    if (ret != KP_SUCCESS || aft_inf->checkpoint_tag != KP_DBG_CHECKPOINT_AFTER_INFERENCE)
    {
        printf("get after-inference data .. failed\n");
        return -1;
    }

    printf("\nafter-inference data:\n");
    printf("inference model ID = %d\n", aft_inf->target_inf_model);
    printf("total raw output size = %d bytes\n", aft_inf->total_output_size);
    printf("number of nodes = %d\n", aft_inf->num_nodes);
    for (int i = 0; i < aft_inf->num_nodes; i++)
    {
        printf("    node %d:\n", i);
        printf("    - width = %d\n", aft_inf->node_metadata[i].width);
        printf("    - height = %d\n", aft_inf->node_metadata[i].height);
        printf("    - channel = %d\n", aft_inf->node_metadata[i].channel);
        printf("    - radix = %d\n", aft_inf->node_metadata[i].radix);
        printf("    - scale = %.3f\n", aft_inf->node_metadata[i].scale);
        printf("    - npu data layout = %s\n", helper_kp_model_tensor_data_layout_to_string(aft_inf->node_metadata[i].data_layout));
    }

    printf("raw output (hex) = 0x %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X ...\n",
           aft_inf->raw_output[0], aft_inf->raw_output[1], aft_inf->raw_output[2],
           aft_inf->raw_output[3], aft_inf->raw_output[4], aft_inf->raw_output[5],
           aft_inf->raw_output[6], aft_inf->raw_output[7], aft_inf->raw_output[8],
           aft_inf->raw_output[9]);

    /******* while checkpoint data done it should receive a END signal  *******/
    ret = kp_dbg_receive_checkpoint_data(_device, &checkpoint_buf);
    printf("got the end of debug loop ... %s\n", (ret == KP_DBG_CHECKPOINT_END_37) ? "OK" : "failed");
    if (ret != KP_DBG_CHECKPOINT_END_37)
        return -1;

    /******* finally receive post-processed inference result  *******/
    ret = kp_customized_inference_receive(_device, (void *)&output_result, result_size, &recv_size);
    printf("receive inference result ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");
    if (ret != KP_SUCCESS)
        return -1;

    kp_dbg_set_enable_checkpoints(_device, checkpoint_flags, false);

    free(_img_buf);
    kp_disconnect_devices(_device);

    return 0;
}
