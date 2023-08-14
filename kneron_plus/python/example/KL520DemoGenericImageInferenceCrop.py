# ******************************************************************************
#  Copyright (c) 2021-2022. Kneron Inc. All rights reserved.                   *
# ******************************************************************************

import os
import sys
import argparse

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '..'))

from utils.ExampleHelper import get_device_usb_speed_by_port_id
from utils.ExamplePostProcess import post_process_tiny_yolo_v3
import kp
import cv2

SCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_scpu.bin')
NCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_ncpu.bin')
MODEL_FILE_PATH = os.path.join(PWD, '../../res/models/KL520/tiny_yolo_v3/models_520.nef')
IMAGE_FILE_PATH = os.path.join(PWD, '../../res/images/one_bike_many_cars_800x800.bmp')
LOOP_TIME = 50

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL520 Demo Generic Image Inference with Crop-Box Example.')
    parser.add_argument('-p',
                        '--port_id',
                        help='Using specified port ID for connecting device (Default: port ID of first scanned Kneron '
                             'device)',
                        default=0,
                        type=int)
    args = parser.parse_args()

    usb_port_id = args.port_id

    """
    check device USB speed (Recommend run KL520 at high speed)
    """
    try:
        if kp.UsbSpeed.KP_USB_SPEED_HIGH != get_device_usb_speed_by_port_id(usb_port_id=usb_port_id):
            print('\033[91m' + '[Warning] Device is not run at high speed.' + '\033[0m')
    except Exception as exception:
        print('Error: check device USB speed fail, port ID = \'{}\', error msg: [{}]'.format(usb_port_id,
                                                                                             str(exception)))
        exit(0)

    """
    connect the device
    """
    try:
        print('[Connect Device]')
        device_group = kp.core.connect_devices(usb_port_ids=[usb_port_id])
        print(' - Success')
    except kp.ApiKPException as exception:
        print('Error: connect device fail, port ID = \'{}\', error msg: [{}]'.format(usb_port_id,
                                                                                     str(exception)))
        exit(0)

    """
    setting timeout of the usb communication with the device
    """
    print('[Set Device Timeout]')
    kp.core.set_timeout(device_group=device_group, milliseconds=5000)
    print(' - Success')

    """
    upload firmware to device
    """
    try:
        print('[Upload Firmware]')
        kp.core.load_firmware_from_file(device_group=device_group,
                                        scpu_fw_path=SCPU_FW_PATH,
                                        ncpu_fw_path=NCPU_FW_PATH)
        print(' - Success')
    except kp.ApiKPException as exception:
        print('Error: upload firmware failed, error = \'{}\''.format(str(exception)))
        exit(0)

    """
    upload model to device
    """
    try:
        print('[Upload Model]')
        model_nef_descriptor = kp.core.load_model_from_file(device_group=device_group,
                                                            file_path=MODEL_FILE_PATH)
        print(' - Success')
    except kp.ApiKPException as exception:
        print('Error: upload model failed, error = \'{}\''.format(str(exception)))
        exit(0)

    """
    prepare the image
    """
    print('[Read Image]')
    img = cv2.imread(filename=IMAGE_FILE_PATH)
    img_bgr565 = cv2.cvtColor(src=img, code=cv2.COLOR_BGR2BGR565)
    print(' - Success')

    """
    prepare the crop box
    """
    crop_box_list = [
        kp.InferenceCropBox(
            crop_box_index=0,
            x=0,
            y=0,
            width=400,
            height=400
        ),
        kp.InferenceCropBox(
            crop_box_index=1,
            x=230,
            y=335,
            width=450,
            height=450
        )
    ]

    """
    prepare generic image inference input descriptor
    """
    generic_inference_input_descriptor = kp.GenericImageInferenceDescriptor(
        model_id=model_nef_descriptor.models[0].id,
        inference_number=0,
        input_node_image_list=[
            kp.GenericInputNodeImage(
                image=img_bgr565,
                image_format=kp.ImageFormat.KP_IMAGE_FORMAT_RGB565,
                resize_mode=kp.ResizeMode.KP_RESIZE_ENABLE,
                padding_mode=kp.PaddingMode.KP_PADDING_CORNER,
                normalize_mode=kp.NormalizeMode.KP_NORMALIZE_KNERON,
                inference_crop_box_list=crop_box_list
            )
        ]
    )

    """
    starting inference work
    """
    print('[Starting Inference Work]')
    print(' - Starting inference loop {} times'.format(LOOP_TIME))
    print(' - ', end='')
    for i in range(LOOP_TIME):
        try:
            kp.inference.generic_image_inference_send(device_group=device_group,
                                                      generic_inference_input_descriptor=generic_inference_input_descriptor)

            generic_crop_raw_result_list = []
            for idx in range(len(generic_inference_input_descriptor.input_node_image_list[0].inference_crop_box_list)):
                generic_raw_result = kp.inference.generic_image_inference_receive(device_group=device_group)
                generic_crop_raw_result_list.append(generic_raw_result)
        except kp.ApiKPException as exception:
            print(' - Error: inference failed, error = {}'.format(exception))
            exit(0)

        print('.', end='', flush=True)
    print()

    """
    retrieve inference node output 
    """
    crop_inference_float_node_output_list = []
    for generic_raw_result in generic_crop_raw_result_list:
        inference_float_node_output_list = []
        for node_idx in range(generic_raw_result.header.num_output_node):
            inference_float_node_output_list.append(
                kp.inference.generic_inference_retrieve_float_node(node_idx=node_idx,
                                                                   generic_raw_result=generic_raw_result,
                                                                   channels_ordering=kp.ChannelOrdering.KP_CHANNEL_ORDERING_CHW))
        crop_inference_float_node_output_list.append(inference_float_node_output_list)

    print(' - Retrieve {} Nodes Success'.format(generic_raw_result.header.num_output_node))

    print('[Post-Processing]')
    try:
        app_yolo_result_list = []
        for idx, generic_crop_raw_result in enumerate(generic_crop_raw_result_list):
            yolo_result = post_process_tiny_yolo_v3(
                inference_float_node_output_list=crop_inference_float_node_output_list[idx],
                hardware_preproc_info=generic_crop_raw_result.header.hw_pre_proc_info_list[0],
                thresh_value=0.2)

            app_yolo_result_list.append(yolo_result)
        print(' - Success')
    except kp.ApiKPException as exception:
        print('Error: do post post processing fail, error = \'{}\''.format(str(exception)))
        exit(0)

    print('[Result]')
    print(" - total inference {} images".format(LOOP_TIME))

    for idx, yolo_result in enumerate(app_yolo_result_list):
        print('\n[Crop Box {}]'.format(idx))
        print(' - [Crop Box Information]\n{}'.format(generic_inference_input_descriptor.input_node_image_list[0].inference_crop_box_list[idx]))
        print(' - [Crop Box Result]')
        print(yolo_result)

    """
    output result image
    """
    print('[Output Result Image]')
    out_img = cv2.imread(filename=IMAGE_FILE_PATH)

    for idx, generic_crop_raw_result in enumerate(generic_crop_raw_result_list):
        output_img_base_name, ext = os.path.splitext(os.path.basename(IMAGE_FILE_PATH))
        output_img_name = 'output_{}_crop{}{}'.format(output_img_base_name, idx, ext)
        print(' - Output bounding boxes on \'{}\''.format(output_img_name))

        inference_crop_box = generic_crop_raw_result.header.hw_pre_proc_info_list[0].crop_area
        crop_out_img = out_img[inference_crop_box.y:inference_crop_box.y + inference_crop_box.height,
                               inference_crop_box.x:inference_crop_box.x + inference_crop_box.width].copy()

        for yolo_result in app_yolo_result_list[idx].box_list:
            b = 100 + (25 * yolo_result.class_num) % 156
            g = 100 + (80 + 40 * yolo_result.class_num) % 156
            r = 100 + (120 + 60 * yolo_result.class_num) % 156
            color = (b, g, r)

            cv2.rectangle(img=crop_out_img,
                          pt1=(int(yolo_result.x1), int(yolo_result.y1)),
                          pt2=(int(yolo_result.x2), int(yolo_result.y2)),
                          color=color)
        cv2.imwrite(os.path.join(PWD, './{}'.format(output_img_name)), img=crop_out_img)
