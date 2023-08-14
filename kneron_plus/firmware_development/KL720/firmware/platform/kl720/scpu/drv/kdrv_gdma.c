/*
 * Kneron KL720 Generic DMA driver
 *
 * Copyright (C) 2020 Kneron, Inc. All rights reserved.
 *
 */

//#define GDMA_DBG

#include "kneron_kl720.h"
#include "kdrv_gdma.h"
#include <stdlib.h>
#include "kdrv_cmsis_core.h"

#define DMA_FIFO_SIZE 3 // 8 entries, from HW designer
#define NUM_DMA_CHANNELS 4
#define NUM_CH_FOR_MEMCPY 2
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

#define regGDMA ((U_regGDMA *)DMAC_AHB_REG_BASE)                 // global setting registers
#define regGDMA_ch ((U_regGDMA_CH *)(DMAC_AHB_REG_BASE + 0x100)) // per-channel registers in array form

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

typedef struct
{
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t llp;
    uint32_t control;
    uint32_t total_size;
} _GDMA_link_list_descriptor_t;

static uint8_t g_dma_inited = 0;
static _GDMA_CH_Ctrl_t *gDmaChCtrl = 0;

static gdma_transfer_width_t _get_width_from_align(uint8_t align_src, uint8_t align_dst)
{
    if ((align_src == 0) && (align_dst == 0))
        return GDMA_TXFER_WIDTH_32_BITS;
    else if ((align_src == 2) && (align_dst == 2))
        return GDMA_TXFER_WIDTH_16_BITS;
    else
        return GDMA_TXFER_WIDTH_8_BITS;
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
        printf("GDMA ISR: int_sts 0x%x, int_tc 0x%x, int_err_abt 0x%x\n", int_sts, int_tc, int_err_abt);
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

#ifdef GDMA_DBG
    if (handled == 0)
    {
        printf("GDMA: interrupt is NOT handled\n");
    }
#endif
}

kdrv_status_t kdrv_gdma_initialize(void)
{
    if (g_dma_inited)
        return KDRV_STATUS_ERROR;

    // global dma register initialization
    regGDMA->dw.CSR = DMACEN; // enable the DMA controller

    // allocate per-channel control blocks
    gDmaChCtrl = malloc(NUM_DMA_CHANNELS * sizeof(_GDMA_CH_Ctrl_t));

    // initialize per-channel dma register settings to default values
    for (int i = 0; i < NUM_DMA_CHANNELS; i++)
    {
        gDmaChCtrl[i].in_use = 0;
        gDmaChCtrl[i].running = 0;

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
        regGDMA_ch[i].bf.CSR_DMA_FF_TH = (DMA_FIFO_SIZE - 1); // threshold value = 4 due to our FIFO size is 8
        regGDMA_ch[i].bf.CSR_TC_MSK = 0;

        // CFG
        regGDMA_ch[i].bf.CFG_INT_TC_MSK = 0;  // enable terminal count interrupt
        regGDMA_ch[i].bf.CFG_INT_ERR_MSK = 0; // enable error interrupt
        regGDMA_ch[i].bf.CFG_INT_ABT_MSK = 0; // enable abort interrupt
        regGDMA_ch[i].bf.CFG_SRC_RS = 0;      // dma request, not use when in normal mode
        regGDMA_ch[i].bf.CFG_SRC_HE = 0;      // dma request, not use when in normal mode
        regGDMA_ch[i].bf.CFG_DST_RS = 0;      // dma request, not use when in normal mode
        regGDMA_ch[i].bf.CFG_DST_HE = 0;      // dma request, not use when in normal mode

        // others we use the reset default
    }

    NVIC_SetVector((IRQn_Type)DMA020_IRQn, (uint32_t)gdma_isr);
    NVIC_ClearPendingIRQ(DMA020_IRQn);
    NVIC_EnableIRQ(DMA020_IRQn);

    g_dma_inited = 1;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_uninitialize(void)
{
    if (!g_dma_inited)
        return KDRV_STATUS_ERROR;

    free(gDmaChCtrl);

    NVIC_DisableIRQ(DMA020_IRQn);

    g_dma_inited = 0;
    return KDRV_STATUS_OK;
}

kdrv_gdma_handle_t kdrv_gdma_acquire_handle(void)
{
    int ch;

    // try to find an unused dma channel for memcpy
    for (ch = NUM_CH_FOR_MEMCPY; ch < NUM_DMA_CHANNELS; ch++)
    {
        if (!gDmaChCtrl[ch].in_use)
        {
            gDmaChCtrl[ch].in_use = true; // claim it is in use now
            return ch;
        }
    }

    return -1;
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
        if (dma_setting->dma_dst_req != GDMA_HW_REQ_NONE)
        {
            regGDMA_ch[ch].bf.CFG_DST_HE = 1;
            regGDMA_ch[ch].bf.CFG_DST_RS = dma_setting->dma_dst_req;
        }
        else
            regGDMA_ch[ch].bf.CFG_DST_HE = 0;

        if (dma_setting->dma_src_req != GDMA_HW_REQ_NONE)
        {
            regGDMA_ch[ch].bf.CFG_SRC_HE = 1;
            regGDMA_ch[ch].bf.CFG_SRC_RS = dma_setting->dma_src_req;
        }
        else
            regGDMA_ch[ch].bf.CFG_SRC_HE = 0;

        // enable SYNC register
        regGDMA->dw.SYNC |= (0x1 << ch);
    }
    else
    {
        // disable SYNC register, FIXME: maybe it can always be enabled ?
        regGDMA->dw.SYNC &= ~(0x1 << ch);
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_release_handle(kdrv_gdma_handle_t handle)
{
    gDmaChCtrl[handle].in_use = 0;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_abort_transfer(kdrv_gdma_handle_t handle)
{
    int ch = handle;

    if (gDmaChCtrl[ch].running != true)
        return KDRV_STATUS_OK;

    regGDMA_ch[ch].bf.CSR_ABT = 1;
    return KDRV_STATUS_OK;
}

kdrv_gdma_copy_list_t kdrv_gdma_allocate_copy_list(kdrv_gdma_handle_t handle, uint32_t dst_addr[],
                                                   uint32_t src_addr[], uint32_t num_bytes[], uint32_t count)
{
    // malloc LLP for users, FIXME: use DDR ??
    void *llp_buf = malloc(sizeof(_GDMA_link_list_descriptor_t) * count);

    if (llp_buf == NULL)
    {
#ifdef GDMA_DBG
        printf("GDMA: %s() failed to allocal memory, needed size %d bytes\n", __FUNCTION__, sizeof(_GDMA_link_list_descriptor_t) * count);
#endif
        return NULL;
    }

    int ch = handle;
    uint32_t control = (regGDMA_ch[ch].bf.CSR_DMA_FF_TH << 29) |
                       (0x1 << 28) |
                       (regGDMA_ch[ch].bf.CSR_SRC_WIDTH << 25) |
                       (regGDMA_ch[ch].bf.CSR_DST_WIDTH << 22) |
                       (regGDMA_ch[ch].bf.CSR_SRCAD_CTL << 20) |
                       (regGDMA_ch[ch].bf.CSR_DSTAD_CTL << 18);

    uint32_t src_width_shift = regGDMA_ch[ch].bf.CSR_SRC_WIDTH;

    _GDMA_link_list_descriptor_t *llp_array = (_GDMA_link_list_descriptor_t *)llp_buf;

    for (uint32_t i = 0; i < count; i++)
    {
        llp_array[i].src_addr = src_addr[i];
        llp_array[i].dst_addr = dst_addr[i];
        llp_array[i].llp = (uint32_t)(&(llp_array[i + 1]));
        llp_array[i].control = control;
        llp_array[i].total_size = (num_bytes[i] >> src_width_shift);
    }
    llp_array[count - 1].llp = 0;
    llp_array[count - 1].control &= ~(0x1 << 28);

    return llp_buf;
}

kdrv_status_t kdrv_gdma_free_copy_list(kdrv_gdma_copy_list_t copy_list)
{
    free((void *)copy_list);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_transfer_copy_list(kdrv_gdma_handle_t handle, kdrv_gdma_copy_list_t copy_list, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg)
{
    int ch = handle;

    gDmaChCtrl[ch].running = true;

    // in case the DMA ch is still busy
    while (regGDMA->dw.CH_BUSY & (0x1 << ch))
        ;

    gDmaChCtrl[ch].user_cb = xfer_isr_cb;
    gDmaChCtrl[ch].user_arg = usr_arg;

    _GDMA_link_list_descriptor_t *llp_array = (_GDMA_link_list_descriptor_t *)copy_list;

    // llp_array[0] will be directly set into Cn_CSR
    regGDMA_ch[ch].dw.DstAddr = llp_array[0].dst_addr;
    regGDMA_ch[ch].dw.SrcAddr = llp_array[0].src_addr;
    regGDMA_ch[ch].dw.SIZE = llp_array[0].total_size;
    regGDMA_ch[ch].dw.LLP = (uint32_t)(&(llp_array[1]));
    regGDMA_ch[ch].bf.CSR_TC_MSK = 1; // the last one of LLP whould set this to '0'

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
            gDmaChCtrl[ch].running = false;
        }
        else
        {
            // Non-OS context
            do
            {
                __WFE();
            } while (gDmaChCtrl[ch].running == true);
        }

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

kdrv_status_t kdrv_gdma_transfer(kdrv_gdma_handle_t handle, uint32_t dst_addr, uint32_t src_addr,
                                 uint32_t num_bytes, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg)
{
    int ch = handle;
    uint32_t  len, w;

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
    w = 1<<regGDMA_ch[ch].bf.CSR_SRC_WIDTH;
    len = (num_bytes + w - 1)/w;
    regGDMA_ch[ch].dw.SIZE = len;//(num_bytes >> regGDMA_ch[ch].bf.CSR_SRC_WIDTH);

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

kdrv_status_t kdrv_gdma_memcpy(uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes,
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
    regGDMA_ch[ch].bf.CSR_DST_WIDTH = _get_width_from_align(src_addr & 0x3, dst_addr & 0x3);
    regGDMA_ch[ch].bf.CSR_SRC_WIDTH = regGDMA_ch[ch].bf.CSR_DST_WIDTH;

    kdrv_status_t ret = kdrv_gdma_transfer(ch, dst_addr, src_addr, num_bytes, xfer_isr_cb, usr_arg);
    kdrv_gdma_release_handle(ch);
    return ret;//kdrv_gdma_transfer(ch, dst_addr, src_addr, num_bytes, xfer_isr_cb, usr_arg);
}

kdrv_status_t kdrv_gdma_memcpy_cropping(gdma_cropping_descriptor_t *cropping_desc, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg)
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

    uint32_t start_src = cropping_desc->start_src_addr;
    uint32_t start_dst = cropping_desc->start_dst_addr;
    uint32_t src_offset = cropping_desc->src_offset;
    uint32_t dst_offset = cropping_desc->dst_offset;
    uint32_t txfer_bytes = cropping_desc->txfer_bytes;
    int count = cropping_desc->num_txfer;

    // malloc LLP, FIXME: use DDR ??
    void *llp_buf = malloc(sizeof(_GDMA_link_list_descriptor_t) * count);

    if (llp_buf == NULL)
    {
#ifdef GDMA_DBG
        printf("GDMA: %s() failed to allocal memory, needed size %d bytes\n", __FUNCTION__, sizeof(_GDMA_link_list_descriptor_t) * count);
#endif
        return KDRV_STATUS_OUT_OF_MEMORY;
    }

    // below src & dst width should vary with address alignment
    regGDMA_ch[ch].bf.CSR_DST_WIDTH = GDMA_TXFER_WIDTH_32_BITS;
    regGDMA_ch[ch].bf.CSR_SRC_WIDTH = GDMA_TXFER_WIDTH_32_BITS;

    uint32_t control = (regGDMA_ch[ch].bf.CSR_DMA_FF_TH << 29) |
                       (0x1 << 28) |
                       (regGDMA_ch[ch].bf.CSR_SRC_WIDTH << 25) |
                       (regGDMA_ch[ch].bf.CSR_DST_WIDTH << 22) |
                       (regGDMA_ch[ch].bf.CSR_SRCAD_CTL << 20) |
                       (regGDMA_ch[ch].bf.CSR_DSTAD_CTL << 18);

    uint32_t src_width_shift = regGDMA_ch[ch].bf.CSR_SRC_WIDTH;

    _GDMA_link_list_descriptor_t *llp_array = (_GDMA_link_list_descriptor_t *)llp_buf;

    for (uint32_t i = 0; i < count; i++)
    {
        llp_array[i].src_addr = start_src + i * src_offset;
        llp_array[i].dst_addr = start_dst + i * dst_offset;
        llp_array[i].llp = (uint32_t)(&(llp_array[i + 1]));
        llp_array[i].control = control;
        llp_array[i].total_size = (txfer_bytes >> src_width_shift);
    }
    llp_array[count - 1].llp = 0;
    llp_array[count - 1].control &= ~(0x1 << 28);

    kdrv_status_t status = kdrv_gdma_transfer_copy_list(ch, (kdrv_gdma_copy_list_t)llp_array, xfer_isr_cb, usr_arg);

    regGDMA_ch[ch].dw.LLP = 0;
    regGDMA_ch[ch].bf.CSR_TC_MSK = 0;

    free(llp_buf);

    return status;
}
