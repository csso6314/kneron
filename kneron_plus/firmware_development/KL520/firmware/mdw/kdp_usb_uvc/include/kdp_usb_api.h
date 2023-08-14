/*
 * Kneron USB host driver API
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#ifndef __KDP_USB_API_H__
#define __KDP_USB_API_H__

#include "kdp_usb_ch9.h"


typedef unsigned int size_t;

#define USB_DEVICE_ID_MATCH_VENDOR              0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT             0x0002
#define USB_DEVICE_ID_MATCH_INT_CLASS           0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS        0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL        0x0200
#define USB_DEVICE_ID_MATCH_DEV_HI              0x0008
#define USB_DEVICE_ID_MATCH_DEVICE \
                (USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)

#define USB_DEVICE_ID_MATCH_INT_INFO \
                (USB_DEVICE_ID_MATCH_INT_CLASS | \
                USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
                USB_DEVICE_ID_MATCH_INT_PROTOCOL)





enum usb_init_type {
    USB_INIT_HOST = 0,
    USB_INIT_DEVICE
};
//typedef void (*usb_complete_t)(struct urb *);

struct usb_endpoint {
    struct usb_endpoint_descriptor desc;
    void *hcpriv;
    unsigned char *extra;   /* Extra descriptors */
    int extralen;
    int enabled;
    int streams;
} __attribute__ ((packed));

struct usb_inf_alt {
    struct usb_interface_descriptor desc;
    int extralen;
    unsigned char *extra;
    uint8_t num_ep;
    struct usb_endpoint *p_ep;
} __attribute__ ((packed));

/* Interface */
struct usb_interface {
    struct usb_inf_alt *p_alt;
    uint8_t num_alt;
    uint8_t cur_num;
} __attribute__ ((packed));

struct usb_config {
    struct usb_config_descriptor *pdesc;
    uint8_t num_inf;
    uint8_t cur_inf;
    struct usb_interface *p_inf;
} __attribute__ ((packed));

struct usb_device {
//    int  devnum;
//    int maxpacketsize;

    struct usb_device_descriptor *dev_desc;
    uint8_t num_conf;
    uint8_t cur_conf;
    struct usb_config *pconf; /* config descriptor */
};

#endif /*_USB_H_ */
