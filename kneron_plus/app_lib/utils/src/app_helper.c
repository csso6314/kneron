#include "app_helper.h"

int get_image_size(kp_image_format_t format, int width, int height, uint32_t *image_size)
{
    switch (format)
    {
    case KP_IMAGE_FORMAT_RGB565:
    case KP_IMAGE_FORMAT_YUYV:
    case KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0:
    case KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR:
    case KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1:
    case KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB:
    case KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR:
        *image_size = width * height * 2;
        return KP_SUCCESS;
    case KP_IMAGE_FORMAT_RGBA8888:
        *image_size = width * height * 4;
        return KP_SUCCESS;
    case KP_IMAGE_FORMAT_RAW8:
        *image_size = width * height;
        return KP_SUCCESS;
    case KP_IMAGE_FORMAT_YUV420:
        *image_size = width * height * 1.5;
        return KP_SUCCESS;
    default:
    case KP_IMAGE_FORMAT_UNKNOWN:
        *image_size = 0;
        return KP_ERROR_INVALID_PARAM_12;
    }
}

bool check_model_id_is_exist_in_nef(kp_device_group_t devices_grp, uint32_t model_id)
{
    bool ret = false;

    for (int m = 0; m < devices_grp->loaded_model_desc.num_models; m++)
    {
        if (devices_grp->loaded_model_desc.models[m].id == model_id)
        {
            ret = true;
            break;
        }
    }

    return ret;
}