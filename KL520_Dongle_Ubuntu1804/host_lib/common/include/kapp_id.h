/**
 * Copyright (C) 2020 Kneron Inc. All right reserved
 *
 * @file kapp_id.h
 * @brief Kneron Application Shared Data
 */

#ifndef __KAPP_ID_H__
#define __KAPP_ID_H__

enum APP_ID {
    //available device:   KL520    KL720
    APP_UNKNOWN = 0,
    APP_RESERVE,       //    v
    APP_SFID,          //    v
    APP_DME,           //    v       v
    APP_AGE_GENDER,    //    v
    APP_OD,            //    v       
    APP_TINY_YOLO3,    //    v
    APP_FD_LM,         //    v
    APP_PD,            //    v
    APP_CENTER_APP,    //            v
	APP_2PASS_OD,      //            v
    APP_JPEG_ENC,      //            v
    APP_JPEG_DEC,      //            v
	APP_YAWNING,       //    v
    APP_ID_MAX     // must be the last entry
};

#endif // __KAPP_ID_H__
