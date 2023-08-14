/*
 * Utility function headers for the postprocess functions.
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */
#ifndef POST_UTILS_H
#define POST_UTILS_H

#include <stdlib.h>

#include "base.h"
#include "model_res.h"

#define YOLO_MAX_DETECTION_PER_CLASS    (20)
#define YOLO_PROB_THRESH_POS            (0)

#define YOLO_GRID_W             10
#define YOLO_GRID_H             10
#define YOLO_GRID_MAX           (YOLO_GRID_W * YOLO_GRID_H)
#define YOLO_CELL_BOX_NUM       5
#define YOLO_CLASS_MAX          80
#ifdef YOLO_GOOD_BOX_MAX
#undef YOLO_GOOD_BOX_MAX
#endif
#define YOLO_GOOD_BOX_MAX       100 /* sync the setting in kp_struct.h */
#define YOLO_BOX_FIX_CH         5   /* x, y, w, h, confidence score */

#define YOLO_V3_O1_GRID_W       (7)
#define YOLO_V3_O1_GRID_H       (7)
#define YOLO_V3_O1_GRID_MAX     (YOLO_V3_O1_GRID_W * YOLO_V3_O1_GRID_H)
#define YOLO_V3_O2_GRID_W       (14)
#define YOLO_V3_O2_GRID_H       (14)
#define YOLO_V3_O2_GRID_MAX     (YOLO_V3_O2_GRID_W * YOLO_V3_O2_GRID_H)
#define YOLO_V3_CELL_BOX_NUM    (3)
#define YOLO_V3_MAX_BOX_NUM     MIN(500, MAX(YOLO_V3_O1_GRID_MAX, YOLO_V3_O2_GRID_MAX) * YOLO_V3_CELL_BOX_NUM)

#define YOLO_V5_O0_GRID_W               (60)
#define YOLO_V5_O0_GRID_H               (32)
#define YOLO_V5_O0_GRID_MAX             (YOLO_V5_O0_GRID_W * YOLO_V5_O0_GRID_H)
#define YOLO_V5_O1_GRID_W               (30)
#define YOLO_V5_O1_GRID_H               (16)
#define YOLO_V5_O1_GRID_MAX             (YOLO_V5_O1_GRID_W * YOLO_V5_O1_GRID_H)
#define YOLO_V5_O2_GRID_W               (15)
#define YOLO_V5_O2_GRID_H               (8)
#define YOLO_V5_O2_GRID_MAX             (YOLO_V5_O2_GRID_W * YOLO_V5_O2_GRID_H)
#define YOLO_V5_CELL_BOX_NUM            (3)
#define YOLO_V5_MAX_BOX_NUM             MIN(500, MAX(MAX(YOLO_V5_O1_GRID_MAX, YOLO_V5_O2_GRID_MAX), YOLO_V5_O0_GRID_MAX) * YOLO_V5_CELL_BOX_NUM)
#define YOLO_V5_FACE_MAX_BOX_NUM        MIN(100, MAX(MAX(YOLO_V5_O1_GRID_MAX, YOLO_V5_O2_GRID_MAX), YOLO_V5_O0_GRID_MAX) * YOLO_V5_CELL_BOX_NUM)

#define KDP_COL_MIN             16 /* Bytes, i.e. 128 bits */

#define NMS_ALL_CLASS       (0)
#define NMS_GROUP_CLASS     (1)
#define NMS_SINGLE_CLASS    (2)

// thresholds for various solutions
// prob_thresh is the score threshold
// nms_thresh is the iou threshold
extern float nms_thresh_od_mbssd;
extern float nms_thresh_ssd;
extern float nms_thresh_yolov3;
extern float nms_thresh_yolov5;
extern float prob_thresh_od_mbssd;
extern float prob_thresh_ssd;
extern float prob_thresh_yolov3;
extern float prob_thresh_yolov5;

extern const float anchors_yolov3_v2[3][2];
extern const float anchors_yolov3_v1[3][2];
extern const float anchors_yolov3_v0[3][2];

extern const float anchors_yolov5_v2[3][2];
extern const float anchors_yolov5_v1[3][2];
extern const float anchors_yolov5_v0[3][2];

extern const float anchors_yolov_face_v2[3][2];
extern const float anchors_yolov_face_v1[3][2];
extern const float anchors_yolov_face_v0[3][2];

// how a output node layout
struct output_node {
    int8_t *base_ptr;
    uint32_t ch;
    uint32_t row;
    uint32_t col;
    uint32_t col_len;
    uint32_t radix;
    uint32_t scale;
    uint32_t format;
};

/* Shared global variable area among models */
struct yolo_post_globals_s {
    float box_class_probs[YOLO_CLASS_MAX];
    struct bounding_box_s bboxes[YOLO_GRID_MAX * YOLO_CELL_BOX_NUM];
    struct bounding_box_s result_tmp_s[YOLO_GOOD_BOX_MAX];
};

struct yolo_v3_post_globals_s {
    float box_class_probs[YOLO_CLASS_MAX];
    struct bounding_box_s bboxes_v3[YOLO_V3_MAX_BOX_NUM];
    struct bounding_box_s result_tmp_s[YOLO_V3_MAX_BOX_NUM];
};

struct yolo_v5_post_globals_s {
    struct bounding_box_s bboxes[YOLO_V5_MAX_BOX_NUM];
    struct bounding_box_s result_tmp_s[YOLO_V5_MAX_BOX_NUM];
};

struct yolo_face_post_globals_s{
    struct bounding_box_landmark_s bboxes[YOLO_V5_FACE_MAX_BOX_NUM];
    struct bounding_box_landmark_s result_tmp_s[YOLO_V5_FACE_MAX_BOX_NUM];
};

struct yolo_x_post_globals_s
{
    struct bounding_box_landmark_plus_s bboxes[YOLO_V5_FACE_MAX_BOX_NUM];
    struct bounding_box_landmark_plus_s result_tmp_s[YOLO_V5_FACE_MAX_BOX_NUM];
};

typedef struct {
    float *sum;
    float *scores;
} post_softmax_result;

/* Model globals */
static struct yolo_post_globals_s        *yolo_gp;
static struct yolo_v3_post_globals_s     *yolov3_gp;
static struct yolo_v5_post_globals_s     *yolov5_gp;
static struct yolo_face_post_globals_s   *yolo_face_gp;
static struct yolo_x_post_globals_s      *yolox_gp;

void *get_gp(void **gp, size_t len);
static inline struct yolo_post_globals_s *get_yolo_gp(void) {
    return (struct yolo_post_globals_s *) get_gp((void**) &yolo_gp, sizeof(struct yolo_post_globals_s));
}

static inline struct yolo_v3_post_globals_s *get_yolov3_gp(void) {
    return (struct yolo_v3_post_globals_s *)get_gp((void **)&yolov3_gp, sizeof(struct yolo_v3_post_globals_s));
}

static inline struct yolo_v5_post_globals_s *get_yolov5_gp() {
    return (struct yolo_v5_post_globals_s *)get_gp((void **)&yolov5_gp, sizeof(struct yolo_v5_post_globals_s));
}

static inline struct yolo_face_post_globals_s *get_yolov_face_gp() {
    return (struct yolo_face_post_globals_s *)get_gp((void **)&yolo_face_gp, sizeof(struct yolo_face_post_globals_s));
}

static inline struct yolo_x_post_globals_s *get_yolox_gp() {
    return (struct yolo_x_post_globals_s *)get_gp((void **)&yolox_gp, sizeof(struct yolo_x_post_globals_s));
}

// conversion functions
float do_div_scale(float v, int div, float scale);
float do_div_scale_optim(float v, float scale);
uint32_t round_up(uint32_t num);
uint32_t round_up_with_num(uint32_t num, uint32_t round_num);

float clamp(float x);
float relu(float in);
float sigmoid(float x);

// bounding box helpers
int float_comparator(float a, float b);
int box_score_comparator(const void *pa, const void *pb);
int box_lm_score_comparator(const void *pa, const void *pb);
float overlap(float l1, float r1, float l2, float r2);
float box_intersection(struct bounding_box_s *a, struct bounding_box_s *b);
float box_union(struct bounding_box_s *a, struct bounding_box_s *b);
float box_iou(struct bounding_box_s *a, struct bounding_box_s *b);
float box_area(struct bounding_box_s *box);
float box_lm_area(struct bounding_box_landmark_s *box);

// functions to get parameters from 520/720 specific structures to common structure
int8_t *get_data(struct output_node node, uint32_t ch_idx, uint32_t row_idx, uint32_t col_idx);
void get_output_node(struct output_node *out_node, struct kdp_image_s *image_p, int node_num);

// build anchors to map bounding boxes
float *build_fd_ssd_anchors(struct kdp_image_s *image_p);
float *build_od_ssd_anchors(struct kdp_image_s *image_p);
float *build_vd_ssd_anchors(struct kdp_image_s *image_p);

// more bounding box helpers
int nms_bbox(struct bounding_box_s *potential_boxes, struct bounding_box_s *temp_results,
             int class_num, int good_box_count, int max_boxes, struct bounding_box_s *results,
             float score_thresh, float iou_thresh, float nms_mode);
int nms_bbox_lm(struct bounding_box_landmark_s *potential_boxes, struct bounding_box_landmark_s *temp_results,
                int class_num, int good_box_count, int max_boxes, struct bounding_box_landmark_s *results,
                float score_thresh, float iou_thresh, float nms_mode);
int nms_bbox_lm_plus(struct bounding_box_landmark_plus_s *potential_boxes, struct bounding_box_landmark_plus_s *temp_results,
                     int class_num, int good_box_count, int max_boxes, struct bounding_box_landmark_plus_s *results,
                     float score_thresh, float iou_thresh, float nms_mode);
void remap_bbox(struct kdp_image_s *image_p, int index, struct bounding_box_s *box, int need_scale);
void remap_bbox_lm_plus(struct kdp_image_s *image_p, int index, struct bounding_box_landmark_plus_s *box, int need_scale);

typedef int (* post_proc_func)(float x,void *feature_out,struct output_node node,int row_idx, int ch_idx,int  col_idx);
int do_op_for_all_element(struct output_node,void *feature_out,post_proc_func func);
int post_func_sigmoid(float x,void *feature_out,struct output_node  node,int row_idx, int ch_idx,int  col_idx);
int post_func_convert_flat_float(float x,void *feature_out,struct output_node  node,int row_idx, int ch_idx,int  col_idx);
int post_func_sum(float x,void *feature_out,struct output_node node,int row_idx, int ch_idx,int  col_idx);
int post_func_softmax(float x,void *feature_out,struct output_node node,int row_idx, int ch_idx,int  col_idx);

#endif

