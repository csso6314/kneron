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
*  KL720
*
*  Description:
*  ------------
*  This is MIPI CSIRX driver
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
#include "kdrv_mipicsirx.h"
#include "kdrv_clock.h"
#include "kdrv_scu_ext.h"
//#include "device_ctrl.h"
#include "kdrv_io.h"
#define  MIPI_DGB
#ifdef MIPI_DGB
#define mipi_msg(fmt, ...) printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define mipi_msg(fmt, ...)
#endif
#define MIPI_LANE_NUM_MAX       2

#define ROUND_UP(x, y) ((((x) + (y - 1)) / y) * y)
#define CSI2RX_REG_VIDR         0x00
#define CSI2RX_REG_DIDR         0x01
#define CSI2RX_REG_CR           0x04 //Control Register
#define CSI2RX_REG_VSCR         0x05 //DPI V Sync Control Register
#define CSI2RX_REG_ECR          0x06
#define CSI2RX_REG_TCNR         0x08 //Time Count number low reister
#define CSI2RX_REG_TCNHR        0x09 //Time Count number high reister
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
#define CSI2RX_REG_HPNHR        0x21 //DPI Horizontal Pixel Number High Register, VC0
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

/* Control register (CR, Address = 0x04) */
#define CSIRX_REG_CR_ECCCE                  BIT3 // ECC Checking Enable
#define CSIRX_REG_CR_CRCCE                  BIT2 // CRC Checking Enable
#define CSIRX_REG_CR_TME                    BIT1
#define CSIRX_REG_CR_SR                     BIT0 // Software reset

/* DPI V Sync Control Register (VSCR, Address = 0x05) */
#define CSIRX_REG_VSCR_SET_VSTU0(addr, val) SET_MASKED_BIT(addr, val, 0)
#define CSIRX_REG_VSCR_VSTU0                BIT0 // VSync Time unit for VC0
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
typedef enum {
    CSIRX_0,
    CSIRX_1,
    CSIRX_NUM,    // = IMGSRC_NUM
}csirx_opt;
typedef volatile union U_regCSIRX
{
    struct
    {
        uint8_t VIDR;       //(0x00)VendorIDRegister
        uint8_t DIDR;       //(0x01)DeviceIDRegister
        uint8_t RSVD_0;     //(0x02)
        uint8_t RSVD_1;     //(0x03)
        uint8_t CR;         //(0x04)ControlRegister
        uint8_t VSCR;       //(0x05)DPIVSyncControlRegister
        uint8_t ECR;        //(0x06)ExtendedControlRegister
        uint8_t RSVD_2;     //(0x07)
        uint8_t TCNR;       //(0x08)TimerCountNumberRegister
        uint8_t RSVD_3;     //(0x09)
        uint8_t HRTVR;      //(0x0A)HSRXTimeoutValueRegister
        uint8_t FIUR;       //(0x0B)FTCInternalUseRegister
        uint8_t RSVD_4[6];  //(0x0C ~ 0x11)
        uint8_t ITR;        //(0x12)InitializationTimerRegister
        uint8_t RSVD_5;     //(0x13)
        uint8_t VSTR0;      //(0x14)DPIVC0VSyncTimingRegister
        uint8_t HSTR0;      //(0x15)DPIVC0HSyncTimingRegister
        uint8_t VSTR1;      //(0x16)DPIVC1VSyncTimingRegister
        uint8_t HSTR1;      //(0x17)DPIVC1HSyncTimingRegister
        uint8_t VSTR2;      //(0x18)DPIVC2VSyncTimingRegister
        uint8_t HSTR2;      //(0x19)DPIVC2HSyncTimingRegister
        uint8_t VSTR3;      //(0x1A)DPIVC3VSyncTimingRegister
        uint8_t HSTR3;      //(0x1B)DPIVC3HSyncTimingRegister
        uint8_t MCR;        //(0x1C)DPIMappingControlRegister
        uint8_t RSVD_6;     //(0x1D)
        uint8_t VSTER;      //(0x1E)DPIVSyncTimingExtendedRegister
        uint8_t RSVD_7;     //(0x1F)
        uint8_t HPNR;       //(0x20)DPIHorizontalPixelNumberRegister
        uint8_t RSVD_8[7];  //(0x21~0x27)
        uint8_t PECR;       //(0x28)PPIEnableControlRegister
        uint8_t RSVD_9;     //(0x29)
        uint8_t DLMR;       //(0x2A)DataLaneMappingRegister
        uint8_t RSVD_10[5]; //(0x2B~2F)
        uint8_t CSIERR;     //(0x30)CSIErrorReportRegister
        uint8_t RSVD_11[2]; //(0x31~32)
        uint8_t INTSTS;     //(0x33)InterruptStatusregister
        uint8_t ESR;        //(0x34)EscapeModeandStopStateRegister
        uint8_t RSVD_12[3]; //(0x35~37)
        uint8_t DPISR;      //(0x38)DPIStatusRegister
        uint8_t RSVD_13[3]; //(0x39~3B)
        uint8_t INTER;      //(0x3C)InterruptEnableRegister
        uint8_t FFR;        //(0x3D)FTCSIRX100FeatureRegister
        uint8_t RSVD_14[10];//(0x3E~47)
        uint8_t DPCMR;      //(0x48)DPCMRegister
        uint8_t RSVD_15[3]; //(0x49~4B)
        uint8_t FRR;        //(0x4C)FTCSIRX100RevisionRegister
        uint8_t RSVD_16[3]; //(0x4D~4F)
        uint8_t PFTR;       //(0x50)PixelFIFOThresholdRegister
        uint8_t RSVD_17[7]; //(0x51~57)
        uint8_t PUDTR;      //(0x58)ProgrammableUserDefined8-bitData
        uint8_t RSVD_18[39];//(0x59~7F)
        uint8_t FRCR;       //(0x80)FrameRateControlRegister
        uint8_t RSVD_19[7]; //(0x81~87)
        uint8_t FNR;        //(0x88)FrameNumberRegister
        uint8_t RSVD_20[7]; //(0x89~8F)
        uint8_t BPGR;       //(0x90)DPI Built-in Pattern Generator Register
    }byte;    //byte
    struct
    {
        //(0x00)VendorIDRegister
        uint8_t VID         : 8;
        //(0x01)DeviceIDRegister
        uint8_t DID         : 8;
        //(0x02)
        uint8_t RSVD_0_     : 8;
        //(0x03)
        uint8_t RSVD_1_     : 8;
        //(0x04)ControlRegister
        uint8_t SR          : 1;
        uint8_t TME         : 1;
        uint8_t CRCCE       : 1;
        uint8_t ECCCE       : 1;
        uint8_t EAPBL       : 1;
        uint8_t HEG         : 1;
        uint8_t RSVD_1_1    : 2;
        //(0x05)DPIVSyncControlRegister
        uint8_t VSTU0       : 1;
        uint8_t VSTU1       : 1;
        uint8_t VSTU2       : 1;
        uint8_t VSTU3       : 1;
        uint8_t VSPC0       : 1;
        uint8_t VSPC1       : 1;
        uint8_t VSPC2       : 1;
        uint8_t VSPC3       : 1;
        //(0x06)ExtendedControlRegister
        uint8_t PCE         : 1;
        uint8_t SF          : 1;
        uint8_t VCID        : 1;
        uint8_t RSVD_1_2    : 5;
        //(0x07)
        uint8_t RSVD_2      : 8;
        //(0x08)TimerCountNumberRegister
        uint8_t TCN         : 8;
        //(0x09)
        uint8_t TCNH        : 2;
        uint8_t RSVD_2_1    : 6;
        //(0x0A)HSRXTimeoutValueRegister
        uint8_t TV          : 8;
        //(0x0B)FTCInternalUseRegister
        uint8_t FIUR_V0     : 8;
        uint8_t FIUR_V1     : 8;
        uint8_t FIUR_V2     : 8;
        uint8_t RSVD_4[4];  //(0x0E ~ 0x11)
        //(0x12)InitializationTimerRegister
        uint8_t IT          : 8;
        uint8_t ITH         : 2;
        uint8_t RSVD_4_1    : 6;
        //(0x14)DPIVC0VSyncTimingRegister
        uint8_t VSTR_VC0    : 8;
        //(0x15)DPIVC0HSyncTimingRegister
        uint8_t HSTR_VC0    : 8;
        //(0x16)DPIVC1VSyncTimingRegister
        uint8_t VSTR_VC1    : 8;
        //(0x17)DPIVC1HSyncTimingRegister
        uint8_t HSTR_VC1    : 8;
        //(0x18)DPIVC2VSyncTimingRegister
        uint8_t VSTR_VC2    : 8;
        //(0x19)DPIVC2HSyncTimingRegister
        uint8_t HSTR_VC2    : 8;
        //(0x1A)DPIVC3VSyncTimingRegister
        uint8_t VSTR_VC3    : 8;
        //(0x1B)DPIVC3HSyncTimingRegister
        uint8_t HSTR_VC3    : 8;
        //(0x1C)DPIMappingControlRegister
        uint8_t M0          : 1;
        uint8_t RSVD_4_2    : 1;
        uint8_t M1          : 1;
        uint8_t RSVD_4_3    : 1;
        uint8_t M2          : 1;
        uint8_t RSVD_4_4    : 1;
        uint8_t M3          : 1;
        uint8_t RSVD_4_5    : 1;
        //(0x1D)
        uint8_t RSVD_5_     : 8;
        //(0x1E)DPIVSyncTimingExtendedRegister
        uint8_t VSTER_VC0   : 4;
        uint8_t VSTER_VC1   : 4;
        //(0x1F)
        uint8_t VSTER_VC2   : 4;
        uint8_t VSTER_VC3   : 4;
        //(0x20)DPIHorizontalPixelNumberRegisterLowerRegister
        uint8_t VC0_HPNL    : 8;
        //(0x21)DPIHorizontalPixelNumberRegisterHigherRegister
        uint8_t VC0_HPNH    : 5;
        uint8_t VC0_HP      : 1;
        uint8_t VC0_VP      : 1;
        uint8_t RSVD_5_1    : 1;
        //(0x22)DPIHorizontalPixelNumberRegisterLowerRegister
        uint8_t VC1_HPNL    : 8;
        //(0x23)DPIHorizontalPixelNumberRegisterHigherRegister
        uint8_t VC1_HPNH    : 5;
        uint8_t VC1_HP      : 1;
        uint8_t VC1_VP      : 1;
        uint8_t RSVD_5_2    : 1;
        //(0x24)DPIHorizontalPixelNumberRegisterLowerRegister
        uint8_t VC2_HPNL    : 8;
        //(0x25)DPIHorizontalPixelNumberRegisterHigherRegister
        uint8_t VC2_HPNH    : 5;
        uint8_t VC2_HP      : 1;
        uint8_t VC2_VP      : 1;
        uint8_t RSVD_5_4    : 1;
        //(0x26)DPIHorizontalPixelNumberRegisterLowerRegister
        uint8_t VC3_HPNL    : 8;
        //(0x27)DPIHorizontalPixelNumberRegisterHigherRegister
        uint8_t VC3_HPNH    : 5;
        uint8_t VC3_HP      : 1;
        uint8_t VC3_VP      : 1;
        uint8_t RSVD_5_5    : 1;
        //(0x28)PPIEnableControlRegister
        uint8_t PEC         : 8;
        //(0x29)
        uint8_t RSVD_6;
        //(0x2A)DataLaneMappingRegister
        uint8_t DLMR_0      : 8;
        //(0x2B)DataLaneMappingRegister
        uint8_t DLMR_1      : 8;
        //(0x2C)DataLaneMappingRegister
        uint8_t DLMR_2      : 8;
        //(0x2D)DataLaneMappingRegister
        uint8_t DLMR_3      : 8;
        uint8_t RSVD_7[2];  //(0x2E~2F)
        //(0x30)CSIErrorReportRegister
        uint8_t SE          : 1;
        uint8_t SSE         : 1;
        uint8_t ESE         : 1;
        uint8_t EMECE       : 1;
        uint8_t PCR         : 1;
        uint8_t HSRT        : 1;
        uint8_t FCE         : 1;
        uint8_t CD          : 1;
        //(0x31)CSIErrorReportRegister
        uint8_t ECCES       : 1;
        uint8_t ECCEM       : 1;
        uint8_t CE          : 1;
        uint8_t UPT         : 1;
        uint8_t CSIVIDI     : 1;
        uint8_t ITL         : 1;
        uint8_t RSVD_7_1    : 2;
        //(0x32)
        uint8_t RSVD_8;
        //(0x33)InterruptStatusregister
        uint8_t CSI         : 1;
        uint8_t RSVD_8_1    : 1;
        uint8_t DPI         : 1;
        uint8_t FS          : 1;
        uint8_t ULPS_X      : 1;
        uint8_t ULPS_E      : 1;
        uint8_t RT          : 1;
        uint8_t RSVD_8_2    : 1;
        //(0x34)ESR0 EscapeModeandStopStateRegister
        uint8_t ULPS        : 4;
        uint8_t STOP        : 4;
        //(0x35)ESR1 EscapeModeandStopStateRegister
        uint8_t RET         : 4;
        uint8_t RUCN        : 1;
        uint8_t RUE         : 1;
        uint8_t CUAN        : 1;
        uint8_t CSTOP       : 1;
        //(0x36)ESR1 EscapeModeandStopStateRegister
        uint8_t ULPS_2      : 4;
        uint8_t STOP_2      : 4;
        //(0x37)
        uint8_t RSVD_9      : 8;
        //(0x38)DPIStatusRegister
        uint8_t DPI0_DDE    : 1;    //Discontinuous Data Enable
        uint8_t RSVD_9_1    : 1;
        uint8_t DPI0_PNE    : 1;    //Pixel Number Exceeding
        uint8_t DPI0_DEOH   : 1;    //Data Enable Overlaps Hsync
        uint8_t DPI0_DEOV   : 1;    //Data Enable Overlaps Vsync
        uint8_t DPI0_DPI_OF : 1;    //DPI FIFO Overflow
        uint8_t RSVD_9_2    : 2;
        //(0x39)DPIStatusRegister
        uint8_t DPI1_DDE    : 1;    //Discontinuous Data Enable
        uint8_t RSVD_9_3    : 1;
        uint8_t DPI1_PNE    : 1;    //Pixel Number Exceeding
        uint8_t DPI1_DEOH   : 1;    //Data Enable Overlaps Hsync
        uint8_t DPI1_DEOV   : 1;    //Data Enable Overlaps Vsync
        uint8_t DPI1_DPI_OF : 1;    //DPI FIFO Overflow
        uint8_t RSVD_9_4    : 2;
        //(0x3A)DPIStatusRegister
        uint8_t DPI2_DDE    : 1;    //Discontinuous Data Enable
        uint8_t RSVD_9_5    : 1;
        uint8_t DPI2_PNE    : 1;    //Pixel Number Exceeding
        uint8_t DPI2_DEOH   : 1;    //Data Enable Overlaps Hsync
        uint8_t DPI2_DEOV   : 1;    //Data Enable Overlaps Vsync
        uint8_t DPI2_DPI_OF : 1;    //DPI FIFO Overflow
        uint8_t RSVD_9_6    : 2;
        //(0x3B)DPIStatusRegister
        uint8_t DPI3_DDE    : 1;    //Discontinuous Data Enable
        uint8_t RSVD_9_7    : 1;
        uint8_t DPI3_PNE    : 1;    //Pixel Number Exceeding
        uint8_t DPI3_DEOH   : 1;    //Data Enable Overlaps Hsync
        uint8_t DPI3_DEOV   : 1;    //Data Enable Overlaps Vsync
        uint8_t DPI3_DPI_OF : 1;    //DPI FIFO Overflow
        uint8_t RSVD_9_8    : 2;
        //(0x3C)InterruptEnableRegister
        uint8_t CSI_EN      : 1;    //CSI Error Events
        uint8_t DEM_EN      : 1;    //DPI Event Mask
        uint8_t DPI_EN      : 1;    //DPI Events
        uint8_t FS_EN       : 1;    //Receive Frame Start Packet
        uint8_t ULPS_X_EN   : 1;    //ULPS exit
        uint8_t ULPS_E_EN   : 1;    //ULPS entry
        uint8_t RT_EN       : 1;    //Receive Trigger
        uint8_t ACDF_EN     : 1;    //Automatically Clear DPI FIFO


        uint8_t FFR[11];        //(0x3D)FTCSIRX100FeatureRegister
        //(0x48)DPCMRegister
        uint8_t DPCMR[4];
        //(0x4C)FTCSIRX100RevisionRegister
        uint8_t FRR[4];
        //(0x50)PixelFIFOThresholdRegister
        uint8_t PFTR_VC0    : 8;
        uint8_t PFTR_VC1    : 8;
        uint8_t PFTR_VC2    : 8;
        uint8_t PFTR_VC3    : 8;
        //(0x54~57)
        uint8_t RSVD_10[4];
        //(0x58)ProgrammableUserDefined8-bitData
        uint8_t PUDTR;
        uint8_t RSVD_11[39];//(0x59~7F)
        //(0x80)TypeRegister
        uint8_t FRCR[8];
        //(0x88)FrameRateControlRegister
        uint8_t FNR[8];
        //(0x90)DPI Built-in Pattern Generator Register
        uint8_t BPGR_VLN_VC0    : 8;
        uint8_t BPGR_VLNH_VC0   : 4;
        uint8_t BPGR_GE_VC0     : 1;
        uint8_t BPGR_PS_VC0     : 1;
        uint8_t BPGR_PT_VC0     : 2;
        //(0x92)DPI Built-in Pattern Generator Register
        uint8_t BPGR_VLN_VC1    : 8;
        uint8_t BPGR_VLNH_VC1   : 4;
        uint8_t BPGR_GE_VC1     : 1;
        uint8_t BPGR_PS_VC1     : 1;
        uint8_t BPGR_PT_VC1     : 2;
        //(0x94)DPI Built-in Pattern Generator Register
        uint8_t BPGR_VLN_VC2    : 8;
        uint8_t BPGR_VLNH_VC2   : 4;
        uint8_t BPGR_GE_VC2     : 1;
        uint8_t BPGR_PS_VC2     : 1;
        uint8_t BPGR_PT_VC2     : 2;
        //(0x96)DPI Built-in Pattern Generator Register
        uint8_t BPGR_VLN_VC3    : 8;
        uint8_t BPGR_VLNH_VC3   : 4;
        uint8_t BPGR_GE_VC3     : 1;
        uint8_t BPGR_PS_VC3     : 1;
        uint8_t BPGR_PT_VC3     : 2;

    }bf;    //bit-field
}U_regCSIRX;
#define regcsirx_0        ((U_regCSIRX  *) CSIRX0_REG_BASE)
#define regcsirx_1        ((U_regCSIRX  *) CSIRX1_REG_BASE)
/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
typedef struct kdrv_csirx_context
{
    uint32_t    csi_rx_base;
    uint32_t    mipi_rx_phy_csr;
    uint32_t    mipi_lane_num_max;
    //struct camera_extio *extio;
}kdrv_csirx_context;

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable
kdrv_csirx_context csirx_ctx[CSIRX_NUM] = {
    {
        .csi_rx_base        = CSIRX0_REG_BASE,
        .mipi_rx_phy_csr    = MIPIRX_PHY0_REG_BASE,
        .mipi_lane_num_max  = MIPI_LANE_NUM_MAX,
    },
    {
        .csi_rx_base        = CSIRX1_REG_BASE,
        .mipi_rx_phy_csr    = MIPIRX_PHY1_REG_BASE,
        .mipi_lane_num_max  = MIPI_LANE_NUM_MAX,
    }
};

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
/*
void kdrv_csirx_register_extio(u32 csirx_idx, struct camera_extio *extio)
{
    struct kdrv_csirx_context *ctx = &csirx_ctx[csirx_idx];
    ctx->extio = extio;
    if (extio->power.set)
        extio->power.set(1);
}

struct camera_extio* kdrv_csirx_get_extio(u32 csirx_idx)
{
    return csirx_ctx[csirx_idx].extio;
}
*/
kdrv_status_t kdrv_csirx_set_settlecnt(uint32_t csirx_idx, uint32_t settle_cnt)
{
    uint32_t mipi_rx_phy_csr = csirx_ctx[csirx_idx].mipi_rx_phy_csr;

    outb(mipi_rx_phy_csr + 0x2E, settle_cnt);
    return KDRV_STATUS_OK;
}

#define apb_write(a,v)  outw(a, v)
kdrv_status_t kdrv_csirx_set_para(uint32_t csirx_idx, cam_format *format, csi_para* para)//uint32_t vstr0, uint32_t vster, uint32_t pftr)
{
    uint32_t width;
    U_regCSIRX* csi_base = (csirx_idx == CSIRX_0)? regcsirx_0: regcsirx_1;
    csi_para* csi_p = (para);

    width = ROUND_UP(format->width, 4);

    csi_base->bf.VC0_HPNL   = (width & 0xff);
    csi_base->bf.VC0_HPNH   = (width & 0x1F00)>>8;
    csi_base->bf.TCN        = (csi_p->timer_count_number & 0xff);
    csi_base->bf.TCNH       = (csi_p->timer_count_number & 0x300)>>8;
    csi_base->bf.TV         = (csi_p->hs_rx_timeout_value & 0xff);
    csi_base->bf.VSTR_VC0   = (csi_p->vstr);
    csi_base->bf.HSTR_VC0   = (csi_p->hstr);
    csi_base->bf.M0         = (csi_p->mapping_control);
    csi_base->bf.VSTER_VC0  = (csi_p->vster);
    csi_base->bf.PFTR_VC0   = (csi_p->pftr);
    csi_base->bf.VSTU0      = (csi_p->vstu);  //CSIRX_REG_VSCR_VSTU0
    csi_base->bf.ECCCE      =   1;
    csi_base->bf.CRCCE      =   1;
    csi_base->bf.SR         =   1;  //Software reset
    //kdrv_csirx_set_settlecnt(csirx_idx, csi_p->timer_count_number);

#ifdef SOURCE_FROM_PATTERN
    uint32_t val;
    //1. Vsync active width
    csi_base->bf.VSTR_VC0   = (csi_p->vstr);
    //2. Hsync active width
    csi_base->bf.HSTR_VC0   = (csi_p->hstr);
    //3. Active pixel in a horizontal line (Tadr)
    csi_base->bf.VC0_HPNL   = (width & 0xff), 0xff);
    csi_base->bf.VC0_HPNH   = (width & 0x1F00)>>8;

    //5. Activer vertical line per frame
    csi_base->bf.BPGR_VLN_VC0   = (format->height & 0xff);
    csi_base->bf.BPGR_VLNH_VC0  = (format->height & 0xf00)>>8;

    //6. Vertical Front Porch ------- fixed 512 DPI pixel clock
    //7. Frame blanking ------ fixed 4096 pixel clocks
    //8. Data alignment
    csi_base->bf.M0         = (csi_p->mapping_control);

    //9. Activer vertical line per frame
    val = 0;//(inb(ctx->csi_rx_base + CSI2RX_REG_BPGHR) & 0x0F);
    val |= 0x40; //PT -> RAW10
    val |= 0x20; //PS -> SWAP COLOR BAR
    val |= 0x10; //GE -> Enable build in pattern generator.
    masked_outw(ctx->csi_rx_base + CSI2RX_REG_BPGLR, (val << 8), 0xf000);

    csi_base->bf.BPGR_GE_VC0    =   0x1;
    csi_base->bf.BPGR_PS_VC0    =   0x1;
    csi_base->bf.BPGR_PT_VC0    =   0x1;

    csi_base->bf.ECCCE      =   1;
    csi_base->bf.CRCCE      =   1;
    csi_base->bf.SR         =   1;  //Software reset
#endif
    kdrv_csirx_set_settlecnt(csirx_idx, csi_p->phy_settle_cnt);

    kdrv_delay_us(1000);
    return KDRV_STATUS_OK;
}
kdrv_status_t kdrv_csirx_start(uint32_t csirx_idx, uint32_t num)
{
    uint32_t i;
    uint32_t val;

    if( num > csirx_ctx[csirx_idx].mipi_lane_num_max)
        return KDRV_STATUS_ERROR;

    val = 0;
    for (i = 0; i < num; i++)
        val |= BIT0 << i;

    CSIRX_REG_PECR_SET_PEC(csirx_ctx[csirx_idx].csi_rx_base + CSI2RX_REG_PECR, val);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_csirx_set_enable(uint32_t csirx_idx, uint32_t enable)
{
    if (csirx_idx == CSIRX_0)
    {
        SET_CSIRX_CTRL(CSIRX_ENABLE_0, enable);
    }
    else if (csirx_idx == CSIRX_1)
    {
        SET_CSIRX_CTRL(CSIRX_ENABLE_1, enable);
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_csirx_set_reset(uint32_t csirx_idx, uint32_t reset)
{
    uint32_t val;
    val = (reset)? 0x0:0x770;
    if (csirx_idx == CSIRX_0)
    {
        masked_outw(SCU_EXTREG_CSIRX_CTRL_REG0, val, 0x770);
    }
    else if (csirx_idx == CSIRX_1)
    {
        masked_outw(SCU_EXTREG_CSIRX_CTRL_REG1, val, 0x770);
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_csirx_stop(uint32_t csirx_idx)
{
    uint32_t val;

    val = 0;

    CSIRX_REG_PECR_SET_PEC(csirx_ctx[csirx_idx].csi_rx_base + CSI2RX_REG_PECR, val);

    return KDRV_STATUS_OK;
}

void kdrv_csirx_auto_clear_fifo(uint32_t csirx_idx)
{
    uint8_t reg_val = CSIRX_REG_PECR_GET_PEC(csirx_ctx[csirx_idx].csi_rx_base+ CSI2RX_REG_INTER);
    reg_val&=0x7f;
    CSIRX_REG_PECR_SET_PEC(csirx_ctx[csirx_idx].csi_rx_base + CSI2RX_REG_INTER, reg_val);
   
}

uint16_t kdrv_csirx_get_reg_value(uint32_t csirx_idx, uint32_t reg_offset)
{
    return readw(csirx_ctx[csirx_idx].csi_rx_base+ reg_offset);
}
kdrv_status_t kdrv_csirx_initialize(uint32_t csirx_idx)
{
    kdrv_csirx_set_reset(csirx_idx, 0);
    kdrv_clock_set_csiclk(csirx_idx, 1);
    kdrv_csirx_set_enable(csirx_idx, 1);
  
    kdrv_csirx_auto_clear_fifo(csirx_idx);
    return KDRV_STATUS_OK;
}
