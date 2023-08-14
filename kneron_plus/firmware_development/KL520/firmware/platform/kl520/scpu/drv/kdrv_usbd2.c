/*
 * Kneron USBD API
 *
 * Copyright (C) s/2019/2020/ Kneron, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "cmsis_os2.h"
#include "base.h"
#include "kdrv_scu_ext.h"
#include "kdrv_usbd2.h"

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

#define FIFO_NUM 4 // we have 4 FIFOs, each has 1-KB

#define UsbRegRead(reg_offset) inw(USB_FOTG210_PA_BASE + reg_offset)
#define UsbRegWrite(reg_offset, val) outw(USB_FOTG210_PA_BASE + reg_offset, val)
#define UsbRegMaskedSet(reg_offset, val) masked_outw((USB_FOTG210_PA_BASE + reg_offset), (val), (val))
#define UsbRegMaskedClr(reg_offset, val) masked_outw((USB_FOTG210_PA_BASE + reg_offset), 0, (val))

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
    RESP_NACK = 1, /* busy now */
    RESP_ACK,      /* reqeust is done */
    RESP_STALL,    /* request is not supported */
};

// a data struct for FIFO and DMA control
typedef struct
{
    // below are initialized once when endpoint is configured
    uint8_t enabled;          // indicate that this FIFO is enabled
    uint8_t enpNo;            // endpoint no (without direction bit)
    uint32_t endpointAddress; // endpoint address (with direction bit)
    uint32_t maxPacketSize;   // the wMaxPacketSize
    uint8_t transferType;     // Control/Iso/Bulk/Interrupt
    uint32_t byteCntReg;      // FIFO byte count register address

    // below are used while transferring data
    uint8_t isTransferring;   // indicate it is in transferring progress
    uint32_t user_buf_addr;   // user commited buffer address for transfer
    uint32_t user_buf_len;    // user buffer length
    uint32_t received_length; // for Out only

    // below variable represents short packet is coming for bulk out
    // or zero-length packet for bulk in
    // 1: True, 0: False
    uint8_t short_or_zl_packet;

    // per-endpoint semaphore
    osSemaphoreId_t semaphore;

    kdrv_status_t status; // internal use for blocking API
} FIFO_Ctrl_t;

// define usb device mode control block
typedef struct
{
    bool ep0_halted;
    osEventFlagsId_t evt_id; // internal use for blocking API
    kdrv_usbd2_device_descriptor_t *dev_desc;
    kdrv_usbd2_device_qualifier_descriptor_t *dev_qual_desc; // is it necessary ?
    kdrv_usbd2_string_descriptor_t *dev_str_desc;
    kdrv_usbd2_link_status_callback_t evt_cb;
    kdrv_usbd2_user_control_callback_t usr_cx_cb;
    uint32_t config_state;
    kdrv_usbd2_link_status_t link_status;
    FIFO_Ctrl_t fifo_cbs[FIFO_NUM]; // FIFO control blocks
} USBD2_Ctrl_t;

enum
{
    READ_FIFO = 0,
    WRITE_FIFO = 1,
};

// an usb device mode object for internal use
USBD2_Ctrl_t usbd2_ctrl =
    {
        .ep0_halted = false,
        .dev_desc = NULL,
        .config_state = CONFIG_DEFAULT_STATE,
        .link_status = USBD2_STATUS_DISCONNECTED,
        .fifo_cbs = {0},
};

static osMutexId_t mutex_usbd = NULL;

kdrv_status_t _reset_endpoint(uint32_t endpoint);

static bool _isInterrupt()
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

static bool _dma_is_busy()
{
    return (UsbRegRead(REG_DMA_TFN) != DMA_TARGET_ACC_NONE);
}

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

// fifo to memory or memory to fifo transfer
// fifo_dir = 1 : memory -> fifo
// fifo_dir = 0 : fifo -> memory
// dma polling way
static bool _dma_fifo_transfer_sync(uint32_t *addr, uint32_t len, uint32_t fifo_sel, uint8_t fifo_dir)
{
    if (_dma_is_busy())
        return false;

    bool status = false;

    // set DMA FIFO selection to accuire DMA
    UsbRegWrite(REG_DMA_TFN, fifo_sel);

    // temporarily disable DMA complt interrupt
    // because we will poll it here
    UsbRegMaskedSet(REG_DEV_MISG2, G2_DMA_CMPLT_RW1C);

    // set DMA address
    UsbRegWrite(REG_DMA_CPS2, _dma_remap_addr((uint32_t)addr));

    // set DMA transfer size
    UsbRegWrite(REG_DMA_CPS1, len << 8);

    if (fifo_dir == WRITE_FIFO)
        UsbRegMaskedSet(REG_DMA_CPS1, BIT1);

    // start DMA
    UsbRegMaskedSet(REG_DMA_CPS1, DMA_START);

    int i;
    // FIXME: why 500 ? just to prevent from being dead forever
    for (i = 0; i < 500; i++)
    {
        // polling DMA completion status
        if (UsbRegRead(REG_DEV_ISG2) & G2_DMA_CMPLT_RW1C)
        {
            UsbRegWrite(REG_DEV_ISG2, G2_DMA_CMPLT_RW1C);
            status = true;
            break;
        }
    }

    // re-enable DMA complt interrupt
    UsbRegMaskedClr(REG_DEV_MISG2, G2_DMA_CMPLT_RW1C);

    // clear DMA FIFO selection
    UsbRegWrite(REG_DMA_TFN, DMA_TARGET_ACC_NONE);

    return status;
}

static bool _dma_fifo_transfer_sync_try(uint32_t *addr, uint32_t len, uint32_t fifo_sel, uint8_t fifo_dir, uint32_t try_count)
{
    bool status = false;
    for (int i = 0; i < try_count; i++)
        if (_dma_fifo_transfer_sync(addr, len, fifo_sel, fifo_dir))
        {
            status = true;
            break;
        }
    return status;
}

// out transfer
// configure DMA settings for fifo-to-memory for non-control transfer
// this implementation follows FOTG210 data sheet : 6.2.4 "Programming DMA"
static bool _enable_dma_read_fifo(uint32_t fifo_no, FIFO_Ctrl_t *fifocb)
{
    if (_dma_is_busy())
        return false;

    // select FIFO for DMA
    UsbRegWrite(REG_DMA_TFN, 0x1 << fifo_no);

    uint32_t fifo_bytecnt = UsbRegRead(fifocb->byteCntReg) & BC_Fn;
    // can transfer only minimum size betwwen FIFO byte count and user buffer residual size
    uint32_t transfer_size = MIN(fifo_bytecnt, fifocb->user_buf_len);

    // set DMA memory addr
    UsbRegWrite(REG_DMA_CPS2, fifocb->user_buf_addr);

    // set DMA_LEN and DMA_TYPE = FIFO_to_Memory
    UsbRegWrite(REG_DMA_CPS1, transfer_size << 8);

    // start DMA
    UsbRegMaskedSet(REG_DMA_CPS1, DMA_START);

    fifocb->user_buf_addr += transfer_size;
    fifocb->user_buf_len -= transfer_size;
    fifocb->received_length += transfer_size;

    return true;
}

// in trasnfer
// configure DMA settings for memory-to-fifo for non-control transfer
// this implementation follows FOTG210 data sheet : 6.2.4 "Programming DMA"
static bool _enable_dma_write_fifo(uint32_t fifo_no, FIFO_Ctrl_t *fifocb)
{
    if (_dma_is_busy())
        return false;

    // select FIFO for DMA
    UsbRegWrite(REG_DMA_TFN, 0x1 << fifo_no);

    // can transfer only minimum size betwwen MaxPacketSize and user buffer residual size
    uint32_t transfer_size = MIN(fifocb->maxPacketSize, fifocb->user_buf_len);

    // set DMA memory addr
    UsbRegWrite(REG_DMA_CPS2, fifocb->user_buf_addr);

    // set DMA_LEN and DMA_TYPE = Memory_to_FIFO
    UsbRegWrite(REG_DMA_CPS1, (transfer_size << 8) | 0x2);

    // start DMA
    UsbRegMaskedSet(REG_DMA_CPS1, DMA_START);

    fifocb->user_buf_addr += transfer_size;
    fifocb->user_buf_len -= transfer_size;

    return true;
}

static void _reset_all_endpoints()
{
    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;
    for (int fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
        if (fifo_cb[fifo_no].enabled)
        {
            _reset_endpoint(fifo_cb[fifo_no].endpointAddress);
        }
}

static void _bus_reset_work()
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

    usbd2_ctrl.config_state = CONFIG_DEFAULT_STATE;
}

static void _handle_dma_error()
{
    // clear this interrupt bit
    UsbRegWrite(REG_DEV_ISG2, G2_DMA_ERROR_RW1C);

    // check which fifo-endpoint is responsible for this

    uint32_t fifoSel = UsbRegRead(REG_DMA_TFN);

    // clear FIFO sel
    UsbRegWrite(REG_DMA_TFN, DMA_TARGET_ACC_NONE);

    uint32_t fifo_no;
    for (fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
        if (fifoSel & (0x1 << fifo_no))
            break;

    FIFO_Ctrl_t *fifo_cb = &(usbd2_ctrl.fifo_cbs[fifo_no]);

    // reset the endpoint due to DMA error
    _reset_endpoint(fifo_cb[fifo_no].endpointAddress);
}

static int8_t _endpoint_to_fifo(uint32_t endpoint)
{
    // check fifo ctrl blocks to get fifo no
    int8_t fifo_no;
    for (fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
        if (usbd2_ctrl.fifo_cbs[fifo_no].endpointAddress == endpoint)
            break;

    return (fifo_no < 4) ? fifo_no : -1;
}

static void _clean_fifo_ctrl(FIFO_Ctrl_t *fifo)
{
    fifo->user_buf_addr = 0;
    fifo->user_buf_len = 0;
    fifo->short_or_zl_packet = false;
}

static inline void _handle_fifo_short_pkt_receive_interrupts(uint32_t interrupt_bits)
{
    for (int fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
    {
        // make sure both OUT and SPK interrupts bits get asserted at the same time !
        if (((interrupt_bits >> (fifo_no * 2)) & (G1_F0_SPK_INT_RO | G1_F0_OUT_INT_RO)) == 0x3)
        {

            // with a short packet coming means data is less than MaxPacketSize
            // and it is the last packet transferd by host
            // this will be handled in DMA completeion time
            usbd2_ctrl.fifo_cbs[fifo_no].short_or_zl_packet = true;

            // disable short packet interrupt
            // shoudl be re-enabled at next DMA completeion time
            UsbRegMaskedSet(REG_DEV_MISG1, MF0_SPK_INT << (fifo_no * 2));

            break;
        }
    }
}

static inline void _handle_fifo_out_receive_interrupts(uint32_t interrupt_bits)
{
    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;

    for (int fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
    {
        // handle interrupts for each fifo
        if (interrupt_bits & (G1_F0_OUT_INT_RO << (fifo_no * 2)))
        {
            // if no user buffer is commited, means that this is a start of OUT transfer
            if (fifo_cb[fifo_no].user_buf_addr == 0)
            {
                // disable this OUT interrrupt
                // it should be re-enabled once user commits a buffer for DMA
                UsbRegMaskedSet(REG_DEV_MISG1, MF0_OUT_INT << (fifo_no * 2));
            }
            else // configure DMA transfer for FIFO to user buffer
            {
                if (fifo_cb[fifo_no].user_buf_len > 0 &&
                    _enable_dma_read_fifo(fifo_no, &fifo_cb[fifo_no]) == true)
                {
                    // DMA has been started, disable this OUT interrupt
                    // and re-enable the interrupt at DMA completion
                    UsbRegMaskedSet(REG_DEV_MISG1, MF0_OUT_INT << (fifo_no * 2));
                }
            }
        }
    }
}

static inline void _handle_fifo_in_send_interrupts(uint32_t interrupt_bits)
{
    // this handles only one fifo interrupt

    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;

    uint32_t fifo_no;
    for (fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
        if (interrupt_bits & (G1_F0_IN_INT_RO << fifo_no))
            break;

    // disable (mask) the FIFO IN interrupt immediately to prevent from interrupt re-trigger
    UsbRegMaskedSet(REG_DEV_MISG1, MF0_IN_INT << fifo_no);

    if (fifo_cb[fifo_no].user_buf_len > 0)
    {
        if (_enable_dma_write_fifo(fifo_no, &fifo_cb[fifo_no]) == false)
        {
            // re-enable the interrupt for next try
            UsbRegMaskedClr(REG_DEV_MISG1, MF0_IN_INT << fifo_no);
        }
    }
    else
    {
        // FIFO is empty and no more data to send, in other words, the bulk-in transfer is done

        // send zero-length packet if needed
        if (fifo_cb[fifo_no].short_or_zl_packet)
        {
            UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (fifo_cb[fifo_no].enpNo - 1), TX0BYTE_IEPn);
            // FIXME: should check G2_TX0BYTE_INT_RW1C in later interrupts
        }

        fifo_cb[fifo_no].status = KDRV_STATUS_OK;
        osEventFlagsSet(usbd2_ctrl.evt_id, (0x1 << fifo_no));
    }
}

#define FIFO_SHORT_PKT_INTERRUPTS (G1_F0_SPK_INT_RO | G1_F1_SPK_INT_RO | G1_F2_SPK_INT_RO | G1_F3_SPK_INT_RO)
#define FIFO_OUT_INTERRUPTS (G1_F0_OUT_INT_RO | G1_F1_OUT_INT_RO | G1_F2_OUT_INT_RO | G1_F3_OUT_INT_RO)
#define FIFO_IN_INTERRUPTS (G1_F0_IN_INT_RO | G1_F1_IN_INT_RO | G1_F2_IN_INT_RO | G1_F3_IN_INT_RO)

static void _handle_fifo_interrupts()
{
    uint32_t fifo_interrupts = UsbRegRead(REG_DEV_ISG1) & ~(UsbRegRead(REG_DEV_MISG1));

    // handle OUT short packets interrupts for short packets
    if (fifo_interrupts & FIFO_SHORT_PKT_INTERRUPTS)
        _handle_fifo_short_pkt_receive_interrupts(fifo_interrupts);

    // handle OUT FIFO interrupts
    if (fifo_interrupts & FIFO_OUT_INTERRUPTS)
        _handle_fifo_out_receive_interrupts(fifo_interrupts);

    // handle IN FIFO interrupts
    if (fifo_interrupts & FIFO_IN_INTERRUPTS)
        _handle_fifo_in_send_interrupts(fifo_interrupts);
}

// FIXME: this does not handle CX DMA complete
static void _handle_dma_complete_interrupt()
{
    // this handles only one DMA complete interrupt
    uint32_t fifo_no;
    uint32_t fifoSel = UsbRegRead(REG_DMA_TFN);
    uint32_t dmaCPS1 = UsbRegRead(REG_DMA_CPS1);

    // clear FIFO selection
    UsbRegWrite(REG_DMA_TFN, DMA_TARGET_ACC_NONE);

    // clear DMA completion interrupt
    UsbRegWrite(REG_DEV_ISG2, G2_DMA_CMPLT_RW1C);

    for (fifo_no = 0; fifo_no < FIFO_NUM; fifo_no++)
        if (fifoSel & (0x1 << fifo_no))
            break;

    // check DMA-FIFO direction
    if (dmaCPS1 & DMA_TYPE)
    {
        // Memory-to-FIFO, is IN (send data) transfer
        // re-enable (unmask) the FIFO IN interrupt here
        // because DMA completion does not mean bulk-in transfer is done
        UsbRegMaskedClr(REG_DEV_MISG1, MF0_IN_INT << fifo_no);
    }
    else
    {
        // FIFO-to-Memory, is OUT (receive data) transfer

        FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;

        // re-enable OUT interrupts whatever fifo or short packet
        UsbRegMaskedClr(REG_DEV_MISG1, (MF0_OUT_INT | MF0_SPK_INT) << (fifo_no * 2));

        if (fifo_cb[fifo_no].short_or_zl_packet)
        {
            // this transfer is a short packet, so transfer is done
            // notify transfer done to user
            fifo_cb[fifo_no].status = KDRV_STATUS_OK;
            osEventFlagsSet(usbd2_ctrl.evt_id, (0x1 << fifo_no));

            _clean_fifo_ctrl(&fifo_cb[fifo_no]);
        }
    }
}

static void _handle_ZLP_receive_interrupt()
{
    uint32_t zl_endp_interrupts = UsbRegRead(REG_DEV_RXZ);
    uint32_t enpNo;
    for (enpNo = 1; enpNo <= 8; enpNo++)
        if (zl_endp_interrupts & (0x1 << (enpNo - 1)))
            break;

    // clean corresponding endpoint's rx zero-length interrupt
    UsbRegWrite(REG_DEV_RXZ, RX0BYTE_EP1 << (enpNo - 1));

    // clear global rx zero-length interrupt
    UsbRegWrite(REG_DEV_ISG2, G2_RX0BYTE_INT_RW1C);

    int8_t fifo_no = _endpoint_to_fifo(enpNo);

    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;
    if (-1 != fifo_no && fifo_cb[fifo_no].isTransferring)
    {

        // re-enable OUT interrupts whatever fifo or short packet
        UsbRegMaskedClr(REG_DEV_MISG1, (MF0_OUT_INT | MF0_SPK_INT) << (fifo_no * 2));

        // received a zero-length packet, notify transfer do to user
        // notify transfer done to user
        fifo_cb[fifo_no].status = KDRV_STATUS_OK;
        osEventFlagsSet(usbd2_ctrl.evt_id, (0x1 << fifo_no));

        _clean_fifo_ctrl(&fifo_cb[fifo_no]);

        return;
    }
}

static void _handle_cmd_abort()
{
    // according to datasheet, this could happen
    // when a new SETUP comes before last one is complete
    // handle it first or the FIFO will be frozen

    // clear the abort status
    UsbRegMaskedSet(REG_DEV_ISG0, G0_CX_COMABT_INT_RW1C);
}

static int8_t _send_host_device_status()
{
    uint8_t sData[4] = {0}; // self-power : 0, remote wakeup : 0

    bool sts = _dma_fifo_transfer_sync_try((uint32_t *)sData, 2, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static int8_t _send_host_device_descriptor(kdrv_usbd2_setup_packet_t *setup)
{
    kdrv_usbd2_device_descriptor_t *desc = usbd2_ctrl.dev_desc;

    // some error checking
    if (!desc)
    {
        return RESP_STALL;
    }

    uint16_t txLen = MIN(desc->bLength, setup->wLength);
    bool sts = _dma_fifo_transfer_sync_try((uint32_t *)desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static int8_t _send_host_device_qual_descriptor(kdrv_usbd2_setup_packet_t *setup)
{
    kdrv_usbd2_device_qualifier_descriptor_t *desc = usbd2_ctrl.dev_qual_desc;

    // some error checking
    if (!desc)
    {
        return RESP_STALL;
    }

    uint16_t txLen = MIN(desc->bLength, setup->wLength);
    bool sts = _dma_fifo_transfer_sync_try((uint32_t *)desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static int8_t _send_host_string_descriptor(kdrv_usbd2_setup_packet_t *setup, uint8_t type)
{
    kdrv_usbd2_string_descriptor_t *desc = usbd2_ctrl.dev_str_desc;
    kdrv_usbd2_prd_string_descriptor_t **desc_str = &usbd2_ctrl.dev_str_desc->desc[0];

    uint16_t txLen = 0;
    bool sts = false;
    if (type == 0) //language id
    {
        txLen = MIN(desc->bLength, setup->wLength);
        sts = _dma_fifo_transfer_sync_try((uint32_t *)desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    }
    else if (type == 1 || type == 2 || type == 3) //iManufacturer,iProduct,iSerialNumber
    {
        txLen = MIN(desc_str[type - 1]->bLength, setup->wLength);
        sts = _dma_fifo_transfer_sync_try((uint32_t *)desc_str[type - 1], txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    }

    if (sts)
    {
        return RESP_ACK;
    }
    else
    {
        return RESP_STALL;
    }
}

static int8_t _send_host_configuration_descriptors(kdrv_usbd2_setup_packet_t *setup)
{
    kdrv_usbd2_device_descriptor_t *dev_desc = usbd2_ctrl.dev_desc;
    uint32_t confIdx = setup->wValue & 0xFF;

    // some error checking
    if (confIdx > 0)
        return RESP_STALL;

    kdrv_usbd2_config_descriptor_t *conf_desc = dev_desc->config;

    // create an temp buffer for combining all sub-descriptors
    uint8_t *buf_desc = malloc(conf_desc->wTotalLength);
    if (NULL == buf_desc)
        return RESP_STALL;

    uint16_t txLen = MIN(conf_desc->wTotalLength, setup->wLength);

    // collect all sub-descriptos int one memory
    uint8_t offset = 0;
    memcpy(buf_desc, conf_desc, conf_desc->bLength);
    offset += conf_desc->bLength;

    kdrv_usbd2_interface_descriptor_t *intf_desc = conf_desc->interface;
    memcpy(buf_desc + offset, intf_desc, intf_desc->bLength);
    offset += intf_desc->bLength;
    for (int j = 0; j < intf_desc->bNumEndpoints; j++)
    {
        kdrv_usbd2_endpoint_descriptor_t *endp_desc = intf_desc->endpoint[j];
        memcpy(buf_desc + offset, endp_desc, endp_desc->bLength);
        offset += endp_desc->bLength;
    }

    bool sts = _dma_fifo_transfer_sync_try((uint32_t *)buf_desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);

    free(buf_desc);

    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static void _init_fifo_configurations(kdrv_usbd2_interface_descriptor_t *intf)
{
    uint32_t fifo_map_val = 0x0;         // for 0x1A8
    uint32_t endp_map0_val = 0x0;        // for 0x1A0
    uint32_t endp_map1_val = 0x0;        // for 0x1A4
    uint32_t fifo_config_val = 0x0;      // for 0x1AC
    uint32_t fifo_int_mask = 0xFFFFFFFF; // for 0x138, default disable all interrupts

    // also need to init fifo-dma control blocks
    for (int fifo = 0; fifo < FIFO_NUM; fifo++)
        usbd2_ctrl.fifo_cbs[fifo].enabled = false;

    // here assume endpoint number order is ascending
    for (int i = 0; i < intf->bNumEndpoints; i++)
    {
        uint8_t bEndpointAddress = intf->endpoint[i]->bEndpointAddress;
        uint8_t bmAttributes = intf->endpoint[i]->bmAttributes;
        uint16_t wMaxPacketSize = intf->endpoint[i]->wMaxPacketSize;

        // i value implies FIFO number
        uint32_t fifo = i;
        uint8_t isIn = !!(bEndpointAddress & 0x80);
        uint8_t enpNo = bEndpointAddress & 0xF; // retrieve endpoint no without direction
        uint32_t bitfield;

        // set FIFO's direction and corresponding endpoint no.
        bitfield = (isIn << 4) | enpNo;
        fifo_map_val |= (bitfield << (fifo * 8));

        // set endpoint's FIFO no.
        bitfield = isIn ? fifo : (fifo << 4);
        if (enpNo <= 4)
            endp_map0_val |= (bitfield << ((enpNo - 1) * 8));
        else // 5~8
            endp_map1_val |= (bitfield << ((enpNo - 5) * 8));

        // enable the corresponding FIFO and set transfer type
        fifo_config_val |= (BIT5 | (bmAttributes & 0x3)) << (fifo * 8);

        // set max packet size & reset toggle bit
        if (isIn)
        {
            // for IN endpoints
            UsbRegWrite(REG_DEV_INMPS_1 + 4 * (enpNo - 1), wMaxPacketSize & 0x7ff);
            UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (enpNo - 1), RSTG_IEPn);
            UsbRegMaskedClr(REG_DEV_INMPS_1 + 4 * (enpNo - 1), RSTG_IEPn);

            // disable interrupt for IN endpoint
            // because when IN fifo is empty, interrupt will be asserted
            // we could enable IN interrupt only when need to watch it
            fifo_int_mask |= (MF0_IN_INT << fifo);
        }
        else
        {
            // for OUT endpoints
            UsbRegWrite(REG_DEV_OUTMPS_1 + 4 * (enpNo - 1), wMaxPacketSize & 0x7ff);
            UsbRegMaskedSet(REG_DEV_OUTMPS_1 + 4 * (enpNo - 1), RSTG_OEPn);
            UsbRegMaskedClr(REG_DEV_OUTMPS_1 + 4 * (enpNo - 1), RSTG_OEPn);

            // enable interrupt for OUT endpoint
            fifo_int_mask &= ~((MF0_SPK_INT | MF0_OUT_INT) << (fifo * 2));
        }

        // init fifo dma control blocks for each enabled fifo
        usbd2_ctrl.fifo_cbs[fifo].enabled = true;
        usbd2_ctrl.fifo_cbs[fifo].enpNo = enpNo;
        usbd2_ctrl.fifo_cbs[fifo].endpointAddress = bEndpointAddress;
        usbd2_ctrl.fifo_cbs[fifo].maxPacketSize = wMaxPacketSize;
        usbd2_ctrl.fifo_cbs[fifo].transferType = bmAttributes & 0x3;
        usbd2_ctrl.fifo_cbs[fifo].byteCntReg = 0x1B0 + 4 * fifo;
        usbd2_ctrl.fifo_cbs[fifo].isTransferring = false;
        usbd2_ctrl.fifo_cbs[fifo].user_buf_addr = 0;
        usbd2_ctrl.fifo_cbs[fifo].user_buf_len = 0;
        usbd2_ctrl.fifo_cbs[fifo].short_or_zl_packet = false;
    }

    // clear all FIFO
    UsbRegMaskedSet(REG_DEV_TST, TST_CLRFF);

    // set FIFO interrupt mask
    UsbRegWrite(REG_DEV_MISG1, fifo_int_mask);

    // endpoint map 0
    UsbRegWrite(REG_DEV_EPMAP0, endp_map0_val);

    // endpoint map1
    UsbRegWrite(REG_DEV_EPMAP1, endp_map1_val);

    // fifo map
    UsbRegWrite(REG_DEV_FMAP, fifo_map_val);

    // fifo config / enable
    UsbRegWrite(REG_DEV_FCFG, fifo_config_val);

    // set Device SOF Mask Timer value as data sheet recommended for HS
    UsbRegWrite(REG_DEV_SMT, 0x44C);

    // set configuration set bit, now allow HW to handle endpoint transfer
    UsbRegMaskedSet(REG_DEV_ADR, AFT_CONF);
}

static int8_t _set_configuration(kdrv_usbd2_setup_packet_t *setup)
{
    int8_t resp = RESP_STALL;

    uint8_t config_val = setup->wValue & 0xFF;
    if (config_val == 0)
    {
        usbd2_ctrl.config_state = CONFIG_ADDRESS_STATE;
        // clear configuration set bit
        UsbRegMaskedClr(REG_DEV_ADR, AFT_CONF);
        resp = RESP_ACK;

        // reset all endpoints and terminate all in-progress transfer
        _reset_all_endpoints();
    }
    else if (config_val == 1)
    {
        kdrv_usbd2_config_descriptor_t *config = usbd2_ctrl.dev_desc->config;

        if (usbd2_ctrl.config_state != CONFIG_CONFIGURED_STATE)
        {
            usbd2_ctrl.config_state = CONFIG_CONFIGURED_STATE;

            // reset enabled endpoint in case of user waiting
            _reset_all_endpoints();

            // init fifo and endpoint stuff
            _init_fifo_configurations(config->interface);

            usbd2_ctrl.link_status = USBD2_STATUS_CONFIGURED;
            usbd2_ctrl.evt_cb(USBD2_STATUS_CONFIGURED);

            // now that it is configured, we enable link status detection for disconnection status
            _set_link_status_detection(true);

            // unlock transfer API
            for (int i = 0; i < FIFO_NUM; i++)
                osSemaphoreRelease(usbd2_ctrl.fifo_cbs[i].semaphore);
        }

        resp = RESP_ACK;
    }

    return resp;
}

static int8_t handle_standard_request(kdrv_usbd2_setup_packet_t *setup)
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

    // if ep0 is halted, some requests should not be done
    if (usbd2_ctrl.ep0_halted)
        return RESP_STALL;

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
            resp = _send_host_device_qual_descriptor(setup);
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

static void _handle_control_transfer()
{
    // ready to read out 8 bytes setup packet
    kdrv_usbd2_setup_packet_t setup = {0};
    uint8_t *temp = (uint8_t *)&setup;

    // if DMA is busy now, do nothing and will handle in later interrupts
    if (_dma_is_busy())
    {
        return;
    }

    // set DMA FIFO selection to CXF
    UsbRegWrite(REG_DMA_TFN, DMA_TARGET_ACC_CXF);

    // directly read DMA_CPS3 twice to get 8-byte setup packet
    *((uint32_t *)temp) = UsbRegRead(REG_DMA_CPS3);
    *((uint32_t *)(temp + 4)) = UsbRegRead(REG_DMA_CPS3);

    // clear DMA FIFO selection
    UsbRegWrite(REG_DMA_TFN, DMA_TARGET_ACC_NONE);

    // parsing bmRequestType to find out which kind of reqeusts
    int8_t resp = RESP_NACK;
    uint8_t bmRequestType_type = ((setup.bmRequestType & 0x60) >> 5);
    switch (bmRequestType_type)
    {
    case 0: // Standard request
        resp = handle_standard_request(&setup);
        break;
    case 1: // Class request
    case 2: // Vendor request
        if (usbd2_ctrl.usr_cx_cb(&setup) == true)
            resp = RESP_ACK;
        else
            resp = RESP_STALL;
        break;
    }

    if (resp == RESP_ACK)
        // indicate an OK request to host
        UsbRegMaskedSet(REG_CXCFE, CX_DONE);
    else if (resp == RESP_STALL)
    {
        // indicate a request error to host
        UsbRegMaskedSet(REG_CXCFE, CX_STL | CX_DONE);
    }
    else
    {
        // NACK, do nothing for now
    }
}

static void _handle_device_interrupts()
{
    uint32_t grp_x_int_status = UsbRegRead(REG_DEV_IGR);
    uint32_t grp_0_interrupts = (UsbRegRead(REG_DEV_ISG0) & ~(UsbRegRead(REG_DEV_MISG0)));
    uint32_t grp_2_interrupts = (UsbRegRead(REG_DEV_ISG2) & ~(UsbRegRead(REG_DEV_MISG2)));

    if (grp_x_int_status & GX_INT_G1_RO)
        _handle_fifo_interrupts();

    if (grp_x_int_status & GX_INT_G2_RO)
    {
        if (grp_2_interrupts & G2_DMA_CMPLT_RW1C)
            _handle_dma_complete_interrupt();
        else if (grp_2_interrupts & G2_RX0BYTE_INT_RW1C)
            _handle_ZLP_receive_interrupt();
        else if (grp_2_interrupts & G2_DMA_ERROR_RW1C)
            _handle_dma_error();
        else if (grp_2_interrupts & G2_USBRST_INT_RW1C)
            _bus_reset_work();
    }

    if (grp_x_int_status & GX_INT_G0_RO)
    {
        if (grp_0_interrupts & G0_CX_COMABT_INT_RW1C)
            // first priority
            _handle_cmd_abort();
        else if (grp_0_interrupts & G0_CX_SETUP_INT_RO)
            _handle_control_transfer();
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
        usbd2_ctrl.link_status = USBD2_STATUS_DISCONNECTED;
        usbd2_ctrl.evt_cb(USBD2_STATUS_DISCONNECTED);

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

    // listen all 4 groups for 0x140
    UsbRegWrite(REG_DEV_MIGR, 0x0);

    // grop 0 interrupt mask
    // FIXME: should care about mroe interrupts
    UsbRegWrite(REG_DEV_MISG0,
                ~(G0_CX_SETUP_INT_RO |
                  G0_CX_COMFAIL_INT_RO |
                  G0_CX_COMABT_INT_RW1C));

    // listen no group 1 interrrupts for 0x148
    UsbRegWrite(REG_DEV_MISG1, 0xFFFFFFFF);

    // enable interested interrupts in group 2 0x14C
    UsbRegWrite(REG_DEV_MISG2,
                ~(G2_RX0BYTE_INT_RW1C |
                  G2_DMA_ERROR_RW1C |
                  G2_USBRST_INT_RW1C));

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

    NVIC_SetVector(OTG_SBS_3_IRQ, (uint32_t)_usbd2_isr);

    // Clear and Enable SAI IRQ
    NVIC_ClearPendingIRQ(OTG_SBS_3_IRQ);
    NVIC_EnableIRQ(OTG_SBS_3_IRQ);
}

static void _default_status_isr_callback(kdrv_usbd2_link_status_t event)
{
}

static bool _default_usr_cx_isr_callback(kdrv_usbd2_setup_packet_t *setup)
{
    return false; // RESP_STALL
}

////////////////// below are API ////////////////////

kdrv_status_t kdrv_usbd2_initialize(
    kdrv_usbd2_device_descriptor_t *dev_desc,
    kdrv_usbd2_device_qualifier_descriptor_t *dev_qual_desc,
    kdrv_usbd2_string_descriptor_t *dev_str_desc,
    kdrv_usbd2_link_status_callback_t status_isr_cb,
    kdrv_usbd2_user_control_callback_t usr_cx_isr_cb)
{
    usbd2_ctrl.ep0_halted = false;
    usbd2_ctrl.dev_desc = dev_desc;
    usbd2_ctrl.dev_qual_desc = dev_qual_desc;
    usbd2_ctrl.dev_str_desc = dev_str_desc;
    usbd2_ctrl.evt_cb = (status_isr_cb == NULL) ? _default_status_isr_callback : status_isr_cb;
    usbd2_ctrl.usr_cx_cb = (usr_cx_isr_cb == NULL) ? _default_usr_cx_isr_callback : usr_cx_isr_cb;

    // early init fifo cb for semaphores locking
    kdrv_usbd2_interface_descriptor_t *intf = dev_desc->config->interface;
    for (int i = 0; i < intf->bNumEndpoints; i++)
    {
        usbd2_ctrl.fifo_cbs[i].endpointAddress = intf->endpoint[i]->bEndpointAddress;
        usbd2_ctrl.fifo_cbs[i].semaphore = osSemaphoreNew(1, 0, NULL); // at first it is locked
    }

    mutex_usbd = osMutexNew(NULL);

    usbd2_ctrl.evt_id = osEventFlagsNew(NULL);

    _usbd2_init_register_isr();

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2_uninitialize(void)
{
    osEventFlagsDelete(usbd2_ctrl.evt_id);
    osMutexDelete(mutex_usbd);

    for (int i = 0; i < FIFO_NUM; i++)
    {
        osSemaphoreDelete(usbd2_ctrl.fifo_cbs[i].semaphore);
        usbd2_ctrl.fifo_cbs[i].semaphore = NULL;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2_set_enable(bool enable)
{
    if (enable)
        // Make PHY work properly, FIXME ?
        UsbRegMaskedClr(REG_PHY_TST, TST_JSTA);
    else
        // Make PHY not work, FIXME ?
        UsbRegMaskedSet(REG_PHY_TST, TST_JSTA);

    return KDRV_STATUS_OK;
}

kdrv_usbd2_link_status_t kdrv_usbd2_get_link_status(void)
{
    return usbd2_ctrl.link_status;
}

kdrv_status_t kdrv_usbd2_reset_device()
{
    // disable bus
    kdrv_usbd2_set_enable(false);

    // reset all endpoints and terminate all in-progress transfer
    _reset_all_endpoints();

    // some delay for USB bus, FIXME ?
    osDelay(100);

    usbd2_ctrl.ep0_halted = false;
    usbd2_ctrl.config_state = CONFIG_DEFAULT_STATE;

    // re-init registers and isr
    _usbd2_init_register_isr();

    // re-enable bus
    kdrv_usbd2_set_enable(true);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2_terminate_endpoint(uint32_t endpoint)
{
    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;
    int8_t fifo_no = _endpoint_to_fifo(endpoint);
    if (fifo_no == -1)
        return KDRV_STATUS_USBD_INVALID_ENDPOINT;

    if (!fifo_cb[fifo_no].isTransferring)
        return KDRV_STATUS_ERROR;

    UsbRegMaskedSet(fifo_cb[fifo_no].byteCntReg, FFRST);

    if (fifo_cb[fifo_no].transferType == TXFER_BULK)
    {
        // notify transfer done to user
        fifo_cb[fifo_no].status = KDRV_STATUS_USBD_TRANSFER_TERMINATED;
        osEventFlagsSet(usbd2_ctrl.evt_id, (0x1 << fifo_no));

        if (!_isInterrupt())
        {
            osMutexAcquire(mutex_usbd, osWaitForever);
            NVIC_DisableIRQ(OTG_SBS_3_IRQ);
        }

        if (fifo_cb[fifo_no].endpointAddress & 0x80)
        {
            // for IN endpoints
            UsbRegMaskedSet(REG_DEV_MISG1, MF0_IN_INT << fifo_no);
        }
        else
        {
            // for OUT endpoints
            UsbRegMaskedClr(REG_DEV_MISG1, (MF0_SPK_INT | MF0_OUT_INT) << (fifo_no * 2));
        }

        if (!_isInterrupt())
        {
            NVIC_EnableIRQ(OTG_SBS_3_IRQ);
            osMutexRelease(mutex_usbd);
        }

        _clean_fifo_ctrl(&fifo_cb[fifo_no]);
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t _reset_endpoint(uint32_t endpoint)
{
    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;
    int8_t fifo_no = _endpoint_to_fifo(endpoint);
    if (fifo_no == -1)
        return KDRV_STATUS_USBD_INVALID_ENDPOINT;

    UsbRegMaskedSet(fifo_cb[fifo_no].byteCntReg, FFRST);

    if (fifo_cb[fifo_no].transferType == TXFER_BULK)
    {
        if (fifo_cb[fifo_no].isTransferring)
        {
            // notify transfer done to user
            fifo_cb[fifo_no].status = KDRV_STATUS_USBD_TRANSFER_DISCONNECTED;
            osEventFlagsSet(usbd2_ctrl.evt_id, (0x1 << fifo_no));
        }

        uint8_t enpNo = fifo_cb[fifo_no].enpNo;

        if (!_isInterrupt())
        {
            osMutexAcquire(mutex_usbd, osWaitForever);
            NVIC_DisableIRQ(OTG_SBS_3_IRQ);
        }

        if (fifo_cb[fifo_no].endpointAddress & 0x80)
        {
            // for IN endpoints
            UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (enpNo - 1), RSTG_IEPn);
            UsbRegMaskedClr(REG_DEV_INMPS_1 + 4 * (enpNo - 1), RSTG_IEPn);
            UsbRegMaskedSet(REG_DEV_MISG1, MF0_IN_INT << fifo_no);
        }
        else
        {
            // for OUT endpoints
            UsbRegMaskedSet(REG_DEV_OUTMPS_1 + 4 * (enpNo - 1), RSTG_OEPn);
            UsbRegMaskedClr(REG_DEV_OUTMPS_1 + 4 * (enpNo - 1), RSTG_OEPn);
            UsbRegMaskedClr(REG_DEV_MISG1, (MF0_SPK_INT | MF0_OUT_INT) << (fifo_no * 2));
        }

        if (!_isInterrupt())
        {
            NVIC_EnableIRQ(OTG_SBS_3_IRQ);
            osMutexRelease(mutex_usbd);
        }

        _clean_fifo_ctrl(&fifo_cb[fifo_no]);
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd2_bulk_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    kdrv_status_t status;
    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;
    int8_t fifo_no = _endpoint_to_fifo(endpoint);
    if (fifo_no == -1)
        return KDRV_STATUS_USBD_INVALID_ENDPOINT;

    osSemaphoreAcquire(fifo_cb[fifo_no].semaphore, osWaitForever);

    osEventFlagsClear(usbd2_ctrl.evt_id, (0x1 << fifo_no));

    // set up fifo cb
    fifo_cb[fifo_no].isTransferring = true;
    fifo_cb[fifo_no].user_buf_addr = _dma_remap_addr((uint32_t)buf);
    fifo_cb[fifo_no].user_buf_len = txLen;
    // check if need to send a zero-length packet at the end of transfer
    fifo_cb[fifo_no].short_or_zl_packet = ((txLen & (fifo_cb[fifo_no].maxPacketSize - 1)) == 0) ? true : false;

    // there is a risk of race condition with IRQ when different endponts are working
    osMutexAcquire(mutex_usbd, osWaitForever);
    NVIC_DisableIRQ(OTG_SBS_3_IRQ);
    {
        // reset FIFO content before transmission
        //UsbRegMaskedSet(fifo_cb[fifo_no].byteCntReg, FFRST);

        // enable (unmask) the FIFO IN interrupt
        UsbRegMaskedClr(REG_DEV_MISG1, MF0_IN_INT << fifo_no);
    }
    NVIC_EnableIRQ(OTG_SBS_3_IRQ);
    osMutexRelease(mutex_usbd);

    if (timeout_ms == 0)
        timeout_ms = osWaitForever;

    uint32_t flags = osEventFlagsWait(usbd2_ctrl.evt_id, (0x1 << fifo_no), osFlagsWaitAny, timeout_ms);
    if (flags == osFlagsErrorTimeout)
        status = KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
    else
        status = fifo_cb[fifo_no].status;

    _clean_fifo_ctrl(&fifo_cb[fifo_no]);
    fifo_cb[fifo_no].isTransferring = false;

    // not release semapohore when disconnected
    if (status != KDRV_STATUS_USBD_TRANSFER_DISCONNECTED)
        osSemaphoreRelease(fifo_cb[fifo_no].semaphore);

    return status;
}

kdrv_status_t kdrv_usbd2_bulk_receive(uint32_t endpoint, uint32_t *buf, uint32_t *blen, uint32_t timeout_ms)
{
    kdrv_status_t status;
    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;
    int8_t fifo_no = _endpoint_to_fifo(endpoint);
    if (fifo_no == -1)
        return KDRV_STATUS_USBD_INVALID_ENDPOINT;

    osSemaphoreAcquire(fifo_cb[fifo_no].semaphore, osWaitForever);

    osEventFlagsClear(usbd2_ctrl.evt_id, (0x1 << fifo_no));

    // set up fifo cb
    fifo_cb[fifo_no].isTransferring = true;
    fifo_cb[fifo_no].user_buf_len = *blen;
    fifo_cb[fifo_no].received_length = 0;
    fifo_cb[fifo_no].user_buf_addr = _dma_remap_addr((uint32_t)buf);

    // there is a risk of race condition with IRQ when different endponts are working
    osMutexAcquire(mutex_usbd, osWaitForever);
    NVIC_DisableIRQ(OTG_SBS_3_IRQ);

    // the OUT interrupts should have been disabled earlier, re-enable it since buffer is commited
    UsbRegMaskedClr(REG_DEV_MISG1, MF0_OUT_INT << (fifo_no * 2));

    NVIC_EnableIRQ(OTG_SBS_3_IRQ);
    osMutexRelease(mutex_usbd);

    if (timeout_ms == 0)
        timeout_ms = osWaitForever;

    uint32_t flags = osEventFlagsWait(usbd2_ctrl.evt_id, (0x1 << fifo_no), osFlagsWaitAny, timeout_ms);
    if (flags == osFlagsErrorTimeout)
        status = KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
    else
    {
        *blen = fifo_cb[fifo_no].received_length;
        status = fifo_cb[fifo_no].status;
    }

    fifo_cb[fifo_no].isTransferring = false;

    // not release semapohore when disconnected
    if (status != KDRV_STATUS_USBD_TRANSFER_DISCONNECTED)
        osSemaphoreRelease(fifo_cb[fifo_no].semaphore);

    return status;
}

bool kdrv_usbd2_interrupt_send_check_buffer_empty(uint32_t endpoint)
{
    int8_t fifo_no = _endpoint_to_fifo(endpoint);
    if (fifo_no == -1)
        return false; //FIXME, is returing false correct?
    return !!(UsbRegRead(REG_CXCFE) & (F_EMP_0 << fifo_no));
}

kdrv_status_t kdrv_usbd2_interrupt_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    FIFO_Ctrl_t *fifo_cb = usbd2_ctrl.fifo_cbs;

    // find out which FIFO no for the IN endpoint address
    int8_t fifo_no = _endpoint_to_fifo(endpoint);

    // below do some error checking
    {
        if (fifo_no == -1)
            return KDRV_STATUS_USBD_INVALID_ENDPOINT;

        if (fifo_cb[fifo_no].transferType != TXFER_INT)
            return KDRV_STATUS_USBD_INVALID_TRANSFER;
    }

    // clear FIFO content before transmission
    UsbRegMaskedSet(fifo_cb[fifo_no].byteCntReg, FFRST);

    uint32_t tryMs = 0;
    while (1)
    {
        bool isDone = false;

        for (int try = 0; try < 100; try ++)
        {
            if (!_dma_is_busy())
            {
                osMutexAcquire(mutex_usbd, osWaitForever);
                NVIC_DisableIRQ(OTG_SBS_3_IRQ);

                isDone = _dma_fifo_transfer_sync(buf, txLen, (0x1 << fifo_no), WRITE_FIFO);

                NVIC_EnableIRQ(OTG_SBS_3_IRQ);
                osMutexRelease(mutex_usbd);

                if (isDone)
                    return KDRV_STATUS_OK;
            }
        }

        if (timeout_ms != 0 && tryMs >= timeout_ms)
            return KDRV_STATUS_USBD_TRANSFER_TIMEOUT;

        osDelay(1);
        ++tryMs;
    }
}
