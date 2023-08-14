/**
 * Kneron Peripheral API - SDC
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */


#include <stdlib.h>
#include <string.h>
#include "kdrv_sdc.h"
//#include "kmdw_console.h"
#include "kdrv_clock.h"
#include "kdrv_scu_ext.h"
#include "kdrv_pwm.h"

//#define KDRV_SDC_DBG          //debug message flag
#ifdef KDRV_SDC_DBG
#define kmdw_dbg_printf   kmdw_printf
#else
#define kmdw_dbg_printf(fmt, ...)
#endif

typedef signed long clock_t;
#define FTSDC021_BASE_CLOCK  100    /* 100MHz */
#define SDC_AHB_CHANNEL      0

#define USE_PIO

/* SDC0 Resources */
kdrv_sdc_res_t sdc0_res = {
    (kdrv_sdc_reg_t *)SDC_FTSDC021_PA_BASE,
    NULL,
    NULL,
    INFINITE_NO,
    0,
    0,//timeout ns
    0,
    0, //insert interrupt
    0, //insert nop
    0, //response type
    0, //inhibit_datchk
};


static const uint32_t tran_exp[] = {
    10000, 100000, 1000000, 10000000,
    0, 0, 0, 0
};

static const uint8_t tran_mant[] = {
    0, 10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static kdrv_sdc_res_t *gdev = &sdc0_res;

uint32_t sdma_bound_mask;
kdrv_sdc_adma2desc_table_t sdc_adma2_desc_table[ADMA2_NUM_OF_LINES];
volatile uint8_t err_recover = 0;
uint32_t rd_bl_len, wr_bl_len;
//volatile BOOL cardChanged = false;

static kdrv_status_t kdrv_sdc_auto_cmd_error_recovery(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_error_recovery(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_wait_for_state(kdrv_sdc_res_t *dev, uint32_t state, uint32_t ms);
static kdrv_status_t kdrv_sdc_transfer_data(kdrv_sdc_res_t *dev, kdrv_sdc_transfer_act_e act, uint32_t * buffer, uint32_t length);
static kdrv_status_t kdrv_sdc_set_transfer_mode(kdrv_sdc_res_t *dev, uint8_t blk_cnt_en, uint8_t auto_cmd, uint8_t dir, uint8_t multi_blk);
static kdrv_status_t kdrv_sdc_send_command(kdrv_sdc_res_t *dev, uint8_t cmd_idx, uint8_t cmd_type, uint8_t data_present, uint8_t resp_type, uint8_t inhibit_datchk, uint32_t argu);
static kdrv_status_t kdrv_sdc_prepare_data(kdrv_sdc_res_t *dev,uint32_t blk_cnt, uint16_t blk_sz, uint32_t buff_addr, kdrv_sdc_transfer_act_e act);

/*
 * MMC card related operations
 */
static kdrv_status_t kdrv_sdc_ops_send_op_cond(kdrv_sdc_res_t *dev, uint32_t ocr, volatile uint32_t * rocr);
static kdrv_status_t kdrv_sdc_ops_mmc_switch(kdrv_sdc_res_t *dev, uint8_t set, uint8_t index, uint8_t value);
static kdrv_status_t kdrv_sdc_ops_send_ext_csd(kdrv_sdc_res_t *dev);

/*
 * SD card related operations(Some applies to MMC)
 */
static kdrv_status_t kdrv_sdc_ops_go_idle_state(kdrv_sdc_res_t *dev, uint32_t arg);
static kdrv_status_t kdrv_sdc_ops_send_if_cond(kdrv_sdc_res_t *dev, uint32_t arg);
static kdrv_status_t kdrv_sdc_ops_send_app_op_cond(kdrv_sdc_res_t *dev, uint32_t ocr, volatile uint32_t * rocr);
static kdrv_status_t kdrv_sdc_ops_all_send_cid(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_ops_send_rca(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_ops_send_csd(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_ops_select_card(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_ops_app_send_scr(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_ops_app_set_bus_width(kdrv_sdc_res_t *dev, uint32_t width);
static kdrv_status_t kdrv_sdc_ops_sd_switch(kdrv_sdc_res_t *dev, uint32_t mode, uint32_t group, uint8_t value, volatile uint8_t * resp);
static kdrv_status_t kdrv_sdc_ops_send_tune_block(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_ops_send_card_status(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_card_read(kdrv_sdc_res_t *dev, uint32_t startAddr, uint32_t blkcnt, uint8_t * read_buf);
static kdrv_status_t kdrv_sdc_card_write(kdrv_sdc_res_t *dev, uint32_t startAddr, uint32_t blkcnt, uint8_t * write_buf);
static kdrv_status_t kdrv_sdc_set_bus_width(kdrv_sdc_res_t *dev, uint8_t width);
static void kdrv_sdc_set_sd_clock(kdrv_sdc_res_t *dev, uint32_t clock);
static kdrv_status_t kdrv_sdc_read_scr(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_read_ext_csd(kdrv_sdc_res_t *dev);
static kdrv_status_t kdrv_sdc_set_bus_speed_mode(kdrv_sdc_res_t *dev, uint8_t speed);
static void kdrv_sdc_set_sd_clock(kdrv_sdc_res_t *dev, uint32_t clock);


/*
 * MMC card related operations
 */
/**
 * @brief kdrv_sdc_ops_send_op_cond() send CMD1 op_cond command to wakeup device
 *
 * @param[in]   ocr         ocr input information
 * @param[out]  rocr
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_op_cond(kdrv_sdc_res_t *dev, uint32_t ocr,
        volatile uint32_t * rocr)
{
    kdrv_status_t err;
    uint32_t i;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    for (i = 100; i; i--) {
        err = kdrv_sdc_send_command(dev, SDHCI_CMD1_MMC_SEND_OP_COND,
                                    SDHCI_CMD_TYPE_NORMAL, 0, SDHCI_CMD_RTYPE_R3R4, 0, ocr);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (card->resp_lo & MMC_CARD_BUSY)
            break;
        kdrv_delay_us(10000);
    }

    if (rocr)
        *rocr = card->resp_lo;

    return err;
}

/**
 * @brief kdrv_sdc_ops_mmc_switch() send CMD6 switch command to change the EXT_CSD data
 *
 * @param[in]   dev         device structure
 * @param[in]   sw_type     switch type (normal, secure, cpsecure)
 * @param[in]   index       data index
 * @param[in]   speed       speed attribute
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_mmc_switch(kdrv_sdc_res_t *dev, uint8_t sw_type, uint8_t index, uint8_t value)
{
    kdrv_status_t err;
    uint32_t arg;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    if (card->card_type != MEMORY_CARD_TYPE_MMC) {
        //kmdw_dbg_printf( "Switch Function: This is not MMC Card !\n");
        return KDRV_STATUS_SDC_CARD_TYPE_ERR;
    }

    arg = (EXT_CSD_WRITE_BYTE << 24) | (index << 16) | (value << 8) | sw_type;

    err = kdrv_sdc_send_command(dev, SDHCI_CMD6_SWITCH_FUNC, SDHCI_CMD_TYPE_NORMAL,
                                0, SDHCI_CMD_RTYPE_R1BR5B, 1, arg);
    return err;
}

/**
 * @brief kdrv_sdc_ops_send_ext_csd() send ext_csd(CMD8) request command
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_ext_csd(kdrv_sdc_res_t *dev)
{
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    if (card_info->card_type == MEMORY_CARD_TYPE_MMC && card_info->csd_mmc.spec_vers < 4) {
        //kmdw_dbg_printf(" Commmand 8 is not supported in this MMC system spec.\n");
        return KDRV_STATUS_OK;
    }

    memset((void *)&card_info->ext_csd_mmc, 0, EXT_CSD_LENGTH);

    kdrv_sdc_set_transfer_mode(dev, 0, 0, SDHCI_TXMODE_READ_DIRECTION, 0);
    kdrv_sdc_prepare_data(dev, 1, EXT_CSD_LENGTH, (uint32_t) & (card_info->ext_csd_mmc), READ);

    /* CMD 8 */
    if ((err = kdrv_sdc_send_command(dev, SDHCI_CMD8_SEND_EXT_CSD,
                                     SDHCI_CMD_TYPE_NORMAL, 1, SDHCI_CMD_RTYPE_R1R5R6R7, 0, 0)) != KDRV_STATUS_OK) {
        //kmdw_dbg_printf("Getting the Ext-CSD failed\n");
        return err;
    }

    return kdrv_sdc_transfer_data(dev, READ, (uint32_t *) & (card_info->ext_csd_mmc), EXT_CSD_LENGTH);
}


/**
 * @brief kdrv_sdc_ops_go_idle_state() enter idle state
 *      MMC4.4:
 *      argument = 0x00000000, Resets the card to idle state.
 *      argument = 0xF0F0F0F0, Resets the card to pre-idle state.
 *      argument = 0xFFFFFFFA, Initiate alternative boot operation.
 *
 *      SD Card: Argument always 0x00000000.
 *
 * @param   dev         device structure
 * @param   arg         input argument
 * @return  kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_go_idle_state(kdrv_sdc_res_t *dev, uint32_t arg)
{
    uint8_t data;

    data = (arg == 0xFFFFFFFA) ? 1 : 0;
    return (kdrv_sdc_send_command(dev, SDHCI_CMD0_GO_IDLE_STATE,
                                  SDHCI_CMD_TYPE_NORMAL, data, SDHCI_CMD_NO_RESPONSE, 0, arg));
}


/* SD card related operations */
/**
 * @brief kdrv_sdc_ops_send_if_cond() send ext_csd(CMD8) request command
 *              CMD8 SD card
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_if_cond(kdrv_sdc_res_t *dev, uint32_t arg)
{
    kdrv_status_t err;
    uint8_t test_pattern = (arg & 0xFF);
    uint8_t result_pattern;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    err = kdrv_sdc_send_command(dev, SDHCI_CMD8_SEND_IF_COND,
                                SDHCI_CMD_TYPE_NORMAL, 0, SDHCI_CMD_RTYPE_R1R5R6R7, 0, arg);
    if (err)
        return err;

    result_pattern = card->resp_lo & 0xFF;

    if (result_pattern != test_pattern)
        return KDRV_STATUS_ERROR;

    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_ops_send_if_cond() send ocr(ACMD41) request command
 *
 * @param[in]   dev         device structure
 * @param[in]   ocr         ocr input data
 * @param[out]  rocr        recevied ocr data
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_app_op_cond(kdrv_sdc_res_t *dev, uint32_t ocr,
        volatile uint32_t *rocr)
{
    uint32_t i;
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    for (i = 100; i; i--) {
        /* CMD 55: Indicate to the card the next cmd is app-specific command */
        if ((err = kdrv_sdc_send_command(dev, SDHCI_CMD55_APP, SDHCI_CMD_TYPE_NORMAL,
                                         0, SDHCI_CMD_RTYPE_R1R5R6R7, 0, 0)) != KDRV_STATUS_OK)
            break;

        if ((err = kdrv_sdc_send_command(dev, SDHCI_CMD41_SD_SEND_OP_COND, SDHCI_CMD_TYPE_NORMAL,
                                         0, SDHCI_CMD_RTYPE_R3R4, 0, ocr)) != KDRV_STATUS_OK)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        if (card->resp_lo & MMC_CARD_BUSY)
            break;
        kdrv_delay_us(80000);
    }

    if (rocr)
        *rocr = card->resp_lo;

    return err;
}


/**
 * @brief kdrv_sdc_ops_all_send_cid() send cid(CMD2) request command
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_all_send_cid(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;
    if (kdrv_sdc_send_command(dev, SDHCI_CMD2_SEND_ALL_CID, SDHCI_CMD_TYPE_NORMAL,
                              0, SDHCI_CMD_RTYPE_R2, 1, 0)) {
        //kmdw_dbg_printf("ALL SEND CID failed !\n");
        return KDRV_STATUS_SDC_CID_READ_ERR;
    }

    card->cid_lo = card->resp_lo;
    card->cid_hi = card->resp_hi;
    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_ops_send_rca() send CMD3 to request rca information
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_rca(kdrv_sdc_res_t *dev)
{
    uint32_t i;
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    for (i = 100; i; i--) {
        err = kdrv_sdc_send_command(dev, SDHCI_CMD3_SEND_RELATIVE_ADDR, SDHCI_CMD_TYPE_NORMAL, 0,
                                    SDHCI_CMD_RTYPE_R1R5R6R7, 0, (card->rca << 16));
        if (err)
            break;

        /* MMC card, Host assign rca to card.
         * SD card, Host ask for rca.
         */
        if (card->rca != 0)
            break;

        if ((card->resp_lo >> 16) & 0xffff)
            break;
        //err = 1;
        kdrv_delay_us(10000);
    }

    if (!err)
        if (!card->rca)
            card->rca = (uint16_t) ((card->resp_lo >> 16) & 0xffff);
    return err;
}

/**
 * @brief kdrv_sdc_ops_send_csd() send csd(CMD9) command request
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_csd(kdrv_sdc_res_t *dev)
{
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    /* CMD 9: Getting the CSD register from SD memory card */
    err = kdrv_sdc_send_command(dev, SDHCI_CMD9_SEND_CSD, SDHCI_CMD_TYPE_NORMAL,
                                0, SDHCI_CMD_RTYPE_R2, 0, card->rca << 16);
    if (err)
        return err;

    card->csd_lo = card->resp_lo;
    card->csd_hi = card->resp_hi;

    if (card->card_type == MEMORY_CARD_TYPE_SD) {
        if ((card->csd_hi >> 54) == 0) {
            card->csd_ver1.csd_structure = (card->csd_hi >> 54) & 0x3;
            card->csd_ver1.reserved1 = (card->csd_hi >> 48) & 0x3F;
            card->csd_ver1.taac = (card->csd_hi >> 40) & 0xFF;
            card->csd_ver1.nsac = (card->csd_hi >> 32) & 0xFF;
            card->csd_ver1.tran_speed = (card->csd_hi >> 24) & 0xFF;
            card->csd_ver1.ccc = (card->csd_hi >> 12) & 0xFFF;
            card->csd_ver1.read_bl_len = (card->csd_hi >> 8) & 0xF;
            card->csd_ver1.read_bl_partial = (card->csd_hi >> 7) & 0x1;
            card->csd_ver1.write_blk_misalign = (card->csd_hi >> 6) & 0x1;
            card->csd_ver1.read_blk_misalign = (card->csd_hi >> 5) & 0x1;
            card->csd_ver1.dsr_imp = (card->csd_hi >> 4) & 0x1;
            card->csd_ver1.reserved2 = (card->csd_hi >> 2) & 0x3;
            card->csd_ver1.c_size = (((card->csd_hi & 0x3) << 10) | ((card->csd_lo >> 54) & 0x3FF));
            card->csd_ver1.vdd_r_curr_min = (card->csd_lo >> 51) & 0x7;
            card->csd_ver1.vdd_r_curr_max = (card->csd_lo >> 48) & 0x7;
            card->csd_ver1.vdd_w_curr_min = (card->csd_lo >> 45) & 0x7;
            card->csd_ver1.vdd_w_curr_max = (card->csd_lo >> 42) & 0x7;
            card->csd_ver1.c_size_mult = (card->csd_lo >> 39) & 0x7;
            card->csd_ver1.erase_blk_en = (card->csd_lo >> 38) & 0x1;
            card->csd_ver1.sector_size = (card->csd_lo >> 31) & 0x7F;
            card->csd_ver1.wp_grp_size = (card->csd_lo >> 24) & 0x7F;
            card->csd_ver1.wp_grp_enable = (card->csd_lo >> 23) & 0x1;
            card->csd_ver1.reserved3 = (card->csd_lo >> 21) & 0x3;
            card->csd_ver1.r2w_factor = (card->csd_lo >> 18) & 0x7;
            card->csd_ver1.write_bl_len = (card->csd_lo >> 14) & 0xF;
            card->csd_ver1.write_bl_partial = (card->csd_lo >> 13) & 0x1;
            card->csd_ver1.reserved4 = (card->csd_lo >> 8) & 0x1F;
            card->csd_ver1.file_format_grp = (card->csd_lo >> 7) & 0x1;
            card->csd_ver1.copy = (card->csd_lo >> 6) & 0x1;
            card->csd_ver1.perm_write_protect = (card->csd_lo >> 5) & 0x1;
            card->csd_ver1.tmp_write_protect = (card->csd_lo >> 4) & 0x1;
            card->csd_ver1.file_format = (card->csd_lo >> 2) & 0x3;
            card->csd_ver1.Reserver5 = (card->csd_lo) & 0x3;
        } else if ((card->csd_hi >> 54) == 1) {
            card->csd_ver2.csd_structure = (card->csd_hi >> 54) & 0x3;
            card->csd_ver2.reserved1 = (card->csd_hi >> 48) & 0x3F;
            card->csd_ver2.taac = (card->csd_hi >> 40) & 0xFF;
            card->csd_ver2.nsac = (card->csd_hi >> 32) & 0xFF;
            card->csd_ver2.tran_speed = (card->csd_hi >> 24) & 0xFF;
            card->csd_ver2.ccc = (card->csd_hi >> 12) & 0xFFF;
            card->csd_ver2.read_bl_len = (card->csd_hi >> 8) & 0xF;
            card->csd_ver2.read_bl_partial = (card->csd_hi >> 7) & 0x1;
            card->csd_ver2.write_blk_misalign = (card->csd_hi >> 6) & 0x1;
            card->csd_ver2.read_blk_misalign = (card->csd_hi >> 5) & 0x1;
            card->csd_ver2.dsr_imp = (card->csd_hi >> 4) & 0x1;
            card->csd_ver2.reserved2 = (((card->csd_hi >> 2) & 0x3) << 2) | ((card->csd_lo >> 62) & 0x3);
            card->csd_ver2.c_size = ((card->csd_lo >> 40) & 0x3FFFFF);
            card->csd_ver2.reserved3 = (card->csd_lo >> 39) & 0x1;
            card->csd_ver2.erase_blk_en = (card->csd_lo >> 38) & 0x1;
            card->csd_ver2.sector_size = (card->csd_lo >> 31) & 0x7F;
            card->csd_ver2.wp_grp_size = (card->csd_lo >> 24) & 0x7F;
            card->csd_ver2.wp_grp_enable = (card->csd_lo >> 23) & 0x1;
            card->csd_ver2.reserved4 = (card->csd_lo >> 21) & 0x3;
            card->csd_ver2.r2w_factor = (card->csd_lo >> 18) & 0x7;
            card->csd_ver2.write_bl_len = (card->csd_lo >> 14) & 0xF;
            card->csd_ver2.write_bl_partial = (card->csd_lo >> 13) & 0x1;
            card->csd_ver2.reserved5 = (card->csd_lo >> 8) & 0x1F;
            card->csd_ver2.file_format_grp = (card->csd_lo >> 7) & 0x1;
            card->csd_ver2.copy = (card->csd_lo >> 6) & 0x1;
            card->csd_ver2.perm_write_protect = (card->csd_lo >> 5) & 0x1;
            card->csd_ver2.tmp_write_protect = (card->csd_lo >> 4) & 0x1;
            card->csd_ver2.file_format = (card->csd_lo >> 2) & 0x3;
            card->csd_ver2.reserver6 = (card->csd_lo) & 0x3;
        }
    } else if (card->card_type == MEMORY_CARD_TYPE_MMC) {
        card->csd_mmc.csd_structure = (card->csd_hi >> 54) & 0x3;
        card->csd_mmc.spec_vers = (card->csd_hi >> 50) & 0xF;
        card->csd_mmc.reserved1 = (card->csd_hi >> 48) & 0x3;
        card->csd_mmc.taac = (card->csd_hi >> 40) & 0xFF;
        card->csd_mmc.nsac = (card->csd_hi >> 32) & 0xFF;
        card->csd_mmc.tran_speed = (card->csd_hi >> 24) & 0xFF;
        card->csd_mmc.ccc = (card->csd_hi >> 12) & 0xFFF;
        card->csd_mmc.read_bl_len = (card->csd_hi >> 8) & 0xF;
        card->csd_mmc.read_bl_partial = (card->csd_hi >> 7) & 0x1;
        card->csd_mmc.write_blk_misalign = (card->csd_hi >> 6) & 0x1;
        card->csd_mmc.read_blk_misalign = (card->csd_hi >> 5) & 0x1;
        card->csd_mmc.dsr_imp = (card->csd_hi >> 4) & 0x1;
        card->csd_mmc.reserved2 = (card->csd_hi >> 2) & 0x3;
        card->csd_mmc.c_size = (((card->csd_hi & 0x3) << 10) | (card->csd_lo >> 54) & 0x3FF);
        card->csd_mmc.vdd_r_curr_min = (card->csd_lo >> 51) & 0x7;
        card->csd_mmc.vdd_r_curr_max = (card->csd_lo >> 48) & 0x7;
        card->csd_mmc.vdd_w_curr_min = (card->csd_lo >> 45) & 0x7;
        card->csd_mmc.vdd_w_curr_max = (card->csd_lo >> 42) & 0x7;
        card->csd_mmc.c_size_mult = (card->csd_lo >> 39) & 0x7;
        card->csd_mmc.erase_grp_size = (card->csd_lo >> 34) & 0x1F;
        card->csd_mmc.erase_grp_mult = (card->csd_lo >> 29) & 0x1F;
        card->csd_mmc.wp_grp_size = (card->csd_lo >> 24) & 0x1F;
        card->csd_mmc.wp_grp_enable = (card->csd_lo >> 23) & 0x1;
        card->csd_mmc.default_ecc = (card->csd_lo >> 21) & 0x3;
        card->csd_mmc.r2w_factor = (card->csd_lo >> 18) & 0x7;
        card->csd_mmc.write_bl_len = (card->csd_lo >> 14) & 0xF;
        card->csd_mmc.write_bl_partial = (card->csd_lo >> 13) & 0x1;
        card->csd_mmc.reserved3 = (card->csd_lo >> 9) & 0xF;
        card->csd_mmc.content_prot_app = (card->csd_lo >> 8) & 0x1;
        card->csd_mmc.file_format_grp = (card->csd_lo >> 7) & 0x1;
        card->csd_mmc.COPY = (card->csd_lo >> 6) & 0x1;
        card->csd_mmc.perm_write_protect = (card->csd_lo >> 5) & 0x1;
        card->csd_mmc.tmp_write_protect = (card->csd_lo >> 4) & 0x1;
        card->csd_mmc.file_format = (card->csd_lo >> 2) & 0x3;
        card->csd_mmc.ecc = (card->csd_lo) & 0x3;
    }
    return err;
}

/**
 * @brief kdrv_sdc_ops_send_csd() send select card command(CMD7)
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_select_card(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    /* send CMD7 to enter transfer mode */
    return kdrv_sdc_send_command(dev, SDHCI_CMD7_SELECT_CARD, SDHCI_CMD_TYPE_NORMAL, 0,
                                 SDHCI_CMD_RTYPE_R1R5R6R7, 0, card_info->rca << 16);
}

/**
 * @brief kdrv_sdc_ops_app_send_scr() send scr request command
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_app_send_scr(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    // Reading the scr through PIO
    /* CMD 55 */
    kdrv_sdc_send_command(dev, SDHCI_CMD55_APP, SDHCI_CMD_TYPE_NORMAL, 0,
                          SDHCI_CMD_RTYPE_R1R5R6R7, 1, (card->rca << 16));

    kdrv_sdc_set_transfer_mode(dev, 0, 0, SDHCI_TXMODE_READ_DIRECTION, 0);
    kdrv_sdc_prepare_data(dev, 1, SCR_LENGTH, (uint32_t) & (card->scr), READ);

    /* ACMD 51 */
    kdrv_sdc_send_command(dev, SDHCI_CMD51_SEND_SCR, SDHCI_CMD_TYPE_NORMAL, 1,
                          SDHCI_CMD_RTYPE_R1R5R6R7, 1, 0);

    return (kdrv_sdc_transfer_data(dev, READ, (uint32_t *) & (card->scr), SCR_LENGTH));
}



/**
 * @brief kdrv_sdc_ops_app_set_bus_width() send bus width set command(ACM6)
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_app_set_bus_width(kdrv_sdc_res_t *dev, uint32_t width)
{
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    /* CMD 55: Indicate to the card the next cmd is app-specific command */
    err = kdrv_sdc_send_command(dev, SDHCI_CMD55_APP, SDHCI_CMD_TYPE_NORMAL,
                                0, SDHCI_CMD_RTYPE_R1R5R6R7, 0,(card->rca << 16));
    if (err)
        return err;

    err = kdrv_sdc_send_command(dev, SDHCI_CMD6_SET_BUS_WIDTH, SDHCI_CMD_TYPE_NORMAL,
                                0, SDHCI_CMD_RTYPE_R1R5R6R7, 0, width);
    if (err)
        return err;

    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_ops_sd_switch() send switch command(CMD6)
 *              Switch Function returns 64 bytes status data. Caller must
 *              make sure "resp" pointer has allocated enough space.
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_sd_switch(kdrv_sdc_res_t *dev, uint32_t mode, uint32_t group,
        uint8_t value, volatile uint8_t * resp)
{
    uint32_t arg;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    if (card->card_type != MEMORY_CARD_TYPE_SD) {
        kmdw_dbg_printf("Switch Function: This is not SD Card !\n");
        return KDRV_STATUS_SDC_CARD_TYPE_ERR;
    }

    kdrv_sdc_set_transfer_mode(dev, 0, 0, SDHCI_TXMODE_READ_DIRECTION, 0);
    kdrv_sdc_prepare_data(dev, 1, 64, (uint32_t) resp, READ);

    //Check Function
    arg = mode << 31 | 0x00FFFFFF;
    arg &= ~(0xF << (group * 4));
    arg |= value << (group * 4);

    // Commented by MikeYeh 081201: The 31st bit of argument for CMD6 is zero to indicate "Check Function".
    // Check function used to query if the card supported a specific function.
    /* CMD 6 */
    if (kdrv_sdc_send_command(dev, SDHCI_CMD6_SWITCH_FUNC, SDHCI_CMD_TYPE_NORMAL,
                              1, SDHCI_CMD_RTYPE_R1R5R6R7, 1, arg)) {
        //kmdw_dbg_printf("CMD6 Failed.\n");
        return KDRV_STATUS_SDC_SWITCH_ERR;
    }

    return kdrv_sdc_transfer_data(dev, READ, (uint32_t *) resp, 64);
}


/**
 * @brief kdrv_sdc_ops_send_tune_block() send tunning command(CMD19) to device
 *              64bytes tunning pattern for SDR50/SDR104
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_tune_block(kdrv_sdc_res_t *dev)
{
    kdrv_status_t err;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;

    if (!(sdc_reg->host_ctl2 & SDHCI_EXECUTE_TUNING)) {
        return KDRV_STATUS_OK;
    }

    kdrv_sdc_set_transfer_mode(dev, 0, 0, SDHCI_TXMODE_READ_DIRECTION, 0);
    kdrv_sdc_prepare_data(dev, 1, 64, NULL, READ);

    if ((err = kdrv_sdc_send_command(dev, SDHCI_CMD19_SEND_TUNE_BLOCK, SDHCI_CMD_TYPE_NORMAL,
                                     1, SDHCI_CMD_NO_RESPONSE, 1, 0)) != KDRV_STATUS_OK)
        return err;

    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_ops_send_card_status() send status request command(CMD13)
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_ops_send_card_status(kdrv_sdc_res_t *dev)
{
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;

    if ((err = kdrv_sdc_send_command(dev, SDHCI_CMD13_SEND_STATUS, SDHCI_CMD_TYPE_NORMAL, 0,
                                     SDHCI_CMD_RTYPE_R1R5R6R7, 0, (card->rca << 16))) != KDRV_STATUS_OK)
        return err;

    return KDRV_STATUS_OK;
}
/**
 * @brief kdrv_sdc_set_base_clock(), set host base clock information
 *
 * @param[in]   host        host structure
 * @param[in]   clk         clock frequency
 * @return      N/A
 */
static void kdrv_sdc_set_base_clock(kdrv_sdc_sd_host_t *host,uint32_t clk)
{
    host->max_clk = clk * 1000000;
    host->min_clk = host->max_clk / 256;
}

/**
 * @brief kdrv_sdc_transfer_data() transfer R/W data to device
 *          Return the "residual" number of bytes write.
 *          Caller check for this for correctness.
 *          Do not use interrupt signal for PIO mode.
 *          The unit of length is bytes.
 * @param[in]   dev         device
 * @param[in]   act         read/write action
 * @param[in]   buffer      read/write buffer
 * @param[in]   length      read/write length bytes
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_transfer_data(kdrv_sdc_res_t *dev, kdrv_sdc_transfer_act_e act,
        uint32_t *buffer, uint32_t length)
{
    //kdrv_status_t rstatus;
    uint32_t trans_sz, len, wait_t;
    uint16_t mask;
    clock_t t0;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    if (length == 0) {
        return KDRV_STATUS_OK;
    }

    if (card_info->flow_set.use_dma == PIO) {
        uint32_t dsize;
        uint8_t *u8_buf;
        uint16_t *u16_buf;

        wait_t = dev->timeout_ms;
        dsize = card_info->flow_set.line_bound;
        switch (dsize) {
        case 1:
            u8_buf = (uint8_t *) buffer;
            break;
        case 2:
            u16_buf = (uint16_t *) buffer;
            break;
        default:
            break;
        }

        if (act == WRITE)
            mask = SDHCI_INTR_STS_BUFF_WRITE_READY;
        else
            mask = SDHCI_INTR_STS_BUFF_READ_READY;

        trans_sz = dev->fifo_depth;
        while (length || (dev->infinite_mode && (sdc_reg->txmode & SDHCI_TXMODE_MULTI_SEL))) {
            t0 = kdrv_current_t1_tick();
            while (!(sdc_reg->intr_sts & mask)) {
                if (kdrv_current_t1_tick() - t0 > wait_t) {
                    /* check R/W ready time out */
                    //rstatus = KDRV_STATUS_SDC_TX_TIEMOUT;
                    goto out;
                }
            }

            /* Clear Interrupt status */
            sdc_reg->intr_sts = mask;

            len = (length < trans_sz) ? length : trans_sz;
            length -= len;
            while (len) {
                switch (dsize) {
                case 1:
                    /* byte read/write */
                    if (act == WRITE) {
                        outb(KDRV_SDC_BASE + SDHCI_REG_DATA_PORT, *u8_buf);
                    } else {
                        *u8_buf = inb((KDRV_SDC_BASE + SDHCI_REG_DATA_PORT));
                    }
                    len -= 1;
                    u8_buf++;
                    break;
                case 2:
                    /* 16bits read/write */
                    if (act == WRITE) {
                        outhw(KDRV_SDC_BASE + SDHCI_REG_DATA_PORT, *u16_buf);
                    } else {
                        *u16_buf = inhw(KDRV_SDC_BASE + SDHCI_REG_DATA_PORT);
                    }
                    len -= 2;
                    u16_buf++;
                    break;
                default:
                    /* 32bits read/write */
                    if (act == WRITE) {
                        outw(KDRV_SDC_BASE + SDHCI_REG_DATA_PORT, *buffer);
                    } else {
                        *buffer = inw(KDRV_SDC_BASE + SDHCI_REG_DATA_PORT);
                    }
                    len -= 4;
                    buffer++;
                    break;
                }
            }
        }

        /* Wait for last block to be completed */
        if (dev->infinite_mode) {
            /* Actually no Transfer Complete for infite transfer,
             * But if we set Stop at Block Gap, then there will be
             * Transfer Complete.
             */
            if (act == WRITE) {
                card_info->cmpl_mask |= WAIT_BLOCK_GAP;
                sdc_reg->blk_gap_ctl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
            } else {
                card_info->cmpl_mask &= ~WAIT_TRANS_COMPLETE;
            }
        }

    } else if (card_info->flow_set.use_dma == SDMA) {
        uint32_t next_addr;
        trans_sz = sdma_bound_mask + 1;
        wait_t = dev->timeout_ms * (trans_sz >> 9);

        do {
            t0 = kdrv_current_t1_tick();
            /* Make sure SDMA finish before we lacth the next address */
            while (card_info->cmpl_mask) {
                if (card_info->err_sts)
                    goto out;
                if ((kdrv_current_t1_tick() - t0) > wait_t) {
                    /* try err recovery */
                    //rstatus = KDRV_STATUS_SDC_TX_TIEMOUT;
                    goto out;
                }
            }

            next_addr = sdc_reg->sdma_addr;
            /* Transfered bytes count */
            len = next_addr - (uint32_t) buffer;
            /* Minus the total desired bytes count. SDMA stops at boundary
             * but it might already exceed our intended bytes
             */
            if ((int32_t) (length - len) < 0)
                length = 0;
            else
                length -= len;

            if (!length)
                break;

            /* Boundary Checking */
            if (next_addr & sdma_bound_mask) {
                return KDRV_STATUS_SDC_TRANSFER_FAIL;
            }

            /* Remaining bytes less than SDMA boundary.
             * For finite transfer, Wait for transfer complete interrupt.
             * Infinite transfer, wait for DMA interrupt.
             */
            if ((length > trans_sz) || ((length <= trans_sz) && dev->infinite_mode))
                card_info->cmpl_mask = WAIT_DMA_INTR;
            else {
                card_info->cmpl_mask = WAIT_TRANS_COMPLETE;
            }
            buffer = (uint32_t*)next_addr;
            sdc_reg->sdma_addr = next_addr;

        } while (1);

        /* Wait for last block to be completed */
        if (dev->infinite_mode) {
            /* Actually no Transfer Complete for infite transfer,
             * But if we set Stop at Block Gap, then there will be
             * Transfer Complete.
             */
            if (act == WRITE) {
                card_info->cmpl_mask |= (WAIT_BLOCK_GAP | WAIT_TRANS_COMPLETE);
                sdc_reg->blk_gap_ctl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
            }
        }
    } else {
        wait_t = dev->timeout_ms * 10;
        length = 0;
    }

    /* Only need to wait transfer complete for DMA */
    t0 = kdrv_current_t1_tick();
    while (card_info->cmpl_mask && !card_info->err_sts) {
        if ((kdrv_current_t1_tick() - t0) > wait_t) {
            /* time out */
            if (card_info->cmpl_mask)
                length = card_info->cmpl_mask;
            else if (card_info->err_sts)
                length = card_info->err_sts;
            goto out;
        }
    }

out:
    if (card_info->err_sts) {
        if (card_info->err_sts & SDHCI_INTR_ERR_AUTOCMD)
            kdrv_sdc_auto_cmd_error_recovery(dev);
        else
            kdrv_sdc_error_recovery(dev);
        if(card_info->err_sts) {
            return KDRV_STATUS_SDC_RECOVERABLE_ERR;
        }
    }
    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_set_bus_width() set sdcard bus width
 *
 * @param[in]   dev         device structure
 * @param[in]   width       1: 1bit, 4:4bits, 8:8bits
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_set_bus_width(kdrv_sdc_res_t *dev, uint8_t width)
{
    uint8_t wdth;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    /* Setting the Bus width */
    if (card_info->card_type == MEMORY_CARD_TYPE_MMC) {
        if (card_info->csd_mmc.spec_vers < 4) {
            //kmdw_dbg_printf(" MMC: Change width not allowed for version less than MMC4.1.\n");
            return KDRV_STATUS_OK;
        }
        kmdw_dbg_printf("kdrv_sdc_set_bus_width=%d\n",width);
        /**
         * MMC bus_width[183] of Extended CSD.
         * 0=1 bit, 1=4bits, 2=8bits.
         */
        switch (width) {
        case 1:
            wdth = 0;
            break;
        case 4:
            wdth = 1;
            break;
        case 8:
            wdth = 2;
            break;
        default:
            //kmdw_dbg_printf(" ERR## ... MMC: Not supported bus witdth %d.\n", width);
            return KDRV_STATUS_SDC_BUS_WIDTH_NOT_SUPPORT;
        }

        if (kdrv_sdc_ops_mmc_switch(dev, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH, wdth)) {
            //kmdw_dbg_printf(" ERR## ... MMC: Set Bus width failed.\n");
            return KDRV_STATUS_SDC_BUS_WIDTH_NOT_SUPPORT;
        }
    } else if (card_info->card_type == MEMORY_CARD_TYPE_SD) {
        /**
          * The Bit2 in scr shows whether the "4bit bus width(DAT0-3)" is supported.
          * ACMD6 command, '00'=1bit or '10'=4bit bus.
          */
        switch (width) {
        case 1:
            if (!(card_info->scr.sd_bus_widths & SDHCI_SCR_SUPPORT_1BIT_BUS)) {
                //kmdw_dbg_printf(" ERR## ... SD: 1 bit width not support by this card.\n");
                return KDRV_STATUS_SDC_BUS_WIDTH_NOT_SUPPORT;
            }
            wdth = SDHCI_1BIT_BUS_WIDTH;
            break;
        case 4:
            if (!(card_info->scr.sd_bus_widths & SDHCI_SCR_SUPPORT_4BIT_BUS)) {
                //kmdw_dbg_printf(" ERR## ... SD: 4 bit width not support by this card.\n");
                return KDRV_STATUS_SDC_BUS_WIDTH_NOT_SUPPORT;
            }
            wdth = SDHCI_4BIT_BUS_WIDTH;
            break;
        default:
            //kmdw_dbg_printf(" ERR## ... SD: Not supported bus witdth %d.\n", width);
            return KDRV_STATUS_SDC_BUS_WIDTH_NOT_SUPPORT;
        }

        if (kdrv_sdc_ops_app_set_bus_width(dev, wdth)) {
            //kmdw_dbg_printf(" ERR## ... SD: Set Bus Width %d failed !\n", width);
            return KDRV_STATUS_SDC_BUS_WIDTH_ERR;
        }
    }

    sdc_reg->hcreg &= ~(SDHCI_HC_BUS_WIDTH_8BIT | SDHCI_HC_BUS_WIDTH_4BIT);
    switch (width) {
    case 1:
        card_info->bus_width = 1;
        break;
    case 4:
        sdc_reg->hcreg |= SDHCI_HC_BUS_WIDTH_4BIT;
        card_info->bus_width = 4;
        break;
    case 8:
        sdc_reg->hcreg |= SDHCI_HC_BUS_WIDTH_8BIT;
        card_info->bus_width = 8;
        break;
    default:
        kmdw_dbg_printf(" Unsupport bus width %d for HW register setting.\n", width);
        break;
    }
    kmdw_dbg_printf(" Set Bus width %d bit(s), Host Control Register: 0x%x.\n", width, sdc_reg->hcreg);
    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_set_transfer_mode() set transfer mode
 */
static kdrv_status_t kdrv_sdc_set_transfer_mode(kdrv_sdc_res_t *dev, uint8_t blk_cnt_en,
        uint8_t auto_cmd, uint8_t dir, uint8_t multi_blk)
{
    uint16_t mode;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;

    if ((dir != SDHCI_TXMODE_READ_DIRECTION) && (dir != 0)) {
        //kmdw_dbg_printf(" ERR## ... Transder Mode, direction value not correct.\n");
        return KDRV_STATUS_SDC_TRANSFER_FAIL;
    }

    /*SDMA can not use ACMD23 */
    if ((card_info->flow_set.use_dma == SDMA) && (auto_cmd == 2))
        auto_cmd = 1;

    auto_cmd <<= 2;
    if ((auto_cmd != SDHCI_TXMODE_AUTOCMD12_EN) && (auto_cmd != SDHCI_TXMODE_AUTOCMD23_EN)
        && (auto_cmd != 0)) {
        //kmdw_dbg_printf(" ERR## ... Transder Mode, auto cmd value not correct.\n");
        return KDRV_STATUS_SDC_TRANSFER_FAIL;
    }

    mode = (auto_cmd | dir);

    if ((card_info->flow_set.use_dma == ADMA) || (card_info->flow_set.use_dma == SDMA))
        mode |= SDHCI_TXMODE_DMA_EN;

    if (blk_cnt_en)
        mode |= SDHCI_TXMODE_BLKCNT_EN;
    if (multi_blk)
        mode |= SDHCI_TXMODE_MULTI_SEL;
    sdc_reg->txmode = mode;

    return KDRV_STATUS_OK;
}




/**
 * @brief kdrv_sdc_set_bus_speed_mode() set bus speed
 *
 * @param[in] dev device structure
 * @param[in] speed bus speed
 *      0x0 : Default/SDR12/Normal Speed
 *      0x1 : SDR25/High Speed
 *      0x2 : SDR50
 *      0x3 : SDR104
 *      0x4 : DDR50
 * @return kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_set_bus_speed_mode(kdrv_sdc_res_t *dev, uint8_t speed)
{
    uint32_t err, i;
    kdrv_status_t rstatus;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    kdrv_sdc_sd_host_t *host = dev->host;
    if (speed > 4) {
        kmdw_dbg_printf(" ERR## ... Bus Speed Mode value %d error.\n", speed);
        return KDRV_STATUS_SDC_SPEED_MOD_ERR;
    }

    if (card_info->card_type == MEMORY_CARD_TYPE_SD) {
        if (card_info->scr.sd_spec == 0) {
            /* Version 1.01 Card does not support CMD6 */
            speed = 0;
            goto out;
        }

        /* Check Group 1 function support */
        //kmdw_dbg_printf(" Checking function Group 1 (Bus Speed Mode) ...");
        for (i = 0; i < 5; i++) {
            if ((rstatus = kdrv_sdc_ops_sd_switch(dev, 0, 0, i, &card_info->switch_sts[0])) != KDRV_STATUS_OK) {
                kmdw_dbg_printf(" ERR## ... Problem reading Switch function(Bus Speed Mode).\n");
                return rstatus;
            }

            if (card_info->switch_sts[13] & (1 << i)) {
                card_info->bs_mode |= (1 << i);
            }
        }


        if (!(sdc_reg->host_ctl2 & SDHCI_18V_SIGNAL) && (speed > 1)) {
            kmdw_dbg_printf(" No 1.8V Signaling Can not set speed more than 1");
            return KDRV_STATUS_ERROR;
        }

        /* Check function support */
        if (!(card_info->bs_mode & (1 << speed))) {
            //kmdw_dbg_printf(" ERR## ... %s not support by Card.\n", SDC_ShowTransferSpeed(speed));
            return KDRV_STATUS_SDC_VOL_ERR;
        }

        //kdrv_sdc_set_sd_clock(25 * 1000000);

        /* Read it back for confirmation */
        err = kdrv_sdc_ops_sd_switch(dev, 1, 0, speed, &card_info->switch_sts[0]);
        if (err || ((card_info->switch_sts[16] & 0xF) != speed)) {
            kmdw_dbg_printf(" ERR## ... Problem switching(Group 1) card into %d mode!\n", speed);
            return KDRV_STATUS_ERROR;
        }
    } else if (card_info->card_type == MEMORY_CARD_TYPE_MMC) {
        if (card_info->csd_mmc.spec_vers < 4) {
            speed = 0;
            kmdw_dbg_printf(" MMC: Change speed not allowed for version less than MMC4.1.\n");
            goto out;
        }
        /*
         * MMC Card only has Default and Hight speed.
         */
        if (speed > 1) {
            kmdw_dbg_printf(" ERR## ... Bus Speed Mode value %d error.\n", speed);
        }

        if (kdrv_sdc_ops_mmc_switch(dev, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, speed)) {
            kmdw_dbg_printf(" ERR## ... MMC: Problem switching card into %d mode!\n", speed);
            return KDRV_STATUS_ERROR;
        }

        // Insure that the state has returned to "Transfer State" according to P.43 in MMC spec.
        if (kdrv_sdc_wait_for_state(dev, CUR_STATE_TRAN, 2000))
            return KDRV_STATUS_ERROR;
    }

out:
    /* SDR mode only valid for 1.8v IO signal */
    if (sdc_reg->host_ctl2 & SDHCI_18V_SIGNAL) {
        sdc_reg->host_ctl2 &= ~0x7;
        sdc_reg->host_ctl2 |= speed;

    } else {
        if (speed == 0)
            sdc_reg->hcreg &= ~(uint8_t) 0x4;
        else if (speed == 1)
            sdc_reg->hcreg |= 0x4;
        else {
            kmdw_dbg_printf(" No 1.8V Signaling Can not set speed more than 1.\n");
            return KDRV_STATUS_ERROR;
        }
    }
    card_info->speed = (kdrv_sdc_bus_speed_e) speed;

    if (speed < 4) {
        if (speed < 3) {
            card_info->max_dtr = 25000000 << speed;
        } else { /* current controller stable @ max 130 MHz */
            card_info->max_dtr = host->max_clk;
        }
    } else
        card_info->max_dtr = 50000000;

    kmdw_dbg_printf("Set Switch function(Bus Speed Mode = %d), Host Control Register: 0x%x, 0x%04x.\n",
                    speed, sdc_reg->hcreg, sdc_reg->host_ctl2);

    return KDRV_STATUS_OK;
}



/**
 * @brief kdrv_sdc_execute_tuning() tuning for clock setting
 */
static kdrv_status_t kdrv_sdc_execute_tuning(kdrv_sdc_res_t *dev, uint32_t try)
{
    kdrv_sdc_transfer_type_e dma;
    uint8_t delay, cnt, hndshk;
    clock_t t0;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;


    if (!((sdc_reg->host_ctl2 & SDHCI_SDR104) && (sdc_reg->host_ctl2 & SDHCI_18V_SIGNAL))) {
        kmdw_dbg_printf(" ERR## ... Tuning only require for SDR104 mode.\n");
        return KDRV_STATUS_ERROR;
    }

    /* Must tune 4 data lines */
    if (card_info->bus_width != 4)
        kdrv_sdc_set_bus_width(dev,4);

    /* Prevent using DMA when do tuning */
    dma = card_info->flow_set.use_dma;
    card_info->flow_set.use_dma = PIO;
    hndshk = sdc_reg->dma_hndshk & 1;
    sdc_reg->dma_hndshk = 0;

    cnt = 0;
retune:
    delay = 0;
    sdc_reg->vendor_reg3 &= ~((uint32_t)0xff << 24);
    sdc_reg->vendor_reg3 = 0x00000804 | (try << 24);

    sdc_reg->softrst |= SDHCI_SOFTRST_DAT;
    /* Set Execute Tuning at Host Control2 Register */
    sdc_reg->host_ctl2 |= SDHCI_EXECUTE_TUNING;

    /* Issue CMD19 repeatedly until Execute Tuning is zero.
     * Abort the loop if reached 40 times or 150 ms
     */
    do {
        if (kdrv_sdc_ops_send_tune_block(dev))
            break;
        /* Wait until Buffer Read Ready */
        t0 = kdrv_current_t1_tick();
        do {
            if ((kdrv_current_t1_tick() - t0 > 100)) {
                kmdw_dbg_printf(" ERR##: Tuning wait for BUFFER READ READY timeout.\n");
                //sdc_reg->softrst |= (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT);
                //while (sdc_reg->softrst & (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT));
                break;
            }
        } while (!(sdc_reg->intr_sts & SDHCI_INTR_STS_BUFF_READ_READY));

        sdc_reg->intr_sts &= SDHCI_INTR_STS_BUFF_READ_READY;

    } while (sdc_reg->host_ctl2 & SDHCI_EXECUTE_TUNING);

    /* Check Sampling Clock Select */
    if (sdc_reg->host_ctl2 & SDHCI_SMPL_CLCK_SELECT) {
        /* FPGA only */
        delay = (sdc_reg->vendor_reg3 >> 16) & 0x1F;
        kmdw_dbg_printf("Tuning Complete(0x%08x), SD Delay : 0x%x.\n", sdc_reg->vendor_reg4, delay);

        if (!(sdc_reg->vendor_reg4 & (1 << delay))) {
            kmdw_dbg_printf("SD Delay does not match tuning record(0x%08x).\n", sdc_reg->vendor_reg4);

            if (cnt++ < 3) {
                goto retune;
            } else {
                kmdw_dbg_printf("Re-Tuning Failed.\n");
                card_info->flow_set.use_dma = dma;
                return KDRV_STATUS_ERROR;
            }
        }
    } else {
        kmdw_dbg_printf("Tuning Failed.\n");
        card_info->flow_set.use_dma = dma;
        return KDRV_STATUS_ERROR;
    }

    card_info->flow_set.use_dma = dma;
    sdc_reg->dma_hndshk |= hndshk;

    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_set_sd_clock() set sdc clcok
 *
 * @param[in]   dev device structure
 * @param[in]   clock
 * @return      N/A
 */
static void kdrv_sdc_set_sd_clock(kdrv_sdc_res_t *dev, uint32_t clock)
{
    int div, timeout, s_clk;
    uint16_t clk;
    uint32_t try;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    kdrv_sdc_sd_host_t *host = dev->host;

    sdc_reg->clk_ctl = 0;

    if (card_info->max_dtr < clock)
        kmdw_dbg_printf(" WARN## ... Set clock(%d Hz) larger than Max Rate(%d Hz).\n", clock, card_info->max_dtr);

    if (clock == 0)
        goto out;

    if (host->max_clk <= clock)
        div =0;
    else {
        for (div = 1; div < 0x3FF; div++) {
            if ((host->max_clk / (2 * div)) <= clock)
                break;
        }
    }
    //div=3;
    kmdw_dbg_printf(" SDLK Freq select, Div: %d, Clock: (%d/%d) Hz, ", div, (host->max_clk / div), clock);

    clk = div << SDHCI_CLK_CTRL_LOW_CLK_SEL_SHIFT;
    clk |= SDHCI_CLK_CTRL_INTERNALCLK_EN;
    kmdw_dbg_printf("sdc_reg->clk_ctl: 0x%x\n", sdc_reg->clk_ctl);
    sdc_reg->clk_ctl = clk;

    /* Wait max 10 ms */
    timeout = 10;
    while (!(sdc_reg->clk_ctl & SDHCI_CLK_CTRL_INTERNALCLK_STABLE)) {
        if (timeout == 0) {
            //fail(" ERR## ... Internal clock never estabilised.\n");
            kmdw_dbg_printf(" ERR## ... Internal clock never estabilised.\n");
            break;
        }
        timeout--;
        kdrv_delay_us(1000);
    }

    clk |= SDHCI_CLK_CTRL_SDCLK_EN;
    sdc_reg->clk_ctl = clk;
    kmdw_dbg_printf("div=0x%x, clk=0x%x\n",div, clk);
    /* Make sure to use pulse latching when run below 50 MHz */
    /* SDR104 use multiphase DLL latching, others use pulse latching */
    if (!div)
        div = 1;
    else
        div *= 2;

    s_clk = host->max_clk / div;
    if (s_clk < 100000000) {
        sdc_reg->vendor_reg0 |= 1;
        kmdw_dbg_printf("sdc_reg->vendor_reg0=0x%x\n",sdc_reg->vendor_reg0);
        /* Adjust the Pulse Latch Offset if div > 0 */
        if (div > 1)
            sdc_reg->vendor_reg0  |= 0x200; //legend: pulse-latching
    } else { /* Sampling tuning for SDR104 and SDR50 if required */

        sdc_reg->vendor_reg0 &= ~1;

        if (s_clk == 100000000)
            try = 16;
        else {
            s_clk /= 1000000;
            try = ((8 * s_clk) / 25) - 16;
        }
        kdrv_sdc_execute_tuning(dev, try);
    }

out:
    host->clock = clock;
    kmdw_dbg_printf(" Clock Control Register: 0x%04x.\n", sdc_reg->clk_ctl);
}


/**
 * @brief kdrv_sdc_set_power() set power voltage
 * @param[in]   dev device structure
 * @param[in]   power   power level bit poistion
 *              OCR power bit position
 *                   7: low voltage
 *                  17: 2.9~3.0
 *                  18: 3.0~3.1
 *                  20: 3.2~3.3
 *                  21: 3.3~3.4
 * @return kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_set_power(kdrv_sdc_res_t *dev, int16_t power)
{
    uint8_t pwr;
    kdrv_status_t err = KDRV_STATUS_OK;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    kdrv_sdc_sd_host_t *host = dev->host;

    if (power == (int16_t) - 1) {
        sdc_reg->pwr_ctl = 0;
        err = KDRV_STATUS_SDC_PWR_SET_ERR;
        goto out;
    }

    /*
     * Spec says that we should clear the power reg before setting
     * a new value.
     */
    sdc_reg->pwr_ctl = 0;

    pwr = SDHCI_POWER_ON;
    switch (power) {
    case 7:
        pwr |= SDHCI_POWER_180;
        break;
    case 17:
    case 18:
        pwr |= SDHCI_POWER_300;
        break;
    case 20:
    case 21:
        pwr |= SDHCI_POWER_330;
        break;
    default:
        err = KDRV_STATUS_SDC_PWR_SET_ERR;
        kmdw_dbg_printf(" ERR## ... Voltage value not correct.\n");
    }

    sdc_reg->pwr_ctl = pwr;

out:
    host->power = power;
    kmdw_dbg_printf(" Power Control Register: 0x%01x.\n", sdc_reg->pwr_ctl);
    return err;
}


/**
 * @brief kdrv_sdc_check_card_insert() check card insert
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 *              kl520 do not havw cd/wp pin, so it will always return pin inserted.
 */
static kdrv_status_t kdrv_sdc_check_card_insert(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    /*
     * kl520 does not have CD(card detect) and WP(card protect pin), so it will
     * always return inserted status
     */
    card_info->card_insert = 1;
    return KDRV_STATUS_OK;
}

/**
 * SDMA: line_bound is Buffer Boundary value.
 * ADMA: line_boud is bytes per descriptor line.
 */
static void kdrv_sdc_set_transfer_type(kdrv_sdc_res_t *dev, kdrv_sdc_transfer_type_e type, uint32_t line_bound, uint8_t ran_mode)
{
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    sdc_reg->hcreg &= ~(uint8_t) (0x3 << 3);
    sdc_reg->dma_hndshk &= ~1;

    card_info->flow_set.use_dma = type;
    card_info->flow_set.line_bound = line_bound;
    card_info->flow_set.adma2rand = ran_mode;

    if (type == PIO) {
        sdc_reg->intr_sig_en = SDHCI_INTR_SIGN_EN_PIO;

    } else if (type == SDMA) {
        sdma_bound_mask = (1 << (card_info->flow_set.line_bound + 12)) - 1;
        sdc_reg->intr_sig_en = SDHCI_INTR_SIGN_EN_SDMA;

    } else if (type == ADMA) {
        sdc_reg->adma_addr = (uint64_t)&sdc_adma2_desc_table;//ADMA2_DESC_TABLE;
        sdc_reg->hcreg |= (uint8_t) SDHCI_HC_USE_ADMA2;
        sdc_reg->intr_sig_en = SDHCI_INTR_SIGN_EN_ADMA;

    } else {
        sdc_reg->dma_hndshk |= 1;

    }

    sdc_reg->err_sig_en = SDHCI_ERR_SIG_EN_ALL;
}

static kdrv_status_t kdrv_sdc_fill_adma_desc_table(kdrv_sdc_res_t *dev,
        uint32_t total_data, uint8_t *data_addr)
{
    uint8_t act, tmp, adma2_random;
    uint32_t byte_cnt, i, ran, bytes_per_line;
    uint8_t *buff;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    kdrv_sdc_adma2desc_table_t *ptr = (kdrv_sdc_adma2desc_table_t *) &sdc_adma2_desc_table;//ADMA2_DESC_TABLE;
    kdrv_sdc_adma2desc_table_t *tptr;

    if (total_data < 4) {
        kmdw_dbg_printf("Data less than 4 bytes !!\n");
        return KDRV_STATUS_ERROR;
    }

    adma2_random = card_info->flow_set.adma2rand;
    if (!card_info->flow_set.line_bound)
        bytes_per_line = 65536;
    else
        bytes_per_line = card_info->flow_set.line_bound;

    if (!adma2_random) {
        act = ADMA2_TRAN;
    }

    buff = (uint8_t *) data_addr;
    i = 0;
    do {
        /* Random Mode = 1, we only random the length inside the descriptor */
        /* Random Mode = 2, we random the action and fix length inside the descriptor */
        /* Random Mode = 3, we random the action and length inside the descriptor */
        if ((adma2_random > 1) && (i < (ADMA2_NUM_OF_LINES - 2))) {
            ran = rand();
            /* Occupy percentage to prevent too many Noop and reserved */
            tmp = ran & 0xF;
            if (tmp < 8)
                act = ADMA2_TRAN;
            else if (tmp < 13)
                act = ADMA2_LINK;
            else
                act = ADMA2_NOP;
        } else {
            act = ADMA2_TRAN;
        }

        tptr = ptr + i;
        memset(tptr, 0, sizeof(kdrv_sdc_adma2desc_table_t));

        switch (act) {
        case ADMA2_TRAN:
            if ((total_data > 256) && (i < (ADMA2_NUM_OF_LINES - 2))) {
                if (!adma2_random || (adma2_random == 2))
                    /* Must be 4 bytes alignment */
                    if (total_data < bytes_per_line)
                        byte_cnt = total_data;
                    else
                        byte_cnt = bytes_per_line;
                else
                    byte_cnt = (ran % total_data) & 0xfffc;
            } else {
                if (total_data > 0xFFFF) {
                    kmdw_dbg_printf(" ERR## ... Not enough descriptor to fill.\n");
                    tptr->attr |= ADMA2_ENTRY_END;
                    return KDRV_STATUS_ERROR;
                }
                byte_cnt = total_data;
            }

            if (byte_cnt < 4)   //bad result from randGen()
                byte_cnt = 4;

            tptr->addr = (uint32_t) buff;//CVS_working_folder
            tptr->attr = ADMA2_TRAN | ADMA2_ENTRY_VALID;
            tptr->lgth = byte_cnt;

            buff += byte_cnt;
            total_data -= byte_cnt;
            i++;

            break;
        case ADMA2_LINK:
            tmp = ran & 0x7;
            i += tmp;

            if (i > (ADMA2_NUM_OF_LINES - 2))
                i = ADMA2_NUM_OF_LINES - 2;

            tptr->addr = (uint32_t) ptr + i;//CVS_working_folder
            tptr->attr = ADMA2_LINK | ADMA2_ENTRY_VALID;

            break;
        /* Do not execute this line, go to next line */
        case ADMA2_NOP:
        case ADMA2_SET:
        default:
            tptr->attr = ADMA2_NOP | ADMA2_ENTRY_VALID;
            i++;

            break;
        }
    } while (total_data > 0);

    if (dev->adma2_insert_nop) {
        tptr = ptr + i;
        memset(tptr, 0, sizeof(kdrv_sdc_adma2desc_table_t));
    }
    tptr->attr |= (ADMA2_ENTRY_VALID | ADMA2_ENTRY_END | dev->adma2_use_interrupt);

    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_send_abort_command() send abort command
 */
static kdrv_status_t kdrv_sdc_send_abort_command(kdrv_sdc_res_t *dev)
{
    uint16_t val;
    clock_t t0;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    card_info->err_sts = 0;
    card_info->cmpl_mask = (WAIT_CMD_COMPLETE | WAIT_TRANS_COMPLETE);

    sdc_reg->cmd_argu = 0;

    val = ((SDHCI_CMD12_STOP_TRANS & 0x3f) << 8) | ((SDHCI_CMD_TYPE_ABORT & 0x3) << 6)
          | ((SDHCI_CMD_RTYPE_R1BR5B & 0x1F));
    sdc_reg->cmd_reg = val;

    t0 = kdrv_current_t1_tick();
    while (card_info->cmpl_mask) {
        /* 1 secs */
        if ((kdrv_current_t1_tick() - t0) > (dev->timeout_ms << 1)) {
            return KDRV_STATUS_SDC_TIMEOUT;
        }
    }

    // Reset the Data and Cmd line due to the None-auto CMD12.
    sdc_reg->softrst |= (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT);
    while (sdc_reg->softrst & (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT)) ;

    if (card_info->err_sts != 0) {
        return KDRV_STATUS_SDC_ABORT_ERR;
    }
    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_wait_for_state() wait state function
 */
kdrv_status_t kdrv_sdc_wait_for_state(kdrv_sdc_res_t *dev, uint32_t state, uint32_t ms)
{
    clock_t t0;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;


    t0 = kdrv_current_t1_tick();
    do {
        if (kdrv_sdc_ops_send_card_status(dev)) {
            kmdw_dbg_printf(" ERR## ... Send Card Status Failed.\n");
            return KDRV_STATUS_ERROR;
        }

        if ((kdrv_current_t1_tick() - t0) > ms) {
            kmdw_dbg_printf(" ERR## ... Card Status not return to define(%d) state.\n", state);
            return KDRV_STATUS_ERROR;
        }
        kdrv_delay_us(3000);
    } while (((card_info->resp_lo >> 9) & 0xF) != state);

    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_error_recovery(), try abort and recovery the sdc status
 *
 * @param[in] dev device structure
 * @return kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_error_recovery(kdrv_sdc_res_t *dev)
{
    uint8_t delayCount = 10;    // more than 40us, The max. freq. for SD is 50MHz.
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    //kdrv_sdc_print_err_msg(dev, card_info->err_sts);

    /* Step 8 and Step 9 to save previous error status and
     * clear error interrupt signal status. This are already
     * done at IRQ handler.
     */
    if (err_recover) {
        /* CMD12 */
        /* Step 10: Issue Abort CMD */
        kdrv_sdc_send_abort_command(dev);

        /* Step 11: Check Command Inhibit DAT and CMD */
        while (sdc_reg->present_state & (SDHCI_STS_CMD_INHIBIT | SDHCI_STS_CMD_DAT_INHIBIT)) ;

        /* Step 12 */
        if (card_info->err_sts & SDHCI_INTR_ERR_CMD_LINE) {
            //kmdw_dbg_printf("Non-recoverable Error:CMD Line Error\n");
            return KDRV_STATUS_SDC_RECOVERABLE_ERR;
        }
        /* Step 13 */
        if (card_info->err_sts & SDHCI_INTR_ERR_DATA_TIMEOUT) {
            //kmdw_dbg_printf("Non-recoverable Error:Data Line Timeout\n");
            return KDRV_STATUS_SDC_RECOVERABLE_ERR;
        }
        /* Step 14 */
        while (delayCount > 0) {
            delayCount--;
        }
        /* Step 15 */
        if (sdc_reg->present_state & SDHCI_STS_DAT_LINE_LEVEL) {
            /* Step 17 */
            //kmdw_dbg_printf("Recoverable Error\n");
            return KDRV_STATUS_SDC_RECOVERABLE_ERR;
        }
    }
    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_auto_cmd_error_recovery(), auto command recovery
 * SD card related operations
 */
static kdrv_status_t kdrv_sdc_auto_cmd_error_recovery(kdrv_sdc_res_t *dev)
{
    uint8_t pcmd;
    kdrv_status_t err;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    //kmdw_dbg_printf("Err: ACMD12 !!\n");

    /* Step 1: Check Auto CMD12 not executed status.
     * Host Controller cannot issue CMD12 to stop multiple
     * block transfer due to some error.
     */
    if (card_info->auto_err & SDHCI_AUTOCMD12_ERR_NOT_EXECUTED) {
        pcmd = 1;
        /* Step 2: Wait for Error Interrupt Recovery for CMD_wo_DAT.
         */
        err = kdrv_sdc_error_recovery(dev);
        if (err != KDRV_STATUS_SDC_RECOVERABLE_ERR)
            return err;
    } else {
        pcmd = 0;
        /* Step 6: Set Software Reset for CMD line (CR) */
        sdc_reg->softrst |= 0x2;
        /* Step 7: Check DR */
        while (sdc_reg->softrst & 0x2) ;
    }

    /* Step 4 & 8: Issue CMD12 */
    if (kdrv_sdc_send_abort_command(dev))
        return KDRV_STATUS_SDC_ABORT_ERR;

    if (kdrv_sdc_wait_for_state(dev,CUR_STATE_TRAN, 1000)) {
        //kmdw_dbg_printf("ERR## ... Get Card Status failed(ACMD12 Error Recovery) !\n");
        return KDRV_STATUS_SDC_RECOVERABLE_ERR;
    }

    /* Step 5 & 9: Check the result of CMD12 */
    if ((card_info->err_sts & 0xF) != 0) {
        /* Step 16 */
        //kmdw_dbg_printf("non-recoverable Error\n");
        return KDRV_STATUS_SDC_RECOVERABLE_ERR;
    }

    if (pcmd == 1 && !(card_info->err_sts & 0x10)) {
        /* Step 17 */
        //kmdw_dbg_printf("Error has occured in both CMD_wo_DAT, but not in the SD memory transfer.\n");
    } else {
        /* Step 11 & 14 */
        sdc_reg->softrst |= 0x4;
        /* Step 12 & 15 */
        while (sdc_reg->softrst & 0x4) ;

        /* Step 10: CMD12 not issued */
        if (!(card_info->auto_err & SDHCI_AUTOCMD12_ERR_CMD_NOT_ISSUE)) {
            if (pcmd) {
                /* Step 18 */
                //kmdw_dbg_printf("Error has occured in CMD_wo_DAT, and also in the SD memory transfer.\n");
            } else {
                /* Step 19 */
                //kmdw_dbg_printf("Error did not occur in CMD_wo_DAT, but in the SD memory transfer.\n");
            }
        } else {
            /* Step 20 */
            // kmdw_dbg_printf
            // ("CMD_wo_DAT has not been issued, and an error errored in the SD memory transfer.\n");
        }
    }
    return KDRV_STATUS_SDC_RECOVERABLE_ERR;
}


/**
 * @brief kdrv_sdc_read_scr() read scr information
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_read_scr(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    if (card_info->card_type != MEMORY_CARD_TYPE_SD)
        return KDRV_STATUS_SDC_READ_FAIL;

    memset((void *)&card_info->scr, 0, sizeof(kdrv_sdc_sd_scr_t));
    if (kdrv_sdc_ops_app_send_scr(dev)) {
        return KDRV_STATUS_SDC_READ_FAIL;
    }
    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_read_ext_csd read ext_csd information
 *
 * @param[in]   dev         structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_read_ext_csd(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    if (card_info->card_type != MEMORY_CARD_TYPE_MMC)
        return KDRV_STATUS_SDC_CARD_TYPE_ERR;

    if (card_info->card_type == MEMORY_CARD_TYPE_MMC && card_info->csd_mmc.spec_vers < 4) {
        kmdw_dbg_printf(" Commmand 8 is not supported in this MMC system spec.\n");
        goto fail;
    }

    if (kdrv_sdc_ops_send_ext_csd(dev)) {
        kmdw_dbg_printf(" ERR## ... MMC: Get EXT-CSD from card is failed\n");
        goto fail;
    }

    if (card_info->ext_csd_mmc.cardtype & 0xE)
        card_info->max_dtr = 52000000;
    else if (card_info->ext_csd_mmc.cardtype & 1)
        card_info->max_dtr = 26000000;
    else
        kmdw_dbg_printf(" WARN## ...  Unknown Max Speed at EXT-CSD.\n");

    if (card_info->card_type == MEMORY_CARD_TYPE_MMC) {
        card_info->num_of_boot_blks = (card_info->ext_csd_mmc.reserved6 << 8
                                       | card_info->ext_csd_mmc.boot_size_mult) * 128 * 1024;
        card_info->num_of_boot_blks >>= 9;
    }

fail:
    /* The block number of MMC card, which capacity is more than 2GB, shall be fetched from Ext-CSD. */
    if (card_info->ext_csd_mmc.ext_csd_rev >= 2) {
        kmdw_dbg_printf(" MMC EXT CSD Version %d\n", card_info->ext_csd_mmc.ext_csd_rev);
        if (card_info->ext_csd_mmc.sec_count) {
            card_info->num_of_blks = card_info->ext_csd_mmc.sec_count;
            card_info->blk_addr = 1;
        }
        kmdw_dbg_printf("Ext-CSD: Block number %d\n", card_info->num_of_blks, card_info->max_dtr);
    } else {
        kmdw_dbg_printf("MMC CSD Version 1.%d\n", card_info->csd_mmc.csd_structure);

        card_info->num_of_blks = (card_info->csd_mmc.c_size + 1) << (card_info->csd_mmc.c_size_mult + 2);
        /* Change to 512 bytes unit */
        card_info->num_of_blks = card_info->num_of_blks << (card_info->csd_mmc.read_bl_len - 9);
        card_info->blk_addr = 0;
        kmdw_dbg_printf("CSD: Block number %d\n", card_info->num_of_blks, card_info->max_dtr);
    }

    /* Index start from zero */
    card_info->num_of_blks -= 1;

    return KDRV_STATUS_OK;
}



/**
 * @brief kdrv_sdc_scan_card() scan sd/mmc memory card
 *
 * @param[in]   dev         device structure
 * @return      kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_scan_cards(kdrv_sdc_res_t *dev)
{
    uint32_t resp;
    uint32_t ocr, s18r;
    kdrv_status_t err;
    uint8_t F8 = 0;
    uint16_t bit_pos;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    kdrv_sdc_sd_host_t *host = dev->host;

    /*
     * It's not necessary to save from error_recovery function until the card is in transfer state.
     * Because some commands are not supported in MMC/SD/SDIO in the mix-initiation.
     * Error_recover function may destoried the initial procedure.
     */
    err_recover = 0;
    if (card_info->already_init == true && card_info->card_type == SDIO_TYPE_CARD) {
        /* CMD 52: Reset the SDIO only card.(Write the specified bit after fetching the content.) */
        kdrv_sdc_send_command(dev, SDHCI_CMD52_IO_RW_DIRECT, SDHCI_CMD_TYPE_NORMAL, 0,
                              SDHCI_CMD_RTYPE_R1R5R6R7, 1,
                              ((SD_CMD52_RW_in_R) | (SD_CMD52_no_RAW) | SD_CMD52_FUNC(0) |
                               SD_CMD52_Reg_Addr(0x06)));

        resp = (uint32_t) sdc_reg->cmd_resplo;
        resp = resp & 0xF;
        resp = resp | 0x8;  // Setting the reset bit in 6th byte of function 0

        kdrv_sdc_send_command(dev, SDHCI_CMD52_IO_RW_DIRECT, SDHCI_CMD_TYPE_NORMAL, 0,
                              SDHCI_CMD_RTYPE_R1R5R6R7, 1,
                              (SD_CMD52_RW_in_W | SD_CMD52_RAW |
                               SD_CMD52_FUNC(0) | SD_CMD52_Reg_Addr(0x06) | resp));
    } else {
        s18r = (sdc_reg->cap_reg2 & (SDHCI_SUPPORT_SDR50 | SDHCI_SUPPORT_SDR104
                                     | SDHCI_SUPPORT_DDR50)) ? 1 : 0;
        /* CMD0, reset the device to idle state */
        if (kdrv_sdc_ops_go_idle_state(dev, 0)) {
            return KDRV_STATUS_ERROR;
        }
        card_info->card_type = CARD_TYPE_UNKNOWN;

        /* CMD8 of SD card, F8 is only referenced by SD and SDIO flow */
        err = kdrv_sdc_ops_send_if_cond(dev, ((host->ocr_avail & 0xFF8000) != 0) << 8 | 0xAA);
        F8 = err ? 0 : 1;

        /*
         * ...then normal SD... ACMD41
         */
        err = kdrv_sdc_ops_send_app_op_cond(dev, 0, &ocr);
        if (!err) {
            card_info->card_type = MEMORY_CARD_TYPE_SD;
            goto set_voltage;
        }
        /*
         * ...and finally MMC. CMD1
         */
        err = kdrv_sdc_ops_send_op_cond(dev, 0, &ocr);
        if (!err) {
            card_info->card_type = MEMORY_CARD_TYPE_MMC;
            goto set_voltage;
        }
    }
set_voltage:
    if (card_info->card_type == CARD_TYPE_UNKNOWN) {
        kmdw_dbg_printf("No Supported Card found !\n");
        return KDRV_STATUS_SDC_CARD_TYPE_ERR;
    }

    if (ocr & 0x7F) {
        kmdw_dbg_printf("Card claims to support voltages below the defined range. These will be ignored.\n");
        ocr &= ~0x7F;
    }

    if (ocr & (1 << 7)) {
        kmdw_dbg_printf(" WARN## ... SD card claims to support the incompletely defined 'low voltage range'."
                        "This will be ignored.\n");
        ocr &= ~(1 << 7);
    }

    /* Mask off any voltage we do not support */
    ocr &= host->ocr_avail;

    /* Select the lowest voltage */
    for (bit_pos = 0; bit_pos < 24; bit_pos++) {
        if (ocr & (1 << bit_pos))
            break;
    }

    if (bit_pos >= 24) {
        kmdw_dbg_printf(" ERR## ... Can not find correct voltage value.\n");
        return KDRV_STATUS_SDC_VOL_ERR;
    }
    kdrv_sdc_set_power(dev, bit_pos);
    ocr |= (3 << bit_pos);

    if (card_info->card_type == MEMORY_CARD_TYPE_MMC) {
        /* Bit 30 indicate Host support high capacity */
        err = kdrv_sdc_ops_send_op_cond(dev, (ocr | (1 << 30)), &card_info->ocr);
        if (err) {
            kmdw_dbg_printf("MMC card init failed ... CMD1 !\n'");
            return KDRV_STATUS_SDC_INIT_ERR;
        }

        if (((card_info->ocr >> 29) & 3) == 2) {
            card_info->blk_addr = 1;
            kmdw_dbg_printf(" Found MMC Card .... sector mode \n");
        } else {
            card_info->blk_addr = 0;
            kmdw_dbg_printf(" Found MMC Card .... byte mode \n");
        }
    } else {
        if (card_info->card_type == MEMORY_CARD_TYPE_SD) {
            if (F8 == 1) {
                ocr |= ((s18r << 24) | (1 << 30));
            }

            if (kdrv_sdc_ops_send_app_op_cond(dev, ocr, &card_info->ocr)) {
                kmdw_dbg_printf(" SD Card init failed ... ACMD41\n");
                return KDRV_STATUS_SDC_INIT_ERR;
            }

            if (card_info->ocr & (1 << 30)) {
                kmdw_dbg_printf(" Found SD Memory Card (SDHC or SDXC).\n");
            } else {
                kmdw_dbg_printf(" Found SD Memory Card Version 1.X.\n");
            }

            /* Accept IO Signal Voltage switch to 1.8v */
        }
    }
    if (card_info->card_type != SDIO_TYPE_CARD) {
        // Commented by MikeYeh 081127
        /* Sending the CMD2 to all card to get each CID number */
        /* CMD 2 */
        err = kdrv_sdc_ops_all_send_cid(dev);
        if (err)
            return err;
    }

    card_info->rca = (card_info->card_type != MEMORY_CARD_TYPE_MMC) ? 0 : 2;
    if ((err = kdrv_sdc_ops_send_rca(dev)) != KDRV_STATUS_OK) {
        kmdw_dbg_printf("ERR## Card init failed ... CMD3 !\n");
        return err;
    }
    card_info->resp_lo = (uint32_t) (sdc_reg->cmd_resplo);


    if ((card_info->card_type == MEMORY_CARD_TYPE_SD) ||
        (card_info->card_type == MEMORY_CARD_TYPE_MMC)) {
        uint32_t e, m;
        if ((err = kdrv_sdc_ops_send_csd(dev)) != KDRV_STATUS_OK) {
            kmdw_dbg_printf("ERR## Card init failed ... CMD9 !\n");
            return err;
        }

        wr_bl_len = rd_bl_len = 9;
        if (card_info->card_type == MEMORY_CARD_TYPE_SD) {
            if ((card_info->csd_hi >> 54) == 0) {
                card_info->num_of_blks =
                    (card_info->csd_ver1.c_size + 1) << (card_info->csd_ver1.c_size_mult + 2);
                e = card_info->csd_ver1.tran_speed & 7;
                m = (card_info->csd_ver1.tran_speed >> 3) & 0xF;
                card_info->blk_addr = 0;
            } else if ((card_info->csd_hi >> 54) == 1) {
                rd_bl_len = wr_bl_len = 9;

                card_info->num_of_blks = (card_info->csd_ver2.c_size + 1) << 10;
                e = card_info->csd_ver2.tran_speed & 7;
                m = (card_info->csd_ver2.tran_speed >> 3) & 0xF;

                card_info->blk_addr = 1;
            } else {
                card_info->num_of_blks = 0;
                return KDRV_STATUS_ERROR;
            }

            card_info->max_dtr = tran_exp[e] * tran_mant[m];

            /* Index start from zero */
            card_info->num_of_blks -= 1;
        }

        if ((err = kdrv_sdc_ops_send_card_status(dev)) != KDRV_STATUS_OK) {
            kmdw_dbg_printf("ERR## Get Card Status Failed after CMD9 !\n");
            return err;
        }
    }

    /* Commented by MikeYeh 081127 */
    /* The following checks whether the card state is in standby or not */
    if ((((card_info->resp_lo >> 9) & 0xF) == CUR_STATE_STBY) ||
        card_info->card_type == SDIO_TYPE_CARD) {
        if (kdrv_sdc_ops_select_card(dev)) {
            kmdw_dbg_printf("ERR## Card init failed ... CMD7 !\n");
            return KDRV_STATUS_ERROR;
        }
    } else {
        kmdw_dbg_printf("Initialization is failed due to state changing from stand-by to transfer\n");
        return KDRV_STATUS_ERROR;
    }

    /* enable error recovery */
    err_recover = 0;
    if (card_info->card_type != SDIO_TYPE_CARD) {
        if (kdrv_sdc_ops_send_card_status(dev)) {
            kmdw_dbg_printf("ERR## Get Card Status Failed (Init) !\n");
            return KDRV_STATUS_ERROR;
        }
    }
    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_prepare_data() prepare data
 */
static kdrv_status_t kdrv_sdc_prepare_data(kdrv_sdc_res_t *dev,uint32_t blk_cnt, uint16_t blk_sz,
        uint32_t buff_addr, kdrv_sdc_transfer_act_e act)
{
    uint32_t bound, length;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    sdc_reg->blk_size = blk_sz;
    if (!dev->infinite_mode)    /* Block Count Reg = Desired Blocks */
        sdc_reg->blk_cnt = blk_cnt;
    else if (dev->infinite_mode == INFINITE_MODE_1) /* Block Count Reg = 2 * Desired Blocks */
        sdc_reg->blk_cnt = blk_cnt << 1;
    else if (dev->infinite_mode == INFINITE_MODE_2) /* Block Count Reg = 0 */
        sdc_reg->blk_cnt = 0;

    if (sdc_reg->txmode & SDHCI_TXMODE_AUTOCMD23_EN)
        sdc_reg->sdma_addr = blk_cnt;

    card_info->cmpl_mask = WAIT_TRANS_COMPLETE;
    //kmdw_dbg_printf("card_info->cmpl_mask=%x\n",card_info->cmpl_mask);
    if (card_info->flow_set.use_dma == SDMA) {
        sdc_reg->sdma_addr = buff_addr;
        sdc_reg->blk_size |= (card_info->flow_set.line_bound << 12);

        length = blk_cnt * blk_sz;
        bound = sdma_bound_mask + 1;
        /* "Infinite transfer" Or "buff_addr + length cross the SDMA boundary",
           Wait for DMA interrupt, no Transfer Complete interrupt  */
        if ((dev->infinite_mode && (sdc_reg->txmode & SDHCI_TXMODE_MULTI_SEL)) ||
            ((buff_addr + length) > ((buff_addr & ~sdma_bound_mask) + bound)) ) {
            card_info->cmpl_mask &= ~WAIT_TRANS_COMPLETE;
            card_info->cmpl_mask |= WAIT_DMA_INTR;
        }
    } else if (card_info->flow_set.use_dma == ADMA) {
        if (kdrv_sdc_fill_adma_desc_table(dev, blk_cnt * blk_sz, (uint8_t*)buff_addr))
            return KDRV_STATUS_ERROR;
        sdc_reg->adma_addr = (uint32_t) &sdc_adma2_desc_table;//CPU_TO_AHB_ADDRSPACE(ADMA2_DESC_TABLE);

        if (dev->adma2_use_interrupt)
            card_info->cmpl_mask |= WAIT_DMA_INTR;
    }
    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_send_data_command() write data to sd/emmc memory card
 *
 * @param[in]   dev         device structure
 * @param[in]   start_addr  address for read
 * @param[in]   blkcnt      block count
 * @param[in]   blksz       block size
 * @param[in]   buf         write buffer
 * @return      kdrv_staus
 */
static kdrv_status_t kdrv_sdc_send_data_command(kdrv_sdc_res_t *dev, kdrv_sdc_transfer_act_e act,
        uint32_t start_addr, uint32_t blkcnt, uint32_t blksz, uint32_t *buf)
{
    uint8_t blk_cnt_en, auto_cmd, multi_blk, cmd_idx;
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    /* set Tx mode */
    if (blkcnt > 1 || dev->infinite_mode) {
        /* Infinite Mode can not use Auto CMD12/CMD23 */
        auto_cmd = (dev->infinite_mode) ? 0 : card_info->flow_set.auto_cmd;

        multi_blk = 1;

        if (dev->infinite_mode) {
            /* For infinite transfer test, Disable the Block Count register */
            blk_cnt_en = 0;
        } else if (blkcnt > 65535) {
            /* Block Count register limits the maximum of 65535 block transfer. */
            if (card_info->flow_set.use_dma == ADMA)
                blk_cnt_en = 0;
            else {
                kmdw_dbg_printf(" Non-ADMA transfer can not larger than 65535 blocks.\n");
                return KDRV_STATUS_ERROR;
            }
        } else {
            blk_cnt_en = 1;
        }
    } else
        multi_blk = auto_cmd = blk_cnt_en = 0;

    if ((err = kdrv_sdc_set_transfer_mode(dev, blk_cnt_en, auto_cmd, (act == READ) ?
                                          SDHCI_TXMODE_READ_DIRECTION : SDHCI_TXMODE_WRITE_DIRECTION,
                                          multi_blk))!= KDRV_STATUS_OK)
        return err;

    if ((err = kdrv_sdc_prepare_data(dev, blkcnt, (1 << blksz),
                                     (uint32_t)buf, act)) != KDRV_STATUS_OK)
        return err;

    if (!card_info->blk_addr)
        start_addr = start_addr * (1 << blksz);

    /* Read :  Single block use CMD17 and multi block uses CMD18 */
    /* Write : Single block uses CMD24 and multi block uses CMD25 */

    if (blkcnt == 1 && !dev->infinite_mode)
        cmd_idx = (act == READ) ? SDHCI_CMD17_READ_SINGLE_BLOCK : SDHCI_CMD24_WRITE_BLOCK;
    else
        cmd_idx = (act == READ) ? SDHCI_CMD18_READ_MULTI_BLOCK : SDHCI_CMD25_WRITE_MULTI_BLOCK;

    if ((err = kdrv_sdc_send_command(dev, cmd_idx, SDHCI_CMD_TYPE_NORMAL, 1,
                                     SDHCI_CMD_RTYPE_R1R5R6R7, 1, start_addr)) != KDRV_STATUS_OK)
        return err;

    if (card_info->resp_lo & SD_STATUS_ERROR_BITS) {
        kmdw_dbg_printf(" Card Status indicate error 0x%08x.\n", card_info->resp_lo);
        return KDRV_STATUS_ERROR;
    }
    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_card_write() write data to sd/emmc memory card
 *
 * @param[in]   dev     device structure
 * @param[in]   addr    address for read
 * @param[in]   blkcnt  block count
 * @param[in]   buf     write buffer
 * @return      kdrv_staus
 */
static kdrv_status_t kdrv_sdc_card_write(kdrv_sdc_res_t *dev, uint32_t addr,
        uint32_t blkcnt, uint8_t *buf)
{
    kdrv_status_t err;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    if ((err = kdrv_sdc_send_data_command(dev, WRITE, addr, blkcnt,
                                          wr_bl_len, (uint32_t *)buf)) != KDRV_STATUS_OK)
        return err;
    if ((err = kdrv_sdc_transfer_data(dev, WRITE, (uint32_t *)buf,
                                      (blkcnt << wr_bl_len))) != KDRV_STATUS_OK)
        return err;

    if (((blkcnt > 1) && !card_info->flow_set.auto_cmd) ||  dev->infinite_mode) {
        if ((card_info->flow_set.use_dma == PIO) &&  dev->infinite_mode) {
            sdc_reg->blk_gap_ctl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;
        }

        /* CMD12 */
        if ((err = kdrv_sdc_send_abort_command(dev)) != KDRV_STATUS_OK)
            return err;
    }

    if ((err = kdrv_sdc_wait_for_state(dev, CUR_STATE_TRAN, 5000)) != KDRV_STATUS_OK)
        return err;

    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_card_read() read sd/emmc memory card
 *
 * @param[in]   dev     device structure
 * @param[in]   addr    address for read
 * @param[in]   blkcnt  block count
 * @param[in]   buf     read buffer
 * @return      kdrv_status
 */
static kdrv_status_t kdrv_sdc_card_read(kdrv_sdc_res_t *dev, uint32_t addr,
                                        uint32_t blkcnt, uint8_t *buf)
{
    kdrv_status_t err;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    if ((err = kdrv_sdc_send_data_command(dev, READ, addr, blkcnt, rd_bl_len,
                                          (uint32_t *) buf)) != KDRV_STATUS_OK) {
        return err;
    }

    if ((err = kdrv_sdc_transfer_data(dev, READ, (uint32_t *) buf,
                                      (blkcnt << rd_bl_len))) != KDRV_STATUS_OK) {
        return err;
    }

    if (((blkcnt > 1) && !card_info->flow_set.auto_cmd) || dev->infinite_mode) {
        if ((card_info->flow_set.use_dma == PIO) && dev->infinite_mode) {
            sdc_reg->blk_gap_ctl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;
        }

        /* CMD12 */
        //err = kdrv_sdc_send_abort_command(dev);
        if ((err = kdrv_sdc_send_abort_command(dev)) != KDRV_STATUS_OK)
            return err;
    }
    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_send_command() send command
 * @param[in] dev device
 * @param[in] cmd_idx command index
 * @param[in] cmd_type command type
 * @param[in] data_present data presetn
 * @param[in] resp_type response type
 * @param[in] inhibit_dat_chk inhibit data check
 * @param[in] argu arguments
 * Set block count, block size, DMA table address.
 */
static kdrv_status_t kdrv_sdc_send_command(kdrv_sdc_res_t *dev, uint8_t cmd_idx,
        uint8_t cmd_type, uint8_t data_present, uint8_t resp_type,
        uint8_t inhibit_datchk, uint32_t argu)
{
    uint16_t val;
    clock_t t0;
    uint8_t wait;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    uint32_t CmdType = SDHCI_CMD_TYPE_NORMAL ;
    dev->inhibit_datchk = inhibit_datchk;
    dev->data_present = data_present;
    dev->response_type = resp_type;

    //kmdw_dbg_printf("cmd=%d \n", cmd_idx);
    // Checking the MMC system spec. version more than 4.0 or newer.
    if (card_info->card_type == MEMORY_CARD_TYPE_MMC && card_info->csd_mmc.spec_vers < 4) {
        if (cmd_idx == 6 || cmd_idx == 8) {
            //kmdw_dbg_printf(" ERR## ... Commmand 6 or 8 is not supported in this MMC system spec.\n");
            return KDRV_STATUS_SDC_CMD_NOT_SUPPORT;
        }
    }

    if (sdc_reg->present_state & SDHCI_STS_CMD_INHIBIT) {
        //kmdw_dbg_printf(" ERR## CMD INHIBIT is one(Index = %d).\n", cmd_idx);
        return KDRV_STATUS_SDC_INHIBIT_ERR;
    }

    card_info->err_sts = 0;

    wait = WAIT_CMD_COMPLETE;
    card_info->cmpl_mask |= WAIT_CMD_COMPLETE;
    if (dev->inhibit_datchk || (dev->response_type == SDHCI_CMD_RTYPE_R1BR5B)) {
        if (sdc_reg->present_state & SDHCI_STS_CMD_DAT_INHIBIT) {
            //kmdw_dbg_printf(" ERR## CMD DATA INHIBIT is one.\n");
            return KDRV_STATUS_SDC_INHIBIT_ERR;
        }

        /* If this is R1B, we need to wait transfer complete here.
         * Otherwise, wait transfer complete after read/write data.
         */
        if (dev->response_type == SDHCI_CMD_RTYPE_R1BR5B) {
            card_info->cmpl_mask |= WAIT_TRANS_COMPLETE;
            wait |= WAIT_TRANS_COMPLETE;
        }
    }
    sdc_reg->cmd_argu = argu;
    if(cmd_idx == SDHCI_CMD12_STOP_TRANS) CmdType = SDHCI_CMD_TYPE_ABORT;
    val = ((cmd_idx & 0x3f) << SDHCI_CMD_IDX_SHIFT)
          | ((CmdType & 0x3) << SDHCI_CMD_TYPE_SHIFT)
          | ((dev->data_present & 0x1) << SDHCI_CMD_DATA_PRESEL_SHIFT)
          | (dev->response_type & 0x1F);
    sdc_reg->cmd_reg = val;

    if (cmd_idx == SDHCI_CMD19_SEND_TUNE_BLOCK)
        return KDRV_STATUS_OK;
    t0 = kdrv_current_t1_tick();
    while (!card_info->err_sts && (card_info->cmpl_mask & wait)) {
        /* 1 secs */
        if ((kdrv_current_t1_tick() - t0) > 1000) {
            kmdw_dbg_printf(" Wait for Interrupt timeout.\n");
            return KDRV_STATUS_SDC_TIMEOUT;
        }
    }

    /* Read Response Data */
    card_info->resp_lo = sdc_reg->cmd_resplo;
    card_info->resp_hi = sdc_reg->cmd_resphi;

    if (card_info->err_sts) {
        kmdw_dbg_printf("SEND: ERR=0x%x", card_info->err_sts);
        if (card_info->err_sts & SDHCI_INTR_ERR_AUTOCMD)
            kdrv_sdc_auto_cmd_error_recovery(dev);
        else {
            kmdw_dbg_printf("ErrorRecovery\n");
            kdrv_sdc_error_recovery(dev);
        }
        return KDRV_STATUS_ERROR;
    }
    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_hw_attr_init() init the hardware attribute setting
 * @return kdrv_status_t
 */
static kdrv_status_t kdrv_sdc_hw_attr_init(kdrv_sdc_res_t *dev)
{
    uint32_t clk;               // ip clock
    kdrv_status_t rstatus;
    volatile kdrv_sdc_reg_t *sdc_reg = dev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    kdrv_sdc_sd_host_t *host = dev->host;

    if(sdc_reg==NULL || card_info == NULL || host ==NULL) {
        return KDRV_STATUS_SDC_MEM_ALLOC_ERR;
    }

    //infinite_mode = 0;
    //cardChanged = false;

    memset ((void *)card_info, 0, sizeof(kdrv_sdc_sdcard_info_t));
    /* Reset the controller */
    sdc_reg->softrst = SDHCI_SOFTRST_ALL;
    //SDC does not provide the correct Base Clock Frequency value to the Capability
    //register (Offset = 0x40), bits[15:8]. FWSDC021 assigns 100-MHz as the default value.
    clk = (sdc_reg->cap_reg >> 8) & 0xFF;

    if (clk == 0) {
        //kmdw_dbg_printf(" ERR## Hardware doesn't specify base clock frequency.\n");

        /* Controller does not specify the base clock frequency.
         * Current design base clock  = SD ODC frequency x 2.
         */
        clk = FTSDC021_BASE_CLOCK;  /* (sdc_reg->cap_reg >> 8) & 0xFF; */
    }
    //clk = FTSDC021_BASE_CLOCK;
    kdrv_sdc_set_base_clock(host, clk);

    if (sdc_reg->cap_reg & SDHCI_CAP_VOLTAGE_33V)
        host->ocr_avail = (3 << 20);

    if (sdc_reg->cap_reg & SDHCI_CAP_VOLTAGE_30V)
        host->ocr_avail |= (3 << 17);

    if (sdc_reg->cap_reg & SDHCI_CAP_VOLTAGE_18V)
        host->ocr_avail |= (1 << 7);

#ifdef KDRV_SDC_DBG
    kmdw_dbg_printf(" - Base Clock Frequency: Min %d Hz, Max %d Hz.\n", host->min_clk, host->max_clk);
    kmdw_dbg_printf(" - Max Block Length: 0x%x bytes.\n", 0x200 << ((sdc_reg->cap_reg >> 16) & 0x3));
    kmdw_dbg_printf(" - 8-bit: %s, ADMA2: %s, HS: %s, SDMA: %s, S/R: %s, Voltage: 0x%x,SDR50/104,DDR50: 0x%x.\n\n",
                    ((sdc_reg->cap_reg >> 18) & 1) ? "Yes" : "No",
                    ((sdc_reg->cap_reg >> 19) & 1) ? "Yes" : "No",
                    ((sdc_reg->cap_reg >> 21) & 1) ? "Yes" : "No",
                    ((sdc_reg->cap_reg >> 22) & 1) ? "Yes" : "No",
                    ((sdc_reg->cap_reg >> 23) & 1) ? "Yes" : "No", ((sdc_reg->cap_reg >> 24) & 7), (sdc_reg->cap_reg2 & 7));
#endif
    /* Support SDR50, SDR104 and DDR50, we might request wot switch 1.8V IO signal */
    if (sdc_reg->cap_reg2 & 7) {
        host->ocr_avail |= (1 << 24);
    }
    /* Clock must be < 400KHz for initialization */
    kdrv_sdc_set_sd_clock(dev, host->min_clk);
    kdrv_sdc_set_power(dev, 17);

    if (sdc_reg->hw_attr & 0x1C) {
        /* FIFO use SRAM: 1K, 2K or 4K */
        dev->fifo_depth = 512;
    } else if (sdc_reg->hw_attr & 0x2)
        dev->fifo_depth = 16 << 2;
    else if (sdc_reg->hw_attr & 0x1)
        dev->fifo_depth = 8 << 2;
#ifdef KDRV_SDC_DBG
    kmdw_dbg_printf(" - FIFO depth %d bytes.\n", dev->fifo_depth);
#endif

    /* set timeout ctl */
    sdc_reg->timeout_ctl = 13;
    //Refer to chapter 2.2.15(Timeout control register(0x2E) of SD Host Controller Spec. v2.0
    dev->timeout_ms = (1 << (sdc_reg->timeout_ctl + 13)) / (clk * 1000); //13 for shift value


#ifdef __FREERTOS__
    NVIC_SetPriority(SDC_FTSDC021_0_IRQ, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
#endif
    NVIC_EnableIRQ(SDC_FTSDC021_IRQ);

    /* Always enable all interrupt */
    sdc_reg->intr_en = SDHCI_INTR_EN_ALL;
    sdc_reg->err_en = SDHCI_ERR_EN_ALL;

    kmdw_dbg_printf(" Checking card ... ");
    if ((rstatus=kdrv_sdc_check_card_insert(dev)) != KDRV_STATUS_OK) {
        return rstatus;
    }

    /* Do not require to enable Error Interrupt Signal.
     * Bit 15 of Normal Interrupt Status will tell if
     * error happens.
     */

    card_info->already_init = true;

    /* Optionally enable interrupt signal to CPU */
#ifdef USE_PIO
    kdrv_sdc_set_transfer_type(dev, PIO, 4, 0);
#else
    kdrv_sdc_set_transfer_type(dev, ADMA, 4, 0);
#endif

    return KDRV_STATUS_OK;
}

/**
 * @brief SDC_IRQHandler() sdc isr function.
 */
//void SDC_IRQHandler()
static void sdc_isr()
{
    uint16_t sts;
    volatile kdrv_sdc_reg_t *sdc_reg = gdev->sdc_reg;
    volatile kdrv_sdc_sdcard_info_t *card_info = gdev->card_info;
    kdrv_sdc_sd_host_t *host = gdev->host;

    /* Read Interrupt Status */
    sts = sdc_reg->intr_sts;


    /* As soon as the command complete, data ready to be read/write.
     * Buffer Read/Write Ready usually used when PIO mode.
     * We don't expect to use interrupt here, but polling.
     * Leave it to read/write data function.
     */
    sts &= ~(SDHCI_INTR_STS_BUFF_READ_READY | SDHCI_INTR_STS_BUFF_WRITE_READY);

    /* Clear Interrupt Status immediately */
    sdc_reg->intr_sts &= sts;

    /* Writing 1 to Card Interrupt Status does not clear it.
     * Instead, disable the Interrupt Status to clear.
     */
    /*
     * WARNING !
     * KL520 does not have the CD/WP pin, so it will not receive the insert/remove INT
     */
    if (sts & SDHCI_INTR_STS_CARD_INTR) {
        sdc_reg->intr_en &= ~SDHCI_INTR_STS_CARD_INTR;
        kmdw_dbg_printf("INTR: Card Interrupt.\n");
        sdc_reg->intr_en |= SDHCI_INTR_STS_CARD_INTR;
    }
#if (STANDARD_SD==1)
    if (sts & SDHCI_INTR_STS_CARD_INSERT) {
#else
    if (sts & SDHCI_INTR_STS_CARD_REMOVE) {
#endif
        kmdw_dbg_printf("INTR: Card Insert\n");
        card_info->card_insert = 1;
        //cardChanged = true;
        if (card_info->already_init == false) {
            kdrv_sdc_hw_attr_init(gdev);
        } else {
            kdrv_sdc_set_sd_clock(gdev,host->min_clk);
            kdrv_sdc_set_power(gdev,17);
        }

    }

#if (STANDARD_SD==1)
    if (sts & SDHCI_INTR_STS_CARD_REMOVE) {
#else
    if (sts & SDHCI_INTR_STS_CARD_INSERT) {
#endif
        //cardChanged = true;
        card_info->card_insert = 0;
        // Stop to provide clock
        kdrv_sdc_set_sd_clock(gdev,0);
        // Stop to provide power
        kdrv_sdc_set_power(gdev,-1);
    }

    if (sts & SDHCI_INTR_STS_CMD_COMPLETE) {
        card_info->cmpl_mask &= ~WAIT_CMD_COMPLETE;
    }

    if (sts & SDHCI_INTR_STS_TXR_COMPLETE) {
        if (card_info->cmpl_mask & WAIT_TRANS_COMPLETE) {
            card_info->cmpl_mask &= ~WAIT_TRANS_COMPLETE;
        } else {
            while(1);
        }
    }

    if (sts & SDHCI_INTR_STS_BLKGAP) {
        card_info->cmpl_mask &= ~WAIT_BLOCK_GAP;
    }

    if (sts & SDHCI_INTR_STS_DMA) {
        card_info->cmpl_mask &= ~WAIT_DMA_INTR;
    }

    if (sts & SDHCI_INTR_STS_ERR) {
        card_info->err_sts = sdc_reg->err_sts;
        kmdw_dbg_printf(" INTR: ERR## &0x%x 0x%x\n", &(sdc_reg->err_sts),sdc_reg->err_sts);
        /* Step 2: Check CMD Line Error */
        if (card_info->err_sts & SDHCI_INTR_ERR_CMD_LINE) {
            /* Step 3: Software Reset for CMD line */
            sdc_reg->softrst |= SDHCI_SOFTRST_CMD;
            /* Step 4 */
            while (sdc_reg->softrst & SDHCI_SOFTRST_CMD) ;
        }

        /* Step 5: Check DAT Line Error */
        if (card_info->err_sts & SDHCI_INTR_ERR_DAT_LINE) {
            /* Step 6: Software Reset for DAT line */
            sdc_reg->softrst |= SDHCI_SOFTRST_DAT;
            /* Step 7 */
            while (sdc_reg->softrst & SDHCI_SOFTRST_DAT) ;
        }

        /* Auto CMD Error Status register is reset */
        if (card_info->err_sts & SDHCI_INTR_ERR_AUTOCMD)
            card_info->auto_err = sdc_reg->auto_cmd_err;

        sdc_reg->err_sts = card_info->err_sts;

        if (card_info->err_sts & SDHCI_INTR_ERR_TUNING) {
            kdrv_sdc_execute_tuning(gdev, 16);
        }
    }
    return;
}

//=============================================================================
//                              KDRV SDC API
//=============================================================================
/**
 * @brief kdrv_sdc_initialize, initail sd/emmc card interface
 *      1. reset sdc status
 *      2. allocate resource and initail driving
 *      3. turn on sdc clock
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_sdc_initialize(void)
{
    //initialize memory structure and hardware attribute
    uint8_t i;
    kdrv_status_t err;
    kdrv_sdc_sdcard_info_t *card_info;
    kdrv_sdc_sd_host_t *host;
    uint32_t data;
    /* clear SD ctrl register */
    outw(SCU_EXTREG_SD_CTRL, 0x0);
    NVIC_SetVector((IRQn_Type)SDC_FTSDC021_IRQ, (uint32_t)sdc_isr);


    /* resource allocate */
    card_info = (kdrv_sdc_sdcard_info_t *)malloc(sizeof(kdrv_sdc_sdcard_info_t));
    if(!card_info) {
        return KDRV_STATUS_SDC_MEM_ALLOC_ERR;
    }
    gdev->card_info = card_info;

    host = (kdrv_sdc_sd_host_t *)malloc(sizeof(kdrv_sdc_sd_host_t));
    if(!host) {
        free((void *)gdev->card_info);
        return KDRV_STATUS_SDC_MEM_ALLOC_ERR;
    }

    for(i = 0; i < 6; i++) {
        /* set sdc pins output driven to 8mA */
        data = inw(SCU_EXTREG_SD_CLK_IOCTRL + i*4);
        outw(SCU_EXTREG_SD_CLK_IOCTRL+ i * 4, (data & ~0x1FF) | 0x48);
    }

    /* sdc clock enable */
    SCU_EXTREG_CLK_EN1_SET_sdclk(1);

    gdev->host = host;
    /* initialize hw attribute setting */
    if ((err = kdrv_sdc_hw_attr_init(gdev)) != KDRV_STATUS_OK) {
        free((void *)gdev->card_info);
        free((void *)gdev->host);
        return err;
    }
    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_uninitialize, uninitail sd card interface and resource
 *      1. reset sdc status and initail driven
 *      2. turn on sdc clock
 */
kdrv_status_t kdrv_sdc_uninitialize(void)
{
    /* uninitialize resource */
    if(gdev->card_info)
        free((void*)gdev->card_info);
    if(gdev->host)
        free((void*)gdev->host);
    /* sdc clock disable */
    SCU_EXTREG_CLK_EN1_SET_sdclk(0);
    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_sdc_dev_scan() scan sd/mmc memory card
 *
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_sdc_dev_scan(void)
{
    kdrv_status_t err;
    volatile kdrv_sdc_sdcard_info_t *card_info = gdev->card_info;
    if ((err = kdrv_sdc_scan_cards(gdev)) != KDRV_STATUS_OK) {
        return err;
    }
    if (card_info->card_type == MEMORY_CARD_TYPE_SD) {
        kdrv_sdc_read_scr(gdev);
    } else if (card_info->card_type == MEMORY_CARD_TYPE_MMC) {
        kdrv_sdc_read_ext_csd(gdev);
    }
    if (!kdrv_sdc_set_bus_speed_mode(gdev, SPEED_SDR25)) {
        kdrv_sdc_set_sd_clock(gdev, (card_info->max_dtr));
    }
    return KDRV_STATUS_OK;
}


/**
 * @brief kdrv_sdc_get_dev() get device infromation
 *
 * @return kdrv_status_t
 */
kdrv_sdc_res_t* kdrv_sdc_get_dev(void)
{
    return gdev;
}

/**
 * @brief kdrv_sdc_read read data from sd/mmc card
 *
 * @param[in]   buf         buffer to write.
 * @param[in]   sd_offset   sd/mmc offset address
 * @param[in]   size        read size(Multiple of 512, 1sector=512Bytes)
 * @return      kdrv_status_t
 */
kdrv_status_t kdrv_sdc_read(uint8_t *buf, uint32_t sd_offset, uint32_t size)
{
    uint32_t blk_cnt, blk_size;
    blk_size = 1 << rd_bl_len;
    blk_cnt = (size + blk_size - 1) / blk_size;
    return(kdrv_sdc_card_read(gdev, sd_offset, blk_cnt, buf));
}

/**
 * @brief kdrv_sdc_write write data from sd/mmc card
 *
 * @param[in]   buf         buffer to write.
 * @param[in]   sd_offset   sd/mmc offset address
 * @param[in]   size        write size(Multiple of 512, 1sector=512Bytes)
 * @return      kdrv_status_t
 */
kdrv_status_t kdrv_sdc_write(uint8_t *buf, uint32_t sd_offset, uint32_t size)
{
    uint32_t blk_cnt, blk_size;
    blk_size = 1 << rd_bl_len;
    blk_cnt = (size + blk_size - 1) / blk_size;

    return(kdrv_sdc_card_write(gdev, sd_offset, blk_cnt, buf));
}

