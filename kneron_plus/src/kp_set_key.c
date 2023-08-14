/**
 * @file        kp_set_key.c
 * @brief       internal set key functions
 * @version     1.1
 * @date        2021-08-09
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

//#define DEBUG_PRINT

#include <pthread.h>

#include "kp_set_key.h"
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
#define dbg_print(format, ...) printf(format, ##__VA_ARGS__)
#else
#define dbg_print(format, ...)
#endif

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

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    kdp2_ipc_cmd_set_ckey_t *cmd_buf;
    int timeout;
    int sts;
} _set_ckey_command_package;

static void *_set_ckey_to_single_device(void *data)
{
    _set_ckey_command_package *cmd_pack = (_set_ckey_command_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;

    dbg_print("[%s][%d] device %p, cmd_buf %p, ckey 0x%08X, timeout %d\n", __FUNCTION__, cmd_pack->dev_idx,
              ll_dev, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->ckey, cmd_pack->timeout);

    int ret = kp_usb_write_data(ll_dev, cmd_pack->cmd_buf, sizeof(kdp2_ipc_cmd_set_ckey_t), cmd_pack->timeout);

    if (KP_USB_RET_OK != ret) {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write cmd_buf failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }

    int32_t return_code;

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(return_code), cmd_pack->timeout);
    int status = check_usb_read_data_error(ret);

    dbg_print("[%s] set ckey sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(int32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    if (ret != KP_SUCCESS) {
        cmd_pack->sts = ret;
        return NULL;
    }

    cmd_pack->sts = return_code;

FUNC_OUT:

    return NULL;
}

int kp_set_ckey(kp_device_group_t devices, uint32_t ckey)
{
    int ret = 0;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        if (((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER == (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL720 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_KL720_LOADER == ll_dev[i]->fw_serial))) {
            dbg_print("one or more devices are running KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    dbg_print("start set ckey...\n");

    kdp2_ipc_cmd_set_ckey_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.command_id = KDP2_COMMAND_SET_CKEY;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_set_ckey_t);
    cmd_buf.ckey = ckey;

    _set_ckey_command_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t set_ckey_thd[MAX_GROUP_DEVICE];

    cmd_packs[0].dev_idx = 0;
    cmd_packs[0].ll_device = ll_dev[0];
    cmd_packs[0].cmd_buf = &cmd_buf;
    cmd_packs[0].timeout = devices->timeout;

    for (int i = 1; i < _devices_grp->num_device; i++) {
        dbg_print("[%s] create thread to set ckey to device %d\n", __FUNCTION__, i);
        memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_set_ckey_command_package));
        cmd_packs[i].ll_device = ll_dev[i];
        cmd_packs[i].dev_idx = i;

        int thd_ret = pthread_create(&set_ckey_thd[i], NULL, _set_ckey_to_single_device, (void *)&cmd_packs[i]);

        if (0 != thd_ret) {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return -1;
        }
    }

    // current thread do first device
    _set_ckey_to_single_device((void *)&cmd_packs[0]);

    for (int i = 1; i < _devices_grp->num_device; i++) {
        pthread_join(set_ckey_thd[i], NULL);
    }

    for (int i = 0; i < _devices_grp->num_device; i++) {
        ret = cmd_packs[i].sts;
        if (KP_USB_RET_OK != ret) {
            dbg_print("[%s] thread set ckey failed at device %d, error %d\n", __FUNCTION__, i, ret);
            break;
        }
    }

    return ret;
}

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    kdp2_ipc_cmd_set_sbt_key_t *cmd_buf;
    int timeout;
    int sts;
} _set_sbt_key_command_package;

static void *_set_sbt_key_to_single_device(void *data)
{
    _set_sbt_key_command_package *cmd_pack = (_set_sbt_key_command_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;

    dbg_print("[%s][%d] device %p, cmd_buf %p, entry 0x%08X, key 0x%08X, timeout %d\n", __FUNCTION__, cmd_pack->dev_idx,
              ll_dev, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->entry, cmd_pack->cmd_buf->key, cmd_pack->timeout);

    int ret = kp_usb_write_data(ll_dev, cmd_pack->cmd_buf, sizeof(kdp2_ipc_cmd_set_sbt_key_t), cmd_pack->timeout);

    if (KP_USB_RET_OK != ret) {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write cmd_buf failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }

    int32_t return_code;

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(return_code), cmd_pack->timeout);
    int status = check_usb_read_data_error(ret);

    dbg_print("[%s] set secure boot key sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(int32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    if (ret != KP_SUCCESS) {
        cmd_pack->sts = ret;
        return NULL;
    }

    cmd_pack->sts = return_code;

FUNC_OUT:

    return NULL;
}

int kp_set_secure_boot_key(kp_device_group_t devices, uint32_t entry, uint32_t key)
{
    int ret = 0;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        if (((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER == (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL720 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_KL720_LOADER == ll_dev[i]->fw_serial))) {
            dbg_print("one or more devices are running KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    dbg_print("start set secure boot key...\n");

    kdp2_ipc_cmd_set_sbt_key_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.command_id = KDP2_COMMAND_SET_SBT_KEY;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_set_sbt_key_t);
    cmd_buf.entry = entry;
    cmd_buf.key = key;

    _set_sbt_key_command_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t set_sbt_key_thd[MAX_GROUP_DEVICE];

    cmd_packs[0].dev_idx = 0;
    cmd_packs[0].ll_device = ll_dev[0];
    cmd_packs[0].cmd_buf = &cmd_buf;
    cmd_packs[0].timeout = devices->timeout;

    for (int i = 1; i < _devices_grp->num_device; i++) {
        dbg_print("[%s] create thread to set sbt key to device %d\n", __FUNCTION__, i);
        memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_set_sbt_key_command_package));
        cmd_packs[i].ll_device = ll_dev[i];
        cmd_packs[i].dev_idx = i;

        int thd_ret = pthread_create(&set_sbt_key_thd[i], NULL, _set_sbt_key_to_single_device, (void *)&cmd_packs[i]);

        if (0 != thd_ret) {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return -1;
        }
    }

    // current thread do first device
    _set_sbt_key_to_single_device((void *)&cmd_packs[0]);

    for (int i = 1; i < _devices_grp->num_device; i++) {
        pthread_join(set_sbt_key_thd[i], NULL);
    }

    for (int i = 0; i < _devices_grp->num_device; i++) {
        ret = cmd_packs[i].sts;
        if (KP_USB_RET_OK != ret) {
            dbg_print("[%s] thread set sbt key failed at device %d, ret error %d\n", __FUNCTION__, i, ret);
            break;
        }
    }

    return ret;
}

typedef struct
{
    int dev_idx;
    kp_usb_device_t *ll_device;
    kdp2_ipc_cmd_set_gpio_t *cmd_buf;
    int timeout;
    int sts;
} _set_gpio_command_package;

static void *_set_gpio_to_single_device(void *data)
{
    _set_gpio_command_package *cmd_pack = (_set_gpio_command_package *)data;
    kp_usb_device_t *ll_dev = cmd_pack->ll_device;

    dbg_print("[%s][%d] device %p, cmd_buf %p, pin 0x%08X, value 0x%08X, timeout %d\n", __FUNCTION__, cmd_pack->dev_idx,
              ll_dev, (void *)cmd_pack->cmd_buf, cmd_pack->cmd_buf->pin, cmd_pack->cmd_buf->value, cmd_pack->timeout);

    int ret = kp_usb_write_data(ll_dev, cmd_pack->cmd_buf, sizeof(kdp2_ipc_cmd_set_gpio_t), cmd_pack->timeout);

    if (KP_USB_RET_OK != ret) {
        cmd_pack->sts = ret;
        dbg_print("[%s][%d] write cmd_buf failed, error %d\n", __FUNCTION__, cmd_pack->dev_idx, cmd_pack->sts);
        goto FUNC_OUT;
    }

    int32_t return_code;

    ret = kp_usb_read_data(ll_dev, (void *)&return_code, sizeof(return_code), cmd_pack->timeout);
    int status = check_usb_read_data_error(ret);

    dbg_print("[%s] set gpio sts : return_code %d, usb_sts %d, timeout %d\n", __FUNCTION__, return_code, ret, cmd_pack->timeout);

    if (status != KP_SUCCESS)
        ret = status;
    else if (return_code != KP_SUCCESS)
        ret = return_code;
    else if (ret == sizeof(int32_t))
        ret = KP_SUCCESS;
    else
        ret = KP_ERROR_OTHER_99;

    if (ret != KP_SUCCESS) {
        cmd_pack->sts = ret;
        return NULL;
    }

    cmd_pack->sts = return_code;

FUNC_OUT:

    return NULL;
}

int kp_set_gpio(kp_device_group_t devices, uint32_t pin, uint32_t value)
{
    int ret = 0;
    _kp_devices_group_t *_devices_grp = (_kp_devices_group_t *)devices;
    kp_usb_device_t **ll_dev = _devices_grp->ll_device;

    for (int i = 0; i < _devices_grp->num_device; i++) {
        if (((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER_V2 == (KP_KDP2_FW_FIND_TYPE_MASK_V2 & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL520 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_LOADER == (KP_KDP2_FW_FIND_TYPE_MASK & ll_dev[i]->fw_serial))) ||
            ((KP_DEVICE_KL720 == ll_dev[i]->dev_descp.product_id) && (KP_KDP2_FW_KL720_LOADER == ll_dev[i]->fw_serial))) {
            dbg_print("one or more devices are running KDP2 Loader...\n");
            return KP_ERROR_INVALID_FIRMWARE_24;
        }
    }

    dbg_print("start set gpio...\n");

    kdp2_ipc_cmd_set_gpio_t cmd_buf;

    cmd_buf.magic_type = KDP2_MAGIC_TYPE_COMMAND;
    cmd_buf.command_id = KDP2_COMMAND_SET_GPIO;
    cmd_buf.total_size = sizeof(kdp2_ipc_cmd_set_gpio_t);
    cmd_buf.pin = pin;
    cmd_buf.value = value;

    _set_gpio_command_package cmd_packs[MAX_GROUP_DEVICE];
    pthread_t set_gpio_thd[MAX_GROUP_DEVICE];

    cmd_packs[0].dev_idx = 0;
    cmd_packs[0].ll_device = ll_dev[0];
    cmd_packs[0].cmd_buf = &cmd_buf;
    cmd_packs[0].timeout = devices->timeout;

    for (int i = 1; i < _devices_grp->num_device; i++) {
        dbg_print("[%s] create thread to set gpio to device %d\n", __FUNCTION__, i);
        memcpy((void *)&cmd_packs[i], (void *)&cmd_packs[0], sizeof(_set_gpio_command_package));
        cmd_packs[i].ll_device = ll_dev[i];

        int thd_ret = pthread_create(&set_gpio_thd[i], NULL, _set_gpio_to_single_device, (void *)&cmd_packs[i]);

        if (0 != thd_ret) {
            dbg_print("[%s] thread creation failed ! error %d\n", __FUNCTION__, thd_ret);
            return -1;
        }
    }

    // current thread do first device
    _set_gpio_to_single_device((void *)&cmd_packs[0]);

    for (int i = 1; i < _devices_grp->num_device; i++) {
        pthread_join(set_gpio_thd[i], NULL);
    }

    for (int i = 0; i < _devices_grp->num_device; i++) {
        ret = cmd_packs[i].sts;
        if (KP_USB_RET_OK != ret) {
            dbg_print("[%s] thread set gpio failed at device %d, ret error %d\n", __FUNCTION__, i, ret);
            break;
        }
    }

    return ret;
}