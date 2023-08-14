/**
 * @file        kp_python_wrap.h
 * @brief       python wrapper
 * @version     0.1
 * @date        2021-04-28
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef KP_PYTHON_WRAP_H
#define KP_PYTHON_WRAP_H

#include "kp_python_structure_wrap.h"
#include "kp_core.h"

#ifdef __cplusplus
extern "C"{
#endif

#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

/**
 * extern hidden api from src/include/local/kp_update_flash.h
*/
EXPORT int kp_update_kdp2_firmware(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size, void *ncpu_fw_buf, int ncpu_fw_size, bool auto_reboot);
EXPORT int kp_update_kdp2_firmware_from_files(kp_device_group_t devices, const char *scpu_fw_file, const char *ncpu_fw_file, bool auto_reboot);

/**
 * extern hidden api from src/include/local/kp_set_key.h
*/
EXPORT int kp_set_ckey(kp_device_group_t devices, uint32_t ckey);
EXPORT int kp_set_secure_boot_key(kp_device_group_t devices, uint32_t entry, uint32_t key);
EXPORT int kp_set_gpio(kp_device_group_t devices, uint32_t pin, uint32_t value);

/**
 * extern std c library
*/
EXPORT void py_c_free(void* free_ptr);

#ifdef __cplusplus
}
#endif

#endif // KP_PYTHON_WRAP_H