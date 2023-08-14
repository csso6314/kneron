/*
 * Kneron USBD API
 *
 * Copyright (C) s/2019/2020/ Kneron, Inc. All rights reserved.
 *
 */

// #define USBD2_DBG

#include <stdlib.h>
#include <string.h>

#include "cmsis_os2.h"
#include "base.h"
#include "kdrv_scu_ext.h"
#include "kdrv_usbd2v.h"

#ifdef USBD2_DBG
#include "kmdw_console.h"
#endif

#ifdef USBD2_DBG
#define usbd2_dbg(__format__, ...) kmdw_printf(__format__, ##__VA_ARGS__)
#else
#define usbd2_dbg(__format__, ...)
#endif

// OTG Control Status Register (0x80)
#define REG_OTG_CSR 0x80
#define SPD_TYPE (BIT22 | BIT23)
#define VBUS_VLD_RO BIT19
#define B_SESS_END_RO BIT16

// OTG Interrupt Stauts Register (0x84)
// OTG Interrupt Enable Register (0x88)
#define REG_OTG_ISR 0x84
#define REG_OTG_IER 0x88
#define OTG_APLGRMV_RW1C BIT12
#define OTG_A_WAIT_CON_RW1C BIT11
#define OTG_OVC_RW1C BIT10
#define OTG_IDCHG_RW1C BIT9
#define OTG_RLCHG_RW1C BIT8
#define OTG_B_SESS_END_RW1C BIT6
#define OTG_A_VBUS_ERR_RW1C BIT5
#define OTG_A_SRP_DET_RW1C BIT4
#define OTG_B_SRP_DN_RW1C BIT0

// Global HC/OTG/DEV Interrupt Status Register (0xC0)
// Global Mask of HC/OTG/DEV Interrupt Register (0xC4)
#define REG_GLB_ISR 0xC0
#define REG_GLB_INT 0xC4
#define INT_POLARITY BIT3
#define OTG_INT BIT1
#define DEV_INT BIT0

// Device Main Control Register (0x100)
#define REG_DEV_CTL 0x100

// Device Address register (0x104)
#define REG_DEV_ADR 0x104
#define AFT_CONF BIT7

// Device Test Register (0x108)
#define REG_DEV_TST 0x108
#define TST_CLRFF BIT0

// Device SOF Mask Timer Register (0x110)
#define REG_DEV_SMT 0x110

// PHY Test Mode Selector Register (0x114)
#define REG_PHY_TST 0x114
#define TST_JSTA BIT0

// Device CX configuration adn FIFO empty status (0x120)
#define REG_CXCFE 0x120
#define F_EMP_0 BIT8
#define CX_CLR BIT3
#define CX_STL BIT2
#define CX_DONE BIT0

// Device Idle Counter Register (0x124)
#define REG_DEV_ICR 0x124

// Group total interrupt mask (0x130)
// Group total interrupt status (0x140)
#define REG_DEV_MIGR 0x130
#define REG_DEV_IGR 0x140
#define GX_INT_G3_RO BIT3
#define GX_INT_G2_RO BIT2
#define GX_INT_G1_RO BIT1
#define GX_INT_G0_RO BIT0

// Group 0 interrupt mask (0x134)
// Group 0 interrupt status (0x144)
// control transfer
#define REG_DEV_MISG0 0x134
#define REG_DEV_ISG0 0x144
#define G0_CX_COMABT_INT_RW1C BIT5
#define G0_CX_COMFAIL_INT_RO BIT4
#define G0_CX_COMEND_INT_RO BIT3
#define G0_CX_OUT_INT_RO BIT2
#define G0_CX_IN_INT_RO BIT1
#define G0_CX_SETUP_INT_RO BIT0

// Group 1 interrupt mask (0x138)
// Group 1 interrupt status (0x148)
// FIFO interrupts
#define REG_DEV_MISG1 0x138
#define REG_DEV_ISG1 0x148
#define MF0_IN_INT BIT16
#define MF0_SPK_INT BIT1
#define MF0_OUT_INT BIT0

// Group 1 interrupts (0x148)
#define G1_F3_IN_INT_RO BIT19
#define G1_F2_IN_INT_RO BIT18
#define G1_F1_IN_INT_RO BIT17
#define G1_F0_IN_INT_RO BIT16
#define G1_F3_SPK_INT_RO BIT7
#define G1_F3_OUT_INT_RO BIT6
#define G1_F2_SPK_INT_RO BIT5
#define G1_F2_OUT_INT_RO BIT4
#define G1_F1_SPK_INT_RO BIT3
#define G1_F1_OUT_INT_RO BIT2
#define G1_F0_SPK_INT_RO BIT1
#define G1_F0_OUT_INT_RO BIT0

// Group 2 interrupt mask (0x13C)
// Group 2 interrupt source (0x14C)
#define REG_DEV_MISG2 0x13C
#define REG_DEV_ISG2 0x14C
#define G2_Dev_Wakeup_byVbus_RO BIT10
#define G2_Dev_Idle_RO BIT9
#define G2_DMA_ERROR_RW1C BIT8
#define G2_DMA_CMPLT_RW1C BIT7
#define G2_RX0BYTE_INT_RW1C BIT6
#define G2_TX0BYTE_INT_RW1C BIT5
#define G2_ISO_SEQ_ABORT_INT_RW1C BIT4
#define G2_ISO_SEQ_ERR_INT_RW1C BIT3
#define G2_RESM_INT_RW1C BIT2
#define G2_SUSP_INT_RW1C BIT1
#define G2_USBRST_INT_RW1C BIT0

// Devcie Receive Zero-Length Data Packet Register (0x150)
#define REG_DEV_RXZ 0x150
#define RX0BYTE_EP1 BIT0

// Device IN endpoint & MaxPacketSize (0x160 + 4(n-1))
#define REG_DEV_INMPS_1 0x160
#define TX0BYTE_IEPn BIT15
#define RSTG_IEPn BIT12
#define STL_IEPn BIT11

// Device IN endpoint & MaxPacketSize (0x180 + 4(n-1))
#define REG_DEV_OUTMPS_1 0x180
#define RSTG_OEPn BIT12
#define STL_IEPn BIT11

// Device Endpoint 1~4 Map Register (0x1A0)
#define REG_DEV_EPMAP0 0x1A0

// Device Endpoint 5~8 Map Register (0x1A4)
#define REG_DEV_EPMAP1 0x1A4

// Device FIFO Map Register (0x1A8)
#define REG_DEV_FMAP 0x1A8

// Device FIFO Configuration Register (0x1AC)
#define REG_DEV_FCFG 0x1AC

// Device FIFO Byte Count Register (0x1B0 + 4(fifo_no-1))
#define FFRST BIT12
#define BC_Fn 0x7ff

// DMA Target FIFO register (0x1C0)
#define REG_DMA_TFN 0x1C0
#define DMA_TARGET_ACC_CXF BIT4
#define DMA_TARGET_ACC_F3 BIT3
#define DMA_TARGET_ACC_F2 BIT2
#define DMA_TARGET_ACC_F1 BIT1
#define DMA_TARGET_ACC_F0 BIT0
#define DMA_TARGET_ACC_NONE 0x0

// DMA Controller Param 1 (0x1C8)
#define REG_DMA_CPS1 0x1C8
#define DMA_TYPE BIT1
#define DMA_START BIT0

// DMA Controller Param 2 (0x1CC)
#define REG_DMA_CPS2 0x1CC

// DMA Controller Param 3 (0x1D0)
// setup packet 8 bytes direct DMA read
#define REG_DMA_CPS3 0x1D0

#define MAX_ENP_NUM 4
#define FIFO_NUM 4 // we have 4 FIFOs, each has 1-KB

#define UsbRegRead(reg_offset) inw(USB_FOTG210_PA_BASE + reg_offset)
#define UsbRegWrite(reg_offset, val) outw(USB_FOTG210_PA_BASE + reg_offset, val)
#define UsbRegMaskedSet(reg_offset, val) masked_outw((USB_FOTG210_PA_BASE + reg_offset), (val), (val))
#define UsbRegMaskedClr(reg_offset, val) masked_outw((USB_FOTG210_PA_BASE + reg_offset), 0, (val))

#define VDMA_ONE_TXFER_SIZE (100 * 1024) // 100 KB

enum
{
    CONFIG_DEFAULT_STATE = 0,
    CONFIG_ADDRESS_STATE,
    CONFIG_CONFIGURED_STATE,
};

// for FIFO_Ctrl_t:transferType
enum
{
    TXFER_CONTROL = 0,
    TXFER_ISO,
    TXFER_BULK,
    TXFER_INT,
};

// for SETUP packet request
enum
{
    RESP_LATER = 1, /* busy now */
    RESP_ACK,       /* reqeust is done */
    RESP_STALL,     /* request is not supported */
};

typedef struct
{
    uint8_t isTransferring;
    uint8_t dir; // 0:OUT, 1:IN
    uint32_t user_buf_len;
    uint32_t received_length;
    bool send_ZLP;
    kdrv_status_t status;
} FIFO_context_t;

static FIFO_context_t _fifo_ctx[FIFO_NUM];
static int _enp_to_fifo[MAX_ENP_NUM + 1] = {-1};
static int _fifo_to_enp[FIFO_NUM] = {0};
static osEventFlagsId_t _evt_id;
static osSemaphoreId_t _enp_semaphore[MAX_ENP_NUM + 1] = {NULL};

static kdrv_usbd2v_device_descriptor_t *_dev_desc = NULL;
static kdrv_usbd2v_string_descriptor_t *_dev_str_desc;
static kdrv_usbd2v_link_status_callback_t _evt_cb;
static kdrv_usbd2v_user_control_callback_t _usr_cx_cb;
static uint32_t _config_state = CONFIG_DEFAULT_STATE;
static kdrv_usbd2v_link_status_t _link_status = USBD2_STATUS_DISCONNECTED;

#if 0
static bool _isInterrupt()
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}
#endif

//  for data SRAM, the address must be remapped
static uint32_t _dma_remap_addr(uint32_t addr)
{
    uint32_t tmp;

    if ((addr & (SdRAM_MEM_BASE)) == SdRAM_MEM_BASE)
    {
        tmp = ((addr) & (~0x10000000)) | 0x20000000;
        return tmp;
    }
    return addr;
}

static void _set_link_status_detection(bool enable)
{
    // use B_SEE_END for link status detection
    if (enable)
    {
        UsbRegWrite(REG_OTG_ISR, OTG_B_SESS_END_RW1C);
        UsbRegWrite(REG_OTG_IER, OTG_B_SESS_END_RW1C);
    }
    else
    {
        UsbRegWrite(REG_OTG_IER, 0x0);
    }
}

static void _vdma_cx_write(uint32_t *addr, uint32_t len)
{
    UsbRegWrite(0x304, _dma_remap_addr((uint32_t)addr));
    UsbRegWrite(0x300, (len << 8) | BIT1 | BIT0);
}

static kdrv_status_t _reset_endpoint(int enp_no)
{
    usbd2_dbg("%s() enp_no %d\n", __FUNCTION__, enp_no);

    uint8_t fifo_no = _enp_to_fifo[enp_no];

    UsbRegWrite(0x1B0 + 4 * fifo_no, FFRST);

    // for IN endpoints
    UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (enp_no - 1), RSTG_IEPn);
    UsbRegMaskedClr(REG_DEV_INMPS_1 + 4 * (enp_no - 1), RSTG_IEPn);

    // for OUT endpoints
    UsbRegMaskedSet(REG_DEV_OUTMPS_1 + 4 * (enp_no - 1), RSTG_OEPn);
    UsbRegMaskedClr(REG_DEV_OUTMPS_1 + 4 * (enp_no - 1), RSTG_OEPn);

    return KDRV_STATUS_OK;
}

static void _reset_all_endpoints()
{
    usbd2_dbg("%s()\n", __FUNCTION__);
    for (int enp_no = 1; enp_no <= MAX_ENP_NUM; enp_no++)
        if (_enp_to_fifo[enp_no] >= 0)
            _reset_endpoint(enp_no);
}

static int8_t _send_host_device_status()
{
    uint8_t sData[2] = {0}; // self-power : 0, remote wakeup : 0

    usbd2_dbg("[int] <Standard Request> send device status\n");

    _vdma_cx_write((uint32_t *)sData, 2);

    return RESP_LATER;
}

static int8_t _send_host_device_descriptor(kdrv_usbd2v_setup_packet_t *setup)
{
    usbd2_dbg("[int] <Standard Request> send device descriptor\n");

    uint16_t txLen = MIN(_dev_desc->bLength, setup->wLength);
    _vdma_cx_write((uint32_t *)_dev_desc, txLen);

    return RESP_LATER;
}

static int8_t _send_host_string_descriptor(kdrv_usbd2v_setup_packet_t *setup, uint8_t type)
{
    usbd2_dbg("[int] <Standard Request> send string descriptor, type %d\n", type);

    uint16_t txLen = 0;
    uint32_t *buf = NULL;

    if (type == 0) //language id
    {
        txLen = MIN(_dev_str_desc->bLength, setup->wLength);
        buf = (uint32_t *)_dev_str_desc;
    }
    else if (type == 1 || type == 2 || type == 3) //iManufacturer,iProduct,iSerialNumber
    {
        txLen = MIN(_dev_str_desc->desc[type - 1]->bLength, setup->wLength);
        buf = (uint32_t *)_dev_str_desc->desc[type - 1];
    }

    if (buf)
    {
        _vdma_cx_write((uint32_t *)buf, txLen);
        return RESP_LATER;
    }
    else
        return RESP_STALL;
}

static int8_t _send_host_configuration_descriptors(kdrv_usbd2v_setup_packet_t *setup)
{
    uint32_t confIdx = setup->wValue & 0xFF;

    usbd2_dbg("[int] <Standard Request> send configuration descriptor\n");

    // some error checking
    if (confIdx > 0)
        return RESP_STALL;

    kdrv_usbd2v_config_descriptor_t *conf_desc = _dev_desc->config;

    uint16_t txLen = MIN(conf_desc->wTotalLength, setup->wLength);

    // create a buffer for combining all sub-descriptors
    static uint8_t *buf_desc = NULL;
    if (!buf_desc)
    {
        buf_desc = malloc(conf_desc->wTotalLength);

        // collect all sub-descriptos int one memory
        uint8_t offset = 0;
        memcpy(buf_desc, conf_desc, conf_desc->bLength);
        offset += conf_desc->bLength;

        kdrv_usbd2v_interface_descriptor_t *intf_desc = conf_desc->interface;
        memcpy(buf_desc + offset, intf_desc, intf_desc->bLength);
        offset += intf_desc->bLength;
        for (int j = 0; j < intf_desc->bNumEndpoints; j++)
        {
            kdrv_usbd2v_endpoint_descriptor_t *endp_desc = intf_desc->endpoint[j];
            memcpy(buf_desc + offset, endp_desc, endp_desc->bLength);
            offset += endp_desc->bLength;
        }
    }

    _vdma_cx_write((uint32_t *)buf_desc, txLen);
    return RESP_LATER;
}

static void _init_fifo_configurations(kdrv_usbd2v_interface_descriptor_t *intf)
{
    uint32_t fifo_map_val = 0x0;    // for 0x1A8
    uint32_t endp_map0_val = 0x0;   // for 0x1A0
    uint32_t fifo_config_val = 0x0; // for 0x1AC

    int fifo_no = 0;
    int fifo_add = 1;

    for (int i = 0; i < MAX_ENP_NUM; i++)
    {
        _enp_to_fifo[i + 1] = -1;
        _fifo_to_enp[i] = 0;
    }

    // here assume endpoint number order is ascending
    for (int i = 0; i < intf->bNumEndpoints; i++)
    {
        uint8_t bEndpointAddress = intf->endpoint[i]->bEndpointAddress;
        uint8_t bmAttributes = intf->endpoint[i]->bmAttributes;
        uint16_t wMaxPacketSize = intf->endpoint[i]->wMaxPacketSize;

        uint8_t isIn = !!(bEndpointAddress & 0x80);
        uint8_t enp_no = bEndpointAddress & 0xF; // retrieve endpoint no without direction
        uint32_t bitfield;

        // set FIFO's direction and corresponding endpoint no.
        bitfield = (isIn << 4) | enp_no;
        fifo_map_val |= (bitfield << (fifo_no * 8));

        // set endpoint's FIFO no.
        // assume enp_no <= MAX_ENP_NUM
        bitfield = isIn ? fifo_no : (fifo_no << 4);
        endp_map0_val |= (bitfield << ((enp_no - 1) * 8));

        // enable the corresponding FIFO and set transfer type
        bitfield = (BIT5 | (bmAttributes & 0x3));
        if (intf->bNumEndpoints <= 2 ||
            (intf->bNumEndpoints == 3 && i == 0))
        {
            bitfield |= BIT2;
            fifo_add = 2;
        }
        else
            fifo_add = 1;

        fifo_config_val |= bitfield << (fifo_no * 8);

        // set max packet size & reset toggle bit
        if (isIn)
        {
            // for IN endpoints
            UsbRegWrite(REG_DEV_INMPS_1 + 4 * (enp_no - 1), wMaxPacketSize & 0x7ff);
            UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (enp_no - 1), RSTG_IEPn);
            UsbRegMaskedClr(REG_DEV_INMPS_1 + 4 * (enp_no - 1), RSTG_IEPn);
        }
        else
        {
            // for OUT endpoints
            UsbRegWrite(REG_DEV_OUTMPS_1 + 4 * (enp_no - 1), wMaxPacketSize & 0x7ff);
            UsbRegMaskedSet(REG_DEV_OUTMPS_1 + 4 * (enp_no - 1), RSTG_OEPn);
            UsbRegMaskedClr(REG_DEV_OUTMPS_1 + 4 * (enp_no - 1), RSTG_OEPn);
        }

        _enp_to_fifo[enp_no] = fifo_no;
        _fifo_to_enp[fifo_no] = enp_no;

        fifo_no += fifo_add;
    }

    // clear all FIFO
    UsbRegMaskedSet(REG_DEV_TST, TST_CLRFF);

    // set FIFO interrupt mask
    // UsbRegWrite(REG_DEV_MISG1, fifo_int_mask);

    // endpoint map 0
    UsbRegWrite(REG_DEV_EPMAP0, endp_map0_val);

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_DEV_EPMAP0);
        usbd2_dbg("\nsetting endpoint -> FIFO map register [0x%03X]:\n", REG_DEV_EPMAP0);

        for (int i = 0; i < 4; i++)
        {
            uint32_t temp = reg_value >> (i * 8);
            usbd2_dbg("IN enp_%d maps to FIFO_%d\n", i + 1, temp & 0x3);
        }

        for (int i = 0; i < 4; i++)
        {
            uint32_t temp = reg_value >> (i * 8) + 4;
            usbd2_dbg("OUT enp_%d maps to FIFO_%d\n", i + 1, temp & 0x3);
        }
    }
#endif

    // fifo map
    UsbRegWrite(REG_DEV_FMAP, fifo_map_val);

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_DEV_FMAP);
        usbd2_dbg("\nsetting FIFO -> endpoint map register [0x%03X]:\n", REG_DEV_FMAP);

        for (int i = 0; i < 4; i++)
        {
            uint32_t temp = reg_value >> (i * 8);
            char temp_str[4];
            switch ((temp >> 4) & 0x3)
            {
            case 0:
                strcpy(temp_str, "Out");
                break;
            case 1:
                strcpy(temp_str, "In");
                break;
            case 2:
                strcpy(temp_str, "Bi");
                break;
            }

            usbd2_dbg("FIFO_%d map to %s enp_%d\n", i, temp_str, temp & 0xF);
        }
    }
#endif

    // fifo config / enable
    UsbRegWrite(REG_DEV_FCFG, fifo_config_val);

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_DEV_FCFG);
        usbd2_dbg("\nsetting FIFO config register [0x%03X]:\n", REG_DEV_FCFG);
        for (int i = 0; i < 4; i++)
        {
            uint32_t temp = reg_value >> (i * 8);
            usbd2_dbg("- FIFO %d configs:\n", i);
            usbd2_dbg("   >> block enabled : %s\n", (temp & BIT5) ? "O" : "X");
            if (!(temp & BIT5))
                continue;

            usbd2_dbg("   >> block size : %s\n", (temp & BIT4) ? "1024" : "512");

            switch ((temp >> 2) & 0x3)
            {
            case 0:
                usbd2_dbg("   >> block number: Single\n");
                break;
            case 1:
                usbd2_dbg("   >> block number: Double\n");
                break;
            case 2:
                usbd2_dbg("   >> block number: Triple\n");
                break;
            case 3:
                usbd2_dbg("   >> block number: Reserved\n");
                break;
            }

            switch (temp & 0x3)
            {
            case 0:
                usbd2_dbg("   >> transfer type: Reserved\n");
                break;
            case 1:
                usbd2_dbg("   >> transfer type: ISOCH\n");
                break;
            case 2:
                usbd2_dbg("   >> transfer type: Bulk\n");
                break;
            case 3:
                usbd2_dbg("   >> transfer type: Interrupt\n");
                break;
            }
        }
        usbd2_dbg("\n");
    }
#endif

    // set Device SOF Mask Timer value as data sheet recommended for HS
    UsbRegWrite(REG_DEV_SMT, 0x44C);

    // set configuration set bit, now allow HW to handle endpoint transfer
    UsbRegMaskedSet(REG_DEV_ADR, AFT_CONF);
}

static int8_t _set_configuration(kdrv_usbd2v_setup_packet_t *setup)
{
    int8_t resp = RESP_STALL;
    uint8_t config_val = setup->wValue & 0xFF;

    usbd2_dbg("[int] <Standard Request> set configuration, config = %d\n", config_val);

    if (config_val == 0)
    {
        _config_state = CONFIG_ADDRESS_STATE;
        // clear configuration set bit
        UsbRegMaskedClr(REG_DEV_ADR, AFT_CONF);
        resp = RESP_ACK;

        // reset all endpoints and terminate all in-progress transfer
        _reset_all_endpoints();
    }
    else if (config_val == 1)
    {
        kdrv_usbd2v_config_descriptor_t *config = _dev_desc->config;

        if (_config_state != CONFIG_CONFIGURED_STATE)
        {
            _config_state = CONFIG_CONFIGURED_STATE;

            kdrv_usbd2v_terminate_all_endpoint();

            // init fifo and endpoint stuff
            _init_fifo_configurations(config->interface);

            _link_status = USBD2_STATUS_CONFIGURED;
            _evt_cb(USBD2_STATUS_CONFIGURED);

            // now that it is configured, we enable link status detection for disconnection status
            _set_link_status_detection(true);

            // unlock transfer API
            for (int enp_no = 1; enp_no <= MAX_ENP_NUM; enp_no++)
                osSemaphoreRelease(_enp_semaphore[enp_no]);
        }

        resp = RESP_ACK;
    }

    return resp;
}

static int8_t handle_standard_request(kdrv_usbd2v_setup_packet_t *setup)
{
    int8_t resp = RESP_STALL;

    // handle requests which are not affected by ep0 halt
    switch (setup->bRequest)
    {
    case 0x0: // GET_STATUS
        if (setup->wValue == 0x0)
        {
            resp = _send_host_device_status();
        }
        break;
    case 0x1: // CLEAR_FEATURE
    {
        if (setup->wValue == 0x0)
        {
            // endpoint halt
            _reset_endpoint(setup->wIndex);
            resp = RESP_ACK;
        }
        break;
    }
    case 0x3: // SET_FEATURE
        break;
    }

    switch (setup->bRequest)
    {
    case 0x5: // SET_ADDRESS
    {
        // USB2.0 spec says should not be greaten than 127
        if (setup->wValue <= 127)
        {
            // set DEVADR and also clear AFT_CONF
            UsbRegWrite(REG_DEV_ADR, setup->wValue);
            resp = RESP_ACK;
        }
    }
    break;
    case 0x6: // GET_DESCRIPTOR
    {
        // low byte: index of specified descriptor type
        uint8_t descp_idx = (setup->wValue & 0xFF);

        // high byte: descriptor type
        switch (setup->wValue >> 8)
        {
        case 1: // DEVICE descriptor
            resp = _send_host_device_descriptor(setup);
            break;
        case 2: // CONFIGURATION descriptor
            resp = _send_host_configuration_descriptors(setup);
            break;
        case 3: // STRING descriptor
            resp = _send_host_string_descriptor(setup, descp_idx);
            break;
        case 4: // INTERFACE descriptor
            break;
        case 5: // ENDPOINT descriptor
            break;
        case 6: // DEVICE_QUALIFIER descriptor
            break;
        case 7: // OTHER_SPEED_CONFIGURATION descriptor
            break;
        case 8: // INTERFACE_POWER descriptor
            break;
        }
    }
    break;
    case 0x7: // SET_DESCRIPTOR
        break;
    case 0x8: // GET_CONFIGURATION
        break;
    case 0x9: // SET_CONFIGURATION
        resp = _set_configuration(setup);
        break;
    }

    return resp;
}

static void _handle_device_interrupts()
{
    uint32_t grp_x_int_status = UsbRegRead(REG_DEV_IGR);

#ifdef USBD2_DBG
    usbd2_dbg("\n[int] !!!!!!!!!! from device group: ");
    for (int i = 0; i < 4; i++)
        if (grp_x_int_status & (0x1 << i))
            usbd2_dbg("%d ", i);
    usbd2_dbg("\n");
#endif

    if (grp_x_int_status & GX_INT_G2_RO)
    {
        uint32_t grp_2_interrupts = (UsbRegRead(REG_DEV_ISG2) & ~(UsbRegRead(REG_DEV_MISG2)));

#ifdef USBD2_DBG
        if (grp_2_interrupts & BIT0)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "USB reset");
        if (grp_2_interrupts & BIT1)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "USB suspend");
        if (grp_2_interrupts & BIT2)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "USB resume");
        if (grp_2_interrupts & BIT3)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "ISOC seq error");
        if (grp_2_interrupts & BIT4)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "ISOC seq abort");
        if (grp_2_interrupts & BIT5)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "sent ZLP done");
        if (grp_2_interrupts & BIT6)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "received ZLP");
        if (grp_2_interrupts & BIT7)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "DMA complete");
        if (grp_2_interrupts & BIT8)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "DMA error");
        if (grp_2_interrupts & BIT9)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "device idle");
        if (grp_2_interrupts & BIT10)
            usbd2_dbg("[int] grp_2_interrupt: %s\n", "ake up by vbus");

        usbd2_dbg("\n");
#endif

        if (grp_2_interrupts & G2_USBRST_INT_RW1C)
        {
            // clear SET_CONFIG state and usb device address
            UsbRegWrite(REG_DEV_ADR, 0x0);

            // clear EP0 STALL bit
            UsbRegMaskedClr(REG_CXCFE, CX_STL);

            // disable (mask) all FIFOs interrupts
            UsbRegWrite(REG_DEV_MISG1, 0xFFFFFFFF);

            // clear all FIFO
            UsbRegMaskedSet(REG_DEV_TST, TST_CLRFF);

            // clear this interrupt bit
            UsbRegWrite(REG_DEV_ISG2, G2_USBRST_INT_RW1C);

            _config_state = CONFIG_DEFAULT_STATE;
        }

        if (grp_2_interrupts & G2_TX0BYTE_INT_RW1C)
        {
            uint32_t zlp_sent_interrupts = UsbRegRead(0x154);
            UsbRegWrite(0x154, zlp_sent_interrupts);

            for (int i = 0; i < MAX_ENP_NUM; i++)
            {
                if (zlp_sent_interrupts & (0x1 << i))
                {
                    int fifo_no = _enp_to_fifo[i + 1];
                    usbd2_dbg("[int] ZLP done, fifo %d IN notify event\n", fifo_no);

                    _fifo_ctx[fifo_no].status = KDRV_STATUS_OK;
                    osEventFlagsSet(_evt_id, (0x1 << fifo_no));
                }
            }

            // clear this interrupt bit
            UsbRegWrite(REG_DEV_ISG2, G2_TX0BYTE_INT_RW1C);
        }
    }

    if (grp_x_int_status & GX_INT_G0_RO)
    {
        uint32_t grp_0_interrupts = (UsbRegRead(REG_DEV_ISG0) & ~(UsbRegRead(REG_DEV_MISG0)));

#ifdef USBD2_DBG
        if (grp_0_interrupts & BIT0)
            usbd2_dbg("[int] grp_0_interrupt: %s\n", "CX SETUP packet");
        if (grp_0_interrupts & BIT1)
            usbd2_dbg("[int] grp_0_interrupt: %s\n", "CX IN");
        if (grp_0_interrupts & BIT2)
            usbd2_dbg("[int] grp_0_interrupt: %s\n", "CX OUT");
        if (grp_0_interrupts & BIT3)
            usbd2_dbg("[int] grp_0_interrupt: %s\n", "CX Status Packet");
        if (grp_0_interrupts & BIT4)
            usbd2_dbg("[int] grp_0_interrupt: %s\n", "CX Failed");
        if (grp_0_interrupts & BIT5)
            usbd2_dbg("[int] grp_0_interrupt: %s\n", "CX abort");
#endif

        if (grp_0_interrupts & G0_CX_SETUP_INT_RO)
        {
            static kdrv_usbd2v_setup_packet_t setup_packet = {0};
            memset(&setup_packet, 0, sizeof(setup_packet));
            UsbRegWrite(0x304, _dma_remap_addr((uint32_t)&setup_packet));
            UsbRegWrite(0x300, (8 << 8) | BIT0);

            // polling VDMA complete
            while ((UsbRegRead(0x328) & BIT0) == 0)
                ;
            UsbRegWrite(0x328, BIT0);

            usbd2_dbg("[int] VDMA recv CX SETUP : bmRequestType 0x%x bRequest 0x%x wValue 0x%x wIndex 0x%x wLength %d\n",
                      setup_packet.bmRequestType, setup_packet.bRequest, setup_packet.wValue,
                      setup_packet.wIndex, setup_packet.wLength);

            int8_t resp;
            uint8_t bmRequestType_type = ((setup_packet.bmRequestType & 0x60) >> 5);

            switch (bmRequestType_type)
            {
            case 0: // Standard request
                usbd2_dbg("[int] <Standard Request> ....\n");
                resp = handle_standard_request(&setup_packet);
                break;
            case 1: // Class request
            case 2: // Vendor request
                usbd2_dbg("[int] <Class or Vendor Request> ....\n");
                if (_usr_cx_cb(&setup_packet) == true)
                    resp = RESP_ACK;
                else
                    resp = RESP_STALL;
                break;
            default:
                usbd2_dbg("[int] <Unknown Request> ....\n");
                resp = RESP_STALL;
                break;
            }

            if (resp == RESP_ACK)
            {
                usbd2_dbg("[int] setting CX_DONE ....\n");
                // indicate an OK request to host
                UsbRegMaskedSet(REG_CXCFE, CX_DONE);
            }
            else if (resp == RESP_STALL)
            {
                usbd2_dbg("[int] setting CX_STL ....\n");

                // indicate a request error to host
                UsbRegMaskedSet(REG_CXCFE, CX_STL | CX_DONE);
            }

            // else RESP_LATER due to VDMA in transfer
        }
    }

    if (grp_x_int_status & GX_INT_G3_RO)
    {
        usbd2_dbg("[int] VDMA interrupts = 0x%x\n", UsbRegRead(0x328));

        uint32_t vdma_intrp = UsbRegRead(0x328);
        UsbRegWrite(0x328, vdma_intrp);

        if (vdma_intrp & BIT0)
        {
            // VDMA CX complete
            usbd2_dbg("[int] VDMA CX complete\n");
            usbd2_dbg("[int] setting CX_DONE ....\n");

            UsbRegMaskedSet(REG_CXCFE, CX_DONE);
        }

        if (vdma_intrp & (BIT1 | BIT2 | BIT3 | BIT4))
        {
            // VDMA FIFO complete
            for (uint32_t fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
            {
                // find which FIFO
                if (vdma_intrp & (0x2 << fifo_no))
                {
                    // IN or OUT
                    if (_fifo_ctx[fifo_no].dir == 0)
                    {
                        usbd2_dbg("[int] VDMA fifo %d OUT complete\n", fifo_no);
                        uint32_t left_size = UsbRegRead(0x308 + 8 * fifo_no) >> 8;

                        if (left_size > 0 || _fifo_ctx[fifo_no].user_buf_len == 0)
                        {
                            usbd2_dbg("[int] VDMA fifo %d OUT transfer done, last recv size %d\n", fifo_no, VDMA_ONE_TXFER_SIZE - left_size);

                            _fifo_ctx[fifo_no].received_length -= left_size;
                            _fifo_ctx[fifo_no].status = KDRV_STATUS_OK;
                            osEventFlagsSet(_evt_id, (0x1 << fifo_no));
                            continue;
                        }

                        uint32_t vdma_txfer_size = MIN(_fifo_ctx[fifo_no].user_buf_len, VDMA_ONE_TXFER_SIZE);

                        usbd2_dbg("[int] VDMA fifo %d OUT next transfer addr 0x%x size %d\n",
                                  fifo_no, UsbRegRead(0x30C + 8 * fifo_no), vdma_txfer_size);

                        _fifo_ctx[fifo_no].user_buf_len -= vdma_txfer_size;
                        _fifo_ctx[fifo_no].received_length += vdma_txfer_size;

                        UsbRegWrite(0x308 + 8 * fifo_no, (vdma_txfer_size << 8) | 0x1);
                    }
                    else
                    {
                        if (_fifo_ctx[fifo_no].user_buf_len == 0)
                        {
                            usbd2_dbg("[int] VDMA fifo %d IN transfer all done\n", fifo_no);

                            if (_fifo_ctx[fifo_no].send_ZLP)
                            {
                                usbd2_dbg("[int] fifo %d IN send ZLP\n", fifo_no);
                                int enp_no = _fifo_to_enp[fifo_no];
                                UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (enp_no - 1), TX0BYTE_IEPn);
                            }
                            else
                            {
                                usbd2_dbg("[int] short pkt, fifo %d IN notify event\n", fifo_no);
                                _fifo_ctx[fifo_no].status = KDRV_STATUS_OK;
                                osEventFlagsSet(_evt_id, (0x1 << fifo_no));
                            }

                            continue;
                        }
                        usbd2_dbg("[int] VDMA fifo %d IN complete\n", fifo_no);

                        uint32_t left_size = UsbRegRead(0x308 + 8 * fifo_no) >> 8;
                        if (left_size != 0)
                            usbd2_dbg("[int] VDMA fifo %d IN error !!!, left_size = %d\n", fifo_no, left_size);

                        uint32_t vdma_txfer_size = MIN(_fifo_ctx[fifo_no].user_buf_len, VDMA_ONE_TXFER_SIZE);

                        usbd2_dbg("[int] VDMA fifo %d IN next transfer addr 0x%x size %d\n",
                                  fifo_no, UsbRegRead(0x30C + 8 * fifo_no), vdma_txfer_size);

                        _fifo_ctx[fifo_no].user_buf_len -= vdma_txfer_size;

                        UsbRegWrite(0x308 + 8 * fifo_no, (vdma_txfer_size << 8) | BIT1 | BIT0);
                    }
                }
            }
        }

        if (vdma_intrp & (BIT16 | BIT17 | BIT18 | BIT19 | BIT20))
        {
            usbd2_dbg("[int] VDMA error !!!!!!!!\n");
        }
    }
}

// USB ISR
static void _usbd2_isr(void)
{
    uint32_t global_int = UsbRegRead(REG_GLB_ISR);

    if (global_int & DEV_INT)
        _handle_device_interrupts();

    if (global_int & OTG_INT)
    {
        // turn off status detection because it can be triggered forever if disconnected
        _set_link_status_detection(false);
        _link_status = USBD2_STATUS_DISCONNECTED;
        _evt_cb(USBD2_STATUS_DISCONNECTED);

        // reset endpoint to make transfer get terminated
        _reset_all_endpoints();
    }
}

static void _usbd2_init_register_isr(void)
{
    SCU_EXTREG_USB_OTG_CTRL_SET_EXTCTRL_SUSPENDM(1);
    SCU_EXTREG_USB_OTG_CTRL_SET_u_iddig(1);
    SCU_EXTREG_USB_OTG_CTRL_SET_wakeup(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_l1_wakeup(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_OSCOUTEN(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_PLLALIV(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_XTLSEL(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_OUTCLKSEL(0);

    _set_link_status_detection(false);

    // enable Device and OTG interrupt
    UsbRegWrite(REG_GLB_INT, ~(INT_POLARITY | DEV_INT | OTG_INT) & 0xF);

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_GLB_INT);
        usbd2_dbg("\nsetting global interrupt register [0x%03X]:\n", REG_GLB_INT);
        usbd2_dbg("- interrupt active %s\n", (reg_value & 0x8) ? "high" : "low");
        usbd2_dbg("- host interrupt : %s\n", (reg_value & 0x4) ? "X" : "O");
        usbd2_dbg("- OTG interrupt : %s\n", (reg_value & 0x2) ? "X" : "O");
        usbd2_dbg("- device interrupt : %s\n", (reg_value & 0x1) ? "X" : "O");
        usbd2_dbg("\n");
    }
#endif

    // listen all 4 groups for 0x140
    //UsbRegWrite(REG_DEV_MIGR, GX_INT_G1_RO | GX_INT_G2_RO);
    //UsbRegWrite(REG_DEV_MIGR, GX_INT_G1_RO);
    UsbRegWrite(REG_DEV_MIGR, 0);

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_DEV_MIGR);
        usbd2_dbg("\nsetting groups interrupt enable register [0x%03X]:\n", REG_DEV_MIGR);
        for (int i = 0; i < 4; i++)
            usbd2_dbg("- group %d interrupt : %s\n", i, (reg_value & (0x1 << i)) ? "X" : "O");
        usbd2_dbg("\n");
    }
#endif

    UsbRegWrite(REG_DEV_MISG0, 0);

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_DEV_MISG0);
        usbd2_dbg("\nsetting group 0 interrupt register [0x%03X]:\n", REG_DEV_MISG0);
        usbd2_dbg("- CX abort interrupt : %s\n", (reg_value & 0x20) ? "X" : "O");
        usbd2_dbg("- CX Failed interrupt : %s\n", (reg_value & 0x10) ? "X" : "O");
        usbd2_dbg("- CX Status Packet interrupt : %s\n", (reg_value & 0x8) ? "X" : "O");
        usbd2_dbg("- CX OUT interrupt : %s\n", (reg_value & 0x4) ? "X" : "O");
        usbd2_dbg("- CX IN interrupt : %s\n", (reg_value & 0x2) ? "X" : "O");
        usbd2_dbg("- CX SETUP interrupt : %s\n", (reg_value & 0x1) ? "X" : "O");
        usbd2_dbg("\n");
    }
#endif

    // grop 1 interrupt mask
    // at initialization turn off all fifo interrupts
    // for FIFO IN, it triggers interrupt while FIFO is empty !
    UsbRegWrite(REG_DEV_MISG1, 0xFFFFFFFF);

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_DEV_MISG1);
        usbd2_dbg("\nsetting group 1 interrupt register [0x%03X]:\n", REG_DEV_MISG1);
        for (int i = 0; i < 4; i++)
        {
            uint32_t temp1 = reg_value >> (i * 2);
            uint32_t temp2 = reg_value >> (16 + i);
            usbd2_dbg("- FIFO %d interrupt >> OUT: %s, ShortPkt: %s, IN: %s\n",
                      i, (temp1 & 0x1) ? "off" : "on", (temp1 & 0x2) ? "off" : "on",
                      (temp2 & 0x1) ? "off" : "on");
        }
        usbd2_dbg("\n");
    }
#endif

    // grop 2 interrupt mask
    UsbRegWrite(REG_DEV_MISG2,
                ~(G2_TX0BYTE_INT_RW1C | G2_USBRST_INT_RW1C));

#ifdef USBD2_DBG
    {
        uint32_t reg_value = UsbRegRead(REG_DEV_MISG2);
        usbd2_dbg("\nsetting group 2 interrupt register [0x%03X]:\n", REG_DEV_MISG2);
        usbd2_dbg("- wake up by vbus interrupt : %s\n", (reg_value & 0x40) ? "X" : "O");
        usbd2_dbg("- device idle interrupt : %s\n", (reg_value & 0x200) ? "X" : "O");
        usbd2_dbg("- DMA error interrupt : %s\n", (reg_value & 0x100) ? "X" : "O");
        usbd2_dbg("- DMA complete interrupt : %s\n", (reg_value & 0x80) ? "X" : "O");
        usbd2_dbg("- received ZLP interrupt : %s\n", (reg_value & 0x40) ? "X" : "O");
        usbd2_dbg("- sent ZLP interrupt : %s\n", (reg_value & 0x20) ? "X" : "O");
        usbd2_dbg("- ISOC seq abort interrupt : %s\n", (reg_value & 0x10) ? "X" : "O");
        usbd2_dbg("- ISOC seq error interrupt : %s\n", (reg_value & 0x8) ? "X" : "O");
        usbd2_dbg("- resume interrupt : %s\n", (reg_value & 0x4) ? "X" : "O");
        usbd2_dbg("- suspend interrupt : %s\n", (reg_value & 0x2) ? "X" : "O");
        usbd2_dbg("- bus reset interrupt : %s\n", (reg_value & 0x1) ? "X" : "O");
        usbd2_dbg("\n");
    }
#endif

    // set device idle counter = 7ms
    UsbRegWrite(REG_DEV_ICR, 0x7);

    // device soft reset
    UsbRegMaskedSet(REG_DEV_CTL, BIT4);

    // clear all FIFO counter
    UsbRegMaskedSet(REG_DEV_TST, BIT0);

    // enable chip
    UsbRegMaskedSet(REG_DEV_CTL, BIT5);

    // clear all interrupts status for RW1C bits
    UsbRegWrite(REG_OTG_ISR, 0xFFFFFFFF);
    UsbRegWrite(REG_DEV_ISG0, 0xFFFFFFFF);
    UsbRegWrite(REG_DEV_ISG2, 0xFFFFFFFF);

    // global interrupt enable
    UsbRegMaskedSet(REG_DEV_CTL, BIT2);

    UsbRegWrite(0x330, 1); // init VDMA

    NVIC_SetVector(OTG_SBS_3_IRQ, (uint32_t)_usbd2_isr);

    // Clear and Enable SAI IRQ
    //NVIC_ClearPendingIRQ(OTG_SBS_3_IRQ);
    NVIC_EnableIRQ(OTG_SBS_3_IRQ);
}

static void _default_status_isr_callback(kdrv_usbd2v_link_status_t event)
{
}

static bool _default_usr_cx_isr_callback(kdrv_usbd2v_setup_packet_t *setup)
{
    return false; // RESP_STALL
}

////////////////// below are API ////////////////////

kdrv_status_t kdrv_usbd2v_initialize(
    kdrv_usbd2v_device_descriptor_t *dev_desc,
    kdrv_usbd2v_string_descriptor_t *dev_str_desc,
    kdrv_usbd2v_link_status_callback_t status_isr_cb,
    kdrv_usbd2v_user_control_callback_t usr_cx_isr_cb)
{

    _dev_desc = dev_desc;
    _dev_str_desc = dev_str_desc;
    _evt_cb = (status_isr_cb == NULL) ? _default_status_isr_callback : status_isr_cb;
    _usr_cx_cb = (usr_cx_isr_cb == NULL) ? _default_usr_cx_isr_callback : usr_cx_isr_cb;

    _evt_id = osEventFlagsNew(NULL);

    _usbd2_init_register_isr();

    for (int enp_no = 1; enp_no <= MAX_ENP_NUM; enp_no++)
        _enp_semaphore[enp_no] = osSemaphoreNew(1, 0, NULL); // at first it is locked

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2v_uninitialize(void)
{
    osEventFlagsDelete(_evt_id);

    for (int enp_no = 1; enp_no <= MAX_ENP_NUM; enp_no++)
    {
        osSemaphoreDelete(_enp_semaphore[enp_no]);
        _enp_semaphore[enp_no] = NULL;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2v_set_enable(bool enable)
{
    if ((UsbRegRead(REG_PHY_TST) & TST_JSTA) == 0)
    {
        UsbRegMaskedSet(REG_PHY_TST, TST_JSTA);
        osDelay(5);
    }

    if (enable)
        // Make PHY work properly, FIXME ?
        UsbRegMaskedClr(REG_PHY_TST, TST_JSTA);
    else
        // Make PHY not work, FIXME ?
        UsbRegMaskedSet(REG_PHY_TST, TST_JSTA);

    return KDRV_STATUS_OK;
}

kdrv_usbd2v_link_status_t kdrv_usbd2v_get_link_status(void)
{
    return _link_status;
}

kdrv_status_t kdrv_usbd2v_reset_device()
{
    usbd2_dbg("%s()\n", __FUNCTION__);

    // disable bus
    kdrv_usbd2v_set_enable(false);

    // reset all endpoints and terminate all in-progress transfer
    _reset_all_endpoints();

    // some delay for USB bus, FIXME ?
    osDelay(100);

    _config_state = CONFIG_DEFAULT_STATE;

    // re-init registers and isr
    _usbd2_init_register_isr();

    // re-enable bus
    kdrv_usbd2v_set_enable(true);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2v_terminate_all_endpoint(void)
{
    usbd2_dbg("%s()\n", __FUNCTION__);

    UsbRegWrite(0x330, 0); // stop VDMA

    for (int enp_no = 1; enp_no <= MAX_ENP_NUM; enp_no++)
    {
        int fifo_no = _enp_to_fifo[enp_no];
        if (fifo_no >= 0)
        {
            UsbRegWrite(0x1B0 + 4 * fifo_no, FFRST);

            if (_fifo_ctx[fifo_no].isTransferring)
            {
                // notify transfer done to user
                _fifo_ctx[fifo_no].status = KDRV_STATUS_USBD_TRANSFER_TERMINATED;
                osEventFlagsSet(_evt_id, (0x1 << fifo_no));
            }
        }
    }

    UsbRegWrite(0x330, 1); // start VDMA

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2v_bulk_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    kdrv_status_t status;
    uint8_t enp_no = endpoint & 0xF;
    uint8_t fifo_no = _enp_to_fifo[enp_no];

    osSemaphoreAcquire(_enp_semaphore[enp_no], osWaitForever);

    usbd2_dbg("[bulk_send] enp 0x%x fifo_no %d buf 0x%x txLen %d\n", endpoint, fifo_no, buf, txLen);

    _fifo_ctx[fifo_no].send_ZLP = ((txLen & 0x1FF) == 0) ? true : false;

    uint32_t vdma_txfer_size = MIN(txLen, VDMA_ONE_TXFER_SIZE);
    txLen -= vdma_txfer_size;

    _fifo_ctx[fifo_no].isTransferring = true;
    _fifo_ctx[fifo_no].user_buf_len = txLen;
    _fifo_ctx[fifo_no].dir = 1;

    osEventFlagsClear(_evt_id, (0x1 << fifo_no));

    UsbRegWrite(0x30C + 8 * fifo_no, _dma_remap_addr((uint32_t)buf));
    UsbRegWrite(0x308 + 8 * fifo_no, (vdma_txfer_size << 8) | BIT1 | BIT0);

    uint32_t flags = osEventFlagsWait(_evt_id, (0x1 << fifo_no), osFlagsWaitAny, timeout_ms);
    if (flags == osFlagsErrorTimeout)
        status = KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
    else
        status = _fifo_ctx[fifo_no].status;

    _fifo_ctx[fifo_no].isTransferring = false;

    usbd2_dbg("[bulk_send] enp 0x%x status %d\n", endpoint, status);

    if (status != KDRV_STATUS_USBD_TRANSFER_DISCONNECTED)
        osSemaphoreRelease(_enp_semaphore[enp_no]);

    return status;
}

#define FAKE_ZLP 0x11223344

kdrv_status_t kdrv_usbd2v_bulk_receive(uint32_t endpoint, uint32_t *buf, uint32_t *blen, uint32_t timeout_ms)
{
    kdrv_status_t status;
    uint8_t enp_no = endpoint & 0xF;
    uint8_t fifo_no = _enp_to_fifo[enp_no];

    osSemaphoreAcquire(_enp_semaphore[enp_no], osWaitForever);

    usbd2_dbg("[bulk_recv] enp 0x%x fifo_no %d buf 0x%x buf_len %d\n", endpoint, fifo_no, buf, *blen);

    uint32_t left_buf_size = *blen;
    uint32_t vdma_txfer_size = MIN(left_buf_size, VDMA_ONE_TXFER_SIZE);

    left_buf_size -= vdma_txfer_size;

    _fifo_ctx[fifo_no].isTransferring = true;
    _fifo_ctx[fifo_no].user_buf_len = left_buf_size;
    _fifo_ctx[fifo_no].received_length = vdma_txfer_size; // pre value
    _fifo_ctx[fifo_no].dir = 0;

    osEventFlagsClear(_evt_id, (0x1 << fifo_no));

    UsbRegWrite(0x30C + 8 * fifo_no, _dma_remap_addr((uint32_t)buf));
    UsbRegWrite(0x308 + 8 * fifo_no, (vdma_txfer_size << 8) | BIT0);

    uint32_t flags = osEventFlagsWait(_evt_id, (0x1 << fifo_no), osFlagsWaitAny, timeout_ms);
    if (flags == osFlagsErrorTimeout)
        status = KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
    else
    {
        *blen = _fifo_ctx[fifo_no].received_length;
        status = _fifo_ctx[fifo_no].status;

        if (((*blen - 4) & 0x1FF) == 0 && *(uint32_t *)((uint32_t)buf + *blen - 4) == FAKE_ZLP)
        {
            usbd2_dbg("[bulk_recv] enp 0x%x recv fake ZLP\n", endpoint);
            *blen -= 4;
        }
    }

    _fifo_ctx[fifo_no].isTransferring = false;

    usbd2_dbg("[bulk_recv] enp 0x%x recv %d status %d\n", endpoint, *blen, status);

    if (status != KDRV_STATUS_USBD_TRANSFER_DISCONNECTED)
        osSemaphoreRelease(_enp_semaphore[enp_no]);

    return status;
}

bool kdrv_usbd2v_interrupt_send_check_buffer_empty(uint32_t endpoint)
{
    uint8_t enp_no = endpoint & 0xF;
    uint8_t fifo_no = _enp_to_fifo[enp_no];
    return !!(UsbRegRead(REG_CXCFE) & (F_EMP_0 << fifo_no));
}

kdrv_status_t kdrv_usbd2v_interrupt_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    kdrv_status_t status;
    uint8_t enp_no = endpoint & 0xF;
    uint8_t fifo_no = _enp_to_fifo[enp_no];

    osSemaphoreAcquire(_enp_semaphore[enp_no], osWaitForever);

    // clear FIFO content before transmission
    UsbRegWrite(0x1B0 + 4 * fifo_no, FFRST);

    uint32_t vdma_txfer_size = MIN(txLen, 1024);

    _fifo_ctx[fifo_no].isTransferring = true;
    _fifo_ctx[fifo_no].user_buf_len = 0;
    _fifo_ctx[fifo_no].dir = 1;

    osEventFlagsClear(_evt_id, (0x1 << fifo_no));

    UsbRegWrite(0x30C + 8 * fifo_no, _dma_remap_addr((uint32_t)buf));
    UsbRegWrite(0x308 + 8 * fifo_no, (vdma_txfer_size << 8) | BIT1 | BIT0);

    uint32_t flags = osEventFlagsWait(_evt_id, (0x1 << fifo_no), osFlagsWaitAny, timeout_ms);
    if (flags == osFlagsErrorTimeout)
        status = KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
    else
        status = _fifo_ctx[fifo_no].status;

    _fifo_ctx[fifo_no].isTransferring = false;

    if (status != KDRV_STATUS_USBD_TRANSFER_DISCONNECTED)
        osSemaphoreRelease(_enp_semaphore[enp_no]);

    return status;
}
