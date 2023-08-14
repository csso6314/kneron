/**
 * @file        kp__update_flash.h
 * @brief       internal flash read write functions
 * @version     1.1
 * @date        2021-07-19
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef __KP_UPDATE_FLASH_H__
#define __KP_UPDATE_FLASH_H__

#pragma once

#include "kp_struct.h"

int kp_update_kdp2_firmware(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size,
                            void *ncpu_fw_buf, int ncpu_fw_size, bool auto_reboot);

int kp_update_kdp2_firmware_from_files(kp_device_group_t devices, const char *scpu_fw_file,
                                       const char *ncpu_fw_file, bool auto_reboot);

int kp_update_kdp_firmware(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size,
                           void *ncpu_fw_buf, int ncpu_fw_size, bool auto_reboot);

int kp_update_kdp_firmware_from_files(kp_device_group_t devices, const char *scpu_fw_file,
                                      const char *ncpu_fw_file, bool auto_reboot);

int kp_update_model(kp_device_group_t devices, void *nef_buf, int nef_size, bool auto_reboot,
                    kp_model_nef_descriptor_t *model_desc);

int kp_update_model_from_file(kp_device_group_t devices, const char *file_path, bool auto_reboot,
                              kp_model_nef_descriptor_t *model_desc);

int kp_update_kdp2_usb_loader(kp_device_group_t devices, void *usb_loader_buf,
                              int usb_loader_size, bool auto_reboot);

int kp_update_kdp2_usb_loader_from_file(kp_device_group_t devices, const char *usb_loader_file,
                                        bool auto_reboot);

int kp_switch_to_kdp2_usb_boot(kp_device_group_t devices, bool auto_reboot);

#endif // __KP_UPDATE_FLASH_H__