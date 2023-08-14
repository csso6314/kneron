/*
 * Kneron Display driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "board.h"
#include <cmsis_os2.h>
#include "kmdw_display.h"
#include "kdev_panel.h"

kdrv_display_t *p_display_drv; 

int kmdw_video_renderer_buffer_initialize(struct video_input_params *dp_params)
{
    if(p_display_drv != NULL) {
        return kdrv_display_buffer_initialize(dp_params);
    }
    return -1;
}

uint32_t kmdw_video_renderer_get_buffer_addr(void)
{
    if(p_display_drv != NULL) {
        return kdrv_display_get_buffer();
    }
    return 0;
}

int kmdw_display_get_params(struct video_input_params *dp_params)
{
    if(p_display_drv != NULL) {
        return kdrv_display_get_params(p_display_drv, dp_params);
    }
    return -1;
}

int kmdw_display_get_device_id(void)
{
    if(p_display_drv != NULL) {
        return kdev_panel_read_display_id(p_display_drv);
    }
    return -1;
}

int kmdw_video_renderer_open(struct video_input_params *params)
{
    if(p_display_drv != NULL) {
        kdrv_display_set_params(p_display_drv, params);
        kdev_panel_initialize(p_display_drv);
        return 0;
    }
    return -1;
}

int kmdw_video_renderer_set_camera(uint8_t cam_idx)
{
    if(p_display_drv != NULL) {
        return kdrv_display_set_camera(p_display_drv, cam_idx);
    }
    return -1;
}

int kmdw_video_renderer_start(void)
{
    if(p_display_drv != NULL) {
        return kdrv_display_start(p_display_drv);
    }
    return -1;
}

int kmdw_video_renderer_stop(void)
{
    if(p_display_drv != NULL) {
        return kdrv_display_stop(p_display_drv);
    }
    return -1;
}

int kmdw_display_set_pen_rgb565(uint16_t color, uint16_t pen_width)
{
    if(p_display_drv != NULL) {
        return kdrv_display_set_pen(p_display_drv, color, pen_width);
    }
    return -1;
}

int kmdw_display_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t draw_mode)
{
    if(p_display_drv != NULL) {
        if(draw_mode == DRAW_HINT_BOUNDINGBOX)
            return kdrv_display_draw_static_rect(p_display_drv, x, y, width, height);
        else if(draw_mode == DRAW_FDR_RESULT_BOUNDINGBOX)
            return kdrv_display_draw_rect(p_display_drv, x, y, width, height);
    }
    return -1;
}

int kmdw_display_draw_line(uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye)
{
    if(p_display_drv != NULL) {
        return kdrv_display_draw_line(p_display_drv, xs, ys, xe, ye);
    }
    return -1;
}

int kmdw_display_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if(p_display_drv != NULL) {
        return kdrv_display_fill_rect(p_display_drv, x, y, width, height);
    }
    return -1;
}

int kmdw_display_draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void *buf)
{
    if(p_display_drv != NULL) {
        return kdrv_display_draw_bitmap(p_display_drv, x, y, width, height, buf);
    }
    return -1;
}

int kmdw_display_update_draw_fb(uint32_t addr, uint8_t cam_idx)
{
    if(p_display_drv != NULL) {
        return kdrv_display_update_draw_fb(p_display_drv, addr, cam_idx);
    }
    return -1;
}

int kmdw_display_refresh(void)
{
    if(p_display_drv != NULL) {
        return kdev_panel_refresh(p_display_drv);
    }
    return -1;
}

int kmdw_display_test_pattern_gen(bool pat_gen)
{
    return kdrv_display_test_pattern_gen(p_display_drv, pat_gen);
}

int kmdw_display_set_backlight(int duty)
{
    if(p_display_drv != NULL) {
        return kdrv_display_set_backlight(p_display_drv, duty);
    }
    return -1;
}

int kmdw_display_initialize(void)
{
    p_display_drv = NULL;
    if((p_display_drv = kdrv_display_initialize()) != NULL) {
        return 0;
    }
    return -1;
}
