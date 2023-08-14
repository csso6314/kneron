/*
 * Kneron KL720 NCPU DMA driver
 *
 * Copyright (C) 2020 Kneron, Inc. All rights reserved.
 *
 * LDM (Local Descriptor Memroy access is not supported in DSP, so  DMA chain mode is not supported
 * all Link List related functions/structs are not defined due to the LDM not-accessible limitation.
 * only normal DMA mode is supported in this driver
 *
 */

//#define GDMA_DBG

#include "kneron_kl720.h"
#include "kdrv_gdma3.h"
#include <stdlib.h>
#include <string.h>
#include "kdrv_cmsis_core.h"

#define DMA_FIFO_SIZE 4 // 16 entries, from HW designer
#define NUM_DMA_CHANNELS 8
#define NUM_CH_FOR_MEMCPY 4
#define DMA_CHANNEL_BIT_MASK ((0x1 << NUM_DMA_CHANNELS) - 1)

// thread flag for transfer done
#define GDMA_FLAG_XFER_DONE 0x00000100
//#define USE_EVENT_FLAG

typedef volatile struct {
    uint32_t INT;                   //0x00
    uint32_t INT_TC;                //0x04
    uint32_t INT_TC_CLR;            //0x08
    uint32_t INT_ERR_ABT;           //0x0C
    uint32_t INT_ERR_ABT_CLR;       //0x10
    uint32_t TC_STS;                //0x14
    uint32_t ERR_ABT_STS;           //0x18
    uint32_t CH_EN_STS;             //0x1C
    uint32_t SPI;                   //0x20
    uint32_t LDM_BASE;              //0x24
    uint32_t WDT;                   //0x28
    uint32_t GER;                   //0x2C
    uint32_t PSE;                   //0x30
    uint32_t REV;                   //0x34
    uint32_t FEATURE1;              //0x38
    uint32_t LDM_FLAG0;             //0x3C
    uint32_t LDM_FLAG1;             //0x40
    uint32_t LDM_FLAG2;             //0x44
    uint32_t LDM_FLAG3;             //0x48
    uint32_t BIG_ENDIAN;            //0x4C
    uint32_t WR_ONLY_VAL;           //0x50
    uint32_t FEATURE2;              //0x54
    uint32_t dummy1[42];
} dma_reg_head_t;

typedef volatile struct {
    uint32_t  ChWEvent:8;           //[7:0] Chanel wait event
    uint32_t  WSync:1;              //[8]  Wait last Write done
    uint32_t  ChsEvent:3;           //[11:9] Channel set event
    uint32_t  SEventEn:1;           //[12] Set Event Enable
    uint32_t  WEventEn:1;           //[13] Wait event enable
    uint32_t  En2D:1;               //[14] 2D tansfer enable
    uint32_t  ExpEn:1;              //[15] Expand control enable
    uint32_t  ChEn:1;               //[16] Channel enable
    uint32_t  WDTEn:1;              //[17] watchdog enable
    uint32_t  DstCtrl:2;            //[19:18] destination burset type ctrl
    uint32_t  SrcCtrl:2;            //[21:20] Srouce burst type control
    uint32_t  DstWidth:3;           //[24:22] Destination data width
    uint32_t  SrcWidth:3;           //[27:25] Source data width
    uint32_t  TCMsk:1;              //[28] Teminal count mask
    uint32_t  SrcTcnt:3;            //[31:29] Source transfer count of one DMA handshake operation
} DMA_CH_CSR_t;

typedef volatile struct {
    uint32_t INT_TC_MSK : 1;        //[0] channel terminal count interrupt mask
    uint32_t INT_ERR_MSK : 1;       //[1] channel error interrupt mask
    uint32_t INT_ABT_MSK : 1;       //[2] channel abort interrupt mask
    uint32_t SRC_RS : 4;            //[6:3] source handshake request source select (for HW handshake mode)
    uint32_t SRC_HE : 1;            //[7] src hardware handshake enable (for HW handshake mode)
    uint32_t REV1:1;                //[8] Reserved
    uint32_t DST_RS : 4;            //[12:9] destination handshake request source select (for HW handshake mode)
    uint32_t DST_HE : 1;            //[13] dst hardware handshake enable (for HW handshake mode)
    uint32_t REV2:2;                //[15:14] Reserved
    uint32_t LLP_CNT : 4;           //[19:16] Link list pointer increase counter
    uint32_t ChGntWin:8;            //[27:20] Channel arbitration grant window
    uint32_t ChPri:1;               //[28] Channel arbitration priority
    uint32_t REV3:1;                //[29] Reserved
    uint32_t WOMode:1;              //[30] Write only mode
    uint32_t UnAlignMode:1;         //[31] Unalign transfer mode
} DMA_CH_CFG_t;

typedef volatile union {
    struct {
        uint32_t CSR;               //0x100 + 0x20*n
        uint32_t CFG;               //0x104 + 0x20*n
        uint32_t SrcAddr;           //0x108 + 0x20*n
        uint32_t DstAddr;           //0x10C + 0x20*n
        uint32_t LLP;               //0x110 + 0x20*n
        uint32_t SIZE;              //0x114 + 0x20*n
        uint32_t SSDA;              //0x118 + 0x20*n
        uint32_t DUMMY;             //0x11C + 0x20*n
    } dw;                           //double word

    struct {
        DMA_CH_CSR_t csr_bits;
        DMA_CH_CFG_t cfg_bits;
    } bf;                           //bit-field
} dma_channel_reg_t;

// feature register
typedef struct {
    uint32_t ch_num     :3;
    uint32_t reserved3  :1;
    uint32_t d_width    :2;
    uint32_t reserved6  :2;
    uint32_t dfifo_depth:3;
    uint32_t reserved11 :1;
    uint32_t pri_on     :1;
    uint32_t reserved13 :3;
    uint32_t pri_num    :4;
    uint32_t ldm_on     :1;
    uint32_t reserved21 :3;
    uint32_t ldm_depth  :2;
    uint32_t reserved26 :2;
    uint32_t cmd_depth  :2;
    uint32_t reserved30 :2;
} DMA_FEATURE_t;

typedef volatile struct {
    dma_reg_head_t      dma_global;
    dma_channel_reg_t   dma_ch[8];
} dma_reg_mem_map_t;

#define pDMA_Register ((dma_reg_mem_map_t *)DMAC_CRS_REG_BASE)                 // global setting registers

// this struct is for internal use only
typedef struct {
    gdma_setting_t settings;
    uint8_t in_use;
    uint8_t running;
    #ifdef USE_EVENT_FLAG
    volatile osEventFlagsId_t client_evt;
    #else
    osThreadId_t client_evt;        // user thread ID, when 0 means non-os context
    #endif
    
    gdma_xfer_callback_t user_cb; // user callback
    void *user_arg;               // user argument
    uint32_t src_end;             // workaround for ending content
    uint32_t dst_end;             // workaround for ending content
    kdrv_status_t ret_sts;
} _GDMA_CH_Ctrl_t;

static uint8_t g_dma_inited = 0;
static _GDMA_CH_Ctrl_t gDmaChCtrl[NUM_DMA_CHANNELS];

static gdma_transfer_width_t _get_width_from_align(uint8_t align_src, uint8_t align_dst)
{
    if ((align_src == 0) && (align_dst == 0))
        return GDMA_TXFER_WIDTH_32_BITS;
    else if ((align_src == 2) && (align_dst == 2))
        return GDMA_TXFER_WIDTH_16_BITS;
    else
        return GDMA_TXFER_WIDTH_8_BITS;
}

int IsDMAChannelEnable(int Channel)
{
    return ((pDMA_Register->dma_global.CH_EN_STS >> Channel) & 0x1);
}

static uint32_t GetDMAIntStatus(void)
{
    return pDMA_Register->dma_global.INT;
}

static uint32_t GetDMAChannelIntStatus(int Channel)
{
    uint32_t IntStatus = 0;
    uint32_t int_tc = pDMA_Register->dma_global.INT_TC;
    uint32_t int_err_abt = pDMA_Register->dma_global.INT_ERR_ABT;
    uint32_t int_abt = (int_err_abt >> 16);

    if((pDMA_Register->dma_global.INT >> Channel) & 0x01) {
        if( int_tc & (0x1 << Channel))
            IntStatus |= 1;
        if((int_err_abt & (0x1 << Channel)) || (int_abt & (0x1 << Channel)))
            IntStatus |= 2;
    }

    return IntStatus;
}

static void EnableDMAChannel(int Channel)
{
    pDMA_Register->dma_ch[Channel].bf.csr_bits.ChEn = 1;
}

static void EnableDMAEndian()
{
    pDMA_Register->dma_global.BIG_ENDIAN = 1<<16;
}

void EnableDMAChannelUnalign(int Channel)
{
    pDMA_Register->dma_ch[Channel].bf.cfg_bits.UnAlignMode = 1;
}

static void DisableDMAChannel(int Channel)
{
    pDMA_Register->dma_ch[Channel].dw.CSR = 0;
}

static void ClearDMAChannelIntStatus(int Channel)
{
    pDMA_Register->dma_global.INT_TC_CLR = 1 << Channel;
    pDMA_Register->dma_global.INT_ERR_ABT_CLR = 1 << Channel;
}

void SetDMAChCsr(int Channel, DMA_CH_CSR_t csr)
{
    pDMA_Register->dma_ch[Channel].dw.CSR = *((uint32_t *)&csr);
}

void SetDMAChCfg(int Channel, DMA_CH_CFG_t cfg)
{
    pDMA_Register->dma_ch[Channel].dw.CFG= *((uint32_t *)&cfg);
}

static void SetDmaChIntMask(int Channel, DMA_CH_CFG_t Mask)
{
    pDMA_Register->dma_ch[Channel].bf.cfg_bits.INT_TC_MSK = Mask.INT_TC_MSK;
    pDMA_Register->dma_ch[Channel].bf.cfg_bits.INT_ERR_MSK = Mask.INT_ERR_MSK;
    pDMA_Register->dma_ch[Channel].bf.cfg_bits.INT_ABT_MSK = Mask.INT_ABT_MSK;
}

static void DMA_CHDataCtrl(int Channel, uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size)
{
    pDMA_Register->dma_ch[Channel].dw.SrcAddr = SrcAddr;
    pDMA_Register->dma_ch[Channel].dw.DstAddr = DstAddr;
    pDMA_Register->dma_ch[Channel].dw.SIZE = Size;
}

static void DMA_SetInterrupt(uint32_t channel, uint32_t tcintr, uint32_t errintr, uint32_t abtintr)
{
    DMA_CH_CFG_t cfg;

    if(tcintr)
        cfg.INT_TC_MSK = 0;  //Enable terminal count interrupt
    else
        cfg.INT_TC_MSK = 1;  //Disable terminal count interrupt

    if(errintr)
        cfg.INT_ERR_MSK = 0; //Enable error interrupt
    else
        cfg.INT_ERR_MSK = 1; //Disable error interrupt

    if(abtintr)
        cfg.INT_ABT_MSK = 0; //Enable abort interrupt
    else
        cfg.INT_ABT_MSK = 1; //Disable abort interrupt

    SetDmaChIntMask(channel, cfg);
}

static void DMA_ResetChannel(int ch)
{
    pDMA_Register->dma_ch[ch].dw.CSR = 0;
    pDMA_Register->dma_ch[ch].dw.CFG = 7;    				//mask TC/Err/Abt interrupt
    pDMA_Register->dma_ch[ch].dw.SrcAddr = 0;
    pDMA_Register->dma_ch[ch].dw.DstAddr = 0;
    pDMA_Register->dma_ch[ch].dw.LLP = 0;
    pDMA_Register->dma_ch[ch].dw.SIZE = 0;
}

static void DMA_ClearAllInterrupt(void)
{
    pDMA_Register->dma_global.INT_TC_CLR = 0xff;
    pDMA_Register->dma_global.INT_ERR_ABT_CLR = 0xFF00FF;
}

static uint32_t DMA_isUnalignSupported(void)
{
    uint32_t  tmp = pDMA_Register->dma_global.FEATURE1 & 0x08;

    if(tmp)
        return 1;
    else
        return 0;
}

int DMA_GetAXIBusWidth()
{
    uint32_t AxiWidth;
    DMA_FEATURE_t *Feature;

    Feature = (DMA_FEATURE_t *)  &pDMA_Register->dma_global.FEATURE1;

    if((Feature->d_width) >2)
    {
#ifdef GDMA_DBG
        printf("AXI data bus width greater than 128 bits doesn't support\n");
#endif
    }

    AxiWidth=(Feature->d_width);
    return   AxiWidth;
}

static void gdma_isr(void)
{
    uint32_t handled = 0;
    uint32_t ch_int;
    int32_t xferStatus;

    uint32_t int_sts = GetDMAIntStatus();

    if (int_sts & DMA_CHANNEL_BIT_MASK) {
        // find the interrupt source
        for (int ch = NUM_CH_FOR_MEMCPY; ch < NUM_DMA_CHANNELS; ch++) {
            xferStatus = -100;
            ch_int = GetDMAChannelIntStatus(ch);
            ClearDMAChannelIntStatus(ch);

            if (ch_int & 0x01) {
                xferStatus = KDRV_STATUS_OK;
            } else if (ch_int & 0x02) {
                xferStatus = KDRV_STATUS_ERROR;
            }

            if (xferStatus == -100) // no status change for this ch
                continue;

            if (gDmaChCtrl[ch].user_cb) {
                gDmaChCtrl[ch].running = 0;
                gDmaChCtrl[ch].user_cb((kdrv_status_t)xferStatus, gDmaChCtrl[ch].user_arg); // NOTE: this callback is from ISR context
            } else {
                gDmaChCtrl[ch].ret_sts = (kdrv_status_t)xferStatus;
                if (gDmaChCtrl[ch].client_evt){
                    #ifdef USE_EVENT_FLAG
                    osEventFlagsSet(gDmaChCtrl[ch].client_evt, GDMA_FLAG_XFER_DONE);
                    #else
                    osThreadFlagsSet(gDmaChCtrl[ch].client_evt, GDMA_FLAG_XFER_DONE);
                    #endif
                } 
                else
                    gDmaChCtrl[ch].running = 0;
            }

            DisableDMAChannel(ch);
        }

        ++handled;
    }

#ifdef GDMA_DBG
    if (handled == 0) {
        printf("GDMA: interrupt is NOT handled\n");
    }
#endif
}

int GetDMAEnableStatus(void)
{
    return pDMA_Register->dma_global.CH_EN_STS;
}

void EnableDMAChannelWriteOnly(int Channel)
{
    pDMA_Register->dma_ch[Channel].bf.cfg_bits.WOMode = 1;
}

kdrv_status_t kdrv_gdma_initialize(void)
{
    uint32_t ret;

    if (g_dma_inited)
        return KDRV_STATUS_ERROR;


    ret = DMA_isUnalignSupported();
    if(!ret) {
#ifdef GDMA_DBG
        printf("Unalign mode shall be supported in DSP due to LDM mode is not supported \n");
#endif
        return KDRV_STATUS_ERROR;
    }

    EnableDMAEndian();

    // initialize per-channel dma register settings to default values
    for (int i = NUM_CH_FOR_MEMCPY; i < NUM_DMA_CHANNELS; i++) {
        gDmaChCtrl[i].in_use = 0;
        gDmaChCtrl[i].running = 0;

        DMA_ResetChannel(i);
        DMA_ClearAllInterrupt();
    }

    NVIC_SetVector((IRQn_Type)DMA030_IRQn, (uint32_t)gdma_isr);
    NVIC_ClearPendingIRQ(DMA030_IRQn);
    NVIC_EnableIRQ(DMA030_IRQn);

    g_dma_inited = 1;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_uninitialize(void)
{
    if (!g_dma_inited)
        return KDRV_STATUS_ERROR;

    NVIC_DisableIRQ(DMA030_IRQn);

    g_dma_inited = 0;
    return KDRV_STATUS_OK;
}

kdrv_gdma_handle_t kdrv_gdma_acquire_handle(void)
{
    int ch;

    // try to find an unused dma channel for memcpy
    for (ch = NUM_CH_FOR_MEMCPY; ch < NUM_DMA_CHANNELS; ch++) {
        if (!gDmaChCtrl[ch].in_use) {
            gDmaChCtrl[ch].in_use = true; // claim it is in use now
            break;
        }
    }

    if(ch == NUM_DMA_CHANNELS) {
#ifdef GDMA_DBG
        printf("DMA open fail\n");
#endif
        return -1;
    }

    return ch;
}

kdrv_status_t kdrv_gdma_configure_setting(kdrv_gdma_handle_t handle, gdma_setting_t *dma_setting)
{
    int ch = handle;

    pDMA_Register->dma_ch[ch].bf.csr_bits.DstWidth = dma_setting->dst_width;
    pDMA_Register->dma_ch[ch].bf.csr_bits.SrcWidth = dma_setting->src_width;
    pDMA_Register->dma_ch[ch].bf.csr_bits.SrcTcnt = dma_setting->burst_size;
    pDMA_Register->dma_ch[ch].bf.csr_bits.DstCtrl = dma_setting->dst_addr_ctrl;
    pDMA_Register->dma_ch[ch].bf.csr_bits.SrcCtrl = dma_setting->src_addr_ctrl;

    if (dma_setting->dma_mode == GDMA_HW_HANDSHAKE_MODE)
    {
        pDMA_Register->dma_ch[ch].bf.cfg_bits.DST_RS = dma_setting->dma_dst_req;
        pDMA_Register->dma_ch[ch].bf.cfg_bits.DST_HE = (dma_setting->dma_dst_req != GDMA_HW_REQ_NONE) ? 1 : 0;

        pDMA_Register->dma_ch[ch].bf.cfg_bits.SRC_RS = dma_setting->dma_src_req;
        pDMA_Register->dma_ch[ch].bf.cfg_bits.SRC_HE = (dma_setting->dma_src_req != GDMA_HW_REQ_NONE) ? 1 : 0;
        // enable SYNC register
        pDMA_Register->dma_global.SPI |= (0x1 << dma_setting->dma_src_req);
    }
    else
    {
        // disable SYNC register, FIXME: maybe it can always be enabled ?
        pDMA_Register->dma_global.SPI &= ~(0x1 << dma_setting->dma_src_req);
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_release_handle(kdrv_gdma_handle_t handle)
{
    gDmaChCtrl[handle].in_use = 0;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_gdma_transfer(kdrv_gdma_handle_t dma_handle, uint32_t dst_addr, uint32_t src_addr,
                                 uint32_t num_bytes, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg)
{
    kdrv_status_t ret;
    uint32_t  len, w;

    int ch = dma_handle;

    gDmaChCtrl[ch].running = 1;

    gDmaChCtrl[ch].user_cb = xfer_isr_cb;
    gDmaChCtrl[ch].user_arg = usr_arg;
    gDmaChCtrl[ch].src_end = src_addr + num_bytes - 1;
    gDmaChCtrl[ch].dst_end = dst_addr + num_bytes - 1;

    w = 1<<pDMA_Register->dma_ch[ch].bf.csr_bits.SrcWidth;
    len = (num_bytes + w - 1)/w;

    if(len > 0x3fffff) {
#ifdef GDMA_DBG
        printf("DMA transfer size exceed 4M\n");
#endif
        return KDRV_STATUS_ERROR;
    }

    DMA_SetInterrupt(ch, true, true, true);
    DMA_CHDataCtrl(ch, src_addr, dst_addr, len);

    if (xfer_isr_cb == 0) {
        
        #ifdef USE_EVENT_FLAG
         
        if(gDmaChCtrl[ch].client_evt == 0)
            gDmaChCtrl[ch].client_evt = osEventFlagsNew(NULL);
        
        #else
            gDmaChCtrl[ch].client_evt = osThreadGetId();
        #endif
        // trigger the DMA channel
        EnableDMAChannel(ch);

        if (gDmaChCtrl[ch].client_evt != 0) {
            #ifdef USE_EVENT_FLAG
            uint32_t flags = osEventFlagsWait(gDmaChCtrl[ch].client_evt,GDMA_FLAG_XFER_DONE, osFlagsWaitAny, osWaitForever);
            #else
            uint32_t flags = osThreadFlagsWait(GDMA_FLAG_XFER_DONE, osFlagsWaitAny, osWaitForever);
            #endif  
            gDmaChCtrl[ch].running = 0;
        } else {
            // Non-OS context
            do
            {
                __WFE();
            } while (gDmaChCtrl[ch].running == 1);
        }

        ret = gDmaChCtrl[ch].ret_sts;
    } else {
        // trigger the DMA channel
        EnableDMAChannel(ch);
        ret = KDRV_STATUS_OK;
    }

    return ret;
}

kdrv_status_t kdrv_gdma_memcpy(uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes,
                               gdma_xfer_callback_t xfer_isr_cb, void *usr_arg)
{
    kdrv_gdma_handle_t hdl;

    hdl = kdrv_gdma_acquire_handle();
    if(hdl == -1) {
        return KDRV_STATUS_ERROR;
    }

    pDMA_Register->dma_ch[hdl].bf.csr_bits.DstWidth  = _get_width_from_align(src_addr & 0x3, dst_addr & 0x3);
    pDMA_Register->dma_ch[hdl].bf.csr_bits.SrcWidth = pDMA_Register->dma_ch[hdl].bf.csr_bits.DstWidth;

    kdrv_gdma_transfer(hdl, dst_addr, src_addr, num_bytes, xfer_isr_cb, usr_arg);
    kdrv_gdma_release_handle(hdl);
    return KDRV_STATUS_OK;
}

