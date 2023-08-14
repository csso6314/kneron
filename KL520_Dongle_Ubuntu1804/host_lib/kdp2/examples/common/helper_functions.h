#ifndef __HELPER_FUNCTIONS_H__
#define __HELPER_FUNCTIONS_H__

#include "kdp2_postprocess.h"

char *read_file_to_buffer_auto_malloc(const char *file_path, long *buffer_size);
void measure_time_begin();
void measure_time_end(double *measued_time);
char *bmp_file_to_rgb565_buffer_auto_malloc(const char *file_path, int *width, int *height);
char *bmp_file_to_rgba8888_buffer_auto_malloc(const char *file_path, int *width, int *height);
void draw_bounding_box_to_bmp_file(const char *in_file_path, const char *out_file_path, kdp2_bounding_box_t boxes[], int box_count);

#endif