/**************************************************************************//**
 * @file     ARMCM4_FP.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           ARMCM4 Device (configured for CM4 with FPU)
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

#ifndef ARMCM4_FP_H
#define ARMCM4_FP_H

#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum IRQn
{
/* -------------------  Processor Exceptions Numbers  ----------------------------- */
  NonMaskableInt_IRQn           = -14,     /*  2 Non Maskable Interrupt */
  HardFault_IRQn                = -13,     /*  3 HardFault Interrupt */
  MemoryManagement_IRQn         = -12,     /*  4 Memory Management Interrupt */
  BusFault_IRQn                 = -11,     /*  5 Bus Fault Interrupt */
  UsageFault_IRQn               = -10,     /*  6 Usage Fault Interrupt */
  SVCall_IRQn                   =  -5,     /* 11 SV Call Interrupt */
  DebugMonitor_IRQn             =  -4,     /* 12 Debug Monitor Interrupt */
  PendSV_IRQn                   =  -2,     /* 14 Pend SV Interrupt */
  SysTick_IRQn                  =  -1,     /* 15 System Tick Interrupt */

/* -------------------  Processor Interrupt Numbers  ------------------------------ */

  SYS_IRQn                      =   0,
  HW_TIMER0_IRQn                =   1,
  HW_TIMER1_IRQn                =   2,
  HW_TIMER2_IRQn                =   3,
  HW_TIMER3_IRQn                =   4,
  PWM0_IRQn                     =   5,
  PWM1_IRQn                     =   6,
  PWM2_IRQn                     =   7,
  WDT_IRQn                      =   8,
  SWI0_IRQn                     =   9,
  TDC_IRQn                      =  10,
  GPIO_IRQn                     =  11,
  DMA020_IRQn                   =  12,
  DMA020_TC_IRQn                =  13,
  DMA020_ERR_IRQn               =  14,
  I2C0_IRQn                     =  15,
  I2C1_IRQn                     =  16,
  I2C2_IRQn                     =  17,
  UART0_IRQn                    =  18,
  UART1_IRQn                    =  19,
  SSP0_IRQn                     =  20,
  SSP1_IRQn                     =  21,
  SPIF_IRQn                     =  22,
  SWI1_IRQn                     =  23,
  SWI2_IRQn                     =  24,
  SWI3_IRQn                     =  25,
  DMA030_IRQn                   =  26,
  DMA030_TC_IRQn                =  27,
  DMA030_ERR_IRQn               =  28,
  CSI_RX0_IRQn                  =  29,
  D2A0_IRQn                     =  30,
  CSI_RX1_IRQn                  =  31,
  D2A1_IRQn                     =  32,
  SGI_IRQn                      =  33,
  SWI4_IRQn                     =  34,
  SWI5_IRQn                     =  35,
  SWI6_IRQn                     =  36,
  SWI7_IRQn                     =  37,
  SWI8_IRQn                     =  38,
  SWI9_IRQn                     =  39,
  SWI10_IRQn                    =  40,
  DDR_IRQn                      =  41,
  AXIIC_IRQn                    =  42,
  H2X0_IRQn                     =  43,
  H2X1_IRQn                     =  44,
  H2X2_IRQn                     =  45,
  USB2_IRQn                     =  46,
  USB3_IRQn                     =  47,
  SDC0_IRQn                     =  48,
  SDC1_IRQn                     =  49,
  LCDC_IRQn                     =  50,
  LCDC_MERR_IRQn                =  51,
  LCDC_FIFO_IRQn                =  52,
  LCDC_BAUPD_IRQn               =  53,
  LCDC_VSTATUS_IRQn             =  54,
  CRYPTO_IRQn                   =  55,
  NPU0_IRQn                     =  56,
  NPU1_IRQn                     =  57,
  NPU2_IRQn                     =  58,
  NPU3_IRQn                     =  59,
  NPU4_IRQn                     =  60,
  NPU5_IRQn                     =  61,
  NPU6_IRQn                     =  62,
  NPU7_IRQn                     =  63

} IRQn_Type;


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
#define __FPU_PRESENT             1U        /* FPU present */

#include "core_cm4.h"                       /* Processor and core peripherals */
#include "system_ARMCM4.h"                  /* System Header */


/* --------  End of section using anonymous unions and disabling warnings  -------- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


#ifdef __cplusplus
}
#endif

#endif  /* ARMCM4_FP_H */
