# ******************************************************************************
#  Copyright (c) 2021-2022. Kneron Inc. All rights reserved.                   *
# ******************************************************************************

import os
import sys
import argparse

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '..'))

from utils.ExampleHelper import get_device_usb_speed_by_port_id
import kp
import cv2

SCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL630/kp_firmware.tar')
MODEL_FILE_PATH = os.path.join(PWD, '../../res/models/KL630/YoloV5s_640_640_3/models_630.nef')
IMAGE_FILE_PATH = os.path.join(PWD, '../../res/images/bike_cars_street_224x224.bmp')
LOOP_TIME = 10

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL630 Demo Generic Image Inference FIFO Queue Configuration Example.')
    parser.add_argument('-p',
                        '--port_id',
                        help='Using specified port ID for connecting device (Default: port ID of first scanned Kneron '
                             'device)',
                        default=0,
                        type=int)
    args = parser.parse_args()

    usb_port_id = args.port_id

    """
        check device USB speed (Recommend run KL630 at high speed)
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
                                        ncpu_fw_path="")
        print(' - Success')
    except kp.ApiKPException as exception:
        print('Error: upload firmware failed, error = \'{}\''.format(str(exception)))
        exit(0)

    """
    setting fifo queue size
    """
    try:
        print('[Setting DDR Manager Attributes]')

        # the attributes with 0 value will be auto adjust to the suitable count or size
        ddr_manager_attributes = kp.DdrManageAttributes(
            # DDR space reserved for model.
            model_size=0,

            # Input buffer count for FIFO queue.
            input_buffer_count=2,

            # Input buffer size for FIFO queue.
            # (640 * 480 * 2) is the size for 640 x 480 RGB565 image and image below this size.
            # 1024 is reserved for header size.
            input_buffer_size=(640 * 480 * 2) + 1024,

            # Result buffer count for FIFO queue.
            result_buffer_count=2,

            # Result buffer size for FIFO queue.
            # (2 * 1024 * 1024) is for at most 2 MB raw output data.
            # 1024 is reserved for header size.
            result_buffer_size=(2 * 1024 * 1024) + 1024
        )

        # the attribute must be stored to kp.DeviceGroup before load model
        kp.core.store_ddr_management_attributes(device_group=device_group,
                                                ddr_manage_attributes=ddr_manager_attributes)
        print(' - Success')
    except kp.ApiKPException as exception:
        print('Error: set DDR attributes failed, error = \'{}\''.format(str(exception)))
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
                image_format=kp.ImageFormat.KP_IMAGE_FORMAT_RGB565
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
                                                                                         channels_ordering=kp.ChannelOrdering.KP_CHANNEL_ORDERING_CHW)
        inf_node_output_list.append(inference_float_node_output)

    print(' - Success')

    print('[Result]')
    print(inf_node_output_list)
