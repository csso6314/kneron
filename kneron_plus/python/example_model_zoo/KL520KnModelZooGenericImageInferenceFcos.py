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
import kp
import cv2
import numpy as np

SCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_scpu.bin')
NCPU_FW_PATH = os.path.join(PWD, '../../res/firmware/KL520/fw_ncpu.bin')
MODEL_FILE_PATH = os.path.join(PWD,
                               '../../res/models/KL520/fcos-drk53s_w512h512_kn-model-zoo/kl520_20004_fcos-drk53s_w512h512.nef')
IMAGE_FILE_PATH = os.path.join(PWD, '../../res/images/people_talk_in_street_1500x1500.bmp')
LOOP_TIME = 1

"""
post-process utils

modified from (path in toolchain docker): /workspace/ai_training/detection/fcos/utils/fcos_det_postprocess.py
"""


def _realnms(dets, only_max=False, iou_thres=0.35):
    """
    non-maximum suppression: if only_max, will ignore iou_thres and return largest score bbox.
    dets: list[list[x, y, w, h]]
    only_max: bool
    iou_thres: float between (0,1)
    """
    dets = np.array(dets)

    if len(dets) == 0:
        return []
    scores = dets[:, 4]
    order = np.argsort(scores)[::-1]
    dets = dets[order, :]
    if only_max:
        return np.array([dets[0]])

    x1, y1, w, h, scores = dets[:, 0], dets[:, 1], dets[:, 2], dets[:, 3], dets[:, 4]
    x2, y2 = x1 + w - 1, y1 + h - 1

    areas = w * h

    order = scores.argsort()[::-1]

    keep_real = []
    tol = 0.1

    while order.size > 0:
        i = order[0]
        keep_real.append(i)
        xx1 = np.maximum(x1[i], x1[order[1:]])
        yy1 = np.maximum(y1[i], y1[order[1:]])
        xx2 = np.minimum(x2[i], x2[order[1:]])
        yy2 = np.minimum(y2[i], y2[order[1:]])

        inter_w = np.maximum(0.0, xx2 - xx1 + 1)
        inter_h = np.maximum(0.0, yy2 - yy1 + 1)

        inter_area = inter_w * inter_h
        iou = inter_area / (areas[i] + areas[order[1:]] - inter_area)

        inds = np.where(iou <= iou_thres)[0]
        order = order[inds + 1]

    return dets[keep_real, :]


def postprocess_(outputs, max_objects=100, score_thres=0.5,
                 scale=None, input_shape=None, w_ori=None, h_ori=None,
                 nms=True, iou_thres=0.35, mapping_func='linear', **kwargs):
    assert len(outputs) % 3 == 0
    n_stage = len(outputs) // 3
    dets = []
    batch_index = 0
    for stage in range(n_stage):

        """get output tensor and anchor tensor"""
        reg, cls, cts = outputs[stage], outputs[stage + n_stage], outputs[stage + n_stage * 2]

        """get dimension info"""
        b, nrows, ncols, nclasses = cls.shape
        assert b == 1
        # calculate here or pass by parameter
        stride = 2 ** int(np.log2(1.0 * input_shape[0] / nrows) + 0.5)

        """iterate over all anchors and select those with valid score"""
        # batch is always 0 here
        for i in range(nrows):
            for j in range(ncols):
                """class_id is the indice of labels. 0 is not background"""
                class_id = np.argmax(cls[batch_index, i, j, :])
                score = np.sqrt(cls[batch_index, i, j, class_id] * cts[batch_index, i, j, 0])

                if score > score_thres:
                    if mapping_func == 'exp':
                        l, t, r, b = np.exp(reg[0, i, j])
                    elif mapping_func == 'linear':
                        reg_relu = np.clip(reg[0, i, j], 0, 1e8)
                        l, t, r, b = (2 ** (3 + stage)) * (reg_relu ** 2)
                    else:
                        assert 0
                    cx, cy = j * stride + stride // 2, i * stride + stride // 2
                    xmin, ymin, xmax, ymax = cx - l, cy - t, cx + r, cy + b

                    dets.append([xmin, ymin, xmax, ymax, score, class_id])

    dets = np.asarray(dets)
    if scale is not None:
        dets[..., :4] = dets[..., :4] * scale

    if w_ori is not None and h_ori is not None and np.size(dets) > 0:
        # clip bbox make it inside image
        dets[..., :4] = np.clip(dets[..., :4], [0., 0., 0., 0.],
                                np.c_[w_ori, h_ori, w_ori, h_ori])

    if len(dets) > 0:
        dets = np.asarray(dets)
        dets[..., 2:4] = dets[..., 2:4] - dets[..., :2]
        if nms:
            dets_real = _realnms(dets, only_max=max_objects == 1, iou_thres=iou_thres)
        else:
            dets_real = dets
    else:
        dets_real = []
    # [[x1,y1,w,h],[x1,y1,w,h]]
    dets_real = np.asarray(dets_real)
    dets_real = dets_real[..., :]
    return dets_real.tolist()


def sigmoid(x):
    return 1 / (1 + np.exp(-x))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL520 Kneron Model Zoo Generic Image Inference Example - FCOS.')
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
    img_height, img_width, img_channels = img.shape
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

    tmp = []
    for o_n in inf_node_output_list:
        o_array = o_n.ndarray.copy()
        o_array = o_array.transpose((0, 2, 3, 1))
        tmp.append(o_array)

    out_data = [tmp[2], tmp[5], tmp[8], sigmoid(tmp[0]), sigmoid(tmp[3]), sigmoid(tmp[6]), sigmoid(tmp[1]),
                sigmoid(tmp[4]), sigmoid(tmp[7])]

    """
    get input node shape (batch, channel, height, width)
    """
    input_node_shape_npu = model_nef_descriptor.models[0].input_nodes[0].shape_npu
    det_res = postprocess_(out_data, max_objects=100, score_thres=0.5,
                           scale=1.0 * img_height / input_node_shape_npu[2],
                           input_shape=(input_node_shape_npu[2], input_node_shape_npu[3]),
                           w_ori=img_width, h_ori=img_height, nms=True, iou_thres=0.35)

    print('[Result]')
    print(" - Number of bounding boxes detected:")
    print(" - " + str(len(det_res)))
    output_img_name = 'output_{}'.format(os.path.basename(IMAGE_FILE_PATH))
    print(" - Output bounding boxes on \'{}\'".format(output_img_name))
    print(" - Bounding boxes info (xmin,ymin,xmax,ymax):")
    img = cv2.imread(IMAGE_FILE_PATH)
    for bbox in det_res:
        xmin = bbox[0]
        ymin = bbox[1]
        xmax = xmin + bbox[2]
        ymax = ymin + bbox[3]
        cv2.rectangle(img, (int(xmin), int(ymin)), (int(xmax), int(ymax)), (0, 255, 0), 2)
        print("(" + str(xmin) + "," + str(ymin) + ',' + str(xmax) + ',' + str(ymax) + ")")
    cv2.imwrite(os.path.join(PWD, './{}'.format(output_img_name)), img=img)
