#ifndef __KDP2_LL_H__
#define __KDP2_LL_H__

#include <stdbool.h>
#include "kdp2_struct.h"

// Khost Low Level API

typedef void *kdp2_ll_device_t;

typedef enum
{
    KDP2_LL_CTRL_HARD_RESET = 0x1, // chip hard reset
    KDP2_LL_CTRL_INFERENCE_RESET = 0x10,   // make fifo queue clean and start-over
    KDP2_LL_CTRL_GET_FIFOQ_INFO = 0x11,    // make fifo queue clean and start-over
    KDP2_LL_CTRL_MAX = 255,
} kdp2_ll_control_command_t;

typedef enum
{
    KDP2_LL_RET_OK = 0,
    KDP2_LL_RET_ERR = -99,
    KDP2_LL_RET_UNSUPPORTED = -100,
    KDP2_LL_RET_TIMEOUT = -7
} kdp2_ll_status_t;

typedef struct
{
    kdp2_ll_control_command_t command;
    unsigned short arg1;
    unsigned short arg2;
} kdp2_ll_control_t;


// scan all Kneron connectable devices and report a list.
// pdev_list is an input, the API will allocate memory and fullfill the content.
// user may call free() to free memory allocated for the list.
int kdp2_ll_scan_devices(kdp2_devices_list_t **pkdev_list);

// connect to Kneron device
// scan_index can be retrieve from kdp2_ll_scan_devices()
kdp2_ll_device_t kdp2_ll_connect_device(int scan_index);

// disconnect device
int kdp2_ll_disconnect_device(kdp2_ll_device_t dev);

kdp2_device_descriptor_t *kdp2_ll_get_device_descriptor(kdp2_ll_device_t dev);

int kdp2_ll_control(kdp2_ll_device_t dev, kdp2_ll_control_t *control_request, int timeout);

// return 0 (KDP2_LL_RET_OK) on success, or < 0 if failed
// timeout in milliseconds, 0 means blocking wait, if timeout it returns KDP2_LL_RET_TIMEOUT
int kdp2_ll_write_data(kdp2_ll_device_t dev, void *buf, int len, int timeout);

// return read size on success, or < 0 if failed
// timeout in milliseconds, 0 means blocking wait, if timeout it returns KDP2_LL_RET_TIMEOUT
int kdp2_ll_read_data(kdp2_ll_device_t dev, void *buf, int len, int timeout);

// return 0 (KDP2_LL_RET_OK) on success, or < 0 if failed
// timeout in milliseconds, 0 means blocking wait, if timeout it returns KDP2_LL_RET_TIMEOUT
int kdp2_ll_write_command(kdp2_ll_device_t dev, void *buf, int len, int timeout);

// return read size on success, or < 0 if failed
// timeout in milliseconds, 0 means blocking wait, if timeout it returns KDP2_LL_RET_TIMEOUT
int kdp2_ll_read_command(kdp2_ll_device_t dev, void *buf, int len, int timeout);

#endif
