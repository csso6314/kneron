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
from utils.ExamplePostProcess import post_process_yolo_v5
import kp
import cv2
import numpy as np
import math

MODEL_FILE_PATH = os.path.join(PWD, '../../res/models/KL720/YoloV5s_640_640_3/models_720.nef')
IMAGE_FILE_PATH = os.path.join(PWD, '../../res/images/people_talk_in_street_640x640.bmp')
LOOP_TIME = 10

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL720 Demo Generic Data Inference Example.')
    parser.add_argument('-p',
                        '--port_id',
                        help='Using specified port ID for connecting device (Default: port ID of first scanned Kneron '
                             'device)',
                        default=0,
                        type=int)
    args = parser.parse_args()

    usb_port_id = args.port_id

    """
    check device USB speed (Recommend run KL720 at super speed)
    """
    try:
        if kp.UsbSpeed.KP_USB_SPEED_SUPER != get_device_usb_speed_by_port_id(usb_port_id=usb_port_id):
            print('\033[91m' + '[Warning] Device is not run at super speed.' + '\033[0m')
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
    prepare the pre-processed data for NPU inference
    (for more information, please refer: https://doc.kneron.com/docs/#plus_python/tutorial/chapter/model_inference_with_data_inference/)
    """
    print('[Prepare NPU Inference Data]')
    # read data file
    img = cv2.imread(filename=IMAGE_FILE_PATH)
    img = cv2.cvtColor(src=img, code=cv2.COLOR_BGR2RGB)
    img_height, img_width, img_channel = img.shape
    data = img.astype(np.int32)

    # get model input radix, scale
    model_input_radix = model_nef_descriptor.models[0].input_nodes[0].quantization_parameters.quantized_fixed_point_descriptor_list[0].radix
    model_input_scale = model_nef_descriptor.models[0].input_nodes[0].quantization_parameters.quantized_fixed_point_descriptor_list[0].scale

    # get model input data layout
    model_input_data_layout = model_nef_descriptor.models[0].input_nodes[0].data_layout

    # get model input size (shape order: BxCxHxW)
    model_input_channel = model_nef_descriptor.models[0].input_nodes[0].shape_npu[1]
    model_input_height = model_nef_descriptor.models[0].input_nodes[0].shape_npu[2]
    model_input_width = model_nef_descriptor.models[0].input_nodes[0].shape_npu[3]

    # do normalization - this model is trained with normalize method: (data - 128) / 256
    data = (data - 128) / 256.0

    # toolchain calculate the radix value from input data (after normalization), and set it into NEF model.
    # NPU will divide input data "2^radix" automatically, so, we have to scaling the input data here due to this reason.
    data *= (np.power(2, model_input_radix) * model_input_scale)
    data = np.round(data)
    data = np.clip(data, -128, 127).astype(np.int8)

    # re-layout the data to fit NPU data layout format
    # KL720 supported NPU input layout format: 4W4C8B, 1W16C8B, 16W1C8B
    if kp.ModelTensorDataLayout.KP_MODEL_TENSOR_DATA_LAYOUT_4W4C8B == model_input_data_layout:
        width_align_base = 4
        channel_align_base = 4
        pass
    elif kp.ModelTensorDataLayout.KP_MODEL_TENSOR_DATA_LAYOUT_1W16C8B == model_input_data_layout:
        width_align_base = 1
        channel_align_base = 16
        pass
    elif kp.ModelTensorDataLayout.KP_MODEL_TENSOR_DATA_LAYOUT_16W1C8B == model_input_data_layout:
        width_align_base = 16
        channel_align_base = 1
        pass
    else:
        print(' - Error: invalid input NPU layout format {}'.format(str(model_input_data_layout)))
        exit(0)

    # calculate width alignment size, channel block count
    model_input_width_align = width_align_base * math.ceil(model_input_width / float(width_align_base))
    model_input_channel_block_num = math.ceil(model_input_channel / float(channel_align_base))

    # create re-layout data container
    # KL720 dimension order: CxHxW
    re_layout_data = np.zeros((model_input_channel_block_num,
                               model_input_height,
                               model_input_width_align,
                               channel_align_base), dtype=np.int8)

    # fill data in re-layout data container
    model_input_channel_block_offset = 0
    for model_input_channel_block_idx in range(model_input_channel_block_num):
        model_input_channel_block_offset_end = model_input_channel_block_offset + channel_align_base
        model_input_channel_block_offset_end = model_input_channel_block_offset_end if model_input_channel_block_offset_end < model_input_channel else model_input_channel

        re_layout_data[model_input_channel_block_idx,
                       :model_input_height,
                       :model_input_width,
                       :(model_input_channel_block_offset_end - model_input_channel_block_offset)] = \
            data[:, :, model_input_channel_block_offset:model_input_channel_block_offset_end]

        model_input_channel_block_offset += channel_align_base

    # convert re-layout data to npu inference buffer
    npu_input_buffer = re_layout_data.tobytes()
    print(' - Success')

    """
    prepare generic data inference input descriptor
    """
    generic_inference_input_descriptor = kp.GenericDataInferenceDescriptor(
        model_id=model_nef_descriptor.models[0].id,
        inference_number=0,
        input_node_data_list=[kp.GenericInputNodeData(buffer=npu_input_buffer)]
    )

    """
    starting inference work
    """
    print('[Starting Inference Work]')
    print(' - Starting inference loop {} times'.format(LOOP_TIME))
    print(' - ', end='')
    for i in range(LOOP_TIME):
        try:
            kp.inference.generic_data_inference_send(device_group=device_group,
                                                     generic_inference_input_descriptor=generic_inference_input_descriptor)

            generic_raw_result = kp.inference.generic_data_inference_receive(device_group=device_group)
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
        inference_float_node_output = kp.inference.generic_inference_retrieve_float_node(
            node_idx=node_idx,
            generic_raw_result=generic_raw_result,
            channels_ordering=kp.ChannelOrdering.KP_CHANNEL_ORDERING_CHW)
        inf_node_output_list.append(inference_float_node_output)

    print(' - Success')

    """
    post-process the last raw output
    """
    print('[Yolo V5 Post-Processing]')
    hardware_preproc_info = kp.HwPreProcInfo(img_width=img_width,
                                             img_height=img_height,
                                             resized_img_width=img_width,
                                             resized_img_height=img_height,
                                             model_input_width=model_input_width,
                                             model_input_height=model_input_height)
    yolo_result = post_process_yolo_v5(inference_float_node_output_list=inf_node_output_list,
                                       hardware_preproc_info=hardware_preproc_info,
                                       thresh_value=0.15,
                                       with_sigmoid=False)

    print(' - Success')

    print('[Result]')
    print(yolo_result)
