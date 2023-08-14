#ifndef __KHOST_API_H__
#define __KHOST_API_H__

#include "base.h"
#include "ipc.h"
#include "khost_ll.h"
#include "khost_ipc_inf_app_tiny_yolo.h"
#include "khost_ipc_inf_app_obejct_detection.h"
#include "khost_ipc_inf_app_person_detection.h"
#include "khost_ipc_inf_app_mobilenet.h"
#include "khost_ipc_inf_app_fid_2d.h"
#include "khost_ipc_inf_app_fd_age_gender.h"
#include "khost_ipc_sys_version.h"


#if defined (__cplusplus) || defined (c_plusplus)
extern "C"{
#endif

/**
 * @brief Configurations sending to Kneron device
 */
enum KHOST_IMG_CONFIG {
    KHOST_IMG_CONFIG_NONE                            = 0,

    /* system realted config */
    KHOST_IMG_SYS_PARALLEL_PROC                      = IMAGE_FORMAT_PARALLEL_PROC,    // pipeling processing
    KHOST_IMG_SYS_BYPASS_CPU_OP                      = IMAGE_FORMAT_BYPASS_CPU_OP,
    KHOST_IMG_SYS_BYPASS_NPU_OP                      = IMAGE_FORMAT_BYPASS_NPU_OP,

    /* pre-processing related config */
    KHOST_IMG_PRE_BYPASS                             = IMAGE_FORMAT_BYPASS_PRE,
    KHOST_IMG_PRE_IMAGE_FORMAT_ROT_SHIFT             = IMAGE_FORMAT_ROT_SHIFT,
    KHOST_IMG_PRE_IMAGE_FORMAT_ROT_CLOCKWISE         = IMAGE_FORMAT_ROT_CLOCKWISE,
    KHOST_IMG_PRE_IMAGE_FORMAT_ROT_COUNTER_CLOCKWISE = IMAGE_FORMAT_ROT_COUNTER_CLOCKWISE,
    KHOST_IMG_PRE_IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT   = IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT,
    KHOST_IMG_PRE_IMAGE_FORMAT_SYMMETRIC_PADDING     = IMAGE_FORMAT_SYMMETRIC_PADDING,
    KHOST_IMG_PRE_IMAGE_FORMAT_SUB128                = IMAGE_FORMAT_SUB128,
    KHOST_IMG_PRE_IMAGE_FORMAT_CHANGE_ASPECT_RATIO   = IMAGE_FORMAT_CHANGE_ASPECT_RATIO,
    KHOST_IMG_PRE_NPU_FORMAT_RGBA8888                = NPU_FORMAT_RGBA8888,
    KHOST_IMG_PRE_NPU_FORMAT_RGB565                  = NPU_FORMAT_RGB565,
    KHOST_IMG_PRE_NPU_FORMAT_NIR                     = NPU_FORMAT_NIR,
    KHOST_IMG_PRE_NPU_FORMAT_YCBCR422                = NPU_FORMAT_YCBCR422,
    KHOST_IMG_PRE_NPU_FORMAT_YCBCR444                = NPU_FORMAT_YCBCR444,

    /* post-processing related config */
    KHOST_IMG_POST_BYPASS                            = IMAGE_FORMAT_BYPASS_POST,
    KHOST_IMG_POST_OUTPUT_RAW                        = IMAGE_FORMAT_RAW_OUTPUT,    // TOTAL_OUT_NUMBER + (H/C/W/RADIX/SCALE) + (H/C/W/RADIX/SCALE) + ... + FP_DATA + FP_DATA + ...
};

enum KHOST_SYS_RESET_MODE {
    KHOST_SYS_HARD_RESET           = 0,    // Higheset level to reset Kneron device. Kneron device would disconnect after this reset.
    KHOST_SYS_SOFT_INF_QUEUE_RESET = 1,    // Soft reset: reset inference FIFO queue.
};

enum KHOST_API_RETURN_CODE {
    KHOST_SUCCESS                   =  0,
    KHOST_ERROR_NO_DEV              = -1,
    KHOST_ERROR_NO_DEV_ID           = -2,
    KHOST_ERROR_DEV_NOT_CONNECTED   = -3,
    KHOST_ERROR_DEV_OP_FAIL         = -4,
    KHOST_ERROR_TIMEOUT             = -5,
    KHOST_ERROR_INVALID_PARAM       = -20,
    KHOST_ERROR_INVALID_VERSION     = -21,
    KHOST_ERROR_SEND_DESC_FAIL      = -40,
    KHOST_ERROR_SEND_DATA_FAIL      = -41,
    KHOST_ERROR_SEND_DATA_TOO_LARGE = -42,
    KHOST_ERROR_SEND_MODEL_FAIL     = -43,
    KHOST_ERROR_RECV_DESC_FAIL      = -60,
    KHOST_ERROR_RECV_DATA_FAIL      = -61,
    KHOST_ERROR_RECV_DATA_TOO_LARGE = -62,
    KHOST_ERROR_NOT_SUPPORTED       = -999,
    KHOST_ERROR_OTHER               = -1000
};


/**
 * @brief Reset Kneron deivce. You could use different mode to reset device.
 *
 * @param[in] dev: Kneron device to connect.
 * @param[in] reset_mode:
 *     KHOST_SYS_HARD_RESET: Higheset level to reset Kneron device. Kneron device would disconnect after this reset.
 *     KHOST_SYS_SOFT_INF_QUEUE_RESET: Soft reset. Reset inference FIFO queue.
 * @param[in] timeout: Timeout(in milliseconds) that this function should wait before giving up. Use value 0 for unlimited timeout.
 *
 * @return KHOST_SUCCESS: Success to reset Kneron device.
 * @return KHOST_ERROR_NO_DEV: No specified Kneron device as API parameter.
 * @return KHOST_ERROR_DEV_OP_FAIL: Cannot reset the device.
 * @return KHOST_ERROR_NOT_SUPPORTED: The reset mode doesn't support.
 */
int khost_reset_device(const khost_device_t dev, uint32_t reset_mode, uint32_t timeout);



/**
 * @brief Send all_models.bin and fw_info.bin to Kneron device for inference.
 *
 * @param[in] dev: Kneron device to connect.
 * @param[in] model: The data of all_models.bin
 * @param[in] model_size: The size of all_models.bin
 * @param[in] fwinfo: The data of fw_info.bin
 * @param[in] fwinfo_size: The size of fw_info.bin
 *
 * @return KHOST_SUCCESS: Success.
 * @return KHOST_ERROR_NO_DEV: No specified Kneron device as API parameter.
 * @return KHOST_ERROR_INVALID_PARAM: Invalid parameters.
 * @return KHOST_ERROR_NO_DEV_ID: Failed to get device ID. This ID is used for kdp_xxx API.
 * @return KHOST_ERROR_SEND_MODEL_FAIL: Failed to send models to Kneron device.
 */
int khost_send_model(const khost_device_t dev, const void* model, const uint32_t model_size, const void* fwinfo, const uint32_t fwinfo_size);



/**
 * @brief Send data to Kneron device for inference.
 *
 * @param[in] dev: Kneron device to connect.
 * @param[in] desc: Descriptor header for communication. See Link.
 * @param[in] desc_size: Descriptor header size.<br>
 * @param[in] data: Data for inference.<br>
 * @param[in] data_size: Data size.<br>
 * @param[in] timeout: Timeout(in milliseconds) that this function should wait before giving up. Use value 0 for unlimited timeout.<br>
 *
 * @return KHOST_SUCCESS: Success.
 * @return KHOST_ERROR_NO_DEV: No specified Kneron device as API parameter.
 * @return KHOST_ERROR_INVALID_PARAM: Invalid parameters.
 * @return KHOST_ERROR_TIMEOUT: Operation timed out.
 * @return KHOST_ERROR_SEND_DESC_FAIL: Failed to send descriptor to Kneron device.
 * @return KHOST_ERROR_SEND_DATA_FAIL: Failed to send data to Kneron device.
 * @return KHOST_ERROR_SEND_DATA_TOO_LARGE: Sending data for inference is too large.
 */
int khost_inference_send(const khost_device_t dev, const void* desc, const uint32_t desc_size, const void* data, const uint32_t data_size, uint32_t timeout);



/**
 * @brief Receive inference results from Kneron device.
 *
 * @param[in] dev: Kneron device to connect.
 * @param[in] recv: Receive buffer from inference results.
 * @param[in] recv_size: Receive buffer size.
 * @param[in] timeout: Timeout(in milliseconds) that this function should wait before giving up. Use value 0 for unlimited timeout.
 *
 * @return KHOST_SUCCESS: Success.
 * @return KHOST_ERROR_NO_DEV: No specified Kneron device as API parameter.
 * @return KHOST_ERROR_TIMEOUT: Operation timed out.
 * @return KHOST_ERROR_RECV_DATA_FAIL: Failed to receive inference result from Kneron device.
 * @return KHOST_ERROR_RECV_DATA_TOO_LARGE: Receiving inference buffer is too large.
 */
int khost_inference_recv(const khost_device_t dev, void* recv, uint32_t recv_size, uint32_t timeout);



/**
 * @brief Validate khost IPC version if on the same page on both sides (host and FW).
 *
 * @param[in] dev: Kneron device to connect.
 * @param[in] timeout: Timeout(in milliseconds) that this function should wait before giving up. Use value 0 for unlimited timeout.
 *
 * @return KHOST_SUCCESS: The version is the same on both sides (host and FW).
 * @return KHOST_ERROR_NO_DEV: No specified Kneron device as API parameter.
 * @return KHOST_ERROR_TIMEOUT: Operation timed out.
 * @return KHOST_ERROR_SEND_DATA_FAIL: Failed to send version check data to Kneron device.
 * @return KHOST_ERROR_RECV_DATA_FAIL: Failed to receive version check data from Kneron device.
 * @return KHOST_ERROR_RECV_DESC_FAIL: The receive descriptor format is incorrect.
 * @return KHOST_ERROR_INVALID_VERSION: The versions on host and FW sides are not the same.
 */
int khost_validate_version(const khost_device_t dev, uint32_t timeout);



/**
 * @brief Returns a constant NULL-terminated string with the ASCII name of a khost API return code.
 *
 * @param[in] ret_code: khost API return code.
 *
 * @return The return name, or the string @b "UNKNOWN return code" if the value of ret_code is not a known error / status code.
 */
const char* khost_ret_code_name(int ret_code);

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif


#endif // __KHOST_API_H__

