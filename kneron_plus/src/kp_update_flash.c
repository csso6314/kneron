/**
 * @file        kp_update_flash.c
 * @brief       internal flash read write functions
 * @version     1.1
 * @date        2021-07-19
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

// #define DEBUG_PRINT

#include <pthread.h>

#include "kp_update_flash.h"
#include "kp_internal.h"
#include "kp_core.h"
#include "kdp2_ipc_cmd.h"
#include "kdp2_inf_generic_raw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "internal_func.h"

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }
#else
#define dbg_print(format, ...)
#endif

// This delay seems like a must especailly while with Ubuntu + usb hub
#ifdef _WIN32
#define USB_DISCONNECT_WAIT_DELAY_US (100 * 1000)
#else
#define USB_DISCONNECT_WAIT_DELAY_US (300 * 1000)
#endif

#define USB_REBOOT_WAIT_DELAY_US (3000 * 1000)

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    int fw_id;
    uint8_t *fw_buf;
    int fw_size;
    int timeout;
    bool auto_reboot;
    int sts;
} _update_kdp2_firmware_package;

int kp_write_data_to_flash(kp_usb_device_t *ll_dev, int timeout, uint32_t flash_offset,
                           uint32_t length, uint8_t *buffer);

static int check_usb_read_data_error(int ret)
{
    if (ret == KP_USB_USB_TIMEOUT)
    {
        dbg_print("[%s] Receive response from FW timeout\n", __func__);
        return KP_ERROR_USB_TIMEOUT_N7;
    }
    else if (ret < KP_USB_RET_OK)
    {
        dbg_print("[%s] Receive response from FW error. kp_usb_read_data() return code = [%d]\n", __func__, ret);
        return KP_ERROR_RECV_DATA_FAIL_17;
    }

    return KP_SUCCESS;
}

static int check_usb_write_data_error(int ret)
{
    if (ret == KP_USB_USB_TIMEOUT)
    {
        dbg_print("[%s] Write command to FIFO queue timeout\n", __func__);
        return KP_ERROR_USB_TIMEOUT_N7;
    }
    if (ret != KP_USB_RET_OK)
    {
        dbg_print("[%s] Write command to FIFO queue error. kp_usb_write_data() return code = [%d]\n", __func__, ret);
        return KP_ERROR_SEND_DATA_FAIL_14;
    }

    return KP_SUCCESS;
}

static void *_update_kdp2_firmware_to_single_device(void *data)
{
    _update_kdp2_firmware_package *cmd_pack = (_update_kdp2_firmware_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;
    int ret = -1;

    if (KP_DEVICE_KL520 == ll_dev->dev_descp.product_id) {
        uint32_t flash_offset = (1 == cmd_pack->fw_id) ? 0x00030000 : 0x00068000;

        if (1 == cmd_pack->fw_id) {
            char boot_type[8] = "FLASH-BT";

            dbg_print("[%s][%d] Start switching to flash boot\n", __FUNCTION__, cmd_pack->dev_idx);

            ret = kp_write_data_to_flash(ll_dev, cmd_pack->timeout, 0x00029000, 8, (uint8_t *)boot_type);

            if (KP_SUCCESS != ret) {
                cmd_pack->sts = ret;
                goto FUNC_OUT;
            }
        }

        dbg_print("[%s][%d] device %p, fw_buf 0x%p, fw_size %d, fw_id %d, timeout %d\n", __FUNCTION__, cmd_pack->dev_idx,
                ll_dev, cmd_pack->fw_buf, cmd_pack->fw_size, cmd_pack->fw_id, cmd_pack->timeout);

        ret = kp_write_data_to_flash(ll_dev, cmd_pack->timeout, flash_offset, cmd_pack->fw_size, (uint8_t *)cmd_pack->fw_buf);
    } else if (KP_DEVICE_KL630 == ll_dev->dev_descp.product_id) {
        /** update firmware */
        kdp2_ipc_cmd_update_firmware_t cmd_update_firmware_buf = {0};
        int status;

        cmd_update_firmware_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
        cmd_update_firmware_buf.total_size = sizeof(kdp2_ipc_cmd_update_firmware_t);
        cmd_update_firmware_buf.command_id = KDP2_COMMAND_UPDATE_FIRMWARE;
        cmd_update_firmware_buf.firmware_id = cmd_pack->fw_id;
        cmd_update_firmware_buf.firmware_size = cmd_pack->fw_size;

        status = kp_usb_write_data(ll_dev, (void *)&cmd_update_firmware_buf, cmd_update_firmware_buf.total_size, cmd_pack->timeout);
        ret = check_usb_write_data_error(status);
        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }

        status = kp_usb_write_data(ll_dev, (void *)cmd_pack->fw_buf, cmd_pack->fw_size, cmd_pack->timeout);
        ret = check_usb_write_data_error(status);
        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }

        status = kp_usb_read_data(ll_dev, (void *)&ret, sizeof(int32_t), cmd_pack->timeout);
        status = check_usb_read_data_error(status);
        if (KP_SUCCESS != status) {
            cmd_pack->sts = status;
            goto FUNC_OUT;
        }

        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }

        /** switch boot-mode */
        kdp2_ipc_cmd_switch_boot_mode_t cmd_switch_boot_mode_buf = {0};

        cmd_switch_boot_mode_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
        cmd_switch_boot_mode_buf.total_size = sizeof(kdp2_ipc_cmd_switch_boot_mode_t);
        cmd_switch_boot_mode_buf.command_id = KDP2_COMMAND_SWITCH_BOOT_MODE;
        cmd_switch_boot_mode_buf.boot_mode = BOOT_MODE_FLASH;

        status = kp_usb_write_data(ll_dev, (void *)&cmd_switch_boot_mode_buf, cmd_switch_boot_mode_buf.total_size, cmd_pack->timeout);
        ret = check_usb_write_data_error(status);
        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }

        status = kp_usb_read_data(ll_dev, (void *)&ret, sizeof(int32_t), cmd_pack->timeout);
        status = check_usb_read_data_error(status);
        if (KP_SUCCESS !=  status) {
            cmd_pack->sts = status;
            goto FUNC_OUT;
        }
    } else {
        ret = KP_ERROR_UNSUPPORTED_DEVICE_44;
    }

    cmd_pack->sts = ret;
    if (KP_SUCCESS != cmd_pack->sts)
        goto FUNC_OUT;

    if (true == cmd_pack->auto_reboot)
    {
        kp_usb_control_t kctrl = {KDP2_CONTROL_REBOOT, 0, 0};
        int ret = kp_usb_control(ll_dev, &kctrl, cmd_pack->timeout);
        if ((ret != KP_USB_USB_PIPE) &&
            (ret != KP_USB_USB_IO) &&
            (ret != KP_USB_USB_NO_DEVICE) &&
            (ret != KP_USB_USB_NOT_FOUND)) // FIXEME: trikcy way
        {
            cmd_pack->sts = KP_ERROR_RESET_FAILED_25;
            return NULL;
        }
    }

FUNC_OUT:

    usleep(USB_DISCONNECT_WAIT_DELAY_US);
    kp_usb_disconnect_device(ll_dev);
    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    cmd_pack->ll_device = NULL;

    return NULL;
}

static void *_update_kdp2_loader_to_single_device(void *data)
{
    _update_kdp2_firmware_package *cmd_pack = (_update_kdp2_firmware_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;
    int ret = -1;

    if (KP_DEVICE_KL630 == ll_dev->dev_descp.product_id) {
        /** update loader */
        kdp2_ipc_cmd_update_loader_t cmd_update_loader_buf = {0};
        int status;

        cmd_update_loader_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
        cmd_update_loader_buf.total_size = sizeof(kdp2_ipc_cmd_update_loader_t);
        cmd_update_loader_buf.command_id = KDP2_COMMAND_UPDATE_LOADER;
        cmd_update_loader_buf.loader_size = cmd_pack->fw_size;

        status = kp_usb_write_data(ll_dev, (void *)&cmd_update_loader_buf, cmd_update_loader_buf.total_size, cmd_pack->timeout);
        ret = check_usb_write_data_error(status);
        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }

        status = kp_usb_write_data(ll_dev, (void *)cmd_pack->fw_buf, cmd_pack->fw_size, cmd_pack->timeout);
        ret = check_usb_write_data_error(status);
        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }

        status = kp_usb_read_data(ll_dev, (void *)&ret, sizeof(int32_t), cmd_pack->timeout);
        status = check_usb_read_data_error(status);
        if (KP_SUCCESS != status) {
            cmd_pack->sts = status;
            goto FUNC_OUT;
        }

        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }
    } else {
        ret = KP_ERROR_UNSUPPORTED_DEVICE_44;
    }

    cmd_pack->sts = ret;
    if (KP_SUCCESS != cmd_pack->sts)
        goto FUNC_OUT;

    if (true == cmd_pack->auto_reboot)
    {
        kp_usb_control_t kctrl = {KDP2_CONTROL_REBOOT, 0, 0};
        int ret = kp_usb_control(ll_dev, &kctrl, cmd_pack->timeout);
        if ((ret != KP_USB_USB_PIPE) &&
            (ret != KP_USB_USB_IO) &&
            (ret != KP_USB_USB_NO_DEVICE) &&
            (ret != KP_USB_USB_NOT_FOUND)) // FIXEME: trikcy way
        {
            cmd_pack->sts = KP_ERROR_RESET_FAILED_25;
            return NULL;
        }
    }

FUNC_OUT:

    usleep(USB_DISCONNECT_WAIT_DELAY_US);
    kp_usb_disconnect_device(ll_dev);
    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    cmd_pack->ll_device = NULL;

    return NULL;
}

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    int timeout;
    bool auto_reboot;
    int sts;
} _update_kdp2_usb_boot_package;

static void *_update_single_device_to_kdp2_usb_boot(void *data)
{
    _update_kdp2_usb_boot_package *cmd_pack = (_update_kdp2_usb_boot_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;
    int ret = -1;

    if (KP_DEVICE_KL520 == ll_dev->dev_descp.product_id) {
        char buffer[8] = "USB-BT..";
        ret = kp_write_data_to_flash(ll_dev, cmd_pack->timeout, 0x00029000, 8, (uint8_t *)buffer);

        if (KP_SUCCESS != ret) {
            dbg_print("[%s][%d] write cmd_buf failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }
    } else if (KP_DEVICE_KL630 == ll_dev->dev_descp.product_id) {
        kdp2_ipc_cmd_switch_boot_mode_t cmd_buf = {0};
        int status;

        cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
        cmd_buf.total_size = sizeof(kdp2_ipc_cmd_switch_boot_mode_t);
        cmd_buf.command_id = KDP2_COMMAND_SWITCH_BOOT_MODE;
        cmd_buf.boot_mode = BOOT_MODE_USB;

        status = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, cmd_pack->timeout);
        ret = check_usb_write_data_error(status);
        if (KP_SUCCESS != ret) {
            cmd_pack->sts = ret;
            goto FUNC_OUT;
        }

        status = kp_usb_read_data(ll_dev, (void *)&ret, sizeof(int32_t), cmd_pack->timeout);
        status = check_usb_read_data_error(status);
        if (KP_SUCCESS !=  status) {
            cmd_pack->sts = status;
            goto FUNC_OUT;
        }
    } else {
        ret = KP_ERROR_UNSUPPORTED_DEVICE_44;
    }

    cmd_pack->sts = ret;
    if (KP_SUCCESS != cmd_pack->sts)
        goto FUNC_OUT;

    if (true == cmd_pack->auto_reboot) {
        kp_usb_control_t kctrl = {KDP2_CONTROL_REBOOT, 0, 0};
        ret = kp_usb_control(ll_dev, &kctrl, cmd_pack->timeout);

        if ((ret != KP_USB_USB_PIPE) &&
            (ret != KP_USB_USB_IO) &&
            (ret != KP_USB_USB_NO_DEVICE) &&
            (ret != KP_USB_USB_NOT_FOUND)) {
            cmd_pack->sts = KP_ERROR_RESET_FAILED_25;
            return NULL;
        }
    }

FUNC_OUT:

    usleep(USB_DISCONNECT_WAIT_DELAY_US);
    kp_usb_disconnect_device(ll_dev);
    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    cmd_pack->ll_device = NULL;

    return NULL;
}

// This ACK packet is from Firmware for KDP old arch.
static uint8_t ack_packet[] = {0x35, 0x8A, 0xC, 0, 0x4, 0, 0x8, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ACK_RESPONSE_LEN 16

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    kdp_firmware_update_cmd_t *cmd_buf;
    void *fw_buf;
    int fw_size;
    int timeout;
    int sts;
} _update_kdp_firmware_command_package;

static void *_update_kdp_firmware_to_single_device(void *data)
{
    _update_kdp_firmware_command_package *cmd_pack = (_update_kdp_firmware_command_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;

    dbg_print("[%s][%d] device %p, cmd_buf %p, fw_buf 0x%p, fw_id %d, timeout %d\n", __FUNCTION__, cmd_pack->dev_idx,
              ll_dev, (void *)cmd_pack->cmd_buf, cmd_pack->fw_buf, cmd_pack->cmd_buf->fw_id, cmd_pack->timeout);

    int ret = kp_usb_write_data(ll_dev, cmd_pack->cmd_buf, sizeof(kdp_firmware_update_cmd_t), cmd_pack->timeout);

    if (KP_USB_RET_OK != ret)
    {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write cmd_buf failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }

    uint8_t response[ACK_RESPONSE_LEN];
    int recv_len = kp_usb_read_data(ll_dev, response, ACK_RESPONSE_LEN, cmd_pack->timeout);

    if (0 > recv_len)
    {
        cmd_pack->sts = recv_len;
        dbg_print("[%s][%d] read ack failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }
    else if ((ACK_RESPONSE_LEN != recv_len) || (0 != memcmp(ack_packet, response, ACK_RESPONSE_LEN)))
    {
        cmd_pack->sts = KP_ERROR_OTHER_99;
        dbg_print("[%s][%d] ack is not correct, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }

    ret = kp_usb_write_data(ll_dev, cmd_pack->fw_buf, cmd_pack->fw_size, cmd_pack->timeout);

    if (KP_USB_RET_OK != ret)
    {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write firmware data failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }

    kdp_firmware_update_response_t cmd_resp;

    recv_len = kp_usb_read_data(ll_dev, (void *)&cmd_resp, sizeof(cmd_resp), cmd_pack->timeout);

    if (recv_len < 0)
    {
        cmd_pack->sts = recv_len;
        dbg_print("[%s][%d] read response failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }
    else if (cmd_resp.preamble != KDP_MSG_HDR_RSP || cmd_resp.cmd != KDP_CMD_UPDATE_FW_RESPONSE)
    {
        cmd_pack->sts = KP_ERROR_OTHER_99;
        dbg_print("[%s][%d] response is not correct, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }
    else if (cmd_resp.rsp_code != 0 || cmd_resp.fw_id != cmd_pack->cmd_buf->fw_id)
    {
        cmd_pack->sts = cmd_resp.rsp_code;
        dbg_print("[%s][%d] response is not correct, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }

    dbg_print("[%s][%d] firmware write OK, now disconnect device\n", __FUNCTION__, cmd_pack->dev_idx);
    cmd_pack->sts = KP_SUCCESS; // all done, successfull

FUNC_OUT:

    // at here, device should be reboot, host should disconnect it
    usleep(USB_DISCONNECT_WAIT_DELAY_US);
    kp_usb_disconnect_device(ll_dev);
    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    cmd_pack->ll_device = NULL;

    return NULL;
}

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    kdp_model_update_cmd_t *cmd_buf;
    void *total_model_buf;
    int total_model_size;
    int timeout;
    int sts;
} _update_model_command_package;

static void *_update_model_to_single_device(void *data)
{
    _update_model_command_package *cmd_pack = (_update_model_command_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;

    dbg_print("[%s] thread : device %p, cmd_buf %p, fw_info_size %d, all_models_size %d, timeout %d\n", __FUNCTION__,
              cmd_pack->ll_device, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->fw_info_size,
              cmd_pack->cmd_buf->all_models_size, cmd_pack->timeout);

    int ret = kp_usb_write_data(ll_dev, cmd_pack->cmd_buf, sizeof(kdp_model_update_cmd_t), cmd_pack->timeout);

    if (KP_USB_RET_OK != ret) {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write cmd_buf failed, error %d\n", __FUNCTION__, ll_dev->dev_descp.port_id, cmd_pack->sts);
        return NULL;
    }

    uint8_t response[ACK_RESPONSE_LEN];
    int recv_len = kp_usb_read_data(ll_dev, response, ACK_RESPONSE_LEN, cmd_pack->timeout);

    if (0 > recv_len) {
        cmd_pack->sts = recv_len;
        dbg_print("[%s][%d] read ack failed, error %d\n", __FUNCTION__, ll_dev->dev_descp.port_id, cmd_pack->sts);
        return NULL;
    } else if ((ACK_RESPONSE_LEN != recv_len) || (0 != memcmp(ack_packet, response, ACK_RESPONSE_LEN))) {
        cmd_pack->sts = KP_ERROR_OTHER_99;
        dbg_print("[%s][%d] ack is not correct, error %d\n", __FUNCTION__, ll_dev->dev_descp.port_id, cmd_pack->sts);
        return NULL;
    }

    dbg_print("[%s][%d] start wirte model to flash\n", __FUNCTION__, cmd_pack->dev_idx);

    ret = kp_usb_write_data(ll_dev, cmd_pack->total_model_buf, cmd_pack->total_model_size, cmd_pack->timeout);

    if (KP_USB_RET_OK != ret) {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write model data failed, error %d\n", __FUNCTION__, ll_dev->dev_descp.port_id, cmd_pack->sts);
        return NULL;
    }

    kdp_model_update_response_t cmd_resp;

    recv_len = kp_usb_read_data(ll_dev, (void *)&cmd_resp, sizeof(cmd_resp), cmd_pack->timeout);

    if (recv_len < 0) {
        cmd_pack->sts = recv_len;
        dbg_print("[%s][%d] read response failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        return NULL;
    } else if (cmd_resp.preamble != KDP_MSG_HDR_RSP || cmd_resp.cmd != KDP_CMD_UPDATE_MODEL_RESPONSE) {
        cmd_pack->sts = KP_ERROR_OTHER_99;
        dbg_print("[%s][%d] response is not correct, preamble %d, resp_cmd %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_resp.preamble, cmd_resp.cmd);
        return NULL;
    } else if (cmd_resp.rsp_code != 0) {
        cmd_pack->sts = KP_ERROR_DEVICE_INCORRECT_RESPONSE_11;
        dbg_print("[%s][%d] response is not correct, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        return NULL;
    }

    dbg_print("[%s][%d] model write OK, now disconnect device %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->dev_idx);

    // at here, device should be reboot, host should disconnect it
    usleep(USB_DISCONNECT_WAIT_DELAY_US);
    kp_usb_disconnect_device(ll_dev);
    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    cmd_pack->ll_device = NULL;
    cmd_pack->sts = KP_SUCCESS;

    return NULL;
}

typedef struct
{
    kp_usb_device_t *ll_device;
    kdp2_ipc_cmd_update_nef_t *cmd_buf;
    void *nef_buf;
    int timeout;
    int sts;
} _update_nef_command_package;

static void *_update_nef_to_single_device(void *data)
{
    _update_nef_command_package *cmd_pack = (_update_nef_command_package *)data;

    dbg_print("[%s] thread : device %p, cmd_buf %p, total_size %d, timeout %d\n", __FUNCTION__, cmd_pack->ll_device, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->total_size, cmd_pack->timeout);

    // write command header including fw_info buffer
    int ret = kp_usb_write_data(cmd_pack->ll_device, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->total_size, cmd_pack->timeout);
    if (ret != KP_USB_RET_OK)
    {
        cmd_pack->sts = check_usb_read_data_error(ret);
        return NULL;
    }

    uint32_t return_code;
    int status;
    ret = kp_usb_read_data(cmd_pack->ll_device, (void *)&return_code, sizeof(uint32_t), cmd_pack->timeout);
    status = check_usb_read_data_error(ret);

    dbg_print("[%s] update nef info sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(uint32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    if (ret != KP_SUCCESS)
    {
        cmd_pack->sts = ret;
        return NULL;
    }

    dbg_print("[%s] thread : device %p, nef_buf %p, nef_size %d, timeout %d\n", __FUNCTION__, cmd_pack->ll_device, (void *)cmd_pack->nef_buf, cmd_pack->cmd_buf->nef_size, cmd_pack->timeout);

    // write nef buffer
    ret = kp_usb_write_data(cmd_pack->ll_device, (void *)cmd_pack->nef_buf, cmd_pack->cmd_buf->nef_size, cmd_pack->timeout);
    if (ret != KP_USB_RET_OK)
    {
        cmd_pack->sts = check_usb_read_data_error(ret);
        return NULL;
    }

    ret = kp_usb_read_data(cmd_pack->ll_device, (void *)&return_code, sizeof(uint32_t), cmd_pack->timeout);
    status = check_usb_read_data_error(ret);

    dbg_print("[%s] update nef info sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(uint32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    // at here, device should be reboot, host should disconnect it
    usleep(USB_DISCONNECT_WAIT_DELAY_US);
    kp_usb_disconnect_device(cmd_pack->ll_device);
    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    cmd_pack->sts = ret;

    return NULL;
}

static char *read_file_to_buffer_auto_malloc(const char *file_path, long *buffer_size)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        dbg_print("%s(): fopen failed, file:%s, %s\n", __FUNCTION__, file_path, strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file); //get the size

    *buffer_size = file_size;

    char *buffer = (char *)malloc(file_size);
    if (NULL == buffer) {
        fclose(file);
        dbg_print("%s(): malloc buffer for file read failed\n", __FUNCTION__);
        return NULL;
    }

    fseek(file, 0, SEEK_SET); //move to begining

    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != (size_t)file_size)
    {
        dbg_print("%s(): fread failed, file size: %u, read size %u\n", __FUNCTION__,
                  (unsigned int)file_size, (unsigned int)read_size);
        free(buffer);
        buffer = NULL;
        *buffer_size = 0;
    }

    fclose(file);

    return buffer;
}

int _kl520_update_kdp2_usb_loader(kp_device_group_t devices, void *usb_loader_buf, int usb_loader_size, bool auto_reboot)
{
    return kp_update_kdp_firmware(devices, usb_loader_buf, usb_loader_size, NULL, 0, auto_reboot);
}

int _kl630_update_kdp2_usb_loader(kp_device_group_t devices, void *usb_loader_buf, int usb_loader_size, bool auto_reboot)
{
    int ret = 0;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        uint16_t fw_type = (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial);
        uint16_t fw_type_legacy = (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial);

        if ((KP_KDP2_FW_LOADER_V2 == fw_type) || (KP_KDP2_FW_LOADER == fw_type_legacy) || (KP_KDP_FW == ll_dev[i]->fw_serial)) {
            dbg_print("one or more devices are running KDP or KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    if (NULL != usb_loader_buf)
    {
        dbg_print("Start updating loader firmware...\n");

        int port_id_list[MAX_GROUP_DEVICE] = {0};

        _update_kdp2_firmware_package cmd_packs[MAX_GROUP_DEVICE];
        pthread_t update_fw_thd[MAX_GROUP_DEVICE];

        cmd_packs[0].dev_idx = 0;
        cmd_packs[0].ll_device = ll_dev[0];
        cmd_packs[0].fw_buf = usb_loader_buf;
        cmd_packs[0].fw_size = usb_loader_size;
        cmd_packs[0].timeout = _devices_grp->timeout;
        cmd_packs[0].auto_reboot = auto_reboot;

        port_id_list[0] = ll_dev[0]->dev_descp.port_id;

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            dbg_print("[%s] create thread to update loader fw to device %d\n", __FUNCTION__, i);
            memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_kdp2_firmware_package));
            cmd_packs[i].dev_idx = i;
            cmd_packs[i].ll_device = ll_dev[i];

            port_id_list[i] = ll_dev[i]->dev_descp.port_id;

            int thd_ret = pthread_create(&update_fw_thd[i], NULL, _update_kdp2_loader_to_single_device, (void *)&cmd_packs[i]);

            if (0 != thd_ret)
            {
                dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
                return -1;
            }
        }

        // current thread do first device
        _update_kdp2_loader_to_single_device((void *)&cmd_packs[0]);

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            pthread_join(update_fw_thd[i], NULL);
        }

        // check all thread update fw status
        for (int i = 0; i < _devices_grp->num_device; i++)
        {
            ret = cmd_packs[i].sts;
            if (KP_USB_RET_OK != ret)
            {
                dbg_print("[%s] thread update loader fw failed at device %d, error %d\n", __FUNCTION__, i, ret);
                break;
            }
        }

        dbg_print("Update loader firmware process finished, try to re-connect device...\n");

        if (true == auto_reboot)
        {
            usleep(USB_REBOOT_WAIT_DELAY_US);
        }
        else
        {
            usleep(USB_DISCONNECT_WAIT_DELAY_US);
        }

        // here USB is disconnected, re-connect it
        if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
            return KP_ERROR_DEVICE_NOT_EXIST_10;

        // FIXME, better code, do this for let FW prepare buffers to receive further commands
        {
            for (int i = 0; i < _devices_grp->num_device; i++)
                _devices_grp->ll_device[i] = ll_dev[i];

            /* Set up fifo queue */
            kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
        }

        dbg_print("Device is re-connected\n\n");
    }

    return ret;
}

int _update_model(kp_device_group_t devices, void *nef_buf, int nef_size, bool auto_reboot,
                  kp_model_nef_descriptor_t *model_desc)
{
    int ret = -1;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;
    kp_metadata_t metadata = {0};
    kp_nef_info_t nef_info = {0};

    for (int i = 0; i < _devices_grp->num_device; i++) {
        if (((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER == (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL720 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_KL720_LOADER == ll_dev[i]->fw_serial))) {
            dbg_print("one or more devices are running KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        } else if (KP_DEVICE_KL630 == ll_dev[i]->dev_descp.product_id) {
            dbg_print("KL630 does not suppot _update_model.\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    ret = load_model_info_from_nef(nef_buf, nef_size, _devices_grp->product_id, &metadata, &nef_info, model_desc);

    int fw_info_size = nef_info.fw_info_size;
    int all_models_size = nef_info.all_models_size;
    int total_model_size = fw_info_size + all_models_size;

    if (0 >= fw_info_size) {
        dbg_print("Not supported target %d!\n", metadata.target);
        return KP_ERROR_INVALID_MODEL_21;
    }

    if (0 >= all_models_size) {
        dbg_print("getting all_models fw_info failed:%d...\n", all_models_size);
        return KP_ERROR_INVALID_MODEL_21;
    }

    char *total_model_buf = (char *)malloc(total_model_size);
    if(NULL == total_model_buf){
        dbg_print("Error allocate memory for models failed\n");
        return KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
    }

    // get fw_info data
    memset(total_model_buf, 0, total_model_size);
    memcpy(total_model_buf, nef_info.fw_info_addr, fw_info_size);
    memcpy(total_model_buf + fw_info_size, nef_info.all_models_addr, all_models_size);

    kdp_model_update_cmd_t cmd_buf;
    int port_id_list[MAX_GROUP_DEVICE] = {0};

    cmd_buf.preamble = KDP_MSG_HDR_CMD;
    cmd_buf.ctrl = 0;
    cmd_buf.cmd = KDP_CMD_UPDATE_MODEL;
    cmd_buf.msg_len = 8;
    cmd_buf.fw_info_size = fw_info_size << 16;
    cmd_buf.all_models_size = all_models_size;
    cmd_buf.auto_reboot = auto_reboot;

    _update_model_command_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t update_model_thd[MAX_GROUP_DEVICE];

    cmd_packs[0].ll_device = ll_dev[0];
    cmd_packs[0].cmd_buf = &cmd_buf;
    cmd_packs[0].total_model_buf = total_model_buf;
    cmd_packs[0].total_model_size = total_model_size;
    cmd_packs[0].timeout = _devices_grp->timeout;

    port_id_list[0] = ll_dev[0]->dev_descp.port_id;

    for (int i = 1; i < _devices_grp->num_device; i++)
    {
        port_id_list[i] = ll_dev[i]->dev_descp.port_id;

        if (0 != metadata.enc_type) {
            uint32_t kn_number = ll_dev[i]->dev_descp.kn_number;

            if (0xFFFF == kn_number) {
                dbg_print("Firmware on device %d does not support get kn number!\n", i);
                continue;
            } else if (kn_number != metadata.kn_num) {
                dbg_print("KN number of model file does not match device %d\n", i);
                continue;
            }
        }

        dbg_print("[%s] create thread to update model to device %d\n", __FUNCTION__, i);
        memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_model_command_package));
        cmd_packs[i].ll_device = ll_dev[i];

        int thd_ret = pthread_create(&update_model_thd[i], NULL, _update_model_to_single_device, (void *)&cmd_packs[i]);

        if (0 != thd_ret) {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return -1;
        }
    }

    if (0 != metadata.enc_type) {
        uint32_t kn_number = ll_dev[0]->dev_descp.kn_number;

        if (0xFFFF == kn_number) {
            dbg_print("Firmware on device 0 does not support get kn number!\n");
            goto AFTER_UPDATE;
        } else if (kn_number != metadata.kn_num) {
            dbg_print("KN number of model file does not match device 0\n");
            goto AFTER_UPDATE;
        }
    }

    // current thread do first device
    _update_model_to_single_device((void *)&cmd_packs[0]);

AFTER_UPDATE:

    for (int i = 1; i < _devices_grp->num_device; i++) {
        pthread_join(update_model_thd[i], NULL);
    }

    // check all thread update model status
    for (int i = 0; i < _devices_grp->num_device; i++) {
        ret = cmd_packs[i].sts;
        if (KP_USB_RET_OK != ret) {
            dbg_print("[%s] thread update model failed at device %d, error %d\n", __FUNCTION__, i, ret);
            break;
        }
    }

    free(total_model_buf);

    if (KP_USB_RET_OK == ret) {
        ret = load_model_info_from_nef(nef_buf, nef_size, _devices_grp->product_id, &metadata, &nef_info, &(_devices_grp->loaded_model_desc));
    }

    dbg_print("Update model process finished, try to re-connect devices...\n");

    if (true == auto_reboot) {
        usleep(USB_REBOOT_WAIT_DELAY_US);
    } else {
        usleep(USB_DISCONNECT_WAIT_DELAY_US);
    }

    // here USB is disconnected, re-connect it
    if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
        return KP_ERROR_DEVICE_NOT_EXIST_10;

    // FIXME, better code, do this for let FW prepare buffers to receive further commands
    {
        for (int i = 0; i < _devices_grp->num_device; i++)
            _devices_grp->ll_device[i] = ll_dev[i];

        /* Set up fifo queue */
        kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
    }

    dbg_print("Devices are re-connected\n\n");

    return ret;
}

int _update_nef(kp_device_group_t devices, void *nef_buf, int nef_size, bool auto_reboot,
                kp_model_nef_descriptor_t *model_desc)
{
    int ret = -1;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;
    kp_metadata_t metadata;
    kp_nef_info_t nef_info;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        if ((KP_DEVICE_KL630 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial))) {
            dbg_print("one or more devices are running KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        } else if ((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) ||
                   (KP_DEVICE_KL720 == ll_dev[i]->dev_descp.product_id) ||
                   (KP_DEVICE_KL720_LEGACY == ll_dev[i]->dev_descp.product_id)) {
            dbg_print("KL520 and KL720 do not suppot _update_nef.\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    ret = load_model_info_from_nef(nef_buf, nef_size, _devices_grp->product_id, &metadata, &nef_info, &(_devices_grp->loaded_model_desc));

    if (KP_SUCCESS != ret)
        return ret;

    // The input model is an encrypted model
    if ((metadata.kn_num != 0 && metadata.enc_type != 0) &&
        (_devices_grp->num_device > 1 || _devices_grp->ll_device[0]->dev_descp.kn_number != metadata.kn_num))
         return KP_ERROR_INVALID_MODEL_21;

    kdp2_ipc_cmd_update_nef_t *cmd_buf = (kdp2_ipc_cmd_update_nef_t *)malloc(sizeof(kdp2_ipc_cmd_update_nef_t));
    int port_id_list[MAX_GROUP_DEVICE] = {0};

    cmd_buf->magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf->total_size = sizeof(kdp2_ipc_cmd_update_nef_t);
    cmd_buf->command_id = KDP2_COMMAND_UPDATE_NEF;
    cmd_buf->nef_size = nef_size;
    cmd_buf->auto_reboot = (true == auto_reboot) ? 1 : 0;

    _update_nef_command_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t update_nef_thd[MAX_GROUP_DEVICE];

    cmd_packs[0].ll_device = _devices_grp->ll_device[0];
    cmd_packs[0].cmd_buf = cmd_buf;
    cmd_packs[0].nef_buf = nef_buf;
    cmd_packs[0].timeout = _devices_grp->timeout;

    port_id_list[0] = ll_dev[0]->dev_descp.port_id;

    for (int i = 1; i < _devices_grp->num_device; i++)
    {
        port_id_list[i] = ll_dev[i]->dev_descp.port_id;

        if (0 != metadata.enc_type) {
            uint32_t kn_number = ll_dev[i]->dev_descp.kn_number;

            if (0xFFFF == kn_number) {
                dbg_print("Firmware on device %d does not support get kn number!\n", i);
                continue;
            } else if (kn_number != metadata.kn_num) {
                dbg_print("KN number of model file does not match device %d\n", i);
                continue;
            }
        }

        dbg_print("[%s] create thread to update model to device %d\n", __FUNCTION__, i);
        memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_model_command_package));
        cmd_packs[i].ll_device = ll_dev[i];

        int thd_ret = pthread_create(&update_nef_thd[i], NULL, _update_nef_to_single_device, (void *)&cmd_packs[i]);

        if (0 != thd_ret) {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return -1;
        }
    }

    if (0 != metadata.enc_type) {
        uint32_t kn_number = ll_dev[0]->dev_descp.kn_number;

        if (0xFFFF == kn_number) {
            dbg_print("Firmware on device 0 does not support get kn number!\n");
            goto AFTER_UPDATE;
        } else if (kn_number != metadata.kn_num) {
            dbg_print("KN number of model file does not match device 0\n");
            goto AFTER_UPDATE;
        }
    }

    // current thread do first device
    _update_nef_to_single_device((void *)&cmd_packs[0]);

AFTER_UPDATE:

    for (int i = 1; i < _devices_grp->num_device; i++) {
        pthread_join(update_nef_thd[i], NULL);
    }

    // check all thread update model status
    for (int i = 0; i < _devices_grp->num_device; i++) {
        ret = cmd_packs[i].sts;
        if (KP_USB_RET_OK != ret) {
            dbg_print("[%s] thread update nef failed at device %d, error %d\n", __FUNCTION__, i, ret);
            break;
        }
    }

    free(cmd_buf);

    if (KP_USB_RET_OK == ret) {
        ret = load_model_info_from_nef(nef_buf, nef_size, _devices_grp->product_id, &metadata, &nef_info, &(_devices_grp->loaded_model_desc));
    }

    dbg_print("Update model process finished, try to re-connect devices...\n");

    if (true == auto_reboot) {
        usleep(USB_REBOOT_WAIT_DELAY_US);
    } else {
        usleep(USB_DISCONNECT_WAIT_DELAY_US);
    }

    // here USB is disconnected, re-connect it
    if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
        return KP_ERROR_DEVICE_NOT_EXIST_10;

    // FIXME, better code, do this for let FW prepare buffers to receive further commands
    {
        for (int i = 0; i < _devices_grp->num_device; i++)
            _devices_grp->ll_device[i] = ll_dev[i];

        /* Set up fifo queue */
        kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
    }

    dbg_print("Devices are re-connected\n\n");

    return ret;
}

int kp_read_data_from_flash(kp_usb_device_t *ll_dev, int timeout, uint32_t flash_offset,
                            uint32_t length, uint8_t *buffer)
{
    kdp2_ipc_cmd_read_flash_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_read_flash_t);
    cmd_buf.command_id = KDP2_COMMAND_READ_FLASH;
    cmd_buf.flash_offset = flash_offset;
    cmd_buf.length = length;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, timeout);
    int status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS)
        return status;

    int32_t return_code;

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(int32_t), timeout);
    status = check_usb_read_data_error(ret);

    if (status != KP_SUCCESS) {
        ret = status;
    } else if (return_code != KP_SUCCESS) {
        ret = return_code;
    } else if (ret == sizeof(int32_t)) {
        ret = KP_SUCCESS;
    } else {
        ret = KP_ERROR_OTHER_99;
    }

    if (ret != KP_SUCCESS) {
        return ret;
    }

    // Get memory data from FW
    ret = kp_usb_read_data(ll_dev, (void *)buffer, (int)length, timeout);
    status = check_usb_read_data_error(ret);

    if (status != KP_SUCCESS) {
        ret = status;
    } else if (ret == (int)length) {
        ret = KP_SUCCESS;
    } else {
        ret = KP_ERROR_OTHER_99;
    }

    return ret;
}

int kp_write_data_to_flash(kp_usb_device_t *ll_dev, int timeout, uint32_t flash_offset,
                           uint32_t length, uint8_t *buffer)
{
    kdp2_ipc_cmd_write_flash_t cmd_buf;
    int32_t return_code = -1;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_write_flash_t);
    cmd_buf.command_id = KDP2_COMMAND_WRITE_FLASH;
    cmd_buf.flash_offset = flash_offset;
    cmd_buf.length = length;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, timeout);
    int status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS) {
        return status;
    }

    ret = kp_usb_write_data(ll_dev, (void *)buffer, length, timeout);
    status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS) {
        return status;
    }

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(int32_t), timeout);
    status = check_usb_read_data_error(ret);
    if (status != KP_SUCCESS) {
        return status;
    }

    return return_code;
}

int kp_update_kdp2_firmware(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size,
                            void *ncpu_fw_buf, int ncpu_fw_size, bool auto_reboot)
{
    int ret = 0;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;
    bool update_kl720 = ((KP_DEVICE_KL720 == ll_dev[0]->dev_descp.product_id) ||
                         (KP_DEVICE_KL720_LEGACY == ll_dev[0]->dev_descp.product_id));

    if (true == update_kl720) {
        dbg_print("Update KL720 ...\n");
        return kp_update_kdp_firmware(devices, scpu_fw_buf, scpu_fw_size, ncpu_fw_buf, ncpu_fw_size, auto_reboot);
    }

    for (int i = 0; i < _devices_grp->num_device; i++) {
        uint16_t fw_type = (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial);
        uint16_t fw_type_legacy = (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial);

        if ((KP_KDP2_FW_LOADER_V2 == fw_type) || (KP_KDP2_FW_LOADER == fw_type_legacy) || (KP_KDP_FW == ll_dev[i]->fw_serial)) {
            dbg_print("one or more devices are running KDP or KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    if (NULL != scpu_fw_buf)
    {
        dbg_print("Start updating scpu firmware...\n");

        int port_id_list[MAX_GROUP_DEVICE] = {0};

        _update_kdp2_firmware_package cmd_packs[MAX_GROUP_DEVICE];
        pthread_t update_fw_thd[MAX_GROUP_DEVICE];

        cmd_packs[0].dev_idx = 0;
        cmd_packs[0].ll_device = ll_dev[0];
        cmd_packs[0].fw_buf = scpu_fw_buf;
        cmd_packs[0].fw_size = scpu_fw_size;
        cmd_packs[0].fw_id = 1;
        cmd_packs[0].timeout = _devices_grp->timeout;
        cmd_packs[0].auto_reboot = auto_reboot;

        port_id_list[0] = ll_dev[0]->dev_descp.port_id;

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            dbg_print("[%s] create thread to update scpu fw to device %d\n", __FUNCTION__, i);
            memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_kdp2_firmware_package));
            cmd_packs[i].dev_idx = i;
            cmd_packs[i].ll_device = ll_dev[i];

            port_id_list[i] = ll_dev[i]->dev_descp.port_id;

            int thd_ret = pthread_create(&update_fw_thd[i], NULL, _update_kdp2_firmware_to_single_device, (void *)&cmd_packs[i]);

            if (0 != thd_ret)
            {
                dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
                return -1;
            }
        }

        // current thread do first device
        _update_kdp2_firmware_to_single_device((void *)&cmd_packs[0]);

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            pthread_join(update_fw_thd[i], NULL);
        }

        // check all thread update fw status
        for (int i = 0; i < _devices_grp->num_device; i++)
        {
            ret = cmd_packs[i].sts;
            if (KP_USB_RET_OK != ret)
            {
                dbg_print("[%s] thread update scpu fw failed at device %d, error %d\n", __FUNCTION__, i, ret);
                break;
            }
        }

        dbg_print("Update scpu firmware process finished, try to re-connect device...\n");

        if (true == auto_reboot)
        {
            usleep(USB_REBOOT_WAIT_DELAY_US);
        }
        else
        {
            usleep(USB_DISCONNECT_WAIT_DELAY_US);
        }

        // here USB is disconnected, re-connect it
        if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
            return KP_ERROR_DEVICE_NOT_EXIST_10;

        // FIXME, better code, do this for let FW prepare buffers to receive further commands
        {
            for (int i = 0; i < _devices_grp->num_device; i++)
                _devices_grp->ll_device[i] = ll_dev[i];

            /* Set up fifo queue */
            kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
        }

        dbg_print("Device is re-connected\n\n");
    }

    if (KP_SUCCESS != ret)
    {
        goto FUNC_OUT;
    }

    if (NULL != ncpu_fw_buf) {
        dbg_print("Start updating ncpu firmware...\n");

        int port_id_list[MAX_GROUP_DEVICE] = {0};

        _update_kdp2_firmware_package cmd_packs[MAX_GROUP_DEVICE];
        pthread_t update_fw_thd[MAX_GROUP_DEVICE];

        cmd_packs[0].dev_idx = 0;
        cmd_packs[0].ll_device = ll_dev[0];
        cmd_packs[0].fw_buf = ncpu_fw_buf;
        cmd_packs[0].fw_size = ncpu_fw_size;
        cmd_packs[0].fw_id = 2;
        cmd_packs[0].timeout = _devices_grp->timeout;
        cmd_packs[0].auto_reboot = auto_reboot;

        port_id_list[0] = ll_dev[0]->dev_descp.port_id;

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            dbg_print("[%s] create thread to update ncpu fw to device %d\n", __FUNCTION__, i);
            memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_kdp2_firmware_package));
            cmd_packs[i].dev_idx = i;
            cmd_packs[i].ll_device = ll_dev[i];

            port_id_list[i] = ll_dev[i]->dev_descp.port_id;

            int thd_ret = pthread_create(&update_fw_thd[i], NULL, _update_kdp2_firmware_to_single_device, (void *)&cmd_packs[i]);

            if (0 != thd_ret)
            {
                dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
                return -1;
            }
        }

        // current thread do first device
        _update_kdp2_firmware_to_single_device((void *)&cmd_packs[0]);

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            pthread_join(update_fw_thd[i], NULL);
        }

        // check all thread update fw status
        for (int i = 0; i < _devices_grp->num_device; i++)
        {
            ret = cmd_packs[i].sts;
            if (KP_USB_RET_OK != ret)
            {
                dbg_print("[%s] thread update ncpu fw failed at device %d, error %d\n", __FUNCTION__, i, ret);
                break;
            }
        }

        dbg_print("Update ncpu firmware process finished, try to re-connect device...\n");

        if (true == auto_reboot)
        {
            usleep(USB_REBOOT_WAIT_DELAY_US);
        }
        else
        {
            usleep(USB_DISCONNECT_WAIT_DELAY_US);
        }

        // here USB is disconnected, re-connect it
        if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
            return KP_ERROR_DEVICE_NOT_EXIST_10;

        // FIXME, better code, do this for let FW prepare buffers to receive further commands
        {
            for (int i = 0; i < _devices_grp->num_device; i++)
                _devices_grp->ll_device[i] = ll_dev[i];

            /* Set up fifo queue */
            kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
        }

        dbg_print("Device is re-connected\n\n");
    }

FUNC_OUT:

    return ret;
}

int kp_update_kdp2_firmware_from_files(kp_device_group_t devices, const char *scpu_fw_file,
                                       const char *ncpu_fw_file, bool auto_reboot)
{
    long scpu_fw_size = 0;
    long ncpu_fw_size = 0;
    char *scpu_fw_buf = NULL;
    char *ncpu_fw_buf = NULL;
    int status = KP_ERROR_FILE_OPEN_FAILED_20;

    if ((NULL != scpu_fw_file) && ('\0' != scpu_fw_file[0]))
    {
        scpu_fw_buf = read_file_to_buffer_auto_malloc(scpu_fw_file, &scpu_fw_size);
    }

    if ((NULL != ncpu_fw_file) && ('\0' != ncpu_fw_file[0]))
    {
        ncpu_fw_buf = read_file_to_buffer_auto_malloc(ncpu_fw_file, &ncpu_fw_size);
    }

    if ((NULL != scpu_fw_buf) || (NULL != ncpu_fw_buf))
    {
        status = kp_update_kdp2_firmware(devices, scpu_fw_buf, scpu_fw_size, ncpu_fw_buf, ncpu_fw_size, auto_reboot);
    }

    if (NULL != scpu_fw_buf)
    {
        free(scpu_fw_buf);
    }

    if (NULL != ncpu_fw_buf)
    {
        free(ncpu_fw_buf);
    }

    return status;
}

int kp_update_kdp_firmware(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size,
                           void *ncpu_fw_buf, int ncpu_fw_size, bool auto_reboot)
{
    int ret = 0;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        if (((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER == (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL720 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_KL720_LOADER == ll_dev[i]->fw_serial)) ||
            ((KP_DEVICE_KL630 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial)))) {
            dbg_print("one or more devices are running KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    if (NULL != scpu_fw_buf) {
        dbg_print("Start updating scpu firmware...\n");

        kdp_firmware_update_cmd_t cmd_buf;
        int port_id_list[MAX_GROUP_DEVICE] = {0};

        cmd_buf.preamble = KDP_MSG_HDR_CMD;
        cmd_buf.ctrl = 0;
        cmd_buf.cmd = KDP_CMD_UPDATE_FW;
        cmd_buf.msg_len = 4;
        cmd_buf.fw_id = 1;
        cmd_buf.auto_reboot = (true == auto_reboot) ? 1 : 0;

        _update_kdp_firmware_command_package cmd_packs[MAX_GROUP_DEVICE];
        pthread_t update_fw_thd[MAX_GROUP_DEVICE];

        cmd_packs[0].ll_device = ll_dev[0];
        cmd_packs[0].cmd_buf = &cmd_buf;
        cmd_packs[0].fw_buf = scpu_fw_buf;
        cmd_packs[0].fw_size = scpu_fw_size;
        cmd_packs[0].timeout = 10000;

        port_id_list[0] = ll_dev[0]->dev_descp.port_id;

        for (int i = 1; i < _devices_grp->num_device; i++) {
            dbg_print("[%s] create thread to update scpu fw to device %d\n", __FUNCTION__, i);
            memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_kdp_firmware_command_package));
            cmd_packs[i].ll_device = ll_dev[i];

            port_id_list[i] = ll_dev[i]->dev_descp.port_id;

            int thd_ret = pthread_create(&update_fw_thd[i], NULL, _update_kdp_firmware_to_single_device, (void *)&cmd_packs[i]);

            if (0 != thd_ret) {
                dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
                return -1;
            }
        }

        // current thread do first device
        _update_kdp_firmware_to_single_device((void *)&cmd_packs[0]);

        for (int i = 1; i < _devices_grp->num_device; i++) {
            pthread_join(update_fw_thd[i], NULL);
        }

        // check all thread upload model status
        for (int i = 0; i < _devices_grp->num_device; i++) {
            ret = cmd_packs[i].sts;
            if (KP_USB_RET_OK != ret) {
                dbg_print("[%s] thread update scpu fw failed at device %d, error %d\n", __FUNCTION__, i, ret);
                break;
            }
        }

        dbg_print("Update scpu firmware process finished, try to re-connect device...\n");

        if (true == auto_reboot) {
            usleep(USB_REBOOT_WAIT_DELAY_US);
        } else {
            usleep(USB_DISCONNECT_WAIT_DELAY_US);
        }

        // here USB is disconnected, re-connect it
        if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
            return KP_ERROR_DEVICE_NOT_EXIST_10;

        // FIXME, better code, do this for let FW prepare buffers to receive further commands
        {
            for (int i = 0; i < _devices_grp->num_device; i++)
                _devices_grp->ll_device[i] = ll_dev[i];

            /* Set up fifo queue */
            kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
        }

        dbg_print("Device is re-connected\n\n");
    }

    if (KP_SUCCESS != ret) {
        goto FUNC_OUT;
    }

    if (NULL != ncpu_fw_buf) {
        dbg_print("Start updating ncpu firmware...\n");

        kdp_firmware_update_cmd_t cmd_buf;
        int port_id_list[MAX_GROUP_DEVICE] = {0};

        cmd_buf.preamble = KDP_MSG_HDR_CMD;
        cmd_buf.ctrl = 0;
        cmd_buf.cmd = KDP_CMD_UPDATE_FW;
        cmd_buf.msg_len = 4;
        cmd_buf.fw_id = 2;
        cmd_buf.auto_reboot = (true == auto_reboot) ? 1 : 0;

        _update_kdp_firmware_command_package cmd_packs[MAX_GROUP_DEVICE];
        pthread_t update_fw_thd[MAX_GROUP_DEVICE];

        cmd_packs[0].ll_device = ll_dev[0];
        cmd_packs[0].cmd_buf = &cmd_buf;
        cmd_packs[0].fw_buf = ncpu_fw_buf;
        cmd_packs[0].fw_size = ncpu_fw_size;
        cmd_packs[0].timeout = 10000;

        port_id_list[0] = ll_dev[0]->dev_descp.port_id;

        for (int i = 1; i < _devices_grp->num_device; i++) {
            dbg_print("[%s] create thread to update ncpu fw to device %d\n", __FUNCTION__, i);
            memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_kdp_firmware_command_package));
            cmd_packs[i].ll_device = ll_dev[i];

            port_id_list[i] = ll_dev[i]->dev_descp.port_id;

            int thd_ret = pthread_create(&update_fw_thd[i], NULL, _update_kdp_firmware_to_single_device, (void *)&cmd_packs[i]);

            if (0 != thd_ret) {
                dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
                return -1;
            }
        }

        // current thread do first device
        _update_kdp_firmware_to_single_device((void *)&cmd_packs[0]);

        for (int i = 1; i < _devices_grp->num_device; i++) {
            pthread_join(update_fw_thd[i], NULL);
        }

        // check all thread update fw status
        for (int i = 0; i < _devices_grp->num_device; i++) {
            ret = cmd_packs[i].sts;
            if (KP_USB_RET_OK != ret) {
                dbg_print("[%s] thread update ncpu fw failed at device %d, error %d\n", __FUNCTION__, i, ret);
                break;
            }
        }

        dbg_print("Updating ncpu firmware process finished, try to re-connect device...\n");

        if (true == auto_reboot) {
            usleep(USB_REBOOT_WAIT_DELAY_US);
        } else {
            usleep(USB_DISCONNECT_WAIT_DELAY_US);
        }

        // here USB is disconnected, re-connect it
        if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
            return KP_ERROR_DEVICE_NOT_EXIST_10;

        // FIXME, better code, do this for let FW prepare buffers to receive further commands
        {
            for (int i = 0; i < _devices_grp->num_device; i++)
                _devices_grp->ll_device[i] = ll_dev[i];

            /* Set up fifo queue */
            kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
        }

        dbg_print("Device is re-connected\n\n");
    }

FUNC_OUT:

    return ret;
}

int kp_update_kdp_firmware_from_files(kp_device_group_t devices, const char *scpu_fw_file,
                                      const char *ncpu_fw_file, bool auto_reboot)
{
    long scpu_fw_size = 0;
    long ncpu_fw_size = 0;
    char *scpu_fw_buf = NULL;
    char *ncpu_fw_buf = NULL;
    int status = KP_ERROR_FILE_OPEN_FAILED_20;

    if ((NULL != scpu_fw_file) && ('\0' != scpu_fw_file[0])) {
        scpu_fw_buf = read_file_to_buffer_auto_malloc(scpu_fw_file, &scpu_fw_size);
    }

    if ((NULL != ncpu_fw_file) && ('\0' != ncpu_fw_file[0])) {
        ncpu_fw_buf = read_file_to_buffer_auto_malloc(ncpu_fw_file, &ncpu_fw_size);
    }

    if ((NULL != scpu_fw_buf) || (NULL != ncpu_fw_buf)) {
        status = kp_update_kdp_firmware(devices, scpu_fw_buf, scpu_fw_size, ncpu_fw_buf, ncpu_fw_size, auto_reboot);
    }

    if (NULL != scpu_fw_buf) {
        free(scpu_fw_buf);
    }

    if (NULL != ncpu_fw_buf) {
        free(ncpu_fw_buf);
    }

    return status;
}

int kp_update_model(kp_device_group_t devices, void *nef_buf, int nef_size, bool auto_reboot,
                    kp_model_nef_descriptor_t *model_desc)
{
    int ret = KP_ERROR_UNSUPPORTED_DEVICE_44;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    if (KP_DEVICE_KL630 == ll_dev[0]->dev_descp.product_id) {
        ret = _update_nef(devices, nef_buf, nef_size, auto_reboot, model_desc);
    } else if ((KP_DEVICE_KL520 == ll_dev[0]->dev_descp.product_id) ||
               (KP_DEVICE_KL720 == ll_dev[0]->dev_descp.product_id) ||
               (KP_DEVICE_KL720_LEGACY == ll_dev[0]->dev_descp.product_id)) {
        ret = _update_model(devices, nef_buf, nef_size, auto_reboot, model_desc);
    }

    return ret;
}

int kp_update_model_from_file(kp_device_group_t devices, const char *file_path, bool auto_reboot,
                              kp_model_nef_descriptor_t *model_desc)
{
    long nef_size;
    char *nef_buf = NULL;
    int ret = KP_ERROR_FILE_OPEN_FAILED_20;

    if ((NULL != file_path) && ('\0' != file_path[0])) {
        nef_buf = read_file_to_buffer_auto_malloc(file_path, &nef_size);

        if (NULL != nef_buf) {
            ret = kp_update_model(devices, (void *)nef_buf, (int)nef_size, auto_reboot, model_desc);
        }
    }

    if (NULL != nef_buf) {
        free(nef_buf);
    }

    return ret;
}

int kp_update_kdp2_usb_loader(kp_device_group_t devices, void *usb_loader_buf,
                              int usb_loader_size, bool auto_reboot)
{
    int ret = 0;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    /** check devices product_id */
    kp_product_id_t product_id = 0;
    for (int i = 0; i < _devices_grp->num_device; i++) {
        if (0 == i) {
            product_id = ll_dev[i]->dev_descp.product_id;

            if ((KP_DEVICE_KL520 != product_id) &&
                (KP_DEVICE_KL630 != product_id)) {
                dbg_print("one or more devices not support KDP2 Loader update...\n");
                ret = KP_ERROR_UNSUPPORTED_DEVICE_44;
                goto FUNC_OUT;
            }
        } else if (product_id != ll_dev[i]->dev_descp.product_id) {
                dbg_print("different device mix in device group...\n");
                ret = KP_ERROR_DEVICE_GROUP_MIX_PRODUCT_29;
                goto FUNC_OUT;
        }
    }

    /** update loader */
    if (KP_DEVICE_KL520 == product_id) {
        ret = _kl520_update_kdp2_usb_loader(devices, usb_loader_buf, usb_loader_size, auto_reboot);
    } else if (KP_DEVICE_KL630 == product_id) {
        ret = _kl630_update_kdp2_usb_loader(devices, usb_loader_buf, usb_loader_size, auto_reboot);
    } else {
        ret = KP_ERROR_UNSUPPORTED_DEVICE_44;
    }

FUNC_OUT:

    return ret;
}

int kp_update_kdp2_usb_loader_from_file(kp_device_group_t devices, const char *usb_loader_file,
                                        bool auto_reboot)
{
    long usb_loader_size = 0;
    char *usb_loader_buf = NULL;
    int status = KP_ERROR_FILE_OPEN_FAILED_20;

    if ((NULL != usb_loader_file) && ('\0' != usb_loader_file[0]))
    {
        usb_loader_buf = read_file_to_buffer_auto_malloc(usb_loader_file, &usb_loader_size);
    }


    if (NULL != usb_loader_buf)
    {
        status = kp_update_kdp2_usb_loader(devices, usb_loader_buf, usb_loader_size, auto_reboot);
    }

    if (NULL != usb_loader_buf)
    {
        free(usb_loader_buf);
    }

    return status;
}

int kp_switch_to_kdp2_usb_boot(kp_device_group_t devices, bool auto_reboot)
{
    int Ret;
    int port_id_list[MAX_GROUP_DEVICE] = {0};
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        uint16_t fw_type = (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial);
        uint16_t fw_type_legacy = (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial);

        if ((KP_KDP2_FW_LOADER_V2 == fw_type) || (KP_KDP2_FW_LOADER == fw_type_legacy) || (KP_KDP_FW == ll_dev[i]->fw_serial)) {
            dbg_print("one or more devices are running KDP or KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    _update_kdp2_usb_boot_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t update_thd[MAX_GROUP_DEVICE];

    cmd_packs[0].dev_idx = 0;
    cmd_packs[0].ll_device = ll_dev[0];
    cmd_packs[0].timeout = _devices_grp->timeout;
    cmd_packs[0].auto_reboot = auto_reboot;

    port_id_list[0] = ll_dev[0]->dev_descp.port_id;

    for (int i = 1; i < _devices_grp->num_device; i++) {
        dbg_print("[%s] create thread to update boot type to device %d\n", __FUNCTION__, i);
        memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_update_kdp2_usb_boot_package));
        cmd_packs[i].dev_idx = i;
        cmd_packs[i].ll_device = ll_dev[i];

        port_id_list[i] = ll_dev[i]->dev_descp.port_id;

        int thd_ret = pthread_create(&update_thd[i], NULL, _update_single_device_to_kdp2_usb_boot, (void *)&cmd_packs[i]);

        if (0 != thd_ret)
        {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return -1;
        }
    }

    // current thread do first device
    _update_single_device_to_kdp2_usb_boot((void *)&cmd_packs[0]);

    for (int i = 1; i < _devices_grp->num_device; i++) {
        pthread_join(update_thd[i], NULL);
    }

    // check all thread update fw status
    for (int i = 0; i < _devices_grp->num_device; i++) {
        Ret = cmd_packs[i].sts;
        if (KP_USB_RET_OK != Ret) {
            dbg_print("[%s] thread write usb boot type failed at device %d, error %d\n", __FUNCTION__, i, Ret);
            break;
        }
    }

    dbg_print("Update to usb loader process finished, try to re-connect device...\n");

    if (true == auto_reboot) {
        usleep(USB_REBOOT_WAIT_DELAY_US);
    } else {
        usleep(USB_DISCONNECT_WAIT_DELAY_US);
    }

    // here USB is disconnected, re-connect it
    if(KP_USB_RET_OK != kp_usb_connect_multiple_devices_v2(_devices_grp->num_device, port_id_list, ll_dev, 100))
        return KP_ERROR_DEVICE_NOT_EXIST_10;

    // FIXME, better code, do this for let FW prepare buffers to receive further commands
    {
        for (int i = 0; i < _devices_grp->num_device; i++)
            _devices_grp->ll_device[i] = ll_dev[i];

        /* Set up fifo queue */
        kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);
    }

    dbg_print("Device is re-connected\n\n");

    return Ret;
}
