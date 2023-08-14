/**
 * @file        helper_functions.c
 * @brief       implementation of helper functions
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>

#include "helper_functions.h"

static struct timeval time_begin;
static struct timeval time_end;

void helper_measure_time_begin()
{
    gettimeofday(&time_begin, NULL);
}

void helper_measure_time_end(double *measued_time)
{
    gettimeofday(&time_end, NULL);

    *measued_time = (double)(time_end.tv_sec - time_begin.tv_sec) + (double)(time_end.tv_usec - time_begin.tv_usec) * .000001;
}

#pragma pack(push, 1)
typedef struct
{
    unsigned short int type;
    unsigned int size;
    unsigned short int reserved1, reserved2;
    unsigned int offset;
} FILEHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    unsigned int size;             /* Info Header size in bytes */
    int width, height;             /* Width and height of image */
    unsigned short int planes;     /* Number of colour planes */
    unsigned short int bits;       /* Bits per pixel */
    unsigned int compression;      /* Compression type */
    unsigned int imagesize;        /* Image size in bytes */
    int xresolution, yresolution;  /* Pixels per meter */
    unsigned int ncolours;         /* Number of colours */
    unsigned int importantcolours; /* Important colours */
} INFOHEADER;
#pragma pack(pop)

static void generate_ycbcr_to_ycbcr_order_mapping(kp_image_format_t in_format, kp_image_format_t out_format, int order_mapping[])
{
    // initialize
    int y0_index = 0;
    int cb_index = 0;
    int y1_index = 0;
    int cr_index = 0;

    switch (in_format)
    {
        case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
            cr_index = 0;
            y1_index = 1;
            cb_index = 2;
            y0_index = 3;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
            cb_index = 0;
            y1_index = 1;
            cr_index = 2;
            y0_index = 3;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
            y1_index = 0;
            cr_index = 1;
            y0_index = 2;
            cb_index = 3;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
            y1_index = 0;
            cb_index = 1;
            y0_index = 2;
            cr_index = 3;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
            cr_index = 0;
            y0_index = 1;
            cb_index = 2;
            y1_index = 3;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
            cb_index = 0;
            y0_index = 1;
            cr_index = 2;
            y1_index = 3;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
            y0_index = 0;
            cr_index = 1;
            y1_index = 2;
            cb_index = 3;
            break;
        case KP_IMAGE_FORMAT_YUYV:
        case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
            y0_index = 0;
            cb_index = 1;
            y1_index = 2;
            cr_index = 3;
            break;
        default:
            printf("input format is not supported\n");
            break;
    }

    switch (out_format)
    {
        case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
            order_mapping[0] = cr_index;
            order_mapping[1] = y1_index;
            order_mapping[2] = cb_index;
            order_mapping[3] = y0_index;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
            order_mapping[0] = cb_index;
            order_mapping[1] = y1_index;
            order_mapping[2] = cr_index;
            order_mapping[3] = y0_index;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
            order_mapping[0] = y1_index;
            order_mapping[1] = cr_index;
            order_mapping[2] = y0_index;
            order_mapping[3] = cb_index;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
            order_mapping[0] = y1_index;
            order_mapping[1] = cb_index;
            order_mapping[2] = y0_index;
            order_mapping[3] = cr_index;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
            order_mapping[0] = cr_index;
            order_mapping[1] = y0_index;
            order_mapping[2] = cb_index;
            order_mapping[3] = y1_index;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
            order_mapping[0] = cb_index;
            order_mapping[1] = y0_index;
            order_mapping[2] = cr_index;
            order_mapping[3] = y1_index;
            break;
        case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
            order_mapping[0] = y0_index;
            order_mapping[1] = cr_index;
            order_mapping[2] = y1_index;
            order_mapping[3] = cb_index;
            break;
        case KP_IMAGE_FORMAT_YUYV:
        case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
            order_mapping[0] = y0_index;
            order_mapping[1] = cb_index;
            order_mapping[2] = y1_index;
            order_mapping[3] = cr_index;
            break;
        default:
            printf("output format is not supported\n");
            break;
    }
}

char *helper_bmp_file_to_raw_buffer(const char *file_path, int *width, int *height, kp_image_format_t format)
{
    FILEHEADER header1;
    INFOHEADER header2;

    unsigned short *b16_buf = NULL;
    unsigned char  *b8_buf  = NULL;
    unsigned char *bmp_buf = NULL;

    char *raw_buf = NULL;

    if(NULL == file_path)
        return NULL;

    FILE *bmp_file = fopen(file_path, "rb");

    if (!bmp_file)
    {
        printf("file read failed\n");
        return NULL;
    }

    fread(&header1, 1, sizeof(header1), bmp_file);
    fread(&header2, 1, sizeof(header2), bmp_file);

    *width = header2.width;
    *height = header2.height;

    unsigned short bytes_per_pixel = header2.bits / 8;

    // length of bmp image width in byte should be aligned with 4
    int padding_byte_num = 0;
    if (header2.width * bytes_per_pixel % 4 != 0)
        padding_byte_num = 4 - header2.width * bytes_per_pixel % 4;

    // bmp_buf is used to store pixel data of bmp image
    int bmp_buf_size = (header2.width * bytes_per_pixel + padding_byte_num) * header2.height; // image width of bmp file should be a multiple of 4
    bmp_buf = (unsigned char *)malloc(bmp_buf_size);
    if (NULL == bmp_buf) {
        fclose(bmp_file);
        printf("Error! malloc memory for image buffer failed\n");
        goto err;
    }

    if (header1.offset != sizeof(FILEHEADER) + sizeof(INFOHEADER))
    {
        // move the position indicator of the bmp_file stream to the data offset specified by the FILEHEADER
        fseek(bmp_file, (long)header1.offset, SEEK_SET);
    }

    // read pixel data from bmp_file
    fread(bmp_buf, 1, bmp_buf_size, bmp_file);

    fclose(bmp_file);

    if (header2.bits == 24)
    {
        switch (format)
        {
        case KP_IMAGE_FORMAT_RGB565:
        {
            b16_buf = (unsigned short *)malloc(header2.width * header2.height * 2);
            if ( NULL == b16_buf ) {
                printf("Error! malloc memory for converted data failed\n");
                goto err;
            }
            int rgb565_pos = 0;

            for (int row = header2.height - 1; row >= 0; row--)
            {
                int bmp_pos = (header2.width * 3 + padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel
                for (int col = 0; col < header2.width; col++)
                {
                    unsigned char blue = bmp_buf[bmp_pos];
                    unsigned char green = bmp_buf[bmp_pos + 1];
                    unsigned char red = bmp_buf[bmp_pos + 2];

                    b16_buf[rgb565_pos] = ((red & 0b11111000) << 8) | ((green & 0b11111100) << 3) | (blue >> 3);

                    bmp_pos += 3;
                    rgb565_pos++;
                }
            }

            raw_buf = (char *)b16_buf;
            break;
        }
        case KP_IMAGE_FORMAT_RGBA8888:
        {
            b8_buf = (unsigned char *)malloc(header2.width * header2.height * 4);
            if ( NULL == b8_buf ) {
                printf("Error! malloc memory for converted data failed\n");
                goto err;
            }
            int rgba8888_pos = 0;

            for (int row = header2.height - 1; row >= 0; row--)
            {
                int bmp_pos = (header2.width * 3 + padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel
                for (int col = 0; col < header2.width; col++)
                {
                    unsigned char blue = bmp_buf[bmp_pos];
                    unsigned char green = bmp_buf[bmp_pos + 1];
                    unsigned char red = bmp_buf[bmp_pos + 2];

                    b8_buf[rgba8888_pos++] = red;
                    b8_buf[rgba8888_pos++] = green;
                    b8_buf[rgba8888_pos++] = blue;
                    b8_buf[rgba8888_pos++] = 0;

                    bmp_pos += 3;
                }
            }

            raw_buf = (char *)b8_buf;
            break;
        }
        case KP_IMAGE_FORMAT_YUYV:
        case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
        case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
        case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
        case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
        case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
        case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
        case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
        case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
        {
            b8_buf = (unsigned char *)malloc(header2.width * header2.height * 2);
            if ( NULL == b8_buf ) {
                printf("Error! malloc memory for converted data failed\n");
                goto err;
            }
            int write_pos = 0;

            // generate the order mappings of (y0, cb, y1, cr) to (cr, y1, cb, y0), (cb, y1, cr, y0), ... described in the yuyv or ycbcr image format
            int order_mapping[4] = {0};
            generate_ycbcr_to_ycbcr_order_mapping(KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR, format, order_mapping);

            for (int row = header2.height - 1; row >= 0; row--)
            {
                int bmp_pos = (header2.width * 3 + padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel

                // FIXME? How if the width is an odd number?
                for (int col = 0; col < header2.width / 2; col++)
                {
                    unsigned char blue0 = bmp_buf[bmp_pos];
                    unsigned char green0 = bmp_buf[bmp_pos + 1];
                    unsigned char red0 = bmp_buf[bmp_pos + 2];

                    unsigned char blue1 = bmp_buf[bmp_pos + 3];
                    unsigned char green1 = bmp_buf[bmp_pos + 4];
                    unsigned char red1 = bmp_buf[bmp_pos + 5];

                    // FIXME: use integer method to speed up
                    float output[4];
                    output[0] = 0.299 * red0 + 0.587 * green0 + 0.114 * blue0; // y0
                    output[1] = -0.169 * (red0 + red1) / 2 - 0.331 * (green0 + green1) / 2 + 0.5 * (blue0 + blue1) / 2 + 128; // cb
                    output[2] = 0.299 * red1 + 0.587 * green1 + 0.114 * blue1; // y1
                    output[3] = 0.5 * (red0 + red1) / 2 - 0.419 * (green0 + green1) / 2 - 0.081 * (blue0 + blue1) / 2 + 128;  // cr

                    b8_buf[write_pos++] = (unsigned char)output[order_mapping[0]];
                    b8_buf[write_pos++] = (unsigned char)output[order_mapping[1]];
                    b8_buf[write_pos++] = (unsigned char)output[order_mapping[2]];
                    b8_buf[write_pos++] = (unsigned char)output[order_mapping[3]];

                    bmp_pos += 6;
                }
            }

            raw_buf = (char *)b8_buf;
            break;
        }
        case KP_IMAGE_FORMAT_YUV420:
        {
            if ((0 != header2.width % 2) || (0 != header2.height % 2)) {
                printf("Error! width or height is not even number\n");
                goto err;
            }

            b8_buf = (unsigned char *)malloc(header2.width * header2.height * 1.5);
            if ( NULL == b8_buf ) {
                printf("Error! malloc memory for converted data failed\n");
                goto err;
            }

            int y_bias = header2.width * header2.height;
            int u_bias = y_bias / 4;
            int u_pos = y_bias;
            int v_pos = y_bias + u_bias;

            for (int row = header2.height - 1; row >= 0; row -= 2)
            {
                int bmp_pos_row_0 = (header2.width * 3 + padding_byte_num) * row;       // we only support bmp file with 3 bytes per pixel
                int bmp_pos_row_1 = (header2.width * 3 + padding_byte_num) * (row - 1); // we only support bmp file with 3 bytes per pixel
                int y_write_pos_row_0 = header2.width * (header2.height - row - 1);
                int y_write_pos_row_1 = header2.width * (header2.height - row);

                for (int col = 0; col < header2.width; col += 2)
                {
                    unsigned char blue0 = bmp_buf[bmp_pos_row_0];
                    unsigned char green0 = bmp_buf[bmp_pos_row_0 + 1];
                    unsigned char red0 = bmp_buf[bmp_pos_row_0 + 2];

                    unsigned char blue1 = bmp_buf[bmp_pos_row_0 + 3];
                    unsigned char green1 = bmp_buf[bmp_pos_row_0 + 4];
                    unsigned char red1 = bmp_buf[bmp_pos_row_0 + 5];

                    unsigned char blue2 = bmp_buf[bmp_pos_row_1];
                    unsigned char green2 = bmp_buf[bmp_pos_row_1 + 1];
                    unsigned char red2 = bmp_buf[bmp_pos_row_1 + 2];

                    unsigned char blue3 = bmp_buf[bmp_pos_row_1 + 3];
                    unsigned char green3 = bmp_buf[bmp_pos_row_1 + 4];
                    unsigned char red3 = bmp_buf[bmp_pos_row_1 + 5];

                    float y[2][2];
                    float u, v;

                    y[0][0] = 0.299 * red0 + 0.587 * green0 + 0.114 * blue0;
                    y[0][1] = 0.299 * red1 + 0.587 * green1 + 0.114 * blue1;
                    y[1][0] = 0.299 * red2 + 0.587 * green2 + 0.114 * blue2;
                    y[1][1] = 0.299 * red3 + 0.587 * green3 + 0.114 * blue3;

                    u = -0.169 * (red0 + red1 + red2 + red3) / 4 - 0.331 * (green0 + green1 + green2 + green3) / 4 + 0.5 * (blue0 + blue1 + blue2 + blue3) / 4 + 128;
                    v = 0.5 * (red0 + red1 + red2 + red3) / 4 - 0.419 * (green0 + green1 + green2 + green3) / 4 - 0.081 * (blue0 + blue1 + blue2 + blue3) / 4 + 128;

                    b8_buf[y_write_pos_row_0++] = (unsigned char)y[0][0];
                    b8_buf[y_write_pos_row_0++] = (unsigned char)y[0][1];
                    b8_buf[y_write_pos_row_1++] = (unsigned char)y[1][0];
                    b8_buf[y_write_pos_row_1++] = (unsigned char)y[1][0];
                    b8_buf[u_pos++] = (unsigned char)u;
                    b8_buf[v_pos++] = (unsigned char)v;

                    bmp_pos_row_0 += 6;
                    bmp_pos_row_1 += 6;
                }
            }

            raw_buf = (char *)b8_buf;
            break;
        }
        case KP_IMAGE_FORMAT_UNKNOWN:
        default:
            printf("image format is not supported\n");
            break;
        }
    }
    else
    {
        printf("support only 24 bit bmp\n");
    }

    free(bmp_buf);

    return raw_buf;

err:
    free(bmp_buf);
    free(b16_buf);
    free(b8_buf);

    return NULL;
}

char *helper_bin_file_to_raw_buffer(const char *file_path, int width, int height, kp_image_format_t format)
{
    if (width <= 0 || height <= 0)
        return NULL;

    float bytes_per_pixel = 0;

    switch (format)
    {
    case KP_IMAGE_FORMAT_RGB565:
    case KP_IMAGE_FORMAT_YUYV:
    case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
    case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
    case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
    case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
        bytes_per_pixel = 2;
        break;
    case KP_IMAGE_FORMAT_RGBA8888:
        bytes_per_pixel = 4;
        break;
    case KP_IMAGE_FORMAT_RAW8:
        bytes_per_pixel = 1;
        break;
    case KP_IMAGE_FORMAT_YUV420:
        if ((0 != width % 2) || (0 != height % 2)) {
            printf("image width and height must be even number\n");
            return NULL;
        }

        bytes_per_pixel = 1.5;
        break;
    default:
    case KP_IMAGE_FORMAT_UNKNOWN:
        printf("image format is not supported\n");
        return NULL;
    }

    FILE *bin_file = fopen(file_path, "rb");
    if (!bin_file)
    {
        printf("%s(): fopen failed, file:%s, %s\n", __FUNCTION__, file_path, strerror(errno));
        return NULL;
    }

    fseek(bin_file, 0, SEEK_END);
    long file_size = ftell(bin_file); //get the size

    if (file_size != width * height * bytes_per_pixel)
    {
        printf("%s(): file size does not match input image width, height or format\n", __FUNCTION__);
        fclose(bin_file);
        return NULL;
    }

    char *buffer = (char *)malloc(file_size);
    if(NULL == buffer) {
        printf("Error! %s(): malloc for buffer failed\n", __FUNCTION__);
        fclose(bin_file);
        return NULL;
    }

    fseek(bin_file, 0, SEEK_SET); //move to beginning

    size_t read_size = fread(buffer, 1, file_size, bin_file);
    if (read_size != (size_t)file_size)
    {
        printf("%s(): fread failed, file size: %u, read size %u\n", __FUNCTION__,
               (unsigned int)file_size, (unsigned int)read_size);
        free(buffer);
        buffer = NULL;
    }

    fclose(bin_file);

    return buffer;
}

static void draw_box_on_bmp_raw_buffer(kp_bounding_box_t boxes[], int box_count, int width, int height, unsigned char *bmp_buf, int padding_byte_num)
{
    const int thickness = 2;

    int bmp_buf_size = (width * 3 + padding_byte_num) * height;

    for (int i = 0; i < box_count; i++)
    {
        if (boxes[i].x2 >= (float)width)
            boxes[i].x2 = (float)width - 1;

        if (boxes[i].y2 >= (float)height)
            boxes[i].y2 = (float)height - 1;

        int bmp_x1 = (int)boxes[i].x1;
        int bmp_y1 = (int)(height - boxes[i].y1 - 1);
        int bmp_x2 = (int)boxes[i].x2;
        int bmp_y2 = (int)(height - boxes[i].y2 - 1);

        int pos;
        int diff_x = bmp_x2 - bmp_x1;
        int diff_y = bmp_y1 - bmp_y2;

        int class = boxes[i].class_num;
        int bb = 100 + (0 + 25 * class) % 156;
        int gg = 100 + (0 + 80 + 40 * class) % 156;
        int rr = 100 + (0 + 120 + 60 * class) % 156;

        // (x1,y1) -> (x2,y1)
        for (int k = 0; k < thickness; k++){
            pos = (width * 3 + padding_byte_num) * (bmp_y1-k) + bmp_x1 * 3;
            for (int j = 0; j <= diff_x; j++) // also draw the right top corner
            {
                if ((bmp_buf_size <= (pos + thickness)) || (0 > pos))
                    break;

                bmp_buf[pos] = bb;
                bmp_buf[pos + 1] = gg;
                bmp_buf[pos + 2] = rr;
                pos += 3;
            }
        }

        // (x1,y2) -> (x2,y2)
        for (int k = 0; k < thickness; k++){
            pos = (width * 3 + padding_byte_num) * (bmp_y2+k) + bmp_x1 * 3;

            for (int j = 0; j < diff_x; j++)
            {
                if ((bmp_buf_size <= (pos + thickness)) || (0 > pos))
                    break;

                bmp_buf[pos] = bb;
                bmp_buf[pos + 1] = gg;
                bmp_buf[pos + 2] = rr;
                pos += 3;
            }
        }

        // (x1,y1) -> (x1,y2)
        for (int k = 0; k < thickness; k++){
            pos = (width * 3 + padding_byte_num) * bmp_y2 + (bmp_x1+k) * 3;

            for (int j = 0; j < diff_y; j++)
            {
                if ((bmp_buf_size <= (pos + thickness)) || (0 > pos))
                    break;

                bmp_buf[pos] = bb;
                bmp_buf[pos + 1] = gg;
                bmp_buf[pos + 2] = rr;
                pos += width * 3 + padding_byte_num;
            }
        }

        // (x2,y1) -> (x2,y2)
        for (int k = 0; k < thickness; k++){
            pos = (width * 3 + padding_byte_num) * bmp_y2 + (bmp_x2-k) * 3;

            for (int j = 0; j < diff_y; j++)
            {
                if ((bmp_buf_size <= (pos + thickness)) || (0 > pos))
                    break;

                bmp_buf[pos] = bb;
                bmp_buf[pos + 1] = gg;
                bmp_buf[pos + 2] = rr;
                pos += width * 3 + padding_byte_num;
            }
        }
    }
}

void helper_draw_box_on_bmp(const char *in_bmp_path, const char *out_bmp_path, kp_bounding_box_t boxes[], int box_count)
{
    FILEHEADER header1;
    INFOHEADER header2;

    unsigned char *bmp_buf = NULL;
    unsigned char *extended_info_header = NULL;

    FILE *in_bmp_file = fopen(in_bmp_path, "rb");
    if (!in_bmp_file)
    {
        printf("file read failed\n");
        return;
    }

    fread(&header1, 1, sizeof(header1), in_bmp_file);
    fread(&header2, 1, sizeof(header2), in_bmp_file);

    unsigned short bytes_per_pixel = header2.bits / 8;

    // length of bmp image width in byte should be aligned with 4
    int padding_byte_num = 0;
    if (header2.width * bytes_per_pixel % 4 != 0)
        padding_byte_num = 4 - header2.width * bytes_per_pixel % 4;

    // bmp_buf is used to store pixel data of bmp image
    int bmp_buf_size = (header2.width * bytes_per_pixel + padding_byte_num) * header2.height; // image width of bmp file should be a multiple of 4
    bmp_buf = (unsigned char *)malloc(bmp_buf_size);
    if(NULL == bmp_buf) {
        printf("Error! %s(): malloc for drawing buffer failed\n", __FUNCTION__);
        goto err;
    }

    // read extended info header if the length of info header exceeds sizeof(INFOHEADER)
    size_t extended_info_header_length = 0;
    if (header1.offset != sizeof(FILEHEADER) + sizeof(INFOHEADER))
    {
        extended_info_header_length = header1.offset - (sizeof(FILEHEADER) + sizeof(INFOHEADER));
        extended_info_header = (unsigned char *)malloc(extended_info_header_length);
        if (NULL == extended_info_header ) {
            printf("Error! %s(): malloc for extended info header failed\n", __FUNCTION__);
            goto err;
        }
        fread(extended_info_header, 1, extended_info_header_length, in_bmp_file);
    }

    fread(bmp_buf, 1, bmp_buf_size, in_bmp_file);
    fclose(in_bmp_file);

    if (header2.bits == 24)
    {
        draw_box_on_bmp_raw_buffer(boxes, box_count, header2.width, header2.height, bmp_buf, padding_byte_num);
    }
    else
    {
        printf("support only 24 bit bmp\n");
    }

    FILE *out_bmp_file = fopen(out_bmp_path, "wb");

    fwrite(&header1, 1, sizeof(header1), out_bmp_file);
    fwrite(&header2, 1, sizeof(header2), out_bmp_file);

    // write extended info header
    if (extended_info_header)
    {
        fwrite(extended_info_header, 1, extended_info_header_length, out_bmp_file);
        free(extended_info_header);
    }

    fwrite(bmp_buf, 1, bmp_buf_size, out_bmp_file);
    fclose(out_bmp_file);

    free(bmp_buf);

    return;

err:
	fclose(in_bmp_file);

    free(bmp_buf);
    free(extended_info_header);

    return;
}

static unsigned char clamp_to_0_255(float num)
{
    if (num > 255)
        return 255;
    else if (num < 0)
        return 0;
    else
        return (unsigned char)num;
}

// Passing bmp_padding_byte_num to avoid recalculation
// bmp_buf is used to store pixel data of bmp image
static void convert_bin_to_bmp_pixel_data(unsigned char *bin_buf, unsigned char *bmp_buf, int width, int height, kp_image_format_t bin_format, int bmp_padding_byte_num)
{
    switch (bin_format)
    {
    case KP_IMAGE_FORMAT_RGB565:
    {
        unsigned short *rgb565_buf = (unsigned short *)bin_buf;

        int rgb565_pos = 0;

        for (int row = height - 1; row >= 0; row--)
        {
            int bmp_pos = (width * 3 + bmp_padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel
            for (int col = 0; col < width; col++)
            {
                bmp_buf[bmp_pos] = (rgb565_buf[rgb565_pos] << 3) & 0b11111000;     // blue
                bmp_buf[bmp_pos + 1] = (rgb565_buf[rgb565_pos] >> 3) & 0b11111100; // green
                bmp_buf[bmp_pos + 2] = (rgb565_buf[rgb565_pos] >> 8) & 0b11111000; // red

                bmp_pos += 3;
                rgb565_pos++;
            }
        }
        break;
    }
    case KP_IMAGE_FORMAT_RGBA8888:
    {
        // Just an alias for buffer naming consistency
        unsigned char *rgba8888_buf = bin_buf;

        int rgba8888_pos = 0;

        for (int row = height - 1; row >= 0; row--)
        {
            int bmp_pos = (width * 3 + bmp_padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel
            for (int col = 0; col < width; col++)
            {
                bmp_buf[bmp_pos] = rgba8888_buf[rgba8888_pos + 2];     // blue
                bmp_buf[bmp_pos + 1] = rgba8888_buf[rgba8888_pos + 1]; // green
                bmp_buf[bmp_pos + 2] = rgba8888_buf[rgba8888_pos];     // red

                bmp_pos += 3;
                rgba8888_pos += 4;
            }
        }
        break;
    }
    case KP_IMAGE_FORMAT_YUYV:
    case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
    case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
    case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
    case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
    {
        // Just an alias for buffer naming consistency
        unsigned char *ycbcr_buf = bin_buf;

        int ycbcr_pos = 0;

        // generate the order mappings of (cr, y1, cb, y0), (cb, y1, cr, y0) to (y0, cb, y1, cr)... described in the yuyv or ycbcr image format
        int order_mapping[4] = {0};
        generate_ycbcr_to_ycbcr_order_mapping(bin_format, KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR, order_mapping);

        for (int row = height - 1; row >= 0; row--)
        {
            int bmp_pos = (width * 3 + bmp_padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel

            // FIXME: How if the width is an odd number?
            for (int col = 0; col < width / 2; col++)
            {
                unsigned char output[4];
                output[0] = ycbcr_buf[ycbcr_pos++]; // one of (y0, cb, y1, cr)
                output[1] = ycbcr_buf[ycbcr_pos++]; // one of (y0, cb, y1, cr)
                output[2] = ycbcr_buf[ycbcr_pos++]; // one of (y0, cb, y1, cr)
                output[3] = ycbcr_buf[ycbcr_pos++]; // one of (y0, cb, y1, cr)

                unsigned char y0 = output[order_mapping[0]];
                unsigned char cb = output[order_mapping[1]];
                unsigned char y1 = output[order_mapping[2]];
                unsigned char cr = output[order_mapping[3]];

                // FIXME: use integer method to speed up
                float b_diff = 1.77 * (cb - 128);
                float g_diff = -0.343 * (cb - 128) - 0.714 * (cr - 128);
                float r_diff = 1.403 * (cr - 128);

                bmp_buf[bmp_pos++] = clamp_to_0_255(y0 + b_diff);
                bmp_buf[bmp_pos++] = clamp_to_0_255(y0 + g_diff);
                bmp_buf[bmp_pos++] = clamp_to_0_255(y0 + r_diff);
                bmp_buf[bmp_pos++] = clamp_to_0_255(y1 + b_diff);
                bmp_buf[bmp_pos++] = clamp_to_0_255(y1 + g_diff);
                bmp_buf[bmp_pos++] = clamp_to_0_255(y1 + r_diff);
            }
        }
        break;
    }
    case KP_IMAGE_FORMAT_RAW8:
    {
        // Just an alias for buffer naming consistency
        unsigned char *raw8_buf = bin_buf;

        int raw8_pos = 0;

        for (int row = height - 1; row >= 0; row--)
        {
            int bmp_pos = (width * 3 + bmp_padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel
            for (int col = 0; col < width; col++)
            {
                memset(bmp_buf + bmp_pos, raw8_buf[raw8_pos++], 3);
                bmp_pos += 3;
            }
        }
        break;
    }
    case KP_IMAGE_FORMAT_YUV420:
    {
        if ((0 != width % 2) || (0 != height % 2)) {
            printf("Error! width or height is not even number\n");
            return;
        }

        // Just an alias for buffer naming consistency
        unsigned char *yuv_buf = bin_buf;
        int y_bias = width * height;
        int u_bias = y_bias / 4;
        int u_pos = y_bias;
        int v_pos = y_bias + u_bias;

        for (int row = height - 1; row >= 0; row -= 2)
        {
            int bmp_pos_row_0 = (width * 3 + bmp_padding_byte_num) * row; // we only support bmp file with 3 bytes per pixel
            int bmp_pos_row_1 = (width * 3 + bmp_padding_byte_num) * (row - 1); // we only support bmp file with 3 bytes per pixel
            int y_pos_row_0 = width * (height - row - 1);
            int y_pos_row_1 = width * (height - row);

            for (int col = 0; col < width; col += 2)
            {
                unsigned char y[2][2];
                unsigned char u, v;

                y[0][0] = yuv_buf[y_pos_row_0++];
                y[0][1] = yuv_buf[y_pos_row_0++];
                y[1][0] = yuv_buf[y_pos_row_1++];
                y[1][1] = yuv_buf[y_pos_row_1++];

                u = yuv_buf[u_pos++];
                v = yuv_buf[v_pos++];

                // FIXME: use integer method to speed up
                float b_diff = 1.77 * (u - 128);
                float g_diff = -0.343 * (u - 128) - 0.714 * (v - 128);
                float r_diff = 1.403 * (v - 128);

                bmp_buf[bmp_pos_row_0++] = clamp_to_0_255(y[0][0] + b_diff);
                bmp_buf[bmp_pos_row_0++] = clamp_to_0_255(y[0][0] + g_diff);
                bmp_buf[bmp_pos_row_0++] = clamp_to_0_255(y[0][0] + r_diff);
                bmp_buf[bmp_pos_row_0++] = clamp_to_0_255(y[0][1] + b_diff);
                bmp_buf[bmp_pos_row_0++] = clamp_to_0_255(y[0][1] + g_diff);
                bmp_buf[bmp_pos_row_0++] = clamp_to_0_255(y[0][1] + r_diff);

                bmp_buf[bmp_pos_row_1++] = clamp_to_0_255(y[1][0] + b_diff);
                bmp_buf[bmp_pos_row_1++] = clamp_to_0_255(y[1][0] + g_diff);
                bmp_buf[bmp_pos_row_1++] = clamp_to_0_255(y[1][0] + r_diff);
                bmp_buf[bmp_pos_row_1++] = clamp_to_0_255(y[1][1] + b_diff);
                bmp_buf[bmp_pos_row_1++] = clamp_to_0_255(y[1][1] + g_diff);
                bmp_buf[bmp_pos_row_1++] = clamp_to_0_255(y[1][1] + r_diff);
            }
        }
        break;
    }
    default:
    case KP_IMAGE_FORMAT_UNKNOWN:
        printf("image format is not supported\n");
        return;
    }
}

void helper_draw_box_on_bmp_from_bin(const char *in_bin_path, int in_bin_width, int in_bin_height, kp_image_format_t in_bin_format,
                                     const char *out_bmp_path, kp_bounding_box_t boxes[], int box_count)
{

    // Open .bmp file to write
    FILE *out_bmp_file = fopen(out_bmp_path, "wb");
    if (!out_bmp_file)
    {
        printf("file read failed\n");
        return;
    }

    unsigned char *bin_buf = NULL;
    unsigned char *bmp_buf = NULL;

    bin_buf = (unsigned char *)helper_bin_file_to_raw_buffer(in_bin_path, in_bin_width, in_bin_height, in_bin_format);
    if( NULL == bin_buf )
    {
        printf("Error! read file to buffer failed\n");
        goto err;
    }

    // Prepare BMP file header
    FILEHEADER header1 = {0};
    INFOHEADER header2 = {0};

    unsigned short bytes_per_pixel_bmp = 3; // This is constant (also for RAW8) for all supported raw formats

    header1.type = 0x4D42; // 'B' and 'M' in ASCII code (little endian)
    header1.size = sizeof(FILEHEADER) + sizeof(INFOHEADER) + in_bin_width * in_bin_height * bytes_per_pixel_bmp;
    header1.offset = sizeof(FILEHEADER) + sizeof(INFOHEADER);

    header2.size = sizeof(INFOHEADER);
    header2.width = in_bin_width;
    header2.height = in_bin_height;
    header2.planes = 1;
    header2.bits = bytes_per_pixel_bmp * 8;
    header2.imagesize = in_bin_width * in_bin_height * bytes_per_pixel_bmp;

    // length of bmp image width in byte should be aligned with 4
    int padding_byte_num = 0;
    if (in_bin_width * bytes_per_pixel_bmp % 4 != 0)
        padding_byte_num = 4 - in_bin_width * bytes_per_pixel_bmp % 4;

    // bmp_buf is used to store pixel data of bmp image
    int bmp_buf_size = (in_bin_width * bytes_per_pixel_bmp + padding_byte_num) * in_bin_height; // image width of bmp file should be a multiple of 4
    bmp_buf = (unsigned char *)malloc(bmp_buf_size);
    if(NULL == bmp_buf ) {
        printf("Error! malloc for image buffer failed\n");
        goto err;
    }

    convert_bin_to_bmp_pixel_data(bin_buf, bmp_buf, in_bin_width, in_bin_height, in_bin_format, padding_byte_num);

    fwrite(&header1, 1, sizeof(header1), out_bmp_file);
    fwrite(&header2, 1, sizeof(header2), out_bmp_file);

    draw_box_on_bmp_raw_buffer(boxes, box_count, in_bin_width, in_bin_height, bmp_buf, padding_byte_num);

    fwrite(bmp_buf, 1, bmp_buf_size, out_bmp_file);

    fclose(out_bmp_file);
    free(bin_buf);
    free(bmp_buf);
	return;

err:
    fclose(out_bmp_file);

    free(bin_buf);
    free(bmp_buf);

    return;
}

void helper_draw_box_of_crop_area_on_bmp(const char *in_bmp_path, const char *out_bmp_path, kp_bounding_box_t boxes[], int box_count, kp_inf_crop_box_t crop_box)
{
    FILEHEADER header1;
    INFOHEADER header2;
    unsigned char *extended_info_header = NULL;
    unsigned char *bmp_buf = NULL;
    unsigned char *crop_bmp_buf = NULL;

    FILE *in_bmp_file = fopen(in_bmp_path, "rb");
    if (!in_bmp_file)
    {
        printf("file read failed\n");
        return;
    }

    fread(&header1, 1, sizeof(header1), in_bmp_file);
    fread(&header2, 1, sizeof(header2), in_bmp_file);

    // read extended info header if the length of info header exceeds sizeof(INFOHEADER)
    size_t extended_info_header_length = 0;
    if (header1.offset != sizeof(FILEHEADER) + sizeof(INFOHEADER))
    {
        extended_info_header_length = header1.offset - (sizeof(FILEHEADER) + sizeof(INFOHEADER));
        extended_info_header = (unsigned char *)malloc(extended_info_header_length);
        if (NULL == extended_info_header ) {
            printf("Error! %s(): malloc for extended info header failed\n", __FUNCTION__);
            goto err;
        }
        fread(extended_info_header, 1, extended_info_header_length, in_bmp_file);
    }

    // Init buffer for origin image
    unsigned short bytes_per_pixel = header2.bits / 8;

    // length of bmp image width in byte should be aligned with 4
    int padding_byte_num_origin = 0;
    if (header2.width * bytes_per_pixel % 4 != 0)
        padding_byte_num_origin = 4 - header2.width * bytes_per_pixel % 4;

    // bmp_buf is used to store pixel data of bmp image
    int bmp_buf_size = (header2.width * bytes_per_pixel + padding_byte_num_origin) * header2.height; // image width of bmp file should be a multiple of 4
    bmp_buf = (unsigned char *)malloc(bmp_buf_size);
    if(NULL == bmp_buf){
        printf("Error! malloc for image buffer failed\n");
        goto err;
    }

    // Init buffer for crop area
    // length of bmp image width in byte should be aligned with 4
    int padding_byte_num_crop = 0;
    if (crop_box.width * bytes_per_pixel % 4 != 0)
        padding_byte_num_crop = 4 - crop_box.width * bytes_per_pixel % 4;

    int crop_bmp_buf_size = (crop_box.width * bytes_per_pixel + padding_byte_num_crop) * crop_box.height; // image width of bmp file should be a multiple of 4
    crop_bmp_buf = (unsigned char *)malloc(crop_bmp_buf_size);
    if(NULL == crop_bmp_buf){
        printf("Error! malloc for cropped area buffer failed\n");
        goto err;
    }

    // Read whole image raw data
    fread(bmp_buf, 1, bmp_buf_size, in_bmp_file);

    // Find out the raw data in crop area, copy data row by row. Note that image width (in byte) of bmp file should be a multiple of 4
    // otherwise padding byte(s) is needed (not necessarily 0 (?) but use 0 here)
    for (int row = 0; row < crop_box.height; row++)
    {
        // we only support bmp file with 3 bytes per pixel
        memcpy(crop_bmp_buf + row * (crop_box.width * 3 + padding_byte_num_crop),
               bmp_buf + crop_box.x1 * 3 + (header2.height - crop_box.y1 - crop_box.height + row) * (header2.width * 3 + padding_byte_num_origin),
               crop_box.width * 3 * sizeof(unsigned char));

        // set padding byte(s) to 0
        memset(crop_bmp_buf + row * (crop_box.width * 3 + padding_byte_num_crop) + crop_box.width * 3, 0, padding_byte_num_crop);
    }

    free(bmp_buf);

    if (header2.bits == 24)
    {
        draw_box_on_bmp_raw_buffer(boxes, box_count, crop_box.width, crop_box.height, crop_bmp_buf, padding_byte_num_crop);
    }
    else
    {
        printf("support only 24 bit bmp\n");
    }

    FILE *out_bmp_file = fopen(out_bmp_path, "wb");

    header1.size = crop_bmp_buf_size + sizeof(FILEHEADER) + sizeof(INFOHEADER);
    header2.width = crop_box.width;
    header2.height = crop_box.height;
    header2.imagesize = 0;

    fwrite(&header1, 1, sizeof(header1), out_bmp_file);
    fwrite(&header2, 1, sizeof(header2), out_bmp_file);

    // write extended info header
    if (extended_info_header)
    {
        fwrite(extended_info_header, 1, extended_info_header_length, out_bmp_file);
        free(extended_info_header);
    }

    fwrite(crop_bmp_buf, 1, crop_bmp_buf_size, out_bmp_file);
    fclose(in_bmp_file);
    fclose(out_bmp_file);
    free(crop_bmp_buf);

    return;

err:
    fclose(in_bmp_file);

    free(bmp_buf);
    free(crop_bmp_buf);
    free(extended_info_header);

    return;
}

void helper_draw_box_of_crop_area_on_bmp_from_bin(const char *in_bin_path, int in_bin_width, int in_bin_height, kp_image_format_t in_bin_format,
                                                  const char *out_bmp_path, kp_bounding_box_t boxes[], int box_count, kp_inf_crop_box_t crop_box)
{
    unsigned char *bmp_buf = NULL;
    unsigned char *crop_bmp_buf = NULL;
    unsigned char *bin_buf = NULL;

    // Open .bmp file to write
    FILE *out_bmp_file = NULL;
    out_bmp_file = fopen(out_bmp_path, "wb");
    if (!out_bmp_file)
    {
        printf("file read failed\n");
        return;
    }

    bin_buf = (unsigned char *)helper_bin_file_to_raw_buffer(in_bin_path, in_bin_width, in_bin_height, in_bin_format);
    if(NULL == bin_buf) {
        printf("error! read file to buffer failed\n");
        goto err;
    }

    // Create corresponding pixel data (in bmp format) of raw image

    // Init buffer for origin image
    unsigned short bytes_per_pixel = 3;

    // length of bmp image width in byte should be aligned with 4
    int padding_byte_num_origin = 0;
    if (in_bin_width * bytes_per_pixel % 4 != 0)
        padding_byte_num_origin = 4 - in_bin_width * bytes_per_pixel % 4;

    // bmp_buf is used to store pixel data of bmp image
    int bmp_buf_size = (in_bin_width * bytes_per_pixel + padding_byte_num_origin) * in_bin_height; // image width of bmp file should be a multiple of 4
    bmp_buf = (unsigned char *)malloc(bmp_buf_size);
    if(NULL == bmp_buf) {
        printf("error! malloc %s failed\n", "bmp_buf");
        goto err;
    }

    convert_bin_to_bmp_pixel_data(bin_buf, bmp_buf, in_bin_width, in_bin_height, in_bin_format, padding_byte_num_origin);

    // Init buffer for crop area
    // length of bmp image width in byte should be aligned with 4
    int padding_byte_num_crop = 0;
    if (crop_box.width * bytes_per_pixel % 4 != 0)
        padding_byte_num_crop = 4 - crop_box.width * bytes_per_pixel % 4;

    int crop_bmp_buf_size = (crop_box.width * bytes_per_pixel + padding_byte_num_crop) * crop_box.height; // image width of bmp file should be a multiple of 4
    crop_bmp_buf = (unsigned char *)malloc(crop_bmp_buf_size);
    if (NULL == crop_bmp_buf) {
        printf("error! malloc %s failed\n", "crop_bmp_buf");
        goto err;
    }

    // Find out the raw data in crop area, copy data row by row. Note that image width (in byte) of bmp file should be a multiple of 4
    // otherwise padding byte(s) is needed (not necessarily 0 (?) but use 0 here)
    for (int row = 0; row < crop_box.height; row++)
    {
        // we only support bmp file with 3 bytes per pixel
        memcpy(crop_bmp_buf + row * (crop_box.width * 3 + padding_byte_num_crop),
               bmp_buf + crop_box.x1 * 3 + (in_bin_height - crop_box.y1 - crop_box.height + row) * (in_bin_width * 3 + padding_byte_num_origin),
               crop_box.width * 3 * sizeof(unsigned char));

        // set padding byte(s) to 0
        memset(crop_bmp_buf + row * (crop_box.width * 3 + padding_byte_num_crop) + crop_box.width * 3, 0, padding_byte_num_crop);
    }

    free(bmp_buf);
    bmp_buf=NULL;

    draw_box_on_bmp_raw_buffer(boxes, box_count, crop_box.width, crop_box.height, crop_bmp_buf, padding_byte_num_crop);

    // Prepare BMP file header
    FILEHEADER header1 = {0};
    INFOHEADER header2 = {0};

    unsigned short bytes_per_pixel_bmp = 3; // This is constant (also for RAW8) for all supported raw formats

    header1.type = 0x4D42; // 'B' and 'M' in ASCII code (little endian)
    header1.size = sizeof(FILEHEADER) + sizeof(INFOHEADER) + crop_box.width * crop_box.height * bytes_per_pixel_bmp;
    header1.offset = sizeof(FILEHEADER) + sizeof(INFOHEADER);

    header2.size = sizeof(INFOHEADER);
    header2.width = crop_box.width;
    header2.height = crop_box.height;
    header2.planes = 1;
    header2.bits = bytes_per_pixel_bmp * 8;
    header2.imagesize = crop_box.width * crop_box.height * bytes_per_pixel_bmp;

    fwrite(&header1, 1, sizeof(header1), out_bmp_file);
    fwrite(&header2, 1, sizeof(header2), out_bmp_file);

    fwrite(crop_bmp_buf, 1, crop_bmp_buf_size, out_bmp_file);

    fclose(out_bmp_file);
    free(bin_buf);

    return;

err:
    fclose(out_bmp_file);

    free(bin_buf);
    free(bmp_buf);
    free(crop_bmp_buf);

    return;
}

void helper_print_yolo_box(kp_yolo_result_t *yolo_result)
{
    printf("\n");
    printf("detectable class count : %d\n", yolo_result->class_count);
    printf("box count : %d\n", yolo_result->box_count);
    for (int i = 0; i < yolo_result->box_count; i++)
    {
        printf("Box %d (x1, y1, x2, y2, score, class) = %.1f, %.1f, %.1f, %.1f, %f, %d\n",
               i,
               yolo_result->boxes[i].x1, yolo_result->boxes[i].y1,
               yolo_result->boxes[i].x2, yolo_result->boxes[i].y2,
               yolo_result->boxes[i].score, yolo_result->boxes[i].class_num);
    }
}

void helper_print_yolo_box_on_bmp(kp_yolo_result_t *yolo_result, char *in_bmp_path)
{
    // output bounding boxes to a new created bmp file
    char out_bmp_path[128];
    sprintf(out_bmp_path, "output_%s", helper_file_name_from_path(in_bmp_path));
    helper_draw_box_on_bmp(in_bmp_path, out_bmp_path, yolo_result->boxes, yolo_result->box_count);

    helper_print_yolo_box(yolo_result);

    if (yolo_result->box_count > 0)
        printf("\noutput bounding boxes on '%s'\n", out_bmp_path);
    else
        printf("\noutput result on '%s'\n", out_bmp_path);
}

void helper_print_yolo_box_on_bmp_from_bin(kp_yolo_result_t *yolo_result, char *in_bin_path, int in_bin_width, int in_bin_height,
                                           kp_image_format_t in_bin_format)
{
    // output bounding boxes to a new created bmp file from bin file with raw format
    char out_bmp_path[128];

    // 128 - 11 = 117 -> see sprintf below, output_.bmp takes up 11 bytes,
    // so the length of this array should be length of out_bmp_path - 11 to prevent overflow
    char file_name_without_ext[117];
    char *file_name = helper_file_name_from_path(in_bin_path);

    strcpy(file_name_without_ext, file_name);

    int len = strlen(file_name_without_ext);
    if (len <= 4)  {
        printf("ERROR! invalid file name: %s\n", file_name_without_ext);
        return;
    }

    // .bin takes 4 chars
    file_name_without_ext[len - 4] = '\0';

    sprintf(out_bmp_path, "output_%s.bmp", file_name_without_ext);

    helper_draw_box_on_bmp_from_bin(in_bin_path, in_bin_width, in_bin_height, in_bin_format, out_bmp_path, yolo_result->boxes, yolo_result->box_count);

    helper_print_yolo_box(yolo_result);

    if (yolo_result->box_count > 0)
        printf("\noutput bounding boxes on '%s'\n", out_bmp_path);
    else
        printf("\noutput result on '%s'\n", out_bmp_path);
}

void helper_print_yolo_box_of_crop_area(kp_yolo_result_t *yolo_result, kp_inf_crop_box_t crop_box)
{
    printf("\n");
    printf("crop box width : %d\n", crop_box.width);
    printf("crop box height : %d\n", crop_box.height);
    printf("crop box number : %d\n", crop_box.crop_number);
    printf("detectable class count : %d\n", yolo_result->class_count);
    printf("box count : %d\n", yolo_result->box_count);
    for (int i = 0; i < yolo_result->box_count; i++)
    {
        printf("Box %d (x1, y1, x2, y2, score, class) = %.1f, %.1f, %.1f, %.1f, %f, %d\n",
               i,
               yolo_result->boxes[i].x1, yolo_result->boxes[i].y1,
               yolo_result->boxes[i].x2, yolo_result->boxes[i].y2,
               yolo_result->boxes[i].score, yolo_result->boxes[i].class_num);
    }
}

void helper_print_yolo_box_of_crop_area_on_bmp(kp_yolo_result_t *yolo_result, char *in_bmp_path, kp_inf_crop_box_t crop_box)
{
    // output bounding boxes to a new created bmp file
    char out_bmp_path[128];

    // 128 - 17 = 111 -> see sprintf below, output__crop%d.bmp takes up 17 bytes (crop num max is 4),
    // so the length of this array should be length of out_bmp_path - 17 to prevent overflow
    char in_bmp_filename_without_ext[111];
    char *in_bmp_filename = helper_file_name_from_path(in_bmp_path);

    strcpy(in_bmp_filename_without_ext, in_bmp_filename);

    int len = strlen(in_bmp_filename_without_ext);
    if (len <= 4)  {
        printf("ERROR! invalid file name: %s\n", in_bmp_filename_without_ext);
        return;
    }

    // .bmp or .bin takes 4 chars
    in_bmp_filename_without_ext[len - 4] = '\0';

    sprintf(out_bmp_path, "output_%s_crop%d.bmp", in_bmp_filename_without_ext, crop_box.crop_number);

    helper_draw_box_of_crop_area_on_bmp(in_bmp_path, out_bmp_path, yolo_result->boxes, yolo_result->box_count, crop_box);

    helper_print_yolo_box_of_crop_area(yolo_result, crop_box);

    if (yolo_result->box_count > 0)
        printf("\noutput bounding boxes on '%s'\n", out_bmp_path);
    else
        printf("\noutput result on '%s'\n", out_bmp_path);
}

void helper_print_yolo_box_of_crop_area_on_bmp_from_bin(kp_yolo_result_t *yolo_result, char *in_bin_path, int in_bin_width, int in_bin_height,
                                                        kp_image_format_t in_bin_format, kp_inf_crop_box_t crop_box)
{
    // output bounding boxes to a new created bmp file
    char out_bmp_path[128];

    // 128 - 17 = 111 -> see sprintf below, output__crop%d.bmp takes up 17 bytes (crop num max is 4),
    // so the length of this array should be length of out_bmp_path - 17 to prevent overflow
    char in_bin_filename_without_ext[111];
    char *in_bin_filename = helper_file_name_from_path(in_bin_path);

    strcpy(in_bin_filename_without_ext, in_bin_filename);

    int len = strlen(in_bin_filename_without_ext);
    if (len <= 4)  {
        printf("ERROR! invalid file name: %s\n", in_bin_filename_without_ext);
        return;
    }

    // .bmp or .bin takes 4 chars
    in_bin_filename_without_ext[len - 4] = '\0';

    sprintf(out_bmp_path, "output_%s_crop%d.bmp", in_bin_filename_without_ext, crop_box.crop_number);

    helper_draw_box_of_crop_area_on_bmp_from_bin(in_bin_path, in_bin_width, in_bin_height, in_bin_format, out_bmp_path, yolo_result->boxes, yolo_result->box_count, crop_box);

    helper_print_yolo_box_of_crop_area(yolo_result, crop_box);

    if (yolo_result->box_count > 0)
        printf("\noutput bounding boxes on '%s'\n", out_bmp_path);
    else
        printf("\noutput result on '%s'\n", out_bmp_path);
}

void helper_string_to_number_array(char *number_string, int *number_group, int *num_devs)
{
    *num_devs = 0;
    char *token;

    /* get the first token */
    token = strtok(number_string, ",");

    /* walk through other tokens */
    while (token != NULL)
    {
        number_group[(*num_devs)++] = atoi(token);
        token = strtok(NULL, ",");
    }
}

char *helper_file_name_from_path(char *path)
{
    if (path == NULL)
        return NULL;

    char *pFileName = path;
    for (char *pCur = path; *pCur != '\0'; pCur++)
    {
        if (*pCur == '/' || *pCur == '\\')
            pFileName = pCur + 1;
    }

    return pFileName;
}

int helper_string_to_crop_box_array(char *crop_box_array, uint32_t *crop_count, kp_inf_crop_box_t *crop_boxes)
{
    char *token;
    uint32_t count = 0;

    /* get the first token */
    token = strtok(crop_box_array, "(,) ");

    /* walk through other tokens */
    while (token != NULL)
    {
        // 4 means there are 4 numbers (stands for x1, y1, width, height)
        switch (count % 4)
        {
        case 0:
            crop_boxes[count / 4].crop_number = count / 4;
            crop_boxes[count / 4].x1 = (uint32_t)atoi(token);
            break;
        case 1:
            crop_boxes[count / 4].y1 = (uint32_t)atoi(token);
            break;
        case 2:
            crop_boxes[count / 4].width = (uint32_t)atoi(token);
            break;
        case 3:
            crop_boxes[count / 4].height = (uint32_t)atoi(token);
            break;
        default:
            break;
        }
        token = strtok(NULL, "(,) ");
        count++;
    }

    if (count % 4 != 0)
        return -1;

    *crop_count = count / 4;
    return 0;
}

char *helper_kp_fixed_point_dtype_to_string(uint32_t fixed_point_dtype)
{
    switch (fixed_point_dtype)
    {
    case KP_FIXED_POINT_DTYPE_INT8:
        return "INT8";
    case KP_FIXED_POINT_DTYPE_INT16:
        return "INT16";
    default:
        return "UNKNOWN";
    }
}

kp_inf_float_node_output_t* helper_fixed_to_floating_node_data(kp_inf_fixed_node_output_t *fixed_node_output)
{
    kp_inf_float_node_output_t *floating_output_nodes = NULL;
    floating_output_nodes = (kp_inf_float_node_output_t *)malloc(sizeof(kp_inf_float_node_output_t) + fixed_node_output->num_data * sizeof(float));

    if (!floating_output_nodes)
    {
        printf("memory is insufficient to allocate buffer for node output\n");
        return NULL;
    }

    floating_output_nodes->channel = fixed_node_output->channel;
    floating_output_nodes->height = fixed_node_output->height;
    floating_output_nodes->width = fixed_node_output->width;
    floating_output_nodes->num_data = fixed_node_output->num_data;

    float ffactor = fixed_node_output->factor;

    if (KP_FIXED_POINT_DTYPE_INT8 == fixed_node_output->fixed_point_dtype) {
        for (int i = 0; i < fixed_node_output->num_data; i++) {
            floating_output_nodes->data[i] = (float)fixed_node_output->data.int8[i] / ffactor;
        }
    } else if (KP_FIXED_POINT_DTYPE_INT16 == fixed_node_output->fixed_point_dtype) {
        for (int i = 0; i < fixed_node_output->num_data; i++) {
            floating_output_nodes->data[i] = (float)fixed_node_output->data.int16[1] / ffactor;
        }
    } else {
        printf("unknown fixed point data type %d ...\n", fixed_node_output->fixed_point_dtype);
        return NULL;
    }

    return floating_output_nodes;
}

// crop_num < 0 means no cropping
static void print_floating_node_data(kp_inf_float_node_output_t *output_nodes[], int num_nodes, int crop_num)
{
    printf("\n");

    if (crop_num >= 0)
        printf("crop box: %d\n", crop_num);

    printf("number of output node : %d\n", num_nodes);

    for (int i = 0; i < num_nodes; i++)
    {
        printf("\n");
        printf("node %d:\n", i);
        printf("width: %d:\n", output_nodes[i]->width);
        printf("height: %d:\n", output_nodes[i]->height);
        printf("channel: %d:\n", output_nodes[i]->channel);
        printf("number of data (float): %d:\n", output_nodes[i]->num_data);

        printf("first 20 data:\n\t");
        for (int j = 0; j < fmin(20, output_nodes[i]->num_data); j++)
        {
            printf("%.3f, ", output_nodes[i]->data[j]);
            if (j > 0 && j % 5 == 0)
                printf("\n\t");
        }

        printf("\n");
    }

    printf("\n");
}

// crop_num < 0 means no cropping
static void dump_floating_node_data_to_files(kp_inf_float_node_output_t *output_nodes[], int num_nodes, int crop_num, char *in_bmp_file)
{
    char in_bmp_filename_without_ext[128];
    char *in_bmp_filename = helper_file_name_from_path(in_bmp_file);

    strcpy(in_bmp_filename_without_ext, in_bmp_filename);

    int len = strlen(in_bmp_filename_without_ext);
    if (len <= 4)  {
        printf("ERROR! invalid file name: %s\n", in_bmp_filename_without_ext);
        return;
    }

    // .bmp or .bin takes 4 chars
    in_bmp_filename_without_ext[len - 4] = '\0';

    for (int i = 0; i < num_nodes; i++)
    {
        char file_name[256];

        if (crop_num >= 0)
            sprintf(file_name, "output_%s_crop%d_node%d_%dx%dx%d.txt", in_bmp_filename_without_ext, crop_num, i, output_nodes[i]->width, output_nodes[i]->height, output_nodes[i]->channel);
        else
            sprintf(file_name, "output_%s_node%d_%dx%dx%d.txt", in_bmp_filename_without_ext, i, output_nodes[i]->width, output_nodes[i]->height, output_nodes[i]->channel);

        FILE *file = fopen(file_name, "w");
        if (file)
        {
            for (int j = 0; j < output_nodes[i]->num_data; j++)
            {
                fprintf(file, "%.3f", output_nodes[i]->data[j]);
                if (j != output_nodes[i]->num_data - 1)
                    fprintf(file, ", ");
            }

            fclose(file);
            printf("dumped node %d output to '%s'\n", i, file_name);
        }
    }
}

// crop_num < 0 means no cropping
static void print_fixed_node_data(kp_inf_fixed_node_output_t *output_nodes[], int num_nodes, int crop_num)
{
    printf("\n");

    if (crop_num >= 0)
        printf("crop box: %d\n", crop_num);

    printf("number of output node : %d\n", num_nodes);

    for (int i = 0; i < num_nodes; i++)
    {
        printf("\n");
        printf("node %d:\n", i);
        printf("width: %d:\n", output_nodes[i]->width);
        printf("height: %d:\n", output_nodes[i]->height);
        printf("channel: %d:\n", output_nodes[i]->channel);
        printf("radix: %d:\n", output_nodes[i]->radix);
        printf("scale: %f:\n", output_nodes[i]->scale);
        printf("factor: %f:\n", output_nodes[i]->factor);
        printf("fixed point dtype: %s:\n", helper_kp_fixed_point_dtype_to_string(output_nodes[i]->fixed_point_dtype));
        printf("number of data (fixed): %d:\n", output_nodes[i]->num_data);

        printf("first 20 data:\n\t");
        if (KP_FIXED_POINT_DTYPE_INT8 == output_nodes[i]->fixed_point_dtype) {
            for (int j = 0; j < fmin(20, output_nodes[i]->num_data); j++)
            {
                printf("%d, ", output_nodes[i]->data.int8[j]);
                if (j > 0 && j % 5 == 0)
                    printf("\n\t");
            }
        } else if (KP_FIXED_POINT_DTYPE_INT16 == output_nodes[i]->fixed_point_dtype) {
            for (int j = 0; j < fmin(20, output_nodes[i]->num_data); j++)
            {
                printf("%d, ", output_nodes[i]->data.int16[j]);
                if (j > 0 && j % 5 == 0)
                    printf("\n\t");
            }
        } else {
            printf("unknown fixed point data type %d ...\n", output_nodes[i]->fixed_point_dtype);
        }

        printf("\n");
    }

    printf("\n");
}

// crop_num < 0 means no cropping
static void dump_fixed_node_data_to_files(kp_inf_fixed_node_output_t *output_nodes[], int num_nodes, int crop_num, char *in_bmp_file)
{
    char in_bmp_filename_without_ext[128];
    char *in_bmp_filename = helper_file_name_from_path(in_bmp_file);

    strcpy(in_bmp_filename_without_ext, in_bmp_filename);

    int len = strlen(in_bmp_filename_without_ext);
    if (len <= 4)  {
        printf("ERROR! invalid file name: %s\n", in_bmp_filename_without_ext);
        return;
    }

    // .bmp or .bin takes 4 chars
    in_bmp_filename_without_ext[len - 4] = '\0';

    for (int i = 0; i < num_nodes; i++)
    {
        char file_name[256];

        if (crop_num >= 0)
            sprintf(file_name, "output_%s_crop%d_node%d_%dx%dx%d.txt", in_bmp_filename_without_ext, crop_num, i, output_nodes[i]->width, output_nodes[i]->height, output_nodes[i]->channel);
        else
            sprintf(file_name, "output_%s_node%d_%dx%dx%d.txt", in_bmp_filename_without_ext, i, output_nodes[i]->width, output_nodes[i]->height, output_nodes[i]->channel);

        FILE *file = fopen(file_name, "w");
        if (file)
        {
            if (KP_FIXED_POINT_DTYPE_INT8 == output_nodes[i]->fixed_point_dtype) {
                for (int j = 0; j < output_nodes[i]->num_data; j++)
                {
                    fprintf(file, "%d", output_nodes[i]->data.int8[j]);
                    if (j != output_nodes[i]->num_data - 1)
                        fprintf(file, ", ");
                }
            } else if (KP_FIXED_POINT_DTYPE_INT16 == output_nodes[i]->fixed_point_dtype) {
                for (int j = 0; j < output_nodes[i]->num_data; j++)
                {
                    fprintf(file, "%d", output_nodes[i]->data.int16[j]);
                    if (j != output_nodes[i]->num_data - 1)
                        fprintf(file, ", ");
                }
            } else {
                printf("unknown fixed point data type %d ...\n", output_nodes[i]->fixed_point_dtype);
            }

            fclose(file);
            printf("dumped node %d output to '%s'\n", i, file_name);
        }
    }
}

void helper_dump_floating_node_data_to_files(kp_inf_float_node_output_t *output_nodes[], int num_nodes, char *in_img_path)
{
    print_floating_node_data(output_nodes, num_nodes, -1);                      // -1 means no cropping
    dump_floating_node_data_to_files(output_nodes, num_nodes, -1, in_img_path); // -1 means no cropping
}

void helper_dump_floating_node_data_of_crop_area_to_files(kp_inf_float_node_output_t *output_nodes[], int num_nodes, int crop_num, char *in_img_path)
{
    print_floating_node_data(output_nodes, num_nodes, crop_num);
    dump_floating_node_data_to_files(output_nodes, num_nodes, crop_num, in_img_path);
}

void helper_dump_fixed_node_data_to_files(kp_inf_fixed_node_output_t *output_nodes[], int num_nodes, char *in_img_path)
{
    print_fixed_node_data(output_nodes, num_nodes, -1);                      // -1 means no cropping
    dump_fixed_node_data_to_files(output_nodes, num_nodes, -1, in_img_path); // -1 means no cropping
}

void helper_dump_fixed_node_data_of_crop_area_to_files(kp_inf_fixed_node_output_t *output_nodes[], int num_nodes, int crop_num, char *in_img_path)
{
    print_fixed_node_data(output_nodes, num_nodes, crop_num);
    dump_fixed_node_data_to_files(output_nodes, num_nodes, crop_num, in_img_path);
}

char *helper_kp_model_tensor_data_layout_to_string(uint32_t tensor_data_layout)
{
    switch (tensor_data_layout)
    {
    case KP_MODEL_TENSOR_DATA_LAYOUT_16W1C8B:
        return "16W1C8B";
    case KP_MODEL_TENSOR_DATA_LAYOUT_4W4C8B:
        return "4W4C8B";
    case KP_MODEL_TENSOR_DATA_LAYOUT_1W16C8B:
        return "1W16C8B";
    case KP_MODEL_TENSOR_DATA_LAYOUT_8W1C16B:
        return "8W1C16B";
    default:
        return "UNKNOWN";
    }
}

char *helper_kp_model_target_chip_to_string(uint32_t target_chip)
{
    switch (target_chip)
    {
    case KP_MODEL_TARGET_CHIP_KL520:
        return "KL520";
    case KP_MODEL_TARGET_CHIP_KL720:
        return "KL720";
    case KP_MODEL_TARGET_CHIP_KL530:
        return "KL530";
    case KP_MODEL_TARGET_CHIP_KL730:
        return "KL730";
    case KP_MODEL_TARGET_CHIP_KL630:
        return "KL630";
    case KP_MODEL_TARGET_CHIP_KL540:
        return "KL540";
    default:
        return "UNKNOWN";
    }
}

void helper_print_model_info(kp_model_nef_descriptor_t *pModel_desc)
{
    printf("\n");
    printf("this NEF contains %d model(s):\n", pModel_desc->num_models);
    for (int i = 0; i < pModel_desc->num_models; i++)
    {
        printf("[%d] model ID = %d\n", i + 1, pModel_desc->models[i].id);
        printf("    model raw output size = %d\n", pModel_desc->models[i].max_raw_out_size);

        printf("    model input:\n");
        for (int node_idx = 0; node_idx < pModel_desc->models[i].input_nodes_num; node_idx++) {
            kp_tensor_descriptor_t *tensor_desc = &(pModel_desc->models[i].input_nodes[node_idx]);

            printf("        [%d] input name = %s\n", node_idx, tensor_desc->name);
            printf("        [%d] input shape = [", node_idx);
            for (int dim_idx = 0; dim_idx < tensor_desc->shape_npu_len; dim_idx++) {
                printf("%u", tensor_desc->shape_npu[dim_idx]);

                if (dim_idx == (tensor_desc->shape_npu_len - 1)) {
                    printf("]\n");
                } else {
                    printf(", ");
                }
            }

            printf("        [%d] data layout format = %s\n", node_idx, helper_kp_model_tensor_data_layout_to_string(tensor_desc->data_layout));
            printf("        [%d] quantization parameters [radix, scale] = [%d, %f]\n",
                    node_idx,
                    tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].radix,
                    tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].scale);
        }

        printf("    model output:\n");
        for (int node_idx = 0; node_idx < pModel_desc->models[i].output_nodes_num; node_idx++) {
            kp_tensor_descriptor_t *tensor_desc = &(pModel_desc->models[i].output_nodes[node_idx]);

            printf("        [%d] output name = %s\n", node_idx, tensor_desc->name);
            printf("        [%d] output shape = [", node_idx);
            for (int dim_idx = 0; dim_idx < tensor_desc->shape_npu_len; dim_idx++) {
                printf("%u", tensor_desc->shape_npu[dim_idx]);

                if (dim_idx == (tensor_desc->shape_npu_len - 1)) {
                    printf("]\n");
                } else {
                    printf(", ");
                }
            }

            printf("        [%d] data layout format = %s\n", node_idx, helper_kp_model_tensor_data_layout_to_string(tensor_desc->data_layout));
            printf("        [%d] quantization parameters [radix, scale] = [%d, %f]\n",
                    node_idx,
                    tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].radix,
                    tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].scale);
        }

        printf("\n");
    }
}

void helper_get_device_usb_speed_by_port_id(kp_devices_list_t *devices_list, int port_id, int *link_speed)
{
    kp_device_descriptor_t *dev_descp;

    if (((0 < devices_list->num_dev)) && (0 == port_id))
    {
        dev_descp = &(devices_list->device[0]);
        *link_speed = dev_descp->link_speed;
        return;
    }
    else
    {
        for (int i = 0; i < devices_list->num_dev; i++)
        {
            dev_descp = &(devices_list->device[i]);

            if (port_id == dev_descp->port_id)
            {
                *link_speed = dev_descp->link_speed;
                return;
            }
        }

        printf("\033[91m[error] no kneron device found.\033[0m\n");
        *link_speed = KP_USB_SPEED_UNKNOWN;
    }
}

void helper_bounding_box_stabilization(uint32_t box_count_last, kp_bounding_box_t *boxes_last,
                                       uint32_t box_count_latest, kp_bounding_box_t *boxes_latest,
                                       uint32_t *box_count_stabilized, kp_bounding_box_t *boxes_stabilized,
                                       int pixel_offset, float box_score_threshold)
{
    bool box_used[YOLO_GOOD_BOX_MAX] = {false};

    uint32_t count = 0;

    for (uint32_t i = 0; i < box_count_latest; i++)
    {
        kp_bounding_box_t *box = &boxes_latest[i];

        for (uint32_t k = 0; k < box_count_last; k++)
        {
            if (box_used[k])
                continue;

            kp_bounding_box_t *obox = &boxes_last[k];

            if (box->class_num == obox->class_num &&
                abs((int)(box->x1 - obox->x1)) < pixel_offset &&
                abs((int)(box->x2 - obox->x2)) < pixel_offset &&
                abs((int)(box->y1 - obox->y1)) < pixel_offset &&
                abs((int)(box->y2 - obox->y2)) < pixel_offset)
            {
                box->x1 = obox->x1;
                box->x2 = obox->x2;
                box->y1 = obox->y1;
                box->y2 = obox->y2;
                box->score = fmax(box->score, obox->score);
                box_used[k] = true;
            }
        }

        if (box->score < box_score_threshold)
            continue;

        memcpy((void *)&boxes_stabilized[count], (void *)box, sizeof(kp_bounding_box_t));
        count++;
    }

    *box_count_stabilized = count;
}
