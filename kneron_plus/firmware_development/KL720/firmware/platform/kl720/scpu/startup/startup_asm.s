;/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
; *
; * The information contained herein is property of Kneron, Inc.
; * Terms and conditions of usage are described in detail in Kneron
; * STANDARD SOFTWARE LICENSE AGREEMENT.
; *
; * Licensees are granted free, non-transferable use of the information.
; * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
; * from the file.
; */
;
;/******************************************************************************
;*  Filename:
;*  ---------
;*  startup_scpu.s
;*
;*  Description:
;*  ------------
;*
;*
;******************************************************************************/
#include "project.h"
#include "version.h"
#include "git_info.h"

Heap_Size       EQU      SYS_HEAP_SIZE

                IF       Heap_Size != 0                      ; Heap is provided
                AREA     HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE    Heap_Size
__heap_limit
                ENDIF

Stack_Size      EQU      SYS_STACK_SIZE

                AREA     STACK, NOINIT, READWRITE, ALIGN=3
__stack_limit
Stack_Mem       SPACE    Stack_Size
__initial_sp

                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset

                AREA     VECTOR_TABLE, DATA, READONLY

                EXPORT   __Vectors
                EXPORT   __Vectors_End
                EXPORT   __Vectors_Size

__Vectors       DCD      __initial_sp                        ;     Top of Stack
                DCD      Reset_Handler                       ;     Reset Handler
                DCD      NMI_Handler                         ; -14 NMI Handler
                DCD      HardFault_Handler                   ; -13 Hard Fault Handler
                DCD      MemManage_Handler                   ; -12 MPU Fault Handler
                DCD      BusFault_Handler                    ; -11 Bus Fault Handler
                DCD      UsageFault_Handler                  ; -10 Usage Fault Handler
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      SVC_Handler                         ;  -5 SVCall Handler
                DCD      DebugMon_Handler                    ;  -4 Debug Monitor Handler
                DCD      0                                   ;     Reserved
                DCD      PendSV_Handler                      ;  -2 PendSV Handler
                DCD      SysTick_Handler                     ;  -1 SysTick Handler

                ; External Interrupts
                DCD      SYS_IRQ_Handler                     ; IRQn = 0
                DCD      HW_TIMER0_IRQ_Handler               ; IRQn = 1
                DCD      HW_TIMER1_IRQ_Handler               ; IRQn = 2
                DCD      HW_TIMER2_IRQ_Handler               ; IRQn = 3
                DCD      HW_TIMER3_IRQ_Handler               ; IRQn = 4
                DCD      PWM0_IRQ_Handler                    ; IRQn = 5
                DCD      PWM1_IRQ_Handler                    ; IRQn = 6
                DCD      PWM2_IRQ_Handler                    ; IRQn = 7
                DCD      WDT_IRQ_Handler                     ; IRQn = 8
                DCD      SWI0_IRQ_Handler                    ; IRQn = 9
                DCD      TDC_IRQ_Handler                     ; IRQn = 10
                DCD      GPIO_IRQ_Handler                    ; IRQn = 11
                DCD      DMA020_IRQ_Handler                  ; IRQn = 12
                DCD      DMA020_TC_IRQ_Handler               ; IRQn = 13
                DCD      DMA020_ERR_IRQ_Handler              ; IRQn = 14
                DCD      I2C0_IRQ_Handler                    ; IRQn = 15
                DCD      I2C1_IRQ_Handler                    ; IRQn = 16
                DCD      I2C2_IRQ_Handler                    ; IRQn = 17
                DCD      UART0_IRQ_Handler                   ; IRQn = 18
                DCD      UART1_IRQ_Handler                   ; IRQn = 19
                DCD      SSP0_IRQ_Handler                    ; IRQn = 20
                DCD      SSP1_IRQ_Handler                    ; IRQn = 21
                DCD      SPIF_IRQ_Handler                    ; IRQn = 22
                DCD      SWI1_IRQ_Handler                    ; IRQn = 23
                DCD      SWI2_IRQ_Handler                    ; IRQn = 24
                DCD      SWI3_IRQ_Handler                    ; IRQn = 25
                DCD      DMA030_IRQ_Handler                  ; IRQn = 26
                DCD      DMA030_TC_IRQ_Handler               ; IRQn = 27
                DCD      DMA030_ERR_IRQ_Handler              ; IRQn = 28
                DCD      CSI_RX0_IRQ_Handler                 ; IRQn = 29
                DCD      D2A0_IRQ_Handler                    ; IRQn = 30
                DCD      CSI_RX1_IRQ_Handler                 ; IRQn = 31
                DCD      D2A1_IRQ_Handler                    ; IRQn = 32
                DCD      SGI_IRQ_Handler                     ; IRQn = 33
                DCD      SWI4_IRQ_Handler                    ; IRQn = 34
                DCD      SWI5_IRQ_Handler                    ; IRQn = 35
                DCD      SWI6_IRQ_Handler                    ; IRQn = 36
                DCD      SWI7_IRQ_Handler                    ; IRQn = 37
                DCD      SWI8_IRQ_Handler                    ; IRQn = 38
                DCD      SWI9_IRQ_Handler                    ; IRQn = 39
                DCD      SWI10_IRQ_Handler                   ; IRQn = 40
                DCD      DDR_IRQ_Handler                     ; IRQn = 41
                DCD      AXIIC_IRQ_Handler                   ; IRQn = 42
                DCD      H2X0_IRQ_Handler                    ; IRQn = 43
                DCD      H2X1_IRQ_Handler                    ; IRQn = 44
                DCD      H2X2_IRQ_Handler                    ; IRQn = 45
            IF :DEF:_BOARD_SN720HAPS_H_
                DCD      USB3_IRQ_Handler                    ; IRQn = 46
                DCD      USB2_IRQ_Handler                    ; IRQn = 47
            ELSE
                DCD      USB2_IRQ_Handler                    ; IRQn = 46
                DCD      USB3_IRQ_Handler                    ; IRQn = 47
            ENDIF
                DCD      SDC0_IRQ_Handler                    ; IRQn = 48
                DCD      SDC1_IRQ_Handler                    ; IRQn = 49
                DCD      LCDC_IRQ_Handler                    ; IRQn = 50
                DCD      LCDC_MERR_IRQ_Handler               ; IRQn = 51
                DCD      LCDC_FIFO_IRQ_Handler               ; IRQn = 52
                DCD      LCDC_BAUPD_IRQ_Handler              ; IRQn = 53
                DCD      LCDC_VSTATUS_IRQ_Handler            ; IRQn = 54
                DCD      CRYPTO_IRQ_Handler                  ; IRQn = 55
                DCD      NPU0_IRQ_Handler                    ; IRQn = 56
                DCD      NPU1_IRQ_Handler                    ; IRQn = 57
                DCD      NPU2_IRQ_Handler                    ; IRQn = 58
                DCD      NPU3_IRQ_Handler                    ; IRQn = 59
                DCD      NPU4_IRQ_Handler                    ; IRQn = 60
                DCD      NPU5_IRQ_Handler                    ; IRQn = 61
                DCD      NPU6_IRQ_Handler                    ; IRQn = 62
                DCD      NPU7_IRQ_Handler                    ; IRQn = 63

__Vectors_End
__Vectors_Size  EQU      __Vectors_End - __Vectors

;*********************
;  scpu image header *
;*********************
                AREA    IMAGE_HEADER, DATA, READONLY
                IMPORT  |Load$$LR$$SCPU$$Length|

                DCB     "0720"                      ;4 bytes chip name
                DCB     IMG_ID                      ;4 bytes image name
                DCD     IMG_FW_VERSION              ;4 bytes version: 0xAABBCCDD, AA=major, BB=minor, CC=update, DD=reserve
                DCD     IMG_FW_GIT_VERSION          ;4 bytes version, short git version
                DCD     |Load$$LR$$SCPU$$Length|    ;4 bytes image size
                DCD     IMG_FW_BUILD                ;4 bytes version: build date
                DCD     IMG_FLAG                    ;4 bytes flag for future usage

;****************
;   Startup     *
;****************
                AREA     |.text|, CODE, READONLY
; Reset Handler
Reset_Handler   PROC
                EXPORT   Reset_Handler             [WEAK]
                IMPORT   StartupInit
                IMPORT   __main

                LDR      R0, =__initial_sp
                MOV      SP, R0
                LDR      R0, =StartupInit
                BLX      R0
                LDR      R0, =__main
                BX       R0
                ENDP


; Macro to define default exception/interrupt handlers.
; Default handler are weak symbols with an endless loop.
; They can be overwritten by real handlers.
                MACRO
                Set_Default_Handler  $Handler_Name
$Handler_Name   PROC
                EXPORT   $Handler_Name             [WEAK]
                B        .
                ENDP
                MEND


; Default exception/interrupt handler

                Set_Default_Handler  NMI_Handler
                Set_Default_Handler  HardFault_Handler
                Set_Default_Handler  MemManage_Handler
                Set_Default_Handler  BusFault_Handler
                Set_Default_Handler  UsageFault_Handler
                Set_Default_Handler  SVC_Handler
                Set_Default_Handler  DebugMon_Handler
                Set_Default_Handler  PendSV_Handler
                Set_Default_Handler  SysTick_Handler

                Set_Default_Handler  SYS_IRQ_Handler
                Set_Default_Handler  HW_TIMER0_IRQ_Handler
                Set_Default_Handler  HW_TIMER1_IRQ_Handler
                Set_Default_Handler  HW_TIMER2_IRQ_Handler
                Set_Default_Handler  HW_TIMER3_IRQ_Handler
                Set_Default_Handler  PWM0_IRQ_Handler
                Set_Default_Handler  PWM1_IRQ_Handler
                Set_Default_Handler  PWM2_IRQ_Handler
                Set_Default_Handler  WDT_IRQ_Handler
                Set_Default_Handler  SWI0_IRQ_Handler
                Set_Default_Handler  TDC_IRQ_Handler
                Set_Default_Handler  GPIO_IRQ_Handler
                Set_Default_Handler  DMA020_IRQ_Handler
                Set_Default_Handler  DMA020_TC_IRQ_Handler
                Set_Default_Handler  DMA020_ERR_IRQ_Handler
                Set_Default_Handler  I2C0_IRQ_Handler
                Set_Default_Handler  I2C1_IRQ_Handler
                Set_Default_Handler  I2C2_IRQ_Handler
                Set_Default_Handler  UART0_IRQ_Handler
                Set_Default_Handler  UART1_IRQ_Handler
                Set_Default_Handler  SSP0_IRQ_Handler
                Set_Default_Handler  SSP1_IRQ_Handler
                Set_Default_Handler  SPIF_IRQ_Handler
                Set_Default_Handler  SWI1_IRQ_Handler
                Set_Default_Handler  SWI2_IRQ_Handler
                Set_Default_Handler  SWI3_IRQ_Handler
                Set_Default_Handler  DMA030_IRQ_Handler
                Set_Default_Handler  DMA030_TC_IRQ_Handler
                Set_Default_Handler  DMA030_ERR_IRQ_Handler
                Set_Default_Handler  CSI_RX0_IRQ_Handler
                Set_Default_Handler  D2A0_IRQ_Handler
                Set_Default_Handler  CSI_RX1_IRQ_Handler
                Set_Default_Handler  D2A1_IRQ_Handler
                Set_Default_Handler  SGI_IRQ_Handler
                Set_Default_Handler  SWI4_IRQ_Handler
                Set_Default_Handler  SWI5_IRQ_Handler
                Set_Default_Handler  SWI6_IRQ_Handler
                Set_Default_Handler  SWI7_IRQ_Handler
                Set_Default_Handler  SWI8_IRQ_Handler
                Set_Default_Handler  SWI9_IRQ_Handler
                Set_Default_Handler  SWI10_IRQ_Handler
                Set_Default_Handler  DDR_IRQ_Handler
                Set_Default_Handler  AXIIC_IRQ_Handler
                Set_Default_Handler  H2X0_IRQ_Handler
                Set_Default_Handler  H2X1_IRQ_Handler
                Set_Default_Handler  H2X2_IRQ_Handler
                Set_Default_Handler  USB2_IRQ_Handler
                Set_Default_Handler  USB3_IRQ_Handler
                Set_Default_Handler  SDC0_IRQ_Handler
                Set_Default_Handler  SDC1_IRQ_Handler
                Set_Default_Handler  LCDC_IRQ_Handler
                Set_Default_Handler  LCDC_MERR_IRQ_Handler
                Set_Default_Handler  LCDC_FIFO_IRQ_Handler
                Set_Default_Handler  LCDC_BAUPD_IRQ_Handler
                Set_Default_Handler  LCDC_VSTATUS_IRQ_Handler
                Set_Default_Handler  CRYPTO_IRQ_Handler
                Set_Default_Handler  NPU0_IRQ_Handler
                Set_Default_Handler  NPU1_IRQ_Handler
                Set_Default_Handler  NPU2_IRQ_Handler
                Set_Default_Handler  NPU3_IRQ_Handler
                Set_Default_Handler  NPU4_IRQ_Handler
                Set_Default_Handler  NPU5_IRQ_Handler
                Set_Default_Handler  NPU6_IRQ_Handler
                Set_Default_Handler  NPU7_IRQ_Handler

                ALIGN


; User setup Stack & Heap

                IF       :LNOT::DEF:__MICROLIB
                IMPORT   __use_two_region_memory
                INFO     1, "Please do check 'Use MicroLIB' from Options-Target menu"
                ENDIF

                EXPORT   __stack_limit
                EXPORT   __initial_sp
                IF       Heap_Size != 0                      ; Heap is provided
                EXPORT   __heap_base
                EXPORT   __heap_limit
                ENDIF

                END
