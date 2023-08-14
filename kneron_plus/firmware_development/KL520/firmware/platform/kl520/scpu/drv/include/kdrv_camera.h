
#ifndef _KDRV_CAMERA_H_
#define _KDRV_CAMERA_H_
#include "base.h"
typedef struct cam_format {
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;    /* fourcc */
    uint32_t field;          /* enum v2k_field */
    uint32_t bytesperline;   /* for padding, zero if unused */
    uint32_t sizeimage;
    uint32_t colorspace;     /* enum v2k_colorspace */
}cam_format;

#endif
