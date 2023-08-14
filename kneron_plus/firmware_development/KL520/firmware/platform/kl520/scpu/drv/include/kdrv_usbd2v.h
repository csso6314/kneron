/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/

#ifndef __KDRV_USBD2V_H__
#define __KDRV_USBD2V_H__

#pragma anon_unions
#include "kdrv_status.h"

#define MAX_USBD_ENDPOINT 4 /**< maximum number of endpoint descriptor */

/** @brief 8-byte setup packet struct */
typedef struct __attribute__((__packed__))
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} kdrv_usbd2v_setup_packet_t;

/** @brief Endpoint descriptor struct*/
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} kdrv_usbd2v_endpoint_descriptor_t;

/** @brief Interface descriptor struct*/
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

    kdrv_usbd2v_endpoint_descriptor_t *endpoint[MAX_USBD_ENDPOINT];

} kdrv_usbd2v_interface_descriptor_t;

/** @brief Configuration descriptor struct*/
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t MaxPower;

    kdrv_usbd2v_interface_descriptor_t *interface; // we support only 1 interface

} kdrv_usbd2v_config_descriptor_t;

/** @brief Device descriptor struct*/
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

    kdrv_usbd2v_config_descriptor_t *config; // we support only 1 config

} kdrv_usbd2v_device_descriptor_t;


/** Device string descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bString[32];
} kdrv_usbd2v_prd_string_descriptor_t;

typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bLanguageID;
    kdrv_usbd2v_prd_string_descriptor_t *desc[3];
} kdrv_usbd2v_string_descriptor_t;

/** @brief Enumeration of USB event name type */
typedef enum
{
    USBD2_STATUS_DISCONNECTED = 0x1,
    USBD2_STATUS_CONFIGURED,
} kdrv_usbd2v_link_status_t;

typedef void (*kdrv_usbd2v_link_status_callback_t)(kdrv_usbd2v_link_status_t link_status);
typedef bool (*kdrv_usbd2v_user_control_callback_t)(kdrv_usbd2v_setup_packet_t *setup);

extern kdrv_status_t kdrv_usbd2v_initialize(
    kdrv_usbd2v_device_descriptor_t *dev_desc,
    kdrv_usbd2v_string_descriptor_t *dev_str_desc,
    kdrv_usbd2v_link_status_callback_t status_isr_cb,
    kdrv_usbd2v_user_control_callback_t usr_cx_isr_cb);

extern kdrv_status_t kdrv_usbd2v_uninitialize(void);

extern kdrv_status_t kdrv_usbd2v_set_enable(bool enable);

extern kdrv_usbd2v_link_status_t kdrv_usbd2v_get_link_status(void);

extern kdrv_status_t kdrv_usbd2v_reset_device(void);

extern kdrv_status_t kdrv_usbd2v_terminate_all_endpoint(void);

extern kdrv_status_t kdrv_usbd2v_bulk_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms);

extern kdrv_status_t kdrv_usbd2v_bulk_receive(uint32_t endpoint, uint32_t *buf, uint32_t *blen, uint32_t timeout_ms);

extern bool kdrv_usbd2v_interrupt_send_check_buffer_empty(uint32_t endpoint);

extern kdrv_status_t kdrv_usbd2v_interrupt_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms);

#endif
