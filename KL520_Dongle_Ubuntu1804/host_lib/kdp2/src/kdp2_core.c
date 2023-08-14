#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "kdp2_ll.h"
#include "kdp2_internal.h"

#include "kdp2_core.h"

#include "com.h"
#include "khost_ipc_sys_version.h"
#include "khost_ipc_inf_user.h"

#include "internal_func.h"

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) printf(format, ##__VA_ARGS__)
#else
#define dbg_print(format, ...)
#endif

// This ACK packet is from Firmware
static uint8_t ack_packet[] = {0x35, 0x8A, 0xC, 0, 0x4, 0, 0x8, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int _send_command_header(kdp2_ll_device_t ll_dev, int cmd_id, void *buf, int len, int timeout)
{
    int sts = kdp2_ll_write_command(ll_dev, buf, len, timeout);
    if (sts != KDP2_LL_RET_OK)
    {
        dbg_print("send command (0x%X) failed, error = %d\n", cmd_id, sts);
        return sts;
    }
    return KDP2_SUCCESS;
}

static int _send_command_data(kdp2_ll_device_t ll_dev, int cmd_id, void *buf, int len, int timeout)
{
    int sts = kdp2_ll_write_command(ll_dev, buf, len, timeout);
    if (sts != KDP2_LL_RET_OK)
    {
        dbg_print("send command (0x%X) data failed, error = %d\n", cmd_id, sts);
        return sts;
    }
    return KDP2_SUCCESS;
}

static int _receive_command_ack(kdp2_ll_device_t ll_dev, int cmd_id, int timeout)
{
    uint8_t response[ACK_RESPONSE_LEN];
    int recv_len = kdp2_ll_read_command(ll_dev, response, ACK_RESPONSE_LEN, timeout);
    if (recv_len < 0)
    {
        dbg_print("receive command (0x%X) ACK failed, error = %d\n", cmd_id, recv_len);
        return recv_len;
    }

    if (recv_len != ACK_RESPONSE_LEN)
    {
        dbg_print("receive command (0x%X) ACK failed, received length %d != %d\n", cmd_id, recv_len, ACK_RESPONSE_LEN);
        return KDP2_ERROR_OTHER;
    }

    if (memcmp(ack_packet, response, ACK_RESPONSE_LEN) != 0)
    {
        dbg_print("receive command (0x%X) ACK failed, packet is not correct\n", cmd_id);
        return KDP2_ERROR_OTHER;
    }

    return KDP2_SUCCESS;
}

static int _recevie_command_response(kdp2_ll_device_t ll_dev, int cmd_id, uint8_t *recv_buf, int recv_buf_size, int timeout)
{
    int recv_len = kdp2_ll_read_command(ll_dev, recv_buf, recv_buf_size, timeout);
    if (recv_len < 0)
    {
        dbg_print("receive command (0x%X) message header failed, error = %d\n", cmd_id, recv_len);
        return recv_len;
    }

    MsgHdr *rsp_hdr = (MsgHdr *)recv_buf;

    if (rsp_hdr->preamble != MSG_HDR_RSP)
    {
        dbg_print("receive command (0x%X) message header failed, wrong preamble 0x%x should be 0x%x\n", cmd_id, rsp_hdr->preamble, MSG_HDR_RSP);
        return KDP2_ERROR_OTHER;
    }

    if ((rsp_hdr->cmd & 0x7FFF) != cmd_id)
    {
        dbg_print("receive command (0x%X) message header failed, wrong cmd_id 0x%x\n", cmd_id, MSG_HDR_RSP);
        return KDP2_ERROR_OTHER;
    }

    return KDP2_SUCCESS;
}

int kdp2_scan_devices(kdp2_devices_list_t **list)
{
    // FIXME: return code
    if (KDP2_LL_RET_OK == kdp2_ll_scan_devices(list))
        return KDP2_SUCCESS;
    else
        return KDP2_ERROR_OTHER;
}

kdp2_device_t kdp2_connect_device(int scan_index)
{
    _kdp2_device_t *_device = NULL;
    kdp2_ll_device_t *ll_dev = kdp2_ll_connect_device(scan_index);
    if (ll_dev)
    {
        _device = (_kdp2_device_t *)malloc(sizeof(_kdp2_device_t));
        _device->ll_device = ll_dev;
        _device->timeout = 0;
    }

    return (kdp2_device_t)_device;
}

kdp2_device_t kdp2_connect_device_by_kn_number(uint32_t kn_num)
{
    if (kn_num == 0x0)
        return NULL;

    kdp2_devices_list_t *list;
    kdp2_ll_scan_devices(&list);

    int scan_index = 0;
    for (int i = 0; i < list->num_dev; i++)
    {
        if (list->device[i].kn_number == kn_num && list->device[i].isConnectable == true)
        {
            scan_index = list->device[i].scan_index;
            break;
        }
    }
    free(list);

    _kdp2_device_t *_device = NULL;

    if (scan_index > 0)
    {
        kdp2_ll_device_t *ll_dev = kdp2_ll_connect_device(scan_index);
        if (ll_dev)
        {
            _device = (_kdp2_device_t *)malloc(sizeof(_kdp2_device_t));
            _device->ll_device = ll_dev;
            _device->timeout = 0;
        }
    }

    return (kdp2_device_t)_device;
}

kdp2_device_t kdp2_connect_device_by_product(kdp2_product_id_t prod_id)
{
    kdp2_devices_list_t *list;
    kdp2_ll_scan_devices(&list);

    int scan_index = 0;
    for (int i = 0; i < list->num_dev; i++)
    {
        if (list->device[i].product_id == prod_id && list->device[i].isConnectable == true)
        {
            scan_index = list->device[i].scan_index;
            break;
        }
    }
    free(list);

    _kdp2_device_t *_device = NULL;

    if (scan_index > 0)
    {
        kdp2_ll_device_t *ll_dev = kdp2_ll_connect_device(scan_index);
        if (ll_dev)
        {
            _device = (_kdp2_device_t *)malloc(sizeof(_kdp2_device_t));
            _device->ll_device = ll_dev;
            _device->timeout = 0;
        }
    }

    return (kdp2_device_t)_device;
}

kdp2_device_t kdp2_connect_device_by_port_path(const char *port_path)
{
    kdp2_devices_list_t *list;
    kdp2_ll_scan_devices(&list);

    int scan_index = 0;
    for (int i = 0; i < list->num_dev; i++)
    {
        if (strcmp(list->device[i].port_path, port_path) == 0 && list->device[i].isConnectable == true)
        {
            scan_index = list->device[i].scan_index;
            break;
        }
    }
    free(list);

    _kdp2_device_t *_device = NULL;

    if (scan_index > 0)
    {
        kdp2_ll_device_t *ll_dev = kdp2_ll_connect_device(scan_index);
        if (ll_dev)
        {
            _device = (_kdp2_device_t *)malloc(sizeof(_kdp2_device_t));
            _device->ll_device = ll_dev;
            _device->timeout = 0;
        }
    }

    return (kdp2_device_t)_device;
}

int kdp2_disconnect_device(kdp2_device_t device)
{
    _kdp2_device_t *_device = (_kdp2_device_t *)device;

    int ret = kdp2_ll_disconnect_device(_device->ll_device);
    free(_device);

    return ret;
}

kdp2_device_descriptor_t *kdp2_get_device_descriptor(kdp2_device_t device)
{
    _kdp2_device_t *_device = (_kdp2_device_t *)device;
    return kdp2_ll_get_device_descriptor(_device->ll_device);
}

void kdp2_set_timeout(kdp2_device_t device, int milliseconds)
{
    _kdp2_device_t *_device = (_kdp2_device_t *)device;
    _device->timeout = milliseconds;
}

// this is from ipc.h/kdp_model_s to parse fw_info.bin
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
} _fw_info_t;

// this is from firmware kdpio.h to parse setup.bin
typedef struct
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
} _cnn_header_t;

int kdp2_load_model(kdp2_device_t device, void *nef_buf, int nef_size, kdp2_all_models_descriptor_t *model_desc)
{
    _kdp2_device_t *_device = (_kdp2_device_t *)device;
    kdp2_ll_device_t *ll_dev = _device->ll_device;
    int timeout = _device->timeout;

    struct kdp2_metadata_s metadata;
    memset(&metadata, 0, sizeof(struct kdp2_metadata_s));
    struct kdp2_nef_info_s nef_info;
    memset(&nef_info, 0, sizeof(struct kdp2_nef_info_s));

    int ret = read_nef(nef_buf, nef_size, &metadata, &nef_info);

    if (ret != 0)
    {
        dbg_print("getting model data failed: %d...\n", ret);
        return -1;
    }

    int fw_info_size = nef_info.fw_info_size;

    if (!fw_info_size)
    {
        dbg_print("Not supported target %d!\n", metadata.target);
        return -1;
    }

    // split model_data to fw_info and all_models
    uint32_t model_size = 0;
    char fw_info[8192]; //8KB
    char *model_buf = NULL;
    int n_len;

    // get fw_info data
    memset(fw_info, 0, sizeof(fw_info));
    memcpy(fw_info, nef_info.fw_info_addr, fw_info_size);

    // get all_models data
    n_len = nef_info.all_models_size;

    if (n_len <= 0)
    {
        dbg_print("getting all_models fw_info failed:%d...\n", n_len);
        return -1;
    }

    model_buf = (char *)malloc(n_len);
    memset(model_buf, 0, n_len);
    memcpy(model_buf, nef_info.all_models_addr, n_len);

    if (model_desc)
    {
        uint32_t m_offset = 0;
        uint32_t num_models = *(uint32_t *)fw_info;
        model_desc->num_models = num_models;
        for (uint32_t i = 0; i < num_models; i++)
        {
            _fw_info_t *fwstruct = (_fw_info_t *)(fw_info + 4 + i * sizeof(_fw_info_t));
            uint32_t setup_offset = fwstruct->setup_mem_addr - fwstruct->cmd_mem_addr;
            _cnn_header_t *cnnh = (_cnn_header_t *)(model_buf + m_offset + setup_offset);

            model_desc->models[i].id = fwstruct->model_type;
            model_desc->models[i].max_raw_out_size = sizeof(khost_ipc_inf_user_recv_t) + 4 + MAX_RAW_OUTPUT_NODE * sizeof(struct _fw_output_node_descriptor) + fwstruct->output_mem_len + MAX_RAW_OUTPUT_NODE * sizeof(kdp2_node_output_t) + 4 * fwstruct->output_mem_len;
            model_desc->models[i].width = cnnh->input_col;
            model_desc->models[i].height = cnnh->input_row;
            model_desc->models[i].channel = cnnh->input_channel;
            model_desc->models[i].img_format = KDP2_IMAGE_FORMAT_RGBA8888; // FIXME, to let user know HW's raw image format

            m_offset = m_offset + setup_offset + fwstruct->setup_mem_len;
        }
    }

    model_size = n_len;

    uint32_t transfer_size = sizeof(start_dme_header) + fw_info_size;
    uint8_t *cmd_buffer = NULL;
    uint8_t *recv_buffer = NULL;

    ret = KDP2_ERROR_OTHER;

    do
    {
        cmd_buffer = (uint8_t *)malloc(transfer_size);

        start_dme_header *dme_message = (start_dme_header *)cmd_buffer;
        dme_message->msg_header.preamble = MSG_HDR_CMD;
        dme_message->msg_header.ctrl = 0;
        dme_message->msg_header.cmd = CMD_DME_START;
        dme_message->msg_header.msg_len = 4 + fw_info_size;
        dme_message->model_size = model_size;
        memcpy(dme_message->fw_info, fw_info, fw_info_size);

        if (KDP2_SUCCESS != (ret = _send_command_header(ll_dev, CMD_DME_START, cmd_buffer, transfer_size, timeout)))
            break;

        if (KDP2_SUCCESS != (ret = _receive_command_ack(ll_dev, CMD_DME_START, timeout)))
            break;

        if (KDP2_SUCCESS != (ret = _send_command_data(ll_dev, CMD_DME_START, model_buf, model_size, timeout)))
            break;

        int recv_buffer_size = sizeof(MsgHdr) + sizeof(RspPram);
        recv_buffer = (uint8_t *)malloc(recv_buffer_size);

        if (KDP2_SUCCESS != (ret = _recevie_command_response(ll_dev, CMD_DME_START, recv_buffer, recv_buffer_size, timeout)))
            break;

        RspPram *rsp_parameters = (RspPram *)(recv_buffer + sizeof(MsgHdr));

        if (rsp_parameters->error != 0)
            break;

        if (rsp_parameters->param2 != model_size)
            break;

        ret = KDP2_SUCCESS; // all done, successfull

    } while (0);

    if (cmd_buffer)
        free(cmd_buffer);

    if (recv_buffer)
        free(recv_buffer);

    if (model_buf)
        free(model_buf);

    return ret;
}

static char *read_file_to_buffer_auto_malloc(const char *file_path, long *buffer_size)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        printf("%s(): fopen failed, file:%s, %s\n", __FUNCTION__, file_path, strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file); //get the size

    *buffer_size = file_size;

    char *buffer = (char *)malloc(file_size);

    fseek(file, 0, SEEK_SET); //move to begining

    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != (size_t)file_size)
    {
        printf("%s(): fread failed, file size: %u, read size %u\n", __FUNCTION__,
               (unsigned int)file_size, (unsigned int)read_size);
        free(buffer);
        buffer = NULL;
        *buffer_size = 0;
    }

    fclose(file);

    return buffer;
}

int kdp2_load_model_from_file(kdp2_device_t device, const char *file_path, kdp2_all_models_descriptor_t *model_desc)
{
    long nef_size;
    char *nef_buf = read_file_to_buffer_auto_malloc(file_path, &nef_size);
    if (!nef_buf)
        return -1;

    int ret = kdp2_load_model(device, (void *)nef_buf, (int)nef_size, model_desc);

    free(nef_buf);

    return ret;
}

int kdp2_update_firmware(kdp2_device_t device, kdp2_firmware_id_t fw_id, void *buffer, int size)
{
    _kdp2_device_t *_device = (_kdp2_device_t *)device;
    kdp2_ll_device_t *ll_dev = _device->ll_device;
    int timeout = _device->timeout;

    int ret = KDP2_ERROR_OTHER;

    do
    {
        firmware_update_header cmd_header;
        firmware_update_response cmd_resp;

        cmd_header.msg_header.preamble = MSG_HDR_CMD;
        cmd_header.msg_header.ctrl = 0;
        cmd_header.msg_header.cmd = CMD_UPDATE_FW;
        cmd_header.msg_header.msg_len = sizeof(firmware_update_header) - sizeof(MsgHdr);
        cmd_header.fw_id = fw_id;

        if (KDP2_SUCCESS != (ret = _send_command_header(ll_dev, CMD_UPDATE_FW, &cmd_header, sizeof(cmd_header), timeout)))
            break;

        if (KDP2_SUCCESS != (ret = _receive_command_ack(ll_dev, CMD_UPDATE_FW, timeout)))
            break;

        if (KDP2_SUCCESS != (ret = _send_command_data(ll_dev, CMD_UPDATE_FW, buffer, size, timeout)))
            break;

        if (KDP2_SUCCESS != (ret = _recevie_command_response(ll_dev, CMD_UPDATE_FW, (uint8_t *)&cmd_resp, sizeof(cmd_resp), timeout)))
            break;

        if (cmd_resp.rsp_code != 0 || cmd_resp.fw_id != fw_id)
        {
            ret = KDP2_ERROR_DEV_OP_FAIL;
            break;
        }

        ret = KDP2_SUCCESS; // all done, successfull

    } while (0);

    return ret;
}

int kdp2_update_firmware_from_file(kdp2_device_t device, kdp2_firmware_id_t fw_id, const char *file_path)
{
    long buf_size;
    char *buf = read_file_to_buffer_auto_malloc(file_path, &buf_size);
    if (!buf)
        return -1;

    int ret = kdp2_update_firmware(device, fw_id, (void *)buf, (int)buf_size);

    free(buf);

    return ret;
}

int kdp2_raw_inference_send(kdp2_device_t device, kdp2_raw_input_descriptor_t *inf_desc,
                            uint8_t *image_buffer)
{
    uint32_t image_size = 0;

    kdp2_ll_device_t *ll_dev = ((_kdp2_device_t *)device)->ll_device;
    int timeout = ((_kdp2_device_t *)device)->timeout;

    khost_ipc_inf_user_send_t ll_inf_desc;

    //uint32_t config = IMAGE_FORMAT_PARALLEL_PROC | IMAGE_FORMAT_RAW_OUTPUT;
    uint32_t config = IMAGE_FORMAT_RAW_OUTPUT; // for RAW mode, this is faster ! why ???

    switch (inf_desc->normalize_mode)
    {
    case KDP2_NORMALIE_NONE:
    case KDP2_NORMALIE_0_TO_1:
        break;
    case KDP2_NORMALIE_NEGATIVE_05_TO_05:
        config |= IMAGE_FORMAT_SUB128;
        break;
    case KDP2_NORMALIE_NEGATIVE_1_TO_1:
        config |= (IMAGE_FORMAT_SUB128 | IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT);
        break;
    }

    switch (inf_desc->image_format)
    {
    case KDP2_IMAGE_FORMAT_RGB565:
        config |= NPU_FORMAT_RGB565;
        image_size = inf_desc->width * inf_desc->height * 2;
        break;
    case KDP2_IMAGE_FORMAT_RGBA8888:
        image_size = inf_desc->width * inf_desc->height * 4;
        break;
    default:
    case KDP2_IMAGE_FORMAT_UNKNOWN:
        return KDP2_ERROR_INVALID_PARAM;
    }

    ll_inf_desc.total_size = sizeof(ll_inf_desc) + image_size;
    ll_inf_desc.desc_type = KHOST_INF_IMAGE_USER;
    ll_inf_desc.request_id = inf_desc->inference_number;
    ll_inf_desc.image.width = inf_desc->width;
    ll_inf_desc.image.height = inf_desc->height;
    ll_inf_desc.image.channel = 3; // FIXME

    if (image_size > IMAGE_BUFFER_SIZE)
    {
        dbg_print("[%s] image buffer size is not enough in firmware\n", __func__);
        return KDP2_ERROR_SEND_DATA_TOO_LARGE;
    }

    ll_inf_desc.image.config = config;
    ll_inf_desc.model.use_builtin_model = false; // FIXME when in flash
    ll_inf_desc.model.model_count = 1;
    ll_inf_desc.model.model_ids[0] = inf_desc->model_id;

    int ret = 0;
    ret = kdp2_ll_write_data(ll_dev, (void *)&ll_inf_desc, sizeof(ll_inf_desc), timeout);
    if (ret == KDP2_LL_RET_TIMEOUT)
    {
        dbg_print("[%s] Write descriptor to FIFO queue timeout [%d ms]\n", __func__, timeout);
        return KDP2_ERROR_TIMEOUT;
    }
    if (ret != KDP2_LL_RET_OK)
    {
        dbg_print("[%s] Write descriptor to FIFO queue error. kdp2_ll_write_data() return code = [%d]\n", __func__, ret);
        return KDP2_ERROR_SEND_DESC_FAIL;
    }

    ret = kdp2_ll_write_data(ll_dev, (void *)image_buffer, image_size, timeout);
    if (ret == KDP2_LL_RET_TIMEOUT)
    {
        dbg_print("[%s] Write data to FIFO queue timeout [%d ms]\n", __func__, timeout);
        return KDP2_ERROR_TIMEOUT;
    }
    if (ret != KDP2_LL_RET_OK)
    {
        dbg_print("[%s] Write data to FIFO queue error. kdp2_ll_write_data() return code = [%d]\n", __func__, ret);
        return KDP2_ERROR_SEND_DATA_FAIL;
    }

    return KDP2_SUCCESS;
}

int kdp2_raw_inference_receive(kdp2_device_t device, kdp2_raw_output_descriptor_t *output_desc,
                               uint8_t *raw_out_buffer, uint32_t buf_size)
{

    kdp2_ll_device_t *ll_dev = ((_kdp2_device_t *)device)->ll_device;
    int timeout = ((_kdp2_device_t *)device)->timeout;

    int ret = kdp2_ll_read_data(ll_dev, (void *)raw_out_buffer, buf_size, timeout);
    if (ret < 0)
    {
        printf("%s() kdp2_ll_read_data() return %d\n", __FUNCTION__, ret);
        return ret;
    }

    khost_ipc_inf_user_recv_t *ipc_desc = (khost_ipc_inf_user_recv_t *)raw_out_buffer;

    if (ipc_desc->return_code != KHOST_IPC_RETURN_SUCCESS)
        return ipc_desc->return_code;

    output_desc->inference_number = ipc_desc->request_id;
    output_desc->num_output_node = *(uint32_t *)(raw_out_buffer + sizeof(khost_ipc_inf_user_recv_t));

    return KDP2_SUCCESS;
}

#define KDP_COL_MIN 16
uint32_t round_up(uint32_t num)
{
    return ((num + (KDP_COL_MIN - 1)) & ~(KDP_COL_MIN - 1));
}

kdp2_node_output_t *kdp2_raw_inference_retrieve_node(uint32_t node_idx, uint8_t *raw_out_buffer)
{
    kdp2_node_output_t *node_output = NULL;
    uint8_t *data_start = raw_out_buffer + sizeof(khost_ipc_inf_user_recv_t);
    uint32_t out_node_num = *(uint32_t *)data_start;

    if (node_idx > out_node_num - 1)
        return NULL;

    struct _fw_output_node_descriptor *node_desc = (struct _fw_output_node_descriptor *)(data_start + 4);

    uint32_t raw_offset = 4 + out_node_num * sizeof(struct _fw_output_node_descriptor);
    uint32_t raw_total_offset = raw_offset;

    for (int i = 0; i < node_idx; i++)
        raw_offset += node_desc[i].height * node_desc[i].channel * round_up(node_desc[i].width);

    for (int i = 0; i < out_node_num; i++)
        raw_total_offset += node_desc[i].height * node_desc[i].channel * round_up(node_desc[i].width);

    int8_t *real_data = (int8_t *)(data_start + raw_offset);
    int data_size = node_desc[node_idx].height * node_desc[node_idx].channel * node_desc[node_idx].width; // FIXME width

    {
        uint32_t real_offset = raw_total_offset;

        for (int i = 0; i < node_idx; i++)
            real_offset += sizeof(kdp2_node_output_t) + node_desc[i].height * node_desc[i].channel * node_desc[i].width * sizeof(float);

        node_output = (kdp2_node_output_t *)(data_start + real_offset);
        node_output->channel = node_desc[node_idx].channel;
        node_output->height = node_desc[node_idx].height;
        node_output->width = node_desc[node_idx].width;
        node_output->num_data = data_size;

        float scale = node_desc[node_idx].scale;
        uint32_t radix = node_desc[node_idx].radix;

        float ffactor = (float)1 / (float)(scale * (0x1 << radix));

        int width_aligned = round_up(node_output->width);
        int n = 0;
        for (int i = 0; i < node_output->height * node_output->channel; i++)
        {
            for (int j = 0; j < node_output->width; j++)
                node_output->data[n++] = (float)real_data[i * width_aligned + j] * ffactor;
        }
    }

    return node_output;
}

#if 0
int kdp2_validate_version(kdp2_device_t device)
{
    if (!device)
    {
        dbg_print("[%s] No Kneron device\n", __func__);
        return KDP2_ERROR_NO_DEV;
    }

    _kdp2_device_t *_device = (_kdp2_device_t *)device;
    kdp2_ll_device_t *ll_dev = _device->ll_device;
    int timeout = _device->timeout;

    int ret = 0;
    khost_ipc_sys_version_t send;
    khost_ipc_sys_version_t recv;

    memset(&send, 0, sizeof(send));
    send.desc_type = KHOST_SYS_VERSION;
    send.total_size = sizeof(send);
    send.version = KHOST_IPC_VERSION;

    dbg_print("[%s] Starting to check khost IPC version...\n", __func__);
    ret = kdp2_ll_write_data(ll_dev, (void *)&send, sizeof(send), 0);
    if (ret == KDP2_LL_RET_TIMEOUT)
    {
        dbg_print("[%s] Write descriptor to FIFO queue timeout [%d ms]\n", __func__, 0);
        return KDP2_ERROR_TIMEOUT;
    }
    if (ret != KDP2_LL_RET_OK)
    {
        dbg_print("[%s] Write descriptor to FIFO queue error. kdp2_ll_write_data() return code = [%d]\n", __func__, ret);
        return KDP2_ERROR_SEND_DATA_FAIL;
    }

    memset(&recv, 0, sizeof(recv));
    ret = kdp2_ll_read_data(ll_dev, (void *)&recv, sizeof(recv), 0);
    if (ret == KDP2_LL_RET_TIMEOUT)
    {
        dbg_print("[%s] Read data from FIFO queue timeout [%d ms]\n", __func__, 0);
        return KDP2_ERROR_TIMEOUT;
    }
    if (ret < KDP2_LL_RET_OK)
    {
        dbg_print("[%s] Read data from FIFO queue error. kdp2_ll_read_data() return code = [%d]\n", __func__, ret);
        return KDP2_ERROR_RECV_DATA_FAIL;
    }

    if (recv.desc_type != KHOST_SYS_VERSION)
    {
        dbg_print("[%s] Recv type should be [version], but type = [%d]\n", __func__, recv.desc_type);
        return KDP2_ERROR_RECV_DESC_FAIL;
    }

    if (recv.return_code != KHOST_IPC_RETURN_SUCCESS)
    {
        dbg_print("[%s] Version validation error on FW. Error code = [%d]\n", __func__, recv.return_code);
        return KDP2_ERROR_RECV_DESC_FAIL;
    }

    if (recv.version != KHOST_IPC_VERSION)
    {
        dbg_print("[%s] Version doesn't match. Recv version = [%d]. Expected version = [%d]\n", __func__, recv.version, KHOST_IPC_VERSION);
        return KDP2_ERROR_INVALID_VERSION;
    }

    return KDP2_SUCCESS;
}
#endif

int kdp2_reset_device(kdp2_device_t device, kdp2_reset_mode_t reset_mode)
{

    if (!device)
    {
        dbg_print("[%s] No Kneron device\n", __func__);
        return KDP2_ERROR_NO_DEV;
    }

    _kdp2_device_t *_device = (_kdp2_device_t *)device;
    kdp2_ll_device_t *ll_dev = _device->ll_device;
    int timeout = _device->timeout;

    int ret = 0;
    kdp2_ll_control_t kctrl;
    memset(&kctrl, 0, sizeof(kctrl));

    if (reset_mode == KDP2_HARD_RESET)
    {
        // For hardware reset, need to send 2 control commands.

        // 1. Make sure if the device is connected or not
        kctrl.command = KDP2_LL_CTRL_HARD_RESET;
        kctrl.arg1 = 0;
        kctrl.arg2 = 0;

        ret = kdp2_ll_control(ll_dev, &kctrl, timeout);
        if (ret != KDP2_LL_RET_OK)
        {
            dbg_print("[%s] Send usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
            return KDP2_ERROR_DEV_OP_FAIL;
        }

        // 2. Really reset the device. The device would be disconnected.
        kctrl.arg1 = 1;
        kdp2_ll_control(ll_dev, &kctrl, timeout);

        // 3. Wait some time to restart hardware. 5 seconds is an experimental time. We could modify this value if something wrong.
        sleep(5);
    }
    else if (reset_mode == KDP2_INFERENCE_RESET)
    {
        // Need to do reset twice due to producer and consumer issues

        kctrl.command = KDP2_LL_CTRL_INFERENCE_RESET;
        kctrl.arg1 = 0;
        kctrl.arg2 = 0;

        int ret = kdp2_ll_control(ll_dev, &kctrl, timeout);
        if (ret != KDP2_LL_RET_OK)
        {
            dbg_print("[%s] Send usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
            return KDP2_ERROR_DEV_OP_FAIL;
        }

        // Give firmware some time to react. 0.1 second is an experimental time. We could modify this value if something wrong.
        usleep(100000);

        ret = kdp2_ll_control(ll_dev, &kctrl, timeout);
        if (ret != KDP2_LL_RET_OK)
        {
            dbg_print("[%s] Send usb control endpoint failed. libusb_control_transfer() return code = [%d]\n", __func__, ret);
            return KDP2_ERROR_DEV_OP_FAIL;
        }
    }
    else
    {
        dbg_print("[%s] Not support this reset mode [%d]\n", __func__, reset_mode);
        return KDP2_ERROR_NOT_SUPPORTED;
    }

    return KDP2_SUCCESS;
}

#if 0
static const char *kdp2_ret_code_name(int ret_code)
{

    switch (ret_code)
    {
    case KDP2_SUCCESS:
        return "Success.";
    case KDP2_ERROR_NO_DEV:
        return "No Kneron device as API parameter.";
    case KDP2_ERROR_NO_DEV_ID:
        return "Cannot get device ID from Kneron device. The device ID is used for kdp2_xxx API.";
    case KDP2_ERROR_DEV_NOT_CONNECTED:
        return "The device isn't connected.";
    case KDP2_ERROR_DEV_OP_FAIL:
        return "Operation failed on the device.";
    case KDP2_ERROR_TIMEOUT:
        return "Operation timed out.";
    case KDP2_ERROR_INVALID_PARAM:
        return "Invalid parameters for API.";
    case KDP2_ERROR_INVALID_VERSION:
        return "The versions on host and FW sides are not the same.";
    case KDP2_ERROR_SEND_DESC_FAIL:
        return "Send descriptor failed.";
    case KDP2_ERROR_SEND_DATA_FAIL:
        return "Send data failed.";
    case KDP2_ERROR_SEND_DATA_TOO_LARGE:
        return "Sending data is too large.";
    case KDP2_ERROR_SEND_MODEL_FAIL:
        return "Send models to Kneron device failed.";
    case KDP2_ERROR_RECV_DESC_FAIL:
        return "Receive descriptor failed. The descriptor format is incorrect.";
    case KDP2_ERROR_RECV_DATA_FAIL:
        return "Receive data failed.";
    case KDP2_ERROR_RECV_DATA_TOO_LARGE:
        return "Receiving data is too large.";
    case KDP2_ERROR_NOT_SUPPORTED:
        return "Operation not supported or not implemented.";
    case KDP2_ERROR_OTHER:
        return "Other error.";
    default:
        return "Unknown return code.";
    }
}
#endif
