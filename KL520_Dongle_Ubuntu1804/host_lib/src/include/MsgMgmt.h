/**
 * @file      MsgMgmt.h
 * @brief     msg management class header file
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __MSG_MGMT_H__
#define __MSG_MGMT_H__

#include "MsgXBase.h"
#include "BaseDev.h"
#include "SyncMsgHandler.h"
#include <map>

#include <semaphore.h>

#define KDP_MSG_STS_NOT_INIT   -1
#define KDP_MSG_STS_GOING      0 
#define KDP_MSG_STS_COMPLT     1

using namespace std;

struct MsgHandleSts {
    timeval         _send_time;
    int             _msg_sts;
};

class MsgMgmt {
public:
    MsgMgmt(SyncMsgHandler* sl);
    virtual ~MsgMgmt();

    int add_device(BaseDev* pDev);
    int add_usb_device(USBDev* pDev);
    int add_usb_device(USBDev* pDev, int idx);

    int rm_usb_device(USBDev* pDev);
    int rm_usb_device(USBDev* pDev, int idx);

    void start();
    pthread_t start_receiver(int idx);
    void stop_receiver();
    bool is_receiver_running();

    void start_handler();
    void stop_handler();
    bool is_handler_running();

    void start_sender();
    void stop_sender();
    bool is_sender_running();

    //void test();

    int send_cmd_msg(int dev_idx, KnBaseMsg* p_msg);
    int send_cmd_msg_sync(int dev_idx, KnBaseMsg* p_msg);
    void handle_msg_rx_queue();

    void set_cmd_going_flag(int idx) { _cmd_on_going[idx] = true; }
    void clear_cmd_going_flag(int idx) { _cmd_on_going[idx] = false; }
    bool is_cmd_on_going(int idx) { return _cmd_on_going[idx]; }

    BaseDev* getDevFromIdx(int idx) { return _com_devs[idx]; }
    bool is_valid_dev(int dev_idx);
    
#if defined(OS_TYPE_MACOS)
    sem_t *recv_sem[MAX_COM_DEV];
#else
    sem_t recv_sem[MAX_COM_DEV];
#endif

private:
    MsgXBase*   p_receiver;
    MsgXBase*   p_sender;

    SyncMsgHandler*        _sync_hdlr;

    BaseDev*               _com_devs[MAX_COM_DEV];

   volatile bool                   _cmd_on_going[MAX_COM_DEV];

    bool running;
    bool stopped;
    #if defined(THREAD_USE_CPP11)
    std::mutex  _mtx;
    #else
    mutex  _mtx;
    #endif

    map<uint16_t, MsgHandleSts> _msg_buf[MAX_COM_DEV];
};

#endif
