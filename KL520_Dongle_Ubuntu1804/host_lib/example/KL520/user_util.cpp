/**
 * @file      user_util.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2019-07-31
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"
#include "user_util.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdarg.h>     /*for valist */
#include <iostream>     /*for std*/

#if defined (__cplusplus) || defined (c_plusplus)
extern "C"{
#endif

extern char* p_ci_logfile_name_g;
volatile uint32_t term_sig = 0;

const char* ci_prepare_logfile_name(int argc, char*argv[])
{
#ifdef KNERON_BUILD_TEST
    int i;
    static std::string filename = argv[0];
    //remove path
    filename = filename.substr(filename.find_last_of("/\\") + 1);
    //remove extention
    std::string::size_type const p(filename.find_last_of('.'));
    filename = filename.substr(0, p);

    for(i=1;i<argc;i++){
        filename += "_";
        filename += argv[i];
    }
    filename += ".log";
    return filename.c_str();
#else
    return NULL;
#endif
}

bool print2log(const char* fmt, ...)
{
#ifdef KNERON_BUILD_TEST
    static FILE* fp = NULL;
    if (NULL == fp) 
        fp = fopen(p_ci_logfile_name_g, "w");
    else
        fp = fopen(p_ci_logfile_name_g, "a");

    if (fp == NULL){
        printf("[error] fail to open file: %s\n", p_ci_logfile_name_g);
        return false;
    }

    char buffer[1024]; 
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    fprintf(fp, "%s", buffer);

    fclose(fp);
#endif
    return true;
}

bool check_ctl_break(void)
{

    return  (1 == term_sig);

}

static void sig_term_handler(int signo)
{
    if (0 != ((HOSTLIB_SIG) & signo)) {
        term_sig = 1;
    }
}

int register_hostlib_signal(void) 
{
    if (signal(SIGINT, sig_term_handler) == SIG_IGN)
        signal(SIGINT, SIG_IGN);

    if (signal(SIGTERM, sig_term_handler) == SIG_IGN)
        signal(SIGTERM, SIG_IGN);
    return 0;
}

int write_buf_to_file(char* buf, const char* fn, uint32_t size) {
    if ((NULL == buf) || (NULL == fn)) {
        printf("file name or buffer is null!\n");
        return -1;
    }

    FILE* out = fopen(fn, "wb");
    if (NULL == out) {
        printf("opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }
 
    fwrite (buf , sizeof(char), size, out);
    fclose(out);
	
    return 0;
}

int read_file_to_buf_2(char** buf, const char* fn) {
    if (NULL == fn) {
        printf("file name is null!\n");
        return -1;
    }

    FILE* in = fopen(fn, "rb");
    if (!in) {
        printf("opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }

    fseek(in, 0, SEEK_END);
    long size = ftell(in);  //get the size
 
    *buf = (char *) malloc(size);
    memset(*buf, 0, size);
    fseek(in, 0, SEEK_SET);  //move to begining

    int res = 0;
    while (1) {
        int len = fread(*buf + res, 1, 1024, in);
        if (len == 0) {
            if (!feof(in)) {
                printf("reading from img file failed:%s.!\n", fn);
                fclose(in);
                return -1;
            }
            break;
        }
        res += len;  //calc size
    }
    fclose(in);
    return res;
}


int read_file_to_buf(char* buf, const char* fn, int nlen) {
    if (buf == NULL || fn == NULL) {
        printf("file name or buffer is null!\n");
        return -1;
    }

    FILE* in = fopen(fn, "rb");
    if (!in) {
        printf("opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }

    fseek(in, 0, SEEK_END);
    long f_n = ftell(in);  //get the size
    if (f_n > nlen) {
        printf("buffer is not enough.!\n");
        fclose(in);
        return -1;
    }

    fseek(in, 0, SEEK_SET);  //move to begining

    int res = 0;
    while (1) {
        int len = fread(buf + res, 1, 1024, in);
        if (len == 0) {
            if (!feof(in)) {
                printf("reading from img file failed:%s.!\n", fn);
                fclose(in);
                return -1;
            }
            break;
        }
        res += len;  //calc size
    }
    fclose(in);
    return res;
}

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

double what_time_is_it_now() {
    struct timeval time;
    if (gettimeofday(&time, NULL)) {
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

struct kdp_dme_cfg_s create_dme_cfg_struct() {
    struct kdp_dme_cfg_s dme_cfg;
    memset(&dme_cfg, 0, sizeof(kdp_dme_cfg_s));
    return dme_cfg;
}

struct kdp_isi_cfg_s create_isi_cfg_struct() {
    struct kdp_isi_cfg_s isi_cfg;
    memset(&isi_cfg, 0, sizeof(kdp_isi_cfg_s));
    return isi_cfg;
}

struct kdp_metadata_s create_metadata_struct() {
    struct kdp_metadata_s metadata;
    memset(&metadata, 0, sizeof(kdp_metadata_s));
    return metadata;
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
	unsigned int size;			   /* Info Header size in bytes */
	int width, height;			   /* Width and height of image */
	unsigned short int planes;	   /* Number of colour planes */
	unsigned short int bits;	   /* Bits per pixel */
	unsigned int compression;	   /* Compression type */
	unsigned int imagesize;		   /* Image size in bytes */
	int xresolution, yresolution;  /* Pixels per meter */
	unsigned int ncolours;		   /* Number of colours */
	unsigned int importantcolours; /* Important colours */
} INFOHEADER;
#pragma pack(pop)

char *bmp_to_rgb565_auto_malloc(const char *file_path, int *width, int *height)
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

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif
