#include "cmsis_os2.h"

#include "kdrv_usbd3.h"

#include "usbd_com.h"
#include "kmdw_console.h"
#include "dual_fifo.h"

#define USBD_ENDPOINT_DATA_RECV 0x01
#define USBD_ENDPOINT_DATA_SEND 0x82
#define USBD_ENDPOINT_DBG_RECV 0x03
#define USBD_ENDPOINT_DBG_SEND 0x84

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
        .idProduct = 0x200,      // Product ID
        .bcdDevice = 0x0001,     // Device release number
        .iManufacturer = 0x00,   // Manufacture string index, FIXME
        .iProduct = 0x00,        // Product string index, FIXME
        .iSerialNumber = 0x0,    // Serial number string index
        .bNumConfigurations = 1, // Number of configurations, FIXME
};

static kdrv_usbd3_config_descriptor_t hs_confg_desc =
    {
        .bLength = 9,               // 9 bytes
        .bDescriptorType = 0x02,    // Type: Configuration Descriptor
        .wTotalLength = 46,         // total bytes including config/interface/endpoint descriptors
        .bNumInterfaces = 1,        // Number of interfaces
        .bConfigurationValue = 0x1, // Configuration number
        .iConfiguration = 0x0,      // No String Descriptor
        .bmAttributes = 0xC0,       // Self-powered, no Remote wakeup
        .MaxPower = 0xFA,           // 500 mA
};

static kdrv_usbd3_interface_descriptor_t hs_intf_desc =
    {
        .bLength = 9,              // 9 bytes
        .bDescriptorType = 0x04,   // Inteface Descriptor
        .bInterfaceNumber = 0x0,   // Interface Number
        .bAlternateSetting = 0x0,  //
        .bNumEndpoints = 4,        // number of endpoint
        .bInterfaceClass = 0xFF,   // Vendor specific
        .bInterfaceSubClass = 0x0, //
        .bInterfaceProtocol = 0x0, //
        .iInterface = 0x0,         // Interface descriptor string index
};

// endpoint 0x01, bulk-out
static kdrv_usbd3_endpoint_descriptor_t hs_desc_data_recv =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DATA_RECV,
        .bmAttributes = 0x02,  // TransferType = Bulk
        .wMaxPacketSize = 512, // max 512 bytes
        .bInterval = 0x00,     // never NAKs
};

// endpoint 0x82, bulk-in
static kdrv_usbd3_endpoint_descriptor_t hs_desc_data_send =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DATA_SEND,
        .bmAttributes = 0x02,  // TransferType = Bulk
        .wMaxPacketSize = 512, // max 512 bytes
        .bInterval = 0x00,     // never NAKs
};

// endpoint 0x03, bulk-out
static kdrv_usbd3_endpoint_descriptor_t hs_desc_dbg_recv =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DBG_RECV,
        .bmAttributes = 0x02,  // TransferType = Bulk
        .wMaxPacketSize = 512, // max 512 bytes
        .bInterval = 0x00,     // never NAKs
};

// endpoint 0x84, bulk-in
static kdrv_usbd3_endpoint_descriptor_t hs_desc_dbg_send =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DBG_SEND,
        .bmAttributes = 0x02,  // TransferType = Bulk
        .wMaxPacketSize = 512, // max 512 bytes
        .bInterval = 0x00,     // never NAKs
};

static kdrv_usbd3_HS_descriptors_t hs_descps =
    {
        .dev_descp = &hs_dev_descp,
        .config_descp = &hs_confg_desc,
        .intf_descp = &hs_intf_desc,
        .enp_descp[0] = &hs_desc_data_recv,
        .enp_descp[1] = &hs_desc_data_send,
        .enp_descp[2] = &hs_desc_dbg_recv,
        .enp_descp[3] = &hs_desc_dbg_send,
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
        .idProduct = 0x0200,     // Product ID
        .bcdDevice = 0x0001,     // Device release number
        .iManufacturer = 0x00,   // Manufacture string index, FIXME
        .iProduct = 0x00,        // Product string index, FIXME
        .iSerialNumber = 0x0,    // Serial number string index
        .bNumConfigurations = 1, // Number of configurations, FIXME
};

static kdrv_usbd3_config_descriptor_t ss_confg_desc =
    {
        .bLength = 9,               // 9 bytes
        .bDescriptorType = 0x02,    // Type: Configuration Descriptor
        .wTotalLength = 70,         // total bytes including config/interface/endpoint/companion descriptors
        .bNumInterfaces = 1,        // Number of interfaces
        .bConfigurationValue = 0x1, // Configuration number
        .iConfiguration = 0x0,      // No String Descriptor
        .bmAttributes = 0xC0,       // Self-powered, no Remote wakeup
        .MaxPower = 0x0,            // 0
};

static kdrv_usbd3_interface_descriptor_t ss_intf_desc =
    {
        .bLength = 9,              // 9 bytes
        .bDescriptorType = 0x04,   // Inteface Descriptor
        .bInterfaceNumber = 0x0,   // Interface Number
        .bAlternateSetting = 0x0,  //
        .bNumEndpoints = 4,        // number of endpoints
        .bInterfaceClass = 0xFF,   // Vendor specific
        .bInterfaceSubClass = 0x0, //
        .bInterfaceProtocol = 0x0, //
        .iInterface = 0x0,         // Interface descriptor string index
};

// endpoint 0x01, bulk-out
static kdrv_usbd3_endpoint_descriptor_t ss_desc_data_recv =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DATA_RECV,
        .bmAttributes = 0x02,   // TransferType = Bulk
        .wMaxPacketSize = 1024, // max 1024 bytes
        .bInterval = 0x00,      // never NAKs
};

// endpoint 0x82, bulk-in
static kdrv_usbd3_endpoint_descriptor_t ss_desc_data_send =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DATA_SEND,
        .bmAttributes = 0x02,   // TransferType = Bulk
        .wMaxPacketSize = 1024, // max 1024 bytes
        .bInterval = 0x00,      // never NAKs
};

// endpoint 0x03, bulk-out
static kdrv_usbd3_endpoint_descriptor_t ss_desc_dbg_recv =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DBG_RECV,
        .bmAttributes = 0x02,   // TransferType = Bulk
        .wMaxPacketSize = 1024, // max 1024 bytes
        .bInterval = 0x00,      // never NAKs
};

// endpoint 0x84, bulk-in
static kdrv_usbd3_endpoint_descriptor_t ss_desc_dbg_send =
    {
        .bLength = 7,            // 7 bytes
        .bDescriptorType = 0x05, // Endpoint Descriptor
        .bEndpointAddress = USBD_ENDPOINT_DBG_SEND,
        .bmAttributes = 0x02,   // TransferType = Bulk
        .wMaxPacketSize = 1024, // max 1024 bytes
        .bInterval = 0x00,      // never NAKs
};

// endpoint 0x04 companion
static kdrv_usbd3_endpoint_companion_descriptor_t ss_desc_bulk_comp =
    {
        .bLength = 6,            // 6 bytes
        .bDescriptorType = 0x30, // Endpoint Companion Descriptor
        .bMaxBurst = 0,          // burst size = 1
        .bmAttributes = 0x0,     // no stream
        .wBytesPerInterval = 0,  // 0 for bulk
};

static kdrv_usbd3_SS_descriptors_t ss_descps =
    {
        .dev_descp = &ss_dev_descp,
        .config_descp = &ss_confg_desc,
        .intf_descp = &ss_intf_desc,
        .enp_descp[0] = &ss_desc_data_recv,
        .enp_cmpn_descp[0] = &ss_desc_bulk_comp,
        .enp_descp[1] = &ss_desc_data_send,
        .enp_cmpn_descp[1] = &ss_desc_bulk_comp,
        .enp_descp[2] = &ss_desc_dbg_recv,
        .enp_cmpn_descp[2] = &ss_desc_bulk_comp,
        .enp_descp[3] = &ss_desc_dbg_send,
        .enp_cmpn_descp[3] = &ss_desc_bulk_comp,
};

#define USB_MANUFACTURER_STRING {'K',0,'n',0,'e',0,'r',0,'o',0,'n',0}
#define USB_PRODUCT_STRING {'K',0,'n',0,'e',0,'r',0,'o',0,'n',0,' ',0,'K',0,'L',0,'7',0,'2',0,'0',0}
#define USB_SERIAL_STRING {'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0}

kdrv_usbd3_prd_string_descriptor_t str_desc_mun =
{
    .bLength = (2 + 12),
    .bDescriptorType = 0x03,
    .bString = USB_MANUFACTURER_STRING,
};

kdrv_usbd3_prd_string_descriptor_t str_desc_prd =
{
    .bLength = (2 + 24),
    .bDescriptorType = 0x03,
    .bString = USB_PRODUCT_STRING,
};

kdrv_usbd3_prd_string_descriptor_t str_desc_serial =
{
    .bLength = (2 + 16),
    .bDescriptorType = 0x03,
    .bString = USB_SERIAL_STRING,
};

kdrv_usbd3_string_descriptor_t str_desc =
{
    .bLength = 4,
    .bDescriptorType = 0x03,
    .bLanguageID = 0x0409,
    .desc[0] = &str_desc_mun,
    .desc[1] = &str_desc_prd,
    .desc[2] = &str_desc_serial,
};

#define STM_CX_START 0x10
#define TFLAG_STM_START 0x100

static dual_fifo_t gImg_dfifo = 0;
static dual_fifo_t gResult_dfifo = 0;

static void data_recv_thread(void *arg)
{
    // the begin of STM, FIXME
    osThreadFlagsWait(TFLAG_STM_START, osFlagsWaitAny, osWaitForever);

    kmdw_printf("[data_rx] ready !\n");

    osStatus_t fifo_sts;
    kdrv_status_t usb_sts;
    uint32_t buf_addr;
    uint32_t buf_size = dual_fifo__buffer_size(gImg_dfifo);

    while (1)
    {
        // get a free buffer with forcely grabbing mode
        fifo_sts = dual_fifo__get_free_buffer(gImg_dfifo, &buf_addr, 0, true);
        if (fifo_sts != osOK)
            break;

        uint32_t recv_len = buf_size;
        usb_sts = kdrv_usbd3_bulk_receive(USBD_ENDPOINT_DATA_RECV, (void *)buf_addr, &recv_len, 0);
        if (usb_sts != KDRV_STATUS_OK)
            break;

        // check the file size match or not, if not receive more
        uint32_t actual_size = *(uint32_t *)buf_addr;
        if (actual_size > recv_len)
        {
            // receive the left part
            uint32_t recv_len2 = buf_size - recv_len;
            usb_sts = kdrv_usbd3_bulk_receive(USBD_ENDPOINT_DATA_RECV, (void *)(buf_addr + recv_len), &recv_len2, 0);
            if (usb_sts != KDRV_STATUS_OK)
                break;
        }

        fifo_sts = dual_fifo__enqueue_data(gImg_dfifo, &buf_addr, osWaitForever);
        if (fifo_sts != osOK)
            break;
    }

    kmdw_printf("[data_rx] stopped !\n");
}

static void result_send_thread(void *arg)
{
    // the begin of STM, FIXME
    osThreadFlagsWait(TFLAG_STM_START, osFlagsWaitAny, osWaitForever);

    kmdw_printf("[result_tx] ready !\n");

    osStatus_t fifo_sts;
    kdrv_status_t usb_sts;
    uint32_t buf_addr;

    while (1)
    {
        // get result data from queue blocking wait
        fifo_sts = dual_fifo__dequeue_data(gResult_dfifo, &buf_addr, osWaitForever);
        if (fifo_sts != osOK)
            break;

        uint32_t txlen = *(uint32_t *)buf_addr;

        // send result to the host, blocking wait
        usb_sts = kdrv_usbd3_bulk_send(USBD_ENDPOINT_DATA_SEND, (void *)buf_addr, txlen, 0);
        if (usb_sts != KDRV_STATUS_OK)
            break;

        // return free buf back to queue
        fifo_sts = dual_fifo__put_free_buffer(gResult_dfifo, &buf_addr, osWaitForever);
        if (fifo_sts != osOK)
            break;
    }

    kmdw_printf("[data_tx] stopped !\n");
}

////////////////////////////////////////////////////////////

#define USBD_BACKDOOR // FIXME: backdoor thread, use differnt pair of usb endpoints

#ifdef USBD_BACKDOOR
extern osThreadId_t usbd_com_backdoor_init(void);
static osThreadId_t usbd_dbg_backdoor_tid = 0;
#endif

static osThreadId_t usbd_queue_rx_tid = 0;
static osThreadId_t usbd_queue_tx_tid = 0;

static void usbd_link_status_notify(kdrv_usbd3_link_status_t link_status)
{
    switch (link_status)
    {
    case USBD3_STATUS_DISCONNECTED:
        kmdw_printf("[usbd] link_status : disconnected\n");
        break;

    case USBD3_STATUS_CONFIGURED:
        kmdw_printf("[usbd] link_status : configured\n");

        if (kdrv_usbd3_get_link_speed() == USBD3_HIGH_SPEED)
            kmdw_printf("[usbd] running at High-Speed\n");
        else if (kdrv_usbd3_get_link_speed() == USBD3_SUPER_SPEED)
            kmdw_printf("[usbd] running at Super-Speed\n");
        else
        {
            kmdw_printf("[usbd] running at NO-Speed\n");
            return;
        }

        osThreadFlagsSet(usbd_queue_rx_tid, TFLAG_STM_START);
        osThreadFlagsSet(usbd_queue_tx_tid, TFLAG_STM_START);
#ifdef USBD_BACKDOOR
        osThreadFlagsSet(usbd_dbg_backdoor_tid, TFLAG_STM_START);
#endif

        break;
    }
}

static bool usbd_usr_cx_callback(kdrv_usbd3_setup_packet_t *setup)
{
    if (setup->bRequest == STM_CX_START)
    {
        kmdw_printf("[usbd] Host set STM_START\n");
    }

    return true;
}

void usbd_com_init(dual_fifo_t image_dfifo, dual_fifo_t result_dfifo)
{
    kdrv_usbd3_initialize(&hs_descps, &ss_descps, &str_desc, usbd_link_status_notify, usbd_usr_cx_callback);
    kdrv_usbd3_set_enable(true);

    usbd_queue_rx_tid = osThreadNew(data_recv_thread, NULL, NULL);
    if( !usbd_queue_rx_tid )
        err_msg("[******** ERROR ********] data_recv_thread not launched\n");
    
    usbd_queue_tx_tid = osThreadNew(result_send_thread, NULL, NULL);
    if( !usbd_queue_tx_tid )
        err_msg("[******** ERROR ********] result_send_thread not launched\n");
#ifdef USBD_BACKDOOR
    usbd_dbg_backdoor_tid = usbd_com_backdoor_init();
#endif
    gImg_dfifo = image_dfifo;
    gResult_dfifo = result_dfifo;
}
