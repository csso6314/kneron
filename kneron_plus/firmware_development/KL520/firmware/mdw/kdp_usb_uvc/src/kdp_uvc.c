/*
 * KDP UVC driver 
 *
 * Copyright (C) 2019 - 2020 Kneron, Inc. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <stdlib.h>
#include "kneron_mozart.h"
#include "kdp_usb.h"
#include "uvc.h"
#include <uvc_video.h>
#include <uvc_ctrl.h>
#include "kmdw_memory.h"
#include "uvc_internal_api.h"
#include "kmdw_camera.h"
#include "kmdw_display.h"
#include "delay.h"
#include "uvc_camera.h"
#include "kmdw_usbh.h"
#include "kmdw_uvc.h"
#include "kmdw_console.h"
#define FLAGS_YOLO_STOP_EVT         BIT1

#define KDP_UVC_DRIVER_DESC		"KDP UVC driver"
#define KDP_UVC_DRIVER 1
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ROUND_UP(x, y) ((((x) + (y - 1)) / y) * y)

static struct uvc_device uvc_video_s;
static struct uvc_device *uvc_video_device = &uvc_video_s;
static USBH_PIPE_HANDLE isoch_pipe = 0;
osThreadId_t tid_to_notify;

#define NUM_FRAME 7
#define FRMAE_SIZE (640 * 480 * 2) // VGA YUV420

// FIXME: temp solution (ping pong buffer) to remove kdrv_fb_mgr, need to test if 520 uvc device is ready
uint32_t ping_pong_buf_addr[2] = {0};

void USBH_UVC_Get_Frame(uint32_t *frame_ptr, uint32_t *frame_size, int *index)
{
    struct uvc_device *udev = uvc_video_device;
    struct uvc_streaming *stream = udev->curr_stream;
#ifdef UVC_USER_ERR
    if (frame_size != FRMAE_SIZE)
        kmdw_printf("uvc_example: frame_ptr 0x%p frame_size %u is wrong\n", frame_ptr, frame_size);
#endif
     *frame_ptr = stream->frame_buf;
     *frame_size = udev->curr_stream->cur_format->bpp * udev->curr_stream->cur_format->cur_frame->wWidth * udev->curr_stream->cur_format->cur_frame->wHeight;
     *index = stream->write_idx;
}


struct uvc_device *video_dev(const char *pname)
{
    if (!memcmp(uvc_video_device->name, pname, sizeof (*pname))) {
         return uvc_video_device;
    }

    return NULL;
}

static int kdp_uvc_get_ctl_list(uint32_t *ctl_list)
{
    struct uvc_device *udev = uvc_video_device;  
    ctl_list[0] = udev->IT[0].ct->bmControls;
    ctl_list[1] = udev->PU[0].bmControls;
 
    return 0;
}

static int uvc_list_ctl(uint32_t *data)
{
    return kdp_uvc_get_ctl_list(data);
}

static int uvc_scanning_mode_ctl(struct uvc_device *udev, struct ct_scm *ctl)
{
    if (ctl->req == SCM_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_SCANNING_MODE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_SCANNING_MODE, ctl->req, (uint8_t *)&ctl->bScanningMode, SCANNING_MODE_LEN);
}

static int uvc_exposure_mode_ctl(struct uvc_device *udev, struct ct_aem *ctl)
{
    if (ctl->req == AEM_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_AUTO_EXPOSURE_MODE, ctl->req,  &ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_AUTO_EXPOSURE_MODE, ctl->req,  &ctl->bAutoExposureMode, AUTO_EXPOSURE_MODE_LEN);		
}

static int uvc_exposure_priority_ctl(struct uvc_device *udev, struct ct_aep *ctl)
{
    if (ctl->req == AEP_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_AUTO_EXPOSURE_PRIORITY, ctl->req,  &ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_AUTO_EXPOSURE_PRIORITY, ctl->req,  &ctl->bAutoExposurePriority, AUTO_EXPOSURE_PRIORITY_LEN);	
}

static int uvc_exposure_time_abs_ctl(struct uvc_device *udev, struct ct_eta *ctl)
{
    if (ctl->req == ETA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_EXPOSURE_TIME_ABSOLUTE, ctl->req,  &ctl->caps, 1); 
    else
        return uvc_send_ctl(udev, UVC_CID_EXPOSURE_TIME_ABSOLUTE, ctl->req, (uint8_t *)&ctl->bExposureTimeAbsolute, EXPOSURE_TIME_ABSOLUTE_LEN);
}

static int uvc_shutter_speed_ctl(struct uvc_device *udev, struct ct_etr *ctl)
{
    if (ctl->req == ETA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_EXPOSURE_TIME_RELATIVE, ctl->req,  &ctl->caps, 1); 
    else
        return uvc_send_ctl(udev, UVC_CID_EXPOSURE_TIME_RELATIVE, ctl->req, (uint8_t *)&ctl->bExposureTimeRelative, EXPOSURE_TIME_RELATIVE_LEN);
}

static int uvc_focus_auto_ctl(struct uvc_device *udev, struct ct_fauto *ctl)
{
    if (ctl->req == FAUTO_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_FOCUS_AUTO, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_FOCUS_AUTO, ctl->req, (uint8_t *)&ctl->bFocusAuto, FOCUS_AUTO_LEN);	
}

static int uvc_focus_abs_ctl(struct uvc_device *udev, struct ct_focus_a *ctl)
{
    if (ctl->req == FA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_FOCUS_ABSOLUTE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_FOCUS_ABSOLUTE, ctl->req, (uint8_t *)&ctl->wFocusAbsolute, FOCUS_ABSOLUTE_LEN);				
}

static int uvc_focus_rel_ctl(struct uvc_device *udev, struct ct_focus_r *ctl)
{
    if (ctl->req == FR_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_FOCUS_RELATIVE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_FOCUS_RELATIVE, ctl->req, (uint8_t *)&ctl->data, FOCUS_RELATIVE_LEN);				
}	

static int uvc_focus_simple_range_ctl(struct uvc_device *udev, struct ct_focus_sr *ctl)
{
    if (ctl->req == FSR_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_FOCUS_SIMPLE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_FOCUS_SIMPLE, ctl->req, (uint8_t *)&ctl->bFocus, FOCUS_SIMPLE_LEN);
}	

static int uvc_iris_abs_ctl(struct uvc_device *udev, struct ct_iris_a *ctl)
{
    if (ctl->req == IRISA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_IRIS_ABSOLUTE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_IRIS_ABSOLUTE, ctl->req, (uint8_t *)&ctl->wIrisAbsolute, IRIS_ABSOLUTE_LEN);			
}

static int uvc_iris_rel_ctl(struct uvc_device *udev, struct ct_iris_r *ctl)
{
    if (ctl->req == IRISR_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_IRIS_RELATIVE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_IRIS_RELATIVE, ctl->req, (uint8_t *)&ctl->bIrisRelative, IRIS_ABSOLUTE_LEN);			
}
	
static int uvc_zoom_abs_ctl(struct uvc_device *udev, struct ct_zoom_a *ctl)
{
    if (ctl->req == ZOOMA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_ZOOM_ABSOLUTE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_ZOOM_ABSOLUTE, ctl->req, (uint8_t *)&ctl->wObjectiveFocalLength, FOCUS_ABSOLUTE_LEN);			
}

static int uvc_zoom_rel_ctl(struct uvc_device *udev, struct ct_zoom_r *ctl)
{
    if (ctl->req == ZOOMR_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_ZOOM_RELATIVE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_ZOOM_RELATIVE, ctl->req, (uint8_t *)&ctl->data, FOCUS_RELATIVE_LEN);
}

static int uvc_hue_ctl(struct uvc_device *udev, struct pu_hue *ctl)
{
    if (ctl->req == HUE_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_HUE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_HUE, ctl->req, (uint8_t *)&ctl->wHue, HUE_LEN);
}    

static int uvc_hue_auto_ctl(struct uvc_device *udev, struct pu_hue_auto * ctl)
{
    if (ctl->req == HUEA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_HUE_AUTO, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_HUE_AUTO, ctl->req, (uint8_t *)&ctl->bHueAuto, HUE_AUTO_LEN);
}

static int uvc_pan_tilt_abs_ctl(struct uvc_device *udev, struct ct_pan_tilt_a *ctl)
{
    if (ctl->req == TILTA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_PANTILT_ABSOLUTE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else		
        return uvc_send_ctl(udev, UVC_CID_PANTILT_ABSOLUTE, ctl->req, (uint8_t *)&ctl->data, PANTILT_ABSOLUTE_LEN);
}		

static int uvc_pan_tilt_rel_ctl(struct uvc_device *udev, struct ct_pan_tilt_r *ctl)
{
    if (ctl->req == TILTR_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_PANTILT_RELATIVE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else			
        return uvc_send_ctl(udev, UVC_CID_PANTILT_RELATIVE, ctl->req, (uint8_t *)&ctl->data, PANTILT_RELATIVE_LEN);
}

static int uvc_roll_abs_ctl(struct uvc_device *udev, struct ct_roll_a *ctl)
{
    if (ctl->req == ROLLA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_ROLL_ABSOLUTE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_ROLL_ABSOLUTE, ctl->req, (uint8_t *)&ctl->wAbsolute, PANTILT_ABSOLUTE_LEN);
}	

static int uvc_roll_rel_ctl(struct uvc_device *udev, struct ct_roll_r *ctl)
{
    if (ctl->req == ROLLR_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_ROLL_RELATIVE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_ROLL_RELATIVE, ctl->req, (uint8_t *)&ctl->data, ROLL_RELATIVE_LEN);
}

static int uvc_digital_windows_ctl(struct uvc_device *udev, struct ct_dwindow *ctl)
{
    return uvc_send_ctl(udev, UVC_CID_WINDOW, ctl->req, (uint8_t *)&ctl->data, WINDOW_LEN);
}	

static int uvc_roi_ctl(struct uvc_device *udev, struct ct_roi *ctl)
{
    return uvc_send_ctl(udev, UVC_CID_REGION_OF_INTEREST, ctl->req, (uint8_t *)&ctl->data, REGION_OF_INTEREST_LEN);	
}

static int uvc_privacy_shutter_ctl(struct uvc_device *udev, struct ct_privacy_shutter *ctl)
{
    if (ctl->req == PS_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_PRIVACY, ctl->req, (uint8_t *)&ctl->caps, 1);	
    else
        return uvc_send_ctl(udev, UVC_CID_PRIVACY, ctl->req, (uint8_t *)&ctl->bPrivacy, PRIVACY_LEN);
}

static int uvc_backlight_compensation_ctl(struct uvc_device *udev, struct pu_backlight *ctl)
{
    if (ctl->req == BKC_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_BACKLIGHT_COMPENSATION, ctl->req, (uint8_t *)&ctl->caps, 1);	
    else		
        return uvc_send_ctl(udev, UVC_CID_BACKLIGHT_COMPENSATION, ctl->req, (uint8_t *)&ctl->wBacklightCompensation, BACKLIGHT_COMPENSATION_LEN);
}

static int uvc_brightness_ctl(struct uvc_device *udev, struct pu_brightness *ctl)
{
    if (ctl->req == BKC_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_BRIGHTNESS, ctl->req, (uint8_t *)&ctl->caps, 1);	
    else
        return uvc_send_ctl(udev, UVC_CID_BRIGHTNESS, ctl->req, (uint8_t *)&ctl->wBrightness, BRIGHTNESS_LEN);
}

static int uvc_contrast_ctl(struct uvc_device *udev, struct pu_contrast *ctl)
{
    if (ctl->req == BKC_GET_CAP)		
        return uvc_send_ctl(udev, UVC_CID_CONTRAST, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_CONTRAST, ctl->req, (uint8_t *)&ctl->wContrast, CONTRAST_LEN);		
}

static int uvc_contrast_auto_ctl(struct uvc_device *udev, struct pu_contrast_auto *ctl)
{
    if (ctl->req == CONTRASTA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_CONTRAST_AUTO, ctl->req, (uint8_t *)&ctl->caps, 1);
    else			
        return uvc_send_ctl(udev, UVC_CID_CONTRAST_AUTO, ctl->req, (uint8_t *)&ctl->bContrastAuto, CONTRAST_AUTO_LEN);
}

static int uvc_gain_ctl(struct uvc_device *udev, struct pu_gain *ctl)
{
    if (ctl->req == GAIN_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_GAIN, ctl->req, (uint8_t *)&ctl->caps, 1);
    else			
        return uvc_send_ctl(udev, UVC_CID_GAIN, ctl->req, (uint8_t *)&ctl->wGain, GAIN_LEN);
}

static int uvc_power_line_frequency_ctl(struct uvc_device *udev, struct pu_power_line_frequency *ctl)
{
    if (ctl->req == GAIN_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_POWER_LINE_FREQUENCY, ctl->req, (uint8_t *)&ctl->caps, 1);
    else			
        return uvc_send_ctl(udev, UVC_CID_POWER_LINE_FREQUENCY, ctl->req, (uint8_t *)&ctl->bPowerLineFrequency, POWER_LINE_FREQUENCY_LEN);
}

static int uvc_saturation_ctl(struct uvc_device *udev, struct pu_saturation *ctl)
{
    if (ctl->req == SATURATION_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_SATURATION, ctl->req, (uint8_t *)&ctl->caps, 1);
    else	
        return uvc_send_ctl(udev, UVC_CID_SATURATION, ctl->req, (uint8_t *)&ctl->wSaturation, SATURATION_LEN);
}

static int uvc_sharpness_ctl(struct uvc_device *udev, struct pu_sharpness *ctl)
{
    if (ctl->req == SHARPNESS_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_SHARPNESS, ctl->req, (uint8_t *)&ctl->caps, 1);
    else			
        return uvc_send_ctl(udev, UVC_CID_SHARPNESS, ctl->req, (uint8_t *)&ctl->wSharpness, SHARPNESS_LEN);
}

static int uvc_gamma_ctl(struct uvc_device *udev, struct pu_gamma *ctl)
{
    if (ctl->req == GAMMA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_GAMMA, ctl->req, (uint8_t *)&ctl->caps, 1);
    else		
        return uvc_send_ctl(udev, UVC_CID_GAMMA, ctl->req, (uint8_t *)&ctl->wGamma, GAMMA_LEN);
}

static int uvc_white_balance_temperature_ctl(struct uvc_device *udev, struct pu_white_balance_temp *ctl)
{
    if (ctl->req == WBT_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_TEMPERATURE, ctl->req, (uint8_t *)&ctl->caps, 1);
    else		
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_TEMPERATURE, ctl->req, (uint8_t *)&ctl->wWhiteBalanceTemperature, WHITE_BALANCE_TEMPERATURE_LEN);	 
}

static int uvc_white_balance_temperature_auto_ctl(struct uvc_device *udev, struct pu_wbc_auto *ctl)
{
    if (ctl->req == WBTA_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_TEMPERATURE_AUTO, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_TEMPERATURE_AUTO, ctl->req, (uint8_t *)&ctl->bWhiteBalanceComponentAuto, WHITE_BALANCE_TEMPERATURE_AUTO_LEN);
}

static int uvc_white_balance_compont_ctl(struct uvc_device *udev, struct pu_whitebalance_comp *ctl)
{
    if (ctl->req == WBC_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_COMPONENT, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_COMPONENT, ctl->req, (uint8_t *)&ctl->data, WHITE_BALANCE_COMPONENT_LEN);
}

static int uvc_white_balance_compont_auto_ctl(struct uvc_device *udev, struct pu_wbc_auto *ctl)
{
    if (ctl->req == WBC_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_COMPONENT_AUTO, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_WHITE_BALANCE_COMPONENT_AUTO, ctl->req, (uint8_t *)&ctl->bWhiteBalanceComponentAuto, WHITE_BALANCE_COMPONENT_AUTO_LEN);
}

static int uvc_digital_multiplier_ctl(struct uvc_device *udev, struct pu_dmultiplier *ctl)
{
    if (ctl->req == MPL_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_DIGITAL_MULTIPLIER, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_DIGITAL_MULTIPLIER, ctl->req, (uint8_t *)&ctl->wMultiplierStep, DIGITAL_MULTIPLIER_LEN);
}

static int uvc_digital_multiplier_limit_ctl(struct uvc_device *udev, struct pu_dmultiplierlimit *ctl)
{
    if (ctl->req == DMPL_GET_CAP)
        return uvc_send_ctl(udev, UVC_CID_DIGITAL_MULTIPLIER_LIMIT, ctl->req, (uint8_t *)&ctl->caps, 1);
    else
        return uvc_send_ctl(udev, UVC_CID_DIGITAL_MULTIPLIER_LIMIT, ctl->req, (uint8_t *)&ctl->wMultiplierLimit, DIGITAL_MULTIPLIER_LIMIT_LEN);
}

static kmdw_status_t kdp_uvc_ioctl(uint32_t cam_idx, uint32_t cid, void *data, uint16_t len)
{
    struct uvc_device *udev = uvc_video_device;
    int ret;
    
    if ((true != udev->opened) || (NULL == udev)) {
        return KMDW_STATUS_ERROR;
    }
    
    switch (cid) 
    {

        case CID_SCANNING_MODE:
            if (len >= sizeof (struct ct_scm))
                ret = uvc_scanning_mode_ctl(udev, (struct ct_scm *) data);
            else
                ret = -1;
            break;
		
        case CID_AUTO_EXPOSURE_MODE:
            if (len >= sizeof (struct ct_aem))
                ret = uvc_exposure_mode_ctl(udev, (struct ct_aem *) data);	
            else
                ret = -1;
            break;
           
        case CID_AUTO_EXPOSURE_PRIORITY:
            if (len >= sizeof (struct ct_aep))            
                ret = uvc_exposure_priority_ctl(udev, (struct ct_aep *)data);				
            else
                ret = -1;
            break;
            
        case CID_EXPOSURE_TIME_ABSOLUTE:
            if (len >= sizeof (struct ct_eta))              
                ret = uvc_exposure_time_abs_ctl(udev, (struct ct_eta *)data);   			
            else
                ret = -1;
            break;
             
        case CID_EXPOSURE_TIME_RELATIVE:
            if (len >= sizeof (struct ct_etr))              
                ret = uvc_shutter_speed_ctl(udev, (struct ct_etr *) data);
            else
                ret = -1;
            break;
            
        case CID_FOCUS_AUTO:
            if (len >= sizeof (struct ct_fauto))              
                ret = uvc_focus_auto_ctl(udev, (struct ct_fauto *) data);			
            else
                ret = -1;
            break;            
           
        case CID_FOCUS_ABSOLUTE:
            if (len >= sizeof (struct ct_focus_a))              
                ret = uvc_focus_abs_ctl(udev, (struct ct_focus_a *) data);
            else
                ret = -1;
            break;
            
        case CID_FOCUS_RELATIVE:
            if (len >= sizeof (struct ct_focus_r))              
                ret = uvc_focus_rel_ctl(udev, (struct ct_focus_r *) data);
            else
                ret = -1;
            break;
            
        case CID_IRIS_ABSOLUTE:
            if (len >= sizeof (struct ct_iris_a))             
                ret = uvc_iris_abs_ctl(udev, (struct ct_iris_a *) data);
            else
                ret = -1;
            break;
            
        case CID_IRIS_RELATIVE:
            if (len >= sizeof (struct ct_iris_r))             
                ret = uvc_iris_rel_ctl(udev, (struct ct_iris_r *) data);				
            else
                ret = -1;
            break;
            
        case CID_ZOOM_ABSOLUTE:
            if (len >= sizeof (struct ct_zoom_a))              
                ret = uvc_zoom_abs_ctl(udev, (struct ct_zoom_a *) data);
            else
                ret = -1;
            break;
		
        case CID_ZOOM_RELATIVE:
            if (len >= sizeof (struct ct_zoom_r))              
                ret = uvc_zoom_rel_ctl(udev, (struct ct_zoom_r *) data);
            else
                ret = -1;
            break;
            
        case CID_PANTILT_ABSOLUTE:
            if (len >= sizeof (struct ct_pan_tilt_a))             
                ret = uvc_pan_tilt_abs_ctl(udev, (struct ct_pan_tilt_a *) data);	
            else
                ret = -1;
            break;
            
        case CID_PANTILT_RELATIVE:
            if (len >= sizeof (struct ct_pan_tilt_r))              
                ret = uvc_pan_tilt_rel_ctl(udev, (struct ct_pan_tilt_r *) data);				
            else
                ret = -1;
            break;
            
        case CID_ROLL_ABSOLUTE:
            if (len >= sizeof (struct ct_roll_a))               
                ret = uvc_roll_abs_ctl(udev, (struct ct_roll_a *) data);	
            else
                ret = -1;
            break;
            
        case CID_ROLL_RELATIVE:
            if (len >= sizeof (struct ct_roll_r))            
                ret = uvc_roll_rel_ctl(udev, (struct ct_roll_r *) data);					
            else
                ret = -1;
            break;
            
        case CID_PRIVACY:
            if (len >= sizeof (struct ct_privacy_shutter))             
                ret = uvc_privacy_shutter_ctl(udev, (struct ct_privacy_shutter *) data);			
            else
                ret = -1;
            break;
            
        case CID_FOCUS_SIMPLE:
            if (len >= sizeof (struct ct_focus_sr))            
                ret = uvc_focus_simple_range_ctl(udev, (struct ct_focus_sr *) data);			
            else
                ret = -1;
            break;
            
        case CID_DIGITAL_WINDOW:
            if (len >= sizeof (struct ct_dwindow))             
                ret = uvc_digital_windows_ctl(udev, (struct ct_dwindow *) data);
            else
                ret = -1;
            break;
            
        case CID_REGION_OF_INTEREST:
            if (len >= sizeof (struct ct_roi))               
                ret = uvc_roi_ctl(udev, (struct ct_roi *) data);					
            else
                ret = -1;
            break;
            
        case CID_BRIGHTNESS:
            if (len >= sizeof (struct pu_brightness))             
                ret = uvc_brightness_ctl(udev, (struct pu_brightness *) data);
            else
                ret = -1;
            break;
            
        case CID_CONTRAST:
            if (len >= sizeof (struct pu_contrast))               
                ret = uvc_contrast_ctl(udev, (struct pu_contrast *) data);
            else
                ret = -1;
            break;
            
        case CID_HUE:
            if (len >= sizeof (struct pu_hue))             
                ret = uvc_hue_ctl(udev, (struct pu_hue *) data);
            else
                ret = -1;
            break;
            
        case CID_SATURATION:
            if (len >= sizeof (struct pu_saturation))             
                ret = uvc_saturation_ctl(udev, (struct pu_saturation *) data);
            else
                ret = -1;
            break;
            
        case CID_SHARPNESS:
            if (len >= sizeof (struct pu_sharpness))             
                ret = uvc_sharpness_ctl(udev, (struct pu_sharpness *) data);	
            else
                ret = -1;
            break;
            
        case CID_GAMMA:
            if (len >= sizeof (struct pu_gamma))            
                ret = uvc_gamma_ctl(udev, (struct pu_gamma *) data);
            else
                ret = -1;
            break;
            
        case CID_WHITE_BALANCE_TEMPERATURE:
            if (len >= sizeof (struct pu_white_balance_temp))             
                ret = uvc_white_balance_temperature_ctl(udev, (struct pu_white_balance_temp *) data);
            else
                ret = -1;
            break;
            
        case CID_WHITE_BALANCE_COMPONENT:
            if (len >= sizeof (struct pu_whitebalance_comp))             
                ret = uvc_white_balance_compont_ctl(udev, (struct pu_whitebalance_comp *) data);
            else
                ret = -1;
            break;
            
        case CID_BACKLIGHT_COMPENSATION:
            if (len >= sizeof (struct pu_backlight))             
                ret = uvc_backlight_compensation_ctl(udev, (struct pu_backlight *) data);			
            else
                ret = -1;
            break;
            
        case CID_GAIN:
            if (len >= sizeof (struct pu_gain))             
                ret = uvc_gain_ctl(udev, (struct pu_gain *) data);
            else
                ret = -1;
            break;
            
        case CID_POWER_LINE_FREQUENCY:
            if (len >= sizeof (struct pu_power_line_frequency))             
                ret = uvc_power_line_frequency_ctl(udev, (struct pu_power_line_frequency *) data);
            else
                ret = -1;
            break;
            
        case CID_HUE_AUTO:
            if (len >= sizeof (struct pu_hue_auto))             
                ret = uvc_hue_auto_ctl(udev, (struct pu_hue_auto *) data);
            else
                ret = -1;
            break;
            
        case CID_WHITE_BALANCE_TEMPERATURE_AUTO:
            if (len >= sizeof (struct pu_wbc_auto))             
                ret = uvc_white_balance_temperature_auto_ctl(udev, (struct pu_wbc_auto *) data);			
            else
                ret = -1;
            break;
            
        case CID_WHITE_BALANCE_COMPONENT_AUTO:
            if (len >= sizeof (struct pu_wbc_auto))             
                ret = uvc_white_balance_compont_auto_ctl(udev, (struct pu_wbc_auto *) data);
            else
                ret = -1;
            break;
            
        case CID_DIGITAL_MULTIPLIER:
            if (len >= sizeof (struct pu_dmultiplier))             
                ret = uvc_digital_multiplier_ctl(udev, (struct pu_dmultiplier *) data);
            else
                ret = -1;
            break;
            
        case CID_DIGITAL_MULTIPLIER_LIMIT:
            if (len >= sizeof (struct pu_dmultiplierlimit))             
                ret = uvc_digital_multiplier_limit_ctl(udev, (struct pu_dmultiplierlimit *) data);				
            else
                ret = -1;
            break;
            
        case CID_CONTRAST_AUTO:
            if (len >= sizeof (struct pu_contrast_auto))             
                ret = uvc_contrast_auto_ctl(udev, (struct pu_contrast_auto *) data);
            else
                ret = -1;
            break;
             
        case CID_LIST_ALL:
            if (len >= 2 * sizeof (uint32_t))             
                ret = uvc_list_ctl((uint32_t *) data);
            else
                ret = -1;
            break;            
          
        default:
            ret = -1;
            break;
    }
    if (ret < 0)
        return KMDW_STATUS_ERROR;
    return KMDW_STATUS_OK;

}

static uint32_t uvc_k_get_bytesperline(const struct uvc_format *format,
                                       const struct uvc_frame *frame)
{
    return format->bpp * frame->wWidth;
}

static uint32_t uvc_try_frame_interval(struct uvc_frame *frame, uint32_t interval)
{
    uint32_t new_interval;
    int i;
    if (frame->bFrameIntervalType != 0) {
        uint32_t best = 0xFFFFFFFF, dist;
        for (i = 0; i < frame->bFrameIntervalType; ++i) {
            dist = interval >  frame->dwFrameInterval[i]
                   ? interval - frame->dwFrameInterval[i]
                   : frame->dwFrameInterval[i] - interval;
            if (dist > best)
                break;
            best = dist;
        }
        new_interval = frame->dwFrameInterval[i-1];
    } else {
        const uint32_t min = frame->dwFrameInterval[0];
        const uint32_t max = frame->dwFrameInterval[1];
        const uint32_t step = frame->dwFrameInterval[2];

        new_interval = min + (new_interval - min + step/2) / step * step;
        if (interval > max)
            interval = max;
    }
    return new_interval;
}

static kmdw_status_t kdp_uvc_stream_off(uint32_t cam_idx)
{
    struct uvc_device *udev = uvc_video_device;
    struct uvc_streaming *stream = udev->curr_stream;
    
    if (0 > uvc_video_disable(stream))
        return KMDW_STATUS_ERROR;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kdp_uvc_stream_on(uint32_t cam_idx)
{
    struct uvc_device *udev = uvc_video_device;
    struct uvc_streaming *stream = udev->curr_stream;

    if (0 > uvc_video_enable(stream))
        return KMDW_STATUS_ERROR;
    return KMDW_STATUS_OK;

}

static kmdw_status_t kdp_uvc_start_capture(uint32_t cam_idx, kmdw_camera_callback_t img_cb)
{
    struct uvc_device *udev = uvc_video_device;
    struct uvc_streaming *stream = udev->curr_stream;

    if (udev->opened != true)
        return KMDW_STATUS_ERROR;
    if ((NULL == udev) || (stream == NULL))
        return KMDW_STATUS_ERROR;
    if (0 > uvc_video_enable(stream))
        return KMDW_STATUS_ERROR;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kdp_uvc_stop_capture(uint32_t cam_idx)
{
    struct uvc_device *udev = uvc_video_device;
    struct uvc_streaming *stream = udev->curr_stream;

    if (udev->opened != true)
        return KMDW_STATUS_ERROR;
    if ((NULL == udev) || (stream == NULL))
        return KMDW_STATUS_ERROR;
    if (0 > uvc_video_disable(stream))
        return KMDW_STATUS_ERROR;
    return KMDW_STATUS_OK;    

}

#define TILE_BLOCK_MAX_W        10
#define TILE_BLOCK_MAX_H        6
#define TILE_BLOCKS_MAX         (TILE_BLOCK_MAX_W * TILE_BLOCK_MAX_H)   //


static kmdw_status_t kdp_uvc_buffer_init(uint32_t cam_idx, uint32_t buf_addr_0, uint32_t buf_addr_1)
{
    struct uvc_device *udev = uvc_video_device;
    uint32_t sizeImage = udev->curr_stream->cur_format->bpp * udev->curr_stream->cur_format->cur_frame->wWidth * udev->curr_stream->cur_format->cur_frame->wHeight;
    
    if (0 != udev->curr_stream->frame_buf)
        return KMDW_STATUS_OK;
    if (0 == isoch_pipe)
        isoch_pipe = USBH_UVC_PipeCreate_Isoch(0, udev->curr_stream->if_alt[udev->curr_stream->curr_altnum-1].addr, 
		                                udev->curr_stream->if_alt[udev->curr_stream->curr_altnum-1].maxpacketsize, 
		                                udev->curr_stream->if_alt[udev->curr_stream->curr_altnum-1].interval);
	
    udev->curr_stream->isoch_pipe = isoch_pipe;

    if (0 == udev->curr_stream->isoch_pipe)
        return KMDW_STATUS_ERROR;

    udev->curr_stream->frame_buf = buf_addr_0;

    ping_pong_buf_addr[0] = buf_addr_0;
    ping_pong_buf_addr[1] = buf_addr_1;

    return KMDW_STATUS_OK;
}

static kmdw_status_t kdp_uvc_get_format(uint32_t cam_idx, struct cam_format *p)
{
    struct uvc_device *udev = uvc_video_device;
    struct uvc_frame *frame;
    struct uvc_format *format;

    format = udev->curr_stream->cur_format;
    frame = udev->curr_stream->cur_format->cur_frame;

    if (format == NULL || frame == NULL) {
        return KMDW_STATUS_ERROR;
    }

    p->pixelformat = format->fcc;
    p->width = frame->wWidth;
    p->height = frame->wHeight;
    p->bytesperline = uvc_k_get_bytesperline(format, frame);
    p->sizeimage = udev->stream->vs_ctrl_info->curr->dwMaxVideoFrameSize;
    p->colorspace = format->colorspace;
    return KMDW_STATUS_OK;
}

static int uvc_try_format(struct uvc_streaming *stream,
                                struct cam_format *cam_fmt, struct uvc_format **uvc_format,
                                struct uvc_frame **uvc_frame)
{
    struct uvc_format *format = NULL;
    struct uvc_frame *frame = NULL;
    uint16_t rw, rh;
    unsigned int d, maxd;
    unsigned int i;
    uint32_t interval;
    int ret = 0;

    for (i = 0; i < stream->nformats; ++i) {
        format = &stream->format[i];
        if (format->fcc == cam_fmt->pixelformat)
            break;
    }
    if (i == stream->nformats) {
        format = stream->cur_format;
        cam_fmt->pixelformat = format->fcc;
    }
  
    rw = cam_fmt->width;
    rh = cam_fmt->height;
    maxd = (unsigned int)-1;

    for (i = 0; i < format->nframes; ++i) {
        uint16_t w = format->frame[i].wWidth;
        uint16_t h = format->frame[i].wHeight;

        d = MIN(w, rw) * MIN(h, rh);
        d = w*h + rw*rh - 2*d;
        if (d < maxd) {
            maxd = d;
            frame = &format->frame[i];
        }
        if (maxd == 0)
            break;
    }
    if (NULL == frame) {
        return -1;
    }

    interval = frame->dwDefaultFrameInterval;
    stream->vs_ctrl_info->curr->wmHint = 1;    /* dwFrameInterval */
    stream->vs_ctrl_info->curr->bFormatIndex = format->index;
    stream->vs_ctrl_info->curr->bFrameIndex = frame->bFrameIndex;
    stream->vs_ctrl_info->curr->dwFrameInterval = uvc_try_frame_interval(frame, interval);
    cam_fmt->width = frame->wWidth;
    cam_fmt->height = frame->wHeight;
    cam_fmt->bytesperline = uvc_k_get_bytesperline(format, frame);
    cam_fmt->sizeimage = format->bpp * frame->wWidth * frame->wHeight;
    cam_fmt->colorspace = format->colorspace;
    stream->imagesize = format->bpp * frame->wWidth * frame->wHeight;
    if (uvc_format != NULL)
        *uvc_format = format;
    if (uvc_frame != NULL)
        *uvc_frame = frame;

    return ret;
}

static kmdw_status_t kdp_uvc_set_format(uint32_t cam_idx, struct cam_format  *p)
{
    struct uvc_device *udev = uvc_video_device;

    struct uvc_format *format;
    struct uvc_frame *frame;

    if (0 >  uvc_try_format(udev->curr_stream, p, &format, &frame))
        return KMDW_STATUS_ERROR;

    udev->curr_stream->cur_format = format;
    udev->curr_stream->cur_format->cur_frame = frame;

    return KMDW_STATUS_OK;
}

static kmdw_status_t kdp_uvc_query_capability(uint32_t cam_idx, struct cam_capability *cap)
{
    strncpy((char *)cap->driver, "KDP UVC", sizeof(cap->driver));

    strncpy(cap->desc, KDP_UVC_DRIVER_DESC, sizeof(cap->desc));
    cap->version = KDP_UVC_DRIVER;
    cap->capabilities = V2K_CAP_VIDEO_CAPTURE | V2K_CAP_STREAMING | V2K_CAP_DEVICE_CAPS;

    return KMDW_STATUS_OK;
}


static kmdw_status_t kdp_uvc_close(uint32_t cam_idx)
{
    struct uvc_device *udev;
    if (NULL == (udev = video_dev("video0")))
        return KMDW_STATUS_ERROR;
    kdp_uvc_stream_off(cam_idx);		
    udev->opened = false;
    return KMDW_STATUS_OK;
}

static kmdw_status_t kdp_uvc_open (uint32_t cam_idx)
{
    struct uvc_device *udev;
    if (NULL == (udev = video_dev("video0")))
        return KMDW_STATUS_ERROR;
    if (udev->inited == false) {
        if (KMDW_STATUS_ERROR == kdp_uvc_buffer_init(cam_idx, NULL, NULL))
            return KMDW_STATUS_ERROR;
    }
    udev->opened = true;
    return KMDW_STATUS_OK;
}

static struct cam_ops kdp_uvc_ops = {
    .open               = kdp_uvc_open,
    .close              = kdp_uvc_close,
    .query_capability   = kdp_uvc_query_capability,
    .set_format         = kdp_uvc_set_format,
    .get_format         = kdp_uvc_get_format,
    .buffer_init        = kdp_uvc_buffer_init,
    .start_capture      = kdp_uvc_start_capture,
    .stop_capture       = kdp_uvc_stop_capture,
    .stream_on          = kdp_uvc_stream_on,
    .stream_off         = kdp_uvc_stream_off,
    .ioctl              = kdp_uvc_ioctl,

};


int uvc_parse_vs_data_ep(struct uvc_streaming *stream, uint8_t *p_data)
{

    struct usb_interface_descriptor *vs_inf;
    struct usb_endpoint_descriptor *ep;
    uint16_t off = 0;
    int i = 0;
    vs_inf = (struct usb_interface_descriptor *)(p_data + off);
    stream->ifnum = vs_inf->bInterfaceNumber;
    stream->num_ep = vs_inf->bNumEndpoints;
    stream->num_alt = 0;

    while (vs_inf->bInterfaceNumber == stream->ifnum) {
        if (vs_inf->bDescriptorType != USB_DT_INTERFACE)
            return -1;
        if (vs_inf->bInterfaceClass != UVC_CC_VIDEO)
            return -1;
        if (vs_inf->bInterfaceSubClass != UVC_SC_VIDEOSTREAMING)
            return -1;
        stream->num_alt++;
        off += vs_inf->bLength;
        ep = (struct usb_endpoint_descriptor*) (p_data + off);
        off += ep->bLength;
        vs_inf = (struct usb_interface_descriptor *)(p_data + off);
    }
    stream->if_alt = (struct uvc_vs_alt_intf*)malloc(stream->num_alt * sizeof (struct uvc_vs_alt_intf));
    if (NULL == stream->if_alt)
        return -1;
#ifdef KDP_UVC_DEBUG		
    kmdw_printf("@@ stream->if_alt %p\n", stream->if_alt);
#endif		
    memset(stream->if_alt, 0, stream->num_alt * sizeof (struct uvc_vs_alt_intf));
    off = 0;
    vs_inf = (struct usb_interface_descriptor *)(p_data + off);
    while (vs_inf->bInterfaceNumber == stream->ifnum) {
        off += vs_inf->bLength;
        ep = (struct usb_endpoint_descriptor*) (p_data + off);
        off += ep->bLength;
  
        stream->if_alt[i].addr = ep->bEndpointAddress;
        stream->if_alt[i].ep_type = ep->bmAttributes;
        stream->if_alt[i].maxpacketsize = ep->wMaxPacketSize;
        stream->if_alt[i].alt_num = vs_inf->bAlternateSetting;
        stream->if_alt[i++].interval = ep->bInterval;
        vs_inf = (struct usb_interface_descriptor *)(p_data + off);
    }
    stream->curr_altnum = 1;
    return off;
}
static int  uvc_parse_still_image(struct uvc_streaming *stream, uint8_t *p_data)
{
    struct uvc_still_image_frame_descriptor  *still_image_frame  = (struct uvc_still_image_frame_descriptor  *)p_data;  
    if (still_image_frame->bDescriptorType != CS_INTERFACE)
        return -1;
    if (still_image_frame->bDescriptorSubType != UVC_VS_STILL_IMAGE_FRAME)
        return 0;
    
    return still_image_frame->bLength;  
}
static int uvc_parse_color_match(struct uvc_streaming *stream, uint8_t *p_data)
{

    struct uvc_color_matching_descriptor *color_match = (struct uvc_color_matching_descriptor *)p_data;

    if (color_match->bDescriptorType != CS_INTERFACE)
        return -1;
    if (color_match->bDescriptorSubType != UVC_VS_COLORFORMAT)
        return 0;
    return color_match->bLength;
}


static int uvc_parse_vs_frame(struct uvc_frame *frame, uint8_t *p_data, uint8_t type)
{
    struct uvc_frame_uncompressed *frame_uncomp;

    if (type == UVC_VS_FORMAT_UNCOMPRESSED ) {
        frame_uncomp = (struct uvc_frame_uncompressed *)p_data;
	    frame->bDescriptorType = frame_uncomp->bDescriptorSubType;
        frame->bFrameIndex = frame_uncomp->bFrameIndex;
        frame->bmCapabilities = frame_uncomp->bmCapabilities;
        frame->wWidth = frame_uncomp->wWidth;
        frame->wHeight = frame_uncomp->wHeight;
        frame->dwMinBitRate = frame_uncomp->dwMinBitRate;
        frame->dwMaxBitRate = frame_uncomp->dwMaxBitRate;
        frame->dwMaxVideoFrameBufferSize = frame_uncomp->dwMaxVideoFrameBufferSize;
        frame->dwDefaultFrameInterval = frame_uncomp->dwDefaultFrameInterval;
        frame->bFrameIntervalType = frame_uncomp->bFrameIntervalType;
        if (frame->bFrameIntervalType == 0) {
            frame->dwFrameInterval = (uint32_t*)malloc(sizeof (struct cont_frame_intervals));
            memset(frame->dwFrameInterval, 0, sizeof (struct cont_frame_intervals));
#ifdef KDP_UVC_DEBUG					
            kmdw_printf("@@ frame->dwFrameInterval %p\n", frame->dwFrameInterval);
#endif						
            ((struct cont_frame_intervals*)frame->dwFrameInterval)->dwMinFrameInterval = frame_uncomp->dwFrameInterval[0];
            ((struct cont_frame_intervals*)frame->dwFrameInterval)->dwMaxFrameInterval = frame_uncomp->dwFrameInterval[1];
            ((struct cont_frame_intervals*)frame->dwFrameInterval)->dwFrameIntervalStep = frame_uncomp->dwFrameInterval[2];
        } else {
            frame->dwFrameInterval = (uint32_t *)malloc(frame->bFrameIntervalType * sizeof (uint32_t));
            if (NULL == frame->dwFrameInterval)
                return -1;
#ifdef KDP_UVC_DEBUG						
           kmdw_printf("@@ frame->dwFrameInterval %p\n", frame->dwFrameInterval);
#endif		
            memset(frame->dwFrameInterval, 0, frame->bFrameIntervalType * sizeof (uint32_t));           
            uint32_t *p = (uint32_t*) frame->dwFrameInterval;
            for ( int j = 0; j < frame->bFrameIntervalType; j++) {
                p[j] = frame_uncomp->dwFrameInterval[j];
            }
        }
    }
    return 0;
}

static int32_t uvc_parse_vs_format(struct uvc_format *format, uint8_t *p_data)
{
    struct uvc_format_desc_head *format_h = (struct uvc_format_desc_head *) p_data;
    uint16_t off = 0;
    struct uvc_frame_desc_head *frame_h;

    if (format_h->bDescriptorType != CS_INTERFACE)
        return -1;

    if (format_h->bDescriptorSubtype == UVC_VS_FORMAT_MJPEG) {
        struct uvc_format_mjpeg *f_mjpeg = (struct uvc_format_mjpeg *)p_data;

        off += f_mjpeg->bLength;
        format->nframes = f_mjpeg->bNumFrameDescriptors;
        format->type = UVC_VS_FORMAT_MJPEG;
    } else if (format_h->bDescriptorSubtype == UVC_VS_FORMAT_UNCOMPRESSED) {
        struct uvc_format_uncompressed *f_uncomp = (struct uvc_format_uncompressed *)p_data;
        off += f_uncomp->bLength;
        format->nframes = f_uncomp->bNumFrameDescriptors;
        format->type = UVC_VS_FORMAT_UNCOMPRESSED;
        format->fcc = V2K_PIX_FMT_YCBCR;
        format->bpp = f_uncomp->bBitsPerPixel >>3;
        format->index = f_uncomp->bFormatIndex;

    } else if (format_h->bDescriptorSubtype == UVC_VS_FORMAT_FRAME_BASED) {
        struct uvc_format_frame_based *f_frame_based = (struct uvc_format_frame_based *)p_data;
        off += f_frame_based->bLength;
        format->nframes = f_frame_based->bNumFrameDescriptors;
        format->type = UVC_VS_FORMAT_FRAME_BASED;
    }
    format->frame = (struct uvc_frame *) malloc(format->nframes * sizeof( struct uvc_frame));
    if (NULL == format->frame)
        return -1;		
    memset(format->frame, 0, format->nframes * sizeof( struct uvc_frame));

#ifdef KDP_UVC_DEBUG		
    kmdw_printf("@@ format->frame %p\n", format->frame);
#endif		
    for (int i = 0; i < format->nframes; i++) {
        frame_h = (struct uvc_frame_desc_head*) (p_data + off);
        uvc_parse_vs_frame(&format->frame[i], p_data + off, format->type);
        if (format->frame[i].bDescriptorType == UVC_VS_FRAME_UNCOMPRESSED)
            format->cur_frame_num = 0;
        off += frame_h->bLength;
    }
    format->cur_frame = &format->frame[format->cur_frame_num];
    return off;
}


int uvc_parse_vs_inf(struct uvc_streaming *stream, const uint8_t *buffer)
{
    int ret;
    uint32_t off = 0;
    struct usb_interface_descriptor  *p_inf = (struct usb_interface_descriptor * )buffer;
    struct uvc_input_header_descriptor *in_head;
    off += p_inf->bLength;

    if (p_inf->bDescriptorType != USB_DT_INTERFACE)
        return -1;
    if (p_inf->bInterfaceClass != UVC_CC_VIDEO)
        return -1;
    if (p_inf->bInterfaceSubClass != UVC_SC_VIDEOSTREAMING)
        return -1;

    stream->ifnum = p_inf->bInterfaceNumber;

    in_head = (struct uvc_input_header_descriptor *) &buffer[off];

    if (in_head->bDescriptorType != CS_INTERFACE)
        return -1;
    if (in_head->bDescriptorSubType != UVC_VS_INPUT_HEADER)
        return -1;
    stream->nformats = in_head->bNumFormats;
    stream->ep_addr = in_head->bEndpointAddress;
    stream->TerminalId = in_head->bTerminalLink;
    stream->ControlSize = in_head->bControlSize;
    stream->cur_format_num = -1;
    off += in_head->bLength;

    stream->format = ( struct uvc_format *)malloc( in_head->bNumFormats * sizeof( struct uvc_format ));
    if (NULL == stream->format)
        return -1;
    memset(stream->format, 0, in_head->bNumFormats * sizeof( struct uvc_format ));
#ifdef KDP_UVC_DEBUG	
    kmdw_printf("@@ stream->nformats %x\n", stream->nformats);	
    kmdw_printf("@@ stream->format %p\n", stream->format);
#endif		
    for (int i = 0; i < in_head->bNumFormats; i++) {

        off += uvc_parse_vs_format(&(stream->format[i]), (uint8_t *)&buffer[off]);
        if (stream->format[i].type == UVC_VS_FORMAT_UNCOMPRESSED) {
            if (0 > stream->cur_format_num)
                stream->cur_format_num = i;
        }
        else
            stream->cur_format_num = 0;
        if (0 > (ret = uvc_parse_still_image(stream, (uint8_t *)&buffer[off])))
            return ret;
        off += ret;
        if (0 > (ret = uvc_parse_color_match(stream, (uint8_t *)&buffer[off])))
            return ret;
        off += ret;
    }

    stream->cur_format =(struct uvc_format *) &(stream->format[stream->cur_format_num]);
 
    return off;
}

static int uvc_parse_vc_int_ep(struct uvc_device *dev,
                               const uint8_t *buffer)
{
    uint32_t off = 0;
    struct usb_endpoint_descriptor *ep = (struct usb_endpoint_descriptor *) buffer;
    struct uvc_control_endpoint_descriptor *vc_int_ep;

    if (ep->bDescriptorType != USB_DT_ENDPOINT)
        return -1;
    if (ep->bmAttributes != USB_ENDPOINT_XFER_INT)
        return -1;
    dev->int_ep = (struct uvc_vc_int_ep *)malloc(sizeof( struct uvc_vc_int_ep));
    if (NULL == dev->int_ep )
        return -1;
    memset(dev->int_ep, 0, sizeof( struct uvc_vc_int_ep));
#ifdef KDP_UVC_DEBUG		
    kmdw_printf("@@ dev->int_ep %p\n", dev->int_ep);
#endif		
    dev->int_ep->addr= ep->bEndpointAddress;
    dev->int_ep->interval = ep->bInterval;
    dev->int_ep->maxpacketsize = ep->wMaxPacketSize;
    off += ep->bLength;
    vc_int_ep = (struct uvc_control_endpoint_descriptor *)(buffer + off);
    if (vc_int_ep->bDescriptorType != CS_ENDPOINT)
        return -1;
    if (vc_int_ep->bDescriptorSubType != USB_ENDPOINT_XFER_INT)
        return -1;
    dev->int_ep->wMaxTransferSize = vc_int_ep->wMaxTransferSize;
    off += vc_int_ep->bLength;

    return off;
}

static int uvc_parse_ET(struct uvc_device *dev,
                        const uint8_t *buffer, int buflen)
{

    struct uvc_ET_Head_descriptor *et_head = (struct uvc_ET_Head_descriptor *)buffer;
   
    uint32_t off = 0;
    uint16_t buf_len_t = buflen;
    uint8_t* buf_t = (uint8_t * )buffer;

    while (buf_len_t) {
        if (et_head->bDescriptorSubType == UVC_VC_INPUT_TERMINAL) {
       
            struct uvc_input_terminal_descriptor *IT = (struct uvc_input_terminal_descriptor *)(buf_t + off);
            buf_len_t -= IT->bLength;
            off += IT->bLength;
            dev->nITs++;
        }
        if (et_head->bDescriptorSubType  == UVC_VC_OUTPUT_TERMINAL) {
       
            struct uvc_output_terminal_descriptor *OT  = (struct uvc_output_terminal_descriptor *) (buf_t + off);
            buf_len_t -= OT->bLength;
            off += OT->bLength;
            dev->nOTs++;
        }
        if (et_head->bDescriptorSubType == UVC_VC_SELECTOR_UNIT) {
        
            struct uvc_selector_unit_descriptor *SU = (struct uvc_selector_unit_descriptor *) (buf_t + off);
            buf_len_t -= SU->bLength;
            off += SU->bLength;
            dev->nSUs++;
        }
        if (et_head->bDescriptorSubType == UVC_VC_PROCESSING_UNIT) {

            struct uvc_processing_unit_descriptor *PU = (struct uvc_processing_unit_descriptor *) (buf_t + off);
            buf_len_t -= PU->bLength;
            off += PU->bLength;
            dev->nPUs++;
        }

        if (et_head->bDescriptorSubType == UVC_VC_EXTENSION_UNIT) {

            struct uvc_extension_unit_descriptor *XU = (struct uvc_extension_unit_descriptor *) (buf_t + off);

            buf_len_t -= XU->bLength;
            off += XU->bLength;
            dev->nXUs++;
        }

        et_head = (struct uvc_ET_Head_descriptor *)((uint8_t*) buf_t + off);

    }

    if (dev->nITs != 0) {
        dev->IT = (struct uvc_it *) malloc(dev->nITs * sizeof(struct uvc_it));
        if (NULL == dev->IT )
            return -ENOMEM;
        memset(dev->IT, 0, dev->nITs * sizeof (struct uvc_it));
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->IT %p\n", dev->IT);
        kmdw_printf("@@ dev->IT size %x\n", dev->nITs * sizeof(struct uvc_it));
#endif				
    }
    if (dev->nOTs != 0) {
        dev->OT = (struct uvc_ot *) malloc(dev->nOTs * sizeof(struct uvc_ot));
        if (NULL == dev->OT)
            return -ENOMEM;
        memset(dev->OT, 0, dev->nOTs * sizeof (struct uvc_ot));
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->OT %p\n", dev->OT);
        kmdw_printf("@@ dev->OT size %x\n", dev->nOTs * sizeof(struct uvc_ot));       
#endif				
    }
    if (dev->nSUs != 0) {
        dev->SU = (struct uvc_su *) malloc(dev->nSUs * sizeof(struct uvc_su));
        if (NULL == dev->SU)
            return -ENOMEM;
        memset(dev->SU, 0, dev->nSUs * sizeof (struct uvc_su));
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->SU %p\n", dev->SU);
        kmdw_printf("@@ dev->SU size %x\n", dev->nSUs * sizeof(struct uvc_ot));            
#endif				
    }
    if (dev->nPUs != 0) {
        dev->PU = (struct uvc_pu *) malloc(dev->nPUs * sizeof(struct uvc_pu));
        if (NULL == dev->PU)
            return -ENOMEM;
        memset(dev->PU, 0, dev->nPUs * sizeof (struct uvc_pu));
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->PU %p\n", dev->PU);
        kmdw_printf("@@ dev->PU size %x\n", dev->nPUs * sizeof(struct uvc_pu));            
#endif				
    }

    if (dev->nXUs != 0) {
        dev->XU = (struct uvc_xu *) malloc(dev->nXUs * sizeof(struct uvc_xu));
        if (NULL == dev->XU)
            return -ENOMEM;
        memset(dev->XU, 0, dev->nXUs * sizeof (struct uvc_xu));
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->XU %p\n", dev->XU);
        kmdw_printf("@@ dev->XU size %x\n", dev->nXUs * sizeof(struct uvc_xu));          
#endif				
    }

    buf_len_t = buflen;
    buf_t = (uint8_t * )buffer;
    et_head = (struct uvc_ET_Head_descriptor *) buffer;
    off = 0;
    uint8_t c = 0, o = 0, s = 0, p = 0, x = 0;
    
    while (buf_len_t) {
        if (et_head->bDescriptorSubType == UVC_VC_INPUT_TERMINAL) {
         
            struct uvc_input_terminal_descriptor *IT = (struct uvc_input_terminal_descriptor *)(buf_t + off);
            buf_len_t -= IT->bLength;
            off += IT->bLength;
            dev->IT[c].id = IT->bTerminalID;
            dev->IT[c].wTerminalType = IT->wTerminalType;
            if (IT->wTerminalType == UVC_ITT_CAMERA) {
     
                struct uvc_camera_terminal_descriptor* CT = (struct uvc_camera_terminal_descriptor *) IT;
                dev->IT[c].ct = (struct uvc_ct*)malloc(sizeof (struct uvc_ct));
                if (NULL == dev->IT[c].ct)
                    return -ENOMEM;
                memset(dev->IT[c].ct, 0, sizeof (struct uvc_ct));
#ifdef KDP_UVC_DEBUG								
                kmdw_printf("@@ dev->IT[c].ct %p\n", dev->IT[c].ct);
#endif								
                dev->IT[c].ct->wObjectiveFocalLengthMax = CT->wObjectiveFocalLengthMax;
                dev->IT[c].ct->wObjectiveFocalLengthMin = CT->wObjectiveFocalLengthMin;
                dev->IT[c].ct->wOcularFocalLength = CT->wOcularFocalLength;
                for (int j = 0; j < CT->bControlSize; j++)
                    dev->IT[c].ct->bmControls |= (uint8_t )((uint8_t *)CT+ UVC_DT_CT_CONST_LEN)[j]  << j*8 ;

            }
             c++;
        }
        if (et_head->bDescriptorSubType  == UVC_VC_OUTPUT_TERMINAL) {

            struct uvc_output_terminal_descriptor * OT  = (struct uvc_output_terminal_descriptor *) (buf_t + off);
            buf_len_t -= OT->bLength;
            off += OT->bLength;

            dev->OT[o].id = OT->bTerminalID;
            dev->OT[o].bAssocTerminal= OT->bAssocTerminal;
            dev->OT[o].wTerminalType = OT->wTerminalType;
            dev->OT[o++].baSourceID = OT->bSourceID;
        }

        if (et_head->bDescriptorSubType == UVC_VC_SELECTOR_UNIT) {

            struct uvc_selector_unit_descriptor *SU = (struct uvc_selector_unit_descriptor *) (buf_t + off);
            buf_len_t -= SU->bLength;
            off += SU->bLength;
            dev->SU[s].id = SU->bUnitID;
            dev->SU[s].bNrInPins = SU->bNrInPins;
            dev->SU[s].baSourceID = (uint8_t *)malloc(dev->SU[s].bNrInPins);
            if (NULL == dev->SU[s].baSourceID)
                return -ENOMEM;
            memset(dev->SU[s].baSourceID, 0, dev->SU[s].bNrInPins);
#ifdef KDP_UVC_DEBUG						
            kmdw_printf("@@ dev->SU[s].baSourceID %p\n", dev->SU[s].baSourceID);
#endif						
            for (int j =0 ; j < dev->SU[s].bNrInPins; j++)
                dev->SU[s].baSourceID[j] = (uint8_t )((uint8_t *)SU + UVC_SU_CONST_LEN)[j];
            s++;
        }
        if (et_head->bDescriptorSubType == UVC_VC_PROCESSING_UNIT) {

            struct uvc_processing_unit_descriptor *PU = (struct uvc_processing_unit_descriptor *) (buf_t + off);
            int j;
            buf_len_t -= PU->bLength;
            off += PU->bLength;
            dev->PU[p].id = PU->bUnitID;
            dev->PU[p].baSourceID = PU->bSourceID;
            for (j = 0 ; j < PU->bControlSize; j++)
                dev->PU[p].bmControls |= (uint8_t )((uint8_t *)PU + UVC_PU_CONST_LEN)[j]  << j*8;

            dev->PU[p++].bmVideoStandards = (uint8_t )((uint8_t *)PU + UVC_PU_CONST_LEN)[++j];

        }

        if (et_head->bDescriptorSubType == UVC_VC_EXTENSION_UNIT) {

            struct uvc_extension_unit_descriptor *XU = (struct uvc_extension_unit_descriptor *) (buf_t + off);
            uint8_t controlsize = 0;
            int j = 0;
            buf_len_t -= XU->bLength;
            off += XU->bLength;
            dev->XU[x].id = XU->bUnitID;
            dev->XU[x].bNumControls = XU->bNumControls;
            dev->XU[x].bNrInPins = XU->bNrInPins;
            dev->XU[x].baSourceID = (uint8_t *)malloc(dev->XU[x].bNrInPins);
            if (NULL == dev->XU[x].baSourceID)
                return -ENOMEM;
            memset(dev->XU[x].baSourceID, 0, dev->XU[x].bNrInPins);
            
#ifdef KDP_UVC_DEBUG						
            kmdw_printf("@@ dev->XU[x].baSourceID %p\n", dev->XU[x].baSourceID);
#endif						
            for (j = 0 ; j < dev->XU[x].bNrInPins; j++)
                dev->XU[x].baSourceID[j] = (uint8_t )((uint8_t *)XU + UVC_XU_CONST_LEN)[j];
            controlsize = (uint8_t )((uint8_t *)XU + UVC_XU_CONST_LEN)[j++];

            for (j = 0 ; j < controlsize; j++)
                dev->XU[x].bmControls |=  (uint8_t )((uint8_t *)XU + UVC_XU_CONST_LEN + dev->XU[x].bNrInPins + 1)[j] << j*8;

            x++;
        }

        et_head = (struct uvc_ET_Head_descriptor *)((uint8_t*) buf_t + off);
			//	c = 0; o = 0; s = 0; p = 0; x = 0;
    }

    return 0;
}

static int uvc_parse_vs(struct uvc_device *dev, uint8_t *buffer)
{
    int ret;
    uint32_t off = 0;

    dev->stream = (struct uvc_streaming *) malloc(dev->num_vs_inf * sizeof (struct uvc_streaming));

    if (NULL == dev->stream)
        return -1;
#ifdef KDP_UVC_DEBUG		
    kmdw_printf("@@ dev->stream %p\n", dev->stream);
#endif
    memset(dev->stream, 0, dev->num_vs_inf * sizeof (struct uvc_streaming));    
    for (int i = 0; i  < dev->num_vs_inf; i++) {
 
        dev->stream[i].frame_buf = 0;
        dev->stream[i].running = false;
        if (0 > (ret = uvc_parse_vs_inf(&dev->stream[i], &buffer[off])))
            return ret;
        off += ret;

        off += uvc_parse_vs_data_ep(&dev->stream[i], &buffer[off]);
        dev->stream[i].vs_ctrl_info = (struct ctrl_vs_info *)malloc(sizeof (struct ctrl_vs_info));
        if (NULL == dev->stream[i].vs_ctrl_info)
            return -ENOMEM;
        memset(dev->stream[i].vs_ctrl_info, 0, sizeof (struct ctrl_vs_info));        
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->stream[i].vs_ctrl_info %p\n", dev->stream[i].vs_ctrl_info);
#endif				
        dev->stream[i].vs_ctrl_info->def = (struct uvc_streaming_control_data*) malloc(sizeof (struct uvc_streaming_control_data));
        if (NULL == dev->stream[i].vs_ctrl_info->def)
            return -ENOMEM;
        memset(dev->stream[i].vs_ctrl_info->def, 0, sizeof (struct uvc_streaming_control_data));        
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->stream[i].vs_ctrl_info->def %p\n", dev->stream[i].vs_ctrl_info->def);
#endif				
        dev->stream[i].vs_ctrl_info->curr = (struct uvc_streaming_control_data*) malloc(sizeof (struct uvc_streaming_control_data));
        if (NULL == dev->stream[i].vs_ctrl_info->curr)
            return -ENOMEM;
        memset(dev->stream[i].vs_ctrl_info->curr, 0, sizeof (struct uvc_streaming_control_data));         
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->stream[i].vs_ctrl_info->curr %p\n", dev->stream[i].vs_ctrl_info->curr);
#endif				
        dev->stream[i].vs_ctrl_info->minimum = (struct uvc_streaming_control_data*) malloc(sizeof (struct uvc_streaming_control_data));
        if (NULL == dev->stream[i].vs_ctrl_info->minimum)
            return -ENOMEM;
        memset(dev->stream[i].vs_ctrl_info->minimum, 0, sizeof (struct uvc_streaming_control_data));        
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->stream[i].vs_ctrl_info->minimum %p\n", dev->stream[i].vs_ctrl_info->minimum);
#endif				
        dev->stream[i].vs_ctrl_info->maximum = (struct uvc_streaming_control_data*) malloc(sizeof (struct uvc_streaming_control_data));
        if (NULL == dev->stream[i].vs_ctrl_info->maximum)
            return -ENOMEM;
        memset(dev->stream[i].vs_ctrl_info->maximum, 0, sizeof (struct uvc_streaming_control_data));         
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->stream[i].vs_ctrl_info->maximum %p\n", dev->stream[i].vs_ctrl_info->maximum);
#endif				
        dev->stream[i].vs_ctrl_info->res = (struct uvc_streaming_control_data*) malloc(sizeof (struct uvc_streaming_control_data));
        if (NULL == dev->stream[i].vs_ctrl_info->res)
            return -ENOMEM;
        memset(dev->stream[i].vs_ctrl_info->res, 0, sizeof (struct uvc_streaming_control_data));             
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ dev->stream[i].vs_ctrl_info->res %p\n", dev->stream[i].vs_ctrl_info->res);
#endif				
    }

    dev->curr_stream = &dev->stream[0];
 
    return ret;
}

static int uvc_parse_vc(struct uvc_device *dev, uint8_t *buffer)
{
    struct usb_interface_descriptor  *p_inf = (struct usb_interface_descriptor * )buffer;
    struct uvc_vc_if_header_descriptor *c_inf;
    uint32_t len_ET;
    uint32_t off = 0;
    int ret;

    if (p_inf->bInterfaceClass != UVC_CC_VIDEO)
        return -1;
    if (p_inf->bInterfaceSubClass != UVC_SC_VIDEOCONTROL)
        return -1;
    off += p_inf->bLength;

    dev->vc_inf = p_inf->bInterfaceNumber;
    c_inf = (struct uvc_vc_if_header_descriptor *)&buffer[off];
    len_ET = c_inf->wTotalLength - c_inf->bLength;
    dev->num_vs_inf = c_inf->bInCollection;
    dev->uvc_version = c_inf->bcdUVC;
    dev->clock_frequency = c_inf->dwClockFrequency;
    off += c_inf->bLength;

    if (0 > (ret = uvc_parse_ET(dev, &buffer[off], len_ET)))
        return ret;
    off += len_ET;
    if (0 > (ret = uvc_parse_vc_int_ep(dev, &buffer[off])))
        return ret;
    off += ret;

    return off;
}

static int kdp_uvc_parse_config(uint8_t *buf)
{
    struct usb_config_descriptor *conf = (struct usb_config_descriptor *)buf;
    struct uvc_device *uvc_dev = uvc_video_device;
    int32_t off = 0;
    uint8_t num_inf;
    int ret;
    struct uvc_inf_assoc_descriptor *iad;

    off += conf->bLength;
#ifdef KDP_UVC_DEBUG    
    kmdw_printf("@@ buf = %p\n", buf);
#endif	    
    iad = (struct uvc_inf_assoc_descriptor *)&buf[off];
    if (iad->bDescriptorType != 0xb)
        return -1;
    if (iad->bFunctionClass != UVC_CC_VIDEO)
        return -1;
    if (iad->bFunctionSubClass != UVC_SC_VIDEO_INTERFACE_COLLECTION)
        return -1;
    off += iad->bLength;
#ifdef KDP_UVC_DEBUG    
    kmdw_printf("@@ iad = %p\n", iad);
#endif	    
    num_inf = iad->bInterfaceCount;

  //  memset((void *) uvc_dev, 0, sizeof (struct uvc_device));
    uvc_dev->num_inf = num_inf;

    strncpy(uvc_dev->name, "video0", sizeof uvc_dev->name);

    if (0 > (ret = uvc_parse_vc(uvc_dev, &buf[off]))) {

        return -1;
    }
    off +=ret;
    if (0 > (ret = uvc_parse_vs(uvc_dev, &buf[off]))) {
        uvc_dev->inited = false;			
        return -1;
    }
    uvc_dev->inited = true;
    return 0;
}

static struct kdp_uvc_id uvc_ids[] = {
    {
        .idVendor = 0xc45,
        .idProduct = 0x6366
    },
	{
        .idVendor = 0x46d,
        .idProduct = 0x85c
    },
	{
        .idVendor = 0x41e,
        .idProduct = 0x4095
    },
	{
        .idVendor = 0x45e,
        .idProduct = 0x772
    }, 
	{
        .idVendor = 0x58f,
        .idProduct = 0x806
    },        
	{}
};

struct kdp_uvc_id *usb_uvc_id_lookup(uint16_t  idVendor,  uint16_t  idProduct)
{
    for (int i = 0; i < sizeof uvc_ids / sizeof (struct kdp_uvc_id); i++)
        if ((uvc_ids[i].idVendor == idVendor) && (uvc_ids[i].idProduct == idProduct))
            return &uvc_ids[i];
#ifdef KDP_UVC_DEBUG							
        kmdw_printf("@@ camera idVendor = 0x%x,   idProduct = 0x%x is not in supported list\n", idVendor,  idProduct);
#endif			
	return NULL;
}

usbStatus USBH_UVC_Disconnected(void)
{
    struct uvc_device *uvc_dev = uvc_video_device;
    
    if (true == uvc_dev->curr_stream->running) {
        USBH_UVC_PipeStop_Isoch(uvc_dev->curr_stream->isoch_pipe);
        uvc_dev->curr_stream->running = false;
    }

    isoch_pipe = 0;
    if (NULL == uvc_dev)
        return usbOK;			
    if (true == uvc_dev->opened)
        kdp_uvc_close(0);
    if (false == uvc_dev->inited)
        return usbOK;
    if (NULL != uvc_dev->IT) { 
        for (int i = 0 ; i < uvc_dev->nITs; i++) {
            if (NULL != uvc_dev->IT[i].ct) {            
                for (int j = 0; j < CT_CTRL_NUM; j++) {
                    if (NULL != uvc_dev->IT[i].ct->data) {
#ifdef KDP_UVC_DEBUG							
                    kmdw_printf("@@ free uvc_dev->IT[i].ct->data %p\n", uvc_dev->IT[i].ct->data);
#endif			
                        if (uvc_dev->IT[i].ct->data[j].para != NULL) {
#ifdef KDP_UVC_DEBUG							
                            kmdw_printf("@@ j = 0x%x\n", j);
                            kmdw_printf("@@ uvc_dev->IT[i].ct->data[j].para %p\n", uvc_dev->IT[i].ct->data[j].para);                    
#endif		                    
                            free(uvc_dev->IT[i].ct->data[j].para);
                        }                    
                        free(uvc_dev->IT[i].ct->data);	
                    }                
                }
 
#ifdef KDP_UVC_DEBUG							
                kmdw_printf("@@ free uvc_dev->IT[i].ct %p\n", uvc_dev->IT[i].ct);	
#endif							
                free(uvc_dev->IT[i].ct);
            }							
        }
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ free uvc_dev->IT5 %p\n", uvc_dev->IT);
#endif				
        free(uvc_dev->IT);
    }
    if (NULL != uvc_dev->OT) {
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ free uvc_dev->OT %p\n", uvc_dev->OT);
#endif			
        free(uvc_dev->OT);
    }
    if (NULL != uvc_dev->PU) {
        for (int i = 0 ; i < uvc_dev->nPUs; i++) {
            if (NULL != uvc_dev->PU[i].data) {            
                for (int j = 0; j < PU_CTRL_NUM; j++) {

                    if (uvc_dev->PU[i].data[j].para != NULL) {
#ifdef KDP_UVC_DEBUG										
                        kmdw_printf("@@ free uvc_dev->PU[i].data[j].para %p\n", uvc_dev->PU[i].data[j].para);
#endif									
                        free(uvc_dev->PU[i].data[j].para);
                    }                    
                }
#ifdef KDP_UVC_DEBUG							
                kmdw_printf("@@ free uvc_dev->PU[i].data %p\n", uvc_dev->PU[i].data);
#endif							
                free(uvc_dev->PU[i].data);                
            }                

        }
#ifdef KDP_UVC_DEBUG					
        kmdw_printf("@@ free uvc_dev->PU %p\n", uvc_dev->PU);
#endif					
        free(uvc_dev->PU);
    }
    if (NULL != uvc_dev->XU) {
        for (int i = 0 ; i < uvc_dev->nXUs; i++) {
            if (uvc_dev->XU[i].baSourceID != NULL) {
#ifdef KDP_UVC_DEBUG								
                kmdw_printf("@@ free uvc_dev->XU[i].baSourceID %p\n", uvc_dev->XU[i].baSourceID);
#endif							
                free(uvc_dev->XU[i].baSourceID);
            }
        }
#ifdef KDP_UVC_DEBUG					
        kmdw_printf("@@ free uvc_dev->XU %p\n", uvc_dev->XU);
#endif				
        free(uvc_dev->XU);
    }
    if (NULL != uvc_dev->SU) {
        for (int i = 0; i < uvc_dev->nSUs; i++)
            if (uvc_dev->SU[i].baSourceID != 0)
                free(uvc_dev->SU[i].baSourceID);
        free(uvc_dev->SU);
    }
    for (int i = 0; i  < uvc_dev->num_vs_inf; i++) {
        if (NULL != uvc_dev->stream[i].vs_ctrl_info->curr) {
#ifdef KDP_UVC_DEBUG						
            kmdw_printf("@@ free uvc_dev->stream[i].vs_ctrl_info->curr %p\n", uvc_dev->stream[i].vs_ctrl_info->curr);
#endif					
		    free(uvc_dev->stream[i].vs_ctrl_info->curr);
        }
        if (NULL != uvc_dev->stream[i].vs_ctrl_info->def)	{
#ifdef KDP_UVC_DEBUG						
            kmdw_printf("@@ free uvc_dev->stream[i].vs_ctrl_info->def %p\n", uvc_dev->stream[i].vs_ctrl_info->def);
#endif					
		    free(uvc_dev->stream[i].vs_ctrl_info->def);
        }
        if (NULL != uvc_dev->stream[i].vs_ctrl_info->minimum)	{
#ifdef KDP_UVC_DEBUG						
            kmdw_printf("@@ free uvc_dev->stream[i].vs_ctrl_info->minimum %p\n", uvc_dev->stream[i].vs_ctrl_info->minimum);
#endif					
            free(uvc_dev->stream[i].vs_ctrl_info->minimum);	
        }
        if (NULL != uvc_dev->stream[i].vs_ctrl_info->maximum)	
        {
#ifdef KDP_UVC_DEBUG					
            kmdw_printf("@@ free uvc_dev->stream[i].vs_ctrl_info->maximum %p\n", uvc_dev->stream[i].vs_ctrl_info->maximum);
#endif					
            free(uvc_dev->stream[i].vs_ctrl_info->maximum);
        }
        if (NULL != uvc_dev->stream[i].vs_ctrl_info->res)	
        {
#ifdef KDP_UVC_DEBUG					
            kmdw_printf("@@ free uvc_dev->stream[i].vs_ctrl_info->res %p\n", uvc_dev->stream[i].vs_ctrl_info->res);
#endif					
            free(uvc_dev->stream[i].vs_ctrl_info->res);
        }
        if (NULL != uvc_dev->stream[i].vs_ctrl_info) {
#ifdef KDP_UVC_DEBUG					
            kmdw_printf("@@ free uvc_dev->stream[i].vs_ctrl_info %p\n", uvc_dev->stream[i].vs_ctrl_info);
#endif					
            free(uvc_dev->stream[i].vs_ctrl_info);
        }
        if (NULL != uvc_dev->stream[i].if_alt) {
#ifdef KDP_UVC_DEBUG					
            kmdw_printf("@@ free uvc_dev->stream[i].if_alt %p\n", uvc_dev->stream[i].if_alt);
#endif					
            free(uvc_dev->stream[i].if_alt);
        }
        for (int j = 0; j < uvc_dev->stream[i].nformats; j++) {
            for (int k = 0; k < uvc_dev->stream[i].format[j].nframes; k++) {
                if (NULL != uvc_dev->stream[i].format[j].frame[k].dwFrameInterval) {
#ifdef KDP_UVC_DEBUG									
                    kmdw_printf("@@ free uvc_dev->stream[i].format[j].frame[k].dwFrameInterval %p\n", uvc_dev->stream[i].format[j].frame[k].dwFrameInterval);
#endif									
                    free(uvc_dev->stream[i].format[j].frame[k].dwFrameInterval);
                }
            }
            if (NULL != uvc_dev->stream[i].format[j].frame) {
#ifdef KDP_UVC_DEBUG								
                kmdw_printf("@@ free uvc_dev->stream[i].format[j].frame %p\n", uvc_dev->stream[i].format[j].frame);
#endif							
                free(uvc_dev->stream[i].format[j].frame);	
            }						
        }
        if (NULL != uvc_dev->stream[i].format) {
#ifdef KDP_UVC_DEBUG						
            kmdw_printf("@@ free uvc_dev->stream[i].format %p\n", uvc_dev->stream[i].format);
#endif					
            free(uvc_dev->stream[i].format);
        }
    }
    if (NULL != uvc_dev->stream) {
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ free uvc_dev->stream %p\n", uvc_dev->stream);
#endif			
        free(uvc_dev->stream);
    }
    if (NULL != uvc_dev->int_ep) {
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ free uvc_dev->int_ep %p\n", uvc_dev->int_ep);
#endif			
        free(uvc_dev->int_ep);	
    }			
    memset(uvc_dev, 0, sizeof (struct uvc_device));
		
	//	osThreadFlagsSet(tid_to_notify, FLAGS_YOLO_STOP_EVT);
    osThreadFlagsSet(tid_to_notify, FLAGS_UVC_CAMERA_INIT_FAILED_EVT);
#ifdef KDP_UVC_DEBUG		
    kmdw_printf("%s: USBH_UVC_Disconnected() OK\n", __func__);
#endif		
    return usbOK;
}

static int kdp_uvc_init(void)
{
    if (false == uvc_video_device->inited)
        return -1;
    if (0 > uvc_init_device_ctrl(uvc_video_device))
        return -1;
    if (0 > uvc_video_init(uvc_video_device))
        return -1;
    if (KMDW_STATUS_ERROR == kdp_uvc_buffer_init(KDP_CAM_0, NULL, NULL))
        return -1;
    uvc_video_device->inited = true;
    return 0;
}

uint8_t USBH_UVC_Configure(uint8_t device, const USB_DEVICE_DESCRIPTOR *ptr_dev_desc, const USB_CONFIGURATION_DESCRIPTOR *ptr_cfg_desc)
{
    struct uvc_device *uvc_dev = uvc_video_device;
    struct kdp_uvc_id *id = usb_uvc_id_lookup(ptr_dev_desc->idVendor, ptr_dev_desc->idProduct);
    memset((void *) uvc_dev, 0, sizeof (struct uvc_device));
	uvc_dev->id = id;
    kmdw_printf("vendor id is 0x%x,  product ID is 0x%x\n", uvc_dev->id->idVendor, uvc_dev->id->idProduct);
    return kdp_uvc_parse_config((uint8_t *)ptr_cfg_desc);
}

usbStatus USBH_UVC_Initialize(uint8_t instance)
{
   
    if (0 > kdp_uvc_init()) {
        osThreadFlagsSet(tid_to_notify, FLAGS_UVC_CAMERA_INIT_FAILED_EVT);
        while (1) {};
    }

    osThreadFlagsSet(tid_to_notify, FLAGS_UVC_CAMERA_INIT_DONE_EVT);

    return usbOK;
}

void kmdw_cam_uvc_init(void)
{
    usbStatus usb_status;

    tid_to_notify = osThreadGetId();

    kmdw_camera_controller_register(KDP_CAM_0, &kdp_uvc_ops);

    // init USB host through MDK middleware
    usb_status = USBH_Initialize(0U);

    if (usb_status != usbOK)
        kmdw_printf("%s: USBH_Initialize() failed %d\n", __func__, usb_status);
    else
        kmdw_printf("%s: USBH_Initialize() OK\n", __func__);
}
