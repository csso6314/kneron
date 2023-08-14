#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "kdp2_app.h"
#include "kdp2_ll.h"
#include "kdp2_core.h"

#include "kdp2_internal.h"
#include "khost_ipc_inf_app_tiny_yolo.h"
#include "model_type.h"

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) printf(format, ##__VA_ARGS__)
#else
#define dbg_print(format, ...)
#endif

int kdp2_app_tiny_yolo_v3_send(kdp2_device_t device, uint32_t inference_number, uint8_t *image_buffer,
                               uint32_t width, uint32_t height, kdp2_image_format_t format)
{
    kdp2_ll_device_t *ll_dev = ((_kdp2_device_t *)device)->ll_device;
    int timeout = ((_kdp2_device_t *)device)->timeout;
    uint32_t image_size = 0;
    uint32_t config = 0;

    khost_ipc_inf_tiny_yolo_send_t ll_ty_inf_desc;

    switch (format)
    {
    case KDP2_IMAGE_FORMAT_RGB565:
        config |= NPU_FORMAT_RGB565;
        image_size = width * height * 2;
        break;
    case KDP2_IMAGE_FORMAT_RGBA8888:
        image_size = width * height * 4;
        break;
    default:
    case KDP2_IMAGE_FORMAT_UNKNOWN:
        return KDP2_ERROR_INVALID_PARAM;
    }

    if (image_size > IMAGE_BUFFER_SIZE)
    {
        dbg_print("[%s] image buffer size is not enough in firmware\n", __func__);
        return KDP2_ERROR_SEND_DATA_TOO_LARGE;
    }

    ll_ty_inf_desc.total_size = sizeof(ll_ty_inf_desc) + image_size;
    ll_ty_inf_desc.desc_type = KHOST_INF_IMAGE_TINY_YOLO;
    ll_ty_inf_desc.request_id = inference_number;

    ll_ty_inf_desc.image.width = width;
    ll_ty_inf_desc.image.height = height;
    ll_ty_inf_desc.image.channel = 3; // ??
    ll_ty_inf_desc.image.config = config | IMAGE_FORMAT_SUB128;

    ll_ty_inf_desc.model.use_builtin_model = false; // FIXME when in flash
    ll_ty_inf_desc.model.model_count = 1;
    ll_ty_inf_desc.model.model_ids[0] = TINY_YOLO_V3_224_224_3; // FIXME, force using this model

    int status = KDP2_SUCCESS;

    do
    {
        int ret = 0;
        ret = kdp2_ll_write_data(ll_dev, (void *)&ll_ty_inf_desc, sizeof(ll_ty_inf_desc), timeout);
        if (ret == KDP2_LL_RET_TIMEOUT)
        {
            dbg_print("[%s] Write descriptor to FIFO queue timeout [%d ms]\n", __func__, timeout);
            status = KDP2_ERROR_TIMEOUT;
            break;
        }
        if (ret != KDP2_LL_RET_OK)
        {
            dbg_print("[%s] Write descriptor to FIFO queue error. kdp2_ll_write_data() return code = [%d]\n", __func__, ret);
            status = KDP2_ERROR_SEND_DESC_FAIL;
            break;
        }

        ret = kdp2_ll_write_data(ll_dev, (void *)image_buffer, image_size, timeout);
        if (ret == KDP2_LL_RET_TIMEOUT)
        {
            dbg_print("[%s] Write data to FIFO queue timeout [%d ms]\n", __func__, timeout);
            status = KDP2_ERROR_TIMEOUT;
            break;
        }
        if (ret != KDP2_LL_RET_OK)
        {
            dbg_print("[%s] Write data to FIFO queue error. kdp2_ll_write_data() return code = [%d]\n", __func__, ret);
            status = KDP2_ERROR_SEND_DATA_FAIL;
            break;
        }
    } while (0);

    return status;
}

int kdp2_app_tiny_yolo_v3_receive(kdp2_device_t device, uint32_t *inference_number, uint32_t *box_count, kdp2_bounding_box_t boxes[])
{
    khost_ipc_inf_tiny_yolo_recv_t *ty_result = (khost_ipc_inf_tiny_yolo_recv_t *)malloc(sizeof(khost_ipc_inf_tiny_yolo_recv_t));

    kdp2_ll_device_t *ll_dev = ((_kdp2_device_t *)device)->ll_device;
    int timeout = ((_kdp2_device_t *)device)->timeout;

    int status = KDP2_SUCCESS;

    do
    {
        int ret = kdp2_ll_read_data(ll_dev, (void *)ty_result, sizeof(khost_ipc_inf_tiny_yolo_recv_t), timeout);
        if (ret == KDP2_LL_RET_TIMEOUT)
        {
            dbg_print("[%s] Read data from FIFO queue timeout [%d ms]\n", __func__, timeout);
            status = KDP2_ERROR_TIMEOUT;
            break;
        }
        if (ret < KDP2_LL_RET_OK)
        {
            dbg_print("[%s] Read data from FIFO queue error. kdp2_ll_read_data() return code = [%d]\n", __func__, ret);
            status = KDP2_ERROR_RECV_DATA_FAIL;
            break;
        }

        if (ty_result->return_code != KHOST_IPC_RETURN_SUCCESS)
        {
            status = ty_result->return_code;
            break;
        }

        if (ty_result->desc_type != KHOST_INF_IMAGE_TINY_YOLO)
        {
            dbg_print("[%s] desc_type is not correct : %d\n", __func__, ty_result->desc_type);
            status = KDP2_ERROR_RECV_DESC_FAIL;
            break;
        }

        *inference_number = ty_result->request_id;
        *box_count = ty_result->box_count;

        memcpy(boxes, ty_result->boxes, ty_result->box_count * sizeof(kdp2_bounding_box_t));

    } while (0);

    free(ty_result);

    return status;
}
