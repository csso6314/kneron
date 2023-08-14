/**
 * @file      model_ppp.h
 * @brief     builtin model pre/post process functions
 * @copyright (c) 2020-2022 Kneron Inc. All right reserved.
 */

#ifndef __MODEL_PPP_H__
#define __MODEL_PPP_H__

#include "ncpu_gen_struct.h"

/*********************************************************************************
                  CNN pre-processing functions list
*********************************************************************************/

int pre_proc_classify_mobilenet_v2(struct kdp_image_s *pKdpImage);
int pre_proc_fd_smallbox(struct kdp_image_s *pKdpImage);
int pre_proc_fd_ssd(struct kdp_image_s *pKdpImage);
int pre_proc_lm_5pts(struct kdp_image_s *pKdpImage);
int pre_proc_fr_mask_vgg10(struct kdp_image_s *pKdpImage);
int pre_proc_fr_vgg10(struct kdp_image_s *pKdpImage);
int pre_proc_tiny_yolo_person(struct kdp_image_s *pKdpImage);
int pre_proc_tiny_yolo_voc(struct kdp_image_s *pKdpImage);
int pre_proc_tiny_yolo_v3(struct kdp_image_s *pKdpImage);
int pre_proc_licenseplate_ocr(struct kdp_image_s *pKdpImage);

/*********************************************************************************
                  CNN post-processing functions list
*********************************************************************************/

int post_proc_classify_res50(struct kdp_image_s *pKdpImage);
int post_proc_classify_mobilenet_v2(struct kdp_image_s *pKdpImage);
int post_proc_fd_smallbox(struct kdp_image_s *pKdpImage);
int post_proc_fd_ssd(struct kdp_image_s *pKdpImage);
int post_proc_lm_5pts(struct kdp_image_s *pKdpImage);
int post_proc_onet_plus(struct kdp_image_s *pKdpImage);
int post_proc_fr_fixfloat_output(struct kdp_image_s *pKdpImage);
int post_proc_fr_mask_fixfloat_output(struct kdp_image_s *pKdpImage);
int post_proc_fr_vgg10(struct kdp_image_s *pKdpImage);
int post_proc_centernet(struct kdp_image_s *pKdpImage);
int post_proc_tiny_yolo_v3(struct kdp_image_s *pKdpImage);
int post_proc_tiny_yolo_voc(struct kdp_image_s *pKdpImage);
int post_proc_tiny_yolo_person(struct kdp_image_s *pKdpImage);
int post_proc_yolov5s(struct kdp_image_s *pKdpImage);
int post_proc_licenseplate_ocr(struct kdp_image_s *pKdpImage);
int post_proc_fcos(struct kdp_image_s *pKdpImage);
int post_proc_yolo_face(struct kdp_image_s *pKdpImage);
int post_proc_yolox(struct kdp_image_s *pKdpImage);
int post_proc_yolox_for_ocr(struct kdp_image_s *pKdpImage);

#endif    /* __MODEL_PPP_H__ */

