//#define FIFO_CMD_DBG

#ifdef FIFIOQ_LOG_VIA_USB

#include <string.h>

#include "cmsis_os2.h"

#include "kmdw_console.h"

#include "usbd_hal.h"

// Print log through interrupt endpoint, only called from logger_thread
static void send_log_via_usb(const char *str)
{
    if (usbd_hal_interrupt_send_check_buffer_empty(KDP2_USB_ENDPOINT_LOG_IN))
    {
        usbd_hal_interrupt_send(KDP2_USB_ENDPOINT_LOG_IN, (void *)(str), strlen(str) + 1, osWaitForever);
    }
}

int kdp2_usb_log_initialize()
{
    kmdw_console_hook_callback(&send_log_via_usb);
    return KMDW_STATUS_OK;
}

#endif
