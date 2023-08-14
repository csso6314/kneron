/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/

#ifndef __KDEV_STATUS_H__
#define __KDEV_STATUS_H__

#include <stdbool.h> // for type 'bool', ture and false.
typedef enum
{
    KDEV_STATUS_OK = 0,                    /**< driver status OK */
    KDEV_STATUS_ECC_ERROR = 0x01,
    KDEV_STATUS_ECC_CORRECT = 0x02,
    KDEV_STATUS_PFAIL = 0x04,
    KDEV_STATUS_EFAIL = 0x08,
    KDEV_STATUS_ERROR = 0xFF,                     /**< driver status error */
} kdev_status_t;

#endif /* __KDEV_STATUS_H__ */
