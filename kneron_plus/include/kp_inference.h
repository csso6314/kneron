/**
 * @file        kp_inference.h
 * @brief       Kneron PLUS inference APIs
 *
 * The inference functions provide sophisticated functionally for different applications.
 * Different set of inference APIs would need different models to make it work.
 *
 * @version     2.0
 * @date        2022-05-23
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "kp_struct.h"

/**
 * @brief Configure inference settings.
 *
 * @param[in] conf refer to kp_inf_configuration_t.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_inference_configure(kp_device_group_t devices, kp_inf_configuration_t *conf);

/**
 * @brief Generic raw inference with multiple input images send.
 *
 * This is to perform one model inference with multiple input images, it is non-blocking if device buffer queue is not full.
 *
 * When this is performed, user can issue kp_generic_image_inference_receive() to get the result.
 *
 * In addition, to have better performance, users can issue multiple kp_generic_image_inference_send() then start to receive results through kp_generic_image_inference_receive().
 *
 * @param[in] devices a set of devices handle.
 * @param[in] inf_data inference data of needed parameters for performing inference including image buffer size, model id.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_image_inference_send(kp_device_group_t devices, kp_generic_image_inference_desc_t *inf_data);

/**
 * @brief
 *
 * @param[in] devices a set of devices handle.
 * @param[in] output_desc refer to kp_generic_image_inference_result_header_t for describing some information of received data.
 * @param[out] raw_out_buffer a user-allocated buffer for receiving the RAW data results, the needed buffer size can be known from the 'max_raw_out_size' in 'model_desc' through kp_load_model().
 * @param[in] buf_size size of raw_out_buffer.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_image_inference_receive(kp_device_group_t devices, kp_generic_image_inference_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size);

/**
 * @brief Generic raw inference with multiple input images and bypass pre-process send.
 *
 * This is to perform one model inference with multiple input images without pre-processing on device, it is non-blocking if device buffer queue is not full.
 *
 * When this is performed, user can issue kp_generic_data_inference_receive() to get the result.
 *
 * In addition, to have better performance, users can issue multiple kp_generic_data_inference_send() then start to receive results through kp_generic_data_inference_receive().
 *
 * @param[in] devices a set of devices handle.
 * @param[in] inf_data inference data of needed parameters for performing inference including image buffer size, model id.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_data_inference_send(kp_device_group_t devices, kp_generic_data_inference_desc_t *inf_data);

/**
 * @brief
 *
 * When a image inference is done, this function can be used to get the results in RAW format.
 *
 * Note that the data received is in Kneron RAW format, users need kp_generic_inference_retrieve_float_node() to convert RAW format data to floating-point data.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] output_desc refer to kp_generic_data_inference_result_header_t for describing some information of received data.
 * @param[out] raw_out_buffer a user-allocated buffer for receiving the RAW data results, the needed buffer size can be known from the 'max_raw_out_size' in 'model_desc' through kp_load_model().
 * @param[in] buf_size size of raw_out_buffer.
 *
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_generic_data_inference_receive(kp_device_group_t devices, kp_generic_data_inference_result_header_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size);

/**
 * @brief Retrieve single node output data from raw output buffer.
 *
 * This function retrieves RAW format data in fixed-point format on the per-node basis.
 *
 * The return pointer of 'kp_inf_raw_fixed_node_output_t' actually points to raw_out_buffer so do not free raw_out_buffer before completing the use of 'kp_inf_raw_fixed_node_output_t *'
 *
 * @param[in] node_idx wanted output node index, starts from 0. Number of total output nodes can be known from 'kp_generic_raw_result_header_t'
 * @param[in] raw_out_buffer the RAW output buffer, it should come from kp_generic_raw_inference_receive().
 *
 * @return refer to kp_inf_raw_fixed_node_output_t. It describes fixed-point values of this node with the Kneron device origin raw data buffer and channel ordering (KL520: height x channel x width (aligned to 16 byte), KL720: channel x height x width (aligned to 16 byte)).
 */
kp_inf_raw_fixed_node_output_t *kp_generic_inference_retrieve_raw_fixed_node(uint32_t node_idx, uint8_t *raw_out_buffer);

/**
 * @brief Retrieve single node output data from raw output buffer.
 *
 * This function retrieves and converts RAW format data to fixed-point data on the per-node basis.
 *
 * @param[in] node_idx wanted output node index, starts from 0. Number of total output nodes can be known from 'kp_generic_raw_result_header_t'
 * @param[in] raw_out_buffer the RAW output buffer, it should come from kp_generic_raw_inference_receive().
 * @param[in] ordering the RAW output channel ordering
 *
 * @return refer to kp_inf_fixed_node_output_t. It describes fixed-point values of this node in specific channel ordering.
 */
kp_inf_fixed_node_output_t *kp_generic_inference_retrieve_fixed_node(uint32_t node_idx, uint8_t *raw_out_buffer, kp_channel_ordering_t ordering);

/**
 * @brief Retrieve single node output data from raw output buffer.
 *
 * This function retrieves and converts RAW format data to floating-point data on the per-node basis.
 *
 * @param[in] node_idx wanted output node index, starts from 0. Number of total output nodes can be known from 'kp_generic_raw_result_header_t'
 * @param[in] raw_out_buffer the RAW output buffer, it should come from kp_generic_raw_inference_receive().
 * @param[in] ordering the RAW output channel ordering
 *
 * @return refer to kp_inf_float_node_output_t. It describes floating-point values of this node in specific channel ordering.
 */
kp_inf_float_node_output_t *kp_generic_inference_retrieve_float_node(uint32_t node_idx, uint8_t *raw_out_buffer, kp_channel_ordering_t ordering);

/**
 * @brief send image for age gender inference
 *
 * @param[in] devices a set of devices handle.
 * @param[in] header user-defined image header, shoud include 'kp_inference_header_stamp_t' in the beginning; in the header stamp, only 'job_id' is needed for user to fill in, others will be handled by API.
 * @param[in] header_size image header size.
 * @param[in] image image buffer.
 * @param[in] image_size image buffer size.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_customized_inference_send(kp_device_group_t devices, void *header, int header_size, uint8_t *image, int image_size);

/**
 * @brief receive inference result of age gender
 *
 * @param[in] devices a set of devices handle.
 * @param[out] result_buffer user-prepared result buffer, when receiving data, it begins with 'kp_inference_header_stamp_t'. user should guarantee buffer size is big enough.
 * @param[in] buf_size result buffer size.
 * @param[out] recv_size received result size.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_customized_inference_receive(kp_device_group_t devices, void *result_buffer, int buf_size, int *recv_size);

/**
 * @brief send a user-defined command and receive the command result, users also need to implement code in firmware side as well.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] cmd user-defined command buffer, shoud include 'kp_inference_header_stamp_t' in the beginning; using 'job_id' as user-defined command ID, others will be handled by API.
 * @param[in] cmd_size command buffer size.
 * @param[out] return_buf user-defined command return buffer in any user-defined format.
 * @param[out] return_buf_size return buffer size.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_customized_command_send(kp_device_group_t devices, void *cmd, int cmd_size, void *return_buf, int return_buf_size);


/**
 * @brief Enable/Disable inference breakpoints in firmware for inference debugging purpose.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] checkpoint_flags bit-fields settings, refer to kp_dbg_checkpoint_flag_t.
 * @param[in] enable set enable/disable.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_dbg_set_enable_checkpoints(kp_device_group_t devices, uint32_t checkpoint_flags, bool enable);

/**
 * @brief To receive debug checkpoint data, use it only if you enable kp_dbg_set_enable_checkpoints().
 *
 * @param[in] devices a set of devices handle.
 * @param[out] checkpoint_buf a buffer contains checkpoint data, memory is allocated automatically while needed.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_dbg_receive_checkpoint_data(kp_device_group_t devices, void **checkpoint_buf);

/**
 * @brief To set enable/disable debug profile.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] enable set enable/disable.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_profile_set_enable(kp_device_group_t devices, bool enable);

/**
 * @brief Collect inference profile results.
 *
 * @param[in] devices a set of devices handle.
 * @param[out] profile_data refer to kp_profile_data_t.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_profile_get_statistics(kp_device_group_t devices, kp_profile_data_t *profile_data);
