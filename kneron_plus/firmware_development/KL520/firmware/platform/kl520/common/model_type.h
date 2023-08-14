/**
 * @file        model_type.h
 * @brief       
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#ifndef __MODEL_TYPE_H
#define __MODEL_TYPE_H

enum model_type {
    INVALID_ID,
    INVALID_TYPE                                                            = 0,
    KNERON_FD_SMALLBOX_200_200_3                                            = 1,
    KNERON_FD_ANCHOR_200_200_3                                              = 2,
    KNERON_FD_MBSSD_200_200_3                                               = 3,
    AVERAGE_POOLING                                                         = 4, //use with FD smallbox and don't use anymore
    KNERON_LM_5PTS_ONET_56_56_3                                             = 5,
    KNERON_LM_68PTS_dlib_112_112_3                                          = 6,
    KNERON_LM_150PTS                                                        = 7,
    KNERON_FR_RES50_112_112_3                                               = 8,
    KNERON_FR_RES34                                                         = 9,
    KNERON_FR_VGG10                                                         = 10,
    KNERON_TINY_YOLO_PERSON_416_416_3                                       = 11,
    KNERON_3D_LIVENESS                                                      = 12, //has two inputs: depth and RGB
    KNERON_GESTURE_RETINANET_320_320_3                                      = 13,
    TINY_YOLO_VOC_224_224_3                                                 = 14,
    IMAGENET_CLASSIFICATION_RES50_224_224_3                                 = 15,
    IMAGENET_CLASSIFICATION_RES34_224_224_3                                 = 16,
    IMAGENET_CLASSIFICATION_INCEPTION_V3_224_224_3                          = 17,
    IMAGENET_CLASSIFICATION_MOBILENET_V2_224_224_3                          = 18,
    TINY_YOLO_V3_224_224_3                                                  = 19,
    KNERON_2D_LIVENESS_224_224_3                                            = 20, //oldest rgb liveness model and don't use anymore
    KNERON_FD_RETINANET_RES50_640_640_3                                     = 21,
    KNERON_PERSON_MOBILENETSSD_224_224_3                                    = 22,
    KNERON_AGE_GENDER                                                       = 23, //oldest age gender model and don't use anymore
    KNERON_LM_5PTS_BLUR_ONET_48_48_3                                        = 24,
    KNERON_2D_LIVENESS_V3_FACEBAGNET_224_224_3                              = 25,
    KNERON_AGE_GENDER_V2_RES18_128_128_3                                    = 26,
    KNERON_OD_MBSSD                                                         = 27, //HW model and don't know input size
    KNERON_PD_MBSSD                                                         = 28, //HW model and don't know which version and input size
    KNERON_FR_MASK_RES50_112_112_3                                          = 29,
    KNERON_NIR_LIVENESS_RES18_112_112_3                                     = 30,
    KNERON_FR_MASK_RES101_112_112_3                                         = 31,
    KNERON_FD_MASK_MBSSD_200_200_3                                          = 32,
    TINY_YOLO_V3_416_416_3                                                  = 33,
    TINY_YOLO_V3_608_608_3                                                  = 34,

    //Category Face related 40~200
    KNERON_CAT_FACE                                                         = 40,
    KNERON_FACE_QAULITY_ONET_56_56_1                                        = KNERON_CAT_FACE,
    KNERON_FUSE_LIVENESS                                                    = KNERON_CAT_FACE + 1, // don't know the model backbone and input size of fuse liveness model
    KNERON_EYELID_DETECTION_ONET_48_48_3                                    = KNERON_CAT_FACE + 2,
    KNERON_YAWN_DETECTION_PFLD_112_112_3                                    = KNERON_CAT_FACE + 3,
    KNERON_DBFACE_MBNET_V2_480_864_3                                        = KNERON_CAT_FACE + 4,
    KNERON_FILTER                                                           = KNERON_CAT_FACE + 5, //No model inference, just pre and post-process
    KNERON_ALIGNMENT                                                        = KNERON_CAT_FACE + 6, //No model inference, just preprocess
    KNERON_FACE_EXPRESSION_112_112_3                                        = KNERON_CAT_FACE + 7,
    KNERON_RBG_OCCLUSION_RES18_112_112_3                                    = KNERON_CAT_FACE + 8,
    KNERON_LM2BBOX                                                          = KNERON_CAT_FACE + 9, //No model inference, just post-process
    KNERON_PUPIL_ONET_48_48_3                                               = KNERON_CAT_FACE + 10,
    KNERON_NIR_OCCLUSION_RES18_112_112_3                                    = KNERON_CAT_FACE + 11,
    KNERON_HEAD_SHOULDER_MBNET_V2_112_112_3                                 = KNERON_CAT_FACE + 12,
    KNERON_RGB_LIVENESS_RES18_112_112_3                                     = KNERON_CAT_FACE + 13,
    KNERON_MOUTH_LM_v1_56_56_1                                              = KNERON_CAT_FACE + 14, //3dLM: nose, upper lip middle, chin, two sides of faces
    KNERON_MOUTH_LM_v2_56_56_1                                              = KNERON_CAT_FACE + 15, //3dLM: nose, upper/lower lip middle, two sides of faces
    KNERON_PUPIL_ONET_48_48_1                                               = KNERON_CAT_FACE + 16,
    KNERON_RGB_LIVENESS_MBV2_112_112_3                                      = KNERON_CAT_FACE + 17,
    KNERON_FACESEG_DLA34_128_128_3                                          = KNERON_CAT_FACE + 18,
    KNERON_OCC_CLS                                                          = KNERON_CAT_FACE + 19, //no model inference, just post-process
    KNERON_LMSEG_FUSE                                                       = KNERON_CAT_FACE + 20, //no model inference, just post-process
    KNERON_AGEGROUP_RES18_128_128_3                                         = KNERON_CAT_FACE + 21,
    KNERON_FR_kface_112_112_3                                               = KNERON_CAT_FACE + 22,
    KNERON_FDmask_ROTATE_MBSSD_200_200_3                                    = KNERON_CAT_FACE + 23,
    KNERON_LM_5PTSROTATE_ONET_56_56_3                                       = KNERON_CAT_FACE + 24,
    KNERON_FUSE_LIVENESS_850MM                                              = KNERON_CAT_FACE + 25,
    KNERON_FUSE_LIVENESS_940MM                                              = KNERON_CAT_FACE + 26,
    KNERON_FACE_QAULITY_ONET_112_112_3                                      = KNERON_CAT_FACE + 27,
    KNERON_FACE_POSE_ONET_56_56_3                                           = KNERON_CAT_FACE + 28,
    KNERON_FUSE_LIVENESS_850MM_RES18_112_112_3                              = KNERON_CAT_FACE + 29, //Gen's resnet+agg fuse model
    KNERON_OCCCLASSIFER_112_112_3                                           = KNERON_CAT_FACE + 30,
    KNERON_FACE_ROTATE_POSE_ONET_56_56_3                                    = KNERON_CAT_FACE + 31,
    KNERON_NIR_LIVENESS_ROT_RES18_112_112_3                                 = KNERON_CAT_FACE + 32,
    KNERON_LM_5PTS_ONETPLUS_56_56_3                                         = KNERON_CAT_FACE + 33,
    KNERON_FUSE_LIVENESS_940MM_18_RES18_112_112_3                           = KNERON_CAT_FACE + 34,
    KNERON_DBFACE_MBNET_V2_256_352_3                                        = KNERON_CAT_FACE + 35,
    KNERON_NIR_Liveness_aligned_112_112_3                                   = KNERON_CAT_FACE + 36,
    KNERON_FUSE_Liveness_940_850_RES18_112_112_3                            = KNERON_CAT_FACE + 37,
    KNERON_FDLM_RETINANET_MBV2_640_640_3                                    = KNERON_CAT_FACE + 38,
    KNERON_NIR_OCCLUSION_MBV2D4_112_112_3                                   = KNERON_CAT_FACE + 39,
    KNERON_FACE_EYEGLASSES_CLS3_56_56_3                                     = KNERON_CAT_FACE + 40,
    KNERON_HEAD_SHOULDER_ROT_MBNET_V2_112_112_3                             = KNERON_CAT_FACE + 41,
    KNERON_YOLOV5_FACE_AND_LM_640_640_3                                     = KNERON_CAT_FACE + 42,  // bbox and lm, 2 outputs
    KNERON_YOLOV5_FACE_AND_LM_NO_UPSAMPLE_640_640_3                         = KNERON_CAT_FACE + 43,  // bbox and lm, 2 outputs
    KNERON_YOLOV5_FACE_FD_640_640_3                                         = KNERON_CAT_FACE + 44,  // only bbox
    KNERON_YOLOV5_FACE_FD_NO_UPSAMPLE_640_640_3                             = KNERON_CAT_FACE + 45,  // only bbox
    KNERON_RSN_AFFINE                                                       = KNERON_CAT_FACE + 46,  // affine preprocess only
    KNERON_MOUTH_LM_v2_ROTATE_56_56_1                                       = KNERON_CAT_FACE + 47,    //3dLM: nose, upper/lower lip middle, two sides of faces

    //Category Object Detection related 200~300
    KNERON_OB_DETECT                                                        = 200,
    KNERON_OBJECTDETECTION_CENTERNET_512_512_3                              = KNERON_OB_DETECT,
    KNERON_OBJECTDETECTION_FCOS_416_416_3                                   = KNERON_OB_DETECT + 1,
    KNERON_PD_MBNET_V2_480_864_3                                            = KNERON_OB_DETECT + 2, //16:9 aspect ratio
    KNERON_CAR_DETECTION_MBSSD_224_416_3                                    = KNERON_OB_DETECT + 3,
    KNERON_PD_CROP_MBSSD_304_304_3                                          = KNERON_OB_DETECT + 4,
    YOLO_V3_416_416_3                                                       = KNERON_OB_DETECT + 5,
    YOLO_V4_416_416_3                                                       = KNERON_OB_DETECT + 6,
    KNERON_CAR_DETECTION_YOLOV5S_CarMotorcycleBusTruck4_352_640_3           = KNERON_OB_DETECT + 7,
    KNERON_LICENSE_DETECT_WPOD_208_416_3                                    = KNERON_OB_DETECT + 8,
    KNERON_2D_UPPERBODY_KEYPOINT_RES18_384_288_3                            = KNERON_OB_DETECT + 9,
    YOLO_V3_608_608_3                                                       = KNERON_OB_DETECT + 10,
    KNERON_YOLOV5S_COCO80_640_640_3                                         = KNERON_OB_DETECT + 11,
	KNERON_YOLOV5S_PersonBicycleCarMotorcycleBusTruckCatDog8_480_256_3      = KNERON_OB_DETECT + 12,
    KNERON_SITTINGPOSTURE_RESNET34_288_384_3                                = KNERON_OB_DETECT + 13,
    KNERON_PERSONDETECTION_FCOS_416_416_3                                   = KNERON_OB_DETECT + 14,
    KNERON_YOLOV5m_COCO80_640_640_3                                         = KNERON_OB_DETECT + 15,
    KNERON_YOLOV5S_PersonBicycleCarMotorcycleBusTruck6_480_256_3            = KNERON_OB_DETECT + 16,
    KNERON_PERSONDETECTION_FCOS_384_288_3                                   = KNERON_OB_DETECT + 17,
    KNERON_PERSONDETECTION_FCOS_720_416_3                                   = KNERON_OB_DETECT + 18,
    KNERON_PERSONDETECTION_dbface_864_480_3                                 = KNERON_OB_DETECT + 19,
    KNERON_PERSONDETECTION_YOLOV5s_480_256_3                                = KNERON_OB_DETECT + 20,
    KNERON_PERSONCLASSIFIER_MB_56_32_3                                      = KNERON_OB_DETECT + 21,
    KNERON_PERSONREID_RESNET_42_82_3                                        = KNERON_OB_DETECT + 22,
    KNERON_PERSONDETECTION_YOLOV5s_928_512_3                                = KNERON_OB_DETECT + 23,
    KNERON_UPKPTS_RSN_256_192_3                                             = KNERON_OB_DETECT + 24,
    KNERON_PERSONDETECTION_YOLOV5sParklot_480_256_3                         = KNERON_OB_DETECT + 25,
    KNERON_CAR_DETECTION_MBSSD_304_544_3                                    = KNERON_OB_DETECT + 26,
    KNERON_KPTSCLASSIFIER_3_11_1                                            = KNERON_OB_DETECT + 27,
    KNERON_YOLOV5S_PersonCatDogBottleChairPottedplant6_480_256_3            = KNERON_OB_DETECT + 28,
    KNERON_DBFAIR_320_160_3                                                 = KNERON_OB_DETECT + 29,
    KNERON_CAR_MOTOR_PERSON_DETECTION_MBSSD_224_416_3                       = KNERON_OB_DETECT + 30,
    KNERON_LICENSE_DETECT_QUANTIZED_WPOD_208_416_3                          = KNERON_OB_DETECT + 31,
    KNERON_PERSON_DETECTION_MBSSD_224_416_3                                 = KNERON_OB_DETECT + 32,
    KNERON_PERSONCLASSIFIER_MB_56_48_3                                      = KNERON_OB_DETECT + 33,
    KNERON_YOLOV5S_CarMotorcycleBusTruckPlate5_640_352_3                    = KNERON_OB_DETECT + 34,
    KNERON_YOLOV5S_PersonBottleChairPottedplant4_640_288_3                  = KNERON_OB_DETECT + 35,
    KNERON_COCO_KPTS_dbpose_640_384_3                                       = KNERON_OB_DETECT + 36,
    KNERON_YOLOV5S_OCR_NUMBER_ALPHABET_PLATE_640_352_3                      = KNERON_OB_DETECT + 37,
    KNERON_LICENSE_DETECT_YOLO5FACE_S_4PT_608_800_3                         = KNERON_OB_DETECT + 73,

    //Category OCR related 300~400
    KNERON_OCR                                                              = 300,
    KNERON_LICENSE_OCR_MBNET_64_160_3                                       = KNERON_OCR,
    KNERON_WATERMETER_OCR_MBNET                                             = KNERON_OCR + 1, //unknown
    KNERON_LICENSE_OCR_MBNETv2_64_160_3                                     = KNERON_OCR + 2,
    KNERON_LICENSE_OCR_MBNETv2_96_256_3                                     = KNERON_OCR + 3,

    //Category SDK test related
    KNERON_CAT_SDK_TEST                                                     = 1000,
    KNERON_SDK_FD                                                           = KNERON_CAT_SDK_TEST,
    KNERON_SDK_LM                                                           = KNERON_CAT_SDK_TEST + 1,
    KNERON_SDK_FR                                                           = KNERON_CAT_SDK_TEST + 2,

    //Category Customer models
    //0x8000 = 32768
    CUSTOMER                                                                = 32768,
    CUSTOMER_MODEL_1                                                        = CUSTOMER,
    CUSTOMER_MODEL_2                                                        = CUSTOMER + 1,
    CUSTOMER_MODEL_3                                                        = CUSTOMER + 2,

    //FID NPU Comparison used model
    //TODO. need to change model id. 
    EMBEDDING_CMP                                                           = 333,
};

#define KNERON_NIR_LIVENESS             24

#endif
