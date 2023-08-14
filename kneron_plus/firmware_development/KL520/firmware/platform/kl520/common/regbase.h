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

#ifndef _REGBASE_H_
#define _REGBASE_H_

#include <stdint.h>


/* ================================================================================ */
/* ================              Peripheral memory map             ================ */
/* ================================================================================ */
// base on Kneron_bus_v013.xlsx
/*  ----------------------------   SAHB Peripheral Device    --------------------------------*/
#define SiRAM_MEM_BASE                              0x10100000
#define SiRAM_MEM_SIZE                              0x18000/*new*/
#define SdRAM_MEM_BASE                              0x10200000
#define SdRAM_MEM_SIZE                              0x18000/*new*/
#define SPIF_XIP_BASE                               0x18000000
#define DDR_MEM_BASE                                0x60000000
#define DDR_MEM_SIZE                                0x40000000/*new*/
#define SPI_FTSPI020_PA_BASE                        0xA0000000
#define DMAC_FTDMAC020_PA_BASE                      0xA0100000

/*  ----------------------------   NAHB Peripheral Device    --------------------------------*/
#define NiRAM_MEM_BASE                              0x28000000/*new*/
#define NiRAM_MEM_SIZE                              0x10000/*new*/
#define NdRAM_MEM_BASE                              0x0FFF0000/*new*/
#define NdRAM_MEM_SIZE                              0x10000/*new*/
#define NPU_PA_BASE                                 0x30000000
#define DMAC_FTDMAC020_1_PA_BASE                    0xB0200000

/*  ----------------------------   PAHB Peripheral Device    --------------------------------*/
#define SDC_FTSDC021_PA_BASE                        0xC0000000
#define USB_FOTG210_PA_BASE                         0xC0100000
        
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


#define LCD_FRAMEBUFFER_BASE                        0x60000000
#define LCD_FRAMEBUFFER_BASE1                       0x60100000
#define LCD_FRAMEBUFFER_BASE2                       0x60200000
#define LCD_FRAMEBUFFER_BASE3                       0x60300000
/*  ----------------------------   APB1 Peripheral Device    --------------------------------*/


#endif //_REGBASE_H_
