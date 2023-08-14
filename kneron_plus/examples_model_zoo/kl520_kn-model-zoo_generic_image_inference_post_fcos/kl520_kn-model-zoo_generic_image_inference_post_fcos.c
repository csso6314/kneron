/**
 * @file        kl520_demo_generic_inference.c
 * @brief       main code of generic inference (output raw data)
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

#include "kp_core.h"
#include "kp_inference.h"
#include "helper_functions.h"
#include "postprocess.h"

#define MAX_POSSIBLE_BOXES 1000
#define NMS_THRESH 0.35
#define MAX_DETECTION_PER_CLASS 100

/* IOU Methods */
enum IOU_TYPE
{
    IOU_UNION = 0,
    IOU_MIN,
};

static char _scpu_fw_path[128] = "../../res/firmware/KL520/fw_scpu.bin";
static char _ncpu_fw_path[128] = "../../res/firmware/KL520/fw_ncpu.bin";
static char _model_file_path[128] = "../../res/models/KL520/fcos-drk53s_w512h512_kn-model-zoo/kl520_20004_fcos-drk53s_w512h512.nef";
static char _image_file_path[128] = "../../res/images/one_bike_many_cars_800x800.bmp";
static int _loop = 1;

static kp_devices_list_t *_device_list;

static kp_device_group_t _device;
static kp_model_nef_descriptor_t _model_desc;
static kp_generic_image_inference_desc_t _input_data;
static kp_generic_image_inference_result_header_t _output_desc;
static char *_img_buf;
static int _img_width, _img_height;


float sigmoid(float x)
{
    float exp_value;
    float return_value;

    exp_value = exp(-x);

    return_value = 1 / (1 + exp_value);

    return return_value;
}

float overlap(float l1, float r1, float l2, float r2)
{
    float left = l1 > l2 ? l1 : l2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

float box_intersection(kp_bounding_box_t *a, kp_bounding_box_t *b)
{
    float w, h, area;

    w = overlap(a->x1, a->x2, b->x1, b->x2);
    h = overlap(a->y1, a->y2, b->y1, b->y2);

    if (w < 0 || h < 0)
        return 0;

    area = w * h;
    return area;
}

float box_union(kp_bounding_box_t *a, kp_bounding_box_t *b)
{
    float i, u;

    i = box_intersection(a, b);
    u = (a->y2 - a->y1) * (a->x2 - a->x1) + (b->y2 - b->y1) * (b->x2 - b->x1) - i;

    return u;
}

float box_iou(kp_bounding_box_t *a, kp_bounding_box_t *b, int nms_type)
{
    float c = 0.;
    switch (nms_type)
    {
    case IOU_MIN:
        if (box_intersection(a, b) / box_intersection(a, a) > box_intersection(a, b) / box_intersection(b, b))
        {
            c = box_intersection(a, b) / box_intersection(a, a);
        }
        else
        {
            c = box_intersection(a, b) / box_intersection(b, b);
        }
        break;
    default:
        if (c < box_intersection(a, b) / box_union(a, b))
        {
            c = box_intersection(a, b) / box_union(a, b);
        }
        break;
    }

    return c;
}

int float_comparator(float float_num_1, float float_num_2)
{
    float diff = float_num_1 - float_num_2;

    if (diff < 0)
        return 1;
    else if (diff > 0)
        return -1;
    return 0;
}

int int_comparator(int int_num_1, int int_num_2)
{
    int diff = int_num_1 - int_num_2;

    if (diff < 0)
        return -1;
    else if (diff > 0)
        return 1;
    return 0;
}

int box_comparator(const void *box_1, const void *box_2)
{
    kp_bounding_box_t *_box_1 = (kp_bounding_box_t *)box_1;
    kp_bounding_box_t *_box_2 = (kp_bounding_box_t *)box_2;

    int res = float_comparator(_box_1->score, _box_2->score);
    if (res != 0)
        return res;

    res = int_comparator(_box_1->x1, _box_2->x1);
    if (res != 0)
        return res;

    res = int_comparator(_box_1->y1, _box_2->y1);
    if (res != 0)
        return res;

    res = int_comparator(_box_1->x2, _box_2->x2);
    if (res != 0)
        return res;

    return int_comparator(_box_1->y2, _box_2->y2);
}



void post_process_fcos(kp_inf_float_node_output_t *node_output[], int num_output_node,
                       kp_hw_pre_proc_info_t *pre_proc_info, float score_thres, kp_yolo_result_t *OutputBBoxResult)
{
    kp_bounding_box_t *candidate_boxes = NULL;
    kp_bounding_box_t *temp_boxes = NULL;
    int img_width = pre_proc_info->img_width;
    int img_height = pre_proc_info->img_height;

    candidate_boxes = (kp_bounding_box_t *)malloc(MAX_POSSIBLE_BOXES * sizeof(kp_bounding_box_t));
    if(NULL == candidate_boxes) {
        printf("error! malloc %s failed\n", "candidate_boxes");
        goto err;
    }
    temp_boxes = (kp_bounding_box_t *)malloc(MAX_POSSIBLE_BOXES * sizeof(kp_bounding_box_t));
    if(NULL == temp_boxes) {
        printf("error! malloc %s failed\n", "candidate_boxes");
        goto err;
    }

    int bbox_cnt = 0;
    int pad_left = pre_proc_info->pad_left;
    int pad_top = pre_proc_info->pad_top;
    float bbox_scale_width = (float)pre_proc_info->img_width / pre_proc_info->resized_img_width;
    float bbox_scale_height = (float)pre_proc_info->img_height / pre_proc_info->resized_img_height;
    int class_count = node_output[3]->channel;

    int stages = num_output_node/3;
    for(int s = 0; s < stages; s++)
    {
        kp_inf_float_node_output_t *reg = node_output[s];
        kp_inf_float_node_output_t *cls = node_output[s + stages];
        kp_inf_float_node_output_t *cts = node_output[s + stages*2];

        int feature_map_w = cls->width;
        int feature_map_h = cls->height;
        int cls_feature_map_c = cls->channel;
        int cts_feature_map_c = cts->channel;
        int reg_feature_map_c = reg->channel;

        float *cls_data = cls->data;
        float *cts_data = cts->data;
        float *reg_data = reg->data;
        int cls_CW_block_size = feature_map_w * cls_feature_map_c; // CW in HCW data ordering
        int cts_CW_block_size = feature_map_w * cts_feature_map_c;
        int reg_CW_block_size = feature_map_w * reg_feature_map_c;
        int stride = pow(2, (int)log2(((float)pre_proc_info->model_input_height / feature_map_h) + 0.5));

        int max_score_cls_idx = -1;
        float max_cls_score = FLT_MIN_10_EXP;
        float cls_score = FLT_MIN_10_EXP;
        float l,t,r,b,cx,cy,xmin,ymin,xmax,ymax;

        for(int row = 0; row < feature_map_h; row++)
        {
            for(int col = 0; col < feature_map_w; col++)
            {

                // find max score
                max_score_cls_idx = -1;
                max_cls_score = FLT_MIN_10_EXP;
                for(int ch = 0; ch < cls_feature_map_c; ch++)
                {
                    cls_score = sigmoid(*(cls_data + cls_CW_block_size*row + ch*feature_map_w + col)); // get data from HCW data ordering
                    if(cls_score > max_cls_score)
                    {
                        max_cls_score = cls_score;
                        max_score_cls_idx = ch;
                    }
                }

                max_cls_score = sqrt(max_cls_score*(sigmoid(*(cts_data + cts_CW_block_size*row + col))));
                if(max_cls_score > score_thres)
                {
                    l = *(reg_data + reg_CW_block_size*row + 0*feature_map_w + col);
                    t = *(reg_data + reg_CW_block_size*row + 1*feature_map_w + col);
                    r = *(reg_data + reg_CW_block_size*row + 2*feature_map_w + col);
                    b = *(reg_data + reg_CW_block_size*row + 3*feature_map_w + col);
                    l = l < 0 ? 0 : pow(2,3+s)*pow(l,2);
                    t = t < 0 ? 0 : pow(2,3+s)*pow(t,2);
                    r = r < 0 ? 0 : pow(2,3+s)*pow(r,2);
                    b = b < 0 ? 0 : pow(2,3+s)*pow(b,2);

                    cx = col*stride + stride/2;
                    cy = row*stride + stride/2;
                    xmin = cx-l;
                    ymin = cy-t;
                    xmax = cx+r;
                    ymax = cy+b;

                    candidate_boxes[bbox_cnt].x1 = xmin < 0 ? 0 : (xmin - pad_left) * bbox_scale_width;
                    candidate_boxes[bbox_cnt].y1 = ymin < 0 ? 0 : (ymin - pad_top) * bbox_scale_height;
                    candidate_boxes[bbox_cnt].x2 = xmax < 0 ? 0 : (xmax - pad_left) * bbox_scale_width;
                    candidate_boxes[bbox_cnt].y2 = ymax < 0 ? 0 : (ymax - pad_top) * bbox_scale_height;
                    candidate_boxes[bbox_cnt].score = max_cls_score;
                    candidate_boxes[bbox_cnt].class_num = max_score_cls_idx;

                    bbox_cnt++;

                    if (bbox_cnt >= MAX_POSSIBLE_BOXES)
                    {
                        printf("fcos post process error! too many boxes\n");
                        free(candidate_boxes);
                        free(temp_boxes);
                    }

                }

            }
        }
    } // end stage

    int good_result_count = 0;

    for (int i = 0; i < class_count; i++)
    {
        kp_bounding_box_t *bbox = candidate_boxes;
        kp_bounding_box_t *r_tmp_p = temp_boxes;

        int class_good_box_count = 0;

        for (int j = 0; j < bbox_cnt; j++)
        {
            if (bbox->class_num == i)
            {
                memcpy(r_tmp_p, bbox, sizeof(kp_bounding_box_t));
                r_tmp_p++;
                class_good_box_count++;
            }
            bbox++;
        }

        if (class_good_box_count == 1)
        {
            memcpy(&(OutputBBoxResult->boxes[good_result_count]), &temp_boxes[0], sizeof(kp_bounding_box_t));
            good_result_count++;
        }
        else if (class_good_box_count >= 2)
        {
            qsort(temp_boxes, class_good_box_count, sizeof(kp_bounding_box_t), box_comparator);
            for (int j = 0; j < class_good_box_count; j++)
            {
                if (temp_boxes[j].score == 0)
                    continue;
                for (int k = j + 1; k < class_good_box_count; k++)
                {
                    if (box_iou(&temp_boxes[j], &temp_boxes[k], IOU_UNION) > NMS_THRESH)
                    {
                        temp_boxes[k].score = 0;
                    }
                }
            }

            int good_count = 0;
            for (int j = 0; j < class_good_box_count; j++)
            {
                if (temp_boxes[j].score > 0)
                {
                    memcpy(&(OutputBBoxResult->boxes[good_result_count]), &temp_boxes[j], sizeof(kp_bounding_box_t));
                    good_result_count++;
                    good_count++;
                }
                if (MAX_DETECTION_PER_CLASS == good_count)
                {
                    break;
                }
            }
        }
    }

    for (int i = 0; i < good_result_count; i++)
    {
        OutputBBoxResult->boxes[i].x1 = (int)(OutputBBoxResult->boxes[i].x1 + (float)0.5) < 0 ? 0 : (int)(OutputBBoxResult->boxes[i].x1 + (float)0.5);
        OutputBBoxResult->boxes[i].y1 = (int)(OutputBBoxResult->boxes[i].y1 + (float)0.5) < 0 ? 0 : (int)(OutputBBoxResult->boxes[i].y1 + (float)0.5);
        OutputBBoxResult->boxes[i].x2 = (int)(OutputBBoxResult->boxes[i].x2 + (float)0.5) > img_width - 1 ? img_width - 1 : (int)(OutputBBoxResult->boxes[i].x2 + (float)0.5);
        OutputBBoxResult->boxes[i].y2 = (int)(OutputBBoxResult->boxes[i].y2 + (float)0.5) > img_height - 1 ? img_height - 1 : (int)(OutputBBoxResult->boxes[i].y2 + (float)0.5);
    }

    OutputBBoxResult->box_count = good_result_count;
    OutputBBoxResult->class_count = class_count;

    free(candidate_boxes);
    free(temp_boxes);

    return;

err:
    free(candidate_boxes);
    free(temp_boxes);

    return;

}

int main(int argc, char *argv[])
{
    // each device has a unique port ID, 0 for auto-search
    int port_id = (argc > 1) ? atoi(argv[1]) : 0;

    /******* check the device USB speed *******/
    int link_speed;
    _device_list = kp_scan_devices();
    helper_get_device_usb_speed_by_port_id(_device_list, port_id, &link_speed);
    if (KP_USB_SPEED_HIGH != link_speed)
        printf("[warning] device is not run at high speed.\n");

    /******* connect the device *******/
    _device = kp_connect_devices(1, &port_id, NULL);
    printf("connect device ... %s\n", (_device) ? "OK" : "failed");

    kp_set_timeout(_device, 5000); // 5 secs timeout

    /******* upload firmware to device *******/
    int ret = kp_load_firmware_from_file(_device, _scpu_fw_path, _ncpu_fw_path);
    printf("upload firmware ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* upload model to device *******/
    ret = kp_load_model_from_file(_device, _model_file_path, &_model_desc);
    printf("upload model ... %s\n", (ret == KP_SUCCESS) ? "OK" : "failed");

    /******* allocate memory for raw output *******/
    // by default here select first model
    uint32_t raw_buf_size = _model_desc.models[0].max_raw_out_size;
    uint8_t *raw_output_buf = (uint8_t *)malloc(raw_buf_size);

    /******* prepare the image buffer read from file *******/
    // here convert a bmp file to RGB565 format buffer
    _img_buf = helper_bmp_file_to_raw_buffer(_image_file_path, &_img_width, &_img_height, KP_IMAGE_FORMAT_RGB565);
    printf("read image ... %s\n", (_img_buf) ? "OK" : "failed");

    /******* set up the input descriptor *******/
    _input_data.model_id = _model_desc.models[0].id;    // first model ID
    _input_data.inference_number = 0;                   // inference number, used to verify with output result
    _input_data.num_input_node_image = 1;               // number of image

    _input_data.input_node_image_list[0].resize_mode = KP_RESIZE_ENABLE;        // enable resize in pre-process
    _input_data.input_node_image_list[0].padding_mode = KP_PADDING_CORNER;      // enable corner padding in pre-process
    _input_data.input_node_image_list[0].normalize_mode = KP_NORMALIZE_KNERON;  // this depends on models
    _input_data.input_node_image_list[0].image_format = KP_IMAGE_FORMAT_RGB565; // image format
    _input_data.input_node_image_list[0].width = _img_width;                    // image width
    _input_data.input_node_image_list[0].height = _img_height;                  // image height
    _input_data.input_node_image_list[0].crop_count = 0;                        // number of crop area, 0 means no cropping
    _input_data.input_node_image_list[0].image_buffer = (uint8_t *)_img_buf;    // buffer of image data

    printf("\nstarting inference loop %d times:\n", _loop);

    /******* starting inference work *******/
    for (int i = 0; i < _loop; i++)
    {
        ret = kp_generic_image_inference_send(_device, &_input_data);
        if (ret != KP_SUCCESS)
            break;

        ret = kp_generic_image_inference_receive(_device, &_output_desc, raw_output_buf, raw_buf_size);
        if (ret != KP_SUCCESS)
            break;

        printf(".");
        fflush(stdout);
    }
    printf("\n");

    free(_img_buf);
    kp_release_model_nef_descriptor(&_model_desc);
    kp_disconnect_devices(_device);

    if (ret != KP_SUCCESS)
    {
        printf("\ninference failed, error = %d\n", ret);
        return -1;
    }

    printf("\ninference loop is done\n");

    kp_inf_float_node_output_t *output_nodes[9] = {NULL}; // tiny yolo v3 outputs only two nodes, described by _output_desc.num_output_node

    // retrieve output nodes in floating point format
    output_nodes[0] = kp_generic_inference_retrieve_float_node(2, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[1] = kp_generic_inference_retrieve_float_node(5, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[2] = kp_generic_inference_retrieve_float_node(8, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[3] = kp_generic_inference_retrieve_float_node(0, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[4] = kp_generic_inference_retrieve_float_node(3, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[5] = kp_generic_inference_retrieve_float_node(6, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[6] = kp_generic_inference_retrieve_float_node(1, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[7] = kp_generic_inference_retrieve_float_node(4, raw_output_buf, KP_CHANNEL_ORDERING_HCW);
    output_nodes[8] = kp_generic_inference_retrieve_float_node(7, raw_output_buf, KP_CHANNEL_ORDERING_HCW);


    kp_yolo_result_t *output_bbox_result = (kp_yolo_result_t *)malloc(sizeof(kp_yolo_result_t));
    // post-process fcos output nodes to class/bounding boxes
    post_process_fcos(output_nodes, _output_desc.num_output_node, &_output_desc.pre_proc_info[0], 0.5, output_bbox_result);

    helper_print_yolo_box_on_bmp(output_bbox_result, _image_file_path);

    free(output_nodes[0]);
    free(output_nodes[1]);
    free(output_nodes[2]);
    free(output_nodes[3]);
    free(output_nodes[4]);
    free(output_nodes[5]);
    free(output_nodes[6]);
    free(output_nodes[7]);
    free(output_nodes[8]);
    free(output_bbox_result);

    free(raw_output_buf);

    return 0;
}
