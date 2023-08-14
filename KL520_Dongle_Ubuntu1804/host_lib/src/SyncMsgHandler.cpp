/**
 * @file      SyncMsgHandler.cpp
 * @brief     implementation file for sync message handler
 * @version   0.1 - 2019-04-23
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "UARTDev.h"
#include "USBDev.h"
#include "MsgMgmt.h"
#include "KnLog.h"
#include "KnUtil.h"
#include "SyncMsgHandler.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <vector>
using namespace std;

extern MsgMgmt*                _p_mgr; //msg manager

SyncMsgHandler::SyncMsgHandler() {
}

SyncMsgHandler::~SyncMsgHandler() {
    //loop for pending rsp msg
    for(int i = 0; i < MAX_COM_DEV; i++) {
        _map_mtx[i].lock();
        map<uint16_t, KnBaseMsg*>::iterator iter;
        for(iter = _runing_map[i].begin(); iter != _runing_map[i].end(); ++iter) {
            KnBaseMsg* p = iter->second;
            if(p) delete p;
        }
        _map_mtx[i].unlock();
    }
}

int SyncMsgHandler::wait_dev_restart(int dev_idx) {
    //get USB dev
    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    for(int i = 0; i < KDP_RESET_NUM; i++) {
        usleep(KDP_RESET_TIME);
        KnPrint(KN_LOG_DBG, "retrying to reconnect usb...:%d.\n", dev_idx);
        int ret = pUsb->reconnect_usb();
        if(ret == 0) {
            KnPrint(KN_LOG_DBG, "usb reconnected...:%d.\n", dev_idx);
            return 0;
        }
    }

    KnPrint(KN_LOG_ERR, "usb reconnect failed...:%d.\n", dev_idx);
    return -1;
}

KnBaseMsg*  SyncMsgHandler::wait_rsp_msg(int dev_idx, uint16_t msg_type, int wait_num)
{
    map<uint16_t, KnBaseMsg*>::iterator iter;
    KnBaseMsg* p_rsp = NULL;
    
    switch(msg_type) {
    case CMD_DME_GET_STATUS:
        wait_num = 2; // 2*200 ms
        break;
    case CMD_DME_SEND_IMAGE:
        break;
    default:
        if(wait_num < 5) wait_num = 5; //min 1 sec
        break;
    } 

    // judge the request message type
    _map_mtx[dev_idx].lock();
     iter = _runing_map[dev_idx].find(msg_type);
      //if could not find
     if(iter == _runing_map[dev_idx].end()) 
     {
        _map_mtx[dev_idx].unlock(); //unlock
        KnTsPrint(KN_LOG_ERR, "no req msg in the local map, no need wait:%d.\n", msg_type);
        _p_mgr->clear_cmd_going_flag(dev_idx);
        return NULL;
     }
     else
     {
          _map_mtx[dev_idx].unlock(); //unlock

         for(int i = 0; i < wait_num; i++)
         {
            //wait for msg
             _cond_mtx.lock();
             if(_p_mgr->is_cmd_on_going(dev_idx) == false)
             {
                 _p_mgr->set_cmd_going_flag(dev_idx);
                 #if defined(OS_TYPE_MACOS)
                 sem_post(_p_mgr->recv_sem[dev_idx]);
                 #else
                 sem_post(&_p_mgr->recv_sem[dev_idx]);
                 #endif
             }

#if defined(THREAD_USE_CPP11)
            _cond_wait.wait_for(_cond_mtx, std::chrono::milliseconds(KDP_SYNC_WAIT_TIME));
#else
            _cond_wait.wait_for(_cond_mtx, KDP_SYNC_WAIT_TIME);
#endif
            _cond_mtx.unlock();

             //lock map
            _map_mtx[dev_idx].lock();
            //get the iter
            iter = _runing_map[dev_idx].find(msg_type);

            if(iter->second == NULL) 
            {
                _map_mtx[dev_idx].unlock(); //unlock
                continue;
            }
            else
            {
                p_rsp = iter->second;
                _runing_map[dev_idx].erase(iter);
                _map_mtx[dev_idx].unlock();
                _p_mgr->clear_cmd_going_flag(dev_idx);
                KnTsPrint(KN_LOG_DBG, "rsp msg from device received:0x%x.\n", msg_type);
                return p_rsp; //return msg
            }
         }
     }   

    KnTsPrint(KN_LOG_ERR, "dev[%d] waiting for rsp timeout.\n",dev_idx);
    _p_mgr->clear_cmd_going_flag(dev_idx);
    return NULL; //timeout
}

int SyncMsgHandler::kdp_sfid_start(int dev_idx, uint32_t* rsp_code, uint32_t* img_size, float thresh,
                uint16_t width, uint16_t height, uint32_t format) {
    uint16_t msg_type = CMD_SFID_START;
    uint16_t msg_len = sizeof(float)+sizeof(uint32_t)*2 + sizeof(uint16_t)*2;

    //create req message
    KnSFIDStartReq* p_req = new KnSFIDStartReq(msg_type, msg_len, thresh,
                width, height, format);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid start req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid start to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid start request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid start req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid start rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid start response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid start rsp from device received.\n");

    //extract msg content
    KnSFIDStartRsp* sfid_msg = (KnSFIDStartRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    *img_size = sfid_msg->_img_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, img size:%d from sfid img rsp.\n", sfid_msg->_rsp_code, sfid_msg->_img_size);
    //free resposne memory
    delete p_rsp;
    return 0;
}

#if 0
int SyncMsgHandler::kdp_sfid_new_user_req(int dev_idx, uint32_t usr_id, uint32_t img_idx) {
    uint16_t msg_type = CMD_SFID_NEW_USER;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(uint32_t);

    //create req message
    KnSFIDNewUserReq* p_req = new KnSFIDNewUserReq(msg_type, msg_len, usr_id, img_idx);

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid new user req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_DBG, "sending sfid new user request to device failed:%d.\n", res);
        return res;
    }
    KnTsPrint(KN_LOG_DBG, "sending sfid new user to dev completed. \n");
    return 0;
}
#endif

int SyncMsgHandler::kdp_sfid_new_user(int dev_idx, uint16_t usr_id, uint16_t img_idx) {
    uint16_t msg_type = CMD_SFID_NEW_USER;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(uint32_t);

    //create req message
    KnSFIDNewUserReq* p_req = new KnSFIDNewUserReq(msg_type, msg_len, usr_id, img_idx);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid new user req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid new user to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid new user request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid new user req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid new user rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid new user response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid new user rsp from device received.\n");

    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_register(int dev_idx, uint32_t usr_id, uint32_t* rsp_code) {
    uint16_t msg_type = CMD_SFID_ADD_DB;
    uint16_t msg_len = sizeof(uint32_t);

    //create req message
    KnSFIDRegisterReq* p_req = new KnSFIDRegisterReq(msg_type, msg_len, usr_id);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid register req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid register to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_DBG, "sending sfid register request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid register req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid register rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid register response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid register rsp from device received.\n");

    //extract msg content
    KnSFIDRegisterRsp* sfid_msg = (KnSFIDRegisterRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, from sfid register rsp.\n", sfid_msg->_rsp_code);

    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_del_db(int dev_idx, uint32_t usr_id, uint32_t* rsp_code) {
    uint16_t msg_type = CMD_SFID_DELETE_DB;
    uint16_t msg_len = sizeof(uint32_t);

    //create req message
    KnSFIDDelDBReq* p_req = new KnSFIDDelDBReq(msg_type, msg_len, usr_id);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid del db req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid del db to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid del db request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid del db req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid del db rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid del db response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid del db rsp from device received.\n");

    //extract msg content
    KnSFIDDelDBRsp* sfid_msg = (KnSFIDDelDBRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from sfid del db rsp.\n", sfid_msg->_rsp_code);
    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_edit_db(int dev_idx, uint32_t mode, char** p_buf, uint32_t *p_len, uint32_t* rsp_code) {
    uint16_t msg_type = CMD_SFID_EDIT_DB;
    uint16_t msg_len = 2 * sizeof(uint32_t) + 2 * sizeof(uint16_t);
   
    //create req message

    KnSFIDEditDBReq* p_req = new KnSFIDEditDBReq(msg_type, msg_len, mode, 0, (uint32_t)0);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid edit db req failed.\n");
        delete p_req;
        return -1;
    }
    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid edit db to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid edit db request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid edit db req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid edit db rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid edit db response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid edit db rsp from device received.\n");

    //extract msg content
    KnSFIDEditDBRsp* sfid_msg = (KnSFIDEditDBRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    uint32_t data_size = sfid_msg->_data_size;
    *p_len = data_size;
    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from sfid edit db rsp.\n", sfid_msg->_rsp_code);
	
    *rsp_code = sfid_msg->_rsp_code;
    if (0 == data_size) {
		KnPrint(KN_LOG_ERR, "msg config error in db.\n");
        return 0;
    }

    *p_buf = (char *) malloc(data_size * sizeof (uint16_t) );

    memcpy(*p_buf, sfid_msg->get_msg_payload()  +  2 * sizeof(uint32_t) + sizeof(KnMsgHdr), data_size * sizeof (uint16_t));
     	
    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_export(int dev_idx, char** p_buf, uint32_t *p_len, uint32_t* rsp_code) {
    uint16_t msg_type = CMD_SFID_EDIT_DB;
    uint16_t msg_len = 2 * sizeof(uint32_t) + 2 * sizeof(uint16_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

     //create req message
    uint32_t mode = DB_EXPORT;
    KnSFIDEditDBReq* p_req = new KnSFIDEditDBReq(msg_type, msg_len, mode, *p_len, 0);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid db query req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid db query to dev ........\n");

    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid db query request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid db query req to local map.\n");
    save_req_to_map(msg_type);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid db query rsp from device ........\n");

    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid db query response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid db query rsp from device received.\n");

    //extract msg content
    KnSFIDEditDBRsp* sfid_msg = (KnSFIDEditDBRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    uint32_t data_size = sfid_msg->_data_size;
    *p_len = data_size;
	
    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, data_size:%d from sfid db query rsp.\n", 
              *rsp_code, data_size);
    //free resposne memory
    delete p_rsp;

    if (0 == data_size) {
		KnPrint(KN_LOG_ERR, "msg config error in db.\n");
        return 0;
    }

    *p_buf = (char *) malloc(data_size);
    uint32_t r_len = (uint32_t)pUsb->receive_bulk(*p_buf, data_size);
 
    if(r_len < 0 || r_len != data_size) {
        KnPrint(KN_LOG_ERR, "receiving sfid db failed: %d.\n", r_len);
        return -1;
    }
    
    return 0;
}

int SyncMsgHandler::kdp_sfid_import(int dev_idx, char* p_buf, uint32_t p_len, uint32_t* rsp_code) {
    uint16_t msg_type = CMD_SFID_EDIT_DB;
    uint16_t msg_len = 2 * sizeof(uint32_t) + 2 * sizeof(uint16_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if (!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

     USBDev* pUsb = (USBDev*)pDev;

    //create req message
    uint32_t mode = DB_IMPORT;
    KnSFIDEditDBReq* p_req = new KnSFIDEditDBReq(msg_type, msg_len, mode, p_len, 0);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid db import req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid import to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid db import request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid db import  req to local map.\n");
    save_req_to_map(msg_type);

    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(p_buf, p_len);
    if (snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return -1;
    }
	//wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid db import rsp from device ........\n");
    KnBaseMsg*   p_rsp1 = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp1) {
        KnPrint(KN_LOG_ERR, "sfid db import response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid db import  rsp from device received.\n");
    //extract msg content
	KnSFIDEditDBRsp* sfid_msg = (KnSFIDEditDBRsp*)p_rsp1;
    *rsp_code = sfid_msg->_rsp_code;
	uint32_t data_size = sfid_msg->_data_size;
	printf("the file writen length is  0x%x\n", data_size);
         delete p_rsp1; 
    return 0;
}


int SyncMsgHandler::kdp_sfid_add_fm(int dev_idx, uint32_t usr_id, char* data_buf, 
                                    int buf_len, uint32_t* rsp_code) {
    uint16_t msg_type = CMD_SFID_EDIT_DB;
    uint16_t msg_len = 2 * sizeof(uint32_t) + 2 * sizeof(uint16_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!data_buf) {
        KnPrint(KN_LOG_ERR, "sfid db add fm is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    uint32_t mode = DB_ADD_FM;
    KnSFIDAddFmReq* p_req = new KnSFIDAddFmReq(msg_type, msg_len, mode, usr_id, 0);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid db add fm req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid db add fm to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid db add fm request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid db add fm req to local map.\n");
    save_req_to_map(msg_type);

    //to call usb interface to transfer fm data.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(data_buf, buf_len);
    if (snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid db add fm rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid db add fm response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid db add fm rsp from device received.\n");

    //extract msg content
    KnSFIDAddFmRsp* sfid_msg = (KnSFIDAddFmRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    uint32_t fm_idx = sfid_msg->_data_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, fm_idx:%d from sfid db add fm rsp.\n", 
              *rsp_code, fm_idx);
    //free resposne memory
    delete p_rsp;
    return fm_idx;
}

int SyncMsgHandler::kdp_sfid_query_fm(int dev_idx, uint32_t usr_id, uint32_t face_id, \
                                      char* data_buf, uint32_t* rsp_code) {
    uint16_t msg_type = CMD_SFID_EDIT_DB;
    uint16_t msg_len = 2 * sizeof(uint32_t) + 2 * sizeof(uint16_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!data_buf) {
        KnPrint(KN_LOG_ERR, "sfid db query fm is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    uint32_t mode = DB_QUERY_FM;
    KnSFIDQueryFmReq* p_req = new KnSFIDQueryFmReq(msg_type, msg_len, mode, usr_id, face_id);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid db query fm req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid db query fm to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid db query fm request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid db query fm req to local map.\n");
    save_req_to_map(msg_type);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid db query fm rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid db query fm response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid db query fm rsp from device received.\n");

    //extract msg content
    KnSFIDQueryFmRsp* sfid_msg = (KnSFIDQueryFmRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    uint32_t data_size = sfid_msg->_data_size;  // param2
    uint32_t feature_map_num = sfid_msg->_data; // data
    uint32_t padding_data = 0;
    uint32_t data_buf_offset = 0;
    uint32_t r_len;

    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, data_size:%d from sfid db query fm rsp.\n", 
              *rsp_code, data_size);

    printf("rsp_code: %d, data_size: %d, feature_map_num: %d from sfid db query fm rsp.\n", *rsp_code, data_size, (int)feature_map_num);
    //free resposne memory
    delete p_rsp;

    if (0 != *rsp_code) {
        return 0;
    }

    if(data_size == 0 || data_size == 0xFFFF) {
        ((uint32_t *)data_buf)[0] = data_size;
        return 0;
    }

    // Get results data
    KnImgTransAck feature_map_ack_msg(CMD_ACK_NACK, 8, 0, CMD_SFID_EDIT_DB);
    if(feature_map_ack_msg.is_msg_valid() == false) {
        KnPrint(KN_LOG_ERR, "creating feature map ack msg failed.\n");
        return -1;
    }

    for (uint32_t i = 0; i < feature_map_num; i++) {
        data_buf_offset = (i * data_size);

        r_len = (uint32_t)pUsb->receive_bulk((data_buf + data_buf_offset), data_size);

        if(r_len < 0 || r_len != data_size) {
            KnPrint(KN_LOG_ERR, "receiving sfid db fm failed: %d.\n", r_len);
            return -1;
        }

        r_len = (uint32_t)pUsb->receive_bulk((char*)(&padding_data), sizeof(uint32_t));

        if(r_len < 0 || r_len != sizeof(uint32_t)) {
            KnPrint(KN_LOG_ERR, "receiving sfid db fm padding data failed: %d.\n", r_len);
            return -1;
        }

        // only append user embedding data in NCPU compare method
        if (padding_data != 0x80) {
            data_buf[data_buf_offset + data_size - 2] = padding_data;
            data_buf[data_buf_offset + data_size - 1] = 0;
        }
    }

    return 0;
}

int SyncMsgHandler::kdp_sfid_send_img_gen(int dev_idx, char* img_buf, int buf_len,
                                          uint32_t* rsp_code, uint16_t* user_id, uint16_t* mode,
                                          uint16_t* mask, char* res) {
    uint16_t msg_type = CMD_SFID_SEND_IMAGE;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(uint16_t)*2;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    //create req message
    uint16_t _msk = 0;
    if (mask != NULL)
        _msk = *mask;
    KnSFIDSendImageReq* p_req = new KnSFIDSendImageReq(msg_type, msg_len, buf_len, _msk);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid send img req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid img to dev ........\n");
    int ret = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid send img request to device failed:%d.\n", ret);
        return ret;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid img req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(img_buf, buf_len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return snd_len;
    }

    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid img rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid send img response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid img rsp from device received.\n");

    //extract msg content
    KnSFIDSendImageRsp* sfid_msg = (KnSFIDSendImageRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    *user_id = sfid_msg->_uid;
    *mode = sfid_msg->_mode;
    _msk = sfid_msg->_out_flag;

    if(mask != NULL){
        *mask = _msk;
    }
    int off = 0;
    if(_msk & 0x01) {
        memcpy((void*)(res+off), sfid_msg->_fd_res, FD_RES_LENGTH);
        KnTsPrint(KN_LOG_DBG, "fd result is available.\n");
        off += FD_RES_LENGTH;
    }

    if(_msk & 0x02) {
        memcpy((void*)(res+off), sfid_msg->_lm_res, LM_RES_LENGTH);
        KnTsPrint(KN_LOG_DBG, "lm result is available.\n");
        off += LM_RES_LENGTH;
    }

    if(_msk & 0x04) {
        memcpy((void*)(res+off), sfid_msg->_fr_res, FR_RES_LENGTH);
        KnTsPrint(KN_LOG_DBG, "fr result is available.\n");
        off += FR_RES_LENGTH;
    }

    if(_msk & 0x08) {
        memcpy((void*)(res+off), &(sfid_msg->_live_res), sizeof(uint16_t));
        KnTsPrint(KN_LOG_DBG, "liveness result is available.\n");
        off += LV_RES_LENGTH;
    }

    if(_msk & 0x10) {
        memcpy((void*)(res+off), &(sfid_msg->_score_res), sizeof(float));
        KnTsPrint(KN_LOG_DBG, "score result is available.\n");
    }

    if(*rsp_code != 0) {
        KnTsPrint(KN_LOG_DBG, "sfid img rsp with response code:%d.\n", *rsp_code);
        delete p_rsp;
        return *rsp_code;
    }

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, user_id:%d, mode:%d, mask:0x%x from sfid img rsp.\n",
              sfid_msg->_rsp_code, sfid_msg->_uid, sfid_msg->_mode, _msk);
    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_set_db_config(int dev_idx, kapp_db_config_parameter_t* db_config_parameter,
                                           uint32_t* rsp_code)
{
    uint16_t msg_type = CMD_SFID_DB_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t);

    //create req message
    uint32_t mode = DB_SET_CONFIG;
    KnSFIDSetDbConfigReq* p_req = new KnSFIDSetDbConfigReq(msg_type, msg_len, mode, db_config_parameter);

    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid config db req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid config db to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid config db request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid config db req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid config db rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid config db response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid config db rsp from device received.\n");

    //extract msg content
    KnSFIDSetDbConfigRsp* sfid_msg = (KnSFIDSetDbConfigRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from sfid config db rsp.\n", sfid_msg->_rsp_code);
    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_get_db_config(int dev_idx, kapp_db_config_parameter_t* db_config_parameter, uint32_t* rsp_code)
{
    uint16_t msg_type = CMD_SFID_DB_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    //create req message
    uint32_t mode = DB_GET_CONFIG;
    KnSFIDGetDbConfigReq* p_req = new KnSFIDGetDbConfigReq(msg_type, msg_len, mode, db_config_parameter);

    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid get db configuration req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid get db configuration to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid get db configuration request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid get db configuration req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid get db configuration rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid get db configuration response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid get db configuration rsp from device received.\n");

    //extract msg content
    KnSFIDGetDbConfigRsp* sfid_msg = (KnSFIDGetDbConfigRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    uint32_t data_size = sfid_msg->_data_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, data_size:%d from sfid get db configuration rsp.\n", 
              *rsp_code, data_size);

    //free resposne memory
    delete p_rsp;

    if(data_size == 0 || data_size == 0xFFFF) {
        db_config_parameter->db_config.db_num = -1;
        db_config_parameter->db_config.max_fid = -1;
        db_config_parameter->db_config.max_uid = -1;
        KnPrint(KN_LOG_ERR, "Response msg config error.\n");
        return -1;
    }

    // Get results data
    KnImgTransAck ack_msg(CMD_ACK_NACK, 8, 0, CMD_SFID_DB_CONFIG);
    if(ack_msg.is_msg_valid() == false) {
        KnPrint(KN_LOG_ERR, "creating ack msg failed.\n");
        return -1;
    }

    uint32_t r_len = (uint32_t)pUsb->receive_bulk((char *) db_config_parameter, data_size);

    if(r_len < 0 || r_len != data_size) {
        KnPrint(KN_LOG_ERR, "receiving sfid db configuration failed: %d.\n", r_len);
        return -1;
    }
    
    return 0;
}

int SyncMsgHandler::kdp_sfid_get_db_index(int dev_idx, uint32_t* db_idx, uint32_t* rsp_code)
{
    uint16_t msg_type = CMD_SFID_DB_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t);

    //create req message
    kapp_db_config_parameter_t db_config_parameter; // empty kapp_db_config_parameter_t for fitting request format 
    uint32_t mode = DB_GET_DB_INDEX;
    KnSFIDGetDbIndexReq* p_req = new KnSFIDGetDbIndexReq(msg_type, msg_len, mode, &db_config_parameter);

    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid get db index req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid  get db index to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid get db index request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid get db index req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid get db index rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid get db index response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid get db index rsp from device received.\n");

    //extract msg content
    KnSFIDGetDbConfigRsp* sfid_msg = (KnSFIDGetDbConfigRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    *db_idx = sfid_msg->_data_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, db_idx: %d from sfid get db index rsp.\n", 
              *rsp_code, *db_idx);

    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_switch_db_index(int dev_idx, uint32_t db_idx, uint32_t* rsp_code)
{
    uint16_t msg_type = CMD_SFID_DB_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t);
    kapp_db_config_parameter_t db_config_parameter;

    memset(&db_config_parameter, 0, sizeof(kapp_db_config_parameter_t));
    db_config_parameter.uint32_value = db_idx;

    //create req message
    uint32_t mode = DB_SWITCH_DB_INDEX;
    KnSFIDSwitchDbIndexReq* p_req = new KnSFIDSwitchDbIndexReq(msg_type, msg_len, mode, &db_config_parameter);

    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid switch db req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid switch db to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid switch db request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid switch db req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid switch db rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid switch db response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid switch db rsp from device received.\n");

    //extract msg content
    KnSFIDSwitchDbIndexRsp* sfid_msg = (KnSFIDSwitchDbIndexRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from sfid switch db rsp.\n", sfid_msg->_rsp_code);
    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_set_db_version(int dev_idx, uint32_t db_version, uint32_t* rsp_code)
{
    uint16_t msg_type = CMD_SFID_DB_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t);
    kapp_db_config_parameter_t db_config_parameter;

    memset(&db_config_parameter, 0, sizeof(kapp_db_config_parameter_t));
    db_config_parameter.uint32_value = db_version;

    //create req message
    uint32_t mode = DB_SET_VERSION;
    KnSFIDSetDbVersionReq* p_req = new KnSFIDSetDbVersionReq(msg_type, msg_len, mode, &db_config_parameter);

    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid set db version req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid set db version to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid set db version request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid set db version req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid set db version rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid set db version response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid set db version rsp from device received.\n");

    //extract msg content
    KnSFIDSetDbVersionRsp* sfid_msg = (KnSFIDSetDbVersionRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from sfid set db version rsp.\n", sfid_msg->_rsp_code);
    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_get_db_version(int dev_idx, uint32_t* db_version, uint32_t* rsp_code)
{
    uint16_t msg_type = CMD_SFID_DB_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t);

    //create req message
    kapp_db_config_parameter_t db_config_parameter; // empty kapp_db_config_parameter_t for fitting request format 
    uint32_t mode = DB_GET_VERSION;
    KnSFIDGetDbVersionReq* p_req = new KnSFIDGetDbVersionReq(msg_type, msg_len, mode, &db_config_parameter);

    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid get db version req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid  get db version to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid get db version request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid get db version req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid get db version rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid get db version response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid get db version rsp from device received.\n");

    //extract msg content
    KnSFIDGetDbVersionRsp* sfid_msg = (KnSFIDGetDbVersionRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    *db_version = sfid_msg->_data_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, db_version:%d from sfid get db version rsp.\n", 
              *rsp_code, *db_version);

    //free resposne memory
    delete p_rsp;
    return 0;
}

int SyncMsgHandler::kdp_sfid_get_db_meta_data_version(int dev_idx, uint32_t* db_meta_data_version, uint32_t* rsp_code)
{
    uint16_t msg_type = CMD_SFID_DB_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t);

    //create req message
    kapp_db_config_parameter_t db_config_parameter; // empty kapp_db_config_parameter_t for fitting request format 
    uint32_t mode = DB_GET_META_DATA_VERSION;
    KnSFIDGetDbMetaDataVersionReq* p_req = new KnSFIDGetDbMetaDataVersionReq(msg_type, msg_len, mode, &db_config_parameter);

    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create sfid get db meta data version req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending sfid  get db meta data version to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending sfid get db meta data version request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving sfid get db meta data version req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sfid get db meta data version rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 100);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "sfid get db meta data version response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "sfid get db meta data version rsp from device received.\n");

    //extract msg content
    KnSFIDGetDbMetaDataVersionRsp* sfid_msg = (KnSFIDGetDbMetaDataVersionRsp*)p_rsp;
    *rsp_code = sfid_msg->_rsp_code;
    *db_meta_data_version = sfid_msg->_data_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, db_meta_data_version:%d from sfid get db meta data version rsp.\n", 
              *rsp_code, *db_meta_data_version);

    //free resposne memory
    delete p_rsp;
    return 0;
}

void SyncMsgHandler::save_req_to_map(uint16_t req_type, int idx) {
    _map_mtx[idx].lock();

    map<uint16_t, KnBaseMsg*>::iterator iter = _runing_map[idx].find(req_type);
    if(iter != _runing_map[idx].end()) {
        KnBaseMsg* p = iter->second;
        iter->second = NULL;
        _map_mtx[idx].unlock();

        KnPrint(KN_LOG_DBG, "req msg %d exist in map for dev:%d.\n", req_type, idx);
        if(p) delete p; //free the old one
    } else {
        _runing_map[idx][req_type] = NULL;
        _map_mtx[idx].unlock();
    }
}

int SyncMsgHandler::msg_received_rx_queue(KnBaseMsg* p_rsp, uint16_t req_type, int idx) {
    if(idx < 0 || idx >= MAX_COM_DEV) {
        KnPrint(KN_LOG_ERR, "invalid dev idx received from rx queue:%d.\n", idx);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "msg 0x%x received from rx queue for dev:%d.\n", req_type, idx);
    map<uint16_t, KnBaseMsg*>::iterator iter;

    _map_mtx[idx].lock();
    iter = _runing_map[idx].find(req_type);

    if(iter == _runing_map[idx].end()) {
        _map_mtx[idx].unlock();

        if(req_type == CMD_ACK_NACK) { //generic ack
            //_uart_mtx.lock();
            //_uart_wait.notify_all();
            //_uart_mtx.unlock();
            KnTsPrint(KN_LOG_DBG, "uart ack msg received.\n");
        } else {
            KnTsPrint(KN_LOG_DBG, "no req msg is waiting for this rsp:0x%x.\n", req_type);
        }
        delete p_rsp; //ignore it;
        return 0;
    } else {
        KnBaseMsg* p_old = iter->second;
        iter->second = p_rsp;
        _map_mtx[idx].unlock();

        if(p_old) {
            KnPrint(KN_LOG_DBG, "msg %d exist in map for dev:%d.\n", req_type, idx);
            delete p_old; //free the msg
        }
    }

    KnTsPrint(KN_LOG_DBG, "sync msg handler notifying wait thread ........\n");
    //notify
    _cond_mtx.lock();
    _cond_wait.notify_all();
    _cond_mtx.unlock();

    return 0;
}

int SyncMsgHandler::kdp_reset_sys_sync(int dev_idx, uint32_t reset_mode) {
    uint16_t msg_type = CMD_RESET;
    uint16_t msg_len = sizeof(uint32_t) * 2;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    if(pDev->dev_type == KDP_USB_DEV) {
        if(reset_mode == 3 || reset_mode == 4) { // for usb dev, 3 and 4 are not supported
            KnPrint(KN_LOG_ERR, "reset mode :%d is not supported by usb dev:%d.\n", 
                    reset_mode, dev_idx);
            return 1; //undefined reset control mode
        }
    }

    //create req message
    KnCmdResetReq* p_req = new KnCmdResetReq(msg_type, msg_len, reset_mode, ~reset_mode);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create reset sys req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending reset sys req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending reset sys request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving reset sys req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for sys reset rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "system reset response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "system reset rsp from device received.\n");

    //extract msg content
    KnCmdResetRsp* p_msg = (KnCmdResetRsp*)p_rsp;
    uint32_t rsp_code = p_msg->_err_code;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d in reset system rsp.\n", rsp_code);
    //free resposne memory
    delete p_rsp;

    if(rsp_code == 0) {
        if(reset_mode == 255) { //system reset
            KnTsPrint(KN_LOG_MSG, "system is doing reset...\n");

            //need wait and re-connect usb
            rsp_code = wait_dev_restart(dev_idx);

        } else if(reset_mode == 256) { //shutdown
            KnTsPrint(KN_LOG_MSG, "shutting down dev...\n");
            pDev->shutdown_dev();
        }
    }

    return rsp_code;
}

int SyncMsgHandler::kdp_report_sys_status_sync(int dev_idx, uint32_t* sfw_id, uint32_t* sbuild_id,
        uint16_t* sys_status, uint16_t* app_status, uint32_t* nfw_id, uint32_t* nbuild_id) {
    uint16_t msg_type = CMD_SYSTEM_STATUS;
    uint16_t msg_len = 0;

    //create req message
    KnReportSysStsReq* p_req = new KnReportSysStsReq(msg_type, msg_len);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create report sys req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending report sys req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending report sys request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving report sys req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for report sys rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "report sys response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "report system rsp from device received.\n");

    //extract msg content
    KnCmdSysStsRsp* p_msg = (KnCmdSysStsRsp*)p_rsp;

    *sfw_id = p_msg->_sfw_id;
    *sbuild_id = p_msg->_sbuild_id;
    *sys_status = p_msg->_sys_sts;
    *app_status = p_msg->_app_sts;
    *nfw_id = p_msg->_nfw_id;
    *nbuild_id = p_msg->_nbuild_id;

    KnTsPrint(KN_LOG_DBG, "report system response:%d,%u,%d,%d,%d,%u.\n", *sfw_id, *sbuild_id, *sys_status,         *app_status, *nfw_id, *nbuild_id);
    //free resposne memory
    delete p_rsp;

    return 0;
}

int SyncMsgHandler::kdp_get_kn_number(int dev_idx, uint32_t *kn_num) {
    uint16_t msg_type = CMD_GET_KN_NUM;
    uint16_t msg_len = 0;

    //create req message
    KnReportSysStsReq* p_req = new KnReportSysStsReq(msg_type, msg_len);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create get KN-number req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending CMD_GET_KN_NUM to dev\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending CMD_GET_KN_NUM to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving CMD_GET_KN_NUM to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for CMD_GET_KN_NUM from device ...\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "CMD_GET_KN_NUM response was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "CMD_GET_KN_NUM rsp from device received.\n");

    KnCmdGetKnNumRsp *p_msg = (KnCmdGetKnNumRsp *)p_rsp;

    *kn_num = p_msg->kn_num;

    KnTsPrint(KN_LOG_DBG, "CMD_GET_KN_NUM: %u\n", *kn_num);
    delete p_rsp;

    return 0;
}

int SyncMsgHandler::kdp_get_model_info(int dev_idx, int from_ddr, char *data_buf) {
    uint16_t msg_type = CMD_GET_MODEL_INFO;
    uint16_t msg_len = sizeof(uint32_t);
    int data_size = 0;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!data_buf) {
        KnPrint(KN_LOG_ERR, "model info is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    KnCmdGetModelInfoReq* p_req = new KnCmdGetModelInfoReq(msg_type, msg_len, from_ddr);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create get model info req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending CMD_GET_MODEL_INFO to dev\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending CMD_GET_MODEL_INFO to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving CMD_GET_MODEL_INFO to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for CMD_GET_MODEL_INFO from device ...\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "CMD_GET_MODEL_INFO response was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "CMD_GET_MODEL_INFO rsp from device received.\n");

    KnCmdGetModelInfoRsp *p_msg = (KnCmdGetModelInfoRsp *)p_rsp;

    data_size = p_msg->data_size;

    KnTsPrint(KN_LOG_DBG, "CMD_GET_MODEL_INFO: %u\n", data_size);
    delete p_rsp;

    if(data_size == 0 || data_size == 0xFFFF) {
        ((uint32_t *)data_buf)[0] = data_size;
        return 0;
    }

    // Get results data
    KnImgTransAck ack_msg(CMD_ACK_NACK, 8, 0, CMD_GET_MODEL_INFO);
    if(ack_msg.is_msg_valid() == false) {
        KnPrint(KN_LOG_ERR, "creating ack msg failed.\n");
        return -1;
    }

    int r_len = pUsb->receive_bulk(data_buf, data_size);

    if(r_len < 0 || r_len != data_size) {
        KnPrint(KN_LOG_ERR, "receiving model info failed: %d.\n", r_len);
        return -1;
    }

    return 0;
}

int SyncMsgHandler::kdp_set_ckey(int dev_idx, uint32_t ckey, uint32_t *set_status) {
    uint16_t msg_type = CMD_SET_CKEY;
    uint16_t msg_len = sizeof(uint32_t);

    //create req message
    KnCmdSetCkeyReq* p_req = new KnCmdSetCkeyReq(msg_type, msg_len, ckey);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create set ckey req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending CMD_SET_CKEY to dev\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending CMD_SET_CKEY to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving CMD_SET_CKEY to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for CMD_SET_CKEY from device ...\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "CMD_SET_CKEY response was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "CMD_SET_CKEY rsp from device received.\n");

    KnCmdSetCkeyRsp *p_msg = (KnCmdSetCkeyRsp *)p_rsp;

    *set_status = p_msg->set_status;

    KnTsPrint(KN_LOG_DBG, "CMD_SET_CKEY: 0x%08X\n", *set_status);
    delete p_rsp;

    return 0;
}

int SyncMsgHandler::kdp_set_sbt_key(int dev_idx, uint32_t entry, uint32_t key, uint32_t *set_status) {
    uint16_t msg_type = CMD_SET_SBT_KEY;
    uint16_t msg_len = sizeof(uint32_t) * 2;

    //create req message
    KnCmdSetSbtKeyReq* p_req = new KnCmdSetSbtKeyReq(msg_type, msg_len, entry, key);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create set sbt key req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending CMD_SET_SBT_KEY to dev\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending CMD_SET_SBT_KEY to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving CMD_SET_SBT_KEY to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for CMD_SET_SBT_KEY from device ...\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "CMD_SET_SBT_KEY response was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "CMD_SET_SBT_KEY rsp from device received.\n");

    KnCmdSetSbtKeyRsp *p_msg = (KnCmdSetSbtKeyRsp *)p_rsp;

    *set_status = p_msg->set_status;

    KnTsPrint(KN_LOG_DBG, "CMD_SET_SBT_KEY: 0x%08X\n", *set_status);
    delete p_rsp;

    return 0;
}

int SyncMsgHandler::kdp_get_crc(int dev_idx, int from_ddr, char *data_buf) {
    uint16_t msg_type = CMD_GET_CRC;
    uint16_t msg_len = sizeof(uint32_t);
    int data_size = 0;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!data_buf) {
        KnPrint(KN_LOG_ERR, "crc is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    KnCmdGetCrcReq* p_req = new KnCmdGetCrcReq(msg_type, msg_len, from_ddr);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create get crc req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending CMD_GET_CRC to dev\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending CMD_GET_CRC to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving CMD_GET_CRC to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for CMD_GET_CRC from device ...\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "CMD_GET_CRC response was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "CMD_GET_CRC rsp from device received.\n");

    KnCmdGetCrcRsp *p_msg = (KnCmdGetCrcRsp *)p_rsp;

    data_size = p_msg->data_size;

    KnTsPrint(KN_LOG_DBG, "CMD_GET_CRC: %u\n", data_size);
    delete p_rsp;

    if(data_size == 0 || data_size == 0xFFFF) {
        ((uint32_t *)data_buf)[0] = data_size;
        return 0;
    }

    // Get results data
    KnImgTransAck ack_msg(CMD_ACK_NACK, 8, 0, CMD_GET_CRC);
    if(ack_msg.is_msg_valid() == false) {
        KnPrint(KN_LOG_ERR, "creating ack msg failed.\n");
        return -1;
    }

    int r_len = pUsb->receive_bulk(data_buf, data_size);

    if(r_len < 0 || r_len != data_size) {
        KnPrint(KN_LOG_ERR, "receiving crc failed: %d.\n", r_len);
        return -1;
    }

    return 0;
}

int SyncMsgHandler::kdp_update_fw_sync(int dev_idx, uint32_t* module_id, char* img_buf, int buf_len) {
    uint16_t msg_type = CMD_UPDATE_FW;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(uint32_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if (*module_id != UPDATE_MODULE_SCPU && *module_id != UPDATE_MODULE_NCPU) {
        KnPrint(KN_LOG_ERR, "invalid module id: %d (dev_idx %d).\n", *module_id, dev_idx);
        return -1;
    }

    //create req message
    KnCmdUdtFmReq* p_req = new KnCmdUdtFmReq(msg_type, msg_len, *module_id);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create update firmware req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending update firmware request to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending update firmware request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving update firmware req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(img_buf, buf_len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return snd_len;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for update firmware rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 250);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "update firmware response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "update firmware rsp from device received.\n");

    //extract msg content
    KnCmdUdtFmRsp* rsp_msg = (KnCmdUdtFmRsp*)p_rsp;
    uint32_t rsp_code = rsp_msg->_rsp_code;
    uint32_t ret_id  = rsp_msg->_module_id;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, ret_id:%d.\n", rsp_code, ret_id);

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0) { //error
        KnTsPrint(KN_LOG_ERR, "udt firmware response code:%d.\n", rsp_code);
        return rsp_code;
    }

    //need wait and re-connect usb
    wait_dev_restart(dev_idx);

    if(ret_id != *module_id) {
        KnTsPrint(KN_LOG_DBG, "confirmation of module id is incorrect:%d.\n", ret_id);
        return -1;
    }

    return rsp_code;
}

int SyncMsgHandler::kdp_update_nef_model_sync(int dev_idx, uint32_t info_size, \
        char* img_buf, int buf_len) {
    uint16_t msg_type = CMD_UPDATE_MODEL;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(uint32_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(info_size == 0 || buf_len == 0) { //if no operation
        return 0;
    }

    uint32_t all_model_size = buf_len - info_size;
    info_size <<= 16;

    //create req message
    KnCmdUdtModelReq* p_req = new KnCmdUdtModelReq(msg_type, msg_len, info_size, all_model_size);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create update nef model req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending update nef model request to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending update nef model request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving update nef model req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(img_buf, buf_len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return snd_len;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for update nef model rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 3000); // 10 min
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "update nef model response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "update nef model rsp from device received.\n");

    //extract msg content
    KnCmdUdtModelRsp* rsp_msg = (KnCmdUdtModelRsp*)p_rsp;
    uint32_t rsp_code = rsp_msg->_rsp_code;
    uint32_t ret_id  = rsp_msg->_model_id;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, ret_id:%d.\n", rsp_code, ret_id);

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0) { //error
        KnTsPrint(KN_LOG_ERR, "udt nef model response code:%d.\n", rsp_code);
        return rsp_code;
    }

    //need wait and re-connect usb
    wait_dev_restart(dev_idx);

    return rsp_code;
}

int SyncMsgHandler::kdp_update_model_sync(int dev_idx, uint32_t* model_id, uint32_t model_size, \
        char* img_buf, int buf_len) {
    uint16_t msg_type = CMD_UPDATE_MODEL;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(uint32_t);

    *model_id = 1; // for future use

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(*model_id == 0) { //if no operation
        return 0;
    }

    //create req message
    KnCmdUdtModelReq* p_req = new KnCmdUdtModelReq(msg_type, msg_len, *model_id, model_size);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create update model req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending update model request to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending update model request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving update model req to local map.\n");
    save_req_to_map(msg_type, dev_idx);
   
    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(img_buf, buf_len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return snd_len;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for update model rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 3000); // 10 min
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "update model response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "update model rsp from device received.\n");

    //extract msg content
    KnCmdUdtModelRsp* rsp_msg = (KnCmdUdtModelRsp*)p_rsp;
    uint32_t rsp_code = rsp_msg->_rsp_code;
    uint32_t ret_id  = rsp_msg->_model_id;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, ret_id:%d.\n", rsp_code, ret_id);

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0) { //error
        KnTsPrint(KN_LOG_ERR, "udt model response code:%d.\n", rsp_code);
        return rsp_code;
    }

    //need wait and re-connect usb
    wait_dev_restart(dev_idx);

    return rsp_code;
}

int SyncMsgHandler::kdp_update_spl_sync(int dev_idx, uint32_t mode, uint16_t auth_type, \
    uint16_t size, uint8_t* auth_key, char* spl_buf, uint32_t* rsp_code, \
    uint32_t* spl_id, uint32_t* spl_build) {
    uint16_t msg_type = CMD_RESERVED;
    uint16_t msg_len;
    uint16_t spl_size = size;
    if (auth_type > 1) {
        KnPrint(KN_LOG_ERR, "unknown auth type: %d.\n", auth_type);
        return -1;
    }
    else {
        msg_len = sizeof(uint32_t) + sizeof(uint16_t)*2 + 4;
        if (mode == 0) {
            spl_size = 0;
        }
        else {
            if (mode > 1) {
                KnPrint(KN_LOG_ERR, "unknown mode: %d.\n", mode);
                return -1;
            }
        }
    }

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    //create req message
    KnCmdUdtSplReq* p_req = new KnCmdUdtSplReq(msg_type, msg_len, mode, auth_type, spl_size, auth_key);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create update spl req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending update spl request to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending update spl request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving update model req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    if (mode == 1) {
        //to call usb interface to transfer image file.
        KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
        int snd_len = pUsb->send_image_data(spl_buf, size);
        if(snd_len < 0) {
            KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
            return snd_len;
        }
        KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);
    }

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for update model rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 500);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "update spl response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "update spl rsp from device received.\n");

    //extract msg content
    KnCmdUdtSplRsp* rsp_msg = (KnCmdUdtSplRsp*)p_rsp;
    *rsp_code = rsp_msg->_rsp_code;
    *spl_id = rsp_msg->_spl_id;
    *spl_build = rsp_msg->_spl_build;


    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, spl_id:%d, spl_build: %d.\n", rsp_code, spl_id, spl_build);

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0) { //error
        KnTsPrint(KN_LOG_ERR, "udt spl response code:%d.\n", rsp_code);
        return *rsp_code;
    }

    return *rsp_code;
}

int SyncMsgHandler::kdp_isi_start(int dev_idx, uint32_t app_id, uint32_t rt_size, uint16_t width, uint16_t height, uint32_t format, uint32_t* rsp_code, uint32_t* buf_size) {
    uint16_t msg_type = CMD_ISI_START;
    uint16_t msg_len = sizeof(uint32_t)*3 + sizeof(uint16_t)*2;

    //create req message
    KnIsiStartReq* p_req = new KnIsiStartReq(msg_type, msg_len, app_id, rt_size, width, height, format);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create isi start req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending isi start to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending isi start request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving isi start req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for isi start rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "isi start response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "isi start rsp from device received.\n");

    //extract msg content
    KnIsiStartRsp* isi_msg = (KnIsiStartRsp*)p_rsp;
    *rsp_code = isi_msg->_rsp_code;
    *buf_size = isi_msg->_buf_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, buffer size:%d from isi start rsp.\n", *rsp_code, *buf_size);
    //free resposne memory
    delete p_rsp;
    if (*rsp_code != 0)
        return -1;
    return 0;
}

int SyncMsgHandler::kdp_isi_start_ext(int dev_idx, char* isi_cfg, int cfg_size, uint32_t* rsp_code, uint32_t* buf_size) {
    uint16_t msg_type = CMD_ISI_START;
    uint16_t msg_len = cfg_size;

    //create req message
    KnIsiStartExtReq* p_req = new KnIsiStartExtReq(msg_type, msg_len, isi_cfg);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create isi start req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending isi start to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending isi start request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving isi start req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for isi start rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "isi start response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "isi start rsp from device received.\n");

    //extract msg content
    KnIsiStartRsp* isi_msg = (KnIsiStartRsp*)p_rsp;
    *rsp_code = isi_msg->_rsp_code;
    *buf_size = isi_msg->_buf_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, buffer size:%d from isi start rsp.\n", *rsp_code, *buf_size);
    //free resposne memory
    delete p_rsp;
    if (*rsp_code != 0)
        return -1;
    return 0;
}

int SyncMsgHandler::kdp_isi_inference(int dev_idx, char* img_buf, int buf_len, uint32_t img_id, uint32_t* rsp_code, uint32_t* img_available) {
    uint16_t msg_type = CMD_ISI_SEND_IMAGE;
    uint16_t msg_len = sizeof(uint32_t)*3;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    //create req message
    KnIsiImageReq* p_req = new KnIsiImageReq(msg_type, msg_len, buf_len, img_id);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create isi img req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending isi img req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending isi img request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving isi img req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(img_buf, buf_len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return snd_len;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for isi img rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "isi img response message was not received.\n");
        return -2;
    }
    KnTsPrint(KN_LOG_DBG, "isi img rsp from device received.\n");

    //extract msg content
    KnIsiImgRsp* isi_msg = (KnIsiImgRsp*)p_rsp;
    *rsp_code = isi_msg->_rsp_code;
    *img_available = isi_msg->_image_available;
    int return_code = isi_msg->_rsp_code;  // get int version of rsp_code
    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, available buffer size:%d from isi img rsp.\n",
            *rsp_code, *img_available);
    //free resposne memory
    delete p_rsp;

    if(*rsp_code != 0) {
        KnTsPrint(KN_LOG_DBG, "isi img rsp with response code:%d.\n", rsp_code);
        return return_code;
    }
    return 0;
}

int SyncMsgHandler::kdp_isi_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data) {
    uint16_t msg_type = CMD_ISI_GET_RESULTS;
    uint16_t msg_len = sizeof(uint32_t)*2;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!r_data) {
        KnPrint(KN_LOG_ERR, "isi inference result is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    KnIsiResultsReq* p_req = new KnIsiResultsReq(msg_type, msg_len, img_id);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create isi get results req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending isi get results req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending isi get results to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving isi get results req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for isi get results rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "isi get results message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "isi get results from device received.\n");

    //extract msg content, ignore img_id field
    KnIsiResultsRsp* isi_msg = (KnIsiResultsRsp*)p_rsp;
    *rsp_code = isi_msg->_rsp_code;
    *r_size = isi_msg->_r_size;
    int return_code = isi_msg->_rsp_code;  // get int version of rsp_code
    int inf_size = isi_msg->_r_size;  // get int version of inference data size

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, result size:%d from isi get results rsp.\n", *rsp_code, *r_size);
    //free resposne memory
    delete p_rsp;

    if(*rsp_code != 0) {
        KnTsPrint(KN_LOG_DBG, "isi get results rsp with response code:%d.\n", rsp_code);
        return return_code;
    }

    if(*r_size == 0) {
        return 0;
    }

    // Get results data
    KnImgTransAck ack_msg(CMD_ACK_NACK, 8, 0, CMD_ISI_GET_RESULTS);
    if(ack_msg.is_msg_valid() == false) {
        KnPrint(KN_LOG_ERR, "creating ack msg failed.\n");
        return -1;
    }

    int r_len = pUsb->receive_bulk(r_data, *r_size);

    if(r_len < 0 || r_len != inf_size) {
        KnPrint(KN_LOG_ERR, "receiving isi inference result failed: %d.\n", r_len);
        return -1;
    }
    return 0;
}

int SyncMsgHandler::kdp_isi_config(int dev_idx, uint32_t model_id, uint32_t param, uint32_t *rsp_code) {
    uint16_t msg_type = CMD_ISI_CONFIG;
    uint16_t msg_len = sizeof(uint32_t) * 3;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if (!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    //create req message
    KnIsiConfigReq* p_req = new KnIsiConfigReq(msg_type, msg_len, model_id, param);
    if (!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create isi config req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending isi config to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if (res < 0) {
        KnPrint(KN_LOG_ERR, "sending isi config request to device failed: %d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving isi config req to local map.\n");
    save_req_to_map(msg_type);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for isi config rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, 51); // 10200 (LIB_USB_TIMEOUT) / 200 (KDP_SYNC_WAIT_TIME) = 51
    if (!p_rsp) {
        KnPrint(KN_LOG_ERR, "isi config response message not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "isi config rsp from device received.\n");

    //extract msg content
    KnIsiConfigRsp* isi_msg = (KnIsiConfigRsp*)p_rsp;
    *rsp_code = isi_msg->_rsp_code;

    KnTsPrint(KN_LOG_DBG, "rsp_code: %d, from isi config rsp.\n", *rsp_code);
    //free resposne memory
    delete p_rsp;
    if (*rsp_code != 0)
        return -1;
    return 0;
}

int SyncMsgHandler::kdp_start_dme(int dev_idx, uint32_t model_size, char* data, int dat_size, \
    uint32_t* ret_size, char* img_buf, int buf_len) {

    uint16_t msg_type = CMD_DME_START;
    uint16_t msg_len = sizeof(uint32_t) + dat_size;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    //create req message
    KnStartDmeReq* p_req = new KnStartDmeReq(msg_type, msg_len, model_size, data, dat_size);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create start dme req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending start dme req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending start dme request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving start dme req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(img_buf, buf_len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return snd_len;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for start dme rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "start dme response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "start dme rsp from device received.\n");

    //extract msg content
    KnStartDmeRsp* dme_msg = (KnStartDmeRsp*)p_rsp;
    uint32_t rsp_code = dme_msg->_rsp_code;
    *ret_size = dme_msg->_inf_size;

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0) {
        KnTsPrint(KN_LOG_DBG, "start dme rsp with response code:%d.\n", rsp_code);
        return rsp_code;
    }

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, infer size:%d from start dme rsp.\n", 
            dme_msg->_rsp_code, dme_msg->_inf_size);
    return 0;
}

int SyncMsgHandler::kdp_dme_configure(int dev_idx, char* data, int dat_size, uint32_t* ret_model_id) {
    uint16_t msg_type = CMD_DME_CONFIG;
    uint16_t msg_len = dat_size;

    //create req message
    KnDmeConfigReq* p_req = new KnDmeConfigReq(msg_type, msg_len, data, dat_size);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create dme config req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending dme config req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending dme config request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving dme config req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for dme config rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "dme config response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "dme config rsp from device received.\n");

    //extract msg content
    KnDmeConfigRsp* dme_msg = (KnDmeConfigRsp*)p_rsp;
    uint32_t rsp_code = dme_msg->_rsp_code;
    *ret_model_id = dme_msg->_model_id;

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0) {
        KnTsPrint(KN_LOG_DBG, "dme config rsp with response code:%d.\n", rsp_code);
        return rsp_code;
    }

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from dme config rsp.\n", dme_msg->_rsp_code);
    return 0;
}

int SyncMsgHandler::kdp_dme_inference(int dev_idx, char* img_buf, int buf_len, uint32_t* inf_size,\
                bool* res_flag, char* inf_res, uint16_t mode, uint16_t model_id) {
    uint16_t msg_type = CMD_DME_SEND_IMAGE;
    uint16_t msg_len = sizeof(uint32_t) + sizeof(uint32_t);

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!res_flag) {
        KnPrint(KN_LOG_ERR, "inference result flag need to be specified.\n");
        return -1;
    }

    if(*res_flag == true && !inf_res) {
        KnPrint(KN_LOG_ERR, "inference result is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    KnDmeImageReq* p_req = new KnDmeImageReq(msg_type, msg_len, buf_len, mode, model_id);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create dme img req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending dme img req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending dme img request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving dme img req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data(img_buf, buf_len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return snd_len;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for dme img rsp from device ........\n");

    int wait_num = 0;
    if (mode) { // async
        wait_num = 2;
    } else { // normal
        wait_num = 5;
    }
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type, wait_num);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "dme img response message was not received.\n");
        return -2;
    }
    KnTsPrint(KN_LOG_DBG, "dme img rsp from device received.\n");

    //extract msg content
    KnDmeImageRsp* dme_msg = (KnDmeImageRsp*)p_rsp;
    uint32_t rsp_code = dme_msg->_rsp_code;
    *inf_size = dme_msg->_inf_size;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, infer size:%d from dme img rsp.\n", 
            dme_msg->_rsp_code, dme_msg->_inf_size);
    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0) {
        KnTsPrint(KN_LOG_DBG, "dme img rsp with response code:%d.\n", rsp_code);
        return rsp_code;
    }

    return 0;
}

int SyncMsgHandler::kdp_dme_retrieve_res(int dev_idx, uint32_t addr, int len, char* inf_res) {
    int snd_len;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev) {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!inf_res) {
        KnPrint(KN_LOG_ERR, "inference result is requested, but buffer is null.\n");
        return -1;
    }

    // Get raw info
    snd_len = pUsb->send_data_ack(addr, 0, len);
    if(snd_len < 0) {
        KnPrint(KN_LOG_ERR, "sending ack to device failed.\n");
        return snd_len;
    }

    int r_len = pUsb->receive_bulk(inf_res, len);

    if(r_len < 0 || r_len != len) {
        KnPrint(KN_LOG_ERR, "receiving inference result failed: %d.\n", r_len);
        return -1;
    }
    
    return 0;
}

int SyncMsgHandler::kdp_dme_get_status(int dev_idx, uint16_t *ssid, uint16_t *status, uint32_t* inf_size, char* inf_res) {
    uint16_t msg_type = CMD_DME_GET_STATUS;
    uint16_t msg_len = 8;

    //create req message
    KnDmeStatusReq* p_req = new KnDmeStatusReq(msg_type, msg_len, *ssid);
    if(!p_req->is_msg_valid()) {
        KnPrint(KN_LOG_ERR, "create dme get req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending dme get req to dev for ssid %d ........\n", *ssid);
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0) {
        KnPrint(KN_LOG_ERR, "sending dme get request to device failed:%d.\n", res);
        return res;
    }

    KnTsPrint(KN_LOG_DBG, "saving dme get req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for dme get rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp) {
        KnPrint(KN_LOG_ERR, "dme get response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "dme get rsp from device received.\n");

    //extract msg content
    KnDmeGetRsp* dme_msg = (KnDmeGetRsp*)p_rsp;
    uint32_t rsp_code = dme_msg->_rsp_code;
    
    if (0xf8 == rsp_code) {
        KnPrint(KN_LOG_ERR, "bad ssid.\n");
        return -1;
    }    
    
    *ssid = dme_msg->_session_id;   
    *status = dme_msg->_inf_status;
    if (*status == 1) {
        *inf_size = dme_msg->_inf_size;

    }
    if(rsp_code != 0) {
        KnTsPrint(KN_LOG_DBG, "dme get rsp with response code:%d.\n", rsp_code);
        return rsp_code;
    }

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, ssid %d, status %d, inf size:%d from dme get rsp.\n", 
            dme_msg->_rsp_code, dme_msg->_session_id, dme_msg->_inf_status, dme_msg->_inf_size);
    //free resposne memory
    delete p_rsp;

    return 0;
}

/**************  msg handler for jpeg enc *************/
int SyncMsgHandler::kdp_jpeg_enc_config(int dev_idx, int img_seq, int width, int height, int fmt, int quality)
{
    uint16_t msg_type = CMD_JPEG_ENC_CONFIG;
    uint16_t msg_len = sizeof(uint32_t)*5;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev)
    {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    //create req message
    KnJpegEncCfgReq* p_req = new KnJpegEncCfgReq(msg_type, msg_len, img_seq, width, height, fmt, quality);
    if(!p_req->is_msg_valid())
    {
        KnPrint(KN_LOG_ERR, "create KnJpegEncCfgReq failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending KnJpegEncCfgReq msg to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0)
    {
        KnPrint(KN_LOG_ERR, "sending jpeg enc config request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving KnJpegEncCfgReq msg to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for jpeg enc config rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp)
    {
        KnPrint(KN_LOG_ERR, "jpeg config response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "jpeg enc config rsp from device received.\n");

    //extract msg content
    KnJpegEncConfigRsp* jpg_msg = (KnJpegEncConfigRsp*)p_rsp;
    uint32_t rsp_code = jpg_msg->_rsp_code;
    if(jpg_msg->_img_seq != (uint32_t)img_seq)
    {
        KnPrint(KN_LOG_ERR, "jpeg_seq=%d config failed\n", img_seq);
        return -1;
    }

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0)
    {
        KnTsPrint(KN_LOG_DBG, "jpeg enc config rsp err code:%d.\n", rsp_code);
        return rsp_code;
    }

    KnTsPrint(KN_LOG_DBG, "jpeg enc config succeed with rsp_code:%d\n", jpg_msg->_rsp_code);

    return 0;
}

int SyncMsgHandler::kdp_jpeg_enc(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len)
{
    uint16_t msg_type = CMD_JPEG_ENC;
    uint16_t msg_len = sizeof(uint32_t)*2;    // img_seq and in_img_len
    int return_code;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev)
    {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    //create req message
    KnJpegEncStartReq* p_req = new KnJpegEncStartReq(msg_type, msg_len, img_seq, in_img_len);
    if(!p_req->is_msg_valid())
    {
        KnPrint(KN_LOG_ERR, "create KnJpegEncCfgReq failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending KnJpegEncStartReq msg to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0)
    {
        KnPrint(KN_LOG_ERR, "sending jpeg encoding request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving KnJpegEncStartReq msg to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data((char *)in_img_buf, in_img_len);
    if(snd_len < 0)
    {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return -2;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for jpeg enc rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp)
    {
        KnPrint(KN_LOG_ERR, "Jpeg enc response message was not received.\n");
        return -2;
    }
    KnTsPrint(KN_LOG_DBG, "Jpeg enc rsp from device received.\n");

    //extract msg content
    KnJpegEncStartRsp* jpg_enc_msg = (KnJpegEncStartRsp*)p_rsp;

    if(jpg_enc_msg->_img_seq != (uint32_t)img_seq)
    {
        KnPrint(KN_LOG_ERR, "return img_seq does not match: send-%d, rsp-%d\n", img_seq, jpg_enc_msg->_img_seq);
        return -1;
    }

    return_code = jpg_enc_msg->_rsp_code;
    *out_img_buf = jpg_enc_msg->_img_buf;
    *out_img_len = jpg_enc_msg->_img_len;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from jpeg enc rsp.\n",
              return_code);
    //free resposne memory
    delete p_rsp;

    if(return_code != 1)     //JPEG_OPERATION_SUCCESS = 1
    {
        KnTsPrint(KN_LOG_DBG, "jpeg enc failed with response code:%d.\n", return_code);
        return return_code;
    }

    return 0;
}

int SyncMsgHandler::kdp_jpeg_enc_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data)
{
    uint16_t msg_type = CMD_JPEG_ENC_GET_RESULT;
    uint16_t msg_len = sizeof(uint32_t)*3;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev)
    {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!r_data)
    {
        KnPrint(KN_LOG_ERR, "jpeg enc result is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    KnJpegEncResultsReq* p_req = new KnJpegEncResultsReq(msg_type, msg_len, img_id);
    if(!p_req->is_msg_valid())
    {
        KnPrint(KN_LOG_ERR, "create jpeg enc results req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending jpeg enc get results req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0)
    {
        KnPrint(KN_LOG_ERR, "sending jpeg enc get results to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving jpeg enc get results req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for jpeg enc get results rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp)
    {
        KnPrint(KN_LOG_ERR, "jpeg enc get results message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "jpeg enc get results from device received.\n");

    //extract msg content, ignore img_id field
    KnJpegEncResultsRsp* jpeg_enc_msg = (KnJpegEncResultsRsp*)p_rsp;
    *rsp_code = jpeg_enc_msg->_rsp_code;
    *r_size = jpeg_enc_msg->_r_size;
    int return_code = jpeg_enc_msg->_rsp_code;  // get int version of rsp_code
    int inf_size = jpeg_enc_msg->_r_size;  // get int version of jpeg enc data size

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, result size:%d from jpeg enc get results rsp.\n", *rsp_code, *r_size);
    //free resposne memory
    delete p_rsp;

    if(*rsp_code != 1)   //JPEG_OPERATION_SUCCESS = 1
    {
        KnTsPrint(KN_LOG_DBG, "jpeg enc get results rsp with response code:%d.\n", rsp_code);
        return return_code;
    }

    if(*r_size == 0)
    {
        return 0;
    }

    // Get results data
    KnImgTransAck ack_msg(CMD_ACK_NACK, 8, 0, CMD_JPEG_ENC_GET_RESULT);
    if(ack_msg.is_msg_valid() == false)
    {
        KnPrint(KN_LOG_ERR, "creating ack msg failed.\n");
        return -1;
    }

    int r_len = pUsb->receive_bulk(r_data, *r_size);

    if(r_len < 0 || r_len != inf_size)
    {
        KnPrint(KN_LOG_ERR, "receiving jpeg enc result failed: %d.\n", r_len);
        return -1;
    }
    return 0;
}

/******************** msg handler for jpeg dec *********************/

int SyncMsgHandler::kdp_jpeg_dec_config(int dev_idx, int img_seq, int width, int height, int fmt, int len)
{
    uint16_t msg_type = CMD_JPEG_DEC_CONFIG;
    uint16_t msg_len = sizeof(uint32_t)*5;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev)
    {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    //create req message
    KnJpegDecCfgReq* p_req = new KnJpegDecCfgReq(msg_type, msg_len, img_seq, width, height, fmt, len);
    if(!p_req->is_msg_valid())
    {
        KnPrint(KN_LOG_ERR, "create KnJpegDecCfgReq failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending KnJpegDecCfgReq msg to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0)
    {
        KnPrint(KN_LOG_ERR, "sending jpeg dec config request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving KnJpegEncCfgReq msg to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for jpeg dec config rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp)
    {
        KnPrint(KN_LOG_ERR, "jpeg dec config response message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "jpeg dec config rsp from device received.\n");

    //extract msg content
    KnJpegDecConfigRsp* jpg_msg = (KnJpegDecConfigRsp*)p_rsp;
    uint32_t rsp_code = jpg_msg->_rsp_code;
    if(jpg_msg->_img_seq != (uint32_t)img_seq)
    {
        KnPrint(KN_LOG_ERR, "jpeg_seq=%d config failed\n", img_seq);
        return -1;
    }

    //free resposne memory
    delete p_rsp;

    if(rsp_code != 0)
    {
        KnTsPrint(KN_LOG_DBG, "jpeg dec config rsp err code:%d.\n", rsp_code);
        return rsp_code;
    }

    KnTsPrint(KN_LOG_DBG, "jpeg dec config succeed with rsp_code:%d\n", jpg_msg->_rsp_code);

    return 0;
}

int SyncMsgHandler::kdp_jpeg_dec(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len)
{
    uint16_t msg_type = CMD_JPEG_DEC;
    uint16_t msg_len = sizeof(uint32_t)*2;    // img_seq and in_img_len
    int return_code;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev)
    {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    //create req message
    KnJpegDecStartReq* p_req = new KnJpegDecStartReq(msg_type, msg_len, img_seq, in_img_len);
    if(!p_req->is_msg_valid())
    {
        KnPrint(KN_LOG_ERR, "create KnJpegDecCfgReq failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending KnJpegDecStartReq msg to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0)
    {
        KnPrint(KN_LOG_ERR, "sending jpeg decoding request to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving KnJpegDecStartReq msg to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //to call usb interface to transfer image file.
    KnTsPrint(KN_LOG_DBG, "calling usb interface to transfer file:........\n");
    int snd_len = pUsb->send_image_data((char *)in_img_buf, in_img_len);
    if(snd_len < 0)
    {
        KnPrint(KN_LOG_ERR, "receving transfer ack msg failed.\n");
        return -2;
    }
    KnTsPrint(KN_LOG_DBG, "file transfer completed :%d.\n", snd_len);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for jpeg dec rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp)
    {
        KnPrint(KN_LOG_ERR, "Jpeg dec response message was not received.\n");
        return -2;
    }
    KnTsPrint(KN_LOG_DBG, "Jpeg dec rsp from device received.\n");

    //extract msg content
    KnJpegDecStartRsp* jpg_dec_msg = (KnJpegDecStartRsp*)p_rsp;

    if(jpg_dec_msg->_img_seq != (uint32_t)img_seq)
    {
        KnPrint(KN_LOG_ERR, "return img_seq does not match: send-%d, rsp-%d\n", img_seq, jpg_dec_msg->_img_seq);
        return -1;
    }

    return_code = jpg_dec_msg->_rsp_code;
    *out_img_buf = jpg_dec_msg->_img_buf;
    *out_img_len = jpg_dec_msg->_img_len;

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d from jpeg dec rsp.\n",
              return_code);
    //free resposne memory
    delete p_rsp;

    if(return_code != 1)     //JPEG_OPERATION_SUCCESS = 1
    {
        KnTsPrint(KN_LOG_DBG, "jpeg dec failed with response code:%d.\n", return_code);
        return return_code;
    }

    return 0;
}

int SyncMsgHandler::kdp_jpeg_dec_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data)
{
    uint16_t msg_type = CMD_JPEG_DEC_GET_RESULT;
    uint16_t msg_len = sizeof(uint32_t)*3;

    BaseDev* pDev = _p_mgr->getDevFromIdx(dev_idx);
    if(!pDev)
    {
        KnPrint(KN_LOG_ERR, "could not get device for idx:%d.\n", dev_idx);
        return -1;
    }

    USBDev* pUsb = (USBDev*)pDev;

    if(!r_data)
    {
        KnPrint(KN_LOG_ERR, "jpeg dec result is requested, but buffer is null.\n");
        return -1;
    }

    //create req message
    KnJpegDecResultsReq* p_req = new KnJpegDecResultsReq(msg_type, msg_len, img_id);
    if(!p_req->is_msg_valid())
    {
        KnPrint(KN_LOG_ERR, "create jpeg dec results req failed.\n");
        delete p_req;
        return -1;
    }

    //sending msg to queue
    KnTsPrint(KN_LOG_DBG, "sending jpeg dec get results req to dev ........\n");
    int res = _p_mgr->send_cmd_msg_sync(dev_idx, p_req);
    if(res < 0)
    {
        KnPrint(KN_LOG_ERR, "sending jpeg dec get results to device failed:%d.\n", res);
        return -1;
    }

    KnTsPrint(KN_LOG_DBG, "saving jpeg dec get results req to local map.\n");
    save_req_to_map(msg_type, dev_idx);

    //wait for rsp
    KnTsPrint(KN_LOG_DBG, "waiting for jpeg dec get results rsp from device ........\n");
    KnBaseMsg* p_rsp = wait_rsp_msg(dev_idx, msg_type);
    if(!p_rsp)
    {
        KnPrint(KN_LOG_ERR, "jpeg dec get results message was not received.\n");
        return -1;
    }
    KnTsPrint(KN_LOG_DBG, "jpeg dec get results from device received.\n");

    //extract msg content, ignore img_id field
    KnJpegDecResultsRsp* jpeg_dec_msg = (KnJpegDecResultsRsp*)p_rsp;
    *rsp_code = jpeg_dec_msg->_rsp_code;
    *r_size = jpeg_dec_msg->_r_size;
    int return_code = jpeg_dec_msg->_rsp_code;  // get int version of rsp_code
    int inf_size = jpeg_dec_msg->_r_size;  // get int version of jpeg enc data size

    KnTsPrint(KN_LOG_DBG, "rsp_code:%d, result size:%d from jpeg dec get results rsp.\n", *rsp_code, *r_size);
    //free resposne memory
    delete p_rsp;

    if(*rsp_code != 1)   //JPEG_OPERATION_SUCCESS = 1
    {
        KnTsPrint(KN_LOG_DBG, "jpeg dec get results rsp with response code:%d.\n", rsp_code);
        return return_code;
    }

    if(*r_size == 0)
    {
        return 0;
    }

    // Get results data
    KnImgTransAck ack_msg(CMD_ACK_NACK, 8, 0, CMD_JPEG_DEC_GET_RESULT);
    if(ack_msg.is_msg_valid() == false)
    {
        KnPrint(KN_LOG_ERR, "creating ack msg failed.\n");
        return -1;
    }

    int r_len = pUsb->receive_bulk(r_data, *r_size);

    if(r_len < 0 || r_len != inf_size)
    {
        KnPrint(KN_LOG_ERR, "receiving isi inference result failed: %d.\n", r_len);
        return -1;
    }
    return 0;
}

