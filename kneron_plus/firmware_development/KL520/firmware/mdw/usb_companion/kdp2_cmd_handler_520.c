// #define FIFO_CMD_DBG

#include <string.h>
#include <stdlib.h>

#include "cmsis_os2.h"
#include "version.h"
#include "project.h"

#include "usbd_hal.h"
#include "kdrv_scu_ext.h"
#include "kdrv_power.h"
#include "kdrv_clock.h"
#include "kdp_system.h"

#include "kmdw_dfu.h"
#include "kmdw_console.h"
#include "kmdw_model.h"
#include "kmdw_memxfer.h"



#include "kmdw_ipc.h" // for kp inference debug
#include "kdp2_ipc_cmd.h"
#include "kdp2_inf_generic_raw.h"
#include "kmdw_fifoq_manager.h"

#ifdef FIFO_CMD_DBG
#define fifo_cmd_dbg(__format__, ...) kmdw_printf("[fifoCmd]"__format__, ##__VA_ARGS__)
#else
#define fifo_cmd_dbg(__format__, ...)
#endif

#define USB_NORMAL_TIMEOUT (2 * 1000) // 2 secs
#define KP_DEBUG_BUF_SIZE (8 * 1024 * 1024) // FIXME, max is 1920x1080 RGB565

typedef struct
{
    /* Model type */
    uint32_t model_type; //defined in model_type.h

    /* Model version */
    uint32_t model_version;

    /* Input in memory */
    uint32_t input_mem_addr;
    int32_t input_mem_len;

    /* Output in memory */
    uint32_t output_mem_addr;
    int32_t output_mem_len;

    /* Working buffer */
    uint32_t buf_addr;
    int32_t buf_len;

    /* command.bin in memory */
    uint32_t cmd_mem_addr;
    int32_t cmd_mem_len;

    /* weight.bin in memory */
    uint32_t weight_mem_addr;
    int32_t weight_mem_len;

    /* setup.bin in memory */
    uint32_t setup_mem_addr;
    int32_t setup_mem_len;
} _fw_info_t; // == kdp_model_t

/* Structure of CNN Header in setup.bin - copy from kdpio.h */
struct cnn_header_s
{
    uint32_t crc;
    uint32_t version;
    uint32_t key_offset;
    uint32_t model_type;
    uint32_t app_type;
    uint32_t dram_start;
    uint32_t dram_size;
    uint32_t input_row;
    uint32_t input_col;
    uint32_t input_channel;
    uint32_t cmd_start;
    uint32_t cmd_size;
    uint32_t weight_start;
    uint32_t weight_size;
    uint32_t input_start;
    uint32_t input_size;
    uint32_t input_radix;
    uint32_t output_nums;
};

extern int kdp_memxfer_flash_to_ddr(uint32_t dst, uint32_t src, size_t bytes);
extern int kdp_memxfer_ddr_to_flash(u32 dst, u32 src, size_t bytes);

static uint32_t _flash_read_callback(uint32_t addr, uint32_t img_size)
{
    fifo_cmd_dbg("[%s]\n", __FUNCTION__);

    int usb_sts = usbd_hal_bulk_receive(KDP2_USB_ENDPOINT_DATA_OUT, (void *)addr, &img_size, USB_NORMAL_TIMEOUT);

    if (KDRV_STATUS_OK != usb_sts) {
        fifo_cmd_dbg("[%s] receive fw data failed, sts %d\n", __FUNCTION__, usb_sts);

        return 0;
    }

    fifo_cmd_dbg("[%s] received fw data %d bytes\n", __FUNCTION__, img_size);

    return img_size;
}

static int _load_model(kdp2_ipc_cmd_load_model_t *cmd_lmd)
{
    fifo_cmd_dbg("[%s] model_size %d fw_info_size %d\n", __FUNCTION__, cmd_lmd->model_size, cmd_lmd->fw_info_size);
    fifo_cmd_dbg("[%s] num_model = %d\n", __FUNCTION__, cmd_lmd->fw_info[0]);

    _fw_info_t *first_fwinfo = (_fw_info_t *)(cmd_lmd->fw_info + 4);

    kmdw_model_fw_info_t *fw_info_p = kmdw_model_get_fw_info(true);
    memcpy(fw_info_p, (void *)cmd_lmd->fw_info, cmd_lmd->fw_info_size);
    fifo_cmd_dbg("fw_info_p = 0x%x, fw_info_size = %d, sizeof(_fw_info_t) = %d\n",
	    fw_info_p, cmd_lmd->fw_info_size, sizeof(_fw_info_t));

    uint32_t reload_model_info_sts = kmdw_model_reload_model_info(true);
    uint32_t return_code = (0 < reload_model_info_sts) ? KP_SUCCESS : KP_FW_LOAD_MODEL_FAILED_104;
    kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);

    if (usb_sts != KDRV_STATUS_OK)
    {
        fifo_cmd_dbg("[%s] send reload model info statsus failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    if (return_code != KP_SUCCESS)
    {
        fifo_cmd_dbg("[%s] reload model info failed, sts %d\n", __FUNCTION__, reload_model_info_sts);
        return -1;
    }

    fifo_cmd_dbg("[%s] receiving model and writing to addr 0x%x\n", __FUNCTION__, first_fwinfo->cmd_mem_addr);

    uint32_t txLen = cmd_lmd->model_size; // should be acceptable

    fifo_cmd_dbg("[%s] max model buf size %d\n", __FUNCTION__, txLen);

    usb_sts = usbd_hal_bulk_receive(KDP2_USB_ENDPOINT_DATA_OUT, (void *)first_fwinfo->cmd_mem_addr, &txLen, 20 * 1000);
    if (usb_sts != KDRV_STATUS_OK)
    {
        fifo_cmd_dbg("[%s] receive model failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    fifo_cmd_dbg("[%s] received model done %d bytes\n", __FUNCTION__, txLen);

    return 0;
}

static uint8_t ack_packet[] = {0x35, 0x8A, 0xC, 0, 0x4, 0, 0x8, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int _update_firmware(kdp_firmware_update_cmd_t *kdp_cmd)
{
    kdrv_status_t usb_sts;
    uint32_t txLen, txLen2;
    int fw_id = kdp_cmd->fw_id;
    bool reboot_after_update = (1 == kdp_cmd->auto_reboot);
    int dfu_update_sts = -1;

    fifo_cmd_dbg("[%s] recognized KDP KDP_CMD_UPDATE_FW\n", __FUNCTION__);

    if ((KDP_UPDATE_MODULE_SCPU == fw_id) || (KDP_UPDATE_MODULE_NCPU == fw_id))
    {
        txLen = sizeof(ack_packet);
        usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)ack_packet, txLen, USB_NORMAL_TIMEOUT);
        if (usb_sts != KDRV_STATUS_OK)
        {
            fifo_cmd_dbg("[%s] send error ack failed, sts %d\n", __FUNCTION__, usb_sts);
            return -1;
        }
    }
    else
    {
        fifo_cmd_dbg("[%s] error fw_id %d\n", __FUNCTION__, fw_id);

        // send some error ack back to host
        txLen = 16;
        memset((void *)kdp_cmd, 0, txLen);
        usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)kdp_cmd, txLen, USB_NORMAL_TIMEOUT);
        if (usb_sts != KDRV_STATUS_OK)
        {
            fifo_cmd_dbg("[%s] send ack failed, sts %d\n", __FUNCTION__, usb_sts);
        }

        return -1;
    }

    // now receive fw content
    fifo_cmd_dbg("[%s] received fw data %d bytes\n", __FUNCTION__, txLen);

    if (fw_id == KDP_UPDATE_MODULE_SCPU)
    {
        fifo_cmd_dbg("[%s] updateing scpu fw\n", __FUNCTION__);
        dfu_update_sts = kmdw_dfu_update_scpu();
    }
    else if (fw_id == KDP_UPDATE_MODULE_NCPU)
    {
        fifo_cmd_dbg("[%s] updateing ncpu fw\n", __FUNCTION__);
        dfu_update_sts = kmdw_dfu_update_ncpu();
    }

    // so far so good, give a response to host
    kdp_firmware_update_response_t kdp_resp = {KDP_MSG_HDR_RSP, 0xc, KDP_CMD_UPDATE_FW_RESPONSE, 8, dfu_update_sts, fw_id};

    txLen2 = sizeof(kdp_resp);
    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&kdp_resp, txLen2, USB_NORMAL_TIMEOUT);
    if (usb_sts != KDRV_STATUS_OK)
    {
        fifo_cmd_dbg("[%s] send response failed, sts %d\n", __FUNCTION__, usb_sts);
    }

    osDelay(1000);

    if (true == reboot_after_update)
        kdrv_power_sw_reset();

    return 0;
}

static int _update_model(kdp_model_update_cmd_t *kdp_cmd)
{
    kdrv_status_t usb_sts;
    uint32_t txLen, txLen2;
    int fw_info_size = kdp_cmd->fw_info_size;
    int all_models_size = kdp_cmd->all_models_size;
    bool reboot_after_update = (1 == kdp_cmd->auto_reboot);

    fifo_cmd_dbg("[%s] recognized KDP KDP_CMD_UPDATE_MODEL\n", __FUNCTION__);

    txLen = sizeof(ack_packet);
    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)ack_packet, txLen, USB_NORMAL_TIMEOUT);

    if (KDRV_STATUS_OK != usb_sts) {
        fifo_cmd_dbg("[%s] send error ack failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    int ret = kmdw_dfu_update_model(fw_info_size, all_models_size);

    if (0 != ret) {
        fifo_cmd_dbg("[%s] write model to flash failed, sts %d\n", __FUNCTION__, ret);
    }

    // so far so good, give a response to host
    kdp_model_update_response_t kdp_resp = {KDP_MSG_HDR_RSP, 0xc, KDP_CMD_UPDATE_MODEL_RESPONSE, 8, ret, ret};

    txLen2 = sizeof(kdp_resp);
    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&kdp_resp, txLen2, USB_NORMAL_TIMEOUT);

    if (KDRV_STATUS_OK != usb_sts) {
        fifo_cmd_dbg("[%s] send response failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    fifo_cmd_dbg("[%s] update model done, sts: %d\n", __FUNCTION__, ret);

    osDelay(1000);

    if (true == reboot_after_update) {
        kdrv_power_sw_reset();
    }

    return 0;
}

static uint8_t _dfu_buf[4 * 1024]; // kmdw_dfu_init() says it need at least 4KB buffer, and it cannot use DDR, really sucks
int kdp2_cmd_handler_initialize()
{
    fifo_cmd_dbg("[%s]\n", __FUNCTION__);

    if(FLASH_TYPE != FLASH_TYPE_NULL) {
        kmdw_dfu_init(_dfu_buf, _flash_read_callback); // TODO: waste of memory
    }

    return 0;
}

static int _memory_read_write(kdp2_ipc_cmd_memory_read_write_t *cmd_buf)
{
    kdrv_status_t usb_sts;

    if (cmd_buf->command_id == KDP2_COMMAND_MEMORY_READ) // read mode
    {
        fifo_cmd_dbg("[%s] read mode\n", __FUNCTION__);

        uint32_t return_code = KP_SUCCESS; // FIXME: error handling

        usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);
        if (usb_sts != KDRV_STATUS_OK)
        {
            fifo_cmd_dbg("[%s] memory read failed, sts %d\n", __FUNCTION__, usb_sts);
            return -1;
        }

        usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)cmd_buf->start_address, cmd_buf->length, USB_NORMAL_TIMEOUT);
        if (usb_sts != KDRV_STATUS_OK)
        {
            fifo_cmd_dbg("[%s] memory read failed, sts %d\n", __FUNCTION__, usb_sts);
            return -1;
        }
    }
    else if (cmd_buf->command_id == KDP2_COMMAND_MEMORY_WRITE) // write mode
    {
        fifo_cmd_dbg("[%s] write mode\n", __FUNCTION__);

        uint32_t rxLen = cmd_buf->length;
        usb_sts = usbd_hal_bulk_receive(KDP2_USB_ENDPOINT_DATA_OUT, (void *)cmd_buf->start_address, &rxLen, USB_NORMAL_TIMEOUT);
        if (usb_sts != KDRV_STATUS_OK)
        {
            fifo_cmd_dbg("[%s] memory write failed, sts %d\n", __FUNCTION__, usb_sts);
            return -1;
        }

        uint32_t return_code = KP_SUCCESS;
        if (rxLen != cmd_buf->length)
        {
            return_code = KP_ERROR_RECEIVE_SIZE_MISMATCH_31;
            fifo_cmd_dbg("[%s] received buffer length is not the same with the length specified in the command, return code %d\n", __FUNCTION__, return_code);
        }

        usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);
        if (usb_sts != KDRV_STATUS_OK)
        {
            fifo_cmd_dbg("[%s] memory write failed, sts %d\n", __FUNCTION__, usb_sts);
            return -1;
        }
    }
    else
        fifo_cmd_dbg("[%s] unsupported command, should not be here\n", __FUNCTION__);

    return 0;
}

static int _get_system_info()
{
    kdrv_status_t usb_sts;
    kdp2_ipc_response_get_system_info_t response_buf;

    response_buf.return_code = KP_SUCCESS; // FIXME: error handling

    response_buf.system_info.kn_number = kdp_sys_get_kn_number();

    response_buf.system_info.firmware_version.reserved = 0;
    response_buf.system_info.firmware_version.major = IMG_FW_MAJOR;
    response_buf.system_info.firmware_version.minor = IMG_FW_MINOR;
    response_buf.system_info.firmware_version.update = IMG_FW_UPDATE;
    response_buf.system_info.firmware_version.build = IMG_FW_BUILD;

    fifo_cmd_dbg("[%s] kn_number 0x%8X\n", __FUNCTION__, response_buf.system_info.kn_number);

    fifo_cmd_dbg("[%s] FW: %d.%d.%d-build.%d\n",
                 __FUNCTION__,
                 response_buf.system_info.firmware_version.major,
                 response_buf.system_info.firmware_version.minor,
                 response_buf.system_info.firmware_version.update,
                 response_buf.system_info.firmware_version.build);

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&response_buf, sizeof(response_buf), USB_NORMAL_TIMEOUT);
    if (usb_sts != KDRV_STATUS_OK)
    {
        fifo_cmd_dbg("[%s] get system info failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    return 0;
}

static int _load_model_from_flash()
{
    int32_t load_model_sts = kmdw_model_load_model(-1);
    uint32_t return_code = (0 < load_model_sts) ? KP_SUCCESS : KP_FW_LOAD_MODEL_FAILED_104;
    kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);

    if (KDRV_STATUS_OK != usb_sts) {
        fifo_cmd_dbg("[%s] send reload model info statsus failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    if (KP_SUCCESS != return_code) {
        fifo_cmd_dbg("[%s] reload model info failed, sts %d\n", __FUNCTION__, load_model_sts);
        return -1;
    }

    return 0;
}

static int _read_flash(kdp2_ipc_cmd_read_flash_t *cmd_buf)
{
    kdrv_status_t usb_sts;
    int32_t return_code = KP_SUCCESS; // FIXME: error handling
    uint32_t buffer = (uint32_t)cmd_buf;
    uint32_t flash_addr = cmd_buf->flash_offset;
    uint32_t length = cmd_buf->length;

    fifo_cmd_dbg("[%s] Read flash on addr %d\n", __FUNCTION__, flash_addr);

    kdp_memxfer_flash_to_ddr(buffer, flash_addr, length);

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);
    if (usb_sts != KDRV_STATUS_OK) {
        fifo_cmd_dbg("[%s] memory read failed, sts %d\n", __FUNCTION__, usb_sts);
        return_code = -1;
        goto FUNC_OUT;
    }

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)buffer, length, USB_NORMAL_TIMEOUT);
    if (usb_sts != KDRV_STATUS_OK) {
        fifo_cmd_dbg("[%s] memory read failed, sts %d\n", __FUNCTION__, usb_sts);
        return_code = -1;
        goto FUNC_OUT;
    }

FUNC_OUT:

    return return_code;
}

static int _write_flash(kdp2_ipc_cmd_write_flash_t *cmd_buf)
{
    kdrv_status_t usb_sts;
    int32_t return_code = KP_SUCCESS; // FIXME: error handling
    uint32_t buffer = (uint32_t)cmd_buf;
    uint32_t flash_addr = cmd_buf->flash_offset;
    uint32_t length = cmd_buf->length;

    fifo_cmd_dbg("[%s] Write flash on addr %d\n", __FUNCTION__, flash_addr);

    usb_sts = usbd_hal_bulk_receive(KDP2_USB_ENDPOINT_DATA_OUT, (void *)buffer, &length, USB_NORMAL_TIMEOUT);

    if (usb_sts != KDRV_STATUS_OK) {
        fifo_cmd_dbg("[%s] receive data failed, sts %d\n", __FUNCTION__, usb_sts);
        return_code = -1;
        goto FUNC_OUT;
    }

    kdp_memxfer_ddr_to_flash(flash_addr, buffer, length);

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);
    if (usb_sts != KDRV_STATUS_OK) {
        fifo_cmd_dbg("[%s] send return code failed, sts %d\n", __FUNCTION__, usb_sts);
        return_code = -1;
        goto FUNC_OUT;
    }

FUNC_OUT:

    return return_code;
}

#define OUT_NODE_HEAD_SIZE 20 // node's width, height, channel, radix, scale

static int _get_model_info(kdp2_ipc_cmd_get_model_info_t *cmd_buf)
{
    kdrv_status_t usb_sts;

    kdp2_ipc_response_get_model_info_fw_info_t fw_info_response_buf = {0};
    kmdw_model_fw_info_t *fw_info_buf_p = kmdw_model_get_fw_info(false);

    if (NULL == fw_info_buf_p)
    {
        fw_info_response_buf.return_code = KP_FW_GET_MODEL_INFO_FAILED_109;
        fw_info_response_buf.fw_info_size = 0;
        fw_info_response_buf.target_chip = KP_MODEL_TARGET_CHIP_KL520;
    }
    else
    {
        fw_info_response_buf.return_code = KP_SUCCESS;
        fw_info_response_buf.fw_info_size = sizeof(kmdw_model_fw_info_t) - sizeof(struct kdp_model_s) + (fw_info_buf_p->model_count * sizeof(struct kdp_model_s)) + sizeof(kmdw_model_fw_info_ext_t);
        fw_info_response_buf.target_chip = KP_MODEL_TARGET_CHIP_KL520;
    }

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&fw_info_response_buf, sizeof(fw_info_response_buf), USB_NORMAL_TIMEOUT);
    if ((usb_sts != KDRV_STATUS_OK) || (fw_info_response_buf.return_code != KP_SUCCESS))
    {
        fifo_cmd_dbg("[%s] get model info send fw_info response failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)fw_info_buf_p, fw_info_response_buf.fw_info_size, USB_NORMAL_TIMEOUT);
    if (usb_sts != KDRV_STATUS_OK)
    {
        fifo_cmd_dbg("[%s] get model info send fw_info data failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    kdp2_ipc_response_get_model_info_setup_t all_setup_response_buf = {0};
    kdp2_ipc_response_get_model_info_setup_t single_setup_response_buf = {0};
    struct kdp_model_s *model_info = NULL;

    all_setup_response_buf.setup_size = 0;
    all_setup_response_buf.return_code = KP_SUCCESS;

    for (int i = 0; i < fw_info_buf_p->model_count; i++) {
        model_info = kmdw_model_get_model_info(i);

        if (NULL != model_info) {
            all_setup_response_buf.setup_size += model_info->setup_mem_len;
        } else {
            all_setup_response_buf.return_code = KP_FW_GET_MODEL_INFO_FAILED_109;
            fifo_cmd_dbg("[%s] get model info failed, invalid model info in ddr\n", __FUNCTION__);
            break;
        }
    }

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&all_setup_response_buf, sizeof(all_setup_response_buf), USB_NORMAL_TIMEOUT);
    if ((usb_sts != KDRV_STATUS_OK) || (all_setup_response_buf.return_code != KP_SUCCESS))
    {
        fifo_cmd_dbg("[%s] get model info send all setup response failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    for (int i = 0; i < fw_info_buf_p->model_count; i++) {
        model_info = kmdw_model_get_model_info(i);

        if (NULL != model_info) {
            single_setup_response_buf.setup_size = model_info->setup_mem_len;
            single_setup_response_buf.return_code = KP_SUCCESS;
        } else {
            single_setup_response_buf.setup_size = 0;
            single_setup_response_buf.return_code = KP_FW_GET_MODEL_INFO_FAILED_109;
        }

        usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&single_setup_response_buf, sizeof(single_setup_response_buf), USB_NORMAL_TIMEOUT);
        if ((usb_sts != KDRV_STATUS_OK) || (single_setup_response_buf.return_code != KP_SUCCESS))
        {
            fifo_cmd_dbg("[%s] get model info send single setup response failed, sts %d\n", __FUNCTION__, usb_sts);
            return -1;
        }

        usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)model_info->setup_mem_addr, model_info->setup_mem_len, USB_NORMAL_TIMEOUT);
        if (usb_sts != KDRV_STATUS_OK)
        {
            fifo_cmd_dbg("[%s] get model info send setup data entry %d failed, sts %d\n", __FUNCTION__, i, usb_sts);
            return -1;
        }
    }

    return 0;
}

#define NCPU_IRAM_ADDR (0x28000000)
#define NCPU_IRAM_SIZE (64 * 1024)

static int _load_firmware(kdp2_ipc_cmd_upload_firmware_t *cmd_buf)
{
    uint32_t txLen = NCPU_IRAM_SIZE;
    uint32_t recv_buf;

    static bool isNCPU_loaded = false;

    if (isNCPU_loaded == false)
        recv_buf = NCPU_IRAM_ADDR;
    else
        // useless, just to recv it
        recv_buf = (uint32_t)cmd_buf + sizeof(kdp2_ipc_cmd_upload_firmware_t);

    kdrv_status_t usb_sts = usbd_hal_bulk_receive(KDP2_USB_ENDPOINT_DATA_OUT, (uint32_t *)recv_buf, &txLen, USB_NORMAL_TIMEOUT);
    if (usb_sts != KDRV_STATUS_OK)
    {
        fifo_cmd_dbg("[%s] receive fw failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    if (isNCPU_loaded)
        return 0;

    kmdw_printf("recv ncpu fw %d\n", txLen);
    osDelay(10);
    SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1); // restart ncpu

    isNCPU_loaded = true;

    return 0;
}

static int _set_ckey(kdp2_ipc_cmd_set_ckey_t *cmd_buf)
{
    kdrv_status_t usb_sts;
    uint32_t ckey = cmd_buf->ckey;
    uint32_t status = 0;
    int32_t resp = 0;

    fifo_cmd_dbg("[%s] set ckey : 0x%08X\n", __FUNCTION__, ckey);

    status = kdp_sys_program_key(ckey);

    switch (status)
    {
    case 0x1:
        resp = KP_FW_EFUSE_CAN_NOT_BURN_300;
        break;
    case 0x2:
        resp = KP_FW_EFUSE_PROTECTED_301;
        break;
    case 0x0:
        resp = KP_SUCCESS;
        break;
    default:
        resp = KP_FW_EFUSE_OTHER_302;
        break;
    }

    usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&resp, sizeof(int32_t), USB_NORMAL_TIMEOUT);

    if (KDRV_STATUS_OK != usb_sts) {
        fifo_cmd_dbg("[%s] send response failed, sts %d\n", __FUNCTION__, usb_sts);
        return -1;
    }

    fifo_cmd_dbg("[%s] set ckey done, sts: %d\n", __FUNCTION__, resp);

    return 0;
}

static int _set_dbg_checkpoint(kdp2_ipc_cmd_set_dbg_checkpoint_t *cmd_buf)
{
    // kp inference debug code
    struct scpu_to_ncpu_s *out_comm = kmdw_ipc_get_output();
    int32_t return_code = KP_SUCCESS;

    if (out_comm->kp_dbg_buffer == NULL)
    {
        out_comm->kp_dbg_buffer = (void *)kmdw_ddr_reserve(KP_DEBUG_BUF_SIZE);
        if (out_comm->kp_dbg_buffer == NULL)
        {
            return_code = KP_FW_DDR_MALLOC_FAILED_102;
        }
    }

    kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);
    if (KDRV_STATUS_OK != usb_sts)
        fifo_cmd_dbg("[%s] send ack failed, sts %d\n", __FUNCTION__, usb_sts);

    uint32_t checkponit_flags = out_comm->kp_dbg_checkpoinots;

    if (cmd_buf->enable)
        checkponit_flags |= cmd_buf->checkpoint_flags;
    else
        checkponit_flags &= ~cmd_buf->checkpoint_flags;

    out_comm->kp_dbg_checkpoinots = checkponit_flags;

    return 0;
}

static int _set_dbg_profile(kdp2_ipc_cmd_set_profile_enable_t *cmd_buf)
{
    struct scpu_to_ncpu_s *out_comm = kmdw_ipc_get_output();

    out_comm->kp_dbg_enable_profile = (cmd_buf->enable) ? 1 : 0;
    if (cmd_buf->enable)
    {
        kp_model_profile_t *profile_recs = (kp_model_profile_t *)out_comm->kp_model_profile_records;
        memset(profile_recs, 0, MULTI_MODEL_MAX * sizeof(kp_model_profile_t));
    }

    int32_t return_code = KP_SUCCESS;
    kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&return_code, sizeof(uint32_t), USB_NORMAL_TIMEOUT);
    if (KDRV_STATUS_OK != usb_sts)
        fifo_cmd_dbg("[%s] send ack failed, sts %d\n", __FUNCTION__, usb_sts);

    return 0;
}

static int _get_dbg_profile(kdp2_ipc_cmd_get_profile_statics_t *cmd_buf)
{
    struct scpu_to_ncpu_s *out_comm = kmdw_ipc_get_output();

    kp_profile_data_t pd = {0};

    kp_model_profile_t *profile_recs = (kp_model_profile_t *)out_comm->kp_model_profile_records;
    for (int i = 0; i < MULTI_MODEL_MAX; i++)
    {
        if (profile_recs[i].model_id == 0)
            break;

        pd.num_model_profiled++;
        pd.model_st[i].model_id = profile_recs[i].model_id;
        pd.model_st[i].inf_count = profile_recs[i].sum_frame_count;
        pd.model_st[i].cpu_op_count = profile_recs[i].sum_cpu_op_count / profile_recs[i].sum_frame_count;
        pd.model_st[i].avg_pre_process_ms = (float)profile_recs[i].sum_ticks_preprocess / profile_recs[i].sum_frame_count;
        pd.model_st[i].avg_inference_ms = (float)profile_recs[i].sum_ticks_inference / profile_recs[i].sum_frame_count;
        pd.model_st[i].avg_cpu_op_ms = (float)profile_recs[i].sum_ticks_cpu_op / profile_recs[i].sum_frame_count;
        pd.model_st[i].avg_cpu_op_per_cpu_node_ms = (float)profile_recs[i].sum_ticks_cpu_op / (0 == profile_recs[i].sum_cpu_op_count ? 1 : profile_recs[i].sum_cpu_op_count);
        pd.model_st[i].avg_post_process_ms = (float)profile_recs[i].sum_ticks_postprocess / profile_recs[i].sum_frame_count;
    }

    kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&pd, sizeof(kp_profile_data_t), USB_NORMAL_TIMEOUT);
    if (KDRV_STATUS_OK != usb_sts)
        fifo_cmd_dbg("[%s] send ack failed, sts %d\n", __FUNCTION__, usb_sts);

    return 0;
}

static int _get_ddr_config(kdp2_ipc_cmd_get_available_ddr_config_t *cmd_buf)
{
    kp_available_ddr_config_t ddr_config = {0};

    ddr_config.ddr_available_begin = DDR_BEGIN;
    ddr_config.ddr_available_end = kmdw_ddr_get_heap_tail();
    ddr_config.ddr_model_end = kmdw_model_get_model_end_addr(true);
    ddr_config.ddr_fifoq_allocated = (true == kmdw_fifoq_manager_get_fifoq_allocated()) ? 1 : 0;

    kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&ddr_config, sizeof(kp_available_ddr_config_t), USB_NORMAL_TIMEOUT);
    if (KDRV_STATUS_OK != usb_sts)
        fifo_cmd_dbg("[%s] send ack failed, sts %d\n", __FUNCTION__, usb_sts);

    return 0;
}

static int _get_fifo_queue_config(kdp2_ipc_cmd_get_fifo_queue_config_t *cmd_buf)
{
    kp_fifo_queue_config_t fifo_queue_config = {0};

    if (true == kmdw_fifoq_manager_get_fifoq_allocated()) {
        kmdw_fifoq_manager_get_fifoq_config(&fifo_queue_config.fifoq_input_buf_count, &fifo_queue_config.fifoq_input_buf_size,
                                            &fifo_queue_config.fifoq_result_buf_count, &fifo_queue_config.fifoq_result_buf_size);
    }

    kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&fifo_queue_config, sizeof(kp_fifo_queue_config_t), USB_NORMAL_TIMEOUT);
    if (KDRV_STATUS_OK != usb_sts)
        fifo_cmd_dbg("[%s] send ack failed, sts %d\n", __FUNCTION__, usb_sts);

    return 0;
}

int kdp2_cmd_handle_kp_command(uint32_t command_buffer)
{
    int ret = -1;
    kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)command_buffer;

    uint32_t command_id = header_stamp->job_id;

    switch (command_id)
    {
    case KDP2_COMMAND_LOAD_MODEL:
        ret = _load_model((kdp2_ipc_cmd_load_model_t *)command_buffer);
        break;
    case KDP2_COMMAND_MEMORY_READ:
    case KDP2_COMMAND_MEMORY_WRITE:
        ret = _memory_read_write((kdp2_ipc_cmd_memory_read_write_t *)command_buffer);
        break;
    case KDP2_COMMAND_GET_SYSTEM_INFO:
        ret = _get_system_info(); // no need to pass argument since we already parsed all useful info
        break;
    case KDP2_COMMAND_LOAD_MODEL_FROM_FLASH:
        ret = _load_model_from_flash(); // no need to pass argument since we already parsed all useful info
        break;
    case KDP2_COMMAND_READ_FLASH:
        ret = _read_flash((kdp2_ipc_cmd_read_flash_t *)command_buffer);
        break;
    case KDP2_COMMAND_WRITE_FLASH:
        ret = _write_flash((kdp2_ipc_cmd_write_flash_t *)command_buffer);
        break;
    case KDP2_COMMAND_GET_MODEL_INFO:
        ret = _get_model_info((kdp2_ipc_cmd_get_model_info_t *)command_buffer);
        break;
    case KDP2_COMMAND_LOAD_FIRMWARE:
        ret = _load_firmware((kdp2_ipc_cmd_upload_firmware_t *)command_buffer);
        break;
    case KDP2_COMMAND_SET_CKEY:
        ret = _set_ckey((kdp2_ipc_cmd_set_ckey_t *)command_buffer);
    case KDP2_COMMAND_SET_DBG_CHECKPOINT:
        ret = _set_dbg_checkpoint((kdp2_ipc_cmd_set_dbg_checkpoint_t *)command_buffer);
        break;
    case KDP2_COMMAND_SET_PROFILE_ENABLE:
        ret = _set_dbg_profile((kdp2_ipc_cmd_set_profile_enable_t *)command_buffer);
        break;
    case KDP2_COMMAND_GET_PROFILE_STATISTICS:
        ret = _get_dbg_profile((kdp2_ipc_cmd_get_profile_statics_t *)command_buffer);
        break;
    case KDP2_COMMAND_GET_DDR_CONFIG:
        ret = _get_ddr_config((kdp2_ipc_cmd_get_available_ddr_config_t *)command_buffer);
        break;
    case KDP2_COMMAND_GET_FIFOQ_CONFIG:
        ret = _get_fifo_queue_config((kdp2_ipc_cmd_get_fifo_queue_config_t *)command_buffer);
        break;
    default:
        kmdw_printf("error ! unknown command id %d\n", command_id);
        break;
    }

    return ret;
}

int kdp2_cmd_handle_legend_kdp_command(uint32_t command_buffer)
{
    fifo_cmd_dbg("[%s]\n", __FUNCTION__);

    int Ret = -1;
    uint16_t cmd = *((uint16_t *)command_buffer + 2);
    fifo_cmd_dbg("kdp_cmd->cmd: %d\n", cmd);

    switch (cmd)
    {
        case KDP_CMD_UPDATE_FW:
            Ret = _update_firmware((kdp_firmware_update_cmd_t *)command_buffer);
            break;
        case KDP_CMD_UPDATE_MODEL:
            Ret = _update_model((kdp_model_update_cmd_t *)command_buffer);
            break;
        default:
            fifo_cmd_dbg("[%s] not recognized KDP command %d, refused to handle it !\n", __FUNCTION__, cmd);

            // send some error ack back to host
            uint32_t txLen = 16;
            memset((void *)command_buffer, 0, txLen);
            kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)command_buffer, txLen, USB_NORMAL_TIMEOUT);

            if (KDRV_STATUS_OK != usb_sts) {
                fifo_cmd_dbg("[%s] send ack failed, sts %d\n", __FUNCTION__, usb_sts);
            }

            break;
    }

    return Ret;
}
