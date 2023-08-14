#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kdp2_postprocess.h"

#define YOLO_V3_CELL_BOX_NUM 3
#define YOLO_V3_BOX_FIX_CH 5
#define PROB_THRESH_YOLOV3 0.2
#define NMS_THRESH_YOLOV3 0.45
#define MAX_POSSIBLE_BOXES 2000
#define MODEL_SHIRNK_RATIO 32
#define YOLO_MAX_DETECTION_PER_CLASS 100

/* IOU Methods */
enum IOU_TYPE
{
    IOU_UNION = 0,
    IOU_MIN,
};

// For output node with small dimensions (public tiny-yolo-v3)
// first [0] is for node 1
// second [1] is for node 2
const float anchers[3][3][2] = {
    {{81, 82}, {135, 169}, {344, 319}},
    {{23, 27}, {37, 58}, {81, 82}},
    {{0, 0}, {0, 0}, {0, 0}}}; // FIXME

static float sigmoid(float x)
{
    float exp_value;
    float return_value;

    exp_value = exp(-x);

    return_value = 1 / (1 + exp_value);

    return return_value;
}

static int float_comparator(float a, float b)
{
    float diff = a - b;

    if (diff < 0)
        return 1;
    else if (diff > 0)
        return -1;
    return 0;
}

static int box_score_comparator(const void *pa, const void *pb)
{
    float a, b;

    a = ((kdp2_bounding_box_t *)pa)->score;
    b = ((kdp2_bounding_box_t *)pb)->score;

    return float_comparator(a, b);
}

static float overlap(float l1, float r1, float l2, float r2)
{
    float left = l1 > l2 ? l1 : l2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

static float box_intersection(kdp2_bounding_box_t *a, kdp2_bounding_box_t *b)
{
    float w, h, area;

    w = overlap(a->x1, a->x2, b->x1, b->x2);
    h = overlap(a->y1, a->y2, b->y1, b->y2);

    if (w < 0 || h < 0)
        return 0;

    area = w * h;
    return area;
}

static float box_union(kdp2_bounding_box_t *a, kdp2_bounding_box_t *b)
{
    float i, u;

    i = box_intersection(a, b);
    u = (a->y2 - a->y1) * (a->x2 - a->x1) + (b->y2 - b->y1) * (b->x2 - b->x1) - i;

    return u;
}

static float box_iou(kdp2_bounding_box_t *a, kdp2_bounding_box_t *b, int nms_type)
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

int kdp2_post_process_yolo_v3(kdp2_node_output_t *node_output[], int num_output_node, int img_width, int img_height, kdp2_yolo_result_t *yoloResult)
{
    int class_count = (node_output[0]->channel / YOLO_V3_CELL_BOX_NUM) - YOLO_V3_BOX_FIX_CH;
    int model_raw_width = node_output[0]->width * MODEL_SHIRNK_RATIO;
    int model_raw_height = node_output[0]->height * MODEL_SHIRNK_RATIO;

    float *box_class_probs = (float *)malloc(class_count * sizeof(float));

    int good_box_count = 0;
    kdp2_bounding_box_t *possible_boxes = (kdp2_bounding_box_t *)malloc(MAX_POSSIBLE_BOXES * sizeof(kdp2_bounding_box_t));
    kdp2_bounding_box_t *temp_boxes = (kdp2_bounding_box_t *)malloc(MAX_POSSIBLE_BOXES * sizeof(kdp2_bounding_box_t));

    int max_w_h = img_width > img_height ? img_width : img_height;

    for (int i = 0; i < num_output_node; i++)
    {
        int grid_w = node_output[i]->width;
        int grid_h = node_output[i]->height;
        int grid_c = node_output[i]->channel;

        int width_size = grid_w * grid_c;
        int anchor_offset = width_size / 3;

        for (int row = 0; row < grid_h; row++)
        {
            for (int an = 0; an < YOLO_V3_CELL_BOX_NUM; an++)
            {
                float *data = node_output[i]->data + row * width_size + an * anchor_offset;

                float *x_p = data;
                float *y_p = x_p + grid_w;
                float *width_p = y_p + grid_w;
                float *height_p = width_p + grid_w;
                float *score_p = height_p + grid_w;
                float *class_p = score_p + grid_w;

                for (int col = 0; col < grid_w; col++)
                {
                    float box_x = *(x_p + col);
                    float box_y = *(y_p + col);
                    float box_w = *(width_p + col);
                    float box_h = *(height_p + col);
                    float box_confidence = sigmoid(*(score_p + col));

                    float x1, y1, x2, y2;
                    bool first_box = false;

                    for (int j = 0; j < class_count; j++)
                    {
                        box_class_probs[j] = (float)*(class_p + col + j * grid_w);
                    }

                    /* Get scores of all class */
                    for (int j = 0; j < class_count; j++)
                    {
                        float max_score = sigmoid(box_class_probs[j]) * box_confidence;
                        if (max_score >= PROB_THRESH_YOLOV3)
                        {
                            if (!first_box)
                            {
                                first_box = true;

                                box_x = (sigmoid(box_x) + col) / grid_w;
                                box_y = (sigmoid(box_y) + row) / grid_h;
                                box_w = exp(box_w) * anchers[i][an][0] / model_raw_width;
                                box_h = exp(box_h) * anchers[i][an][1] / model_raw_height;

                                x1 = (box_x - (box_w / 2)) * max_w_h;
                                y1 = (box_y - (box_h / 2)) * max_w_h;
                                x2 = (box_x + (box_w / 2)) * max_w_h;
                                y2 = (box_y + (box_h / 2)) * max_w_h;
                            }

                            possible_boxes[good_box_count].x1 = x1;
                            possible_boxes[good_box_count].y1 = y1;
                            possible_boxes[good_box_count].x2 = x2;
                            possible_boxes[good_box_count].y2 = y2;
                            possible_boxes[good_box_count].score = max_score;
                            possible_boxes[good_box_count].class_num = j;
                            good_box_count++;

                            if (good_box_count >= MAX_POSSIBLE_BOXES)
                            {
                                printf("post yolo v3: error ! aborted due to too many boxes\n");
                                free(box_class_probs);
                                free(possible_boxes);
                                free(temp_boxes);

                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    int good_result_count = 0;

    for (int i = 0; i < class_count; i++)
    {
        kdp2_bounding_box_t *bbox = possible_boxes;
        kdp2_bounding_box_t *r_tmp_p = temp_boxes;

        int class_good_box_count = 0;

        for (int j = 0; j < good_box_count; j++)
        {
            if (bbox->class_num == i)
            {
                memcpy(r_tmp_p, bbox, sizeof(kdp2_bounding_box_t));
                r_tmp_p++;
                class_good_box_count++;
            }
            bbox++;
        }

        if (class_good_box_count == 1)
        {
            memcpy(&(yoloResult->boxes[good_result_count]), &temp_boxes[0], sizeof(kdp2_bounding_box_t));
            good_result_count++;
        }
        else if (class_good_box_count >= 2)
        {
            qsort(temp_boxes, class_good_box_count, sizeof(kdp2_bounding_box_t), box_score_comparator);
            for (int j = 0; j < class_good_box_count; j++)
            {
                if (temp_boxes[j].score == 0)
                    continue;
                for (int k = j + 1; k < class_good_box_count; k++)
                {
                    if (box_iou(&temp_boxes[j], &temp_boxes[k], IOU_UNION) > NMS_THRESH_YOLOV3)
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
                    memcpy(&(yoloResult->boxes[good_result_count]), &temp_boxes[j], sizeof(kdp2_bounding_box_t));
                    good_result_count++;
                    good_count++;
                }
                if (YOLO_MAX_DETECTION_PER_CLASS == good_count)
                {
                    break;
                }
            }
        }
    }

    for (int i = 0; i < good_result_count; i++)
    {
        yoloResult->boxes[i].x1 = (int)(yoloResult->boxes[i].x1 + (float)0.5) < 0 ? 0 : (int)(yoloResult->boxes[i].x1 + (float)0.5);
        yoloResult->boxes[i].y1 = (int)(yoloResult->boxes[i].y1 + (float)0.5) < 0 ? 0 : (int)(yoloResult->boxes[i].y1 + (float)0.5);
        yoloResult->boxes[i].x2 = (int)(yoloResult->boxes[i].x2 + (float)0.5) > img_width ? img_width : (int)(yoloResult->boxes[i].x2 + (float)0.5);
        yoloResult->boxes[i].y2 = (int)(yoloResult->boxes[i].y2 + (float)0.5) > img_height ? img_height : (int)(yoloResult->boxes[i].y2 + (float)0.5);
    }

    yoloResult->box_count = good_result_count;
    yoloResult->class_count = class_count;

    free(box_class_probs);
    free(possible_boxes);
    free(temp_boxes);

    return 0;
}
