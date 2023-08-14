/**
 * @file      UARTDev.cpp
 * @brief     implementation file for UART Dev class
 * @version   0.1 - 2019-04-12
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "UARTDev.h"
#include "USBDev.h"
#include "KnLog.h"
#include "KnMsg.h"
#include "KnUtil.h"
#include "UARTPkt.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#ifdef __linux__
#include <termios.h>
#elif _WIN32
#include <windows.h>
#endif
#include <unistd.h>

using namespace std;

UARTDev::UARTDev(string dname, bool is_block) {
    dev_fd = -1;
    dev_type = KDP_UART_DEV; 
    dev_shutdown = false;
    if(dname.length() == 0) {
        KnPrint(KN_LOG_ERR, "zero length UART dev name.\n");
        return;
    }
    int flag = O_RDWR | O_NOCTTY;
    if(!is_block) flag |= O_NONBLOCK;

    int fd = open (dname.c_str(), flag);
    if(fd < 0) {
        KnPrint(KN_LOG_ERR, "open UART dev name %s failed. Reason:%s.\n", dname.c_str(), strerror(errno));
        return;
    }

    KnPrint(KN_LOG_DBG, "opened uart dev:%d successfully.\n", fd);

    dev_fd = fd;
}


UARTDev::~UARTDev() {
    if(dev_fd > 0) close (dev_fd);
}

int UARTDev::set_baudrate(int speed) {
    if(dev_fd < 0) {
        KnPrint(KN_LOG_ERR, "UART dev is not open.\n");
        return -1;
    }

    struct termios opt;
    tcgetattr (dev_fd, &opt);

    int SUPPORTED_SPEED[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300};
    speed_t SPEED_ATTR[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};

    int size = sizeof(SUPPORTED_SPEED) / sizeof(int);

    for (int i = 0; i < size; i++) {
        // get user baud rate & set option
        if (speed == SUPPORTED_SPEED[i]) {
            cfsetispeed (&opt, SPEED_ATTR[i]);  // set UART input baud rate
            cfsetospeed (&opt, SPEED_ATTR[i]);  // set UART output baud rate
            int status = tcsetattr (dev_fd, TCSANOW, &opt); // set option and work now
            if (status != 0) {
                KnPrint(KN_LOG_ERR, "calling tcsetattr to set baud rate falied :%s.\n", strerror (errno));
                tcflush (dev_fd, TCIOFLUSH);                // clear I/O buffer
                return -1;
            }
            tcflush (dev_fd, TCIOFLUSH);                // clear I/O buffer
            KnPrint(KN_LOG_DBG, "set uart baudrate to:%d..\n", speed);
            return 0;
        }
    }

    KnPrint(KN_LOG_ERR, "Specified baud rate %d is not supported!\n", speed);
    return -1;
}

int UARTDev::set_parity() {
    if(dev_fd < 0) {
        KnPrint(KN_LOG_ERR, "UART dev is not open.\n");
        return -1;
    }
    struct termios opt;  
    
    // get previous setting option
    if (tcgetattr (dev_fd, &opt) != 0) {
        KnPrint(KN_LOG_ERR, "calling tcgetattr to get attr falied :%s.\n", strerror (errno));
        return -1;
    }
   
    opt.c_cflag &= ~PARENB; //no parity
    opt.c_cflag &= ~CSTOPB; //1 stop bit
    opt.c_cflag &= ~CSIZE; //
    opt.c_cflag |= CS8; //8 bits

    opt.c_cflag &= ~CRTSCTS; //no hardware flow control
    opt.c_cflag |= CREAD | CLOCAL;

    //opt.c_iflag &= ~(IXON | IXOFF | IXANY);
    opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | IXOFF | IXANY | IGNCR | INLCR);
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opt.c_oflag &= ~(OPOST | ONLCR | OCRNL | ONOCR | ONLRET);
  
    // set option
    if (tcsetattr (dev_fd, TCSANOW, &opt) != 0) {
        KnPrint(KN_LOG_ERR, "calling tcsetattr to set parity falied :%s.\n", strerror (errno));
        return -1;  
    }

    // clear 
    tcflush (dev_fd, TCIFLUSH); // clear input buff
    KnPrint(KN_LOG_DBG, "set uart dev parity succeeded.\n");
    return 0;  
}

int UARTDev::send(int8_t* msgbuf, int msglen) {
    //kn_dump_msg(msgbuf, msglen);

    if(dev_shutdown) {
        KnPrint(KN_LOG_ERR, "dev is in shutdown state.\n");
        return -1;
    }

    //need to pack the data
    KnTsPrint(KN_LOG_DBG, "packing payload with uart pkt:%d.\n", msglen);
    UARTPkt pkt(msgbuf, msglen, true, MSG_CRC_FLAG);

    //calling base senf func to send the packed data
    if(pkt.is_pkt_valid()) {
        KnTsPrint(KN_LOG_DBG, "kdp msg is packed with uart pkt of size [%d].\n", pkt.get_pkt_len());
        return BaseDev::send((char *)pkt.get_pkt_buf(), pkt.get_pkt_len());
    } else {
        KnPrint(KN_LOG_ERR, "packing to UART packet falied.\n");
        return -1;
    }
}

int UARTDev::receive(int8_t* msgbuf, int msglen) {
    if(dev_shutdown) {
        KnPrint(KN_LOG_ERR, "dev is in shutdown state.\n");
        return -1;
    }

    //raw data receive
    KnTsPrint(KN_LOG_DBG, "calling base dev to receive raw data...\n");
    int raw_len = BaseDev::receive((char *)msgbuf, msglen);
    KnTsPrint(KN_LOG_DBG, "base dev received raw data:%d.\n", raw_len);

    //unpack the data
    KnPrint(KN_LOG_DBG, "unpacking msg payload...\n");
    UARTPkt pkt(msgbuf, raw_len);
    if(!pkt.is_pkt_valid()) {
        KnPrint(KN_LOG_ERR, "unpacking UART packet falied.\n");
        return -1;
    }

    int bytes_received = pkt.get_payload_len();
    memset(msgbuf, 0, msglen);
    memcpy(msgbuf, pkt.get_payload_buf(), bytes_received);

    KnPrint(KN_LOG_DBG, "get msg payload from uart pkt:%d.\n", bytes_received);

    //kn_dump_msg(msgbuf, bytes_received);

    //payload
    if(bytes_received < (int)sizeof(KnMsgHdr)) {  //less than header
        KnPrint(KN_LOG_ERR, "receive msg len less than hdr.\n");
    } else {
        KnMsgHdr hdr;
        KnBaseMsg::decode_msg_hdr(&hdr, msgbuf, bytes_received);
        int mlen = hdr.msg_len;
        if(bytes_received < mlen + (int)sizeof(KnMsgHdr)) { //only a part msg
            KnPrint(KN_LOG_ERR, "only a part of msg received:%d,%d.\n", bytes_received, mlen);
        }
        //whole msg is ready
    }

    KnPrint(KN_LOG_DBG, "from com dev, got %d bytes msg.\n", bytes_received);

    return bytes_received;
}

