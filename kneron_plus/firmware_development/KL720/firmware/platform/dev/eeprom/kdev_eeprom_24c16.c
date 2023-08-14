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

/******************************************************************************
*  Filename:
*  ---------
*  kdev_eeprom.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  For kl720 evaluation board varify
*
******************************************************************************/
#include "kdrv_i2c.h"
#include "project.h"
static i2c_attr_context i2c_attr_ctx[I2C_NUM] = I2C_ATTR_ARRAY;
#define EEPROM_ADDR             i2c_attr_ctx[0].i2c_devaddr
extern kdrv_status_t kdrv_i2c_set_attribute(i2c_attr_context* ctx);
void kdev_eeprom_initialize(void)
{
}

void kdev_eeprom_uninitialize(void)
{
}

void kdev_eeprom_write(uint16_t reg_addr, uint16_t* data, uint16_t len)
{
    reg_addr = reg_addr & 0x7FF;
    kdrv_i2c_write_register(  (kdrv_i2c_ctrl_t)EEPROM, (EEPROM_ADDR | ((reg_addr & 0x700)>>8 )), (reg_addr & 0xFF), 1, len, data);
    //kdrv_i2c_write_register(  (kdrv_i2c_ctrl_t)EEPROM, (EEPROM_ADDR), (reg_addr & 0xFF), 1, len, data);
}

void kdev_eeprom_read(uint16_t reg_addr, uint16_t* data, uint16_t len)
{
    reg_addr = reg_addr & 0x7FF;
    kdrv_i2c_read_register((kdrv_i2c_ctrl_t)EEPROM, (EEPROM_ADDR | ((reg_addr & 0x700)>>8 )), (reg_addr & 0xFF), 1, len, data);
    //kdrv_i2c_read_register((kdrv_i2c_ctrl_t)EEPROM, (EEPROM_ADDR), (reg_addr & 0xFF), 1, len, data);
}

void kdev_eeprom_changespeed(kdrv_i2c_bus_speed_t bus_speed)
{
    i2c_attr_ctx[EEPROM].i2c_speed = bus_speed;
    kdrv_i2c_set_attribute(i2c_attr_ctx);
}

