/*
 * Kneron System API
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#ifndef __KDP_SYSTEM_H__
#define __KDP_SYSTEM_H__

#include <stdint.h>

uint32_t kdp_sys_get_unique_id(void);
uint32_t kdp_sys_get_ncpu_version(void);
uint32_t kdp_sys_get_ncpu_build(void);
uint32_t kdp_sys_get_spl_version(void);
uint32_t kdp_sys_get_spl_build(void);
uint32_t kdp_sys_update_spl_image(uint32_t addr, uint32_t size);
uint32_t kdp_sys_get_key_status(void);
uint32_t kdp_sys_get_kn_number(void);
uint32_t kdp_sys_program_key(uint32_t cust_key);

#endif
