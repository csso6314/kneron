/**
 * @file        kdp2_ipc_cmd.h
 * @brief       system/model commands
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#include <stdint.h>

#include "kp_struct.h"

// below are for usb control trasnfer
enum
{
    KDP2_CONTROL_REBOOT = 0xFF,                 // chip reboot
    KDP2_CONTROL_SHUTDOWN = 0xFE,               // chip shut down
    KDP2_CONTROL_FIFOQ_RESET = 0x80,            // make fifo queue clean and start-over
    KDP2_CONTROL_FIFOQ_GET_STATUS = 0x81,       // make firmware print fifoq status (debug only)
    KDP2_CONTROL_FIFOQ_CONFIGURE = 0x82,        // configure FIFO Queue buffer size and queue depth
    KDP2_CONTROL_FIFOQ_ENABLE_DROPPABLE = 0x83, // enable/disable droppable inference image attribute (default : disabled)
    KDP2_CONTROL_DDR_HEAP_BOUNDARY_ADJUST = 0x84, // adjust the boundary address of the ddr heap
    KDP2_CONTROL_REBOOT_SYSTEM = 0x85,          // reboot the entire system (KL630 only)
};

// below are for usb bulk command transfer
enum kdp2_command_id
{
    KDP2_COMMAND_LOAD_MODEL = 0xA01,
    KDP2_COMMAND_MEMORY_READ = 0xA02,
    KDP2_COMMAND_MEMORY_WRITE = 0xA03,
    KDP2_COMMAND_GET_SYSTEM_INFO = 0xA04,
    KDP2_COMMAND_GET_MODEL_INFO = 0xA05,
    KDP2_COMMAND_LOAD_FIRMWARE = 0xA06,
    KDP2_COMMAND_LOAD_MODEL_FROM_FLASH = 0xA07,
    KDP2_COMMAND_SET_CKEY = 0xA08,
    KDP2_COMMAND_SET_SBT_KEY = 0xA09,
    KDP2_COMMAND_SET_GPIO = 0xA0A,
    KDP2_COMMAND_SET_DBG_CHECKPOINT = 0xA0B,
    KDP2_COMMAND_SET_PROFILE_ENABLE = 0xA0C,
    KDP2_COMMAND_GET_PROFILE_STATISTICS = 0xA0D,
    KDP2_COMMAND_GET_DDR_CONFIG = 0xA0E,
    KDP2_COMMAND_LOAD_NEF = 0xA0F,          // not supported
    KDP2_COMMAND_UPDATE_FIRMWARE = 0xA10,   // not supported
    KDP2_COMMAND_SWITCH_BOOT_MODE = 0xA11,  // not supported
    KDP2_COMMAND_UPDATE_LOADER = 0xA12,     // not supported
    KDP2_COMMAND_GET_FIFOQ_CONFIG = 0xA13,
    KDP2_COMMAND_READ_FLASH = 0xA98,        // not supported
    KDP2_COMMAND_WRITE_FLASH = 0xA99,       // not supported
};

// below are for firmware serial number
enum kp_firmware_serial_t
{
    KP_KDP_FW = 0x01,                               /**< 00000001 */

    /* ======================= Kdp2 Legacy ========================== */

    KP_KDP2_FW = 0x80,                              /**< 1******* */

    KP_KDP2_FW_USB_TYPE = 0x80,                     /**< 1*****00 */
    KP_KDP2_FW_FLASH_TYPE = 0x81,                   /**< 1*****01 */
    KP_KDP2_FW_JTAG_TYPE = 0x82,                    /**< 1*****10 */
    KP_KDP2_FW_LOADER = 0x83,                       /**< 1*****11 */
    KP_KDP2_FW_FIND_TYPE_MASK = 0x83,               /**< 10000011 */

    KP_KDP2_FW_KL720_USB_DFU = 0x101,               /**< 000100000001 Special Case */
    KP_KDP2_FW_KL720_LOADER = 0xBA,                 /**< 10111010 Special Case */

    KP_KDP2_FW_HOST_MODE = 0x90,                    /**< 1001**** */
    KP_KDP2_FW_HICO_MODE = 0xA0,                    /**< 1010**** */
    KP_KDP2_FW_COMPANION_MODE = 0xB0,               /**< 1011**** */
    KP_KDP2_FW_FIND_MODE_MASK = 0xF0,               /**< 11110000 */

    /* ============================================================== */

    KP_KDP2_FW_V2 = 0x400,                          /**< 01********** */

    KP_KDP2_FW_USB_TYPE_V2 = 0x400,                 /**< 01*******000 */
    KP_KDP2_FW_FLASH_TYPE_V2 = 0x401,               /**< 01*******001 */
    KP_KDP2_FW_JTAG_TYPE_V2 = 0x402,                /**< 01*******010 */
    KP_KDP2_FW_LOADER_V2 = 0x403,                   /**< 01*******011 */
    KP_KDP2_FW_FIND_TYPE_MASK_V2 = 0x407,           /**< 010000000111 */

    KP_KDP2_FW_HOST_MODE_V2 = 0x410,                /**< 01***001**** */
    KP_KDP2_FW_HICO_MODE_V2 = 0x420,                /**< 01***010**** */
    KP_KDP2_FW_COMPANION_MODE_V2 = 0x430,           /**< 01***011**** */
    KP_KDP2_FW_FIND_MODE_MASK_V2 = 0x470,           /**< 010001110000 */

    KP_KDP2_FW_RTOS_OS_V2 = 0x500,                  /**< 0101******** */
    KP_KDP2_FW_LINUX_OS_V2 = 0x600,                 /**< 0110******** */
    KP_KDP2_FW_FIND_OS_MASK_V2 = 0x700,             /**< 011100000000 */
};

#define FW_TYPE_SCPU 0x1
#define FW_TYPE_NCPU 0x2

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_LOAD_FIRMWARE'

    uint32_t fw_type;       // FW_TYPE_SCPU or FW_TYPE_NCPU
    uint32_t fw_start;      // FW code start address
    uint32_t fw_size;       // should equal to following data trasnfer size
    uint32_t fw_bypass[16]; // 64 bytes for future use, used to bypass control settings while uploading fw
} __attribute__((aligned(4))) kdp2_ipc_cmd_upload_firmware_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_LOAD_MODEL'

    uint32_t model_size;
    uint32_t fw_info_size;
    uint8_t fw_info[];
} __attribute__((aligned(4))) kdp2_ipc_cmd_load_model_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_LOAD_NEF'

    uint32_t nef_size;
} __attribute__((aligned(4))) kdp2_ipc_cmd_load_nef_t;

// KDP2_COMMAND_MEMORY_READ and KDP2_COMMAND_MEMORY_WRITE use the same cmd struct
typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_MEMORY_READ' or 'KDP2_COMMAND_MEMORY_WRITE'

    uint32_t start_address;
    uint32_t length;
} __attribute__((aligned(4))) kdp2_ipc_cmd_memory_read_write_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_GET_SYSTEM_INFO'

} __attribute__((aligned(4))) kdp2_ipc_cmd_get_system_info_t;

typedef struct
{
    uint32_t return_code; // KP_API_RETURN_CODE

    kp_system_info_t system_info;
} __attribute__((aligned(4))) kdp2_ipc_response_get_system_info_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_READ_FLASH'

    uint32_t flash_offset;
    uint32_t length;

} __attribute__((aligned(4))) kdp2_ipc_cmd_read_flash_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_WRITE_FLASH'

    uint32_t flash_offset; // 4KB alignment
    uint32_t length;

} __attribute__((aligned(4))) kdp2_ipc_cmd_write_flash_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_GET_MODEL_INFO'

    uint32_t from_ddr; // 0 = from flash, 1 = from ddr not working when value = 0, fix it if needed
} __attribute__((aligned(4))) kdp2_ipc_cmd_get_model_info_t;

typedef struct
{
    uint32_t return_code; // KP_API_RETURN_CODE
    uint32_t fw_info_size;
    uint32_t target_chip; // 1: KL520, 2: KL720 (Ref: kp_model_target_chip_t)
} __attribute__((aligned(4))) kdp2_ipc_response_get_model_info_fw_info_t;

typedef struct
{
    uint32_t return_code; // KP_API_RETURN_CODE
    uint32_t setup_size;
} __attribute__((aligned(4))) kdp2_ipc_response_get_model_info_setup_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_UPDATE_FIRMWARE'

    uint32_t firmware_id; // 1 = scpu, 2 = ncpu
    uint32_t firmware_size;
    uint8_t firmware_content[];
} __attribute__((aligned(4))) kdp2_ipc_cmd_update_firmware_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_UPDATE_LOADER'

    uint32_t loader_size;
} __attribute__((aligned(4))) kdp2_ipc_cmd_update_loader_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_LOAD_MODEL_FROM_FLASH'
} __attribute__((aligned(4))) kdp2_ipc_cmd_load_model_from_flash_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_SET_CKEY'
    uint32_t ckey;
} __attribute__((aligned(4))) kdp2_ipc_cmd_set_ckey_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_SET_SBT_KEY'
    uint32_t entry;
    uint32_t key;
} __attribute__((aligned(4))) kdp2_ipc_cmd_set_sbt_key_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_SET_GPIO'
    uint32_t pin;
    uint32_t value;
} __attribute__((aligned(4))) kdp2_ipc_cmd_set_gpio_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_SET_DBG_CHECKPOINT'
    uint32_t checkpoint_flags;
    bool enable;
} __attribute__((aligned(4))) kdp2_ipc_cmd_set_dbg_checkpoint_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_SET_PROFILE_ENABLE'
    bool enable;
} __attribute__((aligned(4))) kdp2_ipc_cmd_set_profile_enable_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_GET_PROFILE_STATISTICS'
} __attribute__((aligned(4))) kdp2_ipc_cmd_get_profile_statics_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_GET_DDR_CONFIG'
} __attribute__((aligned(4))) kdp2_ipc_cmd_get_available_ddr_config_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_GET_FIFOQ_CONFIG'
} __attribute__((aligned(4))) kdp2_ipc_cmd_get_fifo_queue_config_t;

typedef struct
{
    uint32_t magic_type; // should be 'KDP2_MAGIC_TYPE_COMMAND'
    uint32_t total_size; // size of this data struct
    uint32_t command_id; // should be 'KDP2_COMMAND_SWITCH_BOOT_MODE'
    uint32_t boot_mode;
} __attribute__((aligned(4))) kdp2_ipc_cmd_switch_boot_mode_t;

// below section is for old KDP firmware update, system status and get kn number commands
//////////////////////////////////////////////////////////////////

#define KDP_MSG_HDR_CMD 0xA583
#define KDP_MSG_HDR_RSP 0x8A35

#define KDP_CMD_SYSTEM_STATUS 0x21
#define KDP_CMD_SYSTEM_STATUS_RESPONSE 0x8021
#define KDP_CMD_UPDATE_FW 0x22
#define KDP_CMD_UPDATE_FW_RESPONSE 0x8022
#define KDP_CMD_UPDATE_MODEL 0x23
#define KDP_CMD_UPDATE_MODEL_RESPONSE 0x8023
#define KDP_CMD_GET_KN_NUM 0x25
#define KDP_CMD_GET_KN_NUM_RESPONSE 0x8025

// allowed to connect through passing this value as error_code parameter to kp_connect_devices()
#define KDP_MAGIC_CONNECTION_PASS 536173391

#define KDP_UPDATE_MODULE_SCPU 1
#define KDP_UPDATE_MODULE_NCPU 2

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t fw_id;
    uint32_t auto_reboot;
} __attribute__((aligned(4))) kdp_firmware_update_cmd_t;

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t rsp_code;
    uint32_t fw_id;
} __attribute__((aligned(4))) kdp_firmware_update_response_t;

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t crc;
} __attribute__((aligned(4))) kdp_get_kn_number_cmd_t;

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t kn_number;
    uint32_t dummy; // Seems not used
} __attribute__((aligned(4))) kdp_get_kn_number_response_t;

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t crc;
} __attribute__((aligned(4))) kdp_system_status_cmd_t;

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t sfirmware_id;
    uint32_t sbuild_id;
    uint16_t sys_status;
    uint16_t app_status;
    uint32_t nfirmware_id;
    uint32_t nbuild_id;
} __attribute__((aligned(4))) kdp_system_status_response_t;

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t fw_info_size;
    uint32_t all_models_size;
    uint32_t auto_reboot;
} __attribute__((aligned(4))) kdp_model_update_cmd_t;

typedef struct
{
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
    uint32_t rsp_code;
    uint32_t model_id;
} __attribute__((aligned(4))) kdp_model_update_response_t;

//////////////////////////////////////////////////////////////////
