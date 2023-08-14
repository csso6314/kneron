/*
 * Kneron Peripheral API
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "cmsis_os2.h"
#include "base.h"
#include "kdrv_scu_ext.h"
#include "kdrv_uart.h"
//#include "kdp_io.h"
#include "kdrv_usbd.h"
#include "kmdw_console.h"

// OTG Control Status Register (0x80)
#define REG_OTG_CSR 0x80
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

// Device FIFO Byte Count Register (0x1B0 + 4(fno-1))
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

// for FIFO_Ctrl:transferType
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

    uint8_t isBlockingCall;
    kdrv_usbd_event_name_t cur_event; // internal use for blocking API
} FIFO_Ctrl;

// define usb device mode control block
typedef struct
{
    osThreadId_t notifyTid;
    osEventFlagsId_t evt_id; // internal use for blocking API
    uint32_t notifyFlag;
    bool ep0_halted;
    kdrv_usbd_device_descriptor_t *dev_desc;
    kdrv_usbd_device_qualifier_descriptor_t *dev_qual_desc; // is it necessary ?
    kdrv_usbd_string_descriptor_t *dev_string_desc;
    uint32_t config_state;
    FIFO_Ctrl fifo_cbs[FIFO_NUM]; // FIFO control blocks
} Ctrl_Block;

enum
{
    READ_FIFO = 0,
    WRITE_FIFO = 1,
};

// an usb device mode object for internal use
Ctrl_Block cb =
    {
        .notifyTid = 0,
        .notifyFlag = 0,
        .ep0_halted = false,
        .dev_desc = NULL,
        .dev_string_desc = NULL,
        .config_state = CONFIG_DEFAULT_STATE,
};

#define QLEN 30
static kdrv_usbd_event_t event_queue[QLEN]; // a fifo ring queue
static int eqWriteIdx = 0;
static int eqReadIdx = 0;
static osSemaphoreId_t empty_id = 0;
static osSemaphoreId_t filled_id = 0;

static bool dma_is_busy()
{
    return (UsbRegRead(REG_DMA_TFN) != DMA_TARGET_ACC_NONE);
}

//  for data SRAM, the address must be remapped
static uint32_t dma_remap_addr(uint32_t addr)
{
    uint32_t tmp;

    if ((addr & (SdRAM_MEM_BASE)) == SdRAM_MEM_BASE)
    {
        tmp = ((addr) & (~0x10000000)) | 0x20000000;
        return tmp;
    }
    return addr;
}

// fifo to memory or memory to fifo transfer
// fifo_dir = 1 : memory -> fifo
// fifo_dir = 0 : fifo -> memory
// dma polling way
static bool dma_fifo_transfer_sync(uint32_t *addr, uint32_t len, uint32_t fifo_sel, uint8_t fifo_dir)
{
    if (dma_is_busy())
        return false;

    bool status = false;

    // set DMA FIFO selection to accuire DMA
    // FIXME: better use mutex stuff to occupy DMA resource !!
    UsbRegWrite(REG_DMA_TFN, fifo_sel);

    // temporarily disable DMA complt interrupt
    // because we will poll it here
    UsbRegMaskedSet(REG_DEV_MISG2, G2_DMA_CMPLT_RW1C);

    // set DMA address
    UsbRegWrite(REG_DMA_CPS2, dma_remap_addr((uint32_t)addr));

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
            UsbRegMaskedSet(REG_DEV_ISG2, G2_DMA_CMPLT_RW1C);
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

static bool dma_fifo_transfer_sync_try(uint32_t *addr, uint32_t len, uint32_t fifo_sel, uint8_t fifo_dir, uint32_t try_count)
{
    bool status = false;
    for (int i = 0; i < try_count; i++)
        if (dma_fifo_transfer_sync(addr, len, fifo_sel, fifo_dir))
        {
            status = true;
            break;
        }
    return status;
}

// out transfer
// configure DMA settings for fifo-to-memory for non-control transfer
// this implementation follows FOTG210 data sheet : 6.2.4 "Programming DMA"
static bool dma_start_fifo_to_mem(uint32_t fno, FIFO_Ctrl *fifocb)
{
    if (dma_is_busy())
        return false;

    // select FIFO for DMA
    UsbRegWrite(REG_DMA_TFN, 0x1 << fno);

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
static bool dma_start_mem_to_fifo(uint32_t fno, FIFO_Ctrl *fifocb)
{
    if (dma_is_busy())
        return false;

    // select FIFO for DMA
    UsbRegWrite(REG_DMA_TFN, 0x1 << fno);

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

/* this function must not be used in ISR */
static void reset_event_queue()
{
    osStatus_t sts = osOK;

    if (empty_id != 0)
        sts = osSemaphoreRelease(empty_id);

    if (sts == osOK)
        empty_id = osSemaphoreNew(QLEN, QLEN, NULL);

    if (filled_id != 0)
        sts = osSemaphoreRelease(filled_id);

    if (sts == osOK)
        filled_id = osSemaphoreNew(QLEN, 0, NULL);

    // reset read/write indices
    eqWriteIdx = 0;
    eqReadIdx = 0;
}

static void clean_event_queue()
{
    if (empty_id != 0)
        osSemaphoreRelease(empty_id);
    empty_id = 0;

    if (filled_id != 0)
        osSemaphoreRelease(filled_id);
    filled_id = 0;
}

static int push_event_to_queue(kdrv_usbd_event_t event)
{
    osStatus_t sts;

    sts = osSemaphoreAcquire(empty_id, 0);
    if (sts != osOK)
        // not overwrite event queue
        return 0;

    event_queue[eqWriteIdx] = event;
    ++eqWriteIdx;
    if (eqWriteIdx >= QLEN)
        eqWriteIdx = 0;

    osSemaphoreRelease(filled_id);
    return 1;
}

static int pop_event_from_queue(kdrv_usbd_event_t *event)
{
    osStatus_t sts;

    sts = osSemaphoreAcquire(filled_id, 0);
    // queue is empty
    if (sts != osOK)
        return 0;

    *event = event_queue[eqReadIdx];
    ++eqReadIdx;
    if (eqReadIdx >= QLEN)
        eqReadIdx = 0;

    osSemaphoreRelease(empty_id);
    return 1;
}

// put usb events to an event queue and notify user via thread flag
static void notify_event_to_user(kdrv_usbd_event_name_t ename, uint32_t data1, uint32_t data2)
{
    kdrv_usbd_event_t uevent;
    uevent.ename = ename;
    uevent.data1 = data1;
    uevent.data2 = data2;
    push_event_to_queue(uevent);

    if (cb.notifyTid)
        osThreadFlagsSet(cb.notifyTid, cb.notifyFlag);
}

static void bus_reset_work()
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
    UsbRegMaskedSet(REG_DEV_ISG2, G2_USBRST_INT_RW1C);

    // reset all endpoints, FIXME: better place to do this ?
    {
        FIFO_Ctrl *fifo_cb = cb.fifo_cbs;
        for (int fno = 0; fno < FIFO_NUM; fno++)
            if (fifo_cb[fno].enabled)
            {
                kdrv_usbd_reset_endpoint(fifo_cb[fno].endpointAddress);
            }
    }

    notify_event_to_user(KDRV_USBD_EVENT_BUS_RESET, 0, 0);
}

static void handle_dma_error()
{
    // clear this interrupt bit
    UsbRegMaskedSet(REG_DEV_ISG2, G2_DMA_ERROR_RW1C);

    // check which fifo-endpoint is responsible for this

    uint32_t fifoSel = UsbRegRead(REG_DMA_TFN);

    // clear FIFO sel
    UsbRegWrite(REG_DMA_TFN, DMA_TARGET_ACC_NONE);

    uint32_t fno;
    for (fno = 0; fno < FIFO_NUM; fno++)
        if (fifoSel & (0x1 << fno))
            break;

    FIFO_Ctrl *fifo_cb = &(cb.fifo_cbs[fno]);

    // reset the endpoint due to DMA error
    kdrv_usbd_reset_endpoint(fifo_cb->endpointAddress);

    // also notify user the DMA_ERROR
    notify_event_to_user(KDRV_USBD_EVENT_DMA_ERROR, 0, 0);
}

static void bus_suspend_work()
{
    // clear this interrupt bit
    UsbRegMaskedSet(REG_DEV_ISG2, G2_SUSP_INT_RW1C);

    // FIXME: should do something here ?

    notify_event_to_user(KDRV_USBD_EVENT_BUS_SUSPEND, 0, 0);
}

static void bus_resume_work()
{
    // clear this interrupt bit
    UsbRegMaskedSet(REG_DEV_ISG2, G2_RESM_INT_RW1C);

    // FIXME: should do something here ?

    notify_event_to_user(KDRV_USBD_EVENT_BUS_SUSPEND, 0, 0);
}

static int8_t endpoint_to_fifo(uint32_t endpoint)
{
    // check fifo ctrl blocks to get fifo no
    int8_t fno;
    for (fno = 0; fno < FIFO_NUM; fno++)
        if (cb.fifo_cbs[fno].endpointAddress == endpoint)
            break;

    return (fno < 4) ? fno : -1;
}

static void clean_fifo_cb(FIFO_Ctrl *fifo_cb)
{
    fifo_cb->user_buf_addr = 0;
    fifo_cb->user_buf_len = 0;
    fifo_cb->isTransferring = false;
    fifo_cb->short_or_zl_packet = false;
}

static inline void handle_fifo_short_pkt_interrupts(uint32_t interrupt_bits)
{
    for (int fno = 0; fno < FIFO_NUM; fno++)
    {
        // make sure both OUT and SPK interrupts bits get asserted at the same time !
        if (((interrupt_bits >> (fno * 2)) & (G1_F0_SPK_INT_RO | G1_F0_OUT_INT_RO)) == 0x3)
        {

            // with a short packet coming means data is less than MaxPacketSize
            // and it is the last packet transferd by host
            // this will be handled in DMA completeion time
            cb.fifo_cbs[fno].short_or_zl_packet = true;

            // disable short packet interrupt
            // shoudl be re-enabled at next DMA completeion time
            UsbRegMaskedSet(REG_DEV_MISG1, MF0_SPK_INT << (fno * 2));

            break;
        }
    }
}

static inline void handle_fifo_out_interrupts(uint32_t interrupt_bits)
{
    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;

    for (int fno = 0; fno < FIFO_NUM; fno++)
    {
        // handle interrupts for each fifo
        if (interrupt_bits & (G1_F0_OUT_INT_RO << (fno * 2)))
        {
            // if no user buffer is commited, means that this is a start of OUT transfer
            if (fifo_cb[fno].user_buf_addr == 0)
            {
                // disable this OUT interrrupt
                // it should be re-enabled once user commits a buffer for DMA
                UsbRegMaskedSet(REG_DEV_MISG1, MF0_OUT_INT << (fno * 2));

                // then should notify an event to user
                notify_event_to_user(KDRV_USBD_EVENT_TRANSFER_OUT, fifo_cb[fno].enpNo, 0);
            }
            else // configure DMA transfer for FIFO to user buffer
            {
                if (fifo_cb[fno].user_buf_len > 0 &&
                    dma_start_fifo_to_mem(fno, &fifo_cb[fno]) == true)
                {
                    // DMA has been started, disable this OUT interrupt
                    // and re-enable the interrupt at DMA completion
                    UsbRegMaskedSet(REG_DEV_MISG1, MF0_OUT_INT << (fno * 2));
                }
            }
        }
    }
}

static inline void handle_fifo_in_interrupts(uint32_t interrupt_bits)
{
    // this handles only one fifo interrupt

    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;

    uint32_t fno;
    for (fno = 0; fno < FIFO_NUM; fno++)
        if (interrupt_bits & (G1_F0_IN_INT_RO << fno))
            break;

    // disable (mask) the FIFO IN interrupt immediately to prevent from interrupt re-trigger
    UsbRegMaskedSet(REG_DEV_MISG1, MF0_IN_INT << fno);

    if (fifo_cb[fno].user_buf_len > 0)
    {
        if (dma_start_mem_to_fifo(fno, &fifo_cb[fno]) == false)
        {
            // re-enable the interrupt for next try
            UsbRegMaskedClr(REG_DEV_MISG1, MF0_IN_INT << fno);
        }
    }
    else
    {
        // FIFO is empty and no more data to send, in other words, the bulk-in transfer is done

        // send zero-length packet if needed
        if (fifo_cb[fno].short_or_zl_packet)
        {
            UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (fifo_cb[fno].enpNo - 1), TX0BYTE_IEPn);
            // FIXME: should check G2_TX0BYTE_INT_RW1C in later interrupts
        }

        // notify transfer done to user
        if (fifo_cb[fno].isBlockingCall)
        {
            fifo_cb[fno].cur_event = KDRV_USBD_EVENT_TRANSFER_DONE;
            osEventFlagsSet(cb.evt_id, 0x1 << fno);
        }
        else
        {
            clean_fifo_cb(&fifo_cb[fno]);
            notify_event_to_user(KDRV_USBD_EVENT_TRANSFER_DONE, fifo_cb[fno].endpointAddress, 0);
        }
    }
}

#define FIFO_SHORT_PKT_INTERRUPTS (G1_F0_SPK_INT_RO | G1_F1_SPK_INT_RO | G1_F2_SPK_INT_RO | G1_F3_SPK_INT_RO)
#define FIFO_OUT_INTERRUPTS (G1_F0_OUT_INT_RO | G1_F1_OUT_INT_RO | G1_F2_OUT_INT_RO | G1_F3_OUT_INT_RO)
#define FIFO_IN_INTERRUPTS (G1_F0_IN_INT_RO | G1_F1_IN_INT_RO | G1_F2_IN_INT_RO | G1_F3_IN_INT_RO)

static void handle_fifo_interrupts()
{
    uint32_t fifo_interrupts = UsbRegRead(REG_DEV_ISG1) & ~(UsbRegRead(REG_DEV_MISG1));

    // handle OUT short packets interrupts for short packets
    if (fifo_interrupts & FIFO_SHORT_PKT_INTERRUPTS)
        handle_fifo_short_pkt_interrupts(fifo_interrupts);

    // handle OUT FIFO interrupts
    if (fifo_interrupts & FIFO_OUT_INTERRUPTS)
        handle_fifo_out_interrupts(fifo_interrupts);

    // handle IN FIFO interrupts
    if (fifo_interrupts & FIFO_IN_INTERRUPTS)
        handle_fifo_in_interrupts(fifo_interrupts);
}

// FIXME: this does not handle CX DMA complete
static void handle_dma_complete_interrupt()
{
    // this handles only one DMA complete interrupt
    uint32_t fno;
    uint32_t fifoSel = UsbRegRead(REG_DMA_TFN);
    uint32_t dmaCPS1 = UsbRegRead(REG_DMA_CPS1);

    // clear FIFO selection
    UsbRegWrite(REG_DMA_TFN, DMA_TARGET_ACC_NONE);

    // clear DMA completion interrupt
    UsbRegMaskedSet(REG_DEV_ISG2, G2_DMA_CMPLT_RW1C);

    for (fno = 0; fno < FIFO_NUM; fno++)
        if (fifoSel & (0x1 << fno))
            break;

    // check DMA-FIFO direction
    if (dmaCPS1 & DMA_TYPE)
    {

        // Memory-to-FIFO, is IN transfer
        // re-enable (unmask) the FIFO IN interrupt here
        // because DMA completion does not mean bulk-in transfer is done
        UsbRegMaskedClr(REG_DEV_MISG1, MF0_IN_INT << fno);
    }
    else
    {
        // FIFO-to-Memory, is OUT transfer

        FIFO_Ctrl *fifo_cb = &(cb.fifo_cbs[fno]);

        {
            // re-enable OUT interrupts whatever fifo or short packet
            UsbRegMaskedClr(REG_DEV_MISG1, (MF0_OUT_INT | MF0_SPK_INT) << (fno * 2));

            if (fifo_cb->short_or_zl_packet)
            {
                // this transfer is a short packet, so transfer is done
                // notify transfer done to user
                if (fifo_cb->isBlockingCall)
                {
                    fifo_cb->cur_event = KDRV_USBD_EVENT_TRANSFER_DONE;
                    osEventFlagsSet(cb.evt_id, 0x1 << fno);
                }
                else
                {
                    clean_fifo_cb(fifo_cb);
                    notify_event_to_user(KDRV_USBD_EVENT_TRANSFER_DONE, fifo_cb->endpointAddress, fifo_cb->received_length);
                }
            }
        }
    }
}

static void handle_zero_length_packet_interrupt()
{
    uint32_t zl_endp_interrupts = UsbRegRead(REG_DEV_RXZ);
    uint32_t enpNo;
    for (enpNo = 1; enpNo <= 8; enpNo++)
        if (zl_endp_interrupts & (0x1 << (enpNo - 1)))
            break;

    // clean corresponding endpoint's rx zero-length interrupt
    UsbRegMaskedSet(REG_DEV_RXZ, RX0BYTE_EP1 << (enpNo - 1));

    // clear global rx zero-length interrupt
    UsbRegMaskedSet(REG_DEV_ISG2, G2_RX0BYTE_INT_RW1C);

    int8_t fno = endpoint_to_fifo(enpNo);

    FIFO_Ctrl *fifo_cb = &(cb.fifo_cbs[fno]);
    if (fifo_cb->isTransferring)
    {

        // re-enable OUT interrupts whatever fifo or short packet
        UsbRegMaskedClr(REG_DEV_MISG1, (MF0_OUT_INT | MF0_SPK_INT) << (fno * 2));

        // received a zero-length packet, notify transfer do to user
        // notify transfer done to user
        if (fifo_cb->isBlockingCall)
        {
            fifo_cb->cur_event = KDRV_USBD_EVENT_TRANSFER_DONE;
            osEventFlagsSet(cb.evt_id, 0x1 << fno);
        }
        else
        {
            clean_fifo_cb(fifo_cb);
            notify_event_to_user(KDRV_USBD_EVENT_TRANSFER_DONE, fifo_cb->endpointAddress, fifo_cb->received_length);
        }

        return;
    }
}

static void handle_cmd_abort()
{
    // according to datasheet, this could happen
    // when a new SETUP comes before last one is complete
    // handle it first or the FIFO will be frozen

    // clear the abort status
    UsbRegMaskedSet(REG_DEV_ISG0, G0_CX_COMABT_INT_RW1C);
}

static int8_t _send_host_device_status()
{
    uint8_t sData[2] = {0}; // self-power : 0, remote wakeup : 0

    bool sts = dma_fifo_transfer_sync_try((uint32_t *)sData, 2, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static int8_t send_host_device_descriptor(kdrv_usbd_setup_packet_t *setup)
{
    kdrv_usbd_device_descriptor_t *desc = cb.dev_desc;

    // some error checking
    if (!desc)
    {
        return RESP_STALL;
    }

    uint16_t txLen = MIN(desc->bLength, setup->wLength);
    bool sts = dma_fifo_transfer_sync_try((uint32_t *)desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static int8_t send_host_device_qual_descriptor(kdrv_usbd_setup_packet_t *setup)
{
    kdrv_usbd_device_qualifier_descriptor_t *desc = cb.dev_qual_desc;

    // some error checking
    if (!desc)
    {
        return RESP_STALL;
    }

    uint16_t txLen = MIN(desc->bLength, setup->wLength);
    bool sts = dma_fifo_transfer_sync_try((uint32_t *)desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static int8_t send_host_string_descriptor(kdrv_usbd_setup_packet_t *setup, uint8_t type)
{
    kdrv_usbd_string_descriptor_t *desc = cb.dev_string_desc;
    kdrv_usbd_prd_string_descriptor_t **desc_str = &cb.dev_string_desc->desc[0];

    uint16_t txLen = 0;
    bool sts = false;
    if (type == 0) //language id
    {
        txLen = MIN(desc->bLength, setup->wLength);
        sts = dma_fifo_transfer_sync_try((uint32_t *)desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
    }
    else if (type == 1 || type == 2 || type == 3) //iManufacturer,iProduct,iSerialNumber
    {
        txLen = MIN(desc_str[type - 1]->bLength, setup->wLength);
        sts = dma_fifo_transfer_sync_try((uint32_t *)desc_str[type - 1], txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);
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

static int8_t send_host_configuration_descriptors(kdrv_usbd_setup_packet_t *setup)
{
    kdrv_usbd_device_descriptor_t *dev_desc = cb.dev_desc;
    uint32_t confIdx = setup->wValue & 0xFF;

    // some error checking
    if (!dev_desc ||
        dev_desc->bNumConfigurations > MAX_USBD_CONFIG ||
        confIdx >= MAX_USBD_CONFIG)
    {
        return RESP_STALL;
    }

    kdrv_usbd_config_descriptor_t *conf_desc = dev_desc->config[confIdx];

    // create an temp buffer for combining all sub-descriptors
    uint8_t *buf_desc = malloc(conf_desc->wTotalLength);

    uint16_t txLen = MIN(conf_desc->wTotalLength, setup->wLength);

    // collect all sub-descriptos int one memory
    uint8_t offset = 0;
    memcpy(buf_desc, conf_desc, conf_desc->bLength);
    offset += conf_desc->bLength;

    for (int i = 0; i < conf_desc->bNumInterfaces; i++)
    {
        kdrv_usbd_interface_descriptor_t *intf_desc = conf_desc->interface[i];
        memcpy(buf_desc + offset, intf_desc, intf_desc->bLength);
        offset += intf_desc->bLength;
        for (int j = 0; j < intf_desc->bNumEndpoints; j++)
        {
            kdrv_usbd_endpoint_descriptor_t *endp_desc = intf_desc->endpoint[j];
            memcpy(buf_desc + offset, endp_desc, endp_desc->bLength);
            offset += endp_desc->bLength;
        }
    }

    bool sts = dma_fifo_transfer_sync_try((uint32_t *)buf_desc, txLen, DMA_TARGET_ACC_CXF, WRITE_FIFO, 50);

    free(buf_desc);

    if (sts)
        return RESP_ACK;
    else
        return RESP_NACK;
}

static void init_fifo_configurations(kdrv_usbd_interface_descriptor_t *intf)
{
    uint32_t fifo_map_val = 0x0;         // for 0x1A8
    uint32_t endp_map0_val = 0x0;        // for 0x1A0
    uint32_t endp_map1_val = 0x0;        // for 0x1A4
    uint32_t fifo_config_val = 0x0;      // for 0x1AC
    uint32_t fifo_int_mask = 0xFFFFFFFF; // for 0x138, default disable all interrupts

    // also need to init fifo-dma control blocks
    for (int fifo = 0; fifo < FIFO_NUM; fifo++)
        cb.fifo_cbs[fifo].enabled = false;

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
        cb.fifo_cbs[fifo].enabled = true;
        cb.fifo_cbs[fifo].enpNo = enpNo;
        cb.fifo_cbs[fifo].endpointAddress = bEndpointAddress;
        cb.fifo_cbs[fifo].maxPacketSize = wMaxPacketSize;
        cb.fifo_cbs[fifo].transferType = bmAttributes & 0x3;
        cb.fifo_cbs[fifo].byteCntReg = 0x1B0 + 4 * fifo;
        cb.fifo_cbs[fifo].isTransferring = false;
        cb.fifo_cbs[fifo].user_buf_addr = 0;
        cb.fifo_cbs[fifo].user_buf_len = 0;
        cb.fifo_cbs[fifo].short_or_zl_packet = false;
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

static int8_t set_configuration(kdrv_usbd_setup_packet_t *setup)
{
    int8_t resp = RESP_STALL;

    uint8_t config_val = setup->wValue & 0xFF;
    if (config_val == 0)
    {
        cb.config_state = CONFIG_ADDRESS_STATE;
        // clear configuration set bit
        UsbRegMaskedClr(REG_DEV_ADR, AFT_CONF);
        resp = RESP_ACK;

        // FIXME clear FIFO-endpoint mapping ?
    }
    else
    {
        kdrv_usbd_config_descriptor_t *config;

        // compare with all configuration descriptos
        for (int i = 0; i < cb.dev_desc->bNumConfigurations; i++)
        {
            config = cb.dev_desc->config[i];
            if (config->bConfigurationValue == config_val)
            {
                cb.config_state = CONFIG_CONFIGURED_STATE;
                resp = RESP_ACK;
                break;
            }
        }

        // FIXME? for only-one interface, should set up FIFO-endpont mapping now
        if (resp == RESP_ACK && config->bNumInterfaces == 1)
            init_fifo_configurations(config->interface[0]);

        if (resp == RESP_ACK)
            notify_event_to_user(KDRV_USBD_EVENT_DEV_CONFIGURED, config_val, 0);
    }

    return resp;
}

static int8_t handle_standard_request(kdrv_usbd_setup_packet_t *setup)
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
            kdrv_usbd_reset_endpoint(setup->wIndex);
            resp = RESP_ACK;
        }
        break;
    }
    case 0x3: // SET_FEATURE
        break;
    }

    // if ep0 is halted, some requests should not be done
    if (cb.ep0_halted)
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
            resp = send_host_device_descriptor(setup);
            break;
        case 2: // CONFIGURATION descriptor
            resp = send_host_configuration_descriptors(setup);
            break;
        case 3: //
            resp = send_host_string_descriptor(setup, descp_idx);
            break;
        case 4: // INTERFACE descriptor
            break;
        case 5: // ENDPOINT descriptor
            break;
        case 6: // DEVICE_QUALIFIER descriptor
            resp = send_host_device_qual_descriptor(setup);
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
        resp = set_configuration(setup);
        break;
    }

    return resp;
}

static void handle_control_transfer()
{
    // ready to read out 8 bytes setup packet
    kdrv_usbd_setup_packet_t setup = {0};
    uint8_t *temp = (uint8_t *)&setup;

    // if DMA is busy now, do nothing and will handle in later interrupts
    if (dma_is_busy())
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
        notify_event_to_user(KDRV_USBD_EVENT_SETUP_PACKET,
                             *((uint32_t *)temp), *((uint32_t *)(temp + 4)));
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

static void handle_device_interrupts()
{
    uint32_t grp_x_int_status = UsbRegRead(REG_DEV_IGR);
    uint32_t grp_0_interrupts = (UsbRegRead(REG_DEV_ISG0) & ~(UsbRegRead(REG_DEV_MISG0)));
    uint32_t grp_2_interrupts = (UsbRegRead(REG_DEV_ISG2) & ~(UsbRegRead(REG_DEV_MISG2)));

    if (grp_x_int_status & GX_INT_G1_RO)
        handle_fifo_interrupts();

    if (grp_x_int_status & GX_INT_G2_RO)
    {
        if (grp_2_interrupts & G2_DMA_CMPLT_RW1C)
            handle_dma_complete_interrupt();
        else if (grp_2_interrupts & G2_RX0BYTE_INT_RW1C)
            handle_zero_length_packet_interrupt();
        else if (grp_2_interrupts & G2_DMA_ERROR_RW1C)
            handle_dma_error();
        else if (grp_2_interrupts & G2_SUSP_INT_RW1C)
            bus_suspend_work();
        else if (grp_2_interrupts & G2_RESM_INT_RW1C)
            bus_resume_work();
        else if (grp_2_interrupts & G2_USBRST_INT_RW1C)
            bus_reset_work();
    }

    if (grp_x_int_status & GX_INT_G0_RO)
    {
        if (grp_0_interrupts & G0_CX_COMABT_INT_RW1C)
            // first priority
            handle_cmd_abort();
        else if (grp_0_interrupts & G0_CX_SETUP_INT_RO)
            handle_control_transfer();
    }
}

// USB ISR
static void usbd_isr(void)
{
    handle_device_interrupts();
}

static void init_reg_isr(void)
{
    SCU_EXTREG_USB_OTG_CTRL_SET_EXTCTRL_SUSPENDM(1);
    SCU_EXTREG_USB_OTG_CTRL_SET_u_iddig(1);
    SCU_EXTREG_USB_OTG_CTRL_SET_wakeup(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_l1_wakeup(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_OSCOUTEN(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_PLLALIV(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_XTLSEL(0);
    SCU_EXTREG_USB_OTG_CTRL_SET_OUTCLKSEL(0);

    // disable all OTG interrupts
    UsbRegWrite(REG_OTG_IER, ~(0x0));

    // enable only Device interrupt (no Host or OTG)
    UsbRegWrite(REG_GLB_INT, ~(INT_POLARITY | DEV_INT) & 0xF);

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
                  G2_SUSP_INT_RW1C |
                  G2_RESM_INT_RW1C |
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

    NVIC_SetVector(OTG_SBS_3_IRQ, (uint32_t)usbd_isr);

    // Clear and Enable SAI IRQ
    NVIC_ClearPendingIRQ(OTG_SBS_3_IRQ);
    NVIC_EnableIRQ(OTG_SBS_3_IRQ);
}

static kdrv_status_t bulk_in_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, int8_t isBlockingCall)
{
    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;

    // find out which FIFO no for the IN endpoint address
    int8_t fno = endpoint_to_fifo(endpoint);

    // below do some error checking
    {
        if (fno == -1)
            return KDRV_STATUS_USBD_INVALID_ENDPOINT;

        if (fifo_cb[fno].transferType != TXFER_BULK)
            return KDRV_STATUS_USBD_INVALID_TRANSFER;

        if (fifo_cb[fno].isTransferring)
            return KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS;
    }

    // set up fifo cb
    fifo_cb[fno].isTransferring = true;
    fifo_cb[fno].user_buf_addr = dma_remap_addr((uint32_t)buf);
    fifo_cb[fno].user_buf_len = txLen;
    // check if need to send a zero-length packet at the end of transfer
    fifo_cb[fno].short_or_zl_packet = ((txLen & (fifo_cb[fno].maxPacketSize - 1)) == 0) ? true : false;
    fifo_cb[fno].isBlockingCall = isBlockingCall;

    // there is a risk of race condition with IRQ when different endponts are working
    NVIC_DisableIRQ(OTG_SBS_3_IRQ);
    {
        // reset FIFO content before transmission
        UsbRegMaskedSet(fifo_cb[fno].byteCntReg, FFRST);

        // enable (unmask) the FIFO IN interrupt
        UsbRegMaskedClr(REG_DEV_MISG1, MF0_IN_INT << fno);
    }
    NVIC_EnableIRQ(OTG_SBS_3_IRQ);

    // now leave and let interrupt handler do the transfer work

    return KDRV_STATUS_OK;
}

static kdrv_status_t bulk_out_receive(uint32_t endpoint, uint32_t *buf, uint32_t blen, int8_t isBlockingCall)
{
    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;

    // find out which FIFO no for the IN endpoint address
    int8_t fno = endpoint_to_fifo(endpoint);

    // below do some error checking
    {
        if (fno == -1)
            return KDRV_STATUS_USBD_INVALID_ENDPOINT;

        if (fifo_cb[fno].transferType != TXFER_BULK)
            return KDRV_STATUS_USBD_INVALID_TRANSFER;

        if (fifo_cb[fno].isTransferring)
            return KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS;
    }

    // set up fifo cb
    fifo_cb[fno].isTransferring = true;
    fifo_cb[fno].user_buf_addr = dma_remap_addr((uint32_t)buf);
    fifo_cb[fno].user_buf_len = blen;
    fifo_cb[fno].received_length = 0;
    fifo_cb[fno].isBlockingCall = isBlockingCall;

    // there is a risk of race condition with IRQ when different endponts are working
    NVIC_DisableIRQ(OTG_SBS_3_IRQ);
    {
        // the OUT interrupts should have been disabled earlier, re-enable it since buffer is commited
        UsbRegMaskedClr(REG_DEV_MISG1, MF0_OUT_INT << (fno * 2));
    }
    NVIC_EnableIRQ(OTG_SBS_3_IRQ);

    return KDRV_STATUS_OK;
}

////////////////// below are API ////////////////////

kdrv_status_t kdrv_usbd_initialize(void)
{
    cb.notifyTid = 0;
    cb.notifyFlag = 0x0;
    cb.ep0_halted = false;
    cb.dev_desc = NULL;

    reset_event_queue();

    cb.evt_id = osEventFlagsNew(NULL);

    init_reg_isr();

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_uninitialize(void)
{
    clean_event_queue();

    osEventFlagsDelete(cb.evt_id);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_reset_device()
{
    // if it is not configured, no need to reset
    if (!kdrv_usbd_is_dev_configured())
        return KDRV_STATUS_ERROR;

    // disable bus
    kdrv_usbd_set_enable(false);

    // first reset all endpoints and terminate all in-progress transfer
    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;
    for (int fno = 0; fno < FIFO_NUM; fno++)
        if (fifo_cb[fno].enabled)
        {
            kdrv_usbd_reset_endpoint(fifo_cb[fno].endpointAddress);
        }

    // some delay for USB bus, FIXME ?
    osDelay(100);

    cb.ep0_halted = false;
    cb.config_state = CONFIG_DEFAULT_STATE;

    // re-init registers and isr
    init_reg_isr();

    reset_event_queue();

    // re-enable bus
    kdrv_usbd_set_enable(true);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_set_device_descriptor(
    kdrv_usbd_speed_t speed,
    kdrv_usbd_device_descriptor_t *dev_desc)
{
    // for now support high speed only, FIXME
    if (speed != KDRV_USBD_HIGH_SPEED)
        return KDRV_STATUS_ERROR;

    if (dev_desc->bNumConfigurations > 1)
        return KDRV_STATUS_ERROR;

    cb.dev_desc = dev_desc;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_set_device_qualifier_descriptor(
    kdrv_usbd_speed_t speed,
    kdrv_usbd_device_qualifier_descriptor_t *dev_qual_desc)
{
    cb.dev_qual_desc = dev_qual_desc;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_set_string_descriptor(
    kdrv_usbd_speed_t speed, kdrv_usbd_string_descriptor_t *dev_str_desc)
{
    cb.dev_string_desc = dev_str_desc;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_set_enable(bool enable)
{
    if (enable)
        // Make PHY work properly, FIXME ?
        UsbRegMaskedClr(REG_PHY_TST, TST_JSTA);
    else
        // Make PHY not work, FIXME ?
        UsbRegMaskedSet(REG_PHY_TST, TST_JSTA);

    return KDRV_STATUS_OK;
}

bool kdrv_usbd_is_dev_configured(void)
{
    if (cb.config_state == CONFIG_CONFIGURED_STATE)
        return true;
    else
        return false;
}

kdrv_status_t kdrv_usbd_get_event(kdrv_usbd_event_t *uevent)
{
    int ret = pop_event_from_queue(uevent);

    if (ret)
        return KDRV_STATUS_OK;
    else
        return KDRV_STATUS_ERROR;
}

kdrv_status_t kdrv_usbd_register_thread_notification(osThreadId_t tid, uint32_t tflag)
{
    cb.notifyTid = tid;
    cb.notifyFlag = tflag;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_control_send(uint8_t *buf, uint32_t size, uint32_t timeout_ms)
{
    uint32_t left_send = size;
    const uint32_t cx_fifo_size = 64;
    uint32_t tryMs = 0;
    bool first_written = true;

    // to make sure cx fifo is empty
    UsbRegMaskedSet(REG_CXCFE, CX_CLR);

    while (left_send > 0)
    {
        bool doTxfer = false;

        if (first_written || (UsbRegRead(REG_DEV_ISG0) & G0_CX_IN_INT_RO))
        {
            uint32_t transfer_size = MIN(cx_fifo_size, left_send);
            // note: there is no G0_CX_IN_INT_RO for first 64-byte write
            // so we need to write it first
            if (dma_fifo_transfer_sync_try((uint32_t *)buf, transfer_size, DMA_TARGET_ACC_CXF, WRITE_FIFO, 500))
            {
                doTxfer = true;
                tryMs = 0;
                left_send -= transfer_size;
                buf += transfer_size;
                first_written = false;
            }
        }

        if (!doTxfer)
        {
            osDelay(1);
            ++tryMs;
            if (tryMs >= timeout_ms)
                return KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
        }
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_control_receive(uint8_t *buf, uint32_t *size, uint32_t timeout_ms)
{
    uint32_t wanted_size = *size;
    uint32_t tryMs = 0;
    *size = 0;

    while (wanted_size > 0)
    {
        bool doTxfer = false;

        if (UsbRegRead(REG_DEV_ISG0) & G0_CX_OUT_INT_RO)
        {
            uint32_t fifo_bytes = (UsbRegRead(REG_CXCFE) >> 24) & 0x7F;
            uint32_t transfer_size = MIN(fifo_bytes, wanted_size);

            if (dma_fifo_transfer_sync_try((uint32_t *)buf, transfer_size, DMA_TARGET_ACC_CXF, READ_FIFO, 500))
            {
                doTxfer = true;
                tryMs = 0;
                wanted_size -= transfer_size;
                buf += transfer_size;
                *size += transfer_size;
            }
        }

        if (!doTxfer)
        {
            osDelay(1);
            ++tryMs;
            if (tryMs >= timeout_ms)
                return KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
        }
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_control_respond(kdrv_usbd_status_respond_t status)
{
    if (status == KDRV_USBD_RESPOND_OK)
        // respond ACK
        UsbRegMaskedSet(REG_CXCFE, CX_DONE);
    else if (status == KDRV_USBD_RESPOND_ERROR)
        // respond STALL
        UsbRegMaskedSet(REG_CXCFE, CX_STL | CX_DONE);
    else
    {
        return KDRV_STATUS_ERROR;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_bulk_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    kdrv_status_t status = bulk_in_send(endpoint, buf, txLen, true);

    if (status != KDRV_STATUS_OK)
        return status;

    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;
    int8_t fno = endpoint_to_fifo(endpoint);

    if (timeout_ms == 0)
        timeout_ms = osWaitForever;

    uint32_t flags = osEventFlagsWait(cb.evt_id, (0x1 << fno), osFlagsNoClear, timeout_ms);
    if (flags == osFlagsErrorTimeout)
    {
        clean_fifo_cb(&fifo_cb[fno]);
        status = KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
    }
    else
    {
        if (flags != 0x1 << fno)
            err_msg("[usbd send] 1+ events 0x%08x (0x%x expected)\n", flags, 1 << fno);
        osEventFlagsClear(cb.evt_id, 0x1 << fno);

        if (fifo_cb[fno].cur_event == KDRV_USBD_EVENT_TRANSFER_DONE)
            status = KDRV_STATUS_OK;
        else
            status = KDRV_STATUS_ERROR;
    }

    clean_fifo_cb(&fifo_cb[fno]);
    return status;
}

kdrv_status_t kdrv_usbd_reset_endpoint(uint32_t endpoint)
{
    int8_t fno = endpoint_to_fifo(endpoint);

    if (fno == -1)
        return KDRV_STATUS_USBD_INVALID_ENDPOINT;

    FIFO_Ctrl *fifo_cb = &cb.fifo_cbs[fno];

    UsbRegMaskedSet(fifo_cb->byteCntReg, FFRST);

    if (fifo_cb->transferType == TXFER_BULK)
    {
        if (fifo_cb->isTransferring)
        {
            // notify transfer done to user
            if (fifo_cb->isBlockingCall)
            {
                fifo_cb->cur_event = KDRV_USBD_EVENT_TRANSFER_TERMINATED;
                osEventFlagsSet(cb.evt_id, 0x1 << fno);
            }
            else
            {
                notify_event_to_user(KDRV_USBD_EVENT_TRANSFER_TERMINATED, fifo_cb->endpointAddress, 0);
            }
            fifo_cb->isTransferring = false;
        }

        uint8_t enpNo = fifo_cb->enpNo;
        if (fifo_cb->endpointAddress & 0x80)
        {
            // for IN endpoints
            UsbRegMaskedSet(REG_DEV_INMPS_1 + 4 * (enpNo - 1), RSTG_IEPn);
            UsbRegMaskedClr(REG_DEV_INMPS_1 + 4 * (enpNo - 1), RSTG_IEPn);
            UsbRegMaskedSet(REG_DEV_MISG1, MF0_IN_INT << fno);
        }
        else
        {
            // for OUT endpoints
            UsbRegMaskedSet(REG_DEV_OUTMPS_1 + 4 * (enpNo - 1), RSTG_OEPn);
            UsbRegMaskedClr(REG_DEV_OUTMPS_1 + 4 * (enpNo - 1), RSTG_OEPn);
            UsbRegMaskedClr(REG_DEV_MISG1, (MF0_SPK_INT | MF0_OUT_INT) << (fno * 2));
        }
        clean_fifo_cb(fifo_cb);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_bulk_send_async(uint32_t endpoint, uint32_t *buf, uint32_t txLen)
{
    return bulk_in_send(endpoint, buf, txLen, false);
}

kdrv_status_t kdrv_usbd_bulk_receive(uint32_t endpoint, uint32_t *buf, uint32_t *blen, uint32_t timeout_ms)
{
    kdrv_status_t status = bulk_out_receive(endpoint, buf, *blen, true);

    if (status != KDRV_STATUS_OK)
        return status;

    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;
    int8_t fno = endpoint_to_fifo(endpoint);

    if (timeout_ms == 0)
        timeout_ms = osWaitForever;

    uint32_t flags = osEventFlagsWait(cb.evt_id, (0x1 << fno), osFlagsNoClear, timeout_ms);
    if (flags == osFlagsErrorTimeout)
    {
        status = KDRV_STATUS_USBD_TRANSFER_TIMEOUT;
    }
    else
    {
        if (flags != 0x1 << fno)
            err_msg("[usbd receive] 1+ events 0x%08x (0x%x expected)\n", flags, 1 << fno);
        osEventFlagsClear(cb.evt_id, 0x1 << fno);

        *blen = fifo_cb[fno].received_length;
        if (fifo_cb[fno].cur_event == KDRV_USBD_EVENT_TRANSFER_DONE)
            status = KDRV_STATUS_OK;
        else
            status = KDRV_STATUS_ERROR;
    }

    clean_fifo_cb(&fifo_cb[fno]);
    return status;
}

kdrv_status_t kdrv_usbd_bulk_receive_async(uint32_t endpoint, uint32_t *buf, uint32_t blen)
{
    return bulk_out_receive(endpoint, buf, blen, false);
}

kdrv_status_t kdrv_usbd_interrupt_send(uint32_t endpoint, uint32_t *buf, uint32_t txLen, uint32_t timeout_ms)
{
    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;

    // find out which FIFO no for the IN endpoint address
    int8_t fno = endpoint_to_fifo(endpoint);

    // below do some error checking
    {
        if (fno == -1)
            return KDRV_STATUS_USBD_INVALID_ENDPOINT;

        if (fifo_cb[fno].transferType != TXFER_INT)
            return KDRV_STATUS_USBD_INVALID_TRANSFER;
    }

    // try to wait some time for FIFO empty
    // FIXME: 3000 ?
    for (uint32_t try = 0; try <= 3000; try ++)
        if (UsbRegRead(REG_CXCFE) & (F_EMP_0 << fno))
            break;

    // directly clear FIFO content before transmission
    UsbRegMaskedSet(fifo_cb[fno].byteCntReg, FFRST);

    uint32_t tryMs = 0;
    while (1)
    {
        if (true == dma_fifo_transfer_sync_try(buf, txLen, 0x1 << fno, WRITE_FIFO, 500))
            break;

        if (timeout_ms != 0 && tryMs >= timeout_ms)
            return KDRV_STATUS_USBD_TRANSFER_TIMEOUT;

        osDelay(1);
        ++tryMs;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd_interrupt_receive(uint32_t endpoint, uint32_t *buf, uint32_t *rxLen, uint32_t timeout_ms)
{
    FIFO_Ctrl *fifo_cb = cb.fifo_cbs;

    // find out which FIFO no for the IN endpoint address
    int8_t fno = endpoint_to_fifo(endpoint);

    // below do some error checking
    {
        if (fno == -1)
            return KDRV_STATUS_USBD_INVALID_ENDPOINT;

        if (fifo_cb[fno].transferType != TXFER_INT)
            return KDRV_STATUS_USBD_INVALID_TRANSFER;
    }

    uint32_t fifo_bytecnt = UsbRegRead(fifo_cb[fno].byteCntReg) & BC_Fn;
    // can transfer only minimum size betwwen FIFO byte count and user buffer residual size
    uint32_t transfer_size = MIN(fifo_bytecnt, *rxLen);

    uint32_t tryMs = 0;
    while (1)
    {
        if (true == dma_fifo_transfer_sync_try(buf, transfer_size, 0x1 << fno, READ_FIFO, 500))
            break;

        if (timeout_ms != 0 && tryMs >= timeout_ms)
            return KDRV_STATUS_USBD_TRANSFER_TIMEOUT;

        osDelay(1);
        ++tryMs;
    }

    return KDRV_STATUS_OK;
}
