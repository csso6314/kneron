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

/**@addtogroup  KDEV_EEPROM  KDEV_EEPROM
* @{
* @brief        Kneron eeprom device interface
*
* @copyright    Copyright (C) 2020 Kneron, Inc. All rights reserved.
*/

#ifndef __KDEV_EEPROM_H__
#define __KDEV_EEPROM_H__

#include "kdev_status.h"
#include "regbase.h"
#include "kdrv_cmsis_core.h"

void kdev_eeprom_initialize(void);

void kdev_eeprom_uninitialize(void);
void kdev_eeprom_write(uint16_t reg_addr, uint8_t* data, uint16_t len);

void kdev_eeprom_read(uint16_t reg_addr, uint8_t* data, uint16_t len);

#endif /* __KDEV_EEPROM_H__ */
/** @}*/

