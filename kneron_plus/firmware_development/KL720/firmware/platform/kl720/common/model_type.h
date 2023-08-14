/**
 * @file        model_type.h
 * @brief       model ID
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
    KNERON_RACE_RES18_128_128_3                                             = 35,
    KNERON_AGE_GENDER_WHITE_RES18_128_128_3                                 = 36,
    KNERON_AGE_GENDER_BLACK_RES18_128_128_3                                 = 37,
    KNERON_AGE_GENDER_INDIAN_RES18_128_128_3                                = 38,
    KNERON_FR_RES20MB_112_112_3                                             = 39,

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
    KNERON_YOLOV5_FACE_MASK_LM_640_640_3                                    = KNERON_CAT_FACE + 48,  // bbox and lm, 2 outputs, 640(w)x640(h)
    KNERON_YOLOV5_FACE_MASK_LM_384_640_3                                    = KNERON_CAT_FACE + 49,  // bbox and lm, 2 outputs, 640(w)x384(h)
    KNERON_YOLOV5_FACE_MASK_640_640_3                                       = KNERON_CAT_FACE + 50,  // only bbox, 640(w)x640(h)
    KNERON_YOLOV5_FACE_MASK_384_640_3                                       = KNERON_CAT_FACE + 51,  // only bbox, 640(w)x384(h)
    KNERON_FACE_PUPIL_CLS2_48_48_3                                          = KNERON_CAT_FACE + 52,
    KNERON_LM_5PTS_ROTATE_ONETPLUS_80_80_3                                  = KNERON_CAT_FACE + 53,
    KNERON_FDmask_ROTATE_fcos_224_224_3                                     = KNERON_CAT_FACE + 54,
    KNERON_FDmask_fcos_224_224_3                                            = KNERON_CAT_FACE + 55,
    KNERON_FACE_PUPIL_ROTATE_CLS2_48_48_3                                   = KNERON_CAT_FACE + 56,
    KNERON_NIR_OCCLUSION_ROT_MBV2D4_112_112_3                               = KNERON_CAT_FACE + 57,
    KNERON_FUSE_LIVENESS_sc035_112_112_4                                    = KNERON_CAT_FACE + 58,
    KNERON_FDmask_fcos_512_704_3                                            = KNERON_CAT_FACE + 59,
    KNERON_FACESEG_DLA34_rotate_128_128_3                                   = KNERON_CAT_FACE + 60,
    KNERON_FUSE_LIVENESS_R18_5crops_48_48_3                                 = KNERON_CAT_FACE + 61,
    KNERON_RGB2NIR_MOBILEFACENET_RESNET_6BLOCKS_256_256_3                   = KNERON_CAT_FACE + 62,
    KNERON_FUSE_LIVENESS_1054sc035_112_112_4                                = KNERON_CAT_FACE + 63,
    KNERON_NIR_LIVENESS_ROT_1054_RES18_112_112_3                            = KNERON_CAT_FACE + 64,
    KNERON_FDmask_ROTATE_fcos_256_448_3                                     = KNERON_CAT_FACE + 65,
    KNERON_FDmask_ROTATE_fcos_288_416_3                                     = KNERON_CAT_FACE + 66,
    KNERON_DEPTH_LIVENESS_FeatherNetB_112_112_1                             = KNERON_CAT_FACE + 67,
    KNERON_FUSE_TOF_LIVENESS_DEP_NIR_112_112_1                              = KNERON_CAT_FACE + 68,
    KNERON_NSH_LIVENESS_ROT_1054_MBV2D2_112_112_3                           = KNERON_CAT_FACE + 69,
    KNERON_FDmask_fcos_384_320_3                                            = KNERON_CAT_FACE + 70,
    KNERON_COMBINE_LIVENESS_ROT_1054_MBV2_112_112_3                         = KNERON_CAT_FACE + 71,
    KNERON_RGB2NIR_UNET_FR_112_112_1                                        = KNERON_CAT_FACE + 72,
    KNERON_RGB2NIR_UNET_FR_112_112_3                                        = KNERON_CAT_FACE + 73,


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
    KNERON_PERSONKPTS_ONETPLUS_112_56_3                                     = KNERON_OB_DETECT + 38,
    KNERON_IMAGE_KPT_240_320_3                                              = KNERON_OB_DETECT + 39,
    KNERON_FULLBODYKPTS_LHRN_256_192_3                                      = KNERON_OB_DETECT + 40, //520: 8.81063, 720:119.38
    KNERON_YOLOF_R50_640_384_3                                              = KNERON_OB_DETECT + 41,
    KNERON_SOLOV2_R18_FPN_448_448_3                                         = KNERON_OB_DETECT + 42,
    KNERON_IMAGE_KPT_SP_240_320_3                                           = KNERON_OB_DETECT + 43,
    KNERON_YOLOX_S_640_640_3                                                = KNERON_OB_DETECT + 44,
    KNERON_SCRFD_500M_BNKPS_640_480_3                                       = KNERON_OB_DETECT + 45,
    KNERON_SCRFD_2G_BNKPS_640_480_3                                         = KNERON_OB_DETECT + 46,
    KNERON_SCRFD_10G_BNKPS_640_480_3                                        = KNERON_OB_DETECT + 47,
    KNERON_PERSONCLASSIFIER_HALF_56_32_3                                    = KNERON_OB_DETECT + 48,
    KNERON_SCRFD_10G_BNKPS_640_640_3                                        = KNERON_OB_DETECT + 49,
    KNERON_YOLOV5S_PersonBicycleCarMotorcycleBusTruck6_512_864_3            = KNERON_OB_DETECT + 50,
    KNERON_FULLBODYKPTS_RSN_COCO_256_192_3                                  = KNERON_OB_DETECT + 51, //9.14208, 56.433
    KNERON_YOLOV5S_768192PersonCatDogBottleChairPottedplant6_256_480_3      = KNERON_OB_DETECT + 52,
    KNERON_PERSONCLASSIFIER_768192MB_56_48_3                                = KNERON_OB_DETECT + 53,
    KNERON_FULLBODYKPTS_RSN_MPII_256_256_3                                  = KNERON_OB_DETECT + 54,
    KNERON_CENTERTRACK_PERSON_960_544_7                                     = KNERON_OB_DETECT + 55,
    KNERON_FAIRMOT_PERSON_928_512_3                                         = KNERON_OB_DETECT + 56,
    KNERON_YOLOP_VEHICLE_384_640_3                                          = KNERON_OB_DETECT + 57,
    KNERON_YOLOV5S_PersonBicycleCarMotorcycleBusTruck6_480_832_3            = KNERON_OB_DETECT + 58,
    KNERON_YOLOV5S_PersonBottleChairPottedplant4_480_832_3                  = KNERON_OB_DETECT + 59,
    KNERON_YOLOV5S_PersonHead2_256_480_3                                    = KNERON_OB_DETECT + 60,
    KNERON_HANDKPTS_RSN18_FREIHAND_224_224_3                                = KNERON_OB_DETECT + 62,
    KNERON_YOLOV5S_COCO80_256_480_3                                         = KNERON_OB_DETECT + 63,
    KNERON_YOLOV5S_COCO80_480_640_3                                         = KNERON_OB_DETECT + 64,
    KNERON_YOLOV5S_PersonBicycleCarMotorcycleBusTruck6_192_480_3            = KNERON_OB_DETECT + 65,
    KNERON_YOLOV5S_PersonBicycleCarMotorcycleBusTruck6_352_480_3            = KNERON_OB_DETECT + 66,
    KNERON_YOLOOP_PersonTwowheelVehicle3_384_640_3                          = KNERON_OB_DETECT + 67,
    KNERON_YOLOV5M_PersonBottleChairPottedplant4_160_960_3                  = KNERON_OB_DETECT + 68,
    KNERON_YOLOX_Person_448_800_3                                           = KNERON_OB_DETECT + 69,
    KNERON_YOLOV5S_PersonVboxHead2_256_480_3                                = KNERON_OB_DETECT + 70,
    KNERON_YOLOX_Person_160_960_3                                           = KNERON_OB_DETECT + 71,
    KNERON_IMAGE_KPT_CV_240_320_3                                           = KNERON_OB_DETECT + 72,
    KNERON_LICENSE_DETECT_YOLO5FACE_S_4PT_608_800_3                         = KNERON_OB_DETECT + 73,
    KNERON_LICENSE_DETECT_YOLO5FACE_S_4PT_NO_UPSAMPLE_480_640_3             = KNERON_OB_DETECT + 74,
    KNERON_YOLOV5S_PersonBicycleCarMotorcycleBusTruck6_192_288_3            = KNERON_OB_DETECT + 75,
    KNERON_YOLOV5S0375_PersonBicycleCarMotorcycleBusTruck6_256_352_3        = KNERON_OB_DETECT + 76,
    KNERON_YOLOV5S_ROBOT_VACUUM_61CLS_480_640_3                             = KNERON_OB_DETECT + 77,
    KNERON_YOLOV5M_PersonBottleChairPottedplantTvmonitorProjectorscreenLaptop7_160_1152_3 = KNERON_OB_DETECT + 78,
    KNERON_YOLOV5M_PersonBottleChairPottedplantTvmonitorProjectorscreenLaptop7_192_928_3 = KNERON_OB_DETECT + 79,
    KNERON_YOLOV5M_ROBOT_VACUUM_61CLS_480_640_3                             = KNERON_OB_DETECT + 80,
    KNERON_YOLOV5S_PersonHead2_288_384_3                                    = KNERON_OB_DETECT + 81,
    KNERON_IMAGE_KPT_LOFTR_240_320_3                                        = KNERON_OB_DETECT + 82,
    KNERON_GLUE_300                                                         = KNERON_OB_DETECT + 83,
    KNERON_FCOS_MM_800_1344_3                                               = KNERON_OB_DETECT + 84,
    KNERON_SSD_MM_320_320_3                                                 = KNERON_OB_DETECT + 85,
    KNERON_YOLOV5S_PersonBottleChairPottedplantTvmonitorProjectorscreenLaptop7_448_2112_3 = KNERON_OB_DETECT + 86,
    KNERON_MMOCR_DBNET_736_1248_3                                           = KNERON_OB_DETECT + 87,
    KNERON_YOLOXS_LANDMARK_USLPD_480_832_3                                  = KNERON_OB_DETECT + 95,
    KNERON_YOLOXM_USLPR_96_256_3                                            = KNERON_OB_DETECT + 96,


    //Category OCR related 300~400
    KNERON_OCR                                                              = 300,
    KNERON_LICENSE_OCR_MBNET_64_160_3                                       = KNERON_OCR,
    KNERON_WATERMETER_OCR_MBNET                                             = KNERON_OCR + 1, //unknown
    KNERON_LICENSE_OCR_MBNETv2_64_160_3                                     = KNERON_OCR + 2,
    KNERON_LICENSE_OCR_MBNETv2_96_256_3                                     = KNERON_OCR + 3,
    KNERON_ABINET_OCR_32_128_3                                              = KNERON_OCR + 4,

    //Category Audio related 600~900
    KNERON_AUDIO                                                            = 600,
    KNERON_KWS_DSCNN3_149_10_1                                              = KNERON_AUDIO,
    KNERON_SIMPLE_COMMAND_DSCNN3_49_10_1                                    = KNERON_AUDIO + 1,
    KNERON_SOUND_CLASSIFICATION_RESNET34_128_250_3                          = KNERON_AUDIO + 2,
    KNERON_ASR_ENCODER_1_1000_64                                            = KNERON_AUDIO + 3,
    KNERON_ASR_DECODER_1024                                                 = KNERON_AUDIO + 4,

    //Category Depth prediction related
    KNERON_DEPTH                                                            = 700,
    KNERON_DEPTH_RESNET18_320_480_3                                         = KNERON_DEPTH,
    KNERON_RAFT_FRONT_272_480_3                                             = KNERON_DEPTH + 1,
    KNERON_RAFT_GRU_272_480_3                                               = KNERON_DEPTH + 2,
    KNERON_RAFT_UP_272_480_3                                                = KNERON_DEPTH + 3,
    KNERON_STEREONET_FE_480_640_3                                           = KNERON_DEPTH + 4, // Feature Extraction
    KNERON_STEREONET_CC_120_160_320                                         = KNERON_DEPTH + 5, // Cost Construction
    KNERON_STEREONET_CA_120_160_48                                          = KNERON_DEPTH + 6, // Cost Aggregation
    KNERON_STEREONET_CN_480_640_3                                           = KNERON_DEPTH + 7, // Context Net

    //Category tracker related 900~1000
    KNERON_TRACKER                                                          = 900,
    KNERON_TRACKER_UPDATE                                                   = 901,
    KNERON_TRACKER_GET_DATEBASE                                             = 902,
    KNERON_TRACKER_RESET_DATEBASE                                           = 903,

    //Category SDK test related
    KNERON_CAT_SDK_TEST                                                     = 1000,
    KNERON_SDK_FD                                                           = KNERON_CAT_SDK_TEST,
    KNERON_SDK_LM                                                           = KNERON_CAT_SDK_TEST + 1,
    KNERON_SDK_FR                                                           = KNERON_CAT_SDK_TEST + 2,

    //Category Segmentation
    KNERON_SEGMENTATION                                                     = 3000,
    KNERON_INSTANCE_YOLACT_R18_FPN_448_448_3                                = KNERON_SEGMENTATION,
    KNERON_SEGMENTATION_STDCNET_CITYSCAPES_512_1024_3                       = KNERON_SEGMENTATION + 1,
    KNERON_SEGMENTATION_STDCNET_INDOOR_480_640_3                            = KNERON_SEGMENTATION + 2,
    KNERON_SEGMENTATION_DDRNET23SLIMCONV_INDOOR_480_640_3                   = KNERON_SEGMENTATION + 3,
    KNERON_INSTANCE_SOLO_R18_FPN_448_448_3                                  = KNERON_SEGMENTATION + 4,
    KNERON_SEGMENTATION_DDRNET23SLIMCONV_POLY_INDOOR_480_640_3              = KNERON_SEGMENTATION + 5,
    KNERON_CAM2CAM1_172_224                                                 = KNERON_SEGMENTATION + 6,
    KNERON_CAM2CAM2_172_224                                                 = KNERON_SEGMENTATION + 7,
    KNERON_CAM2CAM3_172_224                                                 = KNERON_SEGMENTATION + 8,
    KNERON_CAM2CAM4_172_224                                                 = KNERON_SEGMENTATION + 9,

    //Category System models
    KNERON_SYSTEMMODELS                                                     = 4000,
    KNERON_IMAGENET_MOBILENETV2_224_224_3                                   = KNERON_SYSTEMMODELS,
    KNERON_IMAGENET_MOBILENETV2_QAT_224_224_3                               = KNERON_SYSTEMMODELS + 1,

    //Category Model Zoo training models
    //20000~30000
    KNERON_MODEL_ZOO                                                        = 20000,
    KNERON_MODEL_ZOO_MOBILENETV2                                            = KNERON_MODEL_ZOO,
    KNERON_MODEL_ZOO_RESNET18                                               = KNERON_MODEL_ZOO + 1,
    KNERON_MODEL_ZOO_RESNET50                                               = KNERON_MODEL_ZOO + 2,
    KNERON_MODEL_ZOO_FP_Classifier                                          = KNERON_MODEL_ZOO + 3,
    KNERON_MODEL_ZOO_FCOS_DARKNET53s                                        = KNERON_MODEL_ZOO + 4,
    KNERON_MODEL_ZOO_YOLOV5s_NoUpsample                                     = KNERON_MODEL_ZOO + 5,
    KNERON_MODEL_ZOO_YOLOV5s                                                = KNERON_MODEL_ZOO + 6,

    //Category Customer models
    //0x8000 = 32768
    CUSTOMER                                                                = 32768,
    CUSTOMER_MODEL_1                                                        = CUSTOMER,
    CUSTOMER_MODEL_2                                                        = CUSTOMER + 1,
    CUSTOMER_MODEL_3                                                        = CUSTOMER + 2,
    CUSTOMER_MODEL_4                                                        = CUSTOMER + 3,
    CUSTOMER_MODEL_5                                                        = CUSTOMER + 4,
    CUSTOMER_MODEL_6                                                        = CUSTOMER + 5,
    CUSTOMER_MODEL_7                                                        = CUSTOMER + 6,
    CUSTOMER_MODEL_8                                                        = CUSTOMER + 7,
    CUSTOMER_MODEL_9                                                        = CUSTOMER + 8,
};

#endif
