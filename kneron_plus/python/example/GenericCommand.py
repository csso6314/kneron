# ******************************************************************************
#  Copyright (c) 2021-2022. Kneron Inc. All rights reserved.                   *
# ******************************************************************************

import os
import sys
import argparse

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '..'))

import kp

CMD_SYSTEM_STR = 'system'
CMD_MODEL_STR = 'model'
CMD_REBOOT_STR = 'reboot'
CMD_SHUTDOWN_STR = 'shutdown'

TARGET_FW_VERSION = 'KDP2'
KDP2_LOADER_FW_VERSION = 'KDP2 Loader'


def _get_connect_device_descriptor(_target_device: str,
                                   _scan_index: int,
                                   _usb_port_id: int):
    print('[Check Device]')

    # scan devices
    _device_list = kp.core.scan_devices()

    # check Kneron device exist
    if _device_list.device_descriptor_number == 0:
        raise AttributeError('Error: no Kneron device !')

    _device_descriptor = None

    # get device_descriptor of specified scan index
    if _scan_index is not None:
        if _device_list.device_descriptor_number > _scan_index >= 0:
            _device_descriptor = _device_list.device_descriptor_list[_scan_index]
        else:
            raise AttributeError('Error: no matched Kneron device of specified scan index !')
    # get device_descriptor of specified port ID
    elif _usb_port_id is not None:
        for __idx, __device_descriptor in enumerate(_device_list.device_descriptor_list):
            if __device_descriptor.usb_port_id == _usb_port_id:
                _device_descriptor = __device_descriptor
                _scan_index = __idx

        if _device_descriptor is None:
            raise AttributeError('Error: no matched Kneron device of specified port ID !')
    # get device_descriptor of by default
    else:
        _scan_index = 0
        _device_descriptor = _device_list.device_descriptor_list[_scan_index]

    # check device_descriptor is specified target device
    if _target_device.lower() == 'kl520':
        _target_device_product_id = kp.ProductId.KP_DEVICE_KL520
    elif _target_device.lower() == 'kl720':
        _target_device_product_id = kp.ProductId.KP_DEVICE_KL720
    elif _target_device.lower() == 'kl630':
        _target_device_product_id = kp.ProductId.KP_DEVICE_KL630

    if _device_descriptor.firmware == 'KDP':
        raise AttributeError('Error: the firmware type of device is not supported !')

    if kp.ProductId(_device_descriptor.product_id) != _target_device_product_id:
        raise AttributeError('Error: no matched Kneron device of specified target device !')

    if TARGET_FW_VERSION not in _device_descriptor.firmware or KDP2_LOADER_FW_VERSION in _device_descriptor.firmware:
        raise AttributeError('Error: device is not running KDP2/KDP2 Loader firmware ...\n' +
                             'please upload firmware first via \'kp.core.load_firmware_from_file()\'')

    print(' - success')

    return _device_descriptor, _scan_index


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generic Command Example.')
    parser.add_argument('-t',
                        '--target',
                        help='Select target platform (Default: {})'.format('KL520'),
                        default='KL520',
                        choices=['KL520', 'KL720', 'KL630'],
                        type=str)
    parser.add_argument('-s',
                        '--scan_index',
                        help='Using specified scan index for connecting device (Default: scan index of first scanned '
                             'Kneron device)',
                        type=int)
    parser.add_argument('-p',
                        '--port_id',
                        help='Using specified port ID for connecting device (Default: port ID of first scanned Kneron '
                             'device) (Notice that scan index has higher priority than port ID)',
                        type=int)
    parser.add_argument('-c',
                        '--cmd',
                        help='Specified Command type (Default: {}) '
                             '(Notice that shutdown command is not supported by KL720)'.format('system'),
                        choices=['system', 'model', 'reboot', 'shutdown'],
                        default='system',
                        type=str)
    args = parser.parse_args()

    target_device = args.target
    scan_index = args.scan_index
    usb_port_id = args.port_id
    command = str(args.cmd)

    # get specified device descriptor
    try:
        device_descriptor, scan_index = _get_connect_device_descriptor(_target_device=target_device,
                                                                       _scan_index=scan_index,
                                                                       _usb_port_id=usb_port_id)
    except AttributeError as error:
        print(error)
        exit(0)

    # connect specified device by USB port ID
    print('[Connect Device]')
    print(' - target device: \'{}\''.format(target_device))
    print(' - scan index: \'{}\''.format(scan_index))
    print(' - port ID: \'{}\''.format(device_descriptor.usb_port_id))
    print(' - command: \'{}\''.format(command))

    try:
        # note that this API function is for high level force connection to Kneron devices, not recommended in common use
        device_group = kp.core.connect_devices_without_check(usb_port_ids=[device_descriptor.usb_port_id])
    except kp.ApiKPException as exception:
        print('Error: connect device fail, port ID = \'{}\', error msg: [{}]'.format(device_descriptor.usb_port_id,
                                                                                     str(exception)))
        exit(0)

    if command.lower() == CMD_SYSTEM_STR:
        # get system information from specified device
        try:
            print('[System Information]')
            system_info = kp.core.get_system_info(device_group=device_group,
                                                  usb_port_id=device_descriptor.usb_port_id)
            print(system_info)
        except kp.ApiKPException as exception:
            print('Error: getting system information error, error msg: [{}]'.format(str(exception)))
            exit(0)

        print('[PLUS Version]')
        print(kp.core.get_version())
    elif command.lower() == CMD_MODEL_STR:
        # get NEF model information specified device
        try:
            print('[Model NEF Information]')
            model_nef_descriptor = kp.core.get_model_info(device_group=device_group,
                                                          usb_port_id=device_descriptor.usb_port_id)
            print(model_nef_descriptor)
        except kp.ApiKPException as exception:
            print('Error: getting model info error, error msg: [{}]'.format(str(exception)))

        print('\nNote that if you want to query the model info in the flash,')
        print('please load model first via \'kp.core.load_model_from_flash()\'')
        print('Be careful that \'kp.core.load_model_from_flash()\' will clean up and replace the model data stored in fw memory!')

        exit(0)
    elif command.lower() == CMD_REBOOT_STR:
        # reboot specified device
        try:
            print('[Reboot Device]')
            kp.core.reset_device(device_group=device_group,
                                 reset_mode=kp.ResetMode.KP_RESET_REBOOT,
                                 sleep_secs=1)
            print(' - reboot command success')
        except kp.ApiKPException as exception:
            print('Error: reboot command failed, error msg: [{}]'.format(str(exception)))
            exit(0)
    elif command.lower() == CMD_SHUTDOWN_STR:
        # shutdown specified device
        try:
            print('[Shutdown Device]')
            kp.core.reset_device(device_group=device_group,
                                 reset_mode=kp.ResetMode.KP_RESET_SHUTDOWN,
                                 sleep_secs=1)
            print(' - shutdown command success')
        except kp.ApiKPException as exception:
            print('Error: shutdown command failed, error msg: [{}]'.format(str(exception)))
            exit(0)
    else:
        print('Error: no matched command to use !')
        exit(0)
