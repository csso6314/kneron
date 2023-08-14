#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kdp_host.h"
#include "khost_api.h"
#include "KnLog.h"
#include "KdpHostApi.h"

int khost_send_model(const khost_device_t dev, const void* model, const uint32_t model_size, const void* fwinfo, const uint32_t fwinfo_size) {

    if (!dev) {
        KnPrint(KN_LOG_ERR, "[%s] No Kneron device\n", __func__);
        return KHOST_ERROR_NO_DEV;
    }

    if (!model) {
        KnPrint(KN_LOG_ERR, "[%s] No all_models.bin data\n", __func__);
        return KHOST_ERROR_INVALID_PARAM;
    }

    if (!fwinfo) {
        KnPrint(KN_LOG_ERR, "[%s] No fw_info.bin data\n", __func__);
        return KHOST_ERROR_INVALID_PARAM;
    }

    int dev_idx = kdp_khost_dev_to_dev_idx(dev);
    if (dev_idx == -1) {
        KnPrint(KN_LOG_ERR, "[%s] Cloud not get device index when send models to Kneron device\n", __func__);
        return KHOST_ERROR_NO_DEV_ID;
    }

    int ret = 0;
    uint32_t ret_size = 0;
    ret = kdp_start_dme(dev_idx, model_size, (char*)fwinfo, fwinfo_size, &ret_size, (char*)model, model_size);
    if (ret != 0) {
        KnPrint(KN_LOG_ERR, "[%s] Could not send model to Kneron device. kdp_start_dme() return code = [%d]\n", __func__, ret_size);
        return KHOST_ERROR_SEND_MODEL_FAIL;
    }

    return KHOST_SUCCESS;
}

int khost_inference_send(const khost_device_t dev, const void* desc, const uint32_t desc_size, const void* data, const uint32_t data_size, uint32_t timeout) {

    if (!dev) {
        KnPrint(KN_LOG_ERR, "[%s] No Kneron device\n", __func__);
        return KHOST_ERROR_NO_DEV;
    }

    if (!desc) {
        KnPrint(KN_LOG_ERR, "[%s] No FIFO descriptor data\n", __func__);
        return KHOST_ERROR_INVALID_PARAM;
    }

    if (desc_size > IMAGE_BUFFER_SIZE) {
        KnPrint(KN_LOG_ERR, "[%s] Sending descriptor size is too large. Send = %d; Max = %d\n", __func__,  desc_size, IMAGE_BUFFER_SIZE);
        return KHOST_ERROR_SEND_DATA_TOO_LARGE;
    }

    if (!data) {
        KnPrint(KN_LOG_ERR, "[%s] No data file\n", __func__);
        return KHOST_ERROR_INVALID_PARAM;
    }

    if (data_size > IMAGE_BUFFER_SIZE) {
        KnPrint(KN_LOG_ERR, "[%s] Sending data size is too large. Send = %d; Max = %d\n", __func__,  data_size, IMAGE_BUFFER_SIZE);
        return KHOST_ERROR_SEND_DATA_TOO_LARGE;
    }

    int ret = 0;
    ret = khost_ll_write_data((void*)dev, (void*)desc, desc_size, timeout);
    if (ret == KHOST_RET_TIMEOUT) {
        KnPrint(KN_LOG_ERR, "[%s] Write descriptor to FIFO queue timeout [%d ms]\n", __func__, timeout);
        return KHOST_ERROR_TIMEOUT;
    }
    if (ret != KHOST_RET_OK) {
        KnPrint(KN_LOG_ERR, "[%s] Write descriptor to FIFO queue error. khost_ll_write_data() return code = [%d]\n", __func__, ret);
        return KHOST_ERROR_SEND_DESC_FAIL;
    }

    ret = khost_ll_write_data((void*)dev, (void*)data, data_size, timeout);
    if (ret == KHOST_RET_TIMEOUT) {
        KnPrint(KN_LOG_ERR, "[%s] Write data to FIFO queue timeout [%d ms]\n", __func__, timeout);
        return KHOST_ERROR_TIMEOUT;
    }
    if (ret != KHOST_RET_OK) {
        KnPrint(KN_LOG_ERR, "[%s] Write data to FIFO queue error. khost_ll_write_data() return code = [%d]\n", __func__, ret);
        return KHOST_ERROR_SEND_DATA_FAIL;
    }

    return KHOST_SUCCESS;
}

int khost_inference_recv(const khost_device_t dev, void* recv, uint32_t recv_size, uint32_t timeout) {

    int ret = 0;

    if (!dev) {
        KnPrint(KN_LOG_ERR, "[%s] No Kneron device\n", __func__);
        return KHOST_ERROR_NO_DEV;
    }

    if (!recv) {
        KnPrint(KN_LOG_ERR, "[%s] No receive buffer\n", __func__);
        return KHOST_ERROR_INVALID_PARAM;
    }

    if (recv_size > RESULT_BUFFER_SIZE) {
        KnPrint(KN_LOG_ERR, "[%s] Receiving data size is too large. Recv = %d; Max = %d\n", __func__,  recv_size, RESULT_BUFFER_SIZE);
        return KHOST_ERROR_RECV_DATA_TOO_LARGE;
    }

    ret = khost_ll_read_data((void*)dev, recv, recv_size, timeout);
    if (ret == KHOST_RET_TIMEOUT) {
        KnPrint(KN_LOG_ERR, "[%s] Read data from FIFO queue timeout [%d ms]\n", __func__, timeout);
        return KHOST_ERROR_TIMEOUT;
    }
    if (ret < KHOST_RET_OK) {
        KnPrint(KN_LOG_ERR, "[%s] Read data from FIFO queue error. khost_ll_read_data() return code = [%d]\n", __func__, ret);
        return KHOST_ERROR_RECV_DATA_FAIL;
    }

    return KHOST_SUCCESS;
}

int khost_validate_version(const khost_device_t dev, uint32_t timeout) {

    if (!dev) {
        KnPrint(KN_LOG_ERR, "[%s] No Kneron device\n", __func__);
        return KHOST_ERROR_NO_DEV;
    }

    int ret = 0;
    struct khost_ipc_sys_version_s send;
    struct khost_ipc_sys_version_s recv;

    memset(&send, 0, sizeof(send));
    send.desc_type = KHOST_SYS_VERSION;
    send.total_size = sizeof(send);
    send.version = KHOST_IPC_VERSION;

    KnPrint(KN_LOG_MSG, "[%s] Starting to check khost IPC version...\n", __func__);
    ret = khost_ll_write_data((void*)dev, (void*)&send, sizeof(send), timeout);
    if (ret == KHOST_RET_TIMEOUT) {
        KnPrint(KN_LOG_ERR, "[%s] Write descriptor to FIFO queue timeout [%d ms]\n", __func__, timeout);
        return KHOST_ERROR_TIMEOUT;
    }
    if (ret != KHOST_RET_OK) {
        KnPrint(KN_LOG_ERR, "[%s] Write descriptor to FIFO queue error. khost_ll_write_data() return code = [%d]\n", __func__, ret);
        return KHOST_ERROR_SEND_DATA_FAIL;
    }

    memset(&recv, 0, sizeof(recv));
    ret = khost_ll_read_data((void*)dev, (void*)&recv, sizeof(recv), timeout);
    if (ret == KHOST_RET_TIMEOUT) {
        KnPrint(KN_LOG_ERR, "[%s] Read data from FIFO queue timeout [%d ms]\n", __func__, timeout);
        return KHOST_ERROR_TIMEOUT;
    }
    if (ret < KHOST_RET_OK) {
        KnPrint(KN_LOG_ERR, "[%s] Read data from FIFO queue error. khost_ll_read_data() return code = [%d]\n", __func__, ret);
        return KHOST_ERROR_RECV_DATA_FAIL;
    }

    if (recv.desc_type != KHOST_SYS_VERSION) {
        KnPrint(KN_LOG_ERR, "[%s] Recv type should be [version], but type = [%d]\n", __func__, recv.desc_type);
        return KHOST_ERROR_RECV_DESC_FAIL;
    }

    if (recv.return_code != KHOST_IPC_RETURN_SUCCESS) {
        KnPrint(KN_LOG_ERR, "[%s] Version validation error on FW. Error code = [%d]\n", __func__, recv.return_code);
        return KHOST_ERROR_RECV_DESC_FAIL;
    }

    if (recv.version != KHOST_IPC_VERSION) {
        KnPrint(KN_LOG_ERR, "[%s] Version doesn't match. Recv version = [%d]. Expected version = [%d]\n", __func__, recv.version, KHOST_IPC_VERSION);
        return KHOST_ERROR_INVALID_VERSION;
    }

    return KHOST_SUCCESS;
}

int khost_reset_device(const khost_device_t dev, uint32_t reset_mode, uint32_t timeout) {

    if (!dev) {
        KnPrint(KN_LOG_ERR, "[%s] No Kneron device\n", __func__);
        return KHOST_ERROR_NO_DEV;
    }

    int ret = 0;
    khost_control_t kctrl;
    memset(&kctrl, 0, sizeof(kctrl));

    if (reset_mode == KHOST_SYS_HARD_RESET) {
        // For hardware reset, need to send 2 control commands.

        // 1. Make sure if the device is connected or not
        kctrl.command = CONTROL_RESET_HARDWARE;
        kctrl.arg1 = 0;
        kctrl.arg2 = 0;

        ret = khost_ll_control(dev, &kctrl, timeout);
        if (ret != KHOST_RET_OK) {
            KnPrint(KN_LOG_ERR, "[%s] Send usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
            return KHOST_ERROR_DEV_OP_FAIL;
        }

        // 2. Really reset the device. The device would be disconnected.
        kctrl.arg1 = 1;
        khost_ll_control(dev, &kctrl, timeout);

        // 3. Wait some time to restart hardware. 5 seconds is an experimental time. We could modify this value if something wrong.
        sleep(5);

    } else if (reset_mode == KHOST_SYS_SOFT_INF_QUEUE_RESET) {
        // Need to do reset twice due to producer and consumer issues

        kctrl.command = CONTROL_FIFOQ_RESET;
        kctrl.arg1 = 0;
        kctrl.arg2 = 0;

        int ret = khost_ll_control(dev, &kctrl, timeout);
        if (ret != KHOST_RET_OK)
        {
            KnPrint(KN_LOG_ERR, "[%s] Send usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
            return KHOST_ERROR_DEV_OP_FAIL;
        }

        // Give firmware some time to react. 0.1 second is an experimental time. We could modify this value if something wrong.
        usleep(100000);

        ret = khost_ll_control(dev, &kctrl, timeout);
        if (ret != KHOST_RET_OK)
        {
            KnPrint(KN_LOG_ERR, "[%s] Send usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
            return KHOST_ERROR_DEV_OP_FAIL;
        }

    } else {
        KnPrint(KN_LOG_ERR, "[%s] Not support this reset mode [%d]\n", __func__, reset_mode);
        return KHOST_ERROR_NOT_SUPPORTED;
    }

    return KHOST_SUCCESS;
}

const char* khost_ret_code_name(int ret_code) {

    switch (ret_code) {
        case KHOST_SUCCESS:
            return "Success.";
        case KHOST_ERROR_NO_DEV:
            return "No Kneron device as API parameter.";
        case KHOST_ERROR_NO_DEV_ID:
            return "Cannot get device ID from Kneron device. The device ID is used for kdp_xxx API.";
        case KHOST_ERROR_DEV_NOT_CONNECTED:
            return "The device isn't connected.";
        case KHOST_ERROR_DEV_OP_FAIL:
            return "Operation failed on the device.";
        case KHOST_ERROR_TIMEOUT:
            return "Operation timed out.";
        case KHOST_ERROR_INVALID_PARAM:
            return "Invalid parameters for API.";
        case KHOST_ERROR_INVALID_VERSION:
            return "The versions on host and FW sides are not the same.";
        case KHOST_ERROR_SEND_DESC_FAIL:
            return "Send descriptor failed.";
        case KHOST_ERROR_SEND_DATA_FAIL:
            return "Send data failed.";
        case KHOST_ERROR_SEND_DATA_TOO_LARGE:
            return "Sending data is too large.";
        case KHOST_ERROR_SEND_MODEL_FAIL:
            return "Send models to Kneron device failed.";
        case KHOST_ERROR_RECV_DESC_FAIL:
            return "Receive descriptor failed. The descriptor format is incorrect.";
        case KHOST_ERROR_RECV_DATA_FAIL:
            return "Receive data failed.";
        case KHOST_ERROR_RECV_DATA_TOO_LARGE:
            return "Receiving data is too large.";
        case KHOST_ERROR_NOT_SUPPORTED:
            return "Operation not supported or not implemented.";
        case KHOST_ERROR_OTHER:
            return "Other error.";
        default:
            return "Unknown return code.";
    }
}
