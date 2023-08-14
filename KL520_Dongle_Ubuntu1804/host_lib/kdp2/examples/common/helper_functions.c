#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#include "kdp2_core.h"
#include "helper_functions.h"

char *read_file_to_buffer_auto_malloc(const char *file_path, long *buffer_size)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        printf("%s(): fopen failed, file:%s, %s\n", __FUNCTION__, file_path, strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file); //get the size

    *buffer_size = file_size;

    char *buffer = (char *)malloc(file_size);

    fseek(file, 0, SEEK_SET); //move to begining

    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != (size_t)file_size)
    {
        printf("%s(): fread failed, file size: %u, read size %u\n", __FUNCTION__,
               (unsigned int)file_size, (unsigned int)read_size);
        free(buffer);
        buffer = NULL;
        *buffer_size = 0;
    }

    fclose(file);

    return buffer;
}

static struct timeval time_begin;
static struct timeval time_end;

void measure_time_begin()
{
    gettimeofday(&time_begin, NULL);
}

void measure_time_end(double *measued_time)
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

char *bmp_file_to_rgb565_buffer_auto_malloc(const char *file_path, int *width, int *height)
{
    FILEHEADER header1;
    INFOHEADER header2;

    unsigned short *rgb_buf = NULL;

    FILE *bmpf = fopen(file_path, "rb");

    if (!bmpf)
    {
        printf("file read failed\n");
        return NULL;
    }

    fread(&header1, 1, sizeof(header1), bmpf);
    fread(&header2, 1, sizeof(header2), bmpf);

    *width = header2.width;
    *height = header2.height;

    int bmp_buf_size = header2.width * header2.height * (header2.bits / 8);
    unsigned char *bmp_buf = (unsigned char *)malloc(bmp_buf_size);

    fread(bmp_buf, 1, bmp_buf_size, bmpf);

    fclose(bmpf);

    if (header2.bits == 24)
    {
        rgb_buf = (unsigned short *)malloc(header2.width * header2.height * 2);
        int rgb565_pos = 0;

        for (int row = header2.height - 1; row >= 0; row--)
        {
            int bmp_pos = header2.width * row * 3;
            for (int col = 0; col < header2.width; col++)
            {
                unsigned char blue = bmp_buf[bmp_pos];
                unsigned char green = bmp_buf[bmp_pos + 1];
                unsigned char red = bmp_buf[bmp_pos + 2];

                rgb_buf[rgb565_pos] = ((red & 0b11111000) << 8) | ((green & 0b11111100) << 3) | (blue >> 3);

                bmp_pos += 3;
                rgb565_pos++;
            }
        }
    }
    else
    {
        printf("support only 24 bit bmp\n");
    }

    free(bmp_buf);

    return (char *)rgb_buf;
}

char *bmp_file_to_rgba8888_buffer_auto_malloc(const char *file_path, int *width, int *height)
{
    FILEHEADER header1;
    INFOHEADER header2;

    unsigned char *rgba_buf = NULL;

    FILE *bmpf = fopen(file_path, "rb");

    if (!bmpf)
    {
        printf("file read failed\n");
        return NULL;
    }

    fread(&header1, 1, sizeof(header1), bmpf);
    fread(&header2, 1, sizeof(header2), bmpf);

    *width = header2.width;
    *height = header2.height;

    int bmp_buf_size = header2.width * header2.height * (header2.bits / 8);
    unsigned char *bmp_buf = (unsigned char *)malloc(bmp_buf_size);

    fread(bmp_buf, 1, bmp_buf_size, bmpf);

    fclose(bmpf);

    if (header2.bits == 24)
    {
        rgba_buf = (unsigned char *)malloc(header2.width * header2.height * 4);
        int rgba8888_pos = 0;

        for (int row = header2.height - 1; row >= 0; row--)
        {
            int bmp_pos = header2.width * row * 3;
            for (int col = 0; col < header2.width; col++)
            {
                unsigned char blue = bmp_buf[bmp_pos];
                unsigned char green = bmp_buf[bmp_pos + 1];
                unsigned char red = bmp_buf[bmp_pos + 2];

                rgba_buf[rgba8888_pos++] = red;
                rgba_buf[rgba8888_pos++] = green;
                rgba_buf[rgba8888_pos++] = blue;
                rgba_buf[rgba8888_pos++] = 0;

                bmp_pos += 3;
            }
        }
    }
    else
    {
        printf("support only 24 bit bmp\n");
    }

    free(bmp_buf);

    return (char *)rgba_buf;
}

void draw_bounding_box_to_bmp_file(const char *in_file_path, const char *out_file_path, kdp2_bounding_box_t boxes[], int box_count)
{
    FILEHEADER header1;
    INFOHEADER header2;

    FILE *bmpf = fopen(in_file_path, "rb");

    if (!bmpf)
    {
        printf("file read failed\n");
        return;
    }

    fread(&header1, 1, sizeof(header1), bmpf);
    fread(&header2, 1, sizeof(header2), bmpf);

    int width = header2.width;
    int height = header2.height;

    int bmp_buf_size = width * height * (header2.bits / 8);
    unsigned char *bmp_buf = (unsigned char *)malloc(bmp_buf_size);

    fread(bmp_buf, 1, bmp_buf_size, bmpf);

    fclose(bmpf);

    if (header2.bits == 24)
    {
        for (int i = 0; i < box_count; i++)
        {
            int bmp_x1 = (int)boxes[i].x1;
            int bmp_y1 = (int)(height - boxes[i].y1);
            int bmp_x2 = (int)boxes[i].x2;
            int bmp_y2 = (int)(height - boxes[i].y2);

            int pos;
            int diff_x = bmp_x2 - bmp_x1;
            int diff_y = bmp_y1 - bmp_y2;

            // (x1,y1) -> (x2,y1)
            pos = 3 * (width * bmp_y1 + bmp_x1);
            for (int j = 0; j < diff_x; j++)
            {
                bmp_buf[pos] = 0;
                bmp_buf[pos + 1] = 0;
                bmp_buf[pos + 2] = 255;
                pos += 3;
            }

            // (x1,y2) -> (x2,y2)
            pos = 3 * (width * bmp_y2 + bmp_x1);
            for (int j = 0; j < diff_x; j++)
            {
                bmp_buf[pos] = 0;
                bmp_buf[pos + 1] = 0;
                bmp_buf[pos + 2] = 255;
                pos += 3;
            }

            // (x1,y1) -> (x1,y2)
            pos = 3 * (width * bmp_y2 + bmp_x1);
            for (int j = 0; j < diff_y; j++)
            {
                bmp_buf[pos] = 0;
                bmp_buf[pos + 1] = 0;
                bmp_buf[pos + 2] = 255;
                pos += 3 * width;
            }

            // (x2,y1) -> (x2,y2)
            pos = 3 * (width * bmp_y2 + bmp_x2);
            for (int j = 0; j < diff_y; j++)
            {
                bmp_buf[pos] = 0;
                bmp_buf[pos + 1] = 0;
                bmp_buf[pos + 2] = 255;
                pos += 3 * width;
            }
        }
    }
    else
    {
        printf("support only 24 bit bmp\n");
    }

    FILE *bmpf_out = fopen(out_file_path, "wb");

    fwrite(&header1, 1, sizeof(header1), bmpf_out);
    fwrite(&header2, 1, sizeof(header2), bmpf_out);
    fwrite(bmp_buf, 1, bmp_buf_size, bmpf_out);
    fclose(bmpf_out);

    free(bmp_buf);

    return;
}