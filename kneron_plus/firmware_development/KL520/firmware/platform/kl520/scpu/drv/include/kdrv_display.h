/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/

/**@addtogroup  KDRV_DISPLAY  KDRV_DISPLAY
 * @{
 * @brief       Kneron display interface for LCDC and LCM driver 
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_DISPLAY_H__
#define __KDRV_DISPLAY_H__

#include "base.h"
#include "kdrv_status.h"
//#include "framework/v2k_image.h"

//--------------------------------//
//#include "framework/v2k_image.h"
//#include "framework/v2k.h"
//---From v2k_color.h---//
/* rgb 565 */
#define WHITE                       0xFFFF
#define BLACK                       0x0000
#define BLUE                        0x001F
#define BRED                        0xF81F
#define GRED                        0xFFE0
#define GBLUE                       0x07FF
#define RED                         0xF800
#define MAGENTA                     0xF81F
#define GREEN                       0x07E0
#define CYAN                        0x7FFF
#define YELLOW                      0xFFE0
#define BROWN                       0xBC40
#define BRRED                       0xFC07
#define GRAY                        0x8430
//----------------------//
//---From v2k.h---//

#define v2k_fourcc(a, b, c, d) \
    ((unsigned int)(a) | ((unsigned int)(b) << 8) | ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24))

#define V2K_PIX_FMT_YCBCR   v2k_fourcc('Y', 'B', 'Y', 'R')
#define V2K_PIX_FMT_RGB565  v2k_fourcc('R', 'G', 'B', 'P')
#define V2K_PIX_FMT_RAW10   v2k_fourcc('R', 'A', '1', '0')
#define V2K_PIX_FMT_RAW8    v2k_fourcc('R', 'A', 'W', '8')

#define QVGA_LANDSCAPE_WIDTH    320
#define QVGA_LANDSCAPE_HEIGHT   240
#define QVGA_PORTRAIT_WIDTH     240
#define QVGA_PORTRAIT_HEIGHT    320

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
//-----------//
//---From v2k_image.h"---//
struct video_input_params {
    uint16_t input_xres;
    uint16_t input_yres;
    uint32_t input_fmt;
};
//-----------------------//
#define FRAME_SIZE_RGB(xres,yres,mbpp)  ((xres) * (yres) * (mbpp) / 8)

/**
 * @brief       inline function to calculate frame size for display
 *
 * @param[in]   width           Width of image
 * @param[in]   height          Height of image
 * @param[in]   input_fmt       Format of image
 * @return      > 0             Frame size
 *              0               Wrong input image format
 */
inline int kdrv_display_cal_framesize(unsigned short width, unsigned short height, unsigned int input_fmt)
{
    switch(input_fmt)
    {
        case V2K_PIX_FMT_RGB565:
        case V2K_PIX_FMT_YCBCR:
            return FRAME_SIZE_RGB(width, height, 16);

        case V2K_PIX_FMT_RAW8:
            return FRAME_SIZE_RGB(width, height, 8);
        
        default:
            break;
    }
    return 0;
}

/** @brief Enumerations of display pen setting */ 
typedef struct
{
    unsigned int width;                     /**< Width of pen */
    uint16_t color;                         /**< Color of pen */
} kdrv_display_pen_info_t;

/** @brief Enumerations of display driver setting */ 
typedef struct {
    struct video_input_params vi_params;    /**< see @ref video_input_params*/
    uint8_t         cam_src_idx;            /**< Camera source index */
    uint32_t        fb_size;                /**< Frame buffer size */
    uint32_t        fb_addr;                /**< Frame buffer address */
    uint32_t        base;                   /**< Display driver base address */
    uint16_t        display_id;             /**< Display driver id */
    kdrv_display_pen_info_t pen_info;       /**< see @ref kdrv_display_pen_info_t */
} kdrv_display_t;


/**
 * @brief       Initialize display driver
 *
 * @param[in]   N/A
 * @return      kdrv_display_t*     see @ref kdrv_display_t
 */
kdrv_display_t *kdrv_display_initialize(void);

/**
 * @brief       Initialize display frame buffer
 *
 * @param[in]   params              see @ref video_input_params
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_buffer_initialize(struct video_input_params *params);

uint32_t kdrv_display_get_buffer(void);

/**
 * @brief       Set display parameters
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[out]  params              see @ref video_input_params
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_set_params(kdrv_display_t *display_drv, struct video_input_params *params);

/**
 * @brief       Get display parameters
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[out]  params              see @ref video_input_params
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_get_params(kdrv_display_t *display_drv, struct video_input_params *params);

/**
 * @brief       Set camera source which will be preview on display
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   cam_idx             Camera source index
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_set_camera(kdrv_display_t *display_drv, uint8_t cam_idx);

/**
 * @brief       Start display image preview
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_start(kdrv_display_t *display_drv);

/**
 * @brief       Stop display image preview
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_stop(kdrv_display_t *display_drv);

/**
 * @brief       Set pen setting
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   color               Color of pen setting
 * @param[in]   width               Width of pen setting
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_set_pen(kdrv_display_t *display_drv, uint16_t color, uint32_t width);

/**
 * @brief       Update frame buffer which be used to draw something on display
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   addr                Frame buffer address
 * @param[in]   cam_idx             Camera source index
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_update_draw_fb(kdrv_display_t *display_drv, uint32_t addr, uint8_t cam_idx);

/**
 * @brief       Draw rectangle without filling color on display
 * @note        Mainly be used to draw hint boundingbox on display
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   org_x               Start x-axis of rectangle on display to draw
 * @param[in]   org_y               Start y-axis of rectangle on display to draw
 * @param[in]   width               Width of rectangle
 * @param[in]   height              Height of rectangle
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_draw_static_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height);

/**
 * @brief       Draw rectangle without filling color on display
 * @note        Mainly be used to draw fdfr result boundingbox on display
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   org_x               Start x-axis of rectangle on display to draw
 * @param[in]   org_y               Start y-axis of rectangle on display to draw
 * @param[in]   width               Width of rectangle
 * @param[in]   height              Height of rectangle
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_draw_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height);

/**
 * @brief       Draw line on display
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   xs                  Start x-axis of line on display to draw
 * @param[in]   xe                  End x-axis of line on display to draw
 * @param[in]   ys                  Start y-axis of line on display to draw
 * @param[in]   ye                  End y-axis of line on display to draw
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_draw_line(kdrv_display_t *display_drv, uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye);

/**
 * @brief       Draw rectangle with filling color on display
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   org_x               Start x-axis of rectangle on display to draw
 * @param[in]   org_y               Start y-axis of rectangle on display to draw
 * @param[in]   width               Width of rectangle
 * @param[in]   height              Height of rectangle
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_fill_rect(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height);

/**
 * @brief       Draw bitmap on display
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   org_x               Start x-axis of bitmap on display to draw
 * @param[in]   org_y               Start y-axis of bitmap on display to draw
 * @param[in]   width               Width of bitmap
 * @param[in]   height              Height of bitmap
 * @param[in]   pBuf                Bitmap of target
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_draw_bitmap(kdrv_display_t *display_drv, uint32_t org_x, uint32_t org_y, uint32_t width, uint32_t height, void* pBuf);

/**
 * @brief       Set display backlight
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @param[in]   pat_gen             yes or no to generate displayu test pattern
 * @return      kdrv_status_t       see @ref kdrv_status_t
 *
 * @note        Exmpale:\n
 *              kdrv_display_test_pattern_gen(true);\n
 *              kmdw_video_renderer_open(params);\n
 */
kdrv_status_t kdrv_display_test_pattern_gen(kdrv_display_t *display_drv, bool pat_gen);

/**
 * @brief       Set display backlight
 *
 * @param[in]   display_drv         see @ref kdrv_display_t
 * @return      kdrv_status_t       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_set_backlight(kdrv_display_t *display_drv, int light);


kdrv_status_t kdrv_display_set_frame_margin_len(uint16_t margin_len);


uint16_t kdrv_display_get_frame_margin_len(void);

kdrv_status_t kdrv_display_set_review_snapshot_en(bool enable);

bool kdrv_display_get_review_snapshot_en(void);

kdrv_status_t kdrv_display_set_snapshot_preview_en(bool enable);

bool kdrv_display_get_snapshot_preview_en(void);


#endif
/** @}*/
