#include <string.h>
#include "project.h"
#include "base.h"
#include "kmdw_dfu.h"
#include "kmdw_utils_crc.h"
#include "kdev_flash.h"
#include "kmdw_model.h"
#include "kmdw_console.h"
#include "kdrv_clock.h"
#include "kmdw_power_manager.h"
#include "kdp_system.h"
#include "kdrv_cmsis_core.h"

#define VERIFY_BLK_SZ FLASH_MINI_BLOCK_SIZE //0x1000

#define MODEL_INFO_FLASH_ADDR       FLASH_MODEL_FW_INFO_ADDR
#define MODEL_ALL_BIN_FLASH_ADDR    FLASH_MODEL_ALL_ADDR

#define BOOT_STATE_CONFIRMED       0x1
#define BOOT_STATE_FIRST_BOOT      0x2
#define BOOT_STATE_POST_FIRST_BOOT 0x4
#define BOOT_STATE_NOT_CONFIRMED   0x8

#define MAX_BOOT_SEQ  0x7ffffff0

typedef struct {
    u32 partition_id;
    u32 seq;
    u32 flag;
} dfu_boot_cfg_item_t;

typedef struct {
    dfu_boot_cfg_item_t scpu_cfg;
    dfu_boot_cfg_item_t ncpu_cfg;
} dfu_boot_cfg_t;

// Initial buffer for boot configuration
static u8 init_ver_buf[0x40 + sizeof(dfu_boot_cfg_t)];

/* tmp buffer for content verification
 * passed by caller, length should be at least VERIFY_BLK_SZ */
static u8 *tmp_ver_buf = init_ver_buf;
static FnReadData fn_read_data = NULL;

/* ############################
 * ##    static functions    ##
 * ############################ */

static dfu_boot_cfg_t boot_cfg_0, boot_cfg_1;

int flashing;

static int flash_wait_ready(int timeout_ms)
{
    kdev_flash_status_t flash_status;
    int i;

    for (i = 0; i < timeout_ms; i++) {
        flash_status = kdev_flash_get_status();
        if (flash_status.busy == 0) break;
        kdrv_delay_us(1*1000);
    }
    if (i == timeout_ms) i = -1;  // we have timed out
    return i;
}

static int dfu_update_sleep(enum kmdw_power_manager_device_id dev_id)
{
    while (flashing == 1) {
        err_msg("dfu_update_sleep: stop for flashing.\n");
        osThreadFlagsWait(BIT27, osFlagsWaitAll, osWaitForever);
    }
    err_msg("dfu_update_sleep: ok\n");

    return 0;
}

static int dfu_update_deep_sleep(enum kmdw_power_manager_device_id dev_id)
{
    while (flashing == 1) {
        err_msg("dfu_update_deep_sleep: stop for flashing.\n");
        osThreadFlagsWait(BIT27, osFlagsWaitAll, osWaitForever);
    }
    err_msg("dfu_update_deep_sleep: ok\n");

    return 0;
}

/*
Compare flash content with buffer content, the size shall be more than 4k,
     because small block (<4k) was verified already, here is to verify the
     consistency/integrity among 4k blocks
Return:
    0 - success
    -1 - fail
*/

static int dfu_post_flash_verify_4kblock(u32 flash_addr, u32 size, u8 *pbuf)
{
    int remainder, loop, i, ret, len;

    loop = size / VERIFY_BLK_SZ;
    if (size % VERIFY_BLK_SZ)
        loop += 1;
    remainder = size;
    u8* flash_sector_check = tmp_ver_buf + 0x40;

    for (i = 0; i < loop; i++) {
        len = remainder > VERIFY_BLK_SZ ? VERIFY_BLK_SZ : remainder;
        kdev_flash_readdata((flash_addr + i * VERIFY_BLK_SZ) & 0xFFFFF000,
                            flash_sector_check, len);  // read the new sector
        ret = flash_wait_ready(300);
        if (ret == -1)
        {
            err_msg("Flash read failure, timeout\n");
            return -1;
        }

        ret = memcmp(flash_sector_check, pbuf + i * VERIFY_BLK_SZ, len);
        if (ret != 0)
        {
            err_msg("Found diff, flash failed\n");
            return -1;
        }
        remainder -= len;
    }

    return 0;
}

/*
Write memory data to flash, the size is 4k blocks (i.e. n*4k)

Return:
    0 - success
    -1 - fail
*/
static int dfu_mem_to_flash_4k_blocks(u32 mem_addr, u32 flash_addr, u32 size)
{
    int ret;

    if (mem_addr & 0x1f != 0)
    {
        err_msg("memory address does not align to 32bit boundary");
        return -1;
    }

    if (flash_addr & 0x1f != 0)
    {
        err_msg("flash address does not align to 32bit boundary");
        return -1;
    }

    /* erase flash sectors firstly */

    u16 sect_num = size / VERIFY_BLK_SZ;     /* sector size = 4K */
    if (size % VERIFY_BLK_SZ)
        sect_num += 1;
    u32 offset = 0;
    u32 length = size;
    u32 len = 0;
    for (int sect = 0; sect < sect_num; sect++)
    {
        len = length > VERIFY_BLK_SZ ? VERIFY_BLK_SZ : length;
        kdev_flash_erase_sector(flash_addr + offset);
        ret = flash_wait_ready(300);  // wait for the erase operation to complete
        if (ret < 0) {
            dbg_msg("Erase Flash Sector Timeout\n");
        }

        kdev_flash_programdata((uint32_t)(flash_addr + offset), (void *)(mem_addr + offset), len);

        offset += len;
        length -= len;
    }

    kdrv_delay_us(500 * 1000);
    return 0;
}

/*
Write memory data to flash, the size is less than 4k

Return:
    0 - success
    -1 - fail
*/
static int dfu_mem_to_flash_small_block(u32 mem_addr, u32 flash_addr, u32 size)
{
    int ret;

    /* validate parameters */
    if (size >= VERIFY_BLK_SZ)
    {
        err_msg("Wrong size, bigger than 4K");
        return -1;
    }

    if (mem_addr & 0x1f != 0)
    {
        err_msg("memory address does not align to 32bit boundary");
        return -1;
    }

    if (flash_addr & 0x1f != 0)
    {
        err_msg("flash address does not align to 32bit boundary");
        return -1;
    }

    /* erase flash sectors firstly */

    kdev_flash_erase_sector(flash_addr);
    ret = flash_wait_ready(300);  // wait for the erase operation to complete
    if (ret < 0) {
        dbg_msg("Erase Flash Sector Timeout\n");
    }

    kdev_flash_programdata(flash_addr, (void *)mem_addr, size);
    flash_wait_ready(500);

    /* read back for confirmation*/
    u8* flash_sector_check = tmp_ver_buf + 0x40;
    kdev_flash_readdata(flash_addr, flash_sector_check, size);
    ret = flash_wait_ready(300);
    if (memcmp((void *)mem_addr, (void *)flash_sector_check, size)) {
        err_msg("Flash readback verification fail at flash addr=%x\n", flash_addr);
        return -1;
    }

    kdrv_delay_us(200 * 1000);

    return 0;
}

static void dfu_init_partition_boot_cfg()
{
    int ret;
    boot_cfg_0.scpu_cfg.partition_id = 0;
    boot_cfg_0.scpu_cfg.seq = 1;
    boot_cfg_0.scpu_cfg.flag = BOOT_STATE_CONFIRMED;

    boot_cfg_0.ncpu_cfg.partition_id = 0;
    boot_cfg_0.ncpu_cfg.seq = 1;
    boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;

    boot_cfg_1.scpu_cfg.partition_id = 1;
    boot_cfg_1.scpu_cfg.seq = 0;
    boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

    boot_cfg_1.ncpu_cfg.partition_id = 1;
    boot_cfg_1.ncpu_cfg.seq = 0;
    boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

    kdev_flash_erase_sector(PARTITION_0_CFG_START_IN_FLASH);

    ret = flash_wait_ready(200);
    if (ret < 0) {
        err_msg("Error: Erase Flash Sector Timeout\n");
    }

    kdev_flash_erase_sector(PARTITION_1_CFG_START_IN_FLASH);

    ret = flash_wait_ready(200);
    if (ret < 0) {
        err_msg("Error: Erase Flash Sector Timeout\n");
    }

    kdev_flash_programdata(PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0, 32);
    ret = flash_wait_ready(200);
    if (ret < 0) {
        err_msg("Error: Flash partition 0 config Timeout\n");
    }

    kdev_flash_programdata(PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1, 32);
    ret = flash_wait_ready(200);
    if (ret < 0) {
        err_msg("Error: Flash partition 1 config Timeout\n");
    }

}

static void dfu_pre_update()
{
    NVIC_DisableIRQ(UART_FTUART010_0_IRQ);   //UART0
    NVIC_DisableIRQ(UART_FTUART010_1_IRQ);   //UART1
    NVIC_DisableIRQ(UART_FTUART010_1_1_IRQ); //UART2
    NVIC_DisableIRQ(UART_FTUART010_1_2_IRQ); //UART3
    NVIC_DisableIRQ(UART_FTUART010_1_3_IRQ); //UART4
    NVIC_DisableIRQ(NPU_NPU_IRQ);            //NPU
}

static void dfu_update_abort(u32 reload_flag)
{
    if (reload_flag)
        kmdw_model_refresh_models();  // reload all the models again
    NVIC_EnableIRQ(UART_FTUART010_0_IRQ);       //UART0
    NVIC_EnableIRQ(UART_FTUART010_1_IRQ);       //UART1
    NVIC_EnableIRQ(UART_FTUART010_1_1_IRQ);     //UART2
    NVIC_EnableIRQ(UART_FTUART010_1_2_IRQ);     //UART3
    NVIC_EnableIRQ(UART_FTUART010_1_3_IRQ);     //UART4
    NVIC_EnableIRQ(NPU_NPU_IRQ);                //NPU
}

/* ############################
 * ##    public functions    ##
 * ############################ */

int kmdw_dfu_init(u8 *tmp_buf, FnReadData fn_read)
{
    static int flash_checked = 0;
    int ret;

    if (tmp_buf)
        tmp_ver_buf = tmp_buf;
    if (fn_read)
        fn_read_data = fn_read;

    if (flash_checked)
        return 0;
    flash_checked = 1;

    struct kmdw_power_manager_s pms = {
        .sleep = dfu_update_sleep,
        .deep_sleep = dfu_update_deep_sleep,
    };

    /* read flash to mem */

    kdev_flash_readdata(PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0, sizeof(boot_cfg_0));
    flash_wait_ready(300);

    kdev_flash_readdata(PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1, sizeof(boot_cfg_1));
    flash_wait_ready(300);

    if ((boot_cfg_0.scpu_cfg.flag == 0xffffffff) && (boot_cfg_1.scpu_cfg.flag == 0xffffffff)) {
        ret = -1;
        goto exit;
    }

    if ((boot_cfg_0.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT) ||
        (boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT) ||
        (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT) ||
        (boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT)) {
        err_msg("Error: wrong state, BOOT_STATE_FIRST_BOOT shall not be here\n");
        ret = -1;
        goto exit;
    }

    /* determine if necessary to read flash */

    if ((boot_cfg_0.scpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT) &&
        (boot_cfg_0.ncpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT) &&
        (boot_cfg_1.scpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT) &&
        (boot_cfg_1.ncpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT)) {

        ret = 0;
        goto exit;
    }

    if (boot_cfg_0.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_0.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_1.scpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("SCPU partition 0 was confirmed\n");

    }

    if (boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("NCPU partition 0 was confirmed\n");
    }

    if (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_1.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("SCPU partition 1 was confirmed\n");
    }

    if (boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("NCPU partition 1 was confirmed\n");
    }

    /* write back to flash */
    ret = dfu_mem_to_flash_small_block((u32)&boot_cfg_0, PARTITION_0_CFG_START_IN_FLASH, sizeof(boot_cfg_0));
    if (ret == -1)
    {
        err_msg("Flash write fail on %x\n", PARTITION_0_CFG_START_IN_FLASH);
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    ret = dfu_mem_to_flash_small_block((u32)&boot_cfg_1, PARTITION_1_CFG_START_IN_FLASH, sizeof(boot_cfg_1));
    if (ret == -1)
    {
        err_msg("Flash write fail on %x\n", PARTITION_1_CFG_START_IN_FLASH);
        ret = MSG_FLASH_FAIL;
    } else {
        ret = 0;
    }

exit:
    kmdw_power_manager_register(KMDW_POWER_MANAGER_DEVICE_DFU_UPDATE, &pms);

    return ret;
}

int kmdw_dfu_get_active_scpu_partition()
{
    kdev_flash_readdata(PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0, sizeof(boot_cfg_0));
    flash_wait_ready(300);
    kdev_flash_readdata(PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1, sizeof(boot_cfg_1));
    flash_wait_ready(300);

    if ((boot_cfg_0.scpu_cfg.flag == 0xffffffff) && (boot_cfg_1.scpu_cfg.flag == 0xffffffff)) {
        // no config data is there, need to create them for partition 0/1
        dfu_init_partition_boot_cfg();
        return 0;
    }

    if (boot_cfg_0.scpu_cfg.flag & boot_cfg_1.scpu_cfg.flag & BOOT_STATE_CONFIRMED)
    {
        err_msg("Critical Error: 2 active SCPU boot config\n");
        return -1;
    }

    if ((boot_cfg_0.scpu_cfg.partition_id == boot_cfg_1.scpu_cfg.partition_id)
        && (boot_cfg_0.scpu_cfg.seq == boot_cfg_1.scpu_cfg.seq))
    {
        // no config data is there, need to create them for partition 0/1
        dfu_init_partition_boot_cfg();
        return 0;
    }

    if ((boot_cfg_0.scpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 0;

    if ((boot_cfg_1.scpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 1;
    return 0;
}

int kmdw_dfu_get_active_ncpu_partition()
{
    kdev_flash_readdata(PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0, sizeof(boot_cfg_0));
    flash_wait_ready(300);

    kdev_flash_readdata(PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1, sizeof(boot_cfg_1));
    flash_wait_ready(300);

    if ((boot_cfg_0.ncpu_cfg.flag == 0xffffffff) && (boot_cfg_1.ncpu_cfg.flag == 0xffffffff)) {
        // no config data is there, need to create them for partition 0/1
        dfu_init_partition_boot_cfg();
        return 0;
    }

    if (boot_cfg_0.ncpu_cfg.flag & boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED)
    {
        err_msg("Critical Error: 2 active NCPU boot config\n");
        return -1;
    }

    if ((boot_cfg_0.ncpu_cfg.partition_id == boot_cfg_1.ncpu_cfg.partition_id) &&
        (boot_cfg_0.ncpu_cfg.seq == boot_cfg_1.ncpu_cfg.seq))
    {
        // no config data is there, need to create them for partition 0/1
        dfu_init_partition_boot_cfg();
        return 0;
    }

    if ((boot_cfg_0.ncpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 0;

    if ((boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 1;
    return 0;
}

int kmdw_dfu_update_scpu()
{
    int ret;
    u8 *pBase;
    u32 local_sum32, remote_sum32;
    u8  pre_active_partition;
    u32 flash_cfg_addr, flash_data_addr;
    dfu_boot_cfg_t dfu_cfg;
    u32 seq;

    flashing = 1;
    err_msg("kmdw_dfu_update_scpu: flashing ...\n");

    dfu_pre_update();

    pBase = (u8 *)DDR_BEGIN;
    memset(pBase, 0, SCPU_IMAGE_SIZE);
    ret = fn_read_data((u32)pBase, SCPU_IMAGE_SIZE);
    if (ret == SCPU_IMAGE_SIZE)
    {
        local_sum32 = kmdw_utils_crc_gen_sum32(pBase, SCPU_IMAGE_SIZE - 4);
        remote_sum32 = *(u32 *)(pBase + SCPU_IMAGE_SIZE - 4);
        if (local_sum32 != remote_sum32) {
            ret = MSG_AUTH_FAIL;
            goto exit;
        }

    }
    else {
        ret = MSG_DATA_ERROR;
        goto exit;
    }

    pre_active_partition = kmdw_dfu_get_active_scpu_partition();
    if (pre_active_partition == 0) {
        // active partition is 0, flash to partition 1
        flash_cfg_addr = PARTITION_1_CFG_START_IN_FLASH;
        flash_data_addr = SCPU_PARTITION1_START_IN_FLASH;
    }
    else if(pre_active_partition == 1){
        // active partition is 1, flash to partition 0
        flash_cfg_addr = PARTITION_0_CFG_START_IN_FLASH;
        flash_data_addr = SCPU_PARTITION0_START_IN_FLASH;
    } else {
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    // flash data
    ret = dfu_mem_to_flash_4k_blocks((u32)pBase, (u32)flash_data_addr, SCPU_IMAGE_SIZE);
    if (ret == -1)
    {
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    ret = dfu_post_flash_verify_4kblock(flash_data_addr, SCPU_IMAGE_SIZE, pBase);
    if (ret != 0) {
        err_msg("Error: post flash verification failed\n");
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    // update boot cfg

    if (pre_active_partition == 0) {
        seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);

        if(seq > MAX_BOOT_SEQ){
            boot_cfg_0.scpu_cfg.seq = 0;
            boot_cfg_1.scpu_cfg.seq = 1;
        } else {
            boot_cfg_1.scpu_cfg.seq = seq + 1;
        }
        boot_cfg_1.scpu_cfg.flag = BOOT_STATE_FIRST_BOOT;
        dfu_cfg = boot_cfg_1;
    }
    else {
        seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);
 
        if(seq > MAX_BOOT_SEQ){
            boot_cfg_0.scpu_cfg.seq = 1;
            boot_cfg_1.scpu_cfg.seq = 0;
        } else {
            boot_cfg_0.scpu_cfg.seq = seq + 1;
        }
        boot_cfg_0.scpu_cfg.flag = BOOT_STATE_FIRST_BOOT;
        dfu_cfg = boot_cfg_0;
    }

    ret = dfu_mem_to_flash_small_block((u32)&dfu_cfg, flash_cfg_addr, sizeof(dfu_cfg));

    kdev_flash_readdata(PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0, sizeof(boot_cfg_0));
    flash_wait_ready(300);

    kdev_flash_readdata(PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1, sizeof(boot_cfg_1));
    flash_wait_ready(300);

    if (pre_active_partition == 0) {
        ret = memcmp(&boot_cfg_1, &dfu_cfg, sizeof(dfu_cfg));
    }
    else {
        ret = memcmp(&boot_cfg_0, &dfu_cfg, sizeof(dfu_cfg));
    }

    if (ret == -1)
        ret = MSG_FLASH_FAIL;
    else
        ret = SUCCESS;

exit:
    err_msg("kmdw_dfu_update_scpu: flashing done\n");
    flashing = 0;
    return ret;
}

int kmdw_dfu_update_ncpu()
{
    int ret;
    u8 *pBase;
    u32 local_sum32, remote_sum32;
    u8  pre_active_partition;
    u32 flash_cfg_addr, flash_data_addr;
    dfu_boot_cfg_t dfu_cfg;
    u32 seq;

    flashing = 1;
    err_msg("kmdw_dfu_update_ncpu: flashing ...\n");

    dfu_pre_update();

    pBase = (u8 *)DDR_BEGIN;
    memset(pBase, 0, NCPU_IMAGE_SIZE);
    ret = fn_read_data((u32)pBase, (u32)NCPU_IMAGE_SIZE);
    if (ret == NCPU_IMAGE_SIZE)
    {
        local_sum32 = kmdw_utils_crc_gen_sum32(pBase, NCPU_IMAGE_SIZE - 4);
        remote_sum32 = *(u32 *)(pBase + NCPU_IMAGE_SIZE - 4);
        if (local_sum32 != remote_sum32) {
            ret = MSG_AUTH_FAIL;
            goto exit;
        }

    }
    else {
        ret = MSG_DATA_ERROR;
        goto exit;
    }

    pre_active_partition = kmdw_dfu_get_active_ncpu_partition();
    if (pre_active_partition == 0) {
        // active partition is 0, flash to partition 1
        flash_cfg_addr = PARTITION_1_CFG_START_IN_FLASH;
        flash_data_addr = NCPU_PARTITION1_START_IN_FLASH;
    }
    else if (pre_active_partition == 1){
        // active partition is 1, flash to partition 0
        flash_cfg_addr = PARTITION_0_CFG_START_IN_FLASH;
        flash_data_addr = NCPU_PARTITION0_START_IN_FLASH;
    } else {
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    // flash data
    ret = dfu_mem_to_flash_4k_blocks((u32)pBase, (u32)flash_data_addr, NCPU_IMAGE_SIZE);
    if (ret == -1)
    {
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    ret = dfu_post_flash_verify_4kblock(flash_data_addr, NCPU_IMAGE_SIZE, pBase);
    if (ret != 0) {
        err_msg("Error: post flash verification failed\n");
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    // update boot cfg

    if (pre_active_partition == 0) {
        seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);

        if(seq > MAX_BOOT_SEQ){
            boot_cfg_0.ncpu_cfg.seq = 0;
            boot_cfg_1.ncpu_cfg.seq = 1;
        } else {
            boot_cfg_1.ncpu_cfg.seq = seq + 1;
        }

        boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_FIRST_BOOT;
        dfu_cfg = boot_cfg_1;
    }
    else {
        seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);

        if(seq > MAX_BOOT_SEQ){
            boot_cfg_0.ncpu_cfg.seq = 1;
            boot_cfg_1.ncpu_cfg.seq = 0;
        } else {
            boot_cfg_0.ncpu_cfg.seq = seq + 1;
        }

        boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_FIRST_BOOT;
        dfu_cfg = boot_cfg_0;
    }

    ret = dfu_mem_to_flash_small_block((u32)&dfu_cfg, flash_cfg_addr, sizeof(dfu_cfg));
    if (ret == -1)
    {
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

    kdev_flash_readdata(PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0, sizeof(boot_cfg_0));
    flash_wait_ready(300);

    kdev_flash_readdata(PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1, sizeof(boot_cfg_1));
    flash_wait_ready(300);

    if (pre_active_partition == 0) {
        ret = memcmp(&boot_cfg_1, &dfu_cfg, sizeof(dfu_cfg));
    }
    else {
        ret = memcmp(&boot_cfg_0, &dfu_cfg, sizeof(dfu_cfg));
    }

    if (ret != 0) {
        err_msg("Error: partition config read back compare fail\n");
        ret = MSG_FLASH_FAIL;
    } else {
        ret = SUCCESS;
    }

exit:
    err_msg("kmdw_dfu_update_ncpu: flashing done\n");
    flashing = 0;
    return ret;
}

int kmdw_dfu_update_model(u32 info_size, u32 model_size)
{
    u32 sum32_download, sum32_embedded;
    u32 ddr_buf;
    int ret;
    u32 size;

    info_size >>= 16;

    if (info_size == 0) {
        // non NEF
        size = model_size;
    } else {
        // NEF
        size = info_size + model_size;
    }

    err_msg("kmdw_dfu_update_model: flashing ...\n");
    dbg_msg("kmdw_dfu_update_model: size: %d %d\n", info_size, model_size);
    dfu_pre_update();

    ddr_buf = DDR_BEGIN;
    ret = fn_read_data(ddr_buf, size);
    if (ret == size) {
        if (info_size == 0) {
            // non NEF
            sum32_embedded = *(u32 *)(ddr_buf + size - 4);
            sum32_download = kmdw_utils_crc_gen_sum32((u8 *)ddr_buf, size - 4);
        } else {
            // NEF
            u32 model_count;
            kmdw_model_fw_info_t *info_p = (kmdw_model_fw_info_t *)ddr_buf;
            kmdw_model_fw_info_ext_t *info2_p;

            model_count = info_p->model_count;
            dbg_msg("kmdw_dfu_update_model: model count is %d\n", model_count);
            info2_p = (kmdw_model_fw_info_ext_t *)(ddr_buf + sizeof(uint32_t) + model_count * sizeof(struct kdp_model_s));
            sum32_embedded = info2_p->model_checksum;
            sum32_download = sum32_embedded;        // TODO: use crc32 algorithm
            dbg_msg("kmdw_dfu_update_model: checksum 0x%X\n", sum32_download);
        }

        if (sum32_embedded != sum32_download) {
            err_msg("kmdw_dfu_update_model: checksum error 0x%X : 0x%X\n", sum32_embedded, sum32_download);
            dfu_update_abort(1);
            return MSG_AUTH_FAIL;
        }
    } else {
        dfu_update_abort(1);
        return MSG_DATA_ERROR;
    }

    if (info_size == 0) {
        // flash model_dfu.bin
        ret = dfu_mem_to_flash_4k_blocks(ddr_buf, MODEL_INFO_FLASH_ADDR, size);
        if (ret == -1) {
            err_msg("Error: flash model_dfu.bin failed %d\n", ret);
            return MSG_FLASH_FAIL;
        }

        ret = dfu_post_flash_verify_4kblock((u32)MODEL_INFO_FLASH_ADDR, size,
                                            (u8 *)ddr_buf);
        if (ret != 0) {
            err_msg("Error: verify model_dfu.bin failed %d\n", ret);
            return MSG_FLASH_FAIL;
        }
    } else {
        // check flash space 
        if ( model_size > (FLASH_END_ADDR - MODEL_ALL_BIN_FLASH_ADDR)  ) {
            err_msg("Error: Not enough flash space\n");
            return MSG_FLASH_NO_SPACE;
        }

        // flash fw_info.bin
        ret = dfu_mem_to_flash_4k_blocks(ddr_buf, MODEL_INFO_FLASH_ADDR, info_size);
        if (ret == -1) {
            err_msg("Error: flash fw_info.bin failed %d\n", ret);
            return MSG_FLASH_FAIL;
        }

        ret = dfu_post_flash_verify_4kblock((u32)MODEL_INFO_FLASH_ADDR, info_size,
                                            (u8 *)ddr_buf);
        if (ret != 0) {
            err_msg("Error: verify fw_info.bin failed %d\n", ret);
            return MSG_FLASH_FAIL;
        }

        // flash all_models.bin
        ret = dfu_mem_to_flash_4k_blocks(ddr_buf+info_size, MODEL_ALL_BIN_FLASH_ADDR, model_size);
        if (ret == -1) {
            err_msg("Error: flash all_models.bin failed %d\n", ret);
            return MSG_FLASH_FAIL;
        }

        ret = dfu_post_flash_verify_4kblock((u32)MODEL_ALL_BIN_FLASH_ADDR, model_size,
                                            (u8 *)(ddr_buf+info_size));
        if (ret != 0) {
            err_msg("Error: verify all_models.bin failed %d\n", ret);
            return MSG_FLASH_FAIL;
        }
    }

    err_msg("kmdw_dfu_update_model: flashing done\n");
    return SUCCESS;

}

int kmdw_dfu_update_spl(u32 file_size)
{
    u32 ddr_buf, size = file_size;
    int ret;
    u16 tmp;

    dbg_msg("kmdw_dfu_spl: flashing ...\n");
    dfu_pre_update();

    ddr_buf = DDR_BEGIN;
    ret = fn_read_data(ddr_buf, size);
    if (ret != size) {  // spl has no checksum
        dfu_update_abort(0);
        return MSG_DATA_ERROR;
    }

    if ((size % VERIFY_BLK_SZ) != 0) {
        tmp = (size + VERIFY_BLK_SZ - 1) / VERIFY_BLK_SZ;
        size = tmp * VERIFY_BLK_SZ;
    }

    // flash data
    ret = dfu_mem_to_flash_4k_blocks(ddr_buf, 0, size);
    if (ret == -1) {
        dfu_update_abort(0);
        return MSG_FLASH_FAIL;
    }

    ret = dfu_post_flash_verify_4kblock(0, size, (u8 *)ddr_buf);
    dfu_update_abort(0);
    if (ret != 0) {
        err_msg("Error: post flash verification failed\n");
        return MSG_FLASH_FAIL;
    }

    dbg_msg("kmdw_dfu_update_spl: flashing done\n");
    kdp_sys_update_spl_image(ddr_buf, file_size);
    return SUCCESS;

}

int kmdw_dfu_switch_active_partition(u32 partition)
{
    u32 seq;
    int ret;

    if((partition != 1) && (partition != 2)) {
        err_msg("Error: wrong partition number\n");
        return -1;
    }

    kdev_flash_readdata(PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0, sizeof(boot_cfg_0));
    flash_wait_ready(300);

    kdev_flash_readdata(PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1, sizeof(boot_cfg_1));
    flash_wait_ready(300);

    if ((boot_cfg_0.scpu_cfg.flag == 0xffffffff) && (boot_cfg_1.scpu_cfg.flag == 0xffffffff)) {
        // no config data is there, need to create them for partition 0/1
        dfu_init_partition_boot_cfg();
        return 0;
    }

    if (boot_cfg_0.ncpu_cfg.flag & boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED)
    {
        err_msg("Critical Error: 2 active NCPU boot config\n");
        return -1;
    }

    if ((boot_cfg_0.ncpu_cfg.partition_id == boot_cfg_1.ncpu_cfg.partition_id) 
        && (boot_cfg_0.ncpu_cfg.seq == boot_cfg_1.ncpu_cfg.seq))
    {
        // no config data is there, need to create them for partition 0/1
        dfu_init_partition_boot_cfg();
        err_msg("only one partition, cannot switch\n");
        return -1;
    }

    if (partition == 1)     //switch SCPU
    {
        /* determine if any update ever happened, if not, just return */
        if ((boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED) && 
           (boot_cfg_0.scpu_cfg.seq == 1) &&
           (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_NOT_CONFIRMED) && 
           (boot_cfg_1.scpu_cfg.seq == 0))
        {
            err_msg("Never have SCPU firmware updated, cannot switch\n");
            return -1;
        }

        if (boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);
            boot_cfg_1.scpu_cfg.seq = seq + 1;
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

        } else if (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);
            boot_cfg_0.scpu_cfg.seq = seq + 1;
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
    }

    if (partition == 2)     //switch NCPU
    {

        /* determine if any update ever happened, if not, just return */
        if ((boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED) && 
           (boot_cfg_0.ncpu_cfg.seq == 1) &&
           (boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_NOT_CONFIRMED) && 
           (boot_cfg_1.ncpu_cfg.seq == 0))
        {
            err_msg("Never have NCPU firmware updated, cannot switch\n");
            return -1;
        }

        if (boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);
            boot_cfg_1.ncpu_cfg.seq = seq + 1;
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

        } else if (boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);
            boot_cfg_0.ncpu_cfg.seq = seq + 1;
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }

    }

    ret = dfu_mem_to_flash_small_block((u32)&boot_cfg_0, PARTITION_0_CFG_START_IN_FLASH, sizeof(boot_cfg_0));
    if (ret == -1)
    {
        err_msg("Flash write fail on %x\n", PARTITION_0_CFG_START_IN_FLASH);
        return MSG_FLASH_FAIL;
    }

    ret = dfu_mem_to_flash_small_block((u32)&boot_cfg_1, PARTITION_1_CFG_START_IN_FLASH, sizeof(boot_cfg_1));
    if (ret == -1)
    {
        err_msg("Flash write fail on %x\n", PARTITION_1_CFG_START_IN_FLASH);
        return MSG_FLASH_FAIL;
    }
    return SUCCESS;
}
