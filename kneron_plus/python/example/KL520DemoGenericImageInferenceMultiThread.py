# ******************************************************************************
#  Copyright (c) 2021-2022. Kneron Inc. All rights reserved.                   *
# ******************************************************************************

from typing import Union
import os
import sys
import argparse
import time
import threading
import queue

import numpy as np

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '..'))

from utils.ExampleHelper import get_device_usb_speed_by_port_id
import kp
import cv2

SCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_scpu.bin')
NCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_ncpu.bin')
MODEL_FILE_PATH = os.path.join(PWD, '../../res/models/KL520/tiny_yolo_v3/models_520.nef')
IMAGE_FILE_PATH = os.path.join(PWD, '../../res/images/bike_cars_street_224x224.bmp')
LOOP_TIME = 100


def _image_send_function(_device_group: kp.DeviceGroup,
                         _loop_time: int,
                         _generic_inference_input_descriptor: kp.GenericImageInferenceDescriptor,
                         _image: Union[bytes, np.ndarray],
                         _image_format: kp.ImageFormat) -> None:
    for _loop in range(_loop_time):
        try:
            _generic_inference_input_descriptor.inference_number = _loop
            _generic_inference_input_descriptor.input_node_image_list = [kp.GenericInputNodeImage(
                image=_image,
                image_format=_image_format,
                resize_mode=kp.ResizeMode.KP_RESIZE_ENABLE,
                padding_mode=kp.PaddingMode.KP_PADDING_CORNER,
                normalize_mode=kp.NormalizeMode.KP_NORMALIZE_KNERON
            )]

            kp.inference.generic_image_inference_send(device_group=device_group,
                                                      generic_inference_input_descriptor=_generic_inference_input_descriptor)
        except kp.ApiKPException as exception:
            print(' - Error: inference failed, error = {}'.format(exception))
            exit(0)


def _result_receive_function(_device_group: kp.DeviceGroup,
                             _loop_time: int,
                             _result_queue: queue.Queue) -> None:
    _generic_raw_result = None

    for _loop in range(_loop_time):
        try:
            _generic_raw_result = kp.inference.generic_image_inference_receive(device_group=device_group)

            if _generic_raw_result.header.inference_number != _loop:
                print(' - Error: incorrect inference_number {} at frame {}'.format(
                    _generic_raw_result.header.inference_number, _loop))

            print('.', end='', flush=True)

        except kp.ApiKPException as exception:
            print(' - Error: inference failed, error = {}'.format(exception))
            exit(0)

    _result_queue.put(_generic_raw_result)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL520 Demo Generic Image Inference Multi-Thread Example.')
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
            print('\033[91m' + '[Error] Device is not run at high speed.' + '\033[0m')
            exit(0)
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
    )

    """
    starting inference work
    """
    print('[Starting Inference Work]')
    print(' - Starting inference loop {} times'.format(LOOP_TIME))
    print(' - ', end='')
    result_queue = queue.Queue()

    send_thread = threading.Thread(target=_image_send_function, args=(device_group,
                                                                      LOOP_TIME,
                                                                      generic_inference_input_descriptor,
                                                                      img_bgr565,
                                                                      kp.ImageFormat.KP_IMAGE_FORMAT_RGB565))

    receive_thread = threading.Thread(target=_result_receive_function, args=(device_group,
                                                                             LOOP_TIME,
                                                                             result_queue))

    start_inference_time = time.time()

    send_thread.start()
    receive_thread.start()

    try:
        while send_thread.is_alive():
            send_thread.join(1)

        while receive_thread.is_alive():
            receive_thread.join(1)
    except (KeyboardInterrupt, SystemExit):
        print('\n - Received keyboard interrupt, quitting threads.')
        exit(0)

    end_inference_time = time.time()
    time_spent = end_inference_time - start_inference_time

    try:
        generic_raw_result = result_queue.get(timeout=3)
    except Exception as exception:
        print('Error: Result queue is empty !')
        exit(0)
    print()

    print('[Result]')
    print(" - Total inference {} images".format(LOOP_TIME))
    print(" - Time spent: {:.2f} secs, FPS = {:.1f}".format(time_spent, LOOP_TIME / time_spent))

    """
    retrieve inference node output 
    """
    print('[Retrieve Inference Node Output ]')
    inf_node_output_list = []
    for node_idx in range(generic_raw_result.header.num_output_node):
        inference_float_node_output = kp.inference.generic_inference_retrieve_float_node(node_idx=node_idx,
                                                                                         generic_raw_result=generic_raw_result,
                                                                                         channels_ordering=kp.ChannelOrdering.KP_CHANNEL_ORDERING_CHW)
        inf_node_output_list.append(inference_float_node_output)

    print(' - Success')

    print('[Result]')
    print(inf_node_output_list)
