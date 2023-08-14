# ******************************************************************************
#  Copyright (c) 2022. Kneron Inc. All rights reserved.                        *
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
import math

LOOP_TIME = 1


def kp_connections(keypoints):  # 3----> 0 ----> 1 ------> 2 ----> wrist, #0----> 3 ----> 2 ------> 1 ----> wrist
    kp_lines = [
        [keypoints.index('wrist'), keypoints.index('Thumb_1')],
        [keypoints.index('wrist'), keypoints.index('Index_1')],
        [keypoints.index('Index_1'), keypoints.index('Middle_1')],
        [keypoints.index('wrist'), keypoints.index('Middle_1')],
        [keypoints.index('Middle_1'), keypoints.index('Ring_1')],
        [keypoints.index('wrist'), keypoints.index('Ring_1')],
        [keypoints.index('Ring_1'), keypoints.index('Pink_1')],
        [keypoints.index('wrist'), keypoints.index('Pink_1')],
        [keypoints.index('Thumb_1'), keypoints.index('Thumb_2')],
        [keypoints.index('Thumb_2'), keypoints.index('Thumb_3')],
        [keypoints.index('Thumb_3'), keypoints.index('Thumb_0')],
        [keypoints.index('Index_1'), keypoints.index('Index_2')],
        [keypoints.index('Index_2'), keypoints.index('Index_3')],
        [keypoints.index('Index_3'), keypoints.index('Index_0')],
        [keypoints.index('Middle_1'), keypoints.index('Middle_2')],
        [keypoints.index('Middle_2'), keypoints.index('Middle_3')],
        [keypoints.index('Middle_3'), keypoints.index('Middle_0')],
        [keypoints.index('Ring_1'), keypoints.index('Ring_2')],
        [keypoints.index('Ring_2'), keypoints.index('Ring_3')],
        [keypoints.index('Ring_3'), keypoints.index('Ring_0')],
        [keypoints.index('Pink_1'), keypoints.index('Pink_2')],
        [keypoints.index('Pink_2'), keypoints.index('Pink_3')],
        [keypoints.index('Pink_3'), keypoints.index('Pink_0')],
    ]
    return kp_lines


def get_keypoints():
    """Get the COCO keypoints and their left/right flip correspondence map."""
    keypoints = [
        'wrist',        # 1
        'Thumb_1',      # 2
        'Thumb_2',      # 3
        'Thumb_3',      # 4
        'Thumb_0',      # 5
        'Index_1',      # 6 # 5
        'Index_2',      # 7 # 6
        'Index_3',      # 8
        'Index_0',      # 9
        'Middle_1',     # 10 , 9
        'Middle_2',     # 11 , 10
        'Middle_3',     # 12
        'Middle_0',     # 13
        'Ring_1',       # 14
        'Ring_2',       # 15
        'Ring_3',       # 16, 15
        'Ring_0',       # 17  16
        'Pink_1',       # 14
        'Pink_2',       # 15
        'Pink_3',       # 16, 15
        'Pink_0',       # 17  16
    ]

    return keypoints


_kp_connections = kp_connections(get_keypoints())


def drawbbox_pose(image, obj, landmark_threshold=0.3, color=None, thickness=2, textcolor=(0, 0, 0),
                  landmarkcolor=(0, 255, 255), landmark_score=False):
    bbox = obj[0]
    landmark = obj[1]
    score = obj[2]
    x, y, w, h = bbox[:4]

    landmark = np.array(landmark)
    landmark = landmark.reshape((21, 2))
    for i in range(21):
        x, y = landmark[i][:2]
        if score[i][0] < 0.2:
            landmarkcolor = (0, 0, 0)
        elif score[i][0] < 0.4:
            landmarkcolor = (0, 0, 255)
        elif score[i][0] < 0.6:
            landmarkcolor = (0, 128, 255)
        elif score[i][0] < 0.8:
            landmarkcolor = (0, 255, 255)
        else:
            landmarkcolor = (0, 255, 0)

        lm_color = (0, 0, 255)
        cv2.circle(image, (int(x), int(y)), 2, lm_color, -1, 16)
        stickwidth = 1
        color = (255, 255, 0)

        for j, e in enumerate(_kp_connections):
            X = [landmark[e[0]][1], landmark[e[1]][1]]
            Y = [landmark[e[0]][0], landmark[e[1]][0]]

            mX = np.mean(X)
            mY = np.mean(Y)

            length = ((X[0] - X[1]) ** 2 + (Y[0] - Y[1]) ** 2) ** 0.5
            angle = math.degrees(math.atan2(X[0] - X[1], Y[0] - Y[1]))
            polygon = cv2.ellipse2Poly((int(mY), int(mX)), (int(length / 2), stickwidth), int(angle), 0, 360, 1)
            cv2.fillConvexPoly(image, polygon, color)

    return image


def _get_max_preds(heatmaps):
    """Get keypoint predictions from score maps.

    Note:
        batch_size: N
        num_keypoints: K
        heatmap height: H
        heatmap width: W

    Args:
        heatmaps (np.ndarray[N, H, W, K): model predicted heatmaps.

    Returns:
        tuple: A tuple containing aggregated results.

        - preds (np.ndarray[N, K, 2]): Predicted keypoint location.
        - maxvals (np.ndarray[N, K, 1]): Scores (confidence) of the keypoints.
    """
    assert isinstance(heatmaps,
                      np.ndarray), ('heatmaps should be numpy.ndarray')
    assert heatmaps.ndim == 4, 'batch_images should be 4-ndim'

    N, _, W, K = heatmaps.shape
    heatmaps_reshaped = heatmaps.reshape((N, -1, K))
    idx = np.argmax(heatmaps_reshaped, 1).reshape((N, K, 1))
    maxvals = np.amax(heatmaps_reshaped, 1).reshape((N, K, 1))

    preds = np.tile(idx, (1, 1, 2)).astype(np.float32)
    preds[:, :, 0] = preds[:, :, 0] % W
    preds[:, :, 1] = preds[:, :, 1] // W

    preds = np.where(np.tile(maxvals, (1, 1, 2)) > 0.0, preds, -1)
    return preds, maxvals


def postprocess_(heatmaps, number_of_keypoints=21, post_process='megvii', body=False, **kwargs):
    """
    Input:
        heatmaps (np.ndarray[N, H, W, K): model predicted heatmaps.
        Note:
            batch_size: N
            num_keypoints: K
            heatmap height: H
            heatmap width: W

    """

    N, H, W, K = heatmaps.shape

    preds, maxvals = _get_max_preds(heatmaps)
    # preds (np.ndarray[N, K, 2]): Predicted keypoint location.
    # maxvals (np.ndarray[N, K, 1]): Scores (confidence) of the keypoints.

    # add +/-0.25 shift to the predicted locations for higher acc.
    for n in range(N):
        for k in range(K):  # k = number of keypoint
            heatmap = heatmaps[n, :, :, k]
            px = int(preds[n][k][0])
            py = int(preds[n][k][1])
            if 1 < px < W - 1 and 1 < py < H - 1:
                diff = np.array([
                    heatmap[py][px + 1] - heatmap[py][px - 1],
                    heatmap[py + 1][px] - heatmap[py - 1][px]
                ])
                preds[n][k] += np.sign(diff) * .25
                if post_process in ['megvii', 'megvii-mpii']:
                    preds[n][k] += 0.5
                else:
                    raise NotImplementedError

    preds *= 4  # scale back to input size

    if post_process in ['megvii', 'megvii-mpii']:  # normalization on the confidence score
        maxvals = maxvals / 255.0 + 0.5

    keypoint = []
    scores = []
    preds = preds[0]
    maxvals = maxvals[0]
    for i in range(number_of_keypoints):
        if post_process == 'megvii' and body == True:
            if i > 0:  # index flip
                if i % 2 == 0:
                    index = i - 1
                else:
                    index = i + 1
            else:
                index = i
        else:
            index = i
        keypoint_pair = preds[index].tolist()
        keypoint.extend(keypoint_pair)
        scores.append([float(maxvals[index][0]), i])

    return keypoint, scores


def convert_numpy_to_rgba_and_width_align_4(data):
    """Converts the numpy data into RGBA.

    720 input is 4 byte width aligned.

    """

    height, width, channel = data.shape

    width_aligned = 4 * math.ceil(width / 4.0)
    aligned_data = np.zeros((height, width_aligned, 4), dtype=np.int8)
    aligned_data[:height, :width, :channel] = data
    aligned_data = aligned_data.flatten()

    return aligned_data.tobytes()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='KL720 Kneron Model Zoo Generic Data Inference Example - RSN18_Hand.')
    parser.add_argument('-p',
                        '--port_id',
                        help='Using specified port ID for connecting device (Default: port ID of first scanned Kneron '
                             'device)',
                        default=0,
                        type=int)
    parser.add_argument('-img',
                        '--img_path',
                        help='input image path',
                        default=os.path.join(PWD, '../../res/images/hand_224x224.png'),
                        type=str)
    parser.add_argument('-nef',
                        '--nef_model_path',
                        help='input NEF model path',
                        default=os.path.join(PWD,
                                             '../../res/models/KL720/kn-model-zoo-mmpose_rsn18hand/example_RSN18Hand_720.nef'),
                        type=str)

    args = parser.parse_args()

    assert args.img_path is not None, "need to set input image but got None"
    assert args.nef_model_path is not None, "need to set nef model path but got None"

    usb_port_id = args.port_id
    nef_model_path = args.nef_model_path
    image_file_path = args.img_path

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
                                                            file_path=nef_model_path)
        print(' - Success')
    except kp.ApiKPException as exception:
        print('Error: upload model failed, error = \'{}\''.format(str(exception)))
        exit(0)

    """
    extract input radix from NEF
    """
    nef_radix = model_nef_descriptor.models[0].input_nodes[0].quantization_parameters.quantized_fixed_point_descriptor_list[0].radix  # only support single model NEF

    """
    prepare the image
    """
    nef_model_width = model_nef_descriptor.models[0].input_nodes[0].shape_npu[3]
    nef_model_height = model_nef_descriptor.models[0].input_nodes[0].shape_npu[2]
    print('[Read Image]')
    img = cv2.imread(filename=image_file_path)
    img_height, img_width, img_channels = img.shape

    # resize to model input size
    img_input = cv2.resize(img, (nef_model_width, nef_model_height), interpolation=cv2.INTER_AREA)

    # this model trained with normalize method: (data - 128)/256 , 
    img_input = img_input / 256.
    img_input -= 0.5

    # toolchain calculate the radix value from input data (after normalization), and set it into NEF model.
    # NPU will divide input data "2^radix" automatically, so, we have to scaling the input data here due to this reason.
    img_input *= pow(2, nef_radix)

    # convert rgb to rgba and width align 4, due to npu requirement.
    img_buffer = convert_numpy_to_rgba_and_width_align_4(img_input)

    print(' - Success')

    """
    prepare generic data inference input descriptor
    """
    generic_inference_input_descriptor = kp.GenericDataInferenceDescriptor(
        model_id=model_nef_descriptor.models[0].id,
        inference_number=0,
        input_node_data_list=[kp.GenericInputNodeData(buffer=img_buffer)]
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
    out_data = inf_node_output_list[0].ndarray.transpose((0, 2, 3, 1))
    lm, score = postprocess_(out_data, number_of_keypoints=21)

    print('[Result]')
    print(' - landmark {}'.format(lm))
    print(' - score {}'.format(score))

    print('[Output Result Image]')
    output_img_name = 'output_{}'.format(os.path.basename(image_file_path))

    rsn18hand_res_vis = cv2.imread(image_file_path)
    rsn18hand_res_vis = cv2.resize(img, (224, 224), interpolation=cv2.INTER_LINEAR)
    drawimg = drawbbox_pose(rsn18hand_res_vis, [[0, 0, 0, 0], lm, score])

    print(' - Output hand-pose result on \'{}\''.format(output_img_name))
    cv2.imwrite(output_img_name, rsn18hand_res_vis)
