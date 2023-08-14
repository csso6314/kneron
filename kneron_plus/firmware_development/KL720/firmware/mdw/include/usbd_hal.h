#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "kdrv_status.h"

#define KDP2_USB_ENDPOINT_DATA_IN 0x81  // endpoint for command input
#define KDP2_USB_ENDPOINT_DATA_OUT 0x02 // endpoint for inference image input or command input

#ifdef FIFIOQ_LOG_VIA_USB
#define KDP2_USB_ENDPOINT_LOG_IN 0x83 // endpoint for log input
#endif

typedef enum
{
    USBD_STATUS_DISCONNECTED = 0x0,
    USBD_STATUS_CONFIGURED, // connected
} usbd_hal_link_status_t;

typedef struct __attribute__((__packed__))
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} usbd_hal_setup_packet_t;

typedef void (*usbd_hal_user_link_status_callback_t)(usbd_hal_link_status_t link_status);
typedef bool (*usbd_hal_user_control_callback_t)(usbd_hal_setup_packet_t *setup);

kdrv_status_t usbd_hal_initialize(
    uint8_t *serial_string,
    uint16_t bcdDevice,
    usbd_hal_user_link_status_callback_t usr_link_isr_cb,
    usbd_hal_user_control_callback_t usr_cx_isr_cb);

kdrv_status_t usbd_hal_set_enable(bool enable);

kdrv_status_t usbd_hal_bulk_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms);

kdrv_status_t usbd_hal_bulk_receive(uint32_t endpoint, uint32_t *buf, uint32_t *blen, uint32_t timeout_ms);

bool usbd_hal_interrupt_send_check_buffer_empty(uint32_t endpoint);

kdrv_status_t usbd_hal_interrupt_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms);

kdrv_status_t usbd_hal_terminate_all_endpoint(void);

kdrv_status_t usbd_hal_terminate_endpoint(uint32_t endpoint);

usbd_hal_link_status_t usbd_hal_get_link_status(void);

void usbd_hal_reset_device(void *arg);
