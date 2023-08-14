/**
 * @file      UARTDev.h
 * @brief     Header file for UART Dev class
 * @version   0.1 - 2019-04-12
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __UART_DEV_H__
#define __UART_DEV_H__

#include "BaseDev.h"

using namespace std;

class UARTDev : public BaseDev {
public:
    UARTDev(string dname, bool is_block = false);
    virtual ~UARTDev();

    virtual int set_baudrate(int speed);
    virtual int set_parity();

    virtual int send(int8_t* msgbuf, int msglen);
    virtual int receive(int8_t* msgbuf, int msglen);
};

#endif
