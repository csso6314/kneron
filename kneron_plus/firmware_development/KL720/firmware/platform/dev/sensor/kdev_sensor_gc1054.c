#if (IMGSRC_0_SENSORID == SENSOR_ID_GC1054_R) || \
    (IMGSRC_1_SENSORID == SENSOR_ID_GC1054_R) || \
    (IMGSRC_0_SENSORID == SENSOR_ID_GC1054_L) || \
    (IMGSRC_1_SENSORID == SENSOR_ID_GC1054_L)
#include <stdlib.h>
#include "project.h"
#include "kdev_sensor.h"
#include "kdrv_i2c.h"
#include "kdrv_gpio.h"
#include "kdrv_clock.h"
#include "base.h"
#define TFT43_WIDTH             480
#define TFT43_HEIGHT            272

#define VGA_LANDSCAPE_WIDTH     640
#define VGA_LANDSCAPE_HEIGHT    480
#define VGA_PORTRAIT_WIDTH      480
#define VGA_PORTRAIT_HEIGHT     640

#define HD_WIDTH                1280
#define HD_HEIGHT               720

#define FHD_WIDTH               1920
#define FHD_HEIGHT              1080

#define HMX_RICA_WIDTH          864
#define HMX_RICA_HEIGHT         491

#define QVGA_WIDTH              1280
#define QVGA_HEIGHT             960

#define UGA_WIDTH               1600
#define UGA_HEIGHT              1200
#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))


#define GC1054_DBG
#ifdef GC1054_DBG
#include "kmdw_console.h"
#define sensor_msg(fmt, ...) kmdw_printf("\n[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define sensor_msg(fmt, ...)
#endif

#define CROPPING_OFFSET_X   (0)
#define CROPPING_OFFSET_Y   (0)

#define ROI_OFFSET_X        ((CROPPING_OFFSET_X + 4) >> 3)
#define ROI_OFFSET_Y        ((CROPPING_OFFSET_Y + 4) >> 3)

#define DEV_LEFT        0
#define DEV_RIGHT       1
#define DEV_NUM         2

static const struct sensor_win_size gc1054_supported_win_sizes[] = {
    { .width = HD_WIDTH,            .height = HD_HEIGHT,            },
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },
    { .width = TFT43_WIDTH,         .height = TFT43_HEIGHT,         },
    { .width = QVGA_WIDTH,          .height = QVGA_HEIGHT,          },
    { .width = UGA_WIDTH,           .height = UGA_HEIGHT,           },
};

static struct sensor_device gc1054_dev[DEV_NUM] = {
    {
        .addr = 0x21,
        .i2c_port =  0,
    },
    {
        .addr = 0x21,
        .i2c_port =  1,
    }
};

struct sensor_init_seq gc1054_init_regs[] = {
//Actual_window_size=1280*720,MIPI 1lane
//Mclk=24Mhz?MIPI_clcok=312Mhz,Frame_rate=30fps
//HD=1726,VD=785,row_time=42.2564us
/////////////////////////////////////////////////////
//////////////////////   SYS   //////////////////////
/////////////////////////////////////////////////////
{0xf2,0x00},
{0xf6,0x00},
{0xfc,0x04},
{0xf7,0x01},
{0xf8,0x0c},
{0xf9,0x06},
{0xfa,0x80},
{0xfc,0x0e},
//////////////////////////////////////////
/////   ANALOG & CISCTL   ////////////////
//////////////////////////////////////////
{0xfe,0x00},
{0x03,0x02},//0x1F},//
{0x04,0xa6},
{0x05,0x01}, //HB
{0x06,0xe0},
{0x07,0x00}, //VB
{0x08,0x2d},
{0x09,0x00},
{0x0a,0x04}, //row start
{0x0b,0x00},
{0x0c,0x00}, //col start
{0x0d,0x02},
{0x0e,0xd4}, //height 724
{0x0f,0x05},
{0x10,0x08}, //width 1288
{0x17,0xc1},
{0x18,0x02},
{0x19,0x08},
{0x1a,0x18},
{0x1d,0x12},
{0x1e,0x50},
{0x1f,0x80},
{0x21,0x30},
{0x23,0xf8},
{0x25,0x10},
{0x28,0x20},
{0x34,0x08}, //data low
{0x3c,0x10},
{0x3d,0x0e},
{0xcc,0x8e},
{0xcd,0x9a},
{0xcf,0x70},
{0xd0,0xa9},
{0xd1,0xc5},
{0xd2,0xed}, //data high
{0xd8,0x3c}, //dacin offset
{0xd9,0x7a},
{0xda,0x12},
{0xdb,0x50},
{0xde,0x0c},
{0xe3,0x60},
{0xe4,0x78},
{0xfe,0x01},
{0xe3,0x01},
{0xe6,0x10}, //ramps offset
//////////////////////////////////////////
///////////   ISP   //////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0x80,0x50},
{0x88,0x73},
{0x89,0x03},
{0x90,0x01},
{0x92,0x02}, //crop win 2<=y<=4
{0x94,0x03}, //crop win 2<=x<=5
{0x95,0x02}, //crop win height
{0x96,0xd0},
{0x97,0x05}, //crop win width
{0x98,0x00},
//////////////////////////////////////////
///////////   BLK   //////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0x40,0x22},
{0x43,0x03},
{0x4e,0x3c},
{0x4f,0x00},
{0x60,0x00},
{0x61,0x80},
//////////////////////////////////////////
///////////   GAIN   /////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0xb0,0x50}, //global gain
{0xb1,0x07},    //0x00}, //pre_gain high byte
{0xb2,0x56}, //pre_gain low byte
{0xb6,0x00},
{0xfe,0x02},
{0x01,0x00},
{0x02,0x01},
{0x03,0x02},
{0x04,0x03},
{0x05,0x04},
{0x06,0x05},
{0x07,0x06},
{0x08,0x0e},
{0x09,0x16},
{0x0a,0x1e},
{0x0b,0x36},
{0x0c,0x3e},
{0x0d,0x56},
{0xfe,0x02},
{0xb0,0x00}, //col_gain[11:8]
{0xb1,0x00},
{0xb2,0x00},
{0xb3,0x11},
{0xb4,0x22},
{0xb5,0x54},
{0xb6,0xb8},
{0xb7,0x60},
{0xb9,0x00}, //col_gain[12]
{0xba,0xc0},
{0xc0,0x20}, //col_gain[7:0]
{0xc1,0x2d},
{0xc2,0x40},
{0xc3,0x5b},
{0xc4,0x80},
{0xc5,0xb5},
{0xc6,0x00},
{0xc7,0x6a},
{0xc8,0x00},
{0xc9,0xd4},
{0xca,0x00},
{0xcb,0xa8},
{0xcc,0x00},
{0xcd,0x50},
{0xce,0x00},
{0xcf,0xa1},
//////////////////////////////////////////
/////////   DARKSUN   ////////////////////
//////////////////////////////////////////
{0xfe,0x02},
{0x54,0xf7},
{0x55,0xf0},
{0x56,0x00},
{0x57,0x00},
{0x58,0x00},
{0x5a,0x04},
//////////////////////////////////////////
////////////   DD   //////////////////////
//////////////////////////////////////////
{0xfe,0x04},
{0x81,0x8a},
//////////////////////////////////////////
///////////     MIPI    /////////////////////
//////////////////////////////////////////
{0xfe,0x03},
{0x01,0x03},
{0x02,0x11},
{0x03,0x90},
{0x10,0x94},
{0x11,0x2a},
{0x12,0x00}, //lwc 1280*5/4
{0x13,0x05},
{0x15,0x06},
{0x21,0x02},
{0x22,0x02},
{0x23,0x08},
{0x24,0x02},
{0x25,0x10},
{0x26,0x04},
{0x29,0x02},
{0x2a,0x02},
{0x2b,0x04},
{0xfe,0x00},


};

#if 1// ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
typedef enum __io_led_mode
{
    LED_STANDBY_MODE = 0x80,
    LED_TORCH_MODE = 0x08,//0x88,
    LED_FLASH_MODE = 0x8C,
} io_led_mode;

static io_led_mode g_LedLightMode = LED_STANDBY_MODE;
#define I2C_ADAP_0      0
#define LED_SLAVE_ID        (0x63)
#define IO_LED1      ( 1 << 0 )
#define IO_LED2      ( 1 << 1 )
extern void led_set_light_mode( io_led_mode nMode );
void led_set_light_mode( io_led_mode nMode )
{
    g_LedLightMode = nMode;
}

static void aw36515_init( io_led_mode nMode )
{
    static u8 nFirstInit = TRUE;
    u8 nDeviceId;
    uint16_t data;
    if ( nFirstInit == TRUE )
    {
        //check device id
        kdrv_i2c_read_register( KDRV_I2C_CTRL_0, LED_SLAVE_ID, 0x00, 1, 1, (uint16_t*)&nDeviceId );
        sensor_msg("  36515 nDeviceId = 0x%x\n",  nDeviceId);
        if ( nDeviceId != 0x30 )
            return;

        //Set LED timing
        data = 0x1F;
        kdrv_i2c_write_register( KDRV_I2C_CTRL_0, LED_SLAVE_ID, 0x08, 1, 1, &data );

        //Set enable register
        led_set_light_mode( nMode );
        kdrv_i2c_write_register( KDRV_I2C_CTRL_0, LED_SLAVE_ID, 0x01, 1, 1, (uint16_t*)&g_LedLightMode );  //set mode and turn off LED
        nFirstInit = FALSE;
    }
}
void nir_led_set_level(u16 level)
{
    u8 nLevel, nLedBriReg;
    uint16_t data;
    if ( g_LedLightMode == LED_STANDBY_MODE )
    {
        led_set_light_mode(LED_TORCH_MODE);
    }

    if ( g_LedLightMode == LED_TORCH_MODE )
    {
        nLedBriReg = 0x05;
    }
    else if ( g_LedLightMode == LED_FLASH_MODE )
    {
        nLedBriReg = 0x03;
    }

    level = (level > 100)? (100):(level);
    nLevel = level * 255 / 100;  //mapping to 0~255

    //dbg_msg_console("[nir_led_open]  mode=0x%x, duty = %d ", g_LedLightMode, nLevel);
    data = (g_LedLightMode|IO_LED1);
    kdrv_i2c_write_register( KDRV_I2C_CTRL_0, LED_SLAVE_ID, 0x01,        1, 1, &data );  //set LED mode and turn on LED1 
    data = nLevel;
    kdrv_i2c_write_register( KDRV_I2C_CTRL_0, LED_SLAVE_ID, nLedBriReg,  1, 1,  &data);

}
void nir_led_close(void) 
{
    uint16_t data;
    led_set_light_mode( LED_TORCH_MODE );
    data = g_LedLightMode;
    kdrv_i2c_write_register( KDRV_I2C_CTRL_0, LED_SLAVE_ID, 0x01, 1, 1, &data );  //Set to torch mode and turn off LED

}
static void nir_led_init(u16 duty)
{
    aw36515_init( LED_TORCH_MODE );
    if ( g_LedLightMode != LED_STANDBY_MODE )
    {
        nir_led_set_level( duty );
    }
}
#endif

uint32_t gainLevelTable[12] = {
                                64,
                                91,
                                127,
                                182,
                                258,
                                369,
                                516,
                                738,
                                1032,
                                1491,
                                2084,
                                0xffffffff,
                                };
static     int total;


static const struct sensor_win_size *gc1054_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(gc1054_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(gc1054_supported_win_sizes); i++) {
        if (gc1054_supported_win_sizes[i].width  >= *width &&
            gc1054_supported_win_sizes[i].height >= *height) {
            *width = gc1054_supported_win_sizes[i].width;
            *height = gc1054_supported_win_sizes[i].height;
            return &gc1054_supported_win_sizes[i];
        }
    }

    *width = gc1054_supported_win_sizes[default_size].width;
    *height = gc1054_supported_win_sizes[default_size].height;
    return &gc1054_supported_win_sizes[default_size];
}

#if (IMGSRC_0_SENSORID == SENSOR_ID_GC1054_R) || (IMGSRC_1_SENSORID == SENSOR_ID_GC1054_R)
static int gc1054_r_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdrv_i2c_write_register((kdrv_i2c_ctrl_t)sensor_dev->i2c_port, sensor_dev->addr, reg, 1, 1, (uint16_t *)&data);

    return ret;
}
static int gc1054_r_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
    int ret;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_RIGHT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_RIGHT].addr;

    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)data);

    return ret;
}

int gc1054_r_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;
    //sensor_msg(" gc1054_init ");
    for (init_fnc_ptr = seq; ; ++init_fnc_ptr) {
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0) break; //reaches end
        gc1054_r_write_reg(dev, init_fnc_ptr->addr , (u8)(init_fnc_ptr->value & 0xFF));
    }
    sensor_msg("gc1054_r_init init over, sensor ID = 0x%x", dev->device_id);
    u8 data = 0;
    gc1054_r_read_reg(dev, 0xf0, &data);
    sensor_msg("gc1054_r_init sensor high id=%x", data);
    dev->device_id = data << 8;
    gc1054_r_read_reg(dev, 0xf1, &data);
    sensor_msg("gc1054_r_init sensor low id=%x", data);
    dev->device_id |= data;

    if(dev->device_id != 0x1054)
        return -1;

    return 0;
}


static int gc1054_r_set_params(
        struct sensor_device *sensor_dev, u32 *width, u32 *height, u32 fourcc)
{
    return 0;
}

static kdev_status_t kdev_sensor_r_init(void)
{
    sensor_msg("   <%s>\n", __func__);
    gc1054_r_init(&gc1054_dev[DEV_RIGHT], gc1054_init_regs);
    led_set_light_mode(LED_TORCH_MODE);
    nir_led_init(90);
    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_r_set_fmt(cam_format *fmt)
{
    //sensor_msg("   <%s>\n", __func__);

    gc1054_select_win(&fmt->width, &fmt->height);

    return (kdev_status_t)gc1054_r_set_params(&gc1054_dev[DEV_RIGHT], &fmt->width, &fmt->height, fmt->pixelformat);
}

int kdev_sensor_r_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_RIGHT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_RIGHT].addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x01;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    //set roi
    data = x1 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x01, 1, 1, (uint16_t *)&data);
    data = x2 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x02, 1, 1, (uint16_t *)&data);
    data = y1 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x03, 1, 1, (uint16_t *)&data);
    data = y2 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x04, 1, 1, (uint16_t *)&data);

    //set center roi
    data = center_x1 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x05, 1, 1, (uint16_t *)&data);
    data = center_x2 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x06, 1, 1, (uint16_t *)&data);
    data = center_y1 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x07, 1, 1, (uint16_t *)&data);
    data = center_y2 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x08, 1, 1, (uint16_t *)&data);

    return 0;
}
kdev_status_t kdev_sensor_r_get_lux(uint16_t* exposure, uint8_t* pre_gain_h, uint8_t* pre_gain_l, uint8_t* global_gain, uint8_t* y_average)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_RIGHT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_RIGHT].addr;

    //set to page 0
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    //get exposure
    reg = 0x03;
    *exposure = 0;
    data = 0;
    int ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *exposure |= (data << 8);

    reg = 0x04;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *exposure |= data;

    // set to page 1
    reg = 0xfe;
    data = 0x01;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    //get pre gain
    reg = 0xb1;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *pre_gain_h = (data & 0x0F);

    reg = 0xb2;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *pre_gain_l = data;

    // get global gain
    reg = 0xb0;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *global_gain = data;

//    // set y_average
//    reg = 0x14;
//    data = 0;
//    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, &data);
//    *y_average = data;

    return KDEV_STATUS_OK;
}

kdev_status_t kdev_sensor_r_set_gain(uint32_t ana_gn1, uint32_t ana_gn2)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_RIGHT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_RIGHT].addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x01;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    u16 gain = 0;
    gain = ana_gn1<<8;
    gain = gain | ana_gn2;

    total = sizeof(gainLevelTable) / sizeof(uint32_t);
    uint16_t Analog_Index;
    uint16_t tol_dig_gain = 0;
    for(Analog_Index = 0; Analog_Index < total; Analog_Index++)
    {
        if((gainLevelTable[Analog_Index] <= gain)&&(gain < gainLevelTable[Analog_Index+1]))
            break;
    }

    tol_dig_gain = gain*64/gainLevelTable[Analog_Index];
    //set gain
    kdrv_i2c_write_register(i2c_port, dev_addr, 0xb6, 1, 1, &Analog_Index);
    data = (tol_dig_gain>>6);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0xb1, 1, 1, (uint16_t *)&data);
    data = ((tol_dig_gain&0x3f)<<2);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0xb2, 1, 1, (uint16_t *)&data);

    return KDEV_STATUS_OK;
}

static int kdev_sensor_r_sleep( bool enable )
{
    #define HW_EN 0

    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_RIGHT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_RIGHT].addr;
    uint16_t data;

    if( enable == 1 )
    {
        #if HW_EN == YES
        // pin high
        kdp520_gpio_setdata( 1<<0);
        #else
        data = 0x03;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x10, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf7, 1, 1, &data);
        data = 0x01;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfc, 1, 1, &data);
        data = 0x01;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf9, 1, 1, &data);

        #endif
    }
    else
    {
        #if HW_EN == YES
        // pin low
        kdp520_gpio_cleardata( 1<<0);
        #else
        data = 0x06;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf9, 1, 1, &data);
        data = 0x01;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf7, 1, 1, &data);
        data = 0x0e;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfc, 1, 1, &data);
        data = 0x03;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);
        data = 0x94;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x10, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);

        #endif
    }

    return 0;
}

kdev_status_t kdev_sensor_r_set_exp_time(uint32_t ana_gn1, uint32_t ana_gn2)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_RIGHT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_RIGHT].addr;


    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    //set exp time
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x03, 1, 1, (uint16_t*)&ana_gn1);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x04, 1, 1, (uint16_t*)&ana_gn2);

    return KDEV_STATUS_OK;
}

static kdev_status_t kdev_sensor_r_set_devaddress(uint32_t address, uint32_t port_id)
{
    gc1054_dev[DEV_RIGHT].addr = address;
    gc1054_dev[DEV_RIGHT].i2c_port = port_id;
    return KDEV_STATUS_OK;
}

static uint32_t kdev_sensor_r_get_dev_id(void)
{
    uint8_t data = 0;
    uint16_t id = 0;

    gc1054_r_read_reg(&gc1054_dev[DEV_RIGHT], 0xf0, &data);
    id = data<<8;
    gc1054_r_read_reg(&gc1054_dev[DEV_RIGHT], 0xf1, &data);
    id += data;
    gc1054_dev[DEV_RIGHT].device_id = id;
    return (uint32_t)id;
}

static struct sensor_ops gc1054_r_ops = {
    .set_fmt            = kdev_sensor_r_set_fmt,
    .set_gain           = kdev_sensor_r_set_gain,
    .set_exp_time       = kdev_sensor_r_set_exp_time,
    .get_lux            = kdev_sensor_r_get_lux,
    .set_aec_roi        = kdev_sensor_r_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_dev_id         = kdev_sensor_r_get_dev_id,
    .set_fps            = NULL,
    .sleep              = kdev_sensor_r_sleep,
    .set_addr           = kdev_sensor_r_set_devaddress,
    .init               = kdev_sensor_r_init,
};
#endif

#if (IMGSRC_0_SENSORID == SENSOR_ID_GC1054_L)||(IMGSRC_1_SENSORID == SENSOR_ID_GC1054_L)
static int gc1054_l_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_LEFT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_LEFT].addr;

    ret = kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    return ret;
}
static int gc1054_l_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
    int ret;
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_LEFT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_LEFT].addr;

    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)data);

    return ret;
}

int gc1054_l_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;
    //sensor_msg(" gc1054_init ");
    for (init_fnc_ptr = seq; ; ++init_fnc_ptr) {
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0) break; //reaches end
        gc1054_l_write_reg(dev, init_fnc_ptr->addr , (u8)(init_fnc_ptr->value & 0xFF));
    }
    sensor_msg("gc1054_l_init init over, sensor ID = 0x%x", dev->device_id);
    u8 data = 0;
    gc1054_l_read_reg(dev, 0xf0, &data);
    sensor_msg("gc1054_l_init sensor high id=%x", data);
    dev->device_id = data << 8;
    gc1054_l_read_reg(dev, 0xf1, &data);
    sensor_msg("gc1054_l_init sensor low id=%x", data);
    dev->device_id |= data;

    if(dev->device_id != 0x1054)
        return -1;

    return 0;
}
static int gc1054_l_set_params(
        struct sensor_device *sensor_dev, u32 *width, u32 *height, u32 fourcc)
{
    return 0;
}
static kdev_status_t kdev_sensor_l_init(void)
{
    sensor_msg("   <%s>\n", __func__);
    gc1054_l_init(&gc1054_dev[DEV_LEFT], gc1054_init_regs);
    return KDEV_STATUS_OK;
}
static kdev_status_t kdev_sensor_l_set_fmt(cam_format *fmt)
{
    //sensor_msg("   <%s>\n", __func__);

    gc1054_select_win(&fmt->width, &fmt->height);

    return (kdev_status_t)gc1054_l_set_params(&gc1054_dev[DEV_LEFT], &fmt->width, &fmt->height, fmt->pixelformat);
}

int kdev_sensor_l_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_LEFT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_LEFT].addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x01;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    //set roi
    data = x1 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x01, 1, 1, (uint16_t *)&data);
    data = x2 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x02, 1, 1, (uint16_t *)&data);
    data = y1 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x03, 1, 1, (uint16_t *)&data);
    data = y2 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x04, 1, 1, (uint16_t *)&data);

    //set center roi
    data = center_x1 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x05, 1, 1, (uint16_t *)&data);
    data = center_x2 + ROI_OFFSET_X;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x06, 1, 1, (uint16_t *)&data);
    data = center_y1 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x07, 1, 1, (uint16_t *)&data);
    data = center_y2 + ROI_OFFSET_Y;
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x08, 1, 1, (uint16_t *)&data);

    return 0;
}
kdev_status_t kdev_sensor_l_get_lux(uint16_t* exposure, uint8_t* pre_gain_h, uint8_t* pre_gain_l, uint8_t* global_gain, uint8_t* y_average)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_LEFT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_LEFT].addr;

    //set to page 0
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    //get exposure
    reg = 0x03;
    *exposure = 0;
    data = 0;
    int ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *exposure |= (data << 8);

    reg = 0x04;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *exposure |= data;

    // set to page 1
    reg = 0xfe;
    data = 0x01;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    //get pre gain
    reg = 0xb1;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *pre_gain_h = (data & 0x0F);

    reg = 0xb2;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *pre_gain_l = data;

    // get global gain
    reg = 0xb0;
    data = 0;
    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);
    *global_gain = data;

//    // set y_average
//    reg = 0x14;
//    data = 0;
//    ret = kdrv_i2c_read_register(i2c_port, dev_addr, reg, 1, 1, &data);
//    *y_average = data;

    return KDEV_STATUS_OK;
}

kdev_status_t kdev_sensor_l_set_gain(uint32_t ana_gn1, uint32_t ana_gn2)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_LEFT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_LEFT].addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x01;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t *)&data);

    u16 gain = 0;
    gain = ana_gn1<<8;
    gain = gain | ana_gn2;

    total = sizeof(gainLevelTable) / sizeof(uint32_t);
    uint16_t Analog_Index;
    uint16_t tol_dig_gain = 0;
    for(Analog_Index = 0; Analog_Index < total; Analog_Index++)
    {
        if((gainLevelTable[Analog_Index] <= gain)&&(gain < gainLevelTable[Analog_Index+1]))
            break;
    }

    tol_dig_gain = gain*64/gainLevelTable[Analog_Index];
    //set gain
    kdrv_i2c_write_register(i2c_port, dev_addr, 0xb6, 1, 1, &Analog_Index);
    data = (tol_dig_gain>>6);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0xb1, 1, 1, (uint16_t *)&data);
    data = ((tol_dig_gain&0x3f)<<2);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0xb2, 1, 1, (uint16_t *)&data);

    return KDEV_STATUS_OK;
}


kdev_status_t kdev_sensor_l_set_exp_time(uint32_t ana_gn1, uint32_t ana_gn2)
{
    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_LEFT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_LEFT].addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdrv_i2c_write_register(i2c_port, dev_addr, reg, 1, 1, (uint16_t*)&data);

    //set exp time
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x03, 1, 1, (uint16_t*)&ana_gn1);
    kdrv_i2c_write_register(i2c_port, dev_addr, 0x04, 1, 1, (uint16_t*)&ana_gn2);

    return KDEV_STATUS_OK;
}


static int kdev_sensor_l_sleep( bool enable )
{
    #define HW_EN 0

    kdrv_i2c_ctrl_t i2c_port = (kdrv_i2c_ctrl_t)gc1054_dev[DEV_LEFT].i2c_port;
    uint16_t dev_addr = gc1054_dev[DEV_LEFT].addr;
    uint16_t data;

    if( enable == 1 )
    {
        data = 0x03;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x10, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf7, 1, 1, &data);
        data = 0x01;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfc, 1, 1, &data);
        data = 0x01;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf9, 1, 1, &data);
    }
    else
    {
        data = 0x06;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf9, 1, 1, &data);
        data = 0x01;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xf7, 1, 1, &data);
        data = 0x0e;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfc, 1, 1, &data);
        data = 0x03;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);
        data = 0x94;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0x10, 1, 1, &data);
        data = 0x00;
        kdrv_i2c_write_register(i2c_port, dev_addr, 0xfe, 1, 1, &data);
    }

    return 0;
}
static kdev_status_t kdev_sensor_l_set_devaddress(uint32_t address, uint32_t port_id)
{
    gc1054_dev[DEV_LEFT].addr = address;
    gc1054_dev[DEV_LEFT].i2c_port = port_id;
    return KDEV_STATUS_OK;
}

static uint32_t kdev_sensor_l_get_dev_id(void)
{
    uint8_t data = 0;
    uint16_t id = 0;

    gc1054_l_read_reg(&gc1054_dev[DEV_LEFT], 0xf0, &data);
    id = data<<8;
    gc1054_l_read_reg(&gc1054_dev[DEV_LEFT], 0xf1, &data);
    id += data;
    gc1054_dev[DEV_LEFT].device_id = id;
    return (uint32_t)id;
}


static struct sensor_ops gc1054_l_ops = {
    .set_fmt            = kdev_sensor_l_set_fmt,
    .set_gain           = kdev_sensor_l_set_gain,
    .set_exp_time       = kdev_sensor_l_set_exp_time,
    .get_lux            = kdev_sensor_l_get_lux,
    .set_aec_roi        = kdev_sensor_l_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_dev_id         = kdev_sensor_l_get_dev_id,
    .set_fps            = NULL,
    .sleep              = kdev_sensor_l_sleep,
    .set_addr           = kdev_sensor_l_set_devaddress,
    .init               = kdev_sensor_l_init,
};
#endif

#if (IMGSRC_0_SENSORID == SENSOR_ID_GC1054_R)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 0, &gc1054_r_ops);
#endif
#if (IMGSRC_1_SENSORID == SENSOR_ID_GC1054_R)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 1, &gc1054_r_ops);
#endif

#if (IMGSRC_0_SENSORID == SENSOR_ID_GC1054_L)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 0, &gc1054_l_ops);
#endif
#if (IMGSRC_1_SENSORID == SENSOR_ID_GC1054_L)
KDEV_CAM_SENSOR_DRIVER_REGISTER(cam_sensor, 1, &gc1054_l_ops);
#endif


#endif
