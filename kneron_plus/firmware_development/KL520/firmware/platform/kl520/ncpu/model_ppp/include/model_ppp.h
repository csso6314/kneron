/**
 * @file      model_ppp.h
 * @brief     Builtin pre-/post-process functions by Kneron
 * @copyright (c) 2020-2022 Kneron Inc. All right reserved.
 */

#ifndef __MODEL_PPP_H__
#define __MODEL_PPP_H__

#include "kdpio.h"

//int cpu_op_nearest_upsample(struct kdp_image_s *image_p);
//int cpu_op_zero_upsample(struct kdp_image_s *image_p);


/* Pre-process functions */
int preproc_face_recognition(struct kdp_image_s *image_p);
int preproc_face_recognition_mask(struct kdp_image_s *image_p);
int preproc_fr_mask(struct kdp_image_s *image_p);
int preproc_licenseplate_ocr(struct kdp_image_s *image_p);


/* Post-process functions */
int post_face_detection(struct kdp_image_s *image_p);
int post_face_recognition(struct kdp_image_s *image_p);
int post_face_recognition_mask(struct kdp_image_s *image_p);
int post_face_landmark_onet_5p(struct kdp_image_s *image_p);
int post_ssd_face_detection(struct kdp_image_s *image_p);
int post_nir_liveness(struct kdp_image_s *image_p);
int post_age_gender(struct kdp_image_s *image_p);
int post_ssd_object_detection(struct kdp_image_s *image_p);
int post_yolov3_optimized(struct kdp_image_s *image_p);
int post_yolov5_optimized(struct kdp_image_s *image_p);
int post_yolo_face(struct kdp_image_s *image_p);
int post_licenseplate_ocr_optimized(struct kdp_image_s *image_p);
int post_imagenet_classification(struct kdp_image_s *image_p);
int post_classifier(struct kdp_image_s *image_p);
int post_fcos_det(struct kdp_image_s *image_p);

#endif
