#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kp_core.h"
#include "kp_inference.h"
#include "helper_functions.h"

#include "kl630_demo_customize_inf_multiple_models.h"

static char _scpu_fw_path[128] = "../../res/firmware/KL630/kp_firmware.tar";
static char _model_file_path[128] = "../../res/models/KL630/YoloV5s480_256_3_pdc/models_630.nef";
static char _image_file_path[128] = "../../res/images/travel_walk_480x256.bmp";
static int _loop = 10;

int main(int argc, char *argv[])
{
    kp_device_group_t device;
    kp_model_nef_descriptor_t model_desc;

    // each device has a unique port ID, 0 for auto-search
    int port_id = (argc > 1) ? atoi(argv[1]) : 0;
    int ret;

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
        ret = kp_load_firmware_from_file(device, _scpu_fw_path, NULL);
        if (ret != KP_SUCCESS)
        {
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
    pd_classification_result_t* pd_classification_result = &output_result.pd_classification_result;

    input_header.header_stamp.job_id = DEMO_KL630_CUSTOMIZE_INF_MULTIPLE_MODEL_JOB_ID;
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
            printf("\ninference failed send, error = %d (%s)\n", ret, kp_error_string(ret));
            break;
        }

        ret = kp_customized_inference_receive(device, (void *)&output_result, result_size, &recv_size);
        if (KP_SUCCESS != ret) {
            printf("\ninference failed send, error = %d (%s)\n", ret, kp_error_string(ret));
            break;
        }

        printf("\n[loop %d]\n", i + 1);

        for (int j = 0; j < pd_classification_result->box_count; j++) {
            printf("Box %d (x1, y1, x2, y2, class, score, pb score) = (%d, %d), (%d, %d), %d, %f, %f\n",
                   j + 1,
                   (int)pd_classification_result->pds[j].pd.x1,
                   (int)pd_classification_result->pds[j].pd.y1,
                   (int)pd_classification_result->pds[j].pd.x2,
                   (int)pd_classification_result->pds[j].pd.y2,
                   (int)pd_classification_result->pds[j].pd.class_num,
                   pd_classification_result->pds[j].pd.score,
                   pd_classification_result->pds[j].pd_class_score);
        }
    }

    printf("\n");

    free(img_buf);
    kp_release_model_nef_descriptor(&model_desc);
    kp_disconnect_devices(device);

    return 0;
}
