/*
 * Kneron Application initialization
 *
 * Copyright (C) 2022 Kneron, Inc. All rights reserved.
 *
 */

#include <stdio.h>
#include "cmsis_os2.h"

// inference core
#include "kp_struct.h"
#include "kmdw_console.h"
#include "kmdw_inference_app.h"

// inference app
#include "kdp2_inf_app_yolo.h"
#include "demo_customize_inf_single_model.h"
#include "demo_customize_inf_multiple_models.h"

// inference client
#include "kdp2_usb_companion.h"

#define MAX_IMAGE_COUNT   10          /**< MAX inference input  queue slot count */
#define MAX_RESULT_COUNT  10          /**< MAX inference output queue slot count */

/**
 * @brief To register AI applications
 * @param[in] num_input_buf number of data inputs in list
 * @param[in] inf_input_buf_list list of data input for inference task
 * @return N/A
 * @note Add a switch case item for a new inf_app application
 */
static void _app_func(int num_input_buf, void **inf_input_buf_list);

void _app_func(int num_input_buf, void **inf_input_buf_list)
{
    // check header stamp
    if (0 >= num_input_buf) {
        kmdw_printf("No input buffer for app function\n");
        return;
    }

    void *first_inf_input_buf = inf_input_buf_list[0];
    kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)first_inf_input_buf;
    uint32_t job_id = header_stamp->job_id;

    switch (job_id) {
    case KDP2_INF_ID_APP_YOLO:
        kdp2_app_yolo_inference(job_id, num_input_buf, inf_input_buf_list);
        break;
    case KDP2_JOB_ID_APP_YOLO_CONFIG_POST_PROC:
        kdp2_app_yolo_config_post_process_parameters(job_id, num_input_buf, inf_input_buf_list);
        break;
    case DEMO_KL720_CUSTOMIZE_INF_SINGLE_MODEL_JOB_ID: // a demo code implementation in SCPU for user-defined/customized inference from one model
        demo_customize_inf_single_model(job_id, num_input_buf, inf_input_buf_list);
        break;
    case DEMO_KL720_CUSTOMIZE_INF_MULTIPLE_MODEL_JOB_ID: // a demo code implementation in SCPU for user-defined/customized inference from multiple model
        demo_customize_inf_multiple_model(job_id, num_input_buf, inf_input_buf_list);
        break;
    default:
        kmdw_inference_app_send_status_code(job_id, KP_FW_ERROR_UNKNOWN_APP);
        break;
    }
}


void app_initialize(void)
{
    info_msg(">> Start running KL720 KDP2 companion mode ...\n");

    /* initialize inference app */
    /* register APP functions */
    /* specify depth of inference queues */
    kmdw_inference_app_init(_app_func, MAX_IMAGE_COUNT, MAX_RESULT_COUNT);

    /* companion mode init */
    kdp2_usb_companion_init();

    return;
}
