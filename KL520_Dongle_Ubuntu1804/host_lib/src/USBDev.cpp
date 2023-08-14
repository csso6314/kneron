/**
 * @file      USBDev.cpp
 * @brief     implementation file for USB Dev class
 * @version   0.1 - 2019-04-28
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "USBDev.h"
#include "KnLog.h"
#include "KdpHostApi.h"
#include "UARTPkt.h"
#include "KnUtil.h"
#include "KnMsg.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>

#define MAX_RETRY_NUM 2
#define LIB_USB_TIMEOUT 10200

#define KL520_PRODUCT_ID 0x0100
#define KL720_PRODUCT_ID 0x0200

int USBDev::_inst_cnt = 0;

typedef struct
{
	libusb_device_handle *usb_handle;
} _kn_usb_device_t;

USBDev::USBDev(khost_device_t khDev)
{
    m_khdev = khDev;

    kneron_device_info_t *dev_info = khost_ll_get_device_info(khDev);

    m_pkneron_device_h = ((_kn_usb_device_t *)m_khdev)->usb_handle; // FIXME
    m_product_id = dev_info->product_id;
    m_usb_speed = dev_info->link_speed;
    m_max_bulk_size = (m_usb_speed <= LIBUSB_SPEED_HIGH) ? 512 : 1024;

    strcpy(m_device_path, dev_info->device_path);

    dev_type = KDP_USB_DEV;

    _inst_cnt++;
    dev_fd = USB_FAKE_FD_START + _inst_cnt;

}

USBDev::~USBDev()
{
    if (m_khdev)
    {
        khost_ll_disconnect_device(m_khdev);
        m_khdev = NULL;
        m_pkneron_device_h = NULL;
    }

    _inst_cnt--;
}

int USBDev::send(char *msgbuf, int msglen)
{
    int status = 0;

    if (NULL == m_pkneron_device_h)
    {
        KnTsPrint(KN_LOG_DBG, "send failed, m_pkneron_device_h is NULL.\n");
        return -1;
    }

    //need to pack the data
    KnTsPrint(KN_LOG_DBG, "packing payload with uart pkt:%d.\n", msglen);
    UARTPkt pkt((int8_t *)msgbuf, msglen, true, MSG_CRC_FLAG);

    //calling base senf func to send the packed data
    if (!pkt.is_pkt_valid())
    {
        KnPrint(KN_LOG_ERR, "packing to UART packet falied.\n");
        return -1;
    }

    kn_dump_msg(pkt.get_pkt_buf(), pkt.get_pkt_len());

    KnTsPrint(KN_LOG_DBG, "kdp msg is packed with uart pkt of size [%d].\n", pkt.get_pkt_len());

    int transfered = 0;
    KnTsPrint(KN_LOG_DBG, "calling usb interrupt send to send data of size [%d]...\n", msglen);
    status = libusb_bulk_transfer(m_pkneron_device_h, USBD_ENDPOINT_CMD_OUT,
                                  (unsigned char *)(pkt.get_pkt_buf()), pkt.get_pkt_len(), &transfered, LIB_USB_TIMEOUT);
    if (status < 0)
    {
        KnPrint(KN_LOG_ERR, "write usb failed: %d, %s.\n", status, libusb_error_name(status));
        return status;
    }

    check_to_send_zero_byte(transfered, USBD_ENDPOINT_CMD_OUT);

    KnTsPrint(KN_LOG_DBG, "usb write sent data :%d.\n", transfered);
    return transfered;
}

int USBDev::send_bulk(char *msgbuf, int msglen, bool send_zero_byte)
{
    int status = 0;

    if (NULL == m_pkneron_device_h)
        return -1;

    int transfered = 0;
    KnTsPrint(KN_LOG_DBG, "calling usb bulk send to send data :%d...\n", msglen);
    status = libusb_bulk_transfer(m_pkneron_device_h, USBD_ENDPOINT_CMD_OUT,
                                  (unsigned char *)msgbuf, msglen, &transfered, LIB_USB_TIMEOUT);
    if (status < 0)
    {
        KnPrint(KN_LOG_ERR, "write usb failed: %d, %s.\n", status, libusb_error_name(status));
        return status;
    }

    if (send_zero_byte)
    {
        check_to_send_zero_byte(transfered, USBD_ENDPOINT_CMD_OUT);
    }

    KnTsPrint(KN_LOG_DBG, "usb write sent data :%d.\n", transfered);
    return transfered;
}

/* due to USB protocol, when data len is multiple of BULK_SIZE_USB,
 * we should send a zero-len data to notify no more data */
int USBDev::check_to_send_zero_byte(size_t len, unsigned char endpoint)
{
    if (len == 0 || (len % m_max_bulk_size) > 0)
    {
        return 0;
    }

    unsigned char data[1];
    int tmp = 0;

    KnTsPrint(KN_LOG_DBG, "sending zero byte...\n");

    if (m_product_id == KL720_PRODUCT_ID)
    {
        // a workaround for KL720
        // dont send a real ZLP but instead a fake one
        unsigned int fake_ZLP = 0x11223344;
        return libusb_bulk_transfer(m_pkneron_device_h, endpoint, (unsigned char *)&fake_ZLP, 4, &tmp, LIB_USB_TIMEOUT);
    }
    else
        return libusb_bulk_transfer(m_pkneron_device_h, endpoint, data, 0, &tmp, LIB_USB_TIMEOUT);
}

int USBDev::check_to_receive_zero_byte(size_t len, unsigned char endpoint)
{
    if (len == 0 || (len % m_max_bulk_size) > 0)
    {
        return 0;
    }

    unsigned char data;
    int tmp = 0;

    KnTsPrint(KN_LOG_DBG, "receiving zero byte...\n");

    int ret = libusb_bulk_transfer(m_pkneron_device_h, endpoint, &data, sizeof(unsigned char), &tmp, LIB_USB_TIMEOUT);

    if (ret == 0 && tmp != 0)
        printf("%s() failed, should be ZLP but it is not !!\n", __FUNCTION__);

    return ret;
}

void USBDev::shutdown_dev()
{
    KnTsPrint(KN_LOG_DBG, "shutdown usb device...\n");

    if (NULL != m_pkneron_device_h)
    {
        libusb_release_interface(m_pkneron_device_h, 0);
        libusb_close(m_pkneron_device_h);
        m_pkneron_device_h = NULL;
    }

}

int USBDev::reconnect_usb()
{
    if (m_khdev)
    {
        khost_ll_disconnect_device(m_khdev);
        m_khdev = NULL;
        m_pkneron_device_h = NULL;
    }

    kdp_device_info_list_t *list;

    kdp_scan_usb_devices(&list);

    // find out the same devie by the port path
    for (int j = 0; j < list->num_dev; j++)
    {
        if (strcmp(m_device_path, list->kdevice[j].device_path) == 0)
        {
            m_khdev = khost_ll_connect_device(list->kdevice[j].scan_index);
            if (m_khdev)
                break; // re-connect successufl
        }
    }
    free(list);

    if (m_khdev == NULL)
        return -1;

    m_pkneron_device_h = ((_kn_usb_device_t *)m_khdev)->usb_handle; // FIXME
    return 0;
}

int USBDev::receive(char *msgbuf, int msglen)
{
    int ret_val;
    if (NULL == m_pkneron_device_h)
        return -1;

    int transfered = 0;
    KnTsPrint(KN_LOG_DBG, "calling usb interrupt read to receive data :%d...\n", msglen);

    for (int i = 0; i < MAX_RETRY_NUM; i++)
    {

        ret_val = libusb_bulk_transfer(m_pkneron_device_h, USBD_ENDPOINT_CMD_IN,
                                       (unsigned char *)msgbuf, msglen, &transfered, LIB_USB_TIMEOUT);
        if (ret_val != 0)
        {
            KnTsPrint(KN_LOG_ERR, "USB EP read failed:%d, %s.\n", ret_val, libusb_error_name(ret_val));
            if (ret_val != LIBUSB_ERROR_NO_DEVICE)
            {
                return -1;
            }
            else
            { //no such device
                int s = reconnect_usb();
                if (s < 0)
                    return -1;
                continue; //retry
            }
        }
        else
        {
            check_to_receive_zero_byte(transfered, USBD_ENDPOINT_CMD_IN);
            break;
        }
    }

    KnTsPrint(KN_LOG_DBG, "usb read received data :%d.\n", transfered);
    return transfered;
}

int USBDev::receive_bulk(char *msgbuf, int msglen)
{
    if (NULL == m_khdev)
        return -1;

    int transfered = 0;
    KnTsPrint(KN_LOG_DBG, "calling usb bulk read to receive data :%d...\n", msglen);

    for (int i = 0; i < MAX_RETRY_NUM; i++)
    {
        transfered = khost_ll_read_command(m_khdev, msgbuf, msglen, LIB_USB_TIMEOUT);
        if (transfered > 0)
            break;
        else // error
        {
            KnTsPrint(KN_LOG_ERR, "USB EP read failed:%d, %s.\n", transfered, libusb_error_name(transfered));

            if (transfered == LIBUSB_ERROR_NO_DEVICE)
            {
                int s = reconnect_usb();
                if (s < 0)
                    return -1;
                else
                    continue; //retry
            }
            else
                return -1;
        }
    }

    KnTsPrint(KN_LOG_DBG, "usb read received data :%d.\n", transfered);

    return transfered;
}

int USBDev::usb_recv(int8_t *msgbuf, int msglen)
{
    if (NULL == m_pkneron_device_h)
        return -1;

    int transfered = 0;
    int ret_val = libusb_bulk_transfer(m_pkneron_device_h, USBD_ENDPOINT_CMD_IN,
                                       (unsigned char *)msgbuf, msglen, &transfered, LIB_USB_TIMEOUT);
    if (ret_val != 0)
    {
        if (ret_val == LIBUSB_ERROR_NO_DEVICE)
        {
            KnTsPrint(KN_LOG_ERR, "USB EP read failed:%d, %s.\n", ret_val, libusb_error_name(ret_val));
            reconnect_usb(); //reconnect to usb in case of no device
        }
        return ret_val;
    }

    check_to_receive_zero_byte(transfered, USBD_ENDPOINT_CMD_IN);

    kn_dump_msg(msgbuf, transfered, false);

    //unpack the data
    KnPrint(KN_LOG_DBG, "unpacking msg payload...\n");
    UARTPkt pkt(msgbuf, transfered);
    if (!pkt.is_pkt_valid())
    {
        KnPrint(KN_LOG_ERR, "unpacking UART packet falied.\n");
        return -1;
    }

    int bytes_received = pkt.get_payload_len();
    assert(bytes_received <= msglen);
    memset(msgbuf, 0, msglen);
    memcpy(msgbuf, pkt.get_payload_buf(), bytes_received);

    KnPrint(KN_LOG_DBG, "get msg payload from uart pkt:%d.\n", bytes_received);

    //payload
    if (bytes_received < (int)sizeof(KnMsgHdr))
    { //less than header
        KnPrint(KN_LOG_ERR, "receive msg len less than hdr.\n");
    }
    else
    {
        KnMsgHdr hdr;
        KnBaseMsg::decode_msg_hdr(&hdr, msgbuf, bytes_received);
        int mlen = hdr.msg_len;
        if (bytes_received < mlen + (int)sizeof(KnMsgHdr))
        { //only a part msg
            KnPrint(KN_LOG_ERR, "only a part of msg received:%d,%d.\n", bytes_received, mlen);
        }
        //whole msg is ready
    }

    KnPrint(KN_LOG_DBG, "from com dev, got %d bytes msg.\n", bytes_received);

    //kn_dump_msg(msgbuf, bytes_received, false);
    return bytes_received;
}

int USBDev::recv_data_ack()
{
    int8_t ack_buffer[ACK_PACKET_SIZE];
    memset(ack_buffer, 0, sizeof(ack_buffer));
    //uint8_t usb_ack[8] = { 0x35, 0x8A, 0x04, 0x00, 0x04, 0x00, 0x08, 0x00 };

    KnTsPrint(KN_LOG_DBG, "usb dev is receiving ack...\n");
    int status = receive_bulk((char *)ack_buffer, ACK_PACKET_SIZE);
    if (status < 4)
    {
        KnPrint(KN_LOG_ERR, "receiving ack failed:%d.\n", status);
        return -1;
    }

    //get the ack msg
    KnImgTransAck ack_msg(ack_buffer + 4, status - 4);
    if (ack_msg.is_msg_valid() == false)
    {
        KnPrint(KN_LOG_ERR, "ack msg parsing failed:%d.\n", status);
        return -1;
    }

    if (ack_msg._action != 0)
    { //not ack
        KnPrint(KN_LOG_ERR, "got nack msg from device %d.\n", ack_msg._action);
        return -1;
    }

    return 0;
}

int USBDev::send_data_ack(uint32_t action, uint32_t src_id, uint32_t len)
{
    uint8_t ack_buffer[ACK_PACKET_SIZE] = {
        0x83,
        0xA5,
        0x04,
        0x00,
    }; //pack header

    KnImgTransAck ack_msg(CMD_ACK_NACK, 8, action, src_id);
    if (ack_msg.is_msg_valid() == false)
    {
        KnPrint(KN_LOG_ERR, "creating ack msg failed.\n");
        return -1;
    }

    //packing msg
    memcpy(ack_buffer + 4, ack_msg.get_msg_payload(), ack_msg.get_msg_len());
    memcpy(ack_buffer + 12, &len, 4);

    //send it
    int status = send_bulk((char *)ack_buffer, ACK_PACKET_SIZE, true);
    if (status != ACK_PACKET_SIZE)
    {
        KnPrint(KN_LOG_ERR, "sending ack msg failed:%d.\n", status);
        return status;
    }

    KnTsPrint(KN_LOG_DBG, "sent ack msg to dev.\n");
    return 0;
}

int USBDev::send_image_data(char *buf, int data_len)
{
    if (data_len <= 0 || !buf)
        return -1;

    KnTsPrint(KN_LOG_DBG, "usb dev is starting to send image :%d...\n", data_len);

    int status = recv_data_ack();
    if (status != 0)
    {
        KnPrint(KN_LOG_ERR, "ack msg wrong:%d.\n", status);
        return -1;
    }

    int leftover = data_len;
    int i = 0;
    int off = 0;

    while (leftover > 0)
    {
        int len = static_cast<int>(USB_MAX_BYTES);
        if (len > leftover)
            len = leftover;

        KnTsPrint(KN_LOG_DBG, "usb dev is sending segment :%d, with len:%d...\n", i, len);
        status = send_bulk(buf + off, len, false);
        if (status < 0)
        {
            KnPrint(KN_LOG_ERR, "sending %d time for img, off:%d.\n", i, off);
            return status;
        }

        KnTsPrint(KN_LOG_DBG, "usb dev sent segment :%d, with len:%d...\n", i, status);

        off += status;
        leftover -= status;
        i++;
    }

    check_to_send_zero_byte(data_len, USBD_ENDPOINT_CMD_OUT);

    KnTsPrint(KN_LOG_DBG, "usb dev finished sending image :%d...\n", off);

    return off;
}

khost_device_t USBDev::to_khost_device()
{
    return m_khdev;
}
