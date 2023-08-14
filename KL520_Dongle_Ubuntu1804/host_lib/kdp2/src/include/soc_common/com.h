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
    CMD_RESERVED,
    CMD_GET_KN_NUM,
    CMD_GET_MODEL_INFO,
    CMD_SET_CKEY,
    CMD_GET_CRC,

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
    CMD_SFID_RESERVE_1,
    CMD_SFID_RESERVE_2,
    CMD_SFID_EDIT_DB,

    CMD_DME_START = 0x118,
    CMD_DME_CONFIG,
    CMD_DME_SEND_IMAGE,
    CMD_DME_GET_STATUS,

    CMD_ISI_START = 0x138,
    CMD_ISI_SEND_IMAGE,
    CMD_ISI_GET_RESULTS,
    CMD_ISI_CONFIG,

    CMD_JPEG_ENC_CONFIG = 0x200,
    CMD_JPEG_ENC,
    CMD_JPEG_ENC_GET_RESULT,
    CMD_JPEG_DEC_CONFIG,
    CMD_JPEG_DEC,
    CMD_JPEG_DEC_GET_RESULT,

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
    CMD_RGB_GET_CONF

} Cmd;

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

typedef struct
{
    MsgHdr msg_header;
    uint32_t model_size;
    uint8_t fw_info[];
} __attribute__((packed)) start_dme_header;

typedef struct
{
    MsgHdr msg_header;
    uint32_t fw_id;
} __attribute__((packed)) firmware_update_header;

typedef struct
{
    MsgHdr msg_header;
    uint32_t rsp_code;
    uint32_t fw_id;
} __attribute__((packed)) firmware_update_response;

#define MSG_HDR_CMD     0xA583
#define MSG_HDR_RSP     0x8A35
#define MSG_HDR_VAL     0xA553  // this is used by the pre-packet code
#define MSG_HDR_SIZE    16  // includes both MsgHdr and CmdPram addr & len
#define PKT_CRC_FLAG    0x4000

#endif