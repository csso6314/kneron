# ******************************************************************************
#  Copyright (c) 2022. Kneron Inc. All rights reserved.                        *
# ******************************************************************************
from typing import List, Union
from utils.ExampleEnum import *

import re
import os
import sys
import cv2

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '../..'))

import kp

TARGET_FW_VERSION = 'KDP2'


def get_device_usb_speed_by_port_id(usb_port_id: int) -> kp.UsbSpeed:
    device_list = kp.core.scan_devices()

    for device_descriptor in device_list.device_descriptor_list:
        if 0 == usb_port_id:
            return device_descriptor.link_speed
        elif usb_port_id == device_descriptor.usb_port_id:
            return device_descriptor.link_speed

    raise IOError('Specified USB port ID {} not exist.'.format(usb_port_id))


def get_connect_device_descriptor(target_device: str,
                                  scan_index_list: Union[List[int], None],
                                  usb_port_id_list: Union[List[int], None]):
    print('[Check Device]')

    # scan devices
    _device_list = kp.core.scan_devices()

    # check Kneron device exist
    if _device_list.device_descriptor_number == 0:
        print('Error: no Kneron device !')
        exit(0)

    _index_device_descriptor_list = []

    # get device_descriptor of specified scan index
    if scan_index_list is not None:
        for _scan_index in scan_index_list:
            if _device_list.device_descriptor_number > _scan_index >= 0:
                _index_device_descriptor_list.append([_scan_index, _device_list.device_descriptor_list[_scan_index]])
            else:
                print('Error: no matched Kneron device of specified scan index !')
                exit(0)
    # get device_descriptor of specified port ID
    elif usb_port_id_list is not None:
        for _scan_index, __device_descriptor in enumerate(_device_list.device_descriptor_list):
            for _usb_port_id in usb_port_id_list:
                if __device_descriptor.usb_port_id == _usb_port_id:
                    _index_device_descriptor_list.append([_scan_index, __device_descriptor])

        if 0 == len(_index_device_descriptor_list):
            print('Error: no matched Kneron device of specified port ID !')
            exit(0)
    # get device_descriptor of by default
    else:
        _index_device_descriptor_list = [[_scan_index, __device_descriptor] for _scan_index, __device_descriptor in
                                         enumerate(_device_list.device_descriptor_list)]

    # check device_descriptor is specified target device
    if target_device.lower() == 'kl520':
        _target_device_product_id = kp.ProductId.KP_DEVICE_KL520
    elif target_device.lower() == 'kl720':
        _target_device_product_id = kp.ProductId.KP_DEVICE_KL720
    elif target_device.lower() == 'kl630':
        _target_device_product_id = kp.ProductId.KP_DEVICE_KL630

    for _scan_index, __device_descriptor in _index_device_descriptor_list:
        if kp.ProductId(__device_descriptor.product_id) != _target_device_product_id:
            print('Error: Not matched Kneron device of specified target device !')
            exit(0)

    for _scan_index, __device_descriptor in _index_device_descriptor_list:
        if TARGET_FW_VERSION not in __device_descriptor.firmware:
            print('Error: device is not running KDP2/KDP2 Loader firmware ...')
            print('please upload firmware first via \'kp.core.load_firmware_from_file()\'')
            exit(0)

    print(' - Success')

    return _index_device_descriptor_list


def read_image(img_path: str, img_type: str, img_format: str):
    print('[Read Image]')
    if img_type == ImageType.GENERAL.value:
        _img = cv2.imread(filename=img_path)

        if len(_img.shape) < 3:
            channel_num = 2
        else:
            channel_num = _img.shape[2]

        if channel_num == 1:
            if img_format == ImageFormat.RGB565.value:
                color_cvt_code = cv2.COLOR_GRAY2BGR565
            elif img_format == ImageFormat.RGBA8888.value:
                color_cvt_code = cv2.COLOR_GRAY2BGRA
            elif img_format == ImageFormat.RAW8.value:
                color_cvt_code = None
            else:
                print('Error: No matched image format !')
                exit(0)
        elif channel_num == 3:
            if img_format == ImageFormat.RGB565.value:
                color_cvt_code = cv2.COLOR_BGR2BGR565
            elif img_format == ImageFormat.RGBA8888.value:
                color_cvt_code = cv2.COLOR_BGR2BGRA
            elif img_format == ImageFormat.RAW8.value:
                color_cvt_code = cv2.COLOR_BGR2GRAY
            else:
                print('Error: No matched image format !')
                exit(0)
        else:
            print('Error: Not support image format !')
            exit(0)

        if color_cvt_code is not None:
            _img = cv2.cvtColor(src=_img, code=color_cvt_code)

    elif img_type == ImageType.BINARY.value:
        with open(file=img_path, mode='rb') as file:
            _img = file.read()
    else:
        print('Error: Not support image type !')
        exit(0)

    print(' - Success')
    return _img


def get_kp_image_format(image_format: str) -> kp.ImageFormat:
    if image_format == ImageFormat.RGB565.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_RGB565
    elif image_format == ImageFormat.RGBA8888.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_RGBA8888
    elif image_format == ImageFormat.YUYV.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YUYV
    elif image_format == ImageFormat.CRY1CBY0.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_CRY1CBY0
    elif image_format == ImageFormat.CBY1CRY0.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_CBY1CRY0
    elif image_format == ImageFormat.Y1CRY0CB.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_Y1CRY0CB
    elif image_format == ImageFormat.Y1CBY0CR.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_Y1CBY0CR
    elif image_format == ImageFormat.CRY0CBY1.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_CRY0CBY1
    elif image_format == ImageFormat.CBY0CRY1.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_CBY0CRY1
    elif image_format == ImageFormat.Y0CRY1CB.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_Y0CRY1CB
    elif image_format == ImageFormat.Y0CBY1CR.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_YCBCR422_Y0CBY1CR
    elif image_format == ImageFormat.RAW8.value:
        _kp_image_format = kp.ImageFormat.KP_IMAGE_FORMAT_RAW8
    else:
        print('Error: Not support image format !')
        exit(0)

    return _kp_image_format


def get_kp_normalize_mode(norm_mode: str) -> kp.NormalizeMode:
    if norm_mode == NormalizeMode.NONE.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_DISABLE
    elif norm_mode == NormalizeMode.KNERON.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_KNERON
    elif norm_mode == NormalizeMode.YOLO.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_YOLO
    elif norm_mode == NormalizeMode.TENSORFLOW.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_TENSOR_FLOW
    elif norm_mode == NormalizeMode.CUSTOMIZED_DEFAULT.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_CUSTOMIZED_DEFAULT
    elif norm_mode == NormalizeMode.CUSTOMIZED_SUB128.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_CUSTOMIZED_SUB128
    elif norm_mode == NormalizeMode.CUSTOMIZED_DIV2.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_CUSTOMIZED_DIV2
    elif norm_mode == NormalizeMode.CUSTOMIZED_SUB128_DIV2.value:
        _kp_norm = kp.NormalizeMode.KP_NORMALIZE_CUSTOMIZED_SUB128_DIV2
    else:
        print('Error: Not support normalize mode !')
        exit(0)

    return _kp_norm


def get_kp_pre_process_resize_mode(resize_mode: str) -> kp.ResizeMode:
    if resize_mode == ResizeMode.NONE.value:
        _kp_resize_mode = kp.ResizeMode.KP_RESIZE_DISABLE
    elif resize_mode == ResizeMode.ENABLE.value:
        _kp_resize_mode = kp.ResizeMode.KP_RESIZE_ENABLE
    else:
        print('Error: Not support pre process resize mode !')
        exit(0)

    return _kp_resize_mode


def get_kp_pre_process_padding_mode(padding_mode: str) -> kp.PaddingMode:
    if padding_mode == PaddingMode.NONE.value:
        _kp_padding_mode = kp.PaddingMode.KP_PADDING_DISABLE
    elif padding_mode == PaddingMode.PADDING_CORNER.value:
        _kp_padding_mode = kp.PaddingMode.KP_PADDING_CORNER
    elif padding_mode == PaddingMode.PADDING_SYMMETRIC.value:
        _kp_padding_mode = kp.PaddingMode.KP_PADDING_SYMMETRIC
    else:
        print('Error: Not support pre process padding mode !')
        exit(0)

    return _kp_padding_mode


def get_ex_post_process_mode(post_proc: str) -> PostprocessMode:
    if post_proc in PostprocessMode._value2member_map_:
        _ex_post_proc = PostprocessMode(post_proc)
    else:
        print('Error: Not support post process mode !')
        exit(0)

    return _ex_post_proc


def parse_crop_box_from_str(crop_box_str: str) -> List[kp.InferenceCropBox]:
    _group_list = re.compile(r'\([\s]*(\d+)[\s]*,[\s]*(\d+)[\s]*,[\s]*(\d+)[\s]*,[\s]*(\d+)[\s]*\)').findall(
        crop_box_str)
    _crop_box_list = []

    for _idx, _crop_box in enumerate(_group_list):
        _crop_box_list.append(
            kp.InferenceCropBox(
                crop_box_index=_idx,
                x=int(_crop_box[0]),
                y=int(_crop_box[1]),
                width=int(_crop_box[2]),
                height=int(_crop_box[3])
            )
        )

    return _crop_box_list
