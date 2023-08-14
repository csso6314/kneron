/**
 * @file        kmdw_usbh2.h
 * @brief       usbh 2.0 APIs
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_USBH2_H__
#define __KMDW_USBH2_H__

#include <stdint.h>

/// USB Default Control Pipe Setup Packet
typedef struct
{
    struct USBH_REQUEST_TYPE
    {
        uint8_t Recipient : 5; ///< D4..0: Recipient
        uint8_t Type : 2;      ///< D6..5: Type
        uint8_t Dir : 1;       ///< D7:    Data Transfer Direction
    } bmRequestType;

    uint8_t bRequest; ///< Specific request
    uint16_t wValue;  ///< Value according to request
    uint16_t wIndex;  ///< Index or Offset according to request
    uint16_t wLength; ///< Number of bytes to transfer if there is a Data stage
} kmdw_usbh2_setup_packet_t;

/// USB Standard Device Descriptor
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} kmdw_usbh2_device_descriptor_t;

/// USB Standard Configuration Descriptor
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} kmdw_usbh2_configuration_descriptor_t;

/// USB Standard Interface Descriptor
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} kmdw_usbh2_interface_descriptor_t;

/// USB Standard Endpoint Descriptor
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} kmdw_usbh2_endpoint_descriptor_t;

/// USB String Descriptor
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bString;
} kmdw_usbh2_string_descriptor_t;

/// USB Interface Association Descriptor
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bFirstInterface;
    uint8_t bInterfaceCount;
    uint8_t bFunctionClass;
    uint8_t bFunctionSubclass;
    uint8_t bFunctionProtocol;
    uint8_t iFunction;
} kmdw_usbh2_interface_association_descriptor_t;

/* USB Endpoint Type */
typedef enum
{
    USB_ENDPOINT_CONTROL = 0,
    USB_ENDPOINT_ISOCHRONOUS = 1,
    USB_ENDPOINT_BULK = 2,
    USB_ENDPOINT_INTERRUPT = 3,
} kmdw_usbh2_endpoint_type_t;

///  ==== USB Constants and Defines ====

/// Status code values returned by USB library functions.
typedef enum
{
    USBH_OK = 0U,                ///< Function completed with no error
    USBH_TIMEOUT = -1,           ///< Function completed; time-out occurred
    USBH_INVALID_PARAMETER = -2, ///< Invalid Parameter error: a mandatory parameter was missing or specified an incorrect object
    USBH_TRANSFER_ERROR = -3,    ///< Transfer error
    USBH_UNKNOWN_ERROR = -100    ///< Unspecified USB error
} kmdw_usbh2_status_t;

typedef uint32_t kmdw_usbh2_pipe_t;
typedef void (*kmdw_usbh2_isoch_transfer_callback_t)(uint32_t *payload, uint32_t length);
typedef void (*kmdw_usbh2_configured_callback_t)(const kmdw_usbh2_device_descriptor_t *dev_desc, const kmdw_usbh2_configuration_descriptor_t *config_desc);
typedef void (*kmdw_usbh2_disconnected_callback_t)(void);

kmdw_usbh2_status_t kmdw_usbh2_initialize(kmdw_usbh2_configured_callback_t config_cb, kmdw_usbh2_disconnected_callback_t discon_cb);
kmdw_usbh2_status_t kmdw_usbh2_uninitialize(void);

kmdw_usbh2_pipe_t kmdw_usbh2_pipe_create(uint8_t ep_addr, uint8_t ep_type, uint16_t ep_max_packet_size, uint8_t ep_interval);

// return received size or kmdw_usbh2_status_t
int32_t kmdw_usbh2_bulk_in(kmdw_usbh2_pipe_t pipe_hndl, uint8_t *buf, uint32_t buf_len);

// return sent size or kmdw_usbh2_status_t
int32_t kmdw_usbh2_bulk_out(kmdw_usbh2_pipe_t pipe_hndl, uint8_t *buf, uint32_t len);

kmdw_usbh2_status_t kmdw_usbh2_interrupt_in(kmdw_usbh2_pipe_t pipe_hndl, uint8_t *buf, uint32_t len);

kmdw_usbh2_status_t kmdw_usbh2_control_transfer(const kmdw_usbh2_setup_packet_t *setup_packet, uint8_t *data, uint32_t len);

kmdw_usbh2_status_t kmdw_usbh2_set_interface(uint8_t index, uint8_t alternate);

kmdw_usbh2_pipe_t kmdw_usbh2_isoch_create(uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval, uint8_t *buf, uint32_t buf_size);
kmdw_usbh2_pipe_t kmdw_usbh2_isoch_delete(kmdw_usbh2_pipe_t pipe_hndl);
kmdw_usbh2_status_t kmdw_usbh2_isoch_start(kmdw_usbh2_pipe_t pipe_hndl, kmdw_usbh2_isoch_transfer_callback_t isoch_cb);
kmdw_usbh2_status_t kmdw_usbh2_isoch_stop(kmdw_usbh2_pipe_t pipe_hndl);

#endif
