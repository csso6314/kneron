/**
 * @file        kdrv_usbd.h
 * @brief       Kneron USB device mode driver
 * @details     This USBD driver API implementation is based on an event-driven architecture.\n
 *              For async mode API usage, to get notified of specific USB events,\n
 *              user of USBD API needs to create a user thread to listen events by waiting for a specified thread flag (CMSIS-RTOS v2)\n
 *              which is registered at early time.\n\n
 *              Listening events is optional for sync mode usage by not setting notification for events and use synchronous mode API to perform transfers.\n
 *              Once user is notified with the specified thread flag, a get-event API can be used to retrieve the exact USB event and take a corresponding action for it.\n
 *              USBD handles hardware interrupts directly in ISR context, based on USB protocol to accomplish USB events and transfer work.\n\n
 *              There are two layers of software for a complete USB device mode driver (software layer block diagram is shown as below),\n
 *              one is USBD driver itself which provides a set of generic APIs with prefix "kdrv_usbd" them,\n
 *              another is the function driver which can leverage USBD API to implement.\n\n
 *              At present there is none of class drivers like MSC or CDC come with the USBD implementation, however users can implement their own function\n
 *              driver for custom use cases.
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */

#ifndef __KDRV_USBD_H__
#define __KDRV_USBD_H__

#pragma anon_unions
#include "kdrv_status.h"

#define MAX_USBD_CONFIG 1    /**< maximum number of configuration descriptor */
#define MAX_USBD_INTERFACE 1 /**< maximum number of interface descriptor */
#define MAX_USBD_ENDPOINT 4  /**< maximum number of endpoint descriptor */

/**
 * @brief 8-byte setup packet struct
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bmRequestType;  /**< Request type */
    uint8_t bRequest;       /**< Request */
    uint16_t wValue;        /**< Write value */
    uint16_t wIndex;        /**< Write index */
    uint16_t wLength;       /**< Write length */
} kdrv_usbd_setup_packet_t;

/**
 * @brief Endpoint descriptor struct
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;                /**< Length */
    uint8_t bDescriptorType;        /**< Descriptor type */
    uint8_t bEndpointAddress;       /**< Endpoint address */
    uint8_t bmAttributes;           /**< Attributes */
    uint16_t wMaxPacketSize;        /**< Max. packet size */
    uint8_t bInterval;              /**< Interval*/
} kdrv_usbd_endpoint_descriptor_t;

/** 
 * @brief Interface descriptor struct
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;                /**< Length */
    uint8_t bDescriptorType;        /**< Descriptor type */
    uint8_t bInterfaceNumber;       /**< Interface mumber */
    uint8_t bAlternateSetting;      /**< Alternate setting */
    uint8_t bNumEndpoints;          /**< Number of endpoints*/
    uint8_t bInterfaceClass;        /**< Interface class*/
    uint8_t bInterfaceSubClass;     /**< Interface sub-class*/
    uint8_t bInterfaceProtocol;     /**< Interface protocol */
    uint8_t iInterface;             /**< Interface */

    kdrv_usbd_endpoint_descriptor_t *endpoint[MAX_USBD_ENDPOINT];   /**< *endpoint, @ref  kdrv_usbd_endpoint_descriptor_t*/

} kdrv_usbd_interface_descriptor_t;

/**
 * @brief Configuration descriptor struct
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;                /**< Length */
    uint8_t bDescriptorType;        /**< Descriptor type */
    uint16_t wTotalLength;          /**< Total length */
    uint8_t bNumInterfaces;         /**< Number of interfaces*/
    uint8_t bConfigurationValue;    /**< Configuration value */
    uint8_t iConfiguration;         /**< Configuration*/    
    uint8_t bmAttributes;           /**< Attribute*/
    uint8_t MaxPower;               /**< Max power*/

    kdrv_usbd_interface_descriptor_t *interface[MAX_USBD_INTERFACE]; /**< *interface, @ref  kdrv_usbd_interface_descriptor_t*/

} kdrv_usbd_config_descriptor_t;

/**
 * @brief Device descriptor struct
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;            /**< Length */
    uint8_t bDescriptorType;    /**< Descriptor type */
    uint16_t bcdUSB;            /**< bcd USB */
    uint8_t bDeviceClass;       /**< Device class */
    uint8_t bDeviceSubClass;    /**< Device sub-class */
    uint8_t bDeviceProtocol;    /**< Device protocol */
    uint8_t bMaxPacketSize0;    /**< Max. packet size */
    uint16_t idVendor;          /**< Vendor id */
    uint16_t idProduct;         /**< Product id */
    uint16_t bcdDevice;         /**< bcd device */
    uint8_t iManufacturer;      /**< Manufacturer */
    uint8_t iProduct;           /**< Product*/
    uint8_t iSerialNumber;      /**< Serial number*/    
    uint8_t bNumConfigurations; /**< Number of configurations */

    kdrv_usbd_config_descriptor_t *config[MAX_USBD_CONFIG]; /**< *config, @ref  kdrv_usbd_config_descriptor_t*/

} kdrv_usbd_device_descriptor_t;

/**
 * @brief Device qualifier descriptor struct
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;            /**< Length */
    uint8_t bDescriptorType;    /**< Descriptor type */
    uint16_t bcdUSB;            /**< bcd USB */
    uint8_t bDeviceClass;       /**< Device class */
    uint8_t bDeviceSubClass;    /**< Device sub-class */
    uint8_t bDeviceProtocol;    /**< Device protocol */
    uint8_t bMaxPacketSize0;    /**< Max. packet size */
    uint8_t bNumConfigurations; /**< Number of configurations */
    uint8_t bReserved;          /**< Reserved */
} kdrv_usbd_device_qualifier_descriptor_t;

/**
 * @brief Device prd string descriptor
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;            /**< Length */
    uint8_t bDescriptorType;    /**< Descriptor type */
    uint8_t bString[32];        /**< 32 Bytes string */
} kdrv_usbd_prd_string_descriptor_t;

/**
 * @brief Device string descriptor
 */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;            /**< Length */
    uint8_t bDescriptorType;    /**< Descriptor type */
    uint16_t bLanguageID;       /**< Language Id */
    kdrv_usbd_prd_string_descriptor_t *desc[3]; /**< *desc, @ref  ;       */
} kdrv_usbd_string_descriptor_t;

/**
 * @brief Enumeration of connection speed
 */
typedef enum
{
    KDRV_USBD_HIGH_SPEED,   /**< Enum 0, USB high speed */
    KDRV_USBD_FULL_SPEED    /**< Enum 1, USB full speed, not supported yet */
} kdrv_usbd_speed_t;

/**
 * @brief Enumeration of USB event name type
 */
typedef enum
{
    KDRV_USBD_EVENT_BUS_RESET = 1,       /**< Enum 1, USBD event of bus reset */
    KDRV_USBD_EVENT_BUS_SUSPEND,         /**< Enum 2, USBD event of bus suspend */
    KDRV_USBD_EVENT_BUS_RESUME,          /**< Enum 3, USBD event of bus resume */
    KDRV_USBD_EVENT_SETUP_PACKET,        /**< Enum 4, USBD event of setup packet */
    KDRV_USBD_EVENT_DEV_CONFIGURED,      /**< Enum 5, USBD event of device configuration*/
    KDRV_USBD_EVENT_TRANSFER_BUF_FULL,   /**< Enum 6, USBD event of transfer buffer full */
    KDRV_USBD_EVENT_TRANSFER_DONE,       /**< Enum 7, USBD event of transfer done */
    KDRV_USBD_EVENT_TRANSFER_OUT,        /**< Enum 8, USBD event of transfer out */
    KDRV_USBD_EVENT_TRANSFER_TERMINATED, /**< Enum 9, USBD event of transfer terminated */
    KDRV_USBD_EVENT_DMA_ERROR,           /**< Enum 10, USBD event of DMA error */
} kdrv_usbd_event_name_t;

/**
 * @brief USB event, it includes kdrv_usbd_event_name_t and related data
 */
typedef struct
{
    kdrv_usbd_event_name_t ename;       /**< see @ref kdrv_usbd_event_name_t */
    union {                             /**< Union struct*/
        kdrv_usbd_setup_packet_t setup; /**< see @ref kdrv_usbd_setup_packet_t */
        struct                          /**< Structure of bit field */
        {
            uint32_t data1;             /**< 4 bytes data 1*/
            uint32_t data2;             /**< 4 bytes data 2*/
        };
    };
} kdrv_usbd_event_t;

/**
 * @brief Enumeration of code for response to host in control transfer
 */
typedef enum
{
    KDRV_USBD_RESPOND_OK,       /**< Enum 0, send ACK in the status stage */
    KDRV_USBD_RESPOND_ERROR     /**< Enum 1, send STALL in the status stage */
} kdrv_usbd_status_respond_t;

/**
 * @brief           USB device mode driver initialization
 *
 * @details         This API should be the first call for USBD driver initialization and to invoke the driver thread. 
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_initialize(void);

/**
 * @brief           USB device mode driver uninitialization
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_uninitialize(void);

/**
 * @brief           reset device and then it can be re-enumerated by host
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_reset_device(void);

/**
 * @brief           configure device descriptor including configuration, interface and all endpoints descriptors
 *
 * @details         USBD driver API provides specific data structs for these descriptors, users must statically declare instances of these descriptors in memory which will be used when being enumerated by a USB host.\n
 *                  At present some limitations should be noted:\n
 *                  1. Support only one configuration descriptor, one interface descriptor and 4 endpoint descriptors.\n
 *                  2. Isochronous transfer is not supported yet.\n
 *                  3. If enabling log message through USB then one endpoint must be reserved for USBD internal use.\n
 *
 * @param[in]       speed                 speed want to run, now support only High-Speed
 * @param[in]       dev_desc              user crated device descriptor, this must be kept during device enumeration
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_set_device_descriptor(
    kdrv_usbd_speed_t speed,
    kdrv_usbd_device_descriptor_t *dev_desc);

/**
 * @brief           configure device qualifier descriptor, this is optional
 *
 * @details         This API is to set other speed when acting in high-speed, users can set a meaningful content in this descriptor. 
 *
 * @param[in]       speed                 speed want to run, now support only High-Speed
 * @param[in]       dev_qual_desc         user crated device qualifier descriptor, this must be kept during device enumeration
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_set_device_qualifier_descriptor(
    kdrv_usbd_speed_t speed,
    kdrv_usbd_device_qualifier_descriptor_t *dev_qual_desc);

/**
 * @brief           configure device manufacturer, product , serial number
 * 
 * @param[in]       speed                 speed want to run, now support only High-Speed
 * @param[in]       dev_str_desc          device string descriptor
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_set_string_descriptor(
    kdrv_usbd_speed_t speed,
    kdrv_usbd_string_descriptor_t *dev_str_desc);

/**
 * @brief           register user thread ID and thread flag for notifications including events or transfer completion/errors
 *
 * @param[in]       tid                   CMSIS-RTOS v2 thread ID
 * @param[in]       tflag                 user defined thread flag to be notified by osThreadFlagsSet()
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_register_thread_notification(osThreadId_t tid, uint32_t tflag);

/**
 * @brief           set enable/disabale of USB device mode, host can enumerate this device only if device is enabled
 *
 * @details         Once above calls are done properly, users can invoke this function to enable the device and after that it can start to be seen and be enumerated by a USB host.\n
 *                  Once device is enabled and enumerated by a host, some USB events may start appearing,\n
 *                  user must start to wait for a specified thread flag to be notified of USB events through the osThreadFlagsWait(), events will be introduced in next section. 
 *
 * @param[in]       enable                true to enable, false to disable
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_set_enable(bool enable);

/**
 * @brief           check if device is enumerated and configured by a host
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
bool kdrv_usbd_is_dev_configured(void);

// get an usb event from internal event queue

/**
 * @brief           get a usbd event, this is a blocking function for sync mode usage of USBD APIs
 *
 * @details         @kdrv_usbd_get_event() when awake from osThreadFlagsWait( ) due to USBD notification,\n
 *                  users can use this function to retrieve which event is appearing and then take the corresponding action.\n
 *                  While performing transfers, user can also get notified through this call such as bulk-in notification or transfer complete notifications. 
 *
 * @param[in]       uevent                usbd event to be notified, see @ref kdrv_usbd_event_t
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_get_event(kdrv_usbd_event_t *uevent);

/**
 * @brief           Control-IN transfer, send data to host through the control endpont
 * @details         for a user-defined vendor reqeust & control IN & wLength > 0,
 *                  user should use this function to send data to host,
 *                  or respond an error via kdrv_usbd_control_respond(KDRV_USBD_RESPOND_ERROR) to claim STALL
 * 
 * @param[in]       buf                   data to be sent to host
 * @param[in]       size                  number of bytes to be transfered
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_control_send(
    uint8_t *buf,
    uint32_t size,
    uint32_t timeout_ms);

/**
 * @brief           Control-OUT transfer, receive data from host through the control endpont
 * @details         for a user-defined vendor reqeust & control OUT & wLength > 0,
 *                  user should use this function to receive data from host,
 *                  or respond an error via kdrv_usbd_control_respond(KDRV_USBD_RESPOND_ERROR) to claim STALL
 * 
 * @param[out]      buf                   buffer for receiving data
 * @param[in]       size                  buffer length
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_control_receive(
    uint8_t *buf,
    uint32_t *size,
    uint32_t timeout_ms);

// to report status for a user-defined vendor request

/**
 * @brief           respond to host through control transfer in the status stage
 * @details         this function is used as response function to report status for a user-defined vendor request
 * 
 * @param[in]       status                status, see @ref kdrv_usbd_status_respond_t
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_control_respond(kdrv_usbd_status_respond_t status);

/**
 * @brief           reset specified endpoint
 * 
 * @param[in]       status                status
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_reset_endpoint(uint32_t endpoint);

/**
 * @brief           Bulk-IN transfser, send data to host through a bulk-in endpont in blocking mode
 * 
 * @param[in]       endpoint              a bulk-in endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   data to be sent to host
 * @param[in]       txLen                 number of bytes to be transfered
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_bulk_send(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t txLen,
    uint32_t timeout_ms);

/**
 * @brief           Bulk-IN transfser, send data to host through a bulk-in endpont in non-blocking mode
 * @details         user can commit a buffer for Bulk In transfer, and then wait for KDRV_USBD_EVENT_TRANSFER_DONE 
 *                  to be notified that the transfer is done or some error code if failed.
 *                  This function works with kdrv_usbd_get_event().
 * 
 * @param[in]       endpoint              a bulk-in endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   data to be sent to host
 * @param[in]       txLen                 number of bytes to be transfered
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_bulk_send_async(
    uint32_t endpoint, // should be the value from bEndpointAddress
    uint32_t *buf,     // memory addres to be read from
    uint32_t txLen);   // transfer length

/**
 * @brief           Bulk-OUT transfser, receive data from the host through a bulk-out endpoint in blocking mode
 * 
 * @param[in]       endpoint              a bulk-out endpoint address, should be the value from bEndpointAddress
 * @param[out]      buf                   buffer for receiving data
 * @param[in,out]   blen                  buffer length for input, actual transfered length for output
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_bulk_receive(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t *blen,
    uint32_t timeout_ms);

/**
 * @brief           Bulk-OUT transfser, receive data from the host through a bulk-out endpoint in non-blocking mode
 * @details         this works with kdrv_usbd_get_event(), when receiving a 'KDRV_USBD_EVENT_TRANSFER_OUT' event,
 *                  user should commit a buffer for Bulk Out transfer through this function.
 *                  when transfer is done by usbd, eihter a 'KDRV_USBD_EVENT_TRANSFER_DONE' or 'KDRV_USBD_EVENT_TRANSFER_BUF_FULL' event
 *                  will be sent to user.
 * 
 * @param[in]       endpoint              a bulk-out endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   buffer for receiving data
 * @param[in]       blen                  buffer length
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_bulk_receive_async(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t blen);

/**
 * @brief           Interrupt-IN transfer in blocking mode
 * @details         Immediately write data to the FIFO buffer for periodic interrupt-in transfer.
 *                  Note even while the old data is not yet read by host, this function will overwrite it.
 * 
 * @param[in]       endpoint              a interrupt-in endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   data to be sent to host
 * @param[in]       txLen                 transfer length, shoudl be less then MaxPacketSize
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_interrupt_send(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t txLen,
    uint32_t timeout_ms);

/**
 * @brief           Interrupt-OUT transfer in blocking mode
 * 
 * @param[in]       endpoint              a interrupt-out endpoint address, should be the value from bEndpointAddress
 * @param[out]      buf                   buffer for receiving data
 * @param[in,out]   rxLen                 buffer length for input, actual transfered length for output, should be less than MaxPacketSize
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbd_interrupt_receive(
    uint32_t endpoint, // should be the value from bEndpointAddress
    uint32_t *buf,
    uint32_t *rxLen,
    uint32_t timeout_ms);

#endif
