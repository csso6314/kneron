/**
 * @file      kl520_dme_yolo_multiple_dongles.cpp
 * @brief     kdp host lib user test examples 
 * @version   0.1 - 2020-12-21
 * @copyright (c) 2020 Kneron Inc. All right reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "user_util.h"
#include "kdp_host.h"

#define MAX_DEV_NUM 10               // changable
#define RESULT_BUF_SIZE (256 * 1024) // if result size is bigger than this, change it

#define IMAGE_FILE ("../../input_images/street_cars_bike.bmp")
#define DME_MODEL_FILE_TYv3 ("../../input_models/KL520/tiny_yolo_v3/models_520.nef")

int gDev_idxs[MAX_DEV_NUM];      // device handle index
int gNum_devices = 0;            // number of devices connected
bool gTest_done = false;         // test done flag
char *gImage_buf = NULL;         // image buffer
int gImage_width, gImage_height; // image width and height
uint32_t gModel_id = 0;          // inference model id
int gTest_loops = 100;           // loops for inference

void *thread_start_inference(void *arg)
{
    int dev_idx = *((int *)arg);

    uint32_t inf_size = 0;
    bool res_flag = true;
    char *inf_res = (char *)malloc(RESULT_BUF_SIZE);

    int image_buf_size = gImage_width * gImage_height * 2; //RGB565

    for (int c = 0; c < gTest_loops; c++)
    {
        int ret = kdp_dme_inference(dev_idx, gImage_buf, image_buf_size, &inf_size, &res_flag, inf_res, 0, gModel_id);
        if (ret != 0)
        {
            printf("device_idx %d failed at kdp_dme_inference()\n", dev_idx);
            break;
        }

        kdp_dme_retrieve_res(dev_idx, 0, inf_size, inf_res);
        printf(".");
        fflush(stdout);
    }

    // return inference result stuff
    return inf_res;
}

void exit_example()
{
    printf("\nkdp_lib_de_init()...\n");
    kdp_lib_de_init();

    printf("\nexample result ... %s\n", gTest_done ? "OK" : "failed");
    exit(1); // exit program
}

void dme_inference_on_multiple_devices()
{
    printf("\nread image file from '%s'\n", IMAGE_FILE);

    gImage_buf = bmp_to_rgb565_auto_malloc(IMAGE_FILE, &gImage_width, &gImage_height);

    if (!gImage_buf)
        exit_example();

    printf("convert image from BMP to RGB565, resolution %d x %d\n\n", gImage_width, gImage_height);

    // dme configuration
    struct kdp_dme_cfg_s dme_cfg;
    memset(&dme_cfg, 0, sizeof(kdp_dme_cfg_s));

    dme_cfg.model_id = TINY_YOLO_V3_224_224_3; // model id when compiling in toolchain
    dme_cfg.output_num = 2;           // number of output node for the model
    dme_cfg.image_col = gImage_width;
    dme_cfg.image_row = gImage_height;
    dme_cfg.image_ch = 3;
    dme_cfg.image_format = IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565;

    for (int i = 0; i < gNum_devices; i++)
    {
        int ret = kdp_dme_configure(gDev_idxs[i], (char *)&dme_cfg, sizeof(struct kdp_dme_cfg_s), &gModel_id);

        printf("configure DME on device_idx %d ", gDev_idxs[i]);
        if (ret == 0)
            printf(",gModel_id = %d ... OK\n", gModel_id);
        else
        {
            printf("... failed\n");
            exit_example();
        }
    }

    pthread_t *device_threads = (pthread_t *)malloc(sizeof(pthread_t) * gNum_devices);

    printf("\nstarting inference %d images with %d KL520 devices:\n\n", (gNum_devices * gTest_loops), gNum_devices);

    double start = what_time_is_it_now();

    // create threads for each KL520 device
    for (int i = 0; i < gNum_devices; i++)
    {
        device_threads[i] = 0;
        if (pthread_create(&device_threads[i], NULL, thread_start_inference, (void *)&gDev_idxs[i]) != 0)
        {
            printf("pthread_create for device_idx %d ... failed\n", gDev_idxs[i]);
        }
    }

    dme_res *result[MAX_DEV_NUM];

    // waiting for all threads done
    for (int i = 0; i < gNum_devices; i++)
    {
        if (pthread_join(device_threads[i], (void **)&result[i]) != 0)
        {
            printf("pthread_join for device_idx %d ... failed\n", gDev_idxs[i]);
        }
    }

    printf("\n\nall device inference loops are done, inference results :\n");

    // print final inference results for all devices
    for (int i = 0; i < gNum_devices; i++)
    {
        printf("\n");
        printf("[device_idx %d]\n", gDev_idxs[i]);
        printf("    Number of class: %d\n", result[i]->class_count);
        printf("    Number of detection: %d\n", result[i]->box_count);
        for (uint32_t j = 0; j < result[i]->box_count; j++)
        {
            printf("    Box[%d] (x1, y1, x2, y2, score, class) = %.0f, %.0f, %.0f, %.0f, %.4f, %d\n",
                   j, result[i]->boxes[j].x1, result[i]->boxes[j].y1,
                   result[i]->boxes[j].x2, result[i]->boxes[j].y2,
                   result[i]->boxes[j].score, result[i]->boxes[j].class_num);
        }

        // free result memory allocated by threads
        free(result[i]);
    }

    float fps = 1. / ((what_time_is_it_now() - start) / (gNum_devices * gTest_loops));

    printf("\n\naverage time on %d frames: %f ms/frame, fps: %f\n", gNum_devices * gTest_loops, (1 / fps) * 1000, fps);
    gTest_done = true;

    free(gImage_buf);
    return;
}

void dme_upload_model_to_devices()
{
    printf("\nread 'tiny_yolo_v3' model files from '%s'\n\n", DME_MODEL_FILE_TYv3);

    long model_size;
    char *model_buf = NULL;

    do
    {
        model_buf = read_file_to_buffer_auto_malloc(DME_MODEL_FILE_TYv3, &model_size);

        if (!model_buf)
            break;

        for (int i = 0; i < gNum_devices; i++)
        {
            uint32_t ret_size = 0;

            int ret = kdp_start_dme_ext(gDev_idxs[i], model_buf, model_size, &ret_size);

            printf("upload model and start DME mode on device_idx %d ", gDev_idxs[i]);
            if (ret == 0)
                printf("... OK\n");
            else
            {
                printf(", ret: %d ... failed\n", ret_size);
                exit_example();
            }
        }

    } while (0);

    if (model_buf)
        free(model_buf);
}

void search_and_connect_KL520_devices()
{
    // scan Kneron devices
    kdp_device_info_list_t *list;

    kdp_scan_usb_devices(&list);

    printf("\n");

    // but we are only interesting in KL520 devices
    for (int i = 0; i < list->num_dev; i++)
    {
        if (list->kdevice[i].product_id == KDP_DEVICE_KL520)
        {
            printf("connect to KL520 device (KN_num '0x%08X', Port '%s', Speed: '%s') ... ",
                   list->kdevice[i].serial_number, list->kdevice[i].device_path,
                   list->kdevice[i].link_speed == KDP_USB_SPEED_SUPER ? "Super-Speed" : "High-Speed");

            int dev_idx = kdp_connect_usb_device(list->kdevice[i].scan_index);
            if (dev_idx >= 0)
            {
                printf("sucessful, device index = %d\n", dev_idx);
                gDev_idxs[gNum_devices++] = dev_idx;
            }
            else
                printf("failed\n");

            if (gNum_devices >= MAX_DEV_NUM)
                break;
        }
    }

    free(list);
}

char* p_ci_logfile_name_g = NULL;

int main()
{
    int ret;

    ret = kdp_lib_init();
    printf("\nkdp_lib_init() ... %s\n", ret < 0 ? "failed" : "OK");

    ret = kdp_lib_start();
    printf("kdp_lib_start() ... %s\n", ret < 0 ? "failed" : "OK");

    // scan all Kneron devices and only connect to KL520 devices
    search_and_connect_KL520_devices();

    // use DME API to upload model to each KL520 devices
    dme_upload_model_to_devices();

    // configure DME and start inference on each KL520 devices parallelly by threads
    dme_inference_on_multiple_devices();

    exit_example();

    return 0;
}
