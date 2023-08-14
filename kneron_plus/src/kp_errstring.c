/**
 * @file        kp_errstring.c
 * @brief       define error code string
 * @version     1.0
 * @date        2021-07-19
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include "kp_struct.h"

typedef struct
{
    int code;
    const char *string;
} err_str_map;

// easy way of search map, efficiency is not important for this
static err_str_map _errmap[] = {
    {KP_SUCCESS, "Operation is successful"},
    {KP_ERROR_USB_IO_N1, "USB: Input/output error"},
    {KP_ERROR_USB_INVALID_PARAM_N2, "USB: Invalid parameter"},
    {KP_ERROR_USB_ACCESS_N3, "USB: Access denied (insufficient permissions)"},
    {KP_ERROR_USB_NO_DEVICE_N4, "USB: No such device (it may have been disconnected)"},
    {KP_ERROR_USB_NOT_FOUND_N5, "USB: Entity not found"},
    {KP_ERROR_USB_BUSY_N6, "USB: Resource busy"},
    {KP_ERROR_USB_TIMEOUT_N7, "USB: Operation timed out"},
    {KP_ERROR_USB_OVERFLOW_N8, "USB: Overflow"},
    {KP_ERROR_USB_PIPE_N9, "USB: Pipe error"},
    {KP_ERROR_USB_INTERRUPTED_N10, "USB: System call interrupted (perhaps due to signal)"},
    {KP_ERROR_USB_NO_MEM_N11, "USB: Insufficient memory"},
    {KP_ERROR_USB_NOT_SUPPORTED_N12, "USB: Operation not supported or unimplemented on this platform"},
    {KP_ERROR_USB_OTHER_N99, "USB: Other error"},
    {KP_ERROR_WDI_BEGIN, "WDI: Error code begin"},
    {KP_ERROR_WDI_IO_N1, "WDI: Input/output error"},
    {KP_ERROR_WDI_INVALID_PARAM_N2, "WDI: Invalid parameter"},
    {KP_ERROR_WDI_ACCESS_N3, "WDI: Access denied (insufficient permissions)"},
    {KP_ERROR_WDI_NO_DEVICE_N4, "WDI: No such device (it may have been disconnected)"},
    {KP_ERROR_WDI_NOT_FOUND_N5, "WDI: Entity not found"},
    {KP_ERROR_WDI_BUSY_N6, "WDI: Resource busy, or API call already running"},
    {KP_ERROR_WDI_TIMEOUT_N7, "WDI: Operation timed out"},
    {KP_ERROR_WDI_OVERFLOW_N8, "WDI: Overflow"},
    {KP_ERROR_WDI_PENDING_INSTALLATION_N9, "WDI: Another installation is pending"},
    {KP_ERROR_WDI_INTERRUPTED_N10, "WDI: System call interrupted (perhaps due to signal)"},
    {KP_ERROR_WDI_RESOURCE_N11, "WDI: Could not acquire resource (Insufficient memory, etc)"},
    {KP_ERROR_WDI_NOT_SUPPORTED_N12, "WDI: Operation not supported or unimplemented on this platform"},
    {KP_ERROR_WDI_EXISTS_N13, "WDI: Entity already exists"},
    {KP_ERROR_WDI_USER_CANCEL_N14, "WDI: Cancelled by user"},
    {KP_ERROR_WDI_NEEDS_ADMIN_N15, "WDI: Couldn't run installer with required privileges"},
    {KP_ERROR_WDI_WOW64_N16, "WDI: Attempted to run the 32 bit installer on 64 bit"},
    {KP_ERROR_WDI_INF_SYNTAX_N17, "WDI: Bad inf syntax"},
    {KP_ERROR_WDI_CAT_MISSING_N18, "WDI: Missing cat file"},
    {KP_ERROR_WDI_UNSIGNED_N19, "WDI: System policy prevents the installation of unsigned drivers"},
    {KP_ERROR_WDI_OTHER_N99, "WDI: Other error"},
    {KP_ERROR_MEMORY_ALLOCATION_FAILURE_9, "Memory allocation failure"},
    {KP_ERROR_DEVICE_NOT_EXIST_10, "Devices not exist"},
    {KP_ERROR_DEVICE_INCORRECT_RESPONSE_11, "Command response from device is not correct"},
    {KP_ERROR_INVALID_PARAM_12, "Invalid user parameters, please check API descriptions"},
    {KP_ERROR_SEND_DESC_FAIL_13, "Failed to send inference header"},
    {KP_ERROR_SEND_DATA_FAIL_14, "Failed to send inference data (image)"},
    {KP_ERROR_SEND_DATA_TOO_LARGE_15, "Sending data (image) size exceeds firmware buffer size"},
    {KP_ERROR_RECV_DESC_FAIL_16, "Failed to receive inference result descriptor"},
    {KP_ERROR_RECV_DATA_FAIL_17, "Failed to receive inference result data"},
    {KP_ERROR_RECV_DATA_TOO_LARGE_18, "Result data is too big to receive"},
    {KP_ERROR_FW_UPDATE_FAILED_19, "Failed to update firmware"},
    {KP_ERROR_FILE_OPEN_FAILED_20, "Failed to open the file"},
    {KP_ERROR_INVALID_MODEL_21, "The loading model is invalid"},
    {KP_ERROR_IMAGE_RESOLUTION_TOO_SMALL_22, "Image resolution is smaller than model's"},
    {KP_ERROR_IMAGE_INVALID_WIDTH_23, "Image width is not compliant with the target platform's requirement"},
    {KP_ERROR_INVALID_FIRMWARE_24, "Device is not running KDP2 firmware"},
    {KP_ERROR_RESET_FAILED_25, "Failed to reset/reboot the device"},
    {KP_ERROR_DEVICES_NUMBER_26, "Number of connecting devices is incorrect, 0 or over-limited"},
    {KP_ERROR_CONFIGURE_DEVICE_27, "Failed to do USB configurations"},
    {KP_ERROR_CONNECT_FAILED_28, "Failed to connect specified device"},
    {KP_ERROR_DEVICE_GROUP_MIX_PRODUCT_29, "Devices of different platforms cannot be mixed into the same group"},
    {KP_ERROR_RECEIVE_INCORRECT_HEADER_STAMP_30, "Received header stamp is not correct"},
    {KP_ERROR_RECEIVE_SIZE_MISMATCH_31, "Actual received data size does not match with header's"},
    {KP_ERROR_RECEIVE_JOB_ID_MISMATCH_32, "Received job ID is unexpected"},
    {KP_ERROR_INVALID_CUSTOMIZED_JOB_ID_33, "Invalid customized job ID"},
    {KP_ERROR_FW_LOAD_FAILED_34, "Failed to load firmware to the device"},
    {KP_ERROR_MODEL_NOT_LOADED_35, "Specified model is not correctly loaded in device"}, // FIXME: actually FW error code
    {KP_ERROR_INVALID_CHECKPOINT_DATA_36, "Received non-checkpoint data"},
    {KP_DBG_CHECKPOINT_END_37, "Received ending of checkpoint data"},
    {KP_ERROR_INVALID_HOST_38, "Invalid host platform"},
    {KP_ERROR_MEMORY_FREE_FAILURE_39, "Failed to free memory"},
    {KP_ERROR_USB_BOOT_LOAD_SECOND_MODEL_40, "Double load model to USB-boot device"},
    {KP_ERROR_CHECK_FW_VERSION_FAILED_41, "Failed to check firmware version from device"},
    {KP_ERROR_FIFOQ_INPUT_BUFF_COUNT_NOT_ENOUGH_42, "The FIFO queue input buffer count is not enough (may occur when the FIFO queue input count less than the model input number)"},
    {KP_ERROR_FIFOQ_SETTING_FAILED_43, "Failed to FIFO queue configuration (may occur when the device DDR size is not enough for this setting)"},
    {KP_ERROR_UNSUPPORTED_DEVICE_44, "The operating device is not supported on Kneron PLUS"},
    {KP_ERROR_IMAGE_INVALID_HEIGHT_45, "Image height is not compliant with the image format requirement"},
    {KP_ERROR_ADJUST_DDR_HEAP_FAILED_46, "Adjust boundary between model and DDR heap failed"},
    {KP_ERROR_DEVICE_NOT_ACCESSIBLE_47, "Device is not accessible"},
    {KP_ERROR_OTHER_99, "Other/unknown errors !"},
    {KP_FW_ERROR_UNKNOWN_APP, "Device cannot handle the specified APP (or JOB ID)"},
    {KP_FW_INFERENCE_ERROR_101, "Device inference failed"},
    {KP_FW_DDR_MALLOC_FAILED_102, "Device is not able to allocate DDR memory"},
    {KP_FW_INFERENCE_TIMEOUT_103, "Device waits inference results timeout"},
    {KP_FW_LOAD_MODEL_FAILED_104, "Device is not able to load specified model (insufficient DDR memory or model count over-limit)"},
    {KP_FW_CONFIG_POST_PROC_ERROR_MALLOC_FAILED_105, "Device has no memory (SRAM) to allocate post-process parameters"},
    {KP_FW_CONFIG_POST_PROC_ERROR_NO_SPACE_106, "Device has no more space set to store post-process parameters (MAX set is 4)"},
    {KP_FW_IMAGE_SIZE_NOT_MATCH_MODEL_INPUT_107, "Image size is not match model input size"},
    {KP_FW_NOT_SUPPORT_PREPROCESSING_108, "Device cannot handle the specified preprocess command"},
    {KP_FW_GET_MODEL_INFO_FAILED_109, "Device is not able to get model information (model not loaded in DDR memory or other fail)"},
    {KP_FW_WRONG_INPUT_BUFFER_COUNT_110, "The number of input data device received is different from model request"},
    {KP_FW_INVALID_PRE_PROC_MODEL_INPUT_SIZE_111, "Device hardware preprocess cannot handle the specified model input size"},
    {KP_FW_INVALID_INPUT_CROP_PARAM_112, "The crop area can not exceed the boundary of input image"},
    {KP_FW_NCPU_INVALID_IMAGE_201, "NPU cannot handle this image data under current pre-process setting (e.g. Padding > 127)"},
    {KP_FW_EFUSE_CAN_NOT_BURN_300, "Device cannot burn eFuse"},
    {KP_FW_EFUSE_PROTECTED_301, "Device eFuse protected"},
    {KP_FW_EFUSE_OTHER_302, "Device unsupported eFuse status"},
};

static int _num_err_code = sizeof(_errmap) / sizeof(err_str_map);
static const char *_no_message = "No error message for the error code";

const char *kp_error_string(int error_code)
{
    for (int i = 0; i < _num_err_code; i++)
    {
        if (error_code == _errmap[i].code)
            return _errmap[i].string;
    }

    return _no_message; // not found
}