/*
 * Kneron Example Post-Processing driver
 *
 * Copyright (C) 2018-2020 Kneron, Inc. All rights reserved.
 *
 */
 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stdio.h"
#include "post_processing_ex.h"
#include "base.h"// header file in /common/include
#include "kdpio.h"

#define YOLO_V3_O1_GRID_W       7
#define YOLO_V3_O1_GRID_H       7
#define YOLO_V3_O1_GRID_MAX     (YOLO_V3_O1_GRID_W * YOLO_V3_O1_GRID_H)
#define YOLO_V3_O2_GRID_W       14
#define YOLO_V3_O2_GRID_H       14
#define YOLO_V3_O2_GRID_MAX     (YOLO_V3_O2_GRID_W * YOLO_V3_O2_GRID_H)
#define YOLO_V3_CELL_BOX_NUM    3

#define YOLO_CELL_BOX_NUM       5
#define YOLO_CLASS_MAX          80
#define YOLO_BOX_FIX_CH         5   /* x, y, w, h, confidence score */

#define YOLO_MAX_DETECTION_PER_CLASS 100

#define IMAGENET_CLASSES_MAX    1000

#define KDP_COL_MIN             16 /* Bytes, i.e. 128 bits */

//const float prob_thresh_yolov3 = 0.3;      // probability threshold for yolo v3
const float prob_thresh_yolov3 = 0.15;      // probability threshold for yolo v3
const float nms_thresh_yolov3 = 0.45;      // non max suppression threshold for yolo v3

// For output node with small dimensions (public tiny-yolo-v3)
const float anchers_v0[3][2] = {{81,82}, {135,169}, {344,319}};
// For output node with large dimensions (public tiny-yolo-v3)
const float anchers_v1[3][2] = {{23,27}, {37,58}, {81,82}};

/* Shared global variable area among models */
struct yolo_v3_post_globals_s {
    float box_class_probs[YOLO_CLASS_MAX];
    struct bounding_box_s bboxes_v3[YOLO_GOOD_BOX_MAX];
    struct bounding_box_s result_tmp_s[YOLO_GOOD_BOX_MAX];
};

struct imagenet_post_globals_s {
    struct imagenet_result_s  temp[IMAGENET_CLASSES_MAX];
};

union post_globals_u_s {
    struct yolo_v3_post_globals_s     yolov3;
    struct imagenet_post_globals_s    imgnet;
} u_globals;

static float do_div_scale(float v, int div, float scale)
{
    return ((v / div) / scale);
}

static float do_div_scale_2(float v, float scale)
{
    return (v * scale);
}

uint32_t round_up(uint32_t num){
    return ((num + (KDP_COL_MIN - 1)) & ~(KDP_COL_MIN - 1));
}

static float sigmoid(float x)
{
    float exp_value;
    float return_value;

    exp_value = expf(-x);

    return_value = 1 / (1 + exp_value);

    return return_value;
}

static void softmax(struct imagenet_result_s input[], int input_len)
{
    int i;
    float m;
    
    m = input[0].score;
    for (i = 1; i < input_len; i++) {
        if (input[i].score > m) {
            m = input[i].score;
        }
    }

    float sum = 0;
    for (i = 0; i < input_len; i++) {
        sum += expf(input[i].score - m);
    }

    for (i = 0; i < input_len; i++) {
        input[i].score = expf(input[i].score - m - log(sum));
    }    
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

    a = ((struct bounding_box_s *)pa)->score;
    b = ((struct bounding_box_s *)pb)->score;

    return float_comparator(a, b);
}

static float overlap(float l1, float r1, float l2, float r2)
{
    float left = l1 > l2 ? l1 : l2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

static float box_intersection(struct bounding_box_s *a, struct bounding_box_s *b)
{
    float w, h, area;

    w = overlap(a->x1, a->x2, b->x1, b->x2);
    h = overlap(a->y1, a->y2, b->y1, b->y2);

    if (w < 0 || h < 0)
        return 0;

    area = w * h;
    return area;
}

static float box_union(struct bounding_box_s *a, struct bounding_box_s *b)
{
    float i, u;

    i = box_intersection(a, b);
    u = (a->y2 - a->y1) * (a->x2 - a->x1) + (b->y2 - b->y1) * (b->x2 - b->x1) - i;

    return u;
}

static float box_iou(struct bounding_box_s *a, struct bounding_box_s *b, int nms_type)
{
    float c = 0.;
    switch (nms_type) {
        case IOU_MIN:
            if (box_intersection(a, b) / box_intersection(a, a) > box_intersection(a, b) / box_intersection(b, b)) {
                c = box_intersection(a, b) / box_intersection(a, a);
            } else {
                c = box_intersection(a, b) / box_intersection(b, b);
            }
            break;
        default:
            if (c < box_intersection(a, b) / box_union(a, b)) {
                c = box_intersection(a, b) / box_union(a, b);
            }
            break;
    }

    return c;
}

int post_yolo_v3(int model_id, struct kdp_image_s *image_p)
{
    struct yolo_v3_post_globals_s *gp = &u_globals.yolov3;
    int i, j, k, div, ch, row, col, max_score_class, good_box_count, class_good_box_count, good_result_count, len, len_div_4;
    float box_x, box_y, box_w, box_h, box_confidence, max_score;
    int8_t *src_p, *x_p, *y_p, *width_p, *height_p, *score_p, *class_p, *dest_p;
    struct bounding_box_s *bbox;
    struct yolo_result_s *result;
    struct bounding_box_s *result_box_p, *result_tmp_p, *r_tmp_p;

    int data_size, grid_w, grid_h, grid_c, class_num, grid_w_bytes_aligned;
    uint32_t src_img_mode;

    float maxlen = RAW_INPUT_COL(image_p) > RAW_INPUT_ROW(image_p) ? (float)RAW_INPUT_COL(image_p) : (float)RAW_INPUT_ROW(image_p);

    src_img_mode = RAW_FORMAT(image_p);
    data_size = (POSTPROC_OUTPUT_FORMAT(image_p) & BIT(0)) + 1;     /* 1 or 2 in bytes */

    result = (struct yolo_result_s *)(POSTPROC_RESULT_MEM_ADDR(image_p));
    result_box_p = result->boxes;

    result_tmp_p = gp->result_tmp_s;

    class_num = POSTPROC_OUT_NODE_CH(image_p, 0) / YOLO_V3_CELL_BOX_NUM - YOLO_BOX_FIX_CH;

    result->class_count = class_num;
    
    bbox = gp->bboxes_v3;
    good_box_count = 0;

    float anchers_v[3][2];
    int idx;
    int offset = sizeof(struct out_node_s);
    struct out_node_s out_p;

    for (idx = 0; idx < POSTPROC_OUTPUT_NUM(image_p); idx++) {
        out_p = (struct out_node_s)(POSTPROC_OUT_NODE(image_p, idx));

        src_p = (int8_t *)POSTPROC_OUT_NODE_ADDR(image_p, idx);
 
        grid_w = OUT_NODE_COL(out_p);
        grid_h = OUT_NODE_ROW(out_p);
        grid_c = OUT_NODE_CH(out_p);
        len = grid_w * data_size;
        grid_w_bytes_aligned = round_up(len);
        //len = grid_w_bytes_aligned;
        //len_div_4 = len >> 2;
        uint32_t all_data = grid_w_bytes_aligned * (grid_c / 3);
        //dest_p = (int8_t*)malloc(all_data);
        //all_data = all_data >> 2;

        div = 1 << OUT_NODE_RADIX(out_p);
        float fScale = OUT_NODE_SCALE(out_p);
        fScale = 1 / (div * fScale);

        //For tiny yolo v3
        if (0 == idx) {
            memcpy(anchers_v, anchers_v0, 6*sizeof(float));
        } else {
            memcpy(anchers_v, anchers_v1, 6*sizeof(float));
        }

        // Divide the data into half as not enough memory for postprocessing of tiny-yolo-v3-608
        uint32_t all_data_half = all_data >> 1;
        //dest_p = (int8_t*)malloc(all_data_half*4);
        dest_p = (int8_t*)malloc(all_data_half);
        //dest_p = (float*)malloc(all_data_half * 4 * 4);
        all_data_half = all_data_half >> 2;
        //int k_end = (grid_w < 8) ? 1 : 2;
        int col_end = 0;
        int cell_box_len = grid_w_bytes_aligned * grid_h * grid_c / 3;

        for (row = 0; row < grid_h; row++) {
            for (ch = 0; ch < YOLO_V3_CELL_BOX_NUM; ch++) {
                x_p = src_p; // dest_p;
                y_p = x_p + grid_w_bytes_aligned * grid_h;
                width_p = y_p + grid_w_bytes_aligned * grid_h;
                height_p = width_p + grid_w_bytes_aligned * grid_h;
                score_p = height_p + grid_w_bytes_aligned * grid_h;
                class_p = score_p + grid_w_bytes_aligned * grid_h;

                col_end = grid_w;
                for (col = 0; col < col_end; col++) {
                    if (data_size == 1) {
                        box_x = (float)*x_p;
                        box_y = (float)*y_p;
                        box_w = (float)*width_p;
                        box_h = (float)*height_p;
                        box_confidence = (float)*score_p;
                    } else {
                        box_x = (float)*(uint16_t *)x_p;
                        box_y = (float)*(uint16_t *)y_p;
                        box_w = (float)*(uint16_t *)width_p;
                        box_h = (float)*(uint16_t *)height_p;
                        box_confidence = (float)*(uint16_t *)score_p;
                    }
                    box_confidence = sigmoid(do_div_scale_2(box_confidence, fScale));

                    /* Get scores of all class */
                    for (i = 0; i < class_num; i++) {
                        if (data_size == 1)
                            gp->box_class_probs[i] = (float)*(class_p + i * grid_w_bytes_aligned*grid_h);
                        else
                            gp->box_class_probs[i] = (float)*(uint16_t *)(class_p + i * grid_w_bytes_aligned*grid_h);
                    }

                    //increase pointer to next position
                    x_p += data_size;
                    y_p += data_size;
                    width_p += data_size;
                    height_p += data_size;
                    score_p += data_size;
                    class_p += data_size;

                    /* Find all classes with score higher than thresh */
                    int done_box = 0;

                    for (i = 0; i < class_num; i++) {
                        max_score_class = -1;

                        max_score = sigmoid(do_div_scale_2(gp->box_class_probs[i], fScale)) * box_confidence;
                        if (max_score >= prob_thresh_yolov3) {
                            max_score_class = i;
                        }
                        if (max_score_class != -1) {
                            if (!done_box) {
                                done_box = 1;
                                box_x = do_div_scale_2(box_x, fScale);
                                box_y = do_div_scale_2(box_y, fScale);
                                box_w = do_div_scale_2(box_w, fScale);
                                box_h = do_div_scale_2(box_h, fScale);

                                box_x = (sigmoid(box_x) + col) / grid_w;
                                box_y = (sigmoid(box_y) + row) / grid_h;
                                box_w = expf(box_w) * anchers_v[ch][0] / DIM_INPUT_COL(image_p);
                                box_h = expf(box_h) * anchers_v[ch][1] / DIM_INPUT_ROW(image_p);

                                if (src_img_mode & (uint32_t)IMAGE_FORMAT_CHANGE_ASPECT_RATIO) {
                                    bbox->x1 = (box_x - (box_w / 2)) * RAW_INPUT_COL(image_p);
                                    bbox->y1 = (box_y - (box_h / 2)) * RAW_INPUT_ROW(image_p);
                                    bbox->x2 = (box_x + (box_w / 2)) * RAW_INPUT_COL(image_p);
                                    bbox->y2 = (box_y + (box_h / 2)) * RAW_INPUT_ROW(image_p);
                                } else {
                                    bbox->x1 = (box_x - (box_w / 2)) * maxlen;
                                    bbox->y1 = (box_y - (box_h / 2)) * maxlen;
                                    bbox->x2 = (box_x + (box_w / 2)) * maxlen;
                                    bbox->y2 = (box_y + (box_h / 2)) * maxlen;
                                }
                            } else {
                                memcpy(bbox, bbox-1, sizeof(struct bounding_box_s));
                            }
                            bbox->score = max_score;
                            bbox->class_num = max_score_class;

                            bbox++;
                            good_box_count++;
                        }
                    }
                }

                //src_p += 4 * all_data;
                 src_p += cell_box_len; // grid_w_bytes_aligned;// -len; // (len - grid_w_len);
            }
            src_p = (int8_t*)OUT_NODE_ADDR(out_p) +  (row+1) * grid_w_bytes_aligned;
        }

        free(dest_p);
    }

    good_result_count = 0;

    for (i = 0; i < class_num; i++) {
        bbox = gp->bboxes_v3;
        class_good_box_count = 0;
        r_tmp_p = result_tmp_p;

        for (j = 0; j < good_box_count; j++) {
            if (bbox->class_num == i) {
                memcpy(r_tmp_p, bbox, sizeof(struct bounding_box_s));
                r_tmp_p++;
                class_good_box_count++;
            }
            bbox++;
        }

        if (class_good_box_count == 1) {
            memcpy(&result_box_p[good_result_count], &result_tmp_p[0], sizeof(struct bounding_box_s));
            good_result_count++;
        } else if (class_good_box_count >= 2) {
            qsort(result_tmp_p, class_good_box_count, sizeof(struct bounding_box_s), box_score_comparator);
            for (j = 0; j < class_good_box_count; j++) {
                if (result_tmp_p[j].score == 0)
                    continue;
                for (k = j + 1; k < class_good_box_count; k++) {
                    if (box_iou(&result_tmp_p[j], &result_tmp_p[k], IOU_UNION) > nms_thresh_yolov3) {
                        result_tmp_p[k].score = 0;
                    }
                }
            }

            int good_count = 0;
            for (j = 0; j < class_good_box_count; j++) {
                if (result_tmp_p[j].score > 0) {
                    memcpy(&result_box_p[good_result_count], &result_tmp_p[j], sizeof(struct bounding_box_s));
                    good_result_count++;
                    good_count++;
                }
                if (YOLO_MAX_DETECTION_PER_CLASS == good_count) {
                    break;
                }
            }
        }
    }

    for (i = 0; i < good_result_count; i++) {
        result->boxes[i].x1 = (int)(result->boxes[i].x1 + (float)0.5) < 0 ? 0 : (int)(result->boxes[i].x1 + (float)0.5);
        result->boxes[i].y1 = (int)(result->boxes[i].y1 + (float)0.5) < 0 ? 0 : (int)(result->boxes[i].y1 + (float)0.5);
        result->boxes[i].x2 = (int)(result->boxes[i].x2 + (float)0.5) > RAW_INPUT_COL(image_p) ? RAW_INPUT_COL(image_p) : (int)(result->boxes[i].x2 + (float)0.5);
        result->boxes[i].y2 = (int)(result->boxes[i].y2 + (float)0.5) > RAW_INPUT_ROW(image_p) ? RAW_INPUT_ROW(image_p) : (int)(result->boxes[i].y2 + (float)0.5);
    }

    printf("good_result_count: %d\n", good_result_count);
    result->box_count = good_result_count;
//    for (i = good_result_count-1; i > -1; i--) {
    for (i = 0; i < good_result_count; i++) {
        printf("post_yolo3[%d] (%d, %d, %d, %d) %f %d\n", i, (int)result->boxes[i].x1, (int)result->boxes[i].y1, (int)result->boxes[i].x2, (int)result->boxes[i].y2, result->boxes[i].score, result->boxes[i].class_num);
    }
    len = good_result_count * sizeof(struct bounding_box_s);

    return len;
}

// yolo v3 for customer models
const float anchers_htw_608_v0[3][2] = {{172,183}, {203,416}, {444,453}};
const float anchers_htw_608_v1[3][2] = {{83,103}, {47,188}, {91,312}};
const float anchers_htw_608_v2[3][2] = {{7,14}, {19,13}, {36,74}};
const float anchers_htw_416_v0[3][2] = {{117,125}, {138,284}, {303,309}};
const float anchers_htw_416_v1[3][2] = {{56,70}, {32,128}, {62,213}};
const float anchers_htw_416_v2[3][2] = {{4,9}, {13,24}, {24,50}};

int post_yolo_custom(int model_id, struct kdp_image_s *image_p)
{
    struct yolo_v3_post_globals_s *gp = &u_globals.yolov3;
    int i, j, k, div, ch, row, col, max_score_class, good_box_count, class_good_box_count, good_result_count, len, len_div_4;
    float box_x, box_y, box_w, box_h, box_confidence, max_score;
    int8_t *src_p, *x_p, *y_p, *width_p, *height_p, *score_p, *class_p, *dest_p;
    struct bounding_box_s *bbox;
    struct yolo_result_s *result;
    struct bounding_box_s *result_box_p, *result_tmp_p, *r_tmp_p;

    if ((model_id != CUSTOMER_MODEL_1) && (model_id != CUSTOMER_MODEL_2)) {
        printf("\nIncorrect model ID\n");
        return -1;
    }
    //printf("\nModel ID = %x\n", model_id);

    int data_size, grid_w, grid_h, grid_c, class_num, grid_w_bytes_aligned;
    uint32_t src_img_mode;

    float maxlen = RAW_INPUT_COL(image_p) > RAW_INPUT_ROW(image_p) ? (float)RAW_INPUT_COL(image_p) : (float)RAW_INPUT_ROW(image_p);

    src_img_mode = RAW_FORMAT(image_p);
    data_size = (POSTPROC_OUTPUT_FORMAT(image_p) & BIT(0)) + 1;     /* 1 or 2 in bytes */

    result = (struct yolo_result_s *)(POSTPROC_RESULT_MEM_ADDR(image_p));
    result_box_p = result->boxes;

    result_tmp_p = gp->result_tmp_s;

    class_num = POSTPROC_OUT_NODE_CH(image_p, 0) / YOLO_V3_CELL_BOX_NUM - YOLO_BOX_FIX_CH;

    result->class_count = class_num;

    bbox = gp->bboxes_v3;
    good_box_count = 0;

    float anchers_v[3][2];
    int idx;
    int offset = sizeof(struct out_node_s);
    struct out_node_s out_p;

    for (idx = 0; idx < POSTPROC_OUTPUT_NUM(image_p); idx++) {
        out_p = (struct out_node_s)(POSTPROC_OUT_NODE(image_p, idx));

        src_p = (int8_t *)POSTPROC_OUT_NODE_ADDR(image_p, idx);

        grid_w = OUT_NODE_COL(out_p);
        grid_h = OUT_NODE_ROW(out_p);
        grid_c = OUT_NODE_CH(out_p);
        len = grid_w * data_size;
        grid_w_bytes_aligned = round_up(len);
        //len = grid_w_bytes_aligned;
        //len_div_4 = len >> 2;
        uint32_t all_data = grid_w_bytes_aligned * (grid_c / 3);
        //dest_p = (int8_t*)malloc(all_data);
        //all_data = all_data >> 2;

        div = OUT_NODE_RADIX(out_p);  // div is unsigned int radix is actually signed
        float div2;
        if (div < 0) {  // div is minus
            div = div * (-1);
            div = pow(2, div);
            div2 = 1 / ((float) div);
        }
        else
	    div2 = (float)OUT_NODE_RADIX(out_p);
        float fScale = OUT_NODE_SCALE(out_p);
        fScale = 1 / (div2 * fScale);

        //For tiny yolo v3 custom models
        if (0 == idx) {
            if (model_id == CUSTOMER_MODEL_1)
                memcpy(anchers_v, anchers_htw_608_v0, 6*sizeof(float));
            else
                memcpy(anchers_v, anchers_htw_416_v0, 6*sizeof(float));
        } else if (1 == idx) {
            if (model_id == CUSTOMER_MODEL_1)
                memcpy(anchers_v, anchers_htw_608_v1, 6*sizeof(float));
            else
                memcpy(anchers_v, anchers_htw_416_v1, 6*sizeof(float));
        }
        else {
            if (model_id == CUSTOMER_MODEL_1)
                memcpy(anchers_v, anchers_htw_608_v2, 6*sizeof(float));
            else
                memcpy(anchers_v, anchers_htw_416_v2, 6*sizeof(float));
        }

        // Divide the data into half as not enough memory for postprocessing of tiny-yolo-v3-608
        uint32_t all_data_half = all_data >> 1;
        //dest_p = (int8_t*)malloc(all_data_half*4);
        dest_p = (int8_t*)malloc(all_data_half);
        //dest_p = (float*)malloc(all_data_half * 4 * 4);
        all_data_half = all_data_half >> 2;
        //int k_end = (grid_w < 8) ? 1 : 2;
        int col_end = 0;
        int cell_box_len = grid_w_bytes_aligned * grid_h * grid_c / 3;

        for (row = 0; row < grid_h; row++) {
            for (ch = 0; ch < YOLO_V3_CELL_BOX_NUM; ch++) {
                x_p = src_p; // dest_p;
                y_p = x_p + grid_w_bytes_aligned * grid_h;
                width_p = y_p + grid_w_bytes_aligned * grid_h;
                height_p = width_p + grid_w_bytes_aligned * grid_h;
                score_p = height_p + grid_w_bytes_aligned * grid_h;
                class_p = score_p + grid_w_bytes_aligned * grid_h;

                col_end = grid_w;
                for (col = 0; col < col_end; col++) {
                    if (data_size == 1) {
                        box_x = (float)*x_p;
                        box_y = (float)*y_p;
                        box_w = (float)*width_p;
                        box_h = (float)*height_p;
                        box_confidence = (float)*score_p;
                    } else {
                        box_x = (float)*(uint16_t *)x_p;
                        box_y = (float)*(uint16_t *)y_p;
                        box_w = (float)*(uint16_t *)width_p;
                        box_h = (float)*(uint16_t *)height_p;
                        box_confidence = (float)*(uint16_t *)score_p;
                    }
                    box_confidence = sigmoid(do_div_scale_2(box_confidence, fScale));

                    /* Get scores of all class */
                    for (i = 0; i < class_num; i++) {
                        if (data_size == 1)
                            gp->box_class_probs[i] = (float)*(class_p + i * grid_w_bytes_aligned*grid_h);
                        else
                            gp->box_class_probs[i] = (float)*(uint16_t *)(class_p + i * grid_w_bytes_aligned*grid_h);
                    }


                    //increase pointer to next position
                    x_p += data_size;
                    y_p += data_size;
                    width_p += data_size;
                    height_p += data_size;
                    score_p += data_size;
                    class_p += data_size;

                    /* Find all classes with score higher than thresh */
                    int done_box = 0;
                    for (i = 0; i < class_num; i++) {
                        max_score_class = -1;

                        max_score = sigmoid(do_div_scale_2(gp->box_class_probs[i], fScale)) * box_confidence;
                        if (max_score >= prob_thresh_yolov3) {
                            max_score_class = i;
                        }
                        if (max_score_class != -1) {
                            if (!done_box) {
                                done_box = 1;
                                box_x = do_div_scale_2(box_x, fScale);
                                box_y = do_div_scale_2(box_y, fScale);
                                box_w = do_div_scale_2(box_w, fScale);
                                box_h = do_div_scale_2(box_h, fScale);

                                box_x = (sigmoid(box_x) + col) / grid_w;
                                box_y = (sigmoid(box_y) + row) / grid_h;
                                box_w = expf(box_w) * anchers_v[ch][0] / DIM_INPUT_COL(image_p);
                                box_h = expf(box_h) * anchers_v[ch][1] / DIM_INPUT_ROW(image_p);

                                if (src_img_mode & (uint32_t)IMAGE_FORMAT_CHANGE_ASPECT_RATIO) {
                                    bbox->x1 = (box_x - (box_w / 2)) * RAW_INPUT_COL(image_p);
                                    bbox->y1 = (box_y - (box_h / 2)) * RAW_INPUT_ROW(image_p);
                                    bbox->x2 = (box_x + (box_w / 2)) * RAW_INPUT_COL(image_p);
                                    bbox->y2 = (box_y + (box_h / 2)) * RAW_INPUT_ROW(image_p);
                                } else {
                                    bbox->x1 = (box_x - (box_w / 2)) * maxlen;
                                    bbox->y1 = (box_y - (box_h / 2)) * maxlen;
                                    bbox->x2 = (box_x + (box_w / 2)) * maxlen;
                                    bbox->y2 = (box_y + (box_h / 2)) * maxlen;
                                }
                            } else {
                                memcpy(bbox, bbox-1, sizeof(struct bounding_box_s));
                            }
                            bbox->score = max_score;
                            bbox->class_num = max_score_class;
                            if (good_box_count < YOLO_GOOD_BOX_MAX)
                                bbox++;
                            else
                                break;
                            good_box_count++;
                        }
                    }
                }

                //src_p += 4 * all_data;
                 src_p += cell_box_len; // grid_w_bytes_aligned;// -len; // (len - grid_w_len);
            }
            src_p = (int8_t*)OUT_NODE_ADDR(out_p) +  (row+1) * grid_w_bytes_aligned;
        }

        free(dest_p);
    }

    good_result_count = 0;
    for (i = 0; i < class_num; i++) {
        bbox = gp->bboxes_v3;
        class_good_box_count = 0;
        r_tmp_p = result_tmp_p;

        for (j = 0; j < good_box_count; j++) {
            if (bbox->class_num == i) {
                memcpy(r_tmp_p, bbox, sizeof(struct bounding_box_s));
                r_tmp_p++;
                class_good_box_count++;
            }
            bbox++;
        }

        if (class_good_box_count == 1) {
            memcpy(&result_box_p[good_result_count], &result_tmp_p[0], sizeof(struct bounding_box_s));
            good_result_count++;
        } else if (class_good_box_count >= 2) {
            qsort(result_tmp_p, class_good_box_count, sizeof(struct bounding_box_s), box_score_comparator);
            for (j = 0; j < class_good_box_count; j++) {
                if (result_tmp_p[j].score == 0)
                    continue;
                for (k = j + 1; k < class_good_box_count; k++) {
                    if (box_iou(&result_tmp_p[j], &result_tmp_p[k], IOU_UNION) > nms_thresh_yolov3) {
                        result_tmp_p[k].score = 0;
                    }
                }
            }

            int good_count = 0;
            for (j = 0; j < class_good_box_count; j++) {
                if (result_tmp_p[j].score > 0) {
                    memcpy(&result_box_p[good_result_count], &result_tmp_p[j], sizeof(struct bounding_box_s));
                    good_result_count++;
                    good_count++;
                }
                if (YOLO_MAX_DETECTION_PER_CLASS == good_count) {
                    break;
                }
            }
        }
    }

    for (i = 0; i < good_result_count; i++) {
        result->boxes[i].x1 = (int)(result->boxes[i].x1 + (float)0.5) < 0 ? 0 : (int)(result->boxes[i].x1 + (float)0.5);
        result->boxes[i].y1 = (int)(result->boxes[i].y1 + (float)0.5) < 0 ? 0 : (int)(result->boxes[i].y1 + (float)0.5);
        result->boxes[i].x2 = (int)(result->boxes[i].x2 + (float)0.5) > RAW_INPUT_COL(image_p) ? RAW_INPUT_COL(image_p) : (int)(result->boxes[i].x2 + (float)0.5);
        result->boxes[i].y2 = (int)(result->boxes[i].y2 + (float)0.5) > RAW_INPUT_ROW(image_p) ? RAW_INPUT_ROW(image_p) : (int)(result->boxes[i].y2 + (float)0.5);
    }

    printf("good_result_count: %d\n", good_result_count);
    result->box_count = good_result_count;
//    for (i = good_result_count-1; i > -1; i--) {
    for (i = 0; i < good_result_count; i++) {
        printf("post_yolo3[%d] (%d, %d, %d, %d) %f %d\n", i, (int)result->boxes[i].x1, (int)result->boxes[i].y1, (int)result->boxes[i].x2, (int)result->boxes[i].y2, result->boxes[i].score, result->boxes[i].class_num);
    }
    len = good_result_count * sizeof(struct bounding_box_s);

    return len;
}

static int inet_comparator(const void *pa, const void *pb)
{
    float a, b;

    a = ((struct imagenet_result_s *)pa)->score;
    b = ((struct imagenet_result_s *)pb)->score;

    return float_comparator(a, b);
}

int post_imgnet_classification(int model_id, struct kdp_image_s *image_p)
{
    struct imagenet_post_globals_s *gp = &u_globals.imgnet;
    uint8_t *result_p;
    int i, len, data_size, div;
    float scale;

    data_size = (POSTPROC_OUTPUT_FORMAT(image_p) & BIT(0)) + 1;     /* 1 or 2 in bytes */

    int8_t *src_p = (int8_t *)POSTPROC_OUT_NODE_ADDR(image_p, 0);
    int grid_w = POSTPROC_OUT_NODE_COL(image_p, 0);
    len = grid_w * data_size;
    int grid_w_bytes_aligned = round_up(len);
    int w_bytes_to_skip = grid_w_bytes_aligned - len;
    len = grid_w_bytes_aligned;

    int ch = POSTPROC_OUT_NODE_CH(image_p, 0);

    /* Convert to float */
    scale = POSTPROC_OUT_NODE_SCALE(image_p, 0);//*(float *)&
    div = 1 << POSTPROC_OUT_NODE_RADIX(image_p, 0);
    for (i = 0; i < ch; i++) {
        gp->temp[i].index = i;
        gp->temp[i].score = (float)*src_p;
        gp->temp[i].score = do_div_scale(gp->temp[i].score, div, scale);
        src_p += data_size + w_bytes_to_skip;
    }

    softmax(gp->temp, ch);
    qsort(gp->temp, ch, sizeof(struct imagenet_result_s), inet_comparator);

    result_p = (uint8_t *)(POSTPROC_RESULT_MEM_ADDR(image_p));
    len = sizeof(struct imagenet_result_s) * IMAGENET_TOP_MAX;
    memcpy(result_p, gp->temp, len);
    return len;
}
