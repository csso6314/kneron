/********************************************************************
* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
*
* The information contained herein is property of Kneron, Inc.
* Terms and conditions of usage are described in detail in Kneron
* STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information.
* NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
* from the file.
********************************************************************/
#ifndef __UVC_CAMERA_H__
#define __UVC_CAMERA_H__
#include <stdbool.h>
#include <stdint.h>
#include "kmdw_status.h"

#define SCANNING_MODE           0x1
#define AUTO_EXPOSURE_MODE      0x2
#define AUTO_EXPOSURE_PRIORITY  0x4
#define EXPOSURE_TIME_ABSOLUTE  0x8
#define EXPOSURE_TIME_RELATIVE  0x10
#define FOCUS_ABSOLUTE          0x20
#define FOCUS_RELATIVE          0x40
#define IRIS_ABSOLUTE           0x80
#define IRIS_RELATIVE           0x100
#define ZOOM_ABSOLUTE           0x200
#define ZOOM_RELATIVE           0x400
#define PANTILT_ABSOLUTE        0x800
#define PANTILT_RELATIVE        0x1000
#define ROLL_ABSOLUTE           0x2000
#define ROLL_RELATIVE           0x4000
#define FOCUS_AUTO              0x20000
#define PRIVACY                 0x40000
#define FOCUS_SIMPLE            0x80000
#define WINDOW                  0x100000
#define REGION_OF_INTEREST      0x200000

#define BRIGHTNESS                     0x1
#define CONTRAST                       0x2
#define HUE                            0x4
#define SATURATION                     0x8
#define SHARPNESS                      0x10
#define GAMMA                          0x20
#define WHITE_BALANCE_TEMPERATURE      0x40
#define WHITE_BALANCE_COMPONENT        0x80
#define BACKLIGHT_COMPENSATION         0x100
#define GAIN                           0x200
#define POWER_LINE_FREQUENCY           0x400
#define HUE_AUTO                       0x800
#define WHITE_BALANCE_TEMPERATURE_AUTO 0x1000
#define WHITE_BALANCE_COMPONENT_AUTO   0x2000
#define DIGITAL_MULTIPLIER             0x4000
#define DIGITAL_MULTIPLIER_LIMIT       0x8000
#define ANALOG_VIDEO_STANDARD          0x10000
#define ANALOG_VIDEO_LOCK_STATUS       0x20000
#define CONTRAST_AUTO                  0x40000

#define UVC_SET_CUR                     0x01
#define UVC_GET_CUR                     0x81
#define UVC_GET_MIN                     0x82
#define UVC_GET_MAX                     0x83
#define UVC_GET_RES                     0x84
#define UVC_GET_LEN                     0x85
#define UVC_GET_INFO                    0x86
#define UVC_GET_DEF                     0x87

#define SCANNING_MODE_CTL_INTERLACED  0x0
#define SCANNING_MODE_CTL_PROGRESSIVE 0x1
enum scm_req {
    SCM_SET_CUR = 0x01,
    SCM_GET_CUR = 0x81,
    SCM_GET_CAP = 0x86
};

struct ct_scm {
    enum scm_req req;
    uint8_t caps;	
    bool bScanningMode;
};

#define  EXPOSURE_MANUAL_MODE            0x1 
#define  EXPOSURE_AUTO_MODE              0x2
#define  EXPOSURE_SHUTTER_PRIORITY_MODE  0x4 
#define  EXPOSURE_APERTURE_PRIORITY_MODE 0x8
enum aem_req {
    AEM_SET_CUR = 0x01,
    AEM_GET_CUR = 0x81,
    AEM_GET_RES = 0x84,
    AEM_GET_CAP = 0x86,    
    AEM_GET_DEF = 0x87	    
};

struct ct_aem {
    enum aem_req req;
    uint8_t caps;		
    uint8_t bAutoExposureMode;
};

#define  EXPOSURE_FRAME_RATE_CONSTANT  0x0 
#define  EXPOSURE_FRAME_RATE_VARIED    0x1
enum aep_req {
    AEP_SET_CUR = 0x01,
    AEP_GET_CUR = 0x81,
    AEP_GET_CAP = 0x86
};

struct ct_aep {
    enum aep_req req;		
    uint8_t caps;	
    uint8_t bAutoExposurePriority;
};

enum eta_req {
    ETA_SET_CUR = 0x01,
    ETA_GET_CUR = 0x81,
    ETA_GET_MIN = 0x82,
    ETA_GET_MAX = 0x83,
    ETA_GET_RES = 0x84,
    ETA_GET_CAP = 0x86,
    ETA_GET_DEF = 0x87,	
};

struct ct_eta {
    enum eta_req req;
    uint8_t caps;	
    uint32_t bExposureTimeAbsolute;
};

enum etr_req {
    ETR_SET_CUR = 0x01,
    ETR_GET_CUR = 0x81,
    ETR_GET_CAP = 0x86
};

struct ct_etr {
    enum etr_req req;
    uint8_t caps;		
    int8_t bExposureTimeRelative;
};

enum focus_a_req {
    FA_SET_CUR = 0x01,
    FA_GET_CUR = 0x81,
    FA_GET_MIN = 0x82,
    FA_GET_MAX = 0x83,
    FA_GET_RES = 0x84,
    FA_GET_CAP = 0x86,
    FA_GET_DEF = 0x87,	
};

struct ct_focus_a {
    enum focus_a_req req;
    uint8_t caps;		
    uint16_t wFocusAbsolute;
};

enum fr_req {
    FR_SET_CUR = 0x01,
    FR_GET_CUR = 0x81,
    FR_GET_MIN = 0x82,
    FR_GET_MAX = 0x83,
    FR_GET_RES = 0x84,
    FR_GET_CAP = 0x86,
    FR_GET_DEF = 0x87	
};

struct ct_focus_r_data {
    int8_t bFocusRelative;
  	uint8_t	bSpeed;
};

struct ct_focus_r{
    enum fr_req	req;	
    uint8_t caps;	
    struct ct_focus_r_data data;
};

#define  FSR_FULL_RANGE 0x1 
#define  FSR_MACRO      0x2
#define  FSR_PEOPLE     0x3 
#define  FSR_SCENE      0x4
enum fsr_req {
    FSR_SET_CUR = 0x01,
    FSR_GET_CUR = 0x81,
    FSR_GET_CAP = 0x86,
    FSR_GET_DEF = 0x87	
};

struct ct_focus_sr{
    enum fsr_req req;
    uint8_t caps;	
    uint8_t bFocus;
};

enum fauto_req {
    FAUTO_SET_CUR = 0x01,
    FAUTO_GET_CUR = 0x81,
    FAUTO_GET_CAP = 0x86,
    FAUTO_GET_DEF = 0x87	
};

struct ct_fauto{
    enum fauto_req req;	
    uint8_t caps;		
    uint8_t bFocusAuto;
};

enum irisa_req {
    IRISA_SET_CUR = 0x01,
    IRISA_GET_CUR = 0x81,
    IRISA_GET_MIN = 0x82,
    IRISA_GET_MAX = 0x83,
    IRISA_GET_RES = 0x84,
    IRISA_GET_CAP = 0x86,
    IRISA_GET_DEF = 0x87	
};

struct ct_iris_a {
    enum irisa_req req;
    uint8_t caps;		
    uint16_t wIrisAbsolute;
};

enum irisr_req {
    IRISR_SET_CUR = 0x01,
    IRISR_GET_CUR = 0x81,
    IRISR_GET_CAP = 0x86
};

struct ct_iris_r{
    enum irisr_req req;	
    uint8_t caps;		
    uint8_t bIrisRelative;
};

enum zooma_req {
    ZOOMA_SET_CUR = 0x01,
    ZOOMA_GET_CUR = 0x81,
    ZOOMA_GET_MIN = 0x82,
    ZOOMA_GET_MAX = 0x83,
    ZOOMA_GET_RES = 0x84,
    ZOOMA_GET_CAP = 0x86,
    ZOOMA_GET_DEF = 0x87	
};

struct ct_zoom_a{
    enum zooma_req req;
    uint8_t caps;			
    uint16_t wObjectiveFocalLength;
};  

enum zoomr_req {
    ZOOMR_SET_CUR = 0x01,
    ZOOMR_GET_CUR = 0x81,
    ZOOMR_GET_MIN = 0x82,
    ZOOMR_GET_MAX = 0x83,
    ZOOMR_GET_RES = 0x84,
    ZOOMR_GET_CAP = 0x86,
    ZOOMR_GET_DEF = 0x87	
};

#define  ZOOM_STOP          0x0 
#define  ZOOM_TELE_DIR      0x1
#define  ZOOM_WIDE_AGLE_DIR 0xFF 
#define  DIGITAL_ZOOM_OFF   0x0
#define  DIGITAL_ZOOM_ON    0x1
struct ct_zoomr_data{	
    int8_t bZoom;
  	bool bDigitalZoom;
    uint16_t bSpeed;
};

struct ct_zoom_r{
    enum zoomr_req req; 
    uint8_t caps;
    struct ct_zoomr_data data;
};

enum pantilta_req {
    TILTA_SET_CUR = 0x01,
    TILTA_GET_CUR = 0x81,
    TILTA_GET_MIN = 0x82,
    TILTA_GET_MAX = 0x83,
    TILTA_GET_RES = 0x84,
    TILTA_GET_CAP = 0x86,
    TILTA_GET_DEF = 0x87,	
};

struct ct_pan_tilta_data {
    int32_t dwPanAbsolute;
    int32_t dwTiltAbsolute;	
};

struct ct_pan_tilt_a{ 
    enum pantilta_req req;
    uint8_t caps;		
    struct ct_pan_tilta_data data;
};

enum pantiltr_req {
    TILTR_SET_CUR = 0x01,
    TILTR_GET_CUR = 0x81,
    TILTR_GET_MIN = 0x82,
    TILTR_GET_MAX = 0x83,
    TILTR_GET_RES = 0x84,
    TILTR_GET_CAP = 0x86,
    TILTR_GET_DEF = 0x87,	
}; 

#define  PAN_STOP                  0x0 
#define  PAN_CLOSEWISE_DIR         0x1
#define  PAN_COUNTER_CLOSEWISE_DIR 0xFF 
#define  TILT_STOP                 0x0
#define  TILT_POINT_IMG_UP         0x1
#define  TILT_POINT_IMG_DOWN       0xFF
struct ct_pan_tiltr_data {
    int8_t   bPanRelative; 
    uint8_t	 bPanSpeed;
    int8_t   bTiltRelative;
    uint8_t	 bTiltSpeed;
};

struct ct_pan_tilt_r {
    enum pantiltr_req req;
    uint8_t caps;		
    struct ct_pan_tiltr_data data;
};

enum rolla_req {
    ROLLA_SET_CUR = 0x01,
    ROLLA_GET_CUR = 0x81,
    ROLLA_GET_MIN = 0x82,
    ROLLA_GET_MAX = 0x83,
    ROLLA_GET_RES = 0x84,
    ROLLA_GET_CAP = 0x86,
    ROLLA_GET_DEF = 0x87,	
}; 

struct ct_roll_a { 
    enum rolla_req req;	
    uint8_t caps;		
    int16_t wAbsolute;
};

enum rollr_req {
    ROLLR_SET_CUR = 0x01,
    ROLLR_GET_CUR = 0x81,
    ROLLR_GET_MIN = 0x82,
    ROLLR_GET_MAX = 0x83,
    ROLLR_GET_RES = 0x84,
    ROLLR_GET_CAP = 0x86,
    ROLLR_GET_DEF = 0x87,	
}; 

struct ct_rollr_data {
    int8_t bRollRelative;
    uint8_t bSpeed;
};

struct ct_roll_r { 
    enum rollr_req	req;
    uint8_t caps;	
    struct ct_rollr_data data;
};

enum ps_req {
    PS_SET_CUR = 0x01,
    PS_GET_CUR = 0x81,
    PS_GET_CAP = 0x86,
};

#define  SHUTTER_OPEN          0x0 
#define  SHUTTER_CLOSE         0x1
struct ct_privacy_shutter { 
    enum ps_req req; 
    uint8_t caps;	
    bool bPrivacy;
};

enum dwindow_req {
    DWINDOW_SET_CUR = 0x01,
    DWINDOW_GET_CUR = 0x81,
    DWINDOW_GET_MIN = 0x82,
    DWINDOW_GET_MAX = 0x83,
    DWINDOW_GET_DEF = 0x87,	
}; 

struct ct_dwindow_data {
    uint16_t wWindow_Top;
    uint16_t wWindow_Left; 
    uint16_t wWindow_Bottom; 
    uint16_t wWindow_Right; 
    uint16_t wNumSteps;
    uint16_t bmNumStepsUnits;
};

struct ct_dwindow{
    enum dwindow_req req;
    struct ct_dwindow_data data;
};

enum roi_req {
    ROI_SET_CUR = 0x01,
    ROI_GET_CUR = 0x81,
    ROI_GET_MIN = 0x82,
    ROI_GET_MAX = 0x83,
    ROI_GET_DEF = 0x87,	
};

struct ct_roi_data {
    uint16_t wROI_Top; 
    uint16_t wROI_Left; 
    uint16_t wROI_Bottom; 
    uint16_t wROI_Right; 
    uint16_t bmAutoControls;
};

struct ct_roi{
    enum roi_req req;
    struct ct_roi_data data;
};

enum backlight_req {
    BKC_SET_CUR = 0x01,
    BKC_GET_CUR = 0x81,
    BKC_GET_MIN = 0x82,
    BKC_GET_MAX = 0x83,
    BKC_GET_RES = 0x84,
    BKC_GET_CAP = 0x86,
    BKC_GET_DEF = 0x87	
}; 

struct pu_backlight {
    enum backlight_req req;
    uint8_t caps;	
    uint16_t wBacklightCompensation;
};

enum brightness_req {
    BRIGHTNESS_SET_CUR = 0x01,
    BRIGHTNESS_GET_CUR = 0x81,
    BRIGHTNESS_GET_MIN = 0x82,
    BRIGHTNESS_GET_MAX = 0x83,
    BRIGHTNESS_GET_RES = 0x84,
    BRIGHTNESS_GET_CAP = 0x86,
    BRIGHTNESS_GET_DEF = 0x87,	
}; 

struct pu_brightness {
    enum brightness_req req;
    uint8_t caps;
    int16_t wBrightness;
};

enum contrast_req {
    CONTRAST_SET_CUR = 0x01,
    CONTRAST_GET_CUR = 0x81,
    CONTRAST_GET_MIN = 0x82,
    CONTRAST_GET_MAX = 0x83,
    CONTRAST_GET_RES = 0x84,
    CONTRAST_GET_CAP = 0x86,
    CONTRAST_GET_DEF = 0x87,	
}; 

struct pu_contrast {
    enum contrast_req req;	
    uint8_t caps;	
    uint16_t wContrast;
};

enum contrast_auto_req {
    CONTRASTA_SET_CUR = 0x01,
    CONTRASTA_GET_CUR = 0x81,
    CONTRASTA_GET_CAP = 0x86,
    CONTRASTA_GET_DEF = 0x87,	
}; 

struct pu_contrast_auto {
    enum contrast_auto_req req;
    uint8_t caps;		
    uint8_t bContrastAuto;
};

enum gain_req {
    GAIN_SET_CUR = 0x01,
    GAIN_GET_CUR = 0x81,
    GAIN_GET_MIN = 0x82,
    GAIN_GET_MAX = 0x83,
    GAIN_GET_RES = 0x84,
    GAIN_GET_CAP = 0x86,
    GAIN_GET_DEF = 0x87,	
}; 

struct pu_gain {
    enum gain_req	req;
    uint8_t caps;			
    uint16_t wGain;
};

enum power_line_freq_req {
    POWER_LINE_FREQUENCY_SET_CUR = 0x01,
    POWER_LINE_FREQUENCY_GET_CUR = 0x81,
    POWER_LINE_FREQUENCY_GET_CAP = 0x86,
    POWER_LINE_FREQUENCY_GET_DEF = 0x87,	
};

#define PLF_DSIABLE      0x0 
#define PLF_50HZ         0x1
#define PLF_60HZ         0x2
#define PLF_AUTO         0x3
struct pu_power_line_frequency {
    enum power_line_freq_req req;	
    uint8_t caps;		
    uint8_t bPowerLineFrequency;
};

enum hue_req {
    HUE_SET_CUR = 0x01,
    HUE_GET_CUR = 0x81,
    HUE_GET_MIN = 0x82,
    HUE_GET_MAX = 0x83,
    HUE_GET_RES = 0x84,
    HUE_GET_CAP = 0x86,
    HUE_GET_DEF = 0x87,	
}; 

struct pu_hue {
    enum hue_req req;
    uint8_t caps;	 	
    uint16_t wHue;
};

enum hue_auto_req {
    HUEA_SET_CUR = 0x01,
    HUEA_GET_CUR = 0x81,
    HUEA_GET_CAP = 0x86,
    HUEA_GET_DEF = 0x87,	
}; 

struct pu_hue_auto {
    enum hue_auto_req req;
    uint8_t caps;	 	
    uint8_t bHueAuto;
};

enum saturation_req {
    SATURATION_SET_CUR = 0x01,
    SATURATION_GET_CUR = 0x81,
    SATURATION_GET_MIN = 0x82,
    SATURATION_GET_MAX = 0x83,
    SATURATION_GET_RES = 0x84,
    SATURATION_GET_CAP = 0x86,
    SATURATION_GET_DEF = 0x87,	
};

struct pu_saturation {
    enum saturation_req req; 	
    uint8_t caps;	
    uint16_t wSaturation;
};

enum sharpness_req {
    SHARPNESS_SET_CUR = 0x01,
    SHARPNESS_GET_CUR = 0x81,
    SHARPNESS_GET_MIN = 0x82,
    SHARPNESS_GET_MAX = 0x83,
    SHARPNESS_GET_RES = 0x84,
    SHARPNESS_GET_CAP = 0x86,
    SHARPNESS_GET_DEF = 0x87,	
}; 

struct pu_sharpness {
    enum sharpness_req req;
    uint8_t caps;		
    uint16_t wSharpness;
};

enum gamma_req {
    GAMMA_SET_CUR = 0x01,
    GAMMA_GET_CUR = 0x81,
    GAMMA_GET_MIN = 0x82,
    GAMMA_GET_MAX = 0x83,
    GAMMA_GET_RES = 0x84,
    GAMMA_GET_CAP = 0x86,
    GAMMA_GET_DEF = 0x87,	
}; 

struct pu_gamma {
    enum gamma_req req;	
    uint8_t caps;		
    uint16_t wGamma;
};

enum wbt_req {
    WBT_SET_CUR = 0x01,
    WBT_GET_CUR = 0x81,
    WBT_GET_MIN = 0x82,
    WBT_GET_MAX = 0x83,
    WBT_GET_RES = 0x84,
    WBT_GET_CAP = 0x86,
    WBT_GET_DEF = 0x87,	
};

struct pu_white_balance_temp {
    enum wbt_req req;
    uint8_t caps;		 
    uint16_t wWhiteBalanceTemperature;
};

enum wbt_auto_req {
    WBTA_SET_CUR = 0x01,
    WBTA_GET_CUR = 0x81,
    WBTA_GET_CAP = 0x86,
    WBTA_GET_DEF = 0x87,	
}; 

struct pu_white_balance_temp_auto {
    enum wbt_auto_req req;
    uint8_t caps;		
    uint8_t bWhiteBalanceTemperatureAuto;
};

enum whitebalance_comp_req {
    WBC_SET_CUR = 0x01,
    WBC_GET_CUR = 0x81,
    WBC_GET_MIN = 0x82,
    WBC_GET_MAX = 0x83,
    WBC_GET_RES = 0x84,
    WBC_GET_CAP = 0x86,
    WBC_GET_DEF = 0x87,	
};

struct pu_whitebalance_comp_data {
    int16_t wWhiteBalanceBlue;
    int16_t wWhiteBalanceRed;
};

struct pu_whitebalance_comp {
    enum whitebalance_comp_req req;
    uint8_t caps;	
    struct pu_whitebalance_comp_data data;
};

enum wbc_auto_req {
    WBCA_SET_CUR = 0x01,
    WBCA_GET_CUR = 0x81,
    WBCA_GET_CAP = 0x86,
    WBCA_GET_DEF = 0x87,	
}; 

struct pu_wbc_auto {
    enum wbc_auto_req req;
    uint8_t caps;
    uint8_t bWhiteBalanceComponentAuto;
};

enum dmpl_req {
    MPL_SET_CUR = 0x01,
    MPL_GET_CUR = 0x81,
    MPL_GET_MIN = 0x82,
    MPL_GET_MAX = 0x83,
    MPL_GET_RES = 0x84,
    MPL_GET_CAP = 0x86,
    MPL_GET_DEF = 0x87,	
};

struct pu_dmultiplier {
    enum dmpl_req req;	
    uint8_t caps;		
    uint16_t wMultiplierStep;
};

enum dmpl_limit_req {
    DMPL_SET_CUR = 0x01,
    DMPL_GET_CUR = 0x81,
    DMPL_GET_MIN = 0x82,
    DMPL_GET_MAX = 0x83,
    DMPL_GET_RES = 0x84,
    DMPL_GET_CAP = 0x86,
    DMPL_GET_DEF = 0x87,	
}; 

struct pu_dmultiplierlimit {
    enum dmpl_limit_req req;	
    uint8_t caps;			
    uint16_t wMultiplierLimit;
};

#endif // __UVC_CAMERA_H__

