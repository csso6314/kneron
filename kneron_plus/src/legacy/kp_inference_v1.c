/**
 * @file        kp_inference_v1.c
 * @brief       legacy inference functions
 * @version     2.0
 * @date        2022-05-23
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <string.h>

#include "legacy/kp_inference_v1.h"

int kp_generic_raw_inference_send(kp_device_group_t devices, kp_generic_raw_image_header_t *inf_desc,
                                  uint8_t *image_buffer)
{
    kp_generic_image_inference_desc_t _inference_desc = {0};

    _inference_desc.model_id                = inf_desc->model_id;
    _inference_desc.inference_number        = inf_desc->inference_number;
    _inference_desc.num_input_node_image    = 1;

    _inference_desc.input_node_image_list[0].image_buffer   = image_buffer;
    _inference_desc.input_node_image_list[0].width          = inf_desc->width;
    _inference_desc.input_node_image_list[0].height         = inf_desc->height;
    _inference_desc.input_node_image_list[0].resize_mode    = inf_desc->resize_mode;
    _inference_desc.input_node_image_list[0].padding_mode   = inf_desc->padding_mode;
    _inference_desc.input_node_image_list[0].image_format   = inf_desc->image_format;
    _inference_desc.input_node_image_list[0].normalize_mode = inf_desc->normalize_mode;
    _inference_desc.input_node_image_list[0].crop_count     = inf_desc->crop_count;

    memcpy(_inference_desc.input_node_image_list[0].inf_crop,
           inf_desc->inf_crop,
           inf_desc->crop_count * sizeof(kp_inf_crop_box_t));

    return kp_generic_image_inference_send(devices, &_inference_desc);
}

int kp_generic_raw_inference_receive(kp_device_group_t devices, kp_generic_raw_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size)
{
    kp_generic_image_inference_result_header_t _inference_result_header = {0};

    int ret = kp_generic_image_inference_receive(devices, &_inference_result_header, raw_out_buffer, buf_size);

    if (KP_SUCCESS != ret) 
        goto FUNC_OUT;

    output_desc->inference_number = _inference_result_header.inference_number;
    output_desc->crop_number = _inference_result_header.crop_number;
    output_desc->num_output_node = _inference_result_header.num_output_node;
    output_desc->product_id = _inference_result_header.product_id;

    memcpy(&(output_desc->pre_proc_info),
           &(_inference_result_header.pre_proc_info[0]),
           sizeof(kp_hw_pre_proc_info_t));

FUNC_OUT:

    return ret;
}

int kp_generic_raw_inference_bypass_pre_proc_send(kp_device_group_t devices, kp_generic_raw_bypass_pre_proc_image_header_t *inf_desc, uint8_t *image_buffer)
{
    kp_generic_data_inference_desc_t _inference_desc = {0};

    _inference_desc.model_id            = inf_desc->model_id;
    _inference_desc.inference_number    = inf_desc->inference_number;
    _inference_desc.num_input_node_data = 1;

    _inference_desc.input_node_data_list[0].buffer      = image_buffer;
    _inference_desc.input_node_data_list[0].buffer_size = inf_desc->image_buffer_size;

    return kp_generic_data_inference_send(devices, &_inference_desc);
}

int kp_generic_raw_inference_bypass_pre_proc_receive(kp_device_group_t devices, kp_generic_raw_bypass_pre_proc_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size)
{
    kp_generic_data_inference_result_header_t _inference_result_header = {0};

    int ret = kp_generic_data_inference_receive(devices, &_inference_result_header, raw_out_buffer, buf_size);

    if (KP_SUCCESS != ret) 
        goto FUNC_OUT;

    output_desc->inference_number   = _inference_result_header.inference_number;
    output_desc->crop_number        = _inference_result_header.crop_number;
    output_desc->num_output_node    = _inference_result_header.num_output_node;
    output_desc->product_id         = _inference_result_header.product_id;

FUNC_OUT:

    return ret;
}
