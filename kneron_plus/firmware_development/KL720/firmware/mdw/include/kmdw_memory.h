/**
 * @file        kmdw_memory.h
 * @brief       ddr memory access APIs
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_MEMORY_H_
#define __KMDW_MEMORY_H_

#include <stdint.h>

#define ALIGN_16BYTE    16   /**< 16 byte alignment annotation */
#define ALIGN_64BYTE    64   /**< 64 byte aligement annotation */

#define ALIGN16(n)          ((n + ALIGN_16BYTE - 1) & ~( ALIGN_16BYTE - 1))
#define ALIGN16_FLOOR(n)    ((n) & ~(ALIGN_16BYTE - 1))

#define ALIGN64(n)         ((n + ALIGN_64BYTE - 1) & ~(ALIGN_64BYTE - 1))
#define ALIGN64_FLOOR(n)   ((n) & ~(ALIGN_64BYTE - 1))

/**
 * @brief To initialize available DDR block
 * @param start_addr the start address of DDR block
 * @param end_addr the end address of DDR block
 */
void kmdw_ddr_init(uint32_t start_addr, uint32_t end_addr);

/**
 * @brief To adjust available DDR boundary based on model usage
 * @param boundary the boundary addr of DDR block
 * @return 0: success; -1: failed
 */
int kmdw_ddr_set_ddr_boundary(uint32_t boundary);

/**
 * @brief to allocate DDR memory aligned at 64 bytes
 * @param numbtye size in byte
 * @return the address of allocated block
 */
uint32_t kmdw_ddr_reserve(uint32_t numbyte);

/**
 * @brief to get available DDR block tail address
 * @return uint32_t
 */
uint32_t kmdw_ddr_get_heap_tail(void);

/**
 * @brief To store DDR block for system reserve
 *
 * @param start_addr the start address of system reserve
 * @param end_addr the end address of system reserve
 */
void kmdw_ddr_store_system_reserve(uint32_t start_addr, uint32_t end_addr);

/**
 * @brief To get DDR block and size of the system reserve
 * @param start_addr [output] the start address of the system reserve
 * @param end_addr [output] the size of the system reserve
 */
void kmdw_ddr_get_system_reserve(uint32_t *start_addr, uint32_t *ddr_size);

#endif

