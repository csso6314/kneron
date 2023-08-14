/**
 * @file        kmdw_usbh.h
 * @brief       usbh 2.0 APIs
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_USBH_H__
#define __KMDW_USBH_H__

#include <stdint.h>
#include <stdbool.h>

#include "usb_def.h"

///  ==== USB Constants and Defines ====

/// Status code values returned by USB library functions.
typedef enum
{
  usbOK = 0U, ///< Function completed with no error

  usbTimeout,          ///< Function completed; time-out occurred
  usbInvalidParameter, ///< Invalid Parameter error: a mandatory parameter was missing or specified an incorrect object

  usbThreadError = 0x10U, ///< CMSIS-RTOS Thread creation/termination failed
  usbTimerError,          ///< CMSIS-RTOS Timer creation/deletion failed
  usbSemaphoreError,      ///< CMSIS-RTOS Semaphore creation failed
  usbMutexError,          ///< CMSIS-RTOS Mutex creation failed

  usbControllerError = 0x20U, ///< Controller does not exist
  usbDeviceError,             ///< Device does not exist
  usbDriverError,             ///< Driver function produced error
  usbDriverBusy,              ///< Driver function is busy
  usbMemoryError,             ///< Memory management function produced error
  usbNotConfigured,           ///< Device is not configured (is connected)
  usbClassErrorADC,           ///< Audio Device Class (ADC) error (no device or device produced error)
  usbClassErrorCDC,           ///< Communication Device Class (CDC) error (no device or device produced error)
  usbClassErrorHID,           ///< Human Interface Device (HID) error (no device or device produced error)
  usbClassErrorMSC,           ///< Mass Storage Device (MSC) error (no device or device produced error)
  usbClassErrorCustom,        ///< Custom device Class (Class) error (no device or device produced error)
  usbUnsupportedClass,        ///< Unsupported Class

  usbTransferStall = 0x40U, ///< Transfer handshake was stall
  usbTransferError,         ///< Transfer error

  usbUnknownError = 0xFFU ///< Unspecified USB error
} usbStatus;

/* USB Host Constants and Defines */

/// USB Host Notification enumerated constants
typedef enum
{
  USBH_NOTIFY_CONNECT = 0U,         ///< Port connection happened
  USBH_NOTIFY_DISCONNECT,           ///< Port disconnection happened
  USBH_NOTIFY_OVERCURRENT,          ///< Port overcurrent happened
  USBH_NOTIFY_REMOTE_WAKEUP,        ///< Port remote wakeup signaling happened
  USBH_NOTIFY_READY,                ///< Device was successfully enumerated, initialized and is ready for communication
  USBH_NOTIFY_UNKNOWN_DEVICE,       ///< Device was successfully enumerated but there is no driver for it
  USBH_NOTIFY_INSUFFICIENT_POWER,   ///< Device requires more power consumption than available
  USBH_NOTIFY_CONFIGURATION_FAILED, ///< Device was not successfully configured (not enough resources)
  USBH_NOTIFY_INITIALIZATION_FAILED ///< Device was not successfully initialized
} USBH_NOTIFY;

/* USB Host Pipe handle type */
typedef uint32_t USBH_PIPE_HANDLE;

//  ==== USB Host Functions ====

/// \brief Get version of USB Host stack
/// \return                             version (major.minor.revision : mmnnnrrrr decimal)
extern uint32_t USBH_GetVersion(void);

/// \brief Initialize USB Host stack and controller
/// \param[in]     ctrl                 index of USB Host controller.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_Initialize(uint8_t ctrl);

/// \brief De-initialize USB Host stack and controller
/// \param[in]     ctrl                 index of USB Host controller.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_Uninitialize(uint8_t ctrl);

/// \brief Suspend a root HUB port on specified controller
/// \param[in]     ctrl                 index of USB Host controller.
/// \param[in]     port                 root HUB port.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_Port_Suspend(uint8_t ctrl, uint8_t port);

/// \brief Resume a root HUB port on specified controller
/// \param[in]     ctrl                 index of USB Host controller.
/// \param[in]     port                 root HUB port.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_Port_Resume(uint8_t ctrl, uint8_t port);

/// \brief Get index of USB Host controller to which USB Device is connected
/// \param[in]     device               index of USB Device.
/// \return                             index of USB Host controller or non-existing USB Host controller :
///                                       - value != 255 : index of USB Host controller
///                                       - value 255 :    non-existing USB Host controller
extern uint8_t USBH_Device_GetController(uint8_t device);

/// \brief Get index of USB Host Root HUB port to which USB Device is connected
/// \param[in]     device               index of USB Device.
/// \return                             index of USB Host Root HUB port or non-existing USB Host Root HUB port :
///                                       - value <= 15 : index of USB Host Root HUB port
///                                       - value 255 :   non-existing USB Host Root HUB port
extern uint8_t USBH_Device_GetPort(uint8_t device);

/// \brief Get status of USB Device
/// \param[in]     device               index of USB Device.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_Device_GetStatus(uint8_t device);

/// \brief Get communication speed of USB Device
/// \param[in]     device               index of USB Device.
/// \return                             communication speed :
///                                       - USB_SPEED_LOW  = low speed
///                                       - USB_SPEED_FULL = full speed
///                                       - USB_SPEED_HIGH = high speed
extern int32_t USBH_Device_GetSpeed(uint8_t device);

/// \brief Get communication address of USB Device
/// \param[in]     device               index of USB Device.
/// \return                             enumerated address or invalid address :
///                                       - value <= 127 : enumerated address
///                                       - value 255 :    invalid address
extern uint8_t USBH_Device_GetAddress(uint8_t device);

/// \brief Get Vendor ID (VID) of USB Device
/// \param[in]     device               index of USB Device.
/// \return                             Vendor ID.
extern uint16_t USBH_Device_GetVID(uint8_t device);

/// \brief Get Product ID (PID) of USB Device
/// \param[in]     device               index of USB Device.
/// \return                             Product ID.
extern uint16_t USBH_Device_GetPID(uint8_t device);

/// \brief Get String Descriptor of USB Device
/// \param[in]     device               index of USB Device.
/// \param[in]     index                index of string descriptor.
/// \param[in]     language_id          language ID.
/// \param[out]    descriptor_data      pointer to where descriptor data will be read.
/// \param[in]     descriptor_length    maximum descriptor length.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_Device_GetStringDescriptor(uint8_t device, uint8_t index, uint16_t language_id, uint8_t *descriptor_data, uint16_t descriptor_length);

/// \brief Callback function called when some event has happened on corresponding controller and port
/// \param[in]     ctrl                 index of USB Host controller.
/// \param[in]     port                 index of Root HUB port.
/// \param[in]     device               index of USB Device :
///                                       - value <= 127: index of of USB Device for device notifications
///                                       - value 255: no device information for port notifications
/// \param[in]     notify               notification :
///                                       - USBH_NOTIFY_CONNECT               = Port connection happened
///                                       - USBH_NOTIFY_DISCONNECT            = Port disconnection happened
///                                       - USBH_NOTIFY_OVERCURRENT           = Port overcurrent happened
///                                       - USBH_NOTIFY_REMOTE_WAKEUP         = Port remote wakeup signaling happened
///                                       - USBH_NOTIFY_READY                 = Device was successfully enumerated, initialized and is ready for communication
///                                       - USBH_NOTIFY_UNKNOWN_DEVICE        = Device was successfully enumerated but there is no driver for it
///                                       - USBH_NOTIFY_INSUFFICIENT_POWER    = Device requires more power consumption than available
///                                       - USBH_NOTIFY_CONFIGURATION_FAILED  = Device was not successfully configured (not enough resources)
///                                       - USBH_NOTIFY_INITIALIZATION_FAILED = Device was not successfully initialized
extern void USBH_Notify(uint8_t ctrl, uint8_t port, uint8_t device, USBH_NOTIFY notify);

//  ==== USB Host Custom Class Functions ====

/// \brief Get Device instance of Custom Class Device
/// \param[in]     instance             instance of Custom Class Device.
/// \return                             instance of Device or non-existing Device instance :
///                                       - value <= 127 : instance of Device
///                                       - value 255 :    non-existing Device instance
extern uint8_t USBH_CustomClass_GetDevice(uint8_t instance);

/// \brief Get status of Custom Class Device
/// \param[in]     instance             instance of Custom Class Device.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_CustomClass_GetStatus(uint8_t instance);

/// \brief Callback function called when custom class device is connected and needs
///        to configure resources used by custom class device instance
/// \param[in]     device               index of USB Device.
/// \param[in]     ptr_dev_desc         pointer to device descriptor.
/// \param[in]     ptr_cfg_desc         pointer to configuration descriptor.
/// \return                             index of configured custom class device instance or configuration failed :
///                                       - value <= 127 : index of configured custom class device instance
///                                       - value 255 :    configuration failed
extern uint8_t USBH_CustomClass_Configure(uint8_t device, const USB_DEVICE_DESCRIPTOR *ptr_dev_desc, const USB_CONFIGURATION_DESCRIPTOR *ptr_cfg_desc);

/// \brief Callback function called when custom class device is disconnected and needs
///        to de-configure resources used by custom class device instance
/// \param[in]     instance             index of custom class device instance.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_CustomClass_Unconfigure(uint8_t instance);

/// \brief Callback function called when custom class device is connected and needs
///        to initialize custom class device instance
/// \param[in]     instance             index of custom class device instance.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_CustomClass_Initialize(uint8_t instance);

/// \brief Callback function called when custom class device is disconnected and needs
///        to de-initialize custom class device instance
/// \param[in]     instance             index of custom class device instance.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_CustomClass_Uninitialize(uint8_t instance);

/// \brief Create Pipe
/// \param[in]     device               index of USB Device.
/// \param[in]     ep_addr              endpoint address :
///                                       - ep_addr.0..3 : address
///                                       - ep_addr.7 :    direction
/// \param[in]     ep_type              endpoint type.
/// \param[in]     ep_max_packet_size   endpoint maximum packet size.
/// \param[in]     ep_interval          endpoint polling interval.
/// \return                             pipe handle or pipe creation failed :
///                                       - value > 0 : pipe handle
///                                       - value 0 :   pipe creation failed
extern USBH_PIPE_HANDLE USBH_PipeCreate(uint8_t device, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_max_packet_size, uint8_t ep_interval);

/// \brief Update Pipe
/// \param[in]     pipe_hndl            pipe handle.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_PipeUpdate(USBH_PIPE_HANDLE pipe_hndl);

/// \brief Delete Pipe
/// \param[in]     pipe_hndl            pipe handle.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_PipeDelete(USBH_PIPE_HANDLE pipe_hndl);

/// \brief Reset Pipe (reset data toggle)
/// \param[in]     pipe_hndl            pipe handle.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_PipeReset(USBH_PIPE_HANDLE pipe_hndl);

/// \brief Receive data on Pipe
/// \param[in]     pipe_hndl            pipe handle.
/// \param[out]    buf                  buffer that receives data.
/// \param[in]     len                  maximum number of bytes to receive.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_PipeReceive(USBH_PIPE_HANDLE pipe_hndl, uint8_t *buf, uint32_t len);

/// \brief Get result of receive data operation on Pipe
/// \param[in]     pipe_hndl            pipe handle.
/// \return                             number of successfully received data bytes.
extern uint32_t USBH_PipeReceiveGetResult(USBH_PIPE_HANDLE pipe_hndl);

/// \brief Send data on Pipe
/// \param[in]     pipe_hndl            pipe handle.
/// \param[in]     buf                  buffer containing data bytes to send.
/// \param[in]     len                  number of bytes to send.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_PipeSend(USBH_PIPE_HANDLE pipe_hndl, const uint8_t *buf, uint32_t len);

/// \brief Get result of send data operation on Pipe
/// \param[in]     pipe_hndl            pipe handle.
/// \return                             number of successfully sent data bytes.
extern uint32_t USBH_PipeSendGetResult(USBH_PIPE_HANDLE pipe_hndl);

/// \brief Abort send/receive operation on Pipe
/// \param[in]     pipe_hndl            pipe handle.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_PipeAbort(USBH_PIPE_HANDLE pipe_hndl);

/// \brief Do a Control Transfer on Default Pipe
/// \param[in]     device               index of USB Device.
/// \param[in]     setup_packet         pointer to setup packet.
/// \param[in,out] data                 buffer containing data bytes to send or where data should be received in data stage of Control Transfer.
/// \param[in]     len                  number of bytes to send or receive in data stage of Control Transfer.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_ControlTransfer(uint8_t device, const USB_SETUP_PACKET *setup_packet, uint8_t *data, uint32_t len);

/// \brief Standard Device Request on Default Pipe - GET_STATUS
/// \param[in]     device               index of USB Device.
/// \param[in]     recipient            recipient.
/// \param[in]     index                interface or endpoint index.
/// \param[out]    ptr_stat_dat         pointer to where status data should be received.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_GetStatus(uint8_t device, uint8_t recipient, uint8_t index, uint8_t *ptr_stat_dat);

/// \brief Standard Device Request on Default Pipe - CLEAR_FEATURE
/// \param[in]     device               index of USB Device.
/// \param[in]     recipient            recipient.
/// \param[in]     index                interface or endpoint index.
/// \param[in]     feature_selector     feature selector.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_ClearFeature(uint8_t device, uint8_t recipient, uint8_t index, uint8_t feature_selector);

/// \brief Standard Device Request on Default Pipe - SET_FEATURE
/// \param[in]     device               index of USB Device.
/// \param[in]     recipient            recipient.
/// \param[in]     index                interface or endpoint index.
/// \param[in]     feature_selector     feature selector.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_SetFeature(uint8_t device, uint8_t recipient, uint8_t index, uint8_t feature_selector);

/// \brief Standard Device Request on Default Pipe - SET_ADDRESS
/// \param[in]     device               index of USB Device.
/// \param[in]     device_address       device address.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_SetAddress(uint8_t device, uint8_t device_address);

/// \brief Standard Device Request on Default Pipe - GET_DESCRIPTOR
/// \param[in]     device               index of USB Device.
/// \param[in]     recipient            recipient.
/// \param[in]     descriptor_type      descriptor type.
/// \param[in]     descriptor_index     descriptor index.
/// \param[in]     language_id          language ID.
/// \param[out]    descriptor_data      pointer to where descriptor data will be read.
/// \param[in]     descriptor_length    maximum descriptor length.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_GetDescriptor(uint8_t device, uint8_t recipient, uint8_t descriptor_type, uint8_t descriptor_index, uint16_t language_id, uint8_t *descriptor_data, uint16_t descriptor_length);

/// \brief Standard Device Request on Default Pipe - SET_DESCRIPTOR
/// \param[in]     device               index of USB Device.
/// \param[in]     recipient            recipient.
/// \param[in]     descriptor_type      descriptor type.
/// \param[in]     descriptor_index     descriptor index.
/// \param[in]     language_id          language ID.
/// \param[in]     descriptor_data      pointer to descriptor data to be written.
/// \param[in]     descriptor_length    descriptor length.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_SetDescriptor(uint8_t device, uint8_t recipient, uint8_t descriptor_type, uint8_t descriptor_index, uint16_t language_id, const uint8_t *descriptor_data, uint16_t descriptor_length);

/// \brief Standard Device Request on Default Pipe - GET_CONFIGURATION
/// \param[in]     device               index of USB Device.
/// \param[out]    ptr_configuration    pointer to where configuration will be read.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_GetConfiguration(uint8_t device, uint8_t *ptr_configuration);

/// \brief Standard Device Request on Default Pipe - SET_CONFIGURATION
/// \param[in]     device               index of USB Device.
/// \param[in]     configuration        configuration.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_SetConfiguration(uint8_t device, uint8_t configuration);

/// \brief Standard Device Request on Default Pipe - GET_INTERFACE
/// \param[in]     device               index of USB Device.
/// \param[in]     index                interface index.
/// \param[out]    ptr_alternate        pointer to where alternate setting data will be read.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_GetInterface(uint8_t device, uint8_t index, uint8_t *ptr_alternate);

/// \brief Standard Device Request on Default Pipe - SET_INTERFACE
/// \param[in]     device               index of USB Device.
/// \param[in]     index                interface index.
/// \param[in]     alternate            alternate setting.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_SetInterface(uint8_t device, uint8_t index, uint8_t alternate);

/// \brief Standard Device Request on Default Pipe - SYNCH_FRAME
/// \param[in]     device               index of USB Device.
/// \param[in]     index                interface or endpoint index.
/// \param[out]    ptr_frame_number     pointer to where frame number data will be read.
/// \return                             status code that indicates the execution status of the function as defined with \ref usbStatus.
extern usbStatus USBH_DeviceRequest_SynchFrame(uint8_t device, uint8_t index, uint8_t *ptr_frame_number);

// below is a set of extended functions for isochronous transfer
typedef void (*USBH_CB_ISR_Isoch_transfer)(uint32_t *payload, uint32_t length);

extern USBH_PIPE_HANDLE USBH_Pipe_ISOCH_PipeDelete(USBH_PIPE_HANDLE pipe_hndl);
extern USBH_PIPE_HANDLE USBH_Pipe_ISOCH_PipeCreate(uint8_t device, uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval, uint8_t *buf, uint32_t buf_size);
extern usbStatus USBH_Pipe_ISOCH_Start(USBH_PIPE_HANDLE pipe_hndl, USBH_CB_ISR_Isoch_transfer isoch_cb);
extern usbStatus USBH_Pipe_ISOCH_Stop(USBH_PIPE_HANDLE pipe_hndl);

#endif
