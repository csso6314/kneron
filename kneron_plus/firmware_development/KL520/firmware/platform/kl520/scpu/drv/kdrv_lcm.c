/*
 * KDP LCM Display driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */


#include <string.h>
#include "board.h"
#include "kdrv_gdma.h"
#include "kdrv_scu_ext.h"
#include "kdrv_lcm.h"
#include "kdrv_pwm.h"
#include "kdrv_clock.h"
#include "kmdw_memory.h"
#include "kmdw_display.h"
//#include "kmdw_console.h"
#include <framework/mutex.h>
#include "kl520_include.h"


//#define LCM_DBG
#ifdef LCM_DBG
#define lcm_msg(fmt, ...) info_msg("[KDRV_LCM] " fmt, ##__VA_ARGS__)
#else
#define lcm_msg(fmt, ...)
#endif

/*=============================== LCM Register Releated Macros ===============================*/
#define LCM_BASE                                            SLCD_FTLCDC210_PA_BASE

#define LCM_REG_TIMING                                      (LCM_BASE + 0x200)
#define LCM_REG_RDY                                         (LCM_BASE + 0x204)
#define LCM_REG_RS                                          (LCM_BASE + 0x208)
#define LCM_REG_DATA                                        (LCM_BASE + 0x20C)
#define LCM_REG_CMD                                         (LCM_BASE + 0x210)
#define LCM_REG_OP_MODE                                     (LCM_BASE + 0x214)
#define LCM_REG_ENABLE                                      (LCM_BASE + 0x228)

#define LCM_REG_TIMING_P(base)                              (base + 0x200)
#define LCM_REG_RDY_P(base)                                 (base + 0x204)
#define LCM_REG_RS_P(base)                                  (base + 0x208)
#define LCM_REG_DATA_P(base)                                (base + 0x20C)
#define LCM_REG_CMD_P(base)                                 (base + 0x210)
#define LCM_REG_OP_MODE_P(base)                             (base + 0x214)
#define LCM_REG_ENABLE_P(base)                              (base + 0x228)

/* LCM Timing Control Register, LCM_TIMING (Offset 0x200) */
#define LCM_REG_TPWH_WIDTH_4_READ_CYCLE()                   GET_BITS(LCM_REG_TIMING, 16, 19)
#define LCM_REG_TAS_WIDTH()                                 GET_BITS(LCM_REG_TIMING, 12, 15)
#define LCM_REG_TAH_WIDTH()                                 GET_BITS(LCM_REG_TIMING, 8, 11)
#define LCM_REG_TPWL_WIDTH()                                GET_BITS(LCM_REG_TIMING, 4, 7)
#define LCM_REG_TPWH_WIDTH_4_WRITE_CYCLE()                  GET_BITS(LCM_REG_TIMING, 0, 3)

/* LCM Ready Register, LCM_RDY (Offset 0x204) */
#define LCM_REG_LCM_RDY()                                   GET_BIT(LCM_REG_RDY, 0)

/* LCM Data Window, LCM_DATA (Offset 0x20C) */
#define LCM_REG_LCM_DATA()                                  GET_BITS(LCM_REG_DATA, 0, 31)

/* LCM Command Register, LCM_CMD (Offset 0x210) */
#define LCM_REG_LCM_CMD()                                   GET_BITS(LCM_REG_CMD, 0, 31)

/* LCM Operation Mode Select Register, LCM_OP_MODE (Offset 0x214) */
#define LCM_REG_LCM_OP_MODE_16_BIT_MODE_SELECTION()         GET_BITS(LCM_REG_OP_MODE, 6, 7)
#define LCM_REG_LCM_OP_MODE_BIT_SELECTION_BUS_INTERFACE()   GET_BITS(LCM_REG_OP_MODE, 4, 5)
#define LCM_REG_LCM_OP_MODE_BIT_SELECTION_PANEL_INTERFACE() GET_BITS(LCM_REG_OP_MODE, 2, 3)
#define LCM_REG_LCM_OP_MODE_MCU_SELECTION()                 GET_BIT(LCM_REG_OP_MODE, 0)

/* LCM Enable Register, LCM_ENABLE (Offset 0x228)*/
#define LCM_REG_LCM_ENABLE_BACKLIGHT_CTRL()                 GET_BIT(LCM_REG_ENABLE, 12)
#define LCM_REG_LCM_ENABLE_SIGNAL_INVERSE()                 GET_BIT(LCM_REG_ENABLE, 11)
#define LCM_REG_LCM_ENABLE_RESET_ASSERT()                   GET_BIT(LCM_REG_ENABLE, 8)
#define LCM_REG_LCM_ENABLE()                                GET_BIT(LCM_REG_ENABLE, 0)

/* LCM Timing Control Register (Offset : 0x200) */
/* Tpwh width for the read cycles */
#define LCM_REG_TIMING_SET_Tpwh_r(val)                      SET_MASKED_BITS(LCM_REG_TIMING, val, 16, 19)
/* Tas width */
#define LCM_REG_TIMING_SET_Tas(val)                         SET_MASKED_BITS(LCM_REG_TIMING, val, 12, 15)
/* Tah width */
#define LCM_REG_TIMING_SET_Tah(val)                         SET_MASKED_BITS(LCM_REG_TIMING, val, 8, 11)
/* Tpwd width */
#define LCM_REG_TIMING_SET_Tpwl(val)                        SET_MASKED_BITS(LCM_REG_TIMING, val, 4, 7)
/* Tpwh width for write cycles */
#define LCM_REG_TIMING_SET_Tpwh_w(val)                      SET_MASKED_BITS(LCM_REG_TIMING, val, 0, 3)

/* LCM Ready Register (Offset : 0x204) */
#define LCM_REG_RDY_READY_FOR_ACCESS                        BIT0
#define LCM_REG_RDY_GET_Rdy_C_D                             GET_BIT(LCM_REG_RDY, 0)

/* LCM Data/Command Read Control Register, LCM_RS (Offset 0x208) */
/* write 1: issue the read data phase at RS=1 */
#define LCM_REG_RS_DMYRD_RS                                 BIT0
#define LCM_REG_RS_GET_DMYRD_RS()                           GET_BIT(LCM_REG_RS, 0)
#define LCM_REG_RS_SET_DMYRD_RS(val)                        SET_MASKED_BIT(LCM_REG_RS, val, 0)

/* LCM Operation Mode Select Register, LCM_OP_MODE (Offset 0x214) */
#define LCM_REG_OP_MODE_SET_16bit_mode(val)                 SET_MASKED_BITS(LCM_REG_TIMING, val, 6, 7)
#define LCM_REG_OP_MODE_SET_Bus_IF(val)                     SET_MASKED_BITS(LCM_REG_TIMING, val, 4, 5)
#define LCM_REG_OP_MODE_SET_Panel_IF(val)                   SET_MASKED_BITS(LCM_REG_TIMING, val, 2, 3)
#define LCM_REG_OP_MODE_SET_C68(val)                        SET_MASKED_BITS(LCM_REG_TIMING, val, 0, 1)

/* LCM Enable Register, LCM_ENABLE (Offset 0x228) */
#define LCM_REG_ENABLE_GET_BLCTRL_SET()                     GET_BIT(LCM_REG_ENABLE, 12)
#define LCM_REG_ENABLE_GET_RST_INV()                        GET_BIT(LCM_REG_ENABLE, 11)
#define LCM_REG_ENABLE_GET_RST_CLR()                        GET_BIT(LCM_REG_ENABLE, 9)
#define LCM_REG_ENABLE_GET_RST_SET()                        GET_BIT(LCM_REG_ENABLE, 8)
#define LCM_REG_ENABLE_GET_LCM_En()                         GET_BIT(LCM_REG_ENABLE, 0)

#define LCM_REG_ENABLE_SET_BLCTRL_CLR(val)                  SET_MASKED_BIT(LCM_REG_ENABLE, val, 13)
#define LCM_REG_ENABLE_SET_BLCTRL_SET(val)                  SET_MASKED_BIT(LCM_REG_ENABLE, val, 12)
#define LCM_REG_ENABLE_SET_RST_INV(val)                     SET_MASKED_BIT(LCM_REG_ENABLE, val, 11)
#define LCM_REG_ENABLE_SET_RST_CLR()                        SET_BIT(LCM_REG_ENABLE, 9)
//#define LCM_REG_ENABLE_SET_RST_CLR(val)                     SET_MASKED_BIT(LCM_REG_ENABLE, val, 9)
//#define LCM_REG_ENABLE_SET_RST_SET()                        SET_BIT(LCM_REG_ENABLE, val, 8)
#define LCM_REG_ENABLE_SET_RST_SET(val)                     //SET_MASKED_BIT(LCM_REG_ENABLE, val, 8)
#define LCM_REG_ENABLE_SET_LCM_En(val)                      SET_MASKED_BIT(LCM_REG_ENABLE, val, 0)



//static u32 _prof_start, _prof_end;
//static u32 _prof_offset, _prof_total = 0, _prof_frame_cnt = 0;
#define LCM_PROFILE_ENABLE
#ifdef LCM_PROFILE_ENABLE 
#define LCM_PROFILE_START() _prof_start = osKernelGetTickCount(); 
#define LCM_PROFILE_STOP() { \
                _prof_end = osKernelGetTickCount(); \
                _prof_offset = _prof_end - _prof_start; \
                _prof_total += _prof_offset; \
                ++_prof_frame_cnt; \
                /*DSG("[%s] offset=%u average=%u", __func__, _prof_offset, _prof_total / _prof_frame_cnt);*/ \
                }
#else
#define LCM_PROFILE_START() 
#define LCM_PROFILE_STOP() 
#endif


#define LCM_OPS_WAIT_TIMEOUT_CNT    (1000)

enum lcm_signal_cycle {
    LCM_SIGNAL_WIDTH_LC_PCLK_X1  = 0x00,
    LCM_SIGNAL_WIDTH_LC_PCLK_X2  = 0x01,
    LCM_SIGNAL_WIDTH_LC_PCLK_X3  = 0x02,
    LCM_SIGNAL_WIDTH_LC_PCLK_X4  = 0x03,
    LCM_SIGNAL_WIDTH_LC_PCLK_X5  = 0x04,
    LCM_SIGNAL_WIDTH_LC_PCLK_X6  = 0x05,
    LCM_SIGNAL_WIDTH_LC_PCLK_X7  = 0x06,
    LCM_SIGNAL_WIDTH_LC_PCLK_X8  = 0x07,
    LCM_SIGNAL_WIDTH_LC_PCLK_X9  = 0x08,
    LCM_SIGNAL_WIDTH_LC_PCLK_X10 = 0x09,
    LCM_SIGNAL_WIDTH_LC_PCLK_X11 = 0x0A,
    LCM_SIGNAL_WIDTH_LC_PCLK_X12 = 0x0B,
    LCM_SIGNAL_WIDTH_LC_PCLK_X13 = 0x0C,
    LCM_SIGNAL_WIDTH_LC_PCLK_X14 = 0x0D,
    LCM_SIGNAL_WIDTH_LC_PCLK_X15 = 0x0E,
    LCM_SIGNAL_WIDTH_LC_PCLK_X16 = 0x0F,
};

enum lcm_16bitmode_selection {
    LCM_16BITMODE_ONESHOT   = 0x00,
    LCM_16BITMODE_16_AT_MSB = 0x01,
    LCM_16BITMODE_16_AT_LSB = 0x01,
};

enum lcm_data_bus_type {
    LCM_DATA_BUS_8_BITS  = 0x00,
    LCM_DATA_BUS_9_BITS  = 0x01,
    LCM_DATA_BUS_16_BITS = 0x02,
    LCM_DATA_BUS_18_BITS = 0x03,
};

enum lcm_panel_interface_type {
    LCM_PANEL_INTEFACE_MONO    = 0x00,
    LCM_PANEL_INTEFACE_16_BITS = 0x00,
    LCM_PANEL_INTEFACE_18_BITS = 0x00,
};

enum lcm_mcu_interface_mode {
    LCM_MCU_INTERFACE_8080 = 0x00,
    LCM_MCU_INTERFACE_6800 = 0x01,
};

typedef struct {
    uint32_t dp_buffer_addr;
} kdrv_lcm_context_t;
kdrv_lcm_context_t kdrv_lcm_ctx;
static kdrv_lcm_context_t *p_lcm_ctx = &kdrv_lcm_ctx;

enum lcm_state {
    LCM_STATE_IDLE = 0,
    LCM_STATE_INITED,
    LCM_STATE_STOPPED,
    LCM_STATE_PROBED,
    LCM_STATE_STARTED,
};

uint8_t _glight = 50;


bool m_show_txt = true;

struct mutex display_reg_lock;

enum lcm_state m_lcm_state;
//static osThreadId_t m_tid_lcm;

typedef struct {
    int         buf_size;
    uint32_t    buf_addr;
    bool        init_done;
} kdrv_lcm_fb_t;
kdrv_lcm_fb_t kdrv_lcm_frame_buffer;
static kdrv_lcm_fb_t *p_lcm_fb = &kdrv_lcm_frame_buffer;

#define PWM6_FREQ_CNT   (2000000)

kdrv_display_t kdrv_display;

/************************************************************************
*                 	        Private API                     
************************************************************************/
static void lcm_wait_ready(uint32_t base)
{
    unsigned int rdata;
    unsigned int timeout_cnt = 0;
    
    while((rdata & LCM_REG_RDY_READY_FOR_ACCESS) != LCM_REG_RDY_READY_FOR_ACCESS)
    { 
        rdata = inw(LCM_REG_RDY_P(base));
        timeout_cnt++;
        if (LCM_OPS_WAIT_TIMEOUT_CNT <= timeout_cnt)
        {
            lcm_msg("_lcm_wait_ready timeout : %d\n", timeout_cnt);
            break;
        }
    }
}

static void lcm_set_16bitmode_selection(enum lcm_16bitmode_selection sel)
{
    LCM_REG_OP_MODE_SET_16bit_mode(sel);
}

static void lcm_set_data_bus_type(enum lcm_data_bus_type type) 
{
    LCM_REG_OP_MODE_SET_Bus_IF(type);
}

static void lcm_set_panel_interface_type(enum lcm_panel_interface_type type) 
{
    LCM_REG_OP_MODE_SET_Bus_IF(type);
}

static void lcm_set_mcu_interface_mode(enum lcm_mcu_interface_mode mode) 
{
    LCM_REG_OP_MODE_SET_C68(mode);
}

static void lcm_set_pwh_r(enum lcm_signal_cycle cycle)
{
    //Tpwh width for the read cycles
    LCM_REG_TIMING_SET_Tpwh_r((u32)cycle);
}

static void lcm_set_as(enum lcm_signal_cycle cycle)
{
    LCM_REG_TIMING_SET_Tas((u32)cycle); 
}

static void lcm_set_ah(enum lcm_signal_cycle cycle)
{
    LCM_REG_TIMING_SET_Tah((u32)cycle); 
}

static void lcm_set_pwl(enum lcm_signal_cycle cycle)
{
    LCM_REG_TIMING_SET_Tpwl((u32)cycle); 
}

static void lcm_set_pwh_w(enum lcm_signal_cycle cycle)
{
    //Tpwh width for the write cycles
    LCM_REG_TIMING_SET_Tpwh_w((u32)cycle); 
}

static void lcm_alloc_framebuffer(void)
{
     p_lcm_ctx->dp_buffer_addr = KDP_DDR_DRV_LCM_START_ADDR;
}

static __inline int _kdp520_lcm_draw_pixel(
        u32 frame_buffer, int x, int y, u32 width, u16 color)
{
    *(u16 *)(frame_buffer + ((y * width + x) << 1) + 0) = color; 
    return 0;
}

/* reserved functon
static int kdp520_lcm_reset(struct display_driver_s *display_drv)
{
    return 0;
}
*/


/************************************************************************
*                 	        Public API                     
************************************************************************/
kdrv_display_t *kdrv_display_initialize(void)
{
    kdrv_display.base = SLCD_FTLCDC210_PA_BASE;

    SCU_EXTREG_SWRST_MASK1_SET_lcm_reset_n(1);
    SCU_EXTREG_MISC_SET_lcm_cken(1);
    
    lcm_set_as(LCM_SIGNAL_WIDTH_LC_PCLK_X2);
    lcm_set_ah(LCM_SIGNAL_WIDTH_LC_PCLK_X2);
    lcm_set_pwl(LCM_SIGNAL_WIDTH_LC_PCLK_X2);
    lcm_set_pwh_r(LCM_SIGNAL_WIDTH_LC_PCLK_X4);
    lcm_set_pwh_w(LCM_SIGNAL_WIDTH_LC_PCLK_X4);

    lcm_set_16bitmode_selection(LCM_16BITMODE_ONESHOT);
    lcm_set_data_bus_type(LCM_DATA_BUS_16_BITS);
    //lcm_set_data_bus_type(LCM_DATA_BUS_8_BITS);
    
    lcm_set_panel_interface_type(LCM_PANEL_INTEFACE_18_BITS);
    lcm_set_mcu_interface_mode(LCM_MCU_INTERFACE_8080);

    // reset lcm
    {
        LCM_REG_ENABLE_SET_RST_SET(); // power on reset
        kdrv_delay_us(5000);
        //delay_ms(50);
        LCM_REG_ENABLE_SET_RST_CLR();
        LCM_REG_ENABLE_SET_LCM_En(1);
        LCM_REG_ENABLE_SET_BLCTRL_SET(1);
    }
    //change to probed
    m_lcm_state = LCM_STATE_INITED;
    return &kdrv_display;
}

kdrv_status_t kdrv_display_set_pen(kdrv_display_t *display_drv, uint16_t color, unsigned int width)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    display_drv->pen_info.color = color;
    display_drv->pen_info.width = width;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_params(kdrv_display_t *display_drv, struct video_input_params *params)
{
    struct video_input_params *p;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    p = &display_drv->vi_params;
    memcpy(p, params, sizeof(*params));
    lcm_msg(" *params=%u i=%u", sizeof(*params), params->input_xres);

    display_drv->fb_size = calc_framesize(p->input_xres, p->input_yres, p->input_fmt);

    lcm_msg("[%s] video_input_params->input_xres=%u", __func__, p->input_xres);
    lcm_msg("[%s] video_input_params->input_yres=%u", __func__, p->input_yres);
    lcm_msg("[%s] video_input_params->input_fmt=%x", __func__, p->input_fmt);  
    lcm_msg("[%s] display_driver->fb_size=%d", __func__, display_drv->fb_size);
    kdrv_display_set_backlight(display_drv, 100);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_get_params(kdrv_display_t *display_drv, struct video_input_params *params)
{
    struct video_input_params *p;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    p = &display_drv->vi_params;
    //memcpy(p, params, sizeof(*params));
    memcpy(params, p, sizeof(*params));
    
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_camera(kdrv_display_t *display_drv, uint8_t cam_idx)
{    
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    display_drv->cam_src_idx = cam_idx;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_buffer_initialize(struct video_input_params *params)
{
    p_lcm_fb->buf_size = calc_framesize(params->input_xres, params->input_yres, params->input_fmt);
    p_lcm_fb->buf_addr = kmdw_ddr_reserve(p_lcm_fb->buf_size);
    p_lcm_fb->init_done = true;
    lcm_alloc_framebuffer();

    return KDRV_STATUS_OK;
}

uint32_t kdrv_display_get_buffer(void)
{
    return p_lcm_fb->buf_addr;
}

kdrv_status_t kdrv_display_start(kdrv_display_t *display_drv)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    kdrv_lcm_write_cmd(display_drv->base, 0x29);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_stop(kdrv_display_t *display_drv)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    kdrv_display_set_backlight(display_drv, 0);
    kdrv_lcm_write_cmd(display_drv->base, 0x28);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_draw_line(kdrv_display_t *display_drv, 
        uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye)
{
    uint32_t x_pos, y_pos;
    uint32_t x_unit = 1;
    uint32_t y_unit = 1;
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t _addr = p_lcm_ctx->dp_buffer_addr;
    uint16_t color;
    uint16_t pen_width;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    frame_width = display_drv->vi_params.input_xres;
    frame_height = display_drv->vi_params.input_yres;
    color = display_drv->pen_info.color;
    pen_width = display_drv->pen_info.width;
    if(xs == xe)
    {
        if(ys > ye)
        {
            ys ^= ye;
            ye ^= ys;
            ys ^= ye;
        }
                    
        for(y_pos = ys; y_pos <= ye; y_pos += y_unit)
        {
            for(x_pos = xs; (x_pos < xs+pen_width) && (x_pos < frame_width); x_pos += x_unit)
            {
                _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
        }
        return KDRV_STATUS_OK;
    }
    else if(ys == ye)
    {
        if(xs > xe)
        {
            xs ^= xe;
            xe ^= xs;
            xs ^= xe;
        }

        for(x_pos = xs; x_pos <= xe; x_pos += x_unit)
        {
            for(y_pos = ys; (y_pos < ys+pen_width) && (y_pos < frame_height); y_pos += y_unit)
            {
                _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
        }
       return KDRV_STATUS_OK;
    }
    return KDRV_STATUS_INVALID_PARAM;
}

kdrv_status_t kdrv_display_draw_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height)
{
    uint16_t color;
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t border_size;
    uint32_t _addr = p_lcm_ctx->dp_buffer_addr;
    int x_pos, y_pos;		 
    int x_unit = 1;
    int y_unit = 1;
    int top = org_y;
    int left = org_x;
    int right;
    int bottom;
    int left_border;
    int right_border;
    int top_lower;
    int bottom_upper;  
    
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    color = display_drv->pen_info.color;
    frame_width = display_drv->vi_params.input_xres;
    frame_height = display_drv->vi_params.input_yres;
    border_size = display_drv->pen_info.width;
    left_border = left + border_size;
    right_border = right - border_size;
    top_lower = top + border_size;
    bottom_upper = bottom - border_size;
    
    for (y_pos=(top>=0)?top:0; (y_pos < bottom) && (y_pos >= 0) && (y_pos < frame_height); y_pos += y_unit)
    {
        if ((y_pos >= top && y_pos < top_lower) || (y_pos >= bottom_upper))
        {
            for (x_pos=(left>=0)?left:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
            {
                //DSG("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
        }
        else
        {
            for (x_pos=(left>=0)?left:0; (x_pos < left_border) && (x_pos < frame_width); x_pos += x_unit)
            {
                //DSG("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
   
            for (x_pos=(right_border>=0)?right_border:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
            {
                //DSG("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_draw_static_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_test_pattern_gen(kdrv_display_t *display_drv, bool pat_gen)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_fill_rect(kdrv_display_t *display_drv, 
        uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height)
{ 
    uint32_t x_pos, y_pos;
    uint32_t _addr = p_lcm_ctx->dp_buffer_addr;
    uint32_t frame_width;
    uint32_t frame_height;
    uint16_t color;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    frame_width = display_drv->vi_params.input_xres;
    frame_height = display_drv->vi_params.input_yres;
    color = display_drv->pen_info.color;
    if((org_x >= frame_width)
    || (org_y >= frame_height)
    || ((org_x + width) > frame_width)
    || ((org_y + height) > frame_height))
    {
        return KDRV_STATUS_INVALID_PARAM;
    }
    
    for(y_pos=org_y; y_pos<(org_y+height); y_pos++)
    {
        for(x_pos=org_x; x_pos<(org_x+width); x_pos++)
        {
            _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_draw_bitmap(kdrv_display_t *display_drv, 
        uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height, void* pBuf)
{
    uint32_t* buf = pBuf;
    uint32_t y_pos;
    uint32_t _addr = p_lcm_ctx->dp_buffer_addr;
    uint32_t frame_width;
    uint32_t frame_height;
    uint8_t bpp = 2;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    frame_width = display_drv->vi_params.input_xres;
    frame_height = display_drv->vi_params.input_yres;
    
    if((org_x >= frame_width)
    || (org_y >= frame_height)
    || ((org_x + width) > frame_width)
    || ((org_y + height) > frame_height))
    {
        return KDRV_STATUS_INVALID_PARAM;
    }
    
    for(y_pos=org_y; y_pos<(org_y+height); y_pos++)
    {
        memcpy((void *)(_addr+((y_pos*frame_width+org_x)<<1)), (void *)buf, width*bpp);
        buf += width*bpp;
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_backlight(kdrv_display_t *display_drv, int light)
{   
    uint32_t  duty;

    if(display_drv == NULL)
       return KDRV_STATUS_ERROR;

    duty = (PWM6_FREQ_CNT * light) / 100;
	kdrv_pwm_config(PWMTIMER6, PWM_POLARITY_INVERSED, duty, PWM6_FREQ_CNT, 0);
    kdrv_pwm_enable(PWMTIMER6);

    if(light == 0)
        return KDRV_STATUS_OK;

    _glight = light;
	return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_display_set_frame_margin_len(uint16_t margin_len)
{
    return KDRV_STATUS_OK;
}

uint16_t kdrv_display_get_frame_margin_len(void)
{
    return 0;;
}

kdrv_status_t kdrv_display_set_review_snapshot_en(bool enable)
{
    return KDRV_STATUS_OK;
}

bool kdrv_display_get_review_snapshot_en(void)
{
    return false;
}

kdrv_status_t kdrv_display_set_snapshot_preview_en(bool enable)
{
    return KDRV_STATUS_OK;
}

bool kdrv_display_get_snapshot_preview_en(void)
{
    return false;
}



kdrv_status_t kdrv_lcm_write_cmd(uint32_t base, unsigned char data)
{
    mutex_lock(&display_reg_lock);

    lcm_wait_ready(base);
    outb(LCM_REG_CMD_P(base), data);

    mutex_unlock(&display_reg_lock);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcm_write_data(uint32_t base, unsigned char data)
{
    mutex_lock(&display_reg_lock);

    lcm_wait_ready(base);    
    outb(LCM_REG_DATA_P(base), data);

    mutex_unlock(&display_reg_lock);
    return KDRV_STATUS_OK;
}

unsigned int kdrv_lcm_read_data(uint32_t base)
{
    mutex_lock(&display_reg_lock);

    lcm_wait_ready(base);
    outb(LCM_REG_RS_P(base), LCM_REG_RS_DMYRD_RS);

    lcm_wait_ready(base);

    mutex_unlock(&display_reg_lock);

    return inw(LCM_REG_DATA_P(base));
}

kdrv_status_t kdrv_lcm_pressingnir(kdrv_display_t *display_drv, u32 addr)
{ 
    uint32_t row_idx = 0;
    uint32_t col_idx = 0;
    uint32_t index = 0;
    uint32_t readAddr;
    uint32_t writeAddr = KDP_DDR_DRV_LCM_START_ADDR;
    uint32_t totalpoint;
    uint16_t data_buf;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    addr = addr + 100 + 480*NIR_Y_OFFSET;
    readAddr = addr;

    totalpoint = (320*240);
    
    for(index = 0; index < totalpoint; index++)
    {
        data_buf = (u16)inb(readAddr);
        data_buf = (u16)((data_buf>>3)<<11) | ((data_buf>>2)<<5) | ((data_buf>>3)<<0);
 
        outw(writeAddr,data_buf);
        writeAddr = writeAddr + 2;
        
        if (0 == index%240 )
        {
            col_idx = 0;
            row_idx = row_idx + 1;
            readAddr = addr + 480*row_idx;
        }
        else
        {
            col_idx = col_idx + 1;
            readAddr = readAddr + 1;
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_lcm_pressing(kdrv_display_t *display_drv, u32 addr)
{      
    const uint16_t input_xres = display_drv->vi_params.input_xres;
    const uint16_t input_yres = display_drv->vi_params.input_yres;

    uint32_t index = 0;
    uint32_t readAddr = addr;
    uint32_t writeAddr = KDP_DDR_DRV_LCM_START_ADDR;
    uint16_t lcm_w = input_xres;
    uint16_t lcm_h = input_yres;

    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;
    
    readAddr += LCD_DISPLAY_X_OFFSET * 2 + LCD_DISPLAY_Y_OFFSET * 640 *2;
    for(index =0;index < lcm_h;index++)
    {
        kdrv_gdma_memcpy((uint32_t)(writeAddr), (uint32_t)(readAddr), lcm_w*2);
        readAddr += 640*2;
        writeAddr += lcm_w*2;
    }
    return KDRV_STATUS_OK;
}

extern osMutexId_t mutex_snapshot;
kdrv_status_t kdrv_display_update_draw_fb(kdrv_display_t *display_drv, uint32_t addr, uint8_t cam_idx)
{
    if(display_drv == NULL)
        return KDRV_STATUS_ERROR;

    display_drv->cam_src_idx = cam_idx;
    
     if(cam_idx == MIPI_CAM_RGB){
#if CFG_SNAPSHOT_ENABLE == YES
        osMutexAcquire(mutex_snapshot, 1000);
        kdrv_lcm_pressing(display_drv, addr);
        osMutexRelease(mutex_snapshot);        
#else   
        kdrv_lcm_pressing(display_drv, addr);
#endif
    }
    else{
#if CFG_SNAPSHOT_ENABLE == YES
        osMutexAcquire(mutex_snapshot, 1000);  
        kdrv_lcm_pressingnir(display_drv, addr);//pressing to display addr
        osMutexRelease(mutex_snapshot);
#else
        kdrv_lcm_pressingnir(display_drv, addr);//pressing to display addr
#endif        
    }    
    return KDRV_STATUS_OK;
}

uint8_t kdrv_lcm_get_backlight(void)
{
    return _glight;
}

uint32_t kdrv_lcm_get_db_frame(void)
{
    return p_lcm_ctx->dp_buffer_addr;
}
