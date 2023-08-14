/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
*
* The information contained herein is property of Kneron, Inc.
* Terms and conditions of usage are described in detail in Kneron
* STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information.
* NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
* from the file.
*/

/* History:
 *  Version 2.00
 *    Renamed driver NOR -> Flash (more generic)
 *    Non-blocking operation
 *    Added Events, Status and Capabilities
 *    Linked Flash information (GetInfo)
 *  Version 1.11
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */

/**@addtogroup  KDEV_FLASH  KDEV_FLASH
 * @{
 * @brief       Kneron flash device
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KDEV_FLASH_H
#define __KDEV_FLASH_H

#include "Driver_Common.h"
#include "kdev_status.h"
#include "project.h"

#if defined(FLASH_TYPE) && (FLASH_TYPE == FLASH_TYPE_WINBOND_NAND)
#include "kdrv_SPI020_nand.h"
#include "kdrv_spif_nand.h"
#include "kdev_flash_nand.h"
#include "kdev_flash_winbond.h"
#elif defined(FLASH_TYPE) && (FLASH_TYPE == FLASH_TYPE_GIGADEVICE_NAND)
#include "kdrv_SPI020_nand.h"
#include "kdrv_spif_nand.h"
#include "kdev_flash_nand.h"
#include "kdev_flash_gd.h"
#else
#include "kdrv_SPI020.h"
#include "kdrv_spif.h"
#include "kdev_flash_nor.h"
#if defined(FLASH_TYPE) && (FLASH_TYPE == FLASH_TYPE_MXIC_NOR)
#include "kdev_flash_mxic.h"
#elif defined(FLASH_TYPE) && (FLASH_TYPE == FLASH_TYPE_GIGADEVICE_NOR)
#include "kdev_flash_gd.h"
#else
#include "kdev_flash_winbond.h"
#endif
#endif

#endif /* __KDEV_FLASH_H */
/** @}*/
