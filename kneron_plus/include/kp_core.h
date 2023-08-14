/**
 * @file        kp_core.h
 * @brief       Kneron PLUS core APIs
 *
 * Core functions provide fundamental functionality like connection and firmware update
 *
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */


#pragma once

#include <stdint.h>

#include "kp_struct.h"

/**
 * @brief Scan all Kneron devices and report a list.
 *
 * This function can get devices connectivity information at runtime.
 *
 * @return refer to kp_devices_list_t.
 */
kp_devices_list_t *kp_scan_devices();

/**
 * @brief To connect multiple (including one) Kneron devices.
 *
 * @param[in] num_devices number of devices
 * @param[in] device_port_ids an array contains device's port ID which can be known from kp_scan_devices(), if '0' is given then it will try to connect first connectable device.
 * @param[out] error_code optional variable to indicate an error if connecting devices failed.
 *
 * @return kp_device_group_t represents a set of devices handle, if NULL means failed.
 *
 */
kp_device_group_t kp_connect_devices(int num_devices, int device_port_ids[], int *error_code);

/**
 * @brief To connect multiple (including one) Kneron devices without any examinations of system info.
 *
 * @param[in] num_devices number of devices
 * @param[in] device_port_ids an array contains device's port ID which can be known from kp_scan_devices(), if '0' is given then it will try to connect first connectable device.
 * @param[out] error_code optional variable to indicate an error if connecting devices failed.
 *
 * @return kp_device_group_t represents a set of devices handle, if NULL means failed.
 *
 */
kp_device_group_t kp_connect_devices_without_check(int num_devices, int device_port_ids[], int *error_code);

/**
 * @brief To disconnect a Kneron device.
 *
 * @param[in] devices a set of devices handle.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_disconnect_devices(kp_device_group_t devices);

/**
 * @brief To set a global timeout value for all USB communications with the device.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] milliseconds pre-set timeout value in milliseconds.
 *
 */
void kp_set_timeout(kp_device_group_t devices, int milliseconds);

/**
 * @brief reset the device in hardware mode or software mode.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] reset_mode refer to kp_reset_mode_t.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_reset_device(kp_device_group_t devices, kp_reset_mode_t reset_mode);

/**
 * @brief upload firmware from buffers
 *
 * @param[in] devices a set of devices handle.
 * @param[in] scpu_fw_buf scpu firmware buffer
 * @param[in] scpu_fw_size scpu firmware size
 * @param[in] ncpu_fw_buf ncpu firmware buffer
 * @param[in] ncpu_fw_size ncpu firmware size
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_load_firmware(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size, void *ncpu_fw_buf, int ncpu_fw_size);

/**
 * @brief upload firmware from file
 *
 * @param[in] devices a set of devices handle.
 * @param[in] scpu_fw_path scpu firmware file path
 * @param[in] ncpu_fw_path ncpu firmware file path
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_load_firmware_from_file(kp_device_group_t devices, const char *scpu_fw_path, const char *ncpu_fw_path);

/**
 * @brief upload models to device through USB, and return kp_model_nef_descriptor_t *model_desc (must release model_desc by kp_release_model_nef_descriptor)
 *
 * @param[in] devices a set of devices handle.
 * @param[in] nef_buf a buffer contains the content of NEF file.
 * @param[in] nef_size file size of the NEF.
 * @param[out] model_desc this parameter is output for describing the uploaded models.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_load_model(kp_device_group_t devices, void *nef_buf, int nef_size, kp_model_nef_descriptor_t *model_desc);

/**
 * @brief Similar to kp_load_model(), and it accepts file path instead of a buffer (must release model_desc by kp_release_model_nef_descriptor)
 *
 * @param[in] devices a set of devices handle.
 * @param[in] file_path a buffer contains the content of NEF file.
 * @param[out] model_desc this parameter is output for describing the uploaded models.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_load_model_from_file(kp_device_group_t devices, const char *file_path, kp_model_nef_descriptor_t *model_desc);

/**
 * @brief upload encrypted models to multiple device through USB, and return kp_model_nef_descriptor_t *model_desc (must release model_desc by kp_release_model_nef_descriptor)
 *
 * @param[in] devices a set of devices handle.
 * @param[in] nef_buf number of buffers that contain the content of NEF files.
 * @param[in] nef_size file size of the NEF.
 * @param[in] nef_num total number of NEF files.
 * @param[out] model_desc this parameter is output for describing the uploaded models.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 * @note for `nef_size`, the lengths of all encrypted NEF files originated from the same unencrypted NEF file should be the same.
 */
int kp_load_encrypted_models(kp_device_group_t devices, void *nef_buf[], int nef_size, int nef_num, kp_model_nef_descriptor_t *model_desc);

/**
 * @brief Similar to kp_load_encrypted_models(), and it accepts file paths instead of buffers (must release model_desc by kp_release_model_nef_descriptor).
 *
 * @param[in] devices a set of devices handle.
 * @param[in] file_path file pathes of NEF files.
 * @param[in] nef_num total number of NEF files.
 * @param[out] model_desc this parameter is output for describing the uploaded models.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_load_encrypted_models_from_file(kp_device_group_t devices, char *file_path[], int nef_num, kp_model_nef_descriptor_t *model_desc);

/**
 * @brief Enable firmware log from certain device.
 *
 * This function enables receiving firmware log from certain device with specific device index.
 * The firmware log could be written to text file or directly output to stdout.
 *
 * @param[in] devices a set of devices handle.
 * @param[in] dev_port_id the device port id to enable firmware log.
 * @param[in] log_file_path the log file path, if NULL is passed then firmware log would be directly output to stdout.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_enable_firmware_log(kp_device_group_t devices, int dev_port_id, char *log_file_path);

/**
 * @brief Disable firmware log of all devices with firmware log enabled.
 *
 * @param[in] devices a set of devices handle.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_disable_firmware_log(kp_device_group_t devices);

/**
 * @brief Get system info (kn number and firmware version).
 *
 * @param[in] devices a set of devices handle.
 * @param[in] dev_port_id specific device port id.
 * @param[out] system_info return value of system info.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_get_system_info(kp_device_group_t devices, int dev_port_id, kp_system_info_t *system_info);

/**
 * @brief Get model info (crc, model id, etc.) (must release model_desc by kp_release_model_nef_descriptor).
 *
 * @param[in] devices a set of devices handle.
 * @param[in] dev_port_id specific device port id.
 * @param[out] all_models_desc return value of model info.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_get_model_info(kp_device_group_t devices, int dev_port_id, kp_model_nef_descriptor_t *all_models_desc);

/**
 * @brief To free a kp_model_nef_descriptor_t data buff.
 *
 * @param[in] model_desc a model info descriptor.
 *
 * @return refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_release_model_nef_descriptor(kp_model_nef_descriptor_t *model_desc);

/**
 * @brief Load model from device flash, and return kp_model_nef_descriptor_t *model_desc (must release model_desc by kp_release_model_nef_descriptor)
 *
 * @param[in] devices a set of devices handle.
 * @param[out] model_desc this parameter is output for describing the uploaded models.

 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_load_model_from_flash(kp_device_group_t devices, kp_model_nef_descriptor_t *model_desc);

/**
 * @brief Install device driver on Windows
 *
 * @param device_pid product id of the device
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_install_driver_for_windows(kp_product_id_t device_pid);

/**
 * @brief Store ddr manage attribute into device group
 *
 * @param devices a set of devices handle.
 * @param ddr_attr ddr manage attributes, the zero item in this parameter will be auto generated
 * @return int refer to KP_API_RETURN_CODE in kp_struct.h
 */
int kp_store_ddr_manage_attr(kp_device_group_t devices, kp_ddr_manage_attr_t ddr_attr);

/**
 * @brief Translate error code to char string.
 *
 * @param[in] error_code status/error code from enum KP_API_RETURN_CODE.
 *
 * @return a char buffer of string represents the error message.
 */
const char *kp_error_string(int error_code);

/**
 * @brief Get PLUS version.
 *
 * @return a string represents the PLUS version.
 */
const char *kp_get_version();
