/**
 * @file        kp_set_key.h
 * @brief       internal set key functions
 * @version     1.1
 * @date        2021-09-08
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef __KP_SET_KEY_H__
#define __KP_SET_KEY_H__

#pragma once

#include "kp_struct.h"

/* KL520 & KL720 */
int kp_set_ckey(kp_device_group_t devices, uint32_t ckey);

/* KL720 */
int kp_set_secure_boot_key(kp_device_group_t devices, uint32_t entry, uint32_t key);

/* KL720 */
int kp_set_gpio(kp_device_group_t devices, uint32_t pin, uint32_t value);

#endif // __KP_SET_KEY_H__