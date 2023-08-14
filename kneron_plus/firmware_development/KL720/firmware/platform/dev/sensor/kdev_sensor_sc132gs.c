/*
 * Kneron SC132GS sensor driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include "project.h"
#include "kmdw_sensor.h"
#include "kdev_sensor.h"
#include "kdrv_i2c.h"
#include "kmdw_console.h"
#include "kdrv_gpio.h"
#include "kdrv_clock.h"
#include "kdrv_timer.h"
//#define SC132GS_DBG
#ifdef SC132GS_DBG
#include "kmdw_console.h"
#define sensor_msg(fmt, ...) kmdw_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define sensor_msg(fmt, ...)
#endif

static const struct sensor_datafmt_info sc132gs_colour_fmts[] = {
    { PIX_FMT_RAW8, COLORSPACE_RAW },
};

static struct sensor_device sc132gs_dev = {
    .addr = 0x30,
};

struct sensor_init_seq sc132gs_init_regs[] = {
    {0x0103, 0x01},
    {0x0100, 0x00},

    //PLL bypass
    {0x36e9, 0x80},
    {0x36f9, 0x80},

    {0x0100, 0x00},
    {0x3018, 0x12},
    {0x3019, 0x0e},
    {0x301a, 0xb4},
    {0x3031, 0x08},   // 0X0A:RAW10;0X08:RAW8
    {0x3032, 0x60},
    {0x3038, 0x44},
    {0x3207, 0x17},
    {0x320c, 0x06},
    {0x320d, 0x40},
    //{0x320e, 0x05}, // for 50 frame rate
    //{0x320f, 0x46},
    {0x320e, 0x06},// for 40 frame rate
    {0x320f, 0x97},
    //{0x320e, 0x0A},   // for 25 frame rate
    //{0x320f, 0x8C},
    {0x3250, 0xcc},
    {0x3251, 0x02},
    {0x3252, 0x05},
    {0x3253, 0x41},
    {0x3254, 0x05},
    {0x3255, 0x3b},
    {0x3306, 0x78},
    {0x330a, 0x00},
    {0x330b, 0xc8},
    {0x330f, 0x24},
    {0x3314, 0x80},
    {0x3315, 0x40},
    {0x3317, 0xf0},
    {0x331f, 0x12},
    {0x3364, 0x00},
    {0x3385, 0x41},
    {0x3387, 0x41},
    {0x3389, 0x09},
    {0x33ab, 0x00},
    {0x33ac, 0x00},
    {0x33b1, 0x03},
    {0x33b2, 0x12},
    {0x33f8, 0x02},
    {0x33fa, 0x01},
    {0x3409, 0x08},
    {0x34f0, 0xc0},
    {0x34f1, 0x20},
    {0x34f2, 0x03},
    {0x3622, 0xf5},
    {0x3630, 0x5c},
    {0x3631, 0x80},
    {0x3632, 0xc8},
    {0x3633, 0x32},
    {0x3638, 0x2a},
    {0x3639, 0x07},
    {0x363b, 0x48},
    {0x363c, 0x83},
    {0x363d, 0x10},
    {0x36ea, 0x3a},
    {0x36fa, 0x25},
    {0x36fb, 0x05},
    {0x36fd, 0x04},
    {0x3900, 0x11},
    {0x3901, 0x05},
    {0x3902, 0xc5},
    {0x3904, 0x04},
    {0x3908, 0x91},
    {0x391e, 0x00},
//    {0x3e01, 0x5B},	    // 23488
//    {0x3e02, 0xC0},	    //
    {0x3e01, 0x68},	    // 26864
    {0x3e02, 0xF0},       
#ifdef ANA_GAIN
    {0x3e03, 0x03},     // ANA AGC
    {0x3e08, 0x00},	    // AGC  default:null
    {0x3e09, 0x80},	    // AGC    default:{0x3e09, 0x20},
#else
    {0x3e08, 0x23},	    // AGC  default:null
    {0x3e09, 0x35},	    // AGC  gain = 3.0  default:{0x3e09, 0x20},  
#endif
    {0x3e0e, 0xd2},
    {0x3e14, 0xb0},
    {0x3e1e, 0x7c},
    {0x3e26, 0x20},
    {0x4418, 0x38},
    {0x4503, 0x10},
    {0x4837, 0x14},
    {0x5000, 0x0e},
    {0x540c, 0x51},
    {0x550f, 0x38},
    {0x5780, 0x67},
    {0x5784, 0x10},
    {0x5785, 0x06},
    {0x5787, 0x02},
    {0x5788, 0x00},
    {0x5789, 0x00},
    {0x578a, 0x02},
    {0x578b, 0x00},
    {0x578c, 0x00},
    {0x5790, 0x00},
    {0x5791, 0x00},
    {0x5792, 0x00},
    {0x5793, 0x00},
    {0x5794, 0x00},
    {0x5795, 0x00},
    {0x5799, 0x04},

    //Vbin
    {0x3220, 0x87},
    {0x3215, 0x22},

    {0x3213, 0x08},
    {0x320a, 0x02}, // 640
    {0x320b, 0x80},

    {0x334f, 0xbe},
    {0x3231, 0x0a},
    {0x3230, 0x0c},

    //Hsum
    {0x5000, 0x40},
    {0x5901, 0x14},
    {0x5900, 0xf6},

    {0x3208, 0x01}, // 480
    {0x3209, 0xe0},

    {0x36ec, 0x03},
    {0x3211, 0x1e},

    //2lane mipi
    {0x3019, 0x0c}, //[3:0] lane disable
    {0x3018, 0x32}, //[6:5] lane num=[6:5]+1

    {0x0100, 0x01},

    //PLL set
    {0x36e9, 0x24},
    {0x36f9, 0x24},

    // [gain>=2]
    {0x33f8, 0x02},
    {0x3314, 0x80},
    {0x33fa, 0x02},
    {0x3317, 0x00},

#ifdef MIPI_EXAMPLE
    // test mode
    {0x4501, 0xAC},     // Bit[3]:incremental pattern enable
    {0x3902, 0x85},
    {0x391d, 0xa8},
    {0x3e06, 0x03},
#endif

#if 1   // flip & mirror
    {0x323b, 0x01},     // flip on
    {0x3221, 0x66},     // bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
#endif
#if 0   // flip
    {0x323b, 0x03},     // flip on
    {0x3221, 0x60},     // bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
#endif
#if 1   // mirror
    //{0x323b, 0x03},   // flip on
    {0x3221, 0x06},     // bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
#endif

    { 0x00, 0x00},
};
static uint32_t kdev_sensor_get_dev_id(void);
static void nir_set_exp_time(uint16_t exp_time)
{
    uint16_t dev_addr = sc132gs_dev.addr;
    uint16_t ana_gn1 = (exp_time >> 8) & 0xff;
    uint16_t ana_gn2 = exp_time & 0xff;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t) sc132gs_dev.i2c_port;

    kdrv_i2c_write_register(i2c_port, dev_addr, 0x3e01, 2, 1, &ana_gn1);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x3e02, 2, 1, &ana_gn2);
}

static void nir_set_gain(uint16_t ana_gn1, uint16_t ana_gn2)
{
    uint16_t dev_addr = sc132gs_dev.addr;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t) sc132gs_dev.i2c_port;

    kdrv_i2c_write_register(i2c_port, dev_addr, 0x3e08, 2, 1, &ana_gn1);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x3e09, 2, 1, &ana_gn2);
}

static void nir_led_open(void)
{
    uint16_t dev_addr = sc132gs_dev.addr;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t) sc132gs_dev.i2c_port;
    uint16_t data = 0x00;

    kdrv_i2c_write_register(i2c_port, dev_addr, 0x3361, 2, 1, (&data));
}

static void nir_led_close(void)
{
    uint16_t dev_addr = sc132gs_dev.addr;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t) sc132gs_dev.i2c_port;
    uint16_t data = 0xc0;

    kdrv_i2c_write_register(i2c_port, dev_addr, 0x3361, 2, 1, &data);
}

static uint32_t sc132gs_write_reg(struct sensor_device *sensor_dev, uint16_t reg, uint16_t data)
{
    uint32_t ret;

    ret = kdrv_i2c_write_register((kdrv_i2c_ctrl_t)sensor_dev->i2c_port, sensor_dev->addr, reg, 2, 1, &data);
    return ret;
}

static uint32_t sc132gs_read_reg(struct sensor_device *sensor_dev, uint16_t reg, uint16_t *data)
{
    uint32_t ret;

    ret = kdrv_i2c_read_register((kdrv_i2c_ctrl_t)sensor_dev->i2c_port, sensor_dev->addr, reg, 2, 1, data);
    return ret;
}

void sc132gs_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;

    for (init_fnc_ptr = seq; ; ++init_fnc_ptr)
    {
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0)
            break; //reaches end
        sc132gs_write_reg(dev, init_fnc_ptr->addr , (uint8_t)(init_fnc_ptr->value & 0xFF));
    }

    uint32_t data = kdev_sensor_get_dev_id();
    sensor_msg(" sc132gs_init sensor id = %x\n", data);
}

static uint32_t sc132gs_set_params(struct sensor_device *sensor_dev)
{
    /* initialize the sensor with default settings */
    return 0;
}

static kdev_status_t kdev_sensor_init(void)
{
    sensor_msg("   <%s>\n", __func__);
    sc132gs_init(&sc132gs_dev, sc132gs_init_regs);

#if (defined(IMGSRC_1_SUPPORT_LED) && IMGSRC_1_SUPPORT_LED == YES)
    for(uint32_t i = 1; i<=IMGSRC_1_LED_STRENGTH;i++)
    {
        kdrv_delay_us(LED_DELAY_INTERVAL);
        kdrv_gpio_write_pin(NIR_LED, true);
        kdrv_delay_us(LED_DELAY_INTERVAL);
        kdrv_gpio_write_pin(NIR_LED, false);
    }
    kdrv_delay_us(LED_DELAY_INTERVAL);
    kdrv_gpio_write_pin(NIR_LED, true);
#endif
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
    sensor_msg("   <%s>\n", __func__);
    return KDEV_STATUS_OK;
}
#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))
static kdev_status_t kdev_sensor_enum_fmt(uint32_t index, uint32_t *code)
{
    if (index >= ARRAY_SIZE(sc132gs_colour_fmts))
        return KDEV_STATUS_ERROR;

    sensor_msg("   <%s>\n", __func__);
    *code = sc132gs_colour_fmts[index].fourcc;
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

    return (kdev_status_t)sc132gs_set_params(&sc132gs_dev);
}

static kdev_status_t kdev_sensor_set_gain(uint32_t gain1, uint32_t gain2)
{
    sensor_msg("   <%s>\n", __func__);

    nir_set_gain(gain1, gain2);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_exp_time(uint32_t exp_time)
{
    sensor_msg("   <%s>\n", __func__);

    nir_set_exp_time(exp_time);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_led_switch(uint32_t on)
{
    sensor_msg("   <%s>\n", __func__);

    if (on)
        nir_led_open();
    else
        nir_led_close();

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_mirror(uint32_t enable)
{
    uint16_t dev_addr = sc132gs_dev.addr;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t) sc132gs_dev.i2c_port;
    uint16_t data = 0;

    kdrv_i2c_read_register(i2c_port, dev_addr, 0x3221, 2, 1, &data);
    data &= ~0x06;

    if (enable)
    {
        data |= 0x06;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x3221, 2, 1, &data);
    }
    else
    {
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x3221, 2, 1, &data);
    }

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_flip(uint32_t enable)
{
    uint16_t dev_addr = sc132gs_dev.addr;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t) sc132gs_dev.i2c_port;
    uint16_t data = 0, data1;

    kdrv_i2c_read_register(i2c_port, dev_addr, 0x3221, 2, 1, &data);
    data &= ~0x60;

    if (enable)
    {
        data1 = 0x01;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x323b, 2, 1, &data1);
        data |= 0x60;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x3221, 2, 1, &data);
    }
    else
    {
        data1 = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x323b, 2, 1, &data1);
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x3221, 2, 1, &data);
    }

    return KDEV_STATUS_OK;
}

static uint32_t kdev_sensor_get_dev_id(void)
{
    uint16_t data = 0;
    uint16_t id = 0;

    sc132gs_read_reg(&sc132gs_dev, 0x3107, &data);
    id = data<<8;
    sc132gs_read_reg(&sc132gs_dev, 0x3108, &data);
    id += data;
    return (uint32_t)id;
}

static kdev_status_t kdev_sensor_set_devaddress(uint32_t address, uint32_t port_id)
{
    sc132gs_dev.addr = address;
    sc132gs_dev.i2c_port = port_id;
    return KDEV_STATUS_OK;
}
static sensor_ops sc132gs_ops = {
    .s_power        = kdev_sensor_power,
    .reset          = kdev_sensor_reset,
    .s_stream       = kdev_sensor_stream,
    .enum_fmt       = kdev_sensor_enum_fmt,
    .get_fmt        = kdev_sensor_get_fmt,
    .set_fmt        = kdev_sensor_set_fmt,
    .set_gain       = kdev_sensor_set_gain,
    .set_aec        = NULL,
    .set_exp_time   = kdev_sensor_set_exp_time,
    .get_lux        = NULL,
    .led_switch     = kdev_sensor_led_switch,
    .set_mirror     = kdev_sensor_set_mirror,
    .set_flip       = kdev_sensor_set_flip,
    .get_dev_id     = kdev_sensor_get_dev_id,
    .set_addr       = kdev_sensor_set_devaddress,
    .init           = kdev_sensor_init,
};


#if (IMGSRC_0_SENSORID == SENSOR_ID_SC132GS)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 0, &sc132gs_ops);
#endif
#if (IMGSRC_1_SENSORID == SENSOR_ID_SC132GS)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 1, &sc132gs_ops);
#endif
