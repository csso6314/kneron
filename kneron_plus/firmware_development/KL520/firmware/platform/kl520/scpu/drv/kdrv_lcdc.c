/*
 * Kneron KL520 LCDC driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <string.h>
#include "project.h"
#include "kdrv_scu_ext.h"
#include "kdrv_clock.h"
#include "kdrv_lcdc.h"
#include "kdrv_pinmux.h"
#include "kmdw_memory.h"
#include "kdrv_pwm.h"

//#define LCDC_DBG
#ifdef LCDC_DBG
#define lcdc_msg(fmt, ...) info_msg("[KDRV_LCDC] " fmt, ##__VA_ARGS__)
#else
#define lcdc_msg(fmt, ...)
#endif

#define PWM0_FREQ_CNT   (2000000)

/*=============================== LCDC Register Releated Macros ===============================*/
#define LCDC_REG_FUNC_ENABLE                                (LCD_FTLCDC210_PA_BASE + 0x0000)
#define LCDC_REG_PANEL_PIXEL                                (LCD_FTLCDC210_PA_BASE + 0x0004)
#define LCDC_REG_INTR_ENABLE_MASK                           (LCD_FTLCDC210_PA_BASE + 0x0008)
#define LCDC_REG_INTR_CLEAR                                 (LCD_FTLCDC210_PA_BASE + 0x000C)
#define LCDC_REG_INTR_STATUS                                (LCD_FTLCDC210_PA_BASE + 0x0010)
#define LCDC_REG_FRAME_BUFFER                               (LCD_FTLCDC210_PA_BASE + 0x0014)
#define LCDC_REG_PANEL_IMAGE0_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x0018)
#define LCDC_REG_PANEL_IMAGE1_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x0024)
#define LCDC_REG_PANEL_IMAGE2_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x0030)
#define LCDC_REG_PANEL_IMAGE3_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x003C)
#define LCDC_REG_PATGEN_PATTERN_BAR_DISTANCE                (LCD_FTLCDC210_PA_BASE + 0x0048)
#define LCDC_REG_FIFO_THRESHOLD                             (LCD_FTLCDC210_PA_BASE + 0x004C)
#define LCDC_REG_BANDWIDTH_CTRL                             (LCD_FTLCDC210_PA_BASE + 0x0050)
#define LCDC_REG_FIFO_THRESHOLD_CTRL_PARAM                  (LCD_FTLCDC210_PA_BASE + 0x0060)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL                     (LCD_FTLCDC210_PA_BASE + 0x0100)
#define LCDC_REG_VERTICAL_TIMING_CTRL                       (LCD_FTLCDC210_PA_BASE + 0x0104)
#define LCDC_REG_VERTICAL_BACK_PORCH                        (LCD_FTLCDC210_PA_BASE + 0x0108)
#define LCDC_REG_POLARITY_CTRL                              (LCD_FTLCDC210_PA_BASE + 0x010C)
#define LCDC_REG_SERIAL_PANEL_PIXEL                         (LCD_FTLCDC210_PA_BASE + 0x0200)

#define LCDC_REG_PIPPOP_FMT_1                               (LCD_FTLCDC210_PA_BASE + 0x0318)

#define LCDC_REG_COLOR_MGR_0                                (LCD_FTLCDC210_PA_BASE + 0x0400)
#define LCDC_REG_COLOR_MGR_1                                (LCD_FTLCDC210_PA_BASE + 0x0404)
#define LCDC_REG_COLOR_MGR_2                                (LCD_FTLCDC210_PA_BASE + 0x0408)
#define LCDC_REG_COLOR_MGR_3                                (LCD_FTLCDC210_PA_BASE + 0x040C)
#define LCDC_REG_LT_OF_GAMMA_RED                            (LCD_FTLCDC210_PA_BASE + 0x0600)
#define LCDC_REG_LT_OF_GAMMA_GREEN                          (LCD_FTLCDC210_PA_BASE + 0x0700)
#define LCDC_REG_LT_OF_GAMMA_BLUE                           (LCD_FTLCDC210_PA_BASE + 0x0800)

#define LCDC_REG_PALETTE                                    (LCD_FTLCDC210_PA_BASE + 0x0A00)

#define LCDC_REG_SCALER_HOR_RES_IN                          (LCD_FTLCDC210_PA_BASE + 0x1100)
#define LCDC_REG_SCALER_VER_RES_IN                          (LCD_FTLCDC210_PA_BASE + 0x1104)
#define LCDC_REG_SCALER_HOR_RES_OUT                         (LCD_FTLCDC210_PA_BASE + 0x1108)
#define LCDC_REG_SCALER_VER_RES_OUT                         (LCD_FTLCDC210_PA_BASE + 0x110C)
#define LCDC_REG_SCALER_MISC                                (LCD_FTLCDC210_PA_BASE + 0x1110)
#define LCDC_REG_SCALER_RES                                 (LCD_FTLCDC210_PA_BASE + 0x112C)

/* LCD Function Enable Parameter (Offset 0x0000) */
#define LCDC_REG_FUNC_ENABLE_SET_PenGen(val)                SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 14)
#define LCDC_REG_FUNC_ENABLE_SET_PiPEn(val)                 SET_MASKED_BITS(LCDC_REG_FUNC_ENABLE, val, 10, 11)
#define LCDC_REG_FUNC_ENABLE_SET_BlendEn(val)               SET_MASKED_BITS(LCDC_REG_FUNC_ENABLE, val, 8, 9)
#define LCDC_REG_FUNC_ENABLE_SET_ScalerEn(val)              SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 5)
#define LCDC_REG_FUNC_ENABLE_SET_OSDEn(val)                 SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 4)
#define LCDC_REG_FUNC_ENABLE_SET_EnYCbCr(val)               SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 3)
#define LCDC_REG_FUNC_ENABLE_SET_EnYCbCr420(val)            SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 2)
#define LCDC_REG_FUNC_ENABLE_SET_LCDon(val)                 SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 1)
#define LCDC_REG_FUNC_ENABLE_SET_LCDen(val)                 SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 0)
#define LCDC_REG_FUNC_ENABLE_GET_Values()                   GET_BITS(LCDC_REG_FUNC_ENABLE, 0, 19)

/* LCD Panel Pixel Parameter (Offset 0x0004) */
#define LCDC_REG_PANEL_PIXEL_SET_AddrUpdate(val)            SET_MASKED_BIT(LCDC_REG_PANEL_PIXEL, val, 16)
#define LCDC_REG_PANEL_PIXEL_SET_UpdateSrc(val)             SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 14, 15)
#define LCDC_REG_PANEL_PIXEL_SET_DitherType(val)            SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 12, 13)
#define LCDC_REG_PANEL_PIXEL_SET_PanelType(val)             SET_MASKED_BIT(LCDC_REG_PANEL_PIXEL, val, 11)
#define LCDC_REG_PANEL_PIXEL_SET_Vcomp(val)                 SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 9, 10)
#define LCDC_REG_PANEL_PIXEL_SET_RGBTYPE(val)               SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 7, 8)
#define LCDC_REG_PANEL_PIXEL_SET_Endian(val)                SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 5, 6)
#define LCDC_REG_PANEL_PIXEL_SET_BGRSW(val)                 SET_MASKED_BIT(LCDC_REG_PANEL_PIXEL, val, 4)
#define LCDC_REG_PANEL_PIXEL_SET_BppFifo(val)               SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 0, 2)
#define LCDC_REG_PANEL_PIXEL_GET_Values()                   GET_BITS(LCDC_REG_PANEL_PIXEL, 0, 20)

#define LCDC_REG_PANEL_PIXEL_RGBTYPE_565                    (0x0 << 7)
#define LCDC_REG_PANEL_PIXEL_RGBTYPE_555                    (0x1 << 7)
#define LCDC_REG_PANEL_PIXEL_RGBTYPE_444                    (0x2 << 7)
#define LCDC_REG_PANEL_PIXEL_RGBTYPE_MASK                   (BIT7 | BIT8)

#define LCDC_REG_PANEL_PIXEL_BppFifo_1bpp                   (0x0)
#define LCDC_REG_PANEL_PIXEL_BppFifo_2bpp                   (0x1)
#define LCDC_REG_PANEL_PIXEL_BppFifo_4bpp                   (0x2)
#define LCDC_REG_PANEL_PIXEL_BppFifo_8bpp                   (0x3)
#define LCDC_REG_PANEL_PIXEL_BppFifo_16bpp                  (0x4)
#define LCDC_REG_PANEL_PIXEL_BppFifo_24bpp                  (0x5)
#define LCDC_REG_PANEL_PIXEL_BppFifo_MASK                   (BIT0 | BIT1 | BIT2)

#define LCDC_REG_PANEL_PIXEL_VSync                          (0x00)
#define LCDC_REG_PANEL_PIXEL_VBackPorch                     (0x01)
#define LCDC_REG_PANEL_PIXEL_VActiveImg                     (0x02)
#define LCDC_REG_PANEL_PIXEL_VFrontPorch                    (0x03)


/* LCD Interrupt Enable Mask Parameter (Offset 0x0008) */
#define LCDC_REG_INTR_ENABLE_MASK_SET(val)                  SET_MASKED_BITS(LCDC_REG_INTR_ENABLE_MASK, val, 0, 3)
#define LCDC_REG_INTR_ENABLE_MASK_GET_Values()              GET_BITS(LCDC_REG_INTR_ENABLE_MASK, 0, 3)

/* LCD Interrupt Status Clear (Offset 0x000C) */
#define LCDC_REG_INTR_CLEAR_BusErr()                        SET_BIT(LCDC_REG_INTR_CLEAR, 3) //Write only
#define LCDC_REG_INTR_CLEAR_Vstatus()                       SET_BIT(LCDC_REG_INTR_CLEAR, 2) //Write only
#define LCDC_REG_INTR_CLEAR_NxtBase()                       SET_BIT(LCDC_REG_INTR_CLEAR, 1) //Write only
#define LCDC_REG_INTR_CLEAR_FIFOUdn()                       SET_BIT(LCDC_REG_INTR_CLEAR, 0) //Write only
#define LCDC_REG_INTR_CLEAR_AllStatus(val)                  SET_MASKED_BITS(LCDC_REG_INTR_CLEAR, val, 0, 3) //Write only

/* LCD Interrupt Status (Offset 0x0010) */ 
#define LCDC_REG_INTR_STATUS_IntBusErr                      BIT3 //Read only
#define LCDC_REG_INTR_STATUS_IntVstatus                     BIT2 //Read only
#define LCDC_REG_INTR_STATUS_IntNxtBase                     BIT1 //Read only
#define LCDC_REG_INTR_STATUS_IntFIFOUdn                     BIT0 //Read only
#define LCDC_REG_INTR_GET_IntStatus()                       GET_BITS(LCDC_REG_INTR_STATUS, 0, 3) //Read only

/* Frame Buffer parameter (Offset 0x0014) */
#define LCDC_REG_FRAME_BUFFER_GET_LmScalDownValues()        GET_BITS(LCDC_REG_FRAME_BUFFER, 8, 15)

/* FIFO Threshold Control (Offset 0x004C) */
#define LCDC_REG_FIFO_THRESHOLD_SET_BufThreshold(val)       SET_MASKED_BITS(LCDC_REG_FIFO_THRESHOLD, val, 0, 31)
#define LCDC_REG_FIFO_THRESHOLD_GET_BufThresholdValues()    GET_BITS(LCDC_REG_FIFO_THRESHOLD, 0, 31)

/* Bus Bandwidth Control Parameter (Offset 0x0050) */
#define LCDC_REG_BANDWIDTH_CTRL_GET_Values()                GET_BITS(LCDC_REG_BANDWIDTH_CTRL, 0, 9)

/* FIFO Threshold Control Parameter (Offset 0x0060) */
#define LCDC_REG_FIFO_THRESHOLD_CTRL_PARAM_GET_Buf4Thrshold() GET_BITS(LCDC_REG_FIFO_THRESHOLD_CTRL_PARAM, 0, 7)

/* LCD Horizontal Timing Control Parameter (Offset 0x0100) */
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HBP(val)        SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 24, 31)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HFP(val)        SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 16, 23)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HW(val)         SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 8, 15)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_PL(val)         SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 0, 7)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_Values()        GET_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, 0, 31)

/* LCD Vertical Timing Control Parameter (Offset 0x0104) */
#define LCDC_REG_VERTICAL_TIMING_CTRL_SET_VFP(val)          SET_MASKED_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, val, 24, 31)
#define LCDC_REG_VERTICAL_TIMING_CTRL_SET_VW(val)           SET_MASKED_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, val, 16, 21)
#define LCDC_REG_VERTICAL_TIMING_CTRL_SET_LF(val)           SET_MASKED_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, val, 0, 11)
#define LCDC_REG_VERTICAL_TIMING_CTRL_GET_Values()          GET_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, 0, 31)

/* LCD Vertical Back Porch Parameter (Offset 0x0108) */
#define LCDC_REG_VERTICAL_BACK_PORCH_SET_VBP(val)           SET_MASKED_BITS(LCDC_REG_VERTICAL_BACK_PORCH, val, 0, 7)
#define LCDC_REG_VERTICAL_BACK_PORCH_GET_VBP()              GET_BITS(LCDC_REG_VERTICAL_BACK_PORCH, 0, 7)

/* LCD Polarity Control Parameter (Offset 0x010C) */
#define LCDC_REG_POLARITY_CTRL_SET_DivNo(val)               SET_MASKED_BITS(LCDC_REG_POLARITY_CTRL, val, 8, 14)
#define LCDC_REG_POLARITY_CTRL_SET_IPWR(val)                SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 4)
#define LCDC_REG_POLARITY_CTRL_SET_IDE(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 3)
#define LCDC_REG_POLARITY_CTRL_SET_ICK(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 2)
#define LCDC_REG_POLARITY_CTRL_SET_IHS(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 1)
#define LCDC_REG_POLARITY_CTRL_SET_IVS(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 0)
#define LCDC_REG_POLARITY_CTRL_GET_Values()                 GET_BITS(LCDC_REG_POLARITY_CTRL, 0, 14)

/* LCD Serial Panel Pixel Parameter (Offset 0x0200) */
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_AUO052(val)         SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 5)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_LSR(val)            SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 4)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_ColorSeq(val)       SET_MASKED_BITS(LCDC_REG_SERIAL_PANEL_PIXEL, val, 2, 3)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_DeltaType(val)      SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 1)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_SerialMode(val)     SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 0)
#define LCDC_REG_SERIAL_PANEL_PIXEL_GET_Values()            GET_BITS(LCDC_REG_SERIAL_PANEL_PIXEL, 0, 5)

/* PiP/PoP Image Format 1 (Offset 0x0318) */
#define LCDC_REG_PIPPOP_FMT_1_SET_BppFifo0(val)             SET_MASKED_BITS(LCDC_REG_PIPPOP_FMT_1, val, 0, 2)
#define LCDC_REG_PIPPOP_FMT_1_GET_BppFifo0()                GET_BITS(LCDC_REG_PIPPOP_FMT_1, 0, 2)

/* Horizontal Resolution Register of Scaler Input (Offset 0x1100) */
#define LCDC_REG_SCALER_HOR_RES_IN_SET_hor_no_in(val)       SET_MASKED_BITS(LCDC_REG_SCALER_HOR_RES_IN, val, 0, 11)
#define LCDC_REG_SCALER_HOR_RES_IN_GET_hor_no_in()          GET_BITS(LCDC_REG_SCALER_HOR_RES_IN, 0, 11)

/* Vertical Resolution Register of Scaler Input (Offset 0x1104) */
#define LCDC_REG_SCALER_VER_RES_IN_SET_ver_no_in(val)       SET_MASKED_BITS(LCDC_REG_SCALER_VER_RES_IN, val, 0, 11)
#define LCDC_REG_SCALER_VER_RES_IN_GET_ver_no_in()          GET_BITS(LCDC_REG_SCALER_VER_RES_IN, 0, 11)

/* Horizontal Resolution Register of Scaler Output (Offset 0x1108) */
#define LCDC_REG_SCALER_HOR_RES_OUT_SET_hor_no_out(val)     SET_MASKED_BITS(LCDC_REG_SCALER_HOR_RES_OUT, val, 0, 13)
#define LCDC_REG_SCALER_HOR_RES_OUT_GET_hor_no_out()        GET_BITS(LCDC_REG_SCALER_HOR_RES_OUT, 0, 13)

/* Vertical Resolution Register of Scaler Output (Offset 0x110C) */
#define LCDC_REG_SCALER_VER_RES_OUT_SET_ver_no_out(val)     SET_MASKED_BITS(LCDC_REG_SCALER_VER_RES_OUT, val, 0, 13)
#define LCDC_REG_SCALER_VER_RES_OUT_GET_ver_no_out()        GET_BITS(LCDC_REG_SCALER_VER_RES_OUT, 0, 13)

/* Scaler Control (Offset 0x1110) */
#define LCDC_REG_SCALER_MISC_SET_fir_sel(val)               SET_MASKED_BITS(LCDC_REG_SCALER_MISC, val, 6, 8)
#define LCDC_REG_SCALER_MISC_SET_hor_inter_mode(val)        SET_MASKED_BITS(LCDC_REG_SCALER_MISC, val, 3, 4)
#define LCDC_REG_SCALER_MISC_SET_ver_inter_mode(val)        SET_MASKED_BITS(LCDC_REG_SCALER_MISC, val, 1, 2)
#define LCDC_REG_SCALER_MISC_SET_bypass_mode(val)           SET_MASKED_BIT(LCDC_REG_SCALER_MISC, val, 0)
#define LCDC_REG_SCALER_MISC_GET_Values()                   GET_BITS(LCDC_REG_SCALER_MISC, 0, 8)

/* Scaler Control (Offset 0x1110) */
#define LCDC_REG_SCALER_RES_SET_scal_hor_num(val)           SET_MASKED_BITS(LCDC_REG_SCALER_RES, val, 8, 15)
#define LCDC_REG_SCALER_RES_SET_scal_ver_num(val)           SET_MASKED_BITS(LCDC_REG_SCALER_RES, val, 0, 7)
#define LCDC_REG_SCALER_RES_GET_Values()                    GET_BITS(LCDC_REG_SCALER_RES, 0, 15)


/*===================================== #define MACRO =====================================*/


/* if enable DOWN_SCALE_WITH_CROP, 
   need to modify result gui boundary due to changed display boundary*/
#define LCDC_DOWN_SCALE_WITH_CROP       (NO)

#define SWAP(a, b)                      do \
                                        { a^=b; \
                                          b^=a; \
                                          a^=b; }while(0)
typedef struct
{
    int         buf_size;
    uint32_t    buf_addr;
    bool        init_done;
} kdrv_lcdc_fb_t;
kdrv_lcdc_fb_t kdrv_lcdc_frame_buffer;
static kdrv_lcdc_fb_t *p_lcdc_fb = &kdrv_lcdc_frame_buffer;

struct lcdc_img_pixfmt_pxp {
    kdrv_lcdc_img_pixfmt_t pixfmt_img0;
    kdrv_lcdc_img_pixfmt_t pixfmt_img1;
    kdrv_lcdc_img_pixfmt_t pixfmt_img2;
    kdrv_lcdc_img_pixfmt_t pixfmt_img3;
};

static bool test_pat_en = false;

static kdrv_display_t kdrv_display;
static uint16_t disp_frame_margin_len = LCDC_HINT_BOUNDINGBOX_MARGIN_LEN;
static bool disp_en_review_snapshot = false;
static bool disp_en_snapshot_preview = false;

extern inline int kdrv_display_cal_framesize(unsigned short width, unsigned short height, unsigned int input_fmt);
/************************************************************************
*                 	        Private API                     
************************************************************************/
static __inline int lcdc_draw_pixel(
    uint32_t frame_buffer, int x, int y, uint32_t width, uint16_t color)
{
    *(uint16_t *)(frame_buffer + ((y * width + x) << 1) + 0) = color;
    
    return 0;
}

static uint32_t lcdc_get_bpp_from_img_pixfmt(kdrv_lcdc_img_pixfmt_t pixfmt)
{
    uint32_t bpp = KDRV_LCDC_IMG_PIXFMT_1BPP;
    if(pixfmt > KDRV_LCDC_IMG_PIXFMT_ARGB1555)
        bpp = 4;
    else
        bpp = pixfmt;
    return bpp;
}

static void lcdc_init_rgb_value()
{
    int i;
    for (i = 0; i < 0x100; i++) {
        writeb(i, LCDC_REG_LT_OF_GAMMA_RED + i);
        writeb(i, LCDC_REG_LT_OF_GAMMA_GREEN + i);
        writeb(i, LCDC_REG_LT_OF_GAMMA_BLUE + i);
    }
}

static void lcdc_init_palette()
{
    int i;
    uint16_t color;

    for (i = 0; i < 256; i++) {
        color = ((i >> 3) << 11) | ((i >> 2) << 5) | ((i >> 3) << 0);
        writew(color, LCDC_REG_PALETTE + i * 2);
    }
}

static void lcdc_set_img_fmt(struct lcdc_img_pixfmt_pxp *fmt)
{
    uint32_t val = inw(LCDC_REG_PIPPOP_FMT_1);
    uint32_t bpp0 = lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img0);
    uint32_t bpp1 = lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img1) << 4;
    uint32_t bpp2 = lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img2) << 8;
    uint32_t bpp3 = lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img3) << 12;

    lcdc_init_rgb_value();
    lcdc_init_palette();
        
    val = bpp0 | bpp1 | bpp2 | bpp3;
    LCDC_REG_PIPPOP_FMT_1_SET_BppFifo0(val);
}

static void lcdc_set_img_input_fmt(kdrv_display_t *display_drv)
{
    struct lcdc_img_pixfmt_pxp img_pixfmt;

    uint32_t pix_param = inw(LCDC_REG_PANEL_PIXEL);
    if (V2K_PIX_FMT_RGB565 == display_drv->vi_params.input_fmt) {
        img_pixfmt.pixfmt_img0 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        img_pixfmt.pixfmt_img1 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        img_pixfmt.pixfmt_img2 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        img_pixfmt.pixfmt_img3 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        lcdc_set_img_fmt(&img_pixfmt);

        LCDC_REG_FUNC_ENABLE_SET_EnYCbCr(0);
        pix_param = (pix_param & ~(LCDC_REG_PANEL_PIXEL_RGBTYPE_MASK |
                                   LCDC_REG_PANEL_PIXEL_BppFifo_MASK) ) |
                    LCDC_REG_PANEL_PIXEL_BppFifo_16bpp |
                    LCDC_REG_PANEL_PIXEL_RGBTYPE_565;

        outw(LCDC_REG_PANEL_PIXEL, pix_param);
    } else if (V2K_PIX_FMT_RAW8 == display_drv->vi_params.input_fmt) {
        img_pixfmt.pixfmt_img0 = KDRV_LCDC_IMG_PIXFMT_8BPP;
        img_pixfmt.pixfmt_img1 = KDRV_LCDC_IMG_PIXFMT_8BPP;
        img_pixfmt.pixfmt_img2 = KDRV_LCDC_IMG_PIXFMT_8BPP;
        img_pixfmt.pixfmt_img3 = KDRV_LCDC_IMG_PIXFMT_8BPP;
        lcdc_set_img_fmt(&img_pixfmt);

        LCDC_REG_FUNC_ENABLE_SET_EnYCbCr(0);
        pix_param = (pix_param & ~(LCDC_REG_PANEL_PIXEL_RGBTYPE_MASK |
                                   LCDC_REG_PANEL_PIXEL_BppFifo_MASK) ) |
                    LCDC_REG_PANEL_PIXEL_BppFifo_8bpp;
        outw(LCDC_REG_PANEL_PIXEL, pix_param);
		    }  else if (V2K_PIX_FMT_YCBCR == display_drv->vi_params.input_fmt) {
        img_pixfmt.pixfmt_img0 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        img_pixfmt.pixfmt_img1 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        img_pixfmt.pixfmt_img2 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        img_pixfmt.pixfmt_img3 = KDRV_LCDC_IMG_PIXFMT_16BPP;
        lcdc_set_img_fmt(&img_pixfmt);
        LCDC_REG_FUNC_ENABLE_SET_EnYCbCr(1);      // enable YCbcr
        LCDC_REG_FUNC_ENABLE_SET_EnYCbCr420(0);  // enable 422
				LCDC_REG_PANEL_PIXEL_SET_Endian(KDRV_LCDC_FB_DATA_ENDIAN_LBLP);
      
        pix_param = (pix_param & ~( LCDC_REG_PANEL_PIXEL_BppFifo_MASK) ) |
                    LCDC_REG_PANEL_PIXEL_BppFifo_16bpp;
        outw(LCDC_REG_PANEL_PIXEL, pix_param);	
    }
}

void lcd_update_snapshot_data(uint32_t addr)
{
    memcpy((void *)KDP_DDR_SNAPSHOT_RGB_IMG_ADDR, (void *)addr, KDP_DDR_SNAPSHOT_RGB_IMG_SIZE);
    lcdc_msg("Snapshot display preview 0x%x done...\n", addr);
    disp_en_snapshot_preview = false;
}

static uint32_t kdp2_disp_buf_addr = 0;
static int kdp2_write_done_idx = 0;
static int kdp2_read_done_idx = 0;
static int kdp2_disp_count = -1;

uint32_t lcdc_kdp2_get_disp_idx(int *read_done_idx)
{
    if(kdp2_disp_count == kdp2_write_done_idx)
    {
        kdp2_read_done_idx ++;
        kdp2_read_done_idx &= 1;
    }
    else
    {
        kdp2_read_done_idx = kdp2_write_done_idx;
        //kdp2_read_done_idx ++;
        //kdp2_read_done_idx &= 1;
    }
    //lcdc_msg("lcdc_kdp2_get_disp_idx: kdp2_read_done_idx = %d\n", kdp2_read_done_idx);
    *read_done_idx = kdp2_read_done_idx;
    
    return 0;
}

uint32_t lcdc_kdp2_set_disp_buf(uint32_t buf_addr, int write_done_idx)
{
    kdp2_write_done_idx = write_done_idx;
    kdp2_disp_buf_addr = buf_addr;
    //lcdc_msg("lcdc_kdp2_set_disp_buf: kdp2_disp_buf_addr=0x%x, write_done_idx=%d\n", buf_addr, write_done_idx);
    return 0;
}
//if buf_addr is null, update the
//local_irq_disable/local_irq_enable
uint32_t lcdc_kdp2_get_disp_buf(int cam_idx, int *disp_idx)
{
    kdp2_disp_count = kdp2_write_done_idx;
    if(kdp2_disp_buf_addr)
        return kdp2_disp_buf_addr;

    if(p_lcdc_fb->buf_addr)//blank the lcd, // No new buffer.
    {
        return p_lcdc_fb->buf_addr;
    }
    else
        return NULL;
}
void lcdc_vstatus_isr(void)
{
    uint16_t status;
    uint32_t buf_addr;

    status = LCDC_REG_INTR_GET_IntStatus();
    kdp2_disp_count ++;
    kdp2_disp_count &= 65535;
    
    if (status & LCDC_REG_INTR_STATUS_IntVstatus) {  

        buf_addr = lcdc_kdp2_get_disp_buf(kdrv_display.cam_src_idx, NULL);

        if(disp_en_snapshot_preview == true) {
            lcd_update_snapshot_data(buf_addr);
        }
        if(disp_en_review_snapshot == true) {
            buf_addr = p_lcdc_fb->buf_addr;
        }
        outw(LCDC_REG_PANEL_IMAGE0_FRAME0, buf_addr);
        LCDC_REG_INTR_CLEAR_Vstatus();
    }
    outw(LCDC_REG_INTR_CLEAR, status);
    NVIC_ClearPendingIRQ((IRQn_Type)LCDC_FTLCDC210_VSTATUS_IRQ);
}

#if 0
static int kdp520_lcdc_reset(struct display_driver_s *display_drv)
{

    SCU_EXTREG_SWRST_SET_LCDC_resetn(1);

    // outw(SCU_EXTREG_SWRST_MASK1,
    //      SCU_EXTREG_SWRST_MASK1_AResetn_u_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_PRESETn_u_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_TV_RSTn_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_LC_SCALER_RSTn_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_LC_RSTn_FTLCDC210);

    return 0;
}
#endif

/************************************************************************
*                 	        Public API                     
************************************************************************/
kdrv_display_t *kdrv_display_initialize(void)
{
    kdrv_display_screen_control(KDRV_LCDC_SCREEN_OFF);
    
    SCU_EXTREG_SWRST_SET_LCDC_resetn(0); //software reset
    SCU_EXTREG_CLK_EN1_SET_LC_SCALER(1);
    SCU_EXTREG_CLK_EN1_SET_LC_CLK(1);
    SCU_EXTREG_SWRST_SET_LCDC_resetn(1); //reset release
    kdrv_clock_mgr_change_pll5_clock(1, 0x54, 5); //***** MUST SET Clock

    //clear all interrupt status
    LCDC_REG_INTR_CLEAR_AllStatus(BIT3|BIT2|BIT1|BIT0);

    //enable buserr / vstatus / next frame / fifo underrun interrupt
    LCDC_REG_INTR_ENABLE_MASK_SET(BIT3|BIT2|BIT1|BIT0);

    // set FIFO thread
    LCDC_REG_FIFO_THRESHOLD_SET_BufThreshold(BIT23|BIT16|BIT9|BIT2);

    LCDC_REG_PANEL_PIXEL_SET_Vcomp(LCDC_REG_PANEL_PIXEL_VSync);

    if(test_pat_en)
         LCDC_REG_FUNC_ENABLE_SET_PenGen(1);

    NVIC_SetVector((IRQn_Type)LCDC_FTLCDC210_VSTATUS_IRQ, (uint32_t)lcdc_vstatus_isr);
    NVIC_EnableIRQ(LCDC_FTLCDC210_VSTATUS_IRQ);
    return &kdrv_display;
}

kdrv_status_t kdrv_display_buffer_initialize(struct video_input_params *params)
{
    if(p_lcdc_fb->init_done == true)
        return KDRV_STATUS_OK;

    p_lcdc_fb->buf_size = kdrv_display_cal_framesize(params->input_xres, params->input_yres, params->input_fmt);
    p_lcdc_fb->buf_addr = kmdw_ddr_reserve(p_lcdc_fb->buf_size);
    memset((void *)p_lcdc_fb->buf_addr, 0, p_lcdc_fb->buf_size);
    p_lcdc_fb->init_done = true;
    return KDRV_STATUS_OK;
}

uint32_t kdrv_display_get_buffer(void)
{
    return p_lcdc_fb->buf_addr;
}

kdrv_status_t kdrv_display_set_params(kdrv_display_t *display_drv, struct video_input_params *params)
{
    struct video_input_params *p;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;
   
    p = &display_drv->vi_params;
    memcpy(p, params, sizeof(*params));
    display_drv->fb_size = kdrv_display_cal_framesize(p->input_xres, p->input_yres, p->input_fmt);
    lcdc_set_img_input_fmt(display_drv);
    kdrv_display_set_backlight(display_drv, 1);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_get_params(kdrv_display_t *display_drv, struct video_input_params *params)
{
    struct video_input_params *p;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    p = &display_drv->vi_params;
    memcpy(params, p, sizeof(*params));
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_camera(kdrv_display_t *display_drv, uint8_t cam_idx)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;
 
    display_drv->cam_src_idx = cam_idx;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_start(kdrv_display_t *display_drv)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    kdrv_display_screen_control(KDRV_LCDC_SCREEN_ON);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_stop(kdrv_display_t *display_drv)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    kdrv_display_set_backlight(display_drv, 0);
    kdrv_display_screen_control(KDRV_LCDC_SCREEN_OFF);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_pen(kdrv_display_t *display_drv, uint16_t color, uint32_t width)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    display_drv->pen_info.color = color;
    display_drv->pen_info.width = width;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_update_draw_fb(kdrv_display_t *display_drv, uint32_t addr, uint8_t cam_idx)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    display_drv->fb_addr = addr;
    display_drv->cam_src_idx = cam_idx;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_draw_static_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height)
{
    uint32_t x_pos, y_pos;
    uint32_t x_unit = 1;
    uint32_t y_unit = 1;
    uint32_t top = org_y;
    uint32_t left = org_x;
    uint32_t right = org_x + width;
    uint32_t bottom = top + height;
    uint32_t border_size;
    uint32_t left_border;
    uint32_t right_border;
    uint32_t top_lower;
    uint32_t bottom_upper;
    uint32_t frame_width;
    uint32_t fb_addr;
    uint16_t color;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    border_size = display_drv->pen_info.width;
    left_border = left + border_size;
    right_border = right - border_size;
    top_lower = top + border_size;
    bottom_upper = bottom - border_size;
    frame_width =  display_drv->vi_params.input_xres;
    fb_addr = display_drv->fb_addr;
    color = display_drv->pen_info.color;
    for (y_pos = top; y_pos < bottom; y_pos += y_unit)
    {
        if ((y_pos >= top && y_pos < top_lower) || 
            (y_pos >= bottom_upper))
        {
            for (x_pos = left; x_pos < right; x_pos += x_unit)
            {
                if ((x_pos >= left && x_pos < (disp_frame_margin_len+border_size)*3) || 
                    (x_pos >= (right_border - (disp_frame_margin_len+border_size)*2)))
                {
                    lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
                }
            }
        }
        else
        {
            if ((y_pos >= top && y_pos < (disp_frame_margin_len+border_size)*3) || 
                (y_pos >= (bottom_upper - (disp_frame_margin_len+border_size)*2)))
            {
                for (x_pos = left; x_pos < left_border; x_pos += x_unit)
                {
                    lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
                }
       
                for (x_pos = right_border; x_pos < right; x_pos += x_unit)
                {
                    lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
                }
            }
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_draw_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height)
{
    uint32_t x_pos, y_pos;
    uint32_t x_unit = 1;
    uint32_t y_unit = 1;
    uint32_t top = org_y;
    uint32_t left = org_x;
    uint32_t right = org_x + width;
    uint32_t bottom = top + height;
    uint32_t border_size;
    uint32_t left_border;
    uint32_t right_border ;
    uint32_t top_lower;
    uint32_t bottom_upper;
    uint32_t frame_width;
    uint32_t fb_addr;
    uint16_t color;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    bottom = top + height;
    border_size = display_drv->pen_info.width;
    left_border = left + border_size;
    right_border = right - border_size;
    top_lower = top + border_size;
    bottom_upper = bottom - border_size;
    frame_width =  display_drv->vi_params.input_xres;
    fb_addr = display_drv->fb_addr;
    color = display_drv->pen_info.color;
    for (y_pos = top; y_pos < bottom; y_pos += y_unit)
    {
        if ((y_pos >= top && y_pos < top_lower) || 
            (y_pos >= bottom_upper))
        {
            for (x_pos = left; x_pos < right; x_pos += x_unit)
            {
                 lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
            }
        }
        else
        {
            for (x_pos = left; x_pos < left_border; x_pos += x_unit)
            {
                lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
            }
   
            for (x_pos = right_border; x_pos < right; x_pos += x_unit)
            {
                lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
            }
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_draw_line(kdrv_display_t *display_drv, uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye)
{    
    uint32_t x_pos, y_pos;
    uint32_t x_unit = 1;
    uint32_t y_unit = 1;
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t fb_addr;
    uint16_t color;
    uint16_t pen_width;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    frame_width = display_drv->vi_params.input_xres;
    frame_height = display_drv->vi_params.input_yres;
    fb_addr = display_drv->fb_addr;
    color = display_drv->pen_info.color;
    pen_width = display_drv->pen_info.width;
    if(xs == xe)
    {
        if(ys > ye)
            SWAP(ys, ye);

        for(y_pos = ys; y_pos <= ye; y_pos += y_unit)
        {
            for(x_pos = xs; (x_pos < xs+pen_width) && (x_pos < frame_width); x_pos += x_unit)
            {
                lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
            }
        }
        return KDRV_STATUS_OK;
    }
    else if(ys == ye)
    {
        if(xs > xe)
            SWAP(xs, xe);

        for(x_pos = xs; x_pos <= xe; x_pos += x_unit)
        {
            for(y_pos = ys; (y_pos < ys+pen_width) && (y_pos < frame_height); y_pos += y_unit)
            {
                lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
            }
        }
        return KDRV_STATUS_OK;
    }
    return KDRV_STATUS_INVALID_PARAM;
}

kdrv_status_t kdrv_display_fill_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height)
{    
    uint32_t x_pos, y_pos;
    uint32_t fb_addr;
    uint32_t frame_width;
    uint32_t frame_height;
    uint16_t color;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    fb_addr = display_drv->fb_addr;
    frame_width = display_drv->vi_params.input_xres;
    frame_height = display_drv->vi_params.input_yres;
    color = display_drv->pen_info.color;
    if((org_x >= frame_width)
    || (org_y >= frame_height)
    || ((org_x + width) > frame_width)
    || ((org_y + height) > frame_height))
    {
        return KDRV_STATUS_INVALID_PARAM;
    }
    
    for(y_pos=org_y; y_pos<(org_y+height); y_pos++)
    {
        for(x_pos=org_x; x_pos<(org_x+width); x_pos++)
        {
            lcdc_draw_pixel(fb_addr, x_pos, y_pos, frame_width, color);
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_draw_bitmap(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height, void* pBuf)
{
    uint8_t bpp = 2;
    uint32_t* buf = pBuf;
    uint32_t y_pos;
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t fb_addr;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;
    
    frame_width = display_drv->vi_params.input_xres;
    frame_height = display_drv->vi_params.input_yres;
    fb_addr = display_drv->fb_addr;
    if((org_x >= frame_width)
    || (org_y >= frame_height)
    || ((org_x + width) > frame_width)
    || ((org_y + height) > frame_height))
    {
        return KDRV_STATUS_INVALID_PARAM;
    }
    
    for(y_pos=org_y; y_pos<(org_y+height); y_pos++)
    {
        memcpy((void *)(fb_addr+((y_pos*frame_width+org_x)<<1)), (void *)buf, width*bpp);
        buf += width*bpp;
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_test_pattern_gen(kdrv_display_t *display_drv, bool pat_gen)
{
    test_pat_en = pat_gen;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_backlight(kdrv_display_t *display_drv, int light)
{
    uint32_t  duty;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    //PINMUX_PWM0_SET(PINMUX_PWM0);//update to kdrv_pinmux_config style with the same register setting
    //reg = GET_BITS(SCU_EXTREG_PWM0_IOCTRL, 0, 7);
    kdrv_pinmux_config(KDRV_PIN_PWM0,PIN_MODE_0,PIN_PULL_NONE,PIN_DRIVING_12MA);

    duty = (PWM0_FREQ_CNT * light ) / 100;

    kdrv_pwm_config(PWMTIMER1, PWM_POLARITY_NORMAL, duty, PWM0_FREQ_CNT, 0);
    kdrv_pwm_enable(PWMTIMER1);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_screen_control(kdrv_lcdc_screen_ctrl_t ctrl)
{
    if(ctrl > KDRV_LCDC_SCREEN_ON)
        return KDRV_STATUS_INVALID_PARAM;

    LCDC_REG_FUNC_ENABLE_SET_LCDon(ctrl);
    LCDC_REG_FUNC_ENABLE_SET_LCDen(ctrl);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_panel_type(kdrv_lcdc_panel_type_t type)
{
    if(type > KDRV_LCDC_8BIT_PER_CHANNEL)
        return KDRV_STATUS_INVALID_PARAM;

    LCDC_REG_PANEL_PIXEL_SET_PanelType(type);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_bgrsw(kdrv_lcdc_output_fmt_t format)
{
    if(format > KDRV_LCDC_OUTPUT_FMT_BGR)
        return KDRV_STATUS_INVALID_PARAM;

    LCDC_REG_PANEL_PIXEL_SET_BGRSW(format);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_pixel_sr(kdrv_lcdc_serial_pix_sr_t rotate)
{
    if(rotate > KDRV_LCDC_SERIAL_PIX_LSR)
        return KDRV_STATUS_INVALID_PARAM;

    LCDC_REG_SERIAL_PANEL_PIXEL_SET_LSR(rotate);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_pixel_colorseq(kdrv_lcdc_serial_pix_colorseq_t color)
{
    if(color > KDRV_LCDC_SERIAL_PIX_COLORSEQ_GBR)
        return KDRV_STATUS_INVALID_PARAM;

    LCDC_REG_SERIAL_PANEL_PIXEL_SET_ColorSeq(color);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_pixel_delta_type(kdrv_lcdc_serial_pix_delta_type_t type)
{
    if(type > KDRV_LCDC_SERIAL_PIX_DELTA_TYPE_DIFF_SEQ)
        return KDRV_STATUS_INVALID_PARAM;

    LCDC_REG_SERIAL_PANEL_PIXEL_SET_DeltaType(type);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_pixel_serial_mode(kdrv_lcdc_serial_pix_output_mode_t mode)
{
    if(mode > KDRV_LCDC_SERIAL_PIX_RGB_SERIAL_OUTPUT)
        return KDRV_STATUS_INVALID_PARAM;

    LCDC_REG_SERIAL_PANEL_PIXEL_SET_SerialMode(mode);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_endian(kdrv_lcdc_fb_data_endianness_t endian_type)
{
    if(endian_type > KDRV_LCDC_FB_DATA_ENDIAN_LBBP)
        return KDRV_STATUS_INVALID_PARAM;
    
    LCDC_REG_PANEL_PIXEL_SET_Endian(endian_type);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_auo052_mode(kdrv_lcdc_auo052_mode_t mode)
{
    if(mode > KDRV_LCDC_AUO052_ON)
        return KDRV_STATUS_INVALID_PARAM;
    
    LCDC_REG_SERIAL_PANEL_PIXEL_SET_AUO052(mode);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_bus_bandwidth_ctrl(uint32_t ctrl)
{
    outw(LCDC_REG_BANDWIDTH_CTRL, ctrl);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_down_scale(uint16_t hor_no_in, uint16_t hor_no_out, uint16_t ver_no_in, uint16_t ver_no_out)
{
#ifdef DOWN_SCALE_WITH_CROP
    //pick 640x360 from 640x480
    ver_no_in = (uin16_t)((float)hor_no_in * (float)ver_no_out / (float)hor_no_out );
#endif

    lcdc_msg("[%s] hor_no_in=%u ver_no_in=%u hor_no_out=%u ver_no_out=%u\n", 
        __func__, hor_no_in, ver_no_in, hor_no_out, ver_no_out);
    //for DOWN_SCALING
    if ((hor_no_in != hor_no_out) || (ver_no_in != ver_no_out)) {
        LCDC_REG_FUNC_ENABLE_SET_ScalerEn(1);  
        LCDC_REG_SCALER_HOR_RES_IN_SET_hor_no_in((hor_no_in - 1)); 
        LCDC_REG_SCALER_VER_RES_IN_SET_ver_no_in((ver_no_in - 1));
        LCDC_REG_SCALER_HOR_RES_OUT_SET_hor_no_out(hor_no_out);
        LCDC_REG_SCALER_VER_RES_OUT_SET_ver_no_out(ver_no_out);
        LCDC_REG_SCALER_MISC_SET_fir_sel(0);
        LCDC_REG_SCALER_MISC_SET_hor_inter_mode(0);
        LCDC_REG_SCALER_MISC_SET_ver_inter_mode(0);
        LCDC_REG_SCALER_MISC_SET_bypass_mode(0);
        //u32 scal_hor_num = mod((hor_no_in + 1) / hor_no_out) * 256 / hor_no_out;
        //u32 scal_ver_num = mod((ver_no_in + 1) / ver_no_out) * 256 / ver_no_out;
        LCDC_REG_SCALER_RES_SET_scal_hor_num(85);
#if LCDC_DOWN_SCALE_WITH_CROP == YES
        //keep ratio but with cropping
        LCDC_REG_SCALER_RES_SET_scal_ver_num(85);
#else
        //all are display but ratio is changed
        LCDC_REG_SCALER_RES_SET_scal_ver_num(195);
#endif
    }
    else {
        //reset the scalar control register	
        outw(LCDC_REG_SCALER_MISC, 0x0);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_framerate(int framerate, int width, int height)
{
    //Pixel Clock = H total * V total * Frame Rate
    //horizontal timing control
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HBP(1);
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HFP(1);
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HW(40);  
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_PL(((width >> 4) - 1));
    
    //vertical timing control
    LCDC_REG_VERTICAL_TIMING_CTRL_SET_VFP(2);
    LCDC_REG_VERTICAL_TIMING_CTRL_SET_VW(9);
    LCDC_REG_VERTICAL_TIMING_CTRL_SET_LF((height - 1)); 

    //vertical back porch
    LCDC_REG_VERTICAL_BACK_PORCH_SET_VBP(2);

    LCDC_REG_POLARITY_CTRL_SET_IDE(1);
    LCDC_REG_POLARITY_CTRL_SET_ICK(1);
    LCDC_REG_POLARITY_CTRL_SET_IHS(1);
    LCDC_REG_POLARITY_CTRL_SET_IVS(1);

#if 1
    LCDC_REG_POLARITY_CTRL_SET_DivNo(5);
#else /*ToDo : Verify flow*/
    uint32_t div;    
    uint32_t serial_mode = LCDC_REG_SERIAL_PANEL_PIXEL_GET_SerialMode();
    uint32_t upper_margin = LCDC_REG_VERTICAL_BACK_PORCH_GET_VBP() + 1;
    uint32_t lower_margin = LCDC_REG_VERTICAL_TIMING_CTRL_GET_VFP();
    uint32_t vsync_len = LCDC_REG_VERTICAL_TIMING_CTRL_GET_VW() + 1;
    uint32_t left_margin = LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HBP() + 1;
    uint32_t right_margin = LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HFP() + 1;
    uint32_t hsync_len = LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HW() + 1;
    uint32_t pixel_clock = (serial_mode ? 3 : 1 ) * framerate *
                      (height + upper_margin + lower_margin + vsync_len) *
                      (width + left_margin + right_margin + hsync_len);

    div = LCD_PLL / pixel_clock;    
    dbg_msg("[%s %d] left_margin=%d, right_margin=%d, hsync_len=%d, div=0x%x, pclk=%d", __func__, __LINE__, left_margin, right_margin, hsync_len, div, pixel_clock);
    if (div < 1) {
        return KDRV_STATUS_INVALID_PARAM;
    }
    if ((height == 1080) && (width == 1920)) {
        div = 0;
    } else if ((height == 480) && (width == 640)) {
        div = 1;
    } else {
        div = 5;
    }
    LCDC_REG_POLARITY_CTRL_SET_DivNo(div);
#endif
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_image_color_params(uint32_t color0, uint32_t color1, uint32_t color2, uint32_t color3)
{
    outw(LCDC_REG_COLOR_MGR_0, color0);
    outw(LCDC_REG_COLOR_MGR_1, color1);
    outw(LCDC_REG_COLOR_MGR_2, color2);
    outw(LCDC_REG_COLOR_MGR_3, color3);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcdc_set_frame_buffer(uint32_t img_scal_down)
{
    outw(LCDC_REG_FRAME_BUFFER, img_scal_down);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_frame_margin_len(uint16_t margin_len)
{
    disp_frame_margin_len = margin_len;
    return KDRV_STATUS_OK;
}

uint16_t kdrv_display_get_frame_margin_len(void)
{
    return disp_frame_margin_len;
}

kdrv_status_t kdrv_display_set_review_snapshot_en(bool enable)
{
    disp_en_review_snapshot = enable;
    return KDRV_STATUS_OK;
}

bool kdrv_display_get_review_snapshot_en(void)
{
    return disp_en_review_snapshot;
}

kdrv_status_t kdrv_display_set_snapshot_preview_en(bool enable)
{
    disp_en_snapshot_preview = enable;
    return KDRV_STATUS_OK;
}

bool kdrv_display_get_snapshot_preview_en(void)
{
    return disp_en_snapshot_preview;
}
