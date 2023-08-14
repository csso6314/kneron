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

/**@addtogroup  KDRV_SDC  KDRV_SDC
 * @{
 * @brief       Kneron sdc sd/emmc driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */


#ifndef __KDRV_SDC_H__
#define __KDRV_SDC_H__

#include "regbase.h"
#include "kdrv_sdc_mmc.h"
#include "kdrv_status.h"


/* Replaced with sdc_adma2_desc_table, defined in kdrv_sdc.c */
#define ADMA2_NUM_OF_LINES  64                      // 1536

#define CARD_TYPE_UNKNOWN       0
#define MEMORY_CARD_TYPE_SD     1
#define MEMORY_CARD_TYPE_MMC    2
#define SDIO_TYPE_CARD          3
#define MEMORY_SDIO_COMBO       4

/* For SD Memory Register (unit of byte) */
#define SCR_LENGTH              8
#define SD_STATUS_LENGTH        64

/* For MMC Memory Register */
#define EXT_CSD_LENGTH          512


typedef struct {
    uint32_t sdma_addr;         // 0x00-0x03, SDMA system address or auto CMD23 argument 2
    uint16_t blk_size;          // 0x04-0x05, block size, [14:12]:sdma_buf_bound, [11:0]:blk_size
    uint16_t blk_cnt;           // 0x06-0x07, block count
    uint32_t cmd_argu;          // 0x08-0x0B, argument 1
    uint16_t txmode;            // 0x0C-0x0D, transfre mode, 5:multi_blk_rw, 4:tran_dir_sel,
                                //            [3:2]auto_cmd_en, 1:blk_cnt_en, 0:dma_en
    uint16_t cmd_reg;           // 0x0E-0x0F, command, [13:8]:cmd_idx, [7:6]:cmd_type,
                                //            5:data_pres_sel, 4:cmd_idx_chk_en, 3:cmd_crc, [1:0]rsp_type
    uint64_t cmd_resplo;        // 0x10-0x17, response register low[63:1]
    uint64_t cmd_resphi;        // 0x18-0x1F, response register high[63:1]
    uint32_t buf_data;          // 0x20-0x23, buffer data port
    uint32_t present_state;     // 0x24-0x27, present state
    uint8_t hcreg;              // 0x28, host control 1
    uint8_t pwr_ctl;            // 0x29, power control, [3:1]:sd_bus_vol, [0]:sd_bus_pow
    uint8_t blk_gap_ctl;        // 0x2A, block gap control, [3]:int_at_blk_gap, [2]:read_wait,
                                //       [1]:cont_req, [0]:sp_blk_gap_req
    uint8_t wakeup_ctl;         // 0x2B
    uint16_t clk_ctl;           // 0x2C-0x2D, clock control, [15:8]:low_bit_sd_clk_sel,
                                //            [7:6]:upper_bit_sd_clk_sel, [5]:clk_gen_sel, [2]:sd_clk_en
    uint8_t timeout_ctl;        // 0x2E, timeout control
    uint8_t softrst;            // 0x2F, software reset, [2]:soft_rst_dat, [1]:soft_rst_cmd, [0]:soft_rst_all
    uint16_t intr_sts;          // 0x30-0x31, normal interrupt status, [15]:err_interrupt,
                                //            [12]:re_tuning_int,[11]:int_c_c ....
    uint16_t err_sts;           // 0x32-0x33, error interrupt status
    uint16_t intr_en;           // 0x34-0x35, normal interrupt status enable
    uint16_t err_en;            // 0x36-0x37, error interrupt status enable
    uint16_t intr_sig_en;       // 0x38-0x39, normal interrupt signal enable
    uint16_t err_sig_en;        // 0x3A-0x3B, error interrupt signal enable
    uint16_t auto_cmd_err;      // 0x3C-0x3D, auto CMD12 error status
    uint16_t host_ctl2;         // 0x3E-0x3F, host control 2
    uint32_t cap_reg;           // 0x40-0x43, capabilities 0/1
    uint32_t cap_reg2;          // 0x44-0x47
    uint64_t max_curr;          // 0x48-0x4F, maximum current capabilities
    uint16_t cmd12_force_evt;   // 0x50-0x51, force event register for auto CMD error status
    uint16_t force_evt;         // 0x52-0x53, force event register for error interrupt status
    uint32_t adma_err_sts;      // 0x54-0x57, ADMA error status
    uint64_t adma_addr;         // 0x58-0x5F, ADMA system address
    /*
     *  Register offset 0x60 - 0x6F
     */
    uint16_t preset_val_init;   // 0x60-0x61, preset value register, 0x60~0x6F
    uint16_t preset_val_ds;     // 0x62-0x63
    uint16_t preset_val_hs;     // 0x64-0x65
    uint16_t preset_val_sdr12;  // 0x66-0x67
    uint16_t preset_val_sdr25;  // 0x68-0x69
    uint16_t preset_val_sdr50;  // 0x6A-0x6B
    uint16_t preset_val_sdr104; // 0x6C-0x6D
    uint16_t preset_val_ddr50;  // 0x6E-0x6F

    uint32_t reserved[28];      // 0x70-0xDF
    uint32_t share_bus_ctl;     // 0xE0-0xE3
    uint32_t reserved2[6];      // 0xE4-0xFB
    uint16_t slt_intr_sts;      // 0xFC-0xFD
    uint16_t hcver;             // 0xFE-0xFF, host controller version
    uint32_t vendor_reg0;       // 0x100-0x103, vendor defined register0
    uint32_t vendor_reg1;       // 0x104-0x107, vendor defined register1
    uint32_t vendor_reg2;       // 0x108-0x10B, vendor defined register2
    uint32_t vendor_reg3;       // 0x10C-0x10F, vendor defined register3
    uint32_t vendor_reg4;       // 0x110-0x113, vendor defined register4
    uint32_t vendor_reg5;       // 0x114-0x117, vendor defined register5
    uint32_t vendor_reg6;       // 0x118-0x11B, vendor defined register6
    uint32_t ahb_err_sts;       // 0x11C-0x11F, vendor defined register7
    uint32_t ahb_err_en;        // 0x120-0x124, vendor defined register8
    uint32_t ahb_err_sig_en;    // 0x124-0x127, vendor defined register9
    uint32_t dma_hndshk;        // 0x128-0x12C, DMA handshake enable
    uint32_t reserved4[19];     // 0x12C-0x177
    uint32_t hw_attr;           // 0x178-0x17B, hardware attributes
    uint32_t ip_ver;            // 0x17C-0x17F, ip revision
    uint32_t ciph_m_ctl;        // 0x180-0x183, cipher mode control
    uint32_t ciph_m_sts;        // 0x184-0x187, cipher mode status
    uint16_t ciph_m_sts_en;     // 0x188-0x189, cipher mode status enable
    uint16_t ciph_m_sig_en;     // 0x18A-0x18B, cipher mode signal enable
    uint32_t in_data_lo;        // 0x18C-0x18F, low word of input data
    uint32_t in_data_hi;        // 0x190-0x193, high word of input data
    uint32_t in_key_lo;         // 0x194-0x197, low word of input key
    uint32_t in_key_hi;         // 0x198-0x19B, high word of input key
    uint32_t out_data_lo;       // 0x19C-0x19F, low word of output data
    uint32_t out_data_hi;       // 0x1A0-0x1A3, high word of output data
    uint32_t secr_table_port;   // 0x1A4-0x1A7, secret constant table data
} kdrv_sdc_reg_t;


typedef enum
{
    KDRV_SDC0_DEV = 0,           /*< Kdrv SDC controller 0 */
    KDRV_SDC1_DEV,               /*< Kdrv SDC controller 1 */
} kdrv_sdc_dev_e;

typedef enum {
    INFINITE_NO = 0,
    INFINITE_MODE_1,    /* Block Count Reg = 2 * Desired Blocks */
    INFINITE_MODE_2     /* Block Count Reg = 0 */
} kdrv_sdc_infinite_test_e;

typedef enum {
    WRITE = 0,
    READ
} kdrv_sdc_transfer_act_e;

typedef enum {
    ADMA = 0,
    SDMA,
    PIO,
    EDMA,
    TRANS_UNKNOWN
} kdrv_sdc_transfer_type_e;

/* For Asyn./ Syn abort*/
typedef enum {
    ABORT_ASYNCHRONOUS = 0,
    ABORT_SYNCHRONOUS,
    ABORT_UNDEFINED
} kdrv_sdc_abort_type_e;

typedef enum {
    CPRM_PROTECT_RW,
    CPRM_FILESYS,
    CPRM_UNKNOWN
} kdrv_sdc_cprm_test_e;

typedef struct {
    kdrv_sdc_transfer_type_e use_dma;   /* 0: PIO 1: SDMA 2: ADMA */
    uint16_t line_bound;
    uint16_t adma2rand;
    kdrv_sdc_abort_type_e sync_abort;     /* 0: Async 1: Sync */
    uint8_t erasing;        /* 0: No earsing in buruin 1:Include erase testing */
    uint8_t auto_cmd;
    uint8_t reserved;
} kdrv_sdc_flow_info_t;

typedef struct {
    uint32_t reserved1:5;       /* 508:502 */
    uint32_t secured_mode:1;    /* 509 */
    uint32_t dat_bus_width:2;   /* 511:510 */
    uint32_t sd_card_type_hi:8; /* 495:488 */
    uint32_t reserved2:8;       /* 501:496 */
    uint32_t sd_card_type_lo:8; /* 487:480 */
    uint32_t size_of_protected_area; /* 479:448 */
    uint8_t speed_class;
    uint8_t performance_move;
    uint32_t reserved3:4;       /* 427:424 */
    uint32_t au_size:4;         /* 431:428 */
    uint8_t erase_size[2];      /* 423:408 */
    uint32_t erase_offset:2;    /* 401:400 */
    uint32_t erase_timeout:6;   /* 407:402 */
    uint8_t reserved4[11];
    uint8_t reserved5[39];
} kdrv_sd_status_t;

typedef struct {
    uint32_t csd_structure:2;
    uint32_t reserved1:6;
    uint8_t taac;
    uint8_t nsac;
    uint8_t tran_speed;
    uint32_t ccc:12;
    uint32_t read_bl_len:4;
    uint32_t read_bl_partial:1;
    uint32_t write_blk_misalign:1;
    uint32_t read_blk_misalign:1;
    uint32_t dsr_imp:1;
    uint32_t reserved2:2;
    uint32_t c_size:12;
    uint32_t vdd_r_curr_min:3;
    uint32_t vdd_r_curr_max:3;
    uint32_t vdd_w_curr_min:3;
    uint32_t vdd_w_curr_max:3;
    uint32_t c_size_mult:3;
    uint32_t erase_blk_en:1;
    uint32_t sector_size:7;
    uint32_t wp_grp_size:7;
    uint32_t wp_grp_enable:1;
    uint32_t reserved3:2;
    uint32_t r2w_factor:3;
    uint32_t write_bl_len:4;
    uint32_t write_bl_partial:1;
    uint32_t reserved4:5;
    uint32_t file_format_grp:1;
    uint32_t copy:1;
    uint32_t perm_write_protect:1;
    uint32_t tmp_write_protect:1;
    uint32_t file_format:2;
    uint32_t Reserver5:2;
} kdrv_sdc_csd_v1_t;

typedef struct {
    uint32_t csd_structure:2;
    uint32_t reserved1:6;
    uint8_t taac;
    uint8_t nsac;
    uint8_t tran_speed;
    uint32_t ccc:12;
    uint32_t read_bl_len:4;
    uint32_t read_bl_partial:1;
    uint32_t write_blk_misalign:1;
    uint32_t read_blk_misalign:1;
    uint32_t dsr_imp:1;
    uint32_t reserved2:6;
    uint32_t c_size:22;
    uint32_t reserved3:1;
    uint32_t erase_blk_en:1;
    uint32_t sector_size:7;
    uint32_t wp_grp_size:7;
    uint32_t wp_grp_enable:1;
    uint32_t reserved4:2;
    uint32_t r2w_factor:3;
    uint32_t write_bl_len:4;
    uint32_t write_bl_partial:1;
    uint32_t reserved5:5;
    uint32_t file_format_grp:1;
    uint32_t copy:1;
    uint32_t perm_write_protect:1;
    uint32_t tmp_write_protect:1;
    uint32_t file_format:2;
    uint32_t reserver6:2;
} kdrv_sdc_csd_v2_t;

// The sequence of variable in kdrv_sdc_sd_scr_t structure is constrained.
typedef struct {
    uint32_t sd_spec:4;         /* [59:56] */
    uint32_t scr_structure:4;   /* [60:63] */

    uint32_t sd_bus_widths:4;   /* [51:48] */
    uint32_t sd_security:3;     /* [52:54] */
    uint32_t data_stat_after_erase:1;   /* [55:55] */

    uint32_t reserved1:7;       /* [46:40] */
    uint32_t sd_spec3:1;        /* [47:47] */

    uint32_t cmd20_support:1;   /* [32:32] */
    uint32_t cmd23_support:1;   /* [33:33] */
    uint32_t reserverd2:6;      /* [34:39] */
    uint32_t reserverd3;        /* [31:0] */
} kdrv_sdc_sd_scr_t;

/* for SCR */
#define SDHCI_SCR_SUPPORT_4BIT_BUS  0x4
#define SDHCI_SCR_SUPPORT_1BIT_BUS  0x1

typedef enum {
    SPEED_DEFAULT = 0,  /* or SDR12 in 1.8v IO signalling level */
    SPEED_SDR25,        /* or SDR25 in 1.8v IO signalling level */
    SPEED_SDR50,
    SPEED_SDR104,
    SPEED_DDR50,
    SPEED_RSRV
} kdrv_sdc_bus_speed_e;

/* To indicate which complete we want to wait */
#define WAIT_CMD_COMPLETE       BIT(0)
#define WAIT_TRANS_COMPLETE     BIT(1)
#define WAIT_DMA_INTR           BIT(2)
#define WAIT_BLOCK_GAP          BIT(3)

typedef struct {
    uint32_t card_insert;
    kdrv_sdc_flow_info_t flow_set;
    uint16_t rca;
    uint16_t dsr;
    kdrv_sdc_sd_scr_t scr;
    uint32_t ocr;
    uint64_t csd_lo;
    uint64_t csd_hi;
    kdrv_sdc_csd_v1_t csd_ver1;
    kdrv_sdc_csd_v2_t csd_ver2;
    uint64_t cid_lo;
    uint64_t cid_hi;
    uint64_t resp_lo;
    uint64_t resp_hi;
    uint32_t num_of_blks;
    uint8_t switch_sts[64];
    volatile uint16_t err_sts;
    volatile uint16_t auto_err;
    volatile uint8_t cmpl_mask;
    kdrv_sd_status_t sd_sts;
    uint16_t bs_mode;        /* Bus Speed Mode */
    uint8_t bus_width;
    uint8_t already_init;
    uint8_t blk_addr;
    uint32_t max_dtr;
    kdrv_sdc_bus_speed_e speed;
    uint32_t fifo_depth;

    /* MMC */
    kdrv_sdc_mmc_csd_t csd_mmc;
    kdrv_sdc_mmc_ext_csd_t ext_csd_mmc;
    uint32_t num_of_boot_blks;

    /* SDMA */
    uint8_t u8_sdma_lock;
    uint8_t sdma_intr;

    uint32_t card_type;

    /* SDIO */
    uint8_t num_io_func;
    uint8_t mem_present;

    /* Drive Name */
    uint32_t drive;

    /* system configurations */
    uint32_t sys_freq;

    uint32_t protected_drive;
    uint32_t cprm_init;
    uint32_t kmu_lo;
    uint32_t kmu_hi;
    uint32_t auto_cbc;      //only used in read/write protected area
} kdrv_sdc_sdcard_info_t;

typedef struct {
    uint32_t max_clk;
    uint32_t min_clk;
    uint32_t clock;
    uint8_t power;
    uint32_t ocr_avail;
} kdrv_sdc_sd_host_t;


/* wdt registers definition */
#ifdef SDC0
#define KDRV_SDC_BASE                   SDC_REG_BASE  //SDC0
#else
#define KDRV_SDC_BASE                   SDIO_REG_BASE   //SDC1
#endif

/* 0x0C: txmode */
#define SDHCI_TXMODE_DMA_EN             BIT(0)
#define SDHCI_TXMODE_BLKCNT_EN          BIT(1)
#define SDHCI_TXMODE_AUTOCMD12_EN       BIT(2)
#define SDHCI_TXMODE_AUTOCMD23_EN       (2 << 2)
#define SDHCI_TXMODE_READ_DIRECTION     BIT(4)
#define SDHCI_TXMODE_WRITE_DIRECTION    (0 << 4)
#define SDHCI_TXMODE_MULTI_SEL          BIT(5)

/* 0x0E: cmd_reg */
/* response type: bit 0 - 4 */
#define SDHCI_CMD_IDX_SHIFT         0x08
#define SDHCI_CMD_TYPE_SHIFT        0x06
#define SDHCI_CMD_DATA_PRESEL_SHIFT 0x05
#define SDHCI_CMD_NO_RESPONSE       0x00    // For no response command
#define SDHCI_CMD_RTYPE_R2          0x09    // For R2, resp(136b)
#define SDHCI_CMD_RTYPE_R3R4        0x02    // For R3,R4, resp(48b)
#define SDHCI_CMD_RTYPE_R1R5R6R7    0x1A    // For R1,R5,R6,R7, resp(48b)
#define SDHCI_CMD_RTYPE_R1BR5B      0x1B    // For R1b, R5b, resp(48b)
#define SDHCI_CMD_TYPE_NORMAL       0x00    // command type
#define SDHCI_CMD_TYPE_SUSPEND      0x01
#define SDHCI_CMD_TYPE_RESUME       0x02
#define SDHCI_CMD_TYPE_ABORT        0x03


#define SDHCI_CMD_DATA_PRESENT      0x01

/* 0x20: Buf data port*/
#define SDHCI_REG_DATA_PORT         0x20

/* 0x24: Present State Register */
#define SDHCI_REG_PRE_STATE         0x24
#define SDHCI_STS_CMD_INHIBIT       BIT(0)
#define SDHCI_STS_CMD_DAT_INHIBIT   BIT(1)
#define SDHCI_STS_DAT_LINE_ACT      BIT(2)
#define SDHCI_STS_WRITE_TRAN_ACT    BIT(8)
#define SDHCI_STS_READ_TRAN_ACT     BIT(9)
#define SDHCI_STS_BUFF_WRITE        BIT(10)
#define SDHCI_STS_BUFF_READ         BIT(11)
#define SDHCI_STS_CARD_INSERT       BIT(16)
#define SDHCI_STS_CARD_STABLE       BIT(17)
#define SDHCI_STS_CARD_WP           BIT(19)
#define SDHCI_STS_DAT_LINE_LEVEL    (0xF << 20)
#define SDHCI_STS_CMD_LINE_LEVEL    BIT(24)

/* 0x28: hcreg */
#define SDHCI_REG_HC                0x28
#define SDHCI_HC_LED_ON             BIT(0)
#define SDHCI_HC_BUS_WIDTH_4BIT     BIT(1)
#define SDHCI_HC_HI_SPEED           BIT(2)
#define SDHCI_HC_USE_ADMA2          BIT(3)
#define SDHCI_HC_BUS_WIDTH_8BIT     BIT(5)
#define SDHCI_HC_CARD_DETECT_TEST   BIT(6)
#define SDHCI_HC_CARD_DETECT_SIGNAL BIT(7)

/* 0x29: */
#define SDHCI_POWER_ON              BIT(0)
#define SDHCI_POWER_180             (5 << 1)
#define SDHCI_POWER_300             (6 << 1)
#define SDHCI_POWER_330             (7 << 1)

/* 0x2A: blk_gap_ctl*/
#define SDHCI_STOP_AT_BLOCK_GAP_REQ BIT(0)
#define SDHCI_CONTINUE_REQ          BIT(1)
#define SDHCI_READ_WAIT_CTL         BIT(2)
#define SDHCI_INT_AT_BLOCK_GAP      BIT(3)

/* 0x2C: ClkCntl */
#define SDHCI_REG_CLK_CTRL          0x2C
#define SDHCI_CLK_CTRL_LOW_CLK_SEL_SHIFT    8
#define SDHCI_CLK_CTRL_UP_CLK_SEL_SHIFT     6
#define SDHCI_CLK_CTRL_INTERNALCLK_EN       BIT(0)
#define SDHCI_CLK_CTRL_INTERNALCLK_STABLE   BIT(1)
#define SDHCI_CLK_CTRL_SDCLK_EN             BIT(2)
#define SDHCI_CLK_CTRL_CLK_GEN_SEL_PRO      BIT(5)

/* 0x2F: softrst */
#define SDHCI_SOFTRST_ALL           BIT(0)
#define SDHCI_SOFTRST_CMD           BIT(1)
#define SDHCI_SOFTRST_DAT           BIT(2)

/* 0x30: intr_sts */
#define SDHCI_REG_INTR_STATE        0x30
#define SDHCI_INTR_STS_ERR          BIT(15)
#define SDHCI_INTR_STS_CARD_INTR    BIT(8)
#define SDHCI_INTR_STS_CARD_REMOVE  BIT(7)
#define SDHCI_INTR_STS_CARD_INSERT  BIT(6)
#define SDHCI_INTR_STS_BUFF_READ_READY  BIT(5)
#define SDHCI_INTR_STS_BUFF_WRITE_READY BIT(4)
#define SDHCI_INTR_STS_DMA          BIT(3)
#define SDHCI_INTR_STS_BLKGAP       BIT(2)
#define SDHCI_INTR_STS_TXR_COMPLETE BIT(1)
#define SDHCI_INTR_STS_CMD_COMPLETE BIT(0)  /* CMD completed, CMD12/CMD23 will not generate this command */

/* 0x32: err_sts */
#define SDHCI_INTR_ERR_TUNING       BIT(10)
#define SDHCI_INTR_ERR_ADMA         BIT(9)
#define SDHCI_INTR_ERR_AUTOCMD      BIT(8)
#define SDHCI_INTR_ERR_CURR_LIMIT   BIT(7)
#define SDHCI_INTR_ERR_DATA_ENDBIT  BIT(6)
#define SDHCI_INTR_ERR_DATA_CRC     BIT(5)
#define SDHCI_INTR_ERR_DATA_TIMEOUT BIT(4)
#define SDHCI_INTR_ERR_CMD_INDEX    BIT(3)
#define SDHCI_INTR_ERR_CMD_ENDBIT   BIT(2)
#define SDHCI_INTR_ERR_CMD_CRC      BIT(1)
#define SDHCI_INTR_ERR_CMD_TIMEOUT  BIT(0)
#define SDHCI_INTR_ERR_CMD_LINE     (SDHCI_INTR_ERR_CMD_INDEX | SDHCI_INTR_ERR_CMD_ENDBIT | SDHCI_INTR_ERR_CMD_CRC | SDHCI_INTR_ERR_CMD_TIMEOUT)
#define SDHCI_INTR_ERR_DAT_LINE     (SDHCI_INTR_ERR_DATA_ENDBIT | SDHCI_INTR_ERR_DATA_CRC | SDHCI_INTR_ERR_DATA_TIMEOUT)

/* 0x34: intr_en */
#define SDHCI_INTR_EN_ALL           (0x10FF)

/* 0x36: err_en */
#define SDHCI_ERR_EN_ALL            (0xF7FF)

/* 0x38: intr_sign_en */
//#define SDHCI_INTR_SIG_EN_ALL     (0xF0CC)
//#define SDHCI_INTR_SIG_EN         (0x10CC)
#define SDHCI_INTR_SIG_EN (SDHCI_INTR_STS_CARD_REMOVE | SDHCI_INTR_STS_CARD_INSERT | SDHCI_INTR_STS_CMD_COMPLETE | SDHCI_INTR_STS_TXR_COMPLETE)
#define SDHCI_INTR_SIGN_EN_SDMA (SDHCI_INTR_SIG_EN | SDHCI_INTR_STS_DMA | SDHCI_INTR_STS_BLKGAP)
#define SDHCI_INTR_SIGN_EN_ADMA (SDHCI_INTR_SIG_EN | SDHCI_INTR_STS_DMA)
#define SDHCI_INTR_SIGN_EN_PIO (SDHCI_INTR_SIG_EN | SDHCI_INTR_STS_BLKGAP)

/* 0x3A: err_sign_en */
#define SDHCI_ERR_SIG_EN_ALL        (0xF3FF)

/* 0x3C: AUTOCMD12 ERR */
#define SDHCI_AUTOCMD12_ERR_NOT_EXECUTED    BIT(0)
#define SDHCI_AUTOCMD12_ERR_TIMEOUT         BIT(1)
#define SDHCI_AUTOCMD12_ERR_CRC             BIT(2)
#define SDHCI_AUTOCMD12_ERR_END_BIT         BIT(3)
#define SDHCI_AUTOCMD12_ERR_INDEX           BIT(4)
#define SDHCI_AUTOCMD12_ERR_CMD_NOT_ISSUE   BIT(7)

/* 0x3E: Host Control 2 */
#define SDHCI_REG_HOST_CTRL2    0x3E
#define SDHCI_PRESET_VAL_EN     BIT(15)
#define SDHCI_ASYNC_INT_EN      BIT(14)
#define SDHCI_SMPL_CLCK_SELECT  BIT(7)
#define SDHCI_EXECUTE_TUNING    BIT(6)    /* Write 1 Auto clear */
#define SDHCI_DRV_TYPE_MASK     BIT(4)
#define SDHCI_DRV_TYPE_SHIFT    4
#define SDHCI_DRV_TYPEB         0
#define SDHCI_DRV_TYPEA         1
#define SDHCI_DRV_TYPEC         2
#define SDHCI_DRV_TYPED         3
#define SDHCI_18V_SIGNAL        BIT(3)
#define SDHCI_UHS_MODE_MASK     (7 << 0)
#define SDHCI_SDR12             0
#define SDHCI_SDR25             1
#define SDHCI_SDR50             2
#define SDHCI_SDR104            3
#define SDHCI_DDR50             4

/* 0x40: Capabilities */
#define SDHCI_CAP_VOLTAGE_33V           BIT(24)
#define SDHCI_CAP_VOLTAGE_30V           BIT(25)
#define SDHCI_CAP_VOLTAGE_18V           BIT(26)
#define SDHCI_CAP_FIFO_DEPTH_16BYTE     (0 << 29)
#define SDHCI_CAP_FIFO_DEPTH_32BYTE     (1 << 29)
#define SDHCI_CAP_FIFO_DEPTH_64BYTE     (2 << 29)
#define SDHCI_CAP_FIFO_DEPTH_512BYTE    (3 << 29)
#define SDHCI_CAP_FIFO_DEPTH_1024BYTE   (4 << 29)
#define SDHCI_CAP_FIFO_DEPTH_2048BYTE   (5 << 29)

/* 0x44 - 0x47 */
#define SDHCI_SUPPORT_SDR50        BIT(0)
#define SDHCI_SUPPORT_SDR104       BIT(1)
#define SDHCI_SUPPORT_DDR50        BIT(2)
#define SDHCI_SUPPORT_DRV_TYPEA    BIT(4)
#define SDHCI_SUPPORT_DRV_TYPEC    BIT(5)
#define SDHCI_SUPPORT_DRV_TYPED    BIT(6)
#define SDHCI_RETUNING_TIME_MAS    0xF
#define SDHCI_RETUNING_TIME_SHIFT  8
#define SDHCI_SDR50_TUNING         BIT(13)
#define SDCHI_RETUNING_MODE_MASK   0x3
#define SDHCI_RETUNING_MODE_SHIFT  14

/* Vendor Defined0(0x100) */
/* Vendor Defined1(0x104) */
#define MMC_BOOT_ACK                BIT(2)
#define MMC_BUS_TEST_MODE           0x3
#define MMC_ALTERNATIVE_BOOT_MODE   0x2
#define MMC_BOOT_MODE               0x1
#define NORMAL_MODE                 0x0
/* Vendor Defined2(0x108) */
/* Vendor Defined3(0x10C) */

//-----------------------------------------------------------------------------
// SDHCI Command Interface Definition
//-----------------------------------------------------------------------------
#define SDHCI_CMD0_GO_IDLE_STATE            0
#define SDHCI_CMD1_MMC_SEND_OP_COND         1
#define SDHCI_CMD2_SEND_ALL_CID             2
#define SDHCI_CMD3_SEND_RELATIVE_ADDR       3
#define SDHCI_CMD5_IO_SEND_OP_COND          5
#define SDHCI_CMD6_SWITCH_FUNC              6
#define SDHCI_CMD6_SET_BUS_WIDTH            6
#define SDHCI_CMD7_SELECT_CARD              7
#define SDHCI_CMD8_SEND_IF_COND             8
#define SDHCI_CMD8_SEND_EXT_CSD             8
#define SDHCI_CMD9_SEND_CSD                 9
#define SDHCI_CMD10_SEND_CID                10
#define SDHCI_CMD11_VOLTAGE_SWITCH          11
#define SDHCI_CMD12_STOP_TRANS              12
#define SDHCI_CMD13_SEND_STATUS             13
#define SDHCI_CMD13_SD_STATUS               13
#define SDHCI_CMD16_SET_BLOCKLEN            16
#define SDHCI_CMD17_READ_SINGLE_BLOCK       17
#define SDHCI_CMD18_READ_MULTI_BLOCK        18
#define SDHCI_CMD19_SEND_TUNE_BLOCK         19
#define SDHCI_CMD23_SET_WR_BLOCK_CNT        23
#define SDHCI_CMD24_WRITE_BLOCK             24
#define SDHCI_CMD25_WRITE_MULTI_BLOCK       25

#define SDHCI_CMD32_ERASE_WR_BLK_START      32
#define SDHCI_CMD33_ERASE_WR_BLK_END        33
#define SDHCI_CMD35_ERASE_GROUP_START       35
#define SDHCI_CMD36_ERASE_GROUP_END         36
#define SDHCI_CMD38_ERASE                   38
#define SDHCI_CMD41_SD_SEND_OP_COND         41
#define SDHCI_CMD43_GET_MKB                 43
#define SDHCI_CMD44_GET_MID                 44
#define SDHCI_CMD45_CER_RN1                 45
#define SDHCI_CMD46_CER_RN2                 46
#define SDHCI_CMD47_CER_RES2                47
#define SDHCI_CMD48_CER_RES1                48
#define SDHCI_CMD51_SEND_SCR                51
#define SDHCI_CMD52_IO_RW_DIRECT            52
#define SDHCI_CMD53_IO_RW_EXTENDED          53
#define SDHCI_CMD55_APP                     55
#define SDHCI_CMD56_GEN                     56

#define SDHCI_CMD8_SEND_IF_COND_ARGU             0x1AA
#define SDHCI_CMD41_SD_SEND_OP_COND_HCS_ARGU     0xC0FF8000
#define SDHCI_CMD41_SD_SEND_OP_COND_ARGU         0x00FF8000
#define SDHCI_CMD1_MMC_SEND_OP_COND_BYTE_MODE    0x80FF8000
#define SDHCI_CMD1_MMC_SEND_OP_COND_SECTOR_MODE  0xC0FF8000

#define CMD_RETRY_CNT               5
#define SDHCI_TIMEOUT               0xFFF


/* For CMD52*/
#define SD_CMD52_RW_in_W        0x80000000
#define SD_CMD52_RW_in_R        0x00000000
#define SD_CMD52_RAW            0x08000000
#define SD_CMD52_no_RAW         0x00000000
#define SD_CMD52_FUNC(Num)      (Num  << 28)
#define SD_CMD52_Reg_Addr(Addr) (Addr << 9)
/* For CMD53*/
#define SD_CMD53_RW_in_W        0x80000000
#define SD_CMD53_RW_in_R        0x00000000
#define SD_CMD53_FUNC(Num)      (Num  << 28)
#define SD_CMD53_Block_Mode     0x08000000
#define SD_CMD53_Byte_Mode      0x00000000
#define SD_CMD53_OP_inc         0x04000000
#define SD_CMD53_OP_fix         0x00000000
#define SD_CMD53_Reg_Addr(Addr) (Addr << 9)
//************************************
/**
 * Card status return from R1 response format.
 * Or use CMD13 to get this status
 */
#define SD_STATUS_OUT_OF_RANGE        0x80000000
#define SD_STATUS_ADDRESS_ERROR       BIT(30)
#define SD_STATUS_BLOCK_LEN_ERROR     BIT(29)
#define SD_STATUS_ERASE_SEQ_ERROR     BIT(28)
#define SD_STATUS_ERASE_PARAM         BIT(27)
#define SD_STATUS_WP_VIOLATION        BIT(26)
#define SD_STATUS_CARD_IS_LOCK        BIT(25)
#define SD_STATUS_LOCK_UNLOCK_FAILED  BIT(24)
#define SD_STATUS_COM_CRC_ERROR       BIT(23)
#define SD_STATUS_ILLEGAL_COMMAND     BIT(22)
#define SD_STATUS_CARD_ECC_FAILED     BIT(21)
#define SD_STATUS_CC_ERROR            BIT(20)
#define SD_STATUS_ERROR               BIT(19)
#define SD_STATUS_UNDERRUN            BIT(18)
#define SD_STATUS_OVERRUN             BIT(17)
#define SD_STATUS_CSD_OVERWRITE       BIT(16)
#define SD_STATUS_WP_ERASE_SKIP       BIT(15)
#define SD_STATUS_CARD_ECC_DISABLE    BIT(14)
#define SD_STATUS_ERASE_RESET         BIT(13)
#define SD_STATUS_CURRENT_STATE       (0xF << 9)

typedef enum {
    CUR_STATE_IDLE = 0,
    CUR_STATE_READY,
    CUR_STATE_IDENT,
    CUR_STATE_STBY,
    CUR_STATE_TRAN,
    CUR_STATE_DATA,
    CUR_STATE_RCV,
    CUR_STATE_PRG,
    CUR_STATE_DIS,
    CUR_STATE_RSV
} kdrv_sdc_card_state_e;

#define SD_STATUS_READY_FOR_DATA      BIT(8)
#define MMC_STATUS_SWITCH_ERROR       BIT(7)
#define SD_STATUS_APP_CMD             BIT(5)
#define SD_STATUS_AKE_SEQ_ERROR       BIT(3)

#define SD_STATUS_ERROR_BITS          (SD_STATUS_OUT_OF_RANGE | SD_STATUS_ADDRESS_ERROR | \
                                       SD_STATUS_BLOCK_LEN_ERROR | SD_STATUS_ERASE_SEQ_ERROR | \
                                       SD_STATUS_ERASE_PARAM | SD_STATUS_WP_VIOLATION | \
                                       SD_STATUS_LOCK_UNLOCK_FAILED | SD_STATUS_CARD_ECC_FAILED | \
                                       SD_STATUS_CC_ERROR | SD_STATUS_ERROR | \
                                       SD_STATUS_UNDERRUN | SD_STATUS_OVERRUN | \
                                       SD_STATUS_CSD_OVERWRITE | SD_STATUS_WP_ERASE_SKIP | \
                                       SD_STATUS_AKE_SEQ_ERROR | MMC_STATUS_SWITCH_ERROR)

#define SDHCI_1BIT_BUS_WIDTH    0x0
#define SDHCI_4BIT_BUS_WIDTH    0x2

/* ADMA Descriptor Table Generator */
#define ADMA2_ENTRY_VALID       BIT(0)
#define ADMA2_ENTRY_END         BIT(1)
#define ADMA2_ENTRY_INT         BIT(2)

#define ADMA2_NOP               (0 << 4)
#define ADMA2_SET               (1 << 4)
#define ADMA2_TRAN              (2 << 4)
#define ADMA2_LINK              (3 << 4)

typedef struct {
    uint16_t attr;
    uint16_t lgth;
    uint32_t addr;
} kdrv_sdc_adma2desc_table_t;


typedef struct {
    volatile kdrv_sdc_reg_t *sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info;
    kdrv_sdc_sd_host_t *host;
    kdrv_sdc_infinite_test_e infinite_mode;
    uint16_t fifo_depth;
    uint32_t timeout_ms;
    uint32_t data_present;
    uint32_t adma2_use_interrupt;
    uint32_t adma2_insert_nop;
    uint32_t response_type;
    uint32_t inhibit_datchk;
} kdrv_sdc_res_t;


//=============================================================================
//                              KDRV SDC API
//=============================================================================

/**
 * @brief kdrv_sdc_initialize, initail sd/emmc card interface
 *      1. reset sdc status
 *      2. allocate resource and initail driving
 *      3. turn on sdc clock
 * @param[in]   dev_id      device id, ref @kdrv_sdc_dev_e
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_sdc_initialize(kdrv_sdc_dev_e dev_id);

/**
 * @brief kdrv_sdc_uninitialize, uninitail sd/emmc card interface and resource
 *      1. reset sdc status and initail driven
 *      2. turn on sdc clock
 *
 * @param[in]   dev_id      device id, ref @kdrv_sdc_dev_e
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_sdc_uninitialize(kdrv_sdc_dev_e dev_id);

/**
 * @brief kdrv_sdc_dev_scan() scan sd/mmc memory card
 *
 * @param[in]   dev_id      device id, ref @kdrv_sdc_dev_e
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_sdc_dev_scan(kdrv_sdc_dev_e dev_id);


/**
 * @brief kdrv_sdc_get_dev() get device structure
 *
 * @param[in]   dev_id      device id, ref @kdrv_sdc_dev_e
 * @return kdrv_status_t
 */
kdrv_sdc_res_t* kdrv_sdc_get_dev(kdrv_sdc_dev_e dev_id);


/**
 * @brief kdrv_sdc_read read data from sd/mmc card
 *
 * @param[in]   dev_id      device id, ref @kdrv_sdc_dev_e
 * @param[in]   buf         buffer to write.
 * @param[in]   sd_offset   sd/mmc offset address
 * @param[in]   size        read size(Multiple of 512, 1sector=512Bytes, max block 65535 ~ 3MB)
 * @return      kdrv_status_t
 */ 
kdrv_status_t kdrv_sdc_read(kdrv_sdc_dev_e dev_id, uint8_t *buf, uint32_t sd_offset, uint32_t size);


/**
 * @brief kdrv_sdc_write write data from sd/mmc card
 *
 * @param[in]   dev_id      device id, ref @kdrv_sdc_dev_e
 * @param[in]   buf         buffer to write.
 * @param[in]   sd_offset   sd/mmc offset address
 * @param[in]   size        write size(Multiple of 512, 1sector=512Bytes, max block 65535 ~ 3MB)
 * @return      kdrv_status_t
 */ 
kdrv_status_t kdrv_sdc_write(kdrv_sdc_dev_e dev_id, uint8_t *buf, uint32_t sd_offset, uint32_t size);
#endif /* __KDRV_SDC_H__ */
