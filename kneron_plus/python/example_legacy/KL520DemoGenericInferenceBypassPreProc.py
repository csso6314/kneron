# ******************************************************************************
#  Copyright (c) 2021-2022. Kneron Inc. All rights reserved.                   *
# ******************************************************************************

import os
import sys
import argparse

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '..'))
sys.path.insert(1, os.path.join(PWD, '../example'))

from utils.ExampleHelper import get_device_usb_speed_by_port_id
from utils.ExamplePostProcess import post_process_tiny_yolo_v3
import kp

SCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_scpu.bin')
NCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_ncpu.bin')
MODEL_FILE_PATH = os.path.join(PWD, '../../res/models/KL520/tiny_yolo_v3/models_520.nef')
IMAGE_FILE_PATH = os.path.join(PWD, '../../res/images/bike_cars_street_224x224_rgba8888_normalized.bin')
LOOP_TIME = 50

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL520 Demo Generic Inference Bypass Pre-Processing Example.')
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
    with open(IMAGE_FILE_PATH, 'rb') as file:
        img_buffer = file.read()
    print(' - Success')

    """
    prepare generic raw inference bypass pre-processing image descriptor
    """
    generic_raw_image_header = kp.v1.GenericRawBypassPreProcImageHeader(
        model_id=model_nef_descriptor.models[0].id,
        image_buffer_size=len(img_buffer),
        inference_number=0
    )

    """
    starting inference work
    """
    print('[Starting Inference Work]')
    print(' - Starting inference loop {} times'.format(LOOP_TIME))
    print(' - ', end='')
    for i in range(LOOP_TIME):
        try:
            kp.v1.inference.generic_raw_inference_bypass_pre_proc_send(
                device_group=device_group,
                generic_raw_image_header=generic_raw_image_header,
                image_buffer=img_buffer)

            generic_raw_bypass_pre_proc_result = kp.v1.inference.generic_raw_inference_bypass_pre_proc_receive(
                device_group=device_group,
                generic_raw_image_header=generic_raw_image_header,
                model_nef_descriptor=model_nef_descriptor)
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
    for node_idx in range(generic_raw_bypass_pre_proc_result.header.num_output_node):
        inference_float_node_output = kp.inference.generic_inference_retrieve_float_node(
            node_idx=node_idx,
            generic_raw_result=generic_raw_bypass_pre_proc_result,
            channels_ordering=kp.ChannelOrdering.KP_CHANNEL_ORDERING_CHW)
        inf_node_output_list.append(inference_float_node_output)

    print(' - Success')

    """
    post-process the last raw output
    """
    print('[Tiny Yolo V3 Post-Processing]')
    hardware_preproc_info = kp.HwPreProcInfo(img_width=224,
                                             img_height=224,
                                             resized_img_width=224,
                                             resized_img_height=224,
                                             model_input_width=model_nef_descriptor.models[0].input_nodes[0].shape_npu[3],
                                             model_input_height=model_nef_descriptor.models[0].input_nodes[0].shape_npu[2])
    yolo_result = post_process_tiny_yolo_v3(inference_float_node_output_list=inf_node_output_list,
                                            hardware_preproc_info=hardware_preproc_info,
                                            thresh_value=0.2)

    print(' - Success')

    print('[Result]')
    print(yolo_result)
