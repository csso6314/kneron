/**
 * @file      USBDev.h
 * @brief     Header file for USB Dev class
 * @version   0.1 - 2019-04-28
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __USB_DEV_H__
#define __USB_DEV_H__

#include <libusb-1.0/libusb.h>

#include "BaseDev.h"

#include "khost_ll.h"

#define VENDOR_ID   0x3231
#define PRODUCT_ID  0x0100

#define USBD_ENDPOINT_CMD_OUT 0x02
#define USBD_ENDPOINT_CMD_IN 0x81

#define ACK_PACKET_SIZE 16
#define USB_MAX_BYTES   (4 << 20)

#define USB_FAKE_FD_START 0x1000

using namespace std;

class USBDev : public BaseDev {
public:
    USBDev(khost_device_t khDev);
    virtual ~USBDev();

    int recv_data_ack();
    int send_data_ack(uint32_t action, uint32_t src_id, uint32_t len);
    int send_image_data(char *buf, int data_len);
    int send(char* msgbuf, int msglen);
    int receive(char* msgbuf, int msglen);

    int receive_bulk(char* msgbuf, int msglen);
    int send_bulk(char* msgbuf, int msglen, bool send_zero_byte);
    int usb_recv(int8_t* msgbuf, int msglen);
    int reconnect_usb();
    virtual void shutdown_dev();

    khost_device_t to_khost_device();
    
    uint8_t usb_port;
    uint8_t usb_bus;
private:

    int check_to_send_zero_byte(size_t len, unsigned char endpoint);
    int check_to_receive_zero_byte(size_t len, unsigned char endpoint);

    libusb_device_handle*    m_pkneron_device_h;

    int m_usb_speed;
    int m_max_bulk_size;

    int m_vendor_id;
    int m_product_id;

    khost_device_t m_khdev; // from khost_ll.h

    char m_device_path[20]; // bus-port path

    static int _inst_cnt;
};

#endif
