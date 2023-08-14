#pragma once

#define DEMO_KL520_CUSTOMIZE_INF_SINGLE_MODEL_JOB_ID    1000
#define YOLO_BOX_MAX                                    100 /**< maximum number of bounding boxes for Yolo models */

/**
 * @brief describe a yolo output result after post-processing
 */
typedef struct
{
    uint32_t class_count;                      /**< total class count detectable by model */
    uint32_t box_count;                        /**< boxes of all classes */
    kp_bounding_box_t boxes[YOLO_BOX_MAX]; /**< box information */
} __attribute__((aligned(4))) kp_custom_yolo_result_t;

typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t width;
    uint32_t height;
} __attribute__((aligned(4))) demo_customize_inf_single_model_header_t;

// result (header + data) for 'Kneron APP Yolo Inference'
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    kp_custom_yolo_result_t yolo_result;
} __attribute__((aligned(4))) demo_customize_inf_single_model_result_t;
