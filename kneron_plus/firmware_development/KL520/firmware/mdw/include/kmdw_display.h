/**
 * @file		kmdw_display.h
 * @brief       Kneron display middleware
 *
 * @copyright   Copyright (c) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KMDW_DISPLAY_H__
#define __KMDW_DISPLAY_H__

#include "kdrv_display.h"

/* Method of draw boundingbox */
#define DRAW_HINT_BOUNDINGBOX          (0x01)   /**< Only display 4-corner hint boundingbox on LDC preview*/
#define DRAW_FDR_RESULT_BOUNDINGBOX    (0x02)   /**< Only display face boundingbox on LDC preview*/
#define ALL_DRAW_BOUNDINGBOX_MODE      (DRAW_HINT_BOUNDINGBOX | DRAW_FDR_RESULT_BOUNDINGBOX)    /**< Display hint 4-corner and face boundingbox on LDC preview*/

/**
 * @brief Structure of representing display and panel driver compatibility
 */
struct kmdw_display_panel_drv {
    struct video_input_params vi_params;        /**< - */
    uint32_t     fb_size;                       /**< - */
    uint32_t     type;                          /**< - */
    uint32_t     base;                          /**< - */
    uint16_t     display_id;                    /**< - */
};

/**
 * @brief       Initialize display and panel driver
 *
 * @return      N/A
 */
int kmdw_display_initialize(void);

/**
 * @brief       Open a video renderer to display frame buffer
 *
 * @param[in]   params          see @ref video_input_params
 * @return      0,              Open a video renderer to display frame buffer successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              struct video_input_params params;\n
 *              params.input_fmt = V2K_PIX_FMT_RGB565;\n
 *              params.input_xres = LCD_WIDTH;\n
 *              params.input_yres = LCD_HEIGHT;\n
 *              kmdw_video_renderer_open(&params);
 */
int kmdw_video_renderer_open(struct video_input_params *params);

/**
 * @brief       Set camera source index which be displayed on LCD
 *
 * @param[in]   cam_idx         Camera source index
 * @return      0,              Set camera source index which be displayed on LCD successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              struct video_input_params params;\n
 *              params.input_fmt = V2K_PIX_FMT_RGB565;\n
 *              params.input_xres = LCD_WIDTH;\n
 *              params.input_yres = LCD_HEIGHT;\n
 *              kmdw_video_renderer_open(&params);\n
 *              kmdw_video_renderer_set_camera(CAMERA_RGB_IDX);
 */
int kmdw_video_renderer_set_camera(uint8_t cam_idx);

/**
 * @brief       Initilize display frame buffer
 *
 * @param[in]   params          see @ref video_input_params
 * @return      0,              Initilize display frame buffer successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              struct video_input_params params;\n
 *              params.input_fmt = V2K_PIX_FMT_RGB565;\n
 *              params.input_xres = LCD_WIDTH;\n
 *              params.input_yres = LCD_HEIGHT;\n
 *              kmdw_video_renderer_open(&params);\n
 *              kmdw_video_renderer_set_camera(CAMERA_RGB_IDX);\n
 *              kmdw_video_renderer_buffer_initialize(&params);
 */
int kmdw_video_renderer_buffer_initialize(struct video_input_params *params);

/**
 * @brief       Turn on display preview
 *
 * @return      0,              Turn on display preview successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              struct video_input_params params;\n
 *              params.input_fmt = V2K_PIX_FMT_RGB565;\n
 *              params.input_xres = LCD_WIDTH;\n
 *              params.input_yres = LCD_HEIGHT;\n
 *              kmdw_video_renderer_open(&params);\n
 *              kmdw_video_renderer_set_camera(CAMERA_RGB_IDX);\n
 *              kmdw_video_renderer_buffer_initialize(&params);\n
 *              kmdw_video_renderer_start();
 */
int kmdw_video_renderer_start(void);

/**
 * @brief       Turn off display preview
 *
 * @return      0,              Turn off display preview successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 */
int kmdw_video_renderer_stop(void);

/**
 * @brief       Get display snapshot frame buffer
 *
 * @return      >0,             Return snapshot frame buffer successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              uint32_t buf_addr;\n
 *              buf_addr = kmdw_video_renderer_get_buffer_addr();
 */
uint32_t kmdw_video_renderer_get_buffer_addr(void);

/**
 * @brief       Get the input parameters of display
 *
 * @param[in]   dp_params       see @ref video_input_params
 * @return      0,              Update frame buffer successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              uint32_t buf_idx, buf_addr;\n
 *              buf_addr = kdp_fb_mgr_next_read(CAMERA_RGB_IDX, &buf_idx);\n
 *              kmdw_display_update_draw_fb(buf_addr, CAMERA_RGB_IDX)\n
 *              kmdw_display_draw_xxx();
 */
int kmdw_display_get_params(struct video_input_params *dp_params);

/**
 * @brief       Get display panel id
 *
 * @return      [id],          display panel id 
 *
 * @note        Exmpale:\n
 *              int dev_id;\n
 *              dev_id = kmdw_display_get_device_id()\n
 */
int kmdw_display_get_device_id(void);

/**
 * @brief       Update frame buffer which be used to draw something on display
 *
 * @param[in]   addr            Frame buffer address
 * @param[in]   cam_idx         Camera source index
 * @return      0,              Update frame buffer successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              uint32_t buf_idx, buf_addr;\n
 *              buf_addr = kdp_fb_mgr_next_read(CAMERA_RGB_IDX, &buf_idx);\n
 *              kmdw_display_update_draw_fb(buf_addr, CAMERA_RGB_IDX)\n
 *              kmdw_display_draw_xxx();
 */
int kmdw_display_update_draw_fb(uint32_t addr, uint8_t cam_idx);

/**
 * @brief       Set pen width and color
 *
 * @param[in]   color           Color of pen
 * @param[in]   pen_width       Width of pen
 * @return      0,              Set pen width and color successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              kmdw_display_set_pen_rgb565(BLUE, 8);
 */
int kmdw_display_set_pen_rgb565(uint16_t color, uint16_t pen_width);

/**
 * @brief       Draw rectangle without filling color on display
 *
 * @param[in]   x               Start x-axis of rectangle on display to draw
 * @param[in]   y               Start y-axis of rectangle on display to draw
 * @param[in]   width           Width of rectangle
 * @param[in]   height          Height of rectangle
 * @param[in]   draw_mode       see @ref KDP_BOUNDINGBOX_MODE
 * @return      0,              Draw rectangle on display successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              uint32_t buf_idx, buf_addr;\n
 *              buf_addr = kdp_fb_mgr_next_read(CAMERA_RGB_IDX, &buf_idx);\n
 *              kmdw_display_update_draw_fb(buf_addr, CAMERA_RGB_IDX);\n
 *              kmdw_display_set_pen_rgb565(BLUE, 8);\n
 *              kmdw_display_draw_rect(50, 55, 120, 100, DRAW_FDR_RESULT_BOUNDINGBOX);
 */
int kmdw_display_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t draw_mode);

/**
 * @brief       Draw line on display
 *
 * @param[in]   xs              Start x-axis of line on display to draw
 * @param[in]   xe              End x-axis of line on display to draw
 * @param[in]   ys              Start y-axis of line on display to draw
 * @param[in]   ye              End y-axis of line on display to draw
 * @return      0,              Draw line on display successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              uint32_t buf_idx, buf_addr;\n
 *              buf_addr = kdp_fb_mgr_next_read(CAMERA_RGB_IDX, &buf_idx);\n
 *              kmdw_display_update_draw_fb(buf_addr, CAMERA_RGB_IDX);\n
 *              kmdw_display_set_pen_rgb565(BLACK, 4);\n
 *              kmdw_display_draw_line(50, 400, 50, 300);
 */
int kmdw_display_draw_line(uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye);

/**
 * @brief       Draw rectangle with filling color on display
 *
 * @param[in]   x               Start x-axis of rectangle on display to draw
 * @param[in]   y               Start y-axis of rectangle on display to draw
 * @param[in]   width           Width of rectangle
 * @param[in]   height          Height of rectangle
 * @return      0,              Draw rectangle on display successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              uint32_t buf_idx, buf_addr;\n
 *              buf_addr = kdp_fb_mgr_next_read(CAMERA_RGB_IDX, &buf_idx);\n
 *              kmdw_display_update_draw_fb(buf_addr, CAMERA_RGB_IDX);\n
 *              kmdw_display_set_pen_rgb565(BLACK, 8);\n
 *              kmdw_display_fill_rect(0, 0, 640, 480);
 */
int kmdw_display_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
 * @brief       Draw bitmap on display
 *
 * @param[in]   x               Start x-axis of bitmap on display to draw
 * @param[in]   y               Start y-axis of bitmap on display to draw
 * @param[in]   width           Width of bitmap
 * @param[in]   height          Height of bitmap
 * @param[in]   buf             Bitmap of target
 * @return      0,              Draw bitmap on display successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              uint32_t buf_idx, buf_addr;\n
 *              buf_addr = kdp_fb_mgr_next_read(CAMERA_RGB_IDX, &buf_idx);\n
 *              kmdw_display_update_draw_fb(buf_addr, CAMERA_RGB_IDX);\n
 *              kmdw_display_draw_bitmap(0, 450, 100, 50, (void *)USER_IMG_ICON_ADDR);
 */
int kmdw_display_draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void *buf);

/**
 * @brief       Generate display test image display
 *
 * @param[in]   pat_gen         TRUE or FALSE to generate test pattern
 * @return      0,              Draw rectangle on display successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              kmdw_display_test_pattern_gen(TRUE);\n
 *              kmdw_display_initialize();
 *              kmdw_video_renderer_open(&params);
 */
int kmdw_display_test_pattern_gen(bool pat_gen);


int kmdw_display_refresh(void);

/**
 * @brief       Set display backlight
 *
 * @param[in]   duty            see @ref kdrv_display_backlight_t
 * @return      0,              Set display backlight successfully\n
 *              -1,             Empty or wrong lcd driver or other errors
 *
 * @note        Exmpale:\n
 *              kkmdw_display_set_backlight(KDRV_DISPLAY_BACKLIGHT_ON);
 */
int kmdw_display_set_backlight(int duty);


#endif
