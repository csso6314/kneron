/**
 * @file      MsgReceiver.cpp
 * @brief     implementation file for msg receiver class
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "MsgReceiver.h"
#include "MsgMgmt.h"
#include "KnLog.h"

#include <unistd.h>
#include <string.h>

MsgReceiver::MsgReceiver(MsgMgmt* p_ref) : MsgXBase(p_ref) {
}

MsgReceiver::~MsgReceiver() {
}


void MsgReceiver::run_usb(int idx) {
    int8_t mbuf[MAX_USB_MSG_LEN];
    int msglen = sizeof(mbuf);
    USBDev* pDev = usb_dev[idx];

    if(!pDev) return;

    KnTsPrint(KN_LOG_DBG, "starting usb receiver :%d. \n", idx);

    while(!stopped) {
       usb_sts[idx] = true;
#if defined(OS_TYPE_MACOS)
       while((sem_wait(p_mgr->recv_sem[idx])==-1)&&(errno == EINTR)) continue;/*restart if interrupted by handler*/
#else
       while((sem_wait(&p_mgr->recv_sem[idx])==-1)&&(errno == EINTR)) continue;/*restart if interrupted by handler*/
#endif
         KnTsPrint(KN_LOG_DBG, "usb receiver :%d,%d,%d. \n", idx,p_mgr->is_cmd_on_going(idx),stopped);
         if(p_mgr->is_cmd_on_going(idx) == true)//add cmd on going flag
         {
             if (!stopped) 
             {
                KnTsPrint(KN_LOG_DBG, "calling usb interrupt yg testing....\n");
                int ret_len = pDev->usb_recv(mbuf, msglen);
                if(ret_len < 0) {
                    if(ret_len == -7) { //timeout
                        p_mgr->clear_cmd_going_flag(idx);
                        continue;
                    } else {
                        KnPrint(KN_LOG_ERR, "usb :%d returned err:%d.\n", idx, ret_len);
                        break;
                    }
                }

                if(ret_len > 0 && (p_mgr->is_cmd_on_going(idx) == true)){ //msg received
                    KnTsPrint(KN_LOG_DBG, "usb dev received msg len:%d...\n", ret_len);
                    add_msg_to_rq(idx, mbuf, ret_len);
                }
            }

         }
    
    }
    usb_sts[idx] = false;

    KnPrint(KN_LOG_ERR, "msg rx polling thread for usb :%d exited.\n", idx);
}

void MsgReceiver::run() {
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = MAX_POLL_TIMEVAL * 1000;
    int8_t mbuf[MAX_MSG_LEN];
    int msglen = sizeof(mbuf);
    int max_fd = -1;
    fd_set org_rfds;
    FD_ZERO (&org_rfds);
    for(int i = 0; i < MAX_COM_DEV; i++) {
        int fd = dev_fds[i];
        if(fd < 0 || fd >= USB_FAKE_FD_START) continue;

        FD_SET (fd, &org_rfds);
        if(fd > max_fd) max_fd = fd;
        KnPrint(KN_LOG_DBG, "add fd:%d to the polling list.\n", fd);
    }
    
    if(max_fd < 0) {
        KnPrint(KN_LOG_ERR, "no com dev is open.\n");
        return;
    }
    
    while(!stopped) {
       
        running_sts = true;
        fd_set rfds = org_rfds;
        
        int ret_val = select (max_fd + 1, &rfds, NULL, NULL, &tv);
        if(ret_val < 0) {
            if(errno == EINTR) {
                KnPrint(KN_LOG_ERR, "polling thread got an interrupt, retrying.\n");
                continue;
            } else {
                KnPrint(KN_LOG_ERR, "polling thread got unexpected exception, aborted.\n");
                break;
            }
        } else if(ret_val == 0) {
            //time out 
            continue;
        }

        for(int i = 0; i < MAX_COM_DEV; i++) {
            int fd = dev_fds[i];
            if(fd < 0) continue;

            if(FD_ISSET (fd, &rfds)) {
                //fd is ready
                memset(mbuf, 0, msglen);

                BaseDev* pDev = p_mgr->getDevFromIdx(i);

                int nlen = pDev->receive((char *)mbuf, msglen);

                if(nlen > 0) add_msg_to_rq(i, mbuf, nlen);
            }
        }
    }
    running_sts = false;

    KnPrint(KN_LOG_ERR, "msg rx polling thread exited.\n");
}


int MsgReceiver::add_msg_to_rq(int idx, int8_t* msgbuf, int msglen) {
    if(idx < 0 || idx >= MAX_COM_DEV) {
        KnPrint(KN_LOG_ERR, "invalid com dev index.\n");
        return -1;
    }

    if(msglen < 1 || msgbuf == NULL) {
        KnPrint(KN_LOG_ERR, "invalid msg buffer to add.\n");
        return -1;
    }

    KnBaseMsg* p_base = KnBaseMsg::create_rsp_msg(msgbuf, msglen);

    if(!p_base || (p_base->is_msg_valid() == false) ) {
        KnPrint(KN_LOG_ERR, "create response message failed.\n");
        return -1;
    }

    //lock the mutex
    _q_mtx.lock();

    int num = add_msg_to_rtx_queue(idx, p_base);
    if(num < 0) {
        KnPrint(KN_LOG_ERR, "msg recv queue is full for :%d.\n", idx);
        //unlock mutex
        _q_mtx.unlock();
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "added rsp message to queue, notifying...\n");
    //unlock mutex and notify
    _q_cond_var.notify_all();
    _q_mtx.unlock();

    return num;
}

KnBaseMsg* MsgReceiver::rcv_cmd_rsp(int& idx) {
    bool is_msg_avail = false;

    //lock
    _q_mtx.lock();
    is_msg_avail = is_msg_in_rtx_queue();

    if(is_msg_avail) {
        int fd = -1;
        KnBaseMsg* p_msg = fetch_msg_from_rtx_queue(fd);
        _q_mtx.unlock();

        idx = get_index_from_fd(fd);
        KnTsPrint(KN_LOG_DBG, "Got response message from fd:%d.\n", fd);
        return p_msg;
    }
     #if defined(THREAD_USE_CPP11)
    _q_cond_var.wait_for(_q_mtx, std::chrono::milliseconds(RCV_Q_IDLE_TIME));
    #else
    
     _q_cond_var.wait_for(_q_mtx, RCV_Q_IDLE_TIME);

    #endif

    _q_mtx.unlock();
    return NULL;
}

