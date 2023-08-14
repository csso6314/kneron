#pragma once

/**
 * @file      kdp2_struct.h
 * @brief     Data structure
 * 
 * The data structures defined here could be used in other header files.
 *
 * @copyright (c) 2021 Kneron Inc. All right reserved.
 */

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief a pointer handle represent connected Kneron device.
 */
typedef void *kdp2_device_t;

/**
 * @brief return code of most APIs.
 */
enum KDP2_API_RETURN_CODE
{
    KDP2_SUCCESS = 0,

    KDP2_ERROR_USB_IO = -1,
    KDP2_ERROR_USB_INVALID_PARAM = -2,
    KDP2_ERROR_USB_ACCESS = -3,
    KDP2_ERROR_USB_NO_DEVICE = -4,
    KDP2_ERROR_USB_NOT_FOUND = -5,
    KDP2_ERROR_USB_BUSY = -6,
    KDP2_ERROR_USB_TIMEOUT = -7,
    KDP2_ERROR_USB_OVERFLOW = -8,
    KDP2_ERROR_USB_PIPE = -9,
    KDP2_ERROR_USB_INTERRUPTED = -10,
    KDP2_ERROR_USB_NO_MEM = -11,
    KDP2_ERROR_USB_NOT_SUPPORTED = -12,
    KDP2_ERROR_USB_OTHER = -99,

    KDP2_ERROR_NO_DEV = -101,
    KDP2_ERROR_NO_DEV_ID = -102,
    KDP2_ERROR_DEV_NOT_CONNECTED = -103,
    KDP2_ERROR_DEV_OP_FAIL = -104,
    KDP2_ERROR_TIMEOUT = -105,
    KDP2_ERROR_INVALID_PARAM = -120,
    KDP2_ERROR_INVALID_VERSION = -121,
    KDP2_ERROR_SEND_DESC_FAIL = -140,
    KDP2_ERROR_SEND_DATA_FAIL = -141,
    KDP2_ERROR_SEND_DATA_TOO_LARGE = -142,
    KDP2_ERROR_SEND_MODEL_FAIL = -143,
    KDP2_ERROR_RECV_DESC_FAIL = -160,
    KDP2_ERROR_RECV_DATA_FAIL = -161,
    KDP2_ERROR_RECV_DATA_TOO_LARGE = -162,
    KDP2_ERROR_NOT_SUPPORTED = -999,
    KDP2_ERROR_OTHER = -1000,

    KDP2_FW_ERROR_ERROR_INVALID_MODEL_ID = 1,
    KDP2_FW_ERROR_ERROR_INVALID_DESCRIPTOR_TYPE,
    KDP2_FW_ERROR_ERROR_OVER_OUTPUT_SIZE,    // need to hard reset Kneron device
    KDP2_FW_ERROR_ERROR_MEM_ALLOCATION_FAIL, // need to hard reset Kneron device

    KDP2_FW_ERROR_ERROR_KAPP_ABORT,
    KDP2_FW_ERROR_ERROR_KAPP_ERR,

    KDP2_FW_ERROR_ERROR_FID_2D_INVALID_INF_MODE = 100,
    KDP2_FW_ERROR_ERROR_FID_2D_SMALL_FACE,       /// face too small
    KDP2_FW_ERROR_ERROR_FID_2D_FDR_WRONG_USAGE,  /// KAPP_FID_CODES
    KDP2_FW_ERROR_ERROR_FID_2D_FDR_NO_FACE,      /// no face
    KDP2_FW_ERROR_ERROR_FID_2D_FDR_BAD_POSE,     /// base face pose
    KDP2_FW_ERROR_ERROR_FID_2D_FDR_FR_FAIL,      /// FR failed
    KDP2_FW_ERROR_ERROR_FID_2D_LM_LOW_CONFIDENT, /// LM low confident
    KDP2_FW_ERROR_ERROR_FID_2D_LM_BLUR,          /// LM blur
    KDP2_FW_ERROR_ERROR_FID_2D_RGB_Y_DARK,

    KDP2_FW_ERROR_ERROR_DB_ADD_USER_FAIL = 200,
    KDP2_FW_ERROR_ERROR_DB_NO_SPACE,      // db register
    KDP2_FW_ERROR_ERROR_DB_ALREADY_SAVED, // db register
    KDP2_FW_ERROR_ERROR_DB_NO_MATCH,      // db compare
    KDP2_FW_ERROR_ERROR_DB_REG_FIRST,     // db add
    KDP2_FW_ERROR_ERROR_DB_DEL_FAIL,      // db delete

    KDP2_FW_ERROR_ERROR_OTHER = 8888, // need to hard reset Kneron device
};

/**
 * @brief enum for USB speed mode
 */
typedef enum
{
    KDP2_USB_SPEED_UNKNOWN = 0,
    KDP2_USB_SPEED_LOW = 1,
    KDP2_USB_SPEED_FULL = 2,
    KDP2_USB_SPEED_HIGH = 3,
    KDP2_USB_SPEED_SUPER = 4,
} kdp2_usb_speed_t;

/**
 * @brief enum for USB PID(Product ID)
 */
typedef enum
{
    KDP2_DEVICE_KL520 = 0x100,
    KDP2_DEVICE_KL720 = 0x200,
} kdp2_product_id_t;

/**
 * @brief reset mode
 */
typedef enum
{
    KDP2_HARD_RESET = 0,      /**< Higheset level to reset Kneron device. Kneron device would disconnect after this reset. */
    KDP2_INFERENCE_RESET = 1, /**< Soft reset: reset inference FIFO queue. */
} kdp2_reset_mode_t;

typedef struct
{
    int scan_index;      /**< scanned order index, can be used by kdp2_connect_device() */
    bool isConnectable;  /**< indicate if this device is connectable. */
    uint16_t vendor_id;  /**< supposed to be 0x3231. */
    uint16_t product_id; /**< enum kdp2_product_id_t. */
    int link_speed;      /**< enum kdp2_usb_speed_t. */
    uint32_t kn_number;  /**< KN number. */
    char port_path[20];  /**< "busNo-hub_portNo-device_portNo"
                                     ex: "1-2-3", means bus 1 - (hub) port 2 - (device) port 3 */
} kdp2_device_descriptor_t;

/**
 * @brief information of connected devices from USB perspectives.
 */
typedef struct
{
    int num_dev;                       /**< connnected devices */
    kdp2_device_descriptor_t device[]; /**< real index range from 0 ~ (num_dev-1) */
} kdp2_devices_list_t;

/**
 * @brief image format supported for inference.
 */
typedef enum
{
    KDP2_IMAGE_FORMAT_UNKNOWN = 0x0,
    KDP2_IMAGE_FORMAT_RGB565 = 0x1,   /**< RGB565 16bits */
    KDP2_IMAGE_FORMAT_RGBA8888 = 0x2, /**< RGBA8888 32bits */
} kdp2_image_format_t;

/**
 * @brief a basic descriptor for a model
 */
typedef struct
{
    uint32_t id;                    /**< model ID */
    uint32_t max_raw_out_size;      /**< needed RAW output buffer size for this model */
    uint32_t width;                 /**< the RAW width of this model */
    uint32_t height;                /**< the RAW height of this model */
    uint32_t channel;               /**<  the RAW channel of this model */
    kdp2_image_format_t img_format; /**< the RAW image format of this model  */
} kdp2_single_model_descriptor_t;

/**
 * @brief a basic descriptor for a NEF
 */
typedef struct
{
    int num_models;                            /**< number of models */
    kdp2_single_model_descriptor_t models[10]; /**< model descriptors */
} kdp2_all_models_descriptor_t;

/**
 * @brief metadata for nef model data: metadata / fw_info / all_models
 */
struct kdp2_metadata_s
{
    char platform[32];     /**< usb dongle, 96 board, etc. */
    uint32_t target;       /**< 0: KL520, 1: KL720, etc. */
    uint32_t crc;          /**< CRC value for all_models data */
    uint32_t kn_num;       /**< KN number */
    uint32_t enc_type;     /**< encrypt type */
    char tc_ver[32];       /**< toolchain version */
    char compiler_ver[32]; /**< compiler version */
};

/**
 * @brief nef info for nef model data: metadata / fw_info / all_models
 */
struct kdp2_nef_info_s
{
    char *fw_info_addr;       /**< Address of fw_info part */
    uint32_t fw_info_size;    /**< Size of fw_info part */
    char *all_models_addr;    /**< Address of all_model part */
    uint32_t all_models_size; /**< Size of all_model part */
};

typedef enum
{
    KDP2_FIRMWARE_SCPU = 1,
    KDP2_FIRMWARE_NCPU = 2,
} kdp2_firmware_id_t;

/**
 * @brief normalization mode
 */
typedef enum
{
    KDP2_NORMALIE_NONE = 0x1,        /**< no atuo-normalization */
    KDP2_NORMALIE_0_TO_1,            /**< auto-normalization 0 ~ 1 */
    KDP2_NORMALIE_NEGATIVE_05_TO_05, /**< auto-normalization -0.5 ~ 0.5 */
    KDP2_NORMALIE_NEGATIVE_1_TO_1,   /**< auto-normalization -1 ~ 1 */
} kdp2_normalize_mode_t;

/**
 * @brief inference descriptor for images
 */
typedef struct
{
    uint32_t width;                       /**< image width */
    uint32_t height;                      /**< image height */
    kdp2_image_format_t image_format;     /**< image format */
    kdp2_normalize_mode_t normalize_mode; /**< inference normalization */
    uint32_t inference_number;            /**< inference sequence number */
    uint32_t model_id;                    /**< target inference model ID */
} kdp2_raw_input_descriptor_t;

/**
 * @brief inference RAW output descriptor
 */
typedef struct
{
    uint32_t inference_number; /**< inference sequence number */
    uint32_t num_output_node;  /**< total number of output nodes */
} kdp2_raw_output_descriptor_t;

/**
 * @brief RAW node output in floating-pint format.
 */
typedef struct
{
    uint32_t width;    /**< node width */
    uint32_t height;   /**< node height */
    uint32_t channel;  /**< node channel */
    uint32_t num_data; /**< total number of floating-poiont values */
    float data[];      /**< array of floating-point values */
} kdp2_node_output_t;

/**
 * @brief describe a bounding box
 */
typedef struct
{
    float x1;          /**< top-left corner: x */
    float y1;          /**< top-left corner: y */
    float x2;          /**< bottom-right corner: x */
    float y2;          /**< bottom-right corner: y */
    float score;       /**< probability score */
    int32_t class_num; /**< class # (of many) with highest probability */
} kdp2_bounding_box_t;

#define YOLO_GOOD_BOX_MAX 2000 /**< maximum number of bounding boxes for Yolo models */

/**
 * @brief describe a yolo output result after post-processing
 */
typedef struct
{
    uint32_t class_count;                         /**< total class count */
    uint32_t box_count;                           /**< boxes of all classes */
    kdp2_bounding_box_t boxes[YOLO_GOOD_BOX_MAX]; /**< box information */
} kdp2_yolo_result_t;