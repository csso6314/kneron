# ******************************************************************************
#  Copyright (c) 2021-2022. Kneron Inc. All rights reserved.                   *
# ******************************************************************************

import os
import sys
import argparse
import threading

PWD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.join(PWD, '..'))

import kp

def _install_driver_function(*args) -> None:
    print('[Installing Driver]')
    for product_id in args[0]:
        try:
            print(' - [{}]'.format(product_id.name))
            kp.core.install_driver_for_windows(product_id=product_id)
            print('    - Success')
        except kp.ApiKPException as exception:
            print('    - Error: install {} driver fail, error msg: [{}]'.format(product_id.name, str(exception)))
            exit(0)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Install Kneron Device Driver for Windows.')
    parser.add_argument('-t',
                        '--target',
                        help='Select target platform (Default: {})'.format('ALL'),
                        default='ALL',
                        choices=['ALL', 'KL520', 'KL720'],
                        type=str)
    args = parser.parse_args()

    target_device = args.target

    print('[Note]')
    print(' - You must run this app as administrator on Windows')

    product_ids = []
    if 'KL520' == target_device:
        product_ids.append(kp.ProductId.KP_DEVICE_KL520)
    elif 'KL720' == target_device:
        product_ids.append(kp.ProductId.KP_DEVICE_KL720_LEGACY)
        product_ids.append(kp.ProductId.KP_DEVICE_KL720)
    elif 'KL630' == target_device:
        product_ids.append(kp.ProductId.KP_DEVICE_KL630)
    elif 'ALL' == target_device:
        product_ids.append(kp.ProductId.KP_DEVICE_KL520)
        product_ids.append(kp.ProductId.KP_DEVICE_KL720_LEGACY)
        product_ids.append(kp.ProductId.KP_DEVICE_KL720)
        product_ids.append(kp.ProductId.KP_DEVICE_KL630)
    else:
        raise AttributeError('Error: no matched Kneron device of specified target device !')

    install_thread = threading.Thread(target=_install_driver_function, args=(product_ids,))

    install_thread.start()

    try:
        while install_thread.is_alive():
            install_thread.join(1)
    except (KeyboardInterrupt, SystemExit):
        print('\n - Received keyboard interrupt, quitting threads.')
        os.system("taskkill /F /IM installer_x64.exe")
        exit(0)
