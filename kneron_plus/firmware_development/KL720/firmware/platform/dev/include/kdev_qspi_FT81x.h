/********************************************************************
 * Copyright (c) 2021 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/
#include "project.h"
#include "base.h"
#include "kdev_status.h"
#include "kdrv_gdma3.h"

#define FT810_CHIP_ID   0x00011008
#define FT811_CHIP_ID   0x00011108
#define FT812_CHIP_ID   0x00011208
#define FT813_CHIP_ID   0x00011308
#define FT81X_ID        0x7C
#define FT81X_RAM_G_SIZE  (1024*1024)
#define FT81X_MAX_WRITE_SIZE 65536

/**@brief Enumerations of ft81x host commands. */
typedef enum
{
    FT81X_CMD_ACTIVE = 0x00,
    FT81X_CMD_STANDBY = 0x41,
    FT81X_CMD_SLEEP = 0x42,
    FT81X_CMD_PWRDOWN = 0x43,
    FT81X_CMD_PDROMS = 0x49,
    FT81X_CMD_CLKEXT = 0x44,
    FT81X_CMD_CLKINT = 0x48,
    FT81X_CMD_CLKSEL = 0x61,
    FT81X_CMD_RST_PULSE = 0x68,
    FT81X_CMD_PINDRIVE = 0x70,
    FT81X_CMD_PINPDSTATE = 0x71,
    FT81X_CMD_MAX = 0xFF
} FT81X_CMD;

typedef enum
{
    FT81X_PWR_ACTIVE = 0,
    FT81X_PWR_STANDBY,
    FT81X_PWR_SLEEP,
    FT81X_PWR_PWRDOWN,
} FT81X_PWR_STATE;

typedef enum
{
    FT81X_SPIWIDTH_SINGLE = 0,      //Max Bus Speed 30MHz
    FT81X_SPIWIDTH_DUAL,            //Max Bus Speed 30MHz
    FT81X_SPIWIDTH_QUAD,            //Max Bus Speed 25MHz
    FT81X_SPIWIDTH_MAX
} FT81X_SPIWIDTH;

typedef enum
{
    FT81X_REG_ID = 0x302000,        //Identification register, always reads as 7Ch
    FT81X_REG_FRAMES = 0x302004,    //Frame counter, since reset
    FT81X_REG_CLOCK = 0x302008,     //Clock cycles, since reset
    FT81X_REG_FREQUENCY = 0x30200C, //Main clock frequency (Hz)
    FT81X_REG_RENDERMODE = 0x302010,//Rendering mode: 0 = normal, 1 = single-line
    FT81X_REG_SNAPY = 0x30302014,   //Scanline select for RENDERMODE 1
    FT81X_REG_SNAPSHOT = 0x302018,  //Trigger for RENDERMODE 1
    FT81X_REG_SNAPFORMAT = 0x30201C,//Pixel format for scanline readout
    FT81X_REG_CPURESET = 0x302020,  //Graphics, audio and touch engines reset control. Bit2: audio, bit1: touch, bit0: graphics
    FT81X_REG_TAP_CRC = 0x302024,   //Live video tap crc. Frame CRC is computed every DL SWAP.
    FT81X_REG_TAP_MASK = 0x302028,  //Live video tap mask
    FT81X_REG_HCYCLE = 0x30202C,    //Horizontal total cycle count
    FT81X_REG_HOFFSET = 0x302030,   //Horizontal display start offset
    FT81X_REG_HSIZE = 0x302034,     //Horizontal display pixel count
    FT81X_REG_HSYNC0 = 0x302038,    //Horizontal sync fall offset
    FT81X_REG_HSYNC1 = 0x30203C,    //Horizontal sync rise offset
    FT81X_REG_VCYCLE = 0x302040,    //Vertical total cycle count
    FT81X_REG_VOFFSET = 0x302044,   //Vertical display start offset
    FT81X_REG_VSIZE = 0x302048,     //Vertical display line count
    FT81X_REG_VSYNC0 = 0x30204C,    //Vertical sync fall offset
    FT81X_REG_VSYNC1 = 0x302050,    //Vertical sync rise offset
    FT81X_REG_DLSWAP = 0x302054,    //Display list swap control
    FT81X_REG_ROTATE = 0x302058,    //Screen rotation control. Allow normal/mirrored/inverted for landscape or portrait orientation.
    FT81X_REG_OUTBITS = 0x30205C,   //Output bit resolution, 3 bits each for R/G/B. Default is 6/6/6 bits for FT810/FT811, and 8/8/8 bits for FT812/FT813 (0b’000 means 8 bits)
    FT81X_REG_DITHER = 0x302060,    //Output dither enable
    FT81X_REG_SWIZZLE = 0x302064,   //Output RGB signal swizzle
    FT81X_REG_CSPREAD = 0x302068,   //Output clock spreading enable
    FT81X_REG_PCLK_POL = 0x30206C,  //PCLK polarity: 0 = output on PCLK rising edge, 1 = output on PCLK falling edge
    FT81X_REG_PCLK = 0x302070,      //PCLK frequency divider, 0 = disable
    FT81X_REG_TAG_X = 0x302074,     //Tag query X coordinate
    FT81X_REG_TAG_Y = 0x302078,     //Tag query Y coordinate
    FT81X_REG_TAG = 0x30207C,       //Tag query result
    FT81X_REG_GPIO_DIR = 0x302090,  //Legacy GPIO pin direction, 0 = input , 1 = output
    FT81X_REG_GPIO = 0x302094,      //Legacy GPIO read/write
    FT81X_REG_GPIOX_DIR = 0x302098, //Extended GPIO pin direction, 0 = input , 1 = output
    FT81X_REG_GPIOX = 0x30209C,     //Extended GPIO read/write
    FT81X_REG_INT_FLAGS = 0x3020A8, //Interrupt flags, clear by read
    FT81X_REG_INT_EN = 0x3020AC,    //Global interrupt enable, 1=enable
    FT81X_REG_INT_MASK = 0x3020B0,  //Individual interrupt enable, 1=enable
    FT81X_REG_PWM_HZ = 0x3020D0,    //BACKLIGHT PWM output frequency (Hz)
    FT81X_REG_PWM_DUTY = 0x3020D4,  //BACKLIGHT PWM output duty cycle 0=0%, 128=100%
    FT81X_REG_MACRO_0 = 0x3020D8,   //Display list macro command 0
    FT81X_REG_MACRO_1 = 0x3020DC,   //Display list macro command 1
    FT81X_REG_CMD_READ = 0x3020F8,  //Command buffer read pointer
    FT81X_REG_CMD_WRITE = 0x3020FC, //Command buffer write pointer
    FT81X_REG_CMD_DL = 0x302100,    //Command display list offset
    FT81X_REG_BIST_EN = 0x302174,   //BIST memory mapping enable
    FT81X_REG_TRIM = 0x302180,      //Internal relaxation clock trimming
    FT81X_REG_ANA_COMP = 0x302184,  //Analogue control register
    FT81X_REG_SPI_WIDTH = 0x302188, //QSPI bus width setting Bit [2]: extra dummy cycle on read Bit [1:0]: bus width (0=1-bit, 1=2-bit, 2=4-bit)
    FT81X_REG_DATESTAMP = 0x302564, //Stamp date code
    FT81X_REG_CMDB_SPACE = 0x302574,//Command DL (bulk) space available
    FT81X_REG_CMDB_WRITE = 0x302578,//Command DL (bulk) write
    FT81X_CHIP_ID = 0x0C0000,       //The FT81x Chip ID can be read at memory location 0C0000h – 0C0003h
} FT81X_RAM_REG;

typedef enum
{
    FT81X_ARGB1555 = 0, //0
    FT81X_L1,           //1
    FT81X_L4,           //2
    FT81X_L8,           //3
    FT81X_RGB332,       //4
    FT81X_ARGB2,        //5
    FT81X_ARGB4,        //6
    FT81X_RGB565,       //7
    FT81X_RSVD,         //8
    FT81X_TEXT8X8,      //9
    FT81X_TEXTVGA,      //10
    FT81X_BARGRAPH,     //11
    FT81X_RSVD1,        //12
    FT81X_RSVD2,        //13
    FT81X_PALETTED565,  //14
    FT81X_PALETTED4444, //15
    FT81X_PALETTED8,    //16
    FT81X_L2            //17
} FT81X_LAYOUT_FORMAT;

typedef enum
{
    FT81X_DL_ALPHA_FUNC = 0x09,     //set the alpha test function
    FT81X_DL_BITMAP_HANDLE = 0x05,  //set the bitmap handle
    FT81X_DL_BITMAP_LAYOUT = 0x07,  //set the source bitmap memory format and layout for the current handle
    FT81X_DL_BITMAP_LAYOUT_H = 0x28,//set the source bitmap memory format and layout for the current handle
    FT81X_DL_BITMAP_SIZE = 0x08,    //set the screen drawing of bitmaps for the current handle
    FT81X_DL_BITMAP_SIZE_H = 0x29,  //set the screen drawing of bitmaps for the current handle
    FT81X_DL_BITMAP_SOURCE = 0x01,  //set the source address for bitmap graphics
    FT81X_DL_TRANSFORM_A = 0x15,    //set the components of the bitmap transform matrix
    FT81X_DL_TRANSFORM_B = 0x16,    //set the components of the bitmap transform matrix
    FT81X_DL_TRANSFORM_C = 0x17,    //set the components of the bitmap transform matrix
    FT81X_DL_TRANSFORM_D = 0x18,    //set the components of the bitmap transform matrix
    FT81X_DL_TRANSFORM_E = 0x19,    //set the components of the bitmap transform matrix
    FT81X_DL_TRANSFORM_F = 0x1A,    //set the components of the bitmap transform matrix
    FT81X_DL_BLEND_FUNC = 0x0B,     //set pixel arithmetic function
    FT81X_DL_CELL = 0x06,           //set the bitmap cell number for the VERTEX2F command
    FT81X_DL_CLEAR = 0x26,          //clear buffers to preset values
    FT81X_DL_CLEAR_COLOR_A = 0x0F,  //set clear value for the alpha channel
    FT81X_DL_CLEAR_COLOR_RGB = 0x02,//set clear values for red, green and blue channels
    FT81X_DL_CLEAR_STENCIL = 0x11,  //set clear value for the stencil buffer
    FT81X_DL_CLEAR_TAG = 0x12,      //set clear value for the tag buffer
    FT81X_DL_COLOR_A = 0x10,        //set the current color alpha
    FT81X_DL_COLOR_MASK = 0x20,     //enable or disable writing of color components
    FT81X_DL_COLOR_RGB = 0x04,      //set the current color red, green and blue
    FT81X_DL_LINE_WIDTH = 0x0E,     //set the line width
    FT81X_DL_POINT_SIZE = 0x0D,     //set point size
    FT81X_DL_RESTORE_CONTEXT = 0x23,//restore the current graphics context from the context stack
    FT81X_DL_SAVE_CONTEXT = 0x22,   //push the current graphics context on the context stack
    FT81X_DL_SCISSOR_SIZE = 0x1C,   //set the size of the scissor clip rectangle
    FT81X_DL_SCISSOR_XY = 0x1B,     //set the top left corner of the scissor clip rectangle
    FT81X_DL_STENCIL_FUNC = 0x0A,   //set function and reference value for stencil testing
    FT81X_DL_STENCIL_MASK = 0x13,   //control the writing of individual bits in the stencil planes
    FT81X_DL_STENCIL_OP = 0x0C,     //set stencil test actions
    FT81X_DL_TAG = 0x03,            //set the current tag value
    FT81X_DL_TAG_MASK = 0x14,       //control the writing of the tag buffer
    FT81X_DL_VERTEX_FORMAT = 0x27,  //set the precision of VERTEX2F coordinates
    FT81X_DL_VERTEX_TRANS_X = 0x2B, //specify the vertex transformation's X translation component
    FT81X_DL_VERTEX_TRANS_Y = 0x2C, //specify the vertex transformation's Y translation component
    FT81X_DL_PALETTE_SOURCE = 0x2A, //Specify the base address of the palette
    FT81X_DL_BEGIN = 0x1F,          //start drawing a graphics primitive
    FT81X_DL_END = 0x21,            //finish drawing a graphics primitive
    FT81X_DL_VERTEX2F = 0x01,       //supply a vertex with fractional coordinates
    FT81X_DL_VERTEX2II = 0x02,      //supply a vertex with unsigned coordinates
    FT81X_DL_NOP = 0x2D,            //No Operation
    FT81X_DL_JUMP = 0x1E,           //execute commands at another location in the display list
    FT81X_DL_MACRO = 0x25,          //execute a single command from a macro register
    FT81X_DL_CALL = 0x1D,           //execute a sequence of commands at another location in the display list
    FT81X_DL_RETURN = 0x24,         //return from a previous CALL command
    FT81X_DL_DISPLAY = 0x00,        //end the display list
} FT81X_RAM_DL;

typedef enum
{
    PRIM_BITMAPS = 1,               //1 //Bitmap drawing primitive
    PRIM_POINTS,                    //2 //Point drawing primitive
    PRIM_LINES,                     //3 //Line drawing primitive
    PRIM_LINE_STRIP,                //4 //Line strip drawing primitive
    PRIM_EDGE_STRIP_R,              //5 //Edge strip right side drawing primitive
    PRIM_EDGE_STRIP_L,              //6 //Edge strip left side drawing primitive
    PRIM_EDGE_STRIP_A,              //7 //Edge strip above drawing primitive
    PRIM_EDGE_STRIP_B,              //8 //Edge strip below side drawing primitive
    PRIM_RECTS                      //9 //Rectangle drawing primitive
} FT81X_PRIM;

typedef enum
{
    MEM_RAM_G = 0x000000,               //General purpose graphics RAM
    MEM_ROM_FONT = 0x1E0000,            //Font table and bitmap
    MEM_ROM_FONT_ADDR = 0x2FFFFC,       //Font table pointer address   
    MEM_RAM_DL = 0x300000,              //Display List RAM
    MEM_RAM_REG = 0x302000,             //Registers
    MEM_RAM_CMD = 0x308000,             //Command buffer
} FT81X_MEM_MAP;

typedef enum
{
    ROM_MAIN = BIT7,
    ROM_RCOSATAN = BIT6,
    ROM_SAMPLE = BIT5,
    ROM_JABOOT = BIT4,
    ROM_J1BOOT = BIT3,
    //BIT[2:0] rsvd
} FT81X_ROMS;

/* 2 to 5 times the osc frequency (i.e. 24 to 60MHz with 12MHz oscillator) */
typedef enum
{
    SPI_CLK_2_TIMES = 0x02,
    SPI_CLK_3_TIMES = 0x03,
    SPI_CLK_4_TIMES = 0x04,
    SPI_CLK_5_TIMES = 0x05,
} FT81X_CLK_MULTIPLY;

typedef enum
{
    PIN_GPIO_0 = 0x00,
    PIN_GPIO_1 = 0x01,
    PIN_GPIO_2 = 0x02,
    PIN_GPIO_3 = 0x03,
    /* 0x04~0x07 rsvd */
    PIN_DISP = 0x08,
    PIN_DE = 0x09,
    PIN_VSYNC_HSYNC = 0x0A,
    PIN_PCLK = 0x0B,
    PIN_BACKLIGHT = 0x0C,
    PIN_RGB = 0x0D,
    PIN_AUDIO_L = 0x0E,
    PIN_INT_N = 0x0F,
    PIN_CTP_RST_N = 0x10,
    PIN_CTP_SCL = 0x11,
    PIN_CTP_SDA = 0x12,
    PIN_SPI_IO1234 = 0x13,
} FT81X_PIN_GROUP;

typedef enum
{
    DRIVE_STRENGTH_5MA = 0,
    DRIVE_STRENGTH_10MA = 1,
    DRIVE_STRENGTH_15MA = 2,
    DRIVE_STRENGTH_20MA = 3,
} FT81X_PIN_DRIVE_STRENGTH;

typedef enum
{
    DPIO_DIR_INPUT = 0,
    DPIO_DIR_OUTPUT = 1,
} FT81X_GPIO_DIR_STATUS;

typedef enum
{
    DPIO_DIR_GPIO0 = BIT0,
    DPIO_DIR_GPIO1 = BIT1,
    DPIO_DIR_DISP = BIT7,
} FT81X_GPIO_DIR;

typedef enum
{
    PIN_PD_STATE_FLOAT = 0,
    PIN_PD_STATE_PULL_DOWN = 1,
    PIN_PD_STATE_PULL_UP = 2,
    PIN_PD_STATE_RSVD = 3,
} FT81X_PIN_PD_STATE;

typedef enum
{
    DLSWAP_AFTER_LINE = 1,
    DLSWAP_AFTER_FRAME = 2,
} FT81X_DLSWAP_CTRL;

typedef enum
{
    OFF = 0,
    ON = 1,
} FT81X_STATE;

#define FT81X_CMDTYPE_MEMREAD 0
#define FT81X_CMDTYPE_HOSTCMD 0x40000000
#define FT81X_CMDTYPE_MEMWRITE 0x80000000

typedef struct {
    FT81X_SPIWIDTH qspi_width_op;
    FT81X_PWR_STATE power_state;                  
    uint32_t bitmap_size_x;
    uint32_t bitmap_size_y;
    uint32_t bitmap_layout_x;
    uint32_t bitmap_layout_y;
    uint32_t orientation_x;
    uint32_t orientation_y;
    uint32_t background_color;                  
} qspi_setting_t;

kdev_status_t kdev_qspi_ft81x_initialize(qspi_setting_t *qspi_setting);
kdev_status_t kdev_qspi_ft81x_uninitialize(qspi_setting_t *qspi_setting);
kdev_status_t kdev_qspi_set_dl_configure(qspi_setting_t* qspi_setting);
kdev_status_t kdev_qspi_ft81x_transfer(kdrv_gdma_handle_t dma_handle, uint32_t *buf, uint32_t size, uint32_t addr, FT81X_SPIWIDTH width);
