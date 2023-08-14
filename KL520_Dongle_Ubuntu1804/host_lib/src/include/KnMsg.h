/**
 * @file      KnMsg.h
 * @brief     Definition for base types
 * @version   0.1 - 2019-04-13
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __KN_MSG_H__
#define __KN_MSG_H__

#include <stdint.h>
#include "com.h"
#include "model_res.h"
#include "KdpHostApi.h"

/*
 * base class for messages
 */
class KnBaseMsg {
public:
    static KnBaseMsg* create_rsp_msg(int8_t* buf, int mlen);
    static int decode_msg_hdr(KnMsgHdr* hdr, int8_t* buf, int mlen);
    static int encode_msg_hdr(KnMsgHdr* hdr, int8_t* buf, int mlen);

    uint16_t get_msg_type() { return hdr.msg_type; }
    uint16_t get_msg_len() { return hdr.msg_len; }
    uint16_t get_req_type(uint16_t msg_type);
    int8_t*  get_msg_payload() { return msg_payload; }

    KnBaseMsg(uint16_t msg_type, uint16_t msg_len);
    KnBaseMsg(int8_t* buf, int mlen);
    virtual ~KnBaseMsg();

    bool is_msg_valid() { return _valid; }

protected:
    int decode_hdr();
    int encode_hdr();

    KnMsgHdr hdr;
    int8_t* msg_payload;
    bool    _valid;
};

//----------------------request message start-----------------------
//base class for req message
//only message type and message length
class KnGenericReq : public KnBaseMsg {
public:
    KnGenericReq(uint16_t msg_type, uint16_t msg_len);
    virtual ~KnGenericReq();
};

//request message for memory read request
class KnTestMemReadReq : public KnGenericReq {
public:
    KnTestMemReadReq(uint16_t msg_type, uint16_t msg_len, uint32_t start_addr, uint32_t num);
    virtual ~KnTestMemReadReq();
    uint32_t _start_addr;
    uint32_t _num_bytes;
};

//request message for memory write request
class KnTestMemWriteReq : public KnTestMemReadReq {
public:
    KnTestMemWriteReq(uint16_t msg_type, uint16_t msg_len, \
                uint32_t start_addr, uint32_t num, int8_t* pdata, int len);
    virtual ~KnTestMemWriteReq();
    int8_t*  _p_data;
    int      _len;
};

//mem clear request is same structure as read req
typedef KnTestMemReadReq KnTestMemClearReq;

//host echo request is same structure as write req
typedef KnTestMemWriteReq KnTestHostEchoReq;

//request message for reset 
class KnCmdResetReq : public KnGenericReq {
public:
    KnCmdResetReq(uint16_t msg_type, uint16_t msg_len, uint32_t rset_mode, uint32_t check_code);
    virtual ~KnCmdResetReq();
    uint32_t _rset_mode;
    uint32_t _check_code;
};

//request message for report sys status 
class KnReportSysStsReq : public KnGenericReq {
public:
    KnReportSysStsReq(uint16_t msg_type, uint16_t msg_len);
    virtual ~KnReportSysStsReq();
};

//request message for getting model info
class KnCmdGetModelInfoReq : public KnGenericReq {
public:
    KnCmdGetModelInfoReq(uint16_t msg_type, uint16_t msg_len, uint32_t from_ddr);
    virtual ~KnCmdGetModelInfoReq();
    uint32_t _from_ddr;
};

//request message for set ckey
class KnCmdSetCkeyReq : public KnGenericReq {
public:
    KnCmdSetCkeyReq(uint16_t msg_type, uint16_t msg_len, uint32_t ckey);
    virtual ~KnCmdSetCkeyReq();
    uint32_t _ckey;
};

//request message for set sbt key
class KnCmdSetSbtKeyReq : public KnGenericReq {
public:
    KnCmdSetSbtKeyReq(uint16_t msg_type, uint16_t msg_len, uint32_t entry, uint32_t key);
    virtual ~KnCmdSetSbtKeyReq();
    uint32_t _entry;
    uint32_t _key;
};

//request message for getting crc
class KnCmdGetCrcReq : public KnGenericReq {
public:
    KnCmdGetCrcReq(uint16_t msg_type, uint16_t msg_len, uint32_t from_ddr);
    virtual ~KnCmdGetCrcReq();
    uint32_t _from_ddr;
};

//request message for update firmware 
class KnCmdUdtFmReq : public KnGenericReq {
public:
    KnCmdUdtFmReq(uint16_t msg_type, uint16_t msg_len, uint32_t module_id);
    virtual ~KnCmdUdtFmReq();
    uint32_t _module_id;
    uint32_t _reserved;
};

//request message for update model 
class KnCmdUdtModelReq : public KnGenericReq {
public:
    KnCmdUdtModelReq(uint16_t msg_type, uint16_t msg_len, uint32_t md_id, uint32_t md_size);
    virtual ~KnCmdUdtModelReq();
    uint32_t _model_id;
    uint32_t _model_size;
};

//request message for update spl
class KnCmdUdtSplReq : public KnGenericReq {
public:
    KnCmdUdtSplReq(uint16_t msg_type, uint16_t msg_len, uint32_t mode, \
                     uint16_t auth_type, uint16_t size, uint8_t* auth_key);
    virtual ~KnCmdUdtSplReq();
    uint32_t _mode;
    uint16_t _auth_type;
    uint16_t _size;
    uint8_t* _auth_key;
};

//operation query app requset is same structure as generic req
typedef KnGenericReq KnOprQueryAppReq;

//request message for operation select app req 
class KnOprSelectAppReq : public KnGenericReq {
public:
    KnOprSelectAppReq(uint16_t msg_type, uint16_t msg_len, uint32_t id);
    virtual ~KnOprSelectAppReq();
    uint32_t _app_id;
};

//request message for SFID start req
class KnSFIDStartReq : public KnGenericReq {
public:
    KnSFIDStartReq(uint16_t msg_type, uint16_t msg_len, float r,
                uint16_t width, uint16_t height, uint32_t format);
    virtual ~KnSFIDStartReq();
    float _fr_thresh;
    uint16_t _wid;
    uint16_t _ht;
    uint32_t _fmt;
};

class KnSFIDNewUserReq : public KnGenericReq {
public:
    KnSFIDNewUserReq(uint16_t msg_type, uint16_t msg_len, uint16_t u_id, uint16_t img_idx);
    virtual ~KnSFIDNewUserReq();
    uint16_t _user_id;
    uint16_t _r0;
    uint16_t _img_idx;
    uint16_t _r1;
};

class KnSFIDRegisterReq : public KnGenericReq {
public:
    KnSFIDRegisterReq(uint16_t msg_type, uint16_t msg_len, uint32_t u_id);
    virtual ~KnSFIDRegisterReq();
    uint32_t _user_id;
};

typedef KnSFIDRegisterReq KnSFIDDelDBReq;

class KnSFIDEditDBReq : public KnGenericReq {
public:
    KnSFIDEditDBReq(uint16_t msg_type, uint16_t msg_len, uint32_t mode, uint32_t u_id, uint32_t f_id);
    virtual ~KnSFIDEditDBReq();
    uint32_t _mode;
    uint32_t _user_id;
    uint32_t _face_id;
};

typedef KnSFIDEditDBReq KnSFIDAddFmReq;
typedef KnSFIDEditDBReq KnSFIDQueryFmReq;

class KnSFIDSendImageReq : public KnGenericReq {
public:
    KnSFIDSendImageReq(uint16_t msg_type, uint16_t msg_len, uint32_t fsize, uint16_t out_mask);
    virtual ~KnSFIDSendImageReq();
    uint32_t _f_size;
    uint16_t _out_mask;
    uint16_t _r0;
};

class KnSFIDDbConfigReq : public KnGenericReq {
public:
    KnSFIDDbConfigReq(uint16_t msg_type, uint16_t msg_len, uint32_t mode, kapp_db_config_parameter_t* db_config_parameter);
    virtual ~KnSFIDDbConfigReq();
    uint32_t _mode;
    kapp_db_config_parameter_t* _db_config_parameter;
};

typedef KnSFIDDbConfigReq KnSFIDSetDbConfigReq;
typedef KnSFIDDbConfigReq KnSFIDGetDbConfigReq;
typedef KnSFIDDbConfigReq KnSFIDGetDbIndexReq;
typedef KnSFIDDbConfigReq KnSFIDSwitchDbIndexReq;
typedef KnSFIDDbConfigReq KnSFIDSetDbVersionReq;
typedef KnSFIDDbConfigReq KnSFIDGetDbVersionReq;
typedef KnSFIDDbConfigReq KnSFIDGetDbMetaDataVersionReq;

//request message for ISI start req
class KnIsiStartReq : public KnGenericReq {
public:
    KnIsiStartReq(uint16_t msg_type, uint16_t msg_len, uint32_t app_id, uint32_t rt_size, uint16_t width, uint16_t height, uint32_t format);
    virtual ~KnIsiStartReq();
    uint32_t _app_id;
    uint32_t _rt_size;
    uint16_t _wid;
    uint16_t _ht;
    uint32_t _fmt;
};

class KnIsiStartExtReq : public KnGenericReq {
public:
    KnIsiStartExtReq(uint16_t msg_type, uint16_t msg_len, char* data);
    virtual ~KnIsiStartExtReq();
};

class KnIsiImageReq : public KnGenericReq {
public:
    KnIsiImageReq(uint16_t msg_type, uint16_t msg_len, uint32_t img_size, uint32_t img_id);
    virtual ~KnIsiImageReq();
    uint32_t _img_size;
    uint32_t _img_id;
};

class KnIsiResultsReq : public KnGenericReq {
public:
    KnIsiResultsReq(uint16_t msg_type, uint16_t msg_len, uint32_t img_id);
    virtual ~KnIsiResultsReq();
    uint32_t _img_id;
};

class KnIsiConfigReq : public KnGenericReq {
public:
    KnIsiConfigReq(uint16_t msg_type, uint16_t msg_len, uint32_t model_id, uint32_t param);
    virtual ~KnIsiConfigReq();
};

//request message for start DME req
class KnStartDmeReq : public KnGenericReq {
public:
    KnStartDmeReq(uint16_t msg_type, uint16_t msg_len, uint32_t mod_size, char* data, int dat_size);
    virtual ~KnStartDmeReq();
    uint32_t _model_size;
};

class KnDmeConfigReq : public KnGenericReq {
public:
    KnDmeConfigReq(uint16_t msg_type, uint16_t msg_len, char* data, int dat_size);
    virtual ~KnDmeConfigReq();
};

class KnDmeImageReq : public KnGenericReq {
public:
    KnDmeImageReq(uint16_t msg_type, uint16_t msg_len, uint32_t fsize, uint16_t mode, uint16_t model_id);
    virtual ~KnDmeImageReq();
    uint32_t _f_size;
    uint16_t _mode;
    uint16_t _model_id;
};

class KnDmeStatusReq : public KnGenericReq {
public:
    KnDmeStatusReq(uint16_t msg_type, uint16_t msg_len, uint16_t ssid);
    virtual ~KnDmeStatusReq();
    uint32_t _r0;
    uint32_t _r1;
};

//request message to configure image parameters for jpeg encoding 
class KnJpegEncCfgReq : public KnGenericReq {
public:
    KnJpegEncCfgReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq,
                uint16_t width, uint16_t height, uint32_t format, uint32_t quality);
    virtual ~KnJpegEncCfgReq();
	uint32_t _seq_num;
    uint32_t _fmt;
    uint32_t _wid;
    uint32_t _ht;
	uint32_t _quality;
};

//request message to start jpeg encoding
class KnJpegEncStartReq : public KnGenericReq {
public:
    KnJpegEncStartReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq, uint32_t buf_len);
    virtual ~KnJpegEncStartReq();
	uint32_t _seq_num;
	uint32_t _buf_len;
};

// request message to retrieve jpeg encoding result
class KnJpegEncResultsReq : public KnGenericReq {
public:
    KnJpegEncResultsReq(uint16_t msg_type, uint16_t msg_len, uint32_t img_id);
    virtual ~KnJpegEncResultsReq();
    uint32_t _img_id;
};

//request message to configure image parameters for jpeg decoding 
class KnJpegDecCfgReq : public KnGenericReq {
public:
    KnJpegDecCfgReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq,
                uint16_t width, uint16_t height, uint32_t format, uint32_t len);
    virtual ~KnJpegDecCfgReq();
	uint32_t _seq_num;
    uint32_t _fmt;
    uint32_t _wid;
    uint32_t _ht;
    uint32_t _len;
};

//request message to start jpeg decoding
class KnJpegDecStartReq : public KnGenericReq {
public:
    KnJpegDecStartReq(uint16_t msg_type, uint16_t msg_len, uint16_t seq, uint32_t buf_len);
    virtual ~KnJpegDecStartReq();
	uint32_t _seq_num;
	uint32_t _buf_len;
};

// request message to retrieve jpeg decoding result
class KnJpegDecResultsReq : public KnGenericReq {
public:
    KnJpegDecResultsReq(uint16_t msg_type, uint16_t msg_len, uint32_t img_id);
    virtual ~KnJpegDecResultsReq();
    uint32_t _img_id;
};

//----------------------request message end-----------------------

//file transfer ACK, used for getting DME result data
class KnImgTransAck : public KnBaseMsg {
public:
    KnImgTransAck(int8_t* buf, int mlen);
    KnImgTransAck(uint16_t msg_type, uint16_t msg_len, uint32_t action, uint32_t src_id);
    virtual ~KnImgTransAck();

    uint32_t _action;
    uint32_t _src_id;
};

//----------------------response message start-----------------------
//base class for rsp message
//only message type and message length
class KnGenericAckRsp : public KnBaseMsg {
public:
    KnGenericAckRsp(int8_t* buf, int mlen);
    virtual ~KnGenericAckRsp();
};

//mem request general ack msg
class KnTestMemAck : public KnGenericAckRsp {
public:
    KnTestMemAck(int8_t* buf, int mlen);
    virtual ~KnTestMemAck();

    uint32_t _ack;
    uint32_t _reserved;
};

//response message for memory write
class KnTestMemWriteRsp : public KnGenericAckRsp {
public:
    KnTestMemWriteRsp(int8_t* buf, int mlen);
    virtual ~KnTestMemWriteRsp();

    uint32_t _rsp_result;
    uint32_t _num_bytes;
};

//response message for memory read
class KnTestMemReadRsp : public KnTestMemWriteRsp {
public:
    KnTestMemReadRsp(int8_t* buf, int mlen);
    virtual ~KnTestMemReadRsp();

    int8_t*  _p_data;
    int      _len;
};

//clear response is same structure as write response
typedef KnTestMemWriteRsp KnTestMemClearRsp;

//host echo response is same as mem read response
typedef KnTestMemReadRsp KnTestHostEchoRsp;

//response message for memory write
class KnCmdResetRsp : public KnGenericAckRsp {
public:
    KnCmdResetRsp(int8_t* buf, int mlen);
    virtual ~KnCmdResetRsp();

    uint32_t _err_code;
    uint32_t _reserved;
};

//response message for report sys rsp 
class KnCmdSysStsRsp : public KnGenericAckRsp {
public:
    KnCmdSysStsRsp(int8_t* buf, int mlen);
    virtual ~KnCmdSysStsRsp();

    uint32_t _sfw_id;
    uint32_t _sbuild_id;
    uint16_t _sys_sts;
    uint16_t _app_sts;
    uint32_t _nfw_id;
    uint32_t _nbuild_id;
};

//response message for update firmware 
class KnCmdUdtFmRsp : public KnGenericAckRsp {
public:
    KnCmdUdtFmRsp(int8_t* buf, int mlen);
    virtual ~KnCmdUdtFmRsp();

    uint32_t _rsp_code;
    uint32_t _module_id;
};

//response message for update model
class KnCmdUdtModelRsp : public KnGenericAckRsp {
public:
    KnCmdUdtModelRsp(int8_t* buf, int mlen);
    virtual ~KnCmdUdtModelRsp();

    uint32_t _rsp_code;
    uint32_t _model_id;
};

//response message for update spl
class KnCmdUdtSplRsp : public KnGenericAckRsp {
public:
    KnCmdUdtSplRsp(int8_t* buf, int mlen);
    virtual ~KnCmdUdtSplRsp();

    uint32_t _rsp_code;
    uint32_t _spl_id;
    uint32_t _spl_build;
};

//response message for query app
class KnOprQueryAppRsp : public KnGenericAckRsp {
public:
    KnOprQueryAppRsp(int8_t* buf, int mlen);
    virtual ~KnOprQueryAppRsp();

    uint32_t _reserved;
    uint32_t _reserved2;
    uint32_t*  _p_app_id;
    int      _len;
};

//response message for start sfid
class KnSFIDStartRsp : public KnGenericAckRsp {
public:
    KnSFIDStartRsp(int8_t* buf, int mlen);
    virtual ~KnSFIDStartRsp();

    uint32_t _rsp_code;
    uint32_t _img_size;
};

//response message for send img resp
#define RES_START_OFF 16

class KnSFIDSendImageRsp : public KnGenericAckRsp {
public:
    KnSFIDSendImageRsp(int8_t* buf, int mlen);
    virtual ~KnSFIDSendImageRsp();

    uint32_t _rsp_code;
    uint16_t _mode;
    uint16_t _uid;
    uint16_t _out_flag;
    uint8_t  _fd_res[FD_RES_LENGTH];
    uint8_t  _lm_res[LM_RES_LENGTH];
    uint8_t  _fr_res[FR_RES_LENGTH];
    uint16_t _live_res;
    float    _score_res;
};

//new user rsp is generic ack
typedef KnGenericAckRsp KnSFIDewUserRsp;

//operation operation select app rsp is same structure as SFID start rsp
typedef KnSFIDStartRsp KnOprSelectAppRsp;

//register rsp, del rsp is same as start sfid rsp
typedef KnSFIDStartRsp KnSFIDRegisterRsp;
typedef KnSFIDStartRsp KnSFIDDelDBRsp;

class KnSFIDEditDBRsp : public KnGenericAckRsp {
public:
    KnSFIDEditDBRsp(int8_t *buf, int mlen);
    virtual ~KnSFIDEditDBRsp();

    uint32_t _rsp_code;
    uint32_t _data_size;
    uint32_t _data;
};

typedef KnSFIDEditDBRsp KnSFIDAddFmRsp;
typedef KnSFIDEditDBRsp KnSFIDQueryFmRsp;

//set db config, set db config, get db index rsp are same as edit db rsp
typedef KnSFIDEditDBRsp KnSFIDDbConfigRsp;
typedef KnSFIDDbConfigRsp KnSFIDSetDbConfigRsp;
typedef KnSFIDDbConfigRsp KnSFIDGetDbConfigRsp;
typedef KnSFIDDbConfigRsp KnSFIDGetDbIndexRsp;
typedef KnSFIDDbConfigRsp KnSFIDSwitchDbIndexRsp;
typedef KnSFIDDbConfigRsp KnSFIDSetDbVersionRsp;
typedef KnSFIDDbConfigRsp KnSFIDGetDbVersionRsp;
typedef KnSFIDDbConfigRsp KnSFIDGetDbMetaDataVersionRsp;

//response message for start isi
class KnIsiStartRsp : public KnGenericAckRsp {
public:
    KnIsiStartRsp(int8_t* buf, int mlen);
    virtual ~KnIsiStartRsp();

    uint32_t _rsp_code;
    uint32_t _buf_size;
};

//response message for send isi image
class KnIsiImgRsp : public KnGenericAckRsp {
public:
    KnIsiImgRsp(int8_t* buf, int mlen);
    virtual ~KnIsiImgRsp();

    uint32_t _rsp_code;
    uint32_t _image_available;// available buffer remaining
    uint32_t _rsv;// reserved
};

//response message for get isi results
class KnIsiResultsRsp : public KnGenericAckRsp {
public:
    KnIsiResultsRsp(int8_t* buf, int mlen);
    virtual ~KnIsiResultsRsp();

    uint32_t _rsp_code;
    uint32_t _r_size;// inference result data size
};

//response message for isi config
class KnIsiConfigRsp : public KnGenericAckRsp {
public:
    KnIsiConfigRsp(int8_t* buf, int mlen);
    virtual ~KnIsiConfigRsp();

    uint32_t _rsp_code;
};

//response message for start dme 
class KnStartDmeRsp : public KnGenericAckRsp {
public:
    KnStartDmeRsp(int8_t* buf, int mlen);
    virtual ~KnStartDmeRsp();

    uint32_t _rsp_code;
    uint32_t _inf_size;
};

//response message for send dme image
class KnDmeImgRsp : public KnGenericAckRsp {
public:
    KnDmeImgRsp(int8_t* buf, int mlen);
    virtual ~KnDmeImgRsp();

    uint32_t _rsp_code;
    uint32_t _inf_mode;// normal and async
    uint32_t _inf_size;// size or ssid+reserved
};

//response message for dme config
class KnDmeConfigRsp : public KnGenericAckRsp {
public:
    KnDmeConfigRsp(int8_t* buf, int mlen);
    virtual ~KnDmeConfigRsp();

    uint32_t _rsp_code;
    uint32_t _model_id;
};

//response message for get dme status
class KnDmeStatusRsp : public KnGenericAckRsp {
public:
    KnDmeStatusRsp(int8_t* buf, int mlen);
    virtual ~KnDmeStatusRsp();

    uint32_t _rsp_code;
    uint16_t _session_id;// normal and async
    uint16_t _inf_status;// normal and async     
    uint32_t _inf_size;
};

typedef KnDmeImgRsp KnDmeImageRsp;
typedef KnDmeStatusRsp KnDmeGetRsp;

//response message for get KN number rsp 
class KnCmdGetKnNumRsp : public KnGenericAckRsp {
public:
    KnCmdGetKnNumRsp(int8_t *buf, int mlen);
    virtual ~KnCmdGetKnNumRsp();

    uint32_t kn_num;
};

//response message for get model info rsp
class KnCmdGetModelInfoRsp : public KnGenericAckRsp {
public:
    KnCmdGetModelInfoRsp(int8_t *buf, int mlen);
    virtual ~KnCmdGetModelInfoRsp();

    uint32_t data_size;
};

//response message for set ckey rsp 
class KnCmdSetCkeyRsp : public KnGenericAckRsp {
public:
    KnCmdSetCkeyRsp(int8_t *buf, int mlen);
    virtual ~KnCmdSetCkeyRsp();

    uint32_t set_status;
};

//response message for set sbt key rsp 
class KnCmdSetSbtKeyRsp : public KnGenericAckRsp {
public:
    KnCmdSetSbtKeyRsp(int8_t *buf, int mlen);
    virtual ~KnCmdSetSbtKeyRsp();

    uint32_t set_status;
};

//response message for get crc rsp
class KnCmdGetCrcRsp : public KnGenericAckRsp {
public:
    KnCmdGetCrcRsp(int8_t *buf, int mlen);
    virtual ~KnCmdGetCrcRsp();

    uint32_t data_size;
};

//response message for jpeg encoding config
class KnJpegEncConfigRsp : public KnGenericAckRsp {
public:
    KnJpegEncConfigRsp(int8_t* buf, int mlen);
    virtual ~KnJpegEncConfigRsp();

    uint32_t _rsp_code;
    uint32_t _img_seq;
};

//response message for jpeg enc
class KnJpegEncStartRsp : public KnGenericAckRsp {
public:
    KnJpegEncStartRsp(int8_t* buf, int mlen);
    virtual ~KnJpegEncStartRsp();

    uint32_t _rsp_code;
    uint32_t _availablity;
    uint32_t _img_seq;
    uint32_t _img_buf;
    uint32_t _img_len;
};

//response message for get jpeg enc results
class KnJpegEncResultsRsp : public KnGenericAckRsp {
public:
    KnJpegEncResultsRsp(int8_t* buf, int mlen);
    virtual ~KnJpegEncResultsRsp();

    uint32_t _rsp_code;
    uint32_t _r_size;// jpeg enc result data size
};

//response message for jpeg decoding config
class KnJpegDecConfigRsp : public KnGenericAckRsp {
public:
    KnJpegDecConfigRsp(int8_t* buf, int mlen);
    virtual ~KnJpegDecConfigRsp();

    uint32_t _rsp_code;
    uint32_t _img_seq;
};

//response message for jpeg dec
class KnJpegDecStartRsp : public KnGenericAckRsp {
public:
    KnJpegDecStartRsp(int8_t* buf, int mlen);
    virtual ~KnJpegDecStartRsp();

    uint32_t _rsp_code;
    uint32_t _availablity;
    uint32_t _img_seq;
    uint32_t _img_buf;
    uint32_t _img_len;
};

//response message for get jpeg dec results
class KnJpegDecResultsRsp : public KnGenericAckRsp {
public:
    KnJpegDecResultsRsp(int8_t* buf, int mlen);
    virtual ~KnJpegDecResultsRsp();

    uint32_t _rsp_code;
    uint32_t _r_size;// jpeg enc result data size
};

//----------------------response message end-----------------------

#endif

