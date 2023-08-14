/**
 * @file      MsgXBase.h
 * @brief     msg tranceiver base class header file
 * @version   0.1 - 2019-04-15
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __MSG_XBASE_H__
#define __MSG_XBASE_H__

#define MAX_QUEUE_SIZE 128
#define MAX_MSG_LEN      (1024*10)
#define MAX_USB_MSG_LEN  (1024*16)

#include "KnMsg.h"
#include "KdpHostApi.h"
#include "BaseDev.h"
#include "USBDev.h"

#include <queue>

#if defined(THREAD_USE_CPP11)
#include <mutex>
#include <condition_variable>
#endif

using namespace std;

struct MsgQ {
    queue<KnBaseMsg*> rtx_q;
};


class MsgMgmt;
class MsgXBase;
typedef struct 
{
    MsgXBase* p_inst;
    int idx;
}runusb_data;
class MsgXBase {
public:
    MsgMgmt* p_mgr;

    virtual void start();
    virtual void stop(); 
    bool is_running(); 

    virtual pthread_t start_recv(int idx);

    virtual void run() = 0; //loop for Rx/Tx msgs
    virtual void run_usb(int idx);
    virtual int send_cmd_msg(KnBaseMsg* p_msg, BaseDev* pDev); 
    virtual int send_cmd_msg_sync(KnBaseMsg* p_msg, BaseDev* pDev);
    virtual KnBaseMsg* rcv_cmd_rsp(int& idx);

    int add_device_rtx(int fd, int idx) { 
        dev_fds[idx] = fd; 
        return idx;
    }

    int add_usb_device(USBDev* pusb_dev, int idx) {
        usb_dev[idx] = pusb_dev;
        dev_fds[idx] = pusb_dev->dev_fd; 
        return idx;
    }

    int rm_usb_device(USBDev* pusb_dev, int idx)
    {
        usb_dev[idx] = NULL;
        dev_fds[idx] = 0; 
        usb_sts[idx] = false;
        return 0;
    }

    int get_index_from_fd(int fd);

    MsgXBase(MsgMgmt* p_ref);
    virtual ~MsgXBase();

   
    #if defined(THREAD_USE_CPP11)
    static void thrd_func(MsgXBase* p_inst);
    static void thrd_func_usb(MsgXBase* p_inst, int idx);
    #else
    static void thrd_func(MsgXBase* p_inst);
    static void thrd_func_usb(runusb_data *data );
    #endif
    
protected:
    bool running_sts;
    bool stopped;
    bool usb_sts[MAX_COM_DEV];

    

    bool is_msg_in_rtx_queue();
    int  add_msg_to_rtx_queue(int idx, KnBaseMsg* p_msg);
    KnBaseMsg* fetch_msg_from_rtx_queue(int& fd);

    MsgQ      msg_q[MAX_COM_DEV];
    int       dev_fds[MAX_COM_DEV];
    #if defined(THREAD_USE_CPP11)
    std::mutex                      _q_mtx;
    std::condition_variable_any     _q_cond_var;
    #else
    mutex         _q_mtx;
    Condition     _q_cond_var;
    #endif

    USBDev*  usb_dev[MAX_COM_DEV];
};

#endif
