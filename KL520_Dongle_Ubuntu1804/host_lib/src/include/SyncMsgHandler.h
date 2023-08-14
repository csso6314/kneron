/**
 * @file      SyncMsgHandler.h
 * @brief     msg handler header file for sync messages 
 * @version   0.1 - 2019-04-23
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __SYNC_MSG_HANDLER_H__
#define __SYNC_MSG_HANDLER_H__

#define KDP_SYNC_WAIT_TIME      200
#define KDP_SYNC_WAIT_NUM       25 //5 seconds
#define KDP_UART_ACK_TIME       1000*5 //5 sec

#define KDP_RESET_NUM           5 //5 times
#define KDP_RESET_TIME          2000000 //2 sec

#include "KdpHostApi.h"
#include "KnMsg.h"
#include "UARTDev.h"
#include "USBDev.h"

#include <stdint.h>


#include <map>
#include <vector>

#if defined(THREAD_USE_CPP11)
#include <mutex>
#include <condition_variable>
#endif

using namespace std;

class SyncMsgHandler {
public:
    SyncMsgHandler();
    virtual ~SyncMsgHandler();

    int kdp_sfid_start(int dev_idx, uint32_t* rsp_code, uint32_t* img_size, float thresh,
                    uint16_t width, uint16_t height, uint32_t format);
    int kdp_sfid_new_user(int dev_idx, uint16_t usr_id, uint16_t img_idx);
    int kdp_sfid_register(int dev_idx, uint32_t usr_id, uint32_t* rsp_code);
    int kdp_sfid_del_db(int dev_idx, uint32_t usr_id, uint32_t* rsp_code);
    int kdp_sfid_edit_db(int dev_idx, uint32_t mode, char** p_buf, uint32_t *p_len, uint32_t* rsp_code);	
    int kdp_sfid_import(int dev_idx, char* p_buf, uint32_t p_len, uint32_t* rsp_code);	
	int kdp_sfid_export(int dev_idx, char** p_buf, uint32_t *p_len, uint32_t* rsp_code);	
    int kdp_sfid_add_fm(int dev_idx, uint32_t usr_id, char* data_buf, \
                        int buf_len, uint32_t* rsp_code);
    int kdp_sfid_query_fm(int dev_idx, uint32_t usr_id, uint32_t face_id, \
                          char *data_buf, uint32_t* rsp_code);
    int kdp_sfid_send_img_gen(int dev_idx, char* img_buf, int buf_len, uint32_t* rsp_code,\
            uint16_t* user_id, uint16_t* mode, uint16_t* mask, char* res);
    int kdp_sfid_set_db_config(int dev_idx, kapp_db_config_parameter_t* db_config_parameter, uint32_t* rsp_code);
    int kdp_sfid_get_db_config(int dev_idx, kapp_db_config_parameter_t* db_config_parameter, uint32_t* rsp_code);
    int kdp_sfid_get_db_index(int dev_idx, uint32_t* db_idx, uint32_t* rsp_code);
    int kdp_sfid_switch_db_index(int dev_idx, uint32_t db_idx, uint32_t* rsp_code);
    int kdp_sfid_set_db_version(int dev_idx, uint32_t db_version, uint32_t* rsp_code);
    int kdp_sfid_get_db_version(int dev_idx, uint32_t* db_version, uint32_t* rsp_code);
    int kdp_sfid_get_db_meta_data_version(int dev_idx, uint32_t* db_meta_data_version, uint32_t* rsp_code);

    int msg_received_rx_queue(KnBaseMsg* p_rsp, uint16_t req_type, int idx);
    int kdp_reset_sys_sync(int dev_idx, uint32_t reset_mode);
    int kdp_report_sys_status_sync(int dev_idx, uint32_t* sfw_id, uint32_t* sbuild_id, \
            uint16_t* sys_status, uint16_t* app_status, uint32_t* nfw_id, uint32_t* nbuild_id);
    int kdp_get_kn_number(int dev_idx, uint32_t *kn_num);
    int kdp_get_model_info(int dev_idx, int from_ddr, char *data_buf);
    int kdp_set_ckey(int dev_idx, uint32_t ckey, uint32_t *set_status);
    int kdp_set_sbt_key(int dev_idx, uint32_t entry, uint32_t key, uint32_t *set_status);
    int kdp_get_crc(int dev_idx, int from_ddr, char *data_buf);
    int kdp_update_fw_sync(int dev_idx, uint32_t* module_id, char* img_buf, int buf_len);
    int kdp_update_model_sync(int dev_idx, uint32_t* model_id, uint32_t model_size, \
            char* img_buf, int buf_len);
    int kdp_update_nef_model_sync(int dev_idx, uint32_t info_size, char* img_buf, int buf_len);
    int kdp_update_spl_sync(int dev_idx, uint32_t mode, uint16_t auth_type, \
            uint16_t size, uint8_t* auth_key, char* spl_buf, uint32_t* rsp_code, \
            uint32_t* spl_id, uint32_t* spl_build);

    int kdp_isi_start(int dev_idx, uint32_t app_id, uint32_t rt_size, uint16_t width, uint16_t height, uint32_t format, uint32_t* rsp_code, uint32_t* buf_size);
    int kdp_isi_start_ext(int dev_idx, char* isi_cfg, int cfg_size, uint32_t* rsp_code, uint32_t* buf_size);
    int kdp_isi_inference (int dev_idx, char* img_buf, int buf_len, uint32_t img_id, uint32_t* rsp_code, uint32_t* img_available);
    int kdp_isi_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data);
    int kdp_isi_config(int dev_idx, uint32_t model_id, uint32_t param, uint32_t* rsp_code);

    int kdp_start_dme(int dev_idx, uint32_t model_size, char* data, int dat_size, \
            uint32_t* ret_size, char* img_buf, int buf_len);
    int kdp_dme_configure(int dev_idx, char* data, int dat_size, uint32_t* ret_model_id);
    int kdp_dme_inference(int dev_idx, char* img_buf, int buf_len, uint32_t* inf_size, \
            bool* res_flag, char* inf_res, uint16_t mode, uint16_t model_id);
    int kdp_dme_retrieve_res(int dev_idx, uint32_t addr, int len, char* inf_res);
    int kdp_dme_get_status(int dev_idx, uint16_t *ssid, uint16_t *status, uint32_t* inf_size, char* inf_res);
    int kdp_jpeg_enc_config(int dev_idx, int img_id, int width, int height, int fmt, int  quality);
    int kdp_jpeg_enc(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len);
    int kdp_jpeg_enc_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data);
    int kdp_jpeg_dec_config(int dev_idx, int img_id, int width, int height, int fmt, int len);
    int kdp_jpeg_dec(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len);
    int kdp_jpeg_dec_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data);

private:
    //local db for saving the command responses and its mutex.
    //for each device, there is one mutex and one map.
    //the index is same as in _com_devs
    mutex                       _map_mtx[MAX_COM_DEV];
    map<uint16_t, KnBaseMsg*>   _runing_map[MAX_COM_DEV];

    //wait for rsp msg and its mutex
    mutex                        _cond_mtx;
    #if defined(THREAD_USE_CPP11)
    std::condition_variable_any  _cond_wait;
    #else
    Condition _cond_wait;
    #endif

    KnBaseMsg*  wait_rsp_msg(int dev_idx, uint16_t msg_type, int wait_num = KDP_SYNC_WAIT_NUM);
    void save_req_to_map(uint16_t req_type, int idx = 0);

    int wait_dev_restart(int dev_idx);

};

#endif
