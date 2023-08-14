//#define FIFO_CMD_DBG

#ifdef FIFIOQ_LOG_VIA_USB

#include <string.h>

#include "cmsis_os2.h"

#include "kmdw_console.h"

#include "usbd_hal.h"

static char *s_usb_log_buf = NULL;
static int s_usb_log_buf_write_pos = 0;

void fifoq_log_send_usb(const char *str);

int kdp2_usb_log_initialize()
{
    s_usb_log_buf = (char *)kmdw_ddr_reserve(1024);
    if (s_usb_log_buf == NULL)
        return KMDW_STATUS_ERROR;

    kmdw_console_hook_callback(&fifoq_log_send_usb);
    return KMDW_STATUS_OK;
}

// Print log through interrupt endpoint, only called from logger_thread
void fifoq_log_send_usb(const char *str)
{
    if (usbd_hal_interrupt_send_check_buffer_empty(KDP2_USB_ENDPOINT_LOG_IN))
        s_usb_log_buf_write_pos = 0;

    // Reset write pos
    if (s_usb_log_buf_write_pos + strlen(str) + 1 > 1000)
        s_usb_log_buf_write_pos = 0;

    memcpy(s_usb_log_buf + s_usb_log_buf_write_pos, str, strlen(str) + 1);
    s_usb_log_buf_write_pos += strlen(str);

    usbd_hal_interrupt_send(KDP2_USB_ENDPOINT_LOG_IN, (void *)(s_usb_log_buf), s_usb_log_buf_write_pos + 1, 1);
}

#endif
