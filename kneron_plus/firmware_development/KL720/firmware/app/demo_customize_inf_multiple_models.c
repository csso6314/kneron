/*
 * Kneron Application general functions
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "model_type.h"
#include "kmdw_console.h"

#include "kmdw_inference_app.h"
#include "kmdw_fifoq_manager.h"
#include "demo_customize_inf_multiple_models.h"

#define YOLO_MAX_BOX_NUM            (500) /** please sync this number with max result box limitation of post_processing */

// for pedestrian detection result, should be in DDR
static struct yolo_result_s *pd_result = NULL;
static struct imagenet_result_s *imagenet_result = NULL;

/**
 * @brief describe a yolo post-process configurations for yolo v5 series
 */
typedef struct
{
    float prob_thresh;
    float nms_thresh;
    uint32_t max_detection_per_class;
    uint16_t anchor_row;
    uint16_t anchor_col;
    uint16_t stride_size;
    uint16_t reserved_size;
    uint32_t data[40];
} __attribute__((aligned(4))) kp_app_yolo_post_proc_config_t;

static kp_app_yolo_post_proc_config_t post_proc_params_v5s = {
    .prob_thresh = 0.3,
    .nms_thresh = 0.65,
    .max_detection_per_class = YOLO_MAX_BOX_NUM,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        10, 13, 16, 30, 33, 23,
        30, 61, 62, 45, 59, 119,
        116, 90, 156, 198, 373, 326,
        // strides[3]
        8, 16, 32,
    },
};

static bool init_temp_buffer()
{
    // allocate DDR memory for ncpu/npu output restult
    pd_result = (struct yolo_result_s *)kmdw_ddr_reserve(sizeof(struct yolo_result_s) + YOLO_MAX_BOX_NUM * sizeof(struct bounding_box_s));
    if (pd_result == NULL)
        return false;

    imagenet_result = (struct imagenet_result_s *)kmdw_ddr_reserve(sizeof(struct imagenet_result_s) * IMAGENET_TOP_MAX);
    if (imagenet_result == NULL)
        return false;

    return true;
}

static int inference_pedestrian_detection(demo_customize_inf_multiple_models_header_t *_input_header,
                                          struct yolo_result_s *_pd_result /* output */)
{
    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    // image buffer address should be just after the header
    inf_config.num_image = 1;

    inf_config.image_list[0].image_buf = (void *)((uint32_t)_input_header + sizeof(demo_customize_inf_multiple_models_header_t));
    inf_config.image_list[0].image_width = _input_header->width;
    inf_config.image_list[0].image_height = _input_header->height;
    inf_config.image_list[0].image_channel = 3;                                     // assume RGB565
    inf_config.image_list[0].image_format = KP_IMAGE_FORMAT_RGB565;                 // assume RGB565
    inf_config.image_list[0].image_norm = KP_NORMALIZE_KNERON;                      // this depends on model
    inf_config.image_list[0].image_resize = KP_RESIZE_ENABLE;                       // enable resize
    inf_config.image_list[0].image_padding = KP_PADDING_CORNER;                     // enable padding on corner
    inf_config.model_id = KNERON_PERSONDETECTION_YOLOV5sParklot_480_256_3;          // this depends on model
    inf_config.user_define_data = (void *)&post_proc_params_v5s;                    // yolo post-process configurations for yolo v5 series

    // set up fd result output buffer for ncpu/npu
    inf_config.ncpu_result_buf = (void *)_pd_result;

    return kmdw_inference_app_execute(&inf_config);
}

static int inference_pedestrian_imagenet_classification(demo_customize_inf_multiple_models_header_t *_input_header,
                                                        struct bounding_box_s *_box,
                                                        struct imagenet_result_s * _imagenet_result/* output */)
{
    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    int32_t left = (int32_t)(_box->x1);
    int32_t top = (int32_t)(_box->y1);
    int32_t right = (int32_t)(_box->x2);
    int32_t bottom = (int32_t)(_box->y2);

    // image buffer address should be just after the header
    inf_config.num_image = 1;

    inf_config.image_list[0].image_buf = (void *)((uint32_t)_input_header + sizeof(demo_customize_inf_multiple_models_header_t));
    inf_config.image_list[0].image_width = _input_header->width;
    inf_config.image_list[0].image_height = _input_header->height;
    inf_config.image_list[0].image_channel = 3;                                 // assume RGB565
    inf_config.image_list[0].image_format = KP_IMAGE_FORMAT_RGB565;             // assume RGB565
    inf_config.image_list[0].image_norm = KP_NORMALIZE_KNERON;                  // this depends on model
    inf_config.image_list[0].image_resize = KP_RESIZE_ENABLE;                   // enable resize
    inf_config.image_list[0].image_padding = KP_PADDING_DISABLE;                // disable padding
    inf_config.image_list[0].enable_crop = true;                                // enable crop image in ncpu/npu
    inf_config.model_id = KNERON_PERSONCLASSIFIER_MB_56_32_3;                   // this depends on model

    // set crop box
    inf_config.image_list[0].crop_area.crop_number = 0;
    inf_config.image_list[0].crop_area.x1 = left;
    inf_config.image_list[0].crop_area.y1 = top;
    inf_config.image_list[0].crop_area.width = right - left;
    inf_config.image_list[0].crop_area.height = bottom - top;

    // set up fd result output buffer for ncpu/npu
    inf_config.ncpu_result_buf = (void *)_imagenet_result;

    return kmdw_inference_app_execute(&inf_config);
}

void demo_customize_inf_multiple_model(int job_id, int num_input_buf, void **inf_input_buf_list)
{
    // 'inf_input_buf' and 'inf_result_buf' are provided by kdp2 middleware
    // the content of 'inf_input_buf' is transmitted from host SW = header + image
    // 'inf_result_buf' is used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)

    // now get an available free result buffer
    // normally the begin part of result buffer should contain app-defined result header
    // and the rest is for ncpu/npu inference output data

    if (1 != num_input_buf) {
        kmdw_inference_app_send_status_code(job_id, KP_FW_WRONG_INPUT_BUFFER_COUNT_110);
        return;
    }

    int inf_status;
    int result_buf_size;
    void *inf_result_buf = kmdw_fifoq_manager_result_get_free_buffer(&result_buf_size);

    demo_customize_inf_multiple_models_header_t *input_header = (demo_customize_inf_multiple_models_header_t *)inf_input_buf_list[0];
    demo_customize_inf_multiple_models_result_t *output_result = (demo_customize_inf_multiple_models_result_t *)inf_result_buf;

    // pre set up result header stuff
    // header_stamp is a must to correctly transfer result data back to host SW
    output_result->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    output_result->header_stamp.total_size = sizeof(demo_customize_inf_multiple_models_result_t);
    output_result->header_stamp.job_id = job_id;

    // this app needs extra DDR buffers for ncpu result
    static bool is_init = false;

    if (!is_init) {
        int status = init_temp_buffer();
        if (!status) {
            // notify host error !
            output_result->header_stamp.status_code = KP_FW_DDR_MALLOC_FAILED_102;
            kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
            return;
        }

        is_init = true;
    }

    // do pedestrian detection
    inf_status = inference_pedestrian_detection(input_header, pd_result);
    if (inf_status != KP_SUCCESS) {
        // notify host error !
        output_result->header_stamp.status_code = inf_status;
        kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
        return;
    }

    int box_count = 0;
    int max_box_count = (pd_result->box_count > DME_OBJECT_MAX) ? DME_OBJECT_MAX : pd_result->box_count;
    pd_classification_result_t *pd_classification_result = &output_result->pd_classification_result;

    for (int i = 0; i < max_box_count; i++) {
        struct bounding_box_s *box = &pd_result->boxes[i];

        // do face landmark for each faces
        inf_status = inference_pedestrian_imagenet_classification(input_header, box, imagenet_result);

        if (KP_SUCCESS != inf_status) {
            // notify host error !
            output_result->header_stamp.status_code = inf_status;
            kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
            return;
        }

        // pedestrian_imagenet_classification result (class 0 : background, class 1: person)
        if (1 == imagenet_result[0].index)
            pd_classification_result->pds[box_count].pd_class_socre = imagenet_result[0].score;
        else
            pd_classification_result->pds[box_count].pd_class_socre = imagenet_result[1].score;

        memcpy(&pd_classification_result->pds[box_count].pd, box, sizeof(kp_bounding_box_t));

        box_count++;
    }

    pd_classification_result->box_count = box_count;

    output_result->header_stamp.status_code = KP_SUCCESS;
    output_result->header_stamp.total_size = sizeof(demo_customize_inf_multiple_models_result_t) - sizeof(pd_classification_result_t) +
                                             sizeof(pd_classification_result->box_count) + box_count * sizeof(one_pd_classification_result_t);
    // send output result buffer back to host SW
    kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
}
