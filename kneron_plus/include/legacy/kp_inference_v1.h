/**
 * @file        kp_inference_v1.h
 * @brief       Legacy Kneron PLUS inference APIs
 *
 * **(To be deprecated in future release)**
 * The inference functions provide sophisticated functionally for different applications.
 * Different set of inference APIs would need different models to make it work.
 *
 * @version     2.0
 * @date        2022-05-23
 *
 * @copyright   Copyright (c) 2022 Kneron Inc. All rights reserved.
 */

#pragma once

#include <stdint.h>

#include "kp_inference.h"
#include "legacy/kp_struct_v1.h"

/**
 * @brief Generic raw inference send.
 *
 * This is to perform a single image inference, it is non-blocking if device buffer queue is not full.
 *
 * When this is performed, user can issue kp_generic_raw_inference_receive() to get the result.
 *
 * In addition, to have better performance, users can issue multiple kp_generic_raw_inference_send() then start to receive results through kp_generic_raw_inference_receive().
 *
 * @param[in] devices a set of devices handle.
 * @param[in] inf_desc needed parameters for performing inference including image width, height ..etc.
 * @param[in] image_buffer the buffer contains the image.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_raw_inference_send(kp_device_group_t devices, kp_generic_raw_image_header_t *inf_desc, uint8_t *image_buffer);

/**
 * @brief Generic raw inference receive.
 *
 * When a image inference is done, this function can be used to get the results in RAW format.
 *
 * Note that the data received is in Kneron RAW format, users need kp_generic_inference_retrieve_float_node() to convert RAW format data to floating-point data.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] output_desc refer to kp_generic_raw_result_header_t for describing some information of received data.
 * @param[out] raw_out_buffer a user-allocated buffer for receiving the RAW data results, the needed buffer size can be known from the 'max_raw_out_size' in 'model_desc' through kp_load_model().
 * @param[in] buf_size size of raw_out_buffer.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_raw_inference_receive(kp_device_group_t devices, kp_generic_raw_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size);

/**
 * @brief Generic raw inference bypass pre-processing send.
 *
 * This is to perform a single image inference, it is non-blocking if device buffer queue is not full.
 *
 * When this is performed, user can issue kp_generic_raw_inference_bypass_pre_proc_receive() to get the result.
 *
 * In addition, to have better performance, users can issue multiple kp_generic_raw_inference_bypass_pre_proc_receive() then start to receive results through kp_generic_raw_inference_receive().
 *
 * @param[in] devices a set of devices handle.
 * @param[in] inf_desc needed parameters for performing inference including image buffer size, model id.
 * @param[in] image_buffer the buffer contains the image.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_raw_inference_bypass_pre_proc_send(kp_device_group_t devices, kp_generic_raw_bypass_pre_proc_image_header_t *inf_desc, uint8_t *image_buffer);

/**
 * @brief Generic raw inference bypass pre-processing receive.
 *
 * When a image inference is done, this function can be used to get the results in RAW format.
 *
 * Note that the data received is in Kneron RAW format, users need kp_generic_inference_retrieve_float_node() to convert RAW format data to floating-point data.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] output_desc refer to kp_generic_raw_bypass_pre_proc_result_header_t for describing some information of received data.
 * @param[out] raw_out_buffer a user-allocated buffer for receiving the RAW data results, the needed buffer size can be known from the 'max_raw_out_size' in 'model_desc' through kp_load_model().
 * @param[in] buf_size size of raw_out_buffer.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_raw_inference_bypass_pre_proc_receive(kp_device_group_t devices, kp_generic_raw_bypass_pre_proc_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size);

