/**
 * @file      BaseDev.h
 * @brief     Header file for BaseDev class
 * @version   0.1 - 2019-04-12
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __BASE_DEV_H__
#define __BASE_DEV_H__

#include <string>

#include <stdint.h>

#define MAX_UART_READ_RETRY 1000

using namespace std;

class BaseDev {
public:
    string dev_name;
    int dev_fd;
    int send_sync(char* buffer, int size);
    int send_async(char* buffer, int size);

    int get_dev_fd() { return dev_fd; }

    virtual int set_baudrate(int speed);
    virtual int set_parity(); 

    virtual ~BaseDev();

    virtual int send(char* msgbuf, int msglen);
    virtual int receive(char* msgbuf, int msglen);
    int dev_type;

    virtual void shutdown_dev();

protected:
    bool dev_shutdown;
};

#endif
