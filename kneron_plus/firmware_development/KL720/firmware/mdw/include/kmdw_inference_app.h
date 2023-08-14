/**
 * @file        kmdw_inference_app.h
 * @brief       for kdp2 fw only - inference structures and functions
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef KMDW_INFERENCE_APP_H
#define KMDW_INFERENCE_APP_H

#include <stdint.h>
#include <stdlib.h>
#include "cmsis_os2.h"
#include "kp_struct.h"
#include "buffer_object.h"

/**
 * @brief prototype for inference entry callback function
 *
 * @param[in] num_input_buf number of input buffer in list
 * @param[in] inf_input_buf_list transmitted from host SW = list of (header + image, input buffer for inference)
 *
 */
typedef void (*kmdw_inference_app_callback_t)(int num_input_buf, void **inf_input_buf_list);

/**
 * @brief prototype for inference result callback function
 *
 * @param[out] status used to indicate exeuction status, refer to KP_API_RETURN_CODE.
 * @param[out] inf_result_buf used to carry inference result back to host SW = header + inferernce result (from ncpu/npu)
 * @param[out] ncpu_result_buf post-processing result buffer done by ncpu
 *
 */
typedef void (*kmdw_inference_app_result_callback_t)(int status, void *inf_result_buf, int inf_result_buf_size, void *ncpu_result_buf);

/**
 * @brief padding values of image for ncpu/npu pre-processing
 */
typedef struct
{
    int32_t pad_top;    /**< padding pixel number at the top of image */
    int32_t pad_bottom; /**< padding pixel number at the bottom of image */
    int32_t pad_left;   /**< padding pixel number at the left of image */
    int32_t pad_right;  /**< padding pixel number at the right of image */
} kp_pad_value_t;

/**
 * @brief structure of image and pre process info
 */
typedef struct
{
    void *image_buf;                                      /**< image buffer address */
    uint32_t image_width;                                 /**< width in pixel */
    uint32_t image_height;                                /**< height in pixel */
    uint32_t image_channel;                               /**< channel count */
    uint32_t image_resize;                                /**< for resize image, part of pre-process, kp_resize_mode_t */
    uint32_t image_padding;                               /**< for padding image, part of pre-process, kp_padding_mode_t */
    uint32_t image_format;                                /**< for color space conversion, part of pre-process, kp_image_format_t */
    uint32_t image_norm;                                  /**< for data normalization, part of pre-process, kp_normalize_mode_t */
    bool enable_crop;                                     /**< if true then 'crop_area' should be set */
    kp_inf_crop_box_t crop_area;                          /**< inference cropping area */
    kp_pad_value_t *pad_value;                            /**< pad_value for ncpu/npu pre-processing */

    bool bypass_pre_proc;                                 /**< if true, then all pre-process will be ignored */
    uint32_t image_buf_size;                              /**< only used for bypass pre-process */
} kp_img_pre_proc_t;

/**
 * @brief inference configuration
 */
typedef struct
{
    /* input */
    int num_image;                                                  /**< number of available images in image_list */
    kp_img_pre_proc_t image_list[MAX_INPUT_NODE_COUNT];    /**< list of images and pre process info */

    int model_id;                                                   /**< target inference model ID */
    bool enable_raw_output;                                         /**< should be true if ncpu does not do post-process */
    bool enable_parallel;                                           /**< only works for single model and post-process in ncpu */
    kmdw_inference_app_result_callback_t result_callback;           /**< callback function for parallel mode */
    void *inf_result_buf;                                           /**< works for enable_parallel=true to carry it back to user callback function */
    int inf_result_buf_size;                                        /**< size of inf_result_buf */
    void *ncpu_result_buf;                                          /**< for ncpu/npu to output, if enable_parallel=true, it will be passed to 'kmdw_inference_app_result_callback_t' */
    void *user_define_data;                                         /**< user define data for ncpu/npu pre-processing */
} kmdw_inference_app_config_t;

/**
 * @brief initialize all components for inference
 *
 * @param[in] app_entry entry function for application
 * @param[in] image_count number of queue size for image buffers. MIN val is 1.
 * @param[in] result_count number of queue size for result buffers. MIN val is 1.
 *
 */
int kmdw_inference_app_init(kmdw_inference_app_callback_t app_entry, uint32_t image_count, uint32_t result_count);

/**
 * @brief send error/status result back to host SW
 *
 * @param[in] job_id user-defind ID to synchronize with host SW side
 * @param[in] error_code error code that needs to send back
 *
 */
void kmdw_inference_app_send_status_code(int job_id, int error_code);

/**
 * @brief do one inference, result_callback works only while enable_parallel = true
 *
 * @param[in] inf_config desired inference configuration
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kmdw_inference_app_execute(kmdw_inference_app_config_t *inf_config);

/**
 * @brief get model raw output data size with specified model id, not include info header
 *
 * @param[in] model_id specified model id
 *
 * @return model raw output data size
 */
uint32_t kmdw_inference_app_get_model_raw_output_size(uint32_t model_id);

/**
 * @brief get model raw output info header
 *
 * @return raw output info header size
 */
uint32_t kmdw_inference_get_raw_output_info_size(void);

/**
 * @brief get model input width and height
 *
 * @param model_id specified model id
 * @param input_index specified input node index
 * @param model_input_width [output] model input width
 * @param model_input_height [output] model input height
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kmdw_inference_get_model_input_image_size(uint32_t model_id, uint32_t input_index, uint32_t *model_input_width, uint32_t *model_input_height);

#endif
