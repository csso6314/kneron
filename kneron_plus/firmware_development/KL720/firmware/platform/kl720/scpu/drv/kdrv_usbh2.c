// #define USBH2_DBG // turn on this for debug logs
//#define USE_SRAM_TEST // FIXME, chip validation test only

#include "cmsis_os2.h" // CMSIS RTOS header file

#include "kdrv_usbh2.h"
#include "kneron_kl720.h"
#include "kdrv_scu_ext.h"

#ifdef USE_SRAM_TEST
#include <stdlib.h>
#else
#include "kmdw_memory.h"
#endif
#include <string.h>

#include "base.h"
#include "io.h"
#include "kdrv_cmsis_core.h"

#ifdef USBH2_DBG
#define usbh2_dbg(__format__, ...) printf(__format__, ##__VA_ARGS__)
#else
#define usbh2_dbg(__format__, ...)
#endif

/*  Register read/write macros */

// FIXME: copied from kdp_usbd_reg.h
#define EHCI_USBCMD 0x10
/* USBSTS bit fields */
#define ASCH_EN BIT5
#define PSCH_EN BIT4
#define HC_RESET BIT1
#define RS BIT0
/* end */

#define EHCI_USBSTS 0x14
/* USBSTS bit fields */
#define ASCH_STS BIT15
#define PSCH_STS BIT14
#define HCHalted BIT12
#define INT_OAA BIT5
#define H_SYSERR BIT4
#define FRL_ROL BIT3
#define PO_CHG_DET BIT2
#define USBERR_INT BIT1
#define USB_INT BIT0
/* end */

#define EHCI_USBINTR 0x18
/* USBINTR bit fields */
#define H_SYSERR_EN BIT4
#define PO_CHG_INT_EN BIT2
#define USBERR_INT_EN BIT1
#define USB_INT_EN BIT0
/* end */

#define EHCI_FRINDEX 0x1C

#define EHCI_PERIODIC_LIST_ADDR 0x24
#define EHCI_ASYNC_LIST_ADDR 0x28

#define EHCI_PORTSC 0x30
/* USBINTR bit fields */
#define PO_RESET BIT8
#define PO_SUSP BIT7
#define F_PO_RESM BIT6
#define PO_EN BIT2
#define CONN_CHG BIT1
#define CONN_STS BIT0
/* end */

#define REG_OTG_CSR 0x80
#define A_BUS_DROP BIT5
#define A_BUS_REQ BIT4

#define REG_OTG_ISR 0x84
#define OTG_OVC_RW1C BIT10

#define REG_GLB_INT 0xC4

#define REG_DEV_CTL 0x100

// PHY Test Mode Selector Register (0x114)
#define REG_PHY_TST 0x114
#define TST_JSTA BIT0

#define UsbRegRead(reg_offset) inw(USB2_REG_BASE + reg_offset)
#define UsbRegWrite(reg_offset, val) outw(USB2_REG_BASE + reg_offset, val)
#define UsbRegMaskedSet(reg_offset, val) masked_outw(USB2_REG_BASE + reg_offset, val, val)
#define UsbRegMaskedClr(reg_offset, val) masked_outw(USB2_REG_BASE + reg_offset, 0, val)

typedef enum
{
    ASYNC_SCHEDULE = ASCH_EN,
    PERIODIC_SCHEDULE = PSCH_EN,
} EHCI_SCHEDULE_TYPE;

#ifdef USE_SRAM_TEST

/* Queue Head */
#define QH_NUM 5
#define QH_REAL_SIZE 48 // EHCI actual size
#define QH_ALLOC_IZE 64 // for 32-byte alignment and some meta-data usage

#define qTD_NUM 10
#define qTD_REAL_SIZE 32  // EHCI actual size
#define qTD_ALLOC_SIZE 64 // for 32-byte alignment and some meta-data usage

#define MAX_PIPES QH_NUM

#define FRAME_LIST_SIZE 1024
#define FRAME_LIST_ROLL_MASK (FRAME_LIST_SIZE - 1)

#define iTD_MAX_NUM 256
#define iTD_SIZE 64

#else

/* Queue Head */
#define QH_NUM 10
#define QH_REAL_SIZE 48 // EHCI actual size
#define QH_ALLOC_IZE 64 // for 32-byte alignment and some meta-data usage

#define qTD_NUM 50
#define qTD_REAL_SIZE 32  // EHCI actual size
#define qTD_ALLOC_SIZE 64 // for 32-byte alignment and some meta-data usage

#define MAX_PIPES QH_NUM

#define FRAME_LIST_SIZE 1024
#define FRAME_LIST_ROLL_MASK (FRAME_LIST_SIZE - 1)

#define iTD_MAX_NUM 1024
#define iTD_SIZE 64

#endif

// for QH, qTD and iTD link pointer dwords
struct Link_Pointer_DWord
{
    uint32_t terminate : 1;
    uint32_t type : 2; //  valid only for QH and iTD
    uint32_t reserved : 2;
    uint32_t ptr_address : 27;
};

// qTD DWORD 2
struct Status_DWord
{
    uint32_t status : 8;
    uint32_t PID : 2;
    uint32_t CERR : 2;
    uint32_t c_page : 3;
    uint32_t ioc : 1;
    uint32_t total_bytes_txfer : 15;
    uint32_t dt : 1;
};

// qTD Buffer Pointer DWORD
struct Buffer_Pointer_DWord
{
    uint32_t cur_offset : 12;  // only valid for DWORD 3
    uint32_t ptr_address : 20; // 4KBytes-aligned addrees
};

// qTD data structure
typedef struct __attribute__((__packed__))
{
    // DWORD 0 : Next qTD Pointer
    struct Link_Pointer_DWord next_qtd;

    // DWORD 1 : Alternate Next qTD Pointer
    struct Link_Pointer_DWord alt_next_qtd;

    // DWORD 2 : Status, PID Code, Error Counter, Current Page, Interrupt On Complete, Total Bytes to Transfer, Data Toggle
    struct Status_DWord status_dword;

    // DWORD 3~7 : Buffer Pointer pages and current offset
    struct Buffer_Pointer_DWord buf_ptr_dword[5];

    // ======== below is for internal metada ========
    uint32_t in_use;
    uint32_t wanted_txfer_bytes; // record the wanted txfer bytes
    // ==============================================

} Q_TD;

// iTD Transaction DWORD
struct Transaction_DWord
{
    uint32_t offset : 12;
    uint32_t pg : 3;
    uint32_t ioc : 1;
    uint32_t length : 12;
    uint32_t status : 4;
};

// iTD Buffer Pointer Page DWORD (generic)
struct Buffer_Page_DWord
{
    uint32_t misc : 12;        // for page 0~2, use differnt structs
    uint32_t ptr_address : 20; // 4KBytes-aligned addrees
};

// iTD Buffer Pointer Page 0 DWORD
struct Buffer_Page_0_DWord
{
    uint32_t dev_addr : 7;
    uint32_t reserved : 1;
    uint32_t endpoint : 4;
    uint32_t ptr_address : 20; // 4KBytes-aligned addrees
};

// iTD Buffer Pointer Page 1 DWORD
struct Buffer_Page_1_DWord
{
    uint32_t max_packet_size : 11;
    uint32_t direction : 1;
    uint32_t ptr_address : 20; // 4KBytes-aligned addrees
};

// iTD Buffer Pointer Page 2 DWORD
struct Buffer_Page_2_DWord
{
    uint32_t mult : 2; // valid values are 1~3, indidcates number of transaction per micro-frame
    uint32_t reserved : 10;
    uint32_t ptr_address : 20; // 4KBytes-aligned addrees
};

// iTD data structure
typedef struct __attribute__((__packed__))
{
    // DWORD 0 : Next qTD Pointer
    struct Link_Pointer_DWord next_link; // QH or iTD

    // DWORD 1~8 : Transaction DWORDs
    struct Transaction_DWord trans_dword[8];

    // DWORD 9~15 : Buffer Pointer pages and others
    struct Buffer_Page_DWord buf_page_dword[7];

} I_TD;

// QH DWORD 1
struct Device_Address_DWord
{
    uint32_t dev_addr : 7;
    uint32_t inactivate : 1;
    uint32_t endpoint : 4;
    uint32_t endpt_speed : 2;
    uint32_t dtc : 1;
    uint32_t H_reclamation : 1;
    uint32_t max_packet_length : 11;
    uint32_t C_endpt_flag : 1;
    uint32_t RL_Nak_count : 4;
};

// QH DWORD 2
struct Mask_DWord
{
    uint32_t S_mask : 8;
    uint32_t C_mask : 8;
    uint32_t hub_addr : 7;
    uint32_t port_number : 7;
    uint32_t mult : 2;
};

// Queue Head data structure
typedef struct __attribute__((__packed__))
{
    // DWORD 0
    struct Link_Pointer_DWord next_qh;

    // DWORD 1
    struct Device_Address_DWord device_dword;

    // DWORD 2
    struct Mask_DWord mask_dword;

    // DWORD 3 : Current qTD Pointer
    uint32_t cur_qtd_ptr;

    // DWORD 4~11 : overlay of qTD
    uint32_t overlay_next_qtd;        // qTD dword 0
    uint32_t overlay_alternate_qtd;   // qTD dword 1
    uint32_t overlay_dword_others[6]; // qTD dword 2~7, dont care

    // ======== below is for internal metada ========
    uint32_t prev_link_ptr;     // make it double linked list
    uint32_t all_txfered_bytes; // record all txfered bytes
    uint8_t is_txfering;        // mark it '1' if this QH is doing transfer, otherwise '0'
    uint8_t in_use;
    uint8_t endpoint_type; // control/bulk/interrupt/isochronous
    // ==============================================

} Queue_Head;

static bool hw_initialized = false;
static uint32_t QH_addr[QH_NUM];
static uint32_t qTD_addr[qTD_NUM];
static uint32_t iTD_addr[iTD_MAX_NUM];
static uint32_t *periodic_frame_list = 0;
static uint32_t ehci_struct_ddr_addr = 0;

static kdrv_usbh2_port_event_callback_t notify_port_event_cb = 0;
static kdrv_usbh2_pipe_event_callback_t notify_pipe_event_cb = 0;

static uint32_t usbh2_handle_iTD_work(void);

static kdrv_usbh2_isoch_data_callback_t notify_isoch_data_cb = 0;
static kdrv_usbh2_isoch_itd_work_func_t isoch_process_itd_work = usbh2_handle_iTD_work; // it can be replaced by user bottom-half callback

// below are all for iTD management
static uint32_t isoch_running_flag = 0;     // ISOCH periodic schelde running flag
static uint32_t isoch_next_process_pos = 0; // next processing pos for iTD
static uint8_t isoch_trans_interval = 0;    // transaction interval in a iTD
static uint32_t isoch_txfer_len = 0;        // total transfer length
static uint8_t isoch_iTD_interval = 0;      // iTD interval in the periodic frame list
static uint8_t isoch_IOC_pos = 0;           // position of the IOC flag
static uint32_t isoch_iTD_num = 0;          // number of iTD

static void fotg210_ehci_USBCMD_schedule_set_enable(EHCI_SCHEDULE_TYPE schedule_type, bool enable)
{

    uint32_t usbsts = UsbRegRead(EHCI_USBSTS);

    uint32_t bit_check;
    if (schedule_type == ASCH_EN)
        bit_check = BIT15;
    else
        bit_check = BIT14;

    if (enable)
    {
        if (usbsts & bit_check)
            return;

        UsbRegMaskedSet(EHCI_USBCMD, schedule_type);

        // polling wait for status change
        while ((UsbRegRead(EHCI_USBSTS) & bit_check) == 0)
            ;
    }
    else
    {
        if ((usbsts & bit_check) == 0)
            return;

        UsbRegMaskedClr(EHCI_USBCMD, schedule_type);

        // polling wait for status change
        while ((UsbRegRead(EHCI_USBSTS) & bit_check) != 0)
            ;
    }
}

static void fotg210_ehci_USBCMD_HC_run()
{
    // According to EHCI spec, doing USBCMD Run only if USBSTS is in HCHalted
    if (UsbRegRead(EHCI_USBSTS) & HCHalted)
    {
        UsbRegMaskedSet(EHCI_USBCMD, RS);

        // polling wait for !HCHalted
        while ((UsbRegRead(EHCI_USBSTS) & HCHalted))
            ;
    }
}

static void fotg210_ehci_USBCMD_HC_stop()
{
    if (!(UsbRegRead(EHCI_USBSTS) & HCHalted))
    {
        // first disable all scheduling works
        fotg210_ehci_USBCMD_schedule_set_enable(ASYNC_SCHEDULE, false);
        fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, false);

        UsbRegMaskedClr(EHCI_USBCMD, RS);

        // polling wait for HCHalted
        while (!(UsbRegRead(EHCI_USBSTS) & HCHalted))
            ;
    }
}

static void fotg210_ehci_data_struct_from_ddr()
{
    // alloacte a pool of memory from DDR

    uint32_t needed_size = QH_NUM * QH_ALLOC_IZE + qTD_NUM * qTD_ALLOC_SIZE + iTD_MAX_NUM * iTD_SIZE + 128; // 128 just for more space

#ifdef USE_SRAM_TEST
    ehci_struct_ddr_addr = (uint32_t)malloc(needed_size);
    if (ehci_struct_ddr_addr == 0)
    {
        printf("[usbh2] malloc failed !\n");
        return;
    }
#else
    ehci_struct_ddr_addr = kmdw_ddr_reserve(needed_size);
#endif

    memset((void *)ehci_struct_ddr_addr, 0, needed_size);
    ehci_struct_ddr_addr &= (~0x1F);

    usbh2_dbg("[usbh2] %s() allocate ddr mem %d bytes\n", __FUNCTION__, needed_size);
    usbh2_dbg("[usbh2] Q_TD size %d, Queue Head size %d\n", sizeof(Q_TD), sizeof(Queue_Head));

    QH_addr[0] = ehci_struct_ddr_addr;
    for (int i = 1; i < QH_NUM; i++)
    {
        QH_addr[i] = QH_addr[i - 1] + QH_ALLOC_IZE;
    }

    qTD_addr[0] = QH_addr[QH_NUM - 1] + QH_ALLOC_IZE;
    for (int i = 1; i < qTD_NUM; i++)
    {
        qTD_addr[i] = qTD_addr[i - 1] + qTD_ALLOC_SIZE;
    }

    iTD_addr[0] = qTD_addr[QH_NUM - 1] + qTD_ALLOC_SIZE;
    for (int i = 1; i < iTD_MAX_NUM; i++)
    {
        iTD_addr[i] = iTD_addr[i - 1] + iTD_SIZE;
    }

#ifdef USE_SRAM_TEST
    periodic_frame_list = (uint32_t *)malloc(2 * sizeof(uint32_t) * FRAME_LIST_SIZE); // FIXME: double the allocated size due to 4-KB alignment
    if (periodic_frame_list == 0)
    {
        printf("[usbh2] malloc failed !\n");
        return;
    }
#else
    periodic_frame_list = (uint32_t *)kmdw_ddr_reserve(2 * sizeof(uint32_t) * FRAME_LIST_SIZE); // FIXME: double the allocated size due to 4-KB alignment
#endif
    periodic_frame_list = (uint32_t *)(((uint32_t)periodic_frame_list & (~0xFFF)) + 0x1000); // now make periodic_frame_list begin from 4-KB boundray address

    for (int i = 0; i < FRAME_LIST_SIZE; i++)
    {
        periodic_frame_list[i] = 0x1; // mark T-bit as 1 indicating invalid pointers
    }
}

// USB host ISR
static void usbh2_isr(void)
{
    // read the USB Status register
    uint32_t usbsts = UsbRegRead(EHCI_USBSTS);

    // USB transaction completeion
    if (usbsts & USB_INT)
    {
        uint32_t txfer_complt = 0; // is any transfer completed ?

        if (isoch_running_flag)
        {
            txfer_complt = isoch_process_itd_work();
        }

        // scan all QHs
        for (int i = 0; i < QH_NUM; i++)
        {
            Queue_Head *qh = (Queue_Head *)QH_addr[i];
            if (qh->is_txfering)
            {
                Q_TD *qTD = (Q_TD *)(qh->cur_qtd_ptr & (~0x1F));

                if (qTD && qTD->in_use == true)
                {
                    uint8_t status_bits = qTD->status_dword.status;
                    if (status_bits <= 0x1) // 0 or 1
                    {
                        qh->is_txfering = false;
                        qTD->in_use = false;
                        qh->all_txfered_bytes += (qTD->wanted_txfer_bytes - qTD->status_dword.total_bytes_txfer);

                        // notify middleware of the completeion
                        notify_pipe_event_cb((uint32_t)qh, USBH2_EVENT_TRANSFER_COMPLETE);

                        txfer_complt = 1;
                    }
                    else if (status_bits == 0x80) // still in Active ? guess DDR update slower ?
                    {
                        // A workaround, we skip this interrupt
                        // just do nothing, keep 'txfer_complt = 0'
                    }
                    else
                    {
                        usbh2_dbg("[usbh2] -- QH 0x%p Q_TD 0x%p error, status 0x%x, CERR 0x%x, in_use %d\n", qh, qTD, qTD->status_dword.status, qTD->status_dword.CERR, qTD->in_use);
                        txfer_complt = 1;
                    }
                }
            }
        }

        if (txfer_complt > 0) // a workaround for ZLP interrupt coming prior to status bit update in DDR
            UsbRegMaskedSet(EHCI_USBSTS, USB_INT);
    }

    // USB transaction error
    if (usbsts & USBERR_INT)
    {
        usbh2_dbg("[usbh2] -- transaction error\n");
        UsbRegMaskedSet(EHCI_USBSTS, USBERR_INT);
        //		UsbRegMaskedSet(EHCI_USBCMD, HC_RESET);
    }

    // port change detected
    if (usbsts & PO_CHG_DET)
    {
        usbh2_dbg("[usbh2] -- port change detected\n");

        // read the PORTSC
        uint32_t portsc = UsbRegRead(EHCI_PORTSC);

        usbh2_dbg("[usbh2] %s(), portsc: 0x%x\n", __FUNCTION__, portsc);
        // check if connect status change in PORTSC
        if (portsc & CONN_CHG)
        {
            // write 1 to clear bit
            UsbRegMaskedSet(EHCI_PORTSC, CONN_CHG);

            usbh2_dbg("[usbh2] -- notify USBH2_EVENT_CONNECT/DISCONNECT\n");

            // call back for USB middleware
            if (portsc & CONN_STS)
            {
                notify_port_event_cb(0, USBH2_EVENT_CONNECT);
            }
            else
            {
                notify_port_event_cb(0, USBH2_EVENT_DISCONNECT);
            }
        }
        // Port changed in USBSTS but no connection change in PORTSC
        // so guess it could be a port reset
        else
        {
            usbh2_dbg("[usbh2] -- notify USBH2_EVENT_RESET\n");
            notify_port_event_cb(0, USBH2_EVENT_RESET);
        }

        UsbRegMaskedSet(EHCI_USBSTS, PO_CHG_DET);
    }

    // host system error
    if (usbsts & H_SYSERR)
    {
        /* serious error here */
        usbh2_dbg("[usbh2] --  host system error\n");
        UsbRegMaskedSet(EHCI_USBSTS, H_SYSERR);
    }

    // async schedule advanced
    if (usbsts & INT_OAA)
    {
        usbh2_dbg("[usbh2] -- async schedule advanced\n");
        UsbRegMaskedSet(EHCI_USBSTS, INT_OAA);
    }

    //NVIC_EnableIRQ(USB2_IRQn);
}

static void usb2_hardware_init()
{
    volatile uint32_t *reg_usb2_phy_control = (volatile uint32_t *)SCU_EXTREG_USB2_PHY_CTRL_REG;

    // clock source
    (*reg_usb2_phy_control) |= (BIT0);

    osDelay(5);

    // assert reset for USB2 phy PONRST
    (*reg_usb2_phy_control) &= ~(BIT24);

    osDelay(1);

    // assert reset for USB2 controller and phy
    (*reg_usb2_phy_control) &= ~(BIT25 | BIT26);

    osDelay(1);

    // de-assert PONRST
    (*reg_usb2_phy_control) |= BIT24;

    osDelay(1);

    // de-assert USB3 phy
    (*reg_usb2_phy_control) |= BIT25;

    osDelay(1);

    // de-assert USB3 controller
    (*reg_usb2_phy_control) |= BIT26;

    osDelay(1);
}

/*******************************************************************************************************/
// API implementations
/*******************************************************************************************************/

kdrv_status_t kdrv_usbh2_initialize(
    kdrv_usbh2_port_event_callback_t cb_port_event,
    kdrv_usbh2_pipe_event_callback_t cb_pipe_event)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    if (hw_initialized)
    {
        return KDRV_STATUS_OK;
    }

    usb2_hardware_init();

    // ask for DDR memory for EHCI data structures
    fotg210_ehci_data_struct_from_ddr();

    notify_port_event_cb = cb_port_event;
    notify_pipe_event_cb = cb_pipe_event;

    NVIC_SetVector(USB2_IRQn, (uint32_t)usbh2_isr);

    // Clear and Enable IRQ
    NVIC_ClearPendingIRQ(USB2_IRQn);
    NVIC_EnableIRQ(USB2_IRQn);

    // set up only host interrupt
    UsbRegWrite(REG_GLB_INT, 0x3);

    // enable chip
    UsbRegMaskedSet(REG_DEV_CTL, BIT5);

    // USBCMD - reset host controller
    UsbRegMaskedSet(EHCI_USBCMD, HC_RESET);

    // USBCMD - polling to wait for the reset complete done
    while ((UsbRegRead(EHCI_USBCMD) & HC_RESET) > 0)
        ;

    // enable EHCI USBINTR
    UsbRegWrite(EHCI_USBINTR, H_SYSERR_EN | PO_CHG_INT_EN | USBERR_INT_EN | USB_INT_EN);

    // zero Async List Addr before first QH is inserted
    UsbRegWrite(EHCI_ASYNC_LIST_ADDR, 0x0);

    // init periodic Aist Addr
    UsbRegWrite(EHCI_PERIODIC_LIST_ADDR, (uint32_t)&periodic_frame_list[0]);

    hw_initialized = true;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_uninitialize(void)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    // FIXME: cannot free DDR memory

#ifdef USE_SRAM_TEST
    free((void *)ehci_struct_ddr_addr);
    free(periodic_frame_list);
#endif

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_set_enable(bool enable)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    // USBCMD - Run HC
    if (enable)
        fotg210_ehci_USBCMD_HC_run();

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_vbus_on_off(uint8_t port, bool vbus)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    if (vbus == true)
    {
        // turn on VBUS
        UsbRegMaskedClr(REG_OTG_CSR, A_BUS_DROP);
        UsbRegMaskedSet(REG_OTG_CSR, A_BUS_REQ);
    }
    else
    {
        // turn off VBUS
        UsbRegMaskedClr(REG_OTG_CSR, A_BUS_REQ);
        UsbRegMaskedSet(REG_OTG_CSR, A_BUS_DROP);
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_port_reset(uint8_t port)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    //NVIC_DisableIRQ(USB2_IRQn);

    // stop host controller
    fotg210_ehci_USBCMD_HC_stop();

    // port disable
    UsbRegMaskedClr(EHCI_PORTSC, PO_EN);

    // holding port reset for 55 ms
    UsbRegMaskedSet(EHCI_PORTSC, PO_RESET);
    osDelay(55);
    UsbRegMaskedClr(EHCI_PORTSC, PO_RESET);

    // run host controller again
    fotg210_ehci_USBCMD_HC_run();

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_port_suspend(uint8_t port)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    // make sure HC is running
    if (UsbRegRead(EHCI_USBSTS) & HCHalted)
        return KDRV_STATUS_ERROR;

    // make sure port is enabled
    if (!(UsbRegRead(EHCI_PORTSC) & PO_EN))
        return KDRV_STATUS_ERROR;

    // disable all scheduling works
    fotg210_ehci_USBCMD_schedule_set_enable(ASYNC_SCHEDULE, false);
    fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, false);

    fotg210_ehci_USBCMD_HC_stop(); // due to Faraday

    // suspend port according to EHCI spec
    UsbRegMaskedSet(EHCI_PORTSC, PO_SUSP);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_port_resume(uint8_t port)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    // make sure port is suspending
    if (!(UsbRegRead(EHCI_PORTSC) & PO_SUSP))
        return KDRV_STATUS_ERROR;

    osDelay(10); // due to Faraday

    // resume port according to EHCI spec
    UsbRegMaskedSet(EHCI_PORTSC, F_PO_RESM);

    osDelay(20); // due to Faraday

    UsbRegMaskedClr(EHCI_PORTSC, F_PO_RESM); // due to Faraday ??

    fotg210_ehci_USBCMD_HC_run(); // due to Faraday

    // PORTSC - polling to wait for the exit from suspend status
    while ((UsbRegRead(EHCI_PORTSC) & PO_SUSP))
        ;

    fotg210_ehci_USBCMD_schedule_set_enable(ASYNC_SCHEDULE, true);
    fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, true);

    return KDRV_STATUS_OK;
}

kdrv_usbh2_port_state_t kdrv_usbh2_port_get_state(uint8_t port)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    kdrv_usbh2_port_state_t port_state;

    port_state.connected = UsbRegRead(EHCI_PORTSC) & CONN_STS;
    port_state.overcurrent = !!(UsbRegRead(REG_OTG_ISR) & OTG_OVC_RW1C);

    // refer to the register OTG_CSR in FOTG210 bloack data sheet
    uint32_t speed = (UsbRegRead(REG_OTG_CSR) >> 22) & 0x3;
    switch (speed)
    {
    case 0x00:
        port_state.speed = USBH2_SPEED_FULL;
        break;
    case 0x01:
        port_state.speed = USBH2_SPEED_LOW;
        break;
    case 0x02:
        port_state.speed = USBH2_SPEED_HIGH;
        break;
    }

    return port_state;
}

#define HORI_LINK_TPYE_iTD 0x0
#define HORI_LINK_TPYE_QH 0x1

// this pipe create is only for Control/Bulk/Interrupt transfer
// so 'ep_type' can only be USBH2_ENDPOINT_CONTROL, USBH2_ENDPOINT_BULK or USBH2_ENDPOINT_INTERRUPT
kdrv_usbh2_pipe_t kdrv_usbh2_pipe_create(uint8_t dev_addr, uint8_t dev_speed, uint8_t hub_addr,
                                         uint8_t hub_port, uint8_t ep_addr, uint8_t ep_type,
                                         uint16_t ep_max_packet_size, uint8_t ep_interval)
{
    usbh2_dbg("[usbh2] %s() ep_addr 0x%x ep_type 0x%x ep_max_packet %u ep_interval %u\n", __FUNCTION__, ep_addr, ep_type, ep_max_packet_size, ep_interval);

    // insert a new QH into the async scheudle list without Q_TD

    Queue_Head *qh = 0;
    uint32_t idx;

    // find a free QH
    for (idx = 0; idx < QH_NUM; idx++)
    {
        qh = (Queue_Head *)QH_addr[idx];
        // check in-use or not
        if (qh->in_use == false)
        {
            // fill up the QH
            memset(qh, 0, QH_REAL_SIZE);
            qh = (Queue_Head *)QH_addr[idx];
            qh->in_use = true;
            break;
        }
    }

    if (idx >= QH_NUM)
    {
        usbh2_dbg("[usbh2] %s() no more QH !!!\n");
        return NULL;
    }

    // DWORD 0

    // DWORD 1
    qh->device_dword.dev_addr = dev_addr;
    qh->device_dword.endpoint = ep_addr & 0xF;
    qh->device_dword.endpt_speed = dev_speed;
    qh->device_dword.dtc = 1;           // ????
    qh->device_dword.H_reclamation = 0; // ????
    qh->device_dword.max_packet_length = ep_max_packet_size;

    if (ep_addr != 0x0)
        qh->device_dword.dtc = 0; // test

    // DWORD 2
    qh->mask_dword.hub_addr = hub_addr;
    qh->mask_dword.port_number = hub_port;

    // Q_TD overlay
    qh->overlay_next_qtd |= 0x1;    // T-bit = 1
    qh->overlay_alternate_qtd |= 1; // T-bit = 1

    // internal use
    qh->all_txfered_bytes = 0;
    qh->endpoint_type = ep_type;

    if (ep_type == USBH2_ENDPOINT_CONTROL || ep_type == USBH2_ENDPOINT_BULK)
    {

        // let's add this QH into the first position
        if (UsbRegRead(EHCI_ASYNC_LIST_ADDR) == 0x0)
        {
            // the very first QH
            qh->next_qh.type = HORI_LINK_TPYE_QH; // QH type
            //qh->next_qh.terminate = 0; // default
            qh->next_qh.ptr_address = ((uint32_t)qh >> 5);
            qh->prev_link_ptr = (uint32_t)qh;
            UsbRegWrite(EHCI_ASYNC_LIST_ADDR, (uint32_t)qh);
        }
        else
        {
            // Insert the new QH into the first position
            Queue_Head *qh_cur = (Queue_Head *)UsbRegRead(EHCI_ASYNC_LIST_ADDR);
            Queue_Head *qh_next = (Queue_Head *)(qh_cur->next_qh.ptr_address << 5);

            qh->next_qh.ptr_address = ((uint32_t)qh_next >> 5);
            qh->prev_link_ptr = (uint32_t)qh_cur;

            qh_next->prev_link_ptr = (uint32_t)qh;
            qh_cur->next_qh.ptr_address = ((uint32_t)qh >> 5);
        }

        // enable schedules only if dev_speed = High Speed (0x2), FIXME
        if (dev_speed == USBH2_SPEED_HIGH)
            fotg210_ehci_USBCMD_schedule_set_enable(ASYNC_SCHEDULE, true);
    }
    else if (ep_type == USBH2_ENDPOINT_INTERRUPT) // FIXME: now it supports only one QH
    {
        if (ep_interval < 4)
            ep_interval = 4; // FIXME, we support minimum interval is 1ms

        uint32_t intv_per_ms = (0x1 << (ep_interval - 4));

        qh->next_qh.type = HORI_LINK_TPYE_QH; // QH type
        qh->next_qh.terminate = 1;            // no other QH
        qh->mask_dword.mult = 0x1;
        qh->mask_dword.S_mask = 0x1; // indicate that trigger at Frindex 000b

        for (int i = 0; i < FRAME_LIST_SIZE; i += intv_per_ms)
        {
            // check T-bit
            if (periodic_frame_list[i] != 0x1)
                usbh2_dbg("[usbh2] warning: this frame list link pointer is occupied!! idx = %u\n", i);

            periodic_frame_list[i] = (uint32_t)qh | (HORI_LINK_TPYE_QH << 1); // setting Frame List Link Pointer T-bit as '0'
        }

        // enable schedules only if dev_speed = High Speed (0x2), FIXME
        if (dev_speed == USBH2_SPEED_HIGH)
            fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, true);
    }
    else // Isochronous transfer
    {
        usbh2_dbg("[usbh2] kdrv_usbh2_pipe_create() does not support Isochronous type\n");
    }

    return (uint32_t)qh;
}

kdrv_status_t kdrv_usbh2_pipe_modify(kdrv_usbh2_pipe_t pipe_hndl, uint8_t dev_addr,
                                     uint8_t dev_speed, uint8_t hub_addr, uint8_t hub_port, uint16_t ep_max_packet_size)
{
    // assume only control transfer do this

    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    Queue_Head *qh = (Queue_Head *)pipe_hndl;

    // DWORD 0
    qh->next_qh.type = 0x1; // QH type

    // DWORD 1
    qh->device_dword.dev_addr = dev_addr;
    qh->device_dword.endpt_speed = dev_speed;
    qh->device_dword.max_packet_length = ep_max_packet_size;

    // DWORD 2
    qh->mask_dword.hub_addr = hub_addr;
    qh->mask_dword.port_number = hub_port;

    // enable schedules only if dev_speed = High Speed (0x2), FIXME
    if (dev_speed == USBH2_SPEED_HIGH)
    {
        fotg210_ehci_USBCMD_schedule_set_enable(ASYNC_SCHEDULE, true);
        fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, true);
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_pipe_transfer(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_packet_t packet, uint8_t *data, uint32_t num)
{

    Queue_Head *qh = (Queue_Head *)pipe_hndl;

    usbh2_dbg("[usbh2] %s() pipe_hndl %p packet 0x%x data %p num %u\n", __FUNCTION__, (void *)pipe_hndl, packet, data, num);

    if (qh->is_txfering)
    {
        usbh2_dbg("[usbh2] error: pipe 0x%p is already doing transfer, %x\n", (void *)pipe_hndl, qh->is_txfering);
        return KDRV_STATUS_ERROR;
    }

    Q_TD *qTD = 0;
    uint32_t qtd_idx;

    // find a free Q_TD
    for (qtd_idx = 0; qtd_idx < qTD_NUM; qtd_idx++)
    {
        qTD = (Q_TD *)qTD_addr[qtd_idx];
        // check in-use or not
        if (qTD->in_use == false)
        {
            memset(qTD, 0, qTD_REAL_SIZE);
            qTD->in_use = true;
            break;
        }
    }

    // no more Q_TD available
    if (qtd_idx >= qTD_NUM)
        return KDRV_STATUS_ERROR;

    qTD->next_qtd.terminate = 1;
    qTD->alt_next_qtd.terminate = 1;

    switch (packet & USBH2_PACKET_TOKEN_Msk)
    {
    case USBH2_PACKET_OUT:
        qTD->status_dword.PID = 0x0;
        break;
    case USBH2_PACKET_IN:
        qTD->status_dword.PID = 0x1;
        break;
    case USBH2_PACKET_SETUP:
        qTD->status_dword.PID = 0x2;
        break;
    }

    switch (packet & USBH2_PACKET_DATA_Msk)
    {
    case USBH2_PACKET_DATA0:
        qTD->status_dword.dt = 0;
        break;
    case USBH2_PACKET_DATA1:
        qTD->status_dword.dt = 1;
        break;
    }

    qTD->status_dword.CERR = 0x0; // FIXME: it 0x3 OK ?
    qTD->status_dword.c_page = 0;
    qTD->status_dword.ioc = 1;
    qTD->status_dword.total_bytes_txfer = num;

    // internal use, not part of EHCI
    qTD->wanted_txfer_bytes = num;

    // set up data 4K address
    uint32_t data_addr = (uint32_t)data;

    // get buf 0 offset
    qTD->buf_ptr_dword[0].cur_offset = data_addr & 0xFFF;

    // make it as 4K page unit
    data_addr >>= 12;

    // whatever the txfer size is, we here fill up all buffer pointer fields
    for (int i = 0; i < 5; i++)
        qTD->buf_ptr_dword[i].ptr_address = data_addr + i;

    // then insert this Q_TD into pipe QH

    qh->overlay_next_qtd = (uint32_t)(qTD) & (~0x1F);
    qh->overlay_alternate_qtd = (uint32_t)(qTD) & (~0x1F);

    qh->all_txfered_bytes = 0; // FIXME: clear 0 here is ok ?

    qh->is_txfering = true; // internal use to mark it is doing transfer

    // set Activate bit, HC shall begin the transaction very soon
    qTD->status_dword.status = 0x80;

    return KDRV_STATUS_OK;
}

uint32_t kdrv_usbh2_pipe_transfer_get_result(kdrv_usbh2_pipe_t pipe_hndl)
{
    Queue_Head *qh = (Queue_Head *)pipe_hndl;

    uint32_t txfer_bytes = qh->all_txfered_bytes;

    usbh2_dbg("[usbh2] %s() bytes: %d\n", __FUNCTION__, txfer_bytes);
    return txfer_bytes;
}

kdrv_status_t kdrv_usbh2_pipe_transfer_abort(kdrv_usbh2_pipe_t pipe_hndl)
{
    // FIXME: this function implementation is not complete

    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    Queue_Head *qh = (Queue_Head *)pipe_hndl;

    if (qh->endpoint_type != USBH2_ENDPOINT_ISOCHRONOUS)
    {
        // handle QH and qTD
        if (qh->is_txfering) // then there is a running qTD
        {
            Q_TD *qTD = (Q_TD *)(qh->overlay_next_qtd & (~0x1F));

            // clear Active bit
            qTD->status_dword.status &= (~0x80);

            qh->is_txfering = false;
            qTD->in_use = false;
        }
    }
    else
    {
        // handle iTD
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbh2_pipe_reset(kdrv_usbh2_pipe_t pipe_hndl)
{
    // FIXME: do the same as kdrv_usbh2_pipe_transfer_abort() ?

    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    return kdrv_usbh2_pipe_transfer_abort(pipe_hndl);
}

kdrv_status_t kdrv_usbh2_pipe_delete(kdrv_usbh2_pipe_t pipe_hndl)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    Queue_Head *qh = (Queue_Head *)pipe_hndl;

    // report error if this pipe is still doing transfers or it is already out of schedule
    // user should invoke kdrv_usbh2_pipe_transfer_abort() before this
    if (qh->is_txfering || !qh->in_use)
        return KDRV_STATUS_ERROR;

    // first check the pipe (endpoint) is async or periodic
    if (qh->endpoint_type == USBH2_ENDPOINT_CONTROL || USBH2_ENDPOINT_BULK)
    {
        // async schedule

        // stop async schedule for safety, FIXME: really need this ?
        fotg210_ehci_USBCMD_schedule_set_enable(ASYNC_SCHEDULE, false);

        Queue_Head *qh_next = (Queue_Head *)(qh->next_qh.ptr_address << 5);

        if (qh_next == qh)
        {
            // in this case only it is the last QH in the scheudle
            // clear the ASYNCLISTADDR register
            UsbRegWrite(EHCI_ASYNC_LIST_ADDR, 0x0);
        }
        else
        {
            // remove it from schedule linked list
            Queue_Head *qh_prev = (Queue_Head *)(qh->prev_link_ptr);
            qh_prev->next_qh.ptr_address = ((uint32_t)qh_next) >> 5;
            qh_next->prev_link_ptr = (uint32_t)qh_prev;

            if (UsbRegRead(EHCI_ASYNC_LIST_ADDR) == (uint32_t)qh)
                UsbRegWrite(EHCI_ASYNC_LIST_ADDR, (uint32_t)qh_next);

            // re-enable the async schedule
            fotg210_ehci_USBCMD_schedule_set_enable(ASYNC_SCHEDULE, true);
        }

        qh->in_use = false;
    }
    else
    {
        // periodic scheudle
        // USBH2_ENDPOINT_ISOCHRONOUS or USBH2_ENDPOINT_INTERRUPT

        // stop periodic schedule for safety, FIXME: really need this ?
        fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, false);

        // FIXME: for now only one endpoint in the periodic schedule is allowed
        // FIXME: below way of handling is not enough

        // search all existence of this QH from periodic scheudle
        for (int i = 0; i < FRAME_LIST_SIZE; i++)
            periodic_frame_list[i] = 0x1;

        qh->in_use = false;
    }

    return KDRV_STATUS_OK;
}

uint16_t kdrv_usbh2_get_frame_number(void)
{
    usbh2_dbg("[usbh2] %s()\n", __FUNCTION__);

    return (UsbRegRead(EHCI_FRINDEX) & 0x1FFF >> 3);
}

// use this for Isochronous endpoint
// 'ep_addr' should also include direction bit
// 'ep_max_packet_mult' is directly from the 'wMaxPacketSize' of an Isoch endpoint descriptor
// NOTE: this driver supports only one isoch endpoint
kdrv_usbh2_pipe_t kdrv_usbh2_pipe_isoch_create(uint8_t dev_addr, uint8_t ep_addr,
                                               uint16_t max_packet_size, uint8_t mult, uint8_t ep_interval,
                                               uint8_t *buf, uint32_t buf_size)
{

    // set up first iTD
    I_TD *iTD_0 = (I_TD *)iTD_addr[0];

    // clear all iTDs
    memset(iTD_0, 0, sizeof(I_TD));

    iTD_0->next_link.terminate = 1;

    struct Buffer_Page_0_DWord *dev_dword = (struct Buffer_Page_0_DWord *)&(iTD_0->buf_page_dword[0]);
    dev_dword->dev_addr = dev_addr;
    dev_dword->endpoint = ep_addr & 0xF;

    struct Buffer_Page_1_DWord *maxpack_dword = (struct Buffer_Page_1_DWord *)&(iTD_0->buf_page_dword[1]);
    maxpack_dword->direction = !!(ep_addr & 0x80);
    maxpack_dword->max_packet_size = max_packet_size; // USB 2.0 spec Table 9-13 and EHCI 1.0 spec Table 3-5

    struct Buffer_Page_2_DWord *mult_dword = (struct Buffer_Page_2_DWord *)&(iTD_0->buf_page_dword[2]);
    mult_dword->mult = mult; // USB 2.0 spec Table 9-13 and EHCI 1.0 spec Table 3-6

    // calcualte intervals
    isoch_trans_interval = (1 << (ep_interval - 1));      // SOF interval, USB 2.0 spec Table 9-13
    isoch_iTD_interval = (isoch_trans_interval >> 4) + 1; // calculate interval of iTD, min is 1

    // calculate needed buffer size for a iTD
    uint8_t num_of_sof;
    switch (isoch_trans_interval)
    {
    case 1:
        num_of_sof = 8;
        isoch_IOC_pos = 7;
        break;
    case 2:
        num_of_sof = 4;
        isoch_IOC_pos = 6;
        break;
    case 4:
        num_of_sof = 2;
        isoch_IOC_pos = 4;
        break;
    default:
        num_of_sof = 1;
        isoch_IOC_pos = 4;
        break;
    }

    isoch_txfer_len = (max_packet_size * mult); // for example: 1020 x 3
    uint32_t buf_per_iTD = (num_of_sof * isoch_txfer_len);

    isoch_iTD_num = (buf_size / buf_per_iTD); // now we know how many iTDs needed

    if (isoch_iTD_num > iTD_MAX_NUM)
        isoch_iTD_num = iTD_MAX_NUM;

    int i;
    // let's use PG field to indicate if this transaction is needed according to isoch_trans_interval
    for (i = 0; i < 8; i++)
        iTD_0->trans_dword[i].pg = 0x7; // 0x7 to indicate NOT in-use

    for (i = 0; i < 8; i += isoch_trans_interval)
    {
        iTD_0->trans_dword[i].length = isoch_txfer_len;
        iTD_0->trans_dword[i].pg = 0;
        iTD_0->trans_dword[i].status = 0x8; // pre-activate it
    }
    // only 1 ioc in a iTD
    iTD_0->trans_dword[isoch_IOC_pos].ioc = 1; // set interrupt only for last transaction, interrtup per 1 ms

    // error checking: isoch_iTD_num must <= iTD_MAX_NUM

    uint32_t data_addr = (uint32_t)buf;

    // memory copy to all other iTDs
    for (i = 0; i < isoch_iTD_num; i++)
    {
        I_TD *iTD = (I_TD *)iTD_addr[i];
        memcpy(iTD, iTD_0, sizeof(I_TD));

        // layout memory
        iTD->trans_dword[0].offset = data_addr & 0xFFF;
        iTD->trans_dword[0].pg = 0;
        iTD->buf_page_dword[0].ptr_address = (data_addr & (~0xFFF)) >> 12;

        uint8_t cp = 0; // curent page looking at

        for (int t = 1; t < 8; t += isoch_trans_interval)
        {
            data_addr += isoch_txfer_len;

            iTD->trans_dword[t].offset = data_addr & 0xFFF;

            uint32_t buf_ptr = (data_addr & (~0xFFF)) >> 12;
            if (buf_ptr != iTD->buf_page_dword[cp].ptr_address)
            {
                cp++;
                iTD->buf_page_dword[cp].ptr_address = buf_ptr;
            }
            iTD->trans_dword[t].pg = cp;
        }

        // below is kind of workaround, give a plus page
        if (cp < 6)
            iTD->buf_page_dword[cp + 1].ptr_address = 1 + iTD->buf_page_dword[cp].ptr_address;

        data_addr += isoch_txfer_len;
    }

    usbh2_dbg("[usbh2] %s() iTD_0 0x%p num %d buf 0x%p\n", __FUNCTION__, iTD_0, isoch_iTD_num, buf);

    return 0x1000F; // just return a value to represent the handle
}

// FIXME, for now it has conflits with interrupt transfer
kdrv_status_t kdrv_usbh2_pipe_isoch_start(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_isoch_data_callback_t isoch_data_cb)
{
    // pipe_hndl is not used because it supports only one ISOCH endpoint

    fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, false);

    for (int i = 0; i < isoch_iTD_num; i++)
    {
        periodic_frame_list[i] = (uint32_t)iTD_addr[i]; // FIXME: should also take care of other iTD or QH here
    }

    isoch_next_process_pos = kdrv_usbh2_get_frame_number();

    fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, true);

    isoch_running_flag = 1; // FIXME

    // at least one of callback should not be NULL
    notify_isoch_data_cb = isoch_data_cb;

    return KDRV_STATUS_OK;
}

int32_t kdrv_usbh2_pipe_isoch_stop(kdrv_usbh2_pipe_t pipe_hndl)
{
    // pipe_hndl is not used because it supports only one ISOCH endpoint

    fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, false);

    // just clear the frame link list
    for (int i = 0; i < FRAME_LIST_SIZE; i++)
    {
        periodic_frame_list[i] = 0x1; // mark T-bit as 1 indicating invalid pointers
    }

    isoch_running_flag = 0;

    //  fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, true);
    return KDRV_STATUS_OK;
}

uint32_t usbh2_handle_iTD_work(void)
{
    uint32_t handle_count = 0;

    // scan some number of iTDs, FIXME: why 100
    for (int32_t r = 0; r < 100; r++)
    {

        I_TD *iTD = (I_TD *)periodic_frame_list[isoch_next_process_pos];

        // find the completed iTD
        if ((periodic_frame_list[isoch_next_process_pos] & 0x3) == 0x0 &&
            iTD->trans_dword[isoch_IOC_pos].ioc == 0x1 &&
            iTD->trans_dword[isoch_IOC_pos].status == 0x0)
        {

            for (int t = 0; t < 8; t += isoch_trans_interval)
            {
                if (iTD->trans_dword[t].status == 0x8)
                    continue;

                if (iTD->trans_dword[t].status != 0)
                {
                    usbh2_dbg("[usbh2] iTD %d trans %d error status = 0x%x\n", isoch_next_process_pos, t, iTD->trans_dword[t].status);
                }

                // callback to uppper layer middleware
                uint8_t pg = iTD->trans_dword[t].pg;
                uint32_t ptr_base = iTD->buf_page_dword[pg].ptr_address;
                uint32_t ptr_offset = iTD->trans_dword[t].offset;
                uint32_t *ptr_buf = (uint32_t *)((ptr_base << 12) | ptr_offset);

                notify_isoch_data_cb(ptr_buf, iTD->trans_dword[t].length);

                iTD->trans_dword[t].length = isoch_txfer_len;
                iTD->trans_dword[t].status = 0x8; // pre-activate it
            }

            // inser iTD in the back, FIXME: there should a better policy

            handle_count++;
        }

        isoch_next_process_pos += isoch_iTD_interval;
        isoch_next_process_pos &= FRAME_LIST_ROLL_MASK;
    }

    return handle_count;
}

// this function is optional, it is for registering bottom-half callback function
kdrv_usbh2_isoch_itd_work_func_t kdrv_usbh2_pipe_isoch_enable_bh(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_isoch_bf_callback_t isoch_bf_callback)
{
    // pipe_hndl is not used because it supports only one ISOCH endpoint

    isoch_process_itd_work = isoch_bf_callback;
    return usbh2_handle_iTD_work;
}

void kdrv_usbh2_pipe_isoch_set_pause(bool bPause)
{
    if (bPause)
        fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, false);
    else
        fotg210_ehci_USBCMD_schedule_set_enable(PERIODIC_SCHEDULE, true);
}
