/**
 * @file      KnMsg.cpp
 * @brief     implementation file for msg class
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "KnMsg.h"
#include "KnLog.h"

#ifdef __linux__
#include <arpa/inet.h>
#elif _WIN32
#include <windows.h>
#endif

#include <string.h>

#define KN_RSP_OFFSET  0x8000

/*
 * Utility API for create an appropriate response message.
 * It is public static function. It can be called outside.
 */

KnBaseMsg* KnBaseMsg::create_rsp_msg(int8_t* buf, int mlen) {
    KnMsgHdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    int hdr_len = sizeof(KnMsgHdr);

    if( !buf || mlen < hdr_len ) {
        KnPrint(KN_LOG_ERR, "invalid buffer given for create msg %d.\n", mlen);
        return NULL;
    }

    if(decode_msg_hdr(&hdr, buf, hdr_len) < 0) {
        KnPrint(KN_LOG_ERR, "decode hdr failed.\n");
        return NULL;
    }

    KnBaseMsg* p_msg = NULL;

    if(hdr.msg_type == 0) {
        KnPrint(KN_LOG_ERR, "invalid msg type received %d.\n", hdr.msg_type);
        return NULL;
    }

    if( ((int)hdr.msg_len + hdr_len) != mlen) {
        KnPrint(KN_LOG_ERR, "msg len specified %d, act:%d.\n", hdr.msg_len, mlen);
        return NULL;
    }

    KnPrint(KN_LOG_DBG, "creating response msg: %d.\n", hdr.msg_type);
    if( hdr.msg_type == CMD_ACK_NACK ) { //generic ack
        p_msg = new KnImgTransAck(buf, mlen);
        return p_msg;
    }

    uint16_t msg_type = hdr.msg_type;
    if(msg_type & KN_RSP_OFFSET) msg_type -= KN_RSP_OFFSET;

    switch(msg_type) {
    case CMD_MEM_READ:
        p_msg = new KnTestMemReadRsp(buf, mlen);
        break;
    case CMD_MEM_WRITE:
        p_msg = new KnTestMemWriteRsp(buf, mlen);
        break;
    case CMD_ACK_NACK:
        p_msg = new KnGenericAckRsp(buf, mlen);
        break;
    case CMD_MEM_CLR:
        p_msg = new KnTestMemClearRsp(buf, mlen);
        break;
    case CMD_TEST_ECHO:
        p_msg = new KnTestHostEchoRsp(buf, mlen);
        break;
    case CMD_SFID_START:
        p_msg = new KnSFIDStartRsp(buf, mlen);
        break;
    case CMD_SFID_NEW_USER:
        p_msg = new KnSFIDewUserRsp(buf, mlen);
        break;
    case CMD_SFID_ADD_DB:
        p_msg = new KnSFIDRegisterRsp(buf, mlen);
        break;
    case CMD_SFID_DELETE_DB:
        p_msg = new KnSFIDDelDBRsp(buf, mlen);
        break;
    case CMD_SFID_EDIT_DB:
        p_msg = new KnSFIDEditDBRsp(buf, mlen);
        break;
    case CMD_SFID_SEND_IMAGE:
        p_msg = new KnSFIDSendImageRsp(buf, mlen);
        break;
    case CMD_SFID_DB_CONFIG:
        p_msg = new KnSFIDDbConfigRsp(buf, mlen);
        break;
    case CMD_RESET:
        p_msg = new KnCmdResetRsp(buf, mlen);
        break;
    case CMD_SYSTEM_STATUS:
        p_msg = new KnCmdSysStsRsp(buf, mlen);
        break;
    case CMD_GET_KN_NUM:
        p_msg = new KnCmdGetKnNumRsp(buf, mlen);
        break;
    case CMD_GET_MODEL_INFO:
        p_msg = new KnCmdGetModelInfoRsp(buf, mlen);
        break;
    case CMD_SET_CKEY:
        p_msg = new KnCmdSetCkeyRsp(buf, mlen);
        break;
    case CMD_SET_SBT_KEY:
        p_msg = new KnCmdSetSbtKeyRsp(buf, mlen);
        break;
    case CMD_GET_CRC:
        p_msg = new KnCmdGetCrcRsp(buf, mlen);
        break;
    case CMD_UPDATE_FW:
        p_msg = new KnCmdUdtFmRsp(buf, mlen);
        break;
    case CMD_UPDATE_MODEL:
        p_msg = new KnCmdUdtModelRsp(buf, mlen);
        break;
    case CMD_RESERVED:
        p_msg = new KnCmdUdtSplRsp(buf, mlen);
        break;
    case CMD_ISI_START:
        p_msg = new KnIsiStartRsp(buf, mlen);
        break;
    case CMD_ISI_SEND_IMAGE:
        p_msg = new KnIsiImgRsp(buf, mlen);
        break;
    case CMD_ISI_GET_RESULTS:
        p_msg = new KnIsiResultsRsp(buf, mlen);
        break;
    case CMD_ISI_CONFIG:
        p_msg = new KnIsiConfigRsp(buf, mlen);
        break;
    case CMD_DME_START:
        p_msg = new KnStartDmeRsp(buf, mlen);
        break;
    case CMD_DME_SEND_IMAGE:
        p_msg = new KnDmeImgRsp(buf, mlen);
        break;
    case CMD_DME_CONFIG:
        p_msg = new KnDmeConfigRsp(buf, mlen);
        break;
    case CMD_DME_GET_STATUS:
        p_msg = new KnDmeStatusRsp(buf, mlen);
        break;

    case CMD_JPEG_ENC_CONFIG:
        p_msg = new KnJpegEncConfigRsp(buf, mlen);
        break;

    case CMD_JPEG_ENC:
        p_msg = new KnJpegEncStartRsp(buf, mlen);
        break;

    case CMD_JPEG_ENC_GET_RESULT:
        p_msg = new KnJpegEncResultsRsp(buf, mlen);
        break;

    case CMD_JPEG_DEC_CONFIG:
        p_msg = new KnJpegDecConfigRsp(buf, mlen);
        break;

    case CMD_JPEG_DEC:
        p_msg = new KnJpegDecStartRsp(buf, mlen);
        break;

    case CMD_JPEG_DEC_GET_RESULT:
        p_msg = new KnJpegDecResultsRsp(buf, mlen);
        break;

    default:
        KnPrint(KN_LOG_ERR, "invalid msg type for create kn msg:%d.\n", hdr.msg_type);
    }

    return p_msg;
}

/*
 * Utility API for decode message header from incoming stream.
 * It is public static function. It can be called outside.
 */
int KnBaseMsg::decode_msg_hdr(KnMsgHdr* hdr, int8_t* buf, int mlen) {
    if(hdr == NULL || buf == NULL) return -1;

    if(mlen < (int)sizeof(KnMsgHdr)) return -1;

    KnMsgHdr* tmp = (KnMsgHdr*)buf;

 //   hdr->msg_type = ntohs(tmp->msg_type);
 //   hdr->msg_len = ntohs(tmp->msg_len);
    hdr->msg_type = (tmp->msg_type);
    hdr->msg_len = (tmp->msg_len);

    return 0;
}

/* 
 * Utility API for encode message header to byte stream for sending 
 * It is public static function. It can be called outside.
 */
int KnBaseMsg::encode_msg_hdr(KnMsgHdr* hdr, int8_t* buf, int mlen) {
    if(hdr == NULL || buf == NULL) return -1;

    if(mlen < (int)sizeof(KnMsgHdr)) return -1;

    uint16_t t = (hdr->msg_type);
    uint16_t l = (hdr->msg_len);

    //uint16_t t = htons(hdr->msg_type);
    //uint16_t l = htons(hdr->msg_len);

    memcpy((void*)buf, (void*)&t, sizeof(uint16_t));
    memcpy((void*)(buf+sizeof(uint16_t)), (void*)&l, sizeof(uint16_t));

    return 0;
}

//map msg type to appropriate request type.
//it could be same as request or request + 0x8000
uint16_t KnBaseMsg::get_req_type(uint16_t msg_type) {
    if(msg_type & KN_RSP_OFFSET) {
        return msg_type - KN_RSP_OFFSET;
    } else {
        return msg_type;
    }
}

//decode header from msg internal payload 
//the incoming byte stream neeed to be copied first
int KnBaseMsg::decode_hdr() {
    if(msg_payload == NULL) return -1;

    return decode_msg_hdr(&hdr, msg_payload, sizeof(KnMsgHdr));
}

//encode header to msg internal payload
//the hdr struct need to be set first. and payload buffer is ready
int KnBaseMsg::encode_hdr() {
    if(msg_payload == NULL) return -1;

    return encode_msg_hdr(&hdr, msg_payload, sizeof(KnMsgHdr));
}

//constructor with msg type and length
KnBaseMsg::KnBaseMsg(uint16_t msg_type, uint16_t msg_len) : _valid(false) {
    hdr.msg_type = msg_type;
    hdr.msg_len = msg_len;

    int tot_len = hdr.msg_len + sizeof(KnMsgHdr);
    msg_payload = new int8_t[tot_len];
    memset((void*)msg_payload, 0, tot_len);

    if(encode_hdr() < 0) {
        KnPrint(KN_LOG_ERR, "encode hdr failed: %d, %d.\n", msg_type, msg_len);
        return;
    }
    _valid = true;
}

//constructor with incoming byte stream 
KnBaseMsg::KnBaseMsg(int8_t* buf, int mlen) : _valid(false) {
    msg_payload = NULL;
    if(!buf) return;
    if(mlen < (int)sizeof(KnMsgHdr)) return;

    if(decode_msg_hdr(&hdr, buf, mlen) < 0) {
        KnPrint(KN_LOG_ERR, "decode hdr failed: %d.\n", mlen);
        return;
    }

    int tot_len = hdr.msg_len + sizeof(hdr);
    if(mlen >= tot_len) {
        msg_payload = new int8_t[tot_len];
        memcpy((void*)msg_payload, (void*)buf, tot_len);

        //change msg payload to host format
        KnMsgHdr* p = (KnMsgHdr*)msg_payload;
        p->msg_type = hdr.msg_type;
        p->msg_len = hdr.msg_len;
        _valid = true;
    } else {
        KnPrint(KN_LOG_ERR, "invalid msg len received:(hdr: %d, tot: %d).\n", hdr.msg_len, mlen);
        msg_payload = NULL;
    }
}

//destructor
KnBaseMsg::~KnBaseMsg() {
    if(msg_payload) {
        delete[] msg_payload; //free the payload
        msg_payload = NULL;
    }
}

//----------------------request message start-----------------------

KnGenericReq::KnGenericReq(uint16_t msg_type, uint16_t msg_len) \
        : KnBaseMsg(msg_type, msg_len) {
}

KnGenericReq::~KnGenericReq() {
}

KnTestMemReadReq::KnTestMemReadReq(uint16_t msg_type, uint16_t msg_len, uint32_t start_addr, uint32_t num) \
        : KnGenericReq(msg_type, msg_len), _start_addr(start_addr), _num_bytes(num) {
    //uint32_t s = htonl(_start_addr);
    uint32_t s = (_start_addr);
    //uint32_t n = htonl(_num_bytes);
    uint32_t n = (_num_bytes);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&s, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&n, sizeof(uint32_t));
    off += sizeof(uint32_t);

    if(off - sizeof(KnMsgHdr) > msg_len) {
        KnPrint(KN_LOG_ERR, "invalid msg len received:(given: %d, act: %d).\n", msg_len, off);
    }
}

KnTestMemReadReq::~KnTestMemReadReq() {
}

KnTestMemWriteReq::KnTestMemWriteReq(uint16_t msg_type, uint16_t msg_len, uint32_t start_addr, uint32_t num, \
        int8_t* pdata, int len) : KnTestMemReadReq(msg_type, msg_len, start_addr, num) {
    _p_data = msg_payload + 8 + sizeof(KnMsgHdr);
    memcpy((void*)_p_data, pdata, len);
    _len = msg_len - 8;

    if(_len != len) {
        KnPrint(KN_LOG_ERR, "invalid msg len received:(given: %d, act: %d).\n", len, _len);
    }
}

KnTestMemWriteReq::~KnTestMemWriteReq() {
    //_p_data re-use payload mem, no need to free
}

KnCmdResetReq::KnCmdResetReq(uint16_t msg_type, uint16_t msg_len, uint32_t r_mode, uint32_t chk_code) \
        : KnGenericReq(msg_type, msg_len), _rset_mode(r_mode), _check_code(chk_code) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t) + sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for reset requsest received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t r = (_rset_mode);
    //uint32_t r = htonl(_rset_mode);
    uint32_t c = (_check_code);
    //uint32_t c = htonl(_check_code);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&c, sizeof(uint32_t));
    off += sizeof(uint32_t);
}

KnCmdResetReq::~KnCmdResetReq() {
}

KnReportSysStsReq::KnReportSysStsReq(uint16_t msg_type, uint16_t msg_len) \
        : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
}

KnReportSysStsReq::~KnReportSysStsReq() {
}

KnCmdGetModelInfoReq::KnCmdGetModelInfoReq(uint16_t msg_type, uint16_t msg_len, uint32_t from_ddr) \
        : KnGenericReq(msg_type, msg_len), _from_ddr(from_ddr) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for model info request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t r = (_from_ddr);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
}

KnCmdGetModelInfoReq::~KnCmdGetModelInfoReq() {
}

KnCmdSetCkeyReq::KnCmdSetCkeyReq(uint16_t msg_type, uint16_t msg_len, uint32_t ckey) \
        : KnGenericReq(msg_type, msg_len), _ckey(ckey) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for ckey request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t r = (_ckey);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
}

KnCmdSetCkeyReq::~KnCmdSetCkeyReq() {
}

KnCmdSetSbtKeyReq::KnCmdSetSbtKeyReq(uint16_t msg_type, uint16_t msg_len, uint32_t entry, uint32_t key) \
        : KnGenericReq(msg_type, msg_len), _entry(entry), _key(key) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for sbt key request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t e = (_entry);
    uint32_t k = (_key);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&e, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&k, sizeof(uint32_t));
    off += sizeof(uint32_t);
}

KnCmdSetSbtKeyReq::~KnCmdSetSbtKeyReq() {
}

KnCmdGetCrcReq::KnCmdGetCrcReq(uint16_t msg_type, uint16_t msg_len, uint32_t from_ddr) \
        : KnGenericReq(msg_type, msg_len), _from_ddr(from_ddr) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for crc request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t r = (_from_ddr);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
}

KnCmdGetCrcReq::~KnCmdGetCrcReq() {
}

KnCmdUdtFmReq::KnCmdUdtFmReq(uint16_t msg_type, uint16_t msg_len, uint32_t module_id) \
        : KnGenericReq(msg_type, msg_len), _module_id(module_id) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t) + sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for udt firmware request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t r = (_module_id);
    //uint32_t r = htonl(_rset_mode);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
}

KnCmdUdtFmReq::~KnCmdUdtFmReq() {
}

KnCmdUdtModelReq::KnCmdUdtModelReq(uint16_t msg_type, uint16_t msg_len, uint32_t md_id, uint32_t md_size) \
        : KnGenericReq(msg_type, msg_len), _model_id(md_id), _model_size(md_size) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t) + sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for udt model request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t r = (_model_id);
    //uint32_t r = htonl(_rset_mode);
    uint32_t c = (_model_size);
    //uint32_t c = htonl(_check_code);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&c, sizeof(uint32_t));
    off += sizeof(uint32_t);
}

KnCmdUdtModelReq::~KnCmdUdtModelReq() {
}

KnCmdUdtSplReq::KnCmdUdtSplReq(uint16_t msg_type, uint16_t msg_len, uint32_t mode, \
    uint16_t auth_type, uint16_t size, uint8_t* auth_key) \
        : KnGenericReq(msg_type, msg_len), _mode(mode), _auth_type(auth_type), \
                _size(size),  _auth_key(auth_key) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)*3) {
        KnPrint(KN_LOG_ERR, "invalid msg len for udt spl request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t m = (_mode);
    uint16_t t = (_auth_type);
    uint16_t z = (_size);
    uint8_t* k = (_auth_key);
    int kz = 0;

    if (m == 0) {
        z = 0;
        t = 0;
    }
    if (t > 1) {
        KnPrint(KN_LOG_ERR, "invalid auth type for udt spl request received: %d.\n", auth_type);
        _valid = false;
        return;
    }
    else {
        kz = 4;
    }
    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&m, sizeof(uint32_t));
    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&t, sizeof(uint16_t));
    off += sizeof(uint16_t);
    memcpy((void*)(msg_payload + off), (void*)&z, sizeof(uint16_t));
    off += sizeof(uint16_t);
    memcpy((void*)(msg_payload + off), (void*)k, kz);
}

KnCmdUdtSplReq::~KnCmdUdtSplReq() {
}

KnOprSelectAppReq::KnOprSelectAppReq(uint16_t msg_type, uint16_t msg_len, uint32_t id) \
        : KnGenericReq(msg_type, msg_len), _app_id(id) {
    //uint32_t a = htonl(_app_id);
    uint32_t a = (_app_id);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&a, sizeof(uint32_t));
    off += sizeof(uint32_t);

    if(off - sizeof(KnMsgHdr) > msg_len) {
        KnPrint(KN_LOG_ERR, "invalid msg len received:(given: %d, act: %d).\n", msg_len, off);
    }
}

KnOprSelectAppReq::~KnOprSelectAppReq() {
}

KnSFIDStartReq::KnSFIDStartReq(uint16_t msg_type, uint16_t msg_len, float r,
                uint16_t width, uint16_t height, uint32_t format) \
        : KnGenericReq(msg_type, msg_len), _fr_thresh(r), \
                _wid(width), _ht(height), _fmt(format) {
    if(!_valid) return;
    if( msg_len < sizeof(float) + sizeof(uint32_t) + sizeof(uint16_t)*2 + sizeof(uint32_t) ) {
        KnPrint(KN_LOG_ERR, "invalid msg len for start sfid received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    float t = (_fr_thresh);
    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&t, sizeof(float));

    off += sizeof(float) + sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&_wid, sizeof(uint16_t));

    off += sizeof(uint16_t);
    memcpy((void*)(msg_payload + off), (void*)&_ht, sizeof(uint16_t));

    off += sizeof(uint16_t);
    memcpy((void*)(msg_payload + off), (void*)&_fmt, sizeof(uint32_t));

}

KnSFIDStartReq::~KnSFIDStartReq() {
}


KnSFIDNewUserReq::KnSFIDNewUserReq(uint16_t msg_type, uint16_t msg_len, uint16_t u_id, uint16_t img_idx) \
        : KnGenericReq(msg_type, msg_len), _user_id(u_id), _img_idx(img_idx) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t) + sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for new user reg received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    //uint16_t u = htons(_user_id);
    uint16_t u = (_user_id);
    //uint16_t i = htons(_img_idx);
    uint16_t i = (_img_idx);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&u, sizeof(uint16_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&i, sizeof(uint16_t));
}

KnSFIDNewUserReq::~KnSFIDNewUserReq() {
}

KnSFIDRegisterReq::KnSFIDRegisterReq(uint16_t msg_type, uint16_t msg_len, uint32_t u_id) \
        : KnGenericReq(msg_type, msg_len), _user_id(u_id) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for register req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    //uint32_t u = htonl(_user_id);
    uint32_t u = (_user_id);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&u, sizeof(uint32_t));
}

KnSFIDRegisterReq::~KnSFIDRegisterReq() {
}


KnSFIDSendImageReq::KnSFIDSendImageReq(uint16_t msg_type, uint16_t msg_len, uint32_t fsize, uint16_t out_mask) \
        : KnGenericReq(msg_type, msg_len), _f_size(fsize), _out_mask(out_mask), _r0(0) {
    if(!_valid) return;
    if(msg_len < (sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) )) {
        KnPrint(KN_LOG_ERR, "invalid msg len for new user reg received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t u = (_f_size);
    uint16_t i = (_out_mask);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&u, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&i, sizeof(uint16_t));
}

KnSFIDSendImageReq::~KnSFIDSendImageReq() {
}

KnSFIDEditDBReq::KnSFIDEditDBReq(uint16_t msg_type, uint16_t msg_len, uint32_t mode, uint32_t u_id, uint32_t f_id) \
        : KnGenericReq(msg_type, msg_len), \
          _mode(mode), _user_id(u_id), _face_id(f_id) {
    if(!_valid) return;
    if(msg_len < (2 * sizeof(uint32_t) + 2 * sizeof(uint16_t))) {
        KnPrint(KN_LOG_ERR, "invalid msg len for edit db request received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t dummy = 0;
    uint32_t user_info = 0;

    if (6/* DB_QUERY_FM */ == _mode) {
        ((uint16_t *)&user_info)[0] = _user_id;
        ((uint16_t *)&user_info)[1] = _face_id;
    } else {
        user_info = u_id;
    }
    
    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_mode, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&dummy, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&user_info, sizeof(uint32_t));
}

KnSFIDEditDBReq::~KnSFIDEditDBReq() {
}

KnSFIDDbConfigReq::KnSFIDDbConfigReq(uint16_t msg_type, uint16_t msg_len, uint32_t mode, kapp_db_config_parameter_t* db_config_parameter)
        : KnGenericReq(msg_type, msg_len), _mode(mode), _db_config_parameter(db_config_parameter) {
    if(!_valid) return;
    if(msg_len < (sizeof(uint32_t) + sizeof(kapp_db_config_parameter_t))) {
        KnPrint(KN_LOG_ERR, "invalid msg len for config db reg received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_mode, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)_db_config_parameter, sizeof(kapp_db_config_parameter_t));
}

KnSFIDDbConfigReq::~KnSFIDDbConfigReq() {
}

KnIsiStartReq::KnIsiStartReq(uint16_t msg_type, uint16_t msg_len, uint32_t app_id, uint32_t rt_size, uint16_t width, uint16_t height, uint32_t format) : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)*4) {
        KnPrint(KN_LOG_ERR, "invalid msg len for start isi req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t i = (app_id);
    uint32_t z = (rt_size);
    uint16_t w = (width);
    uint16_t h = (height);
    uint32_t f = (format);

    int off = sizeof(KnMsgHdr);
    // For app id
    memcpy((void*)(msg_payload + off), (void*)&i, sizeof(uint32_t));
    off += sizeof(uint32_t);
    // For return size
    memcpy((void*)(msg_payload + off), (void*)&z, sizeof(uint32_t));
    off += sizeof(uint32_t);
    // For width
    memcpy((void*)(msg_payload + off), (void*)&w, sizeof(uint16_t));
    off += sizeof(uint16_t);
    // For height
    memcpy((void*)(msg_payload + off), (void*)&h, sizeof(uint16_t));
    off += sizeof(uint16_t);
    // For format
    memcpy((void*)(msg_payload + off), (void*)&f, sizeof(uint32_t));
}

KnIsiStartReq::~KnIsiStartReq() {
}

KnIsiStartExtReq::KnIsiStartExtReq(uint16_t msg_type, uint16_t msg_len, \
        char* data) : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
    if(msg_len <= sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi config req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    int off = sizeof(KnMsgHdr);

    memcpy((void*)(msg_payload + off), (void*)data, msg_len);
}

KnIsiStartExtReq::~KnIsiStartExtReq() {
}

KnIsiImageReq::KnIsiImageReq(uint16_t msg_type, uint16_t msg_len, uint32_t fsize, uint32_t img_id) : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)*3) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi image req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t f = (fsize);
    uint32_t n = (img_id);
    uint32_t r = 0;  // reserved

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&f, sizeof(uint32_t));
    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&n, sizeof(uint32_t));
    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
}

KnIsiImageReq::~KnIsiImageReq() {
}

KnIsiResultsReq::KnIsiResultsReq(uint16_t msg_type, uint16_t msg_len, uint32_t img_id) : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)*2) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi image req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t n = (img_id);
    uint32_t r = 0;  // reserved

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&n, sizeof(uint32_t));
    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
}

KnIsiResultsReq::~KnIsiResultsReq() {
}

KnIsiConfigReq::KnIsiConfigReq(uint16_t msg_type, uint16_t msg_len, uint32_t model_id, uint32_t param) : KnGenericReq(msg_type, msg_len) {
    if (!_valid) return;
    if (msg_len < sizeof(uint32_t) * 2) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi config: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t n = model_id;
    uint32_t r = param;

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&n, sizeof(uint32_t));
    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&r, sizeof(uint32_t));
}

KnIsiConfigReq::~KnIsiConfigReq() {
}

KnStartDmeReq::KnStartDmeReq(uint16_t msg_type, uint16_t msg_len, uint32_t mod_size, \
        char* data, int dat_size) : KnGenericReq(msg_type, msg_len), _model_size(mod_size) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for start dme req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t u = (mod_size);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&u, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)data, dat_size);
}

KnStartDmeReq::~KnStartDmeReq() {
}

KnDmeConfigReq::KnDmeConfigReq(uint16_t msg_type, uint16_t msg_len, \
        char* data, int dat_size) : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
    if(msg_len <= sizeof(uint32_t)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for dme config req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    int off = sizeof(KnMsgHdr);

    memcpy((void*)(msg_payload + off), (void*)data, dat_size);
}

KnDmeConfigReq::~KnDmeConfigReq() {
}

KnDmeImageReq::KnDmeImageReq(uint16_t msg_type, uint16_t msg_len, uint32_t fsize, uint16_t mode, uint16_t model_id) \
        : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)*2) {
        KnPrint(KN_LOG_ERR, "invalid msg len for dme image req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t u = (fsize);
    uint16_t i = (mode);
    uint16_t n = (model_id);

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&u, sizeof(uint32_t));
    off += sizeof(uint32_t);
    // For mode
    memcpy((void*)(msg_payload + off), (void*)&i, sizeof(uint16_t));
    off += sizeof(uint16_t);
    // For model id
    memcpy((void*)(msg_payload + off), (void*)&n, sizeof(uint16_t));
}

KnDmeImageReq::~KnDmeImageReq() {
}

KnDmeStatusReq::KnDmeStatusReq(uint16_t msg_type, uint16_t msg_len, uint16_t ssid) \
        : KnGenericReq(msg_type, msg_len) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)*2) {
        KnPrint(KN_LOG_ERR, "invalid msg len for dme get req received: %d.\n", msg_len);
        _valid = false;
        return;
    }
    uint16_t i = (ssid);
    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&i, sizeof(uint16_t));
}

KnDmeStatusReq::~KnDmeStatusReq() {
}


//request message to configure image parameters for jpeg encoding 
KnJpegEncCfgReq::KnJpegEncCfgReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq,
                           uint16_t width, uint16_t height, uint32_t format, uint32_t quality):KnGenericReq(msg_type, msg_len)
{
    if(!_valid) return;
    if(msg_len != sizeof(uint32_t)*5)   // 4 elements + param1 + param2
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg enc cfg req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    _seq_num = (uint32_t)seq;
    _fmt = (uint32_t)format;
    _wid = (uint32_t)width;
    _ht = (uint32_t)height;
	_quality = (uint32_t)quality;

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_seq_num, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_fmt, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_wid, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_ht, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_quality, sizeof(uint32_t));
}

KnJpegEncCfgReq::~KnJpegEncCfgReq()
{
}

//request message to start jpeg encoding
KnJpegEncStartReq::KnJpegEncStartReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq, uint32_t len):KnGenericReq(msg_type, msg_len)
{
    if(!_valid) return;
    if(msg_len != sizeof(uint32_t)*2)
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg encoding req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    _seq_num = (uint32_t)seq;
    _buf_len = (uint32_t)len;

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_seq_num, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_buf_len, sizeof(uint32_t));

}

KnJpegEncStartReq::~KnJpegEncStartReq()
{
}

//request message to retrieve jpeg enc result
KnJpegEncResultsReq::KnJpegEncResultsReq(uint16_t msg_type, uint16_t msg_len, uint32_t img_id):KnGenericReq(msg_type, msg_len)
{
    if(!_valid) return;
    if(msg_len != sizeof(uint32_t)*3)
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for retrieving jpeg enc result req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    _img_id = img_id;

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_img_id, sizeof(uint32_t));

}

KnJpegEncResultsReq::~KnJpegEncResultsReq()
{
};

//request message to configure image parameters for jpeg decoding
KnJpegDecCfgReq::KnJpegDecCfgReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq,
                           uint16_t width, uint16_t height, uint32_t format, uint32_t len):KnGenericReq(msg_type, msg_len)
{
    if(!_valid) return;
    if(msg_len != sizeof(uint32_t)*5)   // 4 elements + param1 + param2
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg dec cfg req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    _seq_num = (uint32_t)seq;
    _fmt = (uint32_t)format;
    _wid = (uint32_t)width;
    _ht = (uint32_t)height;
    _len = (uint32_t)len;

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_seq_num, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_fmt, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_wid, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_ht, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_len, sizeof(uint32_t));
}

KnJpegDecCfgReq::~KnJpegDecCfgReq()
{
}

//request message to start jpeg decoding
KnJpegDecStartReq::KnJpegDecStartReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq, uint32_t len):KnGenericReq(msg_type, msg_len)
{
    if(!_valid) return;
    if(msg_len != sizeof(uint32_t)*2)
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg dec start req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    _seq_num = (uint32_t)seq;
    _buf_len = (uint32_t)len;

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_seq_num, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)(msg_payload + off), (void*)&_buf_len, sizeof(uint32_t));

}

KnJpegDecStartReq::~KnJpegDecStartReq()
{
}

// request msg to retrieve jpeg decoding result
KnJpegDecResultsReq::KnJpegDecResultsReq(uint16_t msg_type, uint16_t msg_len, uint32_t img_id):KnGenericReq(msg_type, msg_len)
{
    if(!_valid) return;
    if(msg_len != sizeof(uint32_t)*3)
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg dec result retrieving req received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    _img_id = img_id;

    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&_img_id, sizeof(uint32_t));

}

KnJpegDecResultsReq::~KnJpegDecResultsReq()
{
};

//----------------------request message end-----------------------

KnImgTransAck::KnImgTransAck(int8_t* buf, int mlen) : KnBaseMsg(buf, mlen) {
    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _action = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_action, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _src_id = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_src_id, sizeof(uint32_t));
}

KnImgTransAck::KnImgTransAck(uint16_t msg_type, uint16_t msg_len, uint32_t action, \
        uint32_t src_id) : KnBaseMsg(msg_type, msg_len), _action(action), _src_id(src_id) {
    if(!_valid) return;
    if(msg_len < sizeof(uint32_t)*2) {
        KnPrint(KN_LOG_ERR, "invalid msg len for image trans ack received: %d.\n", msg_len);
        _valid = false;
        return;
    }

    uint32_t u = (_action);
    int off = sizeof(KnMsgHdr);
    memcpy((void*)(msg_payload + off), (void*)&u, sizeof(uint32_t));

    u = _src_id;
    off += sizeof(uint32_t);
    memcpy((void*)(msg_payload + off), (void*)&u, sizeof(uint32_t));
}

KnImgTransAck::~KnImgTransAck() {
}

//----------------------response message start-----------------------

KnGenericAckRsp::KnGenericAckRsp(int8_t* buf, int mlen) \
        : KnBaseMsg(buf, mlen) {
}

KnGenericAckRsp::~KnGenericAckRsp() {
}

KnTestMemAck::KnTestMemAck(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _ack = (tmp);
    //_ack = ntohl(tmp);

    memcpy((void*)(msg_payload+off), (void*)&_ack, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _reserved = (tmp);
    //_reserved = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_reserved, sizeof(uint32_t));
}

KnTestMemAck::~KnTestMemAck() {
}

KnTestMemWriteRsp::KnTestMemWriteRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_result = ntohl(tmp);
    _rsp_result = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_result, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_num_bytes = ntohl(tmp);
    _num_bytes = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_num_bytes, sizeof(uint32_t));
}

KnTestMemWriteRsp::~KnTestMemWriteRsp() {
}

KnTestMemReadRsp::KnTestMemReadRsp(int8_t* buf, int mlen) : KnTestMemWriteRsp(buf, mlen) {
    int off = sizeof(KnMsgHdr) + 8;
    _p_data = msg_payload + off;
    _len = hdr.msg_len - 8;
}

KnTestMemReadRsp::~KnTestMemReadRsp() {
    //_p_data re-use payload mem, no need to free
}

KnCmdResetRsp::KnCmdResetRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for command reset rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_err_code = ntohl(tmp);
    _err_code = (tmp);

    memcpy((void*)(msg_payload+off), (void*)&_err_code, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_reserved = ntohl(tmp);
    _reserved = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_reserved, sizeof(uint32_t));
}

KnCmdResetRsp::~KnCmdResetRsp() {
}

KnCmdSysStsRsp::KnCmdSysStsRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) \
            + sizeof(uint16_t)*2 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for report sys sts received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _sfw_id = (tmp);

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _sbuild_id = (tmp);

    off += sizeof(uint32_t);

    uint16_t t16;
    memcpy((void*)&t16, (void*)(msg_payload+off), sizeof(uint16_t));
    _sys_sts = t16; //endian switch

    off += sizeof(uint16_t);

    memcpy((void*)&t16, (void*)(msg_payload+off), sizeof(uint16_t));
    _app_sts = t16; //endian

    off += sizeof(uint16_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _nfw_id = (tmp);

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _nbuild_id = (tmp);
}


KnCmdSysStsRsp::~KnCmdSysStsRsp() {
}

KnCmdGetKnNumRsp::KnCmdGetKnNumRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) +  sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for KnCmdGetKnNumRsp %d\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(buf+off), sizeof(uint32_t));
    kn_num = tmp;
}

KnCmdGetKnNumRsp::~KnCmdGetKnNumRsp() {
}

KnCmdSetCkeyRsp::KnCmdSetCkeyRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) +  sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for KnCmdSetCkeyRsp %d\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(buf+off), sizeof(uint32_t));
    set_status = tmp;
}

KnCmdSetCkeyRsp::~KnCmdSetCkeyRsp() {
}

KnCmdSetSbtKeyRsp::KnCmdSetSbtKeyRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) +  sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for KnCmdSetSbtKeyRsp %d\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(buf+off), sizeof(uint32_t));
    set_status = tmp;
}

KnCmdSetSbtKeyRsp::~KnCmdSetSbtKeyRsp() {
}

KnCmdGetModelInfoRsp::KnCmdGetModelInfoRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) +  sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for KnCmdGetModelInfoRsp %d\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(buf+off), sizeof(uint32_t));
    data_size = tmp;
}

KnCmdGetCrcRsp::~KnCmdGetCrcRsp() {
}

KnCmdGetCrcRsp::KnCmdGetCrcRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) +  sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for KnCmdGetCrcRsp %d\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(buf+off), sizeof(uint32_t));
    data_size = tmp;
}

KnCmdGetModelInfoRsp::~KnCmdGetModelInfoRsp() {
}

KnCmdUdtFmRsp::KnCmdUdtFmRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for command udt firmware rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_err_code = ntohl(tmp);
    _rsp_code = (tmp);

    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_reserved = ntohl(tmp);
    _module_id = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_module_id, sizeof(uint32_t));
}

KnCmdUdtFmRsp::~KnCmdUdtFmRsp() {
}

KnCmdUdtModelRsp::KnCmdUdtModelRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for command udt firmware rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);

    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_reserved = ntohl(tmp);
    _model_id = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_model_id, sizeof(uint32_t));
}

KnCmdUdtModelRsp::~KnCmdUdtModelRsp() {
}

KnCmdUdtSplRsp::KnCmdUdtSplRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t)*3 + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for command udt spl rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _spl_id = (tmp);
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _spl_build = (tmp);
}

KnCmdUdtSplRsp::~KnCmdUdtSplRsp() {
}

KnSFIDStartRsp::KnSFIDStartRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for sfid start rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_size = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_size, sizeof(uint32_t));
}

KnSFIDStartRsp::~KnSFIDStartRsp() {
}

KnSFIDSendImageRsp::KnSFIDSendImageRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < (sizeof(uint32_t) + sizeof(uint32_t)*2 + sizeof(KnMsgHdr) )) {
        KnPrint(KN_LOG_ERR, "invalid msg len for send img rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    //_rsp_code = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);

    uint16_t t16;
    memcpy((void*)&t16, (void*)(msg_payload+off), sizeof(uint16_t));
    _mode = (t16);
    //_mode = ntohs(t16);
    memcpy((void*)(msg_payload+off), (void*)&_mode, sizeof(uint16_t));

    off += sizeof(uint16_t);
    memcpy((void*)&t16, (void*)(msg_payload+off), sizeof(uint16_t));
    //_uid = ntohs(t16);
    _uid = (t16);
    memcpy((void*)(msg_payload+off), (void*)&_uid, sizeof(uint16_t));

    //output flag
    off += sizeof(uint16_t);
    memcpy((void*)&t16, (void*)(msg_payload+off), sizeof(uint16_t));
    _out_flag = (t16);
    memcpy((void*)(msg_payload+off), (void*)&_out_flag, sizeof(uint16_t));

    off = RES_START_OFF;
    //fd result
    if(_out_flag & 0x01) {  //fd result
        if(mlen < off + FD_RES_LENGTH) {
            KnPrint(KN_LOG_ERR, "not received fd result: %d.\n", mlen);
            _valid = false;
            return;
        }
        memcpy((void*)_fd_res, (void*)(msg_payload+off), FD_RES_LENGTH);
        off += FD_RES_LENGTH;
    }

    //lm result
    if(_out_flag & 0x02) {  //lm result
        if(mlen < off + LM_RES_LENGTH) {
            KnPrint(KN_LOG_ERR, "not received lm result: %d.\n", mlen);
            _valid = false;
            return;
        }
        memcpy((void*)_lm_res, (void*)(msg_payload+off), LM_RES_LENGTH);
        off += LM_RES_LENGTH;
    }

    //fr result
    if(_out_flag & 0x04) {  //fr result
        if(mlen < off + FR_RES_LENGTH) {
            KnPrint(KN_LOG_ERR, "not received fr result: %d.\n", mlen);
            _valid = false;
            return;
        }
        memcpy((void*)_fr_res, (void*)(msg_payload+off), FR_RES_LENGTH);
        off += FR_RES_LENGTH;
    }

    //lv result
    if(_out_flag & 0x08) {  //lv result
        if(mlen < off + LV_RES_LENGTH) {
            KnPrint(KN_LOG_ERR, "not received liveness result: %d.\n", mlen);
            _valid = false;
            return;
        }
        memcpy((void*)&_live_res, (void*)(msg_payload+off), sizeof(uint16_t));
        off += LV_RES_LENGTH;
    }

    //score result
    if(_out_flag & 0x10) {  //score result
        if(mlen < off + SCORE_RES_LENGTH) {
            KnPrint(KN_LOG_ERR, "not received score result: %d.\n", mlen);
            _valid = false;
            return;
        }
        memcpy((void*)&_score_res, (void*)(msg_payload+off), SCORE_RES_LENGTH);
    }
}

KnSFIDSendImageRsp::~KnSFIDSendImageRsp() {
}

KnSFIDEditDBRsp::KnSFIDEditDBRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for KnSFIDEditDBRsp %d\n", mlen);
	    _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _data_size = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_data_size, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _data = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_data, sizeof(uint32_t));
}

KnSFIDEditDBRsp::~KnSFIDEditDBRsp() {
}

KnIsiStartRsp::KnIsiStartRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t)*2 + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi start rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _buf_size = (tmp);
}

KnIsiStartRsp::~KnIsiStartRsp() {
}

KnIsiImgRsp::KnIsiImgRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t)*3 + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi send image rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _image_available = (tmp);
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsv = (tmp);
}

KnIsiImgRsp::~KnIsiImgRsp() {
}

KnIsiResultsRsp::KnIsiResultsRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t)*3 + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi get results rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    off += sizeof(uint32_t)*2;  // skip the next field (img_id)
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _r_size = (tmp);
}

KnIsiResultsRsp::~KnIsiResultsRsp() {
}

KnIsiConfigRsp::KnIsiConfigRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if (!_valid) return;
    if ((uint32_t)mlen < sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for isi config rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload + off), sizeof(uint32_t));
    _rsp_code = (tmp);
}

KnIsiConfigRsp::~KnIsiConfigRsp() {
}

KnDmeImgRsp::KnDmeImgRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for sfid start dme rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _inf_mode = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_inf_mode, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _inf_size = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_inf_size, sizeof(uint32_t));
}

KnDmeImgRsp::~KnDmeImgRsp() {
}

KnDmeStatusRsp::KnDmeStatusRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {//sizeof(uint32_t) + 
        KnPrint(KN_LOG_ERR, "invalid msg len for sfid start dme rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }
    //printf(">>>>>>>>>>>KnDmeStatusRsp %d\n", mlen);
    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint16_t));
    _session_id = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_session_id, sizeof(uint16_t));

    off += sizeof(uint16_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint16_t));
    _inf_status = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_inf_status, sizeof(uint16_t));

    off += sizeof(uint16_t);

    if (_inf_status == 1) {
        memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
        _inf_size = (tmp);
        //_img_size = ntohl(tmp);
        memcpy((void*)(msg_payload+off), (void*)&_inf_size, sizeof(uint32_t));
    }
}

KnDmeStatusRsp::~KnDmeStatusRsp() {
}

KnStartDmeRsp::KnStartDmeRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for sfid start dme rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _inf_size = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_inf_size, sizeof(uint32_t));
}

KnStartDmeRsp::~KnStartDmeRsp() {
}

KnDmeConfigRsp::KnDmeConfigRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen) {
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr)) {
        KnPrint(KN_LOG_ERR, "invalid msg len for sfid dme config rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));
    
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _model_id = (tmp);
    
    memcpy((void*)(msg_payload+off), (void*)&_model_id, sizeof(uint32_t));
}

KnDmeConfigRsp::~KnDmeConfigRsp() {
}

/**********   Jpeg enc msgs *********/
KnJpegEncConfigRsp::KnJpegEncConfigRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen)
{
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr))
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg config rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_seq = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_seq, sizeof(uint32_t));
}

KnJpegEncConfigRsp :: ~KnJpegEncConfigRsp()
{
}

KnJpegEncStartRsp::KnJpegEncStartRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen)
{
    if(!_valid) return;
    if((uint32_t)mlen < 5*sizeof(uint32_t) + sizeof(KnMsgHdr))
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg encoding rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    off += sizeof(uint32_t) * 2;   //bypass param1/error_code and param2
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _availablity = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_availablity, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_seq = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_seq, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_buf = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_buf, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_len = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_len, sizeof(uint32_t));

}

KnJpegEncStartRsp :: ~KnJpegEncStartRsp()
{
}

KnJpegEncResultsRsp::KnJpegEncResultsRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen)
{
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t)*2 + sizeof(KnMsgHdr))
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg enc get results rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);

    off += sizeof(uint32_t) * 2;   //bypass param1/error_code and param2=img_seq

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _r_size = (tmp);
}

KnJpegEncResultsRsp::~KnJpegEncResultsRsp()
{
}

/**********   Jpeg dec msgs *********/
KnJpegDecConfigRsp::KnJpegDecConfigRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen)
{
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(KnMsgHdr))
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg dec config rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_seq = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_seq, sizeof(uint32_t));
}

KnJpegDecConfigRsp :: ~KnJpegDecConfigRsp()
{
}

KnJpegDecStartRsp::KnJpegDecStartRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen)
{
    if(!_valid) return;
    if((uint32_t)mlen < 5*sizeof(uint32_t) + sizeof(KnMsgHdr))
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg dec rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);
    off += sizeof(uint32_t) * 2;   //bypass param1/error_code and param2
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    //_rsp_code = ntohl(tmp);
    _rsp_code = (tmp);
    memcpy((void*)(msg_payload+off), (void*)&_rsp_code, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _availablity = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_availablity, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_seq = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_seq, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_buf = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_buf, sizeof(uint32_t));

    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _img_len = (tmp);
    //_img_size = ntohl(tmp);
    memcpy((void*)(msg_payload+off), (void*)&_img_len, sizeof(uint32_t));

}

KnJpegDecStartRsp :: ~KnJpegDecStartRsp()
{
}

KnJpegDecResultsRsp::KnJpegDecResultsRsp(int8_t* buf, int mlen) : KnGenericAckRsp(buf, mlen)
{
    if(!_valid) return;
    if((uint32_t)mlen < sizeof(uint32_t)*2 + sizeof(KnMsgHdr))
    {
        KnPrint(KN_LOG_ERR, "invalid msg len for jpeg dec get results rsp received: %d.\n", mlen);
        _valid = false;
        return;
    }

    uint32_t tmp;
    int off = sizeof(KnMsgHdr);

    off += sizeof(uint32_t) * 2;   //bypass param1/error_code and param2=img_seq

    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _rsp_code = (tmp);
    off += sizeof(uint32_t);
    memcpy((void*)&tmp, (void*)(msg_payload+off), sizeof(uint32_t));
    _r_size = (tmp);
}

KnJpegDecResultsRsp::~KnJpegDecResultsRsp()
{
}

//----------------------response message end-----------------------

