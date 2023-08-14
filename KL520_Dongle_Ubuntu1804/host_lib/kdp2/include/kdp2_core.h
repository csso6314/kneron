#pragma once

/**
 * @file      kdp2_core.h
 * @brief     Core API
 *
 * Core functions provide fundamental functionality like connection and firmware update.
 * 
 * It also contains the generic inference functions.
 *
 * @copyright (c) 2021 Kneron Inc. All right reserved.
 */

#include <stdint.h>
#include "kdp2_struct.h"
#include "kdp2_postprocess.h"

/**
 * @brief Scan all Kneron devices and report a list.
 * 
 * This function can get devices connectivity information at runtime.
 * 
 * **kdp2_devices_list_t** is a data structure containing multiple **kdp2_device_descriptor_t** and each represents one scanned device.
 * 
 * The **scan_index** in the **kdp2_device_descriptor_t** can be used as inputs when connecting a specified device.
 * 
 * Example usage:
 * 
 * > kdp2_devices_list_t *list;
 * 
 * > kdp2_scan_devices(&list);
 * 
 * > .. use list ..
 * 
 * > free(list);
 * 
 *
 * @param list is an input, the API will allocate memory and fullfill the content.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_scan_devices(kdp2_devices_list_t **list);

/**
 * @brief To connect a Kneron device via the 'scan_index'.
 *
 * @param scan_index the dev_idx to connect.  
 *                   value starts from 1, can be retrieved through kdp2_scan_devices().
 *
 * @return kdp2_device_t represents a device handle, if NULL means failed.
 * 
 */
kdp2_device_t kdp2_connect_device(int scan_index);

/**
 * @brief To connect a Kneron device via the 'KN number'.
 *
 * @param kn_num the unique KN number for a Kneron device.
 *
 * @return kdp2_device_t represents a device handle, if NULL means failed.
 * 
 */
kdp2_device_t kdp2_connect_device_by_kn_number(uint32_t kn_num);

/**
 * @brief To connect the first connectable Kneron device for specified 'product ID'.
 *
 * @param prod_id refer to kdp2_product_id_t.
 *
 * @return kdp2_device_t represents a device handle, if NULL means failed.
 * 
 */
kdp2_device_t kdp2_connect_device_by_product(kdp2_product_id_t prod_id);

/**
 * @brief To connect a Kneron device via the port path.
 *
 * @param port_path the USB port path of target device.
 *
 * @return kdp2_device_t represents a device handle, if NULL means failed.
 * 
 */
kdp2_device_t kdp2_connect_device_by_port_path(const char *port_path);

/**
 * @brief To disconnect a Kneron device.
 *
 * @param device a connected device handle.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 * 
 */
int kdp2_disconnect_device(kdp2_device_t device);

/**
 * @brief To get the device descriptor of a connected device from perspective of USB .
 *
 * @param device a connected device handle.
 *
 * @return refer to kdp2_device_descriptor_t.
 * 
 */
kdp2_device_descriptor_t * kdp2_get_device_descriptor(kdp2_device_t device);

/**
 * @brief To set a global timeout value for all USB communications with the device.
 *
 * @param device a connected device handle.
 * @param milliseconds pre-set timeout value in milliseconds.
 *
 */
void kdp2_set_timeout(kdp2_device_t device, int milliseconds);

/**
 * @brief reset the device in hardware mode or software mode.
 *
 * @param device a connected device handle.
 * @param reset_mode refer to kdp2_reset_mode_t.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 * 
 */
int kdp2_reset_device(kdp2_device_t device, kdp2_reset_mode_t reset_mode);

/**
 * @brief upload models to device through USB
 *
 * @param device a connected device handle.
 * @param nef_buf a buffer contains the content of NEF file.
 * @param nef_size file size of the NEF.
 * @param model_desc this parameter is output for describing the uploaded models.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_load_model(kdp2_device_t device, void *nef_buf, int nef_size, kdp2_all_models_descriptor_t *model_desc);

/**
 * @brief Similar to kdp2_load_model(), and it accepts file path instead of a buffer.
 *
 * @param device a connected device handle.
 * @param file_path a buffer contains the content of NEF file.
 * @param model_desc this parameter is output for describing the uploaded models.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_load_model_from_file(kdp2_device_t device, const char *file_path, kdp2_all_models_descriptor_t *model_desc);

/**
 * @brief update firmware to specified device.
 *
 * @param device a connected device handle.
 * @param fw_id update ID, refer to kdp2_firmware_id_t.
 * @param buffer buffer for the update content
 * @param size buffer size
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_update_firmware(kdp2_device_t device, kdp2_firmware_id_t fw_id, void *buffer, int size);

/**
 * @brief Similar to kdp2_update_firmware(), and it accepts file path instead of a buffer.
 *
 * @param device a connected device handle.
 * @param fw_id update ID, refer to kdp2_firmware_id_t.
 * @param file_path a buffer contains the content of NEF file.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_update_firmware_from_file(kdp2_device_t device, kdp2_firmware_id_t fw_id, const char *file_path);

/**
 * @brief Generic raw inference send.
 * 
 * This is to perform a single image inference, it is non-blocking if device buffer queue is not full.
 * 
 * When this is performed, user can issue kdp2_raw_inference_receive() to get the result.
 * 
 * In addition, to have better performance, users can issue multiple kdp2_raw_inference_send() then start to receive results through kdp2_raw_inference_receive().
 *
 * @param device a connected device handle.
 * @param inf_desc needed parameters for performing inference including image width, height ..etc.
 * @param image_buffer the buffer contains the image.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_raw_inference_send(kdp2_device_t device, kdp2_raw_input_descriptor_t *inf_desc, uint8_t *image_buffer);

/**
 * @brief Generic raw inference receive.
 * 
 * When a image inference is done, this function can be used to get the results in RAW format.
 * 
 * Note that the data received is in Kneron RAW format, users need kdp2_raw_inference_retrieve_node() to convert RAW format data to floating-point data.
 *    
 * @param device a connected device handle.
 * @param output_desc refer to kdp2_raw_output_descriptor_t for describing some information of received data.
 * @param raw_out_buffer a user-allocated buffer for receiving the RAW data results, the needed buffer size can be known from the 'max_raw_out_size' in 'model_desc' through kdp2_load_model().
 * @param raw_buf_size size of raw_out_buffer.
 *
 * @return refer to KDP2_API_RETURN_CODE.
 */
int kdp2_raw_inference_receive(kdp2_device_t device, kdp2_raw_output_descriptor_t *output_desc, uint8_t *raw_out_buffer, uint32_t buf_size);

/**
 * @brief Retrieve single node output data from raw output buffer.
 * 
 * This function retrieves and converts RAW format data to floating-point data on the per-node basis.
 * 
 * The pointer of 'kdp2_node_output_t' actually points to raw_out_buffer so do not free raw_out_buffer before completing the use of 'kdp2_node_output_t *'
 *
 * @param node_idx wanted output node index, starts from 0. Number of total output nodes can be known from 'kdp2_raw_output_descriptor_t'
 * @param raw_out_buffer the RAW output buffer, it should come from kdp2_raw_inference_receive().
 *
 * @return refer to kdp2_node_output_t. It describe 'width x height x channel' in conjunction with floating-pint values of this node.
 */
kdp2_node_output_t * kdp2_raw_inference_retrieve_node(uint32_t node_idx, uint8_t *raw_out_buffer);
