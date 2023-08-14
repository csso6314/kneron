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


#ifndef __KDRV_SDC_MMC_H__
#define __KDRV_SDC_MMC_H__
#include "base.h"


typedef struct {
	uint32_t csd_structure:2;
	uint32_t spec_vers:4;
	uint32_t reserved1:2;
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
	uint32_t erase_grp_size:5;
	uint32_t erase_grp_mult:5;
	uint32_t wp_grp_size:5;
	uint32_t wp_grp_enable:1;
	uint32_t default_ecc:2;
	uint32_t r2w_factor:3;
	uint32_t write_bl_len:4;
	uint32_t write_bl_partial:1;
	uint32_t reserved3:4;
	uint32_t content_prot_app:1;
	uint32_t file_format_grp:1;
	uint32_t COPY:1;
	uint32_t perm_write_protect:1;
	uint32_t tmp_write_protect:1;
	uint32_t file_format:2;
	uint32_t ecc:2;
} kdrv_sdc_mmc_csd_t;

typedef struct {
	/* Modes Segment */
	uint8_t reserved27[134];
	uint8_t sec_bad_blk_mgmnt;
	uint8_t reserved26; 
	uint8_t enh_start_addr[4];
	uint8_t enh_size_mult[3];
	uint8_t gp_size_mult[12];
	uint8_t partition_setting_completed;
	uint8_t partitioning_attribute;
	uint8_t max_enh_size_mult[3];
	uint8_t partitioning_support;
	uint8_t reserved25;
	uint8_t rst_n_function;
	uint8_t reserved24[5];
	uint8_t rpmb_size_mult;
	uint8_t fw_config;
	uint8_t reserved23;
	uint8_t user_wp;
	uint8_t reserved22;
	uint8_t boot_wp;		    /* [173] R/W & R/W/C_P */
	uint8_t reserved21;
	uint8_t erase_group_def;
	uint8_t reserved20;
	uint8_t boot_bus_width;	    /* [177] R/W/E */
	uint8_t boot_config_prot;	/* [178] R/W & R/W/C_P */
	uint8_t partition_conf;	    /* [179] */
	uint8_t reserved19;
	uint8_t erased_mem_cont;
	uint8_t reserved18;
	uint8_t bus_width;	        /* [183] W/E_P */
	uint8_t reserved17;
	uint8_t hs_timing;	        /* [185] R/W/E_P */
	uint8_t reserved16;
	uint8_t power_class;
	uint8_t reserved15;
	uint8_t cmd_set_rev;
	uint8_t reserved14;
	uint8_t cmd_set;
	/* Properties Segment */
	uint8_t ext_csd_rev;
	uint8_t reserved13;
	uint8_t csd_structure;
	uint8_t reserved12;
	uint8_t cardtype;
	uint8_t reserved11[3];
	uint8_t pwr_cl_52_195;
	uint8_t pwr_cl_26_195;
	uint8_t pwr_cl_52_360;
	uint8_t pwr_cl_26_360;
	uint8_t reserved10;
	uint8_t min_perf_r_4_26;
	uint8_t min_perf_w_4_26;
	uint8_t min_perf_r_8_26_4_52;
	uint8_t min_perf_w_8_26_4_52;
	uint8_t min_perf_r_8_52;
	uint8_t min_perf_w_8_52;
	uint8_t reserved9;
	uint32_t sec_count;	        /* [215:212] R */
	uint8_t reserved8;
	uint8_t s_a_timeout;
	uint8_t reserved7;
	uint8_t s_c_vccq;
	uint8_t s_c_vcc;
	uint8_t hc_wp_grp_size;
	uint8_t ref_wr_sec_c;
	uint8_t erase_timeout_mult;
	uint8_t hc_erase_grp_size;
	uint8_t acc_size;
	uint8_t boot_size_mult;	/* [226] R */
	uint8_t reserved6;	        /* [227] , (embedded mmc )is 2 bytes width. */
	uint8_t boot_info;	        /* [228] R */
	uint8_t sec_trim_mult;
	uint8_t sec_erase_mult;
	uint8_t sec_feature_support;
	uint8_t trim_mult;
	uint8_t reserved5;
	uint8_t min_perf_ddr_r_8_52;
	uint8_t min_perf_ddr_w_8_52_8_52;
	uint8_t reserved4[2];
	uint8_t pwr_cl_ddr_52_195; 	/* [238] */
	uint8_t pwr_cl_ddr_52_360;	/* [239] */
	uint8_t reserved3;
	uint8_t ini_timeout_ap;	    /* [241] */
	uint8_t reserved2[262];
	uint8_t s_cmd_set;	    /* [504] */
	uint8_t reserved1[7];	/* [511:505] */
} kdrv_sdc_mmc_ext_csd_t;

/* CMD INDEX */
#define SDHCI_MMC_SWITCH		6
#define SDHCI_MMC_VENDOR_CMD	62

/* Cmd Set [2:0] of argument SWITCH command*/
#define EXT_CSD_CMD_SET_NORMAL          (1<<0)
#define EXT_CSD_CMD_SET_SECURE          (1<<1)
#define EXT_CSD_CMD_SET_CPSECURE        (1<<2)

/* Offset at EXT CSD to access */
#define EXT_CSD_PARTITION_SETTING_COMPLETED	156
#define EXT_CSD_PARTITION_CONF			179
#define EXT_CSD_BUS_WIDTH       183	    /* R/W */
#define EXT_CSD_HS_TIMING       185	    /* R/W */
#define EXT_CSD_CARD_TYPE       196	    /* RO */
#define EXT_CSD_SEC_CNT         212	    /* RO, 4 bytes */
#define EXT_CSD_BOOT_SIZE_MULT	226


#define EXT_CSD_CMD_SET		    0x0
#define EXT_CSD_SET_BIT			0x1
#define	EXT_CSD_CLR_BYTE		0x2
#define EXT_CSD_WRITE_BYTE		0x3

#define EXT_CSD_BUS_8BIT		0x2
#define EXT_CSD_BUS_4BIT		0x1
#define EXT_CSD_BUS_1BIT		0x0

#define MMC_CMD6_ACCESS_MODE(x)	(uint32_t)( x << 24)
#define MMC_CMD6_INDEX(x)		(uint32_t)( x << 16)
#define MMC_CMD6_VALUE(x)		(uint32_t)( x << 8)
#define MMC_CMD6_CMD_SET(x)		(uint32_t)( x )

#define MMC_CARD_BUSY   0x80000000	/* Card Power up status bit */

#endif      //__KDRV_SDC_MMC_H__
