/*
 * Kneron USB host API
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#ifndef  __KDP_USB_H__
#define  __KDP_USB_H__

#include "kdp_usb_api.h"
#include <errno.h>
#include <stdbool.h>

struct usb_device_id {
    uint16_t idVendor;
    uint16_t idProduct;
};


#define USB_DEVICE_ID_MATCH_DEVICE \
                (USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)

/**
 * USB_DEVICE - macro used to describe a specific usb device
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device.
 */
#define USB_DEVICE(vend, prod) \
        .match_flags = USB_DEVICE_ID_MATCH_DEVICE, \
        .idVendor = (vend), \
        .idProduct = (prod)


#endif  /* __KDP_USB_H__ */
