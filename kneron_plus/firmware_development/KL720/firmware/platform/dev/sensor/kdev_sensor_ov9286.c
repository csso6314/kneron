#include "config/board_kdp520.h"
#if (V2K_ENABLE_TYPE == V2K_ENABLE_OV9286) || (V2K_ENABLE_TYPE == V2K_ENABLE_HMX2056_OV9286)
#include <stdlib.h>
#include "base.h"
#include "framework/init.h"
#include "framework/v2k.h"
#include "framework/v2k_image.h"
#include "framework/framework_errno.h"

#include "media/camera/sensor.h"
#include "media/camera/sys_camera.h"
#include "media/v2k_subdev.h"
#include "utility.h"
#include "kmdw_console.h"


struct ov9286_context {
    struct v2k_subdev subdev;
    const struct sensor_datafmt_info *fmt;
};

static const struct sensor_datafmt_info ov9286_colour_fmts[] = {
    { V2K_PIX_FMT_RAW10, V2K_COLORSPACE_RAW },
    { V2K_PIX_FMT_RAW8, V2K_COLORSPACE_RAW },
};

static const struct sensor_win_size ov9286_supported_win_sizes[] = {
    { .width = TFT43_WIDTH, .height = TFT43_HEIGHT, },
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },
};

//#define JEFF_SUGGESTION
#define FPS_30      0
#define FPS_16      1
#define OV8286_FPS  FPS_30
//#define ORG
struct sensor_init_seq INITDATA ov9286_init_regs[] = {
{0x0100,0x00}, //software standby mode
{0x0103,0x01}, //software_reset
#ifdef ORG
    {0x0302,0x32},
    {0x030d,0x50},
    {0x030e,0x02},
#else

{0x030A,0x00},//PLL1: pre_div0, 24/1=24MHz
{0x0300,0x01},//PLL1: pre_div, 24/1.5=16MHz
{0x0301,0x00},
{0x0302,0x32},//PLL1: mul, 16MHz*50=800MHz
{0x0305,0x02},//PLL1: sys_pre_div, 800/5=160MHz
{0x0306,0x01},//PLL1: sys_div, 160/2=80MHz (PLL1 sys clk)
{0x0303,0x00},//PLL1: MIPI pre div, 800/1=800MHz
{0x0304,0x03},//PLL1: MIPI div, 800/8=100MHz, (PLL1 pixclk)(MIPI_PCLK) -> *8=800Mbps(MIPI_CLK)

{0x0314,0x00},//PLL2: pre_div0, 24/1=24HMz
{0x030B,0x04},//PLL2: pre div, 24/3=8MHz
{0x030C,0x00},
{0x030D,0x50},//PLL2: mul, 8MHz*80=640MHz
{0x030F,0x03},//PLL2: sys_pre_div, 640/4=160MHz
{0x030E,0x02},//PLL2, sys_div, 160/2=80MHz (PLL2 sys clk)
{0x0313,0x01},//PLL2, ADC div, 640/2=320MHz
{0x0312,0x07},//PLL2, analog div, 640/8=80MHz
#endif

{0x3001,0x00}, //drive strength control
{0x3004,0x00}, //GPIO2 output enable : input. D9 output enable : input
{0x3005,0x00}, //Bit[0] ~ B it[7] : input
{0x3006,0x04}, //ILPWM : output. D0/PCLK/HREF/Strobe output/VSYNC : input
{0x3011,0x0a}, //mipi_pad : 1, pgm_vcm : high speed common mode voltage
{0x3013,0x18}, //pgm_lptx (Driving strength control of low speed tx), r_iref
{0x301c,0xf0}, //sclk_bist/sclk_srb/sclk_grp : 1
{0x3022,0x01}, //pd_mipi enable when rst_sync
{0x3030,0x10}, //r_aslp_repeat
{0x3039,0x12}, //mipi_en (0:DVB 1:MIPI) phy_rst (1:Reset PHY when rst_sync)
{0x303a,0x00}, //MIPI lane disable : 0
{0x3500,0x00}, //exposure

#ifdef JEFF_SUGGESTION
//second
//{0x3501,0x2a}, //exposure
//{0x3502,0x90}, //exposure time : f
//third
{0x3501,0x38}, //exposure
{0x3502,0x20}, //exposure time : f
#else
{0x3501,0x01}, //exposure
{0x3502,0xf4}, //exposure time : f
#endif

{0x3503,0x08}, //gain_prec16_en
{0x3505,0x8c}, //gain conversation option. dac_finegain_highbit : 1;
{0x3507,0x03}, //GAIN SHIFT. left shift 3 bit.
{0x3508,0x00}, //Debug mode
{0x3509,0x10}, //Gain. 1x gain
{0x3610,0x80},
{0x3611,0xa0},
{0x3620,0x6e},
{0x3632,0x56},
{0x3633,0x78},

#if IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW10
{0x3662,0x00},//RAW10
#elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW8
{0x3662,0x02},//RAW8
#endif

{0x3666,0x00}, // output selection 0x0:VSYNC. from fsin pin, used for both frame sync and frame rigger function
{0x366f,0x5a},
{0x3680,0x84},
{0x3712,0x80},
{0x372d,0x22},
{0x3731,0x80},
{0x3732,0x30},

#ifdef JEFF_SUGGESTION
{0x3778,0x00}, // 2x vertical binning enable for monochrome mode
#else
{0x3778,0x10}, // 2x vertical binning enable for monochrome mode
#endif

{0x377d,0x22},
{0x3788,0x02},
{0x3789,0xa4},
{0x378a,0x00},
{0x378b,0x4a},
{0x3799,0x20},
{0x3800,0x00}, // array horizontal start point high
{0x3801,0x00}, // array horizontal start point low
{0x3802,0x00}, // array vertical start point high
{0x3803,0x00}, // array vertical start point low

{0x3804,0x05}, // array horizontal end point high byte
{0x3805,0x0f}, // array horizontal end point low byte
{0x3806,0x03}, // array vertical end point high byte
{0x3807,0x2f}, // array vertical end point low byte

#if IMGSRC_1_RES == RES_640_480
    {0x3808,0x02}, // isp horizontal output width high byte
    {0x3809,0x80}, // isp horizontal output width low byte
    {0x380a,0x01}, // isp vertical output height high byte
    {0x380b,0xe0}, // isp vertical output height low byte
#elif IMGSRC_1_RES == RES_480_272
    {0x3808,0x01}, // isp horizontal output width high byte
    {0x3809,0xe0}, // isp horizontal output width low byte
    {0x380a,0x01}, // isp vertical output height high byte
    {0x380b,0x10}, // isp vertical output height low byte
#endif


{0x380c,0x02}, // total horizontal timing size high byte
{0x380d,0xd8}, // total horizontal timing size low byte

#ifdef JEFF_SUGGESTION
    #if OV8286_FPS == FPS_30
    {0x380e,0x0e},
    {0x380f,0x38},
    #elif OV8286_FPS == FPS_16
    {0x380e,0x1a},
    {0x380f,0xb0},
    #endif
#else
{0x380e,0x02}, // total vertical timing size high byte
{0x380f,0x25}, // total vertical timing size low byte
#endif

#ifdef JEFF_SUGGESTION
{0x3810,0x00}, // isp horizontal windowing offset high byte
{0x3811,0x08}, // isp horizontal windowing offset low byte
{0x3812,0x00}, // isp vertical windowing offset high byte
{0x3813,0x08}, // isp vertical windowing offset low byte
{0x3814,0x11}, // x_odd_inc : 3, x_even_inc : 1
{0x3815,0x11}, // y_odd_inc : 2, y_even_inc : 2
{0x3820,0x40}, // Vflip. vflip_blc : 0
{0x3821,0x00}, // 4x horizontal binning enable
#else
{0x3810,0x00}, // isp horizontal windowing offset high byte
{0x3811,0x54}, // isp horizontal windowing offset low byte
{0x3812,0x00}, // isp vertical windowing offset high byte
{0x3813,0x6c}, // isp vertical windowing offset low byte
{0x3814,0x31}, // x_odd_inc : 3, x_even_inc : 1
{0x3815,0x22}, // y_odd_inc : 2, y_even_inc : 2
{0x3820,0x04}, // Vflip. vflip_blc : 0
{0x3821,0x01}, // 4x horizontal binning enable
#endif


{0x382c,0x05},
{0x382d,0xb0},
{0x389d,0x00},
{0x3881,0x42},
{0x3882,0x01},
{0x3883,0x00},
{0x3885,0x02},
{0x38a8,0x02},
{0x38a9,0x80},
{0x38b1,0x00},
{0x38b3,0x02},
{0x38c4,0x00},
{0x38c5,0xc0},
{0x38c6,0x04},
{0x38c7,0x80},
{0x3920,0xff}, // strobe_pattern
{0x4003,0x40},

#ifdef JEFF_SUGGESTION
{0x4008,0x04}, // r_up_bl_start_o
{0x4009,0x0b}, // r_up_bl_end_o
#else
{0x4008,0x02}, // r_up_bl_start_o
{0x4009,0x05}, // r_up_bl_end_o
#endif

{0x400c,0x00}, // r_dn_bl_start_o

#ifdef JEFF_SUGGESTION
{0x400d,0x07}, // r_dn_bl_end_o
#else
{0x400d,0x03}, // r_dn_bl_end_o
#endif


{0x4010,0x40}, // r_gain_chg_trig_en_o
{0x4043,0x40}, // r_bot_blk_in_en_o
{0x4307,0x30}, // embed_st=3
{0x4317,0x00}, // dvp enable = 0
{0x4501,0x00},

#ifdef JEFF_SUGGESTION
{0x4507,0x00},
{0x4509,0x00},
#else
{0x4507,0x03},
{0x4509,0x80},
#endif

{0x450a,0x08},

#ifdef JEFF_SUGGESTION
//which one ???
//first time
//{0x4601,0x4f}, // VFIFO read start point low byte
//second time
{0x4601,0x04}, // VFIFO read start point low byte
#else
{0x4601,0x04}, // VFIFO read start point low byte
#endif

{0x470f,0x00}, // BYP_SEL, href_sel, bypass_sel = 0
{0x4f07,0x00}, // r_pchg_st_offs ?
{0x4800,0x00}, // clklane first bit = 8'h55, use falling edge of mipi_pclk_o to generate MIPI bus to PHY
#ifdef JEFF_SUGGESTION
{0x4837,0x14},
#endif

#ifdef JEFF_SUGGESTION
{0x5000,0x87}, // isp_sof_sel = 2, bc_en, wc_en, dpc_buf_en, awbg_en, blc_en
#else
{0x5000,0x9f}, // isp_sof_sel = 2, bc_en, wc_en, dpc_buf_en, awbg_en, blc_en
#endif

{0x5001,0x00}, // bypass_isp0, bypass_isp1
{0x5e00,0x00}, // disable test pattern
{0x5d00,0x07},
{0x5d01,0x00},
{0x4f00,0x04}, // r_psv_mode_en : enable
{0x4f10,0x00}, // ana_psv_pch[15:8]
{0x4f11,0x98}, // ana_psv_pch[7:0]
{0x4f12,0x0f}, // ana_psv_strm
{0x4f13,0xc4}, // ana_psv_strm

{0x3501,0x20}, // exposure
{0x3502,0x20}, // exposure

{0x5E00,0x80},

{0x0100,0x01}, // streaming
{ 0,0},
};


static int ov9286_write_reg(struct sensor_device *sensor_dev, uint16_t reg, uint8_t data)
{
    int ret;

    ret = kdp_drv_i2c_write(I2C_ADAP_0, sensor_dev->addr, reg, 2, data);

    return ret;
}

void ov9286_init(struct sensor_device *sensor_device, struct sensor_init_seq *seq)
{
    struct sensor_device *sensor_dev = sensor_device;
    struct sensor_init_seq *init_fnc_ptr;

    for (init_fnc_ptr = seq; ; ++init_fnc_ptr) {
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0) { DSG("byte"); break; }//reaches end
        ov9286_write_reg(sensor_dev, init_fnc_ptr->addr , (uint8_t)(init_fnc_ptr->value & 0xFF));
    }
    //DSG("%s done", __func__);
}

static struct ov9286_context *to_ov9286(const struct sensor_device *sensor_dev)
{
    return container_of(framework_drv_get_drvdata(&sensor_dev->pin_ctx), struct ov9286_context, subdev);
}

static const struct sensor_win_size *ov9286_select_win(uint32_t *width, uint32_t *height)
{
    int i, default_size = ARRAY_SIZE(ov9286_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(ov9286_supported_win_sizes); i++) {
        if (ov9286_supported_win_sizes[i].width  >= *width &&
            ov9286_supported_win_sizes[i].height >= *height) {
            *width = ov9286_supported_win_sizes[i].width;
            *height = ov9286_supported_win_sizes[i].height;
            return &ov9286_supported_win_sizes[i];
        }
    }

    *width = ov9286_supported_win_sizes[default_size].width;
    *height = ov9286_supported_win_sizes[default_size].height;
    return &ov9286_supported_win_sizes[default_size];
}

static
int ov9286_set_params(struct sensor_device *sensor_dev)
{
    struct ov9286_context *ctx = to_ov9286(sensor_dev);

    //DSG("[%s] ctx=%p fourcc=%x", __func__, ctx, fourcc);
    DSG("ov9286_init start");
    /* initialize the sensor with default settings */
    ov9286_init(sensor_dev, ov9286_init_regs);
    DSG("ov9286_init end");

    return 0;
}

static int ov9286_s_power(struct v2k_subdev *sd, int on)
{
    int ret;

    DSG("%s", __func__);

    ret = 0;

    return ret;
}

static int ov9286_reset(struct v2k_subdev *sd)
{
    //DSG("%s", __func__);
    return 0;
}

static int ov9286_s_stream(struct v2k_subdev *sd, int enable)
{
    return 0;
}

static int ov9286_enum_fmt(
        struct v2k_subdev *sd, unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(ov9286_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    *code = ov9286_colour_fmts[index].fourcc;
    return 0;
}

static int ov9286_get_fmt(struct v2k_subdev *sd, struct v2k_format *format)
{
    struct sensor_device *sensor_dev = v2k_get_subdevdata(sd);

    return 0;
}

static int ov9286_set_fmt(struct v2k_subdev *sd, struct v2k_format *format)
{
    struct v2k_format *fmt = format;
    struct sensor_device *sensor_dev = v2k_get_subdevdata(sd);

    DSG("[%s]", __func__);

    ov9286_select_win(&fmt->width, &fmt->height);

    return ov9286_set_params(sensor_dev);
}

static struct v2k_subdev_ops ov9286_subdev_ops = {
    .s_power	= ov9286_s_power,
    .reset	    = ov9286_reset,
    .s_stream	= ov9286_s_stream,
    .enum_fmt 	= ov9286_enum_fmt,
    .get_fmt	= ov9286_get_fmt,
    .set_fmt	= ov9286_set_fmt,
};

static int ov9286_probe(struct sensor_device *sensor_dev)
{
    int ret;
    struct ov9286_context *ctx;

    ctx = calloc(1, sizeof(struct ov9286_context));
    if (!ctx)
        return -KDP_FRAMEWORK_ERRNO_NOMEM;

    v2k_subdev_sensor_init(&ctx->subdev, sensor_dev, &ov9286_subdev_ops);

    ctx->fmt = &ov9286_colour_fmts[0];

    ret = 0;

    return ret;
}

static int ov9286_remove(struct sensor_device *sensor_dev)
{
    //free_bus

    return 0;
}

extern struct core_device ov9286_link_device;
static struct sensor_driver ov9286_i2c_driver = {
    .driver = {
        .name = "sensor-ov9286",
    },
    .probe		= ov9286_probe,
    .remove		= ov9286_remove,
    .core_dev   = &ov9286_link_device,
};
KDP_SENSOR_DRIVER_SETUP(ov9286_i2c_driver);

#endif
