#ifndef __APP_H__
#define __APP_H__

#include <stdint.h>

/* Porting from E2E platform */

#define BOXES_MAX_NUM 80
#define CLASSIFIER_MAX_NUM 1000
#define FR_FEATURE_MAP_SIZE 256
#define KEYPOINT_POINTS 11
#define LANDMARK_POINTS 5
#define MAX_LANDMARK_POINTS 5
//#define OCR_MAX_NUM 7
#define SEG_WIDTH 80
#define SEG_HEIGHT 60


/* These header defines structures shared by scpu/ncpu/host_lib */

#define LAND_MARK_POINTS       5
#define FR_FEAT_SIZE           256
#define LV_R_SIZE              1
#define DUAL_LAND_MARK_POINTS  10
#define DME_OBJECT_MAX         80
#define IMAGENET_TOP_MAX       5

#define FD_RES_LENGTH  (2 * 4)
#define FR_RES_LENGTH  (4 * FR_FEAT_SIZE)
#define LM_RES_LENGTH  (4 * LAND_MARK_POINTS)
#define LV_RES_LENGTH  4
#define SCORE_RES_LENGTH  4
#define MAX_CARS_DETECTED  20
#define MAX_CAR_PLATES     MAX_CARS_DETECTED
#define MAX_PLATE_STR_LEN  20
#define OCR_MAX_NUM 20

#define MAX_YOLO_FACE_LANDMARK_CNT 4 * 2 // (x and y) * max point count

#define EYE_LID_LM_POINTS      7
#define LV_SCORE_SIZE          2
/* Yolo Result */
typedef struct bounding_box_s {
    float x1;      // top-left corner: x
    float y1;      // top-left corner: y
    float x2;      // bottom-right corner: x
    float y2;      // bottom-right corner: y
    float score;   // probability score
    int32_t class_num; // class # (of many) with highest probability
} bounding_box_t;

struct bounding_box_landmark_s {
    float x1;      // top-left corner: x
    float y1;      // top-left corner: y
    float x2;      // bottom-right corner: x
    float y2;      // bottom-right corner: y
    float score;   // probability score
    int32_t class_num; // class # (of many) with highest probability
    float lm[MAX_YOLO_FACE_LANDMARK_CNT];  // landmark data
};

struct bounding_box_landmark_plus_s {
    float x1;                               // top-left x corner
    float y1;                               // top-left y corner
    float x2;                               // bottom-right x corner
    float y2;                               // bottom-right y corner
    float score;                            // probability score
    int32_t class_num;                      // class number (of many) with highest probability
    float score_next;                       // second best probability score
    int32_t class_num_next;                 // class number with second highest probability
    float lm[MAX_YOLO_FACE_LANDMARK_CNT];   // landmark data
};

typedef struct yolo_result_s {
    uint32_t class_count;            // total class count
    uint32_t box_count;              // boxes of all classes
    struct bounding_box_s boxes[1];  // box_count
} yolo_result_t;

struct yolo_and_landmark_result_s {
    uint32_t class_count;            // total class count
    uint32_t box_count;              // boxes of all classes
    struct bounding_box_landmark_s boxes[BOXES_MAX_NUM];  // box_count
};

struct yolox_and_landmark_result_s {
    uint32_t class_count;                                           // total class count
    uint32_t box_count;                                             // total box count
    struct bounding_box_landmark_plus_s boxes[BOXES_MAX_NUM];       // found bounding boxes
};

struct age_gender_result_s {
    uint32_t age;
    uint32_t ismale;
};

typedef struct imagenet_result_s {
    int32_t   index; // index of the class
    float score; // probability score of the class
}imagenet_result_t;

struct facedet_result_s {
    int32_t len;
    int32_t xywh[4]; // 4 values for X, Y, W, H
    float xywh_fl[4]; // 4 values for X, Y, W, H
    float score;     //prob score
    int32_t class_num; //class
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
};

struct fr_flfix_result_s {
    float feature_map[FR_FEAT_SIZE];
    int8_t raw_feature_map[FR_FEAT_SIZE];
};

struct eye_lid_lm_result_s {
    struct {
        uint32_t x;
        uint32_t y;
    } marks[EYE_LID_LM_POINTS];
    float score;
};

struct face_occlude_result_s {
    float yaw;
    float pitch;
    float roll;
    float occ;
};

struct age_group_result_s {
    int32_t age;
};

struct face_quality_result_s {
    float face_score;
};

struct lv_result_s{
    uint32_t real[LV_R_SIZE];
    float    score[LV_SCORE_SIZE];
    _Bool    wb_result;
    float    nir_luma_ratio;
    uint8_t  rgb_quality;
    uint8_t  rgb_corner_y;
    float    effect_2d;
    uint8_t  cal_nir_led_on_tile;
    uint8_t  cal_distance;
    float    id_ref_c; 
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


#define CENTERNET_DETECTION_SIZE 50
/* centernet result */
typedef struct {
    uint32_t    class_count;
    uint32_t    box_count;
    struct bounding_box_s boxes[CENTERNET_DETECTION_SIZE];
}cnet_res;

typedef struct {
    uint32_t start_offset;
    uint32_t buf_len;
    uint32_t node_id;
    uint32_t supernum;
    uint32_t data_format;
    uint32_t row_start;
    uint32_t col_start;
    uint32_t ch_start;
    uint32_t row_length;
    uint32_t col_length;
    uint32_t ch_length;
    uint32_t output_index;
    uint32_t output_radix;
    uint32_t output_scale;
} raw_onode_t;

typedef struct {
    uint32_t total_raw_len;
    int32_t   total_nodes;
    raw_onode_t onode_a[40];
    uint8_t data[];
}raw_cnn_res_t;

typedef struct {
    int16_t     vector1;
    int16_t     vector2;
    int16_t     vector3;
    int16_t     vector4;
} motion_vector_t;

/***** license plate structs *****/

/* car detect result */
typedef struct cd_result_s {
    uint32_t class_count;
    uint32_t box_count;  
    struct bounding_box_s boxes[MAX_CARS_DETECTED];
} cd_result_t;

/* license plate detect result */
typedef struct lpd_result_s {
    struct {
        uint32_t x;
        uint32_t y;
    } marks[LAND_MARK_POINTS];
}lpd_result_t;

/* license plate recog result */

typedef struct lpr_result_s
{
    uint32_t class_count;
    uint32_t chars_count;
    struct bounding_box_s char_boxs[MAX_PLATE_STR_LEN];
}lpr_result_t;

typedef struct ocr_s {
    /*copy from lpr_result_s to store each bounding box of ocr value*/
    uint32_t chars_count;
    struct bounding_box_s char_boxes[MAX_PLATE_STR_LEN];
    /*-------------------------------------------------------------*/
    uint8_t valid;                      // 0 for invalid, 1 for valid
    uint8_t value[OCR_MAX_NUM];         // integers representing characters in order, mapping
                                        //     defined in licenseplate_ocr.c
    uint8_t hyphen;                     // position after which the hyphen should occur
} ocr_t;
/***** end of car plate struct *****/

/////////  porting from e2e platform  /////////

#define MAX_LANDMARK_POINTS 5

// used with post_onet_plus
struct onet_plus_result_s {
    struct {
        float x;
        float y;
    } marks[MAX_LANDMARK_POINTS];
    float scores[MAX_LANDMARK_POINTS * 2];  // each score will be followed by corresponding class
};

#endif
