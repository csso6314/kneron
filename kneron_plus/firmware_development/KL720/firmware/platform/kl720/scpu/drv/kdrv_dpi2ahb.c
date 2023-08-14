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
*  kdrv_dpi2ahb.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  Configuration for KL720
*
*  Author:
*  -------
*

**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

// Sec 0: Comment block of the file

// Sec 1: Include File
#include "kmdw_camera.h"
#include "kdrv_dpi2ahb.h"
#include "kdrv_scu_ext.h"
#include "regbase.h"
#include "base.h"
#include "kdrv_cmsis_core.h"
// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define DPI2AHB_PAGE_NUM        2           // # of pages by one controller

#define D2A_REG_CTRL            0x00        // Control
#define D2A_REG_FNC             0x04        // Frame Number Control
#define D2A_REG_P0ADDR          0x08        // Page 0 Address
#define D2A_REG_P1ADDR          0x0C        // Page 1 Address
#define D2A_REG_ICT             0x10        // Interrupt Control
#define D2A_REG_IS              0x14        // Interrupt Status
#define D2A_REG_ST              0x18        // Status
#define D2A_REG_PT              0x1C        // Packet Type
#define D2A_REG_FIU0            0x20        // FI Use 0
#define D2A_REG_FIU1            0x24        // FI Use 1
#define D2A_REG_TAVR            0x28        // Tile Average Result (n)

/* D2A Packet type register */
#define D2A_PT_YUV422                   0x1E
#define D2A_PT_RGB565                   0x22
#define D2A_PT_RGB888                   0x24
#define D2A_PT_RAW8                     0x2A
#define D2A_PT_RAW10                    0x2B
#define D2A_PT_RAW12                    0x2C
#define D2A_PT_RAW14                    0x2D
#define D2A_PT_RAW16                    0x2E

/* D2A interrupt control & status register */
#define D2A_INT_TILE_AVG_D      BIT5
#define D2A_INT_AHB_TX_ERR      BIT4
#define D2A_INT_FIFO_UF         BIT3
#define D2A_INT_FIFO_OF         BIT2
#define D2A_INT_FN_OL           BIT1
#define D2A_INT_WRD             BIT0

#define D2A_INT_ALL             0x3F
#define D2A_INT_ERRORS          (D2A_INT_AHB_TX_ERR | D2A_INT_FIFO_UF | D2A_INT_FIFO_OF | D2A_INT_FN_OL)

/* D2A status register */
#define D2A_ST_PG               0x3

#define TILE_AVG_SIZE128        0
#define TILE_AVG_SIZE64         1
#define TILE_AVG_SIZE32         2

#define TILE_AVG_SIZE_128       0x00000000
#define TILE_AVG_SIZE_64        BIT16
#define TILE_AVG_SIZE_32        BIT17
#define TILE_AVG_SIZE_VAL       TILE_AVG_SIZE_128
#define TILE_AVG_SIZE_PIXELS    128

#if (IMGSRC_0_SENSORID == SENSOR_ID_HMX2056) || (IMGSRC_0_SENSORID == SENSOR_ID_SC132GS)
#define D2A_REG_CTRL_0          0x3000
#elif(IMGSRC_0_SENSORID == SENSOR_ID_OV9286) || (IMGSRC_0_SENSORID == SENSOR_ID_HMXRICA)
#define D2A_REG_CTRL_0          0x3008
#elif(IMGSRC_0_SENSORID == SENSOR_ID_GC2145)
#define D2A_REG_CTRL_0          0x3000
#elif(IMGSRC_0_SENSORID == SENSOR_ID_EXTERN)
#define D2A_REG_CTRL_0          0xE00004//0x3000//
#elif(IMGSRC_0_SENSORID == SENSOR_ID_SC200AI)
#define D2A_REG_CTRL_0          0x3000
#else
#define D2A_REG_CTRL_0          0
#endif

#if (IMGSRC_1_SENSORID == SENSOR_ID_HMX2056) || (IMGSRC_1_SENSORID == SENSOR_ID_SC132GS)
#define D2A_REG_CTRL_1          0x3000
#elif(IMGSRC_1_SENSORID == SENSOR_ID_OV9286) || (IMGSRC_1_SENSORID == SENSOR_ID_HMXRICA)
#define D2A_REG_CTRL_1          0x3008
#elif(IMGSRC_1_SENSORID == SENSOR_ID_GC2145)
#define D2A_REG_CTRL_1          0x2800
#elif(IMGSRC_1_SENSORID == SENSOR_ID_EXTERN)
#define D2A_REG_CTRL_1          0xE00004
#elif(IMGSRC_1_SENSORID == SENSOR_ID_SC200AI)
#define D2A_REG_CTRL_1          0x3000
#else
#define D2A_REG_CTRL_1          0
#endif

typedef struct dpi2ahb_context {
    uint32_t    irq;
    uint32_t    page_done_num;
    uint32_t    tile_avg_en;
    uint32_t    tile_n_w;
    uint32_t    tile_n_h;
}dpi2ahb_context;

typedef volatile union U_regDPI2AHB
{
    struct
    {
        uint32_t D2ACtrl;           //0x00  D2A Control Register
        uint32_t D2AFrameNumCtrl;   //0x04  D2A Frame Number Control Register
        uint32_t D2APage0Addr;      //0x08  Page0 Address Register
        uint32_t D2APage1Addr;      //0x0c  Page1 Address Register
        uint32_t D2AIntrCtrl;       //0x10  D2A Interrupt Control Register
        uint32_t D2AIntrStat;       //0x14  D2A Interrupt Status Register
        uint32_t D2AStatus;         //0x18  D2A Status Register
        uint32_t D2APacketType;     //0x1c  D2A Packet Type Register
        uint32_t FTCInternal1;      //0x20  FTC Internal Use Register 0
        uint32_t FTCInternal2;      //0x24  FTC Internal Use Register 1
        uint32_t TAVGRn;            //0x28  Tile Average Result n
    }dw;    //double word

    struct
    {
        //0x00  D2A Control Register
        uint32_t D2a_sr         :1;
        uint32_t D2a_hp         :1;
        uint32_t D2a_vp         :1;
        uint32_t D2a_da         :1;
        uint32_t D2a_fr_df      :7;
        uint32_t Gamma_en       :1;
        uint32_t Tile_avg_size  :2;
        uint32_t d2a_ft         :11;
        uint32_t Reserve0       :7;
        //0x04  D2A Frame Number Control register
        uint32_t D2a_fn         :1;
        uint32_t Tile_avg_en    :1;
        uint32_t Reserve1       :30;
        //0x08  Page1 Address Register
        uint32_t D2a_pg0_addr   :32;
        //0x0c  Page1 Address Register
        uint32_t D2a_pg1_addr   :32;
        //0x10  D2A interrupt Control Register
        uint32_t D2a_wrd_ce     :1;
        uint32_t D2a_fn_ol_ce   :1;
        uint32_t D2a_fifo_of_ce :1;
        uint32_t D2a_fifo_uf_ce :1;
        uint32_t Ahb_tx_err_ce  :1;
        uint32_t Tile_avg_d_ce  :1;
        uint32_t line_ol_ce     :1;
        uint32_t vsync_line_ol_ce:1;
        uint32_t d2a_addr_update_ce:1;
        uint32_t Reserve2       :23;
        //0x14  Interrupt status register
        uint32_t D2a_wrd        :1;
        uint32_t D2a_fn_ol      :1;
        uint32_t D2a_fifo_of    :1;
        uint32_t D2a_fifo_uf    :1;
        uint32_t Ahb_tx_err     :1;
        uint32_t tile_avg_d     :1;
        uint32_t line_ol        :1;
        uint32_t vsync_line_ol  :1;
        uint32_t d2a_addr_update:1;
        uint32_t Reserve3       :23;
        //0x18  D2A Status Register
        uint32_t D2a_pg1_b      :1;
        uint32_t D2a_pg0_b      :1;
        uint32_t Reserve4       :30;
        //0x1c  D2A Packet Type Register
        /*  0x18 : YUV420 8-bit
            0x19 : YUV420 10-bit
            0x1E : YUV422 8-bit
            0x22 : RGB565
            0x24 : RGB888
            0x2A : RAW8
            0x2B : RAW10
            0x2C : RAW12
            0x2D : RAW14
            0x2E : RAW16*/
        uint32_t d2a_pt:6;
        uint32_t Reserve5:26;
        //0x20  FTC Internal Use Register 0
        uint32_t D2a_fn_tg:1;
        uint32_t Reserve6:31;
        //0x24  FTC Internal Use Register 1
        uint32_t cs:5;
        uint32_t Reserve7:27;
        //0x28
        uint32_t TAVGR1_0:8;
        uint32_t TAVGR1_1:8;
        uint32_t TAVGR1_2:8;
        uint32_t TAVGR1_3:8;
    }bf;    //bit-field
}U_regDPI2AHB;

typedef volatile union U_regDPI2AHBCtrl
{
    struct
    {
        //0x9c
        uint32_t D2A_CtrlReg;
    }dw;    //double word

    struct
    {
        //0x9c
        uint32_t pwr_rst_n:1;
        uint32_t sys_rst_n:1;
        uint32_t ahb_rst_n:1;
        uint32_t rst_n:1;
        uint32_t pwr_rst_n_1:1;
        uint32_t sys_rst_n_1:1;
        uint32_t ahb_rst_n_1:1;
        uint32_t rst_n_1:1;
        uint32_t Reserve0:24;
    }bf;    //bit-field
}U_regDPI2AHBCtrl;

#define regdpi2ahb_0        ((union U_regDPI2AHB  *) DPI2AHB0_REG_BASE)
#define regdpi2ahb_1        ((union U_regDPI2AHB  *) DPI2AHB1_REG_BASE)
#define regdpi2ahb_ctrl     ((union U_regDPI2AHBCtrl  *) SCU_EXTREG_DPI2AHB_CTRL_REG)

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype


/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable
const uint32_t PixelFmtMapping[4] = {D2A_PT_RGB565, D2A_PT_RAW10, D2A_PT_RAW8, D2A_PT_YUV422};
dpi2ahb_context dpi2ahb_ctx[D2A_NUM] =
{
    {
        .irq = D2A0_IRQn,
    },
    {
        .irq = D2A1_IRQn,
    }
};

static kdrv_dpi2ahb_callback_t dpi2ahb_img_cb[D2A_NUM] = {NULL};

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable


// Sec 9: declaration of static function prototype


/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
// static int err_cnt = 0; // for debug
static int skip_next_count[2] = {0}; // >= 0
volatile uint32_t imgcnt;

void kdrv_dpi2ahb_irqhandler(uint32_t d2a_idx)
{
    uint32_t sta_is, sta_st;
    dpi2ahb_context *ctx = &dpi2ahb_ctx[d2a_idx];
    volatile union U_regDPI2AHB *d2a_base = (d2a_idx == D2A_0) ? regdpi2ahb_0 : regdpi2ahb_1;

    volatile uint32_t *D2APageAddr;

    sta_is = d2a_base->dw.D2AIntrStat;
    sta_st = d2a_base->dw.D2AStatus;
    d2a_base->dw.D2AIntrStat = sta_is;

    if (sta_is & D2A_INT_WRD)
    {
        sta_st = d2a_base->dw.D2AStatus;
        D2APageAddr = (sta_st == BIT0) ? &d2a_base->dw.D2APage0Addr : &d2a_base->dw.D2APage1Addr;
        imgcnt++;

        // hardware ping-pong buffers mechanism
        if (skip_next_count[d2a_idx] == 0)
        {
            uint32_t new_buf;
            dpi2ahb_img_cb[d2a_idx](d2a_idx, *D2APageAddr, &new_buf);
            *D2APageAddr = new_buf;
        }
        else
        {
            // printf("cam %d skip next\n", d2a_idx);
            skip_next_count[d2a_idx]--;
        }
    }

    if (sta_is & D2A_INT_AHB_TX_ERR)
    {
        // printf("%d: cam %d error D2A_INT_AHB_TX_ERR\n", ++err_cnt, d2a_idx);
        // skip what ?
    }

    if (sta_is & D2A_INT_FIFO_UF)
    {
        // printf("%d: cam %d error D2A_INT_FIFO_UF\n", ++err_cnt, d2a_idx);
        // skip what ?
    }

    if (sta_is & D2A_INT_FIFO_OF)
    {
        // printf("%d: cam %d error D2A_INT_FIFO_OF\n", ++err_cnt, d2a_idx);
        skip_next_count[d2a_idx] = 2; // skip next two
    }

    if (sta_is & D2A_INT_FN_OL)
    {
        // printf("%d: cam %d error D2A_INT_FN_OL\n", ++err_cnt, d2a_idx);
        // skip what ?
    }

    if (ctx->tile_avg_en)
    {
        d2a_base->dw.D2AFrameNumCtrl = 0x2;
    }
}

void D2A0_IRQ_Handler(void)//kdrv_dpi2ahb_isr_0(void)
{
    kdrv_dpi2ahb_irqhandler(D2A_0);
}

void D2A1_IRQ_Handler(void)//kdrv_dpi2ahb_isr_1(void)
{
    kdrv_dpi2ahb_irqhandler(D2A_1);
}

kdrv_status_t kdrv_dpi2ahb_set_para(uint32_t d2a_idx, cam_format* fmt, d2a_para* para)
{
    d2a_para* d2a_p = (para);
    dpi2ahb_context* d2a_ctx = &dpi2ahb_ctx[d2a_idx];
    union U_regDPI2AHB* d2a_base = (d2a_idx == D2A_0)? regdpi2ahb_0: regdpi2ahb_1;

    d2a_ctx->tile_avg_en = d2a_p->d2a_tile_ave_en;
    if (d2a_ctx->tile_avg_en)
    {
        d2a_base->bf.d2a_ft         = para->d2a_fifo_threshold;
        d2a_base->bf.D2a_fr_df      = para->d2a_drop_frame_num;
        d2a_base->bf.D2a_da         = para->d2a_data_align;
        d2a_base->bf.D2a_vp         = para->d2a_vsync_polarity;
        d2a_base->bf.D2a_hp         = para->d2a_hsync_polarity;
        d2a_base->bf.Tile_avg_en    = para->d2a_tile_ave_en;
        d2a_base->bf.Tile_avg_size  = para->d2a_tile_ave_size;
        d2a_base->dw.D2AIntrCtrl    = (D2A_INT_WRD | D2A_INT_TILE_AVG_D | D2A_INT_ERRORS);


        d2a_ctx->tile_n_w = (fmt->width + TILE_AVG_SIZE_PIXELS - 1) / TILE_AVG_SIZE_PIXELS;
        d2a_ctx->tile_n_h = (fmt->height + TILE_AVG_SIZE_PIXELS - 1) / TILE_AVG_SIZE_PIXELS;
        if (d2a_ctx->tile_n_w > TILE_BLOCK_MAX_W || d2a_ctx->tile_n_h >= TILE_BLOCK_MAX_H)
        {
            //printf("[%s][%d] can't support %d x %d tiles\n", __func__, d2a_idx, d2a_ctx->tile_n_w, d2a_ctx->tile_n_h);
            d2a_ctx->tile_avg_en = 0;
        }
    }
    else
    {
        d2a_base->bf.Tile_avg_en    = para->d2a_tile_ave_en;
        d2a_base->bf.d2a_ft         = para->d2a_fifo_threshold;
        d2a_base->bf.D2a_fr_df      = para->d2a_drop_frame_num;
        d2a_base->bf.D2a_da         = para->d2a_data_align;
        d2a_base->bf.D2a_vp         = para->d2a_vsync_polarity;
        d2a_base->bf.D2a_hp         = para->d2a_hsync_polarity;
        d2a_base->dw.D2AIntrCtrl = (D2A_INT_WRD|D2A_INT_ERRORS);
    }

    d2a_base->bf.d2a_pt     = para->d2a_packet_type;
    d2a_base->dw.FTCInternal1 = 0x1;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_dpi2ahb_start(uint32_t d2a_idx, kdrv_dpi2ahb_callback_t img_cb)
{
    NVIC_ClearPendingIRQ((IRQn_Type)dpi2ahb_ctx[d2a_idx].irq);
    NVIC_EnableIRQ((IRQn_Type)dpi2ahb_ctx[d2a_idx].irq);

    dpi2ahb_img_cb[d2a_idx] = img_cb;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_dpi2ahb_stop(uint32_t d2a_idx)
{
    NVIC_DisableIRQ((IRQn_Type)dpi2ahb_ctx[d2a_idx].irq);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_dpi2ahb_buf_init(uint32_t d2a_idx, uint32_t buf_addr_0, uint32_t buf_addr_1)
{
    union U_regDPI2AHB* d2a_base = (d2a_idx == D2A_0)? regdpi2ahb_0: regdpi2ahb_1;
    d2a_base->dw.D2APage0Addr = buf_addr_0;
    d2a_base->dw.D2APage1Addr = buf_addr_1;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_dpi2ahb_src_config(uint32_t d2a_idx, dpi_src_opt src)
{
    if(d2a_idx == KDP_CAM_0)
    {
        SET_MISC(DPI_MUX_SEL_0, src);
    }
    else if(d2a_idx == KDP_CAM_1)
    {
        SET_MISC(DPI_MUX_SEL_1, src);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_dpi2ahb_initialize(uint32_t d2a_idx)
{
    if(d2a_idx == D2A_0)
    {
        regdpi2ahb_ctrl->dw.D2A_CtrlReg |= 0xf;
    }
    else if(d2a_idx == D2A_1)
    {
        regdpi2ahb_ctrl->dw.D2A_CtrlReg |= 0xf0;
    }
    return KDRV_STATUS_OK;
}

