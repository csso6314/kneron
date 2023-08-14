/**
 * @file      MsgXBase.cpp
 * @brief     implementation file for msg RX/TX base class
 * @version   0.1 - 2019-04-15
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "MsgXBase.h"
#include "KnLog.h"

#include <unistd.h>

//#include <thread>

MsgXBase::MsgXBase(MsgMgmt* p_ref) {
    stopped = false;
    running_sts = false;
    p_mgr = p_ref;

    for(int i = 0; i < MAX_COM_DEV; i++) {
        dev_fds[i] = -1;
        usb_dev[i] = NULL;
        usb_sts[i] = false;
    }
}

MsgXBase:: ~MsgXBase() {
    _q_mtx.lock();

    while(1) {
        int t;
        KnBaseMsg* pmsg = fetch_msg_from_rtx_queue(t);

        if(!pmsg) break;

        delete pmsg;
    }

    _q_mtx.unlock();
}

void MsgXBase::thrd_func(MsgXBase* p_inst) {
    p_inst->run();
}

#if defined(THREAD_USE_CPP11)
void MsgXBase::thrd_func_usb(MsgXBase* p_inst, int idx) {
    p_inst->run_usb(idx);
}
#else
void MsgXBase::thrd_func_usb(runusb_data *p_runusb) {
    p_runusb->p_inst->run_usb(p_runusb->idx);
}
#endif

void MsgXBase::stop() {
    stopped = true;
}

bool MsgXBase::is_running() {
    if(running_sts) return true; 

    for(int i = 0; i < MAX_COM_DEV; i++) {
        if(usb_sts[i]) return true;
    }
    return false;
}

void MsgXBase::run_usb(int idx) {
    return;
}

void *MsgBox_ThreadFunc_th_handle(void *s)
{
    MsgXBase *p_inst = (MsgXBase *)(s);
    MsgXBase::thrd_func(p_inst);
    return NULL;
}

void *MsgBox_ThreadFunc_th1_handle(void *s)
{
    //extern class MsgReceiver;
    runusb_data test ;
    test = *static_cast< runusb_data *>(s);
    MsgXBase::thrd_func_usb(&test);
    return NULL;
}

pthread_t MsgXBase::start_recv(int idx)
{
      pthread_t th1_t = 0;
     //start the usb receiver
     runusb_data runusb_data;
     runusb_data.p_inst = this; 

     USBDev* pDev = usb_dev[idx];

     if(pDev)
     {
        #if defined(THREAD_USE_CPP11)
        std::thread th1(MsgXBase::thrd_func_usb, this, i);
        th1.detach();
        #else
        runusb_data.idx = idx;
        pthread_create(&th1_t,NULL,MsgBox_ThreadFunc_th1_handle,(void *)&runusb_data);
        pthread_detach(th1_t);
        #endif

        usleep(10000);
        if(usb_sts[idx]) {
            KnTsPrint(KN_LOG_DBG, "rtx thrd for usb:%d  started successfully.\n", idx);
        } else {
            KnTsPrint(KN_LOG_ERR, "rtx thrd for usb:%d could not be started!\n", idx);
        }
     }
     
     return th1_t;
}

void MsgXBase::start() {
    stopped = false;
    running_sts = false;

    for(int i = 0; i < MAX_COM_DEV; i++) {
        usb_sts[i] = false;
    }

    /*create thread;
    #if defined(THREAD_USE_CPP11)
    std::thread th(MsgXBase::thrd_func, this);
    th.detach();
    #else
    #include "pthread.h"
    pthread_t th_t;
    pthread_create(&th_t,NULL,MsgBox_ThreadFunc_th_handle,(void *)this);
    pthread_detach(th_t);
    #endif
    
    //yield to let thrd run first
    usleep(10000);

    if(running_sts) {
        KnTsPrint(KN_LOG_DBG, "rtx thrd started successfully.\n");
    } else {
        KnTsPrint(KN_LOG_ERR, "rtx thrd could not be started!\n");
    }*/
    
    return;
}

int MsgXBase::get_index_from_fd(int fd) {
    for(int i = 0; i < MAX_COM_DEV; i++) {
        if(dev_fds[i] == fd) {
            return i;
        }
    }
    KnPrint(KN_LOG_ERR, "could not find fd:%d.\n", fd);
    return -1;
}

bool MsgXBase::is_msg_in_rtx_queue() {
    for(int i = 0; i < MAX_COM_DEV; i++) {
        if(msg_q[i].rtx_q.empty() == false) {
            return true;
        }
    }
    return false;
}

int MsgXBase::add_msg_to_rtx_queue(int idx, KnBaseMsg* p_msg) {
    //check size
    int size = msg_q[idx].rtx_q.size();
    if(size >= MAX_QUEUE_SIZE) {
        KnPrint(KN_LOG_ERR, "msg queue is full:%d.\n", size);
        return -1; 
    }

    //add it
    msg_q[idx].rtx_q.push(p_msg);

    return size+1;
}

int MsgXBase::send_cmd_msg(KnBaseMsg* p_msg, BaseDev* pDev) {
    return -1;
}

int MsgXBase::send_cmd_msg_sync(KnBaseMsg* p_msg, BaseDev* pDev) {
    return -1;
}

KnBaseMsg* MsgXBase::rcv_cmd_rsp(int& idx) {
    return NULL;
}

KnBaseMsg* MsgXBase::fetch_msg_from_rtx_queue(int& fd) {
    for(int i = 0; i < MAX_COM_DEV; i++) {
        if(msg_q[i].rtx_q.empty() == false) {
            KnBaseMsg* p_msg = msg_q[i].rtx_q.front();
            msg_q[i].rtx_q.pop();
            fd = dev_fds[i];
            return p_msg;
        }
    }
    return NULL;
}

