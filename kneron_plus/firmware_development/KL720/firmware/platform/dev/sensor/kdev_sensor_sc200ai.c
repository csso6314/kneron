
#include "project.h"
#if (CFG_IMGSRC_0_TYPE == IMGSRC_MIPI_SC200AI) || \
    (CFG_IMGSRC_1_TYPE == IMGSRC_MIPI_SC200AI)
#include "kdrv_i2c.h"
#include "base.h"

#include <stdlib.h>
#include "kmdw_sensor.h"
#include "kdev_sensor.h"
#include "kdrv_i2c.h"
#include "kdev_status.h"

#define SC200AI_DBG
#ifdef SC200AI_DBG
#include "kmdw_console.h"
#define sensor_msg(fmt, ...) kmdw_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define sensor_msg(fmt, ...)
#endif
#define SC200AI_SENSOR_ID_RH    (0x3107)
#define SC200AI_SENSOR_ID_RL    (0x3108)
#define SC200AI_SENSOR_ID       (0xCB1C)

#define SC200AI_LONG_EXP_RH     (0x3E00)    //B[3:0]
#define SC200AI_LONG_EXP_RM     (0x3E01)    //B[7:0]
#define SC200AI_LONG_EXP_RL     (0x3E02)    //B[7:4]

#define SC200AI_MF_CTRL_R       (0x3221)    //B[2:1] Mirror: Off[2b'00], On[2b'11]. B[6:5] Flip: Off[2b'00], On[2b'11]


#define SC200AI_INC_PATTERN_R   (0x4501)    //TEST_MODE
#define SC200AI_BLC_R           (0x3902)    //TEST_MODE

//struct sensor_init_seq {
//    u16 addr;
//    u8 value;
//}__attribute__((packed));

struct sensor_init_seq sc200ai_init_regs[] = {
#if 0
        {0x0103,0x01},
        {0x0100,0x00},

        {0x36e9,0x80},
        {0x36f9,0x80},

        //digital initial setting
        //dig set 0319
        {0x59e0,0x60}, //a0
        {0x59e1,0x08},

        {0x59e2,0x3f},
        {0x59e3,0x18}, //10
        {0x59e4,0x18}, //18

        {0x59e5,0x3f},//10
        {0x59e6,0x06},//10
        {0x59e7,0x02},//10

        {0x59e8,0x38},
        {0x59e9,0x10},
        {0x59ea,0x0c},

        {0x59eb,0x10},
        {0x59ec,0x04}, //10
        {0x59ed,0x02}, //10

        {0x59ee,0xa0},
        {0x59ef,0x08},
        {0x59f4,0x18},
        {0x59f5,0x10},
        {0x59f6,0x0c},
        {0x59f7,0x10},
        {0x59f8,0x06},
        {0x59f9,0x02},
        {0x59fa,0x18},
        {0x59fb,0x10},
        {0x59fc,0x0c},
        {0x59fd,0x10},
        {0x59fe,0x04},
        {0x59ff,0x02},
        {0x3e16,0x00},
        {0x3e17,0x80},

        {0x5799,0x00},
        //dig set end

        {0x3f09,0x48},
        {0x3e01,0x8c},
        {0x3e02,0x20},

        {0x391f,0x18}, // take adc high 10bit data
        {0x363a,0x1f}, //avdd psrr compensation
        {0x3637,0x1b}, // adc range
        {0x391d,0x14},

        //0x3902,0x85,
        //0x3909,0x04,
        //0x390a,0x80,
        {0x330b,0x88},
        {0x3908,0x41}, //blc target 16

        {0x3333,0x10}, // vln_pdb all 1
        {0x3301,0x20},
        {0x3304,0x40},
        {0x331e,0x39},
        {0x330f,0x02},
        {0x3306,0x32}, //[2a,32,-]
        //0x330b,0x82, //[-,90]
        {0x363c,0x0e},   //nvdd set -1
        {0x363b,0xc6}, //hvdd set 3.15
        {0x3622,0x16}, //rst sig clamp

        //20191217 DPC
        {0x5787,0x10},
        {0x5788,0x06},
        //0x5789,0x00,
        {0x578a,0x10},
        {0x578b,0x06},
        //0x578c,0x00,
        {0x5790,0x10},
        {0x5791,0x10},
        {0x5792,0x00},
        {0x5793,0x10},
        {0x5794,0x10},
        {0x5795,0x00},
        {0x5799,0x00},
        {0x57c7,0x10},
        {0x57c8,0x06},
        //0x57c9,0x00,
        {0x57ca,0x10},
        {0x57cb,0x06},
        //0x57cc,0x00,
        //0x57d0,0x10,
        {0x57d1,0x10},
        //0x57d2,0x00,
        //0x57d3,0x10,
        {0x57d4,0x10},
        //0x57d5,0x00,
        {0x57d9,0x00},

        {0x3670,0x08},//[3] 0x3633 auto en
        {0x369c,0x40},//gain0
        {0x369d,0x48},//gain1
        {0x3690,0x34},//sel0
        {0x3691,0x33},//sel1
        {0x3692,0x44},//sel2

        {0x3670,0x0a},//[1] 0x3630 auto enable , 0x3681 reaout
        {0x367c,0x48},//auto 0x3630 gain0
        {0x367d,0x58},//auto 0x3630 gain1
        {0x3674,0x82},//auto 0x3630 sel0
        {0x3675,0x76},//auto 0x3630 sel1
        {0x3676,0x78},//auto 0x3630 sel2

        //20200327
        {0x3253,0x08},

        {0x301f,0x03},//setting id

        //20200604
        {0x3271,0x0a},    //pdummy_st
        {0x3243,0x01},    //dumy_end
        {0x3248,0x02},    //blc st
        {0x3249,0x09},    //blc end
        {0x3901,0x02},
        {0x3904,0x04},

        //20200616
        {0x3621,0xe8},

        //20201218
        {0x301f,0x2d},//setting id

        {0x3031,0x08},//8bit
        {0x3037,0x00},

        //1280*960
        {0x3200,0x01},
        {0x3201,0x40},
        {0x3202,0x00},
        {0x3203,0x3c},
        {0x3204,0x06},
        {0x3205,0x47},
        {0x3206,0x04},
        {0x3207,0x03},
        {0x3208,0x05},
        {0x3209,0x00},
        {0x320a,0x03},
        {0x320b,0xc0},
        {0x3210,0x00},
        {0x3211,0x04},
        {0x3212,0x00},
        {0x3213,0x04},

        //vbin
        {0x3220,0x17},
        {0x3215,0x31},
        {0x3213,0x02},
        {0x320a,0x01},
        {0x320b,0xe0},//480

        //hsub
        {0x5000,0x46},
        {0x5901,0x04},
        {0x5900,0xf1},
        {0x3211,0x02},
        {0x3208,0x02},
        {0x3209,0x80},//640

        {0x320c,0x06},//1600
        {0x320d,0x40},

        //mipi timing
        {0x4819,0x04},
        {0x481b,0x02},
        {0x481d,0x06},
        {0x481f,0x02},
        {0x4821,0x07},
        {0x4823,0x02},
        {0x4825,0x02},
        {0x4827,0x02},
        {0x4829,0x03},
        {0x598e,0x06},
        {0x598f,0x06},

        //0x36e9,0x20,//24/2*5*12=720
        {0x36ea,0x34},
        {0x36eb,0x0b},//sclk=720/2/4/2=45
        {0x36ec,0x2a},//mipiclk=720/4=180   mipipclk=720/4/2/4=22.5
        {0x36ed,0x24},
        //0x36f9,0x20,//24/2*6*15=1080
        {0x36fa,0x31},
        {0x36fb,0x00},
        {0x36fc,0x10},//countclk=1080/2=540   countclk:sclk=12:1
        {0x36fd,0x34},

        {0x36e9,0x20},
        {0x36f9,0x20},

        {0x0100,0x01},
        #else
{0x0103,0x01},
{0x0100,0x00},
{0x36e9,0x80},
{0x36f9,0x80},
{0x301f,0xfc},
{0x3031,0x0a},
{0x3037,0x20},
{0x3200,0x03},
{0x3201,0x70},
{0x3202,0x01},
{0x3203,0xe0},
{0x3204,0x04},
{0x3205,0x17},
{0x3206,0x02},
{0x3207,0x5f},
{0x3208,0x00},
{0x3209,0xa0},
{0x320a,0x00},
{0x320b,0x78},
{0x320c,0x09},
{0x320d,0xc4},
{0x320e,0x08},
{0x320f,0x34},
{0x3210,0x00},
{0x3211,0x04},
{0x3212,0x00},
{0x3213,0x04},
{0x3243,0x01},
{0x3248,0x02},
{0x3249,0x09},
{0x3253,0x08},
{0x3271,0x0a},
{0x3301,0x20},
{0x3304,0x40},
{0x3306,0x32},
{0x330b,0x88},
{0x330f,0x02},
{0x331e,0x39},
{0x3333,0x10},
{0x3621,0xe8},
{0x3622,0x16},
{0x3637,0x1b},
{0x363a,0x1f},
{0x363b,0xc6},
{0x363c,0x0e},
{0x3670,0x0a},
{0x3674,0x82},
{0x3675,0x76},
{0x3676,0x78},
{0x367c,0x48},
{0x367d,0x58},
{0x3690,0x34},
{0x3691,0x33},
{0x3692,0x44},
{0x369c,0x40},
{0x369d,0x48},
{0x36ea,0x32},
{0x36eb,0x3d},
{0x36ec,0x1c},
{0x36ed,0x24},
{0x36fa,0x39},
{0x36fb,0x00},
{0x36fc,0x11},
{0x36fd,0x07},
{0x3901,0x02},
{0x3904,0x04},
{0x3908,0x41},
{0x391d,0x14},
{0x391f,0x18},
#if 1
{0x3e00,0x00},
{0x3e01,0x08},
{0x3e02,0x00},
{0x3e08,0x03},//t1 ANA GAIN
{0x3e09,0x40},//t1 ANA FINE GAIN
{0x3e06,0x00},//t1 DIG GAIN
{0x3e07,0x80},//t1 DIG FINE GAIN
#else

{0x3e00,0x01},
{0x3e01,0x06},
{0x3e02,0x00},
#endif
{0x3e16,0x00},
{0x3e17,0x80},
{0x3f09,0x48},
{0x4819,0x02},
{0x481b,0x01},
{0x481d,0x02},
{0x481f,0x01},
{0x4821,0x07},
{0x4823,0x01},
{0x4825,0x01},
{0x4827,0x01},
{0x4829,0x01},
{0x5787,0x10},
{0x5788,0x06},
{0x578a,0x10},
{0x578b,0x06},
{0x5790,0x10},
{0x5791,0x10},
{0x5792,0x00},
{0x5793,0x10},
{0x5794,0x10},
{0x5795,0x00},
{0x5799,0x00},
{0x57c7,0x10},
{0x57c8,0x06},
{0x57ca,0x10},
{0x57cb,0x06},
{0x57d1,0x10},
{0x57d4,0x10},
{0x57d9,0x00},
{0x59e0,0x60},
{0x59e1,0x08},
{0x59e2,0x3f},
{0x59e3,0x18},
{0x59e4,0x18},
{0x59e5,0x3f},
{0x59e6,0x06},
{0x59e7,0x02},
{0x59e8,0x38},
{0x59e9,0x10},
{0x59ea,0x0c},
{0x59eb,0x10},
{0x59ec,0x04},
{0x59ed,0x02},
{0x59ee,0xa0},
{0x59ef,0x08},
{0x59f4,0x18},
{0x59f5,0x10},
{0x59f6,0x0c},
{0x59f7,0x10},
{0x59f8,0x06},
{0x59f9,0x02},
{0x59fa,0x18},
{0x59fb,0x10},
{0x59fc,0x0c},
{0x59fd,0x10},
{0x59fe,0x04},
{0x59ff,0x02},
{0x36e9,0x50},
{0x36f9,0x23},
{0x0100,0x01},
#endif
};
static uint32_t kdev_sensor_get_dev_id(void);
static struct sensor_device sc200ai_dev = {
    .addr = 0x30,
};

static struct kdev_sensor_context kdev_ctx = {
    .sensor_type    = 0xff,//IMGSRC_MIPI_SC200AI,
    .data_align_en  = 0,
    .tile_avg_en    = 0,
    .vstr0          = 0x05, // DPI VC0 V Sync Timing register               //Lucien-WaitCheck
    .vster          = 0x08, // DPI V Sync Timing Extended register          //Lucien-WaitCheck
    .pftr           = 0x30, // Pixel FIFO threshold register                //Lucien-WaitCheck
    .addr           = 0x30,
    .device_id      = 0x00, 
};

static uint32_t sc200ai_write_reg(struct sensor_device *sensor_dev, uint16_t reg, uint8_t data)
{
    uint32_t ret;

    ret = kdrv_i2c_write_register(KDRV_I2C_CTRL_0, sensor_dev->addr, reg, 2, 1, &data);
    return ret;
}

static uint32_t sc200ai_read_reg(struct sensor_device *sensor_dev, uint16_t reg, uint8_t *data)
{
    uint32_t ret;

    ret = kdrv_i2c_read_register(KDRV_I2C_CTRL_0, sensor_dev->addr, reg, 2, 1, data);
    return ret;
}
static int sc200ai_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;
    uint32_t device_id;
    //dbg_msg_camera("[%s] start ctx->addr=%x", __func__, ctx->addr);
    for (init_fnc_ptr = seq; ; ++init_fnc_ptr) {
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0) break; //reaches end
        sc200ai_write_reg(dev, init_fnc_ptr->addr , (uint8_t)(init_fnc_ptr->value & 0xFF));//(I2C_ADAP_0, ctx->addr, init_fnc_ptr->addr , 2, (u8)(init_fnc_ptr->value & 0xFF));
    }
    //dbg_msg_camera("[%s] end", __func__);
    device_id = kdev_sensor_get_dev_id();
    if(device_id != SC200AI_SENSOR_ID)
    {
        sensor_msg("device id not as expect.0x%x, 0x%x\n", device_id, SC200AI_SENSOR_ID);
        return -1;
    }

    return 0;
}

static uint32_t sc200ai_set_params(struct sensor_device *sensor_dev)
{
    /* initialize the sensor with default settings */
    sc200ai_init(sensor_dev, sc200ai_init_regs);

    return 0;
}

static kdev_status_t kdev_sensor_set_fmt(struct cam_format *fmt)
{
    sensor_msg("   <%s>\n", __func__);

    return (kdev_status_t)sc200ai_set_params(&sc200ai_dev);
}

static kdev_status_t kdev_sensor_set_gain(uint32_t gain1, uint32_t gain2)
{

    uint16_t dev_addr = sc200ai_dev.addr;
    uint8_t data = (uint8_t)gain1;

    kdrv_i2c_write_register(KDRV_I2C_CTRL_0, dev_addr, 0x3e08, 2, 1, &data);
    data = (uint8_t)gain2;
    kdrv_i2c_write_register(KDRV_I2C_CTRL_0, dev_addr, 0x3e09, 2, 1, &data);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_exp_time(uint32_t exp_time)
{

    uint16_t dev_addr = sc200ai_dev.addr;
    uint8_t data = 0x00;
    kdrv_i2c_write_register(KDRV_I2C_CTRL_0, dev_addr, SC200AI_LONG_EXP_RH, 2, 1, &data);
    data = (uint8_t)(((exp_time >> 8) & 0xff) << 4);
    kdrv_i2c_write_register(KDRV_I2C_CTRL_0, dev_addr, SC200AI_LONG_EXP_RM, 2, 1, &data);
    data = (uint8_t)(exp_time&0x0F)<<4;
    kdrv_i2c_write_register(KDRV_I2C_CTRL_0, dev_addr, SC200AI_LONG_EXP_RL, 2, 1, &data);
    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_mirror(uint32_t enable)
{


    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_flip(uint32_t enable)
{


    return KDEV_STATUS_OK;
}

static uint32_t kdev_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    return kdev_ctx.device_id;
}

static kdev_status_t kdev_sensor_get_exp_time(void)
{
    uint8_t data = 0;
    uint16_t nRD = 0;    

    sc200ai_read_reg(&sc200ai_dev, SC200AI_LONG_EXP_RH, &data);
    nRD |= (data&0x0F)<<12;
    sc200ai_read_reg(&sc200ai_dev, SC200AI_LONG_EXP_RM, &data);
    nRD |= (data&0xFF)<<4;
    sc200ai_read_reg(&sc200ai_dev, SC200AI_LONG_EXP_RL, &data);
    nRD |= (data&0xF0)>>4;
    sensor_msg("----------%s 0x%x[3:0]/0x%x[7:0]/0x%x[7:4]: 0x%X\r\n", __func__, SC200AI_LONG_EXP_RH, SC200AI_LONG_EXP_RM, SC200AI_LONG_EXP_RL, nRD);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_get_reg_data(u16 nReg)
{
    uint8_t nData = 0;

    sc200ai_read_reg(&sc200ai_dev, nReg, &nData);
    sensor_msg("----------%s Reg: 0x%X, Data: 0x%X", __func__, nReg, nData);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_inc_pattern_mode(u8 en)
{
    uint8_t nData = 0;

    sc200ai_read_reg(&sc200ai_dev, SC200AI_INC_PATTERN_R, &nData);
    sensor_msg("----------%s 0x%x: 0x%X, Switch State: %d\r\n", __func__, SC200AI_INC_PATTERN_R, nData, (nData&BIT3)>>3);
    nData &= ~BIT3;

    if ( en == 1 )
    {
        nData |= BIT3;
    }

    sc200ai_write_reg(&sc200ai_dev, SC200AI_INC_PATTERN_R, nData);
    sensor_msg("----------%s 0x%x: 0x%X, Switch State: %d\r\n", __func__, SC200AI_INC_PATTERN_R, nData, (nData&BIT3)>>3);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_set_blc_mode(u8 en) //Test mode
{
    uint8_t nData = 0;

    sc200ai_read_reg(&sc200ai_dev, SC200AI_BLC_R, &nData);
    sensor_msg("----------%s 0x%x: 0x%X, Switch State: %d", __func__, SC200AI_BLC_R, nData, (nData&BIT6)>>6);
    nData &= ~BIT6;

    if ( en == 1 )
    {
        nData |= BIT6;
    }

    sc200ai_read_reg(&sc200ai_dev, SC200AI_BLC_R, &nData);
    sensor_msg("----------%s 0x%x: 0x%X, Switch State: %d", __func__, SC200AI_BLC_R, nData, (nData&BIT6)>>6);


    return KDEV_STATUS_OK;
}
static uint32_t kdev_sensor_get_dev_id(void)
{
    uint8_t data = 0;
    uint16_t id = 0;

    sc200ai_read_reg(&sc200ai_dev, SC200AI_SENSOR_ID_RH, &data);
    id = data<<8;
    sc200ai_read_reg(&sc200ai_dev, SC200AI_SENSOR_ID_RL, &data);
    id += data;
    return (uint32_t)id;
}

static struct sensor_ops sc200ai_ops = {
    .s_power        = NULL,
    .reset          = NULL,
    .s_stream       = NULL,
    .enum_fmt       = NULL,
    .get_fmt        = NULL,
    .set_fmt        = kdev_sensor_set_fmt,
    .set_gain       = kdev_sensor_set_gain,
    .set_exp_time   = kdev_sensor_set_exp_time,
    .get_lux        = NULL,
    .set_aec        = NULL,
    .set_mirror     = kdev_sensor_set_mirror,
    .set_flip       = kdev_sensor_set_flip,
    .led_switch     = NULL,
    .get_dev_id     = kdev_sensor_get_id,

    .get_exp_time   = kdev_sensor_get_exp_time,
    .get_reg_data   = kdev_sensor_get_reg_data,
    .set_inc        = kdev_sensor_set_inc_pattern_mode,
    .set_blc_mode   = kdev_sensor_set_blc_mode,
};
#if (IMGSRC_0_SENSORID == SENSOR_ID_SC200AI)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 0, &kdev_ctx, &sc200ai_ops);
#endif
#if (IMGSRC_1_SENSORID == SENSOR_ID_SC200AI)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 1, &kdev_ctx, &sc200ai_ops);
#endif


#endif
