/**
  * @file        kdrv_i2c.h
  * @brief       Kneron I2C driver
  * @details     Here are the design highlight points:\n
  *              * At present it supports only 7-bit slave address \n
  *              * It is designed in polling way instead of interrupt way, user should be aware of this
  * @version     1.0
  * @copyright   (c) 2020 Kneron Inc. All right reserved.
  */
#ifndef __KDRV_I2C_H__
#define __KDRV_I2C_H__

#include "cmsis_os2.h"
#include "kdrv_status.h"

/** 
 * @brief Enumerations of I2C bus speed
 */ 
typedef enum
{
    KDRV_I2C_SPEED_100K = 0, /**< Kdrv I2C bus speed 100KHz, standard mode */
    KDRV_I2C_SPEED_200K,     /**< Kdrv I2C bus speed 200KHz */
    KDRV_I2C_SPEED_400K,     /**< Kdrv I2C bus speed 400KHz, fast mode */
    KDRV_I2C_SPEED_1M        /**< Kdrv I2C bus speed 1MHz, fast plus mode */
} kdrv_i2c_bus_speed_t;

/** 
 * @brief Enumerations of I2C controller instances
 */
typedef enum
{
    KDRV_I2C_CTRL_0 = 0,     /**< Kdrv I2C controller 0 */
    KDRV_I2C_CTRL_1,         /**< Kdrv I2C controller 1 */
    KDRV_I2C_CTRL_2,         /**< Kdrv I2C controller 2 */
    KDRV_I2C_CTRL_3,         /**< Kdrv I2C controller 3 */
    TOTAL_KDRV_I2C_CTRL      /**< Total Kdrv I2C controllers */
}kdrv_i2c_ctrl_t;

/**
 * @brief       Initializes Kdrv I2C driver (as master) and configures it for the specified speed.
 *
 * @param[in]   ctrl_id         see @ref kdrv_i2c_ctrl_t
 * @param[in]   bus_speed       see @ref kdrv_i2c_bus_speed_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 *
 * @note        This API MUST be called before using the Read/write APIs for I2C.
 */
kdrv_status_t kdrv_i2c_initialize(kdrv_i2c_ctrl_t ctrl_id, kdrv_i2c_bus_speed_t bus_speed);

/**
 * @brief       Uninitializes Kdrv I2C driver
 *
 * @param[in]   ctrl_id         see @ref kdrv_i2c_ctrl_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 *
 */
kdrv_status_t kdrv_i2c_uninitialize(kdrv_i2c_ctrl_t ctrl_id);

/**
 * @brief       transmit data to a specified slave address, the STOP condition can be optionally not generated.\n
 *
 * @details     This function will first set START condition then send slave address for write operations; \n
 *              if 9th bit is NACK, it returns DEV_NACK error, and if it is ACK, \n
 *              controller will continue to send out all data with specified number of bytes, \n
 *              once it is done it will set STOP condition while the 'with_STOP' is KDP_BOOL_TRUE.\n
 *              For every byte transmission, it returns DEV_NACK error while encountering NACK at 9th bit.
 *
 * @param[in]   ctrl_id         see @ref kdrv_i2c_ctrl_t
 * @param[in]   slave_addr      Address of the slave(7-bit by default)
 * @param[in]   data            data buffer address
 * @param[in]   num             Length of data to be written (in bytes)
 * @param[in]   with_STOP       STOP condition will be generated or not
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdp_i2c_transmit(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint8_t *data, uint32_t num, bool with_STOP);

/**
 * @brief       receive data from a specified slave address, the STOP condition can be optionally not generated.
 *
 * @details     This function will first set START condition then send slave address for write operations;\n
 *              if 9th bit is NACK, it returns DEV_NACK error, and if it is ACK, \n
 *              controller will continue to send out all data with specified number of bytes,\n
 *              once it is done it will set STOP condition while the 'with_STOP' is KDP_BOOL_TRUE.\n
 *              For every byte transmission, it returns DEV_NACK error while encountering NACK at 9th bit
 *
 * @param[in]   ctrl_id         see @ref kdrv_i2c_ctrl_t
 * @param[in]   slave_addr      Address of the slave(7-bit by default)
 * @param[out]  data            data buffer address
 * @param[in]   num             Length of data to be written (in bytes)
 * @param[in]   with_STOP       STOP condition will be generated or not
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdp_i2c_receive(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint8_t *data, uint32_t num, bool with_STOP);

/**
 * @brief       specialized function to write to the register of slave device, register address can be 1 or 2 bytes.
 *
 * @param[in]   ctrl_id         see @ref kdrv_i2c_ctrl_t
 * @param[in]   slave_addr      Address of the slave(7-bit by default)
 * @param[in]   reg             Register address
 * @param[in]   reg_size        Length of register address
 * @param[in]   len             Length of data to be written (in bytes).
 * @param[in]   data            data write register value
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_i2c_write_register(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint16_t reg, uint16_t reg_size, uint16_t len, uint8_t *data);

/**
 * @brief       specialized function to read from the register of slave device, register address can be 1 or 2 bytes.
 *
 * @param[in]   ctrl_id         see @ref kdrv_i2c_ctrl_t
 * @param[in]   slave_addr      Address of the slave(7-bit by default)
 * @param[in]   reg             Register address
 * @param[in]   reg_size        Length of register address
 * @param[in]   len             Length of data to be read (in bytes).
 * @param[out]  data            data buffer to read register value
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_i2c_read_register(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint16_t reg, uint16_t reg_size, uint16_t len, uint8_t *data);

#endif
