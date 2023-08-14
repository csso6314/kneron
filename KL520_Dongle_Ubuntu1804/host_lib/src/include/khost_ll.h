#pragma once

// Khost Low Level API

typedef void *khost_device_t;

typedef enum
{
    KNERON_KL520 = 0x100,
    KNERON_KL720 = 0x200,
} kneron_product_id_t;

typedef enum
{
    CONTROL_RESET_HARDWARE = 0x1, // chip hard reset
    CONTROL_FIFOQ_RESET = 0x10,   // make fifo queue clean and start-over
    CONTROL_FIFOQ_INFO = 0x11,    // make fifo queue clean and start-over
    CONTROL_MAX_COMMANDS = 255,
} khost_control_command_t;

typedef enum
{
    STM_UNKNOWN = -1,
    STM_START = 0xC0,
    STM_ALGO_INFERENCE = 0xC1,
} khost_STM_t;

typedef enum
{
    KHOST_RET_OK = 0,
    KHOST_RET_ERR = -99,
    KHOST_RET_UNSUPPORTED = -100,
    KHOST_RET_TIMEOUT = -7
} khost_status_t;

enum usb_speed
{
    USB_SPEED_UNKNOWN = 0,
    USB_SPEED_LOW = 1,
    USB_SPEED_FULL = 2,
    USB_SPEED_HIGH = 3,
    USB_SPEED_SUPER = 4,
};

typedef struct
{
    khost_control_command_t command;
    unsigned short arg1;
    unsigned short arg2;
} khost_control_t;

typedef struct
{
    int scan_index;
    bool isConnectable;
    unsigned short vendor_id;
    unsigned short product_id;
    enum usb_speed link_speed;
    unsigned int serial_number;
    char device_path[20]; // "busNo-hub_portNo-device_portNo", for ex: "1-2-3", means bus 1 - (hub) port 2 - (device) port 3
} kneron_device_info_t;

typedef struct
{
    int num_dev;
    kneron_device_info_t kdevice[1]; // real index range from 0 ~ (num_dev-1)
} kneron_device_info_list_t;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

    // scan all Kneron connectable devices and report a list.
    // pdev_list is an input, the API will allocate memory and fullfill the content.
    // user may call free() to free memory allocated for the list.
    int khost_ll_scan_devices(kneron_device_info_list_t **pkdev_list);

    // connect to Kneron device
    // scan_index can be retrieve from khost_ll_scan_devices()
    khost_device_t khost_ll_connect_device(int scan_index);

    // disconnect device
    int khost_ll_disconnect_device(khost_device_t dev);

    kneron_device_info_t *khost_ll_get_device_info(khost_device_t dev);

    int khost_ll_control(khost_device_t dev, khost_control_t *control_request, int timeout);

    // return 0 (KHOST_RET_OK) on success, or < 0 if failed
    // timeout in milliseconds, 0 means blocking wait, if timeout it returns KHOST_RET_TIMEOUT
    int khost_ll_write_data(khost_device_t dev, void *buf, int len, int timeout);

    // return read size on success, or < 0 if failed
    // timeout in milliseconds, 0 means blocking wait, if timeout it returns KHOST_RET_TIMEOUT
    int khost_ll_read_data(khost_device_t dev, void *buf, int len, int timeout);

    // return 0 (KHOST_RET_OK) on success, or < 0 if failed
    // timeout in milliseconds, 0 means blocking wait, if timeout it returns KHOST_RET_TIMEOUT
    int khost_ll_write_command(khost_device_t dev, void *buf, int len, int timeout);

    // return read size on success, or < 0 if failed
    // timeout in milliseconds, 0 means blocking wait, if timeout it returns KHOST_RET_TIMEOUT
    int khost_ll_read_command(khost_device_t dev, void *buf, int len, int timeout);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
