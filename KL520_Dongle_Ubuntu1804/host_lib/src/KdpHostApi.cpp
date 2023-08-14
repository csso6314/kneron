/**
 * @file      KdpHostApi.cpp
 * @brief     kdp host api source file
 * @version   0.1 - 2019-04-19
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "KdpHostApi.h"
#include "err_code.h"
#include "MsgMgmt.h"
#include "KnLog.h"
#include "KnUtil.h"
#include "UARTDev.h"

#include <libusb-1.0/libusb.h>

//#include <thread>
#include <map>
#include <vector>
using namespace std;

#define NO_THREAD_WAY // implement command functions without internal threads

#define FLOAT_ZERO_VAL 0.000001
#define MAX_THRESH_VAL 1.0f
#define MIN_THRESH_VAL 0.0f

MsgMgmt*                _p_mgr = NULL; //msg manager
SyncMsgHandler*         _sync_hdlr = NULL;

map<int, USBDev*> g_kdp_khost_map;
map<khost_device_t, int> g_khost_dev_to_dev_idx_map; // FIXME, use other ways

extern "C" {
int read_nef(char* nef_data, uint32_t nef_size, struct kdp_metadata_s *metadata, \
                struct kdp_nef_info_s *nef_info);
}

static bool is_valid_dev(int dev_idx) {
    if(!_p_mgr) return false;
    return _p_mgr->is_valid_dev(dev_idx);
}

/* Checking if the image dimension is valid.
 * There is preprocessing limitation of image width for KL520 NPU.
 * NIR8: width must be multiple of 4; RGB565/YCBCR422: width must be multiple of 2.
 */
static bool is_valid_input(uint32_t image_format, int32_t image_col, int32_t image_row) {
    bool is_valid = true;

    // bypass preprocessing: BIT19
    if (image_format & 0x80000) {
        return true;
    }

    if ((image_col == 0) || (image_row == 0)) {
        return false;
    }

    switch (image_format & 0x00F0) {
    case NPU_FORMAT_RGB565:
        if (image_col % 2) is_valid = false;
        break;
    case NPU_FORMAT_YCBCR422:
        if (image_col % 2) is_valid = false;
        break;
    case NPU_FORMAT_NIR:
        if (image_col % 4) is_valid = false;
        break;
    default:
        break;
    }
    //printf("%x %d %d\n", image_format, image_col, is_valid);
    return is_valid;
}

//init the host lib
int kdp_lib_init() {
    libusb_init(NULL); //init usb lib
    _sync_hdlr = new SyncMsgHandler();
    _p_mgr = new MsgMgmt(_sync_hdlr);

    g_kdp_khost_map.clear();
    g_khost_dev_to_dev_idx_map.clear();

    KnPrint(KN_LOG_DBG, "msg handler and msg mgr created.\n");
    return 0;
}

//start the host lib
int kdp_lib_start() {
    if(!_p_mgr) return -1;
    //start receiver, sender thread

    _p_mgr->start();

    //_p_mgr->start_sender();

    //start msg handling thread
    _p_mgr->start_handler();
    KnPrint(KN_LOG_DBG, "handler thread and receiver thread started.\n");
    return 0;
}

//init the lib log
//dir: directory of the log file
//name: file name
int kdp_init_log(const char* dir, const char* name) {
    string dirs(dir);
    string names(name);
    KnLogInit(dirs, names);
    KnSetLogLevel(KN_LOG_STOP);

    KnPrint(KN_LOG_DBG, "\n\n\n************----**********\n");
    KnPrint(KN_LOG_DBG, " -- kdp host lib restarted -- \n");
    KnPrint(KN_LOG_DBG, "************----**********\n\n\n");

    KnPrint(KN_LOG_DBG, "init host lib log to %s%s.\n", dir, name);
    return 0;
}

int kdp_scan_usb_devices(kdp_device_info_list_t **list)
{
    khost_ll_scan_devices((kneron_device_info_list_t **)list);
    return 0;
}

int kdp_connect_usb_device(int scan_index)
{
    khost_device_t khdev = khost_ll_connect_device(scan_index);

    if(khdev == 0)
        return -1;

    // transfer khost_dev to kdp usb dev
    USBDev* pDev = new USBDev(khdev);

    int dev_idx = _p_mgr->add_usb_device(pDev);
    _p_mgr->start_receiver(dev_idx);

    g_kdp_khost_map[dev_idx] = pDev;

    // khost_dev -> dev_idx relationship. Workaround for usage of both kdp_xxx and khost_xxx API.
    g_khost_dev_to_dev_idx_map[khdev] = dev_idx;

    return dev_idx;
}

static int connect_usb_device_by_product(enum kdp_product_id_e pid)
{
    int dev_idx = -1;
    kdp_device_info_list_t *list = NULL;
    kdp_scan_usb_devices(&list);

    for (int i = 0; i < list->num_dev; i++)
    {
        if (list->kdevice[i].product_id == pid)
        {
            // found it, now try to connect
            dev_idx = kdp_connect_usb_device(list->kdevice[i].scan_index);
            break;
        }
    }

    free(list);
    return dev_idx;
}

//add device to lib
//currently only UART and USB are supported.
//name: device name
int kdp_add_dev(int type, const char* name) {
    if(type == KDP_UART_DEV) {

        // no support UART anymore
        return -1;

    } else if (type == KDP_USB_DEV) {

        // find out the first KL520 device and connect to it
        // in this way, the behavior matches API doc.
        return connect_usb_device_by_product(KDP_DEVICE_KL520);
        
    } else {
        KnPrint(KN_LOG_DBG, "No identifiable device\n");
        return -1;
    }
}

//free the resources
int kdp_lib_de_init() {
    if(_p_mgr) {
        _p_mgr->stop_receiver();
        //_p_mgr->stop_sender();
        _p_mgr->stop_handler();

        for(int i = 0; i < 5000; i++) {
            usleep(1000);
            if(_p_mgr->is_receiver_running() == false &&
              //_p_mgr->is_sender_running() == false &&
                _p_mgr->is_handler_running() == false) break;
        }
    }

    usleep(1000);
    KnPrint(KN_LOG_DBG, "threads stopped, starting deconstructing objs...\n");
    if(_p_mgr) delete _p_mgr;
    if(_sync_hdlr) delete _sync_hdlr;

    libusb_exit(NULL);
    //close log
    KnLogClose();

    g_kdp_khost_map.clear();
    g_khost_dev_to_dev_idx_map.clear();

    return 0;
}

// -------------------system operation APIs
int kdp_reset_sys(int dev_idx, uint32_t reset_mode) {
    KnPrint(KN_LOG_DBG, "\n\ncalling reset sys...:%d,%x.\n", dev_idx, reset_mode);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_reset_sys_sync(dev_idx, reset_mode);
}

int kdp_report_sys_status(int dev_idx, uint32_t* sfirmware_id, uint32_t* sbuild_id,
            uint16_t* sys_status, uint16_t* app_status, uint32_t* nfirmware_id, uint32_t* nbuild_id) {
    KnPrint(KN_LOG_DBG, "\n\ncalling report sys status...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_report_sys_status_sync(dev_idx, sfirmware_id, \
                sbuild_id, sys_status, app_status, nfirmware_id, nbuild_id);
}

int kdp_get_kn_number(int dev_idx, uint32_t *kn_num) {
    KnPrint(KN_LOG_DBG, "\n\ncalling kdp_get_kn_number: %d\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    return _sync_hdlr->kdp_get_kn_number(dev_idx, kn_num);
}

int kdp_get_model_info(int dev_idx, int from_ddr, char *data_buf) {
    KnPrint(KN_LOG_DBG, "\n\ncalling kdp_get_model_info: %d\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    return _sync_hdlr->kdp_get_model_info(dev_idx, from_ddr, data_buf);
}

int kdp_set_ckey(int dev_idx, uint32_t ckey, uint32_t *set_status) {
    KnPrint(KN_LOG_DBG, "\n\ncalling kdp_set_ckey: %d\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    return _sync_hdlr->kdp_set_ckey(dev_idx, ckey, set_status);
}

int kdp_set_sbt_key(int dev_idx, uint32_t entry, uint32_t key, uint32_t *set_status) {
    KnPrint(KN_LOG_DBG, "\n\ncalling kdp_set_sbt_key: %d\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    return _sync_hdlr->kdp_set_sbt_key(dev_idx, entry, key, set_status);
}

int kdp_get_crc(int dev_idx, int from_ddr, char *data_buf) {
    KnPrint(KN_LOG_DBG, "\n\ncalling kdp_get_crc: %d\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    return _sync_hdlr->kdp_get_crc(dev_idx, from_ddr, data_buf);
}

int kdp_get_nef_model_metadata(char* model_data, uint32_t m_size, struct kdp_metadata_s* metadata) {
    KnPrint(KN_LOG_DBG, "\n\ncalling getting metadata from nef model file.\n");

    int metadata_size = 0;
    struct kdp_nef_info_s nef_info;
    memset(&nef_info, 0, sizeof(kdp_nef_info_s));
    int ret = 0;
    ret = read_nef(model_data, m_size, metadata, &nef_info);

    if (ret != 0) {
        KnPrint(KN_LOG_ERR, "getting model data failed:%d...\n", ret);
        return -1;
    }

    metadata_size = m_size - nef_info.fw_info_size - nef_info.all_models_size;

    ret = (metadata_size > 0) ? 0 : -1;

    return ret;
}

int kdp_update_fw(int dev_idx, uint32_t* module_id, char* img_buf, int buf_len) {
    KnPrint(KN_LOG_DBG, "\n\ncalling updating firmware...:%d, %d.\n", dev_idx, *module_id);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_update_fw_sync(dev_idx, module_id, img_buf, buf_len);
}

int kdp_update_model(int dev_idx, uint32_t* model_id, uint32_t model_size, \
                    char* img_buf, int buf_len) {
    KnPrint(KN_LOG_DBG, "\n\ncalling updating bin model...:%d, %d.\n", dev_idx, *model_id);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    return _sync_hdlr->kdp_update_model_sync(dev_idx, model_id, \
                model_size, img_buf, buf_len);
}

int kdp_update_nef_model(int dev_idx, char* img_buf, int buf_len) {
    KnPrint(KN_LOG_DBG, "\n\ncalling updating nef model...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    struct kdp_metadata_s metadata;
    memset(&metadata, 0, sizeof(kdp_metadata_s));
    struct kdp_nef_info_s nef_info;
    memset(&nef_info, 0, sizeof(kdp_nef_info_s));

    int ota_size = 0;
    char* p_buf = NULL;

    int ret = read_nef(img_buf, buf_len, &metadata, &nef_info);

    // get model data (fw_info and all_models)
    if (ret != 0) {
        KnPrint(KN_LOG_ERR, "getting model data failed:%d...\n", ota_size);
        return -1;
    }

    uint32_t fw_info_size = nef_info.fw_info_size;
    uint32_t all_models_size = nef_info.all_models_size;
    ota_size = fw_info_size + all_models_size;
    p_buf = new char[ota_size];
    memset(p_buf, 0, ota_size);
    memcpy(p_buf, nef_info.fw_info_addr, fw_info_size);
    memcpy(p_buf+fw_info_size, nef_info.all_models_addr, all_models_size);

    // no encryption for nef file, update model
    if (!metadata.enc_type) {
        ret = _sync_hdlr->kdp_update_nef_model_sync(dev_idx, fw_info_size, p_buf, ota_size);
        delete[] p_buf;
        return ret;
    }

    // encryption for nef file, check the KN numbers to determine whether update model
    KnPrint(KN_LOG_ERR, "starting get KN number from Kneron device\n");
    uint32_t kn_num;
    ret = _sync_hdlr->kdp_get_kn_number(dev_idx, &kn_num);
    if (ret != 0) {
        KnPrint(KN_LOG_ERR, "Error when getting KN number, model update abort\n");
        delete[] p_buf;
        return -1;
    }

    // old firmware without support of reading KN number, block update model
    if (kn_num == 0xFFFF) {
        KnPrint(KN_LOG_ERR, "Not supported by the version of the firmware\n");
        delete[] p_buf;
        return -1;
    }

    if (kn_num != metadata.kn_num) {
        KnPrint(KN_LOG_ERR, "Could not update model: KN number of model file and Kneron device is not equal\n");
        delete[] p_buf;
        return -1;
    }

    ret = _sync_hdlr->kdp_update_nef_model_sync(dev_idx, fw_info_size, p_buf, ota_size);
    delete[] p_buf;
    return ret;
}

void error_code_parse(uint32_t rsp_code)
{
    if (SUCCESS == rsp_code)
        printf("sucess\n\r");
    else if (MSG_APP_DB_NO_MATCH == rsp_code)
        printf("DB no match\n\r");
    else if (MSG_APP_NOT_LOADED == rsp_code)	
        printf("not loaded\n\r");
    else if (MSG_APP_BAD_MODE == rsp_code)
        printf("bad application mode\n\r");
    else if (MSG_DB_ADD_FM_FAIL == rsp_code)
        printf("add DB uid_indx failed\n\r");
    else if (MSG_DB_DEL_FM_FAIL == rsp_code)
        printf("delete DB uid_indx failed\n\r");
    else if (MSG_DB_BAD_PARAM == rsp_code)
        printf("database action/format error\n\r");
    else if (MSG_SFID_OUT_FAIL == rsp_code)
        printf("data upload failed\n\r"); 
    else if (MSG_SFID_NO_MEM == rsp_code)
        printf("memory allocation failed\n\r"); 
    else if (MSG_AUTH_FAIL == rsp_code)
        printf("authentication failed\n\r"); 
    else if (MSG_FLASH_FAIL == rsp_code)
        printf("flash programming failed (bad sector?)\n\r"); 
    else if (MSG_DATA_ERROR == rsp_code)
        printf("data error (I/O)\n\r");
    else if (MSG_SFID_USR_ZERO == rsp_code)
        printf("user id zero error\n\r");   
    else if (MSG_CONFIG_ERROR == rsp_code)
        printf("no appropriate Start issued previously\n\r");  
    else if (MSG_APP_BAD_IMAGE == rsp_code)
        printf("bad image\n\r");   
    else if (MSG_APP_BAD_INDEX == rsp_code)
        printf("bad index\n\r");     
    else if (MSG_APP_UID_EXIST == rsp_code)
        printf("uid exist\n\r");  
    else if (MSG_APP_UID_DIFF == rsp_code)
        printf("uid different\n\r");  
    else if (MSG_APP_IDX_FIRST == rsp_code)
        printf("idx first\n\r");
    else if (MSG_APP_IDX_MISSING == rsp_code)
        printf("idx missing\n\r"); 
    else if (MSG_APP_DB_NO_USER == rsp_code)
        printf("DB no user\n\r");
    else if (MSG_APP_DB_FAIL == rsp_code)
        printf("DB failed\n\r");
    else if (MSG_APP_LV_FAIL == rsp_code)
        printf("LV failed\n\r");   
    else if (MSG_DB_QUERY_FM_FAIL == rsp_code)
        printf("query DB fm data failed\n\r");   
    else
        printf("unknown_error\n\r");
  return;

}	

int kdp_update_spl(int dev_idx, uint32_t mode, uint16_t auth_type, uint16_t size, \
            uint8_t* auth_key, char* spl_buf, uint32_t* rsp_code, uint32_t* spl_id, \
            uint32_t* spl_build) {
    KnPrint(KN_LOG_DBG, "\n\ncalling updating spl...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_update_spl_sync(dev_idx, mode, auth_type, size, \
            auth_key, spl_buf, rsp_code, spl_id, spl_build);
}

//-------------------------------app level APIs

int kdp_start_sfid_mode(int dev_idx, uint32_t* img_size, float thresh,
            uint16_t width, uint16_t height, uint32_t format) {
    KnPrint(KN_LOG_DBG, "\n\ncalling start verify mode...:%d.\n", dev_idx);

    if (!is_valid_input(format, width, height)) {
        KnPrint(KN_LOG_ERR, "the image with unacceptable dimension could not be processed by KL520 NPU.\n");
        return -2;
    }

    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    if(!img_size) {
        KnPrint(KN_LOG_ERR, "img size is null.\n");
        return -1;
    }

    if(fabsf(thresh) < FLOAT_ZERO_VAL) {
        //0.0f
    } else {
        if(thresh > MAX_THRESH_VAL || thresh < MIN_THRESH_VAL) {
            KnPrint(KN_LOG_ERR, "threshold val is invalid, range:%5.2f ~ %5.2f.\n",
                    MIN_THRESH_VAL, MAX_THRESH_VAL);
            return -1;
        }
    }

    if(width == 0) width = 640;
    if(height == 0) height = 480;

    uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_start(dev_idx, &rsp_code, img_size,
                thresh, width, height, format);
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "start verify mode failed with error:%d.\n", ret);
        return -1;
    }

    if(rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "start verify mode failed with error:%d.\n", rsp_code);
        return rsp_code;
    }
    return 0;
}

int kdp_verify_user_id_generic(int dev_idx, uint16_t* user_id,
                               char* img_buf, int buf_len, uint16_t* mask, char* res)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling verify user id...:%d,%d.\n", dev_idx, buf_len);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    if(!user_id || !img_buf || buf_len <= 0) {
        KnPrint(KN_LOG_ERR, "verify user id input parameter is invalid.\n");
        return -1;
    }

    if( mask && (*mask & 0x1f) ) {
        if(res == NULL) {
            KnPrint(KN_LOG_ERR, "invalid input arguments for res flag, result.\n");
            return -1;
        }
    }

    uint32_t rsp_code = 0;
    uint16_t mode = 1;
    int ret = _sync_hdlr->kdp_sfid_send_img_gen(dev_idx, img_buf, buf_len, &rsp_code, user_id, &mode, 
                mask, res);
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "verify user id failed with error:%d.\n", ret);
        return -1;
    }

    if(rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "verify user id failed with error:%d.\n", rsp_code);
        error_code_parse(rsp_code);			
        return rsp_code;
    }

    if(mode != 0) {
        KnPrint(KN_LOG_ERR, "device is not in user verify mode.\n");
        return -1;
    }

    KnPrint(KN_LOG_DBG, "verify user id completed, user id:%d.\n", *user_id);
    if(*user_id < 1 || *user_id > MAX_USER) {
        KnPrint(KN_LOG_ERR, "invalid user id received:%d.\n", *user_id);
        return -1;
    }
    return 0;
}

int kdp_start_reg_user_mode(int dev_idx, uint16_t usr_id, uint16_t img_idx) {
    KnPrint(KN_LOG_DBG, "\n\ncalling start reg user mode...:%d,%d.\n", usr_id, img_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    if(usr_id < 1 || usr_id > MAX_USER || img_idx < 1 || img_idx > MAX_FID) {
        KnPrint(KN_LOG_ERR, "new user input parameter is invalid.\n");
        return -1;
    }

    int ret =  _sync_hdlr->kdp_sfid_new_user(dev_idx, usr_id, img_idx);
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "start reg mode failed with error:%d.\n", ret);
        return -1;
    }

    return 0;
}

uint16_t kdp_get_res_mask(bool fd, bool lm, bool fr, bool liveness, bool score)
{
    return ((fd << 0) + (lm << 1) + (fr << 2) + (liveness << 3) + (score << 4));
}

uint32_t kdp_get_res_size(bool fd, bool lm, bool fr, bool liveness, bool score)
{
    uint32_t size = 0;

    size += fd ? FD_RES_LENGTH : 0;
    size += lm ? LM_RES_LENGTH : 0;
    size += fr ? FR_RES_LENGTH : 0;
    size += liveness ? LV_RES_LENGTH : 0;
    size += score ? SCORE_RES_LENGTH : 0;

    return size;
}

int kdp_extract_feature_generic(int dev_idx,
                                char* img_buf, int buf_len, uint16_t* mask, char* res)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling extract feature generic..:%d,%d.\n", dev_idx, buf_len);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    if(!img_buf || buf_len <= 0) {
        KnPrint(KN_LOG_ERR, "extract feature input parameter is invalid.\n");
        return -1;
    }

    if( mask && (*mask & 0x1f) ) {
        if(res == NULL) {
            KnPrint(KN_LOG_ERR, "invalid input arguments for res flag, result.\n");
            return -1;
        }
    }

    uint32_t rsp_code = 0;
    uint16_t img_idx = 0;
    uint16_t mode = 0;
    int ret = _sync_hdlr->kdp_sfid_send_img_gen(dev_idx, img_buf, buf_len, &rsp_code, &img_idx, &mode, 
                mask, res);
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "extract feature failed with error:%d.\n", ret);
        return -1;
    }

    if(rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "extract feature failed with rsp_code error:%d.\n", rsp_code);
		error_code_parse(rsp_code);		
        return rsp_code;
    }

    if(mode == 0) {
        KnPrint(KN_LOG_ERR, "device is not in user register mode.\n");
        return -1;
    }

    KnPrint(KN_LOG_DBG, "extract feature completed, img index:%d.\n", img_idx);
    if(img_idx < 1 || img_idx > 5) {
        KnPrint(KN_LOG_ERR, "invalid image received:%d.\n", img_idx);
        return -1;
    }
    return 0;
}

int kdp_register_user(int dev_idx, uint32_t user_id) {
    KnPrint(KN_LOG_DBG, "\n\ncalling register user...:%d,%d.\n", dev_idx, user_id);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    if(user_id < 1 || user_id > MAX_USER) {
        KnPrint(KN_LOG_ERR, "reg user input parameter is invalid.\n");
        return -1;
    }

    uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_register(dev_idx, user_id, &rsp_code);
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "reg user failed with error:%d.\n", ret);
        return -1;
    }

    if(rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "reg user failed with error:%d.\n", rsp_code);
        error_code_parse(rsp_code);			
        return rsp_code;
    }

    KnPrint(KN_LOG_DBG, "reg user completed, user id:%d.\n", user_id);
    return 0;
}

int kdp_remove_user(int dev_idx, uint32_t user_id) {
    KnPrint(KN_LOG_DBG, "\n\ncalling remove user...:%d,%d.\n", dev_idx, user_id);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    if(user_id < 0 || user_id > MAX_USER) {
        KnPrint(KN_LOG_ERR, "del user input parameter is invalid.\n");
        return -1;
    }

    uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_del_db(dev_idx, user_id, &rsp_code);
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "remove user failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if(rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "remove user failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        error_code_parse(rsp_code);		
        return rsp_code;
    }
    return 0;
}

int kdp_list_users(int dev_idx, int user_id) {
    uint32_t mode = DB_QUERY;	
	uint16_t *user_list;
	uint32_t len;
    KnPrint(KN_LOG_DBG, "\n\ncalling list users ...:%d,%d.\n", dev_idx, user_id);

    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    if(user_id < 0 || user_id > MAX_USER) {
        KnPrint(KN_LOG_ERR, "del user input parameter is invalid.\n");
        return -1;
    }

    uint32_t rsp_code = 0;
    uint32_t i;
    int ret = _sync_hdlr->kdp_sfid_edit_db(dev_idx, mode,(char **) &user_list, &len, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "list users  user failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }
    ret = 0;
    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "list users  user failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        error_code_parse(rsp_code);
		
		if (MSG_APP_DB_NO_USER == rsp_code) {
		    if (0 == user_id)
                printf("there is no any user in DB\n\r");
            else 
                printf("user_id %d is not found in DB \r\n", user_id);				
        }
		else if (MSG_APP_DB_FAIL == rsp_code)
		    printf(" read DB failed\r\n");
		
    }
	else {
        for (i = 0; i < len; i++) {
            if (0 == user_id) {
                printf("registered user ID in DB is 0x%x\r\n", user_list[i]);
            }
            else {
                if (user_list[i] == user_id) {
                    printf("found user_id %d in DB \r\n", user_list[i]);
                    break;
                }						
            }
        }
        if ((0 != user_id ) && (i == len))
            printf("user_id %d is not found in DB \r\n", user_id);

    }
    return rsp_code;
}

int kdp_export_db(int dev_idx, char **p_buf, uint32_t *p_len) {
	
    KnPrint(KN_LOG_DBG, "\n\ncalling export db dev_idx ...:%d.\n", dev_idx);

    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
	uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_export(dev_idx, p_buf, p_len, &rsp_code);
	
    if(ret < 0) {
        KnPrint(KN_LOG_ERR, "export db failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if(rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "export db failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        error_code_parse(rsp_code);	
        return -1;
    }
    return 0;
}

int kdp_import_db(int dev_idx, char* p_buf, uint32_t p_len) {
    KnPrint(KN_LOG_DBG, "\n\ncalling import to db ...:%d.\n", dev_idx);

    if (!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_import(dev_idx, p_buf, p_len, &rsp_code);
		
    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "import to db failed for  with ret error:%d.\n",  ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "import to db failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        error_code_parse(rsp_code);			
	    if (MSG_CONFIG_ERROR == rsp_code) {
			printf("MSG_CONFIG_ERROR\n");
			
        }
		else
            if (MSG_DB_BAD_PARAM == rsp_code) {
			    printf("MSG_DB_BAD_PARAM\n");
            }			
        return -1;
    }
    return 0;
}

int kdp_register_user_by_fm(int dev_idx, uint32_t user_id, char* fm, int fm_len)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling add user fm...:%d,%d.\n", dev_idx, user_id);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    if(user_id < 1 || user_id > MAX_USER) {
        KnPrint(KN_LOG_ERR, "add user fm input parameter is invalid.\n");
        return -1;
    }

    uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_add_fm(dev_idx, user_id, fm, fm_len, &rsp_code);
    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "add user fm failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        error_code_parse(rsp_code);			
        KnPrint(KN_LOG_ERR, "add user fm failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return -1;
    }
    return ret;
}

int kdp_query_fm_by_user(int dev_idx, char* fm/*output*/, uint32_t user_id, uint32_t face_id)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling query user fm...:%d,%d,%d.\n", dev_idx, user_id, face_id);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    if(user_id < 1 || user_id > MAX_USER ||
       face_id < 1 || face_id > MAX_FID) {
        KnPrint(KN_LOG_ERR, "query user fm input parameter is invalid.\n");
        return -1;
    }

    uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_query_fm(dev_idx, user_id, face_id, fm, &rsp_code);
    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "query user fm failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        error_code_parse(rsp_code);			
        KnPrint(KN_LOG_ERR, "query user fm failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    return 0;
}

float kdp_fm_compare(float *user_fm_a, float *user_fm_b, size_t fm_size)
{
    //**************
    // calcualte match
    //**************
    float sum_xy = 0.;

    if ((user_fm_a == NULL) || (user_fm_b == NULL) || (fm_size <= 0)) {
        printf("parameter error \n");
        return -1;
    }

    for (size_t i = 0; i < fm_size; i++) {
        sum_xy += powf((user_fm_a[i] - user_fm_b[i]), 2);
    }

    return sqrtf(sum_xy);
}

int kdp_set_db_config(int dev_idx, kdp_db_config_t* db_config)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling configure db ... :%d,%d,%d,%d.\n", dev_idx, db_config->db_num, db_config->max_uid, db_config->max_fid);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    
    uint32_t rsp_code = 0;
    kapp_db_config_parameter_t db_config_parameter;
    memcpy(&db_config_parameter.db_config, db_config, sizeof(kdp_db_config_t));

    int ret = _sync_hdlr->kdp_sfid_set_db_config(dev_idx, &db_config_parameter, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "configure db failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "configure db failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    return 0;
}

int kdp_get_db_config(int dev_idx, kdp_db_config_t* db_config)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling get db configuration ...\n");
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    
    if(NULL == db_config) {
        KnPrint(KN_LOG_ERR, "detect NULL pointer on return parameter.\n");
        return -1;
    }

    uint32_t rsp_code = 0;
    kapp_db_config_parameter_t db_config_parameter;
    
    int ret = _sync_hdlr->kdp_sfid_get_db_config(dev_idx, &db_config_parameter, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "get db configuration failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "get db configuration failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    memcpy(db_config, &db_config_parameter.db_config, sizeof(kdp_db_config_t));

    return 0;
}

int kdp_get_db_index(int dev_idx, uint32_t *db_idx)
{
    uint32_t rsp_code = 0;
    int ret = 0;

    KnPrint(KN_LOG_DBG, "\n\ncalling get current db index ...\n");
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    ret = _sync_hdlr->kdp_sfid_get_db_index(dev_idx, db_idx, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "get current db index failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "get current db index failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    return ret;
}

int kdp_switch_db_index(int dev_idx, uint32_t db_idx)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling switch db ... :%d,%d.\n", dev_idx, db_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }
    
    if(0 > db_idx) {
        KnPrint(KN_LOG_ERR, "switch db input parameter is invalid.\n");
        return MSG_DB_BAD_PARAM;
    }

    uint32_t rsp_code = 0;
    int ret = _sync_hdlr->kdp_sfid_switch_db_index(dev_idx, db_idx, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "switch db failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "switch db failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    return 0;
}

int kdp_set_db_version(int dev_idx, uint32_t db_version)
{
    uint32_t rsp_code = 0;
    int ret = 0;

    KnPrint(KN_LOG_DBG, "\n\ncalling set db version ...\n");
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    ret = _sync_hdlr->kdp_sfid_set_db_version(dev_idx, db_version, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "set db version failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "set db version failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    return ret;
}

int kdp_get_db_version(int dev_idx, uint32_t *db_version)
{
    uint32_t rsp_code = 0;
    int ret = 0;

    KnPrint(KN_LOG_DBG, "\n\ncalling get current db version ...\n");
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    ret = _sync_hdlr->kdp_sfid_get_db_version(dev_idx, db_version, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "get current db version failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "get current db version failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    return ret;
}

int kdp_get_db_meta_data_version(int dev_idx, uint32_t *db_meta_data_version)
{
    uint32_t rsp_code = 0;
    int ret = 0;

    KnPrint(KN_LOG_DBG, "\n\ncalling get current db version ...\n");
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    ret = _sync_hdlr->kdp_sfid_get_db_meta_data_version(dev_idx, db_meta_data_version, &rsp_code);

    if (ret < 0) {
        KnPrint(KN_LOG_ERR, "get current db version failed for dev:%d, with error:%d.\n", dev_idx, ret);
        return -1;
    }

    if (rsp_code != 0) {
        KnPrint(KN_LOG_ERR, "get current db version failed for dev:%d, with error:%d.\n", dev_idx, rsp_code);
        return rsp_code;
    }

    return ret;
}

//-------------------------------ISI APIs
int kdp_start_isi_mode(int dev_idx, uint32_t app_id, uint32_t rt_size, uint16_t width, uint16_t height, uint32_t format, uint32_t* rsp_code, uint32_t* buf_size) {
    KnPrint(KN_LOG_DBG, "\n\ncalling start isi mode...:%d.\n", dev_idx);

    if (!is_valid_input(format, width, height)) {
        KnPrint(KN_LOG_ERR, "the image with unacceptable dimension could not be processed by KL520 NPU.\n");
        return -2;
    }

    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_isi_start(dev_idx, app_id, rt_size, width, height, format, rsp_code, buf_size);
}

int kdp_start_isi_mode_ext(int dev_idx, char* isi_cfg, int cfg_size, uint32_t* rsp_code, uint32_t* buf_size) {
    KnPrint(KN_LOG_DBG, "\n\ncalling start isi mode...:%d.\n", dev_idx);

    struct kdp_isi_cfg_s *cfg = (struct kdp_isi_cfg_s *)isi_cfg;
    if (!is_valid_input(cfg->image_format, cfg->image_col, cfg->image_row)) {
        KnPrint(KN_LOG_ERR, "the image with unacceptable dimension could not be processed by KL520 NPU.\n");
        return -2;
    }

    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_isi_start_ext(dev_idx, isi_cfg, cfg_size, rsp_code, buf_size);
}

int kdp_isi_inference(int dev_idx, char* img_buf, int buf_len, uint32_t img_id, \
            uint32_t* rsp_code, uint32_t* img_available) {
    KnPrint(KN_LOG_DBG, "\n\ncalling isi inference...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_isi_inference(dev_idx, img_buf, buf_len, img_id, rsp_code, img_available);
}

int kdp_isi_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data) {
    KnPrint(KN_LOG_DBG, "\n\ncalling isi retrieve results...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_isi_retrieve_res(dev_idx, img_id, rsp_code, r_size, r_data);
}

int kdp_isi_config(int dev_idx, uint32_t model_id, uint32_t param, uint32_t *rsp_code) {
    KnPrint(KN_LOG_DBG, "\n\ncalling isi config...:%d.\n", dev_idx);
    if (!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    if (!_sync_hdlr) return -1;
    return _sync_hdlr->kdp_isi_config(dev_idx, model_id, param, rsp_code);
}

int kdp_end_isi(int dev_idx) {
    KnPrint(KN_LOG_DBG, "\n\ncalling ending isi...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    return _sync_hdlr->kdp_reset_sys_sync(dev_idx, 255);
}

#ifdef NO_THREAD_WAY

// This ACK packet is from Firmware
static uint8_t ack_packet[] = {0x35, 0x8A, 0xC, 0, 0x4, 0, 0x8, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ACK_RESPONSE_LEN 16
#define USB_CMD_DEFAULT_TIMEOUT 5000

static int send_command_header(khost_device_t dev, int cmd_id, void *buf, int len, int timeout)
{
    int sts = khost_ll_write_command(dev, buf, len, timeout);
    if (sts != KHOST_RET_OK)
    {
        KnPrint(KN_LOG_ERR, "send command (0x%X) failed, error = %d\n", cmd_id, sts);
        return -1;
    }
    return 0;
}

static int send_command_data(khost_device_t dev, int cmd_id, void *buf, int len, int timeout)
{
    int sts = khost_ll_write_command(dev, buf, len, timeout);
    if (sts != KHOST_RET_OK)
    {
        KnPrint(KN_LOG_ERR, "send command (0x%X) data failed, error = %d\n", cmd_id, sts);
        return -1;
    }
    return 0;
}

static int receive_command_ack(khost_device_t dev, int cmd_id, int timeout)
{
    uint8_t response[ACK_RESPONSE_LEN];
    int recv_len = khost_ll_read_command(dev, response, ACK_RESPONSE_LEN, timeout);
    if (recv_len < 0)
    {
        KnPrint(KN_LOG_ERR, "receive command (0x%X) ACK failed, error = %d\n", cmd_id, recv_len);
        return -1;
    }

    if (recv_len != ACK_RESPONSE_LEN)
    {
        KnPrint(KN_LOG_ERR, "receive command (0x%X) ACK failed, received length %d != %d\n", cmd_id, recv_len, ACK_RESPONSE_LEN);
        return -1;
    }

    if (memcmp(ack_packet, response, ACK_RESPONSE_LEN) != 0)
    {
        KnPrint(KN_LOG_ERR, "receive command (0x%X) ACK failed, packet is not correct\n", cmd_id);
        return -1;
    }

    return 0;
}

static int recevie_command_response(khost_device_t dev, int cmd_id, uint8_t *recv_buf, int recv_buf_size, int timeout)
{
    int recv_len = khost_ll_read_command(dev, recv_buf, recv_buf_size, timeout);
    if (recv_len < 0)
    {
        KnPrint(KN_LOG_ERR, "receive command (0x%X) message header failed, error = %d\n", cmd_id, recv_len);
        return -1;
    }

    MsgHdr *rsp_hdr = (MsgHdr *)recv_buf;

    if (rsp_hdr->preamble != MSG_HDR_RSP)
    {
        KnPrint(KN_LOG_ERR, "receive command (0x%X) message header failed, wrong preamble 0x%x should be 0x%x\n", cmd_id, rsp_hdr->preamble, MSG_HDR_RSP);
        return -1;
    }

    if ((rsp_hdr->cmd & 0x7FFF) != cmd_id)
    {
        KnPrint(KN_LOG_ERR, "receive command (0x%X) message header failed, wrong cmd_id 0x%x\n", cmd_id, rsp_hdr->preamble, MSG_HDR_RSP);
        return -1;
    }

    return 0;
}

#endif

//-------------------------------DME APIs
int kdp_start_dme(int dev_idx, uint32_t model_size, char* fw_info, int fw_info_size, \
                  uint32_t* ret_size, char* model_buf, int model_buf_len) {
    KnPrint(KN_LOG_DBG, "\n\ncalling starting dme mode...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

#ifdef NO_THREAD_WAY

    uint32_t transfer_size = sizeof(start_dme_header) + fw_info_size;
    uint8_t *cmd_buffer = NULL;
    uint8_t *recv_buffer = NULL;
    int timeout = USB_CMD_DEFAULT_TIMEOUT;
    int ret = -1;
    *ret_size = 0; // return transferred model_size

    do
    {
        cmd_buffer = new uint8_t[transfer_size];

        start_dme_header *dme_message = (start_dme_header *)cmd_buffer;
        dme_message->msg_header.preamble = MSG_HDR_CMD;
        dme_message->msg_header.ctrl = 0;
        dme_message->msg_header.cmd = CMD_DME_START;
        dme_message->msg_header.msg_len = 4 + fw_info_size;
        dme_message->model_size = model_size;
        memcpy(dme_message->fw_info, fw_info, fw_info_size);

        khost_device_t khdev = kdp_dev_to_khost_dev(dev_idx);

        if (send_command_header(khdev, CMD_DME_START, cmd_buffer, transfer_size, timeout))
            break;

        if (receive_command_ack(khdev, CMD_DME_START, timeout))
            break;

        if (send_command_data(khdev, CMD_DME_START, model_buf, model_size, timeout))
            break;

        int recv_buffer_size = sizeof(MsgHdr) + sizeof(RspPram);
        recv_buffer = new uint8_t[recv_buffer_size];

        if (recevie_command_response(khdev, CMD_DME_START, recv_buffer, recv_buffer_size, timeout))
            break;

        RspPram *rsp_parameters = (RspPram *)(recv_buffer + sizeof(MsgHdr));

        if (rsp_parameters->error != 0)
            break;

        if (rsp_parameters->param2 != model_size)
            break;

        *ret_size = rsp_parameters->param2;

        ret = 0; // all done, successfull

    } while (0);

    if (cmd_buffer)
        delete[] cmd_buffer;

    if (recv_buffer)
        delete[] recv_buffer;

    return ret;

#else
    return _sync_hdlr->kdp_start_dme(dev_idx, model_size, fw_info, fw_info_size,
                    ret_size, model_buf, model_buf_len);
#endif
}

int kdp_start_dme_ext(int dev_idx, char* nef_model_data, uint32_t m_size, uint32_t* ret_size) {
    KnPrint(KN_LOG_DBG, "\n\ncalling starting dme mode...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    struct kdp_metadata_s metadata;
    memset(&metadata, 0, sizeof(kdp_metadata_s));
    struct kdp_nef_info_s nef_info;
    memset(&nef_info, 0, sizeof(kdp_nef_info_s));

    int ota_size = 0;
    int ret = read_nef(nef_model_data, m_size, &metadata, &nef_info);

    if (ret != 0) {
        KnPrint(KN_LOG_ERR, "getting model data failed:%d...\n", ota_size);
        return -1;
    }

    int fw_info_size = nef_info.fw_info_size;

    if (!fw_info_size) {
        KnPrint(KN_LOG_ERR, "Not supported target %d!\n", metadata.target);
        return -1;
    }

    // split model_data to fw_info and all_models
    uint32_t model_size = 0;
    char data[8192];//8KB
    int dat_size = 0;
    char* p_buf = NULL;
    uint32_t buf_len = 0;
    int n_len;

    // get fw_info data
    memset(data, 0, sizeof(data));
    memcpy(data, nef_info.fw_info_addr, fw_info_size);
    dat_size = fw_info_size;

    // get all_models data
    n_len = nef_info.all_models_size;

    if (n_len <= 0) {
        KnPrint(KN_LOG_ERR, "getting all_models data failed:%d...\n", n_len);
        return -1;
    }

    p_buf = new char[n_len];
    memset(p_buf, 0, n_len);
    memcpy(p_buf, nef_info.all_models_addr, n_len);

    buf_len = n_len;
    model_size = n_len;

    ret = kdp_start_dme(dev_idx, model_size, data, dat_size,
                                  ret_size, p_buf, buf_len);

    delete[] p_buf;

    return ret;
}

int kdp_dme_configure(int dev_idx, char* data, int dat_size, uint32_t* ret_model_id) {
    KnPrint(KN_LOG_DBG, "\n\ncalling dme config...:%d.\n", dev_idx);

    struct kdp_dme_cfg_s *dme_cfg = (struct kdp_dme_cfg_s *)data;
    if (!is_valid_input(dme_cfg->image_format, dme_cfg->image_col, dme_cfg->image_row)) {
        KnPrint(KN_LOG_ERR, "the image with unacceptable dimension could not be processed by KL520 NPU.\n");
        return -2;
    }

    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_dme_configure(dev_idx, data, dat_size, ret_model_id);
}

int kdp_dme_inference(int dev_idx, char* img_buf, int buf_len, uint32_t* inf_size, \
            bool* res_flag, char* inf_res, uint16_t mode, uint16_t model_id) {
    KnPrint(KN_LOG_DBG, "\n\ncalling dme inference...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_dme_inference(dev_idx, img_buf, buf_len, inf_size, res_flag, inf_res, mode, model_id);
}

int kdp_dme_retrieve_res(int dev_idx, uint32_t addr, int len, char* inf_res) {
    KnPrint(KN_LOG_DBG, "\n\ncalling dme retrieve results...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_dme_retrieve_res(dev_idx, addr, len, inf_res);
}

int kdp_dme_get_status(int dev_idx, uint16_t *ssid, uint16_t *status, uint32_t* inf_size, char* inf_res) {
    KnPrint(KN_LOG_DBG, "\n\ncalling dme get status...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_dme_get_status(dev_idx, ssid, status, inf_size, inf_res);
}

int kdp_end_dme(int dev_idx) {
    KnPrint(KN_LOG_DBG, "\n\ncalling ending dme...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx)) {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_reset_sys_sync(dev_idx, 255);
}

/*************** JPEG ENC APIs **************/

int kdp_jpeg_enc_config(int dev_idx, int img_id, int width, int height, int fmt, int quality)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling: %s \n", __func__);
    if(!is_valid_dev(dev_idx))
    {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_jpeg_enc_config(dev_idx, img_id, width, height, fmt, quality);
}

int kdp_jpeg_enc(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling: %s \n", __func__);
    if(!is_valid_dev(dev_idx))
    {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_jpeg_enc(dev_idx, img_seq, in_img_buf, in_img_len, out_img_buf, out_img_len);
}

int kdp_jpeg_enc_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling jpeg enc get results...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx))
    {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_jpeg_enc_retrieve_res(dev_idx, img_id, rsp_code, r_size, r_data);
}

/*************** end of JPEG enc ************/

/*************** JPEG DEC APIs **************/

int kdp_jpeg_dec_config(int dev_idx, int img_id, int width, int height, int fmt, int len)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling: %s \n", __func__);
    if(!is_valid_dev(dev_idx))
    {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_jpeg_dec_config(dev_idx, img_id, width, height, fmt, len);
}

int kdp_jpeg_dec(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling: %s \n", __func__);
    if(!is_valid_dev(dev_idx))
    {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_jpeg_dec(dev_idx, img_seq, in_img_buf, in_img_len, out_img_buf, out_img_len);
}

int kdp_jpeg_dec_retrieve_res(int dev_idx, uint32_t img_id, uint32_t* rsp_code, uint32_t* r_size, char* r_data)
{
    KnPrint(KN_LOG_DBG, "\n\ncalling jpeg enc get results...:%d.\n", dev_idx);
    if(!is_valid_dev(dev_idx))
    {
        KnPrint(KN_LOG_ERR, "dev idx is invalid.\n");
        return -1;
    }

    
    return _sync_hdlr->kdp_jpeg_dec_retrieve_res(dev_idx, img_id, rsp_code, r_size, r_data);
}

/*************** end of JPEG dec ************/

void error_code_2_str(int code, char* buf, int len)
{
    memset(buf, 0, len);

    switch(code) {
    case UNKNOWN_ERR:
        snprintf(buf, len-1, "General error");
        break;
    case MSG_APP_NOT_LOADED:
        snprintf(buf, len-1, "SFID application not loaded");
        break;
    case MSG_APP_BAD_INDEX:
        snprintf(buf, len-1, "Invalid User ID or image index specified");
        break;
    case MSG_APP_UID_EXIST:
        snprintf(buf, len-1, "User ID already exists");
        break;
    case MSG_APP_UID_DIFF:
        snprintf(buf, len-1, "User ID changed from the New User session");
        break;
    case MSG_APP_IDX_FIRST:
        snprintf(buf, len-1, "User ID changed from the New User session");
        break;
    case MSG_APP_IDX_MISSING:
        snprintf(buf, len-1, "No feature maps from New User session found");
        break;
    case MSG_APP_DB_NO_USER:
        snprintf(buf, len-1, "User ID not found in DB");
        break;
    case MSG_APP_DB_FAIL:
        snprintf(buf, len-1, "DB delete failure");
        break;
    case MSG_APP_DB_NO_MATCH:
        snprintf(buf, len-1, "No match in User DB");
        break;
    case MSG_APP_BAD_IMAGE:
        snprintf(buf, len-1, "No successful inference (FD-FR)");
        break;
    default:
        snprintf(buf, len-1, "error code [%d] not found", code);
        break;
    }
    return;
}

khost_device_t kdp_dev_to_khost_dev(int dev_idx)
{
    USBDev *usbdev = g_kdp_khost_map[dev_idx];
    return usbdev->to_khost_device();
}

int kdp_khost_dev_to_dev_idx(khost_device_t dev)
{
    map<khost_device_t, int>::iterator it;    
    it = g_khost_dev_to_dev_idx_map.find(dev);

    if (it == g_khost_dev_to_dev_idx_map.end()) {
        return -1;
    }
    return it->second;
}
