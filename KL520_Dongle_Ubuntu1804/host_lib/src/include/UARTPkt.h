/**
 * @file      UARTPkt.h
 * @brief     UART packet header file
 * @version   0.1 - 2019-04-30
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __UART_PKT_H__
#define __UART_PKT_H__

#define UART_MAGIC_CMD   0xA583
#define UART_MAGIC_RSP   0x8A35

#define UART_PKT_HEAD_LEN  4

#define MSG_CRC_FLAG 1

#include <stdint.h>


/* 
 * UART packet class
 */
class UARTPkt {
    /* 
     * UART packet hdr class
     */
    class UARTHdr {
    public:
        uint16_t _preamble;
        uint16_t _ctrl_bytes;

        uint16_t _payload_len;

        int8_t   _cmd_bit;
        int8_t   _crc_bit;
        int8_t   _rsvd_bit;
        int8_t   _nack_bit;

        bool     _valid;

        UARTHdr(uint16_t len, bool is_cmd, bool is_crc, bool is_nack = false);
        UARTHdr();
        virtual ~UARTHdr();
        uint16_t calc_ctrl_bytes();
        void set_uart_hdr(uint16_t pre, uint16_t ctrl);
    };
public:
    UARTPkt(int8_t* msg, uint16_t len, bool is_cmd, bool is_crc, bool is_nack = false);
    UARTPkt(int8_t* pkt, uint16_t pkt_len);
    virtual ~UARTPkt();

    void encode_uart_pkt();
    void decode_uart_pkt();

    int8_t* get_pkt_buf() { return _pkt_buf; }
    int8_t* get_payload_buf() { return _pkt_buf + UART_PKT_HEAD_LEN; }
    
    bool  is_pkt_valid() { return _valid; }

    uint16_t get_pkt_len() { return _tot_len; }
    uint16_t get_payload_len() { return _hdr._payload_len; }

private:
    UARTHdr  _hdr;
    int8_t*  _pkt_buf;
    uint32_t _crc_val;
    uint16_t _tot_len;

    bool     _valid;

};

#endif

