/**
 * @file      BasTDev.cpp
 * @brief     implementation file for base Dev class
 * @version   0.1 - 2019-04-12
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "BaseDev.h"
#include "KnLog.h"
#include "KdpHostApi.h"
#include "KnMsg.h"
#include "KnUtil.h"
#include "UARTPkt.h"

#include <unistd.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <arpa/inet.h>
#endif

using namespace std;

int BaseDev::send_sync(char* buffer, int size) {
    return -1;
}

int BaseDev::send_async(char* buffer, int size) {
    return -1;
}

int BaseDev::set_baudrate(int speed) {
    return -1;
}

int BaseDev::set_parity() {
    return -1;
}

BaseDev::~BaseDev() {
}

void BaseDev::shutdown_dev() {
    dev_shutdown = true;
}

int BaseDev::send(char* msgbuf, int msglen) {
    int fd = dev_fd;
    if(fd < 0) {
        KnPrint(KN_LOG_ERR, "com dev is not open.\n");
        return -1;
    }

    if(msglen < 1 || msgbuf == NULL) {
        KnPrint(KN_LOG_ERR, "invalid msg buffer to send.\n");
        return -1;
    }

    kn_dump_msg((int8_t* )msgbuf, msglen, true);

    KnTsPrint(KN_LOG_DBG, "calling sys call to send data :%d..\n", msglen);
    int bytes_left = msglen;
    int8_t* ptr = (int8_t* )msgbuf;
    while (bytes_left > 0) {
        int n = write(fd, ptr, bytes_left);
        if(n <= 0) {
            if (errno == EINTR) {
                KnPrint(KN_LOG_ERR, "sending msg got an interrupt, retrying.\n");
                continue;
            } else if(errno == EAGAIN) {
                KnPrint(KN_LOG_ERR, "sending msg got an eagain, retrying.\n");
                usleep(1000);
                continue;
            } else {
                KnPrint(KN_LOG_ERR, "sending msg got unexpected exception, aborted.\n");
                return -1;
            }
        }
        bytes_left -= n;
        ptr += n;
    }

    KnTsPrint(KN_LOG_DBG, "base dev sent data completed :%d..\n", msglen);
    return msglen;
}


int BaseDev::receive(char* msgbuf, int msglen) {
    int fd = dev_fd;
    if(fd < 0) {
        KnPrint(KN_LOG_ERR, "com dev is not open.\n");
        return -1;
    }

    if(msglen < 1 || msgbuf == NULL) {
        KnPrint(KN_LOG_ERR, "invalid msg buffer to receive.\n");
        return -1;
    }

    int bytes_received = 0;
    int8_t* ptr = (int8_t* )msgbuf;
    int left = UART_PKT_HEAD_LEN;

    KnTsPrint(KN_LOG_DBG, "calling sys call to receive data..\n");
    for(int i = 0; i < MAX_UART_READ_RETRY;i++) {
        int n = read(fd, ptr, left);
        if(n < 0) {
            if (errno == EINTR) {
                KnPrint(KN_LOG_ERR, "receiving msg got an interrupt, retrying.\n");
                continue;
            } else if(errno == EAGAIN) {
    //            KnPrint(KN_LOG_ERR, "receive msg got an eagain, retrying.\n");
                usleep(1000);
                continue;
            } else {
                KnPrint(KN_LOG_ERR, "receive msg got unexpected exception, aborted.\n");
                break;
            }
        }

        KnTsPrint(KN_LOG_DBG, "Basedev read %d bytes from UART dev.\n", n);
        bytes_received += n;
        ptr += n;
        left -= n;

        if(bytes_received < UART_PKT_HEAD_LEN) continue;

        if(bytes_received == UART_PKT_HEAD_LEN) {
            uint16_t tmp = 0;
            int8_t* tmp_ptr = (int8_t* )msgbuf;
            tmp_ptr += sizeof(uint16_t);
            memcpy((void*)&tmp, tmp_ptr, sizeof(uint16_t));

            //tmp = ntohs(tmp);
            uint16_t len = tmp & 0x0fff; //get the len

            left = len;
            KnTsPrint(KN_LOG_DBG, "header received, remaining payload:%d.\n", left);
        }

        if(left == 0) break; //read all the msg pkt
    }

    kn_dump_msg((int8_t* )msgbuf, bytes_received, false);

    KnTsPrint(KN_LOG_DBG, "base dev received data completed:%d.\n", bytes_received);
    return bytes_received;
}

