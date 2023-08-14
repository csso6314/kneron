/*
 * Kneron Peripheral API - I2C
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include "kdrv_scu.h"
#include "kdrv_clock.h" // for kdrv_delay_us
//#include "kdp_io.h"
#include "kdrv_i2c.h"
//#include "kmdw_console.h"
#include "system_config.h"

#define I2C_ENABLE_THREAD_SYNC // enable I2C thread synchronization
//#define I2C_DEBUG

/************************************************************************
*                 	 I2C Register Releated Macros                     
************************************************************************/
#define I2CRegRead(id, reg_offset) inw(IIC_FTIIC010_0_PA_BASE + id * 0x100000 + reg_offset)
#define I2CRegWrite(id, reg_offset, val) outw(IIC_FTIIC010_0_PA_BASE + id * 0x100000 + reg_offset, val)
#define I2CRegMaskedSet(id, reg_offset, val) masked_outw(IIC_FTIIC010_0_PA_BASE + id * 0x100000 + reg_offset, val, val)
#define I2CRegMaskedClr(id, reg_offset, val) masked_outw(IIC_FTIIC010_0_PA_BASE + id * 0x100000 + reg_offset, 0, val)

// I2C Control Register
#define REG_I2C_CR 0x00
#define CR_ALIRQ BIT13   /* arbitration lost interrupt (master) */
#define CR_SAMIRQ BIT12  /* slave address match interrupt (slave) */
#define CR_STOPIRQ BIT11 /* stop condition interrupt (slave) */
#define CR_NAKRIRQ BIT10 /* NACK response interrupt (master) */
#define CR_DRIRQ BIT9    /* rx interrupt (both) */
#define CR_DTIRQ BIT8    /* tx interrupt (both) */
#define CR_TBEN BIT7     /* tx enable (both) */
#define CR_NAK BIT6      /* NACK (both) */
#define CR_STOP BIT5     /* stop (master) */
#define CR_START BIT4    /* start (master) */
#define CR_GCEN BIT3     /* general call support (slave) */
#define CR_MST_EN BIT2   /* enable clock out (master) */
#define CR_I2C_EN BIT1   /* enable I2C (both) */
#define CR_I2C_RST BIT0  /* reset I2C (both) */
#define CR_ENABLE \
    (CR_ALIRQ | CR_NAKRIRQ | CR_DRIRQ | CR_DTIRQ | CR_MST_EN | CR_I2C_EN)

// I2C Status Register
#define REG_I2C_SR 0x04
#define SR_SBS BIT23   /* start byte */
#define SR_HSS BIT22   /* high speed mode */
#define SR_START BIT11 /* start */
#define SR_AL BIT10    /* arbitration lost */
#define SR_GC BIT9     /* general call */
#define SR_SAM BIT8    /* slave address match */
#define SR_STOP BIT7   /* stop received */
#define SR_NACK BIT6   /* NACK received */
#define SR_TD BIT5     /* transfer done */
#define SR_BB BIT3     /* bus busy */
#define SR_I2CB BIT2   /* chip busy */
#define SR_RW BIT0     /* set when master-rx or slave-tx mode */

// I2C Clock Division Register
#define REG_I2C_CDR 0x08
#define CDR_COUNT_100KHZ \
    ((uint32_t)((APB_CLOCK) / (2 * 100000) - (GSR_VALUE / 2) - 2)) // if APB=100, this will be 496
#define CDR_COUNT_400KHZ \
    ((uint32_t)((APB_CLOCK) / (2 * 400000) - (GSR_VALUE / 2) - 2)) // if APB=100, this will be 121
#define CDR_COUNT_1MHZ \
    ((uint32_t)((APB_CLOCK) / (2 * 1000000) - (GSR_VALUE / 2) - 2)) // if APB=100, this will be 46

// I2C Data Register
#define REG_I2C_DR 0x0C

// I2C Address Register
#define REG_I2C_AR 0x10

// I2C Set/Hold Time and Glitch Suppresion Setting Register
#define REG_I2C_TGSR 0x14
#define GSR_VALUE 0x5
#define TSR_VALUE 0x5

// I2C Bus Monitor Register
#define REG_I2C_BMR 0x18

// I2C Burst Mode Register
#define REG_I2C_BSTMR 0x1C

// I2C Revision Register
#define REG_I2C_REVISION 0x30

#ifdef I2C_DEBUG
void i2c_dump_register(kdrv_i2c_ctrl_t ctrl_id)
{
    info_msg("APB clock = %d\n", APB_CLOCK);
    info_msg("(0x00) control register = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_CR));
    info_msg("(0x04) status register = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_SR));
    info_msg("(0x08) clock division = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_CDR));
    info_msg("(0x0C) data register = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_DR));
    info_msg("(0x10) address register = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_AR));
    info_msg("(0x14) TGSR register = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_TGSR));
    info_msg("(0x18) bus monitor = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_BMR));
    info_msg("(0x1C) burst mode = 0x%x\n", I2CRegRead(ctrl_id, REG_I2C_BSTMR));
    info_msg("(0x30) revision = 0x%x\n\n", I2CRegRead(ctrl_id, REG_I2C_REVISION));
}
#endif

static void i2c_power(kdrv_i2c_ctrl_t ctrl_id, int on)
{
    uint32_t mask, val;

    mask = SCU_REG_APBCLKG_PCLK_EN_I2C0_PCLK << ctrl_id;
    if (on)
        val = mask;
    else
        val = 0;

    masked_outw(SCU_REG_APBCLKG, val, mask);
    kdrv_delay_us(500);
}

static kdrv_status_t i2c_polling_completion(kdrv_i2c_ctrl_t ctrl_id, bool ignore_NAK)
{
    // polling SR and take actions correspondingly
    for (int i = 0; i < 10000; i++)
    {
        uint32_t sr_status = I2CRegRead(ctrl_id, REG_I2C_SR);
        I2CRegWrite(ctrl_id, REG_I2C_SR, sr_status);

        if (!ignore_NAK && (sr_status & SR_NACK))
            return KDRV_STATUS_I2C_DEVICE_NACK;
        else if (sr_status & (SR_AL))
            return KDRV_STATUS_I2C_BUS_BUSY;
        else if (sr_status & SR_TD)
            return KDRV_STATUS_OK;
    }
    return KDRV_STATUS_I2C_TIMEOUT;
}

static bool i2c_waiting_for_bus_available(kdrv_i2c_ctrl_t ctrl_id)
{
    for (int i = 0; i < 10000; i++)
    {
        if (I2CRegRead(ctrl_id, REG_I2C_SR) & SR_BB)
            continue;
        else
            return true;
    }
    return false;
}

// data1 and num1 are first part of data
// data2 and num2 are first part of data
// doing this is for kdrv_i2c_write_register(), kind of workaround way of implementation
// data 1 must not be NULL, num1 must be greater than 0
// data 2 can be NULL, num2 can be 0 if no 2nd part of data
static kdrv_status_t i2c_tx_rx(kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr,
                               uint8_t *data1, uint32_t num1, uint8_t *data2, uint32_t num2, bool isRead, bool with_STOP)
{
    kdrv_status_t status = KDRV_STATUS_OK;
    bool ignore_NAK = false;

#ifdef I2C_ENABLE_THREAD_SYNC
    static osMutexId_t mutex_i2c = NULL;

    if (mutex_i2c == NULL)
    {
        mutex_i2c = osMutexNew(NULL);
#ifdef I2C_DEBUG
        if (mutex_i2c == NULL)
            kmdw_printf("kdrv_i2c: %s: create mutex failed\n", __FUNCTION__);
#endif
    }

    osStatus_t osts = osMutexAcquire(mutex_i2c, 0);
#ifdef I2C_DEBUG
    if (osts != osOK)
        kmdw_printf("kdrv_i2c: %s: mutex lock failed, osStatus_t error = %d\n", __FUNCTION__, status);
#endif
#endif

    do
    {
        if (!i2c_waiting_for_bus_available(ctrl_id))
        {
            status = KDRV_STATUS_I2C_BUS_BUSY;
            break;
        }

        // send slave address and check status

        // write address byte into DR
        // TODO: support 10-bit address
        uint8_t byte_addr = (uint8_t)((slave_addr << 1) & 0xFE);
        if (isRead)
            byte_addr |= 0x1;

        I2CRegWrite(ctrl_id, REG_I2C_DR, byte_addr);

        // send out slave address with START condition by setting CR
        I2CRegWrite(ctrl_id, REG_I2C_CR, (CR_I2C_EN | CR_MST_EN | CR_TBEN | CR_START));

        // wait for complete status
        status = i2c_polling_completion(ctrl_id, ignore_NAK);
        if (status != KDRV_STATUS_OK)
        {
            if (status == KDRV_STATUS_I2C_DEVICE_NACK)
                I2CRegMaskedSet(ctrl_id, REG_I2C_CR, CR_STOP);

            break;
        }

        if (!i2c_waiting_for_bus_available(ctrl_id))
        {
            status = KDRV_STATUS_I2C_BUS_BUSY;
            break;
        }

        uint32_t num = num1 + num2;
        uint8_t *data = data1;

        // send or receive data
        for (int i = 0; i < num; i++)
        {
            ignore_NAK = false;

            uint32_t ctrl_flag = (CR_I2C_EN | CR_MST_EN | CR_TBEN);
            if (with_STOP && (i == (num - 1)))
            {
                ctrl_flag |= CR_STOP;
                if (isRead)
                {
                    ctrl_flag |= CR_NAK;
                    ignore_NAK = true;
                }
            }

            if (!isRead) // write data to the DR
                I2CRegWrite(ctrl_id, REG_I2C_DR, *data);

            // start transmission
            I2CRegWrite(ctrl_id, REG_I2C_CR, ctrl_flag);

            // check status
            // wait for complete status
            status = i2c_polling_completion(ctrl_id, ignore_NAK);
            if (status != KDRV_STATUS_OK)
            {
                if (status == KDRV_STATUS_I2C_DEVICE_NACK)
                    I2CRegMaskedSet(ctrl_id, REG_I2C_CR, CR_STOP);

                break;
            }

            if (isRead) // read data
                *data = (uint8_t)(0xFF & I2CRegRead(ctrl_id, REG_I2C_DR));

            ++data;
            // switch to 2nd part of data buffer, so strange ...
            if ((i + 1) == num1)
                data = data2;
        }

    } while (0);

#ifdef I2C_ENABLE_THREAD_SYNC
    osMutexRelease(mutex_i2c);
#endif
    return status;
}

/*******************************************************************/
/***                		 Public APIs		                 ***/
/*******************************************************************/
kdrv_status_t kdrv_i2c_initialize(kdrv_i2c_ctrl_t ctrl_id, kdrv_i2c_bus_speed_t bus_speed)
{
    if ((ctrl_id >= TOTAL_KDRV_I2C_CTRL) || (bus_speed > KDRV_I2C_SPEED_1M))
        return KDRV_STATUS_INVALID_PARAM;

    i2c_power(ctrl_id, 1);

    // reset i2c controller
    I2CRegMaskedSet(ctrl_id, REG_I2C_CR, CR_I2C_RST);

    int i;
    for (i = 0; i < 500; i++)
        if (!(I2CRegRead(ctrl_id, REG_I2C_CR) & CR_I2C_RST))
            break;

    if (i >= 500)
    {
        return KDRV_STATUS_ERROR;
    }

    // set bus speed, SCL = PCLK / (2 * COUNT  + GSR + 4)

    // set GSR and TSR value
    I2CRegWrite(ctrl_id, REG_I2C_TGSR, (GSR_VALUE << 10) | TSR_VALUE);

    // select CDR Count value depending on bus speed
    uint32_t count_bits;
    switch (bus_speed)
    {
    case KDRV_I2C_SPEED_100K:
        count_bits = CDR_COUNT_100KHZ;
        break;
    case KDRV_I2C_SPEED_400K:
        count_bits = CDR_COUNT_400KHZ;
        break;
    case KDRV_I2C_SPEED_1M:
        count_bits = CDR_COUNT_1MHZ;
        break;
    default:
        // could be user specified clock in Hz
        count_bits = ((APB_CLOCK) / (2 * (uint32_t)bus_speed) - (GSR_VALUE / 2) - 2);
        break;
    }
    I2CRegWrite(ctrl_id, REG_I2C_CDR, count_bits); // not care about COUNTH and DUTY bits
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_i2c_uninitialize(kdrv_i2c_ctrl_t ctrl_id)
{
    i2c_power(ctrl_id, 0);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdp_i2c_transmit(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint8_t *data, uint32_t num, bool with_STOP)
{
    return i2c_tx_rx(ctrl_id, slave_addr, data, num, NULL, 0, false, with_STOP);
}

kdrv_status_t kdp_i2c_receive(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint8_t *data, uint32_t num, bool with_STOP)
{
    return i2c_tx_rx(ctrl_id, slave_addr, data, num, NULL, 0, true, with_STOP);
}

kdrv_status_t kdrv_i2c_write_register(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint16_t reg, uint16_t reg_size, uint16_t len, uint8_t *data)
{
    uint8_t reg_data[2];
    int32_t i = 0;

    if (reg_size == 2)
        reg_data[i++] = reg >> 8; // store MSB of reg
    reg_data[i++] = reg & 0xff;

    return i2c_tx_rx(ctrl_id, slave_addr, reg_data, reg_size, data, len, false, true);
}

kdrv_status_t kdrv_i2c_read_register(
    kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr, uint16_t reg, uint16_t reg_size, uint16_t len, uint8_t *data)
{
    uint8_t reg_data[2];
    int32_t i = 0;

    if (reg_size == 2)
        reg_data[i++] = reg >> 8; // store MSB of reg
    reg_data[i++] = reg & 0xff;

    // first write register address to the device without STOP condition
    kdrv_status_t ret = i2c_tx_rx(ctrl_id, slave_addr, reg_data, reg_size, NULL, 0, false, false);

    // then read data from device
    if (ret == KDRV_STATUS_OK)
        ret = i2c_tx_rx(ctrl_id, slave_addr, data, len, NULL, 0, true, true);

    return ret;
}
