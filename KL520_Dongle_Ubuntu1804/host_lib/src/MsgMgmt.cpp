/**
 * @file      MsgMgmt.cpp
 * @brief     implementation file for msg management class
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "MsgMgmt.h"
#include "MsgReceiver.h"
#include "MsgSender.h"
#include "KnLog.h"

#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#define USB_RECVSEM_NAME "/USB_RECVSEM"
//#include <thread>
using namespace std;

//static function for calling mgr's handle notification
static void handler_entry_func(MsgMgmt* pmgr) {
    pmgr->handle_msg_rx_queue();
}

void* MsgMgmt_pthreadfunc_handle(void *s)
{
   MsgMgmt *pmgr =  static_cast<MsgMgmt *>(s);
   handler_entry_func(pmgr);
   return NULL;
}
MsgMgmt::MsgMgmt(SyncMsgHandler* sl) {
    p_receiver = new MsgReceiver(this);
    p_sender = new MsgSender(this);
    running = false;
    stopped = false;
    _sync_hdlr = sl;
    for(int i = 0 ; i < MAX_COM_DEV; i++) { 
        _com_devs[i] = NULL;
        _cmd_on_going[i] = false;
#if defined(OS_TYPE_MACOS)
        sem_close(recv_sem[i]);
        sem_unlink(USB_RECVSEM_NAME);
        recv_sem[i] = sem_open(USB_RECVSEM_NAME, O_CREAT, 0660, 0);
#else
        sem_init(&recv_sem[i],0,0); //init the message receive semaphore
#endif
    }
}

MsgMgmt::~MsgMgmt() {
    if(p_receiver) delete p_receiver;
    if(p_sender) delete p_sender;
    for(int i = 0 ; i < MAX_COM_DEV; i++) {
        if(_com_devs[i] != NULL) delete _com_devs[i];
    }
}

bool MsgMgmt::is_valid_dev(int idx) {
    if(idx < 0 || idx >= MAX_COM_DEV) return false;

    if(_com_devs[idx] == NULL) return false;
    
    return true;
}

pthread_t MsgMgmt::start_receiver(int idx){
    return p_receiver->start_recv(idx);
}

void MsgMgmt::start() {
    p_receiver->start();
}

void MsgMgmt::stop_receiver() {
    p_receiver->stop();
}

bool MsgMgmt::is_receiver_running() {
    return p_receiver->is_running();
}

void MsgMgmt::start_sender() {
    p_sender->start();
}

void MsgMgmt::stop_sender() {
    p_sender->stop();
}

bool MsgMgmt::is_sender_running() {
    return p_sender->is_running();
}

void MsgMgmt::start_handler() {
    running = false;
    stopped = false;
    #if defined(THREAD_USE_CPP11)
    std::thread hdl (handler_entry_func, this);
    hdl.detach();
    #else
    //Thread hdl(handler_entry_func, this);
    pthread_t hdl_t;
    pthread_create(&hdl_t,NULL,MsgMgmt_pthreadfunc_handle,(void *)this);
    pthread_detach(hdl_t); 
    #endif
   

    //yield to let thrd run first
    usleep(10000);
   
    if(running) {
        KnTsPrint(KN_LOG_DBG, "msg handler started successfully.\n");
    } else {
        KnTsPrint(KN_LOG_ERR, "msg handler could not be started!\n");
    }
    return;  
}

void MsgMgmt::stop_handler() {
    stopped = true;
    for(int i=0;i < MAX_COM_DEV; i++)
    {    
#if defined(OS_TYPE_MACOS)
        sem_post(recv_sem[i]);
        sem_close(recv_sem[i]);
        sem_unlink(USB_RECVSEM_NAME);
#else
        sem_post(&recv_sem[i]);
        sem_close(&recv_sem[i]);
#endif
    }
}

bool MsgMgmt::is_handler_running() {
    return running;
}

int MsgMgmt::add_usb_device(USBDev* pDev, int idx)
{
    if((!pDev) ||  (idx >= MAX_COM_DEV) || (idx < 0))
    {
         KnPrint(KN_LOG_ERR, "add usb device invalid parameters.\n");
          return -1;
    }

    if(_com_devs[idx] != NULL)
    {
        KnPrint(KN_LOG_ERR, "the index is occupied,please use another index or remove old devices.\n");
        return -1;
    }

    int ret1 = p_receiver->add_usb_device(pDev, idx);
    if(ret1 < 0) {
        KnPrint(KN_LOG_ERR, "adding usb dev failed:%d.\n", ret1);
        _com_devs[idx] = NULL;
        return -1;
    }
    else
    {
        _com_devs[idx] = pDev;
        KnPrint(KN_LOG_DBG, "the usb dev idx is %d.\n", idx);
    }
    return idx;
}

int MsgMgmt::add_usb_device(USBDev* pDev) {
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "invalid usb dev.\n");
        return -1;
    }

    int idx = -1;
    for(int i = 0; i < MAX_COM_DEV; i++) {
        if(_com_devs[i] == NULL) {
            idx = i;
            _com_devs[i] = pDev;
            break;
        }
    }

    if(idx < 0) return -1;

    int ret1 = p_receiver->add_usb_device(pDev, idx);
    if(ret1 < 0) {
        KnPrint(KN_LOG_ERR, "adding usb dev failed:%d.\n", ret1);
        _com_devs[idx] = NULL;
        return -1;
    }
    KnPrint(KN_LOG_DBG, "the usb dev idx is %d.\n", idx);
    return idx;
}

int MsgMgmt::rm_usb_device(USBDev* pDev, int idx)
{
    if((!pDev) ||  (idx >= MAX_COM_DEV) || (idx < 0))
    {
         KnPrint(KN_LOG_ERR, "add usb device invalid parameters.\n");
          return -1;
    }

    if(_com_devs[idx] == NULL)
    {
        KnPrint(KN_LOG_ERR, "device not exist.\n");
        return -1;
    }

    int ret = p_receiver->rm_usb_device(pDev, idx);
    _com_devs[idx] = NULL;
    
    return ret;
}

int MsgMgmt::rm_usb_device(USBDev* pDev) {
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "invalid usb dev.\n");
        return -1;
    }

    int idx = -1;
    for(int i = 0; i < MAX_COM_DEV; i++) {
        if(_com_devs[i] == pDev) {
            idx = i;
            _com_devs[i] = NULL;
            break;
        }
    }

    if(idx < 0) return -1;

    int ret1 = p_receiver->rm_usb_device(pDev, idx);
   
    KnPrint(KN_LOG_DBG, "remove the usb dev [%d].\n",idx);

    return ret1;
}

int MsgMgmt::add_device(BaseDev* pDev) {
    if(!pDev || pDev->get_dev_fd() < 0) {
        KnPrint(KN_LOG_ERR, "invalid com dev fd.\n");
        return -1;
    }

    int idx = -1;
    int fd = pDev->get_dev_fd();

    for(int i = 0; i < MAX_COM_DEV; i++) {
        if(_com_devs[i] == NULL) {
            idx = i;
            _com_devs[i] = pDev;
            break;
        }
    }

    if(idx < 0) return -1;

    int ret1 = p_receiver->add_device_rtx(fd, idx);
    int ret2 = p_sender->add_device_rtx(fd, idx);

    if(ret1 < 0 || ret2 < 0) {
        KnPrint(KN_LOG_ERR, "adding fd to rx/tx failed:%d, %d.\n", ret1, ret2);
        _com_devs[idx] = NULL;
        return -1;
    }
    KnPrint(KN_LOG_DBG, "the dev idx is %d.\n", idx);
    return idx;
}

int MsgMgmt::send_cmd_msg(int dev_idx, KnBaseMsg* p_msg) {
    BaseDev* pDev = _com_devs[dev_idx];

    if(!pDev || !p_msg) {
        KnPrint(KN_LOG_ERR, "device pointer or msg pointer is null.\n");
        return -1;
    }

    int fd = pDev->get_dev_fd();

    int n = p_sender->send_cmd_msg(p_msg, pDev);
    if(n < 0) {
        KnPrint(KN_LOG_ERR, "Sending msg to dev failed:%d.\n", n);
        return n;
    }

    MsgHandleSts sts;
    sts._msg_sts = KDP_MSG_STS_GOING;
    gettimeofday(&sts._send_time, NULL);

    int msg_type = p_msg->get_msg_type();

    int idx = p_sender->get_index_from_fd(fd);

    //lock
    _mtx.lock();
    _msg_buf[idx][msg_type] = sts;
    //unlock
    _mtx.unlock();

    return n;
}

int MsgMgmt::send_cmd_msg_sync(int dev_idx, KnBaseMsg* p_msg) {
    BaseDev* pDev = _com_devs[dev_idx];

    if(!pDev || !p_msg) {
        KnPrint(KN_LOG_ERR, "device pointer or msg pointer is null.\n");
        if(p_msg) delete p_msg;
        return -1;
    }

    int n = p_sender->send_cmd_msg_sync(p_msg, pDev);
    if(n < 0) {
        KnPrint(KN_LOG_ERR, "Sending msg to dev failed:%d.\n", n);
        return n;
    }
    return n;
}

void MsgMgmt::handle_msg_rx_queue() {
    running = true;

    while(!stopped) {
        int idx = -1;
        KnBaseMsg* p_msg = p_receiver->rcv_cmd_rsp(idx);

        if( idx < 0 || idx >= MAX_COM_DEV ) continue;
        if(!p_msg) {  //no avail msg
            //TODO check running map for rsp timeout
            continue;
        }

        uint16_t msg_type = p_msg->get_msg_type();
        uint16_t req_type = p_msg->get_req_type(msg_type);

        _mtx.lock();
        map<uint16_t, MsgHandleSts>::iterator iter = _msg_buf[idx].find(req_type);

        if(iter == _msg_buf[idx].end()) {
            _mtx.unlock();

            //need check for rsp messages do not have requests

            if(_sync_hdlr) {
                _sync_hdlr->msg_received_rx_queue(p_msg, req_type, idx);
            }
            continue;
        }

        //MsgHandleSts& sts = iter->second;
        //result msg is received.
        //remove the item
        _msg_buf[idx].erase(iter);
        _mtx.unlock();

    }
    
    running = false;
    KnPrint(KN_LOG_ERR, "msg handling thread exited.\n");
    return;
}

