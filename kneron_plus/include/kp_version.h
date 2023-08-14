/**
 * @file        kp_version.h
 * @brief       Kneron PLUS version
 * @version     0.1
 * @date        2021-08-30
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

/**
 * @brief firmware version index.
 */
typedef enum
{
    VERSION_INDEX_MAJOR = 0,
    VERSION_INDEX_MINOR = 1,
    VERSION_INDEX_REVISION = 2,
    VERSION_INDEX_BUILD = 3,
} fw_version_index_t;

static const char plus_version[] = "2.1.1";
static const int kl520_fw_version[4] = {2, 1, 0, 118};
static const int kl720_fw_version[4] = {2, 1, 0, 118};
static const int kl630_fw_version[4] = {1, 0, 0, 118};
