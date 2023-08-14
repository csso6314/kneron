#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "kdp2_core.h"
#include "kdp2_app.h"

#include "helper_functions.h"

// the model must be tiny yolo v3 224x224
#define MODEL_PATH "../../input_models/KL520/tiny_yolo_v3/models_520.nef"

// some given default settings for a demo
// user can change below settings or using command parameters to change them
static int _scan_index = 1;
static char _image_file_path[128] = "../../input_images/street_cars_bike.bmp";
static int _test_loop = 20;

// global shared variable for convenient demo
kdp2_device_t _device = NULL;
char *_img_buffer;
int _img_width;
int _img_height;

void print_settings()
{
    printf("\n");
    printf("-h : help\n");
    printf("-s : [device index] = '%d'\n", _scan_index);
    printf("-i : [image file path] (.bmp 24bits) = '%s'\n", _image_file_path);
    printf("-l : [test loops] = '%d'\n", _test_loop);
    printf("\n");
}

bool parse_arguments(int argc, char *argv[])
{
    int opt = 0;
    while ((opt = getopt(argc, argv, "hs:i:l:")) != -1)
    {
        switch (opt)
        {
        case 's':
            _scan_index = atoi(optarg);
            break;
        case 'i':
            strcpy(_image_file_path, optarg);
            break;
        case 'l':
            _test_loop = atoi(optarg);
            break;
        case 'h':
        default:
            print_settings();
            exit(0);
        }
    }

    return true;
}

void *image_send_function(void *data)
{
    uint32_t inference_number = 0;

    // best effort to send images
    for (int i = 0; i < _test_loop; i++)
    {
        int ret = kdp2_app_tiny_yolo_v3_send(_device, inference_number++, (uint8_t *)_img_buffer,
                                             _img_width, _img_height, KDP2_IMAGE_FORMAT_RGB565);
        if (ret != KDP2_SUCCESS)
        {
            printf("kdp2_app_tiny_yolo_v3_send() error = %d\n", ret);
            exit(0);
        }
    }

    return NULL;
}

uint32_t _yolo_box_count;
kdp2_bounding_box_t _yolo_boxes[100];

void *result_receive_function(void *data)
{
    uint32_t inference_number = 0;

    // best effort to receive results
    for (int i = 0; i < _test_loop; i++)
    {
        int ret = kdp2_app_tiny_yolo_v3_receive(_device, &inference_number, &_yolo_box_count, _yolo_boxes);
        if (ret != KDP2_SUCCESS)
        {
            printf("kdp2_app_tiny_yolo_v3_receive() error = %d\n", ret);
            exit(0);
        }

        printf(".");
        fflush(stdout);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int ret;
    kdp2_all_models_descriptor_t model_desc;

    parse_arguments(argc, argv);

    print_settings();

    _device = kdp2_connect_device(_scan_index);
    if (!_device)
    {
        printf("error ! connect to the device failed, please check it\n");
        return -1;
    }

    // load model from file
    ret = kdp2_load_model_from_file(_device, MODEL_PATH, &model_desc);
    if (ret != KDP2_SUCCESS)
    {
        printf("error ! load model(s) failed, return = %d\n", ret);
        kdp2_disconnect_device(_device);
        return -1;
    }

    // read image file BMP
    // here convert BMP file into a RGB565 buffer
    _img_buffer = bmp_file_to_rgb565_buffer_auto_malloc(_image_file_path, &_img_width, &_img_height);

    if (!_img_buffer)
    {
        printf("error ! read %s failed\n", _image_file_path);
        kdp2_disconnect_device(_device);
        return -1;
    }

    printf("\n");
    printf("image resolution %dx%d, format RGB565\n", _img_width, _img_height);
    printf("(device will convert it to %dx%d, RGBA8888, only support downscaling)\n",
           model_desc.models[0].width, model_desc.models[0].height);

    printf("\n");
    printf("starting doing inference, loop = %d\n", _test_loop);

    // reset firmware inference queues
    kdp2_reset_device(_device, KDP2_INFERENCE_RESET);

    double time_spent;

    // start inferrence loop and measure FPS
    measure_time_begin();
    {
        pthread_t image_send_thd;
        pthread_t result_recv_thd;

        pthread_create(&image_send_thd, NULL, image_send_function, NULL);
        pthread_create(&result_recv_thd, NULL, result_receive_function, NULL);

        // waitinf for all thread send & receive done

        pthread_join(image_send_thd, NULL);
        pthread_join(result_recv_thd, NULL);
    }
    measure_time_end(&time_spent);

    printf("\n\n");

    printf("total inference %d images\n", _test_loop);
    printf("time spent: %.2f secs, FPS = %.1f\n", time_spent, _test_loop / time_spent);

    printf("\n");
    printf("box count : %d\n", _yolo_box_count);
    for (int i = 0; i < _yolo_box_count; i++)
    {
        printf("Box %d (x1, y1, x2, y2, score, class) = %.1f, %.1f, %.1f, %.1f, %f, %d\n",
               i,
               _yolo_boxes[i].x1, _yolo_boxes[i].y1,
               _yolo_boxes[i].x2, _yolo_boxes[i].y2,
               _yolo_boxes[i].score, _yolo_boxes[i].class_num);
    }

    // draw bounding box on output bmp file
    char out_bmp_file[128];
    sprintf(out_bmp_file, "%s.bmp", argv[0]);
    draw_bounding_box_to_bmp_file(_image_file_path, out_bmp_file, _yolo_boxes, _yolo_box_count);

    printf("\n");
    printf("drew bounding box(es) on '%s'\n", out_bmp_file);

    printf("\ndisconnecting device ...\n");

    free(_img_buffer);

    kdp2_disconnect_device(_device);

    return 0;
}
