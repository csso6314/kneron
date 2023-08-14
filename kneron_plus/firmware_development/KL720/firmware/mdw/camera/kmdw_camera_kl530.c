/*
 * KDP Camera driver for KL530
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <string.h>
#include <stdio.h>
#include "kmdw_camera.h"
#include "kmdw_sensor.h"
#include "kdrv_fb_mgr.h"
#include "kdrv_clock.h"
#include "kdrv_mipicsirx.h"
#include "kdrv_camera.h"
#include "project.h"

//#define CAM_DEBUG

#ifdef CAM_DEBUG
#define cam_msg(fmt, ...) kmdw_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define cam_msg(fmt, ...)
#endif
#if defined SUPPORT_ISP && (SUPPORT_ISP == SUPPORT_ISP_ENABLE)
isp_para_ isp_init_para = {
    .run = {
        .bin_en         = ISP_RUN_BIN_EN,
        .cfg            = ISP_RUN_CFG,
    },
    .dma = {
        .wdma           = 3,
        .rdma           = 3,
        .all            = 15,
        .bl             = 16,
    },
    .rdma = {
        .sa             = ISP_RDMA_SA,
        .sa_t2          = ISP_RDMA_SA_T2,
    },
    .wdma = {
        {
            .sa         = ISP_WDMA_0_SA,
        },
        {
            .sa         = ISP_WDMA_1_SA,
        }
    },
    .bk_reg = {
        {//ISP_BANK_0
            .isize = {
                .row            = ISP_ISIZE_ROW_0,
                .col            = ISP_ISIZE_COL_0,
            },
            .bayer = {
                .mono           = ISP_BAYER_MONO_0,
                .hdr            = ISP_BAYER_HDR_0,
                .cfa            = ISP_BAYER_CFA_0,
            },
            .bls = {
                .k              = ISP_BLS_K_0,
            },
            .gain = {
                .b              = ISP_GAIN_B_0,
                .g              = ISP_GAIN_G_0,
                .r              = ISP_GAIN_R_0,
            },
            .fusion = {
                .t2_min         = ISP_FUSION_T2_MIN_0,
                .t1_max         = ISP_FUSION_T1_MAX_0,
                .mdth           = ISP_FUSION_MDTH_0,
                .t2_ratio       = ISP_FUSION_T2_RATIO_0,
                .t1_ratio       = ISP_FUSION_T1_RATIO_0,
            },
            .demo = {
                .weight         = ISP_DEMO_WEIGHT_0,
                .lpf            = ISP_DEMO_LPF_0,
            },
            .cm = {
                .c0             = (ISP_CM_C0_0),
                .c1             = ((uint32_t)ISP_CM_C1_0),
                .c2             = ((uint32_t)ISP_CM_C2_0),
                .c3             = ((uint32_t)ISP_CM_C3_0),
                .c4             = (ISP_CM_C4_0),
                .c5             = ((uint32_t)ISP_CM_C5_0),
                .c6             = ((uint32_t)ISP_CM_C6_0),
                .c7             = ((uint32_t)ISP_CM_C7_0),
                .c8             = (ISP_CM_C8_0),
            },
            .bias = {
                .k0             = ISP_BIAS_K0_0,
                .k1             = ISP_BIAS_K1_0,
                .k2             = ISP_BIAS_K2_0,
            },
            .gamma = {
                .p0             = ISP_GAMMA_P0_0,
                .p1             = ISP_GAMMA_P1_0,
                .p2             = ISP_GAMMA_P2_0,
                .p3             = ISP_GAMMA_P3_0,
                .p4             = ISP_GAMMA_P4_0,
                .p5             = ISP_GAMMA_P5_0,
                .p6             = ISP_GAMMA_P6_0,
                .p7             = ISP_GAMMA_P7_0,
                .p8             = ISP_GAMMA_P8_0,
                .p9             = ISP_GAMMA_P9_0,
                .p10            = ISP_GAMMA_P10_0,
                .p11            = ISP_GAMMA_P11_0,
                .p12            = ISP_GAMMA_P12_0,
                .p13            = ISP_GAMMA_P13_0,
                .p14            = ISP_GAMMA_P14_0,
                .p15            = ISP_GAMMA_P15_0,
                .p16            = ISP_GAMMA_P16_0,
            },
            .roi = {
                .we             = ISP_ROI_WE_0,
                .wb             = ISP_ROI_WB_0,
                .he             = ISP_ROI_HE_0,
                .hb             = ISP_ROI_HB_0,
            },
            .stats = {
                .sat_th         = ISP_STATS_SAT_TH_0,
                .dark_th        = ISP_STATS_DARK_TH_0,
            },
            .yhpf = {
                .high_th        = ISP_YHPF_HIGH_TH_0,
                .low_th         = ISP_YHPF_LOW_TH_0,
                .high_scale     = ISP_YHPF_HIGH_SCALE_0,
                .low_scale      = ISP_YHPF_LOW_SCALE_0,
            }
        },
        {//ISP_BANK_1
            .isize = {
                .row            = ISP_ISIZE_ROW_1,
                .col            = ISP_ISIZE_COL_1,
            },
            .bayer = {
                .mono           = ISP_BAYER_MONO_1,
                .hdr            = ISP_BAYER_HDR_1,
                .cfa            = ISP_BAYER_CFA_1,
            },
            .bls = {
                .k              = ISP_BLS_K_1,
            },
            .gain = {
                .b              = ISP_GAIN_B_1,
                .g              = ISP_GAIN_G_1,
                .r              = ISP_GAIN_R_1,
            },
            .fusion = {
                .t2_min         = ISP_FUSION_T2_MIN_1,
                .t1_max         = ISP_FUSION_T1_MAX_1,
                .mdth           = ISP_FUSION_MDTH_1,
                .t2_ratio       = ISP_FUSION_T2_RATIO_1,
                .t1_ratio       = ISP_FUSION_T1_RATIO_1,
            },
            .demo = {
                .weight         = ISP_DEMO_WEIGHT_1,
                .lpf            = ISP_DEMO_LPF_1,
            },
            .cm = {
                .c0             = (ISP_CM_C0_1),
                .c1             = ((uint32_t)ISP_CM_C1_1),
                .c2             = ((uint32_t)ISP_CM_C2_1),
                .c3             = ((uint32_t)ISP_CM_C3_1),
                .c4             = (ISP_CM_C4_1),
                .c5             = ((uint32_t)ISP_CM_C5_1),
                .c6             = ((uint32_t)ISP_CM_C6_1),
                .c7             = ((uint32_t)ISP_CM_C7_1),
                .c8             = (ISP_CM_C8_1),
            },
            .bias = {
                .k0             = ISP_BIAS_K0_1,
                .k1             = ISP_BIAS_K1_1,
                .k2             = ISP_BIAS_K2_1,
            },
            .gamma = {
                .p0             = ISP_GAMMA_P0_1,
                .p1             = ISP_GAMMA_P1_1,
                .p2             = ISP_GAMMA_P2_1,
                .p3             = ISP_GAMMA_P3_1,
                .p4             = ISP_GAMMA_P4_1,
                .p5             = ISP_GAMMA_P5_1,
                .p6             = ISP_GAMMA_P6_1,
                .p7             = ISP_GAMMA_P7_1,
                .p8             = ISP_GAMMA_P8_1,
                .p9             = ISP_GAMMA_P9_1,
                .p10            = ISP_GAMMA_P10_1,
                .p11            = ISP_GAMMA_P11_1,
                .p12            = ISP_GAMMA_P12_1,
                .p13            = ISP_GAMMA_P13_1,
                .p14            = ISP_GAMMA_P14_1,
                .p15            = ISP_GAMMA_P15_1,
                .p16            = ISP_GAMMA_P16_1,
            },
            .roi = {
                .we             = ISP_ROI_WE_1,
                .wb             = ISP_ROI_WB_1,
                .he             = ISP_ROI_HE_1,
                .hb             = ISP_ROI_HB_1,
            },
            .stats = {
                .sat_th         = ISP_STATS_SAT_TH_1,
                .dark_th        = ISP_STATS_DARK_TH_1,
            },
            .yhpf = {
                .high_th        = ISP_YHPF_HIGH_TH_1,
                .low_th         = ISP_YHPF_LOW_TH_1,
                .high_scale     = ISP_YHPF_HIGH_SCALE_1,
                .low_scale      = ISP_YHPF_LOW_SCALE_1,
            }
        }
    },
    .inited = false,
};
#endif
kmdw_cam_context cam_ctx[KDP_CAM_NUM] =
{
    {
        .cam_input_type             = IMGSRC_0_IN_PORT,
        .sensor_id                  = IMGSRC_0_SENSORID,
        .sensor_devaddress          = IMGSRC_0_DEV_ADDR,
        .i2c_port_id                = IMGSRC_0_PORT_ID,
        .tile_avg_en                = IMGSRC_0_TILE_AVG,
        .mipi_lane_num              = IMGSRC_0_MIPI_LANE,
        .fmt=
        {
            .width                  = IMGSRC_0_WIDTH,
            .height                 = IMGSRC_0_HEIGHT,
            .pixelformat            = IMGSRC_0_FORMAT,
        },
        .csi_para_ =
        {
            .timer_count_number     = CSIRX_0_TCN,
            .hs_rx_timeout_value    = CSIRX_0_HRTV,
            .mapping_control        = CSIRX_0_MCR,
            .vstu                   = CSIRX_0_VSTU,
            .vstr                   = CSIRX_0_VSTR,
            .vster                  = CSIRX_0_VSTER,
            .hstr                   = CSIRX_0_HSTR,
            .pftr                   = CSIRX_0_PFTR,
            .phy_settle_cnt         = CSIRX_0_SETTLE_CNT
        },
        #if defined SUPPORT_VI && (SUPPORT_VI == SUPPORT_VI_ENABLE)
        .vi_para=
        {/* VI_ID_0 */
            .cfg = {
                .origin         = VI_CFG_ORIGINAL_0,
            },
            .run = {
                .dpi            = 0,
                .force_stop     = VI_RUN_FORCE_STOP_0,
                .aec_en         = VI_RUN_AEC_EN_0,
                .roi_en         = VI_RUN_ROI_EN_0,
                .awb_en         = VI_RUN_AWB_EN_0,
                .fbb_en         = VI_RUN_FBB_EN_0,
                .bin_en         = VI_RUN_BIN_EN_0,
            },
            .dpi = {
                .sife           = VI_DPI_SIFE_0,
                .depth          = VI_DPI_DEPTH_0,
                .format         = VI_DPI_FORMAT_0,
                .row            = VI_DPI_ROW_0,
                .col            = VI_DPI_COL_0,
                .sad            = VI_DPI_SAD_0,
                .drop           = VI_DPI_DROP_0,
            },
            .sif = {
                .aps            = VI_SIF_APS_0,
                .ape            = VI_SIF_APE_0,
                .als            = VI_SIF_ALS_0,
                .ale            = VI_SIF_ALE_0,
                .update         = VI_SIF_UPDATE_0,
                .hsync_inv      = VI_SIF_HSYNC_INV_0,
                .vsync_inv      = VI_SIF_VSYNC_INV_0,
                .intlc          = VI_SIF_INTLC_0,
            },
            .bayer = {
                .dummy          = VI_BAYER_DUMMY_0,
                .hdr            = VI_BAYER_HDR_0,
                .cfa            = VI_BAYER_CFA_0,
            },
            .nir = {
                .advance        = VI_NIR_ADVANCE_0,
            },
            .aec = {
                .tile_size_sel  = VI_AEC_TILE_SIZE_SEL_0,
                .start_w        = VI_AEC_START_W_0,
                .start_h        = VI_AEC_START_H_0,
                .tile_num_w     = VI_AEC_TILE_NUM_W_0,
                .tile_num_h     = VI_AEC_TILE_NUM_H_0,
                .sat_th         = VI_AEC_SAT_TH_0,
                .dark_th        = VI_AEC_DARK_TH_0,
                .roi_we         = VI_AEC_ROI_WE_0,
                .roi_wb         = VI_AEC_ROI_WB_0,
                .roi_he         = VI_AEC_ROI_HE_0,
                .roi_hb         = VI_AEC_ROI_HB_0,
                .black          = VI_AEC_BLACK_0,
            },
            .awb = {
                .start_w        = VI_AWB_START_W_0,
                .start_h        = VI_AWB_START_H_0,
                .bscale         = VI_AWB_BSCALE_0,
                .rscale         = VI_AWB_RSCALE_0,
                .blk_num_w      = VI_AWB_BLK_NUM_W_0,
                .blk_num_h      = VI_AWB_BLK_NUM_H_0,
                .bbegin         = VI_AWB_BBEGIN_0,
                .rbegin         = VI_AWB_RBEGIN_0,
                .num            = VI_AWB_NUM_0,
                .start          = VI_AWB_START_0,
                .end            = VI_AWB_END_0,
            },
            .fbb = {
                .we             = VI_FBB_WE_0,
                .wb             = VI_FBB_WB_0,
                .he             = VI_FBB_HE_0,
                .hb             = VI_FBB_HB_0,
            },
            .dma = {
                .bl             = VI_DMA_BL_0,
                .wdma0 = {
                    .da         = VI_DMA_WDMA0_DA_0,
                    .pitch      = VI_DMA_WDMA0_PITCH_0,
                    .len        = VI_DMA_WDMA0_LEN_0,
                    .line       = VI_DMA_WDMA0_LINE_0,
                },
                .wdma1 = {
                    .da         = VI_DMA_WDMA1_DA_0,
                    .pitch      = VI_DMA_WDMA1_PITCH_0,
                    .len        = VI_DMA_WDMA1_LEN_0,
                    .line       = VI_DMA_WDMA1_LINE_0,
                },
                .wdma2 = {
                    .da0        = VI_DMA_WDMA2_DA0_0,
                    .len0       = VI_DMA_WDMA2_LEN0_0,
                    .da1        = VI_DMA_WDMA2_DA1_0,
                    .len1       = VI_DMA_WDMA2_LEN1_0,
                }
            },
            .ofs = {
                .aec_y_roi_ofs  = VI_DDR_STAT_AEC_OFS_0,
            },
            .stat_ddr_address   = 0,
            .inited = false,
        },
        #endif
    },
    {
        .cam_input_type             = IMGSRC_1_IN_PORT,
        .sensor_id                  = IMGSRC_1_SENSORID,
        .sensor_devaddress          = IMGSRC_1_DEV_ADDR,
        .i2c_port_id                = IMGSRC_1_PORT_ID,
        .tile_avg_en                = IMGSRC_1_TILE_AVG,
        .mipi_lane_num              = IMGSRC_1_MIPI_LANE,
        .fmt=
        {
            .width                  = IMGSRC_1_WIDTH,
            .height                 = IMGSRC_1_HEIGHT,
            .pixelformat            = IMGSRC_1_FORMAT,
        },
        .csi_para_ =
        {
            .timer_count_number     = CSIRX_1_TCN,
            .hs_rx_timeout_value    = CSIRX_1_HRTV,
            .mapping_control        = CSIRX_1_MCR,
            .vstu                   = CSIRX_1_VSTU,
            .vstr                   = CSIRX_1_VSTR,
            .vster                  = CSIRX_1_VSTER,
            .hstr                   = CSIRX_1_HSTR,
            .pftr                   = CSIRX_1_PFTR,
            .phy_settle_cnt         = CSIRX_1_SETTLE_CNT
        },
        #if defined SUPPORT_VI && (SUPPORT_VI == SUPPORT_VI_ENABLE)
        .vi_para =
        {/* VI_ID_1 */
            .cfg = {
                .origin             = VI_CFG_ORIGINAL_1,
            },
            .run = {
                .dpi            = 0,
                .force_stop     = VI_RUN_FORCE_STOP_1,
                .aec_en         = VI_RUN_AEC_EN_1,
                .roi_en         = VI_RUN_ROI_EN_1,
                .awb_en         = VI_RUN_AWB_EN_1,
                .fbb_en         = VI_RUN_FBB_EN_1,
                .bin_en         = VI_RUN_BIN_EN_1,
            },
            .dpi = {
                .sife           = VI_DPI_SIFE_1,
                .depth          = VI_DPI_DEPTH_1,
                .format         = VI_DPI_FORMAT_1,
                .row            = VI_DPI_ROW_1,
                .col            = VI_DPI_COL_1,
                .sad            = VI_DPI_SAD_1,
                .drop           = VI_DPI_DROP_1,
            },
            .sif = {
                .aps            = VI_SIF_APS_1,
                .ape            = VI_SIF_APE_1,
                .als            = VI_SIF_ALS_1,
                .ale            = VI_SIF_ALE_1,
                .update         = VI_SIF_UPDATE_1,
                .hsync_inv      = VI_SIF_HSYNC_INV_1,
                .vsync_inv      = VI_SIF_VSYNC_INV_1,
                .intlc          = VI_SIF_INTLC_1,
            },
            .bayer = {
                .dummy          = VI_BAYER_DUMMY_1,
                .hdr            = VI_BAYER_HDR_1,
                .cfa            = VI_BAYER_CFA_1,
            },
            .nir = {
                .advance        = VI_NIR_ADVANCE_1,
            },
            .aec = {
                .tile_size_sel  = VI_AEC_TILE_SIZE_SEL_1,
                .start_w        = VI_AEC_START_W_1,
                .start_h        = VI_AEC_START_H_1,
                .tile_num_w     = VI_AEC_TILE_NUM_W_1,
                .tile_num_h     = VI_AEC_TILE_NUM_H_1,
                .sat_th         = VI_AEC_SAT_TH_1,
                .dark_th        = VI_AEC_DARK_TH_1,
                .roi_we         = VI_AEC_ROI_WE_1,
                .roi_wb         = VI_AEC_ROI_WB_1,
                .roi_he         = VI_AEC_ROI_HE_1,
                .roi_hb         = VI_AEC_ROI_HB_1,
                .black          = VI_AEC_BLACK_1,
            },
            .awb = {
                .start_w        = VI_AWB_START_W_1,
                .start_h        = VI_AWB_START_H_1,
                .bscale         = VI_AWB_BSCALE_1,
                .rscale         = VI_AWB_RSCALE_1,
                .blk_num_w      = VI_AWB_BLK_NUM_W_1,
                .blk_num_h      = VI_AWB_BLK_NUM_H_1,
                .bbegin         = VI_AWB_BBEGIN_1,
                .rbegin         = VI_AWB_RBEGIN_1,
                .num            = VI_AWB_NUM_1,
                .start          = VI_AWB_START_1,
                .end            = VI_AWB_END_1,
            },
            .fbb = {
                .we             = VI_FBB_WE_1,
                .wb             = VI_FBB_WB_1,
                .he             = VI_FBB_HE_1,
                .hb             = VI_FBB_HB_1,
            },
            .dma = {
                .bl             = VI_DMA_BL_1,
                .wdma0 = {
                    .da         = VI_DMA_WDMA0_DA_1,
                    .pitch      = VI_DMA_WDMA0_PITCH_1,
                    .len        = VI_DMA_WDMA0_LEN_1,
                    .line       = VI_DMA_WDMA0_LINE_1,
                },
                .wdma1 = {
                    .da         = VI_DMA_WDMA1_DA_1,
                    .pitch      = VI_DMA_WDMA1_PITCH_1,
                    .len        = VI_DMA_WDMA1_LEN_1,
                    .line       = VI_DMA_WDMA1_LINE_1,
                },
                .wdma2 = {
                    .da0        = VI_DMA_WDMA2_DA0_1,
                    .len0       = VI_DMA_WDMA2_LEN0_1,
                    .da1        = VI_DMA_WDMA2_DA1_1,
                    .len1       = VI_DMA_WDMA2_LEN1_1,
                }
            },
            .ofs = {
                .aec_y_roi_ofs  = VI_DDR_STAT_AEC_OFS_1,
            },
            .stat_ddr_address   = 0,
            .inited = false,
        },
        #endif
    }
};

__weak kdrv_status_t kdrv_csirx_enable(uint32_t csirx_idx, cam_format *format, uint32_t vstr0, uint32_t vster, uint32_t pftr)
{
    return KDRV_STATUS_OK;
}
__weak kdrv_status_t kdrv_csirx_start(uint32_t csirx_idx, uint32_t num)
{
    return KDRV_STATUS_OK;
}
__weak kdrv_status_t kdrv_csirx_set_para(uint32_t csirx_idx, cam_format *format, csi_para* para)
{
    return KDRV_STATUS_OK;
}
/* API */
static kmdw_status_t kmdw_cam_set_cam_port(uint32_t cam_id, uint32_t input_port_type);
static kmdw_status_t kmdw_cam_open(uint32_t cam_id)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];

    kmdw_cam_set_cam_port(cam_id, NULL);

    if(ctx->cam_input_type != IMG_SRC_IN_PORT_MIPI)
        return KMDW_STATUS_OK;
    kmdw_sensor_init(cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_close(uint32_t cam_id)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];
    if(ctx->cam_input_type != IMG_SRC_IN_PORT_MIPI)
        return KMDW_STATUS_OK;
    cam_msg("cam: %d\n", cam_id);

    kdrv_clock_set_csiclk(cam_id, 0);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_query_capability(uint32_t cam_id, struct cam_capability *cap)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];

    cam_msg("cam: %d\n", cam_id);

    ctx->capabilities = CAP_VIDEO_CAPTURE | CAP_STREAMING | CAP_DEVICE_CAPS;

    strcpy(cap->driver, "kl530_camera");
    strcpy(cap->desc, "kl530_camera");
    cap->version = 0x00010001;
    cap->capabilities = ctx->capabilities;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_format(uint32_t cam_id, cam_format *format)
{
    kmdw_cam_context *ctx = &cam_ctx[cam_id];
    uint32_t bpp;

    ctx->fmt = *format;

    if (format->pixelformat == IMG_FORMAT_RGB565)
        bpp = 2;
    else if (format->pixelformat == IMG_FORMAT_RAW8)
    {
        if(ctx->cam_input_type == IMG_SRC_IN_PORT_DPI)
            bpp = 2;
        else
            bpp = 1;
    }
    //if(ctx->sensor_id == SENSOR_ID_IRS2877C)
    //    ctx->fmt.sizeimage = format->width * format->height * 2;
    //else
    ctx->fmt.sizeimage = format->width * format->height * bpp;

    cam_msg("cam %d: w=%d h=%d p=0x%x f=%d b=%d s=%d c=%d\n", cam_id,
            ctx->fmt.width, ctx->fmt.height, ctx->fmt.pixelformat, ctx->fmt.field,
            ctx->fmt.bytesperline, ctx->fmt.sizeimage, ctx->fmt.colorspace);


    if(ctx->cam_input_type == IMG_SRC_IN_PORT_MIPI)
        kdrv_csirx_set_para( cam_id, &ctx->fmt, &ctx->csi_para_);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_get_format(uint32_t cam_id, cam_format *format)
{
    struct kmdw_cam_context *ctx = &cam_ctx[cam_id];

    cam_msg("cam: %d\n", cam_id);

    *format = ctx->fmt;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_init(uint32_t cam_id, uint32_t num_buf, uint32_t buffers[])
{
    cam_msg("cam %d: size=%d\n", cam_id, ctx->fmt.sizeimage);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_start_capture(uint32_t cam_id, kmdw_camera_callback_t img_cb)
{
    cam_msg("cam: %d\n", cam_id);
    
    if(cam_ctx[cam_id].cam_input_type == IMG_SRC_IN_PORT_MIPI)
        kdrv_csirx_start(cam_id, cam_ctx[cam_id].mipi_lane_num);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stop_capture(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_prepare(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_buffer_capture(uint32_t cam_id, uint32_t *addr, uint32_t *size)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stream_on(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_stream_off(uint32_t cam_id)
{
    cam_msg("cam: %d\n", cam_id);
    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_gain(uint32_t cam_id, uint32_t gain1, uint32_t gain2)
{
    cam_msg("cam: %d: gain1 %d, gain2 %d\n", cam_id, gain1, gain2);

    return kmdw_sensor_set_gain(cam_id, gain1, gain2);
}

static kmdw_status_t kmdw_cam_set_aec(uint32_t cam_id, struct cam_sensor_aec *aec_p)
{
    cam_msg("cam: %d\n", cam_id);

    return kmdw_sensor_set_aec(cam_id, aec_p);
}

static kmdw_status_t kmdw_cam_set_exp_time(uint32_t cam_id, uint32_t gain1, uint32_t gain2)
{
    cam_msg("cam: %d: gain1 %d, gain2 %d\n", cam_id, gain1, gain2);

    return kmdw_sensor_set_exp_time(cam_id, gain1, gain2);
}

static kmdw_status_t kmdw_cam_get_lux(uint32_t cam_id, uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t *global_gain, uint8_t *y_average)
{
    cam_msg("cam: %d\n", cam_id);

    return kmdw_sensor_get_lux(cam_id, expo, pre_gain, post_gain, global_gain, y_average);
}

static kmdw_status_t kmdw_cam_led_switch(uint32_t cam_id, uint32_t on)
{
    cam_msg("cam: %d\n", cam_id);

    return kmdw_sensor_led_switch(cam_id, on);
}

static kmdw_status_t kmdw_cam_set_mirror(uint32_t cam_id, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_mirror(cam_id, enable);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_flip(uint32_t cam_id, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_flip(cam_id, enable);

    return KMDW_STATUS_OK;
}

static uint32_t kmdw_cam_get_device_id(uint32_t cam_id)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    return kmdw_sensor_get_dev_id(cam_id);
}

static kmdw_status_t kmdw_cam_get_expo(uint32_t cam_id)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    return kmdw_sensor_get_expo(cam_id);
}

static kmdw_status_t kmdw_cam_set_inc(uint32_t cam_id, uint32_t enable)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_inc(cam_id, enable);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_devaddress(uint32_t cam_id, uint32_t address, uint32_t port_id)
{
    cam_msg("[%s] cam: %d\n", __func__, cam_id);

    kmdw_sensor_set_devaddress(cam_id, address, port_id);

    return KMDW_STATUS_OK;
}

static kmdw_status_t kmdw_cam_set_clock()
{
    return KMDW_STATUS_OK;
}
static kmdw_status_t kmdw_cam_set_cam_port(uint32_t cam_id, uint32_t input_port_type)
{
    SET_MISC(VI0_SEL, 0);
    SET_MISC(VI1_SEL, 1);
    return KMDW_STATUS_OK;
}

cam_ops camera_ops = {
    .open               = kmdw_cam_open,
    .close              = kmdw_cam_close,
    .query_capability   = kmdw_cam_query_capability,
    .set_format         = kmdw_cam_set_format,
    .get_format         = kmdw_cam_get_format,
    .buffer_init        = kmdw_cam_buffer_init,
    .start_capture      = kmdw_cam_start_capture,
    .stop_capture       = kmdw_cam_stop_capture,
    .buffer_prepare     = kmdw_cam_buffer_prepare,
    .buffer_capture     = kmdw_cam_buffer_capture,
    .stream_on          = kmdw_cam_stream_on,
    .stream_off         = kmdw_cam_stream_off,
    .set_gain           = kmdw_cam_set_gain,
    .set_aec            = kmdw_cam_set_aec,
    .set_exp_time       = kmdw_cam_set_exp_time,
    .get_lux            = kmdw_cam_get_lux,
    .led_switch         = kmdw_cam_led_switch,
    .set_mirror         = kmdw_cam_set_mirror,
    .set_flip           = kmdw_cam_set_flip,
    .get_device_id      = kmdw_cam_get_device_id,
    .get_expo           = kmdw_cam_get_expo,
    .set_inc            = kmdw_cam_set_inc,
    .set_addr           = kmdw_cam_set_devaddress,
    .set_clock          = kmdw_cam_set_clock,
    .set_cam_port       = NULL
};

