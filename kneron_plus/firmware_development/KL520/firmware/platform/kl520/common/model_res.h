#ifndef __APP_H__
#define __APP_H__

#include "common.h"

/* These header defines structures shared by scpu/ncpu/host_lib */

#define BOXES_MAX_NUM           80
#define LAND_MARK_POINTS        5
#define FR_FEAT_SIZE            256
#define LV_R_SIZE               1
#define DUAL_LAND_MARK_POINTS   10
#define DME_OBJECT_MAX          80
#define IMAGENET_TOP_MAX        5
#define LAND_MARK_MOUTH_POINTS  (4)
#define FD_RES_LENGTH           (2 * 5)

#ifdef MASK_FDR
  #define FR_RES_COUNT      2
#else
  #define FR_RES_COUNT      1
#endif
#ifdef EMBED_CMP_NPU
  #define FR_RES_LENGTH     (FR_RES_COUNT * (16 + FR_FEAT_SIZE + 2)) // wt_hdr + user embedding
#else
  #define FR_RES_LENGTH     (FR_RES_COUNT * (4 * FR_FEAT_SIZE))
#endif
#define LM_RES_LENGTH       (4 * LAND_MARK_POINTS)
#define LV_RES_LENGTH       4
#define SCORE_RES_LENGTH    4
#define MAX_PLATE_STR_LEN   10
#define OCR_MAX_NUM         7

#define MAX_YOLO_FACE_LANDMARK_CNT 4 * 2 // (x and y) * max point count

/* Yolo Result */
struct bounding_box_s {
    float x1;      // top-left corner: x
    float y1;      // top-left corner: y
    float x2;      // bottom-right corner: x
    float y2;      // bottom-right corner: y
    float score;   // probability score
    int32_t class_num; // class # (of many) with highest probability
};

struct bounding_box_landmark_s {
    float x1;      // top-left corner: x
    float y1;      // top-left corner: y
    float x2;      // bottom-right corner: x
    float y2;      // bottom-right corner: y
    float score;   // probability score
    int32_t class_num; // class # (of many) with highest probability
    float lm[MAX_YOLO_FACE_LANDMARK_CNT];  // landmark data
};

struct yolo_result_s {
    uint32_t class_count;            // total class count
    uint32_t box_count;              // boxes of all classes
    struct bounding_box_s boxes[BOXES_MAX_NUM];  // box_count
};

struct yolo_and_landmark_result_s {
    uint32_t class_count;            // total class count
    uint32_t box_count;              // boxes of all classes
    struct bounding_box_landmark_s boxes[BOXES_MAX_NUM];  // box_count
};

struct age_gender_result_s {
    uint32_t age;
    uint32_t ismale;
};

struct imagenet_result_s {
    int32_t   index; // index of the class
    float score; // probability score of the class
};

struct facedet_result_s {
#ifdef LARRY_UPDATE_0905
    int32_t len;
#endif
    int32_t xywh[4]; // 4 values for X, Y, W, H
    int32_t class_num; // masked 2 / unmasked 1
};

struct landmark_result_s {
    struct {
        uint32_t x;
        uint32_t y;
    } marks[LAND_MARK_POINTS];
    float score;
    float blur;
    int32_t class_num;
};

struct fr_result_s {
    float feature_map[FR_FEAT_SIZE];
    int8_t feature_map_fixed[FR_FEAT_SIZE];
};

struct lv_result_s{
    float real[LV_R_SIZE];
};

struct dual_landmarks_s {
    struct {
        uint32_t x;
        uint32_t y;
    } marks[DUAL_LAND_MARK_POINTS];
};

struct fd_landmark_result_s {
    struct bounding_box_s boxes;
    struct {
        uint32_t x;
        uint32_t y;
    } marks[LAND_MARK_POINTS];
    float score;
    float blur;
};

typedef struct {
    struct bounding_box_s fd_res;
    struct age_gender_result_s ag_res;
} fd_age_gender_res;

struct fd_age_gender_s {
    uint32_t count;
    fd_age_gender_res boxes[DME_OBJECT_MAX];
};

typedef struct {
    uint32_t class_count; // total class count
    uint32_t box_count;   // boxes of all classes
    struct bounding_box_s boxes[DME_OBJECT_MAX]; // box information
} dme_res;

typedef struct {
    uint32_t class_count; // total class count
    uint32_t box_count;   // boxes of all classes
    struct fd_landmark_result_s fd_lm_res[DME_OBJECT_MAX]; // box and landmark information
} fd_lm_res;

enum mouth {
    MOUTH_T = 0,
    MOUTH_B,
    MOUTH_L,
    MOUTH_R,
};

struct yawning_result_s {
    struct {
        float x;
        float y;
        } marks[LAND_MARK_MOUTH_POINTS]; // mouth 0:top  1:bottom  2: left 3:right
    uint32_t isYawning;
    float yawning_rate;
};

struct fd_yawning_result_s {
    struct bounding_box_s boxes;
    struct yawning_result_s yawning;
};

typedef struct {
    uint32_t count;   // boxes of all classes
    struct fd_yawning_result_s fd_yawning_res[DME_OBJECT_MAX]; // box and landmark information
} fd_yd_res;

#define CLASSIFIER_MAX_NUM 1000
struct classifier_result_s {
    float score[CLASSIFIER_MAX_NUM];    // score for each class index
};

struct yolov5_face_result_s {
    struct bounding_box_s box;
    uint32_t mark_num;
    struct {
        uint32_t x;
        uint32_t y;
    } marks[1];
};

#endif
