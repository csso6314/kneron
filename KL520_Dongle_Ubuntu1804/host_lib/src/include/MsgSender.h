/**
 * @file      MsgSender.h
 * @brief     msg sender class header file
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __MSG_SENDER_H__
#define __MSG_SENDER_H__

#define SND_Q_IDLE_TIME 100

#include "MsgXBase.h"

class MsgSender : public MsgXBase {
public:
    MsgSender(MsgMgmt* p_ref);

    void run(); //loop for sending msgs

    virtual void stop();

    int send_cmd_msg(KnBaseMsg* p_msg, BaseDev* pDev);
    int send_cmd_msg_sync(KnBaseMsg* p_msg, BaseDev* pDev);
    virtual ~MsgSender();

private:
    int send_msg_to_dev(KnBaseMsg* p_msg, BaseDev* pDev);

};

#endif
