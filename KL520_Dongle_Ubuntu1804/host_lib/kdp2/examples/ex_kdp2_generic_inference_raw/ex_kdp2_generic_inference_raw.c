#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "kdp2_core.h"
#include "kdp2_postprocess.h"

#include "helper_functions.h"

// some given default settings for a demo
// user can change below settings or using command parameters to change them
static int _scan_index = 1;
static char _model_file_path[128] = "../../input_models/KL520/tiny_yolo_v3/models_520.nef";
static char _img_file_path[128] = "../../input_images/one_bike_many_cars_224x224.bmp";
static int _model_id = 19;                 // read from model NEF
static char _img_format[16] = "RGB565";    // 'RGB565', 'RGBA8888'
static char _norm_mode[16] = "-0.5_0.5";   // 'none', '0_1', '-0.5_0.5', '-1_1'
static char _post_process[64] = "yolo_v3"; // 'bypass', 'yolo_v3'
static int _test_loop = 20;

// global shared variable for convenient demo
kdp2_device_t _device = NULL;
char *_img_buffer;
int _img_width;
int _img_height;

kdp2_raw_input_descriptor_t _inf_descriptor;
kdp2_raw_output_descriptor_t _raw_out_desc;
kdp2_node_output_t *_output_nodes[50] = {NULL};
uint8_t *_raw_output_buf = NULL; // to receive RAW output from device and node floating point values
int _raw_output_size = 0;

void print_settings()
{
    printf("\n");
    printf("-h : help\n");
    printf("-s : [device index] = '%d'\n", _scan_index);
    printf("-m : [model file path] (.nef) = '%s'\n", _model_file_path);
    printf("-i : [image file path] (.bmp 24bits) = '%s'\n", _img_file_path);
    printf("-d : [inference model ID] = '%d'\n", _model_id);
    printf("-c : [image format] (RGB565, RGBA8888) = '%s'\n", _img_format);
    printf("-n : [normalize mode] ('none', '0_1', '-0.5_0.5', '-1_1') = '%s'\n", _norm_mode);
    printf("-p : [post process] ('bypass', 'yolo_v3') = '%s'\n", _post_process);
    printf("-l : [test loops] = '%d'\n", _test_loop);
    printf("\n");
}

bool parse_arguments(int argc, char *argv[])
{
    int opt = 0;
    while ((opt = getopt(argc, argv, "hL:s:i:d:c:l:n:m:p:")) != -1)
    {
        switch (opt)
        {
        case 's':
            _scan_index = atoi(optarg);
            break;
        case 'i':
            strcpy(_img_file_path, optarg);
            break;
        case 'm':
            strcpy(_model_file_path, optarg);
            break;
        case 'd':
            _model_id = atoi(optarg);
            break;
        case 'c':
            strcpy(_img_format, optarg);
            break;
        case 'n':
            strcpy(_norm_mode, optarg);
            break;
        case 'l':
            _test_loop = atoi(optarg);
            break;
        case 'p':
            strcpy(_post_process, optarg);
            break;
        case 'h':
        default:
            print_settings();
            exit(0);
        }
    }

    return true;
}

void print_model_info(kdp2_all_models_descriptor_t *pModel_desc)
{
    printf("\n");
    printf("this NEF contains %d model(s):\n", pModel_desc->num_models);
    for (int i = 0; i < pModel_desc->num_models; i++)
    {
        printf("[%d] model ID = %d\n", i + 1, pModel_desc->models[i].id);
        printf("[%d] model raw input width = %d\n", i + 1, pModel_desc->models[i].width);
        printf("[%d] model raw input height = %d\n", i + 1, pModel_desc->models[i].height);
        printf("[%d] model input channel = %d\n", i + 1, pModel_desc->models[i].channel);
        printf("[%d] model raw image format = %s\n", i + 1,
               pModel_desc->models[i].img_format == KDP2_IMAGE_FORMAT_RGBA8888 ? "RGBA8888" : "N/A");
        printf("[%d] model raw output size = %d\n", i + 1, pModel_desc->models[i].max_raw_out_size);
        printf("\n");
    }
}

void *image_send_function(void *data)
{
    // best effort to send images
    for (int i = 0; i < _test_loop; i++)
    {
        int ret = kdp2_raw_inference_send(_device, &_inf_descriptor, (uint8_t *)_img_buffer);
        if (ret != KDP2_SUCCESS)
        {
            printf("kdp2_raw_inference_send() error = %d\n", ret);
            exit(0);
        }
    }

    return NULL;
}

uint32_t _yolo_box_count;
kdp2_bounding_box_t _yolo_boxes[100];

void *result_receive_function(void *data)
{
    // best effort to receive results
    for (int i = 0; i < _test_loop; i++)
    {
        int ret = kdp2_raw_inference_receive(_device, &_raw_out_desc, _raw_output_buf, _raw_output_size);
        if (ret != KDP2_SUCCESS)
        {
            printf("kdp2_raw_inference_receive() error = %d\n", ret);
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
    double time_spent;

    kdp2_all_models_descriptor_t model_desc;

    parse_arguments(argc, argv);

    print_settings();

    /***********  connect device ***********/

    _device = kdp2_connect_device(_scan_index);
    if (!_device)
    {
        printf("error ! connect to the device failed, please check it\n");
        return -1;
    }

    printf("\n");
    printf("connect device ... OK\n");

    kdp2_set_timeout(_device, 5000);

    /***********  load model files to device (optional if it is already in device' flash) ***********/

    measure_time_begin();
    ret = kdp2_load_model_from_file(_device, _model_file_path, &model_desc);
    measure_time_end(&time_spent);

    if (ret != KDP2_SUCCESS)
    {
        printf("error ! load model(s) failed, return = %d\n", ret);
        exit(0);
    }

    printf("load model(s) ... OK\n");
    printf("time spent: %.2f secs\n", time_spent);

    print_model_info(&model_desc);

    int midx = 0;
    for (midx = 0; midx < model_desc.num_models; midx++)
    {
        // find the model output size
        if (model_desc.models[midx].id == _model_id)
        {
            _raw_output_size = model_desc.models[midx].max_raw_out_size;
            break;
        }
    }

    // check if target model ID is available in NEF
    if (midx >= model_desc.num_models)
    {
        printf("error ! [inference model ID] = '%d' is not in the NEF models, please check it\n", _model_id);
        exit(0);
    }

    // allocte memory for inference RAW result output
    printf("target inference model ID : %d\n", _model_id);

    printf("allocate memory %d bytes for RAW output\n", _raw_output_size);

    _raw_output_buf = (uint8_t *)malloc(_raw_output_size);

    if (0 == strcmp(_img_format, "RGB565"))
    {
        _inf_descriptor.image_format = KDP2_IMAGE_FORMAT_RGB565;
        _img_buffer = bmp_file_to_rgb565_buffer_auto_malloc(_img_file_path, &_img_width, &_img_height);
    }
    else if (0 == strcmp(_img_format, "RGBA8888"))
    {
        _inf_descriptor.image_format = KDP2_IMAGE_FORMAT_RGBA8888;
        _img_buffer = bmp_file_to_rgba8888_buffer_auto_malloc(_img_file_path, &_img_width, &_img_height);
    }
    else
    {
        printf("invalid [image format] %s\n", _img_format);
        exit(0);
    }

    _inf_descriptor.width = _img_width;
    _inf_descriptor.height = _img_height;

    if (0 == strcmp(_norm_mode, "none"))
        _inf_descriptor.normalize_mode = KDP2_NORMALIE_NONE;
    else if (0 == strcmp(_norm_mode, "0_1"))
        _inf_descriptor.normalize_mode = KDP2_NORMALIE_0_TO_1;
    else if (0 == strcmp(_norm_mode, "-0.5_0.5"))
        _inf_descriptor.normalize_mode = KDP2_NORMALIE_NEGATIVE_05_TO_05;
    else if (0 == strcmp(_norm_mode, "-1_1"))
        _inf_descriptor.normalize_mode = KDP2_NORMALIE_NEGATIVE_1_TO_1;
    else
    {
        printf("invalid [normalize mode] %s\n", _norm_mode);
        exit(0);
    }

    _inf_descriptor.inference_number = 1;
    _inf_descriptor.model_id = _model_id;

    printf("image resolution %dx%d, format %s\n", _img_width, _img_height, _img_format);

    // check if image resoultion is bigger than model raw resolution
    if (_img_width < model_desc.models[midx].width || _img_height < model_desc.models[midx].height)
    {
        printf("error ! image resolution is smaller than models', KL520 does not support upscaling\n");
        exit(0);
    }
    else
    {
        printf("(device will convert it to %dx%d, RGBA8888, only support downscaling)\n",
               model_desc.models[midx].width, model_desc.models[midx].height);
    }

    printf("\n");
    printf("starting doing inference, loop = %d\n", _test_loop);

    // reset firmware inference queues
    kdp2_reset_device(_device, KDP2_INFERENCE_RESET);

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

    if (0 == strcmp(_post_process, "yolo_v3"))
    {
        printf("\n");
        printf("================== [Post Process] '%s' =========================\n", _post_process);

        kdp2_yolo_result_t *yolo_result = (kdp2_yolo_result_t *)malloc(sizeof(kdp2_yolo_result_t));

        // retrieve output nodes in floating point format
        for (int i = 0; i < _raw_out_desc.num_output_node; i++)
            _output_nodes[i] = kdp2_raw_inference_retrieve_node(i, _raw_output_buf);

        // post-process yolo v3 output nodes to class/bounding boxes
        kdp2_post_process_yolo_v3(_output_nodes, _raw_out_desc.num_output_node, _img_width, _img_height, yolo_result);

        printf("\n");
        printf("class count : %d\n", yolo_result->class_count);
        printf("box count : %d\n", yolo_result->box_count);
        for (int i = 0; i < yolo_result->box_count; i++)
        {
            printf("Box %d (x1, y1, x2, y2, score, class) = %.1f, %.1f, %.1f, %.1f, %f, %d\n",
                   i,
                   yolo_result->boxes[i].x1, yolo_result->boxes[i].y1,
                   yolo_result->boxes[i].x2, yolo_result->boxes[i].y2,
                   yolo_result->boxes[i].score, yolo_result->boxes[i].class_num);
        }

        char out_bmp_file[128];
        sprintf(out_bmp_file, "%s.bmp", argv[0]);
        draw_bounding_box_to_bmp_file(_img_file_path, out_bmp_file, yolo_result->boxes, yolo_result->box_count);

        printf("\n");
        printf("drew bounding box(es) on '%s'\n", out_bmp_file);

        free(yolo_result);

        printf("\n");
        printf("===========================================================================\n");
    }
    else if (0 == strcmp(_post_process, "bypass"))
    {
        printf("\n");
        printf("================== [Post Process] '%s' =========================\n", _post_process);

        printf("\n");
        printf("number of output node : %d\n", _raw_out_desc.num_output_node);

        for (int i = 0; i < _raw_out_desc.num_output_node; i++)
            _output_nodes[i] = kdp2_raw_inference_retrieve_node(i, _raw_output_buf);

        for (int i = 0; i < _raw_out_desc.num_output_node; i++)
        {
            printf("\n");
            printf("node %d:\n", i);
            printf("width: %d:\n", _output_nodes[i]->width);
            printf("height: %d:\n", _output_nodes[i]->height);
            printf("channel: %d:\n", _output_nodes[i]->channel);
            printf("number of data (float): %d:\n", _output_nodes[i]->num_data);

            printf("first 20 data:\n\t");
            for (int j = 0; j < 20; j++)
            {
                printf("%.3f, ", _output_nodes[i]->data[j]);
                if (j > 0 && j % 5 == 0)
                    printf("\n\t");
            }

            printf("\n");
        }

        printf("\n");
        for (int i = 0; i < _raw_out_desc.num_output_node; i++)
        {
            char file_name[256];
            sprintf(file_name, "node%d_%dx%dx%d.txt", i, _output_nodes[i]->width, _output_nodes[i]->height, _output_nodes[i]->channel);

            FILE *file = fopen(file_name, "w");
            if (file)
            {
                for (int j = 0; j < _output_nodes[i]->num_data; j++)
                {
                    fprintf(file, "%.3f", _output_nodes[i]->data[j]);
                    if (j != _output_nodes[i]->num_data - 1)
                        fprintf(file, ", ");
                }

                fclose(file);
                printf("dumped node %d output to '%s'\n", i, file_name);
            }
        }

        printf("\n");
        printf("===========================================================================\n");
    }
    else
    {
        printf("error ! incorrect post-process name\n");
    }

    printf("\ndisconnecting device ...\n");

    free(_img_buffer);
    free(_raw_output_buf);

    kdp2_disconnect_device(_device);

    return 0;
}
