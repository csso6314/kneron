#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kp_core.h"
#include "kp_inference.h"
#include "helper_functions.h"

#include "kl520_demo_customize_inf_multiple_models.h"

static char _scpu_fw_path[128] = "../../res/firmware/KL520/fw_scpu.bin";
static char _ncpu_fw_path[128] = "../../res/firmware/KL520/fw_ncpu.bin";
static char _model_file_path[128] = "../../res/models/KL520/ssd_fd_lm/models_520.nef";
static char _image_file_path[128] = "../../res/images/a_woman_640x480.bmp";
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
        if (KP_USB_SPEED_HIGH != link_speed)
            printf("[warning] device is not run at high speed.\n");
    }

    /******* connect the device *******/
    {
        int error_code;

        // internal parameter to indicate the desired port id
        if (argc > 1) {
            port_id = atoi(argv[1]);
        }

        // connect device
        device = kp_connect_devices(1, &port_id, &error_code);
        if (!device) {
            printf("error ! connect device failed, port ID = '%d', error = '%d'\n", port_id, error_code);
            exit(0);
        }

        kp_set_timeout(device, 5000);
        printf("connect device ... OK\n");
    }

    /******* upload firmware to device *******/
    {
        ret = kp_load_firmware_from_file(device, _scpu_fw_path, _ncpu_fw_path);
        if (KP_SUCCESS != ret) {
            printf("error ! upload firmware failed, error = %d (%s)\n", ret, kp_error_string(ret));
            exit(0);
        }

        printf("upload firmware ... OK\n");
    }

    /******* upload model to device *******/
    {
        ret = kp_load_model_from_file(device, _model_file_path, &model_desc);
        if (KP_SUCCESS != ret) {
            printf("error ! upload model failed, error = %d (%s)\n", ret, kp_error_string(ret));
            exit(0);
        }

        printf("upload model ... OK\n");
    }

    /******* prepare the image buffer read from file *******/
    // here convert a bmp file to RGB565 format buffer
    int img_width, img_height;
    char *img_buf = helper_bmp_file_to_raw_buffer(_image_file_path, &img_width, &img_height, KP_IMAGE_FORMAT_RGB565);
    if (!img_buf) {
        printf("error ! read image file failed\n");
        exit(0);
    }

    printf("read image ... OK\n");

    printf("\nstarting inference loop %d times:\n", _loop);

    /******* prepare input and output header/buffers *******/
    demo_customize_inf_multiple_models_header_t input_header;
    demo_customize_inf_multiple_models_result_t output_result;

    input_header.header_stamp.job_id = DEMO_KL520_CUSTOMIZE_INF_MULTIPLE_MODEL_JOB_ID;
    input_header.header_stamp.total_image = 1;
    input_header.header_stamp.image_index = 0;
    input_header.width = img_width;
    input_header.height = img_height;

    int header_size = sizeof(demo_customize_inf_multiple_models_header_t);
    int image_size = img_width * img_height * 2; // RGB565
    int result_size = sizeof(demo_customize_inf_multiple_models_result_t);
    int recv_size = 0;

    /******* starting inference work *******/

    for (int i = 0; i < _loop; i++) {
        ret = kp_customized_inference_send(device, (void *)&input_header, header_size, (uint8_t *)img_buf, image_size);
        if (KP_SUCCESS != ret) {
            printf("\ninference failed, error = %d (%s)\n", ret, kp_error_string(ret));
            break;
        }

        ret = kp_customized_inference_receive(device, (void *)&output_result, result_size, &recv_size);
        if (KP_SUCCESS != ret) {
            printf("\ninference failed, error = %d (%s)\n", ret, kp_error_string(ret));
            break;
        }

        printf("\n[loop %d]\n", i + 1);

        for (int j = 0; j < output_result.face_count; j++) {
            printf("\nFace %d (x1, y1, x2, y2, score) = %d, %d, %d, %d, %f\n", j + 1
                                                                             , (int)output_result.faces[j].fd.x1
                                                                             , (int)output_result.faces[j].fd.y1
                                                                             , (int)output_result.faces[j].fd.x2
                                                                             , (int)output_result.faces[j].fd.y2
                                                                             , output_result.faces[j].fd.score);

            for (int k = 0; k < LAND_MARK_POINTS; k++) {
                printf("    - Landmark %d: (x, y) = %d, %d\n", k + 1
                                                             , output_result.faces[j].lm.marks[k].x
                                                             , output_result.faces[j].lm.marks[k].y);
            }
        }
    }

    printf("\n");

    free(img_buf);
    kp_release_model_nef_descriptor(&model_desc);
    kp_disconnect_devices(device);

    return 0;
}
