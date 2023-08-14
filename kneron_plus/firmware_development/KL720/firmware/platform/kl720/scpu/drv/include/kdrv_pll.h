/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/
/**@addtogroup  KDRV_PLL    KDRV_PLL
 * @{
 * @brief       Kneron PLL driver (extension register)
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
*/

#ifndef _KDRV_PLL_H_
#define _KDRV_PLL_H_

#include <stdint.h>

#define PLL_AXI_DDR         1
#define PLL_MRX1            2
#define PLL_MRX0            3
#define PLL_NPU             4
#define PLL_DSP             5
#define PLL_ADO             6

/***************************************************
Offset Address : SCU+0x30 PLL Control Register (PLL0)
***************************************************/
//PLL0 1200MHZ
#define PLL0_1200_MS                    1
#define PLL0_1200_NS                    100     //0x64  //max:512
#define PLL0_1200_PS                    0
#define PLL0_1200_IS                    0
#define PLL0_1200_MS_MASK               (PLL0_1200_MS<<16)
#define PLL0_1200_NS_MASK               (((PLL0_1200_NS&0xFF)<<24)&((PLL0_1200_NS&0x100)<<19))
#define PLL0_1200_PS_MASK               (PLL0_1200_PS<< 8)
#define PLL0_1200_IS_MASK               (PLL0_1200_IS<<20)

#define SCPU_CLKIN_MUX_12MHZ            1
#define SCPU_CLKIN_MUX_12MHZ_MASK       (SCPU_CLKIN_MUX_12MHZ<<4)
#define SCPU_CLKIN_MUX_PLL0_DIV3        0
#define SCPU_CLKIN_MUX_PLL0_DIV3_MASK   (SCPU_CLKIN_MUX_PLL0_DIV3<<4)
#define PLL0_STABLE                     1
#define PLL0_NOT_STABLE                 0

/***************************************************
Offset Address : SCUEXT+0x08 PLL1~PLL6 setting
***************************************************/

//PLL1 400MHZ
#define PLL1_400_MS             3
#define PLL1_400_NS             100     //0x64
#define PLL1_400_PS             0
#define PLL1_400_IS             0
#define PLL1_400_MS_MASK        ((PLL1_400_MS&0x07 )<<16)
#define PLL1_400_NS_MASK        ((PLL1_400_NS&0x1FF)<<20)
#define PLL1_400_PS_MASK        ((PLL1_400_PS&0x0F )<<12)
#define PLL1_400_IS_MASK        ((PLL1_400_IS&0x03 )<< 8)
#define PLL1_TIMER              0x4C0

//PLL1 668MHZ
#define PLL1_668_MS             3
#define PLL1_668_NS             167     //0xA7
#define PLL1_668_PS             0
#define PLL1_668_IS             0
#define PLL1_668_MS_MASK        ((PLL1_668_MS&0x07 )<<16)
#define PLL1_668_NS_MASK        ((PLL1_668_NS&0x1FF)<<20)
#define PLL1_668_PS_MASK        ((PLL1_668_PS&0x0F )<<12)
#define PLL1_668_IS_MASK        ((PLL1_668_IS&0x03 )<< 8)
#define PLL1_TIMER              0x4C0

//PLL1 800MHZ
#define PLL1_800_MS             3
#define PLL1_800_NS             200     //0xC8
#define PLL1_800_PS             0
#define PLL1_800_IS             0
#define PLL1_800_MS_MASK        ((PLL1_800_MS&0x07 )<<16)
#define PLL1_800_NS_MASK        ((PLL1_800_NS&0x1FF)<<20)
#define PLL1_800_PS_MASK        ((PLL1_800_PS&0x0F )<<12)
#define PLL1_800_IS_MASK        ((PLL1_800_IS&0x03 )<< 8)
#define PLL1_TIMER              0x4C0

//PLL1 933MHZ
#define PLL1_933_MS             3
#define PLL1_933_NS             233     //0xE9
#define PLL1_933_PS             0
#define PLL1_933_IS             0
#define PLL1_933_MS_MASK        ((PLL1_933_MS&0x07 )<<16)
#define PLL1_933_NS_MASK        ((PLL1_933_NS&0x1FF)<<20)
#define PLL1_933_PS_MASK        ((PLL1_933_PS&0x0F )<<12)
#define PLL1_933_IS_MASK        ((PLL1_933_IS&0x03 )<< 8)
#define PLL1_TIMER              0x4C0

//PLL1 1066MHZ
#define PLL1_1066_MS            3
#define PLL1_1066_NS            266     //0x10A
#define PLL1_1066_PS            0
#define PLL1_1066_IS            0
#define PLL1_1066_MS_MASK       ((PLL1_1066_MS&0x07 )<<16)
#define PLL1_1066_NS_MASK       ((PLL1_1066_NS&0x1FF)<<20)
#define PLL1_1066_PS_MASK       ((PLL1_1066_PS&0x0F )<<12)
#define PLL1_1066_IS_MASK       ((PLL1_1066_IS&0x03 )<< 8)
#define PLL1_TIMER              0x4C0

//==============================================================================

//PLL2 1440MHZ
#define PLL2_1440_MS            1
#define PLL2_1440_NS            120     //0x78
#define PLL2_1440_PS            0
#define PLL2_1440_IS            0
#define PLL2_1440_MS_MASK       ((PLL2_1440_MS&0x07 )<<16)
#define PLL2_1440_NS_MASK       ((PLL2_1440_NS&0x1FF)<<20)
#define PLL2_1440_PS_MASK       ((PLL2_1440_PS&0x0F )<<12)
#define PLL2_1440_IS_MASK       ((PLL2_1440_IS&0x03 )<< 8)
#define PLL2_TIMER              0x4C0

//CSIRX1_DIV  //EscClk=36(20MHz), csirx_vc_pclk=4(180MHz), csirx_clk=16(45MHz)
#define PLL2_CSI1_TXESC         0x23
#define PLL2_CSI1_CSI           0x0F
#define PLL2_CSI1_VC0           0x03
#define PLL2_DIV_MASK           ((PLL2_CSI1_CSI)|(PLL2_CSI1_VC0<<8)|(PLL2_CSI1_TXESC<<16))

//PLL3  1440MHZ
#define PLL3_1440_MS            1
#define PLL3_1440_NS            120     //0x78
#define PLL3_1440_PS            0
#define PLL3_1440_IS            0
#define PLL3_1440_MS_MASK       ((PLL3_1440_MS&0x07 )<<16)
#define PLL3_1440_NS_MASK       ((PLL3_1440_NS&0x1FF)<<20)
#define PLL3_1440_PS_MASK       ((PLL3_1440_PS&0x0F )<<12)
#define PLL3_1440_IS_MASK       ((PLL3_1440_IS&0x03 )<< 8)
#define PLL3_TIMER              0x4C0

//PLL3  2640MHZ
#define PLL3_2640_MS            1
#define PLL3_2640_NS            220//220     //0xdc
#define PLL3_2640_PS            0
#define PLL3_2640_IS            0
#define PLL3_2640_MS_MASK       ((PLL3_2640_MS&0x07 )<<16)
#define PLL3_2640_NS_MASK       ((PLL3_2640_NS&0x1FF)<<20)
#define PLL3_2640_PS_MASK       ((PLL3_2640_PS&0x0F )<<12)
#define PLL3_2640_IS_MASK       ((PLL3_2640_IS&0x03 )<< 8)
#define PLL3_TIMER              0x4C0

//CSIRX0_DIV  //EscClk=36(20MHz), csirx_vc_pclk=12(60MHz), csirx_clk=24(30MHz)
#define PLL3_CSI0_TXESC         0x23
#define PLL3_CSI0_CSI           0x17
#define PLL3_CSI0_VC0           0x0B
#define PLL3_DIV_MASK           ((PLL3_CSI0_CSI)|(PLL3_CSI0_VC0<<8)|(PLL3_CSI0_TXESC<<16))

//CSIRX0_DIV //For ToF IRS2877c sensor
#define PLL3_CSI0_TXESC_2       0x3F
#define PLL3_CSI0_CSI_2         0x11
#define PLL3_CSI0_VC0_2         0x07
#define PLL3_DIV_MASK_2         ((PLL3_CSI0_CSI_2)|(PLL3_CSI0_VC0_2<<8)|(PLL3_CSI0_TXESC_2<<16))
//==============================================================================
//PLL4 800MHZ
#define PLL4_800_MS             2
#define PLL4_800_NS             133     //0x85
#define PLL4_800_PS             0
#define PLL4_800_IS             0
#define PLL4_800_MS_MASK        ((PLL4_800_MS&0x07 )<<16)
#define PLL4_800_NS_MASK        ((PLL4_800_NS&0x1FF)<<20)
#define PLL4_800_PS_MASK        ((PLL4_800_PS&0x0F )<<12)
#define PLL4_800_IS_MASK        ((PLL4_800_IS&0x03 )<< 8)
#define PLL4_TIMER              0x4C0

//PLL4 1000MHZ
#define PLL4_1000_MS            2
#define PLL4_1000_NS            167     //0xA7
#define PLL4_1000_PS            0
#define PLL4_1000_IS            0
#define PLL4_1000_MS_MASK       ((PLL4_1000_MS&0x07 )<<16)
#define PLL4_1000_NS_MASK       ((PLL4_1000_NS&0x1FF)<<20)
#define PLL4_1000_PS_MASK       ((PLL4_1000_PS&0x0F )<<12)
#define PLL4_1000_IS_MASK       ((PLL4_1000_IS&0x03 )<< 8)
#define PLL4_TIMER              0x4C0

//PLL4 1200MHZ
#define PLL4_1200_MS            2
#define PLL4_1200_NS            200     //0xC8
#define PLL4_1200_PS            0
#define PLL4_1200_IS            0
#define PLL4_1200_MS_MASK       ((PLL4_1200_MS&0x07 )<<16)
#define PLL4_1200_NS_MASK       ((PLL4_1200_NS&0x1FF)<<20)
#define PLL4_1200_PS_MASK       ((PLL4_1200_PS&0x0F )<<12)
#define PLL4_1200_IS_MASK       ((PLL4_1200_IS&0x03 )<< 8)
#define PLL4_TIMER              0x4C0

//PLL4 1300MHZ DEFAULT
#define PLL4_1300_MS            1
#define PLL4_1300_NS            108     //0x6C
#define PLL4_1300_PS            0
#define PLL4_1300_IS            0
#define PLL4_1300_MS_MASK       ((PLL4_1300_MS&0x07 )<<16)
#define PLL4_1300_NS_MASK       ((PLL4_1300_NS&0x1FF)<<20)
#define PLL4_1300_PS_MASK       ((PLL4_1300_PS&0x0F )<<12)
#define PLL4_1300_IS_MASK       ((PLL4_1300_IS&0x03 )<< 8)
#define PLL4_TIMER              0x4C0

//PLL4 1400MHZ DEFAULT
#define PLL4_1400_MS            1
#define PLL4_1400_NS            116     //0x74
#define PLL4_1400_PS            0
#define PLL4_1400_IS            0
#define PLL4_1400_MS_MASK       ((PLL4_1400_MS&0x07 )<<16)
#define PLL4_1400_NS_MASK       ((PLL4_1400_NS&0x1FF)<<20)
#define PLL4_1400_PS_MASK       ((PLL4_1400_PS&0x0F )<<12)
#define PLL4_1400_IS_MASK       ((PLL4_1400_IS&0x03 )<< 8)
#define PLL4_TIMER              0x4C0
//==============================================================================
//PLL5 800MHZ
#define PLL5_800_MS             2
#define PLL5_800_NS             133     //0x85
#define PLL5_800_PS             0
#define PLL5_800_IS             0
#define PLL5_800_MS_MASK        ((PLL5_800_MS&0x07 )<<16)
#define PLL5_800_NS_MASK        ((PLL5_800_NS&0x1FF)<<20)
#define PLL5_800_PS_MASK        ((PLL5_800_PS&0x0F )<<12)
#define PLL5_800_IS_MASK        ((PLL5_800_IS&0x03 )<< 8)
#define PLL5_TIMER              0x4C0

//PLL5 900MHZ
#define PLL5_900_MS             2
#define PLL5_900_NS             150     //0x96
#define PLL5_900_PS             0
#define PLL5_900_IS             0
#define PLL5_900_MS_MASK        ((PLL5_900_MS&0x07 )<<16)
#define PLL5_900_NS_MASK        ((PLL5_900_NS&0x1FF)<<20)
#define PLL5_900_PS_MASK        ((PLL5_900_PS&0x0F )<<12)
#define PLL5_900_IS_MASK        ((PLL5_900_IS&0x03 )<< 8)
#define PLL5_TIMER              0x4C0

//PLL5 1000MHZ
#define PLL5_1000_MS            2
#define PLL5_1000_NS            167     //0xA7
#define PLL5_1000_PS            0
#define PLL5_1000_IS            0
#define PLL5_1000_MS_MASK       ((PLL5_1000_MS&0x07 )<<16)
#define PLL5_1000_NS_MASK       ((PLL5_1000_NS&0x1FF)<<20)
#define PLL5_1000_PS_MASK       ((PLL5_1000_PS&0x0F )<<12)
#define PLL5_1000_IS_MASK       ((PLL5_1000_IS&0x03 )<< 8)
#define PLL5_TIMER              0x4C0

//PLL5 1200MHZ
#define PLL5_1200_MS            2
#define PLL5_1200_NS            200     //0xC8
#define PLL5_1200_PS            0
#define PLL5_1200_IS            0
#define PLL5_1200_MS_MASK       ((PLL5_1200_MS&0x07 )<<16)
#define PLL5_1200_NS_MASK       ((PLL5_1200_NS&0x1FF)<<20)
#define PLL5_1200_PS_MASK       ((PLL5_1200_PS&0x0F )<<12)
#define PLL5_1200_IS_MASK       ((PLL5_1200_IS&0x03 )<< 8)
#define PLL5_TIMER              0x4C0

//PLL5 1500MHZ DEFAULT
#define PLL5_1500_MS            2
#define PLL5_1500_NS            250     //0xFA
#define PLL5_1500_PS            0
#define PLL5_1500_IS            0
#define PLL5_1500_MS_MASK       ((PLL5_1500_MS&0x07 )<<16)
#define PLL5_1500_NS_MASK       ((PLL5_1500_NS&0x1FF)<<20)
#define PLL5_1500_PS_MASK       ((PLL5_1500_PS&0x0F )<<12)
#define PLL5_1500_IS_MASK       ((PLL5_1500_IS&0x03 )<< 8)
#define PLL5_TIMER              0x4C0

//==============================================================================

//PLL6 1536MHZ
#define PLL6_1536_MS            2
#define PLL6_1536_NS            192     //0xC0
#define PLL6_1536_PS            0
#define PLL6_1536_IS            0
#define PLL6_1536_MS_MASK       ((PLL6_1536_MS&0x07 )<<16)
#define PLL6_1536_NS_MASK       ((PLL6_1536_NS&0x1FF)<<20)
#define PLL6_1536_PS_MASK       ((PLL6_1536_PS&0x0F )<<12)
#define PLL6_1536_IS_MASK       ((PLL6_1536_IS&0x03 )<< 8)
#define PLL6_TIMER              0x4C0

//PLLn for Lock and enable/disable
#define PLLn_LOCKED             1
#define PLLn_ENABLE             1
#define PLLn_ENABLE_MASK        (PLLn_ENABLE<<0)
#define PLLn_DISABLE            0
#define PLLn_DISABLE_MASK       (PLLn_DISABLE<<0)

/** @brief Enumerations of list supported SCPU clock */
typedef enum
{
    //please note that this value will be used for array index
    SCPU_400_CFG1 = 0,           /**< Enum 0 */
    SCPU_CLK_TOTAL_SUPPORTED     /**< Enum 1 */
}scpu_clk_setting;

/** @brief Enumerations of list supported AXI/DDR clock */
typedef enum
{
    //please note that this value will be used for array index
    AXI_DDR_200_CFG1 = 0,           /**< Enum 0 */
    AXI_DDR_333_CFG1,               /**< Enum 1 */
    AXI_DDR_400_CFG1,               /**< Enum 2 */
    AXI_DDR_466_CFG1,               /**< Enum 3 */
    AXI_DDR_533_CFG1,               /**< Enum 4 */
    AXI_DDR_CLK_TOTAL_SUPPORTED     /**< Enum 5 */
}axi_ddr_clk_setting;

/** @brief Enumerations of list supported MRX1 clock */
typedef enum
{
    //please note that this value will be used for array index
    MRX1_720_CFG1 = 0,               /**< Enum 0 */
    MRX1_CLK_TOTAL_SUPPORTED        /**< Enum 1 */
}mrx1_clk_setting;

/** @brief Enumerations of list supported MRX0 clock */
typedef enum
{
    //please note that this value will be used for array index
    MRX0_720_CFG1 = 0,          /**< Enum 0 */
    MRX0_1320_CFG1,             /**< Enum 1 */
    MRX0_CLK_TOTAL_SUPPORTED    /**< Enum 2 */
}mrx0_clk_setting;

/** @brief Enumerations of list supported NPU clock */
typedef enum
{
    //please note that this value will be used for array index
    NPU_200_CFG1 = 0,   /**< Enum 0 */
    NPU_250_CFG1,       /**< Enum 1 */
    NPU_300_CFG1,       /**< Enum 2 */
    NPU_350_CFG1,       /**< Enum 3 */
    NPU_400_CFG1,       /**< Enum 4 */
    NPU_500_CFG1,       /**< Enum 5 */
    NPU_600_CFG1,       /**< Enum 6 */
    NPU_650_CFG1,       /**< Enum 7 */
    NPU_700_CFG1,       /**< Enum 8 */
    NPU_CLK_TOTAL_SUPPORTED /**< Enum 9 */
}npu_clk_setting;

/** @brief Enumerations of list supported DSP clock */
typedef enum
{
    //please note that this value will be used for array index
    DSP_200_CFG1 = 0,   /**< Enum 0 */
    DSP_300_CFG1,       /**< Enum 1 */
    DSP_400_CFG1,       /**< Enum 2 */
    DSP_500_CFG1,       /**< Enum 3*/
    DSP_CLK_TOTAL_SUPPORTED /**< Enum 4 */
}dsp_clk_setting;

/** @brief Enumerations of list supported Audio clock */
typedef enum
{
    //please note that this value will be used for array index
    ADO_12p288_CFG1 = 0,        /**< Enum 0 */
    ADO_CLK_TOTAL_SUPPORTED     /**< Enum 1 */
}audio_clk_setting;

/** @brief Structure of PLL0 config information */
typedef struct
{
    uint32_t dwHz;
    uint32_t dwPLL0Setting;
    void (*pDelayFunc)(uint32_t);
} T_PLL0Config;

/** @brief Structure of PLLn config information */
typedef struct
{
    uint32_t dwHz;
    uint32_t dwPLLnSetting;
    uint32_t dwPLLnLockTime;
    uint32_t dwMux;
    uint32_t dwDivider;
} T_PLLnConfig;

#endif // _KDRV_PLL_H_
/** @}*/

