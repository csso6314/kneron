/**
 * @file        model_descriptor_constructor.c
 * @brief       NEF model related functions - construct model information from NEF binary
 * @version     0.1
 * @date        2022-04-05
 *
 * @copyright   Copyright (c) 2022 Kneron Inc. All rights reserved.
 */

/**
 * [Description]
 *  Model descriptor builder is used to build kp_model_nef_descriptor_t from NEF binary/kp_nef_info_t
 *
 * [Architecture Hierarchical]
 *  model_descriptor_builder
 *      |- kneron_nef_reader
 *      |- setup_reader
 */

// #define DEBUG_PRINT

#include "internal_func.h"
#include <stdio.h>
#include <string.h>

#include "kdp2_inf_generic_raw.h"

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }
#else
#define dbg_print(format, ...)
#endif

#define err_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }

/******************************************************************
 * fw_info.bin reader
 ******************************************************************/

/**
 * single model fw_info schema
 * note: from firmware ipc.h/kdp_model_s to parse fw_info.bin
 */
typedef struct
{
    /* Model type (model id) */
    uint32_t model_type;

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
} __attribute__((packed, aligned(4))) _single_model_firmware_info_t;

/**
 * fw_info.bin schema
 */
typedef struct
{
    uint32_t                        model_num;
    _single_model_firmware_info_t*  model_firmware_info;
    uint32_t                        model_dram_addr_end;
    uint32_t                        model_total_size;
    uint32_t                        model_checksum;
} __attribute__((packed, aligned(4))) _model_firmware_info_list_t;

/******************************************************************
 * setup.bin memory offset
 ******************************************************************/

/**
 * single model setup memory info
 */
typedef struct
{
    uint32_t setup_mem_addr;
    int32_t setup_mem_len;
} __attribute__((packed, aligned(4))) _single_model_setup_memory_info_t;

/**
 * model setup memory info list
 */
typedef struct
{
    uint32_t setup_num;
    _single_model_setup_memory_info_t* model_setup_memory_info_list;
} __attribute__((packed, aligned(4))) _model_setup_memory_info_list_t;

/******************************************************************
 * kp_model_nef_descriptor_t deconstructor
 ******************************************************************/

int deconstruct_model_setup_memory_info_list(_model_setup_memory_info_list_t* model_setup_memory_info_list) {
    int ret = KP_SUCCESS;

    model_setup_memory_info_list->model_setup_memory_info_list = realloc_zero(model_setup_memory_info_list->model_setup_memory_info_list, 0);
    model_setup_memory_info_list->setup_num = 0;

    if (NULL != model_setup_memory_info_list->model_setup_memory_info_list) {
        err_print("deconstruct model setup memoery list fail ...\n");
        ret = KP_ERROR_MEMORY_FREE_FAILURE_39;
    }

    return ret;
}

int deconstruct_quantization_parameters(kp_quantization_parameters_t* quantization_parameters) {
    int ret = KP_SUCCESS;

    quantization_parameters->quantized_fixed_point_descriptor = realloc_zero(quantization_parameters->quantized_fixed_point_descriptor, 0);
    quantization_parameters->quantized_fixed_point_descriptor_num = 0;

    if (NULL != quantization_parameters->quantized_fixed_point_descriptor) {
        err_print("deconstruct tensor quantization fixed point parameter in model_descriptor fail ...\n");
        ret = KP_ERROR_MEMORY_FREE_FAILURE_39;
    }

    return ret;
}

int deconstruct_tensor_descriptor(kp_tensor_descriptor_t* tensor_descriptor) {
    int ret = KP_SUCCESS;
    kp_quantization_parameters_t *quantization_parameters = NULL;

    tensor_descriptor->name =       realloc_zero(tensor_descriptor->name, 0);
    tensor_descriptor->shape_npu =  realloc_zero(tensor_descriptor->shape_npu, 0);
    tensor_descriptor->shape_onnx = realloc_zero(tensor_descriptor->shape_onnx, 0);

    if (NULL != tensor_descriptor->name ||
        NULL != tensor_descriptor->shape_npu ||
        NULL != tensor_descriptor->shape_onnx) {
        err_print("deconstruct tensor descriptor in model_descriptor fail ...\n");
        ret = KP_ERROR_MEMORY_FREE_FAILURE_39;
        goto FUNC_OUT;
    }

    quantization_parameters = &(tensor_descriptor->quantization_parameters);
    ret = deconstruct_quantization_parameters(quantization_parameters);

    if (KP_SUCCESS != ret) {
        err_print("deconstruct tensor quantization parameters in model_descriptor fail ...\n");
        goto FUNC_OUT;
    }

    memset(tensor_descriptor, 0, sizeof(kp_tensor_descriptor_t));

FUNC_OUT:
    return ret;
}

int deconstruct_single_model_descriptor(kp_single_model_descriptor_t* single_model_descriptor) {
    int ret = KP_SUCCESS;
    kp_tensor_descriptor_t *tensor_descriptor = NULL;

    for (int i = 0; i < single_model_descriptor->input_nodes_num; i++) {
        tensor_descriptor = &(single_model_descriptor->input_nodes[i]);
        ret = deconstruct_tensor_descriptor(tensor_descriptor);

        if (KP_SUCCESS != ret) {
            err_print("deconstruct input tensor descriptor fail ...\n");
            goto FUNC_OUT;
        }
    }

    for (int i = 0; i < single_model_descriptor->output_nodes_num; i++) {
        tensor_descriptor = &(single_model_descriptor->output_nodes[i]);
        ret = deconstruct_tensor_descriptor(tensor_descriptor);

        if (KP_SUCCESS != ret) {
            err_print("deconstruct output tensor descriptor fail ...\n");
            goto FUNC_OUT;
        }
    }

    single_model_descriptor->input_nodes =  realloc_zero(single_model_descriptor->input_nodes, 0);
    single_model_descriptor->output_nodes = realloc_zero(single_model_descriptor->output_nodes, 0);

    if (NULL != single_model_descriptor->input_nodes ||
        NULL != single_model_descriptor->output_nodes) {
        err_print("deconstruct single model descriptor attributes in model_descriptor fail ...\n");
        ret = KP_ERROR_MEMORY_FREE_FAILURE_39;
        goto FUNC_OUT;
    }

    memset(single_model_descriptor, 0, sizeof(kp_single_model_descriptor_t));

FUNC_OUT:
    return ret;
}

int deconstruct_model_des_nef_metadata(kp_model_nef_metadata_t* metadata) {
    int ret = KP_SUCCESS;

    if (NULL == metadata) {
        return ret;
    }

    metadata->toolchain_version =           realloc_zero(metadata->toolchain_version, 0);
    metadata->compiler_version =            realloc_zero(metadata->compiler_version, 0);
    metadata->platform =                    realloc_zero(metadata->platform, 0);
    metadata->kn_num =                      0;

    if (NULL != metadata->toolchain_version ||
        NULL != metadata->compiler_version ||
        NULL != metadata->platform) {
        err_print("deconstruct nef metadata attributes in model_descriptor fail ...\n");
        ret = KP_ERROR_MEMORY_FREE_FAILURE_39;
        goto FUNC_OUT;
    }

    memset(metadata, 0, sizeof(kp_model_nef_metadata_t));

FUNC_OUT:
    return ret;
}

int deconstruct_model_des_nef_info(kp_model_nef_descriptor_t* model_desc) {
    int ret = KP_SUCCESS;

    if (NULL == model_desc) {
        goto FUNC_OUT;
    }

    kp_single_model_descriptor_t *single_model_descriptor = NULL;
    for (int i = 0; i < model_desc->num_models; i++) {
        single_model_descriptor = &(model_desc->models[i]);
        ret = deconstruct_single_model_descriptor(single_model_descriptor);

        if (KP_SUCCESS != ret) {
            err_print("deconstruct single model descriptor in model_descriptor fail ...\n");
            goto FUNC_OUT;
        }
    }

    model_desc->models =        realloc_zero(model_desc->models, 0);
    model_desc->num_models =    0;
    model_desc->crc =           0;
    model_desc->target =        0;

    if (NULL != model_desc->models) {
        err_print("deconstruct nef info attributes in model_descriptor fail ...\n");
        ret = KP_ERROR_MEMORY_FREE_FAILURE_39;
        goto FUNC_OUT;
    }

FUNC_OUT:
    return ret;
}

int deconstruct_model_nef_descriptor(kp_model_nef_descriptor_t* loaded_model_desc) {
    int ret = KP_SUCCESS;

    if (NULL == loaded_model_desc) {
        ret = KP_ERROR_INVALID_PARAM_12;
        goto FUNC_OUT;
    }

    if (0x5AA55AA5 != loaded_model_desc->magic)
        goto FUNC_OUT;

    ret = deconstruct_model_des_nef_metadata(&(loaded_model_desc->metadata));

    if (KP_SUCCESS != ret) {
        err_print("deconstruct nef metadata in model_descriptor fail ...\n");
        goto FUNC_OUT;
    }

    ret = deconstruct_model_des_nef_info(loaded_model_desc);

    if (KP_SUCCESS != ret) {
        err_print("deconstruct nef info in model_descriptor fail ...\n");
        goto FUNC_OUT;
    }

FUNC_OUT:
    memset(loaded_model_desc, 0, sizeof(kp_model_nef_descriptor_t));

    return ret;
}

/******************************************************************
 * kp_model_nef_descriptor_t constructor
 ******************************************************************/

_single_model_setup_memory_info_t* realloc_model_setup_memory_info_list(_single_model_setup_memory_info_t* model_setup_memory_info_list, uint32_t element_num) {
    return (_single_model_setup_memory_info_t*)realloc_zero(model_setup_memory_info_list, element_num * sizeof(_single_model_setup_memory_info_t));
}

kp_single_model_descriptor_t* realloc_model_descriptor_list(kp_single_model_descriptor_t *model_descriptor_list, uint32_t element_num) {
    return (kp_single_model_descriptor_t*)realloc_zero(model_descriptor_list, element_num * sizeof(kp_single_model_descriptor_t));
}

uint32_t* realloc_tensor_shape(uint32_t *shape, uint32_t element_num) {
    return (uint32_t *)realloc_zero(shape, element_num * sizeof(uint32_t));
}

kp_tensor_descriptor_t* realloc_tensor_list(kp_tensor_descriptor_t *tensor_list, uint32_t element_num) {
    return (kp_tensor_descriptor_t*)realloc_zero(tensor_list, element_num * sizeof(kp_tensor_descriptor_t));
}

kp_quantized_fixed_point_descriptor_t* realloc_quantized_fixed_point_descriptor_list(kp_quantized_fixed_point_descriptor_t *quantized_fixed_point_descriptor_list, uint32_t element_num) {
    return (kp_quantized_fixed_point_descriptor_t*)realloc_zero(quantized_fixed_point_descriptor_list, element_num * sizeof(kp_quantized_fixed_point_descriptor_t));
}

int initialize_model_des_nef_magic(kp_model_nef_descriptor_t* loaded_model_nef_descriptor) {
    if (NULL == loaded_model_nef_descriptor) {
        err_print("initialize magic number in model_descriptor fail: NULL pointer input parameters ...\n");
        return -1;
    }

    /**
     * First initialization of kp_model_nef_descriptor_t struc.
     * Using 'magic = 0x5AA55AA5' to make sure the kp_model_nef_descriptor_t is initialized.
     */
    loaded_model_nef_descriptor->magic = 0x5AA55AA5;

    return KP_SUCCESS;
}

int construct_model_des_nef_metadata(kp_metadata_t *metadata, kp_model_nef_descriptor_t* loaded_model_nef_descriptor) {
    if (NULL == metadata ||
        NULL == loaded_model_nef_descriptor) {
        err_print("construct nef metadata in model_descriptor fail: NULL pointer input parameters ...\n");
        return -1;
    }

    kp_model_nef_metadata_t *loaded_model_nef_metadata = &(loaded_model_nef_descriptor->metadata);

    loaded_model_nef_metadata->kn_num =                         metadata->kn_num;
    loaded_model_nef_metadata->compiler_version =               strcpy_dst_realloc(loaded_model_nef_metadata->compiler_version,     metadata->compiler_ver);
    loaded_model_nef_metadata->toolchain_version =              strcpy_dst_realloc(loaded_model_nef_metadata->toolchain_version,    metadata->tc_ver);
    loaded_model_nef_metadata->platform =                       strcpy_dst_realloc(loaded_model_nef_metadata->platform,             metadata->platform);
    loaded_model_nef_metadata->nef_schema_version.major =       metadata->nef_schema_version.major;
    loaded_model_nef_metadata->nef_schema_version.minor =       metadata->nef_schema_version.minor;
    loaded_model_nef_metadata->nef_schema_version.revision =    metadata->nef_schema_version.revision;

    if (NULL == loaded_model_nef_metadata->compiler_version ||
        NULL == loaded_model_nef_metadata->toolchain_version||
        NULL == loaded_model_nef_metadata->platform) {
        err_print("construct nef metadata in model_descriptor fail: load nef meta data fail ...\n");
        return KP_ERROR_INVALID_MODEL_21;
    }

    return KP_SUCCESS;
}

int _parse_firmware_info(kp_nef_info_t *nef_info, _model_firmware_info_list_t *model_firmware_info_list) {
    if (NULL == nef_info ||
        NULL == model_firmware_info_list) {
        err_print("construct model firmware info list fail: NULL pointer input parameters ...\n");
        return KP_ERROR_INVALID_PARAM_12;
    }

    uintptr_t fw_info_buff = (uintptr_t)nef_info->fw_info_addr;
    size_t fw_info_buff_offset = 0;

    model_firmware_info_list->model_num =             *(                     (uint32_t *)(fw_info_buff + (fw_info_buff_offset)));
    model_firmware_info_list->model_firmware_info =    ((_single_model_firmware_info_t *)(fw_info_buff + (fw_info_buff_offset += sizeof(model_firmware_info_list->model_num))));
    model_firmware_info_list->model_dram_addr_end =   *(                     (uint32_t *)(fw_info_buff + (fw_info_buff_offset += (model_firmware_info_list->model_num * sizeof(_single_model_firmware_info_t)))));
    model_firmware_info_list->model_total_size =      *(                     (uint32_t *)(fw_info_buff + (fw_info_buff_offset += sizeof(model_firmware_info_list->model_dram_addr_end))));
    model_firmware_info_list->model_checksum =        *(                     (uint32_t *)(fw_info_buff + (fw_info_buff_offset += sizeof(model_firmware_info_list->model_total_size))));

    if (NULL == model_firmware_info_list->model_firmware_info) {
        err_print("parse model firmware info fail: invalid address ...\n");
        return KP_ERROR_INVALID_MODEL_21;
    }

    return KP_SUCCESS;
}

int _parse_setup_memory_info(_model_firmware_info_list_t *model_firmware_info_list, bool from_device, _model_setup_memory_info_list_t* model_setup_memory_info_list) {
    if (NULL == model_firmware_info_list ||
        NULL == model_setup_memory_info_list) {
        err_print("parse model setup memory info list fail: NULL pointer input parameters ...\n");
        return KP_ERROR_INVALID_PARAM_12;
    }

    _single_model_setup_memory_info_t* single_model_setup_memory_info = NULL;
    _single_model_firmware_info_t *single_model_firmware_info = NULL;
    model_setup_memory_info_list->setup_num = model_firmware_info_list->model_num;
    model_setup_memory_info_list->model_setup_memory_info_list = realloc_model_setup_memory_info_list(model_setup_memory_info_list->model_setup_memory_info_list, model_setup_memory_info_list->setup_num);

    if (0 < model_setup_memory_info_list->setup_num &&
        NULL == model_setup_memory_info_list->model_setup_memory_info_list) {
        err_print("parse model setup memory info list fail: remalloc model setup memory fail ...\n");
        return KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
    }

    if (from_device) {
        /**
         * setup buff from device is pure setup info
         *
         * layout:
         * model_setup_0
         * model_setup_1
         * model_setup_2
         * ...
         */
        uint32_t setup_mem_addr = 0;
        for (int i = 0; i < model_setup_memory_info_list->setup_num; i++) {
            single_model_setup_memory_info =    &(model_setup_memory_info_list->model_setup_memory_info_list[i]);
            single_model_firmware_info =        &(model_firmware_info_list->model_firmware_info[i]);

            single_model_setup_memory_info->setup_mem_addr = setup_mem_addr;
            single_model_setup_memory_info->setup_mem_len = single_model_firmware_info->setup_mem_len;

            setup_mem_addr += single_model_firmware_info->setup_mem_len;
        }
    } else {
        /**
         * setup buff from file is all_models info
         *
         * layout:
         * all_model_0
         *      other_info_0
         *      model_setup_0
         *      other_info_0
         * all_model_1
         *      other_info_1
         *      model_setup_1
         *      other_info_1
         * all_model_2
         * ...
         */
        for (int i = 0; i < model_setup_memory_info_list->setup_num; i++) {
            single_model_setup_memory_info =    &(model_setup_memory_info_list->model_setup_memory_info_list[i]);
            single_model_firmware_info =        &(model_firmware_info_list->model_firmware_info[i]);

            single_model_setup_memory_info->setup_mem_addr = single_model_firmware_info->setup_mem_addr;
            single_model_setup_memory_info->setup_mem_len = single_model_firmware_info->setup_mem_len;
        }
    }

    return KP_SUCCESS;
}

int construct_model_des_nef_info(kp_nef_info_t *nef_info, bool from_device, kp_model_nef_descriptor_t* loaded_model_desc) {
    if (NULL == nef_info ||
        NULL == loaded_model_desc) {
        err_print("construct nef firmware info in model_descriptor fail: NULL pointer input parameters ...\n");
        return KP_ERROR_INVALID_PARAM_12;
    }

    int status = KP_SUCCESS;

    // set target information (need to be get from device/nef_metadata)
    loaded_model_desc->target = nef_info->target;

    // parse fw_info
    _model_firmware_info_list_t model_firmware_info_list = {0};
    status = _parse_firmware_info(nef_info, &model_firmware_info_list);

    if (KP_SUCCESS != status) {
        err_print("parse model firmware info fail ...\n");
        status =  KP_ERROR_INVALID_MODEL_21;
        goto FUNC_OUT;
    }

    // parse setup memory address list
    _model_setup_memory_info_list_t model_setup_memory_info_list = {0};
    status = _parse_setup_memory_info(&model_firmware_info_list, from_device, &model_setup_memory_info_list);

    if (KP_SUCCESS != status) {
        err_print("parse model firmware info fail ...\n");
        goto FUNC_OUT;
    }

    // realloc model descriptor list
    loaded_model_desc->num_models = model_firmware_info_list.model_num;
    loaded_model_desc->models = realloc_model_descriptor_list(loaded_model_desc->models, loaded_model_desc->num_models);

    if (0 < loaded_model_desc->num_models &&
        NULL == loaded_model_desc->models) {
        err_print("construct nef model_descriptor fail: remalloc single model descriptor fail ...\n");
        status = KP_ERROR_MEMORY_ALLOCATION_FAILURE_9;
        goto FUNC_OUT;
    }

    // parse setup in all_models
    uintptr_t all_model_buff = (uintptr_t)nef_info->all_models_addr;
    uintptr_t all_model_base = (uintptr_t)((from_device) ? 0 : model_firmware_info_list.model_firmware_info[0].cmd_mem_addr);
    uintptr_t setup_buff = 0;
    size_t setup_buff_instance_offset = 0;
    size_t setup_buff_size = 0;

    if (0 == all_model_buff) {
        err_print("construct nef info in model_descriptor fail: invalid model_all_model_info ...\n");
        status = KP_ERROR_INVALID_MODEL_21;
        goto FUNC_OUT;
    }

    // load fw_info, setup
    kp_single_model_descriptor_t *single_model_descriptor;
    _single_model_firmware_info_t *single_model_firmware_info;
    _single_model_setup_memory_info_t *single_model_setup_memory_info;

    for (int i = 0; i < loaded_model_desc->num_models; i++) {
        single_model_descriptor =           &(loaded_model_desc->models[i]);
        single_model_firmware_info =        &(model_firmware_info_list.model_firmware_info[i]);
        single_model_setup_memory_info =    &(model_setup_memory_info_list.model_setup_memory_info_list[i]);

        // load fw_info
        {
            /**
             * Note: flatbuffer version setup parser will update 'target' in construct_single_setup_info()
             */
            single_model_descriptor->target =           loaded_model_desc->target;
            single_model_descriptor->id =               single_model_firmware_info->model_type;
            single_model_descriptor->version =          single_model_firmware_info->model_version;
        }

        dbg_print("start parse\n");

        // load setup
        {
            setup_buff_instance_offset = single_model_setup_memory_info->setup_mem_addr - (size_t)all_model_base;
            setup_buff_size = single_model_setup_memory_info->setup_mem_len;
            setup_buff = all_model_buff + setup_buff_instance_offset;

            status = construct_single_setup_info(setup_buff, setup_buff_size, single_model_descriptor);

            if (KP_SUCCESS != status)
                goto FUNC_OUT;

            // calculate raw ouput size from firmware
            if (KP_MODEL_TARGET_CHIP_KL520 == loaded_model_desc->target) {
                single_model_descriptor->max_raw_out_size = sizeof(kdp2_ipc_generic_raw_result_t) + 4 + single_model_descriptor->output_nodes_num * sizeof(kp_inf_raw_fixed_node_metadata_t) + single_model_firmware_info->output_mem_len;
            } else if (KP_MODEL_TARGET_CHIP_KL720 == loaded_model_desc->target) {
                single_model_descriptor->max_raw_out_size = sizeof(kdp2_ipc_generic_raw_result_t) + sizeof(_720_raw_cnn_res_t) + single_model_firmware_info->output_mem_len;
            } else if (KP_MODEL_TARGET_CHIP_KL530 == loaded_model_desc->target) {
                // FIXME: specify kl530 raw output buffer size
                single_model_descriptor->max_raw_out_size = sizeof(kdp2_ipc_generic_raw_result_t);
            } else if (KP_MODEL_TARGET_CHIP_KL630 == loaded_model_desc->target) {
                single_model_descriptor->max_raw_out_size = sizeof(kdp2_ipc_generic_raw_result_t) + sizeof(_630_raw_cnn_res_t) + single_model_firmware_info->output_mem_len;
            } else {
                err_print("construct nef info in model_descriptor fail: invalid target %u ...\n", loaded_model_desc->target);
                status = KP_ERROR_INVALID_MODEL_21;
                goto FUNC_OUT;
            }
        }
    }

    loaded_model_desc->crc = model_firmware_info_list.model_checksum;

FUNC_OUT:
    deconstruct_model_setup_memory_info_list(&model_setup_memory_info_list);

    return status;
}

/******************************************************************
 * nef info print
 ******************************************************************/

int print_model_nef_descriptor(kp_model_nef_descriptor_t* loaded_model_desc) {
    if (NULL == loaded_model_desc) {
        err_print("print nef firmware info in model_descriptor fail: NULL pointer input parameters ...\n");
        return KP_ERROR_INVALID_PARAM_12;
    }

    err_print("=======================================================\n");
    err_print("target: %u\n", loaded_model_desc->target);
    err_print("crc: %u\n", loaded_model_desc->crc);
    err_print("num_models: %u\n",  loaded_model_desc->num_models);
    err_print("=======================================================\n");
    err_print("kn_num: %u\n", loaded_model_desc->metadata.kn_num);
    err_print("toolchain_version: %s\n", loaded_model_desc->metadata.toolchain_version);
    err_print("compiler_version: %s\n", loaded_model_desc->metadata.compiler_version);
    err_print("nef_schema_version: %u.%u.%u\n", loaded_model_desc->metadata.nef_schema_version.major,
                                                loaded_model_desc->metadata.nef_schema_version.minor,
                                                loaded_model_desc->metadata.nef_schema_version.revision);
    err_print("platform: %s\n", loaded_model_desc->metadata.platform);
    err_print("=======================================================\n");
    for (int model_idx = 0; model_idx < loaded_model_desc->num_models; model_idx++) {
        kp_single_model_descriptor_t* single_model_descriptor = &(loaded_model_desc->models[model_idx]);
        err_print("target: %u\n", single_model_descriptor->target);
        err_print("version: 0x%x\n",  single_model_descriptor->version);
        err_print("id: %u\n", single_model_descriptor->id);
        err_print("setup_bin_schema_version: %u.%u.%u\n", single_model_descriptor->setup_bin_schema_version.major,
                                                          single_model_descriptor->setup_bin_schema_version.minor,
                                                          single_model_descriptor->setup_bin_schema_version.revision);
        err_print("file_schema_version: %u.%u.%u\n", single_model_descriptor->file_schema_version.major,
                                                     single_model_descriptor->file_schema_version.minor,
                                                     single_model_descriptor->file_schema_version.revision);
        err_print("=======================================================\n");

        err_print("[input node]\n");
        err_print("=======================================================\n");
        for (int node_idx = 0; node_idx < single_model_descriptor->input_nodes_num; node_idx++) {
            kp_tensor_descriptor_t* tensor_descriptor = &(single_model_descriptor->input_nodes[node_idx]);
            kp_quantized_fixed_point_descriptor_t* quantized_fixed_point_descriptor = &(tensor_descriptor->quantization_parameters.quantized_fixed_point_descriptor[0]);

            err_print("index: %u\n", tensor_descriptor->index);
            err_print("name: %s\n", tensor_descriptor->name);
            err_print("data_layout: %u\n",  tensor_descriptor->data_layout);
            err_print("quantization_parameters: scale %f radix %d\n", quantized_fixed_point_descriptor->scale, quantized_fixed_point_descriptor->radix);

            err_print("shape_npu: [");
            for (int i = 0; i < tensor_descriptor->shape_npu_len; i++) {
                err_print(" %u", tensor_descriptor->shape_npu[i]);

                if (i < tensor_descriptor->shape_npu_len - 1)
                    err_print(",");
            }
            err_print("]\n");

            err_print("shape_onnx: [");
            for (int i = 0; i < tensor_descriptor->shape_onnx_len; i++) {
                err_print(" %u", tensor_descriptor->shape_onnx[i]);
                if (i < tensor_descriptor->shape_onnx_len - 1)
                    err_print(",");
            }
            err_print("]\n");
            err_print("=======================================================\n");
        }
        err_print("[output node]\n");
        err_print("=======================================================\n");
        for (int node_idx = 0; node_idx < single_model_descriptor->output_nodes_num; node_idx++) {
            kp_tensor_descriptor_t* tensor_descriptor = &(single_model_descriptor->output_nodes[node_idx]);
            kp_quantized_fixed_point_descriptor_t* quantized_fixed_point_descriptor = &(tensor_descriptor->quantization_parameters.quantized_fixed_point_descriptor[0]);

            err_print("index: %u\n", tensor_descriptor->index);
            err_print("name: %s\n", tensor_descriptor->name);
            err_print("data_layout: %u\n",  tensor_descriptor->data_layout);
            err_print("quantization_parameters: scale %f radix %d\n", quantized_fixed_point_descriptor->scale, quantized_fixed_point_descriptor->radix);

            err_print("shape_npu: [");
            for (int i = 0; i < tensor_descriptor->shape_npu_len; i++) {
                err_print(" %u", tensor_descriptor->shape_npu[i]);

                if (i < tensor_descriptor->shape_npu_len - 1)
                    err_print(",");
            }
            err_print("]\n");

            err_print("shape_onnx: [");
            for (int i = 0; i < tensor_descriptor->shape_onnx_len; i++) {
                err_print(" %u", tensor_descriptor->shape_onnx[i]);
                if (i < tensor_descriptor->shape_onnx_len - 1)
                    err_print(",");
            }
            err_print("]\n");
            err_print("=======================================================\n");
        }
        err_print("=======================================================\n");
    }

    return KP_SUCCESS;
}

/******************************************************************
 * nef info reader
 ******************************************************************/

int _check_model_platform(kp_product_id_t target_pid, kp_model_target_chip_t model_target_chip)
{
    int ret = KP_SUCCESS;

    if ((target_pid == KP_DEVICE_KL520) && (model_target_chip != KP_MODEL_TARGET_CHIP_KL520))
        ret = KP_ERROR_INVALID_MODEL_21;

    if ((target_pid == KP_DEVICE_KL720) && (model_target_chip != KP_MODEL_TARGET_CHIP_KL720))
        ret = KP_ERROR_INVALID_MODEL_21;

    if ((target_pid == KP_DEVICE_KL530) && (model_target_chip != KP_MODEL_TARGET_CHIP_KL530))
        ret = KP_ERROR_INVALID_MODEL_21;

    if ((target_pid == KP_DEVICE_KL730) && (model_target_chip != KP_MODEL_TARGET_CHIP_KL730))
        ret = KP_ERROR_INVALID_MODEL_21;

    if ((target_pid == KP_DEVICE_KL630) && (model_target_chip != KP_MODEL_TARGET_CHIP_KL630))
        ret = KP_ERROR_INVALID_MODEL_21;

    return ret;
}

int build_model_nef_descriptor_from_nef(kp_metadata_t *metadata, kp_nef_info_t *nef_info, kp_model_nef_descriptor_t* loaded_model_desc) {
    if ((NULL == metadata) || (NULL == nef_info))
    {
        dbg_print("invalid parameters, null pointer ...\n");
        return KP_ERROR_INVALID_PARAM_12;
    }

    // parse nef header/fw_info.bin/setup.bin from nef file
    if (0 >= nef_info->fw_info_size)
    {
        dbg_print("invalid model format, fw_info size %d ...\n", nef_info->fw_info_size);
        return KP_ERROR_INVALID_MODEL_21;
    }

    int ret = KP_SUCCESS;
    bool from_device = false;

    ret = deconstruct_model_nef_descriptor(loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("construct model nef descriptor failed: %d...\n", ret);
        return ret;
    }

    ret = initialize_model_des_nef_magic(loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("initialize magic number of model nef descriptor failed: %d...\n", ret);
        return ret;
    }

    ret = construct_model_des_nef_metadata(metadata, loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("construct model nef matadata failed: %d...\n", ret);
        return ret;
    }

    ret = construct_model_des_nef_info(nef_info, from_device, loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("construct model nef information failed: %d...\n", ret);
        return ret;
    }

    return ret;
}

int build_model_nef_descriptor_from_device(kp_nef_info_t *nef_info, kp_model_nef_descriptor_t* loaded_model_desc) {
    if (NULL == nef_info)
    {
        dbg_print("invalid parameters, null pointer ...\n");
        return KP_ERROR_INVALID_PARAM_12;
    }

    // parse nef header/fw_info.bin/setup.bin from nef file
    if (0 >= nef_info->fw_info_size)
    {
        dbg_print("invalid model format, fw_info size %d ...\n", nef_info->fw_info_size);
        return KP_ERROR_INVALID_MODEL_21;
    }

    int ret = KP_SUCCESS;
    bool from_device = true;

    // init empty metadata
    kp_metadata_t metadata = {0};
    metadata.compiler_ver = "";
    metadata.platform = "";
    metadata.tc_ver = "";

    ret = deconstruct_model_nef_descriptor(loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("deconstruct model nef descriptor failed: %d...\n", ret);
        return ret;
    }

    ret = initialize_model_des_nef_magic(loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("initialize magic number of model nef descriptor failed: %d...\n", ret);
        return ret;
    }

    ret = construct_model_des_nef_metadata(&metadata, loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("construct model nef matadata failed: %d...\n", ret);
        return ret;
    }

    ret = construct_model_des_nef_info(nef_info, from_device, loaded_model_desc);
    if (KP_SUCCESS != ret)
    {
        err_print("construct model nef information failed: %d...\n", ret);
        return ret;
    }

    return ret;
}

int load_model_info_from_nef(void *nef_buf, int nef_size, kp_product_id_t target_pid /* input */, kp_metadata_t *metadata, kp_nef_info_t *nef_info, kp_model_nef_descriptor_t *loaded_model_desc /* output */)
{
    if ((NULL == metadata) || (NULL == nef_info))
    {
        dbg_print("invalid parameters, null pointer ...\n");
        return KP_ERROR_INVALID_PARAM_12;
    }

    memset(metadata, 0, sizeof(kp_metadata_t));
    memset(nef_info, 0, sizeof(kp_nef_info_t));

    int ret = read_nef(nef_buf, nef_size, metadata, nef_info);
    if (ret != 0)
    {
        dbg_print("getting model data failed: %d...\n", ret);
        return KP_ERROR_INVALID_MODEL_21;
    }

    ret = _check_model_platform(target_pid, metadata->target);
    if (KP_SUCCESS != ret)
        return ret;

    if (NULL != loaded_model_desc)
        return build_model_nef_descriptor_from_nef(metadata, nef_info, loaded_model_desc);
    else
        return KP_SUCCESS;
}
