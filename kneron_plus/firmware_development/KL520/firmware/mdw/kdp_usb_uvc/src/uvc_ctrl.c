/*
 *      uvc_ctrl.c    
 *
 * Copyright (C) 2019 - 2020 Kneron, Inc. All rights reserved.
 *
 */

#include "errno.h"
#include "uvc_ctrl.h"
#include "uvc_video.h"
#include "uvc_utils.h"
#include "uvc_internal_api.h"
#include "kmdw_memory.h"
#include "kmdw_uvc.h"
#include "kmdw_console.h"
#include "uvc_camera.h"

#ifdef  KDP_UVC
#ifdef KDP_UVC_DEBUG
#define uvc_msg(fmt, ...) MSG(LOG_ERROR, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define uvc_msg(fmt, ...)
#endif

struct uvc_req {
    USB_SETUP_PACKET req;
    uint8_t *data;
} vc_req = {0};

static int uvc_create_ctl_req(struct ctrl_info *ctrl, uint8_t inf_ep,  uint8_t req_num)
{
    memset(&vc_req, 0, sizeof (struct uvc_req));
    if ((((ctrl->cid >> ET_POS) & 0xF) != UVC_DEV_INF) &&
        (((ctrl->cid >> ET_POS) & 0xF) != UVC_ET_PU) &&
        (((ctrl->cid >> ET_POS) & 0xF) != UVC_ET_EU) &&
        (((ctrl->cid >> ET_POS) & 0xF) != UVC_ET_XU) &&
		    (((ctrl->cid >> ET_POS) & 0xF) != UVC_ET_CT))
        return -1;
    if ((req_num == UVC_SET_CUR)||(req_num == UVC_SET_CUR_ALL)) {
        vc_req.req.bmRequestType.Recipient = USB_REQUEST_TO_INTERFACE;
        vc_req.req.bmRequestType.Dir = USB_REQUEST_HOST_TO_DEVICE;
        vc_req.req.bmRequestType.Type = USB_REQUEST_CLASS;
    } else if ((req_num == UVC_GET_CUR)||(req_num == UVC_GET_MIN) ||
               (req_num == UVC_GET_MAX)|| (req_num == UVC_GET_RES)||
               (req_num == UVC_GET_INFO)||(req_num == UVC_GET_DEF)||
               (req_num == UVC_GET_CUR_ALL)||(req_num == UVC_GET_MIN_ALL) ||
               (req_num == UVC_GET_MAX_ALL)||(req_num == UVC_GET_RES_ALL) ||
               (req_num == UVC_GET_DEF_ALL)) {
        vc_req.req.bmRequestType.Recipient = USB_REQUEST_TO_INTERFACE;
        vc_req.req.bmRequestType.Dir = USB_REQUEST_DEVICE_TO_HOST;
        vc_req.req.bmRequestType.Type = USB_REQUEST_CLASS;
    } else
        return -1;

    if ((req_num == UVC_SET_CUR_ALL) || (req_num == UVC_GET_CUR_ALL) ||
        (req_num == UVC_GET_MAX_ALL) || (req_num == UVC_GET_DEF_ALL) ||
        (req_num == UVC_GET_RES_ALL)) {
        vc_req.req.wIndex = ctrl->eid << 8;
        vc_req.req.wValue = 0;
    } else {
        vc_req.req.wIndex = inf_ep | (ctrl->eid << 8);
        vc_req.req.wValue = ctrl->cs << 8;
        vc_req.req.wLength = ctrl->para_size;
        if ((req_num == UVC_SET_CUR ) || (req_num == UVC_GET_CUR) ||
            (req_num == UVC_GET_MIN ) || (req_num == UVC_GET_MAX) ||
            (req_num == UVC_GET_RES ))
            vc_req.data = &ctrl->para[(0xF & req_num)* ctrl->para_size];
        else if (req_num == UVC_GET_DEF) {
        //    kmdw_printf("@@ ctrl->para[0] %p\n", &ctrl->para[0]);            
            vc_req.data = &ctrl->para[0];}
        else if (req_num == UVC_GET_LEN) {
            vc_req.data = &ctrl->len;
            vc_req.req.wLength = 1;
        } else if (req_num == UVC_GET_INFO) {
            vc_req.data = &ctrl->caps;
            vc_req.req.wLength = 1;
        }
    }
    vc_req.req.bRequest= req_num;

    return 0;
}

static int uvc_query_ctrl(struct uvc_device *dev, int timeout)
{
    if (0 != USBH_ControlTransfer(0,&vc_req.req, (uint8_t *) vc_req.data, vc_req.req.wLength)) {
        return -1;
    }
    return 0;
}


static struct ctrl_info *uvc_find_control(struct uvc_device *udev, uint32_t cid)
{
    if ((UVC_CID_DEVICE_POWER_MODE) == cid) {

        return udev->inf_ctl;
    }
    else if ((UVC_CID_CAMERA_CLASS_BASE) == (cid & 0xF0000000)) {
        for (int i = 0; i < udev->nITs; i++){

            if (((cid & 0xFFFFFFF) & udev->IT->ct->bmControls) != 0) {
                int j;
                for (j = 0; j < ET_POS; j++) {
                    if ((cid & 1) == 0x1)
                        break;
                    cid >>= 1;
                }
                return &udev->IT->ct->data[j];
            }
        }
    } else if (UVC_CID_PU_CLASS_BASE == (cid & 0xF0000000))  {
        for (int i = 0; i < udev->nPUs; i++)

            if (((cid & 0xFFFFFFF) & udev->PU->bmControls) != 0) {
                int j;
                for (j = 0; j < ET_POS; j++) {
                    if ((cid & 1) == 0x1)
                        break;
                    cid >>= 1;
                }
                return &udev->PU->data[j];
            }

    } else if (UVC_CID_XU_CLASS_BASE == (cid >> ET_POS))  {
        for ( int i = 0; i < udev->nXUs; i++)

            if (((cid & 0xFFF) & udev->XU->bmControls) != 0) {
                int j;
                for (j = 0; j < ET_POS; j++) {
                    if ((cid & 1) == 0x1)
                        break;
                    cid >>= 1;
                }
                return &udev->XU->data[j];
            }
    }
//    kmdw_printf("no control found \n" );		
    return NULL;
}


int uvc_send_ctrl_req(struct uvc_device *dev, struct ctrl_info *ctrl, uint8_t req)
{
    int ret;
    uint8_t flag_t;
    
    if (req == UVC_SET_CUR)
        flag_t = UVC_CTRL_FLAG_SET_CUR;
    else if (req == UVC_GET_CUR )
        flag_t = UVC_CTRL_FLAG_GET_CUR;
    else if (req == UVC_GET_MIN)
        flag_t = UVC_CTRL_FLAG_GET_MIN;       
    else if (req == UVC_GET_MAX)
        flag_t = UVC_CTRL_FLAG_GET_MAX; 
    else if (req == UVC_GET_RES)
        flag_t = UVC_CTRL_FLAG_GET_RES; 
    else if (req == UVC_GET_LEN)
        flag_t = UVC_CTRL_FLAG_GET_LEN; 
    else if (req == UVC_GET_INFO)
        flag_t = UVC_CTRL_FLAG_GET_INFO;
    else if (req == UVC_GET_DEF)
        flag_t = UVC_CTRL_FLAG_GET_DEF;    
 
    if (ctrl->ctl_flag & flag_t) {
        if (0 > (ret = uvc_create_ctl_req(ctrl, dev->vc_inf, req)))
            return ret;
        return uvc_query_ctrl(dev, UVC_CTRL_CONTROL_TIMEOUT);
    }
    return -1;
}


static int uvc_ctrl_cache(struct ctrl_info *ctrl_data, struct uvc_device *udev)
{
    int ret;

    if (true == ctrl_data->supported) {
        if ((udev->id->idVendor == 0x058F) && (udev->id->idProduct == 0x0806)) {
        
        }
        else {
            if (ctrl_data->ctl_flag & UVC_CTRL_FLAG_GET_DEF) {
                uvc_create_ctl_req(ctrl_data, udev->vc_inf, UVC_GET_DEF);
                if (0 > (ret = uvc_query_ctrl(udev, UVC_CTRL_CONTROL_TIMEOUT)))
                    return ret;
            }
        }
        if (ctrl_data->ctl_flag & UVC_CTRL_FLAG_GET_MIN) {
            uvc_create_ctl_req(ctrl_data, udev->vc_inf, UVC_GET_MIN);
            if (0 > (ret = uvc_query_ctrl(udev, UVC_CTRL_CONTROL_TIMEOUT)))
                return ret;
        }
        if (ctrl_data->ctl_flag & UVC_CTRL_FLAG_GET_MAX) {
            uvc_create_ctl_req(ctrl_data, udev->vc_inf,  UVC_GET_MAX);
            if (0 > (ret = uvc_query_ctrl(udev, UVC_CTRL_CONTROL_TIMEOUT)))
                return ret;
        }
        if (ctrl_data->ctl_flag & UVC_CTRL_FLAG_GET_RES) {
            uvc_create_ctl_req(ctrl_data, udev->vc_inf,  UVC_GET_RES);
            if (0 > (ret = uvc_query_ctrl(udev, UVC_CTRL_CONTROL_TIMEOUT)))
                return ret;
        }
        if (ctrl_data->ctl_flag & UVC_CTRL_FLAG_GET_INFO) {
            uvc_create_ctl_req(ctrl_data, udev->vc_inf,  UVC_GET_INFO);
            if (0 > (ret = uvc_query_ctrl(udev, UVC_CTRL_CONTROL_TIMEOUT)))
                return ret;
        }
        if (ctrl_data->ctl_flag & UVC_CTRL_FLAG_GET_LEN) {
            uvc_create_ctl_req(ctrl_data, udev->vc_inf,  UVC_GET_LEN);
            if (0 > (ret = uvc_query_ctrl(udev,  UVC_CTRL_CONTROL_TIMEOUT)))
                return ret;
        }
        if (ctrl_data->ctl_flag & UVC_CTRL_FLAG_GET_CUR) {
            uvc_create_ctl_req(ctrl_data, udev->vc_inf,  UVC_GET_CUR);
            if (0 > (ret = uvc_query_ctrl(udev, UVC_CTRL_CONTROL_TIMEOUT)))
                return ret;
        }
        ctrl_data->cached = true;
    }
    return 0;
}

static int set_pu_ctrl_flag(struct ctrl_info *info)
{

    if (info->cid & BRIGHTNESS) {
        info->ctl_flag = BRIGHTNESS_CTL_FLAG;
        info->cs = UVC_PU_BRIGHTNESS_CONTROL;
        info->para_size = BRIGHTNESS_LEN;
        info->para = (uint8_t *) malloc(BRIGHTNESS_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, BRIGHTNESS_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & CONTRAST) {
        info->cs = UVC_PU_CONTRAST_CONTROL;
        info->ctl_flag = CONTRAST_CTL_FLAG;
        info->para_size = CONTRAST_LEN;
        info->para = (uint8_t *) malloc(CONTRAST_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, CONTRAST_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;
    } else if (info->cid & HUE) {
        info->cs = UVC_PU_HUE_CONTROL;
        info->ctl_flag = HUE_CTL_FLAG;
        info->para_size = HUE_LEN;
        info->para = (uint8_t *) malloc(HUE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG				
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, HUE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & SATURATION) {
        info->cs = UVC_PU_SATURATION_CONTROL;
        info->ctl_flag = SATURATION_CTL_FLAG;
        info->para_size = SATURATION_LEN;
        info->para = (uint8_t *) malloc(SATURATION_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG		
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, SATURATION_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & SHARPNESS) {
        info->cs = UVC_PU_SHARPNESS_CONTROL;
        info->ctl_flag = SHARPNESS_CTL_FLAG;
        info->para_size = SHARPNESS_LEN;
        info->para = (uint8_t *) malloc(SHARPNESS_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, SHARPNESS_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & GAMMA) {
        info->cs = UVC_PU_GAMMA_CONTROL;
        info->ctl_flag = GAMMA_CTL_FLAG;
        info->para_size = GAMMA_LEN;
        info->para = (uint8_t *) malloc(GAMMA_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, GAMMA_LEN * PARAM_ARRAY_SIZE); 
        else
            return -1;        
    } else if (info->cid & WHITE_BALANCE_TEMPERATURE) {
        info->cs = UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL;
        info->ctl_flag =  WHITE_BALANCE_TEMPERATURE_CTL_FLAG;
        info->para_size = WHITE_BALANCE_TEMPERATURE_LEN;
        info->para = (uint8_t *) malloc(WHITE_BALANCE_TEMPERATURE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, WHITE_BALANCE_TEMPERATURE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & WHITE_BALANCE_COMPONENT) {
        info->cs = UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL;
        info->ctl_flag = WHITE_BALANCE_COMPONENT_CTL_FLAG;
        info->para_size = WHITE_BALANCE_COMPONENT_LEN;
        info->para = (uint8_t *) malloc(WHITE_BALANCE_COMPONENT_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, WHITE_BALANCE_COMPONENT_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & BACKLIGHT_COMPENSATION) {
        info->cs = UVC_PU_BACKLIGHT_COMPENSATION_CONTROL;
        info->ctl_flag = BACKLIGHT_COMPENSATION_CTL_FLAG;
        info->para_size = BACKLIGHT_COMPENSATION_LEN;
        info->para = (uint8_t *) malloc(BACKLIGHT_COMPENSATION_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, BACKLIGHT_COMPENSATION_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & GAIN) {
        info->cs = UVC_PU_GAIN_CONTROL;
        info->ctl_flag = GAIN_CTL_FLAG;
        info->para_size = GAIN_LEN;
        info->para = (uint8_t *) malloc(GAIN_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, GAIN_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & POWER_LINE_FREQUENCY)	{
        info->cs = UVC_PU_POWER_LINE_FREQUENCY_CONTROL;
        info->ctl_flag = POWER_LINE_FREQUENCY_CTL_FLAG;
        info->para_size = POWER_LINE_FREQUENCY_LEN;
        info->para = (uint8_t *) malloc(POWER_LINE_FREQUENCY_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, POWER_LINE_FREQUENCY_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;           
    } else if (info->cid & HUE_AUTO) {
        info->cs = UVC_PU_HUE_AUTO_CONTROL;
        info->ctl_flag = HUE_AUTO_CTL_FLAG;
        info->para_size = HUE_AUTO_LEN;
        info->para = (uint8_t *) malloc(HUE_AUTO_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)        
            memset(info->para, 0, HUE_AUTO_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & WHITE_BALANCE_TEMPERATURE_AUTO)	{
        info->cs = UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL;
        info->ctl_flag = WHITE_BALANCE_TEMPERATURE_AUTO_CTL_FLAG;
        info->para_size = WHITE_BALANCE_TEMPERATURE_AUTO_LEN;
        info->para = (uint8_t *) malloc(WHITE_BALANCE_TEMPERATURE_AUTO_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, WHITE_BALANCE_TEMPERATURE_AUTO_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;         
    } else if (info->cid & WHITE_BALANCE_COMPONENT_AUTO) {
        info->cs = UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL;
        info->ctl_flag = WHITE_BALANCE_COMPONENT_AUTO_CTL_FLAG;
        info->para_size = WHITE_BALANCE_COMPONENT_AUTO_LEN;
        info->para = (uint8_t *) malloc(WHITE_BALANCE_COMPONENT_AUTO_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, WHITE_BALANCE_COMPONENT_AUTO_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & DIGITAL_MULTIPLIER)	{
        info->cs = UVC_PU_DIGITAL_MULTIPLIER_CONTROL;
        info->ctl_flag = DIGITAL_MULTIPLIER_CTL_FLAG;
        info->para_size = DIGITAL_MULTIPLIER_LEN;
        info->para = (uint8_t *) malloc(DIGITAL_MULTIPLIER_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, DIGITAL_MULTIPLIER_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & DIGITAL_MULTIPLIER_LIMIT)	{
        info->cs =UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL;
        info->ctl_flag = DIGITAL_MULTIPLIER_LIMIT_CTL_FLAG;
        info->para_size = DIGITAL_MULTIPLIER_LIMIT_LEN;
        info->para = (uint8_t *) malloc(DIGITAL_MULTIPLIER_LIMIT_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, DIGITAL_MULTIPLIER_LIMIT_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & ANALOG_VIDEO_STANDARD)	{
        info->cs = UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL;
        info->ctl_flag = ANALOG_VIDEO_STANDARD_CTL_FLAG;
        info->para_size = ANALOG_VIDEO_STANDARD_LEN;
        info->para = (uint8_t *) malloc(ANALOG_VIDEO_STANDARD_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, ANALOG_VIDEO_STANDARD_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & ANALOG_VIDEO_LOCK_STATUS)	{
        info->cs = UVC_PU_ANALOG_LOCK_STATUS_CONTROL;
        info->ctl_flag = ANALOG_VIDEO_LOCK_STATUS_CTL_FLAG;
        info->para_size = ANALOG_VIDEO_LOCK_STATUS_LEN;
        info->para = (uint8_t *) malloc(ANALOG_VIDEO_LOCK_STATUS_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, ANALOG_VIDEO_LOCK_STATUS_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & CONTRAST_AUTO) {
        info->cs = UVC_PU_CONTRAST_AUTO_CONTROL;
        info->ctl_flag = CONTRAST_AUTO_CTL_FLAG;
        info->para_size = CONTRAST_AUTO_LEN;
        info->para = (uint8_t *) malloc(CONTRAST_AUTO_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, CONTRAST_AUTO_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    }
    return 0;
}

static int set_ct_ctrl_flag(struct ctrl_info *info)
{
    if (info->cid & SCANNING_MODE) {
        info->ctl_flag = SCANNING_MODE_CTL_FLAG;
        info->cs = UVC_CT_SCANNING_MODE_CONTROL;
        info->para_size = SCANNING_MODE_LEN;
        info->para = (uint8_t *) malloc(SCANNING_MODE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, SCANNING_MODE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;            
    } else if (info->cid & AUTO_EXPOSURE_MODE) {
        info->ctl_flag = AUTO_EXPOSURE_MODE_CTL_FLAG;
        info->cs = UVC_CT_AE_MODE_CONTROL;
        info->para_size = AUTO_EXPOSURE_MODE_LEN;
        info->para = (uint8_t *) malloc(AUTO_EXPOSURE_MODE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, AUTO_EXPOSURE_MODE_LEN * PARAM_ARRAY_SIZE);        
    } else if (info->cid & AUTO_EXPOSURE_PRIORITY)	{
        info->ctl_flag = AUTO_EXPOSURE_PRIORITY_CTL_FLAG;
        info->cs = UVC_CT_AE_PRIORITY_CONTROL;
        info->para_size = AUTO_EXPOSURE_PRIORITY_LEN;
        info->para = (uint8_t *) malloc(AUTO_EXPOSURE_PRIORITY_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, AUTO_EXPOSURE_PRIORITY_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & EXPOSURE_TIME_ABSOLUTE) {
        info->ctl_flag = EXPOSURE_TIME_ABSOLUTE_CTL_FLAG;
        info->cs = UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL;
        info->para_size = EXPOSURE_TIME_ABSOLUTE_LEN;
        info->para = (uint8_t *) malloc(EXPOSURE_TIME_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, EXPOSURE_TIME_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & EXPOSURE_TIME_RELATIVE) {
        info->ctl_flag = EXPOSURE_TIME_RELATIVE_CTL_FLAG;
        info->cs = UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL;
        info->para_size = EXPOSURE_TIME_RELATIVE_LEN;
        info->para = (uint8_t *) malloc(EXPOSURE_TIME_RELATIVE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, EXPOSURE_TIME_RELATIVE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & FOCUS_ABSOLUTE) {
        info->ctl_flag = FOCUS_ABSOLUTE_CTL_FLAG;
        info->cs = UVC_CT_FOCUS_ABSOLUTE_CONTROL;
        info->para_size = FOCUS_ABSOLUTE_LEN;
        info->para = (uint8_t *) malloc(FOCUS_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, FOCUS_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;         
    } else if (info->cid & FOCUS_RELATIVE) {
        info->ctl_flag = FOCUS_RELATIVE_CTL_FLAG;
        info->cs = UVC_CT_FOCUS_RELATIVE_CONTROL;
        info->para_size = FOCUS_RELATIVE_LEN;
        info->para = (uint8_t *) malloc(FOCUS_RELATIVE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, FOCUS_RELATIVE_LEN * PARAM_ARRAY_SIZE); 
        else
            return -1;        
    } else if (info->cid & IRIS_ABSOLUTE) {
        info->ctl_flag = IRIS_ABSOLUTE_CTL_FLAG;
        info->cs = UVC_CT_IRIS_ABSOLUTE_CONTROL;
        info->para_size = IRIS_ABSOLUTE_LEN;
        info->para = (uint8_t *) malloc(IRIS_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, IRIS_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & IRIS_RELATIVE) {
        info->ctl_flag = IRIS_RELATIVE_CTL_FLAG;
        info->cs =UVC_CT_IRIS_RELATIVE_CONTROL;
        info->para_size = IRIS_RELATIVE_LEN;
        info->para = (uint8_t *) malloc(IRIS_RELATIVE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, IRIS_RELATIVE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & ZOOM_ABSOLUTE) {
        info->ctl_flag = ZOOM_ABSOLUTE_CTL_FLAG;
        info->cs = UVC_CT_ZOOM_ABSOLUTE_CONTROL;
        info->para_size = ZOOM_ABSOLUTE_LEN;
        info->para = (uint8_t *) malloc(ZOOM_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, ZOOM_ABSOLUTE_LEN * PARAM_ARRAY_SIZE); 
        else
            return -1;        
    } else if (info->cid & ZOOM_RELATIVE) {
        info->ctl_flag = ZOOM_RELATIVE_CTL_FLAG;
        info->cs = UVC_CT_ZOOM_RELATIVE_CONTROL;
        info->para_size = ZOOM_RELATIVE_LEN;
        info->para = (uint8_t *) malloc(ZOOM_RELATIVE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, ZOOM_RELATIVE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & PANTILT_ABSOLUTE) {
        info->ctl_flag = PANTILT_ABSOLUTE_CTL_FLAG;
        info->cs = UVC_CT_PANTILT_ABSOLUTE_CONTROL;
        info->para_size = PANTILT_ABSOLUTE_LEN;
        info->para = (uint8_t *) malloc(PANTILT_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, PANTILT_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & PANTILT_RELATIVE) {
        info->ctl_flag = PANTILT_RELATIVE_CTL_FLAG;
        info->cs = UVC_CT_PANTILT_RELATIVE_CONTROL;
        info->para_size = PANTILT_RELATIVE_LEN;
        info->para = (uint8_t *) malloc(PANTILT_RELATIVE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, PANTILT_RELATIVE_LEN * PARAM_ARRAY_SIZE); 
        else
            return -1;        
    } else if (info->cid & ROLL_ABSOLUTE) {
        info->ctl_flag = ROLL_ABSOLUTE_CTL_FLAG;
        info->cs = UVC_CT_ROLL_ABSOLUTE_CONTROL;
        info->para_size = ROLL_ABSOLUTE_LEN;
        info->para = (uint8_t *) malloc(ROLL_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, ROLL_ABSOLUTE_LEN * PARAM_ARRAY_SIZE);
         else
            return -1;       
    } else if (info->cid & ROLL_RELATIVE) {
        info->ctl_flag = ROLL_RELATIVE_CTL_FLAG;
        info->cs = UVC_CT_ROLL_RELATIVE_CONTROL;
        info->para_size = ROLL_RELATIVE_LEN;
        info->para = (uint8_t *) malloc(ROLL_RELATIVE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, ROLL_RELATIVE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & FOCUS_AUTO) {
        info->ctl_flag = FOCUS_AUTO_CTL_FLAG;
        info->cs = UVC_CT_FOCUS_AUTO_CONTROL;
        info->para_size = FOCUS_AUTO_LEN;
        info->para = (uint8_t *) malloc(FOCUS_AUTO_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, FOCUS_AUTO_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & PRIVACY) {
        info->ctl_flag = PRIVACY_CTL_FLAG;
        info->cs = UVC_CT_PRIVACY_CONTROL;
        info->para_size = PRIVACY_LEN;
        info->para = (uint8_t *) malloc(PRIVACY_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, PRIVACY_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & FOCUS_SIMPLE) {
        info->ctl_flag = FOCUS_SIMPLE_CTL_FLAG;
        info->cs = UVC_CT_FOCUS_SIMPLE_CONTROL;
        info->para_size = FOCUS_SIMPLE_LEN;
        info->para = (uint8_t *) malloc(FOCUS_SIMPLE_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, FOCUS_SIMPLE_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & WINDOW) {
        info->ctl_flag = WINDOW_CTL_FLAG;
        info->cs = UVC_CT_WINDOW_CONTROL;
        info->para_size = WINDOW_LEN;
        info->para = (uint8_t *) malloc(WINDOW_LEN * PARAM_ARRAY_SIZE);
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif
        if (0 != info->para)
            memset(info->para, 0, WINDOW_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
    } else if (info->cid & REGION_OF_INTEREST) {
        info->ctl_flag = REGION_OF_INTEREST_CTL_FLAG;
        info->cs = UVC_CT_REGION_OF_INTEREST_CONTROL;
        info->para_size = REGION_OF_INTEREST_LEN;
        info->para = (uint8_t *) malloc(REGION_OF_INTEREST_LEN * PARAM_ARRAY_SIZE);
        if (0 != info->para)
            memset(info->para, 0, REGION_OF_INTEREST_LEN * PARAM_ARRAY_SIZE);
        else
            return -1;        
#ifdef KDP_UVC_DEBUG			
        kmdw_printf("@@ info->para %p\n", info->para);
#endif			
    }
    return 0;
}

static void uvc_memory_copy(uint8_t *dst, uint8_t *src, uint8_t len)
{
    for (int i = 0; i < len; i++)
        dst[i] = src[i];
    return;
}

int uvc_send_ctl(struct uvc_device *udev, uint32_t cid, uint16_t req, uint8_t *para, uint8_t len)
{
    struct ctrl_info *ctrl;
    
    if (NULL == (ctrl = uvc_find_control(udev, cid)))
        return -EINVAL;

    if (req & 0x80) {
        if (0 > uvc_send_ctrl_req(udev, ctrl, req))
            return -1;
        if (req == UVC_GET_CUR) {
            uvc_memory_copy(para, &ctrl->para[UVC_CUR * len], len);
            return 0;
        } else if (req == UVC_GET_MIN) {
            uvc_memory_copy(para, &ctrl->para[UVC_MIN * len], len);
            return 0;
        } else if (req == UVC_GET_MAX) {
            uvc_memory_copy(para, &ctrl->para[UVC_MAX * len], len);
            return 0;  
        } else if (req == UVC_GET_RES) {
            uvc_memory_copy(para, &ctrl->para[UVC_RES * len], len);
            return 0;
        } else if (req == UVC_GET_LEN) {
           *para = ctrl->len;
            return 0;
        } else if (req == UVC_GET_INFO) {
            *para = ctrl->caps;
            return 0; 
        } else if (req == UVC_GET_DEF) {
            uvc_memory_copy(para, &ctrl->para[UVC_DEF * len], len);
            return 0;
        } else
            return -1;
    } else {
        if (req == UVC_SET_CUR) {
            if (!(ctrl->ctl_flag & UVC_CTRL_FLAG_GET_RES))
            {
                if ((ctrl->cid == (UVC_CID_FOCUS_AUTO)) ||
                    (ctrl->cid == (UVC_CID_PRIVACY)) ||
                    (ctrl->cid == (UVC_CID_SCANNING_MODE))) {
                    ctrl->para[UVC_CUR] = (*para == 0) ? 0 : 1;
                } else if ((ctrl->cid == (UVC_CID_EXPOSURE_TIME_RELATIVE))
                            || (ctrl->cid == (UVC_CID_IRIS_RELATIVE))) {
                        ctrl->para[UVC_CUR] = (*para == 0) ? 0: ((*para != 0xFF) ? 1: *para);
                } else if ((ctrl->cid == (UVC_CID_CONTRAST_AUTO))
                            || (ctrl->cid == (UVC_CID_HUE_AUTO))
                            || (ctrl->cid == (UVC_CID_WHITE_BALANCE_TEMPERATURE_AUTO))
                            || (ctrl->cid == (UVC_CID_WHITE_BALANCE_COMPONENT_AUTO))){
                        ctrl->para[UVC_CUR] = *para; 
                } else if (ctrl->cid == (UVC_CID_AUTO_EXPOSURE_PRIORITY)) {
                    struct ctrl_info *ctr_t;  
                    if (NULL == (ctr_t = uvc_find_control(udev, UVC_CID_AUTO_EXPOSURE_MODE)))
                          return -EINVAL;                    
                    if (ctr_t->para[UVC_CUR] & 0x6) {
                          ctrl->para[UVC_CUR] = (*para == 0) ? 0 : 1;
                    }
                    else 
                         return  -EINVAL;                       
                } else if (ctrl->cid == (UVC_CID_FOCUS_SIMPLE)) {
                            if (*para >= 4)
                                *para = 0;
                            ctrl->para[UVC_CUR] = *para; 
                } else if ((ctrl->cid == (UVC_CID_WINDOW)) || (ctrl->cid == (UVC_CID_REGION_OF_INTEREST))) {
                            for (int i = 0; i < len; i++)
                                uvc_memory_copy(&ctrl->para[UVC_CUR * len], para, len);
                } else if (ctrl->cid == (UVC_CID_POWER_LINE_FREQUENCY)) {
                            if (*para >= 3)
                                *para = 3;
                            ctrl->para[UVC_CUR] = *para; 
                }
            }
            else {
                if (ctrl->cid  == (UVC_CID_ZOOM_RELATIVE)) {
                    struct ct_zoomr_data *min = (struct ct_zoomr_data *)&ctrl->para[UVC_MIN *len];
                    struct ct_zoomr_data *max = (struct ct_zoomr_data *)&ctrl->para[UVC_MAX *len];
                    struct ct_zoomr_data *step = (struct ct_zoomr_data *)&ctrl->para[UVC_RES *len]; 
                    struct ct_zoomr_data *cur = (struct ct_zoomr_data *)&ctrl->para[UVC_CUR *len];
                    struct ct_zoomr_data *zr_para = (struct ct_zoomr_data *) para;
                    uint8_t zr_step;
                    
                    if (step->bSpeed == 0)
                        zr_step = 1;
                    
                    cur->bDigitalZoom = (zr_para->bDigitalZoom == 0) ? 0: 1;
                    cur->bZoom = (zr_para->bZoom == 0) ? 0: ((zr_para->bZoom != 0xFF) ? 1: zr_para->bZoom);
                    cur->bSpeed = cur->bSpeed + ((uint32_t)(zr_para->bSpeed - min->bSpeed) + (zr_step >> 0x1)) / zr_step * zr_step;
                    cur->bSpeed = clamp(cur->bSpeed, min->bSpeed, max->bSpeed);

                } else if (ctrl->cid  == (UVC_CID_AUTO_EXPOSURE_MODE)) {
                            if ((*para == 0) || ((*para & (*para - 1)) != 0) || (*para > 8) 
                                        || (!((*para == 0) & ctrl->para[UVC_RES])))
                            {
                                *para = 0x1; 
                            }
                            ctrl->para[UVC_CUR]= *para;
                } else if (ctrl->cid  == (UVC_CID_EXPOSURE_TIME_ABSOLUTE)) {
                            struct ctrl_info *ctr_t;  
                            if (NULL == (ctr_t = uvc_find_control(udev, UVC_CID_AUTO_EXPOSURE_MODE)))
                                return -EINVAL;                    
                            if (ctr_t->para[UVC_CUR] & 0x6) {
                                uint32_t *min = (uint32_t *)&ctrl->para[UVC_MIN *len];
                                uint32_t *max = (uint32_t *)&ctrl->para[UVC_MAX *len];
                                uint32_t *step = (uint32_t *)&ctrl->para[UVC_RES *len]; 
                                uint32_t *cur = (uint32_t *)&ctrl->para[UVC_CUR *len];
                                uint32_t *zr_para = (uint32_t *) para;
                                uint8_t zr_step;
 //kmdw_printf("min  is 0x%x,  max  is 0x%x, step  is 0x%x, cur is 0x%x, param is 0x%x \n",*min, *max, *step, *cur, *zr_para);	                               
		                        if (*step == 0)
                                    zr_step = 1;														
                                *cur = *zr_para;
                                *cur = *min + ((uint32_t)(*zr_para - *min) + (zr_step >> 0x1)) / zr_step * zr_step;
                                *cur = clamp(*cur, *min, *max);
                            } 
                            else
                                return -EINVAL;                               
                } else if (ctrl->cid  == (UVC_CID_FOCUS_ABSOLUTE)) {
                            struct ctrl_info *ctr_t;  
                            if (NULL == (ctr_t = uvc_find_control(udev, UVC_CID_FOCUS_AUTO)))
                                return -EINVAL;
                            if (ctr_t->para[UVC_CUR] & 0x1) 
                                return -EINVAL;                                
                            uint16_t *min = (uint16_t *)&ctrl->para[UVC_MIN *len];
                            uint16_t *max = (uint16_t *)&ctrl->para[UVC_MAX *len];
                            uint16_t *step = (uint16_t *)&ctrl->para[UVC_RES *len]; 
                            uint16_t *cur = (uint16_t *)&ctrl->para[UVC_CUR *len];
                            uint16_t *zr_para = (uint16_t *) para;
                            uint16_t fa_step = *step;											 
                            if (fa_step == 0)
                                fa_step = 1;	
                            *cur = *min + ((uint16_t)(*zr_para - *min) + (fa_step >> 0x1)) / fa_step * fa_step;
                            *cur = clamp(*cur, *min, *max);
                } else if (ctrl->cid == (UVC_CID_FOCUS_RELATIVE)) {
                            struct ct_focus_r_data *min = (struct ct_focus_r_data *)&ctrl->para[UVC_MIN *len];
                            struct ct_focus_r_data *max = (struct ct_focus_r_data *)&ctrl->para[UVC_MAX *len];
                            struct ct_focus_r_data *step = (struct ct_focus_r_data *)&ctrl->para[UVC_RES *len]; 
                            struct ct_focus_r_data *cur = (struct ct_focus_r_data *)&ctrl->para[UVC_CUR *len];
                            struct ct_focus_r_data *zr_para = (struct ct_focus_r_data *) para;
                            uint8_t zr_step;											 
                            if (step->bSpeed == 0)
                                zr_step = 1;
                            cur->bFocusRelative = (zr_para->bFocusRelative == 0) ? 0: 
                                                  ((zr_para->bFocusRelative != (int8_t) 0xFF) ? 1: zr_para->bFocusRelative);
                            cur->bSpeed = min->bSpeed + ((uint32_t)(zr_para->bSpeed - min->bSpeed) + (zr_step >> 0x1)) / zr_step * zr_step;
                            cur->bSpeed = clamp(cur->bSpeed, min->bSpeed, max->bSpeed);
                } else if (ctrl->cid == (UVC_CID_PANTILT_ABSOLUTE)) {
                            struct ct_pan_tilta_data *min = (struct ct_pan_tilta_data *)&ctrl->para[UVC_MIN *len];
                            struct ct_pan_tilta_data *max = (struct ct_pan_tilta_data *)&ctrl->para[UVC_MAX *len];
                            struct ct_pan_tilta_data *step = (struct ct_pan_tilta_data *)&ctrl->para[UVC_RES *len]; 
                            struct ct_pan_tilta_data *cur = (struct ct_pan_tilta_data *)&ctrl->para[UVC_CUR *len];
                            struct ct_pan_tilta_data *zr_para = (struct ct_pan_tilta_data *) para;
                            int32_t zr_step;										
                            if (step->dwPanAbsolute == 0)
                                zr_step = 1;
									
                            cur->dwPanAbsolute = min->dwPanAbsolute + ((uint32_t)(zr_para->dwPanAbsolute - min->dwPanAbsolute) 
                                               + (zr_step >> 0x1)) / zr_step * zr_step;
                            cur->dwPanAbsolute = clamp(cur->dwPanAbsolute, min->dwPanAbsolute, max->dwPanAbsolute);
                            if (step->dwTiltAbsolute == 0)
                                zr_step = 1;
                            cur->dwTiltAbsolute = min->dwTiltAbsolute + ((uint32_t)(zr_para->dwTiltAbsolute - min->dwTiltAbsolute) 
                                                  + (zr_step >> 0x1)) / zr_step * zr_step;	
                            cur->dwTiltAbsolute = clamp(cur->dwTiltAbsolute, min->dwTiltAbsolute, max->dwTiltAbsolute);
                } else if (ctrl->cid == (UVC_CID_PANTILT_RELATIVE)) {
                            struct ct_pan_tiltr_data *min = (struct ct_pan_tiltr_data *)&ctrl->para[UVC_MIN *len];
                            struct ct_pan_tiltr_data *max = (struct ct_pan_tiltr_data *)&ctrl->para[UVC_MAX *len];
                            struct ct_pan_tiltr_data *step = (struct ct_pan_tiltr_data *)&ctrl->para[UVC_RES *len]; 
                            struct ct_pan_tiltr_data *cur = (struct ct_pan_tiltr_data *)&ctrl->para[UVC_CUR *len];
                            struct ct_pan_tiltr_data *zr_para = (struct ct_pan_tiltr_data *) para;
                            int32_t zr_step;
													
                            cur->bPanRelative = (zr_para->bPanRelative == 0) ? 0 : 
                                                ((zr_para->bPanRelative != (int8_t) 0xFF) ? 1 : zr_para->bPanRelative);
                            cur->bTiltRelative = (zr_para->bTiltRelative == 0) ? 0: 
                                                ((zr_para->bTiltRelative != (int8_t) 0xFF) ? 1: zr_para->bTiltRelative);	 
                            if (step->bPanSpeed == 0)
                                zr_step = 1;
                            cur->bPanSpeed = min->bPanSpeed + 
                                            ((uint32_t)(zr_para->bPanSpeed - min->bPanSpeed) + (zr_step >> 0x1)) / zr_step * zr_step;	
                            cur->bPanSpeed = clamp(cur->bPanSpeed, min->bPanSpeed, max->bPanSpeed);																						
                            if (step->bTiltSpeed == 0)
                                zr_step = 1;	 
                            cur->bTiltSpeed = min->bTiltSpeed + 
                                              ((uint32_t)(zr_para->bTiltSpeed - min->bTiltSpeed) + (zr_step >> 0x1)) / zr_step * zr_step;
                            cur->bTiltSpeed = clamp(cur->bTiltSpeed, min->bTiltSpeed, max->bTiltSpeed);	
                } else if (ctrl->cid  == (UVC_CID_WHITE_BALANCE_COMPONENT)) { 
                            struct pu_whitebalance_comp_data *min = (struct pu_whitebalance_comp_data *)&ctrl->para[UVC_MIN *len];
                            struct pu_whitebalance_comp_data *max = (struct pu_whitebalance_comp_data *)&ctrl->para[UVC_MAX *len];
                            struct pu_whitebalance_comp_data *step = (struct pu_whitebalance_comp_data *)&ctrl->para[UVC_RES *len]; 
                            struct pu_whitebalance_comp_data *cur = (struct pu_whitebalance_comp_data *)&ctrl->para[UVC_CUR *len];
                            struct pu_whitebalance_comp_data *zr_para = (struct pu_whitebalance_comp_data *) para;
                            int32_t zr_step;
															
                            if (step->wWhiteBalanceBlue == 0)
                                zr_step = 1;
                            cur->wWhiteBalanceBlue = min->wWhiteBalanceBlue + 
                                                    ((uint32_t)(zr_para->wWhiteBalanceBlue - min->wWhiteBalanceBlue) + 
                                                     (zr_step >> 0x1)) / zr_step * zr_step;
                            cur->wWhiteBalanceBlue = clamp(cur->wWhiteBalanceBlue, min->wWhiteBalanceBlue, max->wWhiteBalanceBlue);
                            if (step->wWhiteBalanceRed == 0)
                                zr_step = 1;
                            cur->wWhiteBalanceRed = min->wWhiteBalanceRed + 
                                                    ((uint32_t)(zr_para->wWhiteBalanceRed - min->wWhiteBalanceRed) 
                                                    + (zr_step >> 0x1)) / zr_step * zr_step;	
                            cur->wWhiteBalanceRed = clamp(cur->wWhiteBalanceRed, min->wWhiteBalanceRed, max->wWhiteBalanceRed);
                } else  {
                       if (ctrl->cid  == (UVC_CID_IRIS_ABSOLUTE)) {
                            struct ctrl_info *ctr_t;  
                            if (NULL == (ctr_t = uvc_find_control(udev, UVC_CID_AUTO_EXPOSURE_MODE)))
                                return -EINVAL;                    
                            if (ctr_t->para[UVC_CUR] & 0x6) {
                                return  -EINVAL;  
                            }
                        }
                        if (ctrl->cid  == (UVC_CID_WHITE_BALANCE_TEMPERATURE)) {
                            struct ctrl_info *ctr_t;  
                            if (NULL == (ctr_t = uvc_find_control(udev, UVC_CID_WHITE_BALANCE_TEMPERATURE_AUTO)))
                                return -EINVAL;                    
                            if (ctr_t->para[UVC_CUR] & 0x1) {
                                return  -EINVAL;  
                            }
                        }
                        if (ctrl->cid  == (UVC_CID_WHITE_BALANCE_COMPONENT)) {
                            struct ctrl_info *ctr_t;  
                            if (NULL == (ctr_t = uvc_find_control(udev, UVC_CID_WHITE_BALANCE_COMPONENT_AUTO)))
                                return -EINVAL;                    
                            if (ctr_t->para[UVC_CUR] & 0x1) {
                                return  -EINVAL;  
                            }
                        }                        
                        uint16_t *min = (uint16_t *)&ctrl->para[UVC_MIN *2];
                        uint16_t *max = (uint16_t *)&ctrl->para[UVC_MAX *2];
                        uint16_t *step = (uint16_t *)&ctrl->para[UVC_RES *2]; 
                        uint16_t *cur = (uint16_t *)&ctrl->para[UVC_CUR *2];
                        uint16_t *zr_para = (uint16_t *) para;
                        uint16_t fa_step = *step;											 
                        if (fa_step == 0)
                            fa_step = 1;	
//	kmdw_printf("min  is 0x%x,  max  is 0x%x, step  is 0x%x, cur is 0x%x, param is 0x%x \n",*min, *max, *step, *cur, *zr_para);									
                        *cur = *min + ((uint16_t)(*zr_para - *min) + (fa_step >> 0x1)) / fa_step * fa_step;
                        *cur = clamp(*cur, *min, *max);  
                }
            }
            return uvc_send_ctrl_req(udev, ctrl, req);
        } else
            return -1;
    }
}

static uint32_t pow(int n)
{
    long long power = 1;

    for(int i=1; i<=n; i++) {
        power = power * 2;
    }
    return power;
}

int uvc_init_device_ctrl(struct uvc_device *udev)
{
     for (int i = 0; i < udev->nITs; i++) {
        if (0 == udev->IT[i].ct->bmControls)
            break;
        if (NULL  == (udev->IT[i].ct->data = (struct ctrl_info *)malloc(CT_CTRL_NUM *sizeof(struct ctrl_info))))
            return -ENOMEM;
        memset(udev->IT[i].ct->data, 0, CT_CTRL_NUM *sizeof(struct ctrl_info));

        for (int j = 0; j < CT_CTRL_NUM; j++) {
            udev->IT[i].ct->data[j].cid = pow(j);
            if (udev->IT[i].ct->bmControls & udev->IT[i].ct->data[j].cid) {
#ifdef KDP_UVC_DEBUG			
                kmdw_printf("@@ udev->IT[i].ct->data[j].cid 0x%x\n", udev->IT[i].ct->data[j].cid);
#endif                
                udev->IT[i].ct->data[j].supported = true;
                if (0 > set_ct_ctrl_flag(&udev->IT[i].ct->data[j]))
                    return -1;
                udev->IT[i].ct->data[j].cid |= UVC_CID_CAMERA_CLASS_BASE;
                udev->IT[i].ct->data[j].eid = udev->IT[i].id;
                uvc_ctrl_cache(&udev->IT[i].ct->data[j], udev);
            } else {
             //   udev->IT[i].ct->data[j] = false;
            }
        }
   }
    for (int i = 0; i < udev->nPUs; i++) {

        if (NULL  == (udev->PU[i].data = (struct ctrl_info *)malloc(PU_CTRL_NUM *sizeof(struct ctrl_info))))
            return -ENOMEM;
        memset(udev->PU[i].data, 0, PU_CTRL_NUM *sizeof(struct ctrl_info));
        for (int j = 0; j < PU_CTRL_NUM; j++) {

            udev->PU[i].data[j].cid = pow(j);
            if (udev->PU[i].bmControls & udev->PU[i].data[j].cid) {
                if (0 > set_pu_ctrl_flag(&udev->PU[i].data[j]))
                    return -1;
                udev->PU[i].data[j].supported = true;
                udev->PU[i].data[j].cid |= UVC_CID_PU_CLASS_BASE;
                udev->PU[i].data[j].eid = udev->PU[i].id;
                uvc_ctrl_cache(&udev->PU[i].data[j], udev);
            } else {
                udev->PU[i].data[j].supported = false;
            }

        }
    }

    return 0;

}
#endif

