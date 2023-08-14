#pragma once

#define DEMO_KL520_CUSTOMIZE_INF_MULTIPLE_MODEL_JOB_ID  1001
#define FD_MAX                                          10

typedef struct
{
    kp_bounding_box_t fd;                 /**< fd result */
    kp_landmark_result_t lm;              /**< lm result */
} __attribute__((aligned(4))) one_person_face_data_t;

typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t width;
    uint32_t height;
} __attribute__((aligned(4))) demo_customize_inf_multiple_models_header_t;

// result (header + data) for 'Kneron APP Yolo Inference'
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t face_count;
    one_person_face_data_t faces[FD_MAX];
} __attribute__((aligned(4))) demo_customize_inf_multiple_models_result_t;
