/**
 * @file      MsgReceiver.h
 * @brief     msg receiver class header file
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __MSG_RECEIVER_H__
#define __MSG_RECEIVER_H__

#define MAX_POLL_TIMEVAL 50  //milli sec
#define RCV_Q_IDLE_TIME  100

#include "MsgXBase.h"

#include <stdint.h>

class MsgReceiver : public MsgXBase {
public:
    MsgReceiver(MsgMgmt* p_ref);

    void run(); //loop for receiving msgs

    virtual void run_usb(int idx); //loop for usb messages

    KnBaseMsg* rcv_cmd_rsp(int& idx);
    virtual ~MsgReceiver();

private:
    int add_msg_to_rq(int idx, int8_t* msgbuf, int msglen);

};

#endif
