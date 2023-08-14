/**
 * @file        kmdw_utils_crc.h
 * @brief       Kneron crc driver header
 *
 * @copyright   Copyright (c) 2018 Kneron, Inc. All rights reserved.
 */

#ifndef __KMDW_UTILS_CRC_H__
#define __KMDW_UTILS_CRC_H__

#include "base.h"

#define CRC16_CONSTANT 0x8005 /**< CRC16 constant */
#define ENABLE_CRC32 0 /**< To enable CRC32 calculation or not */

/**
 * @brief generate crc16 code
 * 
 * @param[in] data input data
 * @param[in] size data size
 * 
 */
uint16_t kmdw_utils_crc_gen_crc16(uint8_t *data, uint32_t size);

/**
 * @brief generate sha32
 * 
 * @param[in] data input data
 * @param[in] size data size
 * 
 */
uint32_t kmdw_utils_crc_gen_sha32(uint8_t *data, uint32_t size);

/**
 * @brief generate sum32
 * 
 * @param[in] data input data
 * @param[in] size data size
 * 
 */
uint32_t kmdw_utils_crc_gen_sum32(uint8_t *data, uint32_t size);

/**
 * @brief generate crc32 code
 * 
 * @param[in] data input data
 * @param[in] size data size
 * 
 */
uint32_t kmdw_utils_crc_gen_crc32(uint8_t *data, uint32_t size);

#endif
