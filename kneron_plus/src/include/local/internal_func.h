/**
 * @file        internal_func.h
 * @brief       
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */


#ifndef __INTERNAL_FUNC_H__
#define __INTERNAL_FUNC_H__

#include "kp_struct.h"
#include <stddef.h>

/**
 * @brief metadata for nef model data: metadata / fw_info / all_models
 */
typedef struct
{
    char* platform;                             /**< usb dongle, 96 board, etc. */
    uint32_t target;                            /**< 0: KL520, 1: KL720, etc. */
    uint32_t crc;                               /**< CRC value for all_models data */
    uint32_t kn_num;                            /**< KN number */
    uint32_t enc_type;                          /**< encrypt type */
    char* tc_ver;                               /**< toolchain version */
    char* compiler_ver;                         /**< compiler version */
    kp_nef_schema_version_t nef_schema_version; /**< nef schema version */
} kp_metadata_t;

/**
 * @brief nef info for nef model data: metadata / fw_info / all_models
 */
typedef struct
{
    uint32_t target;            /**< 0: KL520, 1: KL720, etc. */
    char *fw_info_addr;         /**< Address of fw_info part */
    uint32_t fw_info_size;      /**< Size of fw_info part */
    char *all_models_addr;      /**< Address of all_model part */
    uint32_t all_models_size;   /**< Size of all_model part */
} kp_nef_info_t;

/******************************************************************
 * [private] utils
 ******************************************************************/

void* realloc_zero(void* memory, size_t new_size);
void* strcpy_dst_realloc(char* dst_buff, const char* src_buff);

/******************************************************************
 * [private] setup_reader
 ******************************************************************/

int construct_single_setup_info(uintptr_t setup_buff, size_t setup_buff_size, kp_single_model_descriptor_t *single_model_descriptor);

/******************************************************************
 * [private] model_descriptor_builder
 ******************************************************************/

uint32_t* realloc_tensor_shape(uint32_t *shape, uint32_t element_num);
kp_tensor_descriptor_t* realloc_tensor_list(kp_tensor_descriptor_t *tensor_list, uint32_t element_num);
kp_quantized_fixed_point_descriptor_t* realloc_quantized_fixed_point_descriptor_list(kp_quantized_fixed_point_descriptor_t *quantized_fixed_point_descriptor_list, uint32_t element_num);

/******************************************************************
 * [public] setup_reader
 ******************************************************************/

uint32_t convert_data_format_to_kp_tensor_format(uint32_t data_format, uint32_t target_chip);

/******************************************************************
 * [public] kneron_nef_reader
 ******************************************************************/

int read_nef(char *nef_data, uint32_t nef_size, kp_metadata_t *metadata, kp_nef_info_t *nef_info);

/******************************************************************
 * [public] model_descriptor_builder
 ******************************************************************/

int deconstruct_model_nef_descriptor(kp_model_nef_descriptor_t* loaded_model_desc);
int build_model_nef_descriptor_from_nef(kp_metadata_t *metadata, kp_nef_info_t *nef_info, kp_model_nef_descriptor_t* loaded_model_desc);
int build_model_nef_descriptor_from_device(kp_nef_info_t *nef_info, kp_model_nef_descriptor_t* loaded_model_desc);
int load_model_info_from_nef(void *nef_buf, int nef_size, kp_product_id_t target_pid /* input */, kp_metadata_t *metadata, kp_nef_info_t *nef_info, kp_model_nef_descriptor_t *loaded_model_desc /* output */);

#endif
