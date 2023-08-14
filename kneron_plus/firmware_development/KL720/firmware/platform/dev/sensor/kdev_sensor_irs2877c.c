/*
 * Kneron irs2877c sensor driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include "project.h"
#include "kdev_sensor.h"
#include "kdrv_i2c.h"
#include "kdrv_gpio.h"
#include "kdrv_clock.h"
#include "kmdw_console.h"
#include "kdrv_timer.h"
//#define irs2877c_DBG
#ifdef irs2877c_DBG
#define sensor_msg(fmt, ...) kmdw_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define sensor_msg(fmt, ...)
#endif

#if ((IMGSRC_0_SENSORID == SENSOR_ID_IRS2877C) || (IMGSRC_1_SENSORID == SENSOR_ID_IRS2877C))
#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))
static const struct sensor_datafmt_info irs2877c_colour_fmts[] = {
    { PIX_FMT_YCBCR,    COLORSPACE_YUV },
    { PIX_FMT_RGB565,   COLORSPACE_RGB },
};

static struct sensor_device irs2877c_dev = {
    .addr = 0x3D,
};

struct sensor_init_seq irs2877c_init_regs[] = {
    //init sequence
    {0xA007, 0x1313},
    {0xA008, 0x1313},
    {0xA039, 0x16A1},
    {0xA03A, 0x5555},
    {0xA03B, 0x0005},
    {0xA03C, 0x0000},
    {0xA03D, 0x04D0},
    {0xA03E, 0x0000},
    {0xA03F, 0x000F},
    {0xA058, 0x0A08},
    {0xA05B, 0x7422},
    {0x9000, 0x2828},
    {0x9002, 0x539c},
    {0x9004, 0x539c},
    {0x9006, 0x539c},
    {0x9008, 0x539c},
    {0x9080, 0x2828},
    {0x9082, 0x00A0},
    {0x9083, 0x00A0},
    {0x9084, 0x0000},
    {0x9085, 0x9091},
    {0x9087, 0x4100},
    {0x9088, 0x0000},
    {0x9089, 0x0000},
    {0x908A, 0x9091},
    {0x908C, 0x4100},
    {0x908D, 0x0000},
    {0x908E, 0x0003},
    {0x908F, 0x9091},
    {0x9091, 0x4100},
    {0x9092, 0x0000},
    {0x9093, 0x0006},
    {0x9094, 0x9091},
    {0x9096, 0x4100},
    {0x9097, 0x0000},
    {0x9098, 0xC009},
    {0x91C0, 0x0592},
    {0x91C1, 0x0000},
    {0x91C2, 0x027F},
    {0x91C3, 0xEF00},
    {0x91C4, 0x2140},
    {0x91C6, 0x0042},
    {0x91D1, 0x0008},
    {0x91D2, 0x0020},
    {0x91D3, 0x2010},
    {0x91D4, 0x0835},
    {0x91DB, 0x0005},
    {0x91DF, 0x0C35},
    {0x91E7, 0x0008},
    {0x91F6, 0x16A1},
    {0x91F7, 0x1EB8},
    {0x91F8, 0x0005},
    {0x91F9, 0x3F01},
    {0x91FA, 0x0000},
    {0x91FB, 0x04A2},
    {0x91FC, 0x0044},
    {0x91FD, 0x0028},
    {0x922C, 0x000B},
    {0x922D, 0x0FC0},
    {0x9250, 0x000B},
    {0x9251, 0x0F00},
    {0x9274, 0x0202},
    {0x9284, 0x0602},
    {0x9294, 0x024B},
    {0x9295, 0x024F},
    {0x9296, 0x024F},
    {0x9297, 0x024B},
    {0x9298, 0x024F},
    {0x9299, 0x024F},
    {0x929A, 0x024B},
    {0x929B, 0x024F},
    {0x929C, 0x024F},
    {0x929D, 0x024B},
    {0x929E, 0x024F},
    {0x929F, 0x024F},
    {0x92A0, 0x0120},
    {0x92A1, 0x0130},
    {0x92A2, 0x0120},
    {0x92A3, 0x0130},
    {0x92A4, 0x0120},
    {0x92A5, 0x0130},
    {0x92A6, 0x0120},
    {0x92A7, 0x0130},
    {0x92A8, 0x008A},
    {0x92A9, 0x00B6},
    {0x92AA, 0x0080},
    {0x92AB, 0x0080},
    {0x92AC, 0x00C0},
    {0x92AD, 0x0080},
    {0x92AE, 0x0080},
    {0x92AF, 0x0080},
    {0x92B0, 0x0040},
    {0x92B1, 0x0040},
    {0x92B2, 0x0040},
    {0x92B3, 0x0040},
    {0x92B4, 0x0040},
    {0x92B5, 0x0040},
    {0x92B6, 0x0040},
    {0x92B7, 0x0040},
    {0x92B8, 0x0040},
    {0x92B9, 0x0040},
    {0x92BA, 0x0040},
    {0x92BB, 0x0040},
    {0x92BC, 0x0040},
    {0x92BD, 0x0040},
    {0x92BE, 0x0040},
    {0x92BF, 0x0040},
    {0x92C0, 0x0040},
    {0x92C1, 0x0040},
    {0x92C2, 0x0040},
    {0x92C3, 0x0040},
    {0x92C4, 0x0040},
    {0x92C5, 0x0040},
    {0x92C6, 0x0040},
    {0x92C7, 0x0040},
    {0x92C8, 0x0040},
    {0x92C9, 0x0040},
    {0x92CA, 0x0040},
    {0x92CB, 0x0040},
    {0x92CC, 0x0040},
    {0x92CD, 0x0040},
    {0x92CE, 0x0040},
    {0x92CF, 0x0040},
    {0x92D0, 0x2D16},
    {0x92D1, 0x5B44},
    {0x92D2, 0x8972},
    {0x92D3, 0xB6A0},
    {0x92D4, 0x442D},
    {0x92D5, 0x715B},
    {0x92D6, 0x9282},
    {0x92D7, 0x00A2},
    {0x92D8, 0x2F22},
    {0x92D9, 0x453A},
    {0x92DA, 0x5A50},
    {0x92DB, 0x0063},
    {0x92DC, 0x7455},
    {0x92DD, 0xAD92},
    {0x92DE, 0xE1C8},
    {0x92DF, 0x00FA},
    {0x92E0, 0x251A},
    {0x92E1, 0x372E},
    {0x92E2, 0x4840},
    {0x92E3, 0x0051},
    {0x92E4, 0x2821},
    {0x92E5, 0x342E},
    {0x92E6, 0x3E39},
    {0x92E7, 0x0044},
    {0x92E8, 0x4735},
    {0x92E9, 0x6A59},
    {0x92EA, 0x8D7C},
    {0x92EB, 0xB4A0},
    {0x92EC, 0xDCC8},
    {0x92ED, 0x00F0},
    {0x92EE, 0x1040},
    {0x92EF, 0x00EB},
    {0x92F0, 0x0087},
    {0x92F1, 0x0051},
    {0x92F2, 0x0013},
    {0x92F3, 0x0004},
    {0x92F4, 0x00A5},
    {0x92F5, 0x0300},
    {0x92F6, 0x0000},
    {0x92F7, 0x0000},
    {0x92F8, 0x0000},
    {0x92F9, 0x0000},
    {0x92FA, 0x0000},
    {0x92FB, 0x0000},
    {0x92FC, 0x00B7},
    {0x92FD, 0x67AF},
    {0x9401, 0x0002},
    {0x980A, 0xFEFF},
    {0x980C, 0x3F00},
    {0x980D, 0x3F00},
    {0x0000, 0x0000},
};
static uint32_t kdev_sensor_get_dev_id(void);
static void rgb_set_aec_roi(struct cam_sensor_aec *aec_p)
{
    return;
}

static void rgb_get_lux(uint16_t* exposure, uint8_t* pre_gain, uint8_t* post_gain, uint8_t* global_gain, uint8_t* y_average)
{
    return;
}

static uint32_t irs2877c_write_reg(struct sensor_device *sensor_dev, uint16_t reg, uint16_t data)
{
    uint32_t ret;
    uint16_t data_ctx;
    data_ctx = (data&0xFF00)>>8|(data&0xff)<<8;
    ret = kdrv_i2c_write_register((kdrv_i2c_ctrl_t)sensor_dev->i2c_port, sensor_dev->addr, reg, 2, 2, &data_ctx);
    return ret;
}

static uint32_t irs2877c_read_reg(struct sensor_device *sensor_dev, uint16_t reg, uint16_t *data)
{
    uint32_t ret;

    kdrv_i2c_read_register((kdrv_i2c_ctrl_t)sensor_dev->i2c_port, sensor_dev->addr, reg, 2, 2, data);

    ret = (*data & 0xFF)<<8 | (*data & 0xFF00)>>8;
    return ret;
}

uint32_t irs2877c_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;

    for (init_fnc_ptr = seq; ; ++init_fnc_ptr)
    {
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0)
            break;
        //sensor_msg("addr =\t%.4x, data = =\t%.4x\n", init_fnc_ptr->addr, (init_fnc_ptr->value & 0xFF));
        irs2877c_write_reg(dev, init_fnc_ptr->addr , (uint16_t)(init_fnc_ptr->value));

    }

    uint32_t data = kdev_sensor_get_dev_id();
    sensor_msg("irs2877c_init sensor id=%x\n", data);
    return 0;
}

static uint32_t irs2877c_set_params(struct sensor_device *sensor_dev)
{
    sensor_msg("   <%s>\n", __func__);
    /* initialize the sensor with default settings */
    return 0;
}

static kdev_status_t kdev_sensor_init(void)
{
    sensor_msg("   <%s>\n", __func__);
    irs2877c_init(&irs2877c_dev, irs2877c_init_regs);

    return KDEV_STATUS_OK;
}
static kdev_status_t kdev_sensor_power(uint32_t on)
{
    sensor_msg("   <%s>\n", __func__);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_reset()
{
    sensor_msg("   <%s>\n", __func__);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_stream(uint32_t enable)
{
    uint16_t data = (uint16_t)enable;
    sensor_msg("   <%s>\n", __func__);

    irs2877c_write_reg(&irs2877c_dev, 0x9400 ,  data);
    
    data = 0;

    uint16_t id = irs2877c_read_reg(&irs2877c_dev, 0x9400  , &data);
    sensor_msg("01 MIPI_enable value %d\n", id);
    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_enum_fmt(uint32_t index, uint32_t *code)
{
    if (index >= ARRAY_SIZE(irs2877c_colour_fmts))
        return KDEV_STATUS_ERROR;

    sensor_msg("   <%s>\n", __func__);
    *code = irs2877c_colour_fmts[index].fourcc;
    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_get_fmt(cam_format *format)
{
    sensor_msg("   <%s>\n", __func__);
    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_fmt(cam_format *fmt)
{
    sensor_msg("   <%s>\n", __func__);

    return (kdev_status_t)irs2877c_set_params(&irs2877c_dev);
}

static kdev_status_t kdev_sensor_set_aec(struct cam_sensor_aec *aec_p)
{
    sensor_msg("   <%s>\n", __func__);

    rgb_set_aec_roi(aec_p);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_get_lux(uint16_t *expo, uint8_t *pre_gain, uint8_t *post_gain, uint8_t* global_gain, uint8_t* y_average)
{
    sensor_msg("   <%s>\n", __func__);

    rgb_get_lux(expo, pre_gain, post_gain, global_gain, y_average);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_mirror(uint32_t enable)
{
    uint16_t data = 0;

    //step 1: Disable streaming
    data = 0;
    irs2877c_read_reg(&irs2877c_dev, 0x9400  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;
    data &= ~0x01;
    data |= 0x00;
    irs2877c_write_reg(&irs2877c_dev, 0x9400 , data);

    //step 2: wait for 10ms
    kdrv_timer_delay_ms(20);

    //step 3: set registers
    irs2877c_read_reg(&irs2877c_dev, 0x91c4  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;

    data &= ~0x04;

    if (enable)
    {
        data |= 0x04;
        irs2877c_write_reg(&irs2877c_dev, 0x91c4 , data);
    }
    else
    {
        irs2877c_write_reg(&irs2877c_dev, 0x91c4 , data);
    }

    //step 4: update register
    data = 0;
    irs2877c_read_reg(&irs2877c_dev, 0x9402  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;
    data &= ~0x03;
    data |= 0x03;
    irs2877c_write_reg(&irs2877c_dev, 0x9402 , data);

    //step 5: enable streaming
    data = 0;
    irs2877c_read_reg(&irs2877c_dev, 0x9400  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;
    data &= ~0x01;
    data |= 0x01;
    irs2877c_write_reg(&irs2877c_dev, 0x9400 , data);
    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_flip(uint32_t enable)
{
    uint16_t data = 0;

    //step 1: Disable streaming
    data = 0;
    irs2877c_read_reg(&irs2877c_dev, 0x9400  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;
    data &= ~0x01;
    data |= 0x00;
    irs2877c_write_reg(&irs2877c_dev, 0x9400 , data);

    //step 2: wait for 10ms
    kdrv_timer_delay_ms(20);

    //step 3: set registers
    irs2877c_read_reg(&irs2877c_dev, 0x91c4  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;

    data &= ~0x02;

    if (enable)
    {
        data |= 0x02;
        irs2877c_write_reg(&irs2877c_dev, 0x91c4 , data);
    }
    else
    {
        irs2877c_write_reg(&irs2877c_dev, 0x91c4 , data);
    }

    //step 4: update register
    data = 0;
    irs2877c_read_reg(&irs2877c_dev, 0x9402  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;
    data &= ~0x03;
    data |= 0x03;
    irs2877c_write_reg(&irs2877c_dev, 0x9402 , data);

    //step 5: enable streaming
    data = 0; 
    irs2877c_read_reg(&irs2877c_dev, 0x9400  , &data);
    data =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;
    data &= ~0x01;
    data |= 0x01;
    irs2877c_write_reg(&irs2877c_dev, 0x9400 , data);

    return KDEV_STATUS_OK;
}

static uint32_t kdev_sensor_get_dev_id(void)
{
    uint16_t data = 0;
    uint16_t id = 0;

    irs2877c_read_reg(&irs2877c_dev, 0xA0A4  , &data);
    id =  (data & 0xFF)<<8 | (data & 0xFF00)>>8;

    return (uint32_t)id;
}

static kdev_status_t kdev_sensor_set_devaddress(uint32_t address, uint32_t port_id)
{
    irs2877c_dev.addr = address;
    irs2877c_dev.i2c_port = port_id;
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_sensor_get_calibration_data(unsigned char *cali_data, uint8_t len)
{
    //step 1: Disable streaming
    irs2877c_write_reg(&irs2877c_dev, 0x9400 , 0x0000);

    //step 2: wait for 10ms
    kdrv_timer_delay_ms(60);
	
    // enable I2C to SPI bridge
    irs2877c_write_reg(&irs2877c_dev, 0xA087, 0x5003);
    irs2877c_write_reg(&irs2877c_dev, 0xA088, 0x0100);      // block to write to SPI flash
    irs2877c_write_reg(&irs2877c_dev, 0xA089, 0x0000);      // block read out of SPI flash
    

    uint16_t SPI_status = 0;
    uint16_t SPIread_address = 0x0000;
    uint16_t SPIwriteaddress = 0x0100;
    uint32_t cali_start_addr = 0x85000;
    uint16_t write_command = 0x0300 | (((cali_start_addr) & 0xFF0000) >> 16);
    uint16_t  lower_address = (cali_start_addr) & 0x00FFFF;

    irs2877c_write_reg(&irs2877c_dev, SPIwriteaddress, write_command);           // set EEPROM upper Address bits
    irs2877c_write_reg(&irs2877c_dev, SPIwriteaddress + 1, lower_address);       // lower address bits
    uint16_t num_read = len * 4 + 3 + 0xE000;                        // len + 3 + 0xE000;
    irs2877c_write_reg(&irs2877c_dev, 0xA08A, num_read);                         // enable read SDI
    irs2877c_write_reg(&irs2877c_dev, 0xA08B, 0x0002);                           // init and execute

    kdrv_timer_delay_ms(1);
    while(1)
    {
        irs2877c_read_reg(&irs2877c_dev, 0xA08C, &SPI_status);   // A08C  SPI status register   value  0x0001 --> readey for command
        if(SPI_status != 0x0100) {
            fprintf(stderr, "SPI_status:0xA08C eeprom_addr 0x%08x  0x%04x\n", cali_start_addr , SPI_status);
            kdrv_timer_delay_ms(1);
        }
        else
            break;
    }
    kdrv_i2c_read_register((kdrv_i2c_ctrl_t)irs2877c_dev.i2c_port, irs2877c_dev.addr, SPIread_address, 2, len*4, (uint16_t *)&cali_data[0]);
    //step 5: enable streaming
    irs2877c_write_reg(&irs2877c_dev, 0x9400 , 0x0001);
    for(uint16_t i=0; i<len*4; i++)
    {
        dbg_msg("[irs2877c]cali data0[%d] %x\n", i, (unsigned char)*(&cali_data[i]));
    }
	return KDEV_STATUS_OK;
}

static uint16_t expToRegsiter(float frequence, float exp) // caculate exposure register value
{
    int prescaler = 0, prescaler_ID = 0;
    if ((1 << 14) / frequence > exp)
    {
        prescaler = 1;
        prescaler_ID = 0;
    }
    else if ((1 << 17) / frequence > exp)
    {
        prescaler = 8;
        prescaler_ID = 1;
    }
    else if ((1 << 19) / frequence > exp)
    {
        prescaler_ID = 2;
        prescaler = 32;
    }
    else if ((1 << 21) / frequence > exp)
    {
        prescaler_ID = 3;
        prescaler = 128;
    }
    else
    {
        return 0;
    }
    return (uint16_t)(prescaler_ID * 0x4000 + frequence * exp / (prescaler));
}

static kdev_status_t setExposureTime(uint32_t exp_time)
{
    uint16_t NewExposuretime = 0;                              // 計算後 register 的值
    uint16_t Exposuretime = exp_time & 0x0000ffff;
    NewExposuretime = expToRegsiter(80.32, (float)Exposuretime); // 計算register值  Exposuretime單位us
                                                          // 80.32為目前頻率 不用改
    
    irs2877c_write_reg(&irs2877c_dev, 0x9002, NewExposuretime); // image 1  寫入曝光值
    // sensor_msg("irs2877c_write_reg(0x%04x, 0x%04x)\n", 0x9002, NewExposuretime);
    irs2877c_write_reg(&irs2877c_dev, 0x9004, NewExposuretime); // image 2  寫入曝光值
    // sensor_msg("irs2877c_write_reg(0x%04x, 0x%04x)\n", 0x9004, NewExposuretime);
    irs2877c_write_reg(&irs2877c_dev, 0x9006, NewExposuretime); // image 3  寫入曝光值
    // sensor_msg("irs2877c_write_reg(0x%04x, 0x%04x)\n", 0x9006, NewExposuretime);
    irs2877c_write_reg(&irs2877c_dev, 0x9008, NewExposuretime); // image 4  寫入曝光值
    // sensor_msg("irs2877c_write_reg(0x%04x, 0x%04x)\n", 0x9008, NewExposuretime);
    irs2877c_write_reg(&irs2877c_dev, 0x9402, 0x8001);          // update  告知sensor 曝光時間更新
    // sensor_msg("irs2877c_write_reg(0x%04x, 0x%04x)\n", 0x9402, 0x8001);

    sensor_msg("Set 0x%04x)\n", NewExposuretime);

    return KDEV_STATUS_OK;
}

static sensor_ops irs2877c_ops = {
    .s_power        = kdev_sensor_power,
    .reset          = kdev_sensor_reset,
    .s_stream       = kdev_sensor_stream,
    .enum_fmt       = kdev_sensor_enum_fmt,
    .get_fmt        = kdev_sensor_get_fmt,
    .set_fmt        = kdev_sensor_set_fmt,
    .set_gain       = NULL,
    .set_aec        = kdev_sensor_set_aec,
    .set_exp_time   = setExposureTime,
    .get_lux        = kdev_sensor_get_lux,
    .led_switch     = NULL,
    .set_mirror     = kdev_sensor_set_mirror,
    .set_flip       = kdev_sensor_set_flip,
    .get_dev_id     = kdev_sensor_get_dev_id,
    .set_addr       = kdev_sensor_set_devaddress,
    .init           = kdev_sensor_init,
};

#if (IMGSRC_0_SENSORID == SENSOR_ID_IRS2877C)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 0, &irs2877c_ops);
#endif
#if (IMGSRC_1_SENSORID == SENSOR_ID_IRS2877C)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 1, &irs2877c_ops);
#endif

#endif

