/**
 * @file      UARTPkt.cpp
 * @brief     implementation file for UART pkt class
 * @version   0.1 - 2019-04-30
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "UARTPkt.h"
#include "KnLog.h"
#include "KnUtil.h"

#ifdef __linux__
#include <arpa/inet.h>
#elif _WIN32
#include <windows.h>
#endif

#include <string.h>

//constructor with paramters
UARTPkt::UARTHdr::UARTHdr(uint16_t len, bool is_cmd, bool is_crc, bool is_nack) : _payload_len(len), _valid(false) {
    if(is_cmd) {
        _cmd_bit = 1;
        _preamble = UART_MAGIC_CMD;
    } else {
        _cmd_bit = 0;
        _preamble = UART_MAGIC_RSP;
    }
    if(is_crc) {
        _crc_bit = 1;
    } else {
        _crc_bit = 0;
    }
    _rsvd_bit = 0;

    _ctrl_bytes = calc_ctrl_bytes();
    if(_ctrl_bytes != 0) _valid = true;
    KnPrint(KN_LOG_DBG, "uart pkt header control bytes:%x.\n", _ctrl_bytes);
}

UARTPkt::UARTHdr::UARTHdr() : _valid(false) {
}

UARTPkt::UARTHdr::~UARTHdr() {
}

//calculate control bytes
uint16_t UARTPkt::UARTHdr::calc_ctrl_bytes() {
    uint16_t tmp = _payload_len;
    if(_crc_bit) tmp += sizeof(_crc_val);

    if(_cmd_bit) {
        tmp |= (0x01 << 15);
    }

    if(_crc_bit) {
        tmp |= (0x01 << 14);
    }

    return tmp;
}

//decode from preamble and ctrl_bytes
void UARTPkt::UARTHdr::set_uart_hdr(uint16_t pre, uint16_t ctrl) {
    KnPrint(KN_LOG_DBG, "decode uart pkt header with pre:%x, ctrl:%x.\n", pre, ctrl);
    _preamble = pre;
    _ctrl_bytes = ctrl;
    _payload_len = _ctrl_bytes & 0x1fff;

    if(_ctrl_bytes & (0x01 << 15)) {
        _cmd_bit = 1;
    } else {
        _cmd_bit = 0;
    }

    if(_ctrl_bytes & (0x01 << 14)) {
        _crc_bit = 1;
        _payload_len -= sizeof(_crc_val);
    } else {
        _crc_bit = 0;
    }

    _rsvd_bit = 0;
    _valid = true;
    KnPrint(KN_LOG_DBG, "decoded uart pkt header payload:%d, cmd:%d, crc:%d.\n", 
            _payload_len, _cmd_bit, _crc_bit);
}


UARTPkt::UARTPkt(int8_t* msg, uint16_t len, bool is_cmd, bool is_crc, bool is_nack) \
                : _hdr(len, is_cmd, is_crc, is_nack), _pkt_buf(NULL), _valid(false) {
    if(!msg) return;
    if(len == 0) return;
    if(_hdr._valid == false) {
        KnPrint(KN_LOG_ERR, "uart pkt header is invalid:%x.\n", _hdr._ctrl_bytes);
        return;
    }

    _tot_len = len + UART_PKT_HEAD_LEN;
    if(is_crc) _tot_len += sizeof(_crc_val);

    _pkt_buf = new int8_t[_tot_len];
    if(!_pkt_buf) {
        KnPrint(KN_LOG_ERR, "malloc pkt buf failed:%d.\n", _tot_len);
        return;
    }

    KnPrint(KN_LOG_DBG, "created buffer for uart pkt of size [%d].\n", _tot_len);
    memset(_pkt_buf, 0, _tot_len);

    int off = UART_PKT_HEAD_LEN;

    memcpy((void*)(_pkt_buf+off), (void*)msg, len);
    KnPrint(KN_LOG_DBG, "copied payload to uart pkt buffer of len [%d].\n", len);
    encode_uart_pkt();
}

UARTPkt::UARTPkt(int8_t* pkt, uint16_t pkt_len) : _pkt_buf(NULL), _valid(false) {
    if(!pkt) return;
    if(pkt_len == 0) return;

    _tot_len = pkt_len;
    _pkt_buf = new int8_t[_tot_len];
    if(!_pkt_buf) {
        KnPrint(KN_LOG_ERR, "malloc pkt buf failed:%d.\n", _tot_len);
        return;
    }
    KnPrint(KN_LOG_DBG, "created buffer for uart pkt of len [%d].\n", _tot_len);

    memcpy(_pkt_buf, pkt, _tot_len);
    KnPrint(KN_LOG_DBG, "copied whole packet to uart pkt buffer:%d.\n", _tot_len);
    decode_uart_pkt();
}

UARTPkt::~UARTPkt() {
    if(_pkt_buf) { 
        delete[] _pkt_buf;
        _pkt_buf = NULL;
    }
}

void UARTPkt::encode_uart_pkt() {
    if(!_pkt_buf) return;

    int off = 0;
    uint16_t tmp = (_hdr._preamble);
    //uint16_t tmp = htons(_hdr._preamble);
    memcpy((void*)(_pkt_buf+ off), (void*)&tmp, sizeof(uint16_t));
    off += sizeof(uint16_t);
    KnPrint(KN_LOG_DBG, "uart pkt preamble encoded:%x.\n", tmp);

    tmp = (_hdr._ctrl_bytes);
    //tmp = htons(_hdr._ctrl_bytes);
    memcpy((void*)(_pkt_buf+ off), (void*)&tmp, sizeof(uint16_t));
    off += sizeof(uint16_t);
    KnPrint(KN_LOG_DBG, "uart pkt control bytes encoded:%x.\n", tmp);

    off += _hdr._payload_len;

    if(_hdr._crc_bit) {
        _crc_val = gen_crc16(_pkt_buf, _tot_len - sizeof(_crc_val));
        uint32_t tmp32 = (_crc_val);
        //tmp = htons(_crc_val);
        memcpy((void*)(_pkt_buf+ off), (void*)&tmp32, sizeof(_crc_val));
        KnPrint(KN_LOG_DBG, "uart pkt crc val encoded:%x.\n", tmp32);
    }
    _valid = true;
}

void UARTPkt::decode_uart_pkt() {
    //get pre
    uint16_t tmp;
    int off = 0;
    memcpy((void*)&tmp, (void*)(_pkt_buf+ off), sizeof(uint16_t));
    //uint16_t pre = ntohs(tmp);
    uint16_t pre = (tmp);
    off += sizeof(uint16_t);
    //if(pre != UART_MAGIC_RSP) return;

    //get ctrol bytes
    memcpy((void*)&tmp, (void*)(_pkt_buf+ off), sizeof(uint16_t));
    uint16_t ctrl = (tmp);
    //uint16_t ctrl = ntohs(tmp);
    off += sizeof(uint16_t);

    _hdr.set_uart_hdr(pre, ctrl);

    if(_hdr._valid == false) return;

    //check crc
    if(_hdr._crc_bit) {
        _crc_val = gen_crc16(_pkt_buf, _tot_len - sizeof(_crc_val));
        off += _hdr._payload_len;

        uint32_t tmp32;
        memcpy((void*)&tmp32, (void*)(_pkt_buf+ off), sizeof(_crc_val));
        //tmp = ntohs(tmp);
        KnPrint(KN_LOG_DBG, "uart pkt calced crc:%x, in msg crc:%x.\n", _crc_val, tmp32);
        if(_crc_val != tmp32) {
            KnPrint(KN_LOG_ERR, "crc wrong, caculatd:%x, in msg:%x.\n", _crc_val, tmp32);
            return;
        }
    }

    _valid = true;
}


