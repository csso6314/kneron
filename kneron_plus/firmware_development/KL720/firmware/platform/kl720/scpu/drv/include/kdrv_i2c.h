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
/**@addtogroup  KDRV_I2C  KDRV_I2C
 * @{
 * @brief       Kneron I2C driver - Inter-integrated Circuit (I2C) Peripheral API.
 * @details     Here are the design highlight points:\n
 *              * At present it supports only 7-bit slave address \n
 *              * It is designed in polling way instead of interrupt way, user should be aware of this
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KDRV_I2C_H__
#define __KDRV_I2C_H__

#include "cmsis_os2.h"
#include "kdrv_status.h"
// I2C Control Register
#define REG_I2C_CR          0x00
#define CR_HSI_EN           BIT20   /* slave mode : detect HS-mode */
#define CR_SBI_EN           BIT21   /* slave mode : detect START byte */
#define CR_STARTI_EN        BIT14   /* start condition */
#define CR_ALIRQ            BIT13   /* arbitration lost interrupt (master) */
#define CR_SAMIRQ           BIT12   /* slave address match interrupt (slave) */
#define CR_STOPIRQ          BIT11   /* stop condition interrupt (slave) */
#define CR_NAKRIRQ          BIT10   /* NACK response interrupt (master) */
#define CR_DRIRQ            BIT9    /* TDI_EN rx interrupt (both) */
#define CR_DTIRQ            BIT8    /* tx interrupt (both) */
#define CR_TBEN             BIT7    /* tx enable (both) */
#define CR_NAK              BIT6    /* NACK (both) */
#define CR_STOP             BIT5    /* stop (master) */
#define CR_START            BIT4    /* start (master) */
#define CR_GCEN             BIT3    /* general call support (slave) */
#define CR_MST_EN           BIT2    /* enable clock out (master) */
#define CR_I2C_EN           BIT1    /* enable I2C (both) */
#define CR_I2C_RST          BIT0    /* reset I2C (both) */
#define CR_ENABLE           (CR_ALIRQ | CR_NAKRIRQ | CR_DRIRQ | CR_DTIRQ | CR_MST_EN | CR_I2C_EN)

// I2C Status Register
#define REG_I2C_SR          0x04
#define SR_SBS              BIT23   /* start byte */
#define SR_HSS              BIT22   /* high speed mode */
#define SR_START            BIT11   /* start */
#define SR_AL               BIT10   /* arbitration lost */
#define SR_GC               BIT9    /* general call */
#define SR_SAM              BIT8    /* slave address match */
#define SR_STOP             BIT7    /* stop received */
#define SR_NACK             BIT6    /* NACK received */
#define SR_TD               BIT5    /* transfer done */
#define SR_BB               BIT3    /* bus busy */
#define SR_I2CB             BIT2    /* chip busy */
#define SR_RW               BIT0    /* set when master-rx or slave-tx mode */

// I2C Clock Division Register
#define REG_I2C_CDR         0x08
#define CDR_COUNT_100KHZ \
    ((uint32_t)((APB_CLOCK) / (2 * 100000) - (GSR_VALUE / 2) - 2)) // if APB=100, this will be 496
#define CDR_COUNT_200KHZ \
    ((uint32_t)((APB_CLOCK) / (2 * 200000) - (GSR_VALUE / 2) - 2)) // if APB=100, this will be 246
#define CDR_COUNT_400KHZ \
    ((uint32_t)((APB_CLOCK) / (2 * 400000) - (GSR_VALUE / 2) - 2)) // if APB=100, this will be 121
#define CDR_COUNT_1MHZ \
    ((uint32_t)((APB_CLOCK) / (2 * 1000000) - (GSR_VALUE / 2) - 2)) // if APB=100, this will be 46

// I2C Data Register
#define REG_I2C_DR          0x0C

// I2C Address Register
#define REG_I2C_AR          0x10

// I2C Set/Hold Time and Glitch Suppresion Setting Register
#define REG_I2C_TGSR        0x14
#define GSR_VALUE           0x5
#define TSR_VALUE           0x5

// I2C Bus Monitor Register
#define REG_I2C_BMR         0x18

// I2C Burst Mode Register
#define REG_I2C_BSTMR       0x1C

// I2C Revision Register
#define REG_I2C_REVISION    0x30

//#define USE_EVENT_FLAG
#define I2C_INTERRUPT_ENABLE 1
#ifdef I2C_INTERRUPT_ENABLE
#define FLAGS_I2C_SLV_ALL_EVENTS    (SR_NACK | SR_TD | SR_SAM| SR_RW | SR_STOP )
#define FLAGS_I2C_MST_ALL_EVENTS    (SR_NACK | SR_AL | SR_TD | SR_GC)

typedef enum {
    I2C_SLV_Tx_HOST_READ,
    I2C_SLV_Rx_HOST_WRITE,
}i2c_slv_cb_event_t;

typedef void (*i2c_slv_cb_fr_isr_t) (i2c_slv_cb_event_t argument, uint32_t* arg);
typedef struct  {
    #ifdef USE_EVENT_FLAG
    osEventFlagsId_t        tid;
    #else
    osThreadId_t        tid;
    #endif
    i2c_slv_cb_fr_isr_t cb;
}kdrv_i2c_ctx_t;
#endif

/** @brief Enumerations of I2C bus speed */
typedef enum
{
    KDRV_I2C_SPEED_100K = 0, /**< Enum 0, Kdrv I2C bus speed 100KHz, standard mode */
    KDRV_I2C_SPEED_200K,     /**< Enum 1, Kdrv I2C bus speed 200KHz */
    KDRV_I2C_SPEED_400K,     /**< Enum 2, Kdrv I2C bus speed 400KHz, fast mode */
    KDRV_I2C_SPEED_1M        /**< Enum 3,  Kdrv I2C bus speed 1MHz, fast plus mode */
} kdrv_i2c_bus_speed_t;

/** @brief Enumerations of I2C controller instances */
typedef enum
{
    KDRV_I2C_CTRL_0 = 0,     /**< Enum 0, Kdrv I2C controller 0 */
    KDRV_I2C_CTRL_1,         /**< Enum 1, Kdrv I2C controller 1 */
    KDRV_I2C_CTRL_2,         /**< Enum 2, Kdrv I2C controller 2 */
    TOTAL_KDRV_I2C_CTRL      /**< Enum 3, Total Kdrv I2C controllers */
}kdrv_i2c_ctrl_t;

/** @brief Structure of I2C device instances */
typedef struct{
    uint32_t    i2c_port;       /**< I2C peripheral port*/
    uint32_t    i2c_speed;      /**< I2C clock speed */
    uint32_t    i2c_devaddr;    /**< I2C slave address */
    uint32_t    i2c_mode;       /**< I2C Master/Slave */
}i2c_attr_context;

/**
 * @brief       kdrv_i2c_slave_open
 *
 * @return      kdrv_status_t   see @ref kdrv_status_t
 *
 * @note        This API MUST be called before using the I2C slave read/write.
 */
kdrv_status_t kdrv_i2c_slave_open(i2c_slv_cb_fr_isr_t cb);
/**
 * @brief       Initializes Kdrv I2C driver (as master) and configures it for the specified speed.
 *
 * @return      kdrv_status_t   see @ref kdrv_status_t
 *
 * @note        This API MUST be called before using the Read/write APIs for I2C.
 */
kdrv_status_t kdrv_i2c_initialize(uint32_t num, i2c_attr_context* i2c_ctx);

/**
 * @brief       Uninitializes Kdrv I2C driver
 *
 * @param[in]   ctrl_id         see @ref kdrv_i2c_ctrl_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 *
 */
kdrv_status_t kdrv_i2c_uninitialize(kdrv_i2c_ctrl_t ctrl_id);

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
kdrv_status_t kdrv_i2c_write_register(kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint16_t reg, uint16_t reg_size, uint16_t len, uint16_t *data);

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
kdrv_status_t kdrv_i2c_read_register(kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr,
                                uint16_t reg, uint16_t reg_size, uint16_t len, uint16_t *data);

#endif
/** @}*/
