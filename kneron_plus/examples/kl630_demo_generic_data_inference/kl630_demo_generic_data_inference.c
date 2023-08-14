/**
 * @file        kl720_demo_generic_image_inference.c
 * @brief       main code of generic image inference (output raw data) for multiple input
 * @version     0.1
 * @date        2022-12-14
 *
 * @copyright   Copyright (c) 2022 Kneron Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "kp_core.h"
#include "kp_inference.h"
#include "helper_functions.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

static char _scpu_fw_path[128] = "../../res/firmware/KL630/kp_firmware.tar";
static char _model_file_path[128] = "../../res/models/KL630/YoloV5s_640_640_3/models_630.nef";
static char _image_file_path[128] = "../../res/images/people_talk_in_street_640x640.bmp";
static int _loop = 10;

static kp_devices_list_t *_device_list;
static kp_device_group_t _device;
static kp_model_nef_descriptor_t _model_desc;
static kp_generic_data_inference_desc_t _input_data;
static kp_generic_data_inference_result_header_t _output_desc;

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
    uint32_t raw_buf_size = _model_desc.models[0].max_raw_out_size;
    uint8_t *raw_output_buf = (uint8_t *)malloc(raw_buf_size);

    _input_data.model_id = _model_desc.models[0].id;    // first model ID
    _input_data.inference_number = 0;                   // inference number, used to verify with output result
    _input_data.num_input_node_data = 1;                // number of data

    /******* prepare the pre-processed data for NPU inference *******/
    uint8_t *img_buf;
    int img_buf_size;
    int img_width;
    int img_height;
    int img_channel = 4;

    float *pre_processing_buf;
    int32_t pre_processing_buf_size;

    int8_t *re_layout_buf;
    int32_t re_layout_buf_size;

    // read data file
    img_buf = (uint8_t *)helper_bmp_file_to_raw_buffer(_image_file_path, &img_width, &img_height, KP_IMAGE_FORMAT_RGBA8888);
    img_buf_size = img_width * img_height * img_channel;
    printf("read image ... %s\n", (img_buf) ? "OK" : "failed");

    // get model input radix, scale
    kp_tensor_descriptor_t *input_node_0 = &(_model_desc.models[0].input_nodes[0]);
    int32_t model_input_radix = input_node_0->quantization_parameters.quantized_fixed_point_descriptor[0].radix;
    float model_input_scale = input_node_0->quantization_parameters.quantized_fixed_point_descriptor[0].scale;

    // get model input data layout
    uint32_t model_input_data_layout = input_node_0->data_layout;

    // get model input size (shape order: BxCxHxW)
    uint32_t model_input_channel = input_node_0->shape_npu[1];
    uint32_t model_input_height = input_node_0->shape_npu[2];
    uint32_t model_input_width = input_node_0->shape_npu[3];

    // do normalization - this model is trained with normalize method: (data - 128) / 256
    pre_processing_buf_size = img_buf_size * sizeof(float);
    pre_processing_buf = (float *)malloc(pre_processing_buf_size);

    for (int i = 0; i < img_buf_size; i++) {
        pre_processing_buf[i] = ((float)img_buf[i] - 128.0f) / 256.0f;
    }

    free(img_buf);

    // toolchain calculate the radix value from input data (after normalization), and set it into NEF model.
    // NPU will divide input data "2^radix" automatically, so, we have to scaling the input data here due to this reason.
    float quantization_factor = powf(2, model_input_radix) * model_input_scale;

    for (int i = 0; i < img_buf_size; i++) {
        pre_processing_buf[i] = roundf(pre_processing_buf[i] * quantization_factor);
        pre_processing_buf[i] = MAX(-128.0f, MIN(pre_processing_buf[i], 127.0f));
    }

    // re-layout the data to fit NPU data layout format
    // KL720 supported NPU input layout format: 4W4C8B, 1W16C8B, 16W1C8B
    uint32_t width_align_base;
    uint32_t channel_align_base;

    if (KP_MODEL_TENSOR_DATA_LAYOUT_4W4C8B == model_input_data_layout) {
        width_align_base = 4;
        channel_align_base = 4;
    } else if (KP_MODEL_TENSOR_DATA_LAYOUT_1W16C8B == model_input_data_layout) {
        width_align_base = 1;
        channel_align_base = 16;
    } else if (KP_MODEL_TENSOR_DATA_LAYOUT_16W1C8B == model_input_data_layout) {
        width_align_base = 16;
        channel_align_base = 1;
    } else {
        printf("invalid input NPU layout format %s", helper_kp_model_tensor_data_layout_to_string(model_input_data_layout));
        exit(0);
    }

    // calculate width alignment size, channel block count
    uint32_t model_input_width_align = width_align_base * ceil(model_input_width / (float)width_align_base);
    uint32_t model_input_channel_block_num = ceil(model_input_channel / (float)channel_align_base);

    // create re-layout data container
    // KL720 NPU hardware dimension order: CxHxW
    re_layout_buf_size = model_input_channel_block_num * model_input_height *  model_input_width_align * channel_align_base * sizeof(int8_t);
    re_layout_buf = (int8_t *)malloc(re_layout_buf_size);

    // fill data in re-layout data container
    // (for more information, please refer: https://doc.kneron.com/docs/#plus_c/appendix/supported_npu_data_layout_format/)
    uint32_t dst_offset = 0;
    uint32_t src_offset = 0;
    uint32_t model_input_channel_block_idx = 0;
    for (int h = 0; h < model_input_height; h++) {
        for (int w = 0; w < model_input_width; w++) {
            for (int c = 0; c < model_input_channel; c++) {
                model_input_channel_block_idx = c / channel_align_base;

                dst_offset = model_input_channel_block_idx * (model_input_height *  model_input_width_align * channel_align_base) +
                             h * (model_input_width_align * channel_align_base) +
                             w * (channel_align_base) +
                             c % channel_align_base;
                src_offset = h * (img_width * img_channel) +
                             w * (img_channel) +
                             c;

                re_layout_buf[dst_offset] = (int8_t)pre_processing_buf[src_offset];
            }
        }
    }

    free(pre_processing_buf);

    /******* set up the input descriptor *******/
    _input_data.input_node_data_list[0].buffer_size = re_layout_buf_size;
    _input_data.input_node_data_list[0].buffer = (uint8_t *)re_layout_buf;

    printf("\nstarting inference loop %d times:\n", _loop);

    /******* starting inference work *******/
    for (int i = 0; i < _loop; i++)
    {
        ret = kp_generic_data_inference_send(_device, &_input_data);
        if (ret != KP_SUCCESS)
            break;

        ret = kp_generic_data_inference_receive(_device, &_output_desc, raw_output_buf, raw_buf_size);
        if (ret != KP_SUCCESS)
            break;

        printf(".");
        fflush(stdout);
    }

    printf("\n");

    free(re_layout_buf);
    kp_release_model_nef_descriptor(&_model_desc);
    kp_disconnect_devices(_device);

    if (ret != KP_SUCCESS)
    {
        printf("\ninference failed, error = %d (%s)\n", ret, kp_error_string(ret));
        return -1;
    }

    printf("\ninference loop is done.\n");

    kp_inf_float_node_output_t *output_nodes[3] = {NULL}; // yolo v5s outputs only three nodes, described by _output_desc.num_output_node

    // retrieve output nodes in floating point format
    output_nodes[0] = kp_generic_inference_retrieve_float_node(0, raw_output_buf, KP_CHANNEL_ORDERING_CHW);
    output_nodes[1] = kp_generic_inference_retrieve_float_node(1, raw_output_buf, KP_CHANNEL_ORDERING_CHW);
    output_nodes[2] = kp_generic_inference_retrieve_float_node(2, raw_output_buf, KP_CHANNEL_ORDERING_CHW);

    // print and save the content of node data to files
    helper_dump_floating_node_data_to_files(output_nodes, _output_desc.num_output_node, _image_file_path);

    free(output_nodes[0]);
    free(output_nodes[1]);
    free(output_nodes[2]);

    free(raw_output_buf);

    return 0;
}
