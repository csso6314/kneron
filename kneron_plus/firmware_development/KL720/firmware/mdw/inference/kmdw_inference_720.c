/*
 * Kneron Application general functions
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

// #define DEBUG_PRINT
#define ENABLE_INFERENCE_TIMEOUT_WATCH

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kdrv_gdma3.h"
#include "kmdw_ipc.h"
#include "kmdw_memory.h"
#include "kmdw_model.h"
#include "kmdw_console.h"
#include "membase.h"

#include "kp_struct.h"
#include "kmdw_inference_app.h"
#include "kmdw_fifoq_manager.h"
#include "kdp2_inf_generic_raw.h" /*private fucntions for kmdw_inference*/
#include "flatbuffer_setup_reader.h"

#ifdef DEBUG_PRINT
#include "kmdw_console.h"
#define dbg_print(__format__, ...) kmdw_level_printf(LOG_CUSTOM, "[inf]"__format__, ##__VA_ARGS__)
#else
#define dbg_print(__format__, ...)
#endif

#define INF_TIMEOUT     2000                // twice
#define INPROC_MAX_OUTPUT_HEIGHT    2047    // inproc dst resized limitation: 2047 (2^11)
#define INPROC_MAX_OUTPUT_WIDTH     1023    // inproc dst resized limitation: 1023 (2^10)
#define IMG_AVAILABLE_WIDTH_FACTOR  2

static osEventFlagsId_t g_result_event;

static kmdw_inference_app_callback_t _app_entry_func = NULL;

static volatile int g_inf_index = 0;
static volatile uint32_t g_num_parallel_inf = 0;
static volatile uint32_t g_num_parallel_result = 0;

typedef struct
{
    void *inf_result_buf;
    int inf_result_buf_size;
    void *ncpu_result_buf;
    kmdw_inference_app_result_callback_t result_callback_func;
} result_context_t;

// CNN_Header and NetInput_Node are from kneron_api_data.h in ncpu firmware to parse setup.bin
typedef struct CNN_Header {
    uint32_t crc;
    uint32_t version;
    uint32_t reamaining_models;
    uint32_t model_type;
    uint32_t application_type;
    uint32_t dram_start;
    uint32_t dram_size;
    uint32_t cmd_start;
    uint32_t cmd_size;
    uint32_t weight_start;
    uint32_t weight_size;
    uint32_t input_start;
    uint32_t input_size;
    uint32_t input_num;
    uint32_t output_num;
} CNN_Header;

// node_id = 5
typedef struct NetInput_Node {
    uint32_t node_id;
    uint32_t input_index;
    uint32_t input_format;
    uint32_t input_row;
    uint32_t input_col;
    uint32_t input_channel;
    uint32_t input_start;
    uint32_t input_size;
    uint32_t input_radix;
} NetInput_Node;

#define MAX_OUTPUT_CONTEXT_NUM 6 // smaller than IPC_IMAGE_MAX
static result_context_t g_result_ctx[MAX_OUTPUT_CONTEXT_NUM] = {0};

extern void kdp2_generic_raw_inference(int num_input_buf, void **inf_input_buf_list);
extern void kdp2_generic_raw_inference_bypass_pre_proc(int num_input_buf, void **inf_input_buf_list);

void kmdw_inference_image_dispatcher_thread(void *argument)
{
    /**
     * FIXME: [#15503] Need to review the IPC,and decoupling usage of IPC and model.
     * Currently, move kmdw_model_init() in mdw_initialize() for correct IPC initialization flow.
     */
    // /* init section */
    // kmdw_model_init();

    while (1)
    {
        buffer_object_t fifoq_obj; // fifoq buffer object

        // blocking-wait image buffer object
        osStatus_t sts = kmdw_fifoq_manager_image_dequeue(&fifoq_obj, osWaitForever);

        if (0 < fifoq_obj.num_of_buffer) {
            void *inf_input_buf = (void *)fifoq_obj.buffer_addr[0]; // contains header + image

            dbg_print("got a image buffer for inference: buf 0x%x\n", inf_input_buf);

            kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)inf_input_buf;

            if (header_stamp->job_id == KDP2_INF_ID_GENERIC_RAW)
                kdp2_generic_raw_inference(fifoq_obj.num_of_buffer, (void **)fifoq_obj.buffer_addr);
            else if (header_stamp->job_id == KDP2_INF_ID_GENERIC_RAW_BYPASS_PRE_PROC)
                kdp2_generic_raw_inference_bypass_pre_proc(fifoq_obj.num_of_buffer, (void **)fifoq_obj.buffer_addr);
            else
                _app_entry_func(fifoq_obj.num_of_buffer, (void **)fifoq_obj.buffer_addr);
        }

        // return buffer back to fifoq
        for (int i = 0; i < fifoq_obj.num_of_buffer; i++) {
            kmdw_fifoq_manager_image_put_free_buffer(fifoq_obj.buffer_addr[i], fifoq_obj.length[i], osWaitForever);
        }
    }
}

void kmdw_inference_result_handler_callback_thread(void *argument)
{
    uint32_t result_index = 0; // next result sequence index
    uint32_t wait_result_flag;
    int timeout_count = 0;

    while (1)
    {
        wait_result_flag = (0x1 << result_index);

        uint32_t wait_timeout = (kmdw_ipc_get_output()->kp_dbg_checkpoinots == 0x0) ? INF_TIMEOUT : osWaitForever;
        uint32_t flags = osEventFlagsWait(g_result_event, wait_result_flag, osFlagsWaitAny, wait_timeout);

        dbg_print("result_get: osEventFlagsWait() return 0x%x\n", flags);

        // a tricky way to check if inf timeout
        if (flags == (uint32_t)osFlagsErrorTimeout)
        {
            if (g_num_parallel_inf > g_num_parallel_result)
                timeout_count++;

            if (timeout_count >= 2)
            {
                kmdw_printf("[inf] parallel inference timeout\n");
                kmdw_printf("inf req %d done %d timeout %d secs\n", g_num_parallel_inf, g_num_parallel_result, timeout_count);

                void *inf_result_buf = g_result_ctx[result_index].inf_result_buf;
                int inf_result_buf_size = g_result_ctx[result_index].inf_result_buf_size;
                void *ncpu_result_buf = g_result_ctx[result_index].ncpu_result_buf;
                g_result_ctx[result_index].result_callback_func(KP_FW_INFERENCE_TIMEOUT_103, inf_result_buf, inf_result_buf_size, ncpu_result_buf);

                return; // FIXME, game over ?
            }
        }
        else if (flags == wait_result_flag)
        {
            void *inf_result_buf = g_result_ctx[result_index].inf_result_buf;
            int inf_result_buf_size = g_result_ctx[result_index].inf_result_buf_size;
            void *ncpu_result_buf = g_result_ctx[result_index].ncpu_result_buf;
            g_result_ctx[result_index].result_callback_func(KP_SUCCESS, inf_result_buf, inf_result_buf_size, ncpu_result_buf);

            g_num_parallel_result++;

            if (++result_index >= MAX_OUTPUT_CONTEXT_NUM)
                result_index = 0;

            timeout_count = 0;
        }
        else
        {
            // should not be here
            kmdw_printf("[inf] error flag 0x%x\n", flags);
        }
    }
}

/* ############################
 * ##    public functions    ##
 * ############################ */

int kmdw_inference_app_execute(kmdw_inference_app_config_t *inf_config)
{
    dbg_print("run image inference:\n");

    struct kdp_img_cfg ncpu_img_config;

    // Check if image width, height > model width, height
    if (0 == kmdw_model_is_model_loaded(inf_config->model_id))
        return KP_ERROR_MODEL_NOT_LOADED_35;

    int num_input_node = kmdw_model_get_input_tensor_num(inf_config->model_id);
    if (num_input_node != inf_config->num_image)
        return KP_FW_WRONG_INPUT_BUFFER_COUNT_110;

    ncpu_img_config.num_image = inf_config->num_image;
    ncpu_img_config.image_buf_active_index = g_inf_index;
    ncpu_img_config.inf_format = 0;

    // if no post-processing
    if (inf_config->enable_raw_output)
        ncpu_img_config.inf_format |= IMAGE_FORMAT_RAW_OUTPUT;

    // enable parallel post-processing
    if (inf_config->enable_parallel)
        ncpu_img_config.inf_format |= IMAGE_FORMAT_PARALLEL_PROC;

    for (int i = 0; i < num_input_node; i++) {
        // find network input node, we observed that it's put right after CNN_Header stored in setup_mem_addr in many models
        // FIXME: network input node might not be the first node put right after CNN_Header stored in setup_mem_addr
        //        see fw_model_parse() and fw_get_netinput_node() in fw_model_parse.h/.c for how to retrieve network input node from setup_mem_addr
        uint32_t model_width = 0;
        uint32_t model_height = 0;
        uint32_t input_index = 0;

        /* shape order [batch, channel, hieght, width] */
        kmdw_model_tensor_descriptor_t tensor_info = {0};
        if (0 == kmdw_model_get_input_tensor_info(inf_config->model_id, i, &tensor_info))
            return KP_ERROR_INVALID_MODEL_21;

        model_width = tensor_info.shape_npu[3];
        model_height = tensor_info.shape_npu[2];
        input_index = i;

        if ((0 == model_width) && (0 == model_height))
            return KP_ERROR_MODEL_NOT_LOADED_35;

        ncpu_img_config.image_list[input_index].image_mem_addr = (uint32_t)inf_config->image_list[input_index].image_buf;
        ncpu_img_config.image_list[input_index].image_mem_len = inf_config->image_list[input_index].image_buf_size; // FIXME ??
        ncpu_img_config.image_list[input_index].input_col = inf_config->image_list[input_index].image_width;
        ncpu_img_config.image_list[input_index].input_row = inf_config->image_list[input_index].image_height;
        ncpu_img_config.image_list[input_index].input_channel = inf_config->image_list[input_index].image_channel;
        ncpu_img_config.image_list[input_index].format = 0; // sycn with 'ncpu_config'

        dbg_msg("[%d] image_mem_addr = 0x%x\n", input_index, ncpu_img_config.image_list[input_index].image_mem_addr);
        dbg_msg("[%d] image_col = %d\n", input_index, ncpu_img_config.image_list[input_index].input_col);
        dbg_msg("[%d] image_row = %d\n", input_index, ncpu_img_config.image_list[input_index].input_row);
        dbg_msg("[%d] image_ch = %d\n", input_index, ncpu_img_config.image_list[input_index].input_channel);

        if (false == inf_config->image_list[input_index].bypass_pre_proc) // enable pre-processing
        {
            switch (inf_config->image_list[input_index].image_format) {
            case KP_IMAGE_FORMAT_RGB565:
            case KP_IMAGE_FORMAT_RGBA8888:
            case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
            case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
            case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
            case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
            case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
            case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
            case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
            case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
            case KP_IMAGE_FORMAT_RAW8:
                ncpu_img_config.image_list[input_index].format |= inf_config->image_list[input_index].image_format;
                break;
            case KP_IMAGE_FORMAT_YUYV:
                ncpu_img_config.image_list[input_index].format |= NPU_FORMAT_YCBCR422_Y0CBY1CR;
                break;
            default:
                break;
            }

            if (0 != ncpu_img_config.image_list[input_index].input_col % IMG_AVAILABLE_WIDTH_FACTOR)
                return KP_ERROR_IMAGE_INVALID_WIDTH_23;

            switch (inf_config->image_list[input_index].image_norm)
            {
            case KP_NORMALIZE_DISABLE:
            case KP_NORMALIZE_CUSTOMIZED_DEFAULT:
                break;
            case KP_NORMALIZE_YOLO:
            case KP_NORMALIZE_CUSTOMIZED_DIV2:
                ncpu_img_config.image_list[input_index].format |= IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT;
                break;
            case KP_NORMALIZE_KNERON:
            case KP_NORMALIZE_TENSOR_FLOW:
            case KP_NORMALIZE_CUSTOMIZED_SUB128:
                ncpu_img_config.image_list[input_index].format |= (uint32_t)IMAGE_FORMAT_SUB128;
                break;
            case KP_NORMALIZE_CUSTOMIZED_SUB128_DIV2:
                ncpu_img_config.image_list[input_index].format |= ((uint32_t)IMAGE_FORMAT_SUB128 | IMAGE_FORMAT_RIGHT_SHIFT_ONE_BIT);
                break;
            }

            if (inf_config->image_list[input_index].enable_crop)
            {
                uint32_t crop_x1 = inf_config->image_list[input_index].crop_area.x1;
                uint32_t crop_y1 = inf_config->image_list[input_index].crop_area.y1;
                uint32_t crop_width = inf_config->image_list[input_index].crop_area.width;
                uint32_t crop_height = inf_config->image_list[input_index].crop_area.height;
                uint32_t image_width =  inf_config->image_list[input_index].image_width;
                uint32_t image_height =  inf_config->image_list[input_index].image_height;

                if ((crop_x1 > image_width) || (crop_y1 > image_height) ||
                    ((crop_x1 + crop_width) > image_width) || ((crop_y1 + crop_height) > image_height)) {
                    return KP_FW_INVALID_INPUT_CROP_PARAM_112;
                }

                ncpu_img_config.image_list[input_index].params_s.crop_left = crop_x1;
                ncpu_img_config.image_list[input_index].params_s.crop_top = crop_y1;
                ncpu_img_config.image_list[input_index].params_s.crop_right = image_width - crop_width - crop_x1;
                ncpu_img_config.image_list[input_index].params_s.crop_bottom = image_height - crop_height - crop_y1;
            } else {
                ncpu_img_config.image_list[input_index].params_s.crop_left = 0;
                ncpu_img_config.image_list[input_index].params_s.crop_top = 0;
                ncpu_img_config.image_list[input_index].params_s.crop_right = 0;
                ncpu_img_config.image_list[input_index].params_s.crop_bottom = 0;
            }

            if ((KP_RESIZE_DISABLE == inf_config->image_list[input_index].image_resize) && (KP_PADDING_DISABLE == inf_config->image_list[input_index].image_padding))
            {
                if (inf_config->image_list[input_index].enable_crop)
                {
                    if ((inf_config->image_list[input_index].crop_area.width != model_width) ||
                        (inf_config->image_list[input_index].crop_area.height != model_height))
                    {
                        return KP_FW_IMAGE_SIZE_NOT_MATCH_MODEL_INPUT_107;
                    }
                }
                else
                {
                    if ((inf_config->image_list[input_index].image_width != model_width) ||
                        (inf_config->image_list[input_index].image_height != model_height))
                    {
                        return KP_FW_IMAGE_SIZE_NOT_MATCH_MODEL_INPUT_107;
                    }
                }
            }
            else if ((KP_RESIZE_DISABLE == inf_config->image_list[input_index].image_resize) && (KP_PADDING_DISABLE != inf_config->image_list[input_index].image_padding))
            {
                return KP_FW_NOT_SUPPORT_PREPROCESSING_108;
            }
            else if ((KP_RESIZE_DISABLE != inf_config->image_list[input_index].image_resize) && (KP_PADDING_DISABLE == inf_config->image_list[input_index].image_padding))
            {
                ncpu_img_config.image_list[input_index].format |= IMAGE_FORMAT_CHANGE_ASPECT_RATIO;
            }
            else if (KP_PADDING_SYMMETRIC == inf_config->image_list[input_index].image_padding)
            {
                ncpu_img_config.image_list[input_index].format |= IMAGE_FORMAT_SYMMETRIC_PADDING;
            }

            #ifdef KL720_SCPU
            /* KL720 HW inproc limitation: target resized size (h, w): must less than (2047, 1023) */
            if ((INPROC_MAX_OUTPUT_WIDTH < model_width) || (INPROC_MAX_OUTPUT_HEIGHT < model_height)) {
                return KP_FW_INVALID_PRE_PROC_MODEL_INPUT_SIZE_111;
            }
            #endif // KL720_SCPU
        }
        else // no pre-processing, bypass image
        {
            // no color space conversion, RGBA8888 only
            // no normalization
            // no scaling
            // no cropping

            ncpu_img_config.image_list[input_index].format |= IMAGE_FORMAT_BYPASS_PRE;
        }

        dbg_msg("[%d] ncpu inf_format: 0x%X, image_format = 0x%x\n", i, ncpu_img_config.inf_format, ncpu_img_config.image_list[input_index].format);
    }

    kmdw_model_config_img(&ncpu_img_config, inf_config->user_define_data);

    if (inf_config->enable_parallel)
    {
        kmdw_model_config_result(g_result_event, 0x1 << g_inf_index);

        g_result_ctx[g_inf_index].inf_result_buf = inf_config->inf_result_buf;
        g_result_ctx[g_inf_index].inf_result_buf_size = inf_config->inf_result_buf_size;
        g_result_ctx[g_inf_index].ncpu_result_buf = inf_config->ncpu_result_buf;
        g_result_ctx[g_inf_index].result_callback_func = inf_config->result_callback;

        g_num_parallel_inf++;

        g_inf_index++;
        if (g_inf_index >= MAX_OUTPUT_CONTEXT_NUM)
            g_inf_index = 0;
    }
    else
        kmdw_model_config_result(0, 0);

    dbg_print("ncpu_result_buf = 0x%x\n", inf_config->ncpu_result_buf);
    dbg_print("model_id = %d\n", inf_config->model_id);

    int status = kmdw_model_run("", inf_config->ncpu_result_buf, inf_config->model_id, true);

    int img_idx = ncpu_img_config.image_buf_active_index;
    struct kdp_img_raw_s *raw_img = kmdw_model_get_raw_img(img_idx);

    for (int i = 0; i < num_input_node; i++) {
        if (NULL != inf_config->image_list[i].pad_value) {
            inf_config->image_list[i].pad_value->pad_top = raw_img->image_list[i].params_s.pad_top;
            inf_config->image_list[i].pad_value->pad_bottom = raw_img->image_list[i].params_s.pad_bottom;
            inf_config->image_list[i].pad_value->pad_left = raw_img->image_list[i].params_s.pad_left;
            inf_config->image_list[i].pad_value->pad_right = raw_img->image_list[i].params_s.pad_right;
        }

        if (true == inf_config->image_list[i].enable_crop) {
            inf_config->image_list[i].crop_area.x1 = raw_img->image_list[i].params_s.crop_left;
            inf_config->image_list[i].crop_area.y1 = raw_img->image_list[i].params_s.crop_top;
            inf_config->image_list[i].crop_area.width = inf_config->image_list[i].image_width
                                                            - raw_img->image_list[i].params_s.crop_left
                                                            - raw_img->image_list[i].params_s.crop_right;
            inf_config->image_list[i].crop_area.height = inf_config->image_list[i].image_height
                                                            - raw_img->image_list[i].params_s.crop_top
                                                            - raw_img->image_list[i].params_s.crop_bottom;
        }
    }

    if (status == IMAGE_STATE_TIMEOUT)
        return KP_FW_INFERENCE_TIMEOUT_103;
    else if (status != 0)
        return KP_FW_INFERENCE_ERROR_101;
    else
        return KP_SUCCESS;
}

void kmdw_inference_app_send_status_code(int job_id, int error_code)
{
    // we need a result buffer
    int result_stamp_buf_size;
    kp_inference_header_stamp_t *result_stamp = (kp_inference_header_stamp_t *)kmdw_fifoq_manager_result_get_free_buffer(&result_stamp_buf_size);

    result_stamp->magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    result_stamp->total_size = sizeof(kp_inference_header_stamp_t);
    result_stamp->job_id = job_id;
    result_stamp->status_code = error_code;

    kmdw_fifoq_manager_result_enqueue((void *)result_stamp, result_stamp_buf_size, false);
}

uint32_t kmdw_inference_app_get_model_raw_output_size(uint32_t model_id)
{
    // Warning: this raw output size is only for "data"

    // FIXME: should make this faster

    uint32_t *all_model_info = kmdw_model_get_all_model_info(false);
    if (!all_model_info)
        return 0;

    int num_models = all_model_info[0];
    for (int j = 0; j < num_models; j++)
    {
        struct kdp_model_s *model_info = kmdw_model_get_model_info(j);
        if (model_info->model_type == model_id)
        {
            return model_info->output_mem_len;
        }
    }

    return 0;
}

uint32_t kmdw_inference_get_raw_output_info_size(void)
{
    return kdp2_get_raw_output_info_size();
}

int kmdw_inference_get_model_input_image_size(uint32_t model_id, uint32_t input_index, uint32_t *model_input_width, uint32_t *model_input_height)
{
    uint32_t *all_model_info = kmdw_model_get_all_model_info(false);

    if (!all_model_info) {
        return KP_FW_LOAD_MODEL_FAILED_104;
    }

    int num_models = all_model_info[0];

    for (int j = 0; j < num_models; j++) {
        struct kdp_model_s *model_info = kmdw_model_get_model_info(j);

        if (model_info->model_type == model_id) {
            int num_input_node = 0;

            if (SETUP_LEGACY_MAGIC_NUM == *((uint32_t *)model_info->setup_mem_addr)) {
                /******************************************************************
                 * legacy setup.bin model
                 ******************************************************************/
                num_input_node = ((CNN_Header *)model_info->setup_mem_addr)->input_num;

                for (int i = 0; i < num_input_node; i++) {
                    NetInput_Node *net_input_node = (NetInput_Node *)(model_info->setup_mem_addr + sizeof(CNN_Header) + (i * sizeof(NetInput_Node)));

                    if (input_index == net_input_node->input_index) {
                        *model_input_width = net_input_node->input_col;
                        *model_input_height = net_input_node->input_row;

                        return KP_SUCCESS;
                    }
                }
            } else {
                /******************************************************************
                 * flatbuffer setup.bin model
                 ******************************************************************/
                Kneron_INFContent_table_t root = Kneron_INFContent_as_root((void *)model_info->setup_mem_addr);

                if (NULL == root) {
                    err_msg("invalid model setup buffer\n");
                    return KP_FW_LOAD_MODEL_FAILED_104;
                }

                Kneron_Tensor_vec_t tensor_vec = Kneron_INFContent_inputs(root);
                num_input_node = Kneron_Tensor_vec_len(tensor_vec);

                if (num_input_node > input_index) {
                    Kneron_Tensor_table_t tensor = Kneron_Tensor_vec_at(tensor_vec, input_index);
                    flatbuffers_int32_vec_t shape_vec = Kneron_Tensor_shape(tensor);

                    /* shape order [batch, channel, hieght, width] */
                    *model_input_width = shape_vec[3];
                    *model_input_height = shape_vec[2];

                    return KP_SUCCESS;
                }
            }
        }
    }

    return KP_FW_LOAD_MODEL_FAILED_104;
}

int kmdw_inference_app_init(kmdw_inference_app_callback_t app_entry, uint32_t image_count, uint32_t result_count)
{
    kmdw_printf("\n");
    kmdw_printf("starting KDP2 middleware ...\n");

    kmdw_fifoq_manager_init(image_count, result_count);

    _app_entry_func = app_entry;

    g_result_event = osEventFlagsNew(0);

    return 0;
}
