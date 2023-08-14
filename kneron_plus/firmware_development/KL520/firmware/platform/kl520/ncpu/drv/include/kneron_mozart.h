/**************************************************************************//**
 * @file     kneron_mozart.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           ARMCM4 Device (configured for CM4 without FPU)
 * @version  V5.3.1
 * @date     09. July 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef Kneron_Mozart
#define Kneron_Mozart

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------  Interrupt Number Definition  ------------------------ */
// base on Kneron_1009_v009.resmap_int_n_dma.xml
typedef enum {
/* -------------------  Cortex-M4 Processor Exceptions Numbers  ------------------- */
    Reset_IRQn                    = -15,              /*!<   1  Reset Vector, invoked on Power up and warm reset                 */
    NonMaskableInt_IRQn           = -14,              /*!<   2  Non maskable Interrupt, cannot be stopped or preempted           */
    HardFault_IRQn                = -13,              /*!<   3  Hard Fault, all classes of Fault                                 */
    MemoryManagement_IRQn         = -12,              /*!<   4  Memory Management, MPU mismatch, including Access Violation
                                                                and No Match                                                     */
    BusFault_IRQn                 = -11,              /*!<   5  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory
                                                                related Fault                                                    */
    UsageFault_IRQn               = -10,              /*!<   6  Usage Fault, i.e. Undef Instruction, Illegal State Transition    */
    SVCall_IRQn                   =  -5,              /*!<  11  System Service Call via SVC instruction                          */
    DebugMonitor_IRQn             =  -4,              /*!<  12  Debug Monitor                                                    */
    PendSV_IRQn                   =  -2,              /*!<  14  Pendable request for system service                              */
    SysTick_IRQn                  =  -1,              /*!<  15  System Tick Timer                                                */
/* -------------------  Kneron_Mozart Specific Interrupt Numbers  ------------------- */
    PINMUX_FUNCTION_IRQ           =  0,               /*!<  0   PINMUX             */
    DDR_FTDDR3030_IRQ             =  1,               /*!<  1   DDR                */
    ADC_FTADCC010_IRQ             =  2,               /*!<  2   ADC                */
    DMA_FTDMAC020_0_IRQ           =  3,               /*!<  3   DMA0               */
    DMA_FTDMAC020_0_TC_IRQ        =  4,               /*!<  4   DMA0_TC            */
    DMA_FTDMAC020_0_ERR_IRQ       =  5,               /*!<  5   DMA0_ERR           */
    DMA_FTDMAC020_1_IRQ           =  6,               /*!<  6   DMA1_TC            */
    DMA_FTDMAC020_1_TC_IRQ        =  7,               /*!<  7   DMA1               */
    DMA_FTDMAC020_1_ERR_IRQ       =  8,               /*!<  8   DMA1_ERR           */
    D2A_FTDPI2AHB_IRQ             =  9,               /*!<  9   DPI2AHB            */
    GPIO_FTGPIO010_IRQ            =  10,              /*!<  10  GPIO               */
    PWM_FTPWMTMR010_0_IRQ         =  11,              /*!<  11  PWMTMR_0           */
    PWM_FTPWMTMR010_1_IRQ         =  12,              /*!<  12  PWMTMR_1           */
    PWM_FTPWMTMR010_2_IRQ         =  13,              /*!<  13  PWMTMR_2           */
    PWM_FTPWMTMR010_3_IRQ         =  14,              /*!<  14  PWMTMR_3           */
    PWM_FTPWMTMR010_4_IRQ         =  15,              /*!<  15  PWMTMR_4           */
    PWM_FTPWMTMR010_5_IRQ         =  16,              /*!<  16  PWMTMR_5           */
    PWM_FTPWMTMR010_6_IRQ         =  17,              /*!<  17  PWMTMR_6           */
    SDC_FTSDC021_IRQ              =  18,              /*!<  18  SDC                */
    SPI_FTSPI020_IRQ              =  19,              /*!<  19  SPI                */
    SPI_FTSPI2AHB_RD_IRQ          =  20,              /*!<  20  SPI2AHB_RD         */
    SPI_FTSPI2AHB_WR_IRQ          =  21,              /*!<  21  SPI2AHB_WR         */
    SPI_FTSPI2AHB_IRQ             =  22,              /*!<  22  SPI2AHB            */
    SSP_FTSSP010_0_IRQ            =  23,              /*!<  23  SSP                */
    SSP_FTSSP010_1_IRQ            =  24,              /*!<  24  SSP                */
    TMR_FTTMR010_0_IRQ            =  25,              /*!<  25  TMR_0              */
    TMR_FTTMR010_0_1_IRQ          =  26,              /*!<  26  TMR_1              */
    TMR_FTTMR010_0_2_IRQ          =  27,              /*!<  27  TMR_2              */
    TMR_FTTMR010_0_3_IRQ          =  28,              /*!<  28  TMR_3              */
    UART_FTUART010_IRDA_IRQ       =  29,              /*!<  29  UART0_IRDA         */
    UART_FTUART010_0_IRQ          =  30,              /*!<  30  UART0              */
    UART_FTUART010_1_IRQ          =  31,              /*!<  31  UART1              */
    WDT_FTWDT010_IRQ              =  32,              /*!<  32  WDT                */
    NPU_NPU_IRQ                   =  33,              /*!<  33  NPU                */
    CSI_FTCSIRX100_IRQ            =  34,              /*!<  34  CSIRX              */
    OTG_SBS_3_IRQ                 =  35,              /*!<  35  OTG_SBS            */
    TMR_FTTMR010_1_1_IRQ          =  36,              /*!<  36  TMR010_1           */
    TMR_FTTMR010_1_2_IRQ          =  37,              /*!<  37  TMR010_2           */
    TMR_FTTMR010_1_3_IRQ          =  38,              /*!<  38  TMR010_3           */
    TMR_FTTMR010_1_IRQ            =  39,              /*!<  39  TMR010             */
    SYS_SYSTEM_IRQ                =  40,              /*!<  40  SYSC               */
    MIPI_TX_IRQ                   =  41,              /*!<  41  MIPI_TX            */
    IIC_FTIIC010_0_IRQ            =  42,              /*!<  42  I2C0               */
    IIC_FTIIC010_1_IRQ            =  43,              /*!<  43  I2C1               */
    IIC_FTIIC010_2_IRQ            =  44,              /*!<  44  I2C2               */
    IIC_FTIIC010_3_IRQ            =  45,              /*!<  45  I2C3               */
    SSP_FTSSP010_0_1_IRQ          =  46,              /*!<  46  SSP_u0_1           */
    SSP_FTSSP010_1_1_IRQ          =  47,              /*!<  47  SSP_u1_1           */
    UART_FTUART010_1_1_IRQ        =  48,              /*!<  48  UART2              */
    UART_FTUART010_1_2_IRQ        =  49,              /*!<  49  UART3              */
    UART_FTUART010_1_3_IRQ        =  50,              /*!<  50  UART4              */
    SYSC_SGI_S_STATUS_IRQ         =  51,              /*!<  51  SYSC_SGI_S         */
    SBS_CSI_RX_IRQ                =  52,              /*!<  52  CSI_RX             */
    LCDC_FTLCDC210_VSTATUS_IRQ    =  53,              /*!<  53  LCDC_VSTATUS       */
    LCDC_FTLCDC210_BAUPD_IRQ      =  54,              /*!<  54  LCDC_BAUPD         */
    LCDC_FTLCDC210_FUR_IRQ        =  55,              /*!<  55  LCDC_FUR           */
    LCDC_FTLCDC210_MERR_IRQ       =  56,              /*!<  56  LCDC_MERR          */
    LCDC_FTLCDC210_IRQ            =  57,              /*!<  57  LCDC               */
} IRQn_Type;


/* -------------------------  AHB DMA Request Number  ------------------------ */
/* -------------------------      FTDMAC020_u0        ------------------------ */
/* -----[Request Name]                  [Num]           [Instance] ----------- */
#define IRDA_u1_2_NTX_RDY_REQ           0               /*FTUART010_u1_2*/
#define IRDA_u1_2_TX_ACK                0               /*FTUART010_u1_2*/
#define IRDA_u1_2_NRX_RDY_REQ           1               /*FTUART010_u1_2*/
#define IRDA_u1_2_RX_ACK                1               /*FTUART010_u1_2*/
#define IRDA_u1_3_NTX_RDY_REQ           2               /*FTUART010_u1_3*/
#define IRDA_u1_3_TX_ACK                2               /*FTUART010_u1_3*/
#define IRDA_u1_3_NRX_RDY_REQ           3               /*FTUART010_u1_3*/
#define IRDA_u1_3_RX_ACK                3               /*FTUART010_u1_3*/
#define TMR1_DMA_REQ                    4               /*FTPWM010_1*/
#define TMR1_DMA_ACK                    4               /*FTPWM010_1*/
#define TMR2_DMA_REQ                    5               /*FTPWM010_2*/
#define TMR2_DMA_ACK                    5               /*FTPWM010_2*/
#define TMR3_DMA_REQ                    6               /*FTPWM010_3*/
#define TMR3_DMA_ACK                    6               /*FTPWM010_3*/
#define TMR4_DMA_REQ                    7               /*FTPWM010_4*/
#define TMR4_DMA_ACK                    7               /*FTPWM010_4*/
#define TMR5_DMA_REQ                    8               /*FTPWM010_5*/
#define TMR5_DMA_ACK                    8               /*FTPWM010_5*/
#define TMR6_DMA_REQ                    9               /*FTPWM010_6*/
#define TMR6_DMA_ACK                    9               /*FTPWM010_6*/
#define SD_DMA_REQ                      10              /*FTSDC021*/
#define SD_DMA_ACK                      10              /*FTSDC021*/
#define SPI_DMA_REQ                     11              /*FTSPI020*/
#define SPI_DMA_ACK                     11              /*FTSPI020*/
#define SSP_u0_TX_DMA_REQ               12              /*FTSSP010_u0*/
#define SSP_u0_TX_DMA_GNT               12              /*FTSSP010_u0*/
#define SSP_u0_RX_DMA_REQ               13              /*FTSSP010_u0*/
#define SSP_u0_RX_DMA_GNT               13              /*FTSSP010_u0*/
#define SSP_u0_1_TX_DMA_REQ             14              /*FTSSP010_u0_1*/
#define SSP_u0_1_TX_DMA_GNT             14              /*FTSSP010_u0_1*/
#define SSP_u0_1_RX_DMA_REQ             15              /*FTSSP010_u0_1*/
#define SSP_u0_1_RX_DMA_GNT             15              /*FTSSP010_u0_1*/
/* -------------------------      FTDMAC020_u1        ------------------------ */
/* -----[Request Name]                  [Num]           [Instance] ----------- */
#define SSP_u1_TX_DMA_REQ               0               /*FTSSP010_u1*/
#define SSP_u1_TX_DMA_GNT               0               /*FTSSP010_u1*/
#define SSP_u1_RX_DMA_REQ               1               /*FTSSP010_u1*/
#define SSP_u1_RX_DMA_RGNT              1               /*FTSSP010_u1*/
#define SSP_u1_1_TX_DMA_REQ             2               /*FTSSP010_u1_1*/
#define SSP_u1_1_TX_DMA_GNT             2               /*FTSSP010_u1_1*/
#define SSP_u1_1_RX_DMA_REQ             3               /*FTSSP010_u1_1*/
#define SSP_u1_1_RX_DMA_GNT             3               /*FTSSP010_u1_1*/
#define IRDA_u0_DMA_REQ                 4               /*FTUART010_u0*/
#define IRDA_u0_DMA_ACK                 4               /*FTUART010_u0*/
#define IRDA_u0_NTX_RDY                 5               /*FTUART010_u0*/
#define IRDA_u0_TX_ACK                  5               /*FTUART010_u0*/
#define IRDA_u0_NRX_RDY                 6               /*FTUART010_u0*/
#define IRDA_u0_RX_ACK                  6               /*FTUART010_u0*/
#define IRDA_u1_NTX_RDY                 7               /*FTUART010_u1*/
#define IRDA_u1_TX_ACK                  7               /*FTUART010_u1*/
#define IRDA_u1_NRX_RDY                 8               /*FTUART010_u1*/
#define IRDA_u1_RX_ACK                  8               /*FTUART010_u1*/
#define IRDA_u1_1_NTX_RDY               9               /*FTUART010_u1_1*/
#define IRDA_u1_1_TX_ACK                9               /*FTUART010_u1_1*/
#define IRDA_u1_1_NRX_RDY               10              /*FTUART010_u1_1*/
#define IRDA_u1_1_RX_ACK                10              /*FTUART010_u1_1*/
#define ADC_DMA_REQ_0                   11              /*FTADCC010*/
#define ADC_DMA_ACK_0                   11              /*FTADCC010*/
#define ADC_DMA_REQ_1                   12              /*FTADCC010*/
#define ADC_DMA_ACK_1                   12              /*FTADCC010*/
#define ADC_DMA_REQ_2                   13              /*FTADCC010*/
#define ADC_DMA_ACK_2                   13              /*FTADCC010*/
#define ADC_DMA_REQ_3                   14              /*FTADCC010*/
#define ADC_DMA_REQ_3                   14              /*FTADCC010*/

/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
    #pragma push
    #pragma anon_unions
#elif defined (__ICCARM__)
    #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc11-extensions"
    #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
    /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
    /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
    #pragma warning 586
#elif defined (__CSMC__)
    /* anonymous unions are enabled by default */
#else
    #warning Not supported compiler type
#endif

/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM4_REV                 0x0001U   /* Core revision r0p1 */
#define __MPU_PRESENT             1U        /* MPU present */
#define __VTOR_PRESENT            1U        /* VTOR present */
#define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1U        /* with FPU present */

#include "core_cm4.h"                       /* Processor and core peripherals */
#include "system_ARMCM4.h"

/* --------------------  End of section using anonymous unions  ------------------- */
#if defined(__CC_ARM)
  #pragma pop
#elif defined(__ICCARM__)
  /* leave anonymous unions enabled */
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined(__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined(__TASKING__)
  #pragma warning restore
#else
  #warning Not supported compiler type
#endif


/* ================================================================================ */
/* ================              Peripheral memory map             ================ */
/* ================================================================================ */
// base on Kneron_bus_v013.xlsx
/*  ----------------------------   SAHB Peripheral Device    --------------------------------*/
#define SdRAM_MEM_BASE                              0x20200000
#define SdRAM_MEM_SIZE                              0x18000/*new*/
#define SPIF_XIP_BASE                               0x18000000
#define DDR_MEM_BASE                                0x60000000
#define DDR_MEM_SIZE                                0x40000000/*new*/
#define SPI_FTSPI020_PA_BASE                        0xA0000000
#define DMAC_FTDMAC020_PA_BASE                      0xA0100000

/*  ----------------------------   NAHB Peripheral Device    --------------------------------*/
#define NiRAM_MEM_BASE                              0x00000000/*new*/
#define NiRAM_MEM_SIZE                              0x10000/*new*/
#define NdRAM_MEM_BASE                              0x0FFF0000/*new*/
#define NdRAM_MEM_SIZE                              0x10000/*new*/
#define NPU_PA_BASE                                 0x30000000
#define DMAC_FTDMAC020_1_PA_BASE                    0xB0200000

/*  ----------------------------   PAHB Peripheral Device    --------------------------------*/
#define SDC_FTSDC021_PA_BASE                        0xC0000000
#define USB_FOTG210_PA_SIZE                         0xC0100000

/*  ----------------------------   APB0 Peripheral Device    --------------------------------*/
/* IIC */
#define IIC_FTIIC010_0_PA_BASE                      0xC1000000
#define IIC_FTIIC010_1_PA_BASE                      0xC1100000
#define IIC_FTIIC010_2_PA_BASE                      0xC1200000
#define IIC_FTIIC010_3_PA_BASE                      0xC1300000
/* UART */
#define UART_FTUART010_0_PA_BASE                    0xC1400000
#define UART_FTUART010_1_PA_BASE                    0xC1500000
#define UART_FTUART010_1_1_PA_BASE                  0xC1600000
#define UART_FTUART010_1_2_PA_BASE                  0xC1700000
#define UART_FTUART010_1_3_PA_BASE                  0xC1800000
/* SSP */
#define I2S_FTSSP010_0_PA_BASE                      0xC1900000
#define I2S_FTSSP010_1_PA_BASE                      0xC1A00000
#define SPI_FTSSP010_0_PA_BASE                      0xC1B00000
#define SPI_FTSSP010_1_PA_BASE                      0xC1C00000
/* GPIO */
#define GPIO_FTGPIO010_PA_BASE                      0xC1D00000
/* WDT */
#define WDT_FTWDT010_PA_BASE                        0xC1E00000
/* PWM/TMR */
#define PWM_FTPWMTMR010_PA_BASE                     0xC1F00000
#define TMR_FTPWMTMR010_0_PA_BASE                   0xC2000000
#define TMR_FTPWMTMR010_1_PA_BASE                   0xC2100000
/* eFuse */
#define EFUSE_PA_BASE                               0xC2200000
/* SCU */
#define SCU_FTSCU100_PA_BASE                        0xC2300000
#define SCU_EXTREG_PA_BASE                          0xC2380000

/*  ----------------------------   APB1 Peripheral Device    --------------------------------*/
/* ADC */
#define ADC_FTTSC010_0_PA_BASE                      0xC3000000
/* OTG-PHY */
#define OTGPHY_FOTG210_0_PA_BASE                    0xC3100000
/* DDRC */
#define DDRC_FTDDR3030_PA_BASE                      0xC3200000
/* LCDC */
#define LCD_FTLCDC210_PA_BASE                       0xC3300000
#define SLCD_FTLCDC210_PA_BASE                      0xC3400000
/* MIPI */
#define CSIRX_FTCSIRX100_PA_BASE                    0xC3500000
#define CSIRX_FTCSIRX100_1_PA_BASE                  0xC3600000
#define MIPIRX_PHY_CSR_PA_BASE                      0xC3700000
#define MIPIRX_PHY_CSR_1_PA_BASE                    0xC3800000
#define DPI2AHB_CSR_PA_BASE                         0xC3900000
#define DPI2AHB_CSR_1_PA_BASE                       0xC3A00000
#define CSITX_CSR_PA_BASE                           0xC3B00000
#define DSITX_CSR_PA_BASE                           0xC3C00000
#define MIPI_TX_PHY_PA_BASE                         0xC3D00000
#define CSIRX_CSR                                   0xC3500000
#define DPI2AHB_CSR                                 0xC3900000
#define CSIRX1_CSR                                  0xC3500000
#define DPI2AHB1_CSR                                0xC3900000


#define LCD_FRAMEBUFFER_BASE                        0x60000000
#define LCD_FRAMEBUFFER_BASE1                       0x60100000
#define LCD_FRAMEBUFFER_BASE2                       0x60200000
#define LCD_FRAMEBUFFER_BASE3                       0x60300000
/*  ----------------------------   APB1 Peripheral Device    --------------------------------*/

//clock
#define LCD_PLL 50000000        // HAPS PLL2_1
#define APB_CLOCK 100000000     //FPGA:50000000
//#define LCD_PATGEN
// total record how much size, then play it
#define I2S_TOTAL_SIZE          0x2000          
#define UART_CLOCK              (30000000UL) //kneron
#define UART_CLOCK_2            (30000000UL) //kneron

#ifdef __cplusplus
}
#endif


#endif  /* Kneron_Mozart */

