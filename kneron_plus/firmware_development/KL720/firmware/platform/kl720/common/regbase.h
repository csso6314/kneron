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

/******************************************************************************
Head Block of The File
******************************************************************************/
#define SPIF_REG_BASE                   0x24000000
#define DMAC_AHB_REG_BASE               0x24100000      // DMAC020
#define SDIO_REG_BASE                   0x24300000
#define SDC_REG_BASE                    0x24400000

#define SCU_REG_BASE                    0x30000000
#define SCU_EXT_REG_BASE                0x30080000
#define DMAC_CRS_REG_BASE               0x30200000
#define AXIC_REG_BASE                   0x30300000      //need driver?
#define UART0_REG_BASE                  0x30400000
#define UART1_REG_BASE                  0x30500000
#define I2C2_REG_BASE                   0x30600000
#define I2C0_REG_BASE                   0x30700000
#define I2C1_REG_BASE                   0x30800000
#define SSP0_REG_BASE                   0x30900000
#define SSP1_REG_BASE                   0x30A00000
#define WDT_REG_BASE                    0x30B00000
#define TIMER_REG_BASE                  0x30C00000
#define H2X0_REG_BASE                   0x30D00000      //need driver?
#define H2X1_REG_BASE                   0x30E00000      //need driver?
#define X2H_REG_BASE                    0x30F00000      //need driver?
#define USB3_PHY_REG_BASE               0x31000000
#define CSIRX0_REG_BASE                 0x31100000
#define CSIRX1_REG_BASE                 0x31200000
#define MIPIRX_PHY0_REG_BASE            0x31300000
#define MIPIRX_PHY1_REG_BASE            0x31400000
#define DPI2AHB0_REG_BASE               0x31500000
#define DPI2AHB1_REG_BASE               0x31600000
#define USB2_PHY_REG_BASE               0x31700000
                                      //0x31800000 reserved
#define GPIO_REG_BASE                   0x31900000
#define CRYPTO_REG_BASE                 0x31A00000
#define H2X2_REG_BASE                   0x31C00000      //need driver?
#define LCDC_REG_BASE                   0x31D00000
#define PWM_REG_BASE                    0x31E00000

#define DMAC_AXI_REG_BASE               0x50000000
#define USB3_REG_BASE                   0x50100000
#define USB2_REG_BASE                   0x50200000

#define NCPU_REG_BASE                   0x68100000      // ncpu register base
#define NPU_REG_BASE                    0x68200000      // npu register base
#define DDR_REG_BASE                    0x68300000      // ddr register base
#define X2P_REG_BASE                    0x68400000      // need driver?
#define TDC_REG_BASE                    0x68500000

#define RTC_BASE                        (SCU_REG_BASE + 0x200)


/******************************************************************************
DDR
******************************************************************************/
//DDR Controller
#define     __S_R       volatile const       /**< Defines 'Static Read' permissions   */
#define     __S_RW      volatile             /**< Defines 'Static Read/Write' permissions   */
#define     __D_R       volatile const       /**< Defines 'Dynamic Read' permissions  */
#define     __D_RW      volatile             /**< Defines 'Dynamic Read/Write' permissions  */
#define     __D_RW1C    volatile             /**< Defines 'Dynamic Read/Write of 1 clear' permissions  */
#define     __D_RW1S    volatile
#define     __QD_R      volatile const       /**< Defines 'Quasi Dynamic Read' permissions  */
#define     __QD_G1_RW  volatile             /**< Defines 'Quasi Dynamic Read/Write' permissions  */
#define     __QD_G2_RW  volatile             /**< Defines 'Quasi Dynamic Read/Write' permissions  */
#define     __QD_G3_RW  volatile             /**< Defines 'Quasi Dynamic Read/Write' permissions  */
#define     __QD_G4_RW  volatile             /**< Defines 'Quasi Dynamic Read/Write' permissions  */
#define     __QD_G1_4_RW  volatile           /**< Defines 'Quasi Dynamic Read/Write' permissions  */
#define     __QD_G1_2_RW  volatile           /**< Defines 'Quasi Dynamic Read/Write' permissions  */
#define     __QD_G2_4_RW  volatile           /**< Defines 'Quasi Dynamic Read/Write' permissions  */


/** @brief uMCTL2 DDRC Registers */
typedef struct
{
    volatile uint32_t MSTR;      /* Offset:0x0000; Master Register0, Exists: Always */
    volatile uint32_t STAT;      /* Offset:0x0004; Operating Mode Status Register, Exists: Always */
    volatile uint32_t MSTR1;     /* Offset:0x0008; Master Register1, Exists: UMCTL2_DDR4_MRAM_EN_OR_HET_RANK_RFC==1*/
    volatile uint32_t RSV0;      /* Offset:0x000C; Reserved 0 */
    volatile uint32_t MRCTRL0;   /* Offset:0x0010; Mode Register Read/Write Control Register 0. Exists: Always.*/
    volatile uint32_t MRCTRL1;   /* Offset:0x0014; Mode Register Read/Write Control Register 1. Exists: Always */
    volatile uint32_t MRSTAT;    /* Offset:0x0018; Mode Register Read/Write Status Register. Exists: Always */
    volatile uint32_t MRCTRL2;   /* Offset:0x001C; Mode Register Read/Write Control Register 2. Exists: MEMC_DDR4==1 */
    volatile uint32_t DERATEEN;  /* Offset:0x0020; Temperature Derate Enable Register. Exists: MEMC_LPDDR2==1 */
    volatile uint32_t DERATEINT; /* Offset:0x0024; Temperature Derate Interval Register. Exists: MEMC_LPDDR2==1*/
    volatile uint32_t MSTR2;     /* Offset:0x0028; Master Register2. Exists: UMCTL2_FREQUENCY_NUM>2*/
    volatile uint32_t DERATECTL; /* Offset:0x002C; Temperature Derate Control Register. Exists: MEMC_LPDDR2==1 */
    volatile uint32_t PWRCTL;    /* Offset:0x0030; Low Power Control Register. Exists: Always*/
    volatile uint32_t PWRTMG;    /* Offset:0x0034; Low Power Timing Register. Exists: Always*/
    volatile uint32_t HWLPCTL;   /* Offset:0x0038; Hardware Low Power Control Register. Exists: Always */
    volatile uint32_t HWFFCCTL;  /* Offset:0x003C; Hardware Fast Frequency Change (HWFFC) Control Register. Exists: UMCTL2_HWFFC_EN==1 */
    volatile uint32_t HWFFCSTAT; /* Offset:0x0040; Hardware Fast Frequency Change (HWFFC) Status Register. Exists: UMCTL2_HWFFC_EN==1 */
    volatile uint32_t RSV1[3];   /* Offset:0x0044~0x004C; Reserved 1 */
    volatile uint32_t RFSHCTL0;  /* Offset:0x0050; Refresh Control Register 0. Exists: Always*/
    volatile uint32_t RFSHCTL1;  /* Offset:0x0054; Refresh Control Register 1. Exists: MEMC_NUM_RANKS>1*/
    volatile uint32_t RFSHCTL2;  /* Offset:0x0058; Refresh Control Register 2. Exists: MEMC_NUM_RANKS>2*/
    volatile uint32_t RFSHCTL4;  /* Offset:0x005C; Refresh Control Register 4. Exists: MEMC_NUM_RANKS_GT_4_OR_UMCTL2_CID_EN==1*/
    volatile uint32_t RFSHCTL3;  /* Offset:0x0060; Refresh Control Register 3. Exists: Always
                        Can only be programmed during the initialization or when the controller is in self-refresh mode.*/
    volatile uint32_t RFSHTMG;   /* Offset:0x0064; Refresh Timing Register. Exists: Always*/
    volatile uint32_t RFSHTMG1;  /* Offset:0x0068; Refresh Timing Register1. Exists: MEMC_LPDDR4_OR_UMCTL2_CID_EN==1*/
    volatile uint32_t RSV2;                               /* Offset:0x006C; Reserved 2 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCFG0;         /**< Offset:0x0070; ECC Configuration Register 0 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCFG1;         /**< Offset:0x0074; ECC Configuration Register 1 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCSTAT;         /**< Offset:0x0078; SECDED ECC Status Register (Valid only in MEMC_ECC_SUPPORT==1 (SECDED ECC mode)) */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCTL;          /**< Offset:0x007C; ECC Clear Register */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCERRCNT;       /**< Offset:0x0080; ECC Error Counter Register */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCADDR0;       /**< Offset:0x0084; ECC Corrected Error Address Register 0 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCADDR1;       /**< Offset:0x0088; ECC Corrected Error Address Register 1 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCSYN0;        /**< Offset:0x008C; ECC Corrected Syndrome Register 0 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCSYN1;        /**< Offset:0x0090; ECC Corrected Syndrome Register 1 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCCSYN2;        /**< Offset:0x0094; ECC Corrected Syndrome Register 2 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCBITMASK0;     /**< Offset:0x0098; ECC Corrected Data Bit Mask Register 0 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCBITMASK1;     /**< Offset:0x009C; ECC Corrected Data Bit Mask Register 1 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCBITMASK2;     /**< Offset:0x00A0; ECC Corrected Data Bit Mask Register 2 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCUADDR0;       /**< Offset:0x00A4; ECC Uncorrected Error Address Register 0 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCUADDR1;       /**< Offset:0x00A8; ECC Uncorrected Error Address Register 1 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCUSYN0;        /**< Offset:0x00AC; ECC Uncorrected Syndrome Register 0 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCUSYN1;        /**< Offset:0x00B0; ECC Uncorrected Syndrome Register 1 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCUSYN2;        /**< Offset:0x00B4; ECC Uncorrected Syndrome Register 2 */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCPOISONADDR0;  /**< Offset:0x00B8; ECC Data Poisoning Address Register 0. If a HIF write data beat matches the address specified in... */
    volatile uint32_t KDRV_DDRC_CTRL_REG_ECCPOISONADDR1;  /**< Offset:0x00BC; ECC Data Poisoning Address Register 1. If a HIF write data beat matches the address specified in... */
    volatile uint32_t CRCPARCTL0;    /* Offset:0x00C0; CRC Parity Control Register0. Exists: Always
                            Note: Do not perform any APB access to CRCPARCTL0 within 32 pclk cycles*/
    volatile uint32_t CRCPARCTL1;    /* Offset:0x00C4; CRC Parity Control Register1. Exists: MEMC_DDR4==1*/
    volatile uint32_t CRCPARCTL2;    /* Offset:0x00C8; CRC Parity Control Register2. Exists: UMCTL2_CRC_PARITY_RETRY==1*/
    volatile uint32_t CRCPARSTAT;    /* Offset:0x00CC; CRC Parity Status Register. Exists: Always*/
    volatile uint32_t INIT0;         /* Offset:0x00D0; SDRAM Initialization Register 0. Exists: Always*/
    volatile uint32_t INIT1;         /* Offset:0x00D4; SDRAM Initialization Register 1. Exists: Always */
    volatile uint32_t INIT2;         /* Offset:0x00D8; SDRAM Initialization Register 2. Exists: MEMC_LPDDR2==1 */
    volatile uint32_t INIT3;         /* Offset:0x00DC; SDRAM Initialization Register 3. Exists: Always */
    volatile uint32_t INIT4;         /* Offset:0x00E0; SDRAM Initialization Register 4. Exists: Always */
    volatile uint32_t INIT5;         /* Offset:0x00E4; SDRAM Initialization Register 5. Exists: MEMC_DDR3_OR_4_OR_LPDDR2==1*/
    volatile uint32_t INIT6;         /* Offset:0x00E8; SDRAM Initialization Register 6. Exists: MEMC_DDR4_OR_LPDDR4==1*/
    volatile uint32_t INIT7;         /* Offset:0x00EC; SDRAM Initialization Register 7. Exists: MEMC_DDR4_OR_LPDDR4==1*/
    volatile uint32_t DIMMCTL;       /* Offset:0x00F0; DIMM Control Register. Exists: Always */
    volatile uint32_t RANKCTL;       /* Offset:0x00F4; Rank Control Register. Exists: UMCTL2_NUM_LRANKS_TOTAL>1*/
    volatile uint32_t RSV3;          /* Offset:0x00F8; Reserved 3 */
    volatile uint32_t CHCTL;         /* Offset:0x00FC; Channel Control Register. Exists: UMCTL2_SHARED_AC==1 && UMCTL2_PROGCHN_OR_UMCTL2_SHAREDAC_LP4DUAL_COMB */
    volatile uint32_t DRAMTMG0;      /* Offset:0x0100; SDRAM Timing Register 0. Exists: Always*/
    volatile uint32_t DRAMTMG1;      /* Offset:0x0104; SDRAM Timing Register 1. Exists: Always*/
    volatile uint32_t DRAMTMG2;      /* Offset:0x0108; SDRAM Timing Register 2. Exists: Always*/
    volatile uint32_t DRAMTMG3;      /* Offset:0x010C; SDRAM Timing Register 3. Exists: Always*/
    volatile uint32_t DRAMTMG4;      /* Offset:0x0110; SDRAM Timing Register 4. Exists: Always*/
    volatile uint32_t DRAMTMG5;      /* Offset:0x0114; SDRAM Timing Register 5. Exists: Always*/
    volatile uint32_t DRAMTMG6;      /* Offset:0x0118; SDRAM Timing Register 6. Exists: MEMC_MOBILE_OR_LPDDR2==1*/
    volatile uint32_t DRAMTMG7;      /* Offset:0x011C; SDRAM Timing Register 7. Exists: MEMC_MOBILE_OR_LPDDR2==1*/
    volatile uint32_t DRAMTMG8;      /* Offset:0x0120; SDRAM Timing Register 8. Exists: Always*/
    volatile uint32_t DRAMTMG9;      /* Offset:0x0124; SDRAM Timing Register 9. Exists: MEMC_DDR4==1*/
    volatile uint32_t DRAMTMG10;     /* Offset:0x0128; SDRAM Timing Register 10. Exists: MEMC_DDR4==1 && MEMC_CMD_RTN2IDLE==0 && MEMC_FREQ_RATIO==2 */
    volatile uint32_t DRAMTMG11;     /* Offset:0x012C; SDRAM Timing Register 11 . Exists: MEMC_DDR4==1*/
    volatile uint32_t DRAMTMG12;     /* Offset:0x0130; SDRAM Timing Register 12. Exists: MEMC_DDR4_OR_LPDDR4==1*/
    volatile uint32_t DRAMTMG13;     /* Offset:0x0134; SDRAM Timing Register 13. Exists: MEMC_LPDDR4==1*/
    volatile uint32_t DRAMTMG14;     /* Offset:0x0138; SDRAM Timing Register 14. Exists: MEMC_MOBILE_OR_LPDDR2==1 */
    volatile uint32_t DRAMTMG15;     /* Offset:0x013C; SDRAM Timing Register 15. Exists: MEMC_DDR3_OR_4==1*/
    volatile uint32_t DRAMTMG16;     /* Offset:0x0140; SDRAM Timing Register 16. Exists: UMCTL2_CID_EN==1*/
    volatile uint32_t DRAMTMG17;     /* Offset:0x0144; SDRAM Timing Register 17. UMCTL2_HWFFC_EN==1*/
    volatile uint32_t RSV4[2];       /* Offset:0x0148~0x014C; Reserved 4 */
    volatile uint32_t RFSHTMG_HET;   /* Offset:0x0150; Refresh Timing Register Heterogeneous. __QD_G4_RW*/
    volatile uint32_t RSV5[7];       /* Offset:0x0154~0x016C; Reserved 5 */
    volatile uint32_t MRAMTMG0;      /* Offset:0x0170; MRAM Timing Register 0. Exists: UMCTL2_DDR4_MRAM_EN==1*/
    volatile uint32_t MRAMTMG1;      /* Offset:0x0174; MRAM Timing Register 1. Exists: UMCTL2_DDR4_MRAM_EN==1*/
    volatile uint32_t MRAMTMG4;      /* Offset:0x0178; MRAM Timing Register 4. Exists: UMCTL2_DDR4_MRAM_EN==1*/
    volatile uint32_t MRAMTMG9;      /* Offset:0x017C; MRAM Timing Register 9. Exists: UMCTL2_DDR4_MRAM_EN==1*/
    volatile uint32_t ZQCTL0;        /* Offset:0x0180; ZQ Control Register 0. Exists: MEMC_DDR3_OR_4_OR_LPDDR2==1*/
    volatile uint32_t ZQCTL1;        /* Offset:0x0184; ZQ Control Register 1. Exists: MEMC_DDR3_OR_4_OR_LPDDR2==1*/
    volatile uint32_t ZQCTL2;        /* Offset:0x0188; ZQ Control Register 2. Exists: MEMC_LPDDR2==1*/
    volatile uint32_t ZQSTAT;        /* Offset:0x018C; ZQ Status Register. Exists: MEMC_LPDDR2==1*/
    volatile uint32_t DFITMG0;       /* Offset:0x0190; DFI Timing Register 0. Exists: Always*/
    volatile uint32_t DFITMG1;       /* Offset:0x0194; DFI Timing Register 1. Exists: Always*/
    volatile uint32_t DFILPCFG0;     /* Offset:0x0198; DFI Low Power Configuration Register 0. Exists: Always*/
    volatile uint32_t DFILPCFG1;     /* Offset:0x019C; DFI Low Power Configuration Register 1. Exists: MEMC_DDR4==1 */
    volatile uint32_t DFIUPD0;       /* Offset:0x01A0; DFI Update Register 0. Exists: Always*/
    volatile uint32_t DFIUPD1;       /* Offset:0x01A4; DFI Update Register 1. Exists: Always */
    volatile uint32_t DFIUPD2;       /* Offset:0x01A8; DFI Update Register 2. Exists: Always*/
    volatile uint32_t RSV6;          /* Offset:0x01AC; Reserved 6 */
    volatile uint32_t DFIMISC;       /* Offset:0x01B0; DFI Miscellaneous Control Register. Exists: Always*/
    volatile uint32_t DFITMG2;       /* Offset:0x01B4; DFI Timing Register 2. Exists: UMCTL2_DFI_DATA_CS_EN==1*/
    volatile uint32_t DFITMG3;       /* Offset:0x01B8; DFI Timing Register 3. Exists: MEMC_DDR4==1 && MEMC_CMD_RTN2IDLE==0 && MEMC_FREQ_RATIO==2*/
    volatile uint32_t DFISTAT;       /* Offset:0x01BC; DFI Status Register. Exists: Always*/
    volatile uint32_t DBICTL;        /* Offset:0x01C0; DM/DBI Control Register. Exists: MEMC_DDR4_OR_LPDDR4==1*/
    volatile uint32_t DFIPHYMSTR;    /* Offset:0x01C4; DFI PHY Master. Exists: Always*/
    volatile uint32_t RSV7[14];      /* Offset:0x01C8~0x01FC; Reserved 7 */
    volatile uint32_t ADDRMAP0;      /* Offset:0x0200; Address Map Register 0. Exists: (UMCTL2_RANKS_GT_1_OR_DCH_INTL_1==1)*/
    volatile uint32_t ADDRMAP1;      /* Offset:0x0204; Address Map Register 1. Exists: Always*/
    volatile uint32_t ADDRMAP2;      /* Offset:0x0208; Address Map Register 2. Exists: Always*/
    volatile uint32_t ADDRMAP3;      /* Offset:0x020C; Address Map Register 3. Exists: Always*/
    volatile uint32_t ADDRMAP4;      /* Offset:0x0210; Address Map Register 4. Exists: Always*/
    volatile uint32_t ADDRMAP5;      /* Offset:0x0214; Address Map Register 5. Exists: Always*/
    volatile uint32_t ADDRMAP6;      /* Offset:0x0218; Address Map Register 6. Exists: Always*/
    volatile uint32_t ADDRMAP7;      /* Offset:0x021C; Address Map Register 7. Exists: (MEMC_DDR4_OR_LPDDR4==1)*/
    volatile uint32_t ADDRMAP8;      /* Offset:0x0220; Address Map Register 8. Exists: (MEMC_DDR4==1)*/
    volatile uint32_t ADDRMAP9;      /* Offset:0x0224; Address Map Register 9. Exists: Always*/
    volatile uint32_t ADDRMAP10;     /* Offset:0x0228; Address Map Register 10. Exists: Always*/
    volatile uint32_t ADDRMAP11;     /* Offset:0x022C; Address Map Register 11. Exists: Always*/
    volatile uint32_t RSV8[4];       /* Offset:0x0230~0x023C; Reserved 7 */
    volatile uint32_t ODTCFG;        /* Offset:0x0240; ODT Configuration Register. Exists: Always*/
    volatile uint32_t ODTMAP;        /* Offset:0x0244; ODT/Rank Map Register. Exists: MEMC_NUM_RANKS_1_OR_2_OR_4==1*/
    volatile uint32_t RSV9[2];       /* Offset:0x0248 ~ 0x024C; Reserved 8 */
    volatile uint32_t SCHED;         /* Offset:0x0250; Scheduler Control Register. Exists: Always*/
    volatile uint32_t SCHED1;        /* Offset:0x0254; Scheduler Control Register 1. Exists: Always*/
    volatile uint32_t SCHED2;        /* Offset:0x0258; Scheduler Control Register 2. Exists: UMCTL2_DYN_BSM==1*/
    volatile uint32_t PERFHPR1;      /* Offset:0x025C; High Priority Read CAM Register 1 Exists: UPCTL2_EN_1==0*/
    volatile uint32_t RSV10;         /* Offset:0x0260; Reserved 10 */
    volatile uint32_t PERFLPR1;      /* Offset:0x0264; Low Priority Read CAM Register 1. Exists: Always*/
    volatile uint32_t RSV11;         /* Offset:0x0268; Reserved 11 */
    volatile uint32_t PERFWR1;       /* Offset:0x026C; Write CAM Register 1. Exists: Always*/
    volatile uint32_t SCHED3;        /* Offset:0x0270; Scheduler Control Register 3. Exists: MEMC_ENH_RDWR_SWITCH==1*/
    volatile uint32_t SCHED4;        /* Offset:0x0274; Scheduler Control Register 4. Exists: MEMC_ENH_RDWR_SWITCH==1*/
    volatile uint32_t RSV12[2];      /* Offset:0x0278~0x27C; Reserved 12 */
    volatile uint32_t DQMAP0;        /* Offset:0x0280; DQ Map Register 0. Exists: MEMC_DDR4==1 && UMCTL2_DQ_MAPPING==1*/
    volatile uint32_t DQMAP1;        /* Offset:0x0284; DQ Map Register 1. Exists: MEMC_DDR4==1 && UMCTL2_DQ_MAPPING==1 && MEMC_DRAM_DATA_WIDTH>23*/
    volatile uint32_t DQMAP2;        /* Offset:0x0288; DQ Map Register 2. Exists: MEMC_DDR4==1 && UMCTL2_DQ_MAPPING==1 && MEMC_DRAM_DATA_WIDTH>39*/
    volatile uint32_t DQMAP3;        /* Offset:0x028C; DQ Map Register 3. Exists: MEMC_DDR4==1 && UMCTL2_DQ_MAPPING==1 && MEMC_DRAM_DATA_WIDTH>55*/
    volatile uint32_t DQMAP4;        /* Offset:0x0290; DQ Map Register 4. Exists: MEMC_DDR4==1 && UMCTL2_DQ_MAPPING==1 && MEMC_DRAM_-DATA_WIDTH_72_OR_MEMC_SIDEBAND_ECC==1*/
    volatile uint32_t DQMAP5;        /* Offset:0x0294; DQ Map Register 5. Exists: MEMC_DDR4==1 && UMCTL2_DQ_MAPPING==1*/
    volatile uint32_t RSV13[26];     /* Offset:0x0298~0x2FC; Reserved 13 */
    volatile uint32_t DBG0;          /* Offset:0x0300; Debug Register 0. Exists: Always*/
    volatile uint32_t DBG1;          /* Offset:0x0304; Debug Register 1. Exists: Always*/
    volatile uint32_t DBGCAM;        /* Offset:0x0308; CAM Debug Register. Exists: Always*/
    volatile uint32_t DBGCMD;        /* Offset:0x030C; Command Debug Register. Exists: Always*/
    volatile uint32_t DBGSTAT;       /* Offset:0x0310; Status Debug Register. Exists: Always*/
    volatile uint32_t RSV14;         /* Offset:0x0314; Reserved 14 */
    volatile uint32_t DBGCAM1;       /* Offset:0x0318; CAM Debug Register 1 */
    volatile uint32_t RSV15;         /* Offset:0x031C; Reserved 15 */
    volatile uint32_t SWCTL;         /* Offset:0x0320; Software Register Programming Control Enable, Exists: Always*/
    volatile uint32_t SWSTAT;        /* Offset:0x0324; Software Register Programming Control Status, Exists: Always */
    volatile uint32_t SWCTLSTATIC;   /* Offset:0x0328; Static Registers Write Enable, Exists: Always */
    volatile uint32_t RSV16;         /* Offset:0x032C; Reserved 16 */
    volatile uint32_t OCPARCFG0;     /* Offset:0x0330; On-Chip Parity Configuration Register 0, Exists: UMCTL2_OCPAR_OR_OCECC_EN_1==1 */
    volatile uint32_t OCPARCFG1;     /**< Offset:0x0334; On-Chip Parity Configuration Register 1. Exists: UMCTL2_OCPAR_EN_1==1*/
    volatile uint32_t OCPARSTAT0;    /**< Offset:0x0338; On-Chip Parity Status Register 0. Exists: UMCTL2_OCPAR_OR_OCECC_EN_1==1 */
    volatile uint32_t OCPARSTAT1;    /**< Offset:0x033C; On-Chip Parity Status Register 1. Exists: UMCTL2_OCPAR_EN_1==1 */
    volatile uint32_t OCPARSTAT2;    /**< Offset:0x0340; On-Chip Parity Status Register 2. Exists: UMCTL2_OCPAR_EN_1==1 */
    volatile uint32_t OCPARSTAT3;    /**< Offset:0x0344; On-Chip Parity Status Register 3. Exists: UMCTL2_OCPAR_EN_1==1 && MEMC_INLINE_ECC==0 */
    volatile uint32_t OCPARSTAT4;    /**< Offset:0x0348; On-Chip Parity Status Register 4. Exists: UMCTL2_OCPAR_EN_1==1 && MEMC_INLINE_ECC==0 */
    volatile uint32_t OCPARSTAT5;    /**< Offset:0x034C; On-Chip Parity Status Register 5. Exists: UMCTL2_OCPAR_EN_1==1 && MEMC_INLINE_ECC==0 */
    volatile uint32_t OCPARSTAT6;    /**< Offset:0x0350; On-Chip Parity Status Register 6. Exists: UMCTL2_OCPAR_EN_1==1 && MEMC_INLINE_ECC==0 */
    volatile uint32_t OCPARSTAT7;    /**< Offset:0x0354; On-Chip Parity Status Register 7. Exists: UMCTL2_OCPAR_EN_1==1 && MEMC_INLINE_ECC==0 */
    volatile uint32_t OCECCCFG0;     /**< Offset:0x0358; On-Chip ECC Configuration Register 0. Exists: UMCTL2_OCECC_EN_1==1 */
    volatile uint32_t OCECCCFG1;     /**< Offset:0x035C; On-Chip ECC Configuration Register 1. Exists: UMCTL2_OCECC_EN_1==1 */
    volatile uint32_t OCECCSTAT0;    /**< Offset:0x0360; On-Chip ECC Status Register 0. Exists: UMCTL2_OCECC_EN_1==1 */
    volatile uint32_t OCECCSTAT1;    /**< Offset:0x0364; On-Chip ECC Status Register 1. Exists: UMCTL2_OCECC_EN_1==1 */
    volatile uint32_t OCECCSTAT2;    /**< Offset:0x0368; On-Chip ECC Status Register 2. Exists: UMCTL2_OCECC_EN_1==1 */
    volatile uint32_t POISONCFG;     /* Offset:0x036C; AXI Poison Configuration Register. Common for all AXI ports. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_AXI==1 */
    volatile uint32_t POISONSTAT;            /**< Offset:0x0370; AXI Poison Status Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_AXI==1 */
    volatile uint32_t ADVECCINDEX;           /**< Offset:0x0374; Advanced ECC Index Register. Exists: MEMC_ECC_SUPPORT>0 */
    volatile uint32_t ADVECCSTAT;            /**< Offset:0x0378; Advanced ECC Status Register. Exists: MEMC_ECC_SUPPORT==2 */
    volatile uint32_t ECCPOISONPAT0;         /**< Offset:0x037C; ECC Poison Pattern 0 Register. Exists: MEMC_ECC_SUPPORT>0 */
    volatile uint32_t ECCPOISONPAT1;         /**< Offset:0x0380; ECC Poison Pattern 1 Register. Exists: MEMC_ECC_SUPPORT>0 && MEMC_DRAM_DATA_WIDTH==64 */
    volatile uint32_t ECCPOISONPAT2;         /**< Offset:0x0384; ECC Poison Pattern 2 Register. Exists: MEMC_ECC_SUPPORT>0 */
    volatile uint32_t ECCAPSTAT;             /**< Offset:0x0388; Address protection within ECC Status Register. Exists: MEMC_ECCAP==1 */
    volatile uint32_t RSV17[5];              /**< Offset:0x038C~0x39C; Reserved 17 */
    volatile uint32_t CAPARPOISONCTL;        /**< Offset:0x03A0; CA parity poison contrl Register. Exists: MEMC_DDR4==1 && UMCTL2_CRC_PARITY_RETRY==1 */
    volatile uint32_t CAPARPOISONSTAT;       /**< Offset:0x03A4; CA parity poison status Register. Exists: MEMC_DDR4==1 && UMCTL2_CRC_PARITY_RETRY==1 */
    volatile uint32_t RSV18[2];              /**< Offset:0x03A8~0x3AC; Reserved 18 */
    volatile uint32_t DYNBSMSTAT;            /**< Offset:0x03B0; Dynamic BSM Status Register. Exists: UMCTL2_DYN_BSM==1 */
    volatile uint32_t RSV19;                 /**< Offset:0x03B4; Reserved 19 */
    volatile uint32_t CRCPARCTL3;            /**< Offset:0x03B8; CRC Parity Control Register3. Exists: UMCTL2_CRC_PARITY_RETRY==1 */
    volatile uint32_t RSV20;                 /**< Offset:0x03BC; Reserved 20 */
    volatile uint32_t REGPARCFG;             /**< Offset:0x03C0; Register Parity Configuration Register. Exists: UMCTL2_REGPAR_EN_1 */
    volatile uint32_t REGPARSTAT;            /**< Offset:0x03C4; Register Parity Status Register. Exists: UMCTL2_REGPAR_EN_1 */
    volatile uint32_t RSV21[2];              /**< Offset:0x03C8~0x3CC; Reserved 21 */
    volatile uint32_t RCDINIT1;              /**< Offset:0x03D0; Control Word setting Register RCDINIT1. Exists: UMCTL2_HWFFC_EN==1 && MEMC_DDR4==1 */
    volatile uint32_t RCDINIT2;              /**< Offset:0x03D4; Control Word setting Register RCDINIT2. Exists: UMCTL2_HWFFC_EN==1 && MEMC_DDR4==1 */
    volatile uint32_t RCDINIT3;              /**< Offset:0x03D8; Control Word setting Register RCDINIT3. Exists: UMCTL2_HWFFC_EN==1 && MEMC_DDR4==1 */
    volatile uint32_t RCDINIT4;              /**< Offset:0x03DC; Control Word setting Register RCDINIT4. Exists: UMCTL2_HWFFC_EN==1 && MEMC_DDR4==1 */
    volatile uint32_t OCCAPCFG;              /**< Offset:0x03E0; On-Chip command/Address Protection Configuration Register. Exists: UMCTL2_OCCAP_EN_1==1 */
    volatile uint32_t OCCAPSTAT;             /**< Offset:0x03E4; On-Chip command/Address Protection Status Register. Exists: UMCTL2_OCCAP_EN_1==1 && UMCTL2_INCL_ARB==1*/
    volatile uint32_t OCCAPCFG1;             /**< Offset:0x03E8; On-Chip command/Address Protection Configuration Register 1. Exists: UMCTL2_OCCAP_EN_1==1 */
    volatile uint32_t OCCAPSTAT1;            /**< Offset:0x03EC; On-Chip command/Address Protection Status Register 1. Exists: UMCTL2_OCCAP_EN_1==1 */
    volatile uint32_t DERATESTAT;            /**< Offset:0x03F0; Temperature Derate Status Register */
}kdrv_ddrc_ctrl_reg_t;


typedef struct
{
    __S_RW uint32_t BASE_ADDR;      /* [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        Exists: Always*/
} SARBASE_0_t;


typedef struct
{
    __S_RW uint32_t NBLOCKS;        /*  [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        Exists: Always*/
} SARSIZE_0_t;


typedef struct
{
    __S_RW uint32_t BASE_ADDR;      /*  [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        Exists: Always*/
} SARBASE_1_t;


typedef struct
{
    __S_RW uint32_t NBLOCKS;        /*  [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        Exists: Always*/
} SARSIZE_1_t;

typedef struct
{
    __S_RW uint32_t BASE_ADDR;      /*  [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        Exists: Always*/
} SARBASE_2_t;

typedef struct
{
    __S_RW uint32_t NBLOCKS;        /*  [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        Exists: Always*/
} SARSIZE_2_t;

typedef struct
{
    __S_RW uint32_t BASE_ADDR;      /*  [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        xists: Always*/
} SARBASE_3_t;

typedef struct
{
    __S_RW uint32_t NBLOCKS;        /*  [x:0] Base address for address region n specified as awaddr[UMCTL2_A_ADDRW-1:x] and araddr[UMCTL2_A_ADDRW-1:x],
                                        where x is determined by the minimum block size parameter UMCTL2_SARMINSIZE: (x=log2(block size)).
                                        Exists: Always*/
} SARSIZE_3_t;

/** @brief uMCTL2 Multi-Port Registers */
typedef struct
{
    volatile uint32_t PSTAT;                 /* Offset 0x03FC; Port Status Register. Exists: UMCTL2_INCL_ARB==1*/
    volatile uint32_t PCCFG;                 /* Offset:0x0400; Port Common Configuration Register. Exists: UMCTL2_INCL_ARB==1*/
    volatile uint32_t PCFGR_0;               /* Offset:0x0404; Port 0 Configuration Read Register*/
    volatile uint32_t PCFGW_0;               /* Offset:0x0408; Port 0 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_0;               /* Offset:0x040C; Port 0 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_0;      /* Offset:0x0410; Port 0 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_0;     /* Offset:0x0414; Port 0 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_0;      /* Offset:0x0418; Port 0 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_0;     /* Offset:0x041C; Port 0 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_0;      /* Offset:0x0420; Port 0 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_0;     /* Offset:0x0424; Port 0 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_0;      /* Offset:0x0428; Port 0 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_0;     /* Offset:0x042C; Port 0 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_0;      /* Offset:0x0430; Port 0 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_0;     /* Offset:0x0434; Port 0 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_0;      /* Offset:0x0438; Port 0 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_0;     /* Offset:0x043C; Port 0 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_0;      /* Offset:0x0440; Port 0 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_0;     /* Offset:0x0444; Port 0 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_0;      /* Offset:0x0448; Port 0 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_0;     /* Offset:0x044C; Port 0 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_0;      /* Offset:0x0450; Port 0 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_0;     /* Offset:0x0454; Port 0 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_0;      /* Offset:0x0458; Port 0 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_0;     /* Offset:0x045C; Port 0 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_0;     /* Offset:0x0460; Port 0 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_0;    /* Offset:0x0464; Port 0 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_0;     /* Offset:0x0468; Port 0 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_0;    /* Offset:0x046C; Port 0 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_0;     /* Offset:0x0470; Port 0 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_0;    /* Offset:0x0474; Port 0 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_0;     /* Offset:0x0478; Port 0 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_0;    /* Offset:0x047C; Port 0 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_0;     /* Offset:0x0480; Port 0 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_0;    /* Offset:0x0484; Port 0 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_0;     /* Offset:0x0488; Port 0 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_0;    /* Offset:0x048C; Port 0 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_0;               /* Offset: 0x0490; Port 0 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_0;            /* Offset: 0x0494, Port 0 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0*/
    volatile uint32_t PCFGQOS1_0;            /* Offset: 0x0498; Port 0 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1*/
    volatile uint32_t PCFGWQOS0_0;           /* Offset: 0x049C; Port 0 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_0;           /* Offset: 0x04A0; Port 0 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV0[4];               /* Offset: 0x04A4 ~ 0x04B0, Reserved 0 */
    volatile uint32_t PCFGR_1;               /* Offset: 0x04B4; Port 1 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_1;               /* Offset: 0x04B8; Port 1 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_1;               /* Offset: 0x04BC; Port 1 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_1;      /* Offset:0x04C0; Port 1 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_1;     /* Offset:0x04C4; Port 1 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_1;      /* Offset:0x04C8; Port 1 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_1;     /* Offset:0x04CC; Port 1 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_1;      /* Offset:0x04D0; Port 1 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_1;     /* Offset:0x04D4; Port 1 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_1;      /* Offset:0x04D8; Port 1 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_1;     /* Offset:0x04DC; Port 1 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_1;      /* Offset:0x04E0; Port 1 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_1;     /* Offset:0x04E4; Port 1 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_1;      /* Offset:0x04E8; Port 1 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_1;     /* Offset:0x04EC; Port 1 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_1;      /* Offset:0x04F0; Port 1 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_1;     /* Offset:0x04F4; Port 1 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_1;      /* Offset:0x04F8; Port 1 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_1;     /* Offset:0x04FC; Port 1 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_1;      /* Offset:0x0500; Port 1 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_1;     /* Offset:0x0504; Port 1 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_1;      /* Offset:0x0508; Port 1 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_1;     /* Offset:0x050C; Port 1 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_1;     /* Offset:0x0510; Port 1 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_1;    /* Offset:0x0514; Port 1 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_1;     /* Offset:0x0518; Port 1 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_1;    /* Offset:0x051C; Port 1 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_1;     /* Offset:0x0520; Port 1 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_1;    /* Offset:0x0524; Port 1 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_1;     /* Offset:0x0528; Port 1 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_1;    /* Offset:0x052C; Port 1 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_1;     /* Offset:0x0530; Port 1 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_1;    /* Offset:0x0534; Port 1 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_1;     /* Offset:0x0538; Port 1 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_1;    /* Offset:0x053C; Port 1 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_1;               /* Offset:0x0540; Port 1 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_1;            /* Offset:0x0544, Port 1 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0*/
    volatile uint32_t PCFGQOS1_1;            /* Offset:0x0548; Port 1 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1*/
    volatile uint32_t PCFGWQOS0_1;           /* Offset:0x054C; Port 1 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_1;           /* Offset:0x0550; Port 1 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV1[4];               /* Offset:0x0554 ~ 0x0560, Reserved 1 */
    volatile uint32_t PCFGR_2;               /* Offset:0x0564; Port 2 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_2;               /* Offset:0x0568; Port 2 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_2;               /* Offset:0x056C; Port 2 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_2;      /* Offset:0x0570; Port 2 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_2;     /* Offset:0x0574; Port 2 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_2;      /* Offset:0x0578; Port 2 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_2;     /* Offset:0x057C; Port 2 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_2;      /* Offset:0x0580; Port 2 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_2;     /* Offset:0x0584; Port 2 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_2;      /* Offset:0x0588; Port 2 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_2;     /* Offset:0x058C; Port 2 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_2;      /* Offset:0x0590; Port 2 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_2;     /* Offset:0x0594; Port 2 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_2;      /* Offset:0x0598; Port 2 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_2;     /* Offset:0x059C; Port 2 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_2;      /* Offset:0x05A0; Port 2 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_2;     /* Offset:0x05A4; Port 2 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_2;      /* Offset:0x05A8; Port 2 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_2;     /* Offset:0x05AC; Port 2 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_2;      /* Offset:0x05B0; Port 2 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_2;     /* Offset:0x05B4; Port 2 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_2;      /* Offset:0x05B8; Port 2 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_2;     /* Offset:0x05BC; Port 2 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_2;     /* Offset:0x05C0; Port 2 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_2;    /* Offset:0x05C4; Port 2 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_2;     /* Offset:0x05C8; Port 2 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_2;    /* Offset:0x05CC; Port 2 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_2;     /* Offset:0x05D0; Port 2 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_2;    /* Offset:0x05D4; Port 2 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_2;     /* Offset:0x05D8; Port 2 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_2;    /* Offset:0x05DC; Port 2 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_2;     /* Offset:0x05E0; Port 2 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_2;    /* Offset:0x05E4; Port 2 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_2;     /* Offset:0x05E8; Port 2 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_2;    /* Offset:0x05EC; Port 2 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_2;               /* Offset:0x05F0; Port 2 Control Register Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_2;            /* Offset:0x05F4, Port 2 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0*/
    volatile uint32_t PCFGQOS1_2;            /* Offset:0x05F8; Port 2 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1*/
    volatile uint32_t PCFGWQOS0_2;           /* Offset:0x05FC; Port 2 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_2;           /* Offset:0x0600; Port 2 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV2[4];               /* Offset:0x0604 ~ 0x0610, Reserved 2 */
    volatile uint32_t PCFGR_3;               /* Offset: 0x0614; Port 3 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_3;               /* Offset: 0x0618; Port 3 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_3;               /* Offset: 0x061C; Port 3 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_3;      /* Offset:0x0620; Port 3 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_3;     /* Offset:0x0624; Port 3 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_3;      /* Offset:0x0628; Port 3 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_3;     /* Offset:0x062C; Port 3 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_3;      /* Offset:0x0630; Port 3 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_3;     /* Offset:0x0634; Port 3 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_3;      /* Offset:0x0638; Port 3 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_3;     /* Offset:0x063C; Port 3 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_3;      /* Offset:0x0640; Port 3 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_3;     /* Offset:0x0644; Port 3 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_3;      /* Offset:0x0648; Port 3 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_3;     /* Offset:0x064C; Port 3 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_3;      /* Offset:0x0650; Port 3 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_3;     /* Offset:0x0654; Port 3 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_3;      /* Offset:0x0658; Port 3 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_3;     /* Offset:0x065C; Port 3 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_3;      /* Offset:0x0660; Port 3 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_3;     /* Offset:0x0664; Port 3 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_3;      /* Offset:0x0668; Port 3 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_3;     /* Offset:0x066C; Port 3 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_3;     /* Offset:0x0670; Port 3 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_3;    /* Offset:0x0674; Port 3 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_3;     /* Offset:0x0678; Port 3 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_3;    /* Offset:0x067C; Port 3 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_3;     /* Offset:0x0680; Port 3 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_3;    /* Offset:0x0684; Port 3 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_3;     /* Offset:0x0688; Port 3 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_3;    /* Offset:0x068C; Port 3 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_3;     /* Offset:0x0690; Port 3 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_3;    /* Offset:0x0694; Port 3 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_3;     /* Offset:0x0698; Port 3 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_3;    /* Offset:0x069C; Port 3 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_3;               /* Offset: 0x06A0; Port 3 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_3;            /* Offset: 0x06A4, Port 3 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0*/
    volatile uint32_t PCFGQOS1_3;            /* Offset: 0x06A8; Port 3 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1*/
    volatile uint32_t PCFGWQOS0_3;           /* Offset: 0x06AC; Port 3 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_3;           /* Offset: 0x06B0; Port 3 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV3[4];               /* Offset: 0x06B4 ~ 0x06C0, Reserved 3 */
    volatile uint32_t PCFGR_4;               /* Offset: 0x06C4; Port 4 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_4;               /* Offset: 0x06C8; Port 4 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_4;               /* Offset: 0x06CC; Port 4 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_4;      /* Offset: 0x06D0; Port 4 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_4;     /* Offset: 0x06D4; Port 4 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_4;      /* Offset: 0x06D8; Port 4 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_4;     /* Offset: 0x06DC; Port 4 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_4;      /* Offset: 0x06E0; Port 4 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_4;     /* Offset: 0x06E4; Port 4 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_4;      /* Offset: 0x06E8; Port 4 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_4;     /* Offset: 0x06EC; Port 4 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_4;      /* Offset: 0x06F0; Port 4 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_4;     /* Offset: 0x06F4; Port 4 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_4;      /* Offset: 0x06F8; Port 4 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_4;     /* Offset: 0x06FC; Port 4 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_4;      /* Offset: 0x0700; Port 4 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_4;     /* Offset: 0x0704; Port 4 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_4;      /* Offset: 0x0708; Port 4 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_4;     /* Offset: 0x070C; Port 4 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_4;      /* Offset: 0x0710; Port 4 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_4;     /* Offset: 0x0714; Port 4 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_4;      /* Offset: 0x0718; Port 4 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_4;     /* Offset: 0x071C; Port 4 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_4;     /* Offset: 0x0720; Port 4 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_4;    /* Offset: 0x0724; Port 4 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_4;     /* Offset: 0x0728; Port 4 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_4;    /* Offset: 0x072C; Port 4 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_4;     /* Offset: 0x0730; Port 4 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_4;    /* Offset: 0x0734; Port 4 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_4;     /* Offset: 0x0738; Port 4 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_4;    /* Offset: 0x073C; Port 4 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_4;     /* Offset: 0x0740; Port 4 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_4;    /* Offset: 0x0744; Port 4 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_4;     /* Offset: 0x0748; Port 4 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_4;    /* Offset: 0x074C; Port 4 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_4;               /* Offset: 0x0750; Port 4 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_4;            /* Offset: 0x0754; Port 4 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_4;            /* Offset: 0x0758; Port 4 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_4;           /* Offset: 0x075C; Port 4 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_4;           /* Offset: 0x0760; Port 4 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV4[4];               /* Offset: 0x0764 ~ 0x0770, Reserved 4 */
    volatile uint32_t PCFGR_5;               /* Offset: 0x0774; Port 5 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_5;               /* Offset: 0x0778; Port 5 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_5;               /* Offset: 0x077C; Port 5 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_5;      /* Offset: 0x0780; Port 5 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_5;     /* Offset: 0x0784; Port 5 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_5;      /* Offset: 0x0788; Port 5 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_5;     /* Offset: 0x078C; Port 5 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_5;      /* Offset: 0x0790; Port 5 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_5;     /* Offset: 0x0794; Port 5 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_5;      /* Offset: 0x0798; Port 5 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_5;     /* Offset: 0x079C; Port 5 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_5;      /* Offset: 0x07A0; Port 5 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_5;     /* Offset: 0x07A4; Port 5 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_5;      /* Offset: 0x07A8; Port 5 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_5;     /* Offset: 0x07AC; Port 5 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_5;      /* Offset: 0x07B0; Port 5 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_5;     /* Offset: 0x07B4; Port 5 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_5;      /* Offset: 0x07B8; Port 5 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_5;     /* Offset: 0x07BC; Port 5 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_5;      /* Offset: 0x07C0; Port 5 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_5;     /* Offset: 0x07C4; Port 5 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_5;      /* Offset: 0x07C8; Port 5 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_5;     /* Offset: 0x07CC; Port 5 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_5;     /* Offset: 0x07D0; Port 5 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_5;    /* Offset: 0x07D4; Port 5 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_5;     /* Offset: 0x07D8; Port 5 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_5;    /* Offset: 0x07DC; Port 5 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_5;     /* Offset: 0x07E0; Port 5 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_5;    /* Offset: 0x07E4; Port 5 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_5;     /* Offset: 0x07E8; Port 5 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_5;    /* Offset: 0x07EC; Port 5 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_5;     /* Offset: 0x07F0; Port 5 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_5;    /* Offset: 0x07F4; Port 5 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_5;     /* Offset: 0x07F8; Port 5 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_5;    /* Offset: 0x07FC; Port 5 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_5;               /* Offset: 0x0800; Port 5 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_5;            /* Offset: 0x0804; Port 5 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_5;            /* Offset: 0x0808; Port 5 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_5;           /* Offset: 0x080C; Port 5 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_5;           /* Offset: 0x0810; Port 5 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV5[4];               /* Offset: 0x0814 ~ 0x0820, Reserved 5 */
    volatile uint32_t PCFGR_6;               /* Offset: 0x0824; Port 6 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_6;               /* Offset: 0x0828; Port 6 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_6;               /* Offset: 0x082C; Port 6 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_6;      /* Offset: 0x0830; Port 6 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_6;     /* Offset: 0x0834; Port 6 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_6;      /* Offset: 0x0838; Port 6 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_6;     /* Offset: 0x083C; Port 6 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_6;      /* Offset: 0x0840; Port 6 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_6;     /* Offset: 0x0844; Port 6 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_6;      /* Offset: 0x0848; Port 6 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_6;     /* Offset: 0x084C; Port 6 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_6;      /* Offset: 0x0850; Port 6 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_6;     /* Offset: 0x0854; Port 6 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_6;      /* Offset: 0x0858; Port 6 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_6;     /* Offset: 0x085C; Port 6 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_6;      /* Offset: 0x0860; Port 6 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_6;     /* Offset: 0x0864; Port 6 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_6;      /* Offset: 0x0868; Port 6 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_6;     /* Offset: 0x086C; Port 6 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_6;      /* Offset: 0x0870; Port 6 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_6;     /* Offset: 0x0874; Port 6 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_6;      /* Offset: 0x0878; Port 6 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_6;     /* Offset: 0x087C; Port 6 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_6;     /* Offset: 0x0880; Port 6 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_6;    /* Offset: 0x0884; Port 6 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_6;     /* Offset: 0x0888; Port 6 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_6;    /* Offset: 0x088C; Port 6 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_6;     /* Offset: 0x0890; Port 6 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_6;    /* Offset: 0x0894; Port 6 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_6;     /* Offset: 0x0898; Port 6 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_6;    /* Offset: 0x089C; Port 6 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_6;     /* Offset: 0x08A0; Port 6 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_6;    /* Offset: 0x08A4; Port 6 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_6;     /* Offset: 0x08A8; Port 6 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_6;    /* Offset: 0x08AC; Port 6 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_6;               /* Offset: 0x08B0; Port 6 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_6;            /* Offset: 0x08B4; Port 6 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_6;            /* Offset: 0x08B8; Port 6 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_6;           /* Offset: 0x08BC; Port 6 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_6;           /* Offset: 0x08C0; Port 6 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV6[4];               /* Offset: 0x08C4 ~ 0x08D0, Reserved 6 */
    volatile uint32_t PCFGR_7;               /* Offset: 0x08D4; Port 7 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_7;               /* Offset: 0x08D8; Port 7 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_7;               /* Offset: 0x08DC; Port 7 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_7;      /* Offset: 0x08E0; Port 7 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_7;     /* Offset: 0x08E4; Port 7 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_7;      /* Offset: 0x08E8; Port 7 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_7;     /* Offset: 0x08EC; Port 7 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_7;      /* Offset: 0x08F0; Port 7 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_7;     /* Offset: 0x08F4; Port 7 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_7;      /* Offset: 0x08F8; Port 7 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_7;     /* Offset: 0x08FC; Port 7 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_7;      /* Offset: 0x0900; Port 7 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_7;     /* Offset: 0x0904; Port 7 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_7;      /* Offset: 0x0908; Port 7 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_7;     /* Offset: 0x090C; Port 7 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_7;      /* Offset: 0x0910; Port 7 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_7;     /* Offset: 0x0914; Port 7 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_7;      /* Offset: 0x0918; Port 7 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_7;     /* Offset: 0x091C; Port 7 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_7;      /* Offset: 0x0920; Port 7 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_7;     /* Offset: 0x0924; Port 7 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_7;      /* Offset: 0x0928; Port 7 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_7;     /* Offset: 0x092C; Port 7 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_7;     /* Offset: 0x0930; Port 7 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_7;    /* Offset: 0x0934; Port 7 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_7;     /* Offset: 0x0938; Port 7 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_7;    /* Offset: 0x093C; Port 7 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_7;     /* Offset: 0x0940; Port 7 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_7;    /* Offset: 0x0944; Port 7 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_7;     /* Offset: 0x0948; Port 7 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_7;    /* Offset: 0x094C; Port 7 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_7;     /* Offset: 0x0950; Port 7 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_7;    /* Offset: 0x0954; Port 7 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_7;     /* Offset: 0x0958; Port 7 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_7;    /* Offset: 0x095C; Port 7 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_7;               /* Offset: 0x0960; Port 7 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_7;            /* Offset: 0x0964; Port 7 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_7;            /* Offset: 0x0968; Port 7 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_7;           /* Offset: 0x096C; Port 7 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_7;           /* Offset: 0x0970; Port 7 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV7[4];               /* Offset: 0x0974 ~ 0x0980, Reserved 7 */
    volatile uint32_t PCFGR_8;               /* Offset: 0x0984; Port 8 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_8;               /* Offset: 0x0988; Port 8 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_8;               /* Offset: 0x098C; Port 8 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_8;      /* Offset: 0x0990; Port 8 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_8;     /* Offset: 0x0994; Port 8 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_8;      /* Offset: 0x0998; Port 8 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_8;     /* Offset: 0x099C; Port 8 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_8;      /* Offset: 0x09A0; Port 8 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_8;     /* Offset: 0x09A4; Port 8 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_8;      /* Offset: 0x09A8; Port 8 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_8;     /* Offset: 0x09AC; Port 8 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_8;      /* Offset: 0x09B0; Port 8 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_8;     /* Offset: 0x09B4; Port 8 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_8;      /* Offset: 0x09B8; Port 8 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_8;     /* Offset: 0x09BC; Port 8 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_8;      /* Offset: 0x09C0; Port 8 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_8;     /* Offset: 0x09C4; Port 8 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_8;      /* Offset: 0x09C8; Port 8 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_8;     /* Offset: 0x09CC; Port 8 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_8;      /* Offset: 0x09D0; Port 8 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_8;     /* Offset: 0x09D4; Port 8 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_8;      /* Offset: 0x09D8; Port 8 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_8;     /* Offset: 0x09DC; Port 8 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_8;     /* Offset: 0x09E0; Port 8 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_8;    /* Offset: 0x09E4; Port 8 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_8;     /* Offset: 0x09E8; Port 8 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_8;    /* Offset: 0x09EC; Port 8 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_8;     /* Offset: 0x09F0; Port 8 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_8;    /* Offset: 0x09F4; Port 8 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_8;     /* Offset: 0x09F8; Port 8 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_8;    /* Offset: 0x09FC; Port 8 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_8;     /* Offset: 0x0A00; Port 8 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_8;    /* Offset: 0x0A04; Port 8 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_8;     /* Offset: 0x0A08; Port 8 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_8;    /* Offset: 0x0A0C; Port 8 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_8;               /* Offset: 0x0A10; Port 8 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_8;            /* Offset: 0x0A14; Port 8 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_8;            /* Offset: 0x0A18; Port 8 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_8;           /* Offset: 0x0A1C; Port 8 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_8;           /* Offset: 0x0A20; Port 8 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV8[4];               /* Offset: 0x0A24 ~ 0x0A30, Reserved 8 */
    volatile uint32_t PCFGR_9;               /* Offset: 0x0A34; Port 9 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_9;               /* Offset: 0x0A38; Port 9 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_9;               /* Offset: 0x0A3C; Port 9 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_9;      /* Offset: 0x0A40; Port 9 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_9;     /* Offset: 0x0A44; Port 9 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_9;      /* Offset: 0x0A48; Port 9 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_9;     /* Offset: 0x0A4C; Port 9 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_9;      /* Offset: 0x0A50; Port 9 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_9;     /* Offset: 0x0A54; Port 9 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_9;      /* Offset: 0x0A58; Port 9 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_9;     /* Offset: 0x0A5C; Port 9 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_9;      /* Offset: 0x0A60; Port 9 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_9;     /* Offset: 0x0A64; Port 9 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_9;      /* Offset: 0x0A68; Port 9 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_9;     /* Offset: 0x0A6C; Port 9 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_9;      /* Offset: 0x0A70; Port 9 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_9;     /* Offset: 0x0A74; Port 9 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_9;      /* Offset: 0x0A78; Port 9 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_9;     /* Offset: 0x0A7C; Port 9 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_9;      /* Offset: 0x0A80; Port 9 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_9;     /* Offset: 0x0A84; Port 9 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_9;      /* Offset: 0x0A88; Port 9 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_9;     /* Offset: 0x0A8C; Port 9 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_9;     /* Offset: 0x0A90; Port 9 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_9;    /* Offset: 0x0A94; Port 9 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_9;     /* Offset: 0x0A98; Port 9 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_9;    /* Offset: 0x0A9C; Port 9 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_9;     /* Offset: 0x0AA0; Port 9 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_9;    /* Offset: 0x0AA4; Port 9 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_9;     /* Offset: 0x0AA8; Port 9 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_9;    /* Offset: 0x0AAC; Port 9 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_9;     /* Offset: 0x0AB0; Port 9 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_9;    /* Offset: 0x0AB4; Port 9 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_9;     /* Offset: 0x0AB8; Port 9 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_9;    /* Offset: 0x0ABC; Port 9 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_9;               /* Offset: 0x0AC0; Port 9 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_9;            /* Offset: 0x0AC4; Port 9 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_9;            /* Offset: 0x0AC8; Port 9 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_9;           /* Offset: 0x0ACC; Port 9 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_9;           /* Offset: 0x0AD0; Port 9 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV9[4];               /* Offset: 0x0AD4 ~ 0x0AE0, Reserved 9 */
    volatile uint32_t PCFGR_10;              /* Offset: 0x0AE4; Port 10 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_10;              /* Offset: 0x0AE8; Port 10 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_10;              /* Offset: 0x0AEC; Port 10 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_10;     /* Offset: 0x0AF0; Port 10 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_10;    /* Offset: 0x0AF4; Port 10 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_10;     /* Offset: 0x0AF8; Port 10 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_10;    /* Offset: 0x0AFC; Port 10 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_10;     /* Offset: 0x0B00; Port 10 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_10;    /* Offset: 0x0B04; Port 10 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_10;     /* Offset: 0x0B08; Port 10 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_10;    /* Offset: 0x0B0C; Port 10 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_10;     /* Offset: 0x0B10; Port 10 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_10;    /* Offset: 0x0B14; Port 10 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_10;     /* Offset: 0x0B18; Port 10 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_10;    /* Offset: 0x0B1C; Port 10 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_10;     /* Offset: 0x0B20; Port 10 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_10;    /* Offset: 0x0B24; Port 10 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_10;     /* Offset: 0x0B28; Port 10 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_10;    /* Offset: 0x0B2C; Port 10 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_10;     /* Offset: 0x0B30; Port 10 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_10;    /* Offset: 0x0B34; Port 10 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_10;     /* Offset: 0x0B38; Port 10 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_10;    /* Offset: 0x0B3C; Port 10 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_10;    /* Offset: 0x0B40; Port 10 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_10;   /* Offset: 0x0B44; Port 10 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_10;    /* Offset: 0x0B48; Port 10 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_10;   /* Offset: 0x0B4C; Port 10 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_10;    /* Offset: 0x0B50; Port 10 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_10;   /* Offset: 0x0B54; Port 10 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_10;    /* Offset: 0x0B58; Port 10 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_10;   /* Offset: 0x0B5C; Port 10 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_10;    /* Offset: 0x0B60; Port 10 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_10;   /* Offset: 0x0B64; Port 10 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_10;    /* Offset: 0x0B68; Port 10 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_10;   /* Offset: 0x0B6C; Port 10 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_10;              /* Offset: 0x0B70; Port 10 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_10;           /* Offset: 0x0B74; Port 10 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_10;           /* Offset: 0x0B78; Port 10 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_10;          /* Offset: 0x0B7C; Port 10 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_10;          /* Offset: 0x0B80; Port 10 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV10[4];              /* Offset: 0x0B84 ~ 0x0B90, Reserved 10 */
    volatile uint32_t PCFGR_11;              /* Offset: 0x0B94; Port 11 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_11;              /* Offset: 0x0B98; Port 11 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_11;              /* Offset: 0x0B9C; Port 11 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_11;     /* Offset: 0x0BA0; Port 11 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_11;    /* Offset: 0x0BA4; Port 11 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_11;     /* Offset: 0x0BA8; Port 11 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_11;    /* Offset: 0x0BAC; Port 11 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_11;     /* Offset: 0x0BB0; Port 11 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_11;    /* Offset: 0x0BB4; Port 11 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_11;     /* Offset: 0x0BB8; Port 11 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_11;    /* Offset: 0x0BBC; Port 11 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_11;     /* Offset: 0x0BC0; Port 11 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_11;    /* Offset: 0x0BC4; Port 11 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_11;     /* Offset: 0x0BC8; Port 11 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_11;    /* Offset: 0x0BCC; Port 11 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_11;     /* Offset: 0x0BD0; Port 11 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_11;    /* Offset: 0x0BD4; Port 11 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_11;     /* Offset: 0x0BD8; Port 11 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_11;    /* Offset: 0x0BDC; Port 11 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_11;     /* Offset: 0x0BE0; Port 11 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_11;    /* Offset: 0x0BE4; Port 11 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_11;     /* Offset: 0x0BE8; Port 11 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_11;    /* Offset: 0x0BEC; Port 11 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_11;    /* Offset: 0x0BF0; Port 11 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_11;   /* Offset: 0x0BF4; Port 11 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_11;    /* Offset: 0x0BF8; Port 11 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_11;   /* Offset: 0x0BFC; Port 11 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_11;    /* Offset: 0x0C00; Port 11 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_11;   /* Offset: 0x0C04; Port 11 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_11;    /* Offset: 0x0C08; Port 11 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_11;   /* Offset: 0x0C0C; Port 11 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_11;    /* Offset: 0x0C10; Port 11 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_11;   /* Offset: 0x0C14; Port 11 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_11;    /* Offset: 0x0C18; Port 11 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_11;   /* Offset: 0x0C1C; Port 11 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_11;              /* Offset: 0x0C20; Port 11 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_11;           /* Offset: 0x0C24; Port 11 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_11;           /* Offset: 0x0C28; Port 11 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_11;          /* Offset: 0x0C2C; Port 11 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_11;           /* Offset: 0x0C30; Port 11 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV11[4];              /* Offset: 0x0C34 ~ 0x0C40, Reserved 11 */
    volatile uint32_t PCFGR_12;              /* Offset: 0x0C44; Port 12 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_12;              /* Offset: 0x0C48; Port 12 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_12;              /* Offset: 0x0C4C; Port 12 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_12;     /* Offset: 0x0C50; Port 12 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_12;    /* Offset: 0x0C54; Port 12 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_12;     /* Offset: 0x0C58; Port 12 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_12;    /* Offset: 0x0C5C; Port 12 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_12;     /* Offset: 0x0C60; Port 12 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_12;    /* Offset: 0x0C64; Port 12 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_12;     /* Offset: 0x0C68; Port 12 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_12;    /* Offset: 0x0C6C; Port 12 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_12;     /* Offset: 0x0C70; Port 12 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_12;    /* Offset: 0x0C74; Port 12 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_12;     /* Offset: 0x0C78; Port 12 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_12;    /* Offset: 0x0C7C; Port 12 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_12;     /* Offset: 0x0C80; Port 12 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_12;    /* Offset: 0x0C84; Port 12 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_12;     /* Offset: 0x0C88; Port 12 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_12;    /* Offset: 0x0C8C; Port 12 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_12;     /* Offset: 0x0C90; Port 12 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_12;    /* Offset: 0x0C94; Port 12 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_12;     /* Offset: 0x0C98; Port 12 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_12;    /* Offset: 0x0C9C; Port 12 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_12;    /* Offset: 0x0CA0; Port 12 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_12;   /* Offset: 0x0CA4; Port 12 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_12;    /* Offset: 0x0CA8; Port 12 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_12;   /* Offset: 0x0CAC; Port 12 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_12;    /* Offset: 0x0CB0; Port 12 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_12;   /* Offset: 0x0CB4; Port 12 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_12;    /* Offset: 0x0CB8; Port 12 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_12;   /* Offset: 0x0CBC; Port 12 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_12;    /* Offset: 0x0CC0; Port 12 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_12;   /* Offset: 0x0CC4; Port 12 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_12;    /* Offset: 0x0CC8; Port 12 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_12;   /* Offset: 0x0CCC; Port 12 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_12;              /* Offset: 0x0CD0; Port 12 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_12;           /* Offset: 0x0CD4; Port 12 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_12;           /* Offset: 0x0CD8; Port 12 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_12;          /* Offset: 0x0CEC; Port 12 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_12;          /* Offset: 0x0CE0; Port 12 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV12[4];              /* Offset: 0x0CE4 ~ 0x0CF0, Reserved 12 */
    volatile uint32_t PCFGR_13;              /* Offset: 0x0CF4; Port 13 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_13;              /* Offset: 0x0CF8; Port 13 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_13;              /* Offset: 0x0CFC; Port 13 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_13;     /* Offset: 0x0D00; Port 13 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_13;    /* Offset: 0x0D04; Port 13 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_13;     /* Offset: 0x0D08; Port 13 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_13;    /* Offset: 0x0D0C; Port 13 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_13;     /* Offset: 0x0D10; Port 13 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_13;    /* Offset: 0x0D14; Port 13 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_13;     /* Offset: 0x0D18; Port 13 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_13;    /* Offset: 0x0D1C; Port 13 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_13;     /* Offset: 0x0D20; Port 13 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_13;    /* Offset: 0x0D24; Port 13 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_13;     /* Offset: 0x0D28; Port 13 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_13;    /* Offset: 0x0D2C; Port 13 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_13;     /* Offset: 0x0D30; Port 13 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_13;    /* Offset: 0x0D34; Port 13 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_13;     /* Offset: 0x0D38; Port 13 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_13;    /* Offset: 0x0D3C; Port 13 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_13;     /* Offset: 0x0D40; Port 13 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_13;    /* Offset: 0x0D44; Port 13 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_13;     /* Offset: 0x0D48; Port 13 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_13;    /* Offset: 0x0D4C; Port 13 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_13;    /* Offset: 0x0D50; Port 13 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_13;   /* Offset: 0x0D54; Port 13 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_13;    /* Offset: 0x0D58; Port 13 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_13;   /* Offset: 0x0D5C; Port 13 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_13;    /* Offset: 0x0D60; Port 13 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_13;   /* Offset: 0x0D64; Port 13 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_13;    /* Offset: 0x0D68; Port 13 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_13;   /* Offset: 0x0D6C; Port 13 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_13;    /* Offset: 0x0D70; Port 13 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_13;   /* Offset: 0x0D74; Port 13 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_13;    /* Offset: 0x0D78; Port 13 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_13;   /* Offset: 0x0D7C; Port 13 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_13;              /* Offset: 0x0D80; Port 13 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_13;           /* Offset: 0x0D84; Port 13 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_13;           /* Offset: 0x0D88; Port 13 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_13;          /* Offset: 0x0D8C; Port 13 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_13;          /* Offset: 0x0D90; Port 13 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV13[4];              /* Offset: 0x0D94 ~ 0x0DA0, Reserved 13 */
    volatile uint32_t PCFGR_14;              /* Offset: 0x0DA4; Port 14 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_14;              /* Offset: 0x0DA8; Port 14 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_14;              /* Offset: 0x0DAC; Port 14 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_14;     /* Offset: 0x0DB0; Port 14 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_14;    /* Offset: 0x0DB4; Port 14 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_14;     /* Offset: 0x0DB8; Port 14 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_14;    /* Offset: 0x0DBC; Port 14 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_14;     /* Offset: 0x0DC0; Port 14 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_14;    /* Offset: 0x0DC4; Port 14 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_14;     /* Offset: 0x0DC8; Port 14 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_14;    /* Offset: 0x0DCC; Port 14 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_14;     /* Offset: 0x0DD0; Port 14 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_14;    /* Offset: 0x0DD4; Port 14 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_14;     /* Offset: 0x0DD8; Port 14 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_14;    /* Offset: 0x0DDC; Port 14 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_14;     /* Offset: 0x0DE0; Port 14 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_14;    /* Offset: 0x0DE4; Port 14 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_14;     /* Offset: 0x0DE8; Port 14 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_14;    /* Offset: 0x0DEC; Port 14 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_14;     /* Offset: 0x0DF0; Port 14 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_14;    /* Offset: 0x0DF4; Port 14 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_14;     /* Offset: 0x0DF8; Port 14 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_14;    /* Offset: 0x0DFC; Port 14 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_14;    /* Offset: 0x0E00; Port 14 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_14;   /* Offset: 0x0E04; Port 14 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_14;    /* Offset: 0x0E08; Port 14 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_14;   /* Offset: 0x0E0C; Port 14 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_14;    /* Offset: 0x0E10; Port 14 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_14;   /* Offset: 0x0E14; Port 14 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_14;    /* Offset: 0x0E18; Port 14 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_14;   /* Offset: 0x0E1C; Port 14 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_14;    /* Offset: 0x0E20; Port 14 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_14;   /* Offset: 0x0E24; Port 14 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_14;    /* Offset: 0x0E28; Port 14 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_14;   /* Offset: 0x0E2C; Port 14 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_14;              /* Offset: 0x0E30; Port 14 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_14;           /* Offset: 0x0E34; Port 14 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_14;           /* Offset: 0x0E38; Port 14 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_14;          /* Offset: 0x0E3C; Port 14 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_14;          /* Offset: 0x0E40; Port 14 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV14[4];              /* Offset: 0x0E44 ~ 0x0E50, Reserved 14 */
    volatile uint32_t PCFGR_15;              /* Offset: 0x0E54; Port 15 Configuration Read Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGW_15;              /* Offset: 0x0E58; Port 15 Configuration Write Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGC_15;              /* Offset: 0x0E5C; Port 15 Common Configuration Register*/
    volatile uint32_t PCFGIDMASKCH_0_15;     /* Offset: 0x0E60; Port 15 Channel 0 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_0_15;    /* Offset: 0x0E64; Port 15 Channel 0 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_1_15;     /* Offset: 0x0E68; Port 15 Channel 1 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_1_15;    /* Offset: 0x0E6C; Port 15 Channel 1 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_2_15;     /* Offset: 0x0E70; Port 15 Channel 2 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_2_15;    /* Offset: 0x0E74; Port 15 Channel 2 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_3_15;     /* Offset: 0x0E78; Port 15 Channel 3 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_3_15;    /* Offset: 0x0E7C; Port 15 Channel 3 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_4_15;     /* Offset: 0x0E80; Port 15 Channel 4 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_4_15;    /* Offset: 0x0E84; Port 15 Channel 4 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_5_15;     /* Offset: 0x0E88; Port 15 Channel 5 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_5_15;    /* Offset: 0x0E8C; Port 15 Channel 5 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_6_15;     /* Offset: 0x0E80; Port 15 Channel 6 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_6_15;    /* Offset: 0x0E94; Port 15 Channel 6 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_7_15;     /* Offset: 0x0E98; Port 15 Channel 7 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_7_15;    /* Offset: 0x0E9C; Port 15 Channel 7 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_8_15;     /* Offset: 0x0EA0; Port 15 Channel 8 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_8_15;    /* Offset: 0x0EA4; Port 15 Channel 8 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_9_15;     /* Offset: 0x0EA8; Port 15 Channel 9 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_9_15;    /* Offset: 0x0EAC; Port 15 Channel 9 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_10_15;    /* Offset: 0x0EB0; Port 15 Channel 10 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_10_15;   /* Offset: 0x0EB4; Port 15 Channel 10 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_11_15;    /* Offset: 0x0EB8; Port 15 Channel 11 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_11_15;   /* Offset: 0x0EBC; Port 15 Channel 11 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_12_15;    /* Offset: 0x0EC0; Port 15 Channel 12 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_12_15;   /* Offset: 0x0EC4; Port 15 Channel 12 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_13_15;    /* Offset: 0x0EC8; Port 15 Channel 13 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_13_15;   /* Offset: 0x0ECC; Port 15 Channel 13 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_14_15;    /* Offset: 0x0ED0; Port 15 Channel 14 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_14_15;   /* Offset: 0x0ED4; Port 15 Channel 14 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDMASKCH_15_15;    /* Offset: 0x0ED8; Port 15 Channel 15 Configuration ID Mask Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCFGIDVALUECH_15_15;   /* Offset: 0x0EDC; Port 15 Channel 15 Configuration ID Value Register. Exists: UMCTL2_PORT_CH0_0==1*/
    volatile uint32_t PCTRL_15;              /* Offset: 0x0EE0; Port 15 Control Register. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_PORT_0==1*/
    volatile uint32_t PCFGQOS0_15;           /* Offset: 0x0EE4; Port 15 Read QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UPCTL2_EN_1 == 0 */
    volatile uint32_t PCFGQOS1_15;           /* Offset: 0x0EE8; Port 15 Read QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPR_0==1 */
    volatile uint32_t PCFGWQOS0_15;          /* Offset: 0x0EEC; Port 15 Write QoS Configuration Register 0. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1*/
    volatile uint32_t PCFGWQOS1_15;          /* Offset: 0x0EF0; Port 15 Write QoS Configuration Register 1. Exists: UMCTL2_A_AXI_0==1 && UMCTL2_XPI_VPW_0==1 */
    volatile uint32_t RSV15[4];              /* Offset: 0x0EF4 ~ 0x0F00, Reserved 15 */
    SARBASE_0_t SARBASE_0;          /* Offset: 0x0F04; SAR Base Address Register 0. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    SARSIZE_0_t SARSIZE_0;          /* Offset: 0x0F08; SAR Size Register 0. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    SARBASE_1_t SARBASE_1;          /* Offset: 0x0F0C; SAR Base Address Register 1. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    SARSIZE_1_t SARSIZE_1;          /* Offset: 0x0F10; SAR Size Register 1. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    SARBASE_2_t SARBASE_2;          /* Offset: 0x0F14; SAR Base Address Register 2. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    SARSIZE_2_t SARSIZE_2;          /* Offset: 0x0F18; SAR Size Register 2. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    SARBASE_3_t SARBASE_3;          /* Offset: 0x0F1C; SAR Base Address Register 3. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    SARSIZE_3_t SARSIZE_3;          /* Offset: 0x0F20; SAR Size Register 3. Exists: UMCTL2_INCL_ARB==1 && UMCTL2_A_SAR_0==1 */
    volatile uint32_t SBRCTL;                /* Offset: 0x0F24; Scrubber Control Register */
    volatile uint32_t SBRSTAT;               /* Offset: 0x0F28; Scrubber Status Register */
    volatile uint32_t SBRWDATA0;             /* Offset: 0x0F2C; Scrubber Write Data Pattern0 */
    volatile uint32_t SBRWDATA1;             /* Offset: 0x0F30; Scrubber Write Data Pattern1 */
    volatile uint32_t PDCH;                  /* Offset: 0x0F34; Port Data Channel */
    volatile uint32_t SBRSTART0;             /* Offset: 0x0F38; Scrubber Start Address Mask Register 0 */
    volatile uint32_t SBRSTART1;             /* Offset: 0x0F3C; Scrubber Start Address Mask Register 1 */
    volatile uint32_t SBRRANGE0;             /* Offset: 0x0F40; Scrubber Address Range Mask Register 0 */
    volatile uint32_t SBRRANGE1;             /* Offset: 0x0F44; Scrubber Address Range Mask Register 1 */
    volatile uint32_t SBRSTART0DCH1;         /* Offset: 0x0F48; Scrubber Start Address Mask Register 0 for Data Channel 1 */
    volatile uint32_t SBRSTART1DCH1;         /* Offset: 0x0F4C; Scrubber Start Address Mask Register 1 for Data Channel 1 */
    volatile uint32_t SBRRANGE0DCH1;         /* Offset: 0x0F50; Scrubber Address Range Mask Register 0 for Data Channel 1 */
    volatile uint32_t SBRRANGE1DCH1;         /* Offset: 0x0F54; Scrubber Address Range Mask Register 1 for Data Channel 1 */
    volatile uint32_t RSV16[37];             /* Offset: 0x0F58~0x0FE8; Reserved 3 */
    volatile uint32_t UMCTL2_VER_NUM;        /* Offset: 0x0FF0; UMCTL2 Version Number Register */
    volatile uint32_t UMCTL2_VER_TYPE;       /* Offset: 0x0FF4; UMCTL2 Version Type Register */
} kdrv_ddrc_mp_reg_t;


/** @brief DWC_DDRPHY/DWC_DDRPHY_PUB Registers */
typedef struct
{
    volatile uint32_t PIDR;      /* Offset:0x0; Revision Identification Register */
    volatile uint32_t PIR;       /* Offset:0x1; PHY Initialization Register. */
    volatile uint32_t CGCR;      /* Offset:0x2; Clock Gating Configuration Register. Exists: Always*/
    volatile uint32_t CGCR1;     /* Offset:0x3; Clock Gating Configuration Register 1*/
    volatile uint32_t PGCR0;     /* Offset:0x4; PHY General Configuration Register 0*/
    volatile uint32_t PGCR1;     /* Offset:0x5; PHY General Configuration Register 1.*/
    volatile uint32_t PGCR2;     /* Offset:0x6; PHY General Configuration Register 2.*/
    volatile uint32_t PGCR3;     /* Offset:0x7; PHY General Configuration Register 3..*/
    volatile uint32_t PGCR4;     /* Offset:0x8; PHY General Configuration Register 4*/
    volatile uint32_t PGCR5;     /* Offset:0x9; PHY General Configuration Register 5*/
    volatile uint32_t PGCR6;     /* Offset:0xa; PHY General Configuration Register 6*/
    volatile uint32_t PGCR7;     /* Offset:0xb; PHY General Configuration Register 7.*/
    volatile uint32_t PGCR8;     /* Offset:0xc; PHY General Configuration Register 8*/
    volatile uint32_t PGSR0;     /* Offset:0xd; PHY General Status Register 0*/
    volatile uint32_t PGSR1;     /* Offset:0xe; PHY General Status Register 1*/
    volatile uint32_t RSV_f;     /* Offset:0xf; Reserved*/
    volatile uint32_t PTR0;      /* Offset:0x10; PHY Timing Register 0. */
    volatile uint32_t PTR1;      /* Offset:0x11; PHY Timing Register 1. */
    volatile uint32_t PTR2;      /* Offset:0x12; PHY Timing Register 2*/
    volatile uint32_t PTR3;      /* Offset:0x13; PHY Timing Register 3*/
    volatile uint32_t PTR4;      /* Offset:0x14; PHY Timing Register 4*/
    volatile uint32_t PTR5;      /* Offset:0x15; PHY Timing Register 5*/
    volatile uint32_t PTR6;      /* Offset:0x16; PHY Timing Register 6*/
    volatile uint32_t RSV_17_19[3];     /* Offset:0x17~0x19; Reserved*/
    volatile uint32_t PLLCR0;    /* Offset:0x1a; PPLL Control Register 0 (Type B PLL only)*/
    volatile uint32_t PLLCR1;    /* Offset:0x1b; PPLL Control Register 1 (Type B PLL only)*/
    volatile uint32_t PLLCR2;    /* Offset:0x1c; PPLL Control Register 2 (Type B PLL only)*/
    volatile uint32_t PLLCR3;    /* Offset:0x1d; PPLL Control Register 3 (Type B PLL only)*/
    volatile uint32_t PLLCR4;    /* Offset:0x1e; PPLL Control Register 4 (Type B PLL only)*/
    volatile uint32_t PLLCR5;    /* Offset:0x1f; PPLL Control Register 5 (Type B PLL only)*/
    volatile uint32_t PLLCR;     /* Offset:0x20; PLL Control Register (Type A PLL only)*/
    volatile uint32_t RSV_21;    /* Offset:0x21; Reserved*/
    volatile uint32_t DXCCR;     /* Offset:0x22; DATX8 Common Configuration Register.*/
    volatile uint32_t RSV_23;    /* Offset:0x23; Reserved*/
    volatile uint32_t DSGCR;     /* Offset:0x24; DDR System General Configuration Register. Exists: Always*/
    volatile uint32_t RSV_25;    /* Offset:0x25; Reserved*/
    volatile uint32_t ODTCR;     /* Offset:0x26; ODT Configuration Register*/
    volatile uint32_t RSV_27;    /* Offset:0x27; Reserved*/
    volatile uint32_t AACR;      /* Offset:0x28; Anti-Aging Control Register*/
    volatile uint32_t RSV_29_2f[7];    /* Offset:0x29~0x2f; Reserved*/
    volatile uint32_t GPR0;      /* Offset:0x30; General Purpose Register 0*/
    volatile uint32_t GPR1;      /* Offset:0x31; General Purpose Register 1*/
    volatile uint32_t RSV_32_3f[14];   /* Offset:0x32~0x3f; Reserved*/
    volatile uint32_t DCR;       /* Offset:0x40; DRAM Configuration Register. */
    volatile uint32_t RSV_41_43[3];   /* Offset:0x41~0x43; Reserved*/
    volatile uint32_t DTPR0;     /* Offset:0x44; DRAM Timing Parameters Register 0.*/
    volatile uint32_t DTPR1;     /* Offset:0x45; DRAM Timing Parameters Register 1. */
    volatile uint32_t DTPR2;     /* Offset:0x46; DRAM Timing Parameters Register 2. */
    volatile uint32_t DTPR3;     /* Offset:0x47; DRAM Timing Parameters Register 3. */
    volatile uint32_t DTPR4;     /* Offset:0x48; DRAM Timing Parameters Register 4. */
    volatile uint32_t DTPR5;         /* Offset:0x49; DRAM Timing Parameters Register 5. */
    volatile uint32_t DTPR6;         /* Offset:0x4a; DRAM Timing Parameters Register 6*/
    volatile uint32_t RSV_4b_4f[5];  /* Offset:0x4b~4f; Reserved*/
    volatile uint32_t RDIMMGCR0;     /* Offset:0x50; RDIMM General Configuration Register 0*/
    volatile uint32_t RDIMMGCR1;     /* Offset:0x51; RDIMM General Configuration Register 1*/
    volatile uint32_t RDIMMGCR2;     /* Offset:0x52; RDIMM General Configuration Register 2*/
    volatile uint32_t RSV_53;        /* Offset:0x53; Reserved*/
    volatile uint32_t RDIMMCR0;      /* Offset:0x54; RDIMM Control Register 0 (DDR3)*/
    volatile uint32_t RDIMMCR1;      /* Offset:0x55; RDIMM Control Register 1 (DDR3)*/
    volatile uint32_t RDIMMCR2;      /* Offset:0x56; RDIMM Control Register 2*/
    volatile uint32_t RDIMMCR3;      /* Offset:0x57; RDIMM Control Register 3*/
    volatile uint32_t RDIMMCR4;      /* Offset:0x58; RDIMM Control Register 4*/
    volatile uint32_t RSV_59;        /* Offset:0x59; Reserved*/
    volatile uint32_t SCHCR0;        /* Offset:0x5a; Scheduler Command Register 0*/
    volatile uint32_t SCHCR1;        /* Offset:0x5b; Scheduler Command Register 1*/
    volatile uint32_t RSV_5c_5f[4];  /* Offset:0x5c~5f; Reserved*/
    volatile uint32_t MR0_LPDDR3;    /* Offset:0x60; LPDDR3 Mode Register 0*/
    volatile uint32_t MR1_LPDDR3;    /* Offset:0x61; LPDDR3 Mode Register 1. */
    volatile uint32_t MR2_LPDDR3;    /* Offset:0x62; LPDDR3 Mode Register 2. */
    volatile uint32_t MR3_LPDDR3;    /* Offset:0x63; LPDDR3 Mode Register 3. */
    volatile uint32_t MR4_LPDDR3;    /* Offset:0x64; LPDDR3 Mode Register 4*/
    volatile uint32_t MR5_LPDDR3;    /* Offset:0x65; LPDDR3 Mode Register 5*/
    volatile uint32_t MR6;           /* Offset:0x66; DDR3 Mode Register 6*/
    volatile uint32_t MR7_LPDDR3;    /* Offset:0x67; LPDDR3 Mode Register 7*/
    volatile uint32_t RSV_68_6a[3];  /* Offset:0x68~0x6a; Reserved*/
    volatile uint32_t MR11_LPDDR3;   /* Offset:0x6b; LPDDR3 Mode Register 11*/
    volatile uint32_t RSV_6c_7f[20]; /* Offset:0x6c~0x7f; Reserved*/
    volatile uint32_t DTCR0;         /* Offset:0x80; Data Training Configuration Register 0. Exists: Always*/
    volatile uint32_t DTCR1;         /* Offset:0x81; Data Training Configuration Register 1.*/
    volatile uint32_t DTAR0;         /* Offset:0x82; Data Training Address Register 0*/
    volatile uint32_t DTAR1;         /* Offset:0x83; Data Training Address Register 1*/
    volatile uint32_t DTAR2;         /* Offset:0x84; Data Training Address Register 2*/
    volatile uint32_t RSV_85;        /* Offset:0x85; Reserved*/
    volatile uint32_t DTDR0;         /* Offset:0x86; Data Training Data Register 0*/
    volatile uint32_t DTDR1;         /* Offset:0x87; Data Training Data Register 1*/
    volatile uint32_t UDDR0;         /* Offset:0x88; User Defined Data Register 0*/
    volatile uint32_t UDDR1;         /* Offset:0x89; User Defined Data Register 1*/
    volatile uint32_t RSV_8a_8b[2];  /* Offset:0x8a~0x8b; Reserved*/
    volatile uint32_t DTEDR0;        /* Offset:0x8c; Data Training Eye Data Register 0*/
    volatile uint32_t DTEDR1;        /* Offset:0x8d; Data Training Eye Data Register 1*/
    volatile uint32_t DTEDR2;        /* Offset:0x8e; Data Training Eye Data Register 2*/
    volatile uint32_t VTDR;          /* Offset:0x8f; VREF Training Data Register*/
    volatile uint32_t CATR0;         /* Offset:0x90; Command Address Training Register 0 */
    volatile uint32_t CATR1;         /* Offset:0x91; Command Address Training Register 1 */
    volatile uint32_t RSV_92_93[2];  /* Offset:0x92~0x93; Reserved*/
    volatile uint32_t DQSDR0;        /* Offset:0x94; DQS Drift Register 0*/
    volatile uint32_t DQSDR1;        /* Offset:0x95; DQS Drift Register 1*/
    volatile uint32_t DQSDR2;        /* Offset:0x96; DQS Drift Register 2*/
    volatile uint32_t TMP_97_136[160];   /* Offset:0x97~0x136; refer to SPEC*/
    volatile uint32_t RANKIDR;           /* Offset:0x137; Rank ID Register.*/
    volatile uint32_t TMP_138_147[16];   /* Offset:0x138~0x147; refer to SPEC*/
    volatile uint32_t IOVCR0;            /* Offset:0x148; VREF I/O Control Register 0. */
    volatile uint32_t IOVCR1;            /* Offset:0x149; VREF I/O Control Register 1. */
    volatile uint32_t VTCR0;             /* Offset:0x14a; VREF Training Control Register 0*/
    volatile uint32_t VTCR1;             /* Offset:0x14b; VREF Training Control Register 1. */
    volatile uint32_t RSV_14c_14f[4];    /* Offset:0x14c~0x14f; Reserved*/
    volatile uint32_t ACBDLR0;           /* Offset:0x150; AC Bit Delay Register 0 */
    volatile uint32_t ACBDLR1;           /* Offset:0x151; AC Bit Delay Register 1 */
    volatile uint32_t ACBDLR2;           /* Offset:0x152; AC Bit Delay Register 2 */
    volatile uint32_t ACBDLR3;           /* Offset:0x153; AC Bit Delay Register 3 */
    volatile uint32_t ACBDLR4;           /* Offset:0x154; AC Bit Delay Register 4 */
    volatile uint32_t ACBDLR5;           /* Offset:0x155; AC Bit Delay Register 5 */
    volatile uint32_t ACBDLR6;           /* Offset:0x156; AC Bit Delay Register 6 */
    volatile uint32_t ACBDLR7;           /* Offset:0x157; AC Bit Delay Register 7 */
    volatile uint32_t ACBDLR8;           /* Offset:0x158; AC Bit Delay Register 8 */
    volatile uint32_t ACBDLR9;           /* Offset:0x159; AC Bit Delay Register 9 */
    volatile uint32_t ACBDLR10;          /* Offset:0x15a; AC Bit Delay Register 10 */
    volatile uint32_t ACBDLR11;          /* Offset:0x15b; AC Bit Delay Register 11 */
    volatile uint32_t ACBDLR12;          /* Offset:0x15c; AC Bit Delay Register 12 */
    volatile uint32_t ACBDLR13;          /* Offset:0x15d; AC Bit Delay Register 13 */
    volatile uint32_t ACBDLR14;          /* Offset:0x15e; AC Bit Delay Register 14 */
    volatile uint32_t RSV_15f;           /* Offset:0x15f; Reserved*/
    volatile uint32_t ACLCDLR;           /* Offset:0x160; AC Local Calibrated Delay Line Register*/
    volatile uint32_t RSV_161_167[7];    /* Offset:0x161~0x167; Reserved*/
    volatile uint32_t ACMDLR0;           /* Offset:0x168; AC Master Delay Line Register 0*/
    volatile uint32_t ACMDLR1;           /* Offset:0x169; AC Master Delay Line Register 1*/
    volatile uint32_t RSV_16a_19f[54];   /* Offset:0x16a~0x19f; Reserved*/
    volatile uint32_t ZQCR;              /* Offset:0x1a0; ZQ Impedance Control Register. */
    volatile uint32_t ZQ0PR;             /* Offset:0x1a1; ZQ 0 Impedance Control Program Register. */
    volatile uint32_t ZQ0DR;             /* Offset:0x1a2; ZQ 0 Impedance Control Data Register. */
    volatile uint32_t ZQ0SR;             /* Offset:0x1a3; ZQ 0 Impedance Control Status Register. */
    volatile uint32_t RSV_1a4;           /* Offset:0x1a4; Reserved. */
    volatile uint32_t ZQ1PR;             /* Offset:0x1a5; ZQ 1 Impedance Control Program Register. */
    volatile uint32_t ZQ1DR;             /* Offset:0x1a6; ZQ 1 Impedance Control Data Register. */
    volatile uint32_t ZQ1SR;             /* Offset:0x1a7; ZQ 1 Impedance Control Status Register. */
    volatile uint32_t RSV_1a8;           /* Offset:0x1a8; Reserved. */
    volatile uint32_t ZQ2PR;             /* Offset:0x1a9; ZQ 2 Impedance Control Program Register. */
    volatile uint32_t ZQ2DR;             /* Offset:0x1aa; ZQ 2 Impedance Control Data Register. */
    volatile uint32_t ZQ2SR;             /* Offset:0x1ab; ZQ 2 Impedance Control Status Register. */
    volatile uint32_t RSV_1ac;           /* Offset:0x1ac; Reserved. */
    volatile uint32_t ZQ3PR;             /* Offset:0x1ad; ZQ 3 Impedance Control Program Register. */
    volatile uint32_t ZQ3DR;             /* Offset:0x1ae; ZQ 3 Impedance Control Data Register. */
    volatile uint32_t ZQ3SR;             /* Offset:0x1af; ZQ 3 Impedance Control Status Register. */
    volatile uint32_t RSV_1b0_1bf[16];   /* Offset:0x1b0~0x1bf; Reserved*/
    volatile uint32_t DX0GCR0;           /* Offset:0x1c0; DX 0 General Configuration Register 0. */
    volatile uint32_t DX0GCR1;           /* Offset:0x1c1; DX 0 General Configuration Register 1. */
    volatile uint32_t DX0GCR2;           /* Offset:0x1c2; DX 0 General Configuration Register 2. */
    volatile uint32_t DX0GCR3;           /* Offset:0x1c3; DX 0 General Configuration Register 3. */
    volatile uint32_t DX0GCR4;           /* Offset:0x1c4; DX 0 General Configuration Register 4. */
    volatile uint32_t DX0GCR5;           /* Offset:0x1c5; DX 0 General Configuration Register 5. */
    volatile uint32_t DX0GCR6;           /* Offset:0x1c6; DX 0 General Configuration Register 6. */
    volatile uint32_t DX0GCR7;           /* Offset:0x1c7; DX 0 General Configuration Register 7. */
    volatile uint32_t DX0GCR8;           /* Offset:0x1c8; DX 0 General Configuration Register 8. */
    volatile uint32_t DX0GCR9;           /* Offset:0x1c9; DX 0 General Configuration Register 9. */
    volatile uint32_t RSV_1ca_1cf[6];    /* Offset:0x1ca~0x1cf; Reserved*/
    volatile uint32_t DX0BDLR0;          /* Offset:0x1d0; DX 0 Bit Delay Line Register 0. */
    volatile uint32_t DX0BDLR1;          /* Offset:0x1d1; DX 0 Bit Delay Line Register 1. */
    volatile uint32_t DX0BDLR2;          /* Offset:0x1d2; DX 0 Bit Delay Line Register 2. */
    volatile uint32_t RSV_1d3;           /* Offset:0x1d3; Reserved. */
    volatile uint32_t DX0BDLR3;          /* Offset:0x1d4; DX 0 Bit Delay Line Register 3. */
    volatile uint32_t DX0BDLR4;          /* Offset:0x1d5; DX 0 Bit Delay Line Register 4. */
    volatile uint32_t DX0BDLR5;          /* Offset:0x1d6; DX 0 Bit Delay Line Register 5. */
    volatile uint32_t RSV_1d7;           /* Offset:0x1d7; Reserved. */
    volatile uint32_t DX0BDLR6;          /* Offset:0x1d8; DX 0 Bit Delay Line Register 6. */
    volatile uint32_t DX0BDLR7;          /* Offset:0x1d9; DX 0 Bit Delay Line Register 7. */
    volatile uint32_t DX0BDLR8;          /* Offset:0x1da; DX 0 Bit Delay Line Register 8. */
    volatile uint32_t DX0BDLR9;          /* Offset:0x1db; DX 0 Bit Delay Line Register 9. */
    volatile uint32_t RSV_1dc_1df[4];    /* Offset:0x1dc~0x1df; Reserved. */
    volatile uint32_t DX0LCDLR0;         /* Offset:0x1e0; DX 0 Local Calibrated Delay Line Register 0. */
    volatile uint32_t DX0LCDLR1;         /* Offset:0x1e1; DX 0 Local Calibrated Delay Line Register 1. */
    volatile uint32_t DX0LCDLR2;         /* Offset:0x1e2; DX 0 Local Calibrated Delay Line Register 2. */
    volatile uint32_t DX0LCDLR3;         /* Offset:0x1e3; DX 0 Local Calibrated Delay Line Register 3. */
    volatile uint32_t DX0LCDLR4;         /* Offset:0x1e4; DX 0 Local Calibrated Delay Line Register 4. */
    volatile uint32_t DX0LCDLR5;         /* Offset:0x1e5; DX 0 Local Calibrated Delay Line Register 5. */
    volatile uint32_t RSV_1e6_1e7[2];    /* Offset:0x1e6~0x1e7; Reserved. */
    volatile uint32_t DX0MDLR0;          /* Offset:0x1e8; DX 0 Master Delay Line Register 0. */
    volatile uint32_t DX0MDLR1;          /* Offset:0x1e9; DX 0 Master Delay Line Register 1. */
    volatile uint32_t RSV_1ea_1ef[6];    /* Offset:0x1ea~0x1ef; Reserved. */
    volatile uint32_t DX0GTR0;           /* Offset:0x1f0; DX 0 General Timing Register 0. */
    volatile uint32_t RSV_1f1_1f3[3];    /* Offset:0x1f1~0x1f3; Reserved. */
    volatile uint32_t DX0RSR0;           /* Offset:0x1f4; DX 0 Rank Status Register 0. */
    volatile uint32_t DX0RSR1;           /* Offset:0x1f5; DX 0 Rank Status Register 1. */
    volatile uint32_t DX0RSR2;           /* Offset:0x1f6; DX 0 Rank Status Register 2. */
    volatile uint32_t DX0RSR3;           /* Offset:0x1f7; DX 0 Rank Status Register 3. */
    volatile uint32_t DX0GSR0;           /* Offset:0x1f8; DX 0 General Status Register 0. */
    volatile uint32_t DX0GSR1;           /* Offset:0x1f9; DX 0 General Status Register 1. */
    volatile uint32_t DX0GSR2;           /* Offset:0x1fa; DX 0 General Status Register 2. */
    volatile uint32_t DX0GSR3;           /* Offset:0x1fb; DX 0 General Status Register 3. */
    volatile uint32_t DX0GSR4;           /* Offset:0x1fc; DX 0 General Status Register 4. */
    volatile uint32_t DX0GSR5;           /* Offset:0x1fd; DX 0 General Status Register 5. */
    volatile uint32_t DX0GSR6;           /* Offset:0x1fe; DX 0 General Status Register 6. */
    volatile uint32_t RSV_1ff;           /* Offset:0x1ff; Reserved. */
    volatile uint32_t DX1GCR0;           /* Offset:0x200; DX 1 General Configuration Register 0 */
    volatile uint32_t DX1GCR1;           /* Offset:0x201; DX 1 General Configuration Register 1 */
    volatile uint32_t DX1GCR2;           /* Offset:0x202; DX 1 General Configuration Register 2 */
    volatile uint32_t DX1GCR3;           /* Offset:0x203; DX 1 General Configuration Register 3 */
    volatile uint32_t DX1GCR4;           /* Offset:0x204; DX 1 General Configuration Register 4 */
    volatile uint32_t DX1GCR5;           /* Offset:0x205; DX 1 General Configuration Register 5 */
    volatile uint32_t DX1GCR6;           /* Offset:0x206; DX 1 General Configuration Register 6 */
    volatile uint32_t DX1GCR7;           /* Offset:0x207; DX 1 General Configuration Register 7 */
    volatile uint32_t DX1GCR8;           /* Offset:0x208; DX 1 General Configuration Register 8 */
    volatile uint32_t DX1GCR9;           /* Offset:0x209; DX 1 General Configuration Register 9 */
    volatile uint32_t RSV_20a_20f[6];    /* Offset:0x20a~0x20f; Reserved. */
    volatile uint32_t DX1BDLR0;          /* Offset:0x210; DX 1 Bit Delay Line Register 0 */
    volatile uint32_t DX1BDLR1;          /* Offset:0x211; DX 1 Bit Delay Line Register 1 */
    volatile uint32_t DX1BDLR2;          /* Offset:0x212; DX 1 Bit Delay Line Register 2 */
    volatile uint32_t RSV_213;           /* Offset:0x213; Reserved. */
    volatile uint32_t DX1BDLR3;          /* Offset:0x214; DX 1 Bit Delay Line Register 3 */
    volatile uint32_t DX1BDLR4;          /* Offset:0x215; DX 1 Bit Delay Line Register 4 */
    volatile uint32_t DX1BDLR5;          /* Offset:0x216; DX 1 Bit Delay Line Register 5 */
    volatile uint32_t RSV_217;           /* Offset:0x217; Reserved. */
    volatile uint32_t DX1BDLR6;          /* Offset:0x218; DX 1 Bit Delay Line Register 6 */
    volatile uint32_t DX1BDLR7;          /* Offset:0x219; DX 1 Bit Delay Line Register 7 */
    volatile uint32_t DX1BDLR8;          /* Offset:0x21a; DX 1 Bit Delay Line Register 8 */
    volatile uint32_t DX1BDLR9;          /* Offset:0x21b; DX 1 Bit Delay Line Register 9 */
    volatile uint32_t RSV_21c_21f[4];    /* Offset:0x21c~0x21f; Reserved. */
    volatile uint32_t DX1LCDLR0;         /* Offset:0x220; DX 1 Local Calibrated Delay Line Register 0 */
    volatile uint32_t DX1LCDLR1;         /* Offset:0x221; DX 1 Local Calibrated Delay Line Register 1 */
    volatile uint32_t DX1LCDLR2;         /* Offset:0x222; DX 1 Local Calibrated Delay Line Register 2 */
    volatile uint32_t DX1LCDLR3;         /* Offset:0x223; DX 1 Local Calibrated Delay Line Register 3 */
    volatile uint32_t DX1LCDLR4;         /* Offset:0x224; DX 1 Local Calibrated Delay Line Register 4 */
    volatile uint32_t DX1LCDLR5;         /* Offset:0x225; DX 1 Local Calibrated Delay Line Register 5 */
    volatile uint32_t RSV_226_227[2];    /* Offset:0x226~0x227; Reserved. */
    volatile uint32_t DX1MDLR0;          /* Offset:0x228; DX 1 Master Delay Line Register 0 */
    volatile uint32_t DX1MDLR1;          /* Offset:0x229; DX 1 Master Delay Line Register 1 */
    volatile uint32_t RSV_22a_22f[6];    /* Offset:0x22a~0x22f; Reserved. */
    volatile uint32_t DX1GTR0;           /* Offset:0x230; DX 1 General Timing Register 0 */
    volatile uint32_t RSV_231_233[3];    /* Offset:0x231~0x233; Reserved. */
    volatile uint32_t DX1RSR0;           /* Offset:0x234; DX 1 Rank Status Register 0 */
    volatile uint32_t DX1RSR1;           /* Offset:0x235; DX 1 Rank Status Register 1 */
    volatile uint32_t DX1RSR2;           /* Offset:0x236; DX 1 Rank Status Register 2 */
    volatile uint32_t DX1RSR3;           /* Offset:0x237; DX 1 Rank Status Register 3 */
    volatile uint32_t DX1GSR0;           /* Offset:0x238; DX 1 General Status Register 0 */
    volatile uint32_t DX1GSR1;           /* Offset:0x239; DX 1 General Status Register 1 */
    volatile uint32_t DX1GSR2;           /* Offset:0x23a; DX 1 General Status Register 2 */
    volatile uint32_t DX1GSR3;           /* Offset:0x23b; DX 1 General Status Register 3 */
    volatile uint32_t DX1GSR4;           /* Offset:0x23c; DX 1 General Status Register 4 */
    volatile uint32_t DX1GSR5;           /* Offset:0x23d; DX 1 General Status Register 5 */
    volatile uint32_t DX1GSR6;           /* Offset:0x23e; DX 1 General Status Register 6 */
    volatile uint32_t RSV_23f;           /* Offset:0x23f; Reserved. */
    volatile uint32_t DX2GCR0;           /* Offset:0x240; DX 2 General Configuration Register 0 */
    volatile uint32_t DX2GCR1;           /* Offset:0x241; DX 2 General Configuration Register 1 */
    volatile uint32_t DX2GCR2;           /* Offset:0x242; DX 2 General Configuration Register 2 */
    volatile uint32_t DX2GCR3;           /* Offset:0x243; DX 2 General Configuration Register 3 */
    volatile uint32_t DX2GCR4;           /* Offset:0x244; DX 2 General Configuration Register 4 */
    volatile uint32_t DX2GCR5;           /* Offset:0x245; DX 2 General Configuration Register 5 */
    volatile uint32_t DX2GCR6;           /* Offset:0x246; DX 2 General Configuration Register 6 */
    volatile uint32_t DX2GCR7;           /* Offset:0x247; DX 2 General Configuration Register 7 */
    volatile uint32_t DX2GCR8;           /* Offset:0x248; DX 2 General Configuration Register 8 */
    volatile uint32_t DX2GCR9;           /* Offset:0x249; DX 2 General Configuration Register 9 */
    volatile uint32_t RSV_24a_24f[6];    /* Offset:0x24a~0x24f; Reserved. */
    volatile uint32_t DX2BDLR0;          /* Offset:0x250; DX 2 Bit Delay Line Register 0 */
    volatile uint32_t DX2BDLR1;          /* Offset:0x251; DX 2 Bit Delay Line Register 1 */
    volatile uint32_t DX2BDLR2;          /* Offset:0x252; DX 2 Bit Delay Line Register 2 */
    volatile uint32_t RSV_253;           /* Offset:0x253; Reserved. */
    volatile uint32_t DX2BDLR3;          /* Offset:0x254; DX 2 Bit Delay Line Register 3 */
    volatile uint32_t DX2BDLR4;          /* Offset:0x255; DX 2 Bit Delay Line Register 4 */
    volatile uint32_t DX2BDLR5;          /* Offset:0x256; DX 2 Bit Delay Line Register 5 */
    volatile uint32_t RSV_257;           /* Offset:0x257; Reserved. */
    volatile uint32_t DX2BDLR6;          /* Offset:0x258; DX 2 Bit Delay Line Register 6 */
    volatile uint32_t DX2BDLR7;          /* Offset:0x259; DX 2 Bit Delay Line Register 7 */
    volatile uint32_t DX2BDLR8;          /* Offset:0x25a; DX 2 Bit Delay Line Register 8 */
    volatile uint32_t DX2BDLR9;          /* Offset:0x25b; DX 2 Bit Delay Line Register 9 */
    volatile uint32_t RSV_25c_25f[4];    /* Offset:0x25c~0x25f; Reserved. */
    volatile uint32_t DX2LCDLR0;         /* Offset:0x260; DX 2 Local Calibrated Delay Line Register 0. */
    volatile uint32_t DX2LCDLR1;         /* Offset:0x261; DX 2 Local Calibrated Delay Line Register 1. */
    volatile uint32_t DX2LCDLR2;         /* Offset:0x262; DX 2 Local Calibrated Delay Line Register 2. */
    volatile uint32_t DX2LCDLR3;         /* Offset:0x263; DX 2 Local Calibrated Delay Line Register 3. */
    volatile uint32_t DX2LCDLR4;         /* Offset:0x264; DX 2 Local Calibrated Delay Line Register 4. */
    volatile uint32_t DX2LCDLR5;         /* Offset:0x265; DX 2 Local Calibrated Delay Line Register 5. */
    volatile uint32_t RSV_266_267[2];    /* Offset:0x266~0x267; Reserved. */
    volatile uint32_t DX2MDLR0;          /* Offset:0x268; DX 2 Master Delay Line Register 0. */
    volatile uint32_t DX2MDLR1;          /* Offset:0x269; DX 2 Master Delay Line Register 1. */
    volatile uint32_t RSV_26a_26f[6];    /* Offset:0x26a~0x26f; Reserved. */
    volatile uint32_t DX2GTR0;           /* Offset:0x270; DX 2 General Timing Register 0. */
    volatile uint32_t RSV_271_273[3];    /* Offset:0x271~0x273; Reserved. */
    volatile uint32_t DX2RSR0;           /* Offset:0x274; DX 2 Rank Status Register 0. */
    volatile uint32_t DX2RSR1;           /* Offset:0x275; DX 2 Rank Status Register 1. */
    volatile uint32_t DX2RSR2;           /* Offset:0x276; DX 2 Rank Status Register 2. */
    volatile uint32_t DX2RSR3;           /* Offset:0x277; DX 2 Rank Status Register 3. */
    volatile uint32_t DX2GSR0;           /* Offset:0x278; DX 2 General Status Register 0. */
    volatile uint32_t DX2GSR1;           /* Offset:0x279; DX 2 General Status Register 1. */
    volatile uint32_t DX2GSR2;           /* Offset:0x27a; DX 2 General Status Register 2. */
    volatile uint32_t DX2GSR3;           /* Offset:0x27b; DX 2 General Status Register 3. */
    volatile uint32_t DX2GSR4;           /* Offset:0x27c; DX 2 General Status Register 4. */
    volatile uint32_t DX2GSR5;           /* Offset:0x27d; DX 2 General Status Register 5. */
    volatile uint32_t DX2GSR6;           /* Offset:0x27e; DX 2 General Status Register 6. */
    volatile uint32_t RSV_27f;           /* Offset:0x27f; Reserved. */
    volatile uint32_t DX3GCR0;           /* Offset:0x280; DX 3 General Configuration Register 0. */
    volatile uint32_t DX3GCR1;           /* Offset:0x281; DX 3 General Configuration Register 1. */
    volatile uint32_t DX3GCR2;           /* Offset:0x282; DX 3 General Configuration Register 2. */
    volatile uint32_t DX3GCR3;           /* Offset:0x283; DX 3 General Configuration Register 3. */
    volatile uint32_t DX3GCR4;           /* Offset:0x284; DX 3 General Configuration Register 4.*/
    volatile uint32_t DX3GCR5;           /* Offset:0x285; DX 3 General Configuration Register 5. */
    volatile uint32_t DX3GCR6;           /* Offset:0x286; DX 3 General Configuration Register 6. */
    volatile uint32_t DX3GCR7;           /* Offset:0x287; DX 3 General Configuration Register 7. */
    volatile uint32_t DX3GCR8;           /* Offset:0x288; DX 3 General Configuration Register 8. */
    volatile uint32_t DX3GCR9;           /* Offset:0x289; DX 3 General Configuration Register 9. */
    volatile uint32_t RSV_28a_28f[6];    /* Offset:0x28a~0x28f; Reserved. */
    volatile uint32_t DX3BDLR0;          /* Offset:0x290; DX 3 Bit Delay Line Register 0. */
    volatile uint32_t DX3BDLR1;          /* Offset:0x291; DX 3 Bit Delay Line Register 1. */
    volatile uint32_t DX3BDLR2;          /* Offset:0x292; DX 3 Bit Delay Line Register 2. */
    volatile uint32_t RSV_293;           /* Offset:0x293; Reserved. */
    volatile uint32_t DX3BDLR3;          /* Offset:0x294; DX 3 Bit Delay Line Register 3. */
    volatile uint32_t DX3BDLR4;          /* Offset:0x295; DX 3 Bit Delay Line Register 4. */
    volatile uint32_t DX3BDLR5;          /* Offset:0x296; DX 3 Bit Delay Line Register 5. */
    volatile uint32_t RSV_297;           /* Offset:0x297; Reserved. */
    volatile uint32_t DX3BDLR6;          /* Offset:0x298; DX 3 Bit Delay Line Register 6. */
    volatile uint32_t DX3BDLR7;          /* Offset:0x299; DX 3 Bit Delay Line Register 7. */
    volatile uint32_t DX3BDLR8;          /* Offset:0x29a; DX 3 Bit Delay Line Register 8. */
    volatile uint32_t DX3BDLR9;          /* Offset:0x29b; DX 3 Bit Delay Line Register 9. */
    volatile uint32_t RSV_29c_29f[4];    /* Offset:0x29c~0x29f Reserved. */
    volatile uint32_t DX3LCDLR0;         /* Offset:0x2a0; DX 3 Local Calibrated Delay Line Register 0. */
    volatile uint32_t DX3LCDLR1;         /* Offset:0x2a1; DX 3 Local Calibrated Delay Line Register 1. */
    volatile uint32_t DX3LCDLR2;         /* Offset:0x2a2; DX 3 Local Calibrated Delay Line Register 2. */
    volatile uint32_t DX3LCDLR3;         /* Offset:0x2a3; DX 3 Local Calibrated Delay Line Register 3. */
    volatile uint32_t DX3LCDLR4;         /* Offset:0x2a4; DX 3 Local Calibrated Delay Line Register 4. */
    volatile uint32_t DX3LCDLR5;         /* Offset:0x2a5; DX 3 Local Calibrated Delay Line Register 5. */
    volatile uint32_t RSV_2a6_2a7[2];    /* Offset:0x2a6~0x2a7 Reserved. */
    volatile uint32_t DX3MDLR0;          /* Offset:0x2a8; DX 3 Master Delay Line Register 0. */
    volatile uint32_t DX3MDLR1;          /* Offset:0x2a9; DX 3 Master Delay Line Register 1. */
    volatile uint32_t RSV_2aa_2af[6];    /* Offset:0x2aa~0x2af Reserved. */
    volatile uint32_t DX3GTR0;           /* Offset:0x2b0; DX 3 General Timing Register 0. */
    volatile uint32_t RSV_2b1_2b3[3];    /* Offset:0x2b1~0x2b3 Reserved. */
    volatile uint32_t DX3RSR0;           /* Offset:0x2b4; DX 3 Rank Status Register 0. */
    volatile uint32_t DX3RSR1;           /* Offset:0x2b5; DX 3 Rank Status Register 1. */
    volatile uint32_t DX3RSR2;           /* Offset:0x2b6; DX 3 Rank Status Register 2. */
    volatile uint32_t DX3RSR3;           /* Offset:0x2b7; DX 3 Rank Status Register 3. */
    volatile uint32_t DX3GSR0;           /* Offset:0x2b8; DX 3 General Status Register 0. */
    volatile uint32_t DX3GSR1;           /* Offset:0x2b9; DX 3 General Status Register 1. */
    volatile uint32_t DX3GSR2;           /* Offset:0x2ba; DX 3 General Status Register 2. */
    volatile uint32_t DX3GSR3;           /* Offset:0x2bb; DX 3 General Status Register 3. */
    volatile uint32_t DX3GSR4;           /* Offset:0x2bc; DX 3 General Status Register 4. */
    volatile uint32_t DX3GSR5;           /* Offset:0x2bd; DX 3 General Status Register 5. */
    volatile uint32_t DX3GSR6;           /* Offset:0x2be; DX 3 General Status Register 6. */
    volatile uint32_t TMP_2bf_3fe[230];  /* Offset:0x2bf~0x3fe; refer to SPEC*/
} kdrv_ddrphy_pub_t;


#define KDRV_DDR_CTRL_OFFSET    (0x0000)
#define KDRV_DDR_CTRL_MP_OFFSET (0x03fc)
#define KDRV_DDR_PUB_OFFSET     (0x2000)
#define KDRV_DDR_CTRL_REG       ((kdrv_ddrc_ctrl_reg_t*)(DDR_REG_BASE + KDRV_DDR_CTRL_OFFSET))
#define KDRV_DDR_CTRL_MP_REG    ((kdrv_ddrc_mp_reg_t*)(DDR_REG_BASE + KDRV_DDR_CTRL_MP_OFFSET))
#define KDRV_DDR_PHY_REG        ((kdrv_ddrphy_pub_t*)(DDR_REG_BASE + KDRV_DDR_PUB_OFFSET))



#endif //_REGBASE_H_
