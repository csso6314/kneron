/**
 * @file        kp_inference.c
 * @brief       inference functions
 * @version     2.0
 * @date        2022-05-23
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

// #define DEBUG_PRINT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "kp_inference.h"
#include "kp_usb.h"
#include "kp_core.h"

#include "kdp2_ipc_cmd.h"
#include "kdp2_inf_generic_raw.h"
#include "kp_internal.h"
#include "internal_func.h"
#include "model_type.h"

#ifdef DEBUG_PRINT
#define dbg_print(format, ...)  { printf(format, ##__VA_ARGS__); fflush(stdout); }
#else
#define dbg_print(format, ...)
#endif

static int check_inf_desc_error(int ll_return)
{
    if (ll_return == KP_USB_USB_TIMEOUT)
        return KP_ERROR_USB_TIMEOUT_N7;

    if (ll_return != KP_USB_RET_OK)
        return KP_ERROR_SEND_DESC_FAIL_13;

    return KP_SUCCESS;
}

static int check_send_image_error(int ll_return)
{
    if (ll_return == KP_USB_USB_TIMEOUT)
        return KP_ERROR_USB_TIMEOUT_N7;

    if (ll_return != KP_USB_RET_OK)
        return KP_ERROR_SEND_DATA_FAIL_14;

    return KP_SUCCESS;
}

static int get_image_size(kp_image_format_t format, int width, int height, uint32_t *image_size)
{
    switch (format)
    {
    case KP_IMAGE_FORMAT_RGB565:
    case KP_IMAGE_FORMAT_YUYV:
    case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
    case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
    case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
    case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
        *image_size = width * height * 2;
        return KP_SUCCESS;
    case KP_IMAGE_FORMAT_RGBA8888:
        *image_size = width * height * 4;
        return KP_SUCCESS;
    case KP_IMAGE_FORMAT_RAW8:
        *image_size = width * height;
        return KP_SUCCESS;
    case KP_IMAGE_FORMAT_YUV420:
        *image_size = width * height * 1.5;
        return KP_SUCCESS;
    default:
    case KP_IMAGE_FORMAT_UNKNOWN:
        *image_size = 0;
        return KP_ERROR_INVALID_PARAM_12;
    }
}

static bool check_model_id_is_exist_in_nef(_kp_devices_group_t *_devices_grp, uint32_t model_id)
{
    bool ret = false;

    for (int m = 0; m < _devices_grp->loaded_model_desc.num_models; m++)
    {
        if (_devices_grp->loaded_model_desc.models[m].id == model_id)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

static bool check_model_input_node_number_is_correct(_kp_devices_group_t *_devices_grp, uint32_t model_id, uint32_t num_input_node_data)
{
    bool ret = false;

    for (int m = 0; m < _devices_grp->loaded_model_desc.num_models; m++)
    {
        if ((_devices_grp->loaded_model_desc.models[m].id == model_id) &&
            (_devices_grp->loaded_model_desc.models[m].input_nodes_num == num_input_node_data))
        {
            ret = true;
            break;
        }
    }

    return ret;
}

static int verify_result_header_stamp(kp_inference_header_stamp_t *stamp, uint32_t check_total_size, uint32_t check_job_id)
{
    if (stamp->magic_type != KDP2_MAGIC_TYPE_INFERENCE)
        return KP_ERROR_RECEIVE_INCORRECT_HEADER_STAMP_30;

    if (stamp->status_code != KP_SUCCESS)
        return stamp->status_code; // FW report error

    if (check_job_id > 0 && stamp->job_id != check_job_id)
        return KP_ERROR_RECEIVE_JOB_ID_MISMATCH_32;

    if (check_total_size > 0 && stamp->total_size != check_total_size)
        return KP_ERROR_RECEIVE_SIZE_MISMATCH_31;

    return KP_SUCCESS;
}

static kp_channel_ordering_convert_t get_channel_ordering_convert_code(int product_id, kp_channel_ordering_t ordering)
{
    switch (product_id)
    {
    case KP_DEVICE_KL520:
        switch (ordering)
        {
        case KP_CHANNEL_ORDERING_CHW:
            return KP_CHANNEL_ORDERING_CVT_HCW2CHW;
        case KP_CHANNEL_ORDERING_HWC:
            return KP_CHANNEL_ORDERING_CVT_HCW2HWC;
        default:
            return KP_CHANNEL_ORDERING_CVT_NONE;
        }
        break;
    case KP_DEVICE_KL720:
        switch (ordering)
        {
        case KP_CHANNEL_ORDERING_HCW:
            return KP_CHANNEL_ORDERING_CVT_CHW2HCW;
        case KP_CHANNEL_ORDERING_HWC:
            return KP_CHANNEL_ORDERING_CVT_CHW2HWC;
        default:
            return KP_CHANNEL_ORDERING_CVT_NONE;
        }
        break;
    case KP_DEVICE_KL630:
        switch (ordering)
        {
        case KP_CHANNEL_ORDERING_HCW:
            return KP_CHANNEL_ORDERING_CVT_CHW2HCW;
        case KP_CHANNEL_ORDERING_HWC:
            return KP_CHANNEL_ORDERING_CVT_CHW2HWC;
        default:
            return KP_CHANNEL_ORDERING_CVT_NONE;
        }
        break;
    default:
        return KP_CHANNEL_ORDERING_CVT_NONE;
    }
}

static float pow2(int exp)
{
    if (0 <= exp) {
        return (float)(0x1ULL << exp);
    } else {
        return (float)1 / (float)(0x1ULL << abs(exp));
    }
}

int kp_inference_configure(kp_device_group_t devices, kp_inf_configuration_t *conf)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;
    kp_usb_control_t kctrl;

    kctrl.command = KDP2_CONTROL_FIFOQ_ENABLE_DROPPABLE;

    kctrl.arg1 = (conf->enable_frame_drop) ? 1 : 0;

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
        kp_usb_control(ll_dev, &kctrl, timeout); // FIXME for return
    }

    return KP_SUCCESS;
}

int kp_generic_image_inference_send(kp_device_group_t devices, kp_generic_image_inference_desc_t *inf_data)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_send++];

    if (_devices_grp->cur_send >= _devices_grp->num_device)
        _devices_grp->cur_send = 0;

    int timeout = _devices_grp->timeout;

    uint32_t image_size = 0;

    int ret = 0;
    int num_input_node_image = inf_data->num_input_node_image;

    if ((MAX_INPUT_NODE_COUNT < num_input_node_image) ||
        (false == check_model_input_node_number_is_correct(_devices_grp, inf_data->model_id, num_input_node_image))) {
        return KP_ERROR_INVALID_PARAM_12;
    } else if (_devices_grp->ddr_attr.input_buffer_count < num_input_node_image) {
        return KP_ERROR_FIFOQ_INPUT_BUFF_COUNT_NOT_ENOUGH_42;
    }

    for (int i = 0; i < num_input_node_image; i++) {
        ret = get_image_size(inf_data->input_node_image_list[i].image_format, inf_data->input_node_image_list[i].width, inf_data->input_node_image_list[i].height, &image_size);
        if (ret != KP_SUCCESS)
            return ret;

        if (false == check_model_id_is_exist_in_nef(_devices_grp, inf_data->model_id))
        {
            dbg_print("[%s] model id [%d] not exist in nef\n", __func__, inf_data->model_id);
            return KP_ERROR_MODEL_NOT_LOADED_35;
        }

        kdp2_ipc_generic_raw_inf_header_t raw_inf_header;

        raw_inf_header.header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
        raw_inf_header.header_stamp.total_size = sizeof(raw_inf_header) + image_size;
        raw_inf_header.header_stamp.job_id = KDP2_INF_ID_GENERIC_RAW;
        raw_inf_header.header_stamp.total_image = num_input_node_image;
        raw_inf_header.header_stamp.image_index = i;

        if (raw_inf_header.header_stamp.total_size > _devices_grp->ddr_attr.input_buffer_size)
        {
            dbg_print("[%s] image buffer size is not enough in firmware\n", __func__);
            return KP_ERROR_SEND_DATA_TOO_LARGE_15;
        }

        raw_inf_header.inference_number = inf_data->inference_number;
        raw_inf_header.model_id = inf_data->model_id;

        memcpy((void *)&raw_inf_header.image_header, &inf_data->input_node_image_list[i], sizeof(kdp2_ipc_generic_raw_inf_image_header_t));

        ret = kp_usb_write_data(ll_dev, (void *)&raw_inf_header, sizeof(raw_inf_header), timeout);
        int status = check_inf_desc_error(ret);
        if (status != KP_SUCCESS)
            return status;

        ret = kp_usb_write_data(ll_dev, (void *)inf_data->input_node_image_list[i].image_buffer, image_size, timeout);
        status = check_send_image_error(ret);
        if (status != KP_SUCCESS)
            return status;
    }

    return KP_SUCCESS;
}

int kp_generic_image_inference_receive(kp_device_group_t devices, kp_generic_image_inference_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_recv];

    int timeout = _devices_grp->timeout;

    // if return < 0 means libusb error, otherwise return  received size
    int usb_ret = kp_usb_read_data(ll_dev, (void *)raw_out_buffer, buf_size, timeout);
    if (usb_ret < 0)
        return usb_ret;

    // parsing result buffer

    kdp2_ipc_generic_raw_result_t *ipc_result = (kdp2_ipc_generic_raw_result_t *)raw_out_buffer;

    int status = verify_result_header_stamp((kp_inference_header_stamp_t *)ipc_result, 0, KDP2_INF_ID_GENERIC_RAW);

    if (status != KP_SUCCESS) {
        return status;
    }

    output_desc->inference_number = ipc_result->inf_number;
    output_desc->crop_number = ipc_result->crop_number;
    output_desc->product_id = ipc_result->product_id;

    switch (ipc_result->product_id)
    {
    case KP_DEVICE_KL520:
    {
        output_desc->num_output_node = *(uint32_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t));
        break;
    }
    case KP_DEVICE_KL720:
    {
        _720_raw_cnn_res_t *raw_cnn_res = (_720_raw_cnn_res_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t));
        output_desc->num_output_node = raw_cnn_res->total_nodes;
        break;
    }
    case KP_DEVICE_KL630:
    {
        _630_raw_cnn_res_t *raw_cnn_res = (_630_raw_cnn_res_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t));
        output_desc->num_output_node = raw_cnn_res->total_nodes;
        break;
    }
    default:
        break;
    }

    output_desc->num_pre_proc_info = ipc_result->num_of_pre_proc_info;

    memcpy(output_desc->pre_proc_info, ipc_result->pre_proc_info, output_desc->num_pre_proc_info * sizeof(kp_hw_pre_proc_info_t));

    if (ipc_result->is_last_crop == 1)
        _devices_grp->cur_recv++;

    if (_devices_grp->cur_recv >= _devices_grp->num_device)
        _devices_grp->cur_recv = 0;

    return KP_SUCCESS;
}

int kp_generic_data_inference_send(kp_device_group_t devices, kp_generic_data_inference_desc_t *inf_data)
{
    int num_input_node_data = inf_data->num_input_node_data;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_send++];

    if (_devices_grp->cur_send >= _devices_grp->num_device)
        _devices_grp->cur_send = 0;

    if ((MAX_INPUT_NODE_COUNT < num_input_node_data) ||
        (false == check_model_input_node_number_is_correct(_devices_grp, inf_data->model_id, num_input_node_data))) {
        return KP_ERROR_INVALID_PARAM_12;
    } else if (_devices_grp->ddr_attr.input_buffer_count < num_input_node_data) {
        return KP_ERROR_FIFOQ_INPUT_BUFF_COUNT_NOT_ENOUGH_42;
    }

    int timeout = _devices_grp->timeout;

    for (int i = 0; i < num_input_node_data; i++) {
        uint32_t buffer_size = inf_data->input_node_data_list[i].buffer_size;

        int ret = 0;

        if (false == check_model_id_is_exist_in_nef(_devices_grp, inf_data->model_id))
        {
            dbg_print("[%s] model id [%d] not exist in nef\n", __func__, inf_data->model_id);
            return KP_ERROR_MODEL_NOT_LOADED_35;
        }

        kdp2_ipc_generic_raw_inf_bypass_pre_proc_header_t raw_inf_header;

        raw_inf_header.header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
        raw_inf_header.header_stamp.total_size = sizeof(raw_inf_header) + buffer_size;
        raw_inf_header.header_stamp.job_id = KDP2_INF_ID_GENERIC_RAW_BYPASS_PRE_PROC;
        raw_inf_header.header_stamp.total_image = num_input_node_data;
        raw_inf_header.header_stamp.image_index = i;

        if (raw_inf_header.header_stamp.total_size > _devices_grp->ddr_attr.input_buffer_size)
        {
            dbg_print("[%s] image buffer size is not enough in firmware\n", __func__);
            return KP_ERROR_SEND_DATA_TOO_LARGE_15;
        }

        raw_inf_header.inference_number = inf_data->inference_number;
        raw_inf_header.model_id = inf_data->model_id;
        raw_inf_header.image_buffer_size = buffer_size;

        ret = kp_usb_write_data(ll_dev, (void *)&raw_inf_header, sizeof(raw_inf_header), timeout);
        int status = check_inf_desc_error(ret);
        if (status != KP_SUCCESS)
            return status;

        ret = kp_usb_write_data(ll_dev, (void *)inf_data->input_node_data_list[i].buffer, buffer_size, timeout);
        status = check_send_image_error(ret);
        if (status != KP_SUCCESS)
            return status;
    }

    return KP_SUCCESS;
}

int kp_generic_data_inference_receive(kp_device_group_t devices, kp_generic_data_inference_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_recv];

    int timeout = _devices_grp->timeout;

    // if return < 0 means libusb error, otherwise return  received size
    int usb_ret = kp_usb_read_data(ll_dev, (void *)raw_out_buffer, buf_size, timeout);
    if (usb_ret < 0)
        return usb_ret;

    // parsing result buffer

    kdp2_ipc_generic_raw_bypass_pre_proc_result_t *ipc_result = (kdp2_ipc_generic_raw_bypass_pre_proc_result_t *)raw_out_buffer;

    int status = verify_result_header_stamp((kp_inference_header_stamp_t *)ipc_result, 0, KDP2_INF_ID_GENERIC_RAW_BYPASS_PRE_PROC);

    if (status != KP_SUCCESS) {
        return status;
    }

    output_desc->inference_number = ipc_result->inf_number;
    output_desc->crop_number = ipc_result->crop_number;
    output_desc->product_id = ipc_result->product_id;

    switch (ipc_result->product_id)
    {
    case KP_DEVICE_KL520:
    {
        output_desc->num_output_node = *(uint32_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_bypass_pre_proc_result_t));
        break;
    }
    case KP_DEVICE_KL720:
    {
        _720_raw_cnn_res_t *raw_cnn_res = (_720_raw_cnn_res_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_bypass_pre_proc_result_t));
        output_desc->num_output_node = raw_cnn_res->total_nodes;
        break;
    }
    case KP_DEVICE_KL630:
    {
        _630_raw_cnn_res_t *raw_cnn_res = (_630_raw_cnn_res_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_bypass_pre_proc_result_t));
        output_desc->num_output_node = raw_cnn_res->total_nodes;
        break;
    }
    default:
        break;
    }

    if (ipc_result->is_last_crop == 1)
        _devices_grp->cur_recv++;

    if (_devices_grp->cur_recv >= _devices_grp->num_device)
        _devices_grp->cur_recv = 0;

    return KP_SUCCESS;
}

#define KDP_COL_MIN_8       8
#define KDP_COL_MIN_16      16
#define KDP_CHANNEL_MIN_16  16
uint32_t round_up(uint32_t num, uint32_t round_num)
{
    return ((num + (round_num - 1)) & ~(round_num - 1));
}

kp_inf_raw_fixed_node_output_t *kp_generic_inference_retrieve_raw_fixed_node(uint32_t node_idx, uint8_t *raw_out_buffer)
{
    kdp2_ipc_generic_raw_result_t *raw_result = (kdp2_ipc_generic_raw_result_t *)raw_out_buffer;

    switch (raw_result->product_id)
    {
    case KP_DEVICE_KL520:
    {
        uint8_t *data_start = raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t);
        uint32_t out_node_num = *(uint32_t *)data_start;

        if (node_idx > out_node_num - 1)
            return NULL;

        kp_inf_raw_fixed_node_output_t *node_output = (kp_inf_raw_fixed_node_output_t *)malloc(sizeof(kp_inf_raw_fixed_node_output_t));
        if (NULL == node_output)
            return NULL;
        kp_inf_raw_fixed_node_metadata_t *node_desc = (kp_inf_raw_fixed_node_metadata_t *)(data_start + 4);

        uint32_t raw_offset = 4 + out_node_num * sizeof(kp_inf_raw_fixed_node_metadata_t);
        for (int i = 0; i < node_idx; i++)
            raw_offset += node_desc[i].height * node_desc[i].channel * round_up(node_desc[i].width, KDP_COL_MIN_16); // Note: Currently, kl520 output is only support 16W1C8B npu data layout.

        // Copy fixed node output metadata to align with KL720
        memcpy(&node_output->metadata, data_start + 4 + node_idx * sizeof(kp_inf_raw_fixed_node_metadata_t), sizeof(kp_inf_raw_fixed_node_metadata_t));
        node_output->num_data = node_output->metadata.height * node_output->metadata.channel * round_up(node_output->metadata.width, KDP_COL_MIN_16);
        node_output->data = (int8_t *)(data_start + raw_offset);

        // cast npu data layout to kp_tensor_format
        node_output->metadata.data_layout = convert_data_format_to_kp_tensor_format(node_output->metadata.data_layout, KP_MODEL_TARGET_CHIP_KL520);

        return node_output;
    }
    break;

    case KP_DEVICE_KL720:
    {
        _720_raw_cnn_res_t *pRawHead = (_720_raw_cnn_res_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t));

        if (node_idx > pRawHead->total_nodes - 1)
            return NULL;

        kp_inf_raw_fixed_node_output_t *node_output = (kp_inf_raw_fixed_node_output_t *)malloc(sizeof(kp_inf_raw_fixed_node_output_t));
        if (NULL == node_output)
            return NULL;
        node_output->metadata.height = pRawHead->onode_a[node_idx].row_length;
        node_output->metadata.channel = pRawHead->onode_a[node_idx].ch_length;
        node_output->metadata.width = pRawHead->onode_a[node_idx].col_length;
        node_output->metadata.radix = pRawHead->onode_a[node_idx].output_radix;
        node_output->metadata.scale = *(float *)(&pRawHead->onode_a[node_idx].output_scale);
        node_output->metadata.data_layout = pRawHead->onode_a[node_idx].data_format;

        node_output->num_data = pRawHead->total_raw_len;
        node_output->data = (int8_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t) + sizeof(_720_raw_cnn_res_t) + pRawHead->onode_a[node_idx].start_offset);

        // cast npu data layout to kp_tensor_format
        node_output->metadata.data_layout = convert_data_format_to_kp_tensor_format(node_output->metadata.data_layout, KP_MODEL_TARGET_CHIP_KL720);

        return node_output;
    }
    break;

    case KP_DEVICE_KL630:
    {
        _630_raw_cnn_res_t *pRawHead = (_630_raw_cnn_res_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t));

        if (node_idx > (uint32_t)(pRawHead->total_nodes - 1))
            return NULL;

        kp_inf_raw_fixed_node_output_t *node_output = (kp_inf_raw_fixed_node_output_t *)malloc(sizeof(kp_inf_raw_fixed_node_output_t));
        if (NULL == node_output)
            return NULL;
        node_output->metadata.height = pRawHead->onode_a[node_idx].row_length;
        node_output->metadata.channel = pRawHead->onode_a[node_idx].ch_length;
        node_output->metadata.width = pRawHead->onode_a[node_idx].col_length;
        node_output->metadata.radix = pRawHead->onode_a[node_idx].radix;
        node_output->metadata.scale = *(float *)(&pRawHead->onode_a[node_idx].scale);
        node_output->metadata.data_layout = pRawHead->onode_a[node_idx].fmt;

        node_output->num_data = pRawHead->total_raw_len;
        node_output->data = (int8_t *)(raw_out_buffer + sizeof(kdp2_ipc_generic_raw_result_t) + sizeof(_630_raw_cnn_res_t) + pRawHead->onode_a[node_idx].start_offset);

        // cast npu data layout to kp_tensor_format
        node_output->metadata.data_layout = convert_data_format_to_kp_tensor_format(node_output->metadata.data_layout, KP_MODEL_TARGET_CHIP_KL630);

        return node_output;
    }
    break;

    default:
    break;
    }

    return NULL;
}

#define SIZE_OF_FIXED_NODE_DATA 4 // sizeof(int16_t) + padding size for align 4 (ref. kp_inf_fixed_node_output_t)

kp_inf_fixed_node_output_t *kp_generic_inference_retrieve_fixed_node(uint32_t node_idx, uint8_t *raw_out_buffer, kp_channel_ordering_t ordering)
{
    kp_inf_raw_fixed_node_output_t *raw_fixed_node_output = kp_generic_inference_retrieve_raw_fixed_node(node_idx, raw_out_buffer);
    kdp2_ipc_generic_raw_result_t *raw_result = (kdp2_ipc_generic_raw_result_t *)raw_out_buffer;

    if (NULL == raw_fixed_node_output)
        return NULL;

    kp_inf_fixed_node_output_t *fixed_node_output = NULL;

    uint32_t fixed_point_dtype = (KP_MODEL_TENSOR_DATA_LAYOUT_8W1C16B == raw_fixed_node_output->metadata.data_layout) ? KP_FIXED_POINT_DTYPE_INT16 : KP_FIXED_POINT_DTYPE_INT8;
    uint32_t num_data = raw_fixed_node_output->metadata.height * raw_fixed_node_output->metadata.channel * raw_fixed_node_output->metadata.width; // FIXME width
    uint32_t data_size = num_data * ((KP_FIXED_POINT_DTYPE_INT16 == fixed_point_dtype) ? sizeof(int16_t) : sizeof(int8_t));

    fixed_node_output = (kp_inf_fixed_node_output_t *)malloc(sizeof(kp_inf_fixed_node_output_t) - SIZE_OF_FIXED_NODE_DATA + data_size);

    if (NULL == fixed_node_output)
    {
        printf("memory is insufficient to allocate buffer for node output\n");
        free(raw_fixed_node_output); //memory is allocated in kp_generic_inference_retrieve_raw_fixed_node()
        return NULL;
    }

    fixed_node_output->width = raw_fixed_node_output->metadata.width;
    fixed_node_output->height = raw_fixed_node_output->metadata.height;
    fixed_node_output->channel = raw_fixed_node_output->metadata.channel;
    fixed_node_output->radix = raw_fixed_node_output->metadata.radix;
    fixed_node_output->scale = raw_fixed_node_output->metadata.scale;
    fixed_node_output->fixed_point_dtype = fixed_point_dtype;
    fixed_node_output->num_data = num_data;

    #ifdef OPTIMIZED_FIXED_TO_FLOAT
    {
        fixed_node_output->factor = (float)1 / (float)(fixed_node_output->scale * pow2(fixed_node_output->radix));
    }
    #else
    {
        fixed_node_output->factor = (float)(fixed_node_output->scale * pow2(fixed_node_output->radix));
    }
    #endif

    kp_channel_ordering_convert_t channel_ordering_convert_code = get_channel_ordering_convert_code(raw_result->product_id, ordering);
    int width_aligned = 0;
    int n = 0;

    if (KP_MODEL_TENSOR_DATA_LAYOUT_8W1C16B == raw_fixed_node_output->metadata.data_layout)
    {
        /* standard 16-bit fixed-point output */
        width_aligned = round_up(fixed_node_output->width, KDP_COL_MIN_8);

        switch (channel_ordering_convert_code)
        {
        case KP_CHANNEL_ORDERING_CVT_HCW2CHW:
            for (int c = 0; c < fixed_node_output->channel; c++)
            {
                for (int h = 0; h < fixed_node_output->height; h++)
                {
                    for (int w = 0; w < fixed_node_output->width; w++)
                        fixed_node_output->data.int16[n++] = ((int16_t *)(raw_fixed_node_output->data))[(h * fixed_node_output->channel * width_aligned) +
                                                                                                        (c * width_aligned) +
                                                                                                         w];
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HCW:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int c = 0; c < fixed_node_output->channel; c++)
                {
                    for (int w = 0; w < fixed_node_output->width; w++)
                        fixed_node_output->data.int16[n++] = ((int16_t *)(raw_fixed_node_output->data))[(c * fixed_node_output->height * width_aligned) +
                                                                                                        (h * width_aligned) +
                                                                                                         w];
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_HCW2HWC:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int w = 0; w < fixed_node_output->width; w++)
                {
                    for (int c = 0; c < fixed_node_output->channel; c++)
                        fixed_node_output->data.int16[n++] = ((int16_t *)(raw_fixed_node_output->data))[(h * fixed_node_output->channel * width_aligned) +
                                                                                                        (c * width_aligned) +
                                                                                                         w];
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HWC:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int w = 0; w < fixed_node_output->width; w++)
                {
                    for (int c = 0; c < fixed_node_output->channel; c++)
                        fixed_node_output->data.int16[n++] = ((int16_t *)(raw_fixed_node_output->data))[(c * fixed_node_output->height * width_aligned) +
                                                                                                        (h * width_aligned) +
                                                                                                         w];
                }
            }
            break;
        default:
            for (int i = 0; i < fixed_node_output->height * fixed_node_output->channel; i++)
            {
                for (int j = 0; j < fixed_node_output->width; j++)
                    fixed_node_output->data.int16[n++] = ((int16_t *)(raw_fixed_node_output->data))[i * width_aligned + j];
            }
            break;
        }
    }
    else if (KP_MODEL_TENSOR_DATA_LAYOUT_1W16C8B == raw_fixed_node_output->metadata.data_layout)
    {
        /* 8-bit fixed-point output */
        int channel_block_idx = 0;
        int channel_offset_idx = 0;
        int channel_block_size = fixed_node_output->height * fixed_node_output->width * KDP_CHANNEL_MIN_16;

        switch (channel_ordering_convert_code)
        {
        case KP_CHANNEL_ORDERING_CVT_HCW2CHW:
        case KP_CHANNEL_ORDERING_CVT_HCW2HWC:
            /* KL520 not support 1W16C8B ouput NPU data layout format */
            printf("Invalid NPU data layout of HCW to CHW/HWC channel order conversion, NPU data layout = KP_MODEL_TENSOR_DATA_LAYOUT_1W16C8B.\n");
            free(raw_fixed_node_output);
            return NULL;
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HCW:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int c = 0; c < fixed_node_output->channel; c++)
                {
                    channel_block_idx = c / KDP_CHANNEL_MIN_16;
                    channel_offset_idx = c % KDP_CHANNEL_MIN_16;
                    for (int w = 0; w < fixed_node_output->width; w++)
                        fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[(channel_block_idx * channel_block_size) +
                                                                                                      (h * fixed_node_output->width * KDP_CHANNEL_MIN_16) +
                                                                                                      (w * KDP_CHANNEL_MIN_16) +
                                                                                                      (channel_offset_idx)];
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HWC:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int w = 0; w < fixed_node_output->width; w++)
                {
                    for (int c = 0; c < fixed_node_output->channel; c++)
                    {
                        channel_block_idx = c / KDP_CHANNEL_MIN_16;
                        channel_offset_idx = c % KDP_CHANNEL_MIN_16;
                        fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[(channel_block_idx * channel_block_size) +
                                                                                                      (h * fixed_node_output->width * KDP_CHANNEL_MIN_16) +
                                                                                                      (w * KDP_CHANNEL_MIN_16) +
                                                                                                      (channel_offset_idx)];
                    }
                }
            }
            break;
        default:
            for (int c = 0; c < fixed_node_output->channel; c++)
            {
                channel_block_idx = c / KDP_CHANNEL_MIN_16;
                channel_offset_idx = c % KDP_CHANNEL_MIN_16;
                for (int i = 0; i < fixed_node_output->height * fixed_node_output->width; i++)
                    fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[(channel_block_idx * channel_block_size) +
                                                                                                  (i * KDP_CHANNEL_MIN_16) +
                                                                                                  (channel_offset_idx)];
            }
            break;
        }
    }
    else
    {
        /* standard 8-bit fixed-point output */
        width_aligned = round_up(fixed_node_output->width, KDP_COL_MIN_16);

        switch (channel_ordering_convert_code)
        {
        case KP_CHANNEL_ORDERING_CVT_HCW2CHW:
            for (int c = 0; c < fixed_node_output->channel; c++)
            {
                for (int h = 0; h < fixed_node_output->height; h++)
                {
                    for (int w = 0; w < fixed_node_output->width; w++) {
                        fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[(h * fixed_node_output->channel * width_aligned) +
                                                                                                      (c * width_aligned) +
                                                                                                       w];
                    }
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HCW:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int c = 0; c < fixed_node_output->channel; c++)
                {
                    for (int w = 0; w < fixed_node_output->width; w++)
                        fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[(c * fixed_node_output->height * width_aligned) +
                                                                                                      (h * width_aligned) +
                                                                                                       w];
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_HCW2HWC:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int w = 0; w < fixed_node_output->width; w++)
                {
                    for (int c = 0; c < fixed_node_output->channel; c++)
                        fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[(h * fixed_node_output->channel * width_aligned) +
                                                                                                      (c * width_aligned) +
                                                                                                       w];
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HWC:
            for (int h = 0; h < fixed_node_output->height; h++)
            {
                for (int w = 0; w < fixed_node_output->width; w++)
                {
                    for (int c = 0; c < fixed_node_output->channel; c++)
                        fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[(c * fixed_node_output->height * width_aligned) +
                                                                                                      (h * width_aligned) +
                                                                                                       w];
                }
            }
            break;
        default:
            for (int i = 0; i < fixed_node_output->height * fixed_node_output->channel; i++)
            {
                for (int j = 0; j < fixed_node_output->width; j++)
                    fixed_node_output->data.int8[n++] = ((int8_t *)(raw_fixed_node_output->data))[i * width_aligned + j];
            }
            break;
        }
    }

    free(raw_fixed_node_output);

    return fixed_node_output;
}

kp_inf_float_node_output_t *kp_generic_inference_retrieve_float_node(uint32_t node_idx, uint8_t *raw_out_buffer, kp_channel_ordering_t ordering)
{
    kp_inf_raw_fixed_node_output_t *raw_fixed_node_output = kp_generic_inference_retrieve_raw_fixed_node(node_idx, raw_out_buffer);
    kdp2_ipc_generic_raw_result_t *raw_result = (kdp2_ipc_generic_raw_result_t *)raw_out_buffer;

    if (NULL == raw_fixed_node_output)
        return NULL;

    kp_inf_float_node_output_t *float_node_output = NULL;

    int num_data = raw_fixed_node_output->metadata.height * raw_fixed_node_output->metadata.channel * raw_fixed_node_output->metadata.width; // FIXME width

    float_node_output = (kp_inf_float_node_output_t *)malloc(sizeof(kp_inf_float_node_output_t) + num_data * sizeof(float));

    if (NULL == float_node_output)
    {
        printf("memory is insufficient to allocate buffer for node output\n");

        free(raw_fixed_node_output); //memory is allocated in kp_generic_inference_retrieve_raw_fixed_node()
        return NULL;
    }

    float_node_output->channel = raw_fixed_node_output->metadata.channel;
    float_node_output->height = raw_fixed_node_output->metadata.height;
    float_node_output->width = raw_fixed_node_output->metadata.width;
    float_node_output->num_data = num_data;

    float scale = raw_fixed_node_output->metadata.scale;
    int32_t radix = raw_fixed_node_output->metadata.radix;
    float ffactor = 0;

    #ifdef OPTIMIZED_FIXED_TO_FLOAT
    {
        ffactor = (float)1 / (float)(scale * pow2(radix));
    }
    #else
    {
        ffactor = (float)(scale * pow2(radix));
    }
    #endif

    kp_channel_ordering_convert_t channel_ordering_convert_code = get_channel_ordering_convert_code(raw_result->product_id, ordering);
    int width_aligned = 0;
    int n = 0;

    if (KP_MODEL_TENSOR_DATA_LAYOUT_8W1C16B == raw_fixed_node_output->metadata.data_layout)
    {
        /* standard 16-bit floating-point output */
        width_aligned = round_up(float_node_output->width, KDP_COL_MIN_8);

        switch (channel_ordering_convert_code)
        {
        case KP_CHANNEL_ORDERING_CVT_HCW2CHW:
            for (int c = 0; c < float_node_output->channel; c++)
            {
                for (int h = 0; h < float_node_output->height; h++)
                {
                    for (int w = 0; w < float_node_output->width; w++)
                        float_node_output->data[n++] = (float)((int16_t *)(raw_fixed_node_output->data))[(h * float_node_output->channel * width_aligned) +
                                                                                                         (c * width_aligned) +
                                                                                                          w] / ffactor;
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HCW:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int c = 0; c < float_node_output->channel; c++)
                {
                    for (int w = 0; w < float_node_output->width; w++)
                        float_node_output->data[n++] = (float)((int16_t *)(raw_fixed_node_output->data))[(c * float_node_output->height * width_aligned) +
                                                                                                          (h * width_aligned) +
                                                                                                           w] / ffactor;
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_HCW2HWC:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int w = 0; w < float_node_output->width; w++)
                {
                    for (int c = 0; c < float_node_output->channel; c++)
                        float_node_output->data[n++] = (float)((int16_t *)(raw_fixed_node_output->data))[(h * float_node_output->channel * width_aligned) +
                                                                                                         (c * width_aligned) +
                                                                                                          w] / ffactor;
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HWC:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int w = 0; w < float_node_output->width; w++)
                {
                    for (int c = 0; c < float_node_output->channel; c++)
                        float_node_output->data[n++] = (float)((int16_t *)(raw_fixed_node_output->data))[(c * float_node_output->height * width_aligned) +
                                                                                                         (h * width_aligned) +
                                                                                                          w] / ffactor;
                }
            }
            break;
        default:
            for (int i = 0; i < float_node_output->height * float_node_output->channel; i++)
            {
                for (int j = 0; j < float_node_output->width; j++)
                    float_node_output->data[n++] = (float)((int16_t *)(raw_fixed_node_output->data))[i * width_aligned + j] / ffactor;
            }
            break;
        }
    }
    else if (KP_MODEL_TENSOR_DATA_LAYOUT_1W16C8B == raw_fixed_node_output->metadata.data_layout)
    {
        /* 8-bit fixed-point output */
        int channel_block_idx = 0;
        int channel_offset_idx = 0;
        int channel_block_size = float_node_output->height * float_node_output->width * KDP_CHANNEL_MIN_16;

        switch (channel_ordering_convert_code)
        {
        case KP_CHANNEL_ORDERING_CVT_HCW2CHW:
        case KP_CHANNEL_ORDERING_CVT_HCW2HWC:
            /* KL520 not support 1W16C8B ouput NPU data layout format */
            printf("Invalid NPU data layout of HCW to CHW/HWC channel order conversion, NPU data layout = KP_MODEL_TENSOR_DATA_LAYOUT_1W16C8B.\n");
            free(raw_fixed_node_output);
            return NULL;
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HCW:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int c = 0; c < float_node_output->channel; c++)
                {
                    channel_block_idx = c / KDP_CHANNEL_MIN_16;
                    channel_offset_idx = c % KDP_CHANNEL_MIN_16;
                    for (int w = 0; w < float_node_output->width; w++)
                        float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[(channel_block_idx * channel_block_size) +
                                                                                                        (h * float_node_output->width * KDP_CHANNEL_MIN_16) +
                                                                                                        (w * KDP_CHANNEL_MIN_16) +
                                                                                                        (channel_offset_idx)] / ffactor;
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HWC:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int w = 0; w < float_node_output->width; w++)
                {
                    for (int c = 0; c < float_node_output->channel; c++)
                    {
                        channel_block_idx = c / KDP_CHANNEL_MIN_16;
                        channel_offset_idx = c % KDP_CHANNEL_MIN_16;
                        float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[(channel_block_idx * channel_block_size) +
                                                                                                        (h * float_node_output->width * KDP_CHANNEL_MIN_16) +
                                                                                                        (w * KDP_CHANNEL_MIN_16) +
                                                                                                        (channel_offset_idx)] / ffactor;
                    }
                }
            }
            break;
        default:
            for (int c = 0; c < float_node_output->channel; c++)
            {
                channel_block_idx = c / KDP_CHANNEL_MIN_16;
                channel_offset_idx = c % KDP_CHANNEL_MIN_16;
                for (int i = 0; i < float_node_output->height * float_node_output->width; i++)
                    float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[(channel_block_idx * channel_block_size) +
                                                                                                    (i * KDP_CHANNEL_MIN_16) +
                                                                                                    (channel_offset_idx)] / ffactor;
            }
            break;
        }
    }
    else
    {
        /* standard 8-bit floating-point output */
        width_aligned = round_up(float_node_output->width, KDP_COL_MIN_16);

        switch (channel_ordering_convert_code)
        {
        case KP_CHANNEL_ORDERING_CVT_HCW2CHW:
            for (int c = 0; c < float_node_output->channel; c++)
            {
                for (int h = 0; h < float_node_output->height; h++)
                {
                    for (int w = 0; w < float_node_output->width; w++)
                        float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[(h * float_node_output->channel * width_aligned) +
                                                                                                        (c * width_aligned) +
                                                                                                         w] / ffactor;
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HCW:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int c = 0; c < float_node_output->channel; c++)
                {
                    for (int w = 0; w < float_node_output->width; w++)
                        float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[(c * float_node_output->height * width_aligned) +
                                                                                                        (h * width_aligned) +
                                                                                                         w] / ffactor;
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_HCW2HWC:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int w = 0; w < float_node_output->width; w++)
                {
                    for (int c = 0; c < float_node_output->channel; c++)
                        float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[(h * float_node_output->channel * width_aligned) +
                                                                                                        (c * width_aligned) +
                                                                                                         w] / ffactor;
                }
            }
            break;
        case KP_CHANNEL_ORDERING_CVT_CHW2HWC:
            for (int h = 0; h < float_node_output->height; h++)
            {
                for (int w = 0; w < float_node_output->width; w++)
                {
                    for (int c = 0; c < float_node_output->channel; c++)
                        float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[(c * float_node_output->height * width_aligned) +
                                                                                                        (h * width_aligned) +
                                                                                                         w] / ffactor;
                }
            }
            break;
        default:
            for (int i = 0; i < float_node_output->height * float_node_output->channel; i++)
            {
                for (int j = 0; j < float_node_output->width; j++)
                    float_node_output->data[n++] = (float)((int8_t *)(raw_fixed_node_output->data))[i * width_aligned + j] / ffactor;
            }
            break;
        }
    }

    free(raw_fixed_node_output);

    return float_node_output;
}

int kp_customized_inference_send(kp_device_group_t devices, void *header, int header_size, uint8_t *image, int image_size)
{
    int ret;

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_send++];

    if (_devices_grp->cur_send >= _devices_grp->num_device)
        _devices_grp->cur_send = 0;

    // help user to set up header stamp
    kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)header;

    if ((MAX_INPUT_NODE_COUNT < header_stamp->total_image) ||
        (header_stamp->image_index >= header_stamp->total_image)) {
        return KP_ERROR_INVALID_PARAM_12;
    }

    header_stamp->magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    header_stamp->total_size = header_size + image_size;

    if (header_stamp->total_size > _devices_grp->ddr_attr.input_buffer_size)
        return KP_ERROR_SEND_DATA_TOO_LARGE_15;

    ret = kp_usb_write_data(ll_dev, header, header_size, _devices_grp->timeout);
    int status = check_inf_desc_error(ret);
    if (status != KP_SUCCESS)
        return status;

    if (image) // someimtes image buffer could be null
    {
        ret = kp_usb_write_data(ll_dev, image, image_size, _devices_grp->timeout);
        status = check_send_image_error(ret);
        if (status != KP_SUCCESS)
            return status;
    }

    return KP_SUCCESS;
}

int kp_customized_inference_receive(kp_device_group_t devices, void *result_buffer, int buf_size, int *recv_size)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_recv++];

    if (_devices_grp->cur_recv >= _devices_grp->num_device)
        _devices_grp->cur_recv = 0;

    // if return < 0 means libusb error, otherwise return  received size
    int usb_ret = kp_usb_read_data(ll_dev, result_buffer, buf_size, _devices_grp->timeout);
    if (usb_ret < 0)
        return usb_ret;

    *recv_size = usb_ret;

    // verify result buffer
    int status = verify_result_header_stamp((kp_inference_header_stamp_t *)result_buffer, 0, 0);
    if (status != KP_SUCCESS)
        return status;

    return KP_SUCCESS;
}

int kp_customized_command_send(kp_device_group_t devices, void *cmd, int cmd_size, void *return_buf, int return_buf_size)
{
    int ret;

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_send++];

    if (_devices_grp->cur_send >= _devices_grp->num_device)
        _devices_grp->cur_send = 0;

    // help user to set up header stamp
    kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)cmd;
    header_stamp->magic_type = KDP2_MAGIC_TYPE_CUSTOMIZED;
    header_stamp->total_size = cmd_size;

    if (header_stamp->total_size > _devices_grp->ddr_attr.input_buffer_size)
        return KP_ERROR_SEND_DATA_TOO_LARGE_15;

    ret = kp_usb_write_data(ll_dev, cmd, cmd_size, _devices_grp->timeout);
    if (ret != KP_SUCCESS)
        return ret;

    ret = kp_usb_read_data(ll_dev, return_buf, return_buf_size, _devices_grp->timeout);
    if (ret < 0)
        return ret;

    header_stamp = (kp_inference_header_stamp_t *)return_buf;

    if (header_stamp->magic_type != KDP2_MAGIC_TYPE_CUSTOMIZED)
        return KP_ERROR_RECEIVE_INCORRECT_HEADER_STAMP_30;

    return KP_SUCCESS;
}

int kp_dbg_set_enable_checkpoints(kp_device_group_t devices, uint32_t checkpoint_flags, bool enable)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kdp2_ipc_cmd_set_dbg_checkpoint_t cmd;
    cmd.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd.total_size = sizeof(kdp2_ipc_cmd_set_dbg_checkpoint_t);
    cmd.command_id = KDP2_COMMAND_SET_DBG_CHECKPOINT;
    cmd.checkpoint_flags = checkpoint_flags;
    cmd.enable = enable;

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
        int ret = kp_usb_write_data(ll_dev, (void *)&cmd, cmd.total_size, _devices_grp->timeout);
        if (ret != KP_SUCCESS)
            return ret;

        int return_code;
        ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(uint32_t), _devices_grp->timeout);
        if (ret < 0)
            return ret;
        else if (return_code != KP_SUCCESS)
            return return_code;
    }

    return KP_SUCCESS;
}

int kp_dbg_receive_checkpoint_data(kp_device_group_t devices, void **checkpoint_buf)
{
    static void *dbg_buf = NULL;
    int dbg_buf_size = 4 * 1024 * 1024;
    if (dbg_buf == NULL)
    {
        dbg_buf = malloc(dbg_buf_size);
        if (dbg_buf == NULL)
            return KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
    }

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t *ll_dev = _devices_grp->ll_device[_devices_grp->cur_recv++];

    if (_devices_grp->cur_recv >= _devices_grp->num_device)
        _devices_grp->cur_recv = 0;

    // if return < 0 means libusb error, otherwise return received size
    int usb_ret = kp_usb_read_data(ll_dev, dbg_buf, dbg_buf_size, _devices_grp->timeout);
    if (usb_ret < 0)
        return usb_ret;

    kp_inference_header_stamp_t *hdr = (kp_inference_header_stamp_t *)dbg_buf;
    if (hdr->magic_type != KDP2_MAGIC_TYPE_CHECKPOINT_DATA)
        return KP_ERROR_INVALID_CHECKPOINT_DATA_36;

    if (usb_ret == sizeof(kp_inference_header_stamp_t))
        return KP_DBG_CHECKPOINT_END_37;

    *checkpoint_buf = dbg_buf;

    // cast data layout to kp_model_tensor_data_layout_t
    kp_dbg_checkpoint_data_after_inference_t *aft_inf = (kp_dbg_checkpoint_data_after_inference_t *)(*checkpoint_buf);
    if (KP_DBG_CHECKPOINT_AFTER_INFERENCE == aft_inf->checkpoint_tag ||
        KP_DBG_CHECKPOINT_BEFORE_CPU_OP == aft_inf->checkpoint_tag ||
        KP_DBG_CHECKPOINT_AFTER_CPU_OP == aft_inf->checkpoint_tag) {
        for (int i = 0; i < aft_inf->num_nodes; i++)
        {
            aft_inf->node_metadata[i].data_layout = convert_data_format_to_kp_tensor_format(aft_inf->node_metadata[i].data_layout,
                                                                                            _devices_grp->loaded_model_desc.target);
        }
    }

    return KP_SUCCESS;
}

int kp_profile_set_enable(kp_device_group_t devices, bool enable)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[0]; // FIXME

    kdp2_ipc_cmd_set_profile_enable_t cmd_buf;
    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_set_profile_enable_t);
    cmd_buf.command_id = KDP2_COMMAND_SET_PROFILE_ENABLE;
    cmd_buf.enable = enable;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, _devices_grp->timeout);
    if (ret != KP_SUCCESS)
        return ret;

    int return_code;
    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(uint32_t), _devices_grp->timeout);
    if (ret < 0)
        return ret;
    else if (return_code != KP_SUCCESS)
        return return_code;

    return KP_SUCCESS;
}

int kp_profile_get_statistics(kp_device_group_t devices, kp_profile_data_t *profile_data)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[0]; // FIXME

    kdp2_ipc_cmd_get_profile_statics_t cmd_buf;
    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_get_profile_statics_t);
    cmd_buf.command_id = KDP2_COMMAND_GET_PROFILE_STATISTICS;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, _devices_grp->timeout);
    if (ret != KP_SUCCESS)
        return ret;

    ret = kp_usb_read_data(ll_dev, (void *)profile_data, sizeof(kp_profile_data_t), _devices_grp->timeout);
    if (ret < 0)
        return ret;

    return KP_SUCCESS;
}

int kp_performance_monitor_set_enable(kp_device_group_t devices, bool enable)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[0]; // FIXME

    kdp2_ipc_cmd_set_performance_monitor_enable_t cmd_buf;
    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_set_performance_monitor_enable_t);
    cmd_buf.command_id = KDP2_COMMAND_SET_PERFORMANCE_MONITOR_ENABLE;
    cmd_buf.enable = enable;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, _devices_grp->timeout);
    if (ret != KP_SUCCESS)
        return ret;

    int return_code;
    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(uint32_t), _devices_grp->timeout);
    if (ret < 0)
        return ret;
    else if (return_code != KP_SUCCESS)
        return return_code;

    return KP_SUCCESS;
}

int kp_performance_monitor_get_statistics(kp_device_group_t devices, kp_performance_monitor_data_t *performance_monitor_data)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[0]; // FIXME

    kdp2_ipc_cmd_get_performance_monitor_statics_t cmd_buf;
    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_get_performance_monitor_statics_t);
    cmd_buf.command_id = KDP2_COMMAND_GET_PERFORMANCE_MONITOR_STATISTICS;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, _devices_grp->timeout);
    if (ret != KP_SUCCESS)
        return ret;

    ret = kp_usb_read_data(ll_dev, (void *)performance_monitor_data, sizeof(kp_performance_monitor_data_t), _devices_grp->timeout);
    if (ret < 0)
        return ret;

    return KP_SUCCESS;
}
