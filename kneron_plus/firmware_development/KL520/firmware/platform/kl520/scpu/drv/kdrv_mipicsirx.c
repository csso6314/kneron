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
*  kdrv_mipicsirx.c
*
*  Project:
*  --------
*  KL520
*
*  Description:
*  ------------
*  This is MIPI CSIRX driver
*
*  Author:
*  -------
*  Albert Chen
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

// Sec 0: Comment block of the file

// Sec 1: Include File
#include "board.h"
#include "kmdw_camera.h"
#include "kdrv_mipicsirx.h"
#include "kdrv_clock.h"
#include "kdrv_scu_ext.h"

#define ROUND_UP(x, y) ((((x) + (y - 1)) / y) * y)
#define CSI2RX_REG_VIDR         0x00
#define CSI2RX_REG_DIDR         0x01
#define CSI2RX_REG_CR           0x04 //Control Register
#define CSI2RX_REG_VSCR         0x05 //DPI V Sync Control Register
#define CSI2RX_REG_ECR          0x06
#define CSI2RX_REG_TCNR         0x08
#define CSI2RX_REG_HRTVR        0x0A //HS RX Timeout Value Register
#define CSI2RX_REG_FIUR         0x0B
#define CSI2RX_REG_ITR          0x12 //Initialization Timer Register
#define CSI2RX_REG_VSTR0        0x14 //DPI VC0 V Sync Timing Register
#define CSI2RX_REG_HSTR0        0x15 //DPI VC0 H Sync Timing Register
#define CSI2RX_REG_VSTR1        0x16
#define CSI2RX_REG_HSTR1        0x17
#define CSI2RX_REG_VSTR2        0x18
#define CSI2RX_REG_HSTR2        0x19
#define CSI2RX_REG_VSTR3        0x1A
#define CSI2RX_REG_HSTR3        0x1B
#define CSI2RX_REG_MCR          0x1C //DPI Mapping Control Register
#define CSI2RX_REG_VSTER        0x1E
#define CSI2RX_REG_HPNR         0x20 //DPI Horizontal Pixel Number
#define CSI2RX_REG_PECR         0x28 //PPI Enable Control Register
#define CSI2RX_REG_DLMR         0x2A
#define CSI2RX_REG_CSIERR       0x30
#define CSI2RX_REG_INTSTS       0x33
#define CSI2RX_REG_ESR          0x34
#define CSI2RX_REG_DPISR        0x38
#define CSI2RX_REG_INTER        0x3C
#define CSI2RX_REG_FFR          0x3D
#define CSI2RX_REG_DPCMR        0x48
#define CSI2RX_REG_FRR          0x4C
#define CSI2RX_REG_PFTR         0x50
#define CSI2RX_REG_PUDTR        0x58
#define CSI2RX_REG_FRCR         0x80
#define CSI2RX_REG_FNR          0x88
#define CSI2RX_REG_BPGLR        0x90
#define CSI2RX_REG_BPGHR        0x91

#define MAX_VIDEO_MEM           8 << 20

#define CSIRX_0_BASE            CSIRX_FTCSIRX100_PA_BASE
#define CSIRX_1_BASE            CSIRX_FTCSIRX100_1_PA_BASE

/* DPI V Sync Control Register (VSCR, Address = 0x05) */
#define CSIRX_REG_VSCR_SET_VSTU0(addr, val) SET_MASKED_BIT(addr, val, 0)

/* PPI Enable Control Register (PECR, Address = 0x28) */
#define CSIRX_REG_PECR_GET_PEC(addr)        GET_BITS(addr, 0, 7)
#define CSIRX_REG_PECR_SET_PEC(addr, val)   SET_MASKED_BITS(addr, val, 0, 7)

/* Data Lane Mapping Register 0 */
#define CSIRX_REG_0_DLMR0                   (CSIRX_0_BASE + 0x2A)
#define CSIRX_REG_0_DLMR0_GET_L1()          GET_BITS(CSIRX_REG_0_DLMR0, 4, 6)
#define CSIRX_REG_0_DLMR0_GET_L0()          GET_BITS(CSIRX_REG_0_DLMR0, 0, 2)
#define CSIRX_REG_1_DLMR0                   (CSIRX_1_BASE + 0x2A)
#define CSIRX_REG_1_DLMR0_GET_L1()          GET_BITS(CSIRX_REG_1_DLMR0, 4, 6)
#define CSIRX_REG_1_DLMR0_GET_L0()          GET_BITS(CSIRX_REG_1_DLMR0, 0, 2)


#define CSIRX_REG_0_FEATURE0                (CSIRX_0_BASE + 0x40)
#define CSIRX_REG_1_FEATURE0                (CSIRX_1_BASE + 0x40)
/* DPI FIFO Address Depth */
#define CSIRX_REG_0_FEATURE0_GET_DFAD()     GET_BITS(CSIRX_REG_0_FEATURE0, 5, 7)
#define CSIRX_REG_1_FEATURE0_GET_DFAD()     GET_BITS(CSIRX_REG_1_FEATURE0, 5, 7)
/* Lane Number */
#define CSIRX_REG_0_FEATURE0_GET_LN()       GET_BITS(CSIRX_REG_0_FEATURE0, 2, 4)
#define CSIRX_REG_1_FEATURE0_GET_LN()       GET_BITS(CSIRX_REG_1_FEATURE0, 2, 4)
/* Virtual channel number */
#define CSIRX_REG_0_FEATURE0_GET_VCN()      GET_BITS(CSIRX_REG_0_FEATURE0, 0, 1)
#define CSIRX_REG_1_FEATURE0_GET_VCN()      GET_BITS(CSIRX_REG_1_FEATURE0, 0, 1)

#define CSIRX_REG_0_FEATURE6                (CSIRX_0_BASE + 0x46)
#define CSIRX_REG_1_FEATURE6                (CSIRX_1_BASE + 0x46)
/* ASC Indicator */
#define CSIRX_REG_0_FEATURE6_GET_ASC()      GET_BIT(CSIRX_REG_0_FEATURE6, 7)
#define CSIRX_REG_1_FEATURE6_GET_ASC()      GET_BIT(CSIRX_REG_1_FEATURE6, 7)
/* APB Indicator */
#define CSIRX_REG_0_FEATURE6_GET_APB()      GET_BIT(CSIRX_REG_0_FEATURE6, 6)
#define CSIRX_REG_1_FEATURE6_GET_APB()      GET_BIT(CSIRX_REG_1_FEATURE6, 6)
/* I2C Indicator */
#define CSIRX_REG_0_FEATURE6_GET_I2C()      GET_BIT(CSIRX_REG_0_FEATURE6, 5)
#define CSIRX_REG_1_FEATURE6_GET_I2C()      GET_BIT(CSIRX_REG_1_FEATURE6, 5)
/* DPCM_EN Indicator */
#define CSIRX_REG_0_FEATURE6_GET_DPCM()     GET_BIT(CSIRX_REG_0_FEATURE6, 4)
#define CSIRX_REG_1_FEATURE6_GET_DPCM()     GET_BIT(CSIRX_REG_1_FEATURE6, 4)
/* CHK_DATA Indicator */
#define CSIRX_REG_0_FEATURE6_GET_CD()       GET_BIT(CSIRX_REG_0_FEATURE6, 3)
#define CSIRX_REG_1_FEATURE6_GET_CD()       GET_BIT(CSIRX_REG_1_FEATURE6, 3)
/* DUAL_PIXEL Indicator */
#define CSIRX_REG_0_FEATURE6_GET_DP()       GET_BIT(CSIRX_REG_0_FEATURE6, 2)
#define CSIRX_REG_1_FEATURE6_GET_DP()       GET_BIT(CSIRX_REG_1_FEATURE6, 2)
/* FRC Indicator */
#define CSIRX_REG_0_FEATURE6_GET_FRC()      GET_BIT(CSIRX_REG_0_FEATURE6, 1)
#define CSIRX_REG_1_FEATURE6_GET_FRC()      GET_BIT(CSIRX_REG_1_FEATURE6, 1)
/* DPI_PG Indicator */
#define CSIRX_REG_0_FEATURE6_GET_PG()       GET_BIT(CSIRX_REG_0_FEATURE6, 0)
#define CSIRX_REG_1_FEATURE6_GET_PG()       GET_BIT(CSIRX_REG_1_FEATURE6, 0)

#define CSIRX_REG_0_FEATURE7                (CSIRX_0_BASE + 0x47)
#define CSIRX_REG_1_FEATURE7                (CSIRX_1_BASE + 0x47)
/* OCTA_PIXEL Indicator */
#define CSIRX_REG_0_FEATURE7_GET_OP()       GET_BIT(CSIRX_REG_0_FEATURE7, 4)
#define CSIRX_REG_1_FEATURE7_GET_OP()       GET_BIT(CSIRX_REG_1_FEATURE7, 4)
/* MONO_RGB_EN Indicator */
#define CSIRX_REG_0_FEATURE7_GET_MR()       GET_BIT(CSIRX_REG_0_FEATURE7, 3)
#define CSIRX_REG_1_FEATURE7_GET_MR()       GET_BIT(CSIRX_REG_1_FEATURE7, 3)
/* CRC_CHK Indicator */
#define CSIRX_REG_0_FEATURE7_GET_CRC()      GET_BIT(CSIRX_REG_0_FEATURE7, 2)
#define CSIRX_REG_1_FEATURE7_GET_CRC()      GET_BIT(CSIRX_REG_1_FEATURE7, 2)
/* LANE_SWAP Indicator */
#define CSIRX_REG_0_FEATURE7_GET_SWAP()     GET_BIT(CSIRX_REG_0_FEATURE7, 1)
#define CSIRX_REG_1_FEATURE7_GET_SWAP()     GET_BIT(CSIRX_REG_1_FEATURE7, 1)
/* QUAD_PIXEL Indicator */
#define CSIRX_REG_0_FEATURE7_GET_QP()       GET_BIT(CSIRX_REG_0_FEATURE7, 0)
#define CSIRX_REG_1_FEATURE7_GET_QP()       GET_BIT(CSIRX_REG_1_FEATURE7, 0)


#define CSIRX_REG_0_BPGLR0                  (CSIRX_0_BASE + 0x90)
#define CSIRX_REG_1_BPGLR0                  (CSIRX_1_BASE + 0x90)
#define CSIRX_REG_0_BPGLR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_0_BPGLR0, val, 0, 7)
#define CSIRX_REG_1_BPGLR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_1_BPGLR0, val, 0, 7)

#define CSIRX_REG_0_BPGHR0                  (CSIRX_0_BASE + 0x91)
#define CSIRX_REG_1_BPGHR0                  (CSIRX_1_BASE + 0x91)
#define CSIRX_REG_0_BPGHR0_SET_PT(val)      SET_MASKED_BITS(CSIRX_REG_0_BPGHR0, val, 6, 7)
#define CSIRX_REG_1_BPGHR0_SET_PT(val)      SET_MASKED_BITS(CSIRX_REG_1_BPGHR0, val, 6, 7)
#define CSIRX_REG_0_BPGHR0_SET_PS(val)      SET_MASKED_BIT(CSIRX_REG_0_BPGHR0, val, 5)
#define CSIRX_REG_1_BPGHR0_SET_PS(val)      SET_MASKED_BIT(CSIRX_REG_1_BPGHR0, val, 5)
#define CSIRX_REG_0_BPGHR0_SET_GE(val)      SET_MASKED_BIT(CSIRX_REG_0_BPGHR0, val, 4)
#define CSIRX_REG_1_BPGHR0_SET_GE(val)      SET_MASKED_BIT(CSIRX_REG_1_BPGHR0, val, 4)
#define CSIRX_REG_0_BPGHR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_0_BPGHR0, val, 0, 3)
#define CSIRX_REG_1_BPGHR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_1_BPGHR0, val, 0, 3)

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
struct kdrv_csirx_context
{
    uint32_t csi_rx_base;
    uint32_t mipi_rx_phy_csr;
    uint32_t mipi_lane_num;
};

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable
struct kdrv_csirx_context csirx_ctx[CSI2RX_CAM_NUM];

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
kdrv_status_t kdrv_csi2rx_initialize(uint32_t cam_idx)
{
    if(cam_idx == CSI2RX_CAM_0)
    {
        csirx_ctx[cam_idx].csi_rx_base = CSIRX_FTCSIRX100_PA_BASE;
        csirx_ctx[cam_idx].mipi_rx_phy_csr = MIPIRX_PHY_CSR_PA_BASE;
        csirx_ctx[cam_idx].mipi_lane_num = 2;
        SCU_EXTREG_MISC_SET_DPI_MUX_0_sel(0);
    }
    else if(cam_idx == CSI2RX_CAM_1)
    {
        csirx_ctx[cam_idx].csi_rx_base = CSIRX_FTCSIRX100_1_PA_BASE;
        csirx_ctx[cam_idx].mipi_rx_phy_csr = MIPIRX_PHY_CSR_1_PA_BASE;
        csirx_ctx[cam_idx].mipi_lane_num = 2;
        SCU_EXTREG_MISC_SET_DPI_MUX_1_sel(0);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_csi2rx_enable(uint32_t input_type, uint32_t cam_idx, uint32_t sensor_idx, struct cam_format* fmt)
{
    uint32_t csirx_base = csirx_ctx[cam_idx].csi_rx_base;
    int32_t val, width;
    if(input_type != IMGSRC_IN_PORT_MIPI)
        return KDRV_STATUS_OK;
    width = ROUND_UP(fmt->width, 4);
    outw(csirx_base + CSI2RX_REG_HPNR , width);

    masked_outw(csirx_base + CSI2RX_REG_CR, 0x0d, 0xff);
    masked_outw(csirx_base + CSI2RX_REG_MCR, 0x00, 0xff);
    masked_outw(csirx_base + CSI2RX_REG_VSCR, 0x03, 0x03);
    masked_outw(csirx_base + CSI2RX_REG_VSTR0, 0xff, 0xff);
    masked_outw(csirx_base + CSI2RX_REG_VSTR1, 0xff, 0xff);

#ifdef SOURCE_FROM_PATTERN
    CSIRX_REG_PECR_SET_PEC(csirx_base + CSI2RX_REG_PECR, 1);
    val = (inw(csirx_base + CSI2RX_REG_BPGLR) & 0xFF);
    cam_msg("CSI2RX_REG_BPGLR val=%x\n", val);
    val = (inw(csirx_base + CSI2RX_REG_BPGHR) & 0x0F);

    switch (fmt->pixelformat) {
    case V2K_PIX_FMT_RGB565:
        val |= 0x40;
        break;
    case V2K_PIX_FMT_RAW10:
        val |= 0x40;
        break;
    case V2K_PIX_FMT_RAW8:
        val |= 0;
        break;
    }
    outw(csirx_base + CSI2RX_REG_BPGHR, val);
    val = inw(csirx_base + CSI2RX_REG_BPGHR);
#endif

    switch (sensor_idx)
    {
        case SENSOR_ID_OV9286:
        case SENSOR_ID_HMXRICA:
        case SENSOR_ID_SC132GS:
            val = 245;
            outw(csirx_base + CSI2RX_REG_PFTR , val);
            break;
        case SENSOR_ID_GC2145:
            CSIRX_REG_VSCR_SET_VSTU0(csirx_base, 1);
            break;
        default:;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_csi2rx_start(uint32_t input_type, uint32_t cam_idx)
{
    uint32_t i;
    uint32_t val;
    if(input_type != IMGSRC_IN_PORT_MIPI)
        return KDRV_STATUS_OK;
    val = 0;
    for (i = 0; i < csirx_ctx[cam_idx].mipi_lane_num; i++)
        val |= BIT0 << i;

    CSIRX_REG_PECR_SET_PEC(csirx_ctx[cam_idx].csi_rx_base + CSI2RX_REG_PECR, val);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_csi2rx_set_power(uint32_t cam_idx, uint32_t on)
{
    uint32_t mask, val = 0;

    if (cam_idx == CSI2RX_CAM_0)
    {
        mask = (SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable);
        if (on)
        {
            val = mask;
        }
        masked_outw(SCU_EXTREG_CSIRX_CTRL0, val, mask);
    }
    else if (cam_idx == CSI2RX_CAM_1)
    {
        mask = (SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable);
        if (on)
        {
            val = mask;
        }
        masked_outw(SCU_EXTREG_CSIRX_CTRL1, val, mask);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_csi2rx_reset(uint32_t cam_idx, uint32_t sensor_idx)
{
    uint32_t mask, val;
    uint32_t mipi_rx_phy_csr = csirx_ctx[cam_idx].mipi_rx_phy_csr;

    if (cam_idx == CSI2RX_CAM_0)
    {
        mask = SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable |
                SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                SCU_EXTREG_CSIRX_CTRL0_sys_rst_n;
        val = mask;
        masked_outw(SCU_EXTREG_CSIRX_CTRL0, val, mask);

        if (sensor_idx == SENSOR_ID_GC2145)
            outb(mipi_rx_phy_csr + 0x11, 0x7);
        else
            outb(mipi_rx_phy_csr + 0x11, 0x3);
    }
    else if (cam_idx == CSI2RX_CAM_1)
    {
        mask = SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable |
                SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                SCU_EXTREG_CSIRX_CTRL1_sys_rst_n;
        val = mask;
        masked_outw(SCU_EXTREG_CSIRX_CTRL1, val, mask);

        if (sensor_idx == SENSOR_ID_GC2145)
            outb(mipi_rx_phy_csr + 0x11, 0x7);
        else
            outb(mipi_rx_phy_csr + 0x11, 0x3);
    }
    return KDRV_STATUS_OK;
}
