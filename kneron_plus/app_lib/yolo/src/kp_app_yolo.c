/**
 * @file        kp_app_yolo.c
 * @brief       yolo inference functions
 * @version     0.1
 * @date        2021-03-26
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "kp_inference.h"
#include "kp_app_yolo_ipc.h"
#include "app_helper.h"

#define TINY_YOLO_V3_224_224_3 19
#define TINY_YOLO_V3_416_416_3 33
#define TINY_YOLO_V3_608_608_3 34
#define YOLO_V3_416_416_3 205
#define YOLO_V3_608_608_3 210
#define KNERON_YOLOV5S_640_640_3 211
#define KNERON_YOLOV5S_480_256_3 212

#define KDP2_INF_ID_APP_YOLO 11
#define KDP2_JOB_ID_APP_YOLO_CONFIG_POST_PROC 100 // handle set or get

int kp_app_yolo_get_post_proc_parameters(kp_device_group_t devices, int model_id, kp_app_yolo_post_proc_config_t *pp_params)
{
    // FIXME: get only one of devices ?

    int status;
    kdp2_ipc_app_yolo_post_proc_config_t *ipc_pp_cmd = (kdp2_ipc_app_yolo_post_proc_config_t *)malloc(sizeof(kdp2_ipc_app_yolo_post_proc_config_t));
    if(NULL == ipc_pp_cmd)
        return KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;

    ipc_pp_cmd->header_stamp.job_id = KDP2_JOB_ID_APP_YOLO_CONFIG_POST_PROC;
    ipc_pp_cmd->header_stamp.total_image = 1;
    ipc_pp_cmd->header_stamp.image_index = 0;
    ipc_pp_cmd->set_or_get = 0; // get
    ipc_pp_cmd->model_id = model_id;

    do
    {
        // send inference control
        status = kp_customized_inference_send(devices, (void *)ipc_pp_cmd, sizeof(kdp2_ipc_app_yolo_post_proc_config_t), NULL, 0);
        if (status != KP_SUCCESS)
            break;

        // receive inference control
        int recv_size;
        status = kp_customized_inference_receive(devices, (void *)ipc_pp_cmd, sizeof(kdp2_ipc_app_yolo_post_proc_config_t), &recv_size);

        if (status != KP_SUCCESS)
            break;
        if (recv_size != sizeof(kdp2_ipc_app_yolo_post_proc_config_t))
            status = KP_ERROR_RECV_DATA_FAIL_17;

    } while (0);

    memcpy((void *)pp_params, (void *)ipc_pp_cmd->param_data, ipc_pp_cmd->param_size);
    free(ipc_pp_cmd);

    return status;
}

int kp_app_yolo_set_post_proc_parameters(kp_device_group_t devices, int model_id, kp_app_yolo_post_proc_config_t *pp_params)
{
    // apply to all connected devices in the group

    int status;
    kdp2_ipc_app_yolo_post_proc_config_t *ipc_pp_cmd = (kdp2_ipc_app_yolo_post_proc_config_t *)malloc(sizeof(kdp2_ipc_app_yolo_post_proc_config_t));
    if(NULL == ipc_pp_cmd)
        return KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;

    kp_inference_header_stamp_t recv_buf;

    ipc_pp_cmd->header_stamp.job_id = KDP2_JOB_ID_APP_YOLO_CONFIG_POST_PROC;
    ipc_pp_cmd->header_stamp.total_image = 1;
    ipc_pp_cmd->header_stamp.image_index = 0;
    ipc_pp_cmd->set_or_get = 1; // set
    ipc_pp_cmd->model_id = model_id;
    ipc_pp_cmd->param_size = sizeof(kp_app_yolo_post_proc_config_t);
    memcpy((void *)ipc_pp_cmd->param_data, (void *)pp_params, ipc_pp_cmd->param_size); //-V::512

    for (int i = 0; i < devices->num_device; i++)
    {
        do
        {
            // send inference control
            status = kp_customized_inference_send(devices, (void *)ipc_pp_cmd, sizeof(kdp2_ipc_app_yolo_post_proc_config_t), NULL, 0);
            if (status != KP_SUCCESS)
                break;

            // receive inference control
            int recv_size;
            status = kp_customized_inference_receive(devices, (void *)&recv_buf, sizeof(kp_inference_header_stamp_t), &recv_size);
            if (status != KP_SUCCESS)
                break;
            if (recv_size != sizeof(kp_inference_header_stamp_t))
                status = KP_ERROR_RECV_DATA_FAIL_17;

        } while (0);

        if (status != KP_SUCCESS)
            break;
    }

    free(ipc_pp_cmd);

    return status;
}

int kp_app_yolo_inference_send(kp_device_group_t devices, uint32_t inference_number, uint8_t *image_buffer,
                               uint32_t width, uint32_t height, kp_image_format_t format, kp_app_yolo_config_t *yolo_config)
{
    uint32_t image_size = 0;

    int ret = 0;
    ret = get_image_size(format, width, height, &image_size);
    if (ret != KP_SUCCESS)
        return ret;

    kdp2_ipc_app_yolo_inf_header_t app_yolo_header;

    app_yolo_header.header_stamp.job_id = KDP2_INF_ID_APP_YOLO;
    app_yolo_header.header_stamp.total_image = 1;
    app_yolo_header.header_stamp.image_index = 0;

    app_yolo_header.inf_number = inference_number;
    app_yolo_header.width = width;
    app_yolo_header.height = height;
    app_yolo_header.channel = (format == KP_IMAGE_FORMAT_RAW8) ? 1 : 3;
    app_yolo_header.image_format = format;

    uint32_t model_id = 0;                                // means auto-find
    kp_normalize_mode_t model_norm = KP_NORMALIZE_KNERON; // default value

    if (yolo_config)
    {
        model_id = yolo_config->model_id; // if 0, auto find
        if (yolo_config->model_norm != 0)
            model_norm = yolo_config->model_norm;
    }

    if (model_id == 0) // try to find one
    {
        for (int m = 0; m < devices->loaded_model_desc.num_models; m++)
        {
            if (devices->loaded_model_desc.models[m].id == TINY_YOLO_V3_224_224_3 ||
                devices->loaded_model_desc.models[m].id == TINY_YOLO_V3_416_416_3 ||
                devices->loaded_model_desc.models[m].id == TINY_YOLO_V3_608_608_3 ||
                devices->loaded_model_desc.models[m].id == YOLO_V3_416_416_3 ||
                devices->loaded_model_desc.models[m].id == YOLO_V3_608_608_3 ||
                devices->loaded_model_desc.models[m].id == KNERON_YOLOV5S_640_640_3 ||
                devices->loaded_model_desc.models[m].id == KNERON_YOLOV5S_480_256_3)
            {
                model_id = devices->loaded_model_desc.models[m].id;
                break; // choose first suitable
            }
        }
    }

    if (model_id == 0)
        return KP_ERROR_INVALID_MODEL_21;

    app_yolo_header.model_normalize = model_norm; // current default value
    app_yolo_header.model_id = model_id;

    return kp_customized_inference_send(devices, (void *)&app_yolo_header, sizeof(app_yolo_header), image_buffer, image_size);
}

int kp_app_yolo_inference_receive(kp_device_group_t devices, uint32_t *inference_number, kp_app_yolo_result_t *yolo_result)
{
    // directly use user buffer to recv data result
    int recv_size;
    int ret = kp_customized_inference_receive(devices, (void *)yolo_result, sizeof(kp_app_yolo_result_t), &recv_size);

    kdp2_ipc_app_yolo_result_t *yolo_ipc_result = (kdp2_ipc_app_yolo_result_t *)yolo_result;
    *inference_number = yolo_ipc_result->inf_number;

    return ret;
}
