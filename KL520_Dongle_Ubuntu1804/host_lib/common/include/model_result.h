/*******************************************************/
/* This file must be synced between kapp and khost_lib */
/*******************************************************/
#ifndef __MODEL_RESULT_H__
#define __MODEL_RESULT_H__

#include <stdint.h>

/* Post Processing Result Info */

/* Yolo Result */
struct bounding_box_s
{
    float x1; // top-left corner:  x
    float y1; // top-left corner:  y
    float x2; // bottom-right corner:  x
    float y2; // bottom-right corner:  y

    float score;   // probability score
    int class_num; // class # (of many) with highest probability
};

#define YOLO_DETECTION_MAX 80

struct yolo_result_s
{
    uint32_t class_count;           // total class count
    uint32_t box_count;             /* boxes of all classes */
    struct bounding_box_s boxes[1]; /* [box_count] */
};

/* ImageNet Classification Result */

#define IMAGENET_TOP_MAX 5

struct imagenet_result_s
{
    int index;   // index of the class
    float score; // probability score of the class
};

#define MAX_CRC 4
typedef struct aoi_box_s
{
    unsigned int x1;
    unsigned int y1;
    unsigned int x2;
    unsigned int y2;
} aoi_box_t; 

typedef struct ext_classify_s
{
	unsigned int aoi_count;
	unsigned int counter;  // used as counter by application only, init to zero
	struct aoi_box_s box[MAX_CRC];
} ext_classify_t;

typedef struct ext_cls_result_s
{
    unsigned int aoi_count;
    unsigned int counter;  // not used
    struct imagenet_result_s obj[MAX_CRC][IMAGENET_TOP_MAX];
} ext_cls_result_t;

/* Face Detection Result */

#define FACE_DETECTION_XYWH 4

struct facedet_result_s
{
    int len;                       // length of detected X,Y,W,H: 0 or 4
    int xywh[FACE_DETECTION_XYWH]; // 4 values for X, Y, W, H
};

/* Land Mark Result */

#define LAND_MARK_POINTS 5

struct landmark_result_s
{
    struct
    {
        uint32_t x;
        uint32_t y;
    } marks[LAND_MARK_POINTS];
};

/* fr result */

#define FR_FEATURE_MAP_SIZE 512

struct fr_result_s
{
    float feature_map[FR_FEATURE_MAP_SIZE];
};

#if 0
struct npu_dsp_queue_elem_s {
    int8_t nSeq;
    enum model_type nModel;
    union {
        struct yolo_result_s yolo;
        struct imagenet_result_s imgnet;
        struct facedet_result_s facedet;
        struct landmark_result_s landmark;
        struct fr_result_s *p_fr;
    } data_u;
};
#endif

#endif
