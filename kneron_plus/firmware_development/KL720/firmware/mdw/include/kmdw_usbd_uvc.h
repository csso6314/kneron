/**
 * @file        kmdw_usbd_uvc.h
 * @brief       USB device mode UVC middleware
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_USBD_UVC_H__
#define __KMDW_USBD_UVC_H__

#include "kmdw_usbh2.h"
#include "kmdw_status.h"

/* Application-dependent defines */
#define UVC_USBD_VID                            0x3231          /**< Application-dependent USB VID */
#define UVC_USBD_PID                            0x1720          /**< Application-dependent USB PID */

#define UVC_VIDEO_CONTROL_EP                    0x81            /**< Application-dependent UVC Endpoint Number */
#define UVC_VIDEO_STREAM_EP                     0x82            /**< Application-dependent UVC Endpoint Number */

#define UVC_STREAMING_ONE_TXF_SIZE              0x1000          /**< Adjust this size to balance transfer speed vs DDR space consumption */

#define UVC_IMG_WIDTH                           640             /**< Application-dependent UVC frame width */
#define UVC_IMG_HEIGHT                          480             /**< Application-dependent UVC frame height */
#define UVC_BYTE_PER_PIXEL                      2               /**< Application-dependent UVC frame byte-per-pixel */

/* Video Class-Specific Request Codes */
#define UVC_FRAME_FORMAT_Y                     0               /**< Frame format: Y only */
#define UVC_FRAME_FORMAT_YUY2                  1               /**< Frame format: YUY2 */

#define UVC_FRAME_SIZE                          (UVC_IMG_WIDTH*UVC_IMG_HEIGHT*UVC_BYTE_PER_PIXEL)   /**< Video frame size */

#define UVC_FRAME_INTERVAL_10FPS                (10000000U / 10U)   /**< 10fps interval */
#define UVC_FRAME_INTERVAL_20FPS                (10000000U / 20U)   /**< 20fps interval */
#define UVC_FRAME_INTERVAL_25FPS                (10000000U / 25U)   /**< 25fps interval */

#define UVC_BITRATE_MIN                         UVC_FRAME_SIZE*8*10           /**< Video bitrate minimum size*/
#define UVC_BITRATE_MAX                         UVC_FRAME_SIZE*8*25           /**< Video bitrate maximum size*/
/* UVC-related defines */
#define UVC_USBD_NUM_CONFIG                     1               /**< UVC uses only 1 configuration */
#define UVC_USBD_LANGID                         0x409           /**< English */

#define UVC_INTERFACE_VC                        0x00            /**< Video Control interface id. */
#define UVC_INTERFACE_VS                        0x01            /**< Video Streaming interface id. */
#define UVC_EXTENSION_ID                        0x03            /**< Video Extension unit id. */

#define UVC_REQ_UNDEFINED                       0x00            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_SET_CUR                         0x01            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_GET_CUR                         0x81            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_GET_MIN                         0x82            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_GET_MAX                         0x83            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_GET_RES                         0x84            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_GET_LEN                         0x85            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_GET_INFO                        0x86            /**<  Video Class-Specific Request Codes */
#define UVC_REQ_GET_DEF                         0x87            /**<  Video Class-Specific Request Codes */

/* VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_UNDEFINED                        0x00            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_PROBE_CONTROL                    0x01            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_COMMIT_PROBE                     0x02            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_STILL_PROBE_CONTROL              0x03            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_STILL_COMMIT_CONTROL             0x04            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_STILL_IMAGE_TRIGGER_CONTROL      0x05            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_STREAM_ERROR_CODE_CONTROL        0x06            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_GENERATE_KEY_FRAME_CONTROL       0x07            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL     0x08            /**< VideoStreaming Interface Control Selectors (wValue) */
#define UVC_VS_SYNCH_DELAY_CONTROL              0x09            /**< VideoStreaming Interface Control Selectors (wValue) */

#define UVC_VS_PAYLOAD_MAX_HEADER_SIZE          12              /**< UVC payload header size */
#define UVC_PROBE_CONTROL_LEN                   0x22            /**< UVC probe control data len for UVC 1.1 */ 

/* Camera Terminal Control Selectors */
#define UVC_CT_UNDEFINED                        0x00            /**< Camera Terminal Control Selectors */
#define UVC_CT_SCANNING_MODE_CONTROL            0x01            /**< Camera Terminal Control Selectors */
#define UVC_CT_AE_MODE_CONTROL                  0X02            /**< Camera Terminal Control Selectors */
#define UVC_CT_AE_PRIORITY_CONTRO               0X03            /**< Camera Terminal Control Selectors */
#define UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL   0X04            /**< Camera Terminal Control Selectors */
#define UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL   0X05            /**< Camera Terminal Control Selectors */

/**
 * @brief   UVC middleware video payload header
 * 
 */
typedef struct {
    uint8_t bHeaderLen;                         /**< header length */
    struct {                                    /**< structure definition */
        uint8_t frame_id:1;                     /**< frame id*/
        uint8_t end_of_frame:1;                 /**< is end of frame */
        uint8_t time_present:1;                 /**< is time data present */
        uint8_t clock_present:1;                /**< is clock data present */
        uint8_t rsv:1;                          /**< reserved */
        uint8_t still_img:1;                    /**< is still image */
        uint8_t error:1;                        /**< is error */
        uint8_t end_of_header:1;                /**< is end of frame */
    }bHeaderInfo;
    uint8_t dwPresentationTime[4];              /**< time data */
    uint8_t srcSourceClock[6];                  /**< clock data */
}kmdw_uvc_video_payload_header_t;

/**
 * @brief   UVC middleware status
 * 
 */
typedef enum{
    KMDW_USBD_UVC_OK,                           /**< Enum 0, Status OK */
    KMDW_USBD_UVC_ERROR                         /**< Enum 1, Status Error */
}kmdw_usbd_uvc_status_t;

/**
 * @brief   UVC middlware USB link status
 * 
 */
typedef enum{
    KMDW_USBD_UVC_DISCONNECTED,                 /**< Enum 0, Status Disconnected */
    KMDW_USBD_UVC_CONNECTED,                    /**< Enum 1, Status Connected */
    KMDW_USBD_UVC_OPENED,                       /**< Enum 2, Status Opened */
    KMDW_USBD_UVC_CLOSED                        /**< Enum 3, Status Closed */
}kmdw_usbd_uvc_link_status_t;

/**
 * @brief   UVC middlware USB link speed
 * 
 */
typedef enum
{
    KMDW_USBD_UVC_NO_LINK = 0,              /**< Enum 0, USB3 no link speed */
    KMDW_USBD_UVC_HIGH_SPEED,               /**< Enum 1, USB3 high speed */
    KMDW_USBD_UVC_SUPER_SPEED               /**< Enum 2, USB3 super speed */
}kmdw_usbd_uvc_speed_t;

/**
 * @brief   UVC middleware callback definition
 * 
 */
typedef struct{
    void (*kmdw_usbd_uvc_link_status)(kmdw_usbd_uvc_link_status_t status);      /**< callback function for the link status update */
}kmdw_usbd_uvc_callbacks_t;

/**
 * @brief   UVC middleware config definition
 * 
 */
typedef struct{
    uint16_t frame_width;                   /**< frame width */
    uint16_t frame_height;                  /**< frame height */
    uint8_t frame_byte_per_pixel;           /**< frame byte-per-pixel */
    uint8_t frame_format;                   /**< frame format */
}kmdw_usbd_uvc_config_t;

/**
 * @brief   UVC middlware USB link speed
 * 
 */
typedef enum
{
    KMDW_USBD_UVC_FRAME_FLAG_NONE = 0,      /**< Enum 0, Frame Middle */
    KMDW_USBD_UVC_FRAME_FLAG_SOF = 1,       /**< Enum 1, Frame Start */
    KMDW_USBD_UVC_FRAME_FLAG_EOF = 2,       /**< Enum 2, Frame End */
    KMDW_USBD_UVC_FRAME_FLAG_SOF_EOF = 3    /**< Enum 2, Frame Start and End */
}kmdw_usbd_uvc_frame_flag_t;

/**
 * @brief   UVC middlware Defined Bits Containing Capabilities of the Control for GET_INFO
 * 
 */
typedef union
{
    uint8_t val;                    /**< union value */
    struct{                         /**< struct definition */
        uint8_t get:1;              /**< 1=Supports GET value request */
        uint8_t set:1;              /**< 1=Supports SET value request */
        uint8_t disable:1;          /**< 1=Disabled due to automatic mode */
        uint8_t auto_ctl:1;         /**< 1=Autoupdate Control */
        uint8_t asyc_ctl:1;         /**< 1=Asynchronous Control */
        uint8_t rsv:3;              /**< Reserved(set to 0) */
    };
}kmdw_usbd_uvc_info_attribute_t;

/**
 * @brief   UVC video format
 * 
 */
typedef struct
{
    uint32_t width;                 /**< width in pixel */
    uint32_t height;                /**< height in pixel */
    uint32_t byte_per_pixel;        /**< byte count per pixel */
}__attribute__((packed)) kmdw_usbd_uvc_format_t;

/**
 * @brief   UVC probe control format for UVC1.1
 * 
 */
typedef struct
{
    uint16_t bmHint;                        /**< Bitfield control */
    uint8_t bFormatIndex;                   /**< Video format index from a format descriptor */
    uint8_t bFrameIndex;                    /**< Video frame index from a frame descriptor */
    uint32_t dwFrameInterval;               /**< Frame interval in 100 ns units */
    uint16_t wKeyFrameRate;                 /**< Key frame rate in key-frame per video frame unit */
    uint16_t wPFrameRate;                   /**< PFrame rate in PFrame/key frame units */
    uint16_t wCompQuality;                  /**< Compression quality control in abstract units 0 (lowest) to 10000 (highest) */
    uint16_t wCompWindowSize;               /**< Window size for average bit rate control */
    uint16_t wDelay;                        /**< Internal video streaming interface latency in ms from video data capture to presentation on the USB */
    uint32_t dwMaxVideoFrameSize;           /**< Maximum video frame or codec specific segment size in bytes */
    uint32_t dwMaxPayloadTransferSize;      /**< The maximum number of bytes that the device can transmit or receive in a single payload transfer */
    uint32_t dwClockFrequency;              /**< Device clock frequency in Hz for the specified format */
    uint8_t bmFramingInfo;                  /**< Bitfield control for frame info */
    uint8_t bPreferedVersion;               /**< Preferred payload format version supported by the host or device for the specified bFormatIndex value */
    uint8_t bMinVersion;                    /**< Minimum payload format version supported by the device for the specified bFormatIndex value */
    uint8_t bMaxVersion;                    /**< Maximum payload format version supported by the device for the specified bFormatIndex value */
}__attribute__((packed)) kmdw_usbd_uvc_probe_ctl_1_1_t;

/**
 * @brief           Initialize the UVC middleware
 * 
 * @param           cfg                             User created configuration, see @ref kmdw_usbd_uvc_callbacks_t
 * @param           cb                              User created callback setting, see @ref kmdw_usbd_uvc_callbacks_t
 * @return          kmdw_usbd_uvc_status_t          see @ref kmdw_usbd_uvc_status_t
 */
kmdw_usbd_uvc_status_t kmdw_usbd_uvc_init(kmdw_usbd_uvc_config_t *cfg, kmdw_usbd_uvc_callbacks_t *cb);

/**
 * @brief           Deinit the UVC middleware
 * 
 * @return          kmdw_usbd_uvc_status_t          see @ref kmdw_usbd_uvc_status_t 
 */
kmdw_usbd_uvc_status_t kmdw_usbd_uvc_deinit(void);

/**
 * @brief           Get the USB link status 
 * 
 * @return          kmdw_usbd_uvc_link_status_t     see @ref kmdw_usbd_uvc_link_status_t
 */
kmdw_usbd_uvc_link_status_t kmdw_usbd_get_link_status(void);

/**
 * @brief           Get the USB link speed 
 * 
 * @return          kmdw_usbd_uvc_speed_t           see @ref kmdw_usbd_uvc_speed_t
 */
kmdw_usbd_uvc_speed_t kmdw_usbd_get_link_speed(void);

/**
 * @brief           Send a video frame from the video streaming endpoint
 * 
 * @param           frame_buf                       frame buffer for sending data
 * @param           frame_len                       frame data length to be sent
 * @param           flag                            frame flag
 * @return          kmdw_usbd_uvc_status_t          see @ref kmdw_usbd_uvc_status_t  
 */
kmdw_usbd_uvc_status_t kmdw_usbd_uvc_send_frame(uint8_t *frame_buf, uint32_t frame_len, kmdw_usbd_uvc_frame_flag_t flag);

/**
 * @brief           Send status data frame from the video status endpoint
 * 
 * @param           data                            buffer for sending status data
 * @param           len                             data length to be sent
 * @return          kmdw_usbd_uvc_status_t          see @ref kmdw_usbd_uvc_status_t  
 */
kmdw_usbd_uvc_status_t kmdw_usbd_uvc_send_status(uint8_t *data, uint32_t len);

/**
 * @brief           Returns the pointer to array of kmdw_usbd_uvc_format_t
 * 
 * @param           cnt                             count pointer to return the total count of formats in the array
 * @return          kmdw_usbd_uvc_format_t*         see @ref kmdw_usbd_uvc_format_t  
 */
kmdw_usbd_uvc_format_t *kmdw_usbd_uvc_get_support_formats(uint8_t *cnt);

/**
 * @brief           Returns the current format info
 * 
 * @return          uint8_t                         the index of current format  
 */
uint8_t kmdw_usbd_uvc_get_current_format(void);
#endif
