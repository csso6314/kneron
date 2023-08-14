#define WORKAROUND_NO_USB_SELF_REBOOT // Do self-reboot if no usb connectabion

#include "usbd_hal.h"
#include "kdrv_usbd3.h"

#include "kmdw_console.h"

#include <string.h>

#ifdef FIFIOQ_LOG_VIA_USB
#define NUM_ENDPOINT 3
#else
#define NUM_ENDPOINT 2
#endif

/* ============= High-Speed descriptors ============= */

static kdrv_usbd3_device_descriptor_t hs_dev_descp =
    {
        .bLength = 18,           // 18 bytes
        .bDescriptorType = 0x01, // Type : Device Descriptor
        .bcdUSB = 0x210,         // USB 2.10
        .bDeviceClass = 0x00,    // Device class, 0x0: defined by the interface descriptors
        .bDeviceSubClass = 0x00, // Device sub-class
        .bDeviceProtocol = 0x00, // Device protocol
        .bMaxPacketSize0 = 0x40, // Max EP0 packet size: 64 bytes
        .idVendor = 0x3231,      // Vendor ID
        .idProduct = 0x0720,     // Product ID
        .bcdDevice = 0x00B1,     // Device release number
        .iManufacturer = 0x01,   // Manufacture string index, FIXME
        .iProduct = 0x02,        // Product string index, FIXME
        .iSerialNumber = 0x3,    // Serial number string index
        .bNumConfigurations = 1, // Number of configurations, FIXME
};

#define HS_CONFIG_TOTAL_LENGTH (9 + 9 + NUM_ENDPOINT * 7)
static kdrv_usbd3_config_descriptor_t hs_confg_desc =
    {
        .bLength = 9,                           // 9 bytes
        .bDescriptorType = 0x02,                // Type: Configuration Descriptor
        .wTotalLength = HS_CONFIG_TOTAL_LENGTH, // total bytes including config/interface/endpoint descriptors
        .bNumInterfaces = 1,                    // Number of interfaces
        .bConfigurationValue = 0x1,             // Configuration number
        .iConfiguration = 0x0,                  // No String Descriptor
        .bmAttributes = 0xC0,                   // Self-powered, no Remote wakeup
        .MaxPower = 0xFA,                       // 500 mA
};

static kdrv_usbd3_interface_descriptor_t hs_intf_desc =
    {
        .bLength = 9,                  // 9 bytes
        .bDescriptorType = 0x04,       // Inteface Descriptor
        .bInterfaceNumber = 0x0,       // Interface Number
        .bAlternateSetting = 0x0,      //
        .bNumEndpoints = NUM_ENDPOINT, // number of endpoint
        .bInterfaceClass = 0xFF,       // Vendor specific
        .bInterfaceSubClass = 0x0,     //
        .bInterfaceProtocol = 0x0,     //
        .iInterface = 0x0,             // Interface descriptor string index
};

static kdrv_usbd3_endpoint_descriptor_t hs_enp_data_in =
    {
        .bLength = 7,                                  // 7 bytes
        .bDescriptorType = 0x05,                       // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_DATA_IN, //
        .bmAttributes = 0x02,                          // TransferType = Bulk
        .wMaxPacketSize = 512,                         // max 512 bytes
        .bInterval = 0x00,                             // never NAKs
};

static kdrv_usbd3_endpoint_descriptor_t hs_enp_data_out =
    {
        .bLength = 7,                                   // 7 bytes
        .bDescriptorType = 0x05,                        // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_DATA_OUT, //
        .bmAttributes = 0x02,                           // TransferType = Bulk
        .wMaxPacketSize = 512,                          // max 512 bytes
        .bInterval = 0x00,                              // never NAKs
};

#ifdef FIFIOQ_LOG_VIA_USB
static kdrv_usbd3_endpoint_descriptor_t hs_enp_log_in =
    {
        .bLength = 7,                                 // 7 bytes
        .bDescriptorType = 0x05,                      // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_LOG_IN, //
        .bmAttributes = 0x03,                         // TransferType = Interrupt
        .wMaxPacketSize = 1024,                       // max 1024 bytes
        .bInterval = 1,                               // interval
};
#endif

static kdrv_usbd3_HS_descriptors_t hs_descps =
    {
        .dev_descp = &hs_dev_descp,
        .config_descp = &hs_confg_desc,
        .intf_descp = &hs_intf_desc,
        .enp_descp[0] = &hs_enp_data_in,
        .enp_descp[1] = &hs_enp_data_out,
#ifdef FIFIOQ_LOG_VIA_USB
        .enp_descp[2] = &hs_enp_log_in,
#endif
        .qual_descp = NULL,
};

/* ============= Super-Speed descriptors ============= */

static kdrv_usbd3_device_descriptor_t ss_dev_descp =
    {
        .bLength = 18,           // 18 bytes
        .bDescriptorType = 0x01, // Type : Device Descriptor
        .bcdUSB = 0x300,         // USB 3.0
        .bDeviceClass = 0x00,    // Device class, 0x0: defined by the interface descriptors
        .bDeviceSubClass = 0x00, // Device sub-class
        .bDeviceProtocol = 0x00, // Device protocol
        .bMaxPacketSize0 = 0x9,  // Maxpacket size for EP0 : 2^9
        .idVendor = 0x3231,      // Vendor ID
        .idProduct = 0x0720,     // Product ID
        .bcdDevice = 0x00B1,     // Device release number
        .iManufacturer = 0x01,   // Manufacture string index, FIXME
        .iProduct = 0x02,        // Product string index, FIXME
        .iSerialNumber = 0x3,    // Serial number string index
        .bNumConfigurations = 1, // Number of configurations, FIXME
};

#define SS_CONFIG_TOTAL_LENGTH (9 + 9 + NUM_ENDPOINT * (7 + 6))
static kdrv_usbd3_config_descriptor_t ss_confg_desc =
    {
        .bLength = 9,                           // 9 bytes
        .bDescriptorType = 0x02,                // Type: Configuration Descriptor
        .wTotalLength = SS_CONFIG_TOTAL_LENGTH, // total bytes including config/interface/endpoint/companion descriptors
        .bNumInterfaces = 1,                    // Number of interfaces
        .bConfigurationValue = 0x1,             // Configuration number
        .iConfiguration = 0x0,                  // No String Descriptor
        .bmAttributes = 0xC0,                   // Self-powered, no Remote wakeup
        .MaxPower = 0x0,                        // 0
};

static kdrv_usbd3_interface_descriptor_t ss_intf_desc =
    {
        .bLength = 9,                  // 9 bytes
        .bDescriptorType = 0x04,       // Inteface Descriptor
        .bInterfaceNumber = 0x0,       // Interface Number
        .bAlternateSetting = 0x0,      //
        .bNumEndpoints = NUM_ENDPOINT, // number of endpoints
        .bInterfaceClass = 0xFF,       // Vendor specific
        .bInterfaceSubClass = 0x0,     //
        .bInterfaceProtocol = 0x0,     //
        .iInterface = 0x0,             // Interface descriptor string index
};

static kdrv_usbd3_endpoint_descriptor_t ss_enp_data_in =
    {
        .bLength = 7,                                  // 7 bytes
        .bDescriptorType = 0x05,                       // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_DATA_IN, //
        .bmAttributes = 0x02,                          // TransferType = Bulk
        .wMaxPacketSize = 1024,                        // max 1024 bytes
        .bInterval = 0x00,                             // never NAKs
};

static kdrv_usbd3_endpoint_descriptor_t ss_enp_data_out =
    {
        .bLength = 7,                                   // 7 bytes
        .bDescriptorType = 0x05,                        // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_DATA_OUT, //
        .bmAttributes = 0x02,                           // TransferType = Bulk
        .wMaxPacketSize = 1024,                         // max 1024 bytes
        .bInterval = 0x00,                              // never NAKs
};

#ifdef FIFIOQ_LOG_VIA_USB
static kdrv_usbd3_endpoint_descriptor_t ss_enp_log_in =
    {
        .bLength = 7,                                 // 7 bytes
        .bDescriptorType = 0x05,                      // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_LOG_IN, //
        .bmAttributes = 0x03,                         // TransferType = Interrupt
        .wMaxPacketSize = 1024,                       // max 1024 bytes
        .bInterval = 1,                               // never NAKs
};
#endif

static kdrv_usbd3_endpoint_companion_descriptor_t ss_desc_bulk_comp =
    {
        .bLength = 6,            // 6 bytes
        .bDescriptorType = 0x30, // Endpoint Companion Descriptor
        .bMaxBurst = 0,          // burst size = 1
        .bmAttributes = 0x0,     // no stream
        .wBytesPerInterval = 0,  // 0 for bulk
};

#ifdef FIFIOQ_LOG_VIA_USB
static kdrv_usbd3_endpoint_companion_descriptor_t ss_desc_intrpt_comp =
    {
        .bLength = 6,              // 6 bytes
        .bDescriptorType = 0x30,   // Endpoint Companion Descriptor
        .bMaxBurst = 0,            // burst size = 1
        .bmAttributes = 0x0,       // no stream
        .wBytesPerInterval = 1024, //
};
#endif

static kdrv_usbd3_SS_descriptors_t ss_descps =
    {
        .dev_descp = &ss_dev_descp,
        .config_descp = &ss_confg_desc,
        .intf_descp = &ss_intf_desc,
        .enp_descp[0] = &ss_enp_data_in,
        .enp_descp[1] = &ss_enp_data_out,
#ifdef FIFIOQ_LOG_VIA_USB
        .enp_descp[2] = &ss_enp_log_in,
#endif
        .enp_cmpn_descp[0] = &ss_desc_bulk_comp,
        .enp_cmpn_descp[1] = &ss_desc_bulk_comp,
#ifdef FIFIOQ_LOG_VIA_USB
        .enp_cmpn_descp[2] = &ss_desc_intrpt_comp,
#endif
};

#define USB_MANUFACTURER_STRING                        \
    {                                                  \
        'K', 0, 'n', 0, 'e', 0, 'r', 0, 'o', 0, 'n', 0 \
    }
#define USB_PRODUCT_STRING                                                                             \
    {                                                                                                  \
        'K', 0, 'n', 0, 'e', 0, 'r', 0, 'o', 0, 'n', 0, ' ', 0, 'K', 0, 'L', 0, '7', 0, '2', 0, '0', 0 \
    }
#define USB_SERIAL_STRING                                              \
    {                                                                  \
        '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0 \
    }

static kdrv_usbd3_prd_string_descriptor_t str_desc_mun =
    {
        .bLength = (2 + 12),
        .bDescriptorType = 0x03,
        .bString = USB_MANUFACTURER_STRING,
};

static kdrv_usbd3_prd_string_descriptor_t str_desc_prd =
    {
        .bLength = (2 + 24),
        .bDescriptorType = 0x03,
        .bString = USB_PRODUCT_STRING,
};

static kdrv_usbd3_prd_string_descriptor_t str_desc_serial =
    {
        .bLength = (2 + 16),
        .bDescriptorType = 0x03,
        .bString = USB_SERIAL_STRING,
};

static kdrv_usbd3_string_descriptor_t str_desc =
    {
        .bLength = 4,
        .bDescriptorType = 0x03,
        .bLanguageID = 0x0409,
        .desc[0] = &str_desc_mun,
        .desc[1] = &str_desc_prd,
        .desc[2] = &str_desc_serial,
};

static osSemaphoreId_t recv_bulk_mutex;
static osSemaphoreId_t send_bulk_mutex;

kdrv_status_t usbd_hal_initialize(
    uint8_t *serial_string,
    uint16_t bcdDevice,
    usbd_hal_user_link_status_callback_t usr_link_isr_cb,
    usbd_hal_user_control_callback_t usr_cx_isr_cb)
{
    recv_bulk_mutex = osMutexNew(NULL); // for usb bulk recv
    send_bulk_mutex = osMutexNew(NULL); // for usb bulk send

    hs_dev_descp.bcdDevice = bcdDevice;
    ss_dev_descp.bcdDevice = bcdDevice;

    memcpy((void *)str_desc_serial.bString, (void *)serial_string, sizeof(str_desc_serial.bString));
    return kdrv_usbd3_initialize(&hs_descps, &ss_descps, &str_desc, (kdrv_usbd3_link_status_callback_t)usr_link_isr_cb, (kdrv_usbd3_user_control_callback_t)usr_cx_isr_cb);
}

kdrv_status_t usbd_hal_set_enable(bool enable)
{
    return kdrv_usbd3_set_enable(true);
}

kdrv_status_t usbd_hal_bulk_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    osMutexAcquire(send_bulk_mutex, osWaitForever);
    kdrv_status_t sts = kdrv_usbd3_bulk_send(endpoint, buf, txLen, timeout_ms);
    osMutexRelease(send_bulk_mutex);

    return sts;
}

kdrv_status_t usbd_hal_bulk_receive(uint32_t endpoint, uint32_t *buf, uint32_t *blen, uint32_t timeout_ms)
{
    osMutexAcquire(recv_bulk_mutex, osWaitForever);
    kdrv_status_t sts = kdrv_usbd3_bulk_receive(endpoint, buf, blen, timeout_ms);
    osMutexRelease(recv_bulk_mutex);

    return sts;
}

bool usbd_hal_interrupt_send_check_buffer_empty(uint32_t endpoint)
{
    return kdrv_usbd3_interrupt_send_check_buffer_empty(endpoint);
}

kdrv_status_t usbd_hal_interrupt_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    return kdrv_usbd3_interrupt_send(endpoint, buf, txLen, timeout_ms);
}

kdrv_status_t usbd_hal_terminate_all_endpoint(void)
{
    kdrv_usbd3_reset_endpoint(KDP2_USB_ENDPOINT_DATA_IN);
    kdrv_usbd3_reset_endpoint(KDP2_USB_ENDPOINT_DATA_OUT);
    return KDRV_STATUS_OK;
}

kdrv_status_t usbd_hal_terminate_endpoint(uint32_t endpoint)
{
    kdrv_usbd3_reset_endpoint(endpoint);
    return KDRV_STATUS_OK;
}

usbd_hal_link_status_t usbd_hal_get_link_status()
{
    kdrv_usbd3_link_status_t t = kdrv_usbd3_get_link_status();
    usbd_hal_link_status_t ret = USBD_STATUS_DISCONNECTED;
    if(t == USBD3_STATUS_DISCONNECTED){
        ret = USBD_STATUS_DISCONNECTED;
    }
    else if(t == USBD3_STATUS_CONFIGURED){
        ret = USBD_STATUS_CONFIGURED;
    }
    return ret;
}

void usbd_hal_reset_device(void *arg)
{
    (void)arg;
    kdrv_usbd3_reset_device();
}
