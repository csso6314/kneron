/**
 * @file      kdp_host.h
 * @brief     Kneron NPU Host API
 *
 * Host API is the API to setup communication channels between host (such as PC, Embedded Chips)
 * and Kneron NPU Chip. Users can use Host API to write programs that utilize low power and high
 * performance Kneron NPU Chip to accelarate their deep learning model application.
 * There are three types of APIs:
 *
 *     - System API: These APIs are used to monitor system, update firmware and models in flash.
 *     - DME Mode API: These APIs are for setting up dynamic loaded model, and inference.
 *     - ISI Mode API: These APIs are for setting up image streaming interface, and inference.
 *     - Application specific API: These APIs are specific to certain application
 *
 * @version   0.2 - 2021-01-07
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __KDP_HOST__H__
#define __KDP_HOST__H__

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include "common.h"
#include "ipc.h"

#define KDP_UART_DEV   0        			/**< identification of using UART I/F */
#define KDP_USB_DEV    1        			/**< identification of using USB I/F */

#define IMG_FORMAT_RGBA8888     0x80000000  /**< image format: RGBA8888 */
#define IMG_FORMAT_RAW8         0x80000020  /**< image format: RAW8 */
#define IMG_FORMAT_YCbCr422     0x80000037  /**< image format: YCbCr422 [low byte]Y0CbY1CrY2CbY3Cr...[high byte] */
#define IMG_FORMAT_RGB565       0x80000060  /**< image foramt: RGB565 */

#define DEF_FR_THRESH        FID_THRESHOLD  /**< FDR app only, default face recognition threshold */
#define DEF_RGB_FR_THRESH    DEF_FR_THRESH  /**< FDR app only, default face recognition threshold for RGB source */
#define DEF_NIR_FR_THRESH    DEF_FR_THRESH  /**< FDR app only, default face recognition threshold for NIR soruce */

#define CONFIG_USE_FLASH_MODEL      BIT0    /**< config params: 1: use flash model, 0: use model in memory */

/**
 * @brief DME image configuration structure
 */
struct kdp_dme_cfg_s {
    int32_t     model_id;           /**< Model indentification ID */
    int32_t     output_num;         /**< Output number */

    int32_t     image_col;          /**< Column size.
                                         NIR8: must be multiple of 4;
                                         RGB565/YCBCR422: must be multiple of 2. */
    int32_t     image_row;          /**< Row size */
    int32_t     image_ch;           /**< Channel size */
    uint32_t    image_format;       /**< Image format */

    /* Image processing parameters */
    struct kdp_crop_box_s   crop_box;                   /**< y1, y2, x1, x2, for future use */
    struct kdp_pad_value_s  pad_values;                 /**< for future use */
    float                   ext_param[MAX_PARAMS_LEN];  /**< extra parameters, such as threshold */
};


/**
 * @brief ISI image configuration structure
 */
struct kdp_isi_cfg_s {
    uint32_t     app_id;            /**< Application id */
    uint32_t     res_buf_size;      /**< Aesult buffer size */
    uint16_t     image_col;         /**< Column size.
                                         NIR8: must be multiple of 4;
                                         RGB565/YCBCR422: must be multiple of 2. */
    uint16_t     image_row;         /**< row size */
    uint32_t     image_format;      /**< image format */

    /* Image processing parameters */
    struct kdp_crop_box_s   crop_box;                   /**< y1, y2, x1, x2, for future use */
    struct kdp_pad_value_s  pad_values;                 /**< for future use */
    float                   ext_param[MAX_PARAMS_LEN];  /**< extra parameters, such as threshold */
};


/**
 * @brief Metadata for nef model data: metadata / fw_info / all_models
 */
struct kdp_metadata_s {
    char         platform[32];      /**< usb dongle, 96 board, etc. */
    uint32_t     target;            /**< 0: KL520, 1: KL720, etc. */
    uint32_t     crc;               /**< CRC value for all_models data */
    uint32_t     kn_num;            /**< KN number */
    uint32_t     enc_type;          /**< encrypt type */
    char         tc_ver[32];        /**< toolchain version */
    char         compiler_ver[32];  /**< compiler version */
};


/**
 * @brief NEF info for nef model data: metadata / fw_info / all_models
 */
struct kdp_nef_info_s {
    char*        fw_info_addr;      /**< Address of fw_info part */
    uint32_t     fw_info_size;      /**< Size of fw_info part */
    char*        all_models_addr;   /**< Address of all_model part */
    uint32_t     all_models_size;   /**< Size of all_model part */
};

/**
 * @brief FDR App only, DB configuration structure
 */
struct kdp_db_config_s {
    uint16_t    db_num;             /**< number of database */
    uint16_t    max_uid;            /**< max number of user ID in each db */
    uint16_t    max_fid;            /**< max number of feature map */
};
typedef struct kdp_db_config_s kdp_db_config_t;

/**
 * @brief FDR App only, DB config parameter union #total [8 Bytes] (32-bits auto align)
 */
union kapp_db_config_parameter_u {
    kdp_db_config_t db_config;      /**< kdp_db_config_t            [6 Bytes] */
    uint32_t        uint32_value;   /**< config uint32_t parameter  [4 Bytes] */
};
typedef union kapp_db_config_parameter_u kapp_db_config_parameter_t;

//-----------------------library init ----------------------

/**
 * @brief To init the host library
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_lib_init();


/**
 * @brief To start the host library to wait for messages
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_lib_start();


/**
 * @brief Free the resources used by host lib
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_lib_de_init();


/**
 * @brief Init the host lib internal log
 *
 * @param[in] dir the directory name of the log file
 * @param[in] name the log file name
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_init_log(const char* dir, const char* name);


/**
 * @brief enum for USB speed mode
 */
enum kdp_usb_speed_e {
    KDP_USB_SPEED_UNKNOWN = 0,  /**< unknown USB speed */
    KDP_USB_SPEED_LOW = 1,      /**< USB low speed */
    KDP_USB_SPEED_FULL = 2,     /**< USB full speed */
    KDP_USB_SPEED_HIGH = 3,     /**< USB high speed */
    KDP_USB_SPEED_SUPER = 4,    /**< USB super speed */
};


/**
 * @brief enum for USD PID(Product ID)
 */
enum kdp_product_id_e {
    KDP_DEVICE_KL520 = 0x100,  /**< USB PID alias for KL520 */
    KDP_DEVICE_KL720 = 0x200,  /**< USB PID alias for KL720 */
};


/**
 * @brief Device information structure
 */
struct kdp_device_info_s {
    int scan_index;             /**< scanned order index, can be used by kdp_connect_usb_device() */
    bool isConnectable;         /**< indicate if this device is connectable */
    unsigned short vendor_id;   /**< supposed to be 0x3231 */
    unsigned short product_id;  /**< for kdp_product_id_e */
    int link_speed;             /**< for kdp_usb_speed_e */
    unsigned int serial_number; /**< KN number */
    char device_path[20];       /**< "busNo-hub_portNo-device_portNo"
                                     ex: "1-2-3", means bus 1 - (hub) port 2 - (device) port 3 */
};
typedef struct kdp_device_info_s kdp_device_info_t;


/**
 * @brief Information structure of connected devices
 */
struct kdp_device_info_list_s {
    int num_dev;                    /**< connnected devices */
    kdp_device_info_t kdevice[1];   /**< real index range from 0 ~ (num_dev-1) */
};
typedef struct kdp_device_info_list_s kdp_device_info_list_t;


/**
 * @brief To scan all Kneron devices and report a list
 *
 * @param[in] list an input, the API will allocate memory and fullfill the content.
 *
 * @return always 0
 */
int kdp_scan_usb_devices(kdp_device_info_list_t **list);


/**
 * @brief To connect to a Kneron device via the 'scan_index'
 *
 * @param[in] scan_index the dev_idx to connect.
 *            value starts from 1, can be retrieved through kdp_scan_devices()
 *
 * @return 'dev_idx' if connection is established,
 *         '< 0' means connection is not established,
 */
int kdp_connect_usb_device(int scan_index);


/**
 * @brief To add com device to the host lib
 *
 * @param[in] type device type, only KDP_USB_DEV is supported now
 * @param[in] name the UART device name
 *
 * @return 'dev idx' on succeed, -1 on failure
 */
int kdp_add_dev(int type, const char* name);
//---------------------- lib init end------------------------


//-----------------------system operation APIs --------------------------
/**
 * @brief To request for doing system reset
 *
 * @param[in] dev_idx connected device ID. A host can connect several devices
 * @param[in] reset_mode specifies the reset mode
 *                   0 - no operation
 *                   1 - reset message protocol
 *                   3 - switch to suspend mode
 *                   4 - switch to active mode
 *                   255 - reset whole system
 *                   256 - system shutdown(RTC)
 *                   0x1000xxxx - reset debug output level
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_reset_sys(int dev_idx, uint32_t reset_mode);


/**
 * @brief To request for system status
 *
 * @param[in] dev_idx connected device ID. A host can connect several devices
 * @param[out] sfw_id the version of the scpu firmware
 * @param[out] sbuild_id the build number of the scpu firmware
 * @param[out] sys_status system status (Beta)
 * @param[out] app_status application status (Beta)
 * @param[out] nfw_id the version of the ncpu firmware (reserved)
 * @param[out] nbuild_id the build number of the ncpu firmware (reserved)
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_report_sys_status(int dev_idx,
                          uint32_t* sfw_id, uint32_t* sbuild_id,
                          uint16_t* sys_status, uint16_t* app_status,
                          uint32_t* nfw_id, uint32_t* nbuild_id);


/**
 * @brief To request for device KN number
 *
 * @param[in] dev_idx connected device ID.
 * @param[in] kn_num the pointer to store KN number
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_get_kn_number(int dev_idx, uint32_t *kn_num);


/**
 * @brief To request model IDs information for models in DDR or Flash
 *
 * @param[in] dev_idx connected device ID.
 * @param[in] from_ddr query models in ddr (1) or flash (0)
 * @param[out] data_buf the pointer to store model info
 *                 data: total_number(4 bytes) + model_id_1(4 bytes) + model_id_2(4 bytes)
 *
 * @return 0 on succeed, -1 on failure
 * @note caller must allocate memory for data_buf.
 */
int kdp_get_model_info(int dev_idx, int from_ddr, char *data_buf);


/**
 * @brief To set a customized key
 *
 * @param[in] dev_idx connected device ID
 * @param[in] ckey the key to program
 * @param[out] set_status status code
 *                   0xFFFF: command not supported in this FW
 *                   0x0   : OK
 *                   0x1   : cannot burn eFuse
 *                   0x2   : eFuse protected
 *
 * @return 0 on succeed, -1 on failure
 * @note
 * WARNING!!! This API is only for ODM/OEM company
 */
int kdp_set_ckey(int dev_idx, uint32_t ckey, uint32_t *set_status);


/**
 * @brief To set security boot key
 *
 * @param[in] dev_idx connected device ID
 * @param[in] entry 0~13, security key offset
 * @param[in] key key value
 * @param[out] set_status status code
 *
 * @return 0 on succeed, -1 on failure
 * @note for KL720 only
 */
int kdp_set_sbt_key(int dev_idx, uint32_t entry, uint32_t key, uint32_t *set_status);

/**
 * @brief To request for CRC info of models in DDR or Flash
 *
 * @param[in] dev_idx connected device ID. A host can connect several devices
 * @param[in] from_ddr query models in ddr (1) or flash (0)
 * @param[out] data_buf the pointer to store crc
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_get_crc(int dev_idx, int from_ddr, char *data_buf);


/**
 * @brief To request for metadata of NEF model file
 *
 * @param[in] model_data nef model data
 * @param[in] model_size size of NEF model
 * @param[out] metadata returned metadata
 *
 * @return 0 on succeed, -1 on failure
 */
int kdp_get_nef_model_metadata(char* model_data, uint32_t model_size, struct kdp_metadata_s* metadata);


/**
 * @brief To request for update firmware
 *
 * @param[in] dev_idx connected device ID
 * @param[in] module_id the module id of which the firmware to be updated
 *             0 - no operation
 *             1 - scpu module
 *             2 - ncpu module
 * @param[in] img_buf FW image buffer
 * @param[in] buf_len buffer size
 *
 * @return 0 if succeed, error code for failure
 */
int kdp_update_fw(int dev_idx, uint32_t* module_id, char* img_buf, int buf_len);


/**
 * @brief To request for update model (Deprecated)
 *
 * @param[in] dev_idx connected device ID
 * @param[in] model_id (reserved, no function) the model id to be updated
 * @param[in] model_size the size of the model
 * @param[in] img_buf the fw image buffer
 * @param[in] buf_len the buffer size
 *
 * @return 0 if succeed, error code for failure
 */
int kdp_update_model(int dev_idx, uint32_t* model_id, uint32_t model_size,
                     char* img_buf, int buf_len);

/**
 * @brief To request for update nef model
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_buf the nef model buffer
 * @param[in] buf_len the buffer size
 *
 * @return 0 if succeed, else error code
 */
int kdp_update_nef_model(int dev_idx, char* img_buf, int buf_len);


/**
 * @brief To request for update spl
 *
 * @param[in] dev_idx connected device ID
 * @param[in] mode the command mode to be exercised
 * @param[in] auth_type the authenticator type
 * @param[in] spl_size the spl fw image file size
 * @param[in] auth_key the authenticator key
 * @param[in] spl_buf the spl fw image buf
 * @param[out] rsp_code respone code
 * @param[out] spl_id the id of the spl firmware in device (post command execution)
 * @param[out] spl_build the build number of the spl firmware
 *
 * @return 0 if succeed, error code for failure
 * @note
 * WARNING!!! This API is only for ODM/OEM company
 */
int kdp_update_spl(int dev_idx, uint32_t mode,
                   uint16_t auth_type, uint16_t spl_size, uint8_t* auth_key,
                   char* spl_buf, uint32_t* rsp_code,
                   uint32_t* spl_id, uint32_t* spl_build);

//-----------------------system operation APIs end--------------------------

//-----------------------app level APIs--------------------------

/**
 * @brief Start the user sfid mode with specified threshold, image format
 *
 * @param[in] dev_idx connected device ID.
 * @param[in] img_size the required image file size will be returned for confirmation
 * @param[in] thresh threshold used to match face recognition result. range:0.0-1.0
 *              0: use default threshold
 * @param[in] img_width the width of input image
 * @param[in] img_height the height of input image
 * @param[in] format the input image format
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 * - The width of input image MUST be multiple of 2 for RGB565/YCBCR422
 * - The width of input image MUST be multiple of 4 for NIR8
 * - For foramt configuration, refer to common/include/ipc.h and find IMAGE_FORMAT_XXX
 */
int kdp_start_sfid_mode(int dev_idx, uint32_t* img_size, float thresh,
                        uint16_t width, uint16_t height, uint32_t format);


/**
 * @brief Perform the face recognition by input image with specified output
 *
 * @param[in] dev_idx connected device ID.
 * @param[in] user_id found matched user ID
 * @param[in] img_buf the image buffer
 * @param[in] buf_len the buffer size
 * @param[in,out] mask input:indicate the mask of requested data, output:responsed flags
 * @param[out] res pre-allocated memory for the specified output
 *            call kdp_get_res_size() to get result size
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_verify_user_id_generic(int dev_idx, uint16_t* user_id,
                               char* img_buf, int buf_len,
                               uint16_t* mask, char* res);


/**
 * @brief Start the user register mode
 *
 * @param[in] dev_idx connected device ID
 * @param[in] user_id the user id to be registered
 * @param[in] img_idx the image idx to be saved
 *
 * @return 0 on succeed, -1 on failure
 * @note
 * - For FDR application FW only
 * - user_id stars from 1 and limited by DB MAX configuration
 * - img_idx is limited by DB MAX configration
 */
int kdp_start_reg_user_mode(int dev_idx, uint16_t usr_id, uint16_t img_idx);


/**
 * @brief Get result mask with bit flags
 *
 * @param[in] fd checked if need face detection output
 * @param[in] lm checked if need face detection output
 * @param[in] fr checked if need feature map of a face
 * @param[in] liveness checked if need liveness detection output (Beta)
 * @param[in] score checked if need the recognition score output
 *
 * @return mask of bit flags
 * @note
 * - For FDR application FW only
 */
uint16_t kdp_get_res_mask(bool fd, bool lm, bool fr, bool liveness, bool score);


/**
 * @brief Get result size for memory allocation
 *
 * @param[in] fd checked if need face detection output
 * @param[in] lm checked if need face detection output
 * @param[in] fr checked if need feature map of a face
 * @param[in] liveness checked if need liveness detection output (Beta)
 * @param[in] score checked if need the recognition score output
 *
 * @return result size in bytes
 * @note
 * - For FDR application FW only
 */
uint32_t kdp_get_res_size(bool fd, bool lm, bool fr, bool liveness, bool score);


/**
 * @brief To extract face feature from image with specified output mask
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_buf the image buffere
 * @param[in] buf_len the image buffer size
 * @param[in,out] mask input:indicate the mask of requested data, output:updated mask
 * @param[out] res preallocated memory for the specified output
 *            call kdp_get_res_size() to get result size
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_extract_feature_generic(int dev_idx,
                                char* img_buf, int buf_len,
                                uint16_t* mask, char* res);


/**
 * @brief To register the extracted face features to DB in device Flash
 *
 * @param[in] dev_idx connected device ID
 * @param[in] user_id the user id that be registered.
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 * - The mentioned feature map can be extractd by kdp_extract_feature_generic()
 * - user_id must be same as the used in kdp_start_reg_user_mode()
 */
int kdp_register_user(int dev_idx, uint32_t user_id);


/**
 * @brief To remove user from device DB
 *
 * @param[in] dev_idx connected device ID
 * @param[in] user_id the user to be removed. 0 for all users
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 * - Must be called after kdp_start_sfid_mode() is called
 */
int kdp_remove_user(int dev_idx, uint32_t user_id);


/**
 * @brief To test if user in device DB
 *
 * @param[in] dev_idx connected device ID
 * @param[in] user_id the user to be listed.(starts from 1)
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 * - Must be called after kdp_start_sfid_mode() is called
 */
int kdp_list_users(int dev_idx, int user_id);


/**
 * @brief To export from DB image from device
 *
 * @param[in] dev_idx connected device ID
 * @param[out] p_buf an output pointer for the allocated memory with the exported DB
 * @param[out] p_len DB size
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 * - Must be called after kdp_start_sfid_mode() is called
 */
int kdp_export_db(int dev_idx, char **p_buf, uint32_t *p_len);


/**
 * @brief To import customer DB image to device
 *
 * @param[in] dev_idx connected device ID
 * @param[in] buf the customer's image buffer pointer
 * @param[in] p_len image size in buffer
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 * - Must be called after kdp_start_sfid_mode() is called
 */
int kdp_import_db(int dev_idx,  char* p_buf, uint32_t p_len);


/**
 * @brief To register user by feature map to device DB
 *
 * @param[in] dev_idx connected device ID
 * @param[in] user_id the user to be register. start from 1
 * @param[in] fm feature map data to register
 * @param[in] fm_len feature map data size in byte
 *
 * @return fm index on succeed, -1 on failure
 * @note
 * - For FDR application FW only
 */
int kdp_register_user_by_fm(int dev_idx, uint32_t user_id, char* fm, int fm_len);


/**
 * @brief To query user's feature map from device DB
 *
 * @param[in] dev_idx connected device ID
 * @param[out] fm the buffer to store queried fm data
 * @param[in] user_id the user id to be queried. start from 1
 * @param[in] face_id the fm id to be queried. start from 1
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_query_fm_by_user(int dev_idex, char* fm,
                         uint32_t user_id, uint32_t face_id);


/**
 * @brief Calculate similarity of two feature points
 *
 * @param[in] user_fm_a buffer A of user feature map data
 * @param[in] user_fm_b buffer B of user feature map data
 * @param[in] fm_size   size of user feature map data
 *
 * @return similarity score: smaller score meas more similar
 *         errcode -1:parameter error
 * @note  must ensure buffer A and B are the same size
 */
float kdp_fm_compare(float *user_fm_a, float *user_fm_b, size_t fm_size);

/**
 * @brief Configurate DB structure
 *
 * @param[in] dev_idx connected device ID
 * @param[in] db_config configuration data
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 * - WARNING !!! After calling this API, DB will be removed and reformatted
 * DB structure by user configuration
 */
int kdp_set_db_config(int dev_idx, kdp_db_config_t* db_config);

/**
 * @brief Get DB structure configuration
 *
 * @param[in] dev_idx connected device ID
 * @param[out] db_config return configuration data
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_get_db_config(int dev_idx, kdp_db_config_t* db_config);

/**
 * @brief Get current DB index
 *
 * @param[in] dev_idx connected device ID
 * @param[out] db_idx return current DB index
 *
 * @return current DB index
 * @note
 * - For FDR application FW only
 */
int kdp_get_db_index(int dev_idx, uint32_t *db_idx);

/**
 * @brief Switch current DB index
 *
 * @param[in] dev_idx connected device ID
 * @param[in] db_idx index of target db
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_switch_db_index(int dev_idx, uint32_t db_idx);

/**
 * @brief Set DB version number
 *
 * @param[in] dev_idx connected device ID
 * @param[in] db_version version number of DB
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_set_db_version(int dev_idx, uint32_t db_version);

/**
 * @brief Get DB version number
 *
 * @param[in] dev_idx connected device ID
 * @param[out] db_version return DB version number
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_get_db_version(int dev_idx, uint32_t *db_version);

/**
 * @brief get DB meta data version number for DB schema confirmation
 *
 * @param[in] dev_idx connected device ID
 * @param[out] db_meta_data_version return DB meta data version number
 *
 * @return 0 on succeed, error code on failure
 * @note
 * - For FDR application FW only
 */
int kdp_get_db_meta_data_version(int dev_idx, uint32_t *db_meta_data_version);

//-----------------------app level APIs end-------------------------------------

//-----------------------image streaming interface APIs --------------------------
/**
 * @brief start the user isi mode with specified app id and return data size
 *
 * @param[in] dev_idx connected device ID
 * @param[in] app_id application id. Refer to common/include/kapp_id.h
 * @param[in] return_size this size reserved for ISI result
 * @param[in] img_width the width of input image
 * @param[in] img_height the height of input image
 * @param[in] format the input image format
 * @param[out] rep_code response code
 * @param[out] buf_size the depth of the image buffer will be returned.
 *
 * @return 0 on succeed, error code on failure
 * @note
 *  For format, refer to common/include/inp.h and find IMAGE_FORMAT_XXX
 */
int kdp_start_isi_mode(int dev_idx, uint32_t app_id, uint32_t return_size,
                       uint16_t width, uint16_t height, uint32_t format,
                       uint32_t* rsp_code, uint32_t* buf_size);

/**
 * @brief start the user isi mode with isi configuration
 *
 * @param[in] dev_idx connected device ID
 * @param[in] isi_cfg isi configuration data
 * @param[in] cfg_size isi configuration data size
 * @param[out] rsp_code response code from device
 * @param[out] buf_size the depth of the image buffer will be returned.
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_start_isi_mode_ext(int dev_idx,
                           char* isi_cfg, int cfg_size,
                           uint32_t* rsp_code, uint32_t* buf_size);


/**
 * @brief Start an inference with an image
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_buf the image buffer
 * @param[in] buf_len the buffer size
 * @param[in] img_id the sequence id of the image
 * @param[out] rsp_code response code from device
 * @param[out] img_buf_available the number of image buffer still available for input
 *
 * @return 0 on succeed, error code on failure
 * @note Before calling this API, must call kdp_start_isi() first
 */
int kdp_isi_inference(int dev_idx, char* img_buf, int buf_len, uint32_t img_id,
                      uint32_t* rsp_code, uint32_t* img_buf_available);


/**
 * @brief To request for getting an inference results
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_id sequence id to get inference results of an image with the specified id
 * @param[out] rsp_code response code from device
 * @param[out] r_size inference data size
 * @param[out] r_res inference result data
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_isi_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code,
                         uint32_t* r_size, char* r_data);


/**
 * @brief To configure the model for the supported app id
 *
 * @param[in] dev_idx connected device ID
 * @param[in] model_id model id to run image inference
 * @param[in] param the parameter needed for the model
 * @param[out] rsp_code response code from ISI command handler on device
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_isi_config(int dev_idx, uint32_t model_id, uint32_t param, uint32_t *rsp_code);


/**
 * @brief request for ending isi mode
 *
 * @param[in] dev_idx connected device ID
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_end_isi(int dev_idx);


/**
 * @brief To configure for jpeg enc
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_seq image sequential number
 * @param[in] width width of the image
 * @param[in] height height of the image
 * @param[in] fmt input YUV image format
 * @param[in] quality jpeg encoding quality 0 ~ 100, normally 70~75 is recommended
 *
 * @return 0 on succeed, error code on failure
 * @note for KL720 only
 */
int kdp_jpeg_enc_config(int dev_idx, int img_seq, int width, int height, int fmt, int quality);


/**
 * @brief Start jpeg encoding
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_seq image sequential number
 * @param[in] in_img_buf input YUV image buffer
 * @param[in] in_img_len input image size in bytes
 * @param[out] out_img_buf returned encoding ouput jpeg buffer address in SCPU (not host address)
 * @param[out] out_img_len returned encoding output valid length, host can use this size to allocate memory to retrieve jpeg data from SCPU
 *
 * @return 0 on succeed, error code on failure
 * @note for KL720 only
 */
int kdp_jpeg_enc(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len);


/**
 * @brief Retrieve jpeg encoding output
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_seq image sequential number
 * @param[out] rsp_code return code
 * @param[out] r_size returned result size (bytes)
 * @param[out] r_data returned data buffer (host address)
 *
 * @return 0 on succeed, error code on failure
 * @note for KL720 only
 */
int kdp_jpeg_enc_retrieve_res(int dev_idx, uint32_t img_seq,
                              uint32_t* rsp_code, uint32_t* r_size, char* r_data);


/**
 * @brief Configure for jpeg decoding
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_seq image sequential number
 * @param[in] width width of the decoded output image
 * @param[in] height height of the decoded ouput image
 * @param[in] fmt decoded output YUV image format
 * @param[in] len input jpeg valid length in bytes
 *
 * @return 0 on succeed, error code on failure
 * @note for KL720 only
 */
int kdp_jpeg_dec_config(int dev_idx, int img_seq,
                        int width, int height,
                        int fmt, int len);


/**
 * @brief start jpeg decoding
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_seq image sequential number
 * @param[in] in_img_buf input JPEG image buffer
 * @param[in] in_img_len input image size in bytes
 * @param[out] out_img_buf returned encoding ouput YUV buffer address in SCPU (not host address)
 * @param[out] out_img_len returned encoding output valid length,
 *                     host can use this size to allocate memory to retrieve jpeg data from SCPU
 *
 * @return 0 on succeed, error code on failure
 * @note For KL720 only
 */
int kdp_jpeg_dec(int dev_idx, int img_seq,
                 uint8_t *in_img_buf, uint32_t in_img_len,
                 uint32_t *out_img_buf, uint32_t *out_img_len);


/**
 * @brief To retrieve jpeg decoding output
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_seq image sequential number
 * @param[out] rsp_code return code
 * @param[out] r_size returned result size (bytes)
 * @param[out] r_data returned data buffer (host address)
 *
 * @return 0 on succeed, error code on failure
 * @note For KL720 only
 */
int kdp_jpeg_dec_retrieve_res(int dev_idx, uint32_t img_seq,
                              uint32_t* rsp_code, uint32_t* r_size, char* r_data);

//-----------------------image streaming interface APIs end-------------------------------------


//-----------------------dynamic model execution(DME) APIs --------------------------
/**
 * @brief To request for starting dynamic model execution (Deprecated)
 *
 * @param[in] dev_idx connected device ID
 * @param[in] model_size size of inference model
 * @param[in] data firmware setup data
 * @param[in] dat_size setup data size
 * @param[out] ret_size returned model size
 * @param[in] img_buf the model file buffer
 * @param[in] buf_len the buffer size
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_start_dme(int dev_idx, uint32_t model_size, char* data, int dat_size,
                  uint32_t* ret_size, char* img_buf, int buf_len);


/**
 * @brief To request for starting dynamic model execution
 *
 * @param[in] dev_idx connected device ID
 * @param[in] model_data NEF model data
 * @param[in] model_size size of nef model
 * @param[out] ret_size returned model size
 *
 * @return 0 on succeed, error code on failure
 * @note
 *  Only support NEF model file with 1 model
 *  Composed model set is not supported
 */
int kdp_start_dme_ext(int dev_idx,
                      char* nef_model_data, uint32_t model_size,
                      uint32_t* ret_size);


/**
 * @brief To request for configuring dme
 *
 * @param[in] dev_idx connected device ID
 * @param[in] data inference setup data
 * @param[in] dat_size the size of setup data
 * @param[out] ret_model_id the return value of model id for this configuration
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_dme_configure(int dev_idx, char* data, int dat_size, uint32_t* ret_model_id);


/**
 * @brief To do inference with provided model
 *
 * @param[in] dev_idx connected device ID
 * @param[in] img_buf the image buffer
 * @param[in] buf_len the buffer size
 * @param[in] inf_size the size of inference result in DME 'serial mode'
 *                     the session id of the image in DME 'async mode'
 * @param[out] res_flag indicate whether result is requested and available
 * @param[out] inf_res contains the returned inference result
 * @param[in] mode running mode: 0:'serial' or 1: 'async mode'
 * @param[in] model_id the model id for this configuration
 *
 * @return 0 on succeed, error code on failure
 * @note Must call kdp_start_dme() and kdp_dme_configure() to configure the dme model.
 */
int kdp_dme_inference(int dev_idx, char* img_buf, int buf_len, uint32_t* inf_size,
                      bool* res_flag, char* inf_res, uint16_t mode, uint16_t model_id);


/**
 * @brief To request for retrieving DME result
 *
 * @param[in] dev_idx connected device ID
 * @param[in] addr the ddr address to retrieve
 * @param[in] len the size of data to retrieve
 * @param[out] inf_res contains the retrieving result
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_dme_retrieve_res(int dev_idx, uint32_t addr, int len, char* inf_res);


/**
 * @brief To request for getting DME inference status
 *
 * @param[in] dev_idx connected device ID
 * @param[out] ssid ssid to get inference status
 * @param[out] status inference status, 0 for not ready, 1 for ready
 * @param[out] inf_size inference data size
 * @param[out] inf_res inference result data
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_dme_get_status(int dev_idx, uint16_t *ssid, uint16_t *status,
                       uint32_t* inf_size, char* inf_res);


/**
 * @brief request for ending dme mode
 *
 * @param[in] dev_idx connected device ID
 *
 * @return 0 on succeed, error code on failure
 */
int kdp_end_dme(int dev_idx);

//-----------------------dynamic model execution APIs end --------------------------

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

#endif
