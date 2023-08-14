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
#include "model_res.h"
#include "kmdw_console.h"

#include "kmdw_inference_app.h"
#include "kmdw_fifoq_manager.h"
#include "demo_customize_inf_multiple_models.h"

#define TY_MAX_BOX_NUM (50)
#define FACE_SCORE_THRESHOLD 0.8f

// for face detection result, should be in DDR
static struct yolo_result_s *fd_result = NULL;

static int inference_face_detection(demo_customize_inf_multiple_models_header_t *input_header,
                                    struct yolo_result_s *fd_result /* output */)
{
    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    // image buffer address should be just after the header
    inf_config.num_image = 1;
    inf_config.image_list[0].image_buf = (void *)((uint32_t)input_header + sizeof(demo_customize_inf_multiple_models_header_t));
    inf_config.image_list[0].image_width = input_header->width;
    inf_config.image_list[0].image_height = input_header->height;
    inf_config.image_list[0].image_channel = 3;                                       // assume RGB565
    inf_config.image_list[0].image_format = KP_IMAGE_FORMAT_RGB565;                   // assume RGB565
    inf_config.image_list[0].image_resize = KP_RESIZE_ENABLE;                         // enable resize
    inf_config.image_list[0].image_padding = KP_PADDING_CORNER;                       // enable padding on corner
    inf_config.image_list[0].image_norm = KP_NORMALIZE_KNERON;                        // this depends on model
    inf_config.model_id = KNERON_FD_MASK_MBSSD_200_200_3;               // this depends on model

    // set up fd result output buffer for ncpu/npu
    inf_config.ncpu_result_buf = (void *)fd_result;

    return kmdw_inference_app_execute(&inf_config);
}

static int inference_face_landmarks(demo_customize_inf_multiple_models_header_t *input_header,
                                    struct bounding_box_s *face_box,
                                    kp_landmark_result_t *lm_result /* output */)
{
    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    int32_t left = (int32_t)(face_box->x1);
    int32_t top = (int32_t)(face_box->y1);
    int32_t right = (int32_t)(face_box->x2);
    int32_t bottom = (int32_t)(face_box->y2);

    // image buffer address should be just after the header
    inf_config.model_id = KNERON_LM_5PTS_ONET_56_56_3;                              // this depends on model
    inf_config.num_image = 1;
    inf_config.image_list[0].image_buf = (void *)((uint32_t)input_header + sizeof(demo_customize_inf_multiple_models_header_t));
    inf_config.image_list[0].image_width = input_header->width;
    inf_config.image_list[0].image_height = input_header->height;
    inf_config.image_list[0].image_channel = 3;                                     // assume RGB565
    inf_config.image_list[0].image_format = KP_IMAGE_FORMAT_RGB565;                 // assume RGB565
    inf_config.image_list[0].image_norm = KP_NORMALIZE_KNERON;                      // this depends on model
    inf_config.image_list[0].image_resize = KP_RESIZE_ENABLE;                       // enable resize
    inf_config.image_list[0].image_padding = KP_PADDING_CORNER;                     // enable padding on corner
    inf_config.image_list[0].enable_crop = true;                                    // enable crop image in ncpu/npu

    // set crop box
    inf_config.image_list[0].crop_area.crop_number = 0;
    inf_config.image_list[0].crop_area.x1 = left;
    inf_config.image_list[0].crop_area.y1 = top;
    inf_config.image_list[0].crop_area.width = right - left;
    inf_config.image_list[0].crop_area.height = bottom - top;

    // set up landmark result output buffer for ncpu/npu
    inf_config.ncpu_result_buf = (void *)lm_result;

    return kmdw_inference_app_execute(&inf_config);
}

static bool init_temp_buffer()
{
    // allocate DDR memory for ncpu/npu output restult
    fd_result = (struct yolo_result_s *)kmdw_ddr_reserve(sizeof(struct yolo_result_s) + TY_MAX_BOX_NUM * sizeof(struct bounding_box_s));

    if (fd_result == NULL) {
        return false;
    }

    return true;
}

void demo_customize_inf_multiple_models(uint32_t job_id, int num_input_buf, void **inf_input_buf_list)
{
    if (1 != num_input_buf) {
        kmdw_inference_app_send_status_code(job_id, KP_FW_WRONG_INPUT_BUFFER_COUNT_110);
        return;
    }

    // 'inf_input_buf' and 'inf_result_buf' are provided by kdp2 middleware
    // the content of 'inf_input_buf' is transmitted from host SW = header + image
    // 'inf_result_buf' is used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)

    // now get an available free result buffer
    // normally the begin part of result buffer should contain app-defined result header
    // and the rest is for ncpu/npu inference output data
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

    // do face detection
    inf_status = inference_face_detection(input_header, fd_result);
    if (inf_status != KP_SUCCESS) {
        // notify host error !
        output_result->header_stamp.status_code = inf_status;
        kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
        return;
    }

    int face_cnt = 0;
    int max_face = (fd_result->box_count > FD_MAX) ? FD_MAX : fd_result->box_count;

    for (int i = 0; i < max_face; i++) {
        struct bounding_box_s *face_box = &fd_result->boxes[i];
        kp_landmark_result_t *face_lm_result = &output_result->faces[face_cnt].lm;

        if (FACE_SCORE_THRESHOLD < face_box->score) {
            // do face landmark for each faces
            inf_status = inference_face_landmarks(input_header, face_box, face_lm_result);

            if (KP_SUCCESS != inf_status) {
                // notify host error !
                output_result->header_stamp.status_code = inf_status;
                kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
                return;
            }

            // skip it if face lm is not good
            if (0.99f > face_lm_result->score) {
                continue;
            }

            memcpy(&output_result->faces[face_cnt].fd, face_box, sizeof(kp_bounding_box_t));
            face_cnt++;
        }
    }

    output_result->face_count = face_cnt;
    output_result->header_stamp.status_code = KP_SUCCESS;

    kmdw_fifoq_manager_result_enqueue((void *)output_result, result_buf_size, false);
}
