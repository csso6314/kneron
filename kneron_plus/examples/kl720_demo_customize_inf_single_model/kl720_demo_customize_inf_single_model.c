#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kp_core.h"
#include "kp_inference.h"
#include "helper_functions.h"

#include "kl720_demo_customize_inf_single_model.h"

static char _model_file_path[128] = "../../res/models/KL720/YoloV5s_640_640_3/models_720.nef";
static char _image_file_path[128] = "../../res/images/one_bike_many_cars_608x608.bmp";
static int _loop = 10;

int main(int argc, char *argv[])
{
    kp_devices_list_t *device_list;
    kp_device_group_t device;
    kp_model_nef_descriptor_t model_desc;

    // each device has a unique port ID, 0 for auto-search
    int port_id = (argc > 1) ? atoi(argv[1]) : 0;
    int ret;

    /******* check the device USB speed *******/
    {
        int link_speed;
        device_list = kp_scan_devices();

        helper_get_device_usb_speed_by_port_id(device_list, port_id, &link_speed);
        if (KP_USB_SPEED_SUPER != link_speed)
            printf("[warning] device is not run at super speed.\n");
    }

    /******* connect the device *******/
    {
        int error_code;

        // connect device
        device = kp_connect_devices(1, &port_id, &error_code);
        if (!device)
        {
            printf("error ! connect device failed, port ID = '%d', error = '%d'\n", port_id, error_code);
            exit(0);
        }

        printf("connect device ... OK\n");

        kp_set_timeout(device, 5000);
    }

    /******* upload model to device *******/
    {
        ret = kp_load_model_from_file(device, _model_file_path, &model_desc);
        if (ret != KP_SUCCESS)
        {
            printf("error ! upload model failed, error = %d (%s)\n", ret, kp_error_string(ret));
            exit(0);
        }

        printf("upload model ... OK\n");
    }

    /******* prepare the image buffer read from file *******/
    // here convert a bmp file to RGB565 format buffer
    int img_width, img_height;
    char *img_buf = helper_bmp_file_to_raw_buffer(_image_file_path, &img_width, &img_height, KP_IMAGE_FORMAT_RGB565);
    if (!img_buf)
    {
        printf("error ! read image file failed\n");
        exit(0);
    }

    printf("read image ... OK\n");

    printf("\nstarting inference loop %d times:\n", _loop);

    /******* prepare input and output header/buffers *******/
    demo_customize_inf_single_model_header_t input_header;
    demo_customize_inf_single_model_result_t output_result;

    input_header.header_stamp.job_id = DEMO_KL720_CUSTOMIZE_INF_SINGLE_MODEL_JOB_ID;
    input_header.header_stamp.total_image = 1;
    input_header.header_stamp.image_index = 0;
    input_header.width = img_width;
    input_header.height = img_height;

    int header_size = sizeof(demo_customize_inf_single_model_header_t);
    int image_size = img_width * img_height * 2; // RGB565
    int result_size = sizeof(demo_customize_inf_single_model_result_t);
    int recv_size = 0;

    /******* starting inference work *******/

    for (int i = 0; i < _loop; i++)
    {
        ret = kp_customized_inference_send(device, (void *)&input_header, header_size, (uint8_t *)img_buf, image_size);
        if (ret != KP_SUCCESS)
            break;

        ret = kp_customized_inference_receive(device, (void *)&output_result, result_size, &recv_size);
        if (ret != KP_SUCCESS)
            break;

        printf("\n[loop %d]\n", i + 1);
        helper_print_yolo_box_on_bmp((kp_yolo_result_t *)&output_result.yolo_result, _image_file_path);
    }
    printf("\n");

    if (ret != KP_SUCCESS)
    {
        printf("\ninference failed, error = %d (%s)\n", ret, kp_error_string(ret));
        return -1;
    }

    free(img_buf);
    kp_release_model_nef_descriptor(&model_desc);
    kp_disconnect_devices(device);

    return 0;
}
