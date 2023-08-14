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
#include "kdev_qspi_FT81x.h"
#include "kdrv_qspi.h"

#define QSPI_DBG
#ifdef QSPI_DBG
#include "kmdw_console.h"
#define qspi_msg(fmt, ...) info_msg("[KDEV_QSPI] " fmt, ##__VA_ARGS__)
#else
#define qspi_msg(fmt, ...)
#endif

#define BS_filter_NEAREST   0
#define BS_filter_BILINEAR  1
#define BS_wrap_BORDER      0
#define BS_wrap_REPEAT      1
#define min_t(x,y) ( x < y ? x: y )

static kdev_status_t kdev_qspi_ft81x_hostmemwrite_dma(kdrv_gdma_handle_t dma_handle, uint32_t *buf, uint32_t size, uint32_t addr, FT81X_SPIWIDTH width)
{
    uint32_t SPI020_CMD0 = addr & 0x0000FFFF;
    uint32_t SPI020_CMD1 = 0x01000002;
    uint32_t SPI020_CMD2 = size;
    uint32_t SPI020_CMD3 = ((addr & 0x00FF0000) | 0x800000)<<8;
    uint32_t width_bf;
    
    switch(width){

        case FT81X_SPIWIDTH_SINGLE:
            width_bf = 0x00000000;
            break;
        case FT81X_SPIWIDTH_DUAL:
            width_bf = 0x000000A0;
            break;
        case FT81X_SPIWIDTH_QUAD:
            width_bf = 0x000000C0;
        default:
            break;
    }

    SPI020_CMD3 |= (width_bf | 0x2);

    kdrv_qspi_set_commands(SPI020_CMD0, SPI020_CMD1, SPI020_CMD2, SPI020_CMD3);
    /* read data */
	kdrv_qspi_dma_write_data(dma_handle, buf, size);
		
    /* wait for command complete */
    kdrv_qspi_wait_command_complete();

    return KDEV_STATUS_OK;	
}

kdev_status_t kdev_qspi_ft81x_hostmemwrite(uint32_t addr, uint32_t *buf, uint32_t size, FT81X_SPIWIDTH width)
{
    uint32_t SPI020_CMD0 = addr & 0x0000FFFF;
    uint32_t SPI020_CMD1 = 0x01000002;
    uint32_t SPI020_CMD2 = size;
    uint32_t SPI020_CMD3 = (((addr & 0x003F0000) << 8) | FT81X_CMDTYPE_MEMWRITE );
    uint32_t width_bf;
    
    switch(width){

        case FT81X_SPIWIDTH_SINGLE:
            width_bf = 0x00000000;
            break;
        case FT81X_SPIWIDTH_DUAL:
            width_bf = 0x000000A0;
            break;
        case FT81X_SPIWIDTH_QUAD:
            width_bf = 0x000000C0;
        default:
            break;
    }

    SPI020_CMD3 |= (width_bf | 0x2);

    kdrv_qspi_set_commands(SPI020_CMD0, SPI020_CMD1, SPI020_CMD2, SPI020_CMD3);
    /* read data */
    kdrv_qspi_write_data((uint8_t *)buf, size);
    /* wait for command complete */
    kdrv_qspi_wait_command_complete();

    return KDEV_STATUS_OK;	
}

#define dummy_read
static kdev_status_t kdev_qspi_ft81x_hostmemread(uint32_t addr, uint32_t *data, FT81X_SPIWIDTH width)
{
    uint32_t SPI020_CMD0 = addr & 0x0000FFFF;
    uint32_t SPI020_CMD1 = 0x01000002;
    uint32_t SPI020_CMD2 = 0x00000004; //data counts //1 dw
    uint32_t SPI020_CMD3 = (((addr & 0x003F0000)<<8) | FT81X_CMDTYPE_MEMREAD);
    uint32_t width_bf;
#ifdef dummy_read
    uint32_t dummy_bf;
#endif    
    switch(width){

        case FT81X_SPIWIDTH_SINGLE:
        #ifdef dummy_read
            dummy_bf = 0x00080000;
        #endif
            width_bf = 0x00000000;
            break;
        case FT81X_SPIWIDTH_DUAL:
        #ifdef dummy_read
            dummy_bf = 0x00040000;
        #endif
            width_bf = 0x000000A0;
            break;
        case FT81X_SPIWIDTH_QUAD:
        #ifdef dummy_read
            dummy_bf = 0x00020000;
        #endif
            width_bf = 0x000000C0;
        default:
            break;
    }

#ifdef dummy_read
    SPI020_CMD1 |= dummy_bf;
#endif
    SPI020_CMD3 |= width_bf;

    kdrv_qspi_set_commands(SPI020_CMD0, SPI020_CMD1, SPI020_CMD2, SPI020_CMD3);
    /* read data */
    kdrv_qspi_read_data(data, 0x4);
#ifndef dummy_read
    *data >>= 8;
#endif
    /* wait for command complete */
    kdrv_qspi_wait_command_complete();

    return KDEV_STATUS_OK;	
}

kdev_status_t kdev_qspi_ft81x_ramread(uint32_t addr, uint32_t *data, uint32_t size, FT81X_SPIWIDTH width)
{
    uint32_t SPI020_CMD0 = addr & 0x0000FFFF;
    uint32_t SPI020_CMD1 = 0x01000002;
    uint32_t SPI020_CMD2 = size; //data counts //1 dw
    uint32_t SPI020_CMD3 = (((addr & 0x003F0000)<<8) | FT81X_CMDTYPE_MEMREAD);
    uint32_t width_bf;
#ifdef dummy_read
    uint32_t dummy_bf;
#endif    
    switch(width){

        case FT81X_SPIWIDTH_SINGLE:
        #ifdef dummy_read
            dummy_bf = 0x00080000;
        #endif
            width_bf = 0x00000000;
            break;
        case FT81X_SPIWIDTH_DUAL:
        #ifdef dummy_read
            dummy_bf = 0x00040000;
        #endif
            width_bf = 0x000000A0;
            break;
        case FT81X_SPIWIDTH_QUAD:
        #ifdef dummy_read
            dummy_bf = 0x00020000;
        #endif
            width_bf = 0x000000C0;
        default:
            break;
    }

#ifdef dummy_read
    SPI020_CMD1 |= dummy_bf;
#endif
    SPI020_CMD3 |= width_bf;

    kdrv_qspi_set_commands(SPI020_CMD0, SPI020_CMD1, SPI020_CMD2, SPI020_CMD3);
    /* read data */
    kdrv_qspi_read_data(data, size);
#ifndef dummy_read
    *data >>= 8;
#endif
    /* wait for command complete */
    kdrv_qspi_wait_command_complete();

    return KDEV_STATUS_OK;	
}

static kdev_status_t kdev_qspi_ft81x_hostcmd(uint8_t cmd, uint8_t data, FT81X_SPIWIDTH width)
{
    uint32_t SPI020_CMD0 = data<<8;
    uint32_t SPI020_CMD1 = 0x01000002;
    uint32_t SPI020_CMD2 = 0x00000000;
    uint32_t SPI020_CMD3 = cmd<<24;

    uint32_t width_bf;
    
    switch(width){

        case FT81X_SPIWIDTH_SINGLE:
            width_bf = 0x00000000;
            break;
        case FT81X_SPIWIDTH_DUAL:
            width_bf = 0x000000A0;
            break;
        case FT81X_SPIWIDTH_QUAD:
            width_bf = 0x000000C0;
        default:
            break;
    }
		
    SPI020_CMD3 |= (width_bf | 0x2);

    kdrv_qspi_set_commands(SPI020_CMD0, SPI020_CMD1, SPI020_CMD2, SPI020_CMD3);
    /* wait for command complete */
    kdrv_qspi_wait_command_complete();

    return KDEV_STATUS_OK;
}

kdev_status_t kdev_qspi_ft81x_regwrite(uint32_t reg, uint32_t data, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;	
    kdev_qspi_ft81x_hostmemwrite(reg, &data, sizeof(data), width);
    return ret;
}

kdev_status_t kdev_qspi_ft81x_transfer(kdrv_gdma_handle_t dma_handle, uint32_t *buf, uint32_t size, uint32_t addr, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;	
    uint32_t access_byte, loop, index = 0;
    
    loop = size / FT81X_MAX_WRITE_SIZE;
    if(size % FT81X_MAX_WRITE_SIZE)
        loop += 1;
    for(int i=0;i<loop;i++)
    {
        access_byte = min_t(size, FT81X_MAX_WRITE_SIZE);
        size -= access_byte;
        kdev_qspi_ft81x_hostmemwrite_dma(dma_handle, buf, access_byte, index, width);
        buf += (access_byte >> 2);
        index += access_byte;
    }
    return ret;
}

kdev_status_t kdev_qspi_ft81x_regread(uint32_t reg, uint32_t* data, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;	
    kdev_qspi_ft81x_hostmemread(reg, data, width);
    return ret;
}

/*
- 0C0000h: 08h
- 0C0001h: 10h (FT810), 11h(FT811), 12h(FT812), 13h(FT813)
- 0C0002h: 01h
- 0C0003h: 00h
*/
uint32_t kdev_qspi_ft81x_get_chipid(FT81X_SPIWIDTH width)
{
    uint32_t data;
    kdev_qspi_ft81x_hostmemread(FT81X_CHIP_ID, &data, width);
    return data;
}

kdev_status_t kdev_qspi_set_active_mode(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;	
    kdev_qspi_ft81x_hostcmd(FT81X_CMD_ACTIVE, 0, width);
    return ret;
}

kdev_status_t kdev_qspi_set_standby_mode(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;	
    kdev_qspi_ft81x_hostcmd(FT81X_CMD_STANDBY, 0, width);
    return ret;
}

kdev_status_t kdev_qspi_set_sleep_mode(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;	
    kdev_qspi_ft81x_hostcmd(FT81X_CMD_SLEEP, 0, width);
    return ret;
}

kdev_status_t kdev_qspi_set_pwrdown_mode(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;	
    kdev_qspi_ft81x_hostcmd(FT81X_CMD_PWRDOWN, 0, width);
    return ret;
}

kdev_status_t kdev_qspi_set_pd_roms(FT81X_ROMS rom, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
	kdev_qspi_ft81x_hostcmd(FT81X_CMD_PDROMS, rom, width);
    return ret;
}

kdev_status_t kdev_qspi_sel_ext_clk(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
	kdev_qspi_ft81x_hostcmd(FT81X_CMD_CLKEXT, 0, width);
    return ret;
}

kdev_status_t kdev_qspi_sel_int_clk(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
	kdev_qspi_ft81x_hostcmd(FT81X_CMD_CLKINT, 0, width);
    return ret;
}

/* This command will only be effective when the PLL is stopped (SLEEP mode). */
kdev_status_t kdev_qspi_clk_select(FT81X_CLK_MULTIPLY mul_times, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
	kdev_qspi_ft81x_hostcmd(FT81X_CMD_CLKSEL, mul_times, width);
    return ret;
}

/* This will set the drive strength for various pins. */
kdev_status_t kdev_qspi_sel_pin_drive(FT81X_PIN_GROUP pin, FT81X_PIN_DRIVE_STRENGTH driving, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
	kdev_qspi_ft81x_hostcmd(FT81X_CMD_PINDRIVE, ((pin << 2)|driving), width);
    return ret;
}

/* These settings will only be effective during power down and will not affect normal operations. */
kdev_status_t kdev_qspi_pd_pin_state(FT81X_PIN_GROUP pin, FT81X_PIN_PD_STATE state, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
	kdev_qspi_ft81x_hostcmd(FT81X_CMD_PINPDSTATE, ((pin << 2)|state), width);
    return ret;
}

uint32_t kdev_qspi_get_clock_cycle(FT81X_SPIWIDTH width)
{
    uint32_t data=0;
    kdev_qspi_ft81x_hostmemread(FT81X_REG_CLOCK, &data, width);
    return data;
}

kdev_status_t kdev_qspi_set_bus_width(FT81X_SPIWIDTH set_width, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    kdev_qspi_ft81x_hostmemwrite(FT81X_REG_SPI_WIDTH, (uint32_t *)&set_width, sizeof(uint32_t), width);
    return ret;
}

uint32_t kdev_qspi_get_regID(FT81X_SPIWIDTH width)
{
    uint32_t data;
    kdev_qspi_ft81x_hostmemread(FT81X_REG_ID, &data, width);
    return data;
}

kdev_status_t kdev_qspi_set_op_mode(FT81X_SPIWIDTH org_width, FT81X_SPIWIDTH new_width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    uint32_t data;
    kdev_qspi_set_bus_width(new_width, org_width);//kdev_qspi_ft81x_regwrite(0x302188, 0x2, FT81X_SPIWIDTH_SINGLE);
    while (data != FT81X_ID)
    {
        data = kdev_qspi_get_regID(new_width);
        qspi_msg("reg ID quad = 0x%x\n", data);
    }
    return ret;
}

kdev_status_t kdev_qspi_gpio_DISP(FT81X_STATE mode, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    uint32_t gpio_dir;
    uint32_t gpio_en;
    
    kdev_qspi_ft81x_regread(FT81X_REG_GPIO_DIR, &gpio_dir, width); //GPIO dir
    kdev_qspi_ft81x_regread(FT81X_REG_GPIO, &gpio_en, width); //GPIO
    
    kdev_qspi_ft81x_regwrite(FT81X_REG_GPIO_DIR, (gpio_dir | DPIO_DIR_DISP), width); //GPIO dir
    if(mode == ON)
        kdev_qspi_ft81x_regwrite(FT81X_REG_GPIO, (gpio_en | DPIO_DIR_DISP), width); //GPIO
    else //OFF
        kdev_qspi_ft81x_regwrite(FT81X_REG_GPIO, (gpio_en & ~DPIO_DIR_DISP), width); //GPIO
        
    return ret;
}

kdev_status_t kdev_qspi_display_on(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    kdev_qspi_gpio_DISP(ON, width);
    return ret;
}

kdev_status_t kdev_qspi_display_off(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    kdev_qspi_gpio_DISP(OFF, width);
    return ret;
}

kdev_status_t kdev_qspi_dlswap_ctrl(FT81X_DLSWAP_CTRL ctrl, FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    kdev_qspi_ft81x_regwrite(FT81X_REG_DLSWAP, ctrl, width);  //REG_DLSWAP
    return ret;
}

uint32_t kdev_qspi_set_clear_rgb(uint32_t c_red, uint32_t c_green, uint32_t c_blue)
{
    uint32_t cmd = (FT81X_DL_CLEAR_COLOR_RGB<<24 | c_red <<16 | c_green<<8 | c_blue);
    return cmd;
}

uint32_t kdev_qspi_set_clear(uint32_t c_buf, uint32_t s_buf, uint32_t t_buf)
{
    uint32_t cmd = (FT81X_DL_CLEAR<<24 | c_buf <<2 | s_buf<<1 | t_buf); //clear Color/Stencil/Tag buffer
    return cmd;
}

uint32_t kdev_qspi_set_bitmap_handle(uint32_t handle)
{
    uint32_t cmd = (FT81X_DL_BITMAP_HANDLE<<24 | handle);
    return cmd;
}

uint32_t kdev_qspi_set_bitmap_layout(uint32_t format, uint32_t linestride, uint32_t height)
{
    uint32_t cmd = (FT81X_DL_BITMAP_LAYOUT<<24 | format<<19 | linestride<<9 | height);
    return cmd;
}

uint32_t kdev_qspi_set_bitmap_layout_h(uint32_t linestride, uint32_t height)
{
    uint32_t cmd = (FT81X_DL_BITMAP_LAYOUT_H<<24 | linestride<<2 | height);
    return cmd;
}

uint32_t kdev_qspi_set_bitmap_transform_a(uint32_t panel_res, uint32_t gram_width)
{
    uint32_t cmd = FT81X_DL_TRANSFORM_A<<24;
    if(panel_res > gram_width)
    {
        uint32_t diff = panel_res - gram_width;
        diff = (diff * 256) / gram_width;
        diff = 256 - diff;
        cmd += diff;
    }
    else //panel_res <= gram_width
    {
        uint32_t diff = gram_width - panel_res;
        diff = (diff * 256) / gram_width;
        diff += 256;
        cmd += diff;
    }
    return cmd;
}

uint32_t kdev_qspi_set_bitmap_size(uint32_t filter_mode, uint32_t wrapx, uint32_t wrapy, uint32_t width, uint32_t height)
{
    uint32_t cmd = (FT81X_DL_BITMAP_SIZE<<24 | filter_mode<<20 | wrapx<<19 | wrapy<<18 | width<<9 | height);
    return cmd;
}

uint32_t kdev_qspi_set_bitmap_size_h(uint32_t width, uint32_t height)
{
    uint32_t cmd = (FT81X_DL_BITMAP_SIZE_H<<24 | width<<2 | height);
    return cmd;
}

uint32_t kdev_qspi_set_vertex2ii(uint32_t x, uint32_t y, uint32_t handle, uint32_t cell)
{
    uint32_t cmd = 0;
    cmd = (uint32_t)FT81X_DL_VERTEX2II << 30;
    cmd |= (x<<21 | y<<12 | handle<<7 | cell);
    return cmd;
}

uint32_t kdev_qspi_begin(uint32_t prim)
{
    uint32_t cmd = (FT81X_DL_BEGIN<<24 | prim);
    return cmd;
}

uint32_t kdev_qspi_bitmap_source(uint32_t source)
{
    uint32_t cmd = (FT81X_DL_BITMAP_SOURCE<<24 | source);
    return cmd;
}

uint32_t kdev_qspi_display(void)
{
    uint32_t cmd = FT81X_DL_DISPLAY<<24;
    return cmd;
}

kdev_status_t kdev_qspi_ft81x_reset(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    kdev_qspi_ft81x_hostcmd(FT81X_CMD_RST_PULSE, 0, width);
    return ret;
}

kdev_status_t kdev_qspi_panel_timing_init(FT81X_SPIWIDTH width)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    
    kdev_qspi_ft81x_regwrite(FT81X_REG_HCYCLE,  QSPI_REG_HCYCLE,    width); //HCYCLE
    kdev_qspi_ft81x_regwrite(FT81X_REG_HOFFSET, QSPI_REG_HOFFSET,   width); //HOFFSET
    kdev_qspi_ft81x_regwrite(FT81X_REG_HSYNC0,  QSPI_REG_HSYNC0,    width); //HSYNC0
    kdev_qspi_ft81x_regwrite(FT81X_REG_HSYNC1,  QSPI_REG_HSYNC1,    width); //HSYNC1
    kdev_qspi_ft81x_regwrite(FT81X_REG_VCYCLE,  QSPI_REG_VCYCLE,    width); //VCYCLE
    kdev_qspi_ft81x_regwrite(FT81X_REG_VOFFSET, QSPI_REG_VOFFSET,   width); //VOFFSET
    kdev_qspi_ft81x_regwrite(FT81X_REG_VSYNC0,  QSPI_REG_VSYNC0,    width); //VSYNC0
    kdev_qspi_ft81x_regwrite(FT81X_REG_VSYNC1,  QSPI_REG_VSYNC1,    width); //VSYNC1
    kdev_qspi_ft81x_regwrite(FT81X_REG_SWIZZLE, QSPI_REG_SWIZZLE,   width); //SWIZZLE
    kdev_qspi_ft81x_regwrite(FT81X_REG_PCLK_POL,QSPI_REG_PCLK_POL,  width); //PCLK_POL
    kdev_qspi_ft81x_regwrite(FT81X_REG_CSPREAD, QSPI_REG_CSPREAD,   width); //CSPREAD
    kdev_qspi_ft81x_regwrite(FT81X_REG_DITHER,  QSPI_REG_DITHER,    width); //DITHER
    kdev_qspi_ft81x_regwrite(FT81X_REG_HSIZE,   QSPI_REG_HSIZE,     width); //HSIZE
    kdev_qspi_ft81x_regwrite(FT81X_REG_VSIZE,   QSPI_REG_VSIZE,     width); //VSIZE
    return ret;
}

kdev_status_t kdev_qspi_ft81x_initialize(qspi_setting_t *qspi_setting)
{
    uint32_t data;
    kdev_status_t ret = KDEV_STATUS_OK;

    kdrv_qspi_initialize();

    kdev_qspi_ft81x_reset(FT81X_SPIWIDTH_QUAD);
    kdev_qspi_ft81x_reset(FT81X_SPIWIDTH_SINGLE);
    qspi_setting->qspi_width_op = FT81X_SPIWIDTH_SINGLE;
		
    //send command
    kdev_qspi_sel_ext_clk(qspi_setting->qspi_width_op); //FT81X_CMD_CLKEXT
  	kdev_qspi_set_active_mode(qspi_setting->qspi_width_op); //FT81X_CMD_ACTIVE
	qspi_setting->power_state = FT81X_PWR_ACTIVE;
    
    while(data != FT812_CHIP_ID)
    {
        data = kdev_qspi_ft81x_get_chipid(qspi_setting->qspi_width_op);
        qspi_msg("chip ID new = 0x%x\n", data);
    }
    qspi_msg("chip ID new = 0x%x\n", data);
    qspi_msg("-------qspi_ft81x_selftest reset complete--------\n");

    //do not set to quad mode because normal mode dma is faster than quad mode dma.
    kdev_qspi_set_op_mode(qspi_setting->qspi_width_op,FT81X_SPIWIDTH_QUAD);
    qspi_setting->qspi_width_op = FT81X_SPIWIDTH_QUAD;
    qspi_msg("-------qspi_ft81x_selftest switch to QSPI--------\n");    

    //configure video timing registers except REG_PCLK
    kdev_qspi_panel_timing_init(qspi_setting->qspi_width_op);

    //write first display list
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL,        kdev_qspi_set_clear_rgb(0,0,0), qspi_setting->qspi_width_op); //clear color RGB to black
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x04,   kdev_qspi_set_clear(1,1,1),     qspi_setting->qspi_width_op); //clear Color/Stencil/Tag buffer
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x08,   kdev_qspi_display(),            qspi_setting->qspi_width_op); //display
    //write REG_DLSWAP
    kdev_qspi_dlswap_ctrl(DLSWAP_AFTER_FRAME, qspi_setting->qspi_width_op);
    //enable back light control
    kdev_qspi_display_on(qspi_setting->qspi_width_op);
    //write REG_PCLK
    kdev_qspi_ft81x_regwrite(FT81X_REG_PCLK,    QSPI_REG_PCLK,      qspi_setting->qspi_width_op); //PCLK
    return ret;
}

kdev_status_t kdev_qspi_ft81x_uninitialize(qspi_setting_t *qspi_setting)
{
    kdev_qspi_display_off(qspi_setting->qspi_width_op);
    kdev_qspi_set_standby_mode(qspi_setting->qspi_width_op);
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_qspi_set_dl_configure(qspi_setting_t* qspi_setting)
{
    kdev_status_t ret = KDEV_STATUS_OK;
    uint32_t orient_x = qspi_setting->orientation_x;
    uint32_t orient_y = qspi_setting->orientation_y;
    uint32_t layout_x = qspi_setting->bitmap_layout_x;
    uint32_t layout_y = qspi_setting->bitmap_layout_y;
    uint32_t size_x = qspi_setting->bitmap_size_x;
    uint32_t size_y = qspi_setting->bitmap_size_y;
    uint32_t red = (qspi_setting->background_color & 0xFF0000) >> 16;
    uint32_t green = (qspi_setting->background_color & 0x00FF00) >> 8;
    uint32_t blue = qspi_setting->background_color & 0x0000FF;

    #if !defined(_BOARD_SN530HAPS_H_)
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL,        kdev_qspi_set_clear_rgb(red,green,blue),                qspi_setting->qspi_width_op); //clear color RGB to black
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x04,   kdev_qspi_set_clear(1,1,1),                             qspi_setting->qspi_width_op); //clear Color/Stencil/Tag buffer
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x08,   kdev_qspi_set_bitmap_handle(0),                         qspi_setting->qspi_width_op); //dl( BITMAP_HANDLE )
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x0C,   kdev_qspi_set_bitmap_layout(QSPI_PACKET_TYPE, layout_x & 0x3FF, layout_y & 0x1FF), qspi_setting->qspi_width_op); //dl( BITMAP_LAYOUT ) //0x073c0100//FT81X_RGB565, 512*256
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x10,   kdev_qspi_set_bitmap_layout_h(layout_x >> 10, layout_y >> 9), qspi_setting->qspi_width_op); //dl( BITMAP_LAYOUT ) //0x073c0100//FT81X_RGB565, 512*256
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x14,   kdev_qspi_set_bitmap_transform_a(QSPI_REG_HSIZE,size_x),             qspi_setting->qspi_width_op); //display//0x00000000
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x18,   kdev_qspi_set_bitmap_size(BS_filter_BILINEAR, BS_wrap_BORDER, BS_wrap_BORDER, size_x & 0x1FF, size_y & 0x1FF), qspi_setting->qspi_width_op); //dl( BITMAP_SIZE ) //0x08040100
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x1C,   kdev_qspi_set_bitmap_size_h(size_x >> 9, size_y >> 9),  qspi_setting->qspi_width_op); //dl( BITMAP_SIZE ) //0x08040100
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x20,   kdev_qspi_bitmap_source(0),                             qspi_setting->qspi_width_op); //dl( BITMAP_SOURCE(0) )//0x01000000
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x24,   kdev_qspi_begin(PRIM_BITMAPS),                          qspi_setting->qspi_width_op); //dl( BEGIN(BITMAPS) )/0x1F000001
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x28,   kdev_qspi_set_vertex2ii(orient_x,orient_y,0,0),         qspi_setting->qspi_width_op); //dl( VERTEX2II )//0xA0040000
    kdev_qspi_ft81x_regwrite(MEM_RAM_DL+0x2C,   kdev_qspi_display(),                                    qspi_setting->qspi_width_op); //display//0x00000000
    kdev_qspi_dlswap_ctrl(DLSWAP_AFTER_FRAME, qspi_setting->qspi_width_op);//kdev_qspi_ft81x_regwrite(0x302054,   1, qspi_setting->qspi_width_op);  //REG_DLSWAP
    kdev_qspi_display_on(qspi_setting->qspi_width_op);
    #endif
    return ret;
}

