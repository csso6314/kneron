#include "config/board_kdp520.h"
#if V2K_ENABLE_TYPE == V2K_ENABLE_HMXRICA
#include <stdlib.h>
#include "base.h"
#include "framework/init.h"
#include "framework/framework_driver.h"
#include "framework/v2k.h"
#include "framework/v2k_image.h"
#include "framework/framework_errno.h"

#include "media/camera/sensor.h"
#include "media/camera/sys_camera.h"
#include "media/v2k_subdev.h"
#include "utility.h"
#include "kmdw_console.h"


struct hmxrica_context {
    struct v2k_subdev subdev;
    const struct hmxrica_datafmt *fmt;
};

static const struct sensor_datafmt_info hmxrica_colour_fmts[] = {
    { V2K_PIX_FMT_RAW8, V2K_COLORSPACE_RAW },
};

static const struct sensor_win_size hmxrica_supported_win_sizes[] = {
    { .width = HMX_RICA_WIDTH, .height = HMX_RICA_HEIGHT, },
};

static struct sensor_init_seq hmxrica_init_regs[] = {
    {0xC005, 0x49},
    {0xC073, 0x65},
    //pll PLL_O 200MHZ, MCLK is 24, where MCU SPI clock = 25MHz.
    {0xC092, 0x42},
    {0xC093, 0x64},
    //pll PLL_R 200MHZ, where Uploader SPI clock = 50MHz
    {0xC0B9, 0x01},
    //{0xC0A2, 0x42},   //dont need to set it , since it is default.
    {0xC0A3, 0x64},
    {0xC0AA, 0x06},
    {0xC0BF, 0x03},
    {0xC0A0, 0x01},
    {0xC0BF, 0x03},
    {0xC0A0, 0x03},
    {0xC0BF, 0x03},
    //enable N9
    {0xc004, 0x00},
    {0xc0bf, 0x01},
    {0xc003, 0x00},
    {0xc0b6, 0x01},         // register for selecting group of AE Init Parameater.
    {0xc07F, 0x01},         // register for selecting designated package to boot.
    {0xc0bf, 0x01},
    {0xc249, 0x000},
    //{0xc24A, 0x011},      //interleave 11
    //{0xc24B, 0x011},
    //{0xc24C, 0x011},
    {0xc24A, 0x012},        //interleave 12
    {0xc24B, 0x012},        //interleave 12
    {0xc24C, 0x012},        //interleave 12
    {0xC0BF, 0x003},
    // Modified new test test pattern
    {0xC026, 0x007},
    {0xC0BF, 0x003},
    {0x0, 0x0},
};

static int hmxrica_write_reg(struct sensor_device *sensor_dev, uint16_t reg, uint8_t data)
{
    int ret;

    ret = kdp_drv_i2c_write(I2C_ADAP_0, sensor_dev->addr, reg, 2, data);

    return ret;
}

void hmxrica_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq)
{
       struct sensor_device *dev = sensor_dev;
       struct sensor_init_seq *init_fnc_ptr;

    for (init_fnc_ptr = seq; ; ++init_fnc_ptr) {
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0) break; //reaches end
          hmxrica_write_reg(dev, init_fnc_ptr->addr ,init_fnc_ptr->value);
    }
}

static struct hmxrica_context *to_hmxrica(const struct sensor_device *sensor_dev)
{
    return container_of(framework_drv_get_drvdata(&sensor_dev->pin_ctx), struct hmxrica_context, subdev);
}

static const struct sensor_win_size *hmxrica_select_win(uint32_t *width, uint32_t *height)
{
    int i, default_size = ARRAY_SIZE(hmxrica_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(hmxrica_supported_win_sizes); i++) {
        if (hmxrica_supported_win_sizes[i].width  >= *width &&
            hmxrica_supported_win_sizes[i].height >= *height) {
            *width = hmxrica_supported_win_sizes[i].width;
            *height = hmxrica_supported_win_sizes[i].height;
            return &hmxrica_supported_win_sizes[i];
        }
    }

    *width = hmxrica_supported_win_sizes[default_size].width;
    *height = hmxrica_supported_win_sizes[default_size].height;
    return &hmxrica_supported_win_sizes[default_size];
}

static
int hmxrica_set_params(struct sensor_device *sensor_dev)
{
    struct hmxrica_context *ctx = to_hmxrica(sensor_dev);

    /* initialize the sensor with default settings */

    hmxrica_init(sensor_dev, hmxrica_init_regs);

    return 0;
}

static int hmxrica_s_power(struct v2k_subdev *sd, int on)
{


    return 0;
}

static int hmxrica_s_stream(struct v2k_subdev *sd, int enable)
{


    return 0;
}

static int hmxrica_enum_fmt(
        struct v2k_subdev *sd, unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(hmxrica_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    *code = hmxrica_colour_fmts[index].fourcc;
    return 0;
}

static int hmxrica_get_fmt(struct v2k_subdev *sd, struct v2k_format *format)
{
    return 0;
}

static int hmxrica_set_fmt(struct v2k_subdev *sd, struct v2k_format *format)
{
    struct v2k_format *fmt = format;
    struct sensor_device *sensor_dev = v2k_get_subdevdata(sd);

    //DSG("[%s]", __func__);

    hmxrica_select_win(&fmt->width, &fmt->height);

    return hmxrica_set_params(sensor_dev);
}

static struct v2k_subdev_ops hmxrica_subdev_ops = {
    .s_power    = hmxrica_s_power,
    .s_stream   = hmxrica_s_stream,
    .enum_fmt   = hmxrica_enum_fmt,
    .get_fmt    = hmxrica_get_fmt,
    .set_fmt    = hmxrica_set_fmt,
};


static int hmxrica_probe(struct sensor_device *sensor_dev)
{
    int ret;
    struct hmxrica_context *ctx;

    ctx = calloc(1, sizeof(struct hmxrica_context));
    if (!ctx)
        return -KDP_FRAMEWORK_ERRNO_NOMEM;

    v2k_subdev_sensor_init(&ctx->subdev, sensor_dev, &hmxrica_subdev_ops);

    ctx->fmt = &hmxrica_colour_fmts[0];

    ret = 0;

    return ret;
}

static int hmxrica_remove(struct sensor_device *sensor_dev)
{
    //free_bus
    return 0;
}

extern struct core_device hmxrica_link_device;
struct sensor_driver hmxrica_i2c_driver = {
    .driver = {
        .name = "sensor-hmxrica",
    },
    .probe      = hmxrica_probe,
    .remove     = hmxrica_remove,
    .core_dev   = &hmxrica_link_device,
};
KDP_SENSOR_DRIVER_SETUP(hmxrica_i2c_driver);

#endif
