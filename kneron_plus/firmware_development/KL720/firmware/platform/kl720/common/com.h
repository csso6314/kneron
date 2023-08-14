/**
 * @file      com.h
 * @brief     Basic communcation structure
 * @copyright (c) 2018 Kneron Inc. All right reserved.
 */
#ifndef __COM_H__
#define __COM_H__

#include <stdint.h>

typedef enum {
    CMD_NONE = 0,
    CMD_MEM_READ,
    CMD_MEM_WRITE,
    CMD_DATA,
    CMD_ACK_NACK,
    CMD_STS_CLR,
    CMD_MEM_CLR,
    CMD_CRC_ERR,
    CMD_TEST_ECHO,
    CMD_FILE_WRITE,
    CMD_FLASH_MEM_WRITE,  // single sector flash write

    CMD_RESET = 0x20,
    CMD_SYSTEM_STATUS,
    CMD_UPDATE_FW,
    CMD_UPDATE_MODEL,
    /* snapshot  */
    #if CFG_SNAPSHOT_ENABLE == YES
    CMD_SNAPSHOT,
    CMD_SNAPSHOT_CHECK,
    #endif
    CMD_RECEIVE_IMAGE, //0x26
    CMD_SIM_START, //0x27
    CMD_EXTRA_MAP, //0x28
    CMD_EXTRA_MAP_CHECK, //0x29
    CMD_E2E_SET_MODE,
    CMD_E2E_FACE_ADD,
    CMD_E2E_FACE_RECOGNITION,
    CMD_E2E_FACE_RECOGNITION_TEST,
    CMD_E2E_FACE_LIVENESS,
    CMD_E2E_FACE_PRE_ADD,

    CMD_RESERVED,
    CMD_GET_KN_NUM,
    CMD_GET_MODEL_INFO,
    CMD_SET_CKEY,
    CMD_GET_CRC,
    CMD_SET_SBT_KEY,

    CMD_QUERY_APPS = 0x30,
    CMD_SELECT_APP,
    CMD_SET_MODE,
    CMD_SET_EVENTS,
    CMD_UPDATE,
    CMD_IMG_RESULT,  // only RESP message is used.  No CMD version is implemented
    CMD_ABORT,

    CMD_SFID_START = 0x108,
    CMD_SFID_NEW_USER,
    CMD_SFID_ADD_DB,
    CMD_SFID_DELETE_DB,
    CMD_SFID_SEND_IMAGE,
    CMD_SFID_LW3D_START,
    CMD_SFID_LW3D_IMAGE,
    CMD_SFID_EDIT_DB,
    CMD_SFID_DB_CONFIG,
   
    CMD_DME_START = 0x118,
    CMD_DME_CONFIG,
    CMD_DME_SEND_IMAGE,
    CMD_DME_GET_STATUS,

    CMD_ISI_START = 0x138,
    CMD_ISI_SEND_IMAGE,
    CMD_ISI_GET_RESULTS,
    CMD_ISI_CONFIG,

    CMD_SI_START = 0x140,
    CMD_SI_STOP,
    CMD_SI_CONFIG,

    CMD_HICO_START = 0x148,
    CMD_HICO_SEND_IMAGE,
    CMD_HICO_GET_RESULTS,
    CMD_HICO_STOP,

    /* Jpeg encoding */
    CMD_JPEG_ENC_CONFIG = 0x200,
    CMD_JPEG_ENC_START,
    CMD_JPEG_ENC_GET_RESULT,
    CMD_JPEG_DEC_CONFIG,
    CMD_JPEG_DEC_START,
    CMD_JPEG_DEC_GET_RESULT,

    /* Tof encoding */
    CMD_TOF_DEC_CONFIG,
    CMD_TOF_DEC_START,
    CMD_TOF_DEC_GET_RESULT,

    // Flash command
    CMD_FLASH_INFO = 0x1000,
    CMD_FLASH_CHIP_ERASE,
    CMD_FLASH_SECTOR_ERASE,
    CMD_FLASH_READ,
    CMD_FLASH_WRITE,

    /* vincent2do: group similar cmds */
    // for Camera tool
    CMD_DOWNLOAD_IMAGE_NIR = 0x2100,
    CMD_DOWNLOAD_IMAGE_RGB,
    CMD_SET_NIR_AGC,
    CMD_SET_NIR_AEC,
    CMD_SET_RGB_AGC,
    CMD_SET_RGB_AEC,
    CMD_SET_NIR_LED,
    CMD_SET_RGB_LED,
    CMD_CAM_CONNECT,
    CMD_NIR_FD_RES,
    CMD_NIR_LM_RES,
    CMD_RGB_FD_RES,
    CMD_RGB_LM_RES,
    CMD_NIR_GET_CONF,
    CMD_RGB_GET_CONF,

    // MP commands
    CMD_MP_GPIO_SET = 0x7000,
} Cmd;

// legacy 720 commands
typedef enum {
    CMD_ID_MEM_WRITE = 0x1,
    CMD_ID_MEM_READ = 0x2,
    CMD_ID_RUN_YOLO3 = 0x3,
    CMD_ID_STOP_YOLO_EXEC = 0x04,
    CMD_ID_NOTIFY_FW_INFO_LOADED = 0x05,
    CMD_ID_RUN_JPEG_ENC = 0x06,
    CMD_ID_RUN_JPEG_DEC = 0x07,
    CMD_ID_RUN_CROP_RESIZE = 0x08,
    CMD_ID_RUN_FD = 0x09,
    CMD_ID_RUN_LM,
    CMD_ID_RUN_FR,
    CMD_ID_RUN_CLASSIFY,        // 0x0c
    CMD_ID_MAX,
} presilicon_test_cmd_t;

// legacy 720 command structure
typedef struct LegacyMsg_s{
    uint32_t cmd;
    uint32_t arg1;
    uint32_t arg2;
} __attribute__((packed)) LegacyMsg;

typedef struct {
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
} __attribute__((packed)) MsgHdr;

typedef struct {
    uint32_t param1;
    uint32_t param2;
    uint8_t data[];
} __attribute__((packed)) CmdPram;

typedef struct {
    union {
        uint32_t error;
        uint32_t param1;
    } __attribute__((packed));
    uint32_t param2;
    uint8_t data[];
} __attribute__((packed)) RspPram;

#define MSG_HDR_CMD     0xA583
#define MSG_HDR_RSP     0x8A35
#define MSG_HDR_VAL     0xA553  // this is used by the pre-packet code
#define MSG_HDR_SIZE    16  // includes both MsgHdr and CmdPram addr & len
#define PKT_CRC_FLAG    0x4000

#if (HAPS_ID == 2)
#define MSG_DATA_BUF_MAX    0x890
#else
#define MSG_DATA_BUF_MAX    0x2800  // used for testing (worst case msg payload is 4096+72)
#endif

#define UPDATE_MODULE_NONE 0
#define UPDATE_MODULE_SCPU 1
#define UPDATE_MODULE_NCPU 2

#endif
