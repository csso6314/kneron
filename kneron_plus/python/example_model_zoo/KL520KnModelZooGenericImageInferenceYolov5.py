# ******************************************************************************
#  Copyright (c) 2021-2022. Kneron Inc. All rights reserved.                   *
# ******************************************************************************

import os
import sys
import argparse

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '..'))
sys.path.insert(1, os.path.join(PWD, '../example/'))

from utils.ExampleHelper import get_device_usb_speed_by_port_id
from utils.ExamplePostProcess import post_process_yolo_v5
import kp
import cv2

SCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_scpu.bin')
NCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_ncpu.bin')
MODEL_FILE_PATH = os.path.join(PWD,
                               '../../res/models/KL520/yolov5-noupsample_w640h640_kn-model-zoo/kl520_20005_yolov5-noupsample_w640h640.nef')
IMAGE_FILE_PATH = os.path.join(PWD, '../../res/images/people_talk_in_street_1500x1500.bmp')
LOOP_TIME = 1

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL520 Kneron Model Zoo Generic Image Inference Example - YoloV5.')
    parser.add_argument('-p',
                        '--port_id',
                        help='Using specified port ID for connecting device (Default: port ID of first scanned Kneron '
                             'device)',
                        default=0,
                        type=int)
    parser.add_argument('-m',
                        '--model',
                        help='Model file path (.nef) (Default: {})'.format(MODEL_FILE_PATH),
                        default=MODEL_FILE_PATH,
                        type=str)
    parser.add_argument('-i',
                        '--img',
                        help='Image file path (Default: {})'.format(IMAGE_FILE_PATH),
                        default=IMAGE_FILE_PATH,
                        type=str)
    args = parser.parse_args()

    usb_port_id = args.port_id
    MODEL_FILE_PATH = args.model
    IMAGE_FILE_PATH = args.img

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
                normalize_mode=kp.NormalizeMode.KP_NORMALIZE_KNERON
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

            generic_raw_result = kp.inference.generic_image_inference_receive(device_group=device_group)
        except kp.ApiKPException as exception:
            print(' - Error: inference failed, error = {}'.format(exception))
            exit(0)

        print('.', end='', flush=True)
    print()

    """
    retrieve inference node output 
    """
    print('[Retrieve Inference Node Output ]')
    inf_node_output_list = []
    for node_idx in range(generic_raw_result.header.num_output_node):
        inference_float_node_output = kp.inference.generic_inference_retrieve_float_node(node_idx=node_idx,
                                                                                         generic_raw_result=generic_raw_result,
                                                                                         channels_ordering=kp.ChannelOrdering.KP_CHANNEL_ORDERING_CHW
                                                                                         )
        inf_node_output_list.append(inference_float_node_output)

    print(' - Success')

    yolo_result = post_process_yolo_v5(inference_float_node_output_list=inf_node_output_list,
                                       hardware_preproc_info=generic_raw_result.header.hw_pre_proc_info_list[0],
                                       thresh_value=0.2)

    print('[Result]')
    print(' - Number of boxes detected')
    print(' - ' + str(len(yolo_result.box_list)))
    output_img_name = 'output_{}'.format(os.path.basename(IMAGE_FILE_PATH))
    print(' - Output bounding boxes on \'{}\''.format(output_img_name))
    print(" - Bounding boxes info (xmin,ymin,xmax,ymax):")
    for yolo_box_result in yolo_result.box_list:
        b = 100 + (25 * yolo_box_result.class_num) % 156
        g = 100 + (80 + 40 * yolo_box_result.class_num) % 156
        r = 100 + (120 + 60 * yolo_box_result.class_num) % 156
        color = (b, g, r)

        cv2.rectangle(img=img,
                      pt1=(int(yolo_box_result.x1), int(yolo_box_result.y1)),
                      pt2=(int(yolo_box_result.x2), int(yolo_box_result.y2)),
                      color=color,
                      thickness=2)
        print("(" + str(yolo_box_result.x1) + "," + str(yolo_box_result.y1) + ',' + str(yolo_box_result.x2) + ',' + str(
            yolo_box_result.y2) + ")")
    cv2.imwrite(os.path.join(PWD, './{}'.format(output_img_name)), img=img)
