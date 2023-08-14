/*
 * Kneron Example Post-Processing driver
 *
 * Copyright (C) 2018-2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "base.h"
#include "model_res.h"
#include "post_processing.h"

#define YOLO_CLASS_MAX          80                                  /* max result box number per class */
#define YOLO_GOOD_BOX_MAX       80                                  /* max result box number for one time inference */
#define YOLO_BOX_FIX_CH         5                                   /* x, y, w, h, confidence score */

#define YOLO_V3_GRID_W          14                                  /* max output feature map width */
#define YOLO_V3_GRID_H          14                                  /* max output feature map higheit */
#define YOLO_V3_GRID_MAX        (YOLO_V3_GRID_W * YOLO_V3_GRID_H)   /* max predict box number per channel */
#define YOLO_V3_CELL_BOX_NUM    3                                   /* number of anchors on each output node */
#define YOLO_V3_MAX_BOX_NUM     MIN(500, YOLO_V3_GRID_MAX * YOLO_V3_CELL_BOX_NUM)

#define KDP_COL_MIN             16                                  /* hardware 16 bytes alignment, i.e. 128 bits */

/* YOLO default parameters */
const float ex_unpass_score = -999.0;       // used as box filter

const float ex_anchors_v0[3][2] = {{116 ,90}, {156, 198}, {373, 326}};
const float ex_anchors_v1[3][2] = {{30, 61}, {62, 45}, {59, 119}};
const float ex_anchors_v2[3][2] = {{10, 13}, {16, 30}, {33, 23}};

/* Output node layout  */
struct ex_output_node {
    int8_t *base_ptr;
    uint32_t ch;
    uint32_t row;
    uint32_t col;
    uint32_t col_len;
    uint32_t radix;
    uint32_t scale;
};

/* Shared global variable area among models */
struct ex_yolo_v3_post_globals_s {
    float box_class_probs[YOLO_CLASS_MAX];
    struct bounding_box_s bboxes_v3[YOLO_V3_GRID_MAX * YOLO_V3_CELL_BOX_NUM];
    struct bounding_box_s result_tmp_s[YOLO_V3_MAX_BOX_NUM];
};

/* Model globals */
static struct ex_yolo_v3_post_globals_s *ex_yolov3_gp;

void *get_gp(void **gp, size_t len);

static inline struct ex_yolo_v3_post_globals_s *get_yolov3_gp(void) {
    return (struct ex_yolo_v3_post_globals_s *)get_gp((void **)&ex_yolov3_gp, sizeof(struct ex_yolo_v3_post_globals_s));
}

/* Post-Processing utils functions */
float ex_do_div_scale_optim(float v, float scale) {
    return (v * scale);
}

uint32_t ex_round_up(uint32_t num) {
    return ((num + (KDP_COL_MIN - 1)) & ~(KDP_COL_MIN - 1));
}

float ex_sigmoid(float x) {
    float exp_value;
    float return_value;

    exp_value = expf(-x);

    return_value = 1 / (1 + exp_value);

    return return_value;
}

int ex_float_comparator(float a, float b) {
    float diff = a - b;

    if (diff < 0)
        return 1;
    else if (diff > 0)
        return -1;
    return 0;
}

int ex_box_score_comparator(const void *pa, const void *pb) {
    float a, b;

    a = ((struct bounding_box_s *) pa)->score;
    b = ((struct bounding_box_s *) pb)->score;

    return ex_float_comparator(a, b);
}

float ex_overlap(float l1, float r1, float l2, float r2) {
    float left = l1 > l2 ? l1 : l2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

float ex_box_intersection(struct bounding_box_s *a, struct bounding_box_s *b) {
    float w, h, area;

    w = ex_overlap(a->x1, a->x2, b->x1, b->x2);
    h = ex_overlap(a->y1, a->y2, b->y1, b->y2);

    if (w < 0 || h < 0)
        return 0;

    area = w * h;
    return area;
}

float ex_box_union(struct bounding_box_s *a, struct bounding_box_s *b) {
    float i, u;

    i = ex_box_intersection(a, b);
    u = (a->y2 - a->y1) * (a->x2 - a->x1) + (b->y2 - b->y1) * (b->x2 - b->x1) - i;

    return u;
}

float ex_box_iou(struct bounding_box_s *a, struct bounding_box_s *b) {
    /* origin iou */

    float c;
    float intersection_a_b = ex_box_intersection(a, b);
    float union_a_b = ex_box_union(a, b);

    c = intersection_a_b / union_a_b;

    return c;
}

/* Get the output node information */
void ex_get_output_node(struct ex_output_node *out_node, struct kdp_image_s *image_p, int node_num) {
    struct out_node_s *out_p;
    out_p = (struct out_node_s *)((kdp_size_t)POSTPROC_OUT_NODE(image_p) + node_num * sizeof(struct out_node_s));

    out_node->base_ptr = (int8_t *)OUT_NODE_ADDR(out_p);
    out_node->ch = OUT_NODE_CH(out_p);
    out_node->row = OUT_NODE_ROW(out_p);
    out_node->col = OUT_NODE_COL(out_p);
    out_node->col_len = ex_round_up(out_node->col);
    out_node->radix = OUT_NODE_RADIX(out_p);
    out_node->scale = OUT_NODE_SCALE(out_p);
}

/* Get the index corresponding to given channel, row, and column indices */
uint32_t ex_get_index(struct ex_output_node node, uint32_t ch_idx, uint32_t row_idx, uint32_t col_idx) {
    uint32_t index = row_idx * node.ch * node.col_len  + ch_idx * node.col_len + col_idx;
    return index;
}

/* Get the data pointer corresponding to given channel, row, and column indices */
int8_t *ex_get_data(struct ex_output_node node, uint32_t ch_idx, uint32_t row_idx, uint32_t col_idx) {
    uint32_t index = ex_get_index(node, ch_idx, row_idx, col_idx);
    return node.base_ptr + index;
}

/* Performs NMS on the potential boxes */
static int ex_nms_bbox_for_post_yolov3_no_sigmoid(struct bounding_box_s *potential_boxes,
                                                  struct bounding_box_s *temp_results,
                                                  int class_num,
                                                  int good_box_count,
                                                  int max_boxes,
                                                  int single_class_max_boxes,
                                                  struct bounding_box_s *results,
                                                  float score_thresh,
                                                  float iou_thresh) {
    int good_result_count = 0;

    // check overlap between only boxes from same class
    for (int i = 0; i < class_num; i++) {
        int class_good_result_count = 0;
        if (good_result_count == max_boxes) // break out of outer loop as well for future classes
            break;

        int class_good_box_count = 0;

        // find all boxes of a specific class
        for (int j = 0; j < good_box_count; j++) {
            if (potential_boxes[j].class_num == i) {
                memcpy(&temp_results[class_good_box_count], &potential_boxes[j], sizeof(struct bounding_box_s));
                class_good_box_count++;
            }
        }

        if (class_good_box_count == 1) {
            memcpy(&results[good_result_count], temp_results, sizeof(struct bounding_box_s));
            good_result_count++;
        } else if (class_good_box_count >= 2) {
            // sort boxes based on the score
            qsort(temp_results, class_good_box_count, sizeof(struct bounding_box_s), ex_box_score_comparator);
            for (int j = 0; j < class_good_box_count; j++) {
                // if the box score is too low or is already filtered by previous box
                if (temp_results[j].score < score_thresh)
                    continue;

                // filter out overlapping, lower score boxes
                for (int k = j + 1; k < class_good_box_count; k++)
                    if (ex_box_iou(&temp_results[j], &temp_results[k]) > iou_thresh)
                        temp_results[k].score = ex_unpass_score;

                // keep boxes with highest scores, up to a certain amount
                if ((good_result_count == max_boxes) || (class_good_result_count == single_class_max_boxes))
                    break;
                memcpy(&results[good_result_count], &temp_results[j], sizeof(struct bounding_box_s));
                good_result_count++;
                class_good_result_count++;
            }
        }
    }

    return good_result_count;
}

/**
 * Update candidate bbox list, reserve top max_candidate_num candidate bbox.
 */
static int ex_update_candidate_bbox_list(struct bounding_box_s *new_candidate_bbox,
                                         int max_candidate_num,
                                         struct bounding_box_s *candidate_bbox_list,
                                         int *candidate_bbox_num,
                                         int *max_candidate_idx,
                                         int *min_candidate_idx) {

    if ((NULL == new_candidate_bbox) || (NULL == candidate_bbox_list))
        return -1;

    int update_idx = -1;

    if (0 == *candidate_bbox_num) {
        /** add 1-th bbox */
        *max_candidate_idx = 0;
        *min_candidate_idx = 0;
        update_idx = 0;
        (*candidate_bbox_num)++;
        memcpy(&candidate_bbox_list[update_idx], new_candidate_bbox, sizeof(struct bounding_box_s));
    } else {
        if (max_candidate_num > *candidate_bbox_num) {
            /** directly add bbox when the candidate bbox list is not filled */
            update_idx = *candidate_bbox_num;

            if (new_candidate_bbox->score > candidate_bbox_list[*max_candidate_idx].score)
                *max_candidate_idx = update_idx;
            else if (new_candidate_bbox->score < candidate_bbox_list[*min_candidate_idx].score)
                *min_candidate_idx = update_idx;

            (*candidate_bbox_num)++;

            if (0 <= update_idx)
                memcpy(&candidate_bbox_list[update_idx], new_candidate_bbox, sizeof(struct bounding_box_s));
        } else {
            /** update candidate bbox list when candidate bbox list is filled */
            if (new_candidate_bbox->score >= candidate_bbox_list[*max_candidate_idx].score) {
                /** update the largest score candidate index */
                update_idx = *min_candidate_idx;
                *max_candidate_idx = *min_candidate_idx;
            } else if (new_candidate_bbox->score > candidate_bbox_list[*min_candidate_idx].score) {
                update_idx = *min_candidate_idx;
            }

            if (0 <= update_idx) {
                memcpy(&candidate_bbox_list[update_idx], new_candidate_bbox, sizeof(struct bounding_box_s));

                for (int i = 0; i < *candidate_bbox_num; i++) {
                    /** update the smallest score candidate index */
                    if (candidate_bbox_list[i].score < candidate_bbox_list[*min_candidate_idx].score)
                        *min_candidate_idx = i;
                }
            }
        }
    }

    return 0;
}

/* Remap one bounding box to original image coordinates */
void ex_remap_bbox(struct kdp_image_s *image_p, struct bounding_box_s *box, int need_scale) {
    // original box values are percentages, scale to model sizes
    if (need_scale) {
        box->x1 *= DIM_INPUT_COL(image_p);
        box->y1 *= DIM_INPUT_ROW(image_p);
        box->x2 *= DIM_INPUT_COL(image_p);
        box->y2 *= DIM_INPUT_ROW(image_p);
    }

    // scale from model sizes to original input sizes
    box->x1 = (box->x1 - RAW_PAD_LEFT(image_p)) * RAW_SCALE_WIDTH(image_p) + RAW_CROP_LEFT(image_p);
    box->y1 = (box->y1 - RAW_PAD_TOP(image_p)) * RAW_SCALE_HEIGHT(image_p) + RAW_CROP_TOP(image_p);
    box->x2 = (box->x2 - RAW_PAD_LEFT(image_p)) * RAW_SCALE_WIDTH(image_p) + RAW_CROP_LEFT(image_p);
    box->y2 = (box->y2 - RAW_PAD_TOP(image_p)) * RAW_SCALE_HEIGHT(image_p) + RAW_CROP_TOP(image_p);

    // clip to boundaries of image
    box->x1 = (int)((box->x1 < 0 ? 0 : box->x1) + (float)0.5);
    box->y1 = (int)((box->y1 < 0 ? 0 : box->y1) + (float)0.5);
    box->x2 = (int)((box->x2 > RAW_INPUT_COL(image_p) ? RAW_INPUT_COL(image_p) : box->x2) + (float)0.5);
    box->y2 = (int)((box->y2 > RAW_INPUT_ROW(image_p) ? RAW_INPUT_ROW(image_p) : box->y2) + (float)0.5);
}

/* YOLO parameters */
static float iou_threshold = 0.45;
static float score_threshold = 0.6;
static uint32_t max_detection_box_num = YOLO_V3_MAX_BOX_NUM;
static uint32_t anchors[3][3][2] = {{{0}}};

/* User YOLO post processing */
int user_post_yolo(struct kdp_image_s *image_p)
{
    /************************* Input parameters ******************************/
    host_od_post_params_t *pHostParam = (host_od_post_params_t *)POSTPROC_PARAMS_P(image_p);

    if (pHostParam->prob_thresh > 0)
        score_threshold = pHostParam->prob_thresh;

    if (pHostParam->nms_thresh > 0)
        iou_threshold = pHostParam->nms_thresh;

    if (pHostParam->max_detection_per_class > 0)
    {
        max_detection_box_num = pHostParam->max_detection_per_class;
        if (max_detection_box_num > YOLO_V3_MAX_BOX_NUM)
            max_detection_box_num = YOLO_V3_MAX_BOX_NUM;
    }

    // use passed anchor table
    uint32_t *p_anchors = (uint32_t *)pHostParam->data;
    if (pHostParam->anchor_row * pHostParam->anchor_col > 0 && pHostParam->anchor_row <= 3 && pHostParam->anchor_col <= 6)
    {
        for (int i = 0; i < pHostParam->anchor_row; i++)
        {
            for (int j = 0; j < (pHostParam->anchor_col / 2); j++)
            {
                anchors[i][j][0] = *p_anchors++;
                anchors[i][j][1] = *p_anchors++;
            }
        }
    }
    else
    {
        memcpy(anchors[0], ex_anchors_v0, sizeof(float) * 6);
        memcpy(anchors[1], ex_anchors_v1, sizeof(float) * 6);
        memcpy(anchors[2], ex_anchors_v2, sizeof(float) * 6);
    }

    /*************************************************************************/

    // get result buffer
    struct yolo_result_s *result = (struct yolo_result_s *)(POSTPROC_RESULT_MEM_ADDR(image_p));
    struct ex_yolo_v3_post_globals_s *gp = get_yolov3_gp();
    struct bounding_box_s *bbox = gp->bboxes_v3;
    struct ex_output_node node_yolo;
    int good_box_count = 0;
    int max_candidate_idx = 0;
    int min_candidate_idx = 0;

    ex_get_output_node(&node_yolo, image_p, 0);
    int class_num = node_yolo.ch / YOLO_V3_CELL_BOX_NUM - YOLO_BOX_FIX_CH;
    result->class_count = class_num;

    for (int idx = 0; idx < POSTPROC_OUTPUT_NUM(image_p); idx++) {
        // get output node parameters
        ex_get_output_node(&node_yolo, image_p, idx);

        // get radix and scale for floating conversion
        float div = pow(2, node_yolo.radix);
        float scale = *(float *)&node_yolo.scale;

        // convert threshold to fp for fast comparison
        int prob_thresh_yolov3_fp = floor(-log(1.f / score_threshold - 1.f) * div * scale);
        scale = 1.0f / (div * scale);

        for (int ch = 0; ch < YOLO_V3_CELL_BOX_NUM; ch++) {
            for (int row = 0; row < node_yolo.row; row++) {
                for (int col = 0; col < node_yolo.col; col++) {
                    // check if the score (4th channel) better than threshold
                    int8_t box_confidence_fp = *ex_get_data(node_yolo, ch * (class_num + 5) + 4, row, col);

                    // filter out small box score
                    if (box_confidence_fp <= prob_thresh_yolov3_fp)
                        continue;

                    // find maximum score among all classes
                    // get the predicted class and score in fixed
                    int max_score_class = 0;
                    int8_t max_score_int = *ex_get_data(node_yolo, ch * (class_num + YOLO_BOX_FIX_CH) + 5, row, col);
                    for (int i = 1; i < class_num; i++) {
                        int8_t cur_score = *ex_get_data(node_yolo, ch * (class_num + YOLO_BOX_FIX_CH) + 5 + i, row, col);
                        if (cur_score > max_score_int) {
                            max_score_int = cur_score;
                            max_score_class = i;
                        }
                    }

                    // filter out small class number
                    if (max_score_int <= prob_thresh_yolov3_fp)
                        continue;

                    // get the confidence score in floating
                    float box_confidence = ex_sigmoid(ex_do_div_scale_optim(box_confidence_fp, scale));
                    float max_score = ex_sigmoid(ex_do_div_scale_optim(max_score_int, scale));
                    float score = max_score * box_confidence;

                    // check if score is larger than threshold we set in floating
                    if (score > score_threshold) {
                        if ((YOLO_V3_MAX_BOX_NUM == good_box_count) && (score <= bbox[min_candidate_idx].score))
                            continue;
                        float box_x = (float)*ex_get_data(node_yolo, ch * (class_num + YOLO_BOX_FIX_CH), row, col);
                        float box_y = (float)*ex_get_data(node_yolo, ch * (class_num + YOLO_BOX_FIX_CH) + 1, row, col);
                        float box_w = (float)*ex_get_data(node_yolo, ch * (class_num + YOLO_BOX_FIX_CH) + 2, row, col);
                        float box_h = (float)*ex_get_data(node_yolo, ch * (class_num + YOLO_BOX_FIX_CH) + 3, row, col);

                        box_x = ex_do_div_scale_optim(box_x, scale);
                        box_y = ex_do_div_scale_optim(box_y, scale);
                        box_w = ex_do_div_scale_optim(box_w, scale);
                        box_h = ex_do_div_scale_optim(box_h, scale);

                        box_x = (ex_sigmoid(box_x) + col) * (DIM_INPUT_COL(image_p) / node_yolo.col);
                        box_y = (ex_sigmoid(box_y) + row) * (DIM_INPUT_ROW(image_p) / node_yolo.row);
                        box_w = expf(box_w) * anchors[idx][ch][0];
                        box_h = expf(box_h) * anchors[idx][ch][1];

                        struct bounding_box_s new_candidate_bbox = {0};
                        new_candidate_bbox.x1 = (box_x - (box_w / 2));
                        new_candidate_bbox.y1 = (box_y - (box_h / 2));
                        new_candidate_bbox.x2 = (box_x + (box_w / 2));
                        new_candidate_bbox.y2 = (box_y + (box_h / 2));

                        new_candidate_bbox.score = score;
                        new_candidate_bbox.class_num = max_score_class;

                        ex_update_candidate_bbox_list(&new_candidate_bbox,
                                                      YOLO_V3_MAX_BOX_NUM,
                                                      bbox,
                                                      &good_box_count,
                                                      &max_candidate_idx,
                                                      &min_candidate_idx);
                    }
                }
            }
        }
    }

    // do NMS
    result->box_count = ex_nms_bbox_for_post_yolov3_no_sigmoid(gp->bboxes_v3,
                                                               gp->result_tmp_s,
                                                               class_num,
                                                               good_box_count,
                                                               max_detection_box_num,
                                                               max_detection_box_num,
                                                               result->boxes,
                                                               0,
                                                               iou_threshold);

    // remap boxes to original coordinates
    for (int i = 0; i < result->box_count; i++)
        ex_remap_bbox(image_p, &result->boxes[i], 0);

    return result->box_count;
}
