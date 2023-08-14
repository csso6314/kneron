#include "usbd_hal.h"
#include "kdrv_usbd2v.h"

#include <string.h>

#include "kmdw_console.h"

#ifdef FIFIOQ_LOG_VIA_USB
#define NUM_ENDPOINT 3
#else
#define NUM_ENDPOINT 2
#endif

static kdrv_usbd2v_endpoint_descriptor_t hs_enp_data_out =
    {
        .bLength = 0x07,                                // 7 bytes
        .bDescriptorType = 0x05,                        // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_DATA_OUT, //
        .bmAttributes = 0x02,                           // TransferType = Bulk
        .wMaxPacketSize = 512,                          // max 512 bytes
        .bInterval = 0x00,                              // never NAKs
};

static kdrv_usbd2v_endpoint_descriptor_t hs_enp_data_in =
    {
        .bLength = 0x07,                               // 7 bytes
        .bDescriptorType = 0x05,                       // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_DATA_IN, //
        .bmAttributes = 0x02,                          // TransferType = Bulk
        .wMaxPacketSize = 512,                         // max 512 bytes
        .bInterval = 0x00,                             // never NAKs
};

#ifdef FIFIOQ_LOG_VIA_USB
static kdrv_usbd2v_endpoint_descriptor_t hs_enp_log_in =
    {
        .bLength = 0x07,                              // 7 bytes
        .bDescriptorType = 0x05,                      // Endpoint Descriptor
        .bEndpointAddress = KDP2_USB_ENDPOINT_LOG_IN, //
        .bmAttributes = 0x03,                         // TransferType = Interrupt
        .wMaxPacketSize = 1024,                       // max 1024 bytes
        .bInterval = 1,                               // interval
};
#endif

kdrv_usbd2v_interface_descriptor_t intf_desc =
    {
        .bLength = 0x9,                  // 9 bytes
        .bDescriptorType = 0x04,         // Inteface Descriptor
        .bInterfaceNumber = 0x0,         // Interface Number
        .bAlternateSetting = 0x0,        //
        .bNumEndpoints = NUM_ENDPOINT,   // 4 endpoints
        .bInterfaceClass = 0xFF,         // Vendor specific
        .bInterfaceSubClass = 0x0,       // 3rd party uses 0x01
        .bInterfaceProtocol = 0x0,       // 3rd party uses 0x50
        .iInterface = 0x0,               // No String Descriptor
        .endpoint[0] = &hs_enp_data_out, // receive image or command from host
        .endpoint[1] = &hs_enp_data_in,  // send command response back to host
#ifdef FIFIOQ_LOG_VIA_USB
        .endpoint[2] = &hs_enp_log_in, // send logs to host
#endif
};

kdrv_usbd2v_config_descriptor_t confg_desc =
    {
        .bLength = 0x09,                                                                    // 9 bytes
        .bDescriptorType = 0x02,                                                            // Type: Configuration Descriptor
        .wTotalLength = (9 + 9 + NUM_ENDPOINT * sizeof(kdrv_usbd2v_endpoint_descriptor_t)), // stotal bytes including config/interface/endpoint descriptors
        .bNumInterfaces = 0x1,                                                              // Number of interfaces
        .bConfigurationValue = 0x1,                                                         // Configuration number
        .iConfiguration = 0x0,                                                              // No String Descriptor
        .bmAttributes = 0xC0,                                                               // Self-powered, no Remote wakeup
        .MaxPower = 0x0,                                                                    // 0 syould be ok for self-powered device
        .interface = &intf_desc,
};

kdrv_usbd2v_device_descriptor_t dev_desc =
    {
        .bLength = 0x12,         // 18 bytes
        .bDescriptorType = 0x01, // Type : Device Descriptor
        .bcdUSB = 0x200,         // USB 2.0
        .bDeviceClass = 0x00,    // Device class, 0x0: defined by the interface descriptors
        .bDeviceSubClass = 0x00, // Device sub-class
        .bDeviceProtocol = 0x00, // Device protocol
        .bMaxPacketSize0 = 0x40, // Max EP0 packet size: 64 bytes
        .idVendor = 0x3231,      // Vendor ID
        .idProduct = 0x0100,     // Product ID
        .bcdDevice = 0x00B0,     // Device release number
        .iManufacturer = 0x01,   // Manufacture string index, FIXME
        .iProduct = 0x02,        // Product string index, FIXME
        .iSerialNumber = 0x3,    // Serial number string index
        .bNumConfigurations = 1, // Number of configurations, FIXME
        .config = &confg_desc,   // configuration descriptor
};

#define USB_MANUFACTURER_STRING                        \
    {                                                  \
        'K', 0, 'n', 0, 'e', 0, 'r', 0, 'o', 0, 'n', 0 \
    }
#define USB_PRODUCT_STRING                                                                             \
    {                                                                                                  \
        'K', 0, 'n', 0, 'e', 0, 'r', 0, 'o', 0, 'n', 0, ' ', 0, 'K', 0, 'L', 0, '5', 0, '2', 0, '0', 0 \
    }
#define USB_SERIAL_STRING                                              \
    {                                                                  \
        '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0 \
    }

static kdrv_usbd2v_prd_string_descriptor_t str_desc_mun =
    {
        .bLength = (2 + 12),
        .bDescriptorType = 0x03,
        .bString = USB_MANUFACTURER_STRING,
};

static kdrv_usbd2v_prd_string_descriptor_t str_desc_prd =
    {
        .bLength = (2 + 24),
        .bDescriptorType = 0x03,
        .bString = USB_PRODUCT_STRING,
};

static kdrv_usbd2v_prd_string_descriptor_t str_desc_serial =
    {
        .bLength = (2 + 16),
        .bDescriptorType = 0x03,
        .bString = USB_SERIAL_STRING,
};

static kdrv_usbd2v_string_descriptor_t str_desc =
    {
        .bLength = 4,
        .bDescriptorType = 0x03,
        .bLanguageID = 0x0409,
        .desc[0] = &str_desc_mun,
        .desc[1] = &str_desc_prd,
        .desc[2] = &str_desc_serial,
};

kdrv_status_t usbd_hal_initialize(
    uint8_t *serial_string,
    uint16_t bcdDevice,
    usbd_hal_user_link_status_callback_t usr_link_isr_cb,
    usbd_hal_user_control_callback_t usr_cx_isr_cb)
{
    memcpy((void *)str_desc_serial.bString, (void *)serial_string, sizeof(str_desc_serial.bString));

    dev_desc.bcdDevice = bcdDevice;

    return kdrv_usbd2v_initialize(&dev_desc, &str_desc, (kdrv_usbd2v_link_status_callback_t)usr_link_isr_cb, (kdrv_usbd2v_user_control_callback_t)usr_cx_isr_cb);
}

kdrv_status_t usbd_hal_set_enable(bool enable)
{
    return kdrv_usbd2v_set_enable(true);
}

kdrv_status_t usbd_hal_bulk_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    return kdrv_usbd2v_bulk_send(endpoint, buf, txLen, timeout_ms);
}

kdrv_status_t usbd_hal_bulk_receive(uint32_t endpoint, uint32_t *buf, uint32_t *blen, uint32_t timeout_ms)
{
    return kdrv_usbd2v_bulk_receive(endpoint, buf, blen, timeout_ms);
}

bool usbd_hal_interrupt_send_check_buffer_empty(uint32_t endpoint)
{
    return kdrv_usbd2v_interrupt_send_check_buffer_empty(endpoint);
}

kdrv_status_t usbd_hal_interrupt_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    return kdrv_usbd2v_interrupt_send(endpoint, buf, txLen, timeout_ms);
}

kdrv_status_t usbd_hal_terminate_all_endpoint(void)
{
    return kdrv_usbd2v_terminate_all_endpoint();
}

usbd_hal_link_status_t usbd_hal_get_link_status(){
    kdrv_usbd2v_link_status_t t = kdrv_usbd2v_get_link_status();
    usbd_hal_link_status_t ret = USBD_STATUS_DISCONNECTED;
    if(t == USBD2_STATUS_DISCONNECTED){
        ret = USBD_STATUS_DISCONNECTED;
    }
    else if(t == USBD2_STATUS_CONFIGURED){
        ret = USBD_STATUS_CONFIGURED;
    }
    return ret;
}
