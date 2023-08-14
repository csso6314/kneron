/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
*
* The information contained herein is property of Kneron, Inc.
* Terms and conditions of usage are described in detail in Kneron
* STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information.
* NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
* from the file.
*/

/******************************************************************************
*  Filename:
*  ---------
*  kdrv_gdma.c
*
*  Project:
*  --------
*  KL520
*
*  Description:
*  ------------
*  This GDMA driver is for Generic Direct Memory Access
*  HW: Faraday FTDMAC020 (DMAC32), connect to AHB BUS 0 only, AHB BUS 1 is not in use
*
*  Author:
*  -------
*  Hans Yang
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

//#define GDMA_DBG
//#define GDMA_ERR
//#define GDMA_FEWER_CODE_SIZE

// Sec 0: Comment block of the file

// Sec 1: Include File

#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "kdrv_gdma.h"
#include <stdlib.h>

#if defined(GDMA_DBG) | defined(GDMA_ERR)
#include "kmdw_console.h"
#endif

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

// some fixed numbers
#define DMA_FIFO_SIZE 4 // 16 entries, from HW designer
#define NUM_DMA_CHANNELS 8
#define NUM_CH_FOR_MEMCPY 3
#define DMA_CHANNEL_BIT_MASK ((0x1 << NUM_DMA_CHANNELS) - 1)

// register bit : DMA enable
#define DMACEN 0x1

// thread flag for transfer done
#define GDMA_FLAG_XFER_DONE 0x100

typedef volatile union {
    struct
    {
        uint32_t INT;             // 0x00
        uint32_t INT_TC;          // 0x04
        uint32_t INT_TC_CLR;      // 0x08
        uint32_t INT_ERR_ABT;     // 0x0C
        uint32_t INT_ERR_ABT_CLR; // 0x10
        uint32_t TC;              // 0x14
        uint32_t ERR_ABT;         // 0x18
        uint32_t CH_EN;           // 0x1C
        uint32_t CH_BUSY;         // 0x20
        uint32_t CSR;             // 0x24
        uint32_t SYNC;            // 0x28
        uint32_t DMAC_REVISION;   // 0x30
        uint32_t DMAC_FEATURE;    // 0x34
    } dw;                         //double word
} U_regGDMA;

typedef volatile union {
    struct
    {
        uint32_t CSR;         // 0x100 + 0x20*n
        uint32_t CFG;         // 0x104 + 0x20*n
        uint32_t SrcAddr;     // 0x108 + 0x20*n
        uint32_t DstAddr;     // 0x10C + 0x20*n
        uint32_t LLP;         // 0x110 + 0x20*n
        uint32_t SIZE;        // 0x114 + 0x20*n
        uint32_t reserved[2]; // 0x118 + 0x20*n
    } dw;                     //double word

    struct
    {
        // CSR
        uint32_t CSR_CH_EN : 1;     // channel enable
        uint32_t CSR_DST_SEL : 1;   // dst select AHB 0/1
        uint32_t CSR_SRC_SEL : 1;   // src select AHB 0/1
        uint32_t CSR_DSTAD_CTL : 2; // dst address control: inc/dec/fixed
        uint32_t CSR_SRCAD_CTL : 2; // src address control: inc/dec/fixed
        uint32_t CSR_MODE : 1;      // normal/HW handshake mode
        uint32_t CSR_DST_WIDTH : 3; // dst transfer width, use gdma_transfer_width_t
        uint32_t CSR_SRC_WIDTH : 3; // src transfer width, use gdma_transfer_width_t
        uint32_t CSR_reserved0 : 1; //
        uint32_t CSR_ABT : 1;       // abort current transaction
        uint32_t CSR_SRC_SIZE : 3;  // src busrt size, use gdma_burst_size_t
        uint32_t CSR_PROT1 : 1;     // we dont use it
        uint32_t CSR_PROT2 : 1;     // we dont use it
        uint32_t CSR_PROT3 : 1;     // we dont use it
        uint32_t CSR_PRIORITY : 2;  // priority ?
        //uint32_t CSR_reserved1 : 2; //
        uint32_t CSR_DMA_FF_TH : 3; // DMA FIFO threshold value, fix it to '3'(8) as our FIFO size is 16
        uint32_t CSR_reserved2 : 4; //
        uint32_t CSR_TC_MSK : 1;    // terminal count status mask

        // CFG
        uint32_t CFG_INT_TC_MSK : 1;  // channel terminal count interrupt mask
        uint32_t CFG_INT_ERR_MSK : 1; // channel error interrupt mask
        uint32_t CFG_INT_ABT_MSK : 1; // channel abort interrupt mask
        uint32_t CFG_SRC_RS : 4;      // src DMA request select (for HW handshake mode)
        uint32_t CFG_SRC_HE : 1;      // src hardware handshake mode enable (for HW handshake mode)
        uint32_t CFG_BUSY : 1;        // DMA channel is busy
        uint32_t CFG_DST_RS : 4;      // dst DMA request select (for HW handshake mode)
        uint32_t CFG_DST_HE : 1;      // dst hardware handshake mode enable (for HW handshake mode)
        uint32_t CFG_reserved1 : 2;   //
        uint32_t CFG_LLP_CNT : 4;     // chain transfer counter
        uint32_t CFG_reserved2 : 12;  //
    } bf;                             //bit-field
} U_regGDMA_CH;

#define regGDMA ((U_regGDMA *)DMAC_FTDMAC020_PA_BASE)                 // global setting registers
#define regGDMA_ch ((U_regGDMA_CH *)(DMAC_FTDMAC020_PA_BASE + 0x100)) // per-channel registers in array form

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list

// this struct is for internal use only
typedef struct
{
    gdma_setting_t settings;
    uint8_t in_use;
    uint8_t running;
    osThreadId_t user_tid;        // user thread ID, when 0 means non-os context
    gdma_xfer_callback_t user_cb; // user callback
    void *user_arg;               // user argument
    uint32_t src_end;             // workaround for ending content
    uint32_t dst_end;             // workaround for ending content
    kdrv_status_t ret_sts;
} _GDMA_CH_Ctrl_t;

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable

static uint8_t g_dma_inited = 0;
static _GDMA_CH_Ctrl_t *gDmaChCtrl = 0;

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/

static gdma_transfer_width_t _get_width_from_align(uint8_t align)
{
    if (align == 0)
        return GDMA_TXFER_WIDTH_32_BITS;
    else if (align == 2)
        return GDMA_TXFER_WIDTH_16_BITS;
    else
        return GDMA_TXFER_WIDTH_8_BITS;
}

static void _fill_up_last_few_bytes(int ch)
{
    // doing this is because the GDMA does not complete the last few bytes in some cases (address/num is not divisiable by 4)
    for (int i = 0; i < 3; i++)
        *((uint8_t *)(gDmaChCtrl[ch].dst_end - i)) = *((uint8_t *)(gDmaChCtrl[ch].src_end - i));
}

static void gdma_isr(void)
{
    uint32_t int_sts = regGDMA->dw.INT;
    uint32_t handled = 0;

    if (int_sts & DMA_CHANNEL_BIT_MASK) // check INT register low 8 bits (8 channels)
    {
        uint32_t int_tc = regGDMA->dw.INT_TC;
        uint32_t int_err_abt = regGDMA->dw.INT_ERR_ABT;

        // clear all interrupt status as early as possible
        regGDMA->dw.INT_TC_CLR = int_tc;
        regGDMA->dw.INT_ERR_ABT_CLR = int_err_abt;

#ifdef GDMA_DBG
        kmdw_printf("GDMA ISR: int_sts 0x%x, int_tc 0x%x, int_err_abt 0x%x\n", int_sts, int_tc, int_err_abt);
#endif

        uint32_t int_abt = (int_err_abt >> 16);

        // scanl all channels
        for (int ch = 0; ch < NUM_DMA_CHANNELS; ch++)
        {
            int32_t xferStatus = -100;

            if (int_tc & (0x1 << ch))
            {
                xferStatus = KDRV_STATUS_OK;
            }
            else if ((int_err_abt & (0x1 << ch)) || (int_abt & (0x1 << ch)))
            {
                xferStatus = KDRV_STATUS_ERROR;
            }

            if (xferStatus == -100) // no status change for this ch
                continue;

            if (gDmaChCtrl[ch].user_cb)
            {
                _fill_up_last_few_bytes(ch);
                gDmaChCtrl[ch].running = 0;
                gDmaChCtrl[ch].user_cb((kdrv_status_t)xferStatus, gDmaChCtrl[ch].user_arg); // NOTE: this callback is from ISR context
            }
            else
            {
                gDmaChCtrl[ch].ret_sts = (kdrv_status_t)xferStatus;

                if (gDmaChCtrl[ch].user_tid)
                    osThreadFlagsSet(gDmaChCtrl[ch].user_tid, GDMA_FLAG_XFER_DONE);
                else
                    gDmaChCtrl[ch].running = 0;
            }
        }

        ++handled;
    }

#ifdef GDMA_ERR
    if (handled == 0)
    {
        kmdw_printf("GDMA: interrupt is handled\n");
    }
#endif
}

kdrv_status_t kdrv_gdma_initialize(void)
{
    if (g_dma_inited)
        return KDRV_STATUS_ERROR;

    // FIXME: enable DMAC0 clock (hclk_en[3]) with better API/Macro
    {
        uint32_t ahb_clock_reg = (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x50));
        ahb_clock_reg |= 0x8;
        (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x50)) = ahb_clock_reg;
    }

    // global dma register initialization
    {
        regGDMA->dw.CSR = DMACEN; // enable the DMA controller
    }

    // allocate per-channel control blocks
    gDmaChCtrl = malloc(NUM_DMA_CHANNELS * sizeof(_GDMA_CH_Ctrl_t));

    // initialize per-channel dma register settings to default values
    for (int i = 0; i < NUM_DMA_CHANNELS; i++)
    {
        gDmaChCtrl[i].in_use = 0;
        gDmaChCtrl[i].running = 0;

#ifdef GDMA_FEWER_CODE_SIZE
        regGDMA_ch[i].dw.CSR = 0x3001200;
        regGDMA_ch[i].dw.CFG = 0x0;
#else
        // CSR
        regGDMA_ch[i].bf.CSR_CH_EN = 0;   // disabled at initialization time
        regGDMA_ch[i].bf.CSR_DST_SEL = 0; // AHB 0
        regGDMA_ch[i].bf.CSR_SRC_SEL = 0; // AHB 0
        regGDMA_ch[i].bf.CSR_DSTAD_CTL = GDMA_INCREMENT_ADDRESS;
        regGDMA_ch[i].bf.CSR_SRCAD_CTL = GDMA_INCREMENT_ADDRESS;
        regGDMA_ch[i].bf.CSR_MODE = GDMA_NORMAL_MODE;
        regGDMA_ch[i].bf.CSR_DST_WIDTH = GDMA_TXFER_WIDTH_32_BITS;
        regGDMA_ch[i].bf.CSR_SRC_WIDTH = GDMA_TXFER_WIDTH_32_BITS;
        regGDMA_ch[i].bf.CSR_ABT = 0;
        regGDMA_ch[i].bf.CSR_SRC_SIZE = GDMA_BURST_SIZE_16;
        //regGDMA_ch[i].bf.CSR_PRIORITY = 0;
        regGDMA_ch[i].bf.CSR_DMA_FF_TH = (DMA_FIFO_SIZE - 1); // threshold value = 8 due to our FIFO size is 16
        regGDMA_ch[i].bf.CSR_TC_MSK = 0;

        // CFG
        regGDMA_ch[i].bf.CFG_INT_TC_MSK = 0;  // enable terminal count interrupt
        regGDMA_ch[i].bf.CFG_INT_ERR_MSK = 0; // enable error interrupt
        regGDMA_ch[i].bf.CFG_INT_ABT_MSK = 0; // enable abort interrupt
        regGDMA_ch[i].bf.CFG_SRC_RS = 0;      // dma request, not use when in normal mode
        regGDMA_ch[i].bf.CFG_SRC_HE = 0;      // dma request, not use when in normal mode
        regGDMA_ch[i].bf.CFG_DST_RS = 0;      // dma request, not use when in normal mode
        regGDMA_ch[i].bf.CFG_DST_HE = 0;      // dma request, not use when in normal mode
#endif

        // others we use the reset default
    }

    NVIC_SetVector((IRQn_Type)DMA_FTDMAC020_0_IRQ, (uint32_t)gdma_isr);
    NVIC_ClearPendingIRQ(DMA_FTDMAC020_0_IRQ);
    NVIC_EnableIRQ(DMA_FTDMAC020_0_IRQ);

    g_dma_inited = 1;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_uninitialize(void)
{
    if (!g_dma_inited)
        return KDRV_STATUS_ERROR;

    // FIXME: disable DMAC0 clock (hclk_en[3]) with better API/Macro
    {
        uint32_t ahb_clock_reg = (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x50));
        ahb_clock_reg &= ~0x8;
        (*(volatile unsigned int *)(SCU_FTSCU100_PA_BASE + 0x50)) = ahb_clock_reg;
    }

    free(gDmaChCtrl);

    NVIC_DisableIRQ(DMA_FTDMAC020_0_IRQ);

    g_dma_inited = 0;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_acquire_handle(kdrv_gdma_handle_t *handle)
{
    int ch = 0;
    // try to find an unused dma channel for memcpy
    for (ch = NUM_CH_FOR_MEMCPY; ch < NUM_DMA_CHANNELS; ch++)
    {
        if (!gDmaChCtrl[ch].in_use)
        {
            gDmaChCtrl[ch].in_use = 1; // claim it is in use now
            *handle = ch;
            return KDRV_STATUS_OK;
        }
    }

    return KDRV_STATUS_ERROR;
}

kdrv_status_t kdrv_gdma_configure_setting(kdrv_gdma_handle_t handle, gdma_setting_t *dma_setting)
{
    int ch = handle;

    regGDMA_ch[ch].bf.CSR_DST_WIDTH = dma_setting->dst_width;
    regGDMA_ch[ch].bf.CSR_SRC_WIDTH = dma_setting->src_width;
    regGDMA_ch[ch].bf.CSR_SRC_SIZE = dma_setting->burst_size;
    regGDMA_ch[ch].bf.CSR_DSTAD_CTL = dma_setting->dst_addr_ctrl;
    regGDMA_ch[ch].bf.CSR_SRCAD_CTL = dma_setting->src_addr_ctrl;
    regGDMA_ch[ch].bf.CSR_MODE = dma_setting->dma_mode;

    if (dma_setting->dma_mode == GDMA_HW_HANDSHAKE_MODE)
    {
        regGDMA_ch[ch].bf.CFG_DST_RS = dma_setting->dma_dst_req;
        regGDMA_ch[ch].bf.CFG_DST_HE = (dma_setting->dma_dst_req != 0) ? 1 : 0;

        regGDMA_ch[ch].bf.CFG_SRC_RS = dma_setting->dma_src_req;
        regGDMA_ch[ch].bf.CFG_SRC_HE = (dma_setting->dma_src_req != 0) ? 1 : 0;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_release_handle(kdrv_gdma_handle_t handle)
{
    gDmaChCtrl[handle].in_use = 0;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_transfer_async(kdrv_gdma_handle_t handle, uint32_t dst_addr, uint32_t src_addr,
                                       uint32_t num_bytes, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg)
{
    int ch = handle;

    gDmaChCtrl[ch].running = 1;

    // in case the DMA ch is still busy
    while (regGDMA->dw.CH_BUSY & (0x1 << ch))
        ;

    gDmaChCtrl[ch].user_cb = xfer_isr_cb;
    gDmaChCtrl[ch].user_arg = usr_arg;
    gDmaChCtrl[ch].src_end = src_addr + num_bytes - 1;
    gDmaChCtrl[ch].dst_end = dst_addr + num_bytes - 1;

    // for memcpy we dont have to set up all register setting because many are done at initialization
    regGDMA_ch[ch].dw.DstAddr = dst_addr;
    regGDMA_ch[ch].dw.SrcAddr = src_addr;
    regGDMA_ch[ch].dw.SIZE = (num_bytes >> regGDMA_ch[ch].bf.CSR_SRC_WIDTH);

    kdrv_status_t ret;

    if (xfer_isr_cb == 0)
    {
        // check execution context
        gDmaChCtrl[ch].user_tid = osThreadGetId();

        // trigger the DMA channel
        regGDMA_ch[ch].bf.CSR_CH_EN = 1;

        if (gDmaChCtrl[ch].user_tid != 0)
        {
            // OS thread context
            // blocking here until transfer is done
            uint32_t flags = osThreadFlagsWait(GDMA_FLAG_XFER_DONE, osFlagsWaitAny, osWaitForever);
            gDmaChCtrl[ch].running = 0;
        }
        else
        {
            // Non-OS context
            do
            {
                __WFE();
            } while (gDmaChCtrl[ch].running == 1);
        }

        _fill_up_last_few_bytes(ch); // workaround last few bytes if needed

        ret = gDmaChCtrl[ch].ret_sts;
    }
    else
    {
        // trigger the DMA channel
        regGDMA_ch[ch].bf.CSR_CH_EN = 1;
        ret = KDRV_STATUS_OK;
    }

    return ret;
}

kdrv_status_t kdrv_gdma_transfer(kdrv_gdma_handle_t handle,
                                 uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes)
{
    return kdrv_gdma_transfer_async(handle, dst_addr, src_addr, num_bytes, NULL, NULL);
}

kdrv_status_t kdrv_gdma_memcpy_async(uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes,
                                     gdma_xfer_callback_t xfer_isr_cb, void *usr_arg)
{
    int ch = 0;
    // try to find an unused dma channel for memcpy
    for (ch = 0; ch < NUM_CH_FOR_MEMCPY; ch++)
    {
        if (!gDmaChCtrl[ch].running)
        {
            gDmaChCtrl[ch].running = 1;
            break;
        }
    }

    // at present no available dma channel
    if (ch >= NUM_CH_FOR_MEMCPY)
        return KDRV_STATUS_GDMA_ERROR_NO_RESOURCE;

    // below src & dst width should vary with address alignment
    regGDMA_ch[ch].bf.CSR_DST_WIDTH = _get_width_from_align(dst_addr & 0x3);
    regGDMA_ch[ch].bf.CSR_SRC_WIDTH = _get_width_from_align(src_addr & 0x3);

    return kdrv_gdma_transfer_async(ch, dst_addr, src_addr, num_bytes, xfer_isr_cb, usr_arg);
}

kdrv_status_t kdrv_gdma_memcpy(uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes)
{
    return kdrv_gdma_memcpy_async(dst_addr, src_addr, num_bytes, NULL, NULL);
}
