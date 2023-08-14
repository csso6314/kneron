/**
 * @file      jpeg_decoding.cpp
 * @brief     kdp host lib user test examples
 * @version   0.1 - 2020-09-25
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include "errno.h"
#include "kdp_host.h"
#include "stdio.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include "user_util.h"
#include <sys/time.h>

#include "kapp_id.h"
#include "model_res.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

#define JPEG_DEC_APP_ID APP_JPEG_DEC

#define TEST_JPEG_DEC_IN_DIR "../../input_images/jpeg_test_stuff/djpeg/"
#define TEST_JPEG_DEC_OUT_DIR "../../input_images/jpeg_test_stuff/out/"

//#define TEST_JPEG_DEC_IN_DIR "../../../Zhong/kneron/AI/Beethoven/khost/test_images/jpeg_test_stuff/djpeg/"
//#define TEST_JPEG_DEC_OUT_DIR "../../../Zhong/kneron/AI/Beethoven/khost/test_images/jpeg_test_stuff/out/"

enum
{
    JPEG_FMT_JCS_UNKNOWN,       /* error/unspecified */
    JPEG_FMT_JCS_GRAYSCALE,     /* monochrome */
    JPEG_FMT_JCS_RGB,       /* red/green/blue */
    JPEG_FMT_JCS_BGR565,
    JPEG_FMT_JCS_RGB565,
    JPEG_FMT_JCS_YCbCr,     /* Y/Cb/Cr (also known as YUV) */
    JPEG_FMT_JCS_YUYV,      /* Y/U/Y/V */
    JPEG_FMT_JCS_CMYK,      /* C/M/Y/K */
    JPEG_FMT_JCS_YCCK,       /* Y/Cb/Cr/K */

    JPEG_YUV_420 = JPEG_FMT_JCS_YCbCr,
    JPEG_YUYV = JPEG_FMT_JCS_YUYV,
    JPEG_YUV_422,
    JPEG_YUV_444,

};


// test jpeg inputs: stardard HD 720p
#define HD_IN_JPEG_WIDTH  1280
#define HD_IN_JPEG_HEIGHT 720
#define HD_JPEG_DEC_IMAGE_IN_FILE1  (TEST_JPEG_DEC_IN_DIR "chessboards.jpg")
#define HD_JPEG_DEC_IMAGE_OUT_FILE1 (TEST_JPEG_DEC_OUT_DIR "chessboards.yuv")
#define HD_JPEG_DEC_IMAGE_IN_FILE2  (TEST_JPEG_DEC_IN_DIR "car_1280x720.jpg")
#define HD_JPEG_DEC_IMAGE_OUT_FILE2 (TEST_JPEG_DEC_OUT_DIR "car_1280x720.yuv")
#define HD_JPEG_DEC_IMAGE_IN_FILE3  (TEST_JPEG_DEC_IN_DIR "cat_1280x720.jpg")
#define HD_JPEG_DEC_IMAGE_OUT_FILE3 (TEST_JPEG_DEC_OUT_DIR "cat_1280x720.yuv")
#define HD_JPEG_DEC_IMAGE_IN_FILE4  (TEST_JPEG_DEC_IN_DIR "pokemon_1280x720.jpg")
#define HD_JPEG_DEC_IMAGE_OUT_FILE4 (TEST_JPEG_DEC_OUT_DIR "pokemon_1280x720.yuv")

static char * hd_jpeg_in_files[4];
static char * hd_yuv_out_files[4];

//static char * fhd_jpeg_in_files[] = {FHD_JPEG_ENC_IMAGE_IN_FILE1, FHD_JPEG_ENC_IMAGE_IN_FILE2, FHD_JPEG_ENC_IMAGE_IN_FILE3};
//static char * fhd_jpeg_out_files[] = {FHD_JPEG_ENC_IMAGE_OUT_FILE1, FHD_JPEG_ENC_IMAGE_OUT_FILE2, FHD_JPEG_ENC_IMAGE_OUT_FILE3};

#define PIXEL_LEN_RGB565   2

/* max resoutions supported */
#define FHD_IN_YUV_WIDTH  1920
#define FHD_IN_YUV_HEIGHT 1080

#define MAX_YUV_IN_SIZE   (FHD_IN_YUV_WIDTH*PIXEL_LEN_RGB565)


static int read_file_to_buf(char* buf, const char* fn, int nlen)
{
    if (buf == NULL || fn == NULL)
    {
        printf("file name or buffer is null!\n");
        return -1;
    }

    FILE* in = fopen(fn, "rb");
    if (!in)
    {
        printf("opening file %s failed, error:%s.!\n", fn, strerror(errno));
        return -1;
    }

    fseek(in, 0, SEEK_END);
    long f_n = ftell(in);  //get the size
    if (f_n > nlen)
    {
        printf("buffer is not enough.!\n");
        fclose(in);
        return -1;
    }

    fseek(in, 0, SEEK_SET);  //move to begining

    int res = 0;
    while (1)
    {
        int len = fread(buf + res, 1, 1024, in);
        if (len == 0)
        {
            if (!feof(in))
            {
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


static int get_file_size(const char *file_path)
{
    unsigned int file_size = 0;

    FILE* filep = fopen(file_path, "rb");
    fseek(filep, 0, SEEK_END);
    file_size = ftell(filep);
    fclose(filep);

    return file_size;
}

static double what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time, NULL))
    {
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

static int do_config(int dev_idx, int img_seq, int width, int height, int fmt, int len)
{
    int ret;

    ret = kdp_jpeg_dec_config(dev_idx, img_seq, width, height, fmt, len);
    if (ret)
    {
        printf("Jpeg dec config failed : %d\n", ret);
        return -1;
    }

    return 0;
}


static int do_start(int dev_idx, int img_seq, uint8_t *in_img_buf, uint32_t in_img_len, uint32_t *out_img_buf, uint32_t *out_img_len)
{
    int ret;

    ret = kdp_jpeg_dec(dev_idx, img_seq, in_img_buf, in_img_len, out_img_buf, out_img_len);
    if (ret)
    {
        printf("Jpeg Decoding failed : %d\n", ret);
        return -1;
    }
    else
    {
        printf("Jpeg decoding output: addr(SCPU)=0x%08x, size=%d\n", *out_img_buf, *out_img_len);
    }

    return 0;
}

static int do_get_result(int dev_idx, uint32_t img_seq, uint32_t *rsp_code, uint32_t *r_size, char *r_data, char *fileName)
{
    int ret;
    FILE *pf;

    memset(r_data, 0, *r_size);

    ret = kdp_jpeg_dec_retrieve_res(dev_idx, img_seq, rsp_code, r_size, r_data);

    if (ret)
    {
        printf("Jpeg dec get [%d] result failed : %d\n", img_seq, ret);
        return -1;
    }

    if (*rsp_code != 1)
    {
        printf("Jpeg dec get [%d] result error_code: [%d] [%d]\n", img_seq, *rsp_code, *r_size);
        return -1;
    }

    if ((*r_size >= sizeof(uint32_t)) && (fileName != NULL))
    {
        pf = fopen(fileName, "wb");

        if(pf == NULL)
        {
            printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, fileName);
            return -1;
        }

        fwrite(r_data, *r_size, 1, pf);
        fclose(pf);
    }
    else if(*r_size < sizeof(uint32_t))
    {
        printf("Img [%d] : result_size %d too small\n", img_seq, *r_size);
        return -1;
    }

    return 0;
}

int user_test_jpeg_dec(int dev_idx,  int test_loop, int bSaveToFile)
{
    int ret = 0;
    uint32_t result_code, yuv_scpu_addr;
    int i, j, loop_cnt, fmt, len;
    uint32_t  yuv_out_len;
    char *p_jpeg_in, *p_yuv_out;
    double start_time, end_time;
    double elapsed_time, avg_elapsed_time, avg_fps;
    int total_loop;
    char *pFileName;
    int total_tested_frames;

    total_tested_frames = 0;

    start_time = what_time_is_it_now();

    fmt = JPEG_YUV_420;

    loop_cnt = sizeof(hd_jpeg_in_files)/ sizeof (char *);
    total_loop = test_loop * loop_cnt;

    for(j=0; j< test_loop; j++)
    {
        for(i=0; i<loop_cnt; i++)
        {
            int in_fsize = get_file_size(hd_jpeg_in_files[i]);
            if(in_fsize < 0 ) {
                printf("file: %s does not exist\n", hd_jpeg_in_files[i]);
            }

            ret = do_config(dev_idx, i, HD_IN_JPEG_WIDTH, HD_IN_JPEG_HEIGHT, fmt, in_fsize);

            if(ret != 0)
                goto test_end;

            p_jpeg_in = new char[in_fsize];
            memset(p_jpeg_in, 0, in_fsize);
            len = read_file_to_buf((char *)p_jpeg_in, hd_jpeg_in_files[i], in_fsize);
            if (len <= 0)
            {
                printf("failed to read file: %s \n", hd_jpeg_in_files[i]);
                delete[] p_jpeg_in;
                return -1;
            }

            ret = do_start(dev_idx, i, (uint8_t *)p_jpeg_in, len, &yuv_scpu_addr, &yuv_out_len);
            if(ret != 0)
                goto test_end;

            p_yuv_out = new char[yuv_out_len];
            memset(p_yuv_out, 0, yuv_out_len);
            if(bSaveToFile)
                pFileName = hd_yuv_out_files[i];
            else
                pFileName = NULL;

            ret = do_get_result(dev_idx, i, &result_code, &yuv_out_len, p_yuv_out, pFileName);
            if(ret != 0)
                goto test_end;
            else
                total_tested_frames++;

        }  // i
    }   //j


    end_time = what_time_is_it_now();

    elapsed_time = (end_time - start_time) * 1000;
    avg_elapsed_time = elapsed_time / total_loop;
    avg_fps = 1000.0f / avg_elapsed_time;

    printf("\n=> Avg %.2f FPS (%.2f ms = %.2f/%d)\n\n",
           avg_fps, avg_elapsed_time, elapsed_time, test_loop);
    if(bSaveToFile)
        printf("Jpeg enc outputs are here: %s\n\n", TEST_JPEG_DEC_OUT_DIR);

test_end:
    printf("tested frames: %d\n", total_tested_frames);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("\n");
        printf("usage: ./%s   [loop times] [save result to file]\n", argv[0]);
        printf("[loop times] number of loops to run through all images\n");
        printf("[save result to file] 0: does not save to file, run full speed; 1: save to file for verification\n");
        return -1;
    }

    int loops = atoi(argv[1]);
    int bSaveRst = atoi(argv[2]);

    for(int i=0; i<4; i++) {
        hd_jpeg_in_files[i] = new char[128];
        memset(hd_jpeg_in_files[i], 0, 128);
        hd_yuv_out_files[i] = new char[128];
        memset(hd_yuv_out_files[i], 0,128);
    }

    memcpy(hd_jpeg_in_files[0], HD_JPEG_DEC_IMAGE_IN_FILE1, strlen(HD_JPEG_DEC_IMAGE_IN_FILE1));
    memcpy(hd_jpeg_in_files[1], HD_JPEG_DEC_IMAGE_IN_FILE2, strlen(HD_JPEG_DEC_IMAGE_IN_FILE2));
    memcpy(hd_jpeg_in_files[2], HD_JPEG_DEC_IMAGE_IN_FILE3, strlen(HD_JPEG_DEC_IMAGE_IN_FILE3));
    memcpy(hd_jpeg_in_files[3], HD_JPEG_DEC_IMAGE_IN_FILE4, strlen(HD_JPEG_DEC_IMAGE_IN_FILE4));

    memcpy(hd_yuv_out_files[0], HD_JPEG_DEC_IMAGE_OUT_FILE1, strlen(HD_JPEG_DEC_IMAGE_OUT_FILE1));
    memcpy(hd_yuv_out_files[1], HD_JPEG_DEC_IMAGE_OUT_FILE2, strlen(HD_JPEG_DEC_IMAGE_OUT_FILE2));
    memcpy(hd_yuv_out_files[2], HD_JPEG_DEC_IMAGE_OUT_FILE3, strlen(HD_JPEG_DEC_IMAGE_OUT_FILE3));
    memcpy(hd_yuv_out_files[3], HD_JPEG_DEC_IMAGE_OUT_FILE4, strlen(HD_JPEG_DEC_IMAGE_OUT_FILE4));

    printf("init kdp host lib log....\n");

    if (kdp_lib_init() < 0)
    {
        printf("init for kdp host lib failed.\n");
        return -1;
    }

    printf("adding devices....\n");
    int dev_idx = kdp_connect_usb_device(1);
    if (dev_idx < 0)
    {
        printf("add device failed.\n");
        return -1;
    }

    printf("start kdp host lib....\n");
    if (kdp_lib_start() < 0)
    {
        printf("start kdp host lib failed.\n");
        return -1;
    }

    user_test_jpeg_dec(dev_idx,  loops, bSaveRst);

    return 0;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
