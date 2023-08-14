/**
 * @file        kp_usb.h
 * @brief       internal usb control functions
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */


#ifndef __KP_USB_H__
#define __KP_USB_H__

#include <stdbool.h>
#include "kp_struct.h"

#include <pthread.h>
#include <libusb-1.0/libusb.h>

// kdp2 Low Level API

typedef struct
{
    libusb_device_handle *usb_handle;
    kp_device_descriptor_t dev_descp;
    pthread_mutex_t mutex_send;
    pthread_mutex_t mutex_recv;
    uint16_t fw_serial; // for KL520 workaround, if bit 8 = 1, it uses USBD2V, fake short packet
    uint8_t endpoint_cmd_in;
    uint8_t endpoint_cmd_out;
    uint8_t endpoint_log_in;
} kp_usb_device_t;

typedef enum
{
    KP_USB_RET_OK = 0,

    KP_USB_USB_IO = -1,
    KP_USB_USB_INVALID_PARAM = -2,
    KP_USB_USB_ACCESS = -3,
    KP_USB_USB_NO_DEVICE = -4,
    KP_USB_USB_NOT_FOUND = -5,
    KP_USB_USB_BUSY = -6,
    KP_USB_USB_TIMEOUT = -7,
    KP_USB_USB_OVERFLOW = -8,
    KP_USB_USB_PIPE = -9,
    KP_USB_USB_INTERRUPTED = -10,
    KP_USB_USB_NO_MEM = -11,
    KP_USB_USB_NOT_SUPPORTED = -12,

    KP_USB_CONFIGURE_ERR = 98,
    KP_USB_RET_ERR = 99,
    KP_USB_RET_UNSUPPORTED = 100,
} kp_usb_status_t;

typedef struct
{
    uint32_t command; // value defined by upper layer
    unsigned short arg1;
    unsigned short arg2;
} kp_usb_control_t;

// scan all Kneron connectable devices and report a list.
kp_devices_list_t *kp_usb_scan_devices();

// connect one or multiple devices by port_id
int kp_usb_connect_multiple_devices_v2(int num_open, int port_id[], kp_usb_device_t *output_devs[], int try_count);

// disconnect device
int kp_usb_disconnect_device(kp_usb_device_t *dev);
int kp_usb_disconnect_multiple_devices(int num_dev, kp_usb_device_t *devs[]);

kp_device_descriptor_t *kp_usb_get_device_descriptor(kp_usb_device_t *dev);

void kp_usb_flush_out_buffers(kp_usb_device_t *dev);

int kp_usb_control(kp_usb_device_t *dev, kp_usb_control_t *control_request, int timeout);

// return 0 (KP_USB_RET_OK) on success, or < 0 if failed
// timeout in milliseconds, 0 means blocking wait, if timeout it returns KP_USB_USB_TIMEOUT
int kp_usb_write_data(kp_usb_device_t *dev, void *buf, int len, int timeout);

// return read size on success, or < 0 if failed
// timeout in milliseconds, 0 means blocking wait, if timeout it returns KP_USB_USB_TIMEOUT
int kp_usb_read_data(kp_usb_device_t *dev, void *buf, int len, int timeout);

int kp_usb_endpoint_write_data(kp_usb_device_t *dev, int endpoint, void *buf, int len, int timeout);
int kp_usb_endpoint_read_data(kp_usb_device_t *dev, int endpoint, void *buf, int len, int timeout);

int kp_usb_read_firmware_log(kp_usb_device_t *dev, void *buf, int len, int timeout);

#endif
