#include <string.h>
#include "cmsis_os2.h"
#include "kdrv_usbd3.h"
#include "kdrv_gdma.h"
#include "kmdw_memory.h"
#include "kmdw_console.h"
#include "kmdw_usbd_uvc.h"
#include "project.h"

static kmdw_usbd_uvc_callbacks_t _cbs = {0};
static uint8_t *_kmdw_usbd_uvc_frame_ptr = NULL;
static kdrv_gdma_handle_t _kmdw_usbd_uvc_gdma;
static kmdw_usbd_uvc_config_t _cfg = {0};
static uint8_t _current_format = 0;

#define KMDW_UVC_EVENT_DATA_SENT        (1 << 0)

static volatile kmdw_usbd_uvc_link_status_t _uvc_link_stauts = KMDW_USBD_UVC_DISCONNECTED;
static const uint8_t kmdw_uvc_vs_payload_header[UVC_VS_PAYLOAD_MAX_HEADER_SIZE] =
{
    0x0C,                           /* Header Length */
    0x8C,                           /* Bit field header field */
    0x00,0x00,0x00,0x00,            /* Presentation time stamp field */
    0x00,0x00,0x00,0x00,0x00,0x00   /* Source clock reference field */
};

static const uint8_t kmdw_uvc_usb_device_desc_hs[] = {
    KDRV_USB_LEN_DEV_DESC,              /* bLength */
    KDRV_USB_DESC_TYPE_DEVICE,          /* bDescriptorType */
    0x10,                               /* bcdUSB */
    0x02,
    0xEF,                               /* bDeviceClass */
    0x02,                               /* bDeviceSubClass */
    0x01,                               /* bDeviceProtocol */
    KDRV_USB_MAX_EP0_SIZE,              /* bMaxPacketSize */
    KDRV_USB_LOW_BYTE(UVC_USBD_VID),    /* idVendor */
    KDRV_USB_HIGH_BYTE(UVC_USBD_VID),   /* idVendor */
    KDRV_USB_LOW_BYTE(UVC_USBD_PID),    /* idVendor */
    KDRV_USB_HIGH_BYTE(UVC_USBD_PID),   /* idVendor */
    0x00,                               /* bcdDevice rel. 2.00 */
    0x02,
    KDRV_USBD_IDX_MFC_STR,              /* Index of manufacturer string */
    KDRV_USBD_IDX_PRODUCT_STR,          /* Index of product string */
    KDRV_USBD_IDX_SERIAL_STR,           /* Index of serial number string */
    UVC_USBD_NUM_CONFIG                 /* bNumConfigurations */
};

static const uint8_t kmdw_uvc_usb_device_desc_ss[] = {
    KDRV_USB_LEN_DEV_DESC,              /* bLength */
    KDRV_USB_DESC_TYPE_DEVICE,          /* bDescriptorType */
    0x00,                               /* bcdUSB */
    0x03,
    0xEF,                               /* bDeviceClass */
    0x02,                               /* bDeviceSubClass */
    0x01,                               /* bDeviceProtocol */
    KDRV_USB_MAX_EP0_SIZE_SS,           /* bMaxPacketSize */
    KDRV_USB_LOW_BYTE(UVC_USBD_VID),    /* idVendor */
    KDRV_USB_HIGH_BYTE(UVC_USBD_VID),   /* idVendor */
    KDRV_USB_LOW_BYTE(UVC_USBD_PID),    /* idVendor */
    KDRV_USB_HIGH_BYTE(UVC_USBD_PID),   /* idVendor */
    0x00,                               /* bcdDevice rel. 2.00 */
    0x02,
    KDRV_USBD_IDX_MFC_STR,              /* Index of manufacturer string */
    KDRV_USBD_IDX_PRODUCT_STR,          /* Index of product string */
    KDRV_USBD_IDX_SERIAL_STR,           /* Index of serial number string */
    UVC_USBD_NUM_CONFIG                 /* bNumConfigurations */
};

static uint8_t kmdw_uvc_usb_configuration_desc_hs[] = {
    /* Configuration descriptor */
    KDRV_USB_LEN_CFG_DESC,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_CONFIGURATION,        /* Configuration descriptor type */
    0xEC,0x00,                      /* Length of this descriptor and all sub descriptors */
    0x02,                           /* Number of interfaces */
    0x01,                           /* Configuration number */
    0x00,                           /* COnfiguration string index */
    0x80,                           /* Config characteristics - bus powered */
    0xFA,                           /* Max power consumption of device (in 2mA unit) : 500mA */ //8

    /* Interface association descriptor */
    0x08,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_INTERFACE_ASSOCIATION,      /* Interface association descr type */
    0x00,                           /* I/f number of first video control i/f */
    0x02,                           /* Number of video streaming i/f */
    0x0E,                           /* CC_VIDEO : Video i/f class code */
    0x03,                           /* SC_VIDEO_INTERFACE_COLLECTION : subclass code */
    0x00,                           /* Protocol : not used */
    0x00,                           /* String desc index for interface */ //16

    /* Standard video control interface descriptor */
    0x09,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_INTERFACE,        /* Interface descriptor type */
    0x00,                           /* Interface number */
    0x00,                           /* Alternate setting number */
    0x01,                           /* Number of end points */
    0x0E,                           /* CC_VIDEO : Interface class */
    0x01,                           /* CC_VIDEOCONTROL : Interface sub class */
    0x00,                           /* Interface protocol code */
    0x00,                           /* Interface descriptor string index */ //25

    /* Class specific VC interface header descriptor */
    0x0D,                           /* Descriptor size */
    0x24,                           /* Class Specific I/f header descriptor type */
    0x01,                           /* Descriptor sub type : VC_HEADER */
    0x10,0x01,                      /* Revision of class spec : 1.1 */
    0x51,0x00,                      /* Total size of class specific descriptors (till output terminal) */
    0x00,0x6C,0xDC,0x02,            /* Clock frequency : 48MHz, Use of this field has been deprecated */
    0x01,                           /* Number of streaming interfaces */
    0x01,                           /* Video streaming I/f 1 belongs to VC i/f */ //38

    /* Input (camera) terminal descriptor */
    0x12,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x02,                           /* Input Terminal Descriptor type */
    0x01,                           /* ID of this terminal */
    0x01,0x02,                      /* Camera terminal type */
    0x00,                           /* No association terminal */
    0x00,                           /* String desc index : not used */
    0x00,0x00,                      /* No optical zoom supported */
    0x00,0x00,                      /* No optical zoom supported */
    0x00,0x00,                      /* No optical zoom supported */
    0x03,                           /* Size of controls field for this terminal : 3 bytes */
    0x10,0x00,0x00,                 /* No controls supported */ //56

    /* Processing unit descriptor */
    0x0D,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x05,                           /* Processing unit descriptor type */
    0x02,                           /* ID of this terminal */
    0x01,                           /* Source ID : 1 : connected to input terminal */
    0x00,0x40,                      /* Digital multiplier */
    0x03,                           /* Size of controls field for this terminal : 3 bytes */
    0x00,0x00,0x00,                 /* No controls supported */
    0x00,                           /* String desc index : not used */
    0x00,                           /* No analog mode support. */ //69

    /* Extension unit descriptor */
    0x1C,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x06,                           /* Extension unit descriptor type */
    UVC_EXTENSION_ID,               /* ID of this terminal */
    0x49,0xD6,0xF0,0xE9,            /* 16 byte GUID */   
    0x62,0x60,0x4F,0x60,
    0x9D,0x37,0x03,0x9E,
    0x0D,0x4A,0x51,0x71,
    0x01,                           /* Number of controls in this terminal */
    0x01,                           /* Number of input pins in this terminal */
    0x02,                           /* Source ID : 2 : connected to proc unit */
    0x03,                           /* Size of controls field for this terminal : 3 bytes */
    0x01,0x00,0x00,                 /* No controls supported */
    0x00,                           /* String desc index : not used */ //97

    /* Output terminal descriptor */
    0x09,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x03,                           /* Output terminal descriptor type */
    0x04,                           /* ID of this terminal */
    0x01,0x01,                      /* USB Streaming terminal type */
    0x00,                           /* No association terminal */
    0x03,                           /* Source ID : 3 : connected to extn unit */
    0x00,                           /* String desc index : not used */ //106

    /* Video control status interrupt endpoint descriptor */
    0x07,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_ENDPOINT,        /* Endpoint descriptor type */
    UVC_VIDEO_CONTROL_EP,        /* Endpoint address and description */
    KDRV_USBD_EP_TYPE_INTR,             /* Interrupt end point type */
    0x00,0x02,                      /* Max packet size = 512 bytes */
    0x08,                           /* Servicing interval : 8ms */ //113

    /* Class specific interrupt endpoint descriptor */
    0x05,                           /* Descriptor size */
    0x25,                           /* Class specific endpoint descriptor type */
    KDRV_USBD_EP_TYPE_INTR,             /* End point sub type */
    0x00,0x02,                      /* Max packet size = 512 bytes */ //118

    /* Standard video streaming interface descriptor (alternate setting 0) */
    0x09,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_INTERFACE,        /* Interface descriptor type */
    0x01,                           /* Interface number */
    0x00,                           /* Alternate setting number */
    0x01,                           /* Number of end points : zero bandwidth */
    0x0E,                           /* Interface class : CC_VIDEO */
    0x02,                           /* Interface sub class : CC_VIDEOSTREAMING */
    0x00,                           /* Interface protocol code : undefined */
    0x00,                           /* Interface descriptor string index */ //127

    /* Class-specific video streaming input header descriptor */
    0x0E,                           /* Descriptor size */
    0x24,                           /* Class-specific VS i/f type */
    0x01,                           /* Descriptor subtype : input header */
    0x01,                           /* 1 format desciptor follows */
    0x65,0x00,                      /* Total size of class specific VS descr = 55 bytes */
    UVC_VIDEO_STREAM_EP,            /* EP address for BULK video data */
    0x00,                           /* No dynamic format change supported */
    0x04,                           /* Output terminal ID : 4 */
    0x00,                           /* No still image capture support. */
    0x00,                           /* No hardware trigger support. */
    0x00,                           /* Hardware to initiate still image capture */
    0x01,                           /* Size of controls field : 1 byte */
    0x00,                           /* D2 : Compression quality supported */ //141
    
    /* Class specific VS format descriptor */
    0x1B,                           /* Descriptor size */
    0x24,                           /* Class-specific VS i/f type */                                  
    0x04,                           /* Descriptor subtype : VS_FORMAT_UNCOMPRESSED */
    0x01,                           /* Format desciptor index */
    0x02,                           /* 1 Frame desciptor follows */
    'Y',  'U',  'Y',  '2',          /* guidFormat */     //147
    0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0xaa, 
    0x00, 0x38, 0x9b, 0x71,
    UVC_BYTE_PER_PIXEL*8,           /* bBitsPerPixel */  //163
    0x01,                           /* bDefaultFrameIndex */
    0x00,                           /* bAspectRatioX */
    0x00,                           /* bAspectRatioY */
    0x00,                           /* bmInterlaceFlags */
    0x00,                           /* bCopyProtect */ //168
    /* Class specific VS frame descriptor */
    0x1E,                           /* Descriptor size */
    0x24,                           /* Class-specific VS I/f Type */
    0x05,                           /* Descriptor subtype : VS_FRAME_UNCOMPRESSED */
    0x01,                           /* Frame desciptor index */
    0x00,                           /* Still image capture method not supported */
    KDRV_USB_LOW_BYTE(UVC_IMG_WIDTH), /* Width of the frame */ //174
    KDRV_USB_HIGH_BYTE(UVC_IMG_WIDTH),                     
    KDRV_USB_LOW_BYTE(UVC_IMG_HEIGHT), /* Height of the frame */ //176
    KDRV_USB_HIGH_BYTE(UVC_IMG_HEIGHT),                     
    KDRV_USB_BYTE0(UVC_BITRATE_MIN),/* Min bit rate bits/s */  //178
    KDRV_USB_BYTE1(UVC_BITRATE_MIN),
    KDRV_USB_BYTE2(UVC_BITRATE_MIN),
    KDRV_USB_BYTE3(UVC_BITRATE_MIN),            
    KDRV_USB_BYTE0(UVC_BITRATE_MAX),/* Max bit rate bits/s */ //182
    KDRV_USB_BYTE1(UVC_BITRATE_MAX),
    KDRV_USB_BYTE2(UVC_BITRATE_MAX),
    KDRV_USB_BYTE3(UVC_BITRATE_MAX),
    KDRV_USB_BYTE0(UVC_FRAME_SIZE),/* Maximum video or still frame size in bytes */ //186
    KDRV_USB_BYTE1(UVC_FRAME_SIZE),
    KDRV_USB_BYTE2(UVC_FRAME_SIZE),
    KDRV_USB_BYTE3(UVC_FRAME_SIZE),              
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Default frame interval in unit 100ns */ //190
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
    0x01,                           /* 1 Frame interval type : No of discrete intervals */
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Frame interval=10 fps */ //195
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
    
    /* Class specific VS frame descriptor */
    0x1E,                           /* Descriptor size */
    0x24,                           /* Class-specific VS I/f Type */
    0x05,                           /* Descriptor subtype : VS_FRAME_UNCOMPRESSED */
    0x02,                           /* Frame desciptor index */
    0x00,                           /* Still image capture method not supported */
    KDRV_USB_LOW_BYTE(UVC_IMG_WIDTH*2), /* Width of the frame */ //204
    KDRV_USB_HIGH_BYTE(UVC_IMG_WIDTH*2),                     
    KDRV_USB_LOW_BYTE(UVC_IMG_HEIGHT), /* Height of the frame */ //206
    KDRV_USB_HIGH_BYTE(UVC_IMG_HEIGHT),                     
    KDRV_USB_BYTE0(UVC_BITRATE_MIN*2),/* Min bit rate bits/s */ //208
    KDRV_USB_BYTE1(UVC_BITRATE_MIN*2),
    KDRV_USB_BYTE2(UVC_BITRATE_MIN*2),
    KDRV_USB_BYTE3(UVC_BITRATE_MIN*2),            
    KDRV_USB_BYTE0(UVC_BITRATE_MAX*2),/* Max bit rate bits/s */ //212
    KDRV_USB_BYTE1(UVC_BITRATE_MAX*2),
    KDRV_USB_BYTE2(UVC_BITRATE_MAX*2),
    KDRV_USB_BYTE3(UVC_BITRATE_MAX*2),
    KDRV_USB_BYTE0(UVC_FRAME_SIZE*2),/* Maximum video or still frame size in bytes */ //216
    KDRV_USB_BYTE1(UVC_FRAME_SIZE*2),
    KDRV_USB_BYTE2(UVC_FRAME_SIZE*2),
    KDRV_USB_BYTE3(UVC_FRAME_SIZE*2),              
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Default frame interval in unit 100ns */ //220
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
    0x01,                           /* 1 Frame interval type : No of discrete intervals */
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Frame interval=10 fps */ //224
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
    
    /* Endpoint descriptor for streaming video data */
    0x07,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_ENDPOINT,        /* Endpoint descriptor type */
    UVC_VIDEO_STREAM_EP,            /* Endpoint address and description */
    KDRV_USBD_EP_TYPE_BULK,             /* Bulk Endpoint */
    0x00, 0x02,                     /* 512 Bytes Maximum Packet Size. */
    0x00                            /* Servicing interval for data transfers */
};

static uint8_t kmdw_uvc_usb_configuration_desc_ss[] = {
    /* Configuration descriptor */
    KDRV_USB_LEN_CFG_DESC,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_CONFIGURATION,        /* Configuration descriptor type */
    0xF8,0x00,                      /* Length of this descriptor and all sub descriptors */
    0x02,                           /* Number of interfaces */
    0x01,                           /* Configuration number */
    0x00,                           /* COnfiguration string index */
    0x80,                           /* Config characteristics - bus powered */
    0x32,                           /* Max power consumption of device (in 8mA unit) : 400mA */

    /* Interface association descriptor */
    0x08,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_INTERFACE_ASSOCIATION,      /* Interface association descr type */
    0x00,                           /* I/f number of first video control i/f */
    0x02,                           /* Number of video streaming i/f */
    0x0E,                           /* CC_VIDEO : Video i/f class code */
    0x03,                           /* SC_VIDEO_INTERFACE_COLLECTION : subclass code */
    0x00,                           /* Protocol : not used */
    0x00,                           /* String desc index for interface */

    /* Standard video control interface descriptor */
    0x09,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_INTERFACE,        /* Interface descriptor type */
    0x00,                           /* Interface number */
    0x00,                           /* Alternate setting number */
    0x01,                           /* Number of end points */
    0x0E,                           /* CC_VIDEO : Interface class */
    0x01,                           /* CC_VIDEOCONTROL : Interface sub class */
    0x00,                           /* Interface protocol code */
    0x00,                           /* Interface descriptor string index */

    /* Class specific VC interface header descriptor */
    0x0D,                           /* Descriptor size */
    0x24,                           /* Class Specific I/f header descriptor type */
    0x01,                           /* Descriptor sub type : VC_HEADER */
    0x10,0x01,                      /* Revision of class spec : 1.1 */
    0x51,0x00,                      /* Total size of class specific descriptors (till output terminal) */
    0x00,0x6C,0xDC,0x02,            /* Clock frequency : 48MHz, Use of this field has been deprecated */
    0x01,                           /* Number of streaming interfaces */
    0x01,                           /* Video streaming I/f 1 belongs to VC i/f */

    /* Input (camera) terminal descriptor */
    0x12,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x02,                           /* Input Terminal Descriptor type */
    0x01,                           /* ID of this terminal */
    0x01,0x02,                      /* Camera terminal type */
    0x00,                           /* No association terminal */
    0x00,                           /* String desc index : not used */
    0x00,0x00,                      /* No optical zoom supported */
    0x00,0x00,                      /* No optical zoom supported */
    0x00,0x00,                      /* No optical zoom supported */
    0x03,                           /* Size of controls field for this terminal : 3 bytes */
    0x10,0x00,0x00,                 /* No controls supported */

    /* Processing unit descriptor */
    0x0D,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x05,                           /* Processing unit descriptor type */
    0x02,                           /* ID of this terminal */
    0x01,                           /* Source ID : 1 : connected to input terminal */
    0x00,0x40,                      /* Digital multiplier */
    0x03,                           /* Size of controls field for this terminal : 3 bytes */
    0x00,0x00,0x00,                 /* No controls supported */
    0x00,                           /* String desc index : not used */
    0x00,                           /* No analog mode support. */

    /* Extension unit descriptor */
    0x1C,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x06,                           /* Extension unit descriptor type */
    UVC_EXTENSION_ID,               /* ID of this terminal */
    0x49,0xD6,0xF0,0xE9,            /* 16 byte GUID */   
    0x62,0x60,0x4F,0x60,
    0x9D,0x37,0x03,0x9E,
    0x0D,0x4A,0x51,0x71,
    0x01,                           /* Number of controls in this terminal */
    0x01,                           /* Number of input pins in this terminal */
    0x02,                           /* Source ID : 2 : connected to proc unit */
    0x03,                           /* Size of controls field for this terminal : 3 bytes */
    0x01,0x00,0x00,                 /* No controls supported */
    0x00,                           /* String desc index : not used */

    /* Output terminal descriptor */
    0x09,                           /* Descriptor size */
    0x24,                           /* Class specific interface desc type */
    0x03,                           /* Output terminal descriptor type */
    0x04,                           /* ID of this terminal */
    0x01,0x01,                      /* USB Streaming terminal type */
    0x00,                           /* No association terminal */
    0x03,                           /* Source ID : 3 : connected to extn unit */
    0x00,                           /* String desc index : not used */

    /* Video control status interrupt endpoint descriptor */
    0x07,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_ENDPOINT,        /* Endpoint descriptor type */
    UVC_VIDEO_CONTROL_EP,        /* Endpoint address and description */
    KDRV_USBD_EP_TYPE_INTR,             /* Interrupt end point type */
    0x40,0x00,                      /* Max packet size = 64 bytes */
    0x01,                           /* Servicing interval */
    
    /* Super speed endpoint companion descriptor */
    0x06,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_ENDPOINT_COMPANION,       /* SS endpoint companion descriptor type */
    0x00,                           /* Max no. of packets in a Burst : 1 */
    0x00,                           /* Mult.: Max number of packets : 1 */
    0x40, 0x00,                     /* Bytes per interval : 64 */
    
    /* Class specific interrupt endpoint descriptor */
    0x05,                           /* Descriptor size */
    0x25,                           /* Class specific endpoint descriptor type */
    KDRV_USBD_EP_TYPE_INTR,             /* End point sub type */
    0x40,0x00,                      /* Max packet size = 64 bytes */

    /* Standard video streaming interface descriptor (alternate setting 0) */
    0x09,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_INTERFACE,        /* Interface descriptor type */
    0x01,                           /* Interface number */
    0x00,                           /* Alternate setting number */
    0x01,                           /* Number of end points : zero bandwidth */
    0x0E,                           /* Interface class : CC_VIDEO */
    0x02,                           /* Interface sub class : CC_VIDEOSTREAMING */
    0x00,                           /* Interface protocol code : undefined */
    0x00,                           /* Interface descriptor string index */

    /* Class-specific video streaming input header descriptor */
    0x0E,                           /* Descriptor size */
    0x24,                           /* Class-specific VS i/f type */
    0x01,                           /* Descriptor subtype : input header */
    0x01,                           /* 1 format desciptor follows */
    0x65,0x00,                      /* Total size of class specific VS descr = 55 bytes */
    UVC_VIDEO_STREAM_EP,            /* EP address for BULK video data */
    0x00,                           /* No dynamic format change supported */
    0x04,                           /* Output terminal ID : 4 */
    0x00,                           /* No still image capture support. */
    0x00,                           /* No hardware trigger support. */
    0x00,                           /* Hardware to initiate still image capture */
    0x01,                           /* Size of controls field : 1 byte */
    0x00,                           /* D2 : Compression quality supported */ //147
    
    /* Class specific VS format descriptor */
    0x1B,                           /* Descriptor size */
    0x24,                           /* Class-specific VS i/f type */                                  
    0x04,                           /* Descriptor subtype : VS_FORMAT_UNCOMPRESSED */
    0x01,                           /* Format desciptor index */
    0x02,                           /* 1 Frame desciptor follows */
    'Y',  'U',  'Y',  '2',//'Y',  '8',  ' ',  ' ',          /* guidFormat */ //153
    0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0xaa, 
    0x00, 0x38, 0x9b, 0x71,
    UVC_BYTE_PER_PIXEL*8,           /* bBitsPerPixel */ //169
    0x01,                           /* bDefaultFrameIndex */
    0x00,                           /* bAspectRatioX */
    0x00,                           /* bAspectRatioY */
    0x00,                           /* bmInterlaceFlags */
    0x00,                           /* bCopyProtect */
    /* Class specific VS frame descriptor */
    0x1E,                           /* Descriptor size */
    0x24,                           /* Class-specific VS I/f Type */
    0x05,                           /* Descriptor subtype : VS_FRAME_UNCOMPRESSED */
    0x01,                           /* Frame desciptor index */
    0x00,                           /* Still image capture method not supported */
    KDRV_USB_LOW_BYTE(UVC_IMG_WIDTH), /* Width of the frame */ //180
    KDRV_USB_HIGH_BYTE(UVC_IMG_WIDTH),                     
    KDRV_USB_LOW_BYTE(UVC_IMG_HEIGHT), /* Height of the frame */ //182
    KDRV_USB_HIGH_BYTE(UVC_IMG_HEIGHT),                     
    KDRV_USB_BYTE0(UVC_BITRATE_MIN),/* Min bit rate bits/s */ //184
    KDRV_USB_BYTE1(UVC_BITRATE_MIN),
    KDRV_USB_BYTE2(UVC_BITRATE_MIN),
    KDRV_USB_BYTE3(UVC_BITRATE_MIN),            
    KDRV_USB_BYTE0(UVC_BITRATE_MAX),/* Max bit rate bits/s */ //188
    KDRV_USB_BYTE1(UVC_BITRATE_MAX),
    KDRV_USB_BYTE2(UVC_BITRATE_MAX),
    KDRV_USB_BYTE3(UVC_BITRATE_MAX),
    KDRV_USB_BYTE0(UVC_FRAME_SIZE),/* Maximum video or still frame size in bytes */ //192
    KDRV_USB_BYTE1(UVC_FRAME_SIZE),
    KDRV_USB_BYTE2(UVC_FRAME_SIZE),
    KDRV_USB_BYTE3(UVC_FRAME_SIZE),              
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Default frame interval in unit 100ns */ //196
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
    0x01,                           /* 1 Frame interval type : No of discrete intervals */
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Frame interval=10 fps */ //201
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
 
    /* Class specific VS frame descriptor */
    0x1E,                           /* Descriptor size */
    0x24,                           /* Class-specific VS I/f Type */
    0x05,                           /* Descriptor subtype : VS_FRAME_UNCOMPRESSED */
    0x02,                           /* Frame desciptor index */
    0x00,                           /* Still image capture method not supported */
    KDRV_USB_LOW_BYTE(UVC_IMG_WIDTH*2), /* Width of the frame */ //210
    KDRV_USB_HIGH_BYTE(UVC_IMG_WIDTH*2),                     
    KDRV_USB_LOW_BYTE(UVC_IMG_HEIGHT), /* Height of the frame */ //212
    KDRV_USB_HIGH_BYTE(UVC_IMG_HEIGHT),                     
    KDRV_USB_BYTE0(UVC_BITRATE_MIN*2),/* Min bit rate bits/s */ //214
    KDRV_USB_BYTE1(UVC_BITRATE_MIN*2),
    KDRV_USB_BYTE2(UVC_BITRATE_MIN*2),
    KDRV_USB_BYTE3(UVC_BITRATE_MIN*2),            
    KDRV_USB_BYTE0(UVC_BITRATE_MAX*2),/* Max bit rate bits/s */ //218
    KDRV_USB_BYTE1(UVC_BITRATE_MAX*2),
    KDRV_USB_BYTE2(UVC_BITRATE_MAX*2),
    KDRV_USB_BYTE3(UVC_BITRATE_MAX*2),
    KDRV_USB_BYTE0(UVC_FRAME_SIZE*2),/* Maximum video or still frame size in bytes */ //192
    KDRV_USB_BYTE1(UVC_FRAME_SIZE*2),
    KDRV_USB_BYTE2(UVC_FRAME_SIZE*2),
    KDRV_USB_BYTE3(UVC_FRAME_SIZE*2),              
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Default frame interval in unit 100ns */ //196
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
    0x01,                           /* 1 Frame interval type : No of discrete intervals */
    KDRV_USB_BYTE0(UVC_FRAME_INTERVAL_10FPS),/* Frame interval=10 fps */ //201
    KDRV_USB_BYTE1(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE2(UVC_FRAME_INTERVAL_10FPS),
    KDRV_USB_BYTE3(UVC_FRAME_INTERVAL_10FPS),
    
    /* Endpoint descriptor for streaming video data */
    0x07,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_ENDPOINT,        /* Endpoint descriptor type */
    UVC_VIDEO_STREAM_EP,            /* Endpoint address and description */
    KDRV_USBD_EP_TYPE_BULK,             /* Bulk Endpoint */
    0x00, 0x04,                     /* 1024 Bytes Maximum Packet Size. */
    0x00,                            /* Servicing interval for data transfers */
    
    /* Super speed endpoint companion descriptor */
    0x06,                           /* Descriptor size */
    KDRV_USB_DESC_TYPE_ENDPOINT_COMPANION,       /* SS endpoint companion descriptor type */
    0x00,                           /* Max no. of packets in a Burst : 1 */
    0x00,                           /* Mult.: Max number of packets : 1 */
    0x00,0x00                  	    /* Field Valid only for Periodic Endpoints */
};

static const uint8_t kmdw_uvc_usb_langid_str_desc[] = {
  KDRV_USB_LEN_LANGID_STR_DESC,         
  KDRV_USB_DESC_TYPE_STRING,       
  KDRV_USB_LOW_BYTE(UVC_USBD_LANGID),
  KDRV_USB_HIGH_BYTE(UVC_USBD_LANGID)
};

static const kdrv_usbd3_prd_string_descriptor_t kmdw_uvc_manufacturer_str_desc = {
    .bLength = 2 + 12,
    .bDescriptorType = KDRV_USB_DESC_TYPE_STRING,
    .bString = {
        'K',0x00,
        'n',0x00,
        'e',0x00,
        'r',0x00,
        'o',0x00,
        'n',0x00}
};

static const kdrv_usbd3_prd_string_descriptor_t kmdw_uvc_product_str_desc = {
    .bLength = 2 + 20,
    .bDescriptorType = KDRV_USB_DESC_TYPE_STRING,
    .bString = {
        'K',0x00,
        'n',0x00,
        'e',0x00,
        'r',0x00,
        'o',0x00,
        'n',0x00,
        ' ',0x00,
        'U',0x00,
        'V',0x00,
        'C',0x00}
};

static const kdrv_usbd3_prd_string_descriptor_t kmdw_uvc_serial_str_desc = {
    .bLength = 2 + 16,
    .bDescriptorType = KDRV_USB_DESC_TYPE_STRING,
    .bString = {
        '5',0x00,
        '2',0x00,
        '3',0x00,
        '2',0x00,
        '5',0x00,
        '0',0x00,
        '6',0x00,
        '3',0x00}
};


static uint8_t kmdw_uvc_probe_ctl[UVC_PROBE_CONTROL_LEN] = {
    0x00,0x00,                       /* bmHint : No fixed parameters */
    0x01,                            /* Use 1st Video format index */
    0x01,                            /* Use 1st Video frame index */
    0x2A, 0x2C, 0x0A, 0x00,             /* Desired frame interval in 100ns */
    0x00,0x00,                       /* Key frame rate in key frame/video frame units */
    0x00,0x00,                       /* PFrame rate in PFrame / key frame units */
    0x00,0x00,                       /* Compression quality control */
    0x00,0x00,                       /* Window size for average bit rate */
    0x00,0x00,                       /* Internal video streaming i/f latency in ms */
    KDRV_USB_BYTE0(UVC_FRAME_SIZE),  /* Maximum video or still frame size in bytes */ //18
    KDRV_USB_BYTE1(UVC_FRAME_SIZE),
    KDRV_USB_BYTE2(UVC_FRAME_SIZE),
    KDRV_USB_BYTE3(UVC_FRAME_SIZE),  
    KDRV_USB_BYTE0(UVC_STREAMING_ONE_TXF_SIZE),   /* No. of bytes device can transmit in single payload */
    KDRV_USB_BYTE1(UVC_STREAMING_ONE_TXF_SIZE),
    KDRV_USB_BYTE2(UVC_STREAMING_ONE_TXF_SIZE),
    KDRV_USB_BYTE3(UVC_STREAMING_ONE_TXF_SIZE),      
    0x00,0x60,0xE3,0x16,             /* Device clock. */
    0x00,0x00,0x00,0x00              /* Framing and format information. */
};

static kmdw_usbd_uvc_format_t _support_format[2] = {
    {640, 480, 2},
    {1280, 480, 2},
};

static uint8_t kmdw_uvc_commit_ctl[UVC_PROBE_CONTROL_LEN];
static uint8_t kmdw_uvc_knver[16] = {0};

static uint32_t *kmdw_uvc_get_device_descriptor(kdrv_usbd3_speed_t speed, uint16_t *length){
    if(speed == USBD3_SUPER_SPEED){
        *length = sizeof(kmdw_uvc_usb_device_desc_ss);
        return (uint32_t *)kmdw_uvc_usb_device_desc_ss;
    }
    else{
        *length = sizeof(kmdw_uvc_usb_device_desc_hs);
        return (uint32_t *)kmdw_uvc_usb_device_desc_hs;
    }
}

static uint32_t *kmdw_uvc_get_config_descriptor(kdrv_usbd3_speed_t speed, uint16_t *length){
    if(speed == USBD3_SUPER_SPEED){
        *length = sizeof(kmdw_uvc_usb_configuration_desc_ss);
        return (uint32_t *)kmdw_uvc_usb_configuration_desc_ss;
    }
    else{
        *length = sizeof(kmdw_uvc_usb_configuration_desc_hs);
        return (uint32_t *)kmdw_uvc_usb_configuration_desc_hs;
    }
}

static uint32_t *kmdw_uvc_get_langid_str_descriptor(kdrv_usbd3_speed_t speed, uint16_t *length){
    *length = sizeof(kmdw_uvc_usb_langid_str_desc);
    return (uint32_t *)kmdw_uvc_usb_langid_str_desc;
}

static uint32_t *kmdw_uvc_get_manc_str_descriptor(kdrv_usbd3_speed_t speed, uint16_t *length){
    *length = kmdw_uvc_manufacturer_str_desc.bLength;
    return (uint32_t *)&kmdw_uvc_manufacturer_str_desc;
}

static uint32_t *kmdw_uvc_get_prod_str_descriptor(kdrv_usbd3_speed_t speed, uint16_t *length){
    *length = kmdw_uvc_product_str_desc.bLength;
    return (uint32_t *)&kmdw_uvc_product_str_desc;
}

static uint32_t *kmdw_uvc_get_serial_str_descriptor(kdrv_usbd3_speed_t speed, uint16_t *length){
    *length = kmdw_uvc_serial_str_desc.bLength;
    return (uint32_t *)&kmdw_uvc_serial_str_desc;
}


static void kmdw_uvc_link_status_update(kdrv_usbd3_link_status_t link_status){
    //kmdw_printf("kmdw_uvc_link_status_update %d\n", link_status);
    uint8_t send = 0;
    if(link_status == USBD3_STATUS_DISCONNECTED){
        _uvc_link_stauts = KMDW_USBD_UVC_DISCONNECTED;
        send = 1;
    }
    if(link_status == USBD3_STATUS_CONFIGURED){
        _uvc_link_stauts = KMDW_USBD_UVC_CONNECTED;
        send = 1;
    }
    if(_cbs.kmdw_usbd_uvc_link_status != NULL && send){
        _cbs.kmdw_usbd_uvc_link_status(_uvc_link_stauts);
    }
}


// USB device send complete callback
static void kmdw_uvc_data_in_callback(kdrv_status_t status, uint8_t endpoint){
    //kmdw_printf("kmdw_uvc_data_in_callback %x\n", endpoint);
}

// USB device receive complete callback
static void kmdw_uvc_data_out_callback(kdrv_status_t status, uint8_t endpoint, uint32_t size){
    
}

static kdrv_usbd3_ctl_req_resp_t kmdw_uvc_set_interface(kdrv_usbd3_setup_packet_t *setup){
    //kmdw_printf("kmdw_uvc_set_interface %02X %02X %04X %04X %04X\n", setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);

    return REQ_RESP_ACK;
}

static kdrv_usbd3_ctl_req_resp_t kmdw_uvc_feature_request(kdrv_usbd3_setup_packet_t *setup){
    //kmdw_printf("kmdw_uvc_feature_request %02X %02X %04X %04X %04X\n", setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
    kdrv_usbd3_ctl_req_resp_t resp = REQ_RESP_ACK;
    if(KDRV_USB_REQ_CLEAR_FEATURE == setup->bRequest && UVC_VIDEO_STREAM_EP == setup->wIndex){
        if(_uvc_link_stauts != KMDW_USBD_UVC_CLOSED){
            _uvc_link_stauts = KMDW_USBD_UVC_CLOSED;
            if(_cbs.kmdw_usbd_uvc_link_status != NULL){
                _cbs.kmdw_usbd_uvc_link_status(_uvc_link_stauts);
            }
        }
    }
    /*
    if(KDRV_USB_REQ_SET_FEATURE == setup->bRequest && KDRV_USB_REQ_RECIPIENT_INTERFACE == setup->bmRequestType){
        if(setup->wIndex == 0){
            if(_uvc_link_stauts != KMDW_USBD_UVC_OPENED){
                _uvc_link_stauts = KMDW_USBD_UVC_OPENED;
                if(_cbs.kmdw_usbd_uvc_link_status != NULL){
                    _cbs.kmdw_usbd_uvc_link_status(_uvc_link_stauts);
                }
            }
        }
        else if(setup->wIndex == 0x0100){

            if(_uvc_link_stauts != KMDW_USBD_UVC_CLOSED){
                _uvc_link_stauts = KMDW_USBD_UVC_CLOSED;
                if(_cbs.kmdw_usbd_uvc_link_status != NULL){
                    _cbs.kmdw_usbd_uvc_link_status(_uvc_link_stauts);
                }
            }

        }
    }
    */
    return resp;
}

static kdrv_usbd3_ctl_req_resp_t kmdw_usbd_uvc_camera_terimal_handler(uint8_t req, uint8_t val){
    uint8_t send_data = 0;
    switch (val){
        case UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL: // 100us as unit
            if(UVC_REQ_GET_INFO == req){ 
                uint32_t info = 0x02;
                send_data = 1;
                kdrv_usbd3_control_send(&info, 1);
            }
            else if(UVC_REQ_GET_MAX == req){
                send_data = 1;
                uint32_t info = 255;
                kdrv_usbd3_control_send(&info, 4);
            }
            else if(UVC_REQ_GET_MIN == req){
                send_data = 1;
                uint32_t info = 1;
                kdrv_usbd3_control_send(&info, 4);
            }
            else if(UVC_REQ_GET_RES == req){
                send_data = 1;
                uint32_t info = 1;
                kdrv_usbd3_control_send(&info, 4);
            }
            else if(UVC_REQ_GET_DEF == req){
                send_data = 1;
                uint32_t info = 100;
                kdrv_usbd3_control_send(&info, 4);
            }
            else if(UVC_REQ_GET_CUR == req){
                send_data = 1;
                uint32_t info = 200;
                kdrv_usbd3_control_send(&info, 4);
            }
            else if(UVC_REQ_SET_CUR == req){
                uint32_t exp = 0;
                kdrv_usbd3_control_read(&exp, 4);
            }
            break;
            
    }
    if(send_data == 1){
        return REQ_RESP_SEND_DATA;
    }
    return REQ_RESP_ACK;
}

static kdrv_usbd3_ctl_req_resp_t kmdw_usbd_uvc_extension_unit_handler(uint8_t req, uint8_t val){
    uint8_t send_data = 0;
    uint32_t info = 0;
    switch (val){
        case 0x01:
            if(UVC_REQ_GET_LEN == req){ 
                info = 16;
                send_data = 1;
                kdrv_usbd3_control_send(&info, 2);
            }else if(UVC_REQ_GET_INFO == req){ 
                kmdw_usbd_uvc_info_attribute_t _info = {0};
                _info.get = 1;
                _info.set = 1;
                info = _info.val;
                send_data = 1;
                kdrv_usbd3_control_send(&info, 1);
            }
            else if(UVC_REQ_GET_MAX == req || UVC_REQ_GET_MIN == req || UVC_REQ_GET_RES == req || UVC_REQ_GET_DEF == req || UVC_REQ_GET_CUR == req){
                send_data = 1;
                kdrv_usbd3_control_send((uint32_t *)kmdw_uvc_knver, 16);
            }
            else if(UVC_REQ_SET_CUR == req){
                while(kdrv_usbd3_control_read((uint32_t *)kmdw_uvc_knver, 16) == KDRV_STATUS_ERROR){
                        }
            }
            break;
            
    }
    if(send_data == 1){
        return REQ_RESP_SEND_DATA;
    }
    return REQ_RESP_ACK;
}

static void parse_probe_control(uint8_t *msg){
    kmdw_usbd_uvc_probe_ctl_1_1_t *ctl = (kmdw_usbd_uvc_probe_ctl_1_1_t *)msg;
    kmdw_usbd_uvc_probe_ctl_1_1_t *cur = (kmdw_usbd_uvc_probe_ctl_1_1_t *)kmdw_uvc_probe_ctl;
    cur->bFormatIndex = ctl->bFormatIndex;
    cur->bFrameIndex = ctl->bFrameIndex;
    _current_format = cur->bFrameIndex-1;
    uint32_t frame_size = _support_format[_current_format].height * _support_format[_current_format].width * _support_format[_current_format].byte_per_pixel;
    cur->dwMaxVideoFrameSize = frame_size;
}

static kdrv_usbd3_ctl_req_resp_t kmdw_uvc_class_request(kdrv_usbd3_setup_packet_t *setup){
    kdrv_usbd3_ctl_req_resp_t resp = REQ_RESP_STALL;
    //kmdw_printf("kmdw_uvc_class_request %02X %02X %04X %04X %04X\n", setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
    //uint8_t req_dir = KDRV_USB_SETUP_REQUEST_TYPE_DIR(setup->bmRequestType);
    uint8_t req_receipent = (setup->bmRequestType) & KDRV_USB_REQ_RECIPIENT_MASK;
    uint8_t wValue_MSB = (setup->wValue) >> 8;
    uint8_t wIndex_eIDorIntf = (setup->wIndex) >> 8;
    uint8_t wIndex_ep = (setup->wIndex) & 0xff;
    if(KDRV_USB_REQ_RECIPIENT_INTERFACE == req_receipent && UVC_INTERFACE_VC == wIndex_ep){
        //kmdw_printf("UVC_INTERFACE_VC\n");
        if(wIndex_eIDorIntf == UVC_INTERFACE_VS){
            resp = kmdw_usbd_uvc_camera_terimal_handler(setup->bRequest, wValue_MSB);
        }
        else if(wIndex_eIDorIntf == UVC_EXTENSION_ID){
            resp = kmdw_usbd_uvc_extension_unit_handler(setup->bRequest, wValue_MSB);
        }
    }
    else if(KDRV_USB_REQ_RECIPIENT_INTERFACE == req_receipent && UVC_INTERFACE_VS == wIndex_ep){
        switch(wValue_MSB){
            case UVC_VS_PROBE_CONTROL:
            case UVC_VS_COMMIT_PROBE:
                switch(setup->bRequest){
                    case UVC_REQ_GET_CUR:
                    case UVC_REQ_GET_DEF:
                    case UVC_REQ_GET_MIN:
                    case UVC_REQ_GET_MAX:
                        kdrv_usbd3_control_send((uint32_t *)kmdw_uvc_probe_ctl, UVC_PROBE_CONTROL_LEN);
                        resp = REQ_RESP_SEND_DATA;
                        break;
                    case UVC_REQ_SET_CUR:
                        // Currently We would receive and ignore the value from host since we only support one configuration
                        while(kdrv_usbd3_control_read((uint32_t *)kmdw_uvc_commit_ctl, UVC_PROBE_CONTROL_LEN) == KDRV_STATUS_ERROR){
                        }
                        resp = REQ_RESP_ACK;
                        if(wValue_MSB == UVC_VS_COMMIT_PROBE){
                            _uvc_link_stauts = KMDW_USBD_UVC_OPENED;
                            if(_cbs.kmdw_usbd_uvc_link_status != NULL){
                                _cbs.kmdw_usbd_uvc_link_status(_uvc_link_stauts);
                            }
                        }
                        else{
                            parse_probe_control(kmdw_uvc_commit_ctl);
                        }
                        break;
                }
                
                break;
        }
    }
    return resp;
}

static kdrv_usbd3_ctl_req_resp_t kmdw_uvc_vendor_request(kdrv_usbd3_setup_packet_t *setup){
    //kmdw_printf("kmdw_uvc_vendor_request %02X %02X %04X %04X %04X\n", setup->bmRequestType, setup->bRequest, setup->wIndex, setup->wLength, setup->wValue);

    return REQ_RESP_ACK;
}

static uint8_t kmdw_usbd_uvc_int_usb(void){
    if(kdrv_usbd3_get_link_speed() == USBD3_SUPER_SPEED){
        kdrv_usbd3_open_endpoint(UVC_VIDEO_CONTROL_EP, KDRV_USBD_EP_TYPE_INTR, KDRB_USB_SS_MAX_PACKET_SIZE);
        kdrv_usbd3_open_endpoint(UVC_VIDEO_STREAM_EP, KDRV_USBD_EP_TYPE_BULK, KDRB_USB_SS_MAX_PACKET_SIZE);
    }
    else if(kdrv_usbd3_get_link_speed() == USBD3_HIGH_SPEED){
        kdrv_usbd3_open_endpoint(UVC_VIDEO_CONTROL_EP, KDRV_USBD_EP_TYPE_INTR, KDRV_USB_HS_MAX_PACKET_SIZE);
        kdrv_usbd3_open_endpoint(UVC_VIDEO_STREAM_EP, KDRV_USBD_EP_TYPE_BULK, KDRV_USB_HS_MAX_PACKET_SIZE);
    }
    return 0;
}

static void kmdw_usbd_uvc_set_vs_payload_header(uint8_t eof, uint8_t sof){
    kmdw_uvc_video_payload_header_t *header = (kmdw_uvc_video_payload_header_t *)_kmdw_usbd_uvc_frame_ptr;
    if(eof){
        header->bHeaderInfo.end_of_frame = 1;
    }
    else{
        header->bHeaderInfo.end_of_frame = 0;
    }
    if(sof){
        header->bHeaderInfo.frame_id ^= 1;
    }
    //memcpy(internal_buf, kmdw_uvc_vs_payload_header, UVC_VS_PAYLOAD_MAX_HEADER_SIZE);
}

kmdw_usbd_uvc_status_t kmdw_usbd_uvc_init(kmdw_usbd_uvc_config_t *cfg, kmdw_usbd_uvc_callbacks_t *cb)
{
    strcpy((char*)kmdw_uvc_knver, "Kneron_UVC_v1_0");
    memcpy(&_cfg, cfg, sizeof(kmdw_usbd_uvc_config_t));
    
    // setup usb descriptors
    uint32_t frame_size = _cfg.frame_width*_cfg.frame_height*_cfg.frame_byte_per_pixel;
    uint32_t bit_rate_min = frame_size*8*10;
    uint32_t bit_rate_max = frame_size*8*25;
    uint32_t frame_size2 = frame_size<<1;
    uint32_t bit_rate_min2 = frame_size2*8*10;
    uint32_t bit_rate_max2 = frame_size2*8*25;
    
    _support_format[0].width = _cfg.frame_width;
    _support_format[0].height = _cfg.frame_height;
    _support_format[0].byte_per_pixel = _cfg.frame_byte_per_pixel;
    
    _support_format[1].width = _cfg.frame_width*2;
    _support_format[1].height = _cfg.frame_height;
    _support_format[1].byte_per_pixel = _cfg.frame_byte_per_pixel;
    
    if(_cfg.frame_format == UVC_FRAME_FORMAT_Y){
        kmdw_uvc_usb_configuration_desc_hs[147] = 'Y';
        kmdw_uvc_usb_configuration_desc_hs[148] = '8';
        kmdw_uvc_usb_configuration_desc_hs[149] = ' ';
        kmdw_uvc_usb_configuration_desc_hs[150] = ' ';
        kmdw_uvc_usb_configuration_desc_ss[153] = 'Y';
        kmdw_uvc_usb_configuration_desc_ss[154] = '8';
        kmdw_uvc_usb_configuration_desc_ss[155] = ' ';
        kmdw_uvc_usb_configuration_desc_ss[156] = ' ';
    }
    else if(_cfg.frame_format == UVC_FRAME_FORMAT_YUY2){
        kmdw_uvc_usb_configuration_desc_hs[147] = 'Y';
        kmdw_uvc_usb_configuration_desc_hs[148] = 'U';
        kmdw_uvc_usb_configuration_desc_hs[149] = 'Y';
        kmdw_uvc_usb_configuration_desc_hs[150] = '2';
        kmdw_uvc_usb_configuration_desc_ss[153] = 'Y';
        kmdw_uvc_usb_configuration_desc_ss[154] = 'U';
        kmdw_uvc_usb_configuration_desc_ss[155] = 'Y';
        kmdw_uvc_usb_configuration_desc_ss[156] = '2';
    }
    
    kmdw_uvc_usb_configuration_desc_hs[163] = _cfg.frame_byte_per_pixel*8;
    kmdw_uvc_usb_configuration_desc_ss[169] = _cfg.frame_byte_per_pixel*8;
    
    kmdw_uvc_usb_configuration_desc_hs[174] = KDRV_USB_LOW_BYTE(_cfg.frame_width);
    kmdw_uvc_usb_configuration_desc_hs[175] = KDRV_USB_HIGH_BYTE(_cfg.frame_width);
    kmdw_uvc_usb_configuration_desc_hs[204] = KDRV_USB_LOW_BYTE(_cfg.frame_width<<1);
    kmdw_uvc_usb_configuration_desc_hs[205] = KDRV_USB_HIGH_BYTE(_cfg.frame_width<<1);
    kmdw_uvc_usb_configuration_desc_ss[180] = KDRV_USB_LOW_BYTE(_cfg.frame_width);
    kmdw_uvc_usb_configuration_desc_ss[181] = KDRV_USB_HIGH_BYTE(_cfg.frame_width);
    kmdw_uvc_usb_configuration_desc_ss[210] = KDRV_USB_LOW_BYTE(_cfg.frame_width<<1);
    kmdw_uvc_usb_configuration_desc_ss[211] = KDRV_USB_HIGH_BYTE(_cfg.frame_width<<1);
    
    kmdw_uvc_usb_configuration_desc_hs[176] = KDRV_USB_LOW_BYTE(_cfg.frame_height);
    kmdw_uvc_usb_configuration_desc_hs[177] = KDRV_USB_HIGH_BYTE(_cfg.frame_height);
    kmdw_uvc_usb_configuration_desc_hs[206] = KDRV_USB_LOW_BYTE(_cfg.frame_height);
    kmdw_uvc_usb_configuration_desc_hs[207] = KDRV_USB_HIGH_BYTE(_cfg.frame_height);
    kmdw_uvc_usb_configuration_desc_ss[182] = KDRV_USB_LOW_BYTE(_cfg.frame_height);
    kmdw_uvc_usb_configuration_desc_ss[183] = KDRV_USB_HIGH_BYTE(_cfg.frame_height);
    kmdw_uvc_usb_configuration_desc_ss[212] = KDRV_USB_LOW_BYTE(_cfg.frame_height);
    kmdw_uvc_usb_configuration_desc_ss[213] = KDRV_USB_HIGH_BYTE(_cfg.frame_height);
    
    kmdw_uvc_usb_configuration_desc_hs[178] = KDRV_USB_BYTE0(bit_rate_min);
    kmdw_uvc_usb_configuration_desc_hs[179] = KDRV_USB_BYTE1(bit_rate_min);
    kmdw_uvc_usb_configuration_desc_hs[180] = KDRV_USB_BYTE2(bit_rate_min);
    kmdw_uvc_usb_configuration_desc_hs[181] = KDRV_USB_BYTE3(bit_rate_min); 
    kmdw_uvc_usb_configuration_desc_hs[182] = KDRV_USB_BYTE0(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_hs[183] = KDRV_USB_BYTE1(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_hs[184] = KDRV_USB_BYTE2(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_hs[185] = KDRV_USB_BYTE3(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_hs[186] = KDRV_USB_BYTE0(frame_size);
    kmdw_uvc_usb_configuration_desc_hs[187] = KDRV_USB_BYTE1(frame_size);
    kmdw_uvc_usb_configuration_desc_hs[188] = KDRV_USB_BYTE2(frame_size);
    kmdw_uvc_usb_configuration_desc_hs[189] = KDRV_USB_BYTE3(frame_size);    
    kmdw_uvc_usb_configuration_desc_hs[208] = KDRV_USB_BYTE0(bit_rate_min2);
    kmdw_uvc_usb_configuration_desc_hs[209] = KDRV_USB_BYTE1(bit_rate_min2);
    kmdw_uvc_usb_configuration_desc_hs[210] = KDRV_USB_BYTE2(bit_rate_min2);
    kmdw_uvc_usb_configuration_desc_hs[211] = KDRV_USB_BYTE3(bit_rate_min2); 
    kmdw_uvc_usb_configuration_desc_hs[212] = KDRV_USB_BYTE0(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_hs[213] = KDRV_USB_BYTE1(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_hs[214] = KDRV_USB_BYTE2(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_hs[215] = KDRV_USB_BYTE3(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_hs[216] = KDRV_USB_BYTE0(frame_size2);
    kmdw_uvc_usb_configuration_desc_hs[217] = KDRV_USB_BYTE1(frame_size2);
    kmdw_uvc_usb_configuration_desc_hs[218] = KDRV_USB_BYTE2(frame_size2);
    kmdw_uvc_usb_configuration_desc_hs[219] = KDRV_USB_BYTE3(frame_size2);    
    
    kmdw_uvc_usb_configuration_desc_ss[184] = KDRV_USB_BYTE0(bit_rate_min);
    kmdw_uvc_usb_configuration_desc_ss[185] = KDRV_USB_BYTE1(bit_rate_min);
    kmdw_uvc_usb_configuration_desc_ss[186] = KDRV_USB_BYTE2(bit_rate_min);
    kmdw_uvc_usb_configuration_desc_ss[187] = KDRV_USB_BYTE3(bit_rate_min); 
    kmdw_uvc_usb_configuration_desc_ss[188] = KDRV_USB_BYTE0(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_ss[189] = KDRV_USB_BYTE1(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_ss[190] = KDRV_USB_BYTE2(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_ss[191] = KDRV_USB_BYTE3(bit_rate_max);
    kmdw_uvc_usb_configuration_desc_ss[192] = KDRV_USB_BYTE0(frame_size);
    kmdw_uvc_usb_configuration_desc_ss[193] = KDRV_USB_BYTE1(frame_size);
    kmdw_uvc_usb_configuration_desc_ss[194] = KDRV_USB_BYTE2(frame_size);
    kmdw_uvc_usb_configuration_desc_ss[195] = KDRV_USB_BYTE3(frame_size); 
    kmdw_uvc_usb_configuration_desc_ss[214] = KDRV_USB_BYTE0(bit_rate_min2);
    kmdw_uvc_usb_configuration_desc_ss[215] = KDRV_USB_BYTE1(bit_rate_min2);
    kmdw_uvc_usb_configuration_desc_ss[216] = KDRV_USB_BYTE2(bit_rate_min2);
    kmdw_uvc_usb_configuration_desc_ss[217] = KDRV_USB_BYTE3(bit_rate_min2); 
    kmdw_uvc_usb_configuration_desc_ss[218] = KDRV_USB_BYTE0(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_ss[219] = KDRV_USB_BYTE1(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_ss[220] = KDRV_USB_BYTE2(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_ss[221] = KDRV_USB_BYTE3(bit_rate_max2);
    kmdw_uvc_usb_configuration_desc_ss[222] = KDRV_USB_BYTE0(frame_size2);
    kmdw_uvc_usb_configuration_desc_ss[223] = KDRV_USB_BYTE1(frame_size2);
    kmdw_uvc_usb_configuration_desc_ss[224] = KDRV_USB_BYTE2(frame_size2);
    kmdw_uvc_usb_configuration_desc_ss[225] = KDRV_USB_BYTE3(frame_size2);  

    kmdw_uvc_probe_ctl[18] = KDRV_USB_BYTE0(frame_size);
    kmdw_uvc_probe_ctl[19] = KDRV_USB_BYTE1(frame_size);
    kmdw_uvc_probe_ctl[20] = KDRV_USB_BYTE2(frame_size);
    kmdw_uvc_probe_ctl[21] = KDRV_USB_BYTE3(frame_size);
    
    
    _uvc_link_stauts = KMDW_USBD_UVC_DISCONNECTED;
    kdrv_usbd3_class_t uvc_class = {
        .init = kmdw_usbd_uvc_int_usb,
        .de_init = NULL,
        .status_isr_cb = kmdw_uvc_link_status_update,
        .class_ctl_setup = kmdw_uvc_class_request,
        .vendor_ctl_setup = kmdw_uvc_vendor_request,
        .set_intf_setup = kmdw_uvc_set_interface,
        .feature_ctl_setup = kmdw_uvc_feature_request,
        .data_in_cb = kmdw_uvc_data_in_callback,
        .data_out_cb = kmdw_uvc_data_out_callback,
        .get_device_desc = kmdw_uvc_get_device_descriptor,
        .get_configuration_desc = kmdw_uvc_get_config_descriptor,
        .get_lang_id_str_desc = kmdw_uvc_get_langid_str_descriptor,
        .get_manufacturer_str_desc = kmdw_uvc_get_manc_str_descriptor,
        .get_product_str_desc = kmdw_uvc_get_prod_str_descriptor,
        .get_serial_str_desc = kmdw_uvc_get_serial_str_descriptor
    };
    _kmdw_usbd_uvc_frame_ptr = (uint8_t *)kmdw_ddr_reserve(UVC_STREAMING_ONE_TXF_SIZE);
    if(NULL == _kmdw_usbd_uvc_frame_ptr){
        return KMDW_USBD_UVC_ERROR;
    }
    memcpy(_kmdw_usbd_uvc_frame_ptr, kmdw_uvc_vs_payload_header, UVC_VS_PAYLOAD_MAX_HEADER_SIZE);
    kdrv_gdma_initialize();
    _kmdw_usbd_uvc_gdma = kdrv_gdma_acquire_handle();
    kdrv_usbd3_init();
    kdrv_usbd3_register_class(&uvc_class);
    _cbs.kmdw_usbd_uvc_link_status = cb->kmdw_usbd_uvc_link_status;
    kdrv_usbd3_set_enable(true);
    return KMDW_USBD_UVC_OK;
}

kmdw_usbd_uvc_status_t kmdw_usbd_uvc_deinit(void){
    _cbs.kmdw_usbd_uvc_link_status = NULL;
    kdrv_gdma_release_handle(_kmdw_usbd_uvc_gdma);
    kdrv_usbd3_set_enable(false);
    return KMDW_USBD_UVC_OK;
}

kmdw_usbd_uvc_link_status_t kmdw_usbd_get_link_status(void){
    return _uvc_link_stauts;
}

kmdw_usbd_uvc_speed_t kmdw_usbd_get_link_speed(void){
    return (kmdw_usbd_uvc_speed_t)kdrv_usbd3_get_link_speed();
}

kmdw_usbd_uvc_status_t kmdw_usbd_uvc_send_frame(uint8_t *frame_buf, uint32_t frame_len, kmdw_usbd_uvc_frame_flag_t flag){
    uint32_t remain_len = frame_len;
    uint32_t offset = 0, tx_len = 0;
    uint8_t frame_start = 0, frame_end = 0;
    if(flag & KMDW_USBD_UVC_FRAME_FLAG_SOF){
        frame_start = 1;
    }
    while(1){
        if(_uvc_link_stauts != KMDW_USBD_UVC_OPENED){
            break;
        }
        if(remain_len > (UVC_STREAMING_ONE_TXF_SIZE-UVC_VS_PAYLOAD_MAX_HEADER_SIZE)){
            kmdw_usbd_uvc_set_vs_payload_header(frame_end, frame_start);
            frame_start = 0;
            kdrv_gdma_transfer(_kmdw_usbd_uvc_gdma, (uint32_t)&_kmdw_usbd_uvc_frame_ptr[UVC_VS_PAYLOAD_MAX_HEADER_SIZE], (uint32_t)&frame_buf[offset], UVC_STREAMING_ONE_TXF_SIZE-UVC_VS_PAYLOAD_MAX_HEADER_SIZE, NULL, NULL);
            //memcpy(&_kmdw_usbd_uvc_frame_ptr[UVC_VS_PAYLOAD_MAX_HEADER_SIZE], &frame_buf[offset], UVC_STREAMING_ONE_TXF_SIZE-UVC_VS_PAYLOAD_MAX_HEADER_SIZE);
            offset += UVC_STREAMING_ONE_TXF_SIZE-UVC_VS_PAYLOAD_MAX_HEADER_SIZE;
            remain_len -= UVC_STREAMING_ONE_TXF_SIZE-UVC_VS_PAYLOAD_MAX_HEADER_SIZE;
            tx_len = UVC_STREAMING_ONE_TXF_SIZE;
        }
        else{
            if(flag & KMDW_USBD_UVC_FRAME_FLAG_EOF){
                frame_end = 1;
            }
            kmdw_usbd_uvc_set_vs_payload_header(frame_end, frame_start);
            frame_start = 0;
            kdrv_gdma_transfer(_kmdw_usbd_uvc_gdma, (uint32_t)&_kmdw_usbd_uvc_frame_ptr[UVC_VS_PAYLOAD_MAX_HEADER_SIZE], (uint32_t)&frame_buf[offset],remain_len, NULL, NULL);
            //memcpy(&_kmdw_usbd_uvc_frame_ptr[UVC_VS_PAYLOAD_MAX_HEADER_SIZE], &frame_buf[offset], remain_len);
            tx_len = remain_len + UVC_VS_PAYLOAD_MAX_HEADER_SIZE;
            remain_len -= remain_len;
        }
        if(kdrv_usbd3_bulk_send(UVC_VIDEO_STREAM_EP, (uint32_t *)_kmdw_usbd_uvc_frame_ptr, tx_len, 100) == KDRV_STATUS_OK){
        }
        else{
            //kmdw_printf("kdrv_usbd3_bulk_send error\n");
            kdrv_usbd3_reset_endpoint(UVC_VIDEO_STREAM_EP);
            //kdrv_usbd3_reset_endpoint_seq_num(UVC_VIDEO_STREAM_EP);
            return KMDW_USBD_UVC_ERROR;
        }
        if(remain_len == 0)
            break;
    }
    return KMDW_USBD_UVC_OK;
}

static uint8_t control_status[] = {
    0x01,
    0x03,
    0x00,
    0x00,
    0x00,
    0x01
};

kmdw_usbd_uvc_status_t kmdw_usbd_uvc_send_status(uint8_t *data, uint32_t len){
    kdrv_usbd3_interrupt_send(UVC_VIDEO_CONTROL_EP, control_status, 6, 1000);
    return KMDW_USBD_UVC_OK;
}

kmdw_usbd_uvc_format_t *kmdw_usbd_uvc_get_support_formats(uint8_t *cnt){
    *cnt = 2;
    return _support_format;
}

uint8_t kmdw_usbd_uvc_get_current_format(void){
    return _current_format;
}
