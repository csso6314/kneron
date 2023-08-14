/*
 * Kneron UVC driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#ifndef _UVC_DEV_H_
#define _UVC_DEV_H_

#include <uvc.h>
#include <kmdw_usbh.h>

#define FLAGS_UVC_CAMERA_INIT_DONE_EVT         BIT24
#define FLAGS_UVC_CAMERA_INIT_FAILED_EVT       BIT25
#define UVC_TERM_INPUT			0x0000
#define UVC_TERM_OUTPUT			0x8000
#define UVC_TERM_DIRECTION(term)	((term)->type & 0x8000)

#define UVC_ENTITY_TYPE(entity)		((entity)->type & 0x7fff)
#define UVC_ENTITY_IS_UNIT(entity)	(((entity)->type & 0xff00) == 0)
#define UVC_ENTITY_IS_TERM(entity)	(((entity)->type & 0xff00) != 0)
#define UVC_ENTITY_IS_ITERM(entity) \
	(UVC_ENTITY_IS_TERM(entity) && \
	((entity)->type & 0x8000) == UVC_TERM_INPUT)
#define UVC_ENTITY_IS_OTERM(entity) \
	(UVC_ENTITY_IS_TERM(entity) && \
	((entity)->type & 0x8000) == UVC_TERM_OUTPUT)

#define  UVC_ET_IT  UVC_TERM_INPUT
#define  UVC_ET_OT  UVC_TERM_OUTPUT

#define  UVC_ET_CT   0x1
#define  UVC_ET_PU   0x2
#define  UVC_ET_SU   0x3
#define  UVC_ET_EU   0x4
#define  UVC_ET_XU   0x5
#define  UVC_DEV_INF 0x6
#define  UVC_DEF       0
#define  UVC_CUR       1
#define  UVC_MIN       2
#define  UVC_MAX       3
#define  UVC_RES       4


#define DRIVER_VERSION                 "1.0.1"
#define UVC_CTRL_CONTROL_TIMEOUT	30
#define UVC_CTRL_STREAMING_TIMEOUT	5000

#define UVC_FMT_FLAG_COMPRESSED	0x00000001
#define UVC_FMT_FLAG_STREAM		0x00000002


struct uvc_format_desc {
    char *name;
    uint32_t fcc;
};

struct kdp_uvc_id {

    uint16_t  idVendor;
    uint16_t  idProduct;

};

#define CAP_SUPPORT_GET 0x0
#define CAP_SUPPORT_SET 0x1
#define STATE_DISABLED_AUTO_MODE_STATE 0x2
#define CAP_SUPPORT_AUTOUPDATE_CTRL 0x4
#define CAP_SUPPORT_ASYNC_CTRL 0x8
#define STATE_DISABLED_INCOMP_COMMIT_STATE 0x10

#define UVC_ENTITY_FLAG_DEFAULT		(1 << 0)
#define PARAM_ARRAY_SIZE 5 
struct ctrl_info {
    uint32_t cid;
    uint8_t eid;
    uint8_t cs;
    bool supported;
    bool cached;
    uint8_t para_size;
    uint8_t *para;
    uint8_t caps;
    uint8_t len;
    uint8_t ctl_flag;
}__attribute__((packed,aligned(4)));

struct uvc_ct {
    uint16_t wObjectiveFocalLengthMin;
    uint16_t wObjectiveFocalLengthMax;
    uint16_t wOcularFocalLength;
    uint32_t bmControls;
    struct ctrl_info *data;
}__attribute__((packed,aligned(4)));

struct uvc_it {
    uint8_t id;
    uint16_t wTerminalType;
    uint8_t  bAssocTerminal;
    struct uvc_ct *ct;
}__attribute__((packed,aligned(4)));

struct uvc_ot {
    uint8_t id;
    uint16_t wTerminalType;
    uint8_t  bAssocTerminal;
    uint8_t baSourceID;
}__attribute__((packed,aligned(4)));

struct uvc_su {
    uint8_t id;
    uint8_t bNrInPins;
    uint8_t *baSourceID;

}__attribute__((packed,aligned(4)));

struct uvc_pu {
    uint8_t id;
    uint8_t baSourceID;
    uint16_t wMaxMultiplier;
    uint32_t bmControls;
    uint8_t bmVideoStandards;
    struct ctrl_info *data;
}__attribute__((packed,aligned(4)));

struct uvc_xu {
    uint8_t id;
    uint8_t bNrInPins;
    uint8_t *baSourceID;
    uint8_t  bNumControls;
    uint32_t bmControls;
    struct ctrl_info *data;
}__attribute__((packed,aligned(4)));

struct uvc_eu {
    uint8_t id;
    uint8_t s_id;
    uint8_t bSourceID;
    uint8_t bNumControls;
    uint32_t bmControls;
    uint32_t bmControlsRuntime;
    struct ctrl_info *data;
}__attribute__((packed,aligned(4)));

struct uvc_control {
    uint8_t entity_id;
    uint8_t index;     /* Bit index in bmControls */
    uint8_t size;
    uint8_t flags;
}__attribute__((packed,aligned(4)));

struct uvc_streaming_header {
    uint8_t bNumFormats;
    uint8_t bEndpointAddress;
    uint8_t bTerminalLink;
    uint8_t bControlSize;
    uint8_t *bmaControls;
    uint8_t bmInfo;
    uint8_t bStillCaptureMethod;
    uint8_t bTriggerSupport;
    uint8_t bTriggerUsage;
}__attribute__((packed,aligned(4)));

struct uvc_decode_op {
    struct uvc_buffer *buf;
    void *dst;
    const uint8_t *src;
    int len;
}__attribute__((packed,aligned(4)));

struct uvc_color_mat_desc {
    uint8_t  bColorPrimaries;
    uint8_t  bTransferCharacteristics;
    uint8_t  bMatrixCoefficients;
} __attribute__((packed,aligned(4)));

struct uvc_vs_format {
    uint8_t len;
    union {
        struct uvc_format_uncompressed *p_uncomp;
        struct uvc_format_mjpeg *p_mjpg;
    } format;
    uint8_t num_frame;
    uint8_t curr_framenum;
    uint8_t frame_len;
    union {
        struct uvc_frame_uncompressed *p_uncomp;
        struct uvc_frame_mjpeg *p_mjpg;
    } frame;
}__attribute__((packed,aligned(4)));

struct cont_frame_intervals {
    uint32_t dwMinFrameInterval; // Number Shortest frame interval supported (at highest frame rate), in 100ns units. 30
    uint32_t dwMaxFrameInterval; // Number Longest frame interval supported (at lowest frame rate), in 100ns units. 34
    uint32_t dwFrameIntervalStep; //
}__attribute__((packed,aligned(4)));

struct uvc_frame {
    uint8_t bDescriptorType;
    uint8_t  bFrameIndex;
    uint8_t  bmCapabilities;
    uint16_t wWidth;
    uint16_t wHeight;
    uint32_t dwMinBitRate;
    uint32_t dwMaxBitRate;
    uint32_t dwMaxVideoFrameBufferSize;
    uint32_t dwDefaultFrameInterval;
    uint8_t  bFrameIntervalType;
    uint32_t *dwFrameInterval;
}__attribute__((packed,aligned(4)));

struct uvc_format {
    uint8_t type;
    uint8_t index;
    uint8_t bpp;
    uint8_t colorspace;
    uint32_t fcc;
    char name[32];
    unsigned int nframes;
    uint8_t cur_frame_num;
    struct uvc_frame *frame;
    struct uvc_frame *cur_frame;
}__attribute__((packed,aligned(4)));

struct uvc_vs_alt_intf {
    uint8_t ep_type;
    uint16_t addr;
    uint16_t maxpacketsize;
    uint8_t interval;
    uint8_t alt_num;
}__attribute__((packed,aligned(4)));

struct uvc_vs_ctl_data {
    uint16_t wmHint;
    uint8_t bFormatIndex;
    uint8_t bFrameIndex;
    uint32_t dwFrameInterval;

    uint16_t wDelay;
    uint32_t dwMaxVideoFrameSize;
    uint32_t dwMaxPayloadTransferSize;
    uint32_t dwClockFrequency;

    uint8_t  bPreferedVersion;
    uint8_t  bMinVersion;
    uint8_t  bMaxVersion;
}__attribute__((packed,aligned(4)));

struct ctrl_vs_info {
    bool cached;
    struct uvc_streaming_control_data  *def;
    struct uvc_streaming_control_data  *curr;
    struct uvc_streaming_control_data  *minimum;
    struct uvc_streaming_control_data  *maximum;
    struct uvc_streaming_control_data  *res;
    uint8_t caps;
    uint16_t len;
}__attribute__((packed,aligned(4)));

struct uvc_streaming {
 //   struct uvc_device *uvc_dev;
    int ifnum;
    uint8_t num_alt;
    uint8_t curr_altnum;
    struct uvc_vs_alt_intf *if_alt;
    uint8_t num_ep;
    uint16_t ep_addr;
//    struct uvc_streaming_header header;

    uint8_t ep_type;
    uint8_t TerminalId;
    uint8_t ControlSize;
    unsigned int nformats;
    struct uvc_format *format;
    int8_t cur_format_num;
    struct uvc_format *cur_format;

    struct ctrl_vs_info *vs_ctrl_info;
    struct uvc_color_mat_desc *color_match;

    USBH_PIPE_HANDLE  isoch_pipe;

    uint32_t imagesize;
    uint32_t frame_buf;
    int write_idx;
    bool running;
}__attribute__((packed,aligned(4)));

struct uvc_vc_int_ep {
    uint16_t addr;
    uint16_t maxpacketsize;
    uint16_t wMaxTransferSize;
    uint8_t interval;
 //   struct urb *int_urb;
}__attribute__((packed,aligned(4)));

struct uvc_device {
    char name[32];
    uint32_t device_caps;
    struct kdp_uvc_id *id;

    uint8_t num_inf;
    uint8_t vc_inf;
    struct ctrl_info *inf_ctl;

    uint16_t uvc_version;
    uint32_t clock_frequency;

    uint8_t nITs;
    struct uvc_it *IT;
    uint8_t nOTs;
    struct uvc_ot *OT;
    uint8_t nSUs;
    struct uvc_su *SU;
    uint8_t nPUs;
    struct uvc_pu *PU;
//  uint8_t nEUs;
//  struct uvc_eu *EU;
    uint8_t nXUs;
    struct uvc_xu *XU;

    /* Video Streaming  */
    uint8_t num_vs_inf;

    struct uvc_streaming *curr_stream;
    struct uvc_streaming *stream;
    /* Status Interrupt Endpoint */
    struct uvc_vc_int_ep *int_ep;

    uint8_t *status;
    bool inited;
    bool opened;

}__attribute__((packed,aligned(4)));

struct uvc_entity *uvc_entity_by_id(struct uvc_device *dev, int id);
struct uvc_device *video_dev(const char *pname);
//int usb_uvc_id_lookup(uint16_t  idVendor,  uint16_t  idProduct);
void kmdw_cam_uvc_init(void);
#endif  //_UVC_DEV_H_
