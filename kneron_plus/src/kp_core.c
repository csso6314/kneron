/**
 * @file        kp_core.c
 * @brief       core functions
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

// #define DEBUG_PRINT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>

#include <pthread.h>

#include "kp_usb.h"
#include "kp_internal.h"
#include "kp_update_flash.h"

#include "kp_core.h"

#include "kdp2_ipc_cmd.h"
#include "kdp2_inf_generic_raw.h"

#include "internal_func.h"

#include "kp_version.h"

#ifdef _WIN32
    #include "libwdi.h"
#endif

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }
#else
#define dbg_print(format, ...)
#endif

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define USB_DISCONNECT_WAIT_DELAY_US    (500 * 1000)
#define BUFFER_SIZE_10_KB               (10 * 1024)

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

static int kdp_get_kn_number(kp_usb_device_t *ll_dev, uint32_t *kn_number, int timeout)
{
    kdp_get_kn_number_cmd_t kn_num_cmd_buf;
    kn_num_cmd_buf.preamble = KDP_MSG_HDR_CMD;
    kn_num_cmd_buf.ctrl = 0xC008; // Use fixed value for convenience, actually it seems to require some calculation
    kn_num_cmd_buf.cmd = KDP_CMD_GET_KN_NUM;
    kn_num_cmd_buf.msg_len = 0;
    kn_num_cmd_buf.crc = 0x2AB7; // Use fixed value for convenience, actually it seems to require some calculation

    int ret = kp_usb_write_data(ll_dev, (void *)&kn_num_cmd_buf, sizeof(kn_num_cmd_buf), timeout);
    int status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS)
        return status;

    kdp_get_kn_number_response_t kn_num_rsp_buf;

    ret = kp_usb_read_data(ll_dev, (void *)&kn_num_rsp_buf, sizeof(kdp_get_kn_number_response_t), timeout);
    status = check_usb_read_data_error(ret);

    if (status != KP_SUCCESS)
        ret = status;
    else if (ret == sizeof(kdp_get_kn_number_response_t)) // Copy data only when reading success
    {
        if (kn_num_rsp_buf.preamble != KDP_MSG_HDR_RSP ||
            kn_num_rsp_buf.ctrl != 0xC ||
            kn_num_rsp_buf.cmd != KDP_CMD_GET_KN_NUM_RESPONSE ||
            kn_num_rsp_buf.msg_len != sizeof(kn_num_rsp_buf.kn_number) + sizeof(kn_num_rsp_buf.dummy))
            return KP_ERROR_DEVICE_INCORRECT_RESPONSE_11;

        *kn_number = kn_num_rsp_buf.kn_number;
        ret = KP_SUCCESS;
    }
    else
        ret = KP_ERROR_OTHER_99;

    return ret;
}

static int kdp_get_system_status(kp_usb_device_t *ll_dev, kp_firmware_version_t *fw_ver, int timeout)
{
    kdp_system_status_cmd_t fw_ver_cmd_buf;
    fw_ver_cmd_buf.preamble = KDP_MSG_HDR_CMD;
    fw_ver_cmd_buf.ctrl = 0xC008; // Use fixed value for convenience, actually it seems to require some calculation
    fw_ver_cmd_buf.cmd = KDP_CMD_SYSTEM_STATUS;
    fw_ver_cmd_buf.msg_len = 0;
    fw_ver_cmd_buf.crc = 0x1AB6; // Use fixed value for convenience, actually it seems to require some calculation

    int ret = kp_usb_write_data(ll_dev, (void *)&fw_ver_cmd_buf, sizeof(fw_ver_cmd_buf), timeout);
    int status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS)
        return status;

    kdp_system_status_response_t fw_ver_rsp_buf;

    ret = kp_usb_read_data(ll_dev, (void *)&fw_ver_rsp_buf, sizeof(kdp_system_status_response_t), timeout);
    status = check_usb_read_data_error(ret);

    if (status != KP_SUCCESS)
        ret = status;
    else if (ret == sizeof(kdp_system_status_response_t)) // Copy data only when reading success
    {
        if (fw_ver_rsp_buf.preamble != KDP_MSG_HDR_RSP ||
            fw_ver_rsp_buf.ctrl != 0x18 ||
            fw_ver_rsp_buf.cmd != KDP_CMD_SYSTEM_STATUS_RESPONSE ||
            fw_ver_rsp_buf.msg_len != sizeof(fw_ver_rsp_buf.sfirmware_id) + sizeof(fw_ver_rsp_buf.sbuild_id) +
                                          sizeof(fw_ver_rsp_buf.sys_status) + sizeof(fw_ver_rsp_buf.app_status) +
                                          sizeof(fw_ver_rsp_buf.nfirmware_id) + sizeof(fw_ver_rsp_buf.nbuild_id))
            return KP_ERROR_DEVICE_INCORRECT_RESPONSE_11;

        fw_ver->reserved = (fw_ver_rsp_buf.sfirmware_id >> 24) & 0xFF;
        fw_ver->major = (fw_ver_rsp_buf.sfirmware_id >> 16) & 0xFF;
        fw_ver->minor = (fw_ver_rsp_buf.sfirmware_id >> 8) & 0xFF;
        fw_ver->update = (fw_ver_rsp_buf.sfirmware_id) & 0xFF;
        fw_ver->build = fw_ver_rsp_buf.sbuild_id;

        ret = KP_SUCCESS;
    }
    else
        ret = KP_ERROR_OTHER_99;

    return ret;
}

static int get_system_info(kp_usb_device_t *ll_dev, kp_system_info_t *system_info, int timeout)
{
    // check the firmware version
    if (((ll_dev->fw_serial & KP_KDP2_FW_V2) == KP_KDP2_FW_V2) ||
        ((ll_dev->fw_serial & KP_KDP2_FW) == KP_KDP2_FW)) // KDP2 FW or KDP2 Loader or KDP2 JTAG
    {
        kdp2_ipc_cmd_get_system_info_t cmd_buf;

        cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
        cmd_buf.command_id = KDP2_COMMAND_GET_SYSTEM_INFO;

        int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, sizeof(cmd_buf), timeout);
        int status = check_usb_write_data_error(ret);
        if (status != KP_SUCCESS)
            return status;

        // Wait for FW getting system info ? Need some delay ?

        kdp2_ipc_response_get_system_info_t response_buf;

        ret = kp_usb_read_data(ll_dev, (void *)&response_buf, sizeof(kdp2_ipc_response_get_system_info_t), timeout);
        status = check_usb_read_data_error(ret);

        if (status != KP_SUCCESS)
            ret = status;
        else if (response_buf.return_code != KP_SUCCESS)
            ret = response_buf.return_code;
        else if (ret == sizeof(kdp2_ipc_response_get_system_info_t)) // Copy data only when reading success
        {
            memcpy(system_info, &response_buf.system_info, sizeof(kp_system_info_t));
            ret = KP_SUCCESS;
        }
        else
            ret = KP_ERROR_OTHER_99;

        return ret;
    }
    else // KDP FW
    {
        int ret = kdp_get_kn_number(ll_dev, &system_info->kn_number, timeout);
        if (ret != KP_SUCCESS)
            return ret;

        ret = kdp_get_system_status(ll_dev, &system_info->firmware_version, timeout);
        if (ret != KP_SUCCESS)
            return ret;

        return KP_SUCCESS;
    }
}

kp_devices_list_t *kp_scan_devices()
{
    return kp_usb_scan_devices();
}

// FIXME
static int _kp_set_up_inference_queues(kp_device_group_t devices, uint32_t image_count, uint32_t image_size, uint32_t result_count, uint32_t result_size);

kp_device_group_t connect_devices(int num_devices, int device_port_ids[], int *error_code, bool with_examination)
{
    int re_connect_device_times = 0;

CONNECT_DEVICE:

    if (num_devices > MAX_GROUP_DEVICE || num_devices < 1)
    {
        if (error_code)
            *error_code = KP_ERROR_DEVICES_NUMBER_26;
        return NULL;
    }

    if (num_devices == 1 && device_port_ids[0] == 0)
    {
        // auto-scan first device to connect
        kp_devices_list_t *list;
        list = kp_scan_devices();

        if (list->num_dev < 1)
        {
            if (error_code)
                *error_code = KP_ERROR_DEVICE_NOT_EXIST_10;
            return NULL;
        }

        device_port_ids[0] = list->device[0].port_id;
    }

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)malloc(sizeof(_kp_devices_group_t));
    if (NULL == _devices_grp)
    {
        if (error_code)
            *error_code = KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
        return NULL;
    }

    memset(_devices_grp, 0, sizeof(_kp_devices_group_t));

    int ret = kp_usb_connect_multiple_devices_v2(num_devices, device_port_ids, _devices_grp->ll_device, 10);

    if (ret != KP_USB_RET_OK)
    {
        if (error_code)
        {
            if (ret < 0)
                *error_code = ret;
            else if (ret == KP_USB_CONFIGURE_ERR)
                *error_code = KP_ERROR_CONFIGURE_DEVICE_27;
            else
                *error_code = KP_ERROR_CONNECT_FAILED_28;
        }

        free(_devices_grp);
        return NULL;
    }

    // further check things

    bool check_pass = true;
    uint16_t first_dev_pid = _devices_grp->ll_device[0]->dev_descp.product_id;

    for (int i = 0; i < num_devices; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];

        // check mix use of different target platforms (KL520, KL720 ...etc)
        // only same target devices can be grouped
        if (ll_dev->dev_descp.product_id != first_dev_pid)
        {
            if (error_code)
                *error_code = KP_ERROR_DEVICE_GROUP_MIX_PRODUCT_29;
            check_pass = false;
            break;
        }

        // check if it is in KDP2 Loader or KDP2 FW status, but allow connection to KDP FW if error_code input is KDP_MAGIC_CONNECTION_PASS
        if ((((ll_dev->fw_serial & KP_KDP2_FW_V2) != KP_KDP2_FW_V2) &&
             ((ll_dev->fw_serial & KP_KDP2_FW) != KP_KDP2_FW)) &&
            (!error_code || *error_code != KDP_MAGIC_CONNECTION_PASS))
        {
            if (error_code)
                *error_code = KP_ERROR_INVALID_FIRMWARE_24;
            check_pass = false;
            break;
        }
    }

    if (!check_pass)
    {
        kp_usb_disconnect_multiple_devices(num_devices, _devices_grp->ll_device);
        free(_devices_grp);
        return NULL;
    }

    _devices_grp->num_device = num_devices;
    _devices_grp->timeout = 0;
    _devices_grp->cur_send = 0;
    _devices_grp->cur_recv = 0;
    _devices_grp->product_id = first_dev_pid;
    _devices_grp->loaded_model_desc.num_models = 0;

    /* Set up fifo queue */
    kp_reset_device((kp_device_group_t)_devices_grp, KP_RESET_INFERENCE);

    if (false == with_examination) {
        goto FUNC_OUT;
    }

    /* check kneron plus & firmware version is compatible for flash-boot */
    bool is_device_connected = true;
    for (int i = 0; i < num_devices; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
        uint16_t fw_type_legacy = ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK;
        uint16_t fw_type = ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK_V2;

        if ((KP_KDP2_FW_USB_TYPE_V2 == fw_type) ||
            (KP_KDP2_FW_FLASH_TYPE_V2 == fw_type) ||
            (KP_KDP2_FW_USB_TYPE == fw_type_legacy) ||
            (KP_KDP2_FW_FLASH_TYPE == fw_type_legacy)) {
            kp_system_info_t system_info = {0};
            int ret = get_system_info(ll_dev, &system_info, _devices_grp->timeout);
            const int *fw_version = NULL;

            if (KP_USB_RET_OK == ret) {
                if (KP_DEVICE_KL520 == ll_dev->dev_descp.product_id) {
                    fw_version = kl520_fw_version;
                } else if ((KP_DEVICE_KL720 == ll_dev->dev_descp.product_id) ||
                           (KP_DEVICE_KL720_LEGACY == ll_dev->dev_descp.product_id)) {
                    fw_version = kl720_fw_version;
                } else if (KP_DEVICE_KL630 == ll_dev->dev_descp.product_id) {
                    fw_version = kl630_fw_version;
                } else {
                    printf("invalid device product ID ... %d\n", ll_dev->dev_descp.product_id);
                    continue;
                }

                if ((fw_version[VERSION_INDEX_MAJOR] == 0) &&
                    (fw_version[VERSION_INDEX_MINOR] == 0) &&
                    (fw_version[VERSION_INDEX_REVISION] == 0) &&
                    (fw_version[VERSION_INDEX_BUILD] == 0)) {
                    /* no firmware version check when kp_version.h is default setting */
                    continue;
                }

                if ((fw_version[VERSION_INDEX_MAJOR] != system_info.firmware_version.major) ||
                    (fw_version[VERSION_INDEX_MINOR] != system_info.firmware_version.minor) ||
                    (fw_version[VERSION_INDEX_REVISION] != system_info.firmware_version.update) ||
                    (fw_version[VERSION_INDEX_BUILD] != system_info.firmware_version.build)) {
                    printf("\033[0;33m[warnning] The version of firmware (%d.%d.%d.%d) on the port ID %u is not the corresponding version for this Kneron PLUS (%d.%d.%d.%d).\n\033[0m",
                           system_info.firmware_version.major, system_info.firmware_version.minor, system_info.firmware_version.update, system_info.firmware_version.build,
                           ll_dev->dev_descp.port_id,
                           fw_version[VERSION_INDEX_MAJOR], fw_version[VERSION_INDEX_MINOR], fw_version[VERSION_INDEX_REVISION], fw_version[VERSION_INDEX_BUILD]);
                    fflush(stdout);
                }
            } else {
                is_device_connected = false;
                break;
            }
        }
    }

    if (!is_device_connected) {
        if (3 > re_connect_device_times) {
            kp_disconnect_devices((kp_device_group_t)_devices_grp);
            usleep(USB_DISCONNECT_WAIT_DELAY_US * (re_connect_device_times + 1));
            re_connect_device_times++;
            goto CONNECT_DEVICE;
        } else {
            kp_disconnect_devices((kp_device_group_t)_devices_grp);

            if (error_code)
                *error_code = KP_ERROR_CONNECT_FAILED_28;

            return NULL;
        }
    }

FUNC_OUT:

    if (error_code)
        *error_code = KP_SUCCESS; // connect success

    return (kp_device_group_t)_devices_grp;
}

kp_device_group_t kp_connect_devices(int num_devices, int device_port_ids[], int *error_code)
{
    /* Do connect devices with examination of system info */
    return connect_devices(num_devices, device_port_ids, error_code, true);
}

kp_device_group_t kp_connect_devices_without_check(int num_devices, int device_port_ids[], int *error_code)
{
    /* Do connect devices without examination of system info */
    return connect_devices(num_devices, device_port_ids, error_code, false);
}

int kp_disconnect_devices(kp_device_group_t devices)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_release_model_nef_descriptor(&(_devices_grp->loaded_model_desc));

    for (int i = 0; i < _devices_grp->num_device; i++)
        kp_usb_disconnect_device(_devices_grp->ll_device[i]);

    free(_devices_grp);

    return KP_SUCCESS;
}

void kp_set_timeout(kp_device_group_t devices, int milliseconds)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    _devices_grp->timeout = milliseconds;
}

typedef struct
{
    kp_usb_device_t *ll_device;
    kdp2_ipc_cmd_load_model_t *cmd_buf;
    void *model_buf;
    int timeout;
    int sts;
} _load_model_command_package;

static void *_load_model_to_single_device(void *data)
{
    _load_model_command_package *cmd_pack = (_load_model_command_package *)data;

    dbg_print("[%s] thread : device %p, cmd_buf %p, total_size %d, timeout %d\n", __FUNCTION__, cmd_pack->ll_device, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->total_size, cmd_pack->timeout);

    // write command header including fw_info buffer
    int ret = kp_usb_write_data(cmd_pack->ll_device, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->total_size, cmd_pack->timeout);
    if (ret != KP_USB_RET_OK)
    {
        cmd_pack->sts = check_usb_read_data_error(ret);
        return NULL;
    }

    dbg_print("[%s] thread : device %p, model_buf %p, model_size %d, timeout %d\n", __FUNCTION__, cmd_pack->ll_device, (void *)cmd_pack->model_buf, cmd_pack->cmd_buf->model_size, cmd_pack->timeout);

    uint32_t return_code;
    int status;
    ret = kp_usb_read_data(cmd_pack->ll_device, (void *)&return_code, sizeof(uint32_t), cmd_pack->timeout);
    status = check_usb_read_data_error(ret);

    dbg_print("[%s] load model info sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

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

    // write model buffer
    ret = kp_usb_write_data(cmd_pack->ll_device, (void *)cmd_pack->model_buf, cmd_pack->cmd_buf->model_size, cmd_pack->timeout);
    cmd_pack->sts = check_usb_read_data_error(ret);

    return NULL;
}

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    kdp2_ipc_cmd_load_model_from_flash_t *cmd_buf;
    int timeout;
    int sts;
} _load_model_from_flash_command_package;

static void *_load_single_device_model_from_flash(void *data)
{
    _load_model_from_flash_command_package *cmd_pack = (_load_model_from_flash_command_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;

    dbg_print("[%s] thread : device %p, cmd_buf %p, timeout %d\n", __FUNCTION__,
              cmd_pack->ll_device, (void *)cmd_pack->cmd_buf, cmd_pack->timeout);

    int ret = kp_usb_write_data(ll_dev, cmd_pack->cmd_buf, sizeof(kdp2_ipc_cmd_load_model_from_flash_t), cmd_pack->timeout);

    if (KP_USB_RET_OK != ret) {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write cmd_buf failed, error %d\n", __FUNCTION__, ll_dev->dev_descp.port_id, cmd_pack->sts);
        return NULL;
    }

    uint32_t return_code;

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(return_code), cmd_pack->timeout);
    int status = check_usb_read_data_error(ret);

    dbg_print("[%s] load model info sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(uint32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    if (ret != KP_SUCCESS) {
        cmd_pack->sts = ret;
        return NULL;
    }

    cmd_pack->sts = return_code;

    return NULL;
}

typedef struct
{
    kp_usb_device_t *ll_device;
    kdp2_ipc_cmd_load_nef_t *cmd_buf;
    void *nef_buf;
    int timeout;
    int sts;
} _load_nef_command_package;

static void *_load_nef_to_single_device(void *data)
{
    _load_nef_command_package *cmd_pack = (_load_nef_command_package *)data;

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

    dbg_print("[%s] load nef info sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

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

    dbg_print("[%s] load nef info sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(uint32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    cmd_pack->sts = ret;

    return NULL;
}

// #define DEBUG_MODEL

static int _spawn_thread_to_load_model_to_devices(int num_device, _load_model_command_package cmd_packs[], pthread_t load_model_thd[])
{
    for (int i = 1; i < num_device; i++)
    {
        dbg_print("[%s] create thread to upload model to device %d\n", __FUNCTION__, i);

        int thd_ret = pthread_create(&load_model_thd[i], NULL, _load_model_to_single_device, (void *)&cmd_packs[i]);
        if (thd_ret != 0)
        {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return KP_ERROR_OTHER_99;
        }
    }

    // current thread do first device
    _load_model_to_single_device((void *)&cmd_packs[0]);

    for (int i = 1; i < num_device; i++)
        pthread_join(load_model_thd[i], NULL);

    // check all thread upload model status
    for (int i = 0; i < num_device; i++)
    {
        int ret = cmd_packs[i].sts;
        if (ret != KP_SUCCESS)
        {
            dbg_print("[%s] thread upload model failed at device %d, error %d\n", __FUNCTION__, i, ret);
            return ret;
        }
    }

    return KP_SUCCESS;
}

static int _spawn_thread_to_load_nef_to_devices(int num_device, _load_nef_command_package cmd_packs[], pthread_t load_model_thd[])
{
    for (int i = 1; i < num_device; i++)
    {
        dbg_print("[%s] create thread to upload model to device %d\n", __FUNCTION__, i);

        int thd_ret = pthread_create(&load_model_thd[i], NULL, _load_nef_to_single_device, (void *)&cmd_packs[i]);
        if (thd_ret != 0)
        {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return KP_ERROR_OTHER_99;
        }
    }

    // current thread do first device
    _load_nef_to_single_device((void *)&cmd_packs[0]);

    for (int i = 1; i < num_device; i++)
        pthread_join(load_model_thd[i], NULL);

    // check all thread upload model status
    for (int i = 0; i < num_device; i++)
    {
        int ret = cmd_packs[i].sts;
        if (ret != KP_SUCCESS)
        {
            dbg_print("[%s] thread upload model failed at device %d, error %d\n", __FUNCTION__, i, ret);
            return ret;
        }
    }

    return KP_SUCCESS;
}

static int _kp_adjust_ddr_heap_boundary(kp_device_group_t devices, uint32_t boundary_addr)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;
    int ret = KP_SUCCESS;

    kp_usb_control_t kctrl;

    kctrl.command = KDP2_CONTROL_DDR_HEAP_BOUNDARY_ADJUST;
    kctrl.arg1 = (uint16_t)(boundary_addr >> 16);
    kctrl.arg2 = (uint16_t)(boundary_addr & 0x0000FFFF);

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];

        ret = kp_usb_control(ll_dev, &kctrl, timeout);

        if (ret != KP_USB_RET_OK)
        {
            // this could happen if FW has already complete the set-up
            dbg_print("[%s] adjust ddr heap boundary failed return code = [%d]\n", __func__, ret);
            ret = KP_ERROR_ADJUST_DDR_HEAP_FAILED_46;
            break;
        }
    }

    return ret;
}

static int _kp_get_device_available_ddr_config(kp_device_group_t devices, kp_available_ddr_config_t *ddr_config)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;

    kdp2_ipc_cmd_get_available_ddr_config_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_get_available_ddr_config_t);
    cmd_buf.command_id = KDP2_COMMAND_GET_DDR_CONFIG;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        int status = kp_usb_write_data(_devices_grp->ll_device[i], (void *)&cmd_buf, sizeof(kdp2_ipc_cmd_get_available_ddr_config_t), timeout);

        if (KP_SUCCESS == status) {
            status = kp_usb_read_data(_devices_grp->ll_device[i], (void *)ddr_config, sizeof(kp_available_ddr_config_t), timeout);

            if (0 < status) {
                return KP_SUCCESS;
            }
        }
    }

    return KP_ERROR_OTHER_99;
}

static int _kp_get_device_fifo_queue_config(kp_device_group_t devices, kp_fifo_queue_config_t *fifo_queue_config)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;

    kdp2_ipc_cmd_get_fifo_queue_config_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_get_fifo_queue_config_t);
    cmd_buf.command_id = KDP2_COMMAND_GET_FIFOQ_CONFIG;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        int status = kp_usb_write_data(_devices_grp->ll_device[i], (void *)&cmd_buf, sizeof(kdp2_ipc_cmd_get_fifo_queue_config_t), timeout);

        if (KP_SUCCESS == status) {
            status = kp_usb_read_data(_devices_grp->ll_device[i], (void *)fifo_queue_config, sizeof(kp_fifo_queue_config_t), timeout);

            if (0 < status) {
                return KP_SUCCESS;
            }
        }
    }

    return KP_ERROR_OTHER_99;
}

#define DEFAULT_INPUT_BUF_COUNT         3
#define DEFAULT_RESULT_BUF_COUNT        3
#define MINIMUM_INPUT_BUF_COUNT         2
#define MINIMUM_RESULT_BUF_COUNT        1
#define SIZE_RESERVED_FOR_HEADER        1024
#define AUTO_ALLOCATE_MIN_INPUT_SIZE    ((2 * 1920 * 1080) + SIZE_RESERVED_FOR_HEADER)
#define AUTO_ALLOCATE_MAX_INPUT_SIZE    ((2 * 3840 * 2160) + SIZE_RESERVED_FOR_HEADER)
#define KL520_SYSTEM_RESERVE_FOR_OTHERS (15 * 1024 * 1024)  // 15MB
#define KL720_SYSTEM_RESERVE_FOR_OTHERS (15 * 1024 * 1024)  // 15MB

static void _kp_set_ddr_attr_to_zero(kp_device_group_t devices)
{
    kp_ddr_manage_attr_t *ddr_attr = &devices->ddr_attr;

    ddr_attr->input_buffer_count = 0;
    ddr_attr->input_buffer_size = 0;
    ddr_attr->result_buffer_count = 0;
    ddr_attr->result_buffer_size = 0;
}

static int _kp_allocate_ddr_memory(kp_device_group_t devices)
{
    kp_model_nef_descriptor_t *model_desc = &devices->loaded_model_desc;
    kp_ddr_manage_attr_t *ddr_attr = &devices->ddr_attr;
    kp_available_ddr_config_t ddr_config;
    kp_fifo_queue_config_t fifo_queue_config;
    uint32_t heap_size = 0;
    uint32_t available_ddr_size = 0;
    uint32_t heap_boundary_addr = 0;
    uint32_t auto_allocate_min_input_size = (uint32_t)ceil(((double)AUTO_ALLOCATE_MIN_INPUT_SIZE / BUFFER_SIZE_10_KB)) * BUFFER_SIZE_10_KB;
    uint32_t auto_allocate_max_input_size = (uint32_t)ceil(((double)AUTO_ALLOCATE_MAX_INPUT_SIZE / BUFFER_SIZE_10_KB)) * BUFFER_SIZE_10_KB;
    uint32_t min_input_buffer_count = MINIMUM_INPUT_BUF_COUNT;

    int ret = _kp_get_device_available_ddr_config(devices, &ddr_config);

    if (KP_SUCCESS != ret) {
        return ret;
    } else if (0 != ddr_config.ddr_fifoq_allocated) {
        dbg_print("[%s] Fifoq memory has been allocated.\n", __FUNCTION__);
        devices->ddr_attr.model_size = ddr_config.ddr_model_end - ddr_config.ddr_available_begin;

        ret = _kp_get_device_fifo_queue_config(devices, &fifo_queue_config);

        if (KP_SUCCESS != ret) {
            return ret;
        } else {
            devices->ddr_attr.input_buffer_count = fifo_queue_config.fifoq_input_buf_count;
            devices->ddr_attr.input_buffer_size = fifo_queue_config.fifoq_input_buf_size;
            devices->ddr_attr.result_buffer_count = fifo_queue_config.fifoq_result_buf_count;
            devices->ddr_attr.result_buffer_size = fifo_queue_config.fifoq_result_buf_size;
        }

        return KP_SUCCESS;
    }

    available_ddr_size = ddr_config.ddr_available_end - ddr_config.ddr_available_begin;

    if (0 == ddr_attr->model_size) {
        ddr_attr->model_size = ddr_config.ddr_model_end - ddr_config.ddr_available_begin;
    }

    heap_size = available_ddr_size - ddr_attr->model_size;
    heap_boundary_addr = ddr_config.ddr_available_end - heap_size;

    ret = _kp_adjust_ddr_heap_boundary(devices, heap_boundary_addr);

    if (KP_SUCCESS != ret) {
        return ret;
    }

    if (KP_DEVICE_KL520 == devices->product_id) {
        heap_size -= KL520_SYSTEM_RESERVE_FOR_OTHERS;
    } else if ((KP_DEVICE_KL720 == devices->product_id) ||
               (KP_DEVICE_KL720_LEGACY == devices->product_id)) {
        heap_size -= KL720_SYSTEM_RESERVE_FOR_OTHERS;
    } else if (KP_DEVICE_KL630 == devices->product_id) {
        /* Do nothing for KL630 */
    } else {
        printf("[%s] Error: The target device product ID 0x%X is unsupported\n", __FUNCTION__, devices->product_id);
        return KP_ERROR_UNSUPPORTED_DEVICE_44;
    }

    /* find max model raw output buff size */
    if (0 == ddr_attr->result_buffer_size) {
        int model_count = model_desc->num_models;

        for (int i = 0; i < model_count; i++) {
            if (ddr_attr->result_buffer_size < model_desc->models[i].max_raw_out_size) {
                ddr_attr->result_buffer_size = (uint32_t)ceil((double)(model_desc->models[i].max_raw_out_size + SIZE_RESERVED_FOR_HEADER) / BUFFER_SIZE_10_KB) * BUFFER_SIZE_10_KB;
            }
        }
    }

    /* find max model input node count */
    if (0 == ddr_attr->input_buffer_count) {
        int model_count = model_desc->num_models;

        for (int i = 0; i < model_count; i++) {
            if (min_input_buffer_count < model_desc->models[i].input_nodes_num) {
                min_input_buffer_count = model_desc->models[i].input_nodes_num;
            }
        }
    }

    bool fix_input_buffer_count = (0 != ddr_attr->input_buffer_count);
    bool fix_result_buffer_count = (0 != ddr_attr->result_buffer_count);
    bool fix_input_buffer_size = (0 != ddr_attr->input_buffer_size);
    uint32_t base_input_buf_count_default = (DEFAULT_INPUT_BUF_COUNT > min_input_buffer_count) ? DEFAULT_INPUT_BUF_COUNT : min_input_buffer_count;
    uint32_t base_input_buf_count = base_input_buf_count_default;
    uint32_t base_result_buf_count = DEFAULT_RESULT_BUF_COUNT;

    if ((true == fix_input_buffer_count) &&
        (true == fix_result_buffer_count) &&
        (true == fix_input_buffer_size)) {
        goto FUNC_OUT;
    }

    /* auto calculated input/result buffer count */
    for (int i = 0; ; i++) {
        if (false == fix_input_buffer_count) {
            /* the input buffer count must large than model input node number in generic inference */
            if (min_input_buffer_count > base_input_buf_count) {
                printf("[%s] Error: The auto calculated input buffer count %u is less than minimum input buffer count %u\n", __FUNCTION__, base_input_buf_count, min_input_buffer_count);
                return KP_ERROR_FIFOQ_SETTING_FAILED_43;
            }

            ddr_attr->input_buffer_count = base_input_buf_count;
        }

        if (false == fix_result_buffer_count) {
            if (MINIMUM_RESULT_BUF_COUNT > base_result_buf_count) {
                printf("[%s] Error: The auto calculated result buffer count %u is less than minimum result buffer count %u\n", __FUNCTION__, base_result_buf_count, MINIMUM_RESULT_BUF_COUNT);
                _kp_set_ddr_attr_to_zero(devices);
                return KP_ERROR_FIFOQ_SETTING_FAILED_43;
            }

            ddr_attr->result_buffer_count = base_result_buf_count;
        }

        uint32_t result_total_size = ddr_attr->result_buffer_count * ddr_attr->result_buffer_size;

        if ((0 == ddr_attr->input_buffer_count) || (0 == ddr_attr->result_buffer_count)) {
            printf("[%s] Error: The input result buffer count %u or result buffer count %u can not be 0\n", __FUNCTION__, ddr_attr->input_buffer_count, ddr_attr->result_buffer_count);
            _kp_set_ddr_attr_to_zero(devices);
            return KP_ERROR_FIFOQ_SETTING_FAILED_43;
        }

        /* find the maximum result buffer count if the result buffer is oversize */
        if (result_total_size >= heap_size) {
            if (true == fix_result_buffer_count) {
                printf("[%s] Error: The total auto calculated result buffer size %u (bytes) is large than heap_size %u (bytes)\n", __FUNCTION__, result_total_size, heap_size);
                _kp_set_ddr_attr_to_zero(devices);
                return KP_ERROR_FIFOQ_SETTING_FAILED_43;
            }

            base_result_buf_count--;
            continue;
        }

        uint32_t remain_heap_size = heap_size - result_total_size;

        /* find the suitable input/result buffer count in limited heap size */
        if (false == fix_input_buffer_size) {
            /* auto calculated input buff size */

            uint32_t size_for_one_input = ((int)(remain_heap_size / ddr_attr->input_buffer_count / BUFFER_SIZE_10_KB)) * BUFFER_SIZE_10_KB;

            if (auto_allocate_min_input_size < size_for_one_input) {
                ddr_attr->input_buffer_size = MIN(size_for_one_input, auto_allocate_max_input_size);
                break;
            } else if ((true == fix_input_buffer_count) && (true == fix_result_buffer_count)) {
                _kp_set_ddr_attr_to_zero(devices);
                return KP_ERROR_FIFOQ_SETTING_FAILED_43;
            } else if ((true == fix_input_buffer_count) && (false == fix_result_buffer_count)) {
                base_result_buf_count--;
                continue;
            } else if ((false == fix_input_buffer_count) && (true == fix_result_buffer_count)) {
                base_input_buf_count--;
                continue;
            } else if ((false == fix_input_buffer_count) && (false == fix_result_buffer_count)) {
                /**
                 * calculated input/result buffer count rules:
                 * (1) change large one first (and the base_input_buf_count must large than min_input_buffer_count)
                 * (2) base_input_buf_count must large than result_buffer_count
                 * (3) reset base_input_buf_count when try to decrease base_result_buf_count
                 */

                if ((ddr_attr->input_buffer_count > ddr_attr->result_buffer_count) &&
                    (base_input_buf_count > min_input_buffer_count)) {
                    base_input_buf_count--;
                    continue;
                } else {
                    base_result_buf_count--;
                    base_input_buf_count = base_input_buf_count_default;
                    continue;
                }
            }
        } else if (remain_heap_size < (ddr_attr->input_buffer_size * ddr_attr->input_buffer_count)) {
            /* using customized input buff size */

            if ((true == fix_input_buffer_count) && (false == fix_result_buffer_count)) {
                base_result_buf_count--;
                continue;
            } else if ((false == fix_input_buffer_count) && (true == fix_result_buffer_count)) {
                base_input_buf_count--;
                continue;
            } else if ((false == fix_input_buffer_count) && (false == fix_result_buffer_count)) {
                /**
                 * calculated input/result buffer count rules:
                 * (1) change large one first (and the base_input_buf_count must large than min_input_buffer_count)
                 * (2) base_input_buf_count must large than result_buffer_count
                 * (3) reset base_input_buf_count when try to decrease base_result_buf_count
                 */

                if ((ddr_attr->input_buffer_count > ddr_attr->result_buffer_count) &&
                    (base_input_buf_count > min_input_buffer_count)) {
                    base_input_buf_count--;
                    continue;
                } else {
                    base_result_buf_count--;
                    base_input_buf_count = base_input_buf_count_default;
                    continue;
                }
            }
        } else {
            break;
        }
    }


FUNC_OUT:

    /* align input/output buffer size with BUFFER_SIZE_10_KB */
    ddr_attr->result_buffer_size = (uint32_t)ceil((double)ddr_attr->result_buffer_size / BUFFER_SIZE_10_KB) * BUFFER_SIZE_10_KB;
    ddr_attr->input_buffer_size = (uint32_t)ceil((double)ddr_attr->input_buffer_size / BUFFER_SIZE_10_KB) * BUFFER_SIZE_10_KB;

    dbg_print("[%s] input buf %u x %u, result buf %u x %u\n", __FUNCTION__,
                                                              ddr_attr->input_buffer_count,
                                                              ddr_attr->input_buffer_size,
                                                              ddr_attr->result_buffer_count,
                                                              ddr_attr->result_buffer_size);

    if (heap_size < (ddr_attr->input_buffer_count * ddr_attr->input_buffer_size) +
                    (ddr_attr->result_buffer_count * ddr_attr->result_buffer_size)) {
        printf("[%s] Error: Heap memory %d is not sufficient for input buf %u x %u, result buf %u x %u\n", __FUNCTION__,
                                                                                                           heap_size,
                                                                                                           ddr_attr->input_buffer_count,
                                                                                                           ddr_attr->input_buffer_size,
                                                                                                           ddr_attr->result_buffer_count,
                                                                                                           ddr_attr->result_buffer_size);
        _kp_set_ddr_attr_to_zero(devices);
        ret = KP_ERROR_FIFOQ_SETTING_FAILED_43;
    } else {
        ret = _kp_set_up_inference_queues(devices, ddr_attr->input_buffer_count, ddr_attr->input_buffer_size / BUFFER_SIZE_10_KB,
                                          ddr_attr->result_buffer_count, ddr_attr->result_buffer_size / BUFFER_SIZE_10_KB);

        if (KP_SUCCESS != ret) {
            printf("[%s] Error: Fifo Queue setup failed for input buf %u x %u, result buf %u x %u\n", __FUNCTION__,
                                                                                                      ddr_attr->input_buffer_count,
                                                                                                      ddr_attr->input_buffer_size,
                                                                                                      ddr_attr->result_buffer_count,
                                                                                                      ddr_attr->result_buffer_size);
            _kp_set_ddr_attr_to_zero(devices);
            ret = KP_ERROR_FIFOQ_SETTING_FAILED_43;
        }
    }

    return ret;
}

int reboot_one_device(kp_device_group_t devices, int dev_index)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t *ll_dev = _devices_grp->ll_device[dev_index];
    int timeout = _devices_grp->timeout;

    // reset/reboot the device
    kp_usb_control_t kctrl = {KDP2_CONTROL_REBOOT, 0, 0};
    int ret = kp_usb_control(ll_dev, &kctrl, timeout);
    if ((ret != KP_USB_USB_PIPE) &&
        (ret != KP_USB_USB_IO) &&
        (ret != KP_USB_USB_NO_DEVICE) &&
        (ret != KP_USB_USB_NOT_FOUND) &&
        (ret != KP_SUCCESS)) {// FIXME: tricky way
                                // Note that device return KP_SUCCESS in some Windows device and KP_USB_USB_NO_DEVICE in Ubuntu
        dbg_print("[%s] Send reboot command to usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
        return KP_ERROR_RESET_FAILED_25;
    }

    kp_usb_disconnect_device(ll_dev);

    return KP_SUCCESS;
}

int reboot_if_model_is_loaded(kp_device_group_t devices)
{
    int ret = KP_SUCCESS;
    bool usb_boot = false;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int num_device = _devices_grp->num_device;
    int num_reboot_device = 0;
    uint32_t *reboot_dev_port_id = (uint32_t *)malloc(num_device * sizeof(uint32_t));
    uint32_t *reboot_dev_scan_idx = (uint32_t *)malloc(num_device * sizeof(uint32_t));

    /* Check whether usb boot exist */
    for (int i = 0; i < _devices_grp->num_device; i++) {
        if ((KP_KDP2_FW_USB_TYPE_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & _devices_grp->ll_device[i]->fw_serial)) ||
            (KP_KDP2_FW_USB_TYPE == (KP_KDP2_FW_FIND_TYPE_MASK & _devices_grp->ll_device[i]->fw_serial))) {
            usb_boot = true;
            break;
        }
    }

    /* Reboot all devices if _devices_grp has loaded model */
    if (0 != _devices_grp->loaded_model_desc.num_models) {
        for (int i = 0; i < _devices_grp->num_device; i++) {
            if (false == usb_boot) {
                reboot_dev_port_id[num_reboot_device] = _devices_grp->ll_device[i]->dev_descp.port_id;
                reboot_dev_scan_idx[num_reboot_device] = i;
                ret = reboot_one_device(devices, i);
            } else {
                ret = KP_ERROR_USB_BOOT_LOAD_SECOND_MODEL_40;
            }

            if (KP_SUCCESS != ret) {
                goto FUNC_OUT;
            }

            num_reboot_device++;
        }

        goto RECONNECT;
    }

    kp_model_nef_descriptor_t all_models_desc;
    int port_id = 0;

    /* Reboot the device which has loaded model */
    for (int i = 0; i < _devices_grp->num_device; i++) {
        port_id = _devices_grp->ll_device[i]->dev_descp.port_id;
        ret = kp_get_model_info(devices, port_id, &all_models_desc);

        if (KP_SUCCESS != ret) {
            continue;
        } else if (0 != all_models_desc.num_models) {
            if (false == usb_boot) {
                reboot_dev_port_id[num_reboot_device] = _devices_grp->ll_device[i]->dev_descp.port_id;
                reboot_dev_scan_idx[num_reboot_device] = i;
                ret = reboot_one_device(devices, i);
            } else {
                ret = KP_ERROR_USB_BOOT_LOAD_SECOND_MODEL_40;
            }

            if (KP_SUCCESS != ret) {
                kp_release_model_nef_descriptor(&all_models_desc);
                goto FUNC_OUT;
            }

            num_reboot_device++;
        }

        kp_release_model_nef_descriptor(&all_models_desc);
    }

RECONNECT:

    if (0 == num_reboot_device) {
        goto FUNC_OUT;
    }

    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    for (int i = 0; i < num_reboot_device; i++) {
        int port_ids[1] = {reboot_dev_port_id[i]};
        kp_usb_device_t *devs[1];

        for(int j = 0; j < 3; j++) {
            ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);

            if (KP_USB_RET_OK != ret) {
                usleep(USB_DISCONNECT_WAIT_DELAY_US * (j + 1));
                continue;
            }

            kp_system_info_t system_info;

            ret = get_system_info(devs[0], &system_info, _devices_grp->timeout);

            if (KP_USB_RET_OK != ret) {
                kp_usb_disconnect_device(devs[0]);
                usleep(USB_DISCONNECT_WAIT_DELAY_US * (j + 1));
                continue;
            } else {
                break;
            }
        }

        if (KP_USB_RET_OK != ret) {
            ret = KP_ERROR_DEVICE_NOT_EXIST_10;
            goto FUNC_OUT;
        }

        // update back
        _devices_grp->ll_device[reboot_dev_scan_idx[i]] = devs[0];
    }

FUNC_OUT:

    free(reboot_dev_port_id);
    free(reboot_dev_scan_idx);

    return KP_SUCCESS;
}

static int check_fw_is_loaded(kp_device_group_t devices)
{
    int ret = KP_SUCCESS;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        if ((KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & _devices_grp->ll_device[i]->fw_serial)) ||
            (KP_KDP2_FW_LOADER == (KP_KDP2_FW_FIND_TYPE_MASK & _devices_grp->ll_device[i]->fw_serial)) ||
            (KP_KDP_FW == _devices_grp->ll_device[i]->fw_serial) ||
            (KP_KDP2_FW_KL720_USB_DFU == _devices_grp->ll_device[i]->fw_serial) ||
            (KP_KDP2_FW_KL720_LOADER == _devices_grp->ll_device[i]->fw_serial)) {
            ret = KP_ERROR_INVALID_FIRMWARE_24;
            break;
        }
    }

    return ret;
}

int kp_load_model(kp_device_group_t devices, void *nef_buf, int nef_size, kp_model_nef_descriptor_t *model_desc)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    kp_metadata_t metadata;
    kp_nef_info_t nef_info;

    int ret = check_fw_is_loaded(devices);

    if (KP_SUCCESS != ret)
        return ret;

    ret = reboot_if_model_is_loaded(devices);

    if (KP_SUCCESS != ret)
        return ret;

    ret = load_model_info_from_nef(nef_buf, nef_size, _devices_grp->product_id, &metadata, &nef_info, &(_devices_grp->loaded_model_desc));

    if (KP_SUCCESS != ret)
        return ret;

    // The input model is an encrypted model
    if ((metadata.kn_num != 0 && metadata.enc_type != 0) &&
        (_devices_grp->num_device > 1 || _devices_grp->ll_device[0]->dev_descp.kn_number != metadata.kn_num))
         return KP_ERROR_INVALID_MODEL_21;

    if (KP_DEVICE_KL630 == devices->product_id) {
        kdp2_ipc_cmd_load_nef_t *cmd_buf = (kdp2_ipc_cmd_load_nef_t *)malloc(sizeof(kdp2_ipc_cmd_load_nef_t));

        cmd_buf->magic_type = KDP2_MAGIC_TYPE_COMMAND;
        cmd_buf->total_size = sizeof(kdp2_ipc_cmd_load_nef_t);
        cmd_buf->command_id = KDP2_COMMAND_LOAD_NEF;
        cmd_buf->nef_size = nef_size;

        _load_nef_command_package cmd_packs[MAX_GROUP_DEVICE];
        pthread_t load_nef_thd[MAX_GROUP_DEVICE];

        cmd_packs[0].ll_device = _devices_grp->ll_device[0];
        cmd_packs[0].cmd_buf = cmd_buf;
        cmd_packs[0].nef_buf = nef_buf;
        cmd_packs[0].timeout = _devices_grp->timeout;

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_load_nef_command_package));
            cmd_packs[i].ll_device = _devices_grp->ll_device[i];
        }

        ret = _spawn_thread_to_load_nef_to_devices(_devices_grp->num_device, cmd_packs, load_nef_thd);
        free(cmd_buf);

        if (ret != KP_SUCCESS)
            return ret;
    } else {
        uint32_t transfer_size = sizeof(kdp2_ipc_cmd_load_model_t) + nef_info.fw_info_size;
        kdp2_ipc_cmd_load_model_t *cmd_buf = (kdp2_ipc_cmd_load_model_t *)malloc(transfer_size);
        if (NULL == cmd_buf)
            return KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;

        cmd_buf->magic_type = KDP2_MAGIC_TYPE_COMMAND;
        cmd_buf->total_size = transfer_size;
        cmd_buf->command_id = KDP2_COMMAND_LOAD_MODEL;
        cmd_buf->model_size = nef_info.all_models_size;
        cmd_buf->fw_info_size = nef_info.fw_info_size;
        memcpy(cmd_buf->fw_info, nef_info.fw_info_addr, nef_info.fw_info_size);

        _load_model_command_package cmd_packs[MAX_GROUP_DEVICE];
        pthread_t load_model_thd[MAX_GROUP_DEVICE];

        cmd_packs[0].ll_device = _devices_grp->ll_device[0];
        cmd_packs[0].cmd_buf = cmd_buf;
        cmd_packs[0].model_buf = nef_info.all_models_addr;
        cmd_packs[0].timeout = _devices_grp->timeout;

        for (int i = 1; i < _devices_grp->num_device; i++)
        {
            memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_load_model_command_package));
            cmd_packs[i].ll_device = _devices_grp->ll_device[i];
        }

        ret = _spawn_thread_to_load_model_to_devices(_devices_grp->num_device, cmd_packs, load_model_thd);
        free(cmd_buf);

        if (ret != KP_SUCCESS)
            return ret;
    }

    if (ret == KP_SUCCESS && model_desc != NULL)
        ret = load_model_info_from_nef(nef_buf, nef_size, _devices_grp->product_id, &metadata, &nef_info, model_desc);

    ret = _kp_allocate_ddr_memory(devices);

    return ret;
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
    if (NULL == buffer)
    {
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

int kp_load_model_from_file(kp_device_group_t devices, const char *file_path, kp_model_nef_descriptor_t *model_desc)
{
    long nef_size;
    char *nef_buf = read_file_to_buffer_auto_malloc(file_path, &nef_size);
    if (!nef_buf)
        return KP_ERROR_FILE_OPEN_FAILED_20;

    int ret = kp_load_model(devices, (void *)nef_buf, (int)nef_size, model_desc);

    free(nef_buf);

    return ret;
}

// There should be only 1 model_desc since all dongles in a device group only run the same model at the same time
// Note that the CRC of all_models.bin in encrypted models based on the same unencrypted model should be the same
int kp_load_encrypted_models(kp_device_group_t devices, void *nef_buf[], int nef_size, int nef_num, kp_model_nef_descriptor_t *model_desc)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kdp2_ipc_cmd_load_model_t *cmd_buf = NULL;

    // At most 20 devices and 20 models are supported
    // Number of encrypted models should be the same as number of devices in the device group (Can change the rule ?)
    if (nef_num <= 0 || nef_num > MAX_GROUP_DEVICE || nef_num != _devices_grp->num_device)
        return KP_ERROR_INVALID_PARAM_12;

    kp_metadata_t metadata[MAX_GROUP_DEVICE];
    kp_nef_info_t nef_info[MAX_GROUP_DEVICE];
    kp_model_nef_descriptor_t temp_model_desc[MAX_GROUP_DEVICE];

    int ret = check_fw_is_loaded(devices);

    if (KP_SUCCESS != ret)
        return ret;

    ret = reboot_if_model_is_loaded(devices);

    if (KP_SUCCESS != ret)
        return ret;

    for (int i = 0; i < nef_num; i++)
    {
        ret = load_model_info_from_nef(nef_buf[i], nef_size, _devices_grp->product_id, &metadata[i], &nef_info[i], &temp_model_desc[i]);
        if (ret != KP_SUCCESS) {
            goto FUNC_OUT;
        }

        // Check if all encrypted nef files refer to the same all_models.bin
        if (i > 0 && temp_model_desc[i - 1].crc != temp_model_desc[i].crc) {
            ret = KP_ERROR_INVALID_MODEL_21;
            goto FUNC_OUT;
        }
    }

    uint32_t transfer_size = sizeof(kdp2_ipc_cmd_load_model_t) + nef_info[0].fw_info_size;
    cmd_buf = (kdp2_ipc_cmd_load_model_t *)malloc(transfer_size);
    if (NULL == cmd_buf) {
        ret = KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
        goto FUNC_OUT;
    }

    cmd_buf->magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf->total_size = transfer_size;
    cmd_buf->command_id = KDP2_COMMAND_LOAD_MODEL;
    cmd_buf->model_size = nef_info[0].all_models_size;
    cmd_buf->fw_info_size = nef_info[0].fw_info_size;
    memcpy(cmd_buf->fw_info, nef_info[0].fw_info_addr, nef_info[0].fw_info_size);

    _load_model_command_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t load_model_thd[MAX_GROUP_DEVICE];

    // Find the target device of the encrypted model through KN number
    // FIXME: search could be faster
    int match_count = 0;
    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        for (int j = 0; j < nef_num; j++)
        {
            if ((_devices_grp->ll_device[i]->dev_descp.kn_number == metadata[j].kn_num) &&
                (_devices_grp->ll_device[i] != cmd_packs[i].ll_device))
            {
                cmd_packs[i].ll_device = _devices_grp->ll_device[i];
                cmd_packs[i].cmd_buf = cmd_buf;
                cmd_packs[i].model_buf = nef_info[j].all_models_addr;
                cmd_packs[i].timeout = _devices_grp->timeout;
                match_count++;
            }
        }
    }

    // Check if each device in device group have one encrypted all_models.bin
    if (match_count != _devices_grp->num_device) {
        ret = KP_ERROR_INVALID_MODEL_21;
        goto FUNC_OUT;
    }

    _spawn_thread_to_load_model_to_devices(_devices_grp->num_device, cmd_packs, load_model_thd);

    if (ret == KP_SUCCESS) {
        ret = load_model_info_from_nef(nef_buf[0], nef_size, _devices_grp->product_id, &metadata[0], &nef_info[0], &_devices_grp->loaded_model_desc);
        if (ret != KP_SUCCESS) {
            goto FUNC_OUT;
        }

        if (model_desc != NULL) {
            ret = load_model_info_from_nef(nef_buf[0], nef_size, _devices_grp->product_id, &metadata[0], &nef_info[0], model_desc);
            if (ret != KP_SUCCESS) {
                goto FUNC_OUT;
            }
        }
    }

    ret = _kp_allocate_ddr_memory(devices);

FUNC_OUT:
    if (NULL != cmd_buf) {
        free(cmd_buf);
    }

    for (int i = 0; i < nef_num; i++)
    {
        if (KP_SUCCESS != kp_release_model_nef_descriptor(&temp_model_desc[i])) {
            dbg_print("[%s] release temp model descriptor failed on NEF order %d, error %d\n", __FUNCTION__, i, ret);
            break;
        }
    }

    return ret;
}

int kp_load_encrypted_models_from_file(kp_device_group_t devices, char *file_path[], int nef_num, kp_model_nef_descriptor_t *model_desc)
{
    void *nef_buf[MAX_GROUP_DEVICE];

    long nef_size_prev = 0;
    long nef_size = 0;

    for (int i = 0; i < nef_num; i++)
    {
        nef_buf[i] = (void *)read_file_to_buffer_auto_malloc(file_path[i], &nef_size);
        if (!nef_buf[i])
            return KP_ERROR_FILE_OPEN_FAILED_20;

        // Check if the size of all encrypted nef files are the same
        if (i > 0 && nef_size_prev != nef_size)
            return KP_ERROR_INVALID_PARAM_12;

        nef_size_prev = nef_size;
    }

    int ret = kp_load_encrypted_models(devices, nef_buf, nef_size, nef_num, model_desc);

    for (int i = 0; i < nef_num; i++)
        free(nef_buf[i]);

    return ret;
}

#define KL520_SCPU_START_ADDR (0x10104000) // FIXME, hard code for now

int _load_firmware_to_520(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size, void *ncpu_fw_buf, int ncpu_fw_size)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];

        // remember port_id here
        int port_id = ll_dev->dev_descp.port_id;

        bool do_rest = true;
        bool do_load_scpu_fw = (NULL != scpu_fw_buf);
        bool do_load_ncpu_fw = (NULL != ncpu_fw_buf);
        uint16_t fw_type = (ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK_V2);
        uint16_t fw_type_legacy = (ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK);

        if ((KP_KDP2_FW_LOADER_V2 == fw_type) ||
            (KP_KDP2_FW_LOADER == fw_type_legacy)) // FW Loader Only
        {
            do_rest = false;
        }
        else if ((KP_KDP2_FW_JTAG_TYPE_V2 == fw_type) ||
                 (KP_KDP2_FW_JTAG_TYPE == fw_type_legacy)) // FW JTAG
        {
            do_rest = false;
            do_load_scpu_fw = false;
        }
        else if ((KP_KDP2_FW_FLASH_TYPE_V2 == fw_type) ||
                 (KP_KDP2_FW_FLASH_TYPE == fw_type_legacy)) // FW Flash Boot
        {
            printf("[Notice]: The device with port id: %d is in flash boot mode ... upload firmware from file is skipped\n", port_id);
            continue;
        }
        else if ((KP_KDP2_FW_USB_TYPE_V2 == fw_type) ||
                 (KP_KDP2_FW_USB_TYPE == fw_type_legacy)) // FW Usb Boot
        {
            // do defaults
        }
        else
        {
            // firmware is not correct
            return KP_ERROR_INVALID_FIRMWARE_24;
        }

        // reset stage
        if (do_rest)
        {
            // reset/reboot the device
            kp_usb_control_t kctrl = {KDP2_CONTROL_REBOOT, 0, 0};
            int ret = kp_usb_control(ll_dev, &kctrl, timeout);
            if ((ret != KP_USB_USB_PIPE) &&
                (ret != KP_USB_USB_IO) &&
                (ret != KP_USB_USB_NO_DEVICE) &&
                (ret != KP_USB_USB_NOT_FOUND) &&
                (ret != KP_SUCCESS)) {// FIXME: tricky way
                                      // Note that device return KP_SUCCESS in some Windows device and KP_USB_USB_NO_DEVICE in Ubuntu
                dbg_print("[%s] Send reboot command to usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
                return KP_ERROR_RESET_FAILED_25;
            }

            kp_usb_disconnect_device(ll_dev);

            usleep(USB_DISCONNECT_WAIT_DELAY_US);

            // re-connect device, polling
            int port_ids[1] = {port_id};
            kp_usb_device_t *devs[1];

            ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
            if(ret != KP_USB_RET_OK)
                return KP_ERROR_DEVICE_NOT_EXIST_10;

            // update back
            ll_dev = devs[0];
            _devices_grp->ll_device[i] = ll_dev;
        }

        // SCPU firmware
        if (do_load_scpu_fw)
        {
            kdp2_ipc_cmd_upload_firmware_t cmd_buf;

            cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
            cmd_buf.total_size = sizeof(cmd_buf) + scpu_fw_size;
            cmd_buf.command_id = KDP2_COMMAND_LOAD_FIRMWARE;
            cmd_buf.fw_type = FW_TYPE_SCPU;
            cmd_buf.fw_start = KL520_SCPU_START_ADDR; // FIXME
            cmd_buf.fw_size = scpu_fw_size;

            int ret = 0;
            int re_connect_device_times = 0;

            while (true) {
                ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, sizeof(cmd_buf), _devices_grp->timeout);

                if (ret < 0) {
                    if (3 > re_connect_device_times) {
                        kp_usb_disconnect_device(ll_dev);
                        usleep(USB_DISCONNECT_WAIT_DELAY_US * (re_connect_device_times + 1));

                        // re-connect device, polling
                        int port_ids[1] = {port_id};
                        kp_usb_device_t *devs[1];

                        ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
                        if(ret != KP_USB_RET_OK)
                            return KP_ERROR_DEVICE_NOT_EXIST_10;

                        // update back
                        ll_dev = devs[0];
                        _devices_grp->ll_device[i] = ll_dev;
                        re_connect_device_times++;
                    } else {
                        printf("Line %d, usb failed code = %d\n", __LINE__, ret);
                        return ret;
                    }
                } else {
                    break;
                }
            }

            ret = kp_usb_write_data(ll_dev, scpu_fw_buf, scpu_fw_size, _devices_grp->timeout);
            if (ret < 0)
            {
                printf("Line %d, usb failed code = %d\n", __LINE__, ret);
                return ret;
            }

            kp_usb_disconnect_device(ll_dev);

            usleep(USB_DISCONNECT_WAIT_DELAY_US);

            // here USB is disconnected, re-connect it
            // re-connect device, polling
            int port_ids[1];
            port_ids[0] = port_id;
            kp_usb_device_t *devs[1];

            ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
            if(ret != KP_USB_RET_OK)
                return KP_ERROR_DEVICE_NOT_EXIST_10;

            // update back
            ll_dev = devs[0];
            _devices_grp->ll_device[i] = ll_dev;
        }

        // FIXME !! for buffer allocation
        {
            kp_usb_control_t kctrl;
            kctrl.command = KDP2_CONTROL_FIFOQ_RESET;
            kctrl.arg1 = 0;
            kctrl.arg2 = 0;

            int ret = 0;
            int re_connect_device_times = 0;

            while (true) {
                ret = kp_usb_control(ll_dev, &kctrl, _devices_grp->timeout);

                if (ret < 0) {
                    if (3 > re_connect_device_times) {
                        kp_usb_disconnect_device(ll_dev);
                        usleep(USB_DISCONNECT_WAIT_DELAY_US * (re_connect_device_times + 1));

                        // re-connect device, polling
                        int port_ids[1] = {port_id};
                        kp_usb_device_t *devs[1];

                        ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
                        if (ret != KP_USB_RET_OK)
                            return KP_ERROR_DEVICE_NOT_EXIST_10;

                        // update back
                        ll_dev = devs[0];
                        _devices_grp->ll_device[i] = ll_dev;
                        re_connect_device_times++;
                    } else {
                        printf("reset fifoq error\n");
                        return ret;
                    }
                } else {
                    break;
                }
            }

            usleep(50 * 1000);
        }

        // NCPU firmware
        if (do_load_ncpu_fw)
        {
            kdp2_ipc_cmd_upload_firmware_t cmd_buf;

            cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
            cmd_buf.total_size = sizeof(cmd_buf) + ncpu_fw_size;
            cmd_buf.command_id = KDP2_COMMAND_LOAD_FIRMWARE;
            cmd_buf.fw_type = FW_TYPE_NCPU;
            cmd_buf.fw_size = ncpu_fw_size;

            int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, sizeof(cmd_buf), _devices_grp->timeout);
            if (ret < 0)
            {
                printf("Line %d, usb failed code = %d\n", __LINE__, ret);
                return ret;
            }

            ret = kp_usb_write_data(ll_dev, ncpu_fw_buf, ncpu_fw_size, _devices_grp->timeout);
            if (ret < 0)
            {
                printf("Line %d, usb failed code = %d\n", __LINE__, ret);
                printf("do_rest %d do_load_scpu_fw %d do_load_ncpu_fw %d\n", do_rest, do_load_scpu_fw, do_load_ncpu_fw);

                return ret;
            }

            usleep(10 * 1000);
        }
    }

    return KP_SUCCESS;
}

int _load_firmware_to_630(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];

        // remember port_id here
        int port_id = ll_dev->dev_descp.port_id;

        bool do_rest = true;
        bool do_load_scpu_fw = (NULL != scpu_fw_buf);
        uint16_t fw_type = (ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK_V2);

        if ((KP_KDP2_FW_LOADER_V2 == fw_type)) // FW Loader Only
        {
            do_rest = false;
            // do defaults
        }
        else if ((KP_KDP2_FW_FLASH_TYPE_V2 == fw_type)) // FW Flash Boot
        {
            printf("[Notice]: The device with port id: %d is in flash boot mode ... upload firmware from file is skipped\n", port_id);
            continue;
        }
        else if ((KP_KDP2_FW_USB_TYPE_V2 == fw_type)) // FW Usb Boot
        {
            // do defaults
        }
        else
        {
            // firmware is not correct
            return KP_ERROR_INVALID_FIRMWARE_24;
        }

        // reset stage
        if (do_rest)
        {
            // reset/reboot the device
            kp_usb_control_t kctrl = {KDP2_CONTROL_REBOOT, 0, 0};
            int ret = kp_usb_control(ll_dev, &kctrl, timeout);
            if ((ret != KP_USB_USB_PIPE) &&
                (ret != KP_USB_USB_IO) &&
                (ret != KP_USB_USB_NO_DEVICE) &&
                (ret != KP_USB_USB_NOT_FOUND) &&
                (ret != KP_SUCCESS)) {// FIXME: tricky way
                                      // Note that device return KP_SUCCESS in some Windows device and KP_USB_USB_NO_DEVICE in Ubuntu
                dbg_print("[%s] Send reboot command to usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
                return KP_ERROR_RESET_FAILED_25;
            }

            kp_usb_disconnect_device(ll_dev);

            usleep(USB_DISCONNECT_WAIT_DELAY_US);

            // re-connect device, polling
            int port_ids[1] = {port_id};
            kp_usb_device_t *devs[1];

            ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
            if(ret != KP_USB_RET_OK)
                return KP_ERROR_DEVICE_NOT_EXIST_10;

            // update back
            ll_dev = devs[0];
            _devices_grp->ll_device[i] = ll_dev;
        }

        // SCPU firmware
        if (do_load_scpu_fw)
        {
            kdp2_ipc_cmd_upload_firmware_t cmd_buf;

            cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
            cmd_buf.total_size = sizeof(cmd_buf);
            cmd_buf.command_id = KDP2_COMMAND_LOAD_FIRMWARE;
            cmd_buf.fw_type = FW_TYPE_SCPU;
            cmd_buf.fw_start = 0;
            cmd_buf.fw_size = scpu_fw_size;

            int ret = 0;
            int re_connect_device_times = 0;

            while (true) {
                ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, sizeof(cmd_buf), _devices_grp->timeout);

                if (ret < 0) {
                    if (3 > re_connect_device_times) {
                        kp_usb_disconnect_device(ll_dev);
                        usleep(USB_DISCONNECT_WAIT_DELAY_US * (re_connect_device_times + 1));

                        // re-connect device, polling
                        int port_ids[1] = {port_id};
                        kp_usb_device_t *devs[1];

                        ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
                        if(ret != KP_USB_RET_OK)
                            return KP_ERROR_DEVICE_NOT_EXIST_10;

                        // update back
                        ll_dev = devs[0];
                        _devices_grp->ll_device[i] = ll_dev;
                        re_connect_device_times++;
                    } else {
                        printf("Line %d, usb failed code = %d\n", __LINE__, ret);
                        return ret;
                    }
                } else {
                    break;
                }
            }

            ret = kp_usb_write_data(ll_dev, scpu_fw_buf, scpu_fw_size, _devices_grp->timeout);
            if (ret < 0)
            {
                printf("Line %d, usb failed code = %d\n", __LINE__, ret);
                return ret;
            }

            kp_usb_disconnect_device(ll_dev);

            usleep(USB_DISCONNECT_WAIT_DELAY_US);

            // here USB is disconnected, re-connect it
            // re-connect device, polling
            int port_ids[1];
            port_ids[0] = port_id;
            kp_usb_device_t *devs[1];

            ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
            if(ret != KP_USB_RET_OK)
                return KP_ERROR_DEVICE_NOT_EXIST_10;

            // update back
            ll_dev = devs[0];
            _devices_grp->ll_device[i] = ll_dev;
        }

        // FIXME !! for buffer allocation
        {
            kp_usb_control_t kctrl;
            kctrl.command = KDP2_CONTROL_FIFOQ_RESET;
            kctrl.arg1 = 0;
            kctrl.arg2 = 0;

            int ret = 0;
            int re_connect_device_times = 0;

            while (true) {
                ret = kp_usb_control(ll_dev, &kctrl, _devices_grp->timeout);

                if (ret < 0) {
                    if (3 > re_connect_device_times) {
                        kp_usb_disconnect_device(ll_dev);
                        usleep(USB_DISCONNECT_WAIT_DELAY_US * (re_connect_device_times + 1));

                        // re-connect device, polling
                        int port_ids[1] = {port_id};
                        kp_usb_device_t *devs[1];

                        ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
                        if (ret != KP_USB_RET_OK)
                            return KP_ERROR_DEVICE_NOT_EXIST_10;

                        // update back
                        ll_dev = devs[0];
                        _devices_grp->ll_device[i] = ll_dev;
                        re_connect_device_times++;
                    } else {
                        printf("reset fifoq error\n");
                        return ret;
                    }
                } else {
                    break;
                }
            }

            usleep(50 * 1000);
        }
    }

    return KP_SUCCESS;
}

//////////////////////// below 720 DFU/DFW stuff ///////////////////////
// Note: below code is original from host_lib 'kl720_usb_dfw' with some modifications
// FIXME: better efficiency

#define CRC16_CONSTANT 0x8005
static uint16_t gen_crc16(uint8_t *data, uint16_t size)
{
    uint16_t out = 0;
    int bits_read = 0, bit_flag, i;

    /* Sanity check: */
    if (data == NULL)
        return 0;

    while (size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if (bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if (bit_flag)
            out ^= CRC16_CONSTANT;
    }

    // push out the last 16 bits
    for (i = 0; i < 16; ++i)
    {
        bit_flag = out >> 15;
        out <<= 1;
        if (bit_flag)
            out ^= CRC16_CONSTANT;
    }

    // reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>= 1, j <<= 1)
    {
        if (i & out)
            crc |= j;
    }

    return crc;
}

typedef struct
{
    uint16_t header;
    uint16_t crc16;
    uint32_t cmd;
    uint32_t addr;
    uint32_t len;
} MsgHdr_t;

typedef struct
{
    uint32_t error;
    uint32_t bytes;
} RspPram_t;

#define MSG_DATA_BUF_MAX 0xF0000
#define MSG_HDR_SIZE 16 // includes both MsgHdr and CmdPram addr & len
#define MSG_HDR_CMD 0xA583
#define CMD_MEM_WRITE 2
#define ENP_BULK_CMD_OUT 0x01
#define ENP_BULK_CMD_IN 0x82

typedef struct
{
    uint32_t result;     // 0 = pass, non-0 failed
    uint32_t output[20]; // free to use, case by case with different cmd item
} test_response_t;

static int _720_send_data_to_usb_minion(kp_usb_device_t *usb_dev, uintptr_t p_buf, long file_size, uint32_t mem_addr, int timeout)
{
    MsgHdr_t msghdr;
    unsigned char fw_buf[MSG_DATA_BUF_MAX + MSG_HDR_SIZE];
    int dfw_count = file_size / MSG_DATA_BUF_MAX;
    uint32_t len = file_size < MSG_DATA_BUF_MAX ? file_size : MSG_DATA_BUF_MAX;

    int ret;

    //printf("******  unit length %d repeat count %d  ******\n",len, dfw_count);
    if (((file_size % MSG_DATA_BUF_MAX) != 0) || (file_size == 0))
        dfw_count++;

    for (int repeat = 0; repeat < dfw_count; repeat++)
    {

        memcpy((void *)&fw_buf[MSG_HDR_SIZE], (void *)(p_buf + (repeat * MSG_DATA_BUF_MAX)), len);
        msghdr.header = MSG_HDR_CMD;
        msghdr.crc16 = 0;
        msghdr.cmd = CMD_MEM_WRITE;
        msghdr.addr = mem_addr + (repeat * MSG_DATA_BUF_MAX);
        msghdr.len = len;
        memcpy(&fw_buf[0], &msghdr, MSG_HDR_SIZE);
        msghdr.crc16 = gen_crc16(&fw_buf[4], (len + MSG_HDR_SIZE - 4));
        memcpy(&fw_buf[2], &msghdr.crc16, 2);

        unsigned char *fw_buf_addr = &fw_buf[0];
        ret = kp_usb_endpoint_write_data(usb_dev, ENP_BULK_CMD_OUT, (void *)fw_buf_addr, (len + MSG_HDR_SIZE), timeout);
        if (ret < 0)
            return ret;

        test_response_t gResponse;
        ret = kp_usb_endpoint_read_data(usb_dev, ENP_BULK_CMD_IN, (void *)&gResponse, sizeof(gResponse), timeout);
        if (ret < 0)
            return ret;

        if (gResponse.result != 0)
            return KP_ERROR_FW_LOAD_FAILED_34;
    }

    return KP_SUCCESS;
}

#define KL720_SCPU_START_ADDR 0x1FFC0000 // SiRAM_MEM_BASE
#define KL720_NCPU_START_ADDR 0x6F000000 // NiRAM_MEM_BASE
#define KL720_NiRAM_MEM_SIZE 0x20000     // 128KB
#define KL720_NCPU_FW_SIZE 0x200000      // 2MB
#define KL720_NCPU_FW_IRAM_SIZE KL720_NiRAM_MEM_SIZE
#define KL720_NCPU_FW_DDR_BASE 0x80020000
#define KL720_NCPU_FW_DDR_SIZE (KL720_NCPU_FW_SIZE - KL720_NCPU_FW_IRAM_SIZE) // 2MB - 128KB
#define CMD_SCPU_RUN 0x1005

int _load_firmware_to_720(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size, void *ncpu_fw_buf, int ncpu_fw_size)
{
    uintptr_t s_fw = (uintptr_t)scpu_fw_buf;
    uintptr_t n_fw = (uintptr_t)ncpu_fw_buf;

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;

    int ret;

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *usb_dev = _devices_grp->ll_device[i];

        // remember port_id here
        int port_id = usb_dev->dev_descp.port_id;

        // firware is already loaded, skip it
        if (KP_KDP2_FW_KL720_LOADER != usb_dev->fw_serial)
        {
            printf("[Notice]: A firmware is running on device with port id: %d ... upload firmware from file is skipped\n", port_id);
            continue;
        }

        ret = _720_send_data_to_usb_minion(usb_dev, s_fw, scpu_fw_size, KL720_SCPU_START_ADDR, timeout);
        if (ret < 0)
            return ret;

        ret = _720_send_data_to_usb_minion(usb_dev, n_fw, KL720_NiRAM_MEM_SIZE, KL720_NCPU_START_ADDR, timeout);
        if (ret < 0)
            return ret;

        ret = _720_send_data_to_usb_minion(usb_dev, n_fw + KL720_NiRAM_MEM_SIZE, KL720_NCPU_FW_DDR_SIZE, KL720_NCPU_FW_DDR_BASE, timeout);
        if (ret < 0)
            return ret;

        //==========================================================================================
        /* BOOT UP */
        uint8_t msg_tbuf[MSG_DATA_BUF_MAX + sizeof(MsgHdr_t) + sizeof(RspPram_t) + 4];
        MsgHdr_t *msghdr;
        msghdr = (MsgHdr_t *)msg_tbuf; //-V::1032
        msghdr->header = MSG_HDR_CMD;
        msghdr->crc16 = 0;
        msghdr->cmd = CMD_SCPU_RUN;
        msghdr->len = 0;
        msghdr->addr = KL720_SCPU_START_ADDR;
        msghdr->crc16 = gen_crc16((uint8_t *)(msg_tbuf + 4), MSG_HDR_SIZE - 4);

        // printf("send scpu_run command to boot from 0x%x\n", msghdr->addr);

        ret = kp_usb_endpoint_write_data(usb_dev, ENP_BULK_CMD_OUT, (void *)&msg_tbuf, MSG_HDR_SIZE, timeout);
        if (ret < 0)
            return ret;
        //==========================================================================================

        kp_usb_disconnect_device(usb_dev);

        usleep(USB_DISCONNECT_WAIT_DELAY_US);

        // here USB is disconnected, re-connect it
        // re-connect device, polling
        int port_ids[1];
        port_ids[0] = port_id;
        kp_usb_device_t *devs[1];

        ret = kp_usb_connect_multiple_devices_v2(1, port_ids, devs, 100);
        if(ret != KP_USB_RET_OK)
            return KP_ERROR_DEVICE_NOT_EXIST_10;

        // FIXME !! for buffer allocation
        {
            kp_usb_control_t kctrl;
            kctrl.command = KDP2_CONTROL_FIFOQ_RESET;
            kctrl.arg1 = 0;
            kctrl.arg2 = 0;
            int ret = kp_usb_control(devs[0], &kctrl, _devices_grp->timeout);
            if (ret != KP_USB_RET_OK) {
                printf("reset fifoq error\n");
                return ret;
            }

            usleep(50 * 1000);
        }

        // update back
        _devices_grp->ll_device[i] = devs[0];
    }

    return KP_SUCCESS;
}

//////////////////////// above 720 DFU/DFW stuff ///////////////////////

int kp_load_firmware(kp_device_group_t devices, void *scpu_fw_buf, int scpu_fw_size, void *ncpu_fw_buf, int ncpu_fw_size)
{
    int status = KP_SUCCESS;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    if (_devices_grp->product_id == KP_DEVICE_KL520)
        status = _load_firmware_to_520(devices, scpu_fw_buf, scpu_fw_size, ncpu_fw_buf, ncpu_fw_size);
    else if (_devices_grp->product_id == KP_DEVICE_KL720)
        status = _load_firmware_to_720(devices, scpu_fw_buf, scpu_fw_size, ncpu_fw_buf, ncpu_fw_size);
    else if (_devices_grp->product_id == KP_DEVICE_KL630)
        status = _load_firmware_to_630(devices, scpu_fw_buf, scpu_fw_size);
    else
    {
        printf("error ! %s() does not implement PID = 0x%x\n", __FUNCTION__, _devices_grp->product_id);
        status = KP_ERROR_OTHER_99;
    }

    if (KP_SUCCESS != status)
        goto FUNC_OUT;

    /* check kneron plus & firmware version is compatible for usb-boot */
    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
        uint16_t fw_type = ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK_V2;
        uint16_t fw_type_legacy = (ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK);

        if ((KP_KDP2_FW_USB_TYPE_V2 == fw_type) ||
            (KP_KDP2_FW_USB_TYPE == fw_type_legacy)) {
            kp_system_info_t system_info = {0};
            int ret = get_system_info(ll_dev, &system_info, _devices_grp->timeout);
            const int *fw_version = NULL;

            if (KP_USB_RET_OK == ret) {
                if (KP_DEVICE_KL520 == ll_dev->dev_descp.product_id) {
                    fw_version = kl520_fw_version;
                } else if ((KP_DEVICE_KL720 == ll_dev->dev_descp.product_id) ||
                           (KP_DEVICE_KL720_LEGACY == ll_dev->dev_descp.product_id)) {
                    fw_version = kl720_fw_version;
                } else if (KP_DEVICE_KL630 == ll_dev->dev_descp.product_id) {
                    fw_version = kl630_fw_version;
                } else {
                    printf("invalid device product ID ... %d\n", ll_dev->dev_descp.product_id);
                    continue;
                }

                if ((fw_version[VERSION_INDEX_MAJOR] == 0) &&
                    (fw_version[VERSION_INDEX_MINOR] == 0) &&
                    (fw_version[VERSION_INDEX_REVISION] == 0) &&
                    (fw_version[VERSION_INDEX_BUILD] == 0)) {
                    /* no firmware version check when kp_version.h is default setting */
                    continue;
                }

                if ((fw_version[VERSION_INDEX_MAJOR] != system_info.firmware_version.major) ||
                    (fw_version[VERSION_INDEX_MINOR] != system_info.firmware_version.minor) ||
                    (fw_version[VERSION_INDEX_REVISION] != system_info.firmware_version.update) ||
                    (fw_version[VERSION_INDEX_BUILD] != system_info.firmware_version.build)) {
                    printf("\033[0;33m[warnning] The version of firmware (%d.%d.%d.%d) on the port ID %u is not the corresponding version for this Kneron PLUS (%d.%d.%d.%d).\n\033[0m",
                           system_info.firmware_version.major, system_info.firmware_version.minor, system_info.firmware_version.update, system_info.firmware_version.build,
                           ll_dev->dev_descp.port_id,
                           fw_version[VERSION_INDEX_MAJOR], fw_version[VERSION_INDEX_MINOR], fw_version[VERSION_INDEX_REVISION], fw_version[VERSION_INDEX_BUILD]);
                    fflush(stdout);
                }
            } else {
                status = KP_ERROR_CHECK_FW_VERSION_FAILED_41;
                goto FUNC_OUT;
            }
        }
    }

FUNC_OUT:

    return status;
}

int kp_load_firmware_from_file(kp_device_group_t devices, const char *scpu_fw_path, const char *ncpu_fw_path)
{
    int ret = KP_ERROR_FILE_OPEN_FAILED_20;
    long scpu_fw_size = 0;
    long ncpu_fw_size = 0;
    char *scpu_fw_buf = NULL;
    char *ncpu_fw_buf = NULL;

    if ((NULL != scpu_fw_path) && ('\0' != scpu_fw_path[0]))
    {
        scpu_fw_buf = read_file_to_buffer_auto_malloc(scpu_fw_path, &scpu_fw_size);
    }

    if ((NULL != ncpu_fw_path) && ('\0' != ncpu_fw_path[0]))
    {
        ncpu_fw_buf = read_file_to_buffer_auto_malloc(ncpu_fw_path, &ncpu_fw_size);
    }

    if ((NULL != scpu_fw_buf) || (NULL != ncpu_fw_buf))
    {
        ret = kp_load_firmware(devices, (void *)scpu_fw_buf, (int)scpu_fw_size, (void *)ncpu_fw_buf, (int)ncpu_fw_size);
    }

    if (NULL != scpu_fw_buf)
    {
        free(scpu_fw_buf);
    }

    if (NULL != ncpu_fw_buf)
    {
        free(ncpu_fw_buf);
    }

    return ret;
}

// image_count : number of image buffers, value 1~8
// image_size : image buffer size in 10-KB, value 1~8192 (10KB~80MB)
// result_count : number of result buffers, value 1~8
// result_size : result buffer size in 10-KB, value 1~8192 (10KB~80MB)
static int _kp_set_up_inference_queues(kp_device_group_t devices, uint32_t image_count, uint32_t image_size, uint32_t result_count, uint32_t result_size)
{
#define MAX_BUF_COUNT 8
#define MAX_BUF_SIZE 8192 // 10-KB unit

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    int timeout = _devices_grp->timeout;

    if (image_count > MAX_BUF_COUNT)
        image_count = MAX_BUF_COUNT;
    if (image_size > MAX_BUF_SIZE)
        image_count = MAX_BUF_SIZE;

    if (result_count > MAX_BUF_COUNT)
        result_count = MAX_BUF_COUNT;
    if (result_size > MAX_BUF_SIZE)
        result_size = MAX_BUF_SIZE;

    kp_usb_control_t kctrl;
    int ret = KP_SUCCESS;

    kctrl.command = KDP2_CONTROL_FIFOQ_CONFIGURE;
    kctrl.arg1 = ((image_size - 1) << 3) | (image_count - 1);
    kctrl.arg2 = ((result_size - 1) << 3) | (result_count - 1);

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];

        ret = kp_usb_control(ll_dev, &kctrl, timeout);
        if (ret != KP_USB_RET_OK)
        {
            // this could happen if FW has already complete the set-up
            dbg_print("[%s] set up inference queue failed return code = [%d]\n", __func__, ret);
        }
    }

    return ret;
}

int kp_reset_device(kp_device_group_t devices, kp_reset_mode_t reset_mode)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    int timeout = _devices_grp->timeout;

    int ret = 0;
    kp_usb_control_t kctrl;
    memset(&kctrl, 0, sizeof(kctrl));

    if (reset_mode == KP_RESET_REBOOT)
    {
        kctrl.command = KDP2_CONTROL_REBOOT;

        for (int i = 0; i < _devices_grp->num_device; i++)
        {
            kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
            ret = kp_usb_control(ll_dev, &kctrl, timeout);
            if ((ret != KP_USB_USB_PIPE) &&
                (ret != KP_USB_USB_IO) &&
                (ret != KP_USB_USB_NO_DEVICE) &&
                (ret != KP_USB_USB_NOT_FOUND) &&
                (ret != KP_SUCCESS)) {// FIXME: tricky way
                                      // Note that device return KP_SUCCESS in some Windows device and KP_USB_USB_NO_DEVICE in Ubuntu
                dbg_print("[%s] Send reboot command to usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
                return KP_ERROR_RESET_FAILED_25;
            }
        }
    }
    else if (reset_mode == KP_RESET_INFERENCE)
    {
        kctrl.command = KDP2_CONTROL_FIFOQ_RESET;
        kctrl.arg1 = 0;
        kctrl.arg2 = 0;

        for (int i = 0; i < _devices_grp->num_device; i++)
        {
            kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
            uint16_t fw_type = ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK_V2;
            uint16_t fw_type_legacy = ll_dev->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK;

            // Usb Boot or Flash Boot Fw
            if ((KP_KDP2_FW_USB_TYPE_V2 == fw_type) ||
                (KP_KDP2_FW_FLASH_TYPE_V2 == fw_type) ||
                (KP_KDP2_FW_USB_TYPE == fw_type_legacy) ||
                (KP_KDP2_FW_FLASH_TYPE == fw_type_legacy))
            {
                kp_usb_flush_out_buffers(ll_dev);

                int ret = kp_usb_control(ll_dev, &kctrl, timeout);
                if (ret != KP_USB_RET_OK)
                {
                    dbg_print("[%s] Send usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
                    return ret;
                }
            }
        }
    }
    else if (reset_mode == KP_RESET_SHUTDOWN)
    {
        // FIXME: this could be better if the error code is returned from KL720 firmware; however,
        // control transfer response from device has not been implemented yet
        if (_devices_grp->product_id == 0x720)
        {
            // shutdown command is not supported by KL720
            return KP_ERROR_INVALID_PARAM_12;
        }

        kctrl.command = KDP2_CONTROL_SHUTDOWN;

        for (int i = 0; i < _devices_grp->num_device; i++)
        {
            kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
            ret = kp_usb_control(ll_dev, &kctrl, timeout);
            if ((ret != KP_USB_USB_PIPE) &&
                (ret != KP_USB_USB_IO) &&
                (ret != KP_USB_USB_NO_DEVICE) &&
                (ret != KP_USB_USB_NOT_FOUND) &&
                (ret != KP_SUCCESS)) {// FIXME: tricky way
                                      // Note that device return KP_SUCCESS in some Windows device and KP_USB_USB_NO_DEVICE in Ubuntu
                dbg_print("[%s] Send shutdown command to usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
                return KP_ERROR_RESET_FAILED_25;
            }
        }
    }
    else if (reset_mode == KP_RESET_REBOOT_SYSTEM)
    {
        kctrl.command = KDP2_CONTROL_REBOOT_SYSTEM;

        for (int i = 0; i < _devices_grp->num_device; i++)
        {
            kp_usb_device_t *ll_dev = _devices_grp->ll_device[i];
            ret = kp_usb_control(ll_dev, &kctrl, timeout);
            if ((ret != KP_USB_USB_PIPE) &&
                (ret != KP_USB_USB_IO) &&
                (ret != KP_USB_USB_NO_DEVICE) &&
                (ret != KP_USB_USB_NOT_FOUND) &&
                (ret != KP_SUCCESS)) {// FIXME: tricky way
                                      // Note that device return KP_SUCCESS in some Windows device and KP_USB_USB_NO_DEVICE in Ubuntu
                dbg_print("[%s] Send reboot command to usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
                return KP_ERROR_RESET_FAILED_25;
            }
        }
    }
    else
    {
        dbg_print("[%s] Not support this reset mode [%d]\n", __func__, reset_mode);
        return KP_ERROR_INVALID_PARAM_12;
    }

    return KP_SUCCESS;
}

static pthread_t print_log_thd[MAX_GROUP_DEVICE] = {0};
static bool stop_print_log = false;

typedef struct
{
    kp_usb_device_t *ll_dev;
    FILE *file;
} log_context_t;

static void *_print_log_function_per_dev(void *data)
{
#define MAX_LOG_LEN 1000 // due to interrupt max packet size <= 1024

    log_context_t *log_context = (log_context_t *)data;

    char log[MAX_LOG_LEN];
    int ret;

    while (1)
    {
        ret = kp_usb_read_firmware_log(log_context->ll_dev, (void *)log, MAX_LOG_LEN, 100);
        if (ret == KP_ERROR_USB_TIMEOUT_N7)
        {
            if (stop_print_log)
                break;
            else
                continue;
        }
        else if (ret < 0)
        {
            printf("%s() kp_usb_read_firmware_log() return %d\n", __FUNCTION__, ret);
            break;
        }

        if (log_context->file)
            fprintf(log_context->file, "%s", log);
        else
            printf("%s", log);
    }

    if (log_context->file)
        fclose(log_context->file);

    free(log_context);

    return NULL;
}

int kp_enable_firmware_log(kp_device_group_t devices, int dev_port_id, char *log_file_path)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    FILE *file = NULL;
    if (log_file_path)
    {
        file = fopen(log_file_path, "w");
        if (!file)
        {
            printf("%s() fopen failed\n", __FUNCTION__);
            return KP_ERROR_FILE_OPEN_FAILED_20;
        }
    }

    // Search for device with matched port id and corresponding scan index
    int scan_index;
    for (scan_index = 0; scan_index < _devices_grp->num_device; scan_index++)
    {
        if (dev_port_id == _devices_grp->ll_device[scan_index]->dev_descp.port_id)
            break;
    }

    if (scan_index == _devices_grp->num_device)
    {
        if (file)
            fclose(file);
        return KP_ERROR_DEVICE_NOT_EXIST_10;
    }

    log_context_t *log_context = (log_context_t *)malloc(sizeof(log_context_t));
    if (NULL == log_context)
    {
        if (file)
            fclose(file);
        return KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
    }

    log_context->ll_dev = _devices_grp->ll_device[scan_index];
    log_context->file = file;

    pthread_create(&print_log_thd[scan_index], NULL, _print_log_function_per_dev, log_context);

    return KP_SUCCESS;
}

int kp_disable_firmware_log(kp_device_group_t devices)
{
    stop_print_log = true; // notify log thread to stop

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    for (int i = 0; i < _devices_grp->num_device; i++)
    {
        if (print_log_thd[i])
            pthread_join(print_log_thd[i], NULL);
    }

    return KP_SUCCESS;
}

int kp_get_system_info(kp_device_group_t devices, int dev_port_id, kp_system_info_t *system_info)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    // Search for device with matched port id and corresponding scan index
    int scan_index;
    for (scan_index = 0; scan_index < _devices_grp->num_device; scan_index++)
    {
        if (dev_port_id == _devices_grp->ll_device[scan_index]->dev_descp.port_id)
            break;
    }

    if (scan_index == _devices_grp->num_device)
        return KP_ERROR_DEVICE_NOT_EXIST_10;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[scan_index];

    return get_system_info(ll_dev, system_info, _devices_grp->timeout);
}

int kp_get_model_info(kp_device_group_t devices, int dev_port_id, kp_model_nef_descriptor_t *all_models_desc)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    // Search for device with matched port id and corresponding scan index
    int scan_index;
    for (scan_index = 0; scan_index < _devices_grp->num_device; scan_index++)
    {
        if (dev_port_id == _devices_grp->ll_device[scan_index]->dev_descp.port_id)
            break;
    }

    if (scan_index == _devices_grp->num_device)
        return KP_ERROR_DEVICE_NOT_EXIST_10;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[scan_index];

    kdp2_ipc_cmd_get_model_info_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.command_id = KDP2_COMMAND_GET_MODEL_INFO;
    cmd_buf.from_ddr = 1; // not working when value = 0, fix it if needed

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, sizeof(cmd_buf), _devices_grp->timeout);
    int status = check_usb_write_data_error(ret);
    if (KP_SUCCESS != status)
        return status;

    // get firmware info from device
    kdp2_ipc_response_get_model_info_fw_info_t fw_info_response_buf = {0};
    kp_nef_info_t nef_info = {0};

    ret = kp_usb_read_data(ll_dev, (void *)&fw_info_response_buf, sizeof(kdp2_ipc_response_get_model_info_fw_info_t), _devices_grp->timeout);
    status = check_usb_read_data_error(ret);

    if (KP_SUCCESS != status) {
        ret = status;
        goto FUNC_OUT;
    } else if (KP_SUCCESS != fw_info_response_buf.return_code) {
        ret = fw_info_response_buf.return_code;
        goto FUNC_OUT;
    } else if (sizeof(kdp2_ipc_response_get_model_info_fw_info_t) == ret) {
        nef_info.target = fw_info_response_buf.target_chip;
        nef_info.fw_info_size = fw_info_response_buf.fw_info_size;
        nef_info.fw_info_addr = malloc(nef_info.fw_info_size);

        if (NULL == nef_info.fw_info_addr) {
            ret = KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
            goto FUNC_OUT;
        }

        ret = kp_usb_read_data(ll_dev, (void *)nef_info.fw_info_addr, nef_info.fw_info_size, _devices_grp->timeout);
        status = check_usb_read_data_error(ret);

        if (KP_SUCCESS != status) {
            ret = status;
            goto FUNC_OUT;
        } else if (nef_info.fw_info_size == ret) {
            ret = KP_SUCCESS;
        } else {
            ret = KP_ERROR_OTHER_99;
            goto FUNC_OUT;
        }
    } else {
        ret = KP_ERROR_OTHER_99;
        goto FUNC_OUT;
    }

    // get setup info from device
    kdp2_ipc_response_get_model_info_setup_t all_setup_response_buf = {0};
    kdp2_ipc_response_get_model_info_setup_t single_setup_response_buf = {0};
    uint32_t all_models_offset = 0;

    ret = kp_usb_read_data(ll_dev, (void *)&all_setup_response_buf, sizeof(kdp2_ipc_response_get_model_info_setup_t), _devices_grp->timeout);
    status = check_usb_read_data_error(ret);

    if (KP_SUCCESS != status) {
        ret = status;
        goto FUNC_OUT;
    } else if (KP_SUCCESS != all_setup_response_buf.return_code) {
        ret = all_setup_response_buf.return_code;
        goto FUNC_OUT;
    } else if (sizeof(kdp2_ipc_response_get_model_info_setup_t) == ret) {
        nef_info.all_models_size = all_setup_response_buf.setup_size;
        nef_info.all_models_addr = malloc(nef_info.all_models_size);

        if (NULL == nef_info.all_models_addr) {
            ret = KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
            goto FUNC_OUT;
        }

        while (nef_info.all_models_size > all_models_offset) {
            ret = kp_usb_read_data(ll_dev, (void *)&single_setup_response_buf, sizeof(kdp2_ipc_response_get_model_info_setup_t), _devices_grp->timeout);
            status = check_usb_read_data_error(ret);

            if (KP_SUCCESS != status) {
                ret = status;
                goto FUNC_OUT;
            } else if (sizeof(kdp2_ipc_response_get_model_info_setup_t) == ret) {
                ret = KP_SUCCESS;
            } else {
                ret = KP_ERROR_OTHER_99;
                goto FUNC_OUT;
            }

            ret = kp_usb_read_data(ll_dev, (void *)(nef_info.all_models_addr + all_models_offset), single_setup_response_buf.setup_size, _devices_grp->timeout);
            status = check_usb_read_data_error(ret);

            if (KP_SUCCESS != status) {
                ret = status;
                goto FUNC_OUT;
            } else if (single_setup_response_buf.setup_size == ret) {
                all_models_offset += ret;
                ret = KP_SUCCESS;
            } else {
                ret = KP_ERROR_OTHER_99;
                goto FUNC_OUT;
            }

            if (all_models_offset > nef_info.all_models_size) {
                ret = KP_ERROR_OTHER_99;
                goto FUNC_OUT;
            }
        }
    } else {
        ret = KP_ERROR_OTHER_99;
        goto FUNC_OUT;
    }

    // build model descriptor
    ret = build_model_nef_descriptor_from_device(&nef_info, all_models_desc);

FUNC_OUT:
    if (NULL != nef_info.fw_info_addr)
        free(nef_info.fw_info_addr);

    if (NULL != nef_info.all_models_addr)
        free(nef_info.all_models_addr);

    return ret;
}

int kp_release_model_nef_descriptor(kp_model_nef_descriptor_t *model_desc)
{
    return deconstruct_model_nef_descriptor(model_desc);
}

int kp_load_model_from_flash(kp_device_group_t devices, kp_model_nef_descriptor_t *model_desc)
{
    int ret = check_fw_is_loaded(devices);

    if (KP_SUCCESS != ret)
        return ret;

    ret = reboot_if_model_is_loaded(devices);

    if (KP_SUCCESS != ret)
        return ret;

    ret = KP_ERROR_FILE_OPEN_FAILED_20;

    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;
    kdp2_ipc_cmd_load_model_from_flash_t cmd_buf;
    uint32_t prev_model_crc = 0;
    kp_model_nef_descriptor_t temp_model_desc = {0};

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.command_id = KDP2_COMMAND_LOAD_MODEL_FROM_FLASH;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_load_model_from_flash_t);

    _load_model_from_flash_command_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t update_fw_thd[MAX_GROUP_DEVICE];

    cmd_packs[0].dev_idx = 0;
    cmd_packs[0].ll_device = ll_dev[0];
    cmd_packs[0].cmd_buf = &cmd_buf;
    cmd_packs[0].timeout = devices->timeout;

    for (int i = 1; i < _devices_grp->num_device; i++) {
        dbg_print("[%s] create thread to load model from device %d flash\n", __FUNCTION__, i);
        memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_load_model_from_flash_command_package));
        cmd_packs[i].dev_idx = i;
        cmd_packs[i].ll_device = ll_dev[i];

        int thd_ret = pthread_create(&update_fw_thd[i], NULL, _load_single_device_model_from_flash, (void *)&cmd_packs[i]);

        if (0 != thd_ret) {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return -1;
        }
    }

    // current thread do first device
    _load_single_device_model_from_flash((void *)&cmd_packs[0]);

    for (int i = 1; i < _devices_grp->num_device; i++) {
        pthread_join(update_fw_thd[i], NULL);
    }

    // check all thread update fw status
    for (int i = 0; i < _devices_grp->num_device; i++) {
        ret = cmd_packs[i].sts;

        if (KP_USB_RET_OK != ret) {
            dbg_print("[%s] thread load model from flash failed at device %d, error %d\n", __FUNCTION__, i, ret);
            break;
        }
    }

    if (KP_SUCCESS != ret) {
        goto FUNC_OUT;
    }

    ret = kp_get_model_info(devices, ll_dev[0]->dev_descp.port_id, &temp_model_desc);

    if (KP_USB_RET_OK != ret) {
        dbg_print("[%s] load model descriptor failed on device %d, error %d\n", __FUNCTION__, 0, ret);
        goto FUNC_OUT;
    }

    prev_model_crc = temp_model_desc.crc;

    ret = kp_release_model_nef_descriptor(&temp_model_desc);

    if (KP_SUCCESS != ret) {
        dbg_print("[%s] release temp model descriptor failed on device %d, error %d\n", __FUNCTION__, 0, ret);
        goto FUNC_OUT;
    }

    for (int i = 1; i < _devices_grp->num_device; i++) {
        ret = kp_get_model_info(devices, ll_dev[i]->dev_descp.port_id, &temp_model_desc);

        if (KP_USB_RET_OK != ret) {
            dbg_print("[%s] load model descriptor failed on device %d, error %d\n", __FUNCTION__, i, ret);
            goto FUNC_OUT;
        }

        if (prev_model_crc != temp_model_desc.crc) {
            dbg_print("[%s] model crc on device %d (0x%X) is different from device %d (0x%X)\n",
                      __FUNCTION__, i - 1, prev_model_crc, i, temp_model_desc.crc);

            ret = KP_ERROR_INVALID_MODEL_21;
            goto FUNC_OUT;
        }

        ret = kp_release_model_nef_descriptor(&temp_model_desc);

        if (KP_SUCCESS != ret) {
            dbg_print("[%s] release temp model descriptor failed on device %d, error %d\n", __FUNCTION__, 0, ret);
            goto FUNC_OUT;
        }
    }

    if (KP_SUCCESS == ret) {
        ret = kp_get_model_info(devices, ll_dev[0]->dev_descp.port_id, &_devices_grp->loaded_model_desc);

        if ((NULL != model_desc) && (KP_SUCCESS == ret)) {
            ret = kp_get_model_info(devices, ll_dev[0]->dev_descp.port_id, model_desc);
        }
    }

    ret = _kp_allocate_ddr_memory(devices);

FUNC_OUT:
    if (KP_SUCCESS != kp_release_model_nef_descriptor(&temp_model_desc)) {
        dbg_print("[%s] release temp model descriptor failed\n", __FUNCTION__);
    }

    usleep(USB_DISCONNECT_WAIT_DELAY_US);

    return ret;
}

#define KNERON_PRODUCT_USB_VID                  0x3231
#define KL520_PRODUCT_NAME                      "KL520"
#define KL720_CURRENT_PRODUCT_NAME              "KL720_720"
#define KL720_LAGECY_PRODUCT_NAME               "KL720_200"
#define KL630_PRODUCT_NAME                      "KL630"
#define KL520_FULL_PRODUCT_NAME                 "Kneron KL520"
#define KL720_FULL_PRODUCT_NAME                 "Kneron KL720"
#define KL630_FULL_PRODUCT_NAME                 "Kneron KL630"
#define DRIVER_FILE_PATH                        "win_driver"

int kp_install_driver_for_windows(kp_product_id_t device_pid)
{
    int Ret = KP_ERROR_INVALID_PARAM_12;

#ifdef _WIN32
    if ((KP_DEVICE_KL520 != device_pid) &&
        (KP_DEVICE_KL720 != device_pid) &&
        (KP_DEVICE_KL720_LEGACY != device_pid) &&
        (KP_DEVICE_KL630 != device_pid)) {
        printf("[%s] Device PID 0x%X is not supported.\n", __FUNCTION__, device_pid);
        goto FUNC_OUT;
    }

    wdi_set_log_level(WDI_LOG_LEVEL_NONE);

    uint16_t device_vid = KNERON_PRODUCT_USB_VID;
    char strDeviceInfName[30] = "kneron_device.inf";
    char strDeviceProductName[15] = {0};
    char strDeviceFullProductName[20] = {0};

    switch (device_pid) {
    case KP_DEVICE_KL520:
        strcpy(strDeviceInfName, "kneron_kl520.inf");
        strcpy(strDeviceProductName, KL520_PRODUCT_NAME);
        strcpy(strDeviceFullProductName, KL520_FULL_PRODUCT_NAME);
        break;
    case KP_DEVICE_KL720:
        strcpy(strDeviceInfName, "kneron_kl720.inf");
        strcpy(strDeviceProductName, KL720_CURRENT_PRODUCT_NAME);
        strcpy(strDeviceFullProductName, KL720_FULL_PRODUCT_NAME);
        break;
    case KP_DEVICE_KL720_LEGACY:
        strcpy(strDeviceInfName, "kneron_kl720_legacy.inf");
        strcpy(strDeviceProductName, KL720_LAGECY_PRODUCT_NAME);
        strcpy(strDeviceFullProductName, KL720_FULL_PRODUCT_NAME);
        break;
    case KP_DEVICE_KL630:
        strcpy(strDeviceInfName, "kneron_kl630.inf");
        strcpy(strDeviceProductName, KL630_PRODUCT_NAME);
        strcpy(strDeviceFullProductName, KL630_FULL_PRODUCT_NAME);
        break;
    default:
        break;
    }

    struct wdi_device_info *ldev, dev = {NULL, device_vid, device_pid, FALSE, 0,
                                         strDeviceFullProductName, NULL, NULL, NULL};
    struct wdi_options_create_list ocl = { 0 };
    struct wdi_options_prepare_driver opd = { 0 };
    struct wdi_options_install_driver oid = { 0 };
    struct wdi_options_install_cert oic = { 0 };
    BOOL matching_device_found = FALSE;
    char *cert_name = NULL;

    ocl.list_all = TRUE;
    ocl.list_hubs = TRUE;
    ocl.trim_whitespaces = TRUE;
    opd.driver_type = WDI_WINUSB;

    dbg_print("[%s] Extracting '%s' driver files...  ", __FUNCTION__, strDeviceProductName);
    Ret = wdi_prepare_driver(&dev, DRIVER_FILE_PATH, strDeviceInfName, &opd);
    dbg_print("%s\n", wdi_strerror(Ret));

    if (WDI_SUCCESS != Ret) {
        goto FUNC_OUT;
    }

    if (NULL != cert_name) {
        dbg_print("[%s] Installing certificate '%s' as a Trusted Publisher...  ", __FUNCTION__, cert_name);
        Ret = wdi_install_trusted_certificate(cert_name, &oic);
        dbg_print("%s\n", wdi_strerror(Ret));

        if (WDI_SUCCESS != Ret) {
            goto FUNC_OUT;
        }
    }

    // Try to match against a plugged device to avoid device manager prompts
    Ret = wdi_create_list(&ldev, &ocl);

    if (WDI_SUCCESS == Ret) {
        for (; (NULL != ldev) && (WDI_SUCCESS == Ret); ldev = ldev->next) {
            if ((ldev->vid == dev.vid) && (ldev->pid == dev.pid) &&
                (ldev->mi == dev.mi) && (ldev->is_composite == dev.is_composite) ) {
                dev.hardware_id = ldev->hardware_id;
                dev.device_id = ldev->device_id;
                matching_device_found = TRUE;

                dbg_print("[%s] Installing '%s' driver...  ", __FUNCTION__, strDeviceProductName);
                Ret = wdi_install_driver(&dev, DRIVER_FILE_PATH, strDeviceInfName, &oid);
                dbg_print("%s\n", wdi_strerror(Ret));

                if (WDI_SUCCESS != Ret) {
                    goto FUNC_OUT;
                }
            }
        }
    }

    // No plugged USB device matches this one -> install driver
    if (!matching_device_found) {
        dbg_print("[%s] Installing '%s' driver...  ", __FUNCTION__, strDeviceProductName);
        Ret = wdi_install_driver(&dev, DRIVER_FILE_PATH, strDeviceInfName, &oid);
        dbg_print("%s\n", wdi_strerror(Ret));

        if (WDI_SUCCESS != Ret) {
            goto FUNC_OUT;
        }
    }

    /* Delete drivers folder. */
    SHFILEOPSTRUCT FileOp;
    FileOp.fFlags = FOF_NOCONFIRMATION;
    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
    FileOp.pFrom = DRIVER_FILE_PATH;
    FileOp.pTo = NULL;
    FileOp.wFunc = FO_DELETE;

    SHFileOperationA(&FileOp);

FUNC_OUT:
    /* Re-mapping error code from wdi_error(libwdi) to KP_API_RETURN_CODE */
    if (WDI_SUCCESS > Ret) {
        Ret += KP_ERROR_WDI_BEGIN;
    }

#else
    Ret = KP_ERROR_INVALID_HOST_38;
#endif

    return Ret;
}

int kp_store_ddr_manage_attr(kp_device_group_t devices, kp_ddr_manage_attr_t ddr_attr)
{
    memcpy(&devices->ddr_attr, &ddr_attr, sizeof(kp_ddr_manage_attr_t));

    return KP_SUCCESS;
}

// For debug use, only support 1 device
int kp_memory_read(kp_device_group_t devices, int dev_port_id, uint32_t start_address, uint32_t length, uint8_t *buffer)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    // Search for device with matched port id and corresponding scan index
    int scan_index;
    for (scan_index = 0; scan_index < _devices_grp->num_device; scan_index++)
    {
        if (dev_port_id == _devices_grp->ll_device[scan_index]->dev_descp.port_id)
            break;
    }

    if (scan_index == _devices_grp->num_device)
        return KP_ERROR_DEVICE_NOT_EXIST_10;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[scan_index];

    kdp2_ipc_cmd_memory_read_write_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_memory_read_write_t);
    cmd_buf.command_id = KDP2_COMMAND_MEMORY_READ;
    cmd_buf.start_address = start_address;
    cmd_buf.length = length;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, _devices_grp->timeout);
    int status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS)
        return status;

    // Wait for FW reading the specified memory section ? Need some delay ?

    uint32_t return_code;

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(uint32_t), _devices_grp->timeout);
    status = check_usb_read_data_error(ret);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(uint32_t))
    {
        ret = KP_SUCCESS;
    }
    else
        ret = KP_ERROR_OTHER_99;

    if (ret != KP_SUCCESS)
        return ret;

    // Get memory data from FW
    ret = kp_usb_read_data(ll_dev, (void *)buffer, (int)length, _devices_grp->timeout);
    status = check_usb_read_data_error(ret);

    if (status != KP_SUCCESS)
        ret = status;
    else if (ret == (int)length)
    {
        ret = KP_SUCCESS;
    }
    else
        ret = KP_ERROR_OTHER_99;

    return ret;
}

// For debug use, only support 1 device
int kp_memory_write(kp_device_group_t devices, int dev_port_id, uint32_t start_address, uint32_t length, uint8_t *buffer)
{
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;

    // Search for device with matched port id and corresponding scan index
    int scan_index;
    for (scan_index = 0; scan_index < _devices_grp->num_device; scan_index++)
    {
        if (dev_port_id == _devices_grp->ll_device[scan_index]->dev_descp.port_id)
            break;
    }

    if (scan_index == _devices_grp->num_device)
        return KP_ERROR_DEVICE_NOT_EXIST_10;

    kp_usb_device_t *ll_dev = _devices_grp->ll_device[scan_index];

    kdp2_ipc_cmd_memory_read_write_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_memory_read_write_t);
    cmd_buf.command_id = KDP2_COMMAND_MEMORY_WRITE;
    cmd_buf.start_address = start_address;
    cmd_buf.length = length;

    int ret = kp_usb_write_data(ll_dev, (void *)&cmd_buf, cmd_buf.total_size, _devices_grp->timeout);
    int status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS)
        return status;

    ret = kp_usb_write_data(ll_dev, (void *)buffer, (int)length, _devices_grp->timeout);
    status = check_usb_write_data_error(ret);
    if (status != KP_SUCCESS)
        return status;

    // Wait for FW writing the specified memory section ? Need some delay ?

    uint32_t return_code;

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(uint32_t), _devices_grp->timeout);
    status = check_usb_read_data_error(ret);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(uint32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    return ret;
}

const char *kp_get_version()
{
    return plus_version;
}
