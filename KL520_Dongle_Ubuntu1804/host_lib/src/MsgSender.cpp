/**
 * @file      MsgSender.cpp
 * @brief     implementation file for msg sender class
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "MsgSender.h"
#include "MsgMgmt.h"
#include "KnLog.h"

#include <unistd.h>
#include <string.h>

MsgSender::MsgSender(MsgMgmt* p_ref) : MsgXBase(p_ref) {
}

MsgSender::~MsgSender() {
}

void MsgSender::stop() {
    stopped = true;
    
    //notify
    _q_mtx.lock();
    _q_cond_var.notify_all();
    _q_mtx.unlock();
    return;
}

int MsgSender::send_msg_to_dev(KnBaseMsg* p_msg, BaseDev* pDev) {
    int8_t buf[MAX_MSG_LEN];
    int mlen;

    mlen = p_msg->get_msg_len() + sizeof(KnMsgHdr);
    memcpy(buf, p_msg->get_msg_payload(), mlen);

    delete p_msg; //free the request msg

    KnPrint(KN_LOG_DBG, "sending msg to dev len:%d.\n", mlen);
    return pDev->send((char *)buf, mlen);
}

int MsgSender::send_cmd_msg_sync(KnBaseMsg* p_msg, BaseDev* pDev) {
    if(!p_msg || !pDev) {
        KnPrint(KN_LOG_ERR, "invalid msg pointer in add msg to queue.\n");
        if(p_msg) delete p_msg;
        return -1;
    }

    return send_msg_to_dev(p_msg, pDev);
}

int MsgSender::send_cmd_msg(KnBaseMsg* p_msg, BaseDev* pDev) {
    if(!p_msg || !pDev) {
        KnPrint(KN_LOG_ERR, "invalid msg pointer in add msg to queue.\n");
        return -1;
    }
    int fd = pDev->get_dev_fd();

    int idx = get_index_from_fd(fd);
    if (idx < 0) {
        KnPrint(KN_LOG_ERR, "invalid fd in add msg to queue:%d.\n", fd);
        delete p_msg;
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "add async msg to sender queue for fd : %d.\n", fd);

    //lock
    _q_mtx.lock();

    int size = add_msg_to_rtx_queue(idx, p_msg);
    if(size < 0) {
        KnPrint(KN_LOG_ERR, "msg sending queue is full:%d.\n", size);
        _q_mtx.unlock();
        delete p_msg;
        return -1; 
    }

    //notify
    _q_cond_var.notify_all();

    //unlock
    _q_mtx.unlock();

    return size;
}

void MsgSender::run() {
    running_sts = true;

    while(!stopped) {
        bool is_msg_avail = false;

        //lock
        _q_mtx.lock();
        is_msg_avail = is_msg_in_rtx_queue();

        if(is_msg_avail) {
            //get one msg
            int fd = -1;
            KnBaseMsg* p_msg = fetch_msg_from_rtx_queue(fd);

            //unlock
            _q_mtx.unlock();

            //send it
            int idx = get_index_from_fd(fd);
            BaseDev* pDev = p_mgr->getDevFromIdx(idx);

            KnTsPrint(KN_LOG_DBG, "Got msg from sender queue, sending it: %d.\n", fd);

            send_msg_to_dev(p_msg, pDev);
        } else {
            //wait for notity
            
    #if defined(THREAD_USE_CPP11)
            _q_cond_var.wait_for(_q_mtx, std::chrono::milliseconds(SND_Q_IDLE_TIME));

    #else
             _q_cond_var.wait_for(_q_mtx, SND_Q_IDLE_TIME);
    #endif
            _q_mtx.unlock();
        }
    }

    KnPrint(KN_LOG_ERR, "sending thread exited.\n");
    running_sts = false;
}


