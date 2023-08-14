/*
 * KDP UVC test and example
 *
 * Copyright (C) 2019 - 2020 Kneron, Inc. All rights reserved.
 *
 */
 
#include "kmdw_camera.h"
#include "kmdw_console.h"
#include "uvc_camera.h" 
#include "kmdw_status.h"
#ifdef KDP_UVC_DEBUG
int uvc_camera_cotrol_list(uint32_t cam_idx)
{
    uint32_t ctl_list[2] = {0,0};

  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_LIST_ALL, ctl_list, sizeof (ctl_list)))
    {
        info_msg("***  UVC list error\n");
        return -1;
    }
    info_msg("***  UVC control list \n");
    if (ctl_list[0] & SCANNING_MODE) {
        info_msg("Scanning Mode \n");
    }
    if (ctl_list[0] & AUTO_EXPOSURE_MODE) {
        info_msg("Auto-Exposure Mode \n");
    }
    if (ctl_list[0] & AUTO_EXPOSURE_PRIORITY) {
        info_msg("Auto-Exposure Priority \n");
    }
    if (ctl_list[0] & EXPOSURE_TIME_ABSOLUTE) {
        info_msg("Exposure Time (Absolute) \n");
    }
    if (ctl_list[0] & EXPOSURE_TIME_RELATIVE) {
        info_msg("Exposure Time (Relative) \n");
    }
    if (ctl_list[0] & FOCUS_ABSOLUTE) {
        info_msg("Focus (Absolute) \n");
    }
    if (ctl_list[0] & FOCUS_RELATIVE) {
        info_msg("Focus (Relative) \n");
    }
    if (ctl_list[0] & IRIS_ABSOLUTE) {
        info_msg("Iris (Absolute) \n");
    }
    if (ctl_list[0] & IRIS_RELATIVE) {
        info_msg("Iris (Relative) \n");
    }
    if (ctl_list[0] & ZOOM_ABSOLUTE) {
        info_msg("Zoom (Absolute)\n");
    }
    if (ctl_list[0] & ZOOM_RELATIVE) {
        info_msg("Zoom (Relative) \n");
    }
    if (ctl_list[0] & PANTILT_ABSOLUTE) {
        info_msg("Pan (Absolute) \n");
    }
    if (ctl_list[0] & PANTILT_RELATIVE) {
        info_msg("Pan (Relative) \n");
    }
    if (ctl_list[0] & ROLL_ABSOLUTE) {
        info_msg("Roll (Absolute) \n");
    }
    if (ctl_list[0] & ROLL_RELATIVE) {
        info_msg("Roll (Relative) \n");
    }
    if (ctl_list[0] & FOCUS_AUTO) {
        info_msg("focus, auto\n");
    }
    if (ctl_list[0] & PRIVACY) {
        info_msg("privacy \n");
    }
    if (ctl_list[0] & FOCUS_SIMPLE) {
        info_msg("focus,simple \n");
    }
    if (ctl_list[0] & WINDOW) {
        info_msg("window \n");
    }
    if (ctl_list[0] & REGION_OF_INTEREST) {
        info_msg("region_of_interest \n");
    }

    if (ctl_list[1] & BRIGHTNESS) {
        info_msg("Brightness \n");
    }
    if (ctl_list[1] & CONTRAST) {
        info_msg("Contrast \n");
    }
    if (ctl_list[1] & HUE) {
        info_msg("Hue \n");
    }
    if (ctl_list[1] & SATURATION) {
        info_msg("Saturation \n");
    }
    if (ctl_list[1] & SHARPNESS) {
        info_msg("Sharpness \n");
    }
    if (ctl_list[1] & GAMMA) {
        info_msg("Gamma \n");
    }
    if (ctl_list[1] & WHITE_BALANCE_TEMPERATURE) {
        info_msg("White Balance Temperature \n");
    }
    if (ctl_list[1] & WHITE_BALANCE_COMPONENT) {
        info_msg("White Balance Component \n");
    }
    if (ctl_list[1] & BACKLIGHT_COMPENSATION) {
        info_msg("Backlight Compensation \n");
    }
    if (ctl_list[1] & GAIN) {
        info_msg("Gain \n");
    }
    if (ctl_list[1] & POWER_LINE_FREQUENCY) {
        info_msg("Power Line Frequency \n");
    }
    if (ctl_list[1] & HUE_AUTO) {
        info_msg("Hue, Auto \n");
    }
    if (ctl_list[1] & WHITE_BALANCE_TEMPERATURE_AUTO) {
        info_msg("White Balance Temperature, Auto \n");
    }
    if (ctl_list[1] & WHITE_BALANCE_COMPONENT_AUTO) {
        info_msg("White Balance Component, Auto \n");
    }
    if (ctl_list[1] & DIGITAL_MULTIPLIER) {
        info_msg("Digital Multiplier \n");
    }
    if (ctl_list[1] & DIGITAL_MULTIPLIER_LIMIT) {
        info_msg("Digital Multiplier Limit \n");
    }
    if (ctl_list[1] & ANALOG_VIDEO_STANDARD) {
        info_msg("analog_video_standard \n");
    }
    if (ctl_list[1] & ANALOG_VIDEO_LOCK_STATUS) {
        info_msg("analog_video_lock_status \n");
    }
    if (ctl_list[1] & CONTRAST_AUTO) {
        info_msg("auto-contrast mode \n");
    }

    info_msg("----------------------------------- \n");
    return 0;
}


static void uvc_scanning_mode_test(int cam_idx)
{
    struct ct_scm  scm;
    info_msg("***  UVC test scanning mode \n");    
	  
    scm.req = SCM_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SCANNING_MODE, &scm, sizeof (struct ct_scm)))
    {
        info_msg("***  UVC scanning mode is not supported \n");
        return;
    }
    info_msg("***  UVC  scanning mode cap is  0x%x\n", scm.caps);
    scm.bScanningMode = 0; 
    scm.req = SCM_GET_CUR;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SCANNING_MODE, &scm, sizeof (struct ct_scm)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }
		 
    if (scm.bScanningMode == SCANNING_MODE_CTL_INTERLACED )
        info_msg("***  UVC  scanning mode is interlaced \n");
			
    if (scm.bScanningMode == SCANNING_MODE_CTL_PROGRESSIVE )
	    info_msg("***  UVC  scanning mode is progressive \n");
    
    scm.bScanningMode = SCANNING_MODE_CTL_PROGRESSIVE;		
    scm.req = SCM_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SCANNING_MODE, &scm, sizeof (struct ct_scm)))
    {
        info_msg("***  UVC SET current scanning mode is error \n");
        return;
    }

    info_msg("***  UVC  test verfication \n");
    scm.bScanningMode = 0; 
    scm.req = SCM_GET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SCANNING_MODE, &scm, sizeof (struct ct_scm)))
    {
        info_msg("***  UVC SET current scanning mode is error \n");
        return;
    }

    info_msg("***  UVC  current scanning mode is 0x%x \n", scm.bScanningMode);
    info_msg("***  UVC test  pass \n");
}	

static void uvc_exposure_mode_test(int cam_idx)
{
    struct ct_aem aem;

    info_msg("***  UVC test exposure mode \n");	
    aem.req = AEM_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_MODE, &aem, sizeof (struct ct_aem)))
    {
        info_msg("***  UVC CID_AUTO_EXPOSURE_MODE is not supported \n");
        return;
    }
    info_msg("***  UVC  exposure mode cap is  0x%x\n", aem.caps);    
    aem.bAutoExposureMode = 0;
    aem.req = AEM_GET_DEF;
  
    if (KMDW_STATUS_ERROR ==  kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_MODE, &aem, sizeof (struct ct_aem)))
    {
        info_msg("***  UVC exposure mode is not supported \n");
        return;
    }
  
    if (aem.bAutoExposureMode & EXPOSURE_MANUAL_MODE)
        info_msg("***  default is  exposure manual mode \n");			
    if (aem.bAutoExposureMode & EXPOSURE_AUTO_MODE)
        info_msg("***  default is exposure auto mode \n");						
    if (aem.bAutoExposureMode & EXPOSURE_SHUTTER_PRIORITY_MODE)
        info_msg("***  default is exposure shutter priority mode \n");				
    if (aem.bAutoExposureMode & EXPOSURE_APERTURE_PRIORITY_MODE)
        info_msg("***  default is exposure aperture priority mode \n");

    aem.bAutoExposureMode = 0;
    aem.req = AEM_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_MODE, &aem, sizeof (struct ct_aem)))
    {
        info_msg("***  UVC resolution is not supported \n");
        return;
    }
    if (aem.bAutoExposureMode & EXPOSURE_MANUAL_MODE)
        info_msg("***  manual mode is supported  \n");			
    if (aem.bAutoExposureMode & EXPOSURE_AUTO_MODE)
        info_msg("***  exposure auto mode is supported \n");						
    if (aem.bAutoExposureMode & EXPOSURE_SHUTTER_PRIORITY_MODE)
        info_msg("***  exposure shutter priority mode is supported\n");				
    if (aem.bAutoExposureMode & EXPOSURE_APERTURE_PRIORITY_MODE)
        info_msg("***  exposure aperture priority mode is supported\n");
    
    aem.bAutoExposureMode = 0;
    aem.req = AEM_GET_CUR;
  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_MODE, &aem, sizeof (struct ct_aem)))
    {
        info_msg("***  UVC exposure mode is not supported \n");
        return;
    }
  
    if (aem.bAutoExposureMode & EXPOSURE_MANUAL_MODE)
        info_msg("***  current is  exposure manual mode \n");			
    if (aem.bAutoExposureMode & EXPOSURE_AUTO_MODE)
        info_msg("***  current is exposure auto mode \n");						
    if (aem.bAutoExposureMode & EXPOSURE_SHUTTER_PRIORITY_MODE)
        info_msg("***  current is exposure shutter priority mode \n");				
    if (aem.bAutoExposureMode & EXPOSURE_APERTURE_PRIORITY_MODE)
        info_msg("***  current is exposure aperture priority mode \n");
    
    aem.bAutoExposureMode = EXPOSURE_MANUAL_MODE;
    aem.req = AEM_SET_CUR;  

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_MODE, &aem, sizeof (struct ct_aem)))
    {
        info_msg("***  UVC exposure mode is not supported \n");
        return;
    }
    aem.bAutoExposureMode = 0;
    aem.req = AEM_GET_CUR;    
     
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_MODE, &aem, sizeof (struct ct_aem)))
    {
        info_msg("***  UVC GET exposure mode  is error \n");
        return;
    }
    if (aem.bAutoExposureMode & EXPOSURE_MANUAL_MODE)
        info_msg("***  current exposure manual mode \n");			
    if (aem.bAutoExposureMode & EXPOSURE_AUTO_MODE)
        info_msg("***  current exposure auto mode \n");						
    if (aem.bAutoExposureMode & EXPOSURE_SHUTTER_PRIORITY_MODE)
        info_msg("***  current exposure shutter priority mode \n");				
    if (aem.bAutoExposureMode & EXPOSURE_APERTURE_PRIORITY_MODE)
        info_msg("***  current exposure aperture priority mode \n");				

    info_msg("***  UVC test  pass \n");	
}

static void uvc_exposure_priority_test(int cam_idx)
{
    struct ct_aep  aep;
    info_msg("***  UVC test exposure priority mode \n");    
    aep.req = AEP_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_PRIORITY, &aep, sizeof (struct ct_aep)))
    {
        info_msg("***  UVC exposure priority is not supported \n");
        return;
    }
    info_msg("***  UVC  exposure priority cap is  0x%x\n", aep.caps);
    aep.bAutoExposurePriority = 0; 
    aep.req = AEP_GET_CUR;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_PRIORITY, &aep, sizeof (struct ct_aep)))
    {
        info_msg("***  UVC GET exposure priority mode is error \n");
        return;
    }
	info_msg("***  UVC  current exposure priority is 0x%x \n", aep.bAutoExposurePriority);
    
    aep.bAutoExposurePriority = EXPOSURE_FRAME_RATE_CONSTANT;		
    aep.req = AEP_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_PRIORITY, &aep, sizeof (struct ct_aep)))
    {
        info_msg("***  UVC SET exposure priority is error,  camera is not in Auto Mode or Shuttter Priority Mode \n");
        return;
    }

    info_msg("***  UVC  test verfication \n");
    aep.bAutoExposurePriority = 0; 
    aep.req = AEP_GET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_PRIORITY, &aep, sizeof (struct ct_aep)))
    {
        info_msg("***  UVC GET exposure priority is error \n");
        return;
    }

    info_msg("***  UVC  current exposure priority is 0x%x \n", aep.bAutoExposurePriority);
    info_msg("***  UVC test  pass \n");
}	

static void uvc_exposure_time_test(uint32_t cam_idx)
{
    struct ct_eta eta;
    
    info_msg("***  UVC  test exposure time \n");
    eta.caps = 0;
    eta.req = ETA_GET_CAP;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_ABSOLUTE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC exposure time is not supported \n");
        return;
    }
    info_msg("***  UVC  exposure priority time caps is  0x%x\n", eta.caps); 
    
    eta.bExposureTimeAbsolute = 0;
    eta.req = ETA_GET_DEF;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_ABSOLUTE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC exposure time is not supported \n");
        return;
    }   
    info_msg("***  UVC  default exposure priority time  is  0x%x\n", eta.bExposureTimeAbsolute);    

    eta.bExposureTimeAbsolute = 0;
    eta.req = ETA_GET_CUR;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_ABSOLUTE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC exposure time is not supported \n");
        return;
    }   
    info_msg("***  UVC  current exposure priority time  is  0x%x\n", eta.bExposureTimeAbsolute);    
    
    eta.bExposureTimeAbsolute = 0;
    eta.req = ETA_GET_MIN;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_ABSOLUTE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC exposure time is not supported \n");
        return;
    }   
    info_msg("***  UVC  min exposure priority time  is  0x%x\n", eta.bExposureTimeAbsolute);    
    
    eta.bExposureTimeAbsolute = 0;
    eta.req = ETA_GET_MAX;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_ABSOLUTE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC exposure time is not supported \n");
        return;
    }   
    info_msg("***  UVC  max exposure priority time  is  0x%x\n", eta.bExposureTimeAbsolute);    
       
    eta.bExposureTimeAbsolute = 0;
    eta.req = ETA_GET_RES;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_ABSOLUTE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC resolution is not supported \n");
        return;
    }   
    info_msg("***  UVC   exposure priority resolution  is  0x%x\n", eta.bExposureTimeAbsolute);    

   
    eta.bExposureTimeAbsolute = 4; 
    if (!(eta.caps & 0x2))
    {
        info_msg("***  UVC set exposure time not support \n");        
       return;
    }
    eta.bExposureTimeAbsolute = 4;     
    eta.req = ETA_SET_CUR;  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_ABSOLUTE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC set exposure time error , camera is not in Auto Mode or Shuttter Priority Mode\n");
        return;
    }    
	
    eta.bExposureTimeAbsolute = 0;  
    eta.req = ETA_GET_CUR;  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_AUTO_EXPOSURE_MODE, &eta, sizeof (struct ct_eta)))
    {
        info_msg("***  UVC get exposure time error \n");
        return;
    }    
    info_msg("***  UVC  exposure time is 0x%x \n", eta.bExposureTimeAbsolute);
    info_msg("***  UVC test  pass \n");		
}

static void uvc_shutter_speed_test(int cam_idx)
{
    struct ct_etr etr;
   
    info_msg("***  UVC  test shutter speed \n");
    etr.caps = 0;
    etr.req = ETR_GET_CAP;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_RELATIVE, &etr, sizeof (struct ct_etr)))
    {
        info_msg("***  UVC shutter speed  is not supported \n");
        return;
    }
    info_msg("***  UVC  shutter speed caps is  0x%x\n", etr.caps);
    
    etr.bExposureTimeRelative = 0;
    etr.req = ETR_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_RELATIVE, &etr, sizeof (struct ct_etr)))
    {
        info_msg("***  UVC shutter speed  is not supported \n");
        return;
    }
    info_msg("***  UVC  shutter speed is  0x%x\n", etr.bExposureTimeRelative);
        
    etr.bExposureTimeRelative = 0xFF;
    etr.req = ETR_SET_CUR;    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_RELATIVE, &etr, sizeof (struct ct_etr)))
    {
        info_msg("***  UVC shutter speed is not supported \n");
        return;
    }    
    etr.bExposureTimeRelative = 0;
    etr.req = ETR_GET_CUR;    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_EXPOSURE_TIME_RELATIVE, &etr, sizeof (struct ct_etr)))
    {
        info_msg("***  UVC shutter speed is not supported \n");
        return;
    }        
    info_msg("***  UVC  shutter speed is  0x%x\n", etr.bExposureTimeRelative);
    info_msg("***  UVC test  pass \n");		
}


static void uvc_focus_absolute_test(uint32_t cam_idx)
{
    struct ct_focus_a focus;
    info_msg("***  UVC test focues absolute focus \n");
   
    focus.caps = 0;
    focus.req = FA_GET_CAP;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC focus absolute is not supported \n");
        return;
    }
    info_msg("***  UVC  focus absolute caps is  0x%x\n", focus.caps);

    focus.wFocusAbsolute = 0;
    focus.req = FA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC focus absolute is not supported \n");
        return;
    }
    info_msg("***  UVC  default focus absolute is 0x%x\n", focus.wFocusAbsolute);

    focus.wFocusAbsolute = 0;
    focus.req = FA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC focus absolute is not supported \n");
        return;
    }
    info_msg("***  UVC  current focus absolute is 0x%x\n", focus.wFocusAbsolute);

    focus.wFocusAbsolute = 0;
    focus.req = FA_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC focus absolute is not supported \n");
        return;
    }
    info_msg("***  UVC  min focus absolute is 0x%x\n", focus.wFocusAbsolute);

    focus.wFocusAbsolute = 0;
    focus.req = FA_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC focus absolute is not supported \n");
        return;
    }
    info_msg("***  UVC  maximum focus absolute is 0x%x\n", focus.wFocusAbsolute);
    
    focus.wFocusAbsolute = 0;
    focus.req = FA_GET_RES;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC focus resolution is not supported \n");
        return;
    }   
    info_msg("***  UVC   focus absolute resolution  is  0x%x\n", focus.wFocusAbsolute);    
    if (!(focus.caps & 0x2))
    {
        info_msg("***  UVC set exposure time not support \n");        
       return;
    }
    focus.wFocusAbsolute = 2;
    focus.req = FA_SET_CUR;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC camera is auto-focus mode \n");
        return;
    }   
    focus.wFocusAbsolute = 0;
    focus.req = FA_GET_CUR;   
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_ABSOLUTE, &focus, sizeof (struct ct_focus_a)))
    {
        info_msg("***  UVC GET focus absolute \n");
        return;
    }   
    info_msg("***  UVC  current focus absolute  is 0x%x \n", focus.wFocusAbsolute);		
    info_msg("***  UVC test  pass \n");		
}

static void uvc_focus_relative_test(int cam_idx)
{
    struct ct_focus_r focus_r;

    info_msg("***  UVC test focus relative length \n");

    focus_r.req = FR_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC focus relative is not supported \n");
        return;
    }
    info_msg("***  UVC  focus relative cap is  0x%x\n", focus_r.caps);
    
    focus_r.data.bFocusRelative = 0;
    focus_r.data.bSpeed = 0;
    focus_r.req = FR_GET_DEF;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC GET current focus relative is error \n");
        return;
    }
    info_msg("***  UVC  default focus relative is  0x%x\n", focus_r.data.bFocusRelative);
    info_msg("***  UVC  default Speed is  0x%x\n", focus_r.data.bSpeed); 
        
    
    focus_r.data.bFocusRelative = 0;
    focus_r.data.bSpeed = 0;
    focus_r.req = FR_GET_CUR;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC GET current focus relative is error \n");
        return;
    }
    info_msg("***  UVC  current focus relative is  0x%x\n", focus_r.data.bFocusRelative);
    info_msg("***  UVC  current Speed is  0x%x\n", focus_r.data.bSpeed); 
    
    focus_r.data.bFocusRelative = 0;
    focus_r.data.bSpeed = 0;
    focus_r.req = FR_GET_MIN;    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC GET current focus relative is error \n");
        return;
    }
    info_msg("***  UVC  Mim focus relative is  0x%x\n", focus_r.data.bFocusRelative);
    info_msg("***  UVC  Mim Speed is  0x%x\n", focus_r.data.bSpeed);   
    
    focus_r.data.bFocusRelative = 0;
    focus_r.data.bSpeed = 0;
    focus_r.req = FR_GET_MAX;    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC GET current focus relative is error \n");
        return;
    }
    info_msg("***  UVC  Max focus relative is  0x%x\n", focus_r.data.bFocusRelative);
    info_msg("***  UVC  Max Speed is  0x%x\n", focus_r.data.bSpeed);  

    focus_r.data.bFocusRelative = 0;
    focus_r.data.bSpeed = 0;
    focus_r.req = FR_GET_RES;    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC GET current focus resolution is error \n");
        return;
    }
    info_msg("***  UVC  focus relative resolution is  0x%x\n", focus_r.data.bFocusRelative);
    info_msg("***  UVC  Speed resolution is  0x%x\n", focus_r.data.bSpeed);       
    
    focus_r.data.bFocusRelative = 0xFF;
    focus_r.data.bSpeed = 1;
    focus_r.req = FR_SET_CUR;      

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC SET focus relative is error \n");
        return;
    }

    info_msg("***  UVC  test verfication \n");
    focus_r.data.bFocusRelative = 0xFF;
    focus_r.data.bSpeed = 1;
    focus_r.req = FR_GET_CUR;      

    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_RELATIVE, &focus_r, sizeof (struct ct_focus_r)))
    {
        info_msg("***  UVC GET focus relative is error \n");
        return;
    }

    info_msg("***  UVC  current focus relative is 0x%x \n", focus_r.data.bFocusRelative);
    info_msg("***  UVC  current speed is 0x%x \n", focus_r.data.bSpeed);    
    info_msg("***  UVC test  pass \n");
	
}

static void uvc_focus_len_range_test(int cam_idx)
{
    struct ct_focus_sr focus_sr;
    info_msg("***  UVC test focus len range \n");    
	  
    focus_sr.req = FSR_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_SIMPLE, &focus_sr, sizeof (struct ct_focus_sr)))
    {
        info_msg("***  UVC focus len range is not supported \n");
        return;
    }
    info_msg("***  UVC  focus len range cap is  0x%x\n", focus_sr.caps);
    focus_sr.bFocus = 0; 
    focus_sr.req = FSR_GET_DEF;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_SIMPLE, &focus_sr, sizeof (struct ct_focus_sr)))
    {
        info_msg("***  UVC GET current focus len range is error \n");
        return;
    }
    info_msg("***  UVC  focus len range is  0x%x\n", focus_sr.bFocus);	

    focus_sr.bFocus = 0;
    focus_sr.req = FSR_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_SIMPLE, &focus_sr, sizeof (struct ct_focus_sr)))
    {
        info_msg("***  UVC GET current focus len range is error \n");
        return;
    }
    info_msg("***  UVC  focus len range is  0x%x\n", focus_sr.bFocus);	

    focus_sr.bFocus = FSR_FULL_RANGE;
    focus_sr.req = FSR_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_SIMPLE, &focus_sr, sizeof (struct ct_focus_sr)))
    {
        info_msg("***  UVC SET current focus len range is error \n");
        return;
    }

    info_msg("***  UVC  test verfication \n");
    focus_sr.bFocus = 0; 
    focus_sr.req = FSR_GET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_SIMPLE, &focus_sr, sizeof (struct ct_focus_sr)))
    {
        info_msg("***  UVC SET current focus len range is error \n");
        return;
    }

    info_msg("***  UVC  current focus len range is 0x%x \n", focus_sr.bFocus);
    info_msg("***  UVC test  pass \n");
}

static void uvc_focus_auto_test(int cam_idx)
{
    struct ct_fauto fauto;
    info_msg("***  UVC test focus auto \n");    
	  
    fauto.req = FAUTO_GET_CAP;
    fauto.caps = 0;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_AUTO, &fauto, sizeof (struct ct_fauto)))
    {
        info_msg("***  UVC focus auto is not supported \n");
        return;
    }
    info_msg("***  UVC  focus auto cap is  0x%x\n", fauto.caps);
    
    fauto.bFocusAuto = 0; 
    fauto.req = FAUTO_GET_DEF;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_AUTO, &fauto, sizeof (struct ct_fauto)))
    {
        info_msg("***  UVC GET default focus  is error \n");
        return;
    }
    info_msg("***  UVC  default focus auto  is  0x%x\n", fauto.bFocusAuto);		 
    fauto.bFocusAuto = 0;
    fauto.req = FAUTO_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_AUTO, &fauto, sizeof (struct ct_fauto)))
    {
        info_msg("***  UVC GET current focus len range is error \n");
        return;
    }
    info_msg("***  UVC  current focus auto  is  0x%x\n", fauto.bFocusAuto);		    
    fauto.bFocusAuto = 1; 
    fauto.req = FAUTO_SET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_AUTO, &fauto, sizeof (struct ct_fauto)))
    {
        info_msg("***  UVC SET current focus is error \n");
        return;
    }

    info_msg("***  UVC  test verfication \n");
    fauto.bFocusAuto = 0; 
    fauto.req = FAUTO_GET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_FOCUS_AUTO, &fauto, sizeof (struct ct_fauto)))
    {
        info_msg("***  UVC GET current focus is error \n");
        return;
    }    
    info_msg("***  UVC  current focus auto  is  0x%x\n", fauto.bFocusAuto);		    
    info_msg("***  UVC test  pass \n");
}    


static void uvc_iris_absolute_test(int cam_idx)
{
    struct ct_iris_a iris;
    info_msg("***  UVC test aperture setting\n");
    
    iris.req = IRISA_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC iris absolute is not supported \n");
        return;
    }
    info_msg("***  UVC  iris absolute cap is  0x%x\n", iris.caps);
    iris.wIrisAbsolute = 0; 
    iris.req = IRISA_GET_RES;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC GET iris resolution is error \n");
        return;
    }
    info_msg("***  UVC  iris absolute resolution  is  0x%x\n", iris.wIrisAbsolute);
    
    iris.wIrisAbsolute = 0; 
    iris.req = IRISA_GET_DEF;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC GET default iris is error \n");
        return;
    }
    info_msg("***  UVC  default  iris absolute  is  0x%x\n", iris.wIrisAbsolute);
        
    iris.wIrisAbsolute = 0; 
    iris.req = IRISA_GET_MIN;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC GET iris min is error \n");
        return;
    }
    info_msg("***  UVC  min iris absolute  is  0x%x\n", iris.wIrisAbsolute);
            
    iris.wIrisAbsolute = 0;     
    iris.req = IRISA_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC GET iris max is error \n");
        return;
    }
    info_msg("***  UVC  max iris absolute  is  0x%x\n", iris.wIrisAbsolute);
    
    iris.wIrisAbsolute = 0;     
    iris.req = IRISA_GET_CUR;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC GET current iris is error \n");
        return;
    } 
    info_msg("***  UVC  current iris absolute  is  0x%x\n", iris.wIrisAbsolute);  
    if (!(iris.caps & 0x2))
    {
        info_msg("***  UVC set  iris absolute is not supported \n");        
       return;
    }    
    iris.wIrisAbsolute = 0;     
    iris.req = IRISA_SET_CUR;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC GET current iris is error \n");
        return;
    }
   
    info_msg("***  UVC  test verfication \n");
    iris.wIrisAbsolute = 0;     
    iris.req = IRISA_GET_CUR;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_ABSOLUTE, &iris, sizeof (struct ct_iris_a)))
    {
        info_msg("***  UVC GET current iris is error \n");
        return;
    }
    info_msg("***  UVC  current iris is 0x%x \n", iris.wIrisAbsolute);

    info_msg("***  UVC test  pass \n");			
}

static void uvc_iris_relative_test(int cam_idx)
{
    struct ct_iris_r iris;
    info_msg("***  UVC test iris relative\n");
 
    iris.req = IRISR_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_RELATIVE, &iris, sizeof (struct ct_iris_r)))
    {
        info_msg("***  UVC  iris relative is not supported \n");
        return;
    }
    info_msg("***  UVC  iris relative caps is  0x%x\n", iris.caps);
    iris.bIrisRelative = 0; 
    iris.req = IRISR_GET_CUR;
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_RELATIVE, &iris, sizeof (struct ct_iris_r)))
    {
        info_msg("***  UVC GET iris relative  is error \n");
        return;
    }
    info_msg("***  UVC  current iris is 0x%x \n", iris.bIrisRelative);
    iris.bIrisRelative = 0xFF; 
    iris.req = IRISR_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_RELATIVE, &iris, sizeof (struct ct_iris_r)))
    {
        info_msg("***  UVC SET iris relative is error \n");
        return;
    }

    info_msg("***  UVC  test verfication \n");
    iris.bIrisRelative = 0x0; 
    iris.req = IRISR_GET_CUR;   
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_IRIS_RELATIVE, &iris, sizeof (struct ct_iris_r)))
    {
        info_msg("***  UVC GET iris relative is error \n");
        return;
    }

    info_msg("***  UVC  iris relative is 0x%x \n", iris.bIrisRelative);
    info_msg("***  UVC test  pass \n");			
}

static void uvc_zoom_absolute_test(int cam_idx)
{
    struct ct_zoom_a zoom;
    info_msg("***  UVC test zoom absolute \n");  
    
	zoom.caps = 0; 
    zoom.req = ZOOMA_GET_CAP;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC zoom absolute is not supported \n");
        return;
    }
    info_msg("***  UVC zoom absolute cap is  0x%x\n", zoom.caps);
    
    zoom.wObjectiveFocalLength = 0; 
    zoom.req = ZOOMA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC GET default zoom absolute is error \n");
        return;
    }
    info_msg("***  UVC GET default zoom is  0x%x\n", zoom.wObjectiveFocalLength);	
    
    zoom.wObjectiveFocalLength = 0; 
    zoom.req = ZOOMA_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC GET min zoom absolute is error \n");
        return;
    }
    info_msg("***  UVC GET min zoom absolute is  0x%x\n", zoom.wObjectiveFocalLength);
    
    zoom.wObjectiveFocalLength = 0; 
    zoom.req = ZOOMA_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC max zoom absolute is error \n");
        return;
    }
    info_msg("***  UVC GET max zoom absolute is  0x%x\n", zoom.wObjectiveFocalLength);
    zoom.wObjectiveFocalLength = 0; 
    zoom.req = ZOOMA_GET_RES; 
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC resolution zoom absolute is error \n");
        return;
    }
    info_msg("***  UVC  zoom absolute resolution is 0x%x \n", zoom.wObjectiveFocalLength);    
    
    if (!(zoom.caps & 0x2))
    {
        info_msg("***  UVC set exposure time not support \n");        
       return;
    }
    zoom.wObjectiveFocalLength = 0; 
    zoom.req = ZOOMA_GET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC SET zoom absolute is error \n");
        return;
    }
    info_msg("***  UVC  current zoom absolute is 0x%x \n", zoom.wObjectiveFocalLength);        
    zoom.wObjectiveFocalLength = 2; 
    zoom.req = ZOOMA_SET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC SET zoom absolute is error \n");
        return;
    }

    info_msg("***  UVC  test verfication \n");
    zoom.wObjectiveFocalLength = 0; 
    zoom.req = ZOOMA_GET_CUR;    
    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_ABSOLUTE, &zoom, sizeof (struct ct_zoom_a)))
    {
        info_msg("***  UVC SET zoom absolute is error \n");
        return;
    }
    info_msg("***  UVC  current zoom absolute is 0x%x \n", zoom.wObjectiveFocalLength);    
    
    info_msg("***  UVC test  pass \n");		
}

static void uvc_zoom_relative_test(int cam_idx)
{
    struct ct_zoom_r zoom;
    
    info_msg("***  UVC test zoom relative \n");
   
    zoom.req = ZOOMR_GET_CAP;
    zoom.caps = 0;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_RELATIVE, &zoom, sizeof (struct ct_zoom_r)))
    {
        info_msg("***  UVC zoom relative is not supported \n");
        return;
    }
    info_msg("***  UVC  zoom relative cap is  0x%x\n", zoom.caps);

    zoom.req = ZOOMR_GET_DEF;
    zoom.data.bDigitalZoom = 0; 
    zoom.data.bSpeed = 0;
    zoom.data.bZoom = 0;  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_RELATIVE, &zoom, sizeof (struct ct_zoom_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }
    info_msg("***  UVC  default bDigitalZoom is  0x%x\n", zoom.data.bDigitalZoom);		 
    info_msg("***  UVC  default bSpeed is  0x%x\n", zoom.data.bSpeed);	 
    info_msg("***  UVC  default bZoom is  0x%x\n", zoom.data.bZoom);

    zoom.req = ZOOMR_GET_MIN;
    zoom.data.bDigitalZoom = 0; 
    zoom.data.bSpeed = 0;
    zoom.data.bZoom = 0;  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_RELATIVE, &zoom, sizeof (struct ct_zoom_r)))
    {
        info_msg("***  UVC GET min zoom relative is error \n");
        return;
    }
    info_msg("***  UVC  min bDigitalZoom is  0x%x\n", zoom.data.bDigitalZoom);		 
    info_msg("***  UVC  min bSpeed is  0x%x\n", zoom.data.bSpeed);	 
    info_msg("***  UVC  min bZoom is  0x%x\n", zoom.data.bZoom);
    
    zoom.req = ZOOMR_GET_MAX;
    zoom.data.bDigitalZoom = 0; 
    zoom.data.bSpeed = 0;
    zoom.data.bZoom = 0;  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_RELATIVE, &zoom, sizeof (struct ct_zoom_r)))
    {
        info_msg("***  UVC GET max zoom relative is error \n");
        return;
    }
    info_msg("***  UVC  max bDigitalZoom is  0x%x\n", zoom.data.bDigitalZoom);		 
    info_msg("***  UVC  max bSpeed is  0x%x\n", zoom.data.bSpeed);	 
    info_msg("***  UVC  max bZoom is  0x%x\n", zoom.data.bZoom);
    
    zoom.req = ZOOMR_GET_RES;
    zoom.data.bDigitalZoom = 0; 
    zoom.data.bSpeed = 0;
    zoom.data.bZoom = 0;  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_RELATIVE, &zoom, sizeof (struct ct_zoom_r)))
    {
        info_msg("***  UVC GET max zoom resolution is error \n");
        return;
    }
    info_msg("***  UVC  bDigitalZoom resolution is  0x%x\n", zoom.data.bDigitalZoom);		 
    info_msg("***  UVC  bSpeed resolution is  0x%x\n", zoom.data.bSpeed);	 
    info_msg("***  UVC  bZoom resolution is  0x%x\n", zoom.data.bZoom);

    zoom.req = ZOOMR_SET_CUR;    
    zoom.data.bDigitalZoom = 1; 
    zoom.data.bSpeed = 1;
    zoom.data.bZoom = 1;  
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_RELATIVE, &zoom, sizeof (struct ct_zoom_r)))
    {
        info_msg("***  UVC GET max zoom relative is error \n");
        return;
    } 
    info_msg("***  UVC  test verfication \n");
    zoom.req = ZOOMR_GET_CUR;    
    zoom.data.bDigitalZoom = 0; 
    zoom.data.bSpeed = 0;
    zoom.data.bZoom = 0;      
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ZOOM_RELATIVE, &zoom, sizeof (struct ct_zoom_r)))
    {
        info_msg("***  UVC SET current scanning mode is error \n");
        return;
    }
    info_msg("***  UVC  bDigitalZoom resolution is  0x%x\n", zoom.data.bDigitalZoom);		 
    info_msg("***  UVC  bSpeed resolution is  0x%x\n", zoom.data.bSpeed);	 
    info_msg("***  UVC  bZoom resolution is  0x%x\n", zoom.data.bZoom);
    info_msg("***  UVC test  pass \n");		
}
	
static void uvc_pan_tilt_absolute_test(int cam_idx)
{
    struct ct_pan_tilt_a pan_tilt;
    info_msg("***  UVC test pan tilt absolute  \n");
 
    pan_tilt.req = TILTA_GET_CAP;
    pan_tilt.caps = 0;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC pan tilt is not supported \n");
        return;
    }
    info_msg("***  UVC  pan tilt cap is  0x%x\n", pan_tilt.caps);
    
    pan_tilt.data.dwPanAbsolute = 0; 
    pan_tilt.data.dwTiltAbsolute = 0;     
    pan_tilt.req = TILTA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC GET default pan tilt test is error \n");
        return;
    }
    info_msg("***  UVC  default pan tilt dwPanAbsolute is  0x%x\n", pan_tilt.data.dwPanAbsolute);
    info_msg("***  UVC  default pan tilt dwTiltAbsolute is  0x%x\n", pan_tilt.data.dwTiltAbsolute);    
    pan_tilt.data.dwPanAbsolute = 0; 
    pan_tilt.data.dwTiltAbsolute = 0;     
    pan_tilt.req = TILTA_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC GET min pan tilt test is error \n");
        return;
    }
    info_msg("***  UVC  min pan tilt dwPanAbsolute is  0x%x\n", pan_tilt.data.dwPanAbsolute);
    info_msg("***  UVC  min pan tilt dwTiltAbsolute is  0x%x\n", pan_tilt.data.dwTiltAbsolute);      
    pan_tilt.data.dwPanAbsolute = 0; 
    pan_tilt.data.dwTiltAbsolute = 0;     
    pan_tilt.req = TILTA_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC GET max pan tilt test error \n");
        return;
    }
    info_msg("***  UVC  max pan tilt dwPanAbsolute is  0x%x\n", pan_tilt.data.dwPanAbsolute);
    info_msg("***  UVC  max pan tilt dwTiltAbsolute is  0x%x\n", pan_tilt.data.dwTiltAbsolute);  
    pan_tilt.data.dwPanAbsolute = 0; 
    pan_tilt.data.dwTiltAbsolute = 0;     
    pan_tilt.req = TILTA_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC GET pan tilt resolution is error \n");
        return;
    }
    info_msg("***  UVC  resulotion pan tilt dwPanAbsolute is  0x%x\n", pan_tilt.data.dwPanAbsolute);
    info_msg("***  UVC  resulotion pan tilt dwTiltAbsolute is  0x%x\n", pan_tilt.data.dwTiltAbsolute);  
    pan_tilt.data.dwPanAbsolute = 0; 
    pan_tilt.data.dwTiltAbsolute = 0;     
    pan_tilt.req = TILTA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC GET current pan tilt is error \n");
        return;
    }
    info_msg("***  UVC  current pan tilt dwPanAbsolute is  0x%x\n", pan_tilt.data.dwPanAbsolute);
    info_msg("***  UVC  current pan tilt dwTiltAbsolute is  0x%x\n", pan_tilt.data.dwTiltAbsolute); 
    
    info_msg("***  UVC  test verfication \n"); 
    if (!(pan_tilt.caps & 0x2))
    {
        info_msg("***  UVC set exposure time not support \n");        
       return;
    }    
    pan_tilt.data.dwPanAbsolute = 1; 
    pan_tilt.data.dwTiltAbsolute = 1;     
    pan_tilt.req = TILTA_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC SET  pan tilt  is error \n");
        return;
    }    

   
    pan_tilt.data.dwPanAbsolute = 0; 
    pan_tilt.data.dwTiltAbsolute = 0;
    
    pan_tilt.req = TILTA_GET_CUR;    
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_ABSOLUTE, &pan_tilt, sizeof (struct ct_pan_tilt_a)))
    {
        info_msg("***  UVC SET current scanning mode is error \n");
        return;
    }
    info_msg("***  UVC  current pan tilt dwPanAbsolute is  0x%x\n", pan_tilt.data.dwPanAbsolute);
    info_msg("***  UVC  current pan tilt dwTiltAbsolute is  0x%x\n", pan_tilt.data.dwTiltAbsolute); 
    info_msg("***  UVC test  pass \n");	
}

static void uvc_pan_tilt_direction_test(int cam_idx)
{
    struct ct_pan_tilt_r pan_tilt;
    
    info_msg("***  UVC test set pan_tilt direction \n");
    
    pan_tilt.caps = 0; 
    pan_tilt.req = TILTR_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC pan tilt is not supported \n");
        return;
    }
    info_msg("***  UVC  pan tilt cap is  0x%x\n", pan_tilt.caps);
    
    pan_tilt.data.bPanRelative = 0; 
    pan_tilt.data.bPanSpeed = 0;
    pan_tilt.data.bTiltRelative = 0;
    pan_tilt.data.bTiltSpeed = 0;    
    pan_tilt.req = TILTR_GET_DEF;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }
		 
    pan_tilt.data.bPanRelative = 0; 
    pan_tilt.data.bPanSpeed = 0;
    pan_tilt.data.bTiltRelative = 0;
    pan_tilt.data.bTiltSpeed = 0;    
    pan_tilt.req = TILTR_GET_MIN;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }
			
    pan_tilt.data.bPanRelative = 0; 
    pan_tilt.data.bPanSpeed = 0;
    pan_tilt.data.bTiltRelative = 0;
    pan_tilt.data.bTiltSpeed = 0;    
    pan_tilt.req = TILTR_GET_MAX;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }
    
    pan_tilt.data.bPanRelative = 0; 
    pan_tilt.data.bPanSpeed = 0;
    pan_tilt.data.bTiltRelative = 0;
    pan_tilt.data.bTiltSpeed = 0;    
    pan_tilt.req = TILTR_GET_RES;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }

    pan_tilt.data.bPanRelative = 0; 
    pan_tilt.data.bPanSpeed = 0;
    pan_tilt.data.bTiltRelative = 0;
    pan_tilt.data.bTiltSpeed = 0;    
    pan_tilt.req = TILTR_GET_CUR;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }

    pan_tilt.data.bPanRelative = 1; 
    pan_tilt.data.bPanSpeed = 2;
    pan_tilt.data.bTiltRelative = 3;
    pan_tilt.data.bTiltSpeed = 4;    
    pan_tilt.req = TILTR_SET_CUR;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }    
    info_msg("***  UVC  test verfication \n");
    pan_tilt.data.bPanRelative = 0; 
    pan_tilt.data.bPanSpeed = 0;
    pan_tilt.data.bTiltRelative = 0;
    pan_tilt.data.bTiltSpeed = 0;    
    pan_tilt.req = TILTR_GET_CUR;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_PANTILT_RELATIVE, &pan_tilt, sizeof (struct ct_pan_tilt_r)))
    {
        info_msg("***  UVC GET current scanning mode is error \n");
        return;
    }       
    info_msg("***  UVC test  pass \n");		
}

static void uvc_roll_test(int cam_idx)
{
    struct ct_roll_a roll;
    
    info_msg("***  UVC test roll \n");
 
    roll.req = ROLLA_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC roll is not supported \n");
        return;
    }
    info_msg("***  UVC  roll cap is  0x%x\n", roll.caps);
    
    roll.wAbsolute = 0; 
    roll.req = ROLLA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  default roll is  0x%x\n", roll.wAbsolute);		 
    roll.wAbsolute = 0; 
    roll.req = ROLLA_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  min roll is  0x%x\n", roll.wAbsolute);				
    roll.wAbsolute = 0; 
    roll.req = ROLLA_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  max roll is  0x%x\n", roll.wAbsolute);	    
    roll.wAbsolute = 0; 
    roll.req = ROLLA_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC GET resolution roll is error \n");
        return;
    }
    info_msg("***  UVC  resolution roll is  0x%x\n", roll.wAbsolute);	
    roll.wAbsolute = 0; 
    roll.req = ROLLA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }    
    info_msg("***  UVC  current roll is  0x%x\n", roll.wAbsolute); 
    if (!(roll.caps & 0x2))
    {
       info_msg("***  UVC set roll supported \n");        
       return;
    }        
    roll.wAbsolute = 1; 
    roll.req = ROLLA_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC SET set curret is error \n");
        return;
    }    

    roll.wAbsolute = 0; 
    roll.req = ROLLA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_ABSOLUTE, &roll, sizeof (struct ct_roll_a)))
    {
        info_msg("***  UVC GET current roll is error \n");
        return;
    }  
    info_msg("***  UVC  current roll is  0x%x\n", roll.wAbsolute);     
    info_msg("***  UVC test  pass \n");		
}

static void uvc_roll_direction_test(int cam_idx)
{
    struct ct_roll_r roll;
    roll.req = ROLLR_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC roll relative is not supported \n");
        return;
    }
    info_msg("***  UVC  roll cap is  0x%x\n", roll.caps);
    
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll relative is error \n");
        return;
    }
    info_msg("***  UVC  default roll bRollRelative is  0x%x\n", roll.data.bRollRelative);     
    info_msg("***  UVC  default roll bSpeed is  0x%x\n", roll.data.bSpeed); 		 
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET min roll is error \n");
        return;
    }
    info_msg("***  UVC  min roll bRollRelative is  0x%x\n", roll.data.bRollRelative);     
    info_msg("***  UVC  min roll bSpeed is  0x%x\n", roll.data.bSpeed); 		 			
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET max roll is error \n");
        return;
    }
    info_msg("***  UVC  max roll bRollRelative is  0x%x\n", roll.data.bRollRelative);     
    info_msg("***  UVC  max roll bSpeed is  0x%x\n", roll.data.bSpeed); 
    
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET resolution roll is error \n");
        return;
    }
    info_msg("***  UVC  default resolution bRollRelative is  0x%x\n", roll.data.bRollRelative);     
    info_msg("***  UVC  default resolution bSpeed is  0x%x\n", roll.data.bSpeed); 		 
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET currnt roll is error \n");
        return;
    }
    info_msg("***  UVC  current  bRollRelative is  0x%x\n", roll.data.bRollRelative);     
    info_msg("***  UVC  current  bSpeed is  0x%x\n", roll.data.bSpeed); 		     
    roll.data.bRollRelative = 1; 
    roll.data.bSpeed = 1;    
    roll.req = ROLLR_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    } 
    info_msg("***  UVC  current  bRollRelative is  0x%x\n", roll.data.bRollRelative);     
    info_msg("***  UVC  current  bSpeed is  0x%x\n", roll.data.bSpeed); 	    
    info_msg("***  UVC test  pass \n");		
}

static void uvc_shutter_test(int cam_idx)
{
    struct ct_roll_r roll;
    roll.req = ROLLR_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC roll is not supported \n");
        return;
    }
    info_msg("***  UVC  roll cap is  0x%x\n", roll.caps);
    
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
		 
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
			
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET resolution roll is error \n");
        return;
    }

    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    
    roll.data.bRollRelative = 1; 
    roll.data.bSpeed = 1;    
    roll.req = ROLLR_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    
    roll.data.bRollRelative = 0; 
    roll.data.bSpeed = 0;    
    roll.req = ROLLR_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_ROLL_RELATIVE, &roll, sizeof (struct ct_roll_r)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }    
    info_msg("***  UVC test  pass \n");		
}


static void uvc_digital_windows_test(int cam_idx)
{
    struct ct_dwindow dwindow;
    
    dwindow.data.wWindow_Top = 0; 
    dwindow.data.wWindow_Left = 0;  
    dwindow.data.wWindow_Bottom = 0; 
    dwindow.data.wWindow_Right = 0; 
    dwindow.data.wNumSteps = 0; 
    dwindow.data.bmNumStepsUnits = 0;     
    dwindow.req = DWINDOW_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_WINDOW, &dwindow, sizeof (struct ct_dwindow)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  default  wWindow_Top is  0x%x\n", dwindow.data.wWindow_Top);     
    info_msg("***  UVC  default  wWindow_Left is  0x%x\n", dwindow.data.wWindow_Left); 
    info_msg("***  UVC  default  wWindow_Bottom is  0x%x\n", dwindow.data.wWindow_Bottom);     
    info_msg("***  UVC  default  wWindow_Right is  0x%x\n", dwindow.data.wWindow_Right); 
    info_msg("***  UVC  default  wNumSteps is  0x%x\n", dwindow.data.wNumSteps);     
    info_msg("***  UVC  default  bmNumStepsUnits is  0x%x\n", dwindow.data.bmNumStepsUnits);     
    dwindow.data.wWindow_Top = 0; 
    dwindow.data.wWindow_Left = 0;  
    dwindow.data.wWindow_Bottom = 0; 
    dwindow.data.wWindow_Right = 0; 
    dwindow.data.wNumSteps = 0; 
    dwindow.data.bmNumStepsUnits = 0;      
    dwindow.req = DWINDOW_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_WINDOW, &dwindow, sizeof (struct ct_dwindow)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
			
    info_msg("***  UVC  min  wWindow_Top is  0x%x\n", dwindow.data.wWindow_Top);     
    info_msg("***  UVC  min  wWindow_Left is  0x%x\n", dwindow.data.wWindow_Left); 
    info_msg("***  UVC  min  wWindow_Bottom is  0x%x\n", dwindow.data.wWindow_Bottom);     
    info_msg("***  UVC  min  wWindow_Right is  0x%x\n", dwindow.data.wWindow_Right); 
    info_msg("***  UVC  min  wNumSteps is  0x%x\n", dwindow.data.wNumSteps);     
    info_msg("***  UVC  min  bmNumStepsUnits is  0x%x\n", dwindow.data.bmNumStepsUnits); 
    dwindow.req = DWINDOW_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_WINDOW, &dwindow, sizeof (struct ct_dwindow)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    
    info_msg("***  UVC  max  wWindow_Top is  0x%x\n", dwindow.data.wWindow_Top);     
    info_msg("***  UVC  max  wWindow_Left is  0x%x\n", dwindow.data.wWindow_Left); 
    info_msg("***  UVC  max  wWindow_Bottom is  0x%x\n", dwindow.data.wWindow_Bottom);     
    info_msg("***  UVC  max  wWindow_Right is  0x%x\n", dwindow.data.wWindow_Right); 
    info_msg("***  UVC  max  wNumSteps is  0x%x\n", dwindow.data.wNumSteps);     
    info_msg("***  UVC  max  bmNumStepsUnits is  0x%x\n", dwindow.data.bmNumStepsUnits);   
    dwindow.req = DWINDOW_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_WINDOW, &dwindow, sizeof (struct ct_dwindow)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  current  wWindow_Top is  0x%x\n", dwindow.data.wWindow_Top);     
    info_msg("***  UVC  current  wWindow_Left is  0x%x\n", dwindow.data.wWindow_Left); 
    info_msg("***  UVC  current  wWindow_Bottom is  0x%x\n", dwindow.data.wWindow_Bottom);     
    info_msg("***  UVC  current  wWindow_Right is  0x%x\n", dwindow.data.wWindow_Right); 
    info_msg("***  UVC  current  wNumSteps is  0x%x\n", dwindow.data.wNumSteps);     
    info_msg("***  UVC  current  bmNumStepsUnits is  0x%x\n", dwindow.data.bmNumStepsUnits);   
    dwindow.data.wWindow_Top = 1; 
    dwindow.data.wWindow_Left = 2;  
    dwindow.data.wWindow_Bottom = 3; 
    dwindow.data.wWindow_Right = 3; 
    dwindow.data.wNumSteps = 1; 
    dwindow.data.bmNumStepsUnits = 1;      
    dwindow.req = DWINDOW_SET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_WINDOW, &dwindow, sizeof (struct ct_dwindow)))
    {
        info_msg("***  UVC SET default windows is error \n");
        return;
    }
    dwindow.data.wWindow_Top = 0; 
    dwindow.data.wWindow_Left = 0;  
    dwindow.data.wWindow_Bottom = 0; 
    dwindow.data.wWindow_Right = 0; 
    dwindow.data.wNumSteps = 0; 
    dwindow.data.bmNumStepsUnits = 0;      
    dwindow.req = DWINDOW_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_WINDOW, &dwindow, sizeof (struct ct_dwindow)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  current  wWindow_Top is  0x%x\n", dwindow.data.wWindow_Top);     
    info_msg("***  UVC  current  wWindow_Left is  0x%x\n", dwindow.data.wWindow_Left); 
    info_msg("***  UVC  current  wWindow_Bottom is  0x%x\n", dwindow.data.wWindow_Bottom);     
    info_msg("***  UVC  current  wWindow_Right is  0x%x\n", dwindow.data.wWindow_Right); 
    info_msg("***  UVC  current  wNumSteps is  0x%x\n", dwindow.data.wNumSteps);     
    info_msg("***  UVC  current  bmNumStepsUnits is  0x%x\n", dwindow.data.bmNumStepsUnits);      
    info_msg("***  UVC test  pass \n");		
}

static void uvc_digital_roi_test(int cam_idx)
{
    struct ct_roi roi;
    
    roi.req = ROI_GET_DEF;
    roi.data.wROI_Top = 0; 
    roi.data.wROI_Left = 0; 
    roi.data.wROI_Bottom = 0; 
    roi.data.wROI_Right = 0; 
    roi.data.bmAutoControls = 0; 
 
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_REGION_OF_INTEREST, &roi, sizeof (struct ct_roi)))
    {
        info_msg("***  UVC roll is not supported \n");
        return;
    }
    info_msg("***  UVC  default  wROI_Top is  0x%x\n", roi.data.wROI_Top);     
    info_msg("***  UVC  default  wROI_Left is  0x%x\n", roi.data.wROI_Left); 
    info_msg("***  UVC  default  wROI_Bottom is  0x%x\n", roi.data.wROI_Bottom);     
    info_msg("***  UVC  default  wROI_Right is  0x%x\n", roi.data.wROI_Right); 
    info_msg("***  UVC  default  bmAutoControls is  0x%x\n", roi.data.bmAutoControls);     
       
    roi.req = ROI_GET_MIN;
    roi.data.wROI_Top = 0; 
    roi.data.wROI_Left = 0; 
    roi.data.wROI_Bottom = 0; 
    roi.data.wROI_Right = 0; 
    roi.data.bmAutoControls = 0; 
 
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_REGION_OF_INTEREST, &roi, sizeof (struct ct_roi)))
    {
        info_msg("***  UVC roll is not supported \n");
        return;
    }
    info_msg("***  UVC  min  wROI_Top is  0x%x\n", roi.data.wROI_Top);     
    info_msg("***  UVC  min  wROI_Left is  0x%x\n", roi.data.wROI_Left); 
    info_msg("***  UVC  min  wROI_Bottom is  0x%x\n", roi.data.wROI_Bottom);     
    info_msg("***  UVC  min  wROI_Right is  0x%x\n", roi.data.wROI_Right); 
    info_msg("***  UVC  min  bmAutoControls is  0x%x\n", roi.data.bmAutoControls); 
    
    roi.req = ROI_GET_MAX;
    roi.data.wROI_Top = 0; 
    roi.data.wROI_Left = 0; 
    roi.data.wROI_Bottom = 0; 
    roi.data.wROI_Right = 0; 
    roi.data.bmAutoControls = 0; 
 
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_REGION_OF_INTEREST, &roi, sizeof (struct ct_roi)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  max  wROI_Top is  0x%x\n", roi.data.wROI_Top);     
    info_msg("***  UVC  max  wROI_Left is  0x%x\n", roi.data.wROI_Left); 
    info_msg("***  UVC  max  wROI_Bottom is  0x%x\n", roi.data.wROI_Bottom);     
    info_msg("***  UVC  max  wROI_Right is  0x%x\n", roi.data.wROI_Right); 
    info_msg("***  UVC  max  bmAutoControls is  0x%x\n", roi.data.bmAutoControls);   
    
    roi.req = ROI_GET_CUR;
    roi.data.wROI_Top = 0; 
    roi.data.wROI_Left = 0; 
    roi.data.wROI_Bottom = 0; 
    roi.data.wROI_Right = 0; 
    roi.data.bmAutoControls = 0; 
 
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_REGION_OF_INTEREST, &roi, sizeof (struct ct_roi)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }
    info_msg("***  UVC  current  wROI_Top is  0x%x\n", roi.data.wROI_Top);     
    info_msg("***  UVC  current  wROI_Left is  0x%x\n", roi.data.wROI_Left); 
    info_msg("***  UVC  current  wROI_Bottom is  0x%x\n", roi.data.wROI_Bottom);     
    info_msg("***  UVC  current  wROI_Right is  0x%x\n", roi.data.wROI_Right); 
    info_msg("***  UVC  current  bmAutoControls is  0x%x\n", roi.data.bmAutoControls);       
    roi.req = ROI_SET_CUR;
    roi.data.wROI_Top = 2; 
    roi.data.wROI_Left = 2; 
    roi.data.wROI_Bottom = 2; 
    roi.data.wROI_Right = 2; 
    roi.data.bmAutoControls = 4; 
 
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_REGION_OF_INTEREST, &roi, sizeof (struct ct_roi)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }    

    roi.req = ROI_GET_CUR;
    roi.data.wROI_Top = 0; 
    roi.data.wROI_Left = 0; 
    roi.data.wROI_Bottom = 0; 
    roi.data.wROI_Right = 0; 
    roi.data.bmAutoControls = 0; 
 
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_REGION_OF_INTEREST, &roi, sizeof (struct ct_roi)))
    {
        info_msg("***  UVC GET default roll is error \n");
        return;
    }    
    info_msg("***  UVC  current  wROI_Top is  0x%x\n", roi.data.wROI_Top);     
    info_msg("***  UVC  current  wROI_Left is  0x%x\n", roi.data.wROI_Left); 
    info_msg("***  UVC  current  wROI_Bottom is  0x%x\n", roi.data.wROI_Bottom);     
    info_msg("***  UVC  current  wROI_Right is  0x%x\n", roi.data.wROI_Right); 
    info_msg("***  UVC  current  bmAutoControls is  0x%x\n", roi.data.bmAutoControls);        
    info_msg("***  UVC test  pass \n");		
}


static void uvc_backlight_compensation_test(int cam_idx)
{
    struct pu_backlight backlight;
    backlight.req = BKC_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC backlight is not supported \n");
        return;
    }
    info_msg("***  UVC  backlight cap is  0x%x\n", backlight.caps);
    
    backlight.wBacklightCompensation = 0; 
    backlight.req = BKC_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC GET default wBacklightCompensation is error \n");
        return;
    }
    info_msg("***  UVC  default  wBacklightCompensation is  0x%x\n", backlight.wBacklightCompensation);     		 
    backlight.wBacklightCompensation = 0; 
    backlight.req = BKC_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC GET min wBacklightCompensation is error \n");
        return;
    }
    info_msg("***  UVC  min  wBacklightCompensation is  0x%x\n", backlight.wBacklightCompensation);  			
    backlight.wBacklightCompensation = 0; 
    backlight.req = BKC_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC GET max wBacklightCompensation is error \n");
        return;
    }
    info_msg("***  UVC  max  wBacklightCompensation is  0x%x\n", backlight.wBacklightCompensation);      
    backlight.wBacklightCompensation = 0;     
    backlight.req = BKC_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC GET resolution wBacklightCompensation is error \n");
        return;
    }
    info_msg("***  UVC  resulation   is  0x%x\n", backlight.wBacklightCompensation);  
    backlight.wBacklightCompensation = 0;     
    backlight.req = BKC_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC GET current wBacklightCompensation is error \n");
        return;
    }
    info_msg("***  UVC  current  wBacklightCompensation   is  0x%x\n", backlight.wBacklightCompensation);      
    backlight.wBacklightCompensation = 1;     
    backlight.req = BKC_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC SET wBacklightCompensation is error \n");
        return;
    }
    
    backlight.wBacklightCompensation = 0;     
    backlight.req = BKC_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BACKLIGHT_COMPENSATION, &backlight, sizeof (struct pu_backlight)))
    {
        info_msg("***  UVC GET current wBacklightCompensation is error \n");
        return;
    }
    info_msg("***  UVC  current  wBacklightCompensation   is  0x%x\n", backlight.wBacklightCompensation);    
    info_msg("***  UVC test  pass \n");		
}

static void uvc_brightness_test(int cam_idx)
{
    struct pu_brightness brightness;
    brightness.req = BRIGHTNESS_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC brightness is not supported \n");
        return;
    }
    info_msg("***  UVC  brightness cap is  0x%x\n", brightness.caps);
    
    brightness.wBrightness = 0; 
    brightness.req = BRIGHTNESS_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC GET default brightness is error \n");
        return;
    }
    info_msg("***  UVC  default brightness is  0x%x\n", brightness.wBrightness);		 
    brightness.wBrightness = 0; 
    brightness.req = BRIGHTNESS_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC GET min brightness is error \n");
        return;
    }
    info_msg("***  UVC  min brightness is  0x%x\n", brightness.wBrightness);				
    brightness.wBrightness = 0; 
    brightness.req = BRIGHTNESS_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC GET max brightness is error \n");
        return;
    }
    info_msg("***  UVC  max brightness is  0x%x\n", brightness.wBrightness);	    
    brightness.wBrightness = 0;     
    brightness.req = BRIGHTNESS_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC GET resolution brightness is error \n");
        return;
    }
    info_msg("***  UVC  resolution brightness is  0x%x\n", brightness.wBrightness);	
    brightness.wBrightness = 0;     
    brightness.req = BRIGHTNESS_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC GET current brightness is error \n");
        return;
    }
    info_msg("***  UVC  current brightness is  0x%x\n", brightness.wBrightness);	    
    brightness.wBrightness = 1;     
    brightness.req = BRIGHTNESS_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC SET current brightness is error \n");
        return;
    }
    
    brightness.wBrightness = 0;     
    brightness.req = BRIGHTNESS_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_BRIGHTNESS, &brightness, sizeof (struct pu_brightness)))
    {
        info_msg("***  UVC GET current brightness is error \n");
        return;
    }
    info_msg("***  UVC  current brightness is  0x%x\n", brightness.wBrightness);	    
    info_msg("***  UVC test  pass \n");		
}


static void uvc_contrast_test(int cam_idx)
{
    struct pu_contrast contrast;
    contrast.req = CONTRAST_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC contrast is not supported \n");
        return;
    }
    info_msg("***  UVC  contrast cap is  0x%x\n", contrast.caps);
    
    contrast.wContrast = 0; 
    contrast.req = CONTRAST_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC GET default contrast is error \n");
        return;
    }
    info_msg("***  UVC  default contrast is  0x%x\n", contrast.wContrast);		 
    contrast.wContrast = 0; 
    contrast.req = CONTRAST_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC min contrast roll is error \n");
        return;
    }
    info_msg("***  UVC  min contrast is  0x%x\n", contrast.wContrast);			
    contrast.wContrast = 0; 
    contrast.req = CONTRAST_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC GET max contrast is error \n");
        return;
    }
    info_msg("***  UVC  max contrast is  0x%x\n", contrast.wContrast);    
    contrast.wContrast = 0;     
    contrast.req = CONTRAST_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC GET resolution  is error \n");
        return;
    }
    info_msg("***  UVC  resolution contrast is  0x%x\n", contrast.wContrast);
    contrast.wContrast = 0;     
    contrast.req = CONTRAST_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC GET current contrast error \n");
        return;
    }
    
    contrast.wContrast = 1;     
    contrast.req = CONTRAST_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC SET default contrast. is error \n");
        return;
    }
    
    contrast.wContrast = 0;     
    contrast.req = CONTRAST_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST, &contrast, sizeof (struct pu_contrast)))
    {
        info_msg("***  UVC GET default contrast. is error \n");
        return;
    }
    info_msg("***  UVC  current contrast is  0x%x\n", contrast.wContrast);   
    info_msg("***  UVC test  pass \n");		

}

static void uvc_auto_contrast_test(int cam_idx)
{
    struct pu_contrast_auto contrast;
    contrast.req = CONTRASTA_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST_AUTO, &contrast, sizeof (struct pu_contrast_auto)))
    {
        info_msg("***  UVC auto contrast is not supported \n");
        return;
    }
    info_msg("***  UVC  auto contrast cap is  0x%x\n", contrast.caps);
    
    contrast.bContrastAuto = 0; 
    contrast.req = CONTRASTA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST_AUTO, &contrast, sizeof (struct pu_contrast_auto)))
    {
        info_msg("***  UVC GET default auto contrast is error \n");
        return;
    }
    info_msg("***  UVC  default auto contrast is  0x%x\n", contrast.bContrastAuto);
    contrast.bContrastAuto = 0;     
    contrast.req = CONTRASTA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST_AUTO, &contrast, sizeof (struct pu_contrast_auto)))
    {
        info_msg("***  UVC GET default auto contrast is error \n");
        return;
    }
    info_msg("***  UVC  current auto contrast is  0x%x\n", contrast.bContrastAuto);   
    contrast.bContrastAuto = 1;     
    contrast.req = CONTRASTA_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST_AUTO, &contrast, sizeof (struct pu_contrast_auto)))
    {
        info_msg("***  UVC GET default auto contrast is error \n");
        return;
    }
    
    contrast.bContrastAuto = 0;     
    contrast.req = CONTRASTA_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_CONTRAST_AUTO, &contrast, sizeof (struct pu_contrast_auto)))
    {
        info_msg("***  UVC GET default auto contrast is error \n");
        return;
    }
    info_msg("***  UVC  current auto contrast is  0x%x\n", contrast.bContrastAuto);     
    info_msg("***  UVC test  pass \n");		
	
}

static void uvc_gain_test(int cam_idx)
{
    struct pu_gain gain;
    gain.req = GAIN_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC gain is not supported \n");
        return;
    }
    info_msg("***  UVC  gain cap is  0x%x\n", gain.caps);
    
    gain.wGain = 0; 
    gain.req = GAIN_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC GET default gain is error \n");
        return;
    }
    info_msg("***  UVC  default gain  is  0x%x\n", gain.wGain);		 
    gain.wGain = 0; 
    gain.req = GAIN_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC GET min gain is error \n");
        return;
    }
    info_msg("***  UVC  min gain  is  0x%x\n", gain.wGain);				
    gain.wGain = 0; 
    gain.req = GAIN_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC GET max gain is error \n");
        return;
    }
    info_msg("***  UVC  max gain  is  0x%x\n", gain.wGain);	    
    gain.wGain = 0;     
    gain.req = GAIN_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC GET resolution gain is error \n");
        return;
    }
    info_msg("***  UVC  resolution gain  is  0x%x\n", gain.wGain);	 
    gain.wGain = 0;     
    gain.req = GAIN_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC GET current gain is error \n");
        return;
    }
    info_msg("***  UVC GET current gain is  0x%x\n", gain.wGain);    
    gain.wGain = 1;     
    gain.req = GAIN_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC SET current gain is error \n");
        return;
    }
    
    gain.wGain = 0;     
    gain.req = GAIN_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAIN, &gain, sizeof (struct pu_gain)))
    {
        info_msg("***  UVC GET current gain is error \n");
        return;
    }
    info_msg("***  UVC GET current gain is 0x%x\n", gain.wGain);      
    info_msg("***  UVC test  pass \n");		
}

static void uvc_power_line_frequency_test(int cam_idx)
{
    struct pu_power_line_frequency frequency;
    frequency.req = POWER_LINE_FREQUENCY_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_POWER_LINE_FREQUENCY, &frequency, sizeof (struct pu_power_line_frequency)))
    {
        info_msg("***  UVC power line frequency is not supported \n");
        return;
    }
    info_msg("***  UVC  power line frequency cap is  0x%x\n", frequency.caps);
    
    frequency.bPowerLineFrequency = 0; 
    frequency.req = POWER_LINE_FREQUENCY_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_POWER_LINE_FREQUENCY, &frequency, sizeof (struct pu_power_line_frequency)))
    {
        info_msg("***  UVC GET default power line frequency is error \n");
        return;
    }
    info_msg("***  UVC  default power line frequency  is  0x%x\n", frequency.bPowerLineFrequency);
    frequency.bPowerLineFrequency = 0;     
    frequency.req = POWER_LINE_FREQUENCY_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_POWER_LINE_FREQUENCY, &frequency, sizeof (struct pu_power_line_frequency)))
    {
        info_msg("***  UVC GET current power line frequency is error \n");
        return;
    }
    info_msg("***  UVC  current power line frequency  is  0x%x\n", frequency.bPowerLineFrequency);   
    frequency.bPowerLineFrequency = PLF_AUTO;     
    frequency.req = POWER_LINE_FREQUENCY_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_POWER_LINE_FREQUENCY, &frequency, sizeof (struct pu_power_line_frequency)))
    {
        info_msg("***  UVC SET default power line frequency is error \n");
        return;
    }
    
    frequency.bPowerLineFrequency = 0;     
    frequency.req = POWER_LINE_FREQUENCY_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_POWER_LINE_FREQUENCY, &frequency, sizeof (struct pu_power_line_frequency)))
    {
        info_msg("***  UVC GET current power line frequency is error \n");
        return;
    }
    info_msg("***  UVC  current power line frequency  is  0x%x\n", frequency.bPowerLineFrequency);    
    info_msg("***  UVC test  pass \n");		
}


static void uvc_saturation_test(int cam_idx)
{
    struct pu_saturation saturation;
    saturation.req = SATURATION_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC saturation is not supported \n");
        return;
    }
    info_msg("***  UVC  saturation cap is  0x%x\n", saturation.caps);
    
    saturation.wSaturation = 0; 
    saturation.req = SATURATION_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC GET default saturation is error \n");
        return;
    }
    info_msg("***  UVC GET current saturation is  0x%x\n", saturation.wSaturation);    		 
    saturation.wSaturation = 0; 
    saturation.req = SATURATION_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC GET min saturation is error \n");
        return;
    }
    info_msg("***  UVC GET current saturation is  0x%x\n", saturation.wSaturation);    			
    saturation.wSaturation = 0; 
    saturation.req = SATURATION_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC GET max saturation is error \n");
        return;
    }
    info_msg("***  UVC GET current saturation is  0x%x\n", saturation.wSaturation);        
    saturation.wSaturation = 0;     
    saturation.req = SATURATION_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC GET resolution saturation is error \n");
        return;
    }
    info_msg("***  UVC GET resolution saturation is  0x%x\n", saturation.wSaturation);      
    saturation.wSaturation = 0;     
    saturation.req = SATURATION_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC GET current saturation is error \n");
        return;
    }
    info_msg("***  UVC GET current saturation is  0x%x\n", saturation.wSaturation);        
    saturation.wSaturation = 1;     
    saturation.req = SATURATION_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC SET current saturation is error \n");
        return;
    }
    
    saturation.wSaturation = 0;     
    saturation.req = SATURATION_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SATURATION, &saturation, sizeof (struct pu_saturation)))
    {
        info_msg("***  UVC GET current saturation is error \n");
        return;
    }
    info_msg("***  UVC GET current saturation is  0x%x\n", saturation.wSaturation);        
    info_msg("***  UVC test  pass \n");		
	
}

static void uvc_sharpness_test(int cam_idx)
{
    struct pu_sharpness sharpness;
    sharpness.req = SHARPNESS_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC sharpness is not supported \n");
        return;
    }
    info_msg("***  UVC  sharpness cap is  0x%x\n", sharpness.caps);
    
    sharpness.wSharpness = 0; 
    sharpness.req = SHARPNESS_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC GET default sharpness is error \n");
        return;
    }
    info_msg("***  UVC GET default sharpness is  0x%x\n", sharpness.wSharpness);    		 
    sharpness.wSharpness = 0; 
    sharpness.req = SHARPNESS_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC GET min sharpness is error \n");
        return;
    }
    info_msg("***  UVC GET min sharpness is  0x%x\n", sharpness.wSharpness);    				
    sharpness.wSharpness = 0; 
    sharpness.req = SHARPNESS_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC GET max sharpness is error \n");
        return;
    }
    info_msg("***  UVC GET max sharpness is  0x%x\n", sharpness.wSharpness);    	    
    sharpness.wSharpness = 0;     
    sharpness.req = SHARPNESS_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC GET resolution sharpness is error \n");
        return;
    }
    info_msg("***  UVC  resolution is  0x%x\n", sharpness.wSharpness);    	
    sharpness.wSharpness = 0;     
    sharpness.req = SHARPNESS_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC GET current sharpness is error \n");
        return;
    }
    info_msg("***  UVC GET current sharpness is  0x%x\n", sharpness.wSharpness);    	    
    sharpness.wSharpness = 1;     
    sharpness.req = SHARPNESS_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC GET current sharpness is error \n");
        return;
    }
    
    sharpness.wSharpness = 0;     
    sharpness.req = SHARPNESS_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_SHARPNESS, &sharpness, sizeof (struct pu_sharpness)))
    {
        info_msg("***  UVC GET current sharpness is error \n");
        return;
    }
    info_msg("***  UVC GET current sharpness is  0x%x\n", sharpness.wSharpness);    	    
    info_msg("***  UVC test  pass \n");		
}

static void uvc_gamma_test(int cam_idx)
{
    struct pu_gamma gamma;
    gamma.req = GAMMA_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC gamma is not supported \n");
        return;
    }
    info_msg("***  UVC  gamma cap is  0x%x\n", gamma.caps);
    
    gamma.wGamma = 0; 
    gamma.req = GAMMA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC GET default gamma is error \n");
        return;
    }
    info_msg("***  UVC  default gamma  is  0x%x\n", gamma.wGamma);		 
    gamma.wGamma = 0; 
    gamma.req = GAMMA_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC GET default gamma is error \n");
        return;
    }
    info_msg("***  UVC  min gamma  is  0x%x\n", gamma.wGamma);				
    gamma.wGamma = 0; 
    gamma.req = GAMMA_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC GET default gamma is error \n");
        return;
    }
    info_msg("***  UVC  max gamma  is  0x%x\n", gamma.wGamma);	    
    gamma.wGamma = 0;     
    gamma.req = GAMMA_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC  resolution is error \n");
        return;
    }
    info_msg("***  UVC  resolution   is  0x%x\n", gamma.wGamma);	
    gamma.wGamma = 0;     
    gamma.req = GAMMA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC GET current gamma is error \n");
        return;
    }
    info_msg("***  UVC  current gamma  is  0x%x\n", gamma.wGamma);	    
    gamma.wGamma = 1;     
    gamma.req = GAMMA_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC SET current gamma is error \n");
        return;
    }
    
    gamma.wGamma = 0;     
    gamma.req = GAMMA_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_GAMMA, &gamma, sizeof (struct pu_gamma)))
    {
        info_msg("***  UVC GET current gamma is error \n");
        return;
    }
    info_msg("***  UVC  current gamma  is  0x%x\n", gamma.wGamma);	    
    info_msg("***  UVC test  pass \n");		
}	


static void uvc_white_balance_temperature_test(int cam_idx)
{
    struct pu_white_balance_temp wbt;
    wbt.req = WBT_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC white balance temperature is not supported \n");
        return;
    }
    info_msg("***  UVC  white balance temperature cap is  0x%x\n", wbt.caps);
    
    wbt.wWhiteBalanceTemperature = 0; 
    wbt.req = WBT_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC GET default white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  default white balance temperature is  0x%x\n", wbt.wWhiteBalanceTemperature);		 
    wbt.wWhiteBalanceTemperature = 0; 
    wbt.req = WBT_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC GET min white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  min white balance temperature is  0x%x\n", wbt.wWhiteBalanceTemperature);			
    wbt.wWhiteBalanceTemperature = 0; 
    wbt.req = WBT_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC GET max white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  max white balance temperature is  0x%x\n", wbt.wWhiteBalanceTemperature);    
    wbt.wWhiteBalanceTemperature = 0;     
    wbt.req = WBT_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC GET resolution white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  resolution  is  0x%x\n", wbt.wWhiteBalanceTemperature);
    wbt.wWhiteBalanceTemperature = 0;     
    wbt.req = WBT_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC GET current white_balance_temperature is error \n");
        return;
    }
    info_msg("***  UVC  current white balance temperature is  0x%x\n", wbt.wWhiteBalanceTemperature);    
    wbt.wWhiteBalanceTemperature = 0x7d0;     
    wbt.req = WBT_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC SET current white_balance_temperature is error \n");
        return;
    }
    
    wbt.wWhiteBalanceTemperature = 0;     
    wbt.req = WBT_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE, &wbt, sizeof (struct pu_white_balance_temp)))
    {
        info_msg("***  UVC GET current  white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  current white balance temperature is  0x%x\n", wbt.wWhiteBalanceTemperature);    
    info_msg("***  UVC test  pass \n");		
}

static void uvc_white_balance_compont_test(int cam_idx)
{
    struct pu_whitebalance_comp wbc;
    wbc.req = WBC_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC white balance compont is not supported \n");
        return;
    }
    info_msg("***  UVC  white balance compont cap is  0x%x\n", wbc.caps);
    
    wbc.data.wWhiteBalanceBlue = 0; 
    wbc.data.wWhiteBalanceRed = 0;    
    wbc.req = WBC_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC GET default white balance compont is error \n");
        return;
    }
    info_msg("***  UVC  default white balance compont blue is  0x%x\n", wbc.data.wWhiteBalanceBlue);
    info_msg("***  UVC  default white balance compont red is  0x%x\n", wbc.data.wWhiteBalanceRed);    
    wbc.data.wWhiteBalanceBlue = 0; 
    wbc.data.wWhiteBalanceRed = 0; 
    wbc.req = WBC_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC GET min white balance compont is error \n");
        return;
    }
    info_msg("***  UVC  min white balance compont blue is  0x%x\n", wbc.data.wWhiteBalanceBlue);
    info_msg("***  UVC  min white balance compont red is  0x%x\n", wbc.data.wWhiteBalanceRed);    			
    wbc.data.wWhiteBalanceBlue = 0; 
    wbc.data.wWhiteBalanceRed = 0; 
    wbc.req = WBC_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC GET max white balance compont is error \n");
        return;
    }
    info_msg("***  UVC  max white balance compont blue is  0x%x\n", wbc.data.wWhiteBalanceBlue);
    info_msg("***  UVC  max white balance compont red is  0x%x\n", wbc.data.wWhiteBalanceRed);        
    wbc.data.wWhiteBalanceBlue = 0; 
    wbc.data.wWhiteBalanceRed = 0;   
    wbc.req = WBC_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC GET resolution is error \n");
        return;
    }

    wbc.data.wWhiteBalanceBlue = 0; 
    wbc.data.wWhiteBalanceRed = 0;   
    wbc.req = WBC_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC GET current  white balance compont is error \n");
        return;
    }
    info_msg("***  UVC  current white balance compont blue is  0x%x\n", wbc.data.wWhiteBalanceBlue);
    info_msg("***  UVC  current white balance compont red is  0x%x\n", wbc.data.wWhiteBalanceRed);        
    wbc.data.wWhiteBalanceBlue = 1; 
    wbc.data.wWhiteBalanceRed = 1;  
    wbc.req = WBC_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC SET current white balance compont is error \n");
        return;
    }
    
    wbc.data.wWhiteBalanceBlue = 0; 
    wbc.data.wWhiteBalanceRed = 0;  
    wbc.req = WBC_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT, &wbc, sizeof (struct pu_whitebalance_comp)))
    {
        info_msg("***  UVC GET current white balance compont is error \n");
        return;
    }
    info_msg("***  UVC  current white balance compont blue is  0x%x\n", wbc.data.wWhiteBalanceBlue);
    info_msg("***  UVC  current white balance compont red is  0x%x\n", wbc.data.wWhiteBalanceRed);     
    info_msg("***  UVC test  pass \n");		
}

static void uvc_auto_white_balance_temperature_test(int cam_idx)
{
    struct pu_white_balance_temp_auto wbt_auto;
    wbt_auto.req = WBTA_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE_AUTO, &wbt_auto, sizeof (struct pu_white_balance_temp_auto)))
    {
        info_msg("***  UVC auto white balance temperature is not supported \n");
        return;
    }
    info_msg("***  UVC  auto white balance temperature cap is  0x%x\n", wbt_auto.caps);
    
    wbt_auto.bWhiteBalanceTemperatureAuto = 0; 
    wbt_auto.req = WBTA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE_AUTO, &wbt_auto, sizeof (struct pu_white_balance_temp_auto)))
    {
        info_msg("***  UVC GET default auto white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  default auto white balance temperature cap is  0x%x\n", wbt_auto.bWhiteBalanceTemperatureAuto);
    wbt_auto.bWhiteBalanceTemperatureAuto = 0;     
    wbt_auto.req = WBTA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE_AUTO, &wbt_auto, sizeof (struct pu_white_balance_temp_auto)))
    {
        info_msg("***  UVC GET current auto white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  current auto white balance temperature cap is  0x%x\n", wbt_auto.bWhiteBalanceTemperatureAuto);    
    wbt_auto.bWhiteBalanceTemperatureAuto = 0;     
    wbt_auto.req = WBTA_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE_AUTO, &wbt_auto, sizeof (struct pu_white_balance_temp_auto)))
    {
        info_msg("***  UVC SET current auto white balance temperature error \n");
        return;
    }
    
    wbt_auto.bWhiteBalanceTemperatureAuto = 0;     
    wbt_auto.req = WBTA_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_TEMPERATURE_AUTO, &wbt_auto, sizeof (struct pu_white_balance_temp_auto)))
    {
        info_msg("***  UVC GET current auto white balance temperature is error \n");
        return;
    }
    info_msg("***  UVC  current auto white balance temperature is  0x%x\n", wbt_auto.bWhiteBalanceTemperatureAuto);    
    info_msg("***  UVC test  pass \n");		
	
}

static void uvc_auto_white_balance_compont_test(int cam_idx)
{
    struct pu_wbc_auto wbc;
    wbc.req = WBCA_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT_AUTO, &wbc, sizeof (struct pu_wbc_auto)))
    {
        info_msg("***  UVC auto white balance compont is not supported \n");
        return;
    }
    info_msg("***  UVC  auto white balance_compont cap is  0x%x\n", wbc.caps);
    
    wbc.bWhiteBalanceComponentAuto = 0; 
    wbc.req = WBCA_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT_AUTO, &wbc, sizeof (struct pu_wbc_auto)))
    {
        info_msg("***  UVC GET default auto white balance compont is error \n");
        return;
    }
    info_msg("***  UVC default  auto white balance_compont  is  0x%x\n", wbc.bWhiteBalanceComponentAuto);
    wbc.bWhiteBalanceComponentAuto = 0;     
    wbc.req = WBCA_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT_AUTO, &wbc, sizeof (struct pu_wbc_auto)))
    {
        info_msg("***  UVC GET default current white balance compont is error \n");
        return;
    }
    info_msg("***  UVC current  auto white balance_compont is  0x%x\n", wbc.bWhiteBalanceComponentAuto);   
    wbc.bWhiteBalanceComponentAuto = 1;     
    wbc.req = WBCA_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT_AUTO, &wbc, sizeof (struct pu_wbc_auto)))
    {
        info_msg("***  UVC SET default auto white balance compont is error \n");
        return;
    }
    
    wbc.bWhiteBalanceComponentAuto = 0;     
    wbc.req = WBCA_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_WHITE_BALANCE_COMPONENT_AUTO, &wbc, sizeof (struct pu_wbc_auto)))
    {
        info_msg("***  UVC GET current auto white balance compont is error \n");
        return;
    }
    info_msg("***  UVC current  auto white balance_compont  is  0x%x\n", wbc.bWhiteBalanceComponentAuto);   
    info_msg("***  UVC test  pass \n");		
	
}

static void uvc_set_digital_multiplier(int cam_idx)
{
    struct pu_dmultiplier dmlpr;
    dmlpr.req = MPL_GET_CAP;

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC digital multiplier is not supported \n");
        return;
    }
    info_msg("***  UVC  digital multiplier cap is  0x%x\n", dmlpr.caps);
    
    dmlpr.wMultiplierStep = 0;    
    dmlpr.req = MPL_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC GET default digital multiplier is error \n");
        return;
    }
    info_msg("***  UVC  default digital multiplier is  0x%x\n", dmlpr.wMultiplierStep);		 
    dmlpr.wMultiplierStep = 0;  
    dmlpr.req = MPL_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC GET min digital multiplier is error \n");
        return;
    }
    info_msg("***  UVC  min digital multiplier is  0x%x\n", dmlpr.wMultiplierStep);				
    dmlpr.wMultiplierStep = 0;  
    dmlpr.req = MPL_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC GET max digital multiplier is error \n");
        return;
    }
    info_msg("***  UVC  max digital multiplier is  0x%x\n", dmlpr.wMultiplierStep);	    
    dmlpr.wMultiplierStep = 0;  
    dmlpr.req = MPL_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC GET resolution is error \n");
        return;
    }
    info_msg("***  UVC  resolution is  0x%x\n", dmlpr.wMultiplierStep);	
    dmlpr.wMultiplierStep = 0;   
    dmlpr.req = MPL_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC GET current digital multiplier is error \n");
        return;
    }
    info_msg("***  UVC  current digital multiplier is  0x%x\n", dmlpr.wMultiplierStep);	    
    dmlpr.wMultiplierStep = 0;  
    dmlpr.req = MPL_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC SET current digital multiplier is error \n");
        return;
    }
    
    dmlpr.wMultiplierStep = 0;  
    dmlpr.req = MPL_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER, &dmlpr, sizeof (struct pu_dmultiplier)))
    {
        info_msg("***  UVC GET current digital multiplier is error \n");
        return;
    }
    info_msg("***  UVC  current digital multiplier is  0x%x\n", dmlpr.wMultiplierStep);	    
    info_msg("***  UVC test  pass \n");		
}

static void uvc_digital_multiplier_limit_test(int cam_idx)
{
    struct pu_dmultiplierlimit dmlpl;
    dmlpl.req = DMPL_GET_CAP;
    dmlpl.caps = 0;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC digital multiplier limit is not supported \n");
        return;
    }
    info_msg("***  UVC  digital multiplier limit cap is  0x%x\n", dmlpl.caps);
    
    dmlpl.wMultiplierLimit = 0;    
    dmlpl.req = DMPL_GET_DEF;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC GET default digital multiplier limit is error \n");
        return;
    }
    info_msg("***  UVC  default digital multiplier limit cap is  0x%x\n", dmlpl.wMultiplierLimit);		 
    dmlpl.wMultiplierLimit = 0;  
    dmlpl.req = DMPL_GET_MIN;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC GET min digital multiplier limit is error \n");
        return;
    }
    info_msg("***  UVC  min digital multiplier limit cap is  0x%x\n", dmlpl.wMultiplierLimit);				
    dmlpl.wMultiplierLimit = 0;  
    dmlpl.req = DMPL_GET_MAX;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC GET max digital multiplier limit is error \n");
        return;
    }
     info_msg("***  UVC  max digital multiplier limit cap is  0x%x\n", dmlpl.wMultiplierLimit);	   
    dmlpl.wMultiplierLimit = 0;  
    dmlpl.req = DMPL_GET_RES;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC GET resolution is error \n");
        return;
    }
    info_msg("***  UVC  resolution is  0x%x\n", dmlpl.wMultiplierLimit);	
    dmlpl.wMultiplierLimit = 0;   
    dmlpl.req = DMPL_GET_CUR;
    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC GET current  digital multiplier limit is error \n");
        return;
    }
    info_msg("***  UVC  current digital multiplier limit cap is  0x%x\n", dmlpl.wMultiplierLimit);	   
    dmlpl.wMultiplierLimit = 0;  
    dmlpl.req = DMPL_SET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC SET current digital multiplier limit is error \n");
        return;
    }
    
    dmlpl.wMultiplierLimit = 0;  
    dmlpl.req = DMPL_GET_CUR; 

    if (KMDW_STATUS_ERROR == kmdw_camera_ioctl(cam_idx, CID_DIGITAL_MULTIPLIER_LIMIT, &dmlpl, sizeof (struct pu_dmultiplierlimit)))
    {
        info_msg("***  UVC GET current  digital multiplier limit is error \n");
        return;
    }
    info_msg("***  UVC  current digital multiplier limit cap is  0x%x\n", dmlpl.wMultiplierLimit);	    
    info_msg("***  UVC test  pass \n");		
}

int sample_uvc_ctl_test(void)
{
    int ret, cam_idx = 0;
    
    if (0 != (ret = kmdw_camera_open(cam_idx)))
        return ret;
    ret = uvc_camera_cotrol_list(cam_idx);
    uvc_scanning_mode_test(cam_idx);

    uvc_exposure_mode_test(cam_idx);
    uvc_exposure_priority_test(cam_idx);	
    uvc_exposure_time_test(cam_idx);
     
    uvc_shutter_speed_test(cam_idx);
   
    uvc_focus_auto_test(cam_idx);    
    uvc_focus_absolute_test(cam_idx);
    uvc_focus_relative_test(cam_idx);
    uvc_focus_len_range_test(cam_idx);

     
    uvc_iris_absolute_test(cam_idx);
    uvc_iris_relative_test(cam_idx);
     
    uvc_zoom_absolute_test(cam_idx);
    uvc_zoom_relative_test(cam_idx);
     
    uvc_pan_tilt_absolute_test(cam_idx);
    uvc_pan_tilt_direction_test(cam_idx);
 
    uvc_roll_test(cam_idx);
    uvc_roll_direction_test(cam_idx);
    uvc_shutter_test(cam_idx);
    uvc_digital_windows_test(cam_idx);
    uvc_digital_roi_test(cam_idx);

    uvc_backlight_compensation_test(cam_idx);
    uvc_brightness_test(cam_idx);
    uvc_contrast_test(cam_idx);    
    uvc_auto_contrast_test(cam_idx);
    uvc_gain_test(cam_idx);
    uvc_power_line_frequency_test(cam_idx);
    uvc_saturation_test(cam_idx);
    uvc_sharpness_test(cam_idx);
    uvc_gamma_test(cam_idx);
    uvc_white_balance_temperature_test(cam_idx);
    uvc_white_balance_compont_test(cam_idx);
    uvc_auto_white_balance_temperature_test(cam_idx);
    uvc_auto_white_balance_compont_test(cam_idx);
    uvc_set_digital_multiplier(cam_idx);
    uvc_digital_multiplier_limit_test(cam_idx);
    
    return ret;
}	
#endif

