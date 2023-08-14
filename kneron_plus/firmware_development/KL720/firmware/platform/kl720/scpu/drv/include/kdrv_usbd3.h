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

/**@addtogroup  KDRV_USBD3
 * @{
 * @brief       Kneron USB3 device mode driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_USBD3_H__
#define __KDRV_USBD3_H__
#include <stdint.h>
#include "kdrv_status.h"

#define MAX_USBD3_ENDPOINT 4 /**< maximum number of endpoint descriptor */

/** @brief Common USB descriptor definition */
#define KDRV_USB_LEN_DEV_QUALIFIER_DESC                     0x0AU
#define KDRV_USB_LEN_DEV_DESC                               0x12U
#define KDRV_USB_LEN_CFG_DESC                               0x09U
#define KDRV_USB_LEN_IF_DESC                                0x09U
#define KDRV_USB_LEN_EP_DESC                                0x07U
#define KDRV_USB_LEN_OTG_DESC                               0x03U
#define KDRV_USB_LEN_LANGID_STR_DESC                        0x04U
#define KDRV_USB_LEN_OTHER_SPEED_DESC_SIZ                   0x09U

#define KDRV_USBD_IDX_LANGID_STR                            0x00U
#define KDRV_USBD_IDX_MFC_STR                               0x01U
#define KDRV_USBD_IDX_PRODUCT_STR                           0x02U
#define KDRV_USBD_IDX_SERIAL_STR                            0x03U
#define KDRV_USBD_IDX_CONFIG_STR                            0x04U
#define KDRV_USBD_IDX_INTERFACE_STR                         0x05U

#define KDRV_USB_REQ_TYPE_STANDARD                          0x00U
#define KDRV_USB_REQ_TYPE_CLASS                             0x20U
#define KDRV_USB_REQ_TYPE_VENDOR                            0x40U
#define KDRV_USB_REQ_TYPE_MASK                              0x60U

#define KDRV_USB_REQ_RECIPIENT_DEVICE                       0x00U
#define KDRV_USB_REQ_RECIPIENT_INTERFACE                    0x01U
#define KDRV_USB_REQ_RECIPIENT_ENDPOINT                     0x02U
#define KDRV_USB_REQ_RECIPIENT_MASK                         0x05U

#define KDRV_USB_REQ_GET_STATUS                             0x00U
#define KDRV_USB_REQ_CLEAR_FEATURE                          0x01U
#define KDRV_USB_REQ_SET_FEATURE                            0x03U
#define KDRV_USB_REQ_SET_ADDRESS                            0x05U
#define KDRV_USB_REQ_GET_DESCRIPTOR                         0x06U
#define KDRV_USB_REQ_SET_DESCRIPTOR                         0x07U
#define KDRV_USB_REQ_GET_CONFIGURATION                      0x08U
#define KDRV_USB_REQ_SET_CONFIGURATION                      0x09U
#define KDRV_USB_REQ_GET_INTERFACE                          0x0AU
#define KDRV_USB_REQ_SET_INTERFACE                          0x0BU
#define KDRV_USB_REQ_SYNCH_FRAME                            0x0CU

#define KDRV_USB_DESC_TYPE_DEVICE                           0x01U
#define KDRV_USB_DESC_TYPE_CONFIGURATION                    0x02U
#define KDRV_USB_DESC_TYPE_STRING                           0x03U
#define KDRV_USB_DESC_TYPE_INTERFACE                        0x04U
#define KDRV_USB_DESC_TYPE_ENDPOINT                         0x05U
#define KDRV_USB_DESC_TYPE_DEVICE_QUALIFIER                 0x06U
#define KDRV_USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        0x07U
#define KDRV_USB_DESC_TYPE_INTERFACE_ASSOCIATION            0x0BU
#define KDRV_USB_DESC_TYPE_BOS                              0x0FU
#define KDRV_USB_DESC_TYPE_ENDPOINT_COMPANION               0x30U

#define KDRB_USB_SS_MAX_PACKET_SIZE                         1024U
#define KDRV_USB_HS_MAX_PACKET_SIZE                         512U
#define KDRV_USB_FS_MAX_PACKET_SIZE                         64U
#define KDRV_USB_MAX_EP0_SIZE                               64U
#define KDRV_USB_MAX_EP0_SIZE_SS                            9U //(2^9)

#define KDRV_USB_SETUP_REQUEST_TYPE_DIR(x)                  ((uint8_t)((x) & 0x80U) >> 7U)
#define KDRV_USB_SETUP_REQUEST_TYPE_DIR_GET                 1
#define KDRV_USB_SETUP_REQUEST_TYPE_DIR_SET                 0

#define KDRV_USB_LOW_BYTE(x)                                ((uint8_t)((x) & 0x00FFU))
#define KDRV_USB_HIGH_BYTE(x)                               ((uint8_t)(((x) & 0xFF00U) >> 8U))

#define KDRV_USB_BYTE0(x)                                   ((uint8_t)((x) & 0x000000FFU))
#define KDRV_USB_BYTE1(x)                                   ((uint8_t)(((x) & 0x0000FF00U) >> 8U))
#define KDRV_USB_BYTE2(x)                                   ((uint8_t)(((x) & 0x00FF0000U) >> 16U))
#define KDRV_USB_BYTE3(x)                                   ((uint8_t)(((x) & 0xFF000000U) >> 24U))


// Stand feature selector for set/clear feature (wValue)
#define KDRV_USB_FEATURE_ENDPOINT_ENDPOINT_HALT             0x00U
#define KDRV_USB_FEATURE_INTERFACE_FUNCTION_SUSPEND         0x00U
#define KDRV_USB_FEATURE_DEVICE_U1_ENABLE                   0x30U
#define KDRV_USB_FEATURE_DEVICE_U2_ENABLE                   0x31U
#define KDRV_USB_FEATURE_DEVICE_LTM_ENABLE                  0x32U


/** @brief Structure of 8-byte setup packet */
typedef struct __attribute__((__packed__))
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} kdrv_usbd3_setup_packet_t;

/** @brief Structure of Device descriptor */
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
} kdrv_usbd3_device_descriptor_t;

/** @brief Structure of Configuration descriptor */
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
} kdrv_usbd3_config_descriptor_t;

/** @brief Structure of Interface descriptor */
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
} kdrv_usbd3_interface_descriptor_t;

/** @brief Structure of Endpoint descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} kdrv_usbd3_endpoint_descriptor_t;

/** @brief Structure of SS Endpoint companion descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bMaxBurst;
    uint8_t bmAttributes;
    uint16_t wBytesPerInterval;
} kdrv_usbd3_endpoint_companion_descriptor_t;

/** @brief Structure of device qualifier descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint8_t bNumConfigurations;
    uint8_t bReserved;
} kdrv_usbd3_device_qualifier_descriptor_t;

/** @brief Structure of all High-Speed descriptors combination  */
typedef struct __attribute__((__packed__))
{
    kdrv_usbd3_device_descriptor_t *dev_descp;
    kdrv_usbd3_config_descriptor_t *config_descp;  // we support only 1 config
    kdrv_usbd3_interface_descriptor_t *intf_descp; // we support only 1 interface
    kdrv_usbd3_endpoint_descriptor_t *enp_descp[MAX_USBD3_ENDPOINT];
    kdrv_usbd3_device_qualifier_descriptor_t *qual_descp; // only for HS, optional
} kdrv_usbd3_HS_descriptors_t;

/** @brief Structure of all Super-Speed descriptors combination  */
typedef struct __attribute__((__packed__))
{
    kdrv_usbd3_device_descriptor_t *dev_descp;
    kdrv_usbd3_config_descriptor_t *config_descp;  // we support only 1 config
    kdrv_usbd3_interface_descriptor_t *intf_descp; // we support only 1 interface
    kdrv_usbd3_endpoint_descriptor_t *enp_descp[MAX_USBD3_ENDPOINT];
    kdrv_usbd3_endpoint_companion_descriptor_t *enp_cmpn_descp[MAX_USBD3_ENDPOINT];
} kdrv_usbd3_SS_descriptors_t;

/** Device string descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bString[32];
} kdrv_usbd3_prd_string_descriptor_t;

typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bLanguageID;
    kdrv_usbd3_prd_string_descriptor_t *desc[3];
} kdrv_usbd3_string_descriptor_t;
/** @brief Enumerations of all usb3 speed  */
typedef enum
{
    USBD3_NO_LINK_SPEED = 0,        /**< Enum 0, USB3 no link speed */
    USBD3_HIGH_SPEED,               /**< Enum 1, USB3 high speed */
    USBD3_SUPER_SPEED               /**< Enum 2, USB3 super speed */
} kdrv_usbd3_speed_t;

/** @brief Enumerations of all usb3 link status */
typedef enum
{
    USBD3_STATUS_DISCONNECTED = 0x0,    /**< Enum 0, USB3 status disconnection */
    USBD3_STATUS_CONFIGURED,            /**< Enum 1, USB3 status connections */
    USBD3_STATUS_RESET,                 /**< Enum 2, USB3 status HS/FS bus reset */
    USBD3_STATUS_SUSPEND,               /**< Enum 3, USB3 status HS/FS bus suspend */
    USBD3_STATUS_RESUME,                /**< Enum 4, USB3 status HS/FS bus resume */
    USBD3_STATUS_SS_WARM_RESET,         /**< Enum 5, USB3 status SS warm reset */
    USBD3_STATUS_SS_HOT_RESET,          /**< Enum 6, USB3 status SS hot reset */
    USBD3_STATUS_SS_U1_ENTRY,           /**< Enum 7, USB3 status SS U1 entry */
    USBD3_STATUS_SS_U2_ENTRY,           /**< Enum 8, USB3 status SS U2 entry */
    USBD3_STATUS_SS_U3_ENTRY,           /**< Enum 9, USB3 status SS U3 entry */
    USBD3_STATUS_SS_U1_EXIT,            /**< Enum 10, USB3 status SS U1 exit */
    USBD3_STATUS_SS_U2_EXIT,            /**< Enum 11, USB3 status SS U2 exit */
    USBD3_STATUS_SS_U3_EXIT             /**< Enum 12, USB3 status SS U3 exit */
} kdrv_usbd3_link_status_t;

typedef enum
{
    REQ_RESP_ACK         = 2U,
    REQ_RESP_STALL       = 3U,
    REQ_RESP_SEND_DATA   = 4U,
}kdrv_usbd3_ctl_req_resp_t;

typedef enum{
    KDRV_USBD_EP_TYPE_CTRL,
    KDRV_USBD_EP_TYPE_ISOC,
    KDRV_USBD_EP_TYPE_BULK,
    KDRV_USBD_EP_TYPE_INTR
} kdrv_usbd3_ep_type_t;

typedef void (*kdrv_usbd3_link_status_callback_t)(kdrv_usbd3_link_status_t link_status);
typedef bool (*kdrv_usbd3_user_control_callback_t)(kdrv_usbd3_setup_packet_t *setup);
// USB device send complete callback
typedef void (*kdrv_usbd3_data_in_callback_t)(kdrv_status_t status, uint8_t endpoint);
// USB device receive complete callback
typedef void (*kdrv_usbd3_data_out_callback_t)(kdrv_status_t status, uint8_t endpoint, uint32_t size);

typedef struct
{
  uint8_t (*init)(void);
  uint8_t (*de_init)(void);
  /* Link Status */
  kdrv_usbd3_link_status_callback_t status_isr_cb;

  /* Control Endpoints*/
  kdrv_usbd3_ctl_req_resp_t (*set_intf_setup)(kdrv_usbd3_setup_packet_t *req);  
  kdrv_usbd3_ctl_req_resp_t (*class_ctl_setup)(kdrv_usbd3_setup_packet_t *req);  
  kdrv_usbd3_ctl_req_resp_t (*vendor_ctl_setup)(kdrv_usbd3_setup_packet_t *req);
  kdrv_usbd3_ctl_req_resp_t (*feature_ctl_setup)(kdrv_usbd3_setup_packet_t *req);
    
  /* Class Specific Endpoints*/
  kdrv_usbd3_data_in_callback_t data_in_cb;
  kdrv_usbd3_data_out_callback_t data_out_cb;
  
  /* Class Specific Descriptors */
  uint32_t *(*get_device_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
  uint32_t *(*get_configuration_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
  uint32_t *(*get_lang_id_str_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
  uint32_t *(*get_manufacturer_str_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
  uint32_t *(*get_product_str_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
  uint32_t *(*get_serial_str_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
  uint32_t *(*get_configuration_str_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
  uint32_t *(*get_interface_str_desc)(kdrv_usbd3_speed_t speed, uint16_t *length);
} kdrv_usbd3_class_t;

/**
 * @brief           USB3 device mode driver initialization
 * @param[in]       hs_descs              user created HS device descriptor, this must be kept during device enumeration, see @ref kdrv_usbd3_HS_descriptors_t
 * @param[in]       ss_descs              user created SS device descriptor, this must be kept during device enumeration, see @ref kdrv_usbd3_SS_descriptors_t
 * @param[in]       status_isr_cb        USBD event callback function, can be NULL, see @ref kdrv_usbd3_link_status_callback_t
 * @param[in]       usr_cx_isr_cb       see @ref kdrv_usbd3_user_control_callback_t

 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_initialize(
    kdrv_usbd3_HS_descriptors_t *hs_descs,
    kdrv_usbd3_SS_descriptors_t *ss_descs,
    kdrv_usbd3_string_descriptor_t *dev_str_desc,
    kdrv_usbd3_link_status_callback_t status_isr_cb,
    kdrv_usbd3_user_control_callback_t usr_cx_isr_cb);

/**
 * @brief           Initialize the USB3 IP
 * 
 * @param[in]       N/A 
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_init(void);

/**
 * @brief           Register a USB class to driver
 * 
 * @param           cls                   user defined class definition
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_register_class(kdrv_usbd3_class_t *cls);

/**
 * @brief          USB3 device mode driver de-initialization
 *
 * @param[in]       N/A
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_uninitialize(void);

/**
 * @brief           set enable/disabale of USB device mode, host can enumerate this device only if device is enabled
 *
 * @param[in]       enable                true to enable, false to disable
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_set_enable(bool enable);

/**
 * @brief          Reset USB3 device
 *
 * @param[in]       N/A
 * @note            this cannot be called in ISR context
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_reset_device(void);

/**
 * @brief          Reset USB3 endpoint
 *
 * @param[in]       N/A
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_reset_endpoint(uint8_t endpoint);

/**
 * @brief          Reset USB3 endpoint sequence number
 *
 * @param[in]       N/A
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_reset_endpoint_seq_num(uint8_t endpoint);

/**
 * @brief          Get USB3 link status
 *
 * @param[in]       N/A
 * @return          kdrv_usbd3_link_status_t         see @ref kdrv_usbd3_link_status_t
 */
kdrv_usbd3_link_status_t kdrv_usbd3_get_link_status(void);

/**
 * @brief          Get USB3 link speed
 *
 * @param[in]       N/A
 * @return          kdrv_usbd3_speed_t         see @ref kdrv_usbd3_speed_t
 */
kdrv_usbd3_speed_t kdrv_usbd3_get_link_speed(void);

/**
 * @brief           Bulk-OUT transfser, receive data from the host through a bulk-out endpoint in blocking mode
 *
 * @param[in]       endpoint              a bulk-out endpoint address, should be the value from bEndpointAddress
 * @param[out]      buf                   buffer for receiving data
 * @param[in,out]   blen                  buffer length for input, actual transfered length for output
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_bulk_receive(uint8_t endpoint, void *buf, uint32_t *blen, uint32_t timeout_ms);

/**
 * @brief          Bulk-OUT transfser, receive zip data from the host through a bulk-out endpoint in blocking mode
 *
 * @param[in]    endpoint                 a bulk-out endpoint address, should be the value from bEndpointAddress
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_bulk_receive_zlp(uint8_t endpoint);

/**
 * @brief           Bulk-IN transfser, send data to the host through a bulk-in endpoint in blocking mode
 *
 * @param[in]       endpoint              a bulk-in endpoint address, should be the value from bEndpointAddress
 * @param[out]      buf                   buffer for sending data
 * @param[in]       txlen                 data length to be sent
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_bulk_send(uint8_t endpoint, void *buf, uint32_t txlen, uint32_t timeout_ms);

/**
 * @brief           Interrupt transfser
 *
 * @param[in]       endpoint              an interrupt-in endpoint address, should be the value from bEndpointAddress
 * @param[out]      buf                   buffer for sending data
 * @param[in]       txlen                 data length to be sent
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_interrupt_send(uint32_t endpoint, void *buf, uint32_t txLen, uint32_t timeout_ms);

/**
 * @brief           Check Interrupt buffer empty
 *
 * @param[in]       endpoint              an interrupt-in endpoint address, should be the value from bEndpointAddress
 * @return          bool
 */
bool kdrv_usbd3_interrupt_send_check_buffer_empty(uint32_t endpoint);

/**
 * @brief 
 * 
 * @param ep_no 
 * @param ep_type 
 * @param ep_size 
 */
void kdrv_usbd3_open_endpoint(uint8_t ep_no, kdrv_usbd3_ep_type_t ep_type, uint16_t ep_size);

/**
 * @brief           Send data from the USB control endpoint
 * 
 * @param[out]      buf                   buffer for sending data
 * @param[in]       tx_len                data length to be sent 
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_control_send(uint32_t *buf, uint32_t tx_len);

/**
 * @brief           Read data from the USB control endpoint
 * 
 * @param[in]       buf                   buffer for reading data
 * @param[in]       rx_len                data length to be read 
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd3_control_read(uint32_t *buf, uint32_t rx_len);

#endif
/** @}*/

