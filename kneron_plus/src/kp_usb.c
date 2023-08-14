/**
 * @file        kp_usb.c
 * @brief       low level usb functions
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "kp_usb.h"
#include "KL720_usb_minion.h"
#include "kdp2_ipc_cmd.h"

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) printf(format, ##__VA_ARGS__)
#else
#define dbg_print(format, ...)
#endif

#define KP_DEVICE_KL720_PREV 0x200

#define VID_KNERON 0x3231

#define MAX_TXFER_SIZE (2 * 1024 * 1024)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// *********************************************************************************************** //
// Below are internal or static data structure or functions
// *********************************************************************************************** //

pthread_mutex_t _g_mutex = PTHREAD_MUTEX_INITIALIZER; // global mutex
static int _g_libusb_ref_count = 0; // reference count of libusb

static int __kn_usb_bulk_out(kp_usb_device_t *dev, unsigned char endpoint, void *buf, int length, unsigned int timeout)
{
	int status;
	int transferred;

	int total_txfer = length;
	int one_txfer = 0;
	uintptr_t cur_write_address = (uintptr_t)buf;

	libusb_device_handle *usbdev = dev->usb_handle;
	int speed = dev->dev_descp.link_speed;
	int max_psize = (speed <= LIBUSB_SPEED_HIGH) ? 512 : 1024;

	while (1)
	{
		if (total_txfer > MAX_TXFER_SIZE)
			one_txfer = MAX_TXFER_SIZE;
		else
			one_txfer = total_txfer;

		status = libusb_bulk_transfer(usbdev, endpoint, (unsigned char *)cur_write_address, one_txfer, &transferred, timeout);

		if (status != 0)
			return status;
		else if (one_txfer != transferred)
			return status; // FIXME

		total_txfer -= transferred;

		if (total_txfer == 0)
			break; // transfer is done

		cur_write_address += transferred;
	}

	// check if need to send zero length packet
	if ((length % max_psize) == 0)
	{
		unsigned int zlp_buf;
		int len = 0;
		int transferred;

		if ((dev->dev_descp.product_id == KP_DEVICE_KL720) ||
			(dev->dev_descp.product_id == KP_DEVICE_KL720_PREV) ||
			((dev->fw_serial & KP_KDP2_FW_V2) == KP_KDP2_FW_V2) ||
			((dev->fw_serial & KP_KDP2_FW) == KP_KDP2_FW))
		{
			// use fake ZLP as workaround
			zlp_buf = 0x11223344;
			len = 4;
		}

		status = libusb_bulk_transfer(usbdev, endpoint, (unsigned char *)&zlp_buf, len, &transferred, timeout);

		if (status != 0 || transferred != len)
		{
			dbg_print("[%s] [kp_usb] send fake ZLP failed error: %s\n", __func__, libusb_strerror((enum libusb_error)status));
			return status;
		}
	}

	return KP_USB_RET_OK;
}

static int __kn_usb_bulk_in(kp_usb_device_t *dev, unsigned char endpoint, void *buf, int buf_size, int *recv_size, unsigned int timeout)
{
	libusb_device_handle *usbdev = dev->usb_handle;

	int status;
	int transferred;
	uintptr_t cur_read_address = (uintptr_t)buf;
	int speed = dev->dev_descp.link_speed;
	int max_psize = (speed <= LIBUSB_SPEED_HIGH) ? 512 : 1024;

	*recv_size = 0;

	int _buf_size = buf_size;
	while (1)
	{
		int one_buf_size = MIN(_buf_size, MAX_TXFER_SIZE);
		_buf_size -= one_buf_size;

		status = libusb_bulk_transfer(usbdev, endpoint, (unsigned char *)cur_read_address, one_buf_size, &transferred, timeout);

		if (status != 0)
		{
			dbg_print("[kp_usb] recv data failed error: %s\n", libusb_strerror((enum libusb_error)status));
			return status;
		}

		*recv_size += transferred;

		cur_read_address += transferred;

		if (transferred < one_buf_size || _buf_size == 0)
			break;
	}

	// try to receive zlp
	if (buf_size == *recv_size && (*recv_size & (max_psize - 1)) == 0)
	{
		int zlp_buf;

		status = libusb_bulk_transfer(usbdev, endpoint, (unsigned char *)&zlp_buf, 4, &transferred, 5);

		if (status != 0)
		{
			dbg_print("[kp_usb] libusb_bulk_transfer ZLP failed error: %s\n", libusb_strerror((enum libusb_error)status));
			return status;
		}

		if (transferred != 0)
		{
			dbg_print("[%s] [kp_usb] error, should be ZLP !!\n", __func__);
			return KP_USB_RET_ERR;
		}
	}

	return KP_USB_RET_OK;
}

static void __increase_usb_refcnt()
{
	pthread_mutex_lock(&_g_mutex);
	if (_g_libusb_ref_count == 0)
	{
		if (0 != libusb_init(NULL))
			// FIXME: do something ?
			dbg_print("[%s] [kp_usb] libusb_init() failed\n", __func__);
	}
	++_g_libusb_ref_count;
	pthread_mutex_unlock(&_g_mutex);
}

static void __decrease_usb_refcnt()
{
	pthread_mutex_lock(&_g_mutex);
	--_g_libusb_ref_count;
	if (_g_libusb_ref_count == 0)
	{
		libusb_exit(NULL);
	}
	pthread_mutex_unlock(&_g_mutex);
}

static int __kn_configure_usb_device(libusb_device_handle *usbdev_handle)
{
	int status;
	int config;

	status = libusb_get_configuration(usbdev_handle, &config);
	if (status)
	{
		dbg_print("[%s] [khost_usb] get config failed: %s\n", __func__, libusb_strerror((enum libusb_error)status));
		return status;
	}

	if (config == 0)
	{
		status = libusb_set_configuration(usbdev_handle, 1);
		if (status)
		{
			dbg_print("[%s] [khost_usb] set config failed: %s\n", __func__, libusb_strerror((enum libusb_error)status));
			return status;
		}
	}

	status = libusb_claim_interface(usbdev_handle, 0);
	if (status)
	{
		dbg_print("[%s] [khost_usb] libusb_claim_interface() failed: %s\n", __func__, libusb_strerror((enum libusb_error)status));
		return status;
	}

	return 0;
}

static int __kn_usb_interrupt_in(kp_usb_device_t *dev, unsigned char endpoint, void *buf, int buf_size, int *recv_size, unsigned int timeout)
{
	libusb_device_handle *usbdev = dev->usb_handle;

	int status;
	int transferred;
	uintptr_t cur_read_address = (uintptr_t)buf;

	*recv_size = 0;

	int _buf_size = buf_size;
	while (1)
	{
		int one_buf_size = MIN(_buf_size, MAX_TXFER_SIZE);
		_buf_size -= one_buf_size;

		status = libusb_interrupt_transfer(usbdev, endpoint, (unsigned char *)cur_read_address, one_buf_size, &transferred, timeout);

		if (status != 0)
		{
			dbg_print("[%s] [kp_usb] recv data failed error: %s\n", __func__, libusb_strerror((enum libusb_error)status));
			return status;
		}

		*recv_size += transferred;

		cur_read_address += transferred;

		if (transferred < one_buf_size || _buf_size == 0)
			break;
	}

	return KP_USB_RET_OK;
}

enum dfu_state
{
	DFU_STATE_appIDLE = 0,
	DFU_STATE_appDETACH = 1,
	DFU_STATE_dfuIDLE = 2,
	DFU_STATE_dfuDNLOAD_SYNC = 3,
	DFU_STATE_dfuDNBUSY = 4,
	DFU_STATE_dfuDNLOAD_IDLE = 5,
	DFU_STATE_dfuMANIFEST_SYNC = 6,
	DFU_STATE_dfuMANIFEST = 7,
	DFU_STATE_dfuMANIFEST_WAIT_RST = 8,
	DFU_STATE_dfuUPLOAD_IDLE = 9,
	DFU_STATE_dfuERROR = 10
};

/* DFU commands */
#define DFU_DETACH 0
#define DFU_DNLOAD 1
#define DFU_UPLOAD 2
#define DFU_GETSTATUS 3
#define DFU_CLRSTATUS 4
#define DFU_GETSTATE 5
#define DFU_ABORT 6

#include <unistd.h>

static int usb_dfu_get_status(libusb_device_handle *usb_handle)
{
	int length = 6;
	unsigned char data[0x10];
	uint8_t bmRequestType = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
	uint8_t bRequest = DFU_GETSTATUS;
	int ret = libusb_control_transfer(usb_handle, bmRequestType, bRequest, 0, 0, data, (uint16_t)length, 1000);

	return (0 < ret) ? data[4] : ret;
}

static int usb_dfu_download(libusb_device *dev, unsigned char *p_buf, int buf_size)
{
	dbg_print("starting loading file ...\n");

	libusb_device_handle *usb_handle;
	int ret = libusb_open(dev, &usb_handle);

	if (0 != ret) {
		return ret;
	}

	if (0 != __kn_configure_usb_device(usb_handle))
	{
		libusb_close(usb_handle);

		return KP_USB_CONFIGURE_ERR;
	}

	int cnt = 0;
	int dfu_status = -1;
	while (buf_size)
	{
		dfu_status = usb_dfu_get_status(usb_handle);
		if (dfu_status != DFU_STATE_dfuERROR)
		{
			uint8_t bmRequestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
			long txfer_size = (buf_size > 2048) ? 2048 : buf_size;
			libusb_control_transfer(usb_handle, bmRequestType, DFU_DNLOAD, cnt, 0, p_buf + (cnt * 2048), (uint16_t)txfer_size, 1000);
			buf_size -= txfer_size;
			cnt++;
		}
		else //if(dfu_status==DFU_STATE_dfuERROR)
		{
			printf("usb dfu device report ERROR STATE\n");
		}
	}

	dfu_status = usb_dfu_get_status(usb_handle);
	if (dfu_status != DFU_STATE_dfuERROR)
	{
		uint8_t bmRequestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_REQUEST_TYPE_CLASS;
		libusb_control_transfer(usb_handle, bmRequestType, DFU_DNLOAD, cnt, 0, NULL, 0, 1000);
	}
	else //if(dfu_status==DFU_STATE_dfuERROR)
	{
		printf("usb dfu device report ERROR STATE\n");
	}

	do
	{
		dfu_status = usb_dfu_get_status(usb_handle);
	} while (dfu_status != DFU_STATE_appIDLE);

	libusb_reset_device(usb_handle);
	libusb_close(usb_handle);

	return 0;
}

static int usb_dfu_scan_download()
{
	int ret;

	__increase_usb_refcnt();

	libusb_device **devs_list;
	bool ifHappend = false;

	pthread_mutex_lock(&_g_mutex);
	ssize_t cnt = libusb_get_device_list(NULL, &devs_list);
	pthread_mutex_unlock(&_g_mutex);

	if (cnt >= 0)
	{
		ret = 0;

		for (int i = 0; i < cnt; i++)
		{
			libusb_device *dev = devs_list[i];

			struct libusb_device_descriptor desc;
			int r = libusb_get_device_descriptor(dev, &desc);
			if (r < 0)
				continue;

			if (desc.idVendor == VID_KNERON && desc.idProduct == KP_DEVICE_KL720 && desc.bcdDevice == KP_KDP2_FW_KL720_USB_DFU)
			{
				// BetterMe: use threads to handle multiple devices
				ifHappend = true;
				usb_dfu_download(dev, kl720_usb_minion_fw, sizeof(kl720_usb_minion_fw));
			}

			continue;
		}
	}
	else
	{
		ret = (int)cnt; // libusb_error
	}

	libusb_free_device_list(devs_list, 1);

	__decrease_usb_refcnt();

	if (ifHappend)
		usleep(500 * 1000); // FIXME, better timing

	return ret;
}

static void get_fw_name_by_fw_serial(char *fw_name, uint16_t product_id, uint16_t fw_serial)
{
	uint16_t fw_mode = (KP_KDP2_FW_FIND_MODE_MASK_V2 & fw_serial);
	uint16_t fw_mode_legacy = (KP_KDP2_FW_FIND_MODE_MASK & fw_serial);
	uint16_t fw_type = (KP_KDP2_FW_FIND_TYPE_MASK_V2 & fw_serial);
	uint16_t fw_type_legacy = (KP_KDP2_FW_FIND_TYPE_MASK & fw_serial);
	uint16_t fw_os = (KP_KDP2_FW_FIND_OS_MASK_V2 & fw_serial);

	if (KP_DEVICE_KL520 == product_id) {
		if ((KP_KDP2_FW_V2 & fw_serial) == KP_KDP2_FW_V2) {
			if (KP_KDP2_FW_LOADER_V2 == fw_type)
				strcpy(fw_name, "KDP2 Loader");
			else {
				if (KP_KDP2_FW_COMPANION_MODE_V2 == fw_mode)
					strcpy(fw_name, "KDP2 Comp");
				else if (KP_KDP2_FW_HICO_MODE_V2 == fw_mode)
					strcpy(fw_name, "KDP2 HICO");
				else if (KP_KDP2_FW_HOST_MODE_V2 == fw_mode)
					strcpy(fw_name, "KDP2 Host");
				else
					strcpy(fw_name, "KDP2 Unknown");

				if (KP_KDP2_FW_JTAG_TYPE_V2 == fw_type)
					strcpy(&fw_name[strlen(fw_name)], "/J");
				else if (KP_KDP2_FW_FLASH_TYPE_V2 == fw_type)
					strcpy(&fw_name[strlen(fw_name)], "/F");
				else if (KP_KDP2_FW_USB_TYPE_V2 == fw_type)
					strcpy(&fw_name[strlen(fw_name)], "/U");
			}
		} else if ((KP_KDP2_FW & fw_serial) == KP_KDP2_FW) {
			if (KP_KDP2_FW_LOADER == fw_type_legacy)
				strcpy(fw_name, "KDP2 Loader");
			else {
				if (KP_KDP2_FW_COMPANION_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 Comp");
				else if (KP_KDP2_FW_HICO_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 HICO");
				else if (KP_KDP2_FW_HOST_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 Host");
				else
					strcpy(fw_name, "KDP2 Unknown");

				if (KP_KDP2_FW_JTAG_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/J");
				else if (KP_KDP2_FW_FLASH_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/F");
				else if (KP_KDP2_FW_USB_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/U");
			}
		} else {
			strcpy(fw_name, "KDP");
		}
	} else if ((KP_DEVICE_KL720 == product_id) || (KP_DEVICE_KL720_PREV == product_id)) {
		if (KP_KDP2_FW_KL720_USB_DFU == fw_serial)
			strcpy(fw_name, "USB DFU (error!)");
		else if (KP_KDP2_FW_KL720_LOADER == fw_serial)
			strcpy(fw_name, "KDP2 Loader");
		else if ((KP_KDP2_FW_V2 & fw_serial) == KP_KDP2_FW_V2) {
			if (KP_KDP2_FW_COMPANION_MODE_V2 == fw_mode)
				strcpy(fw_name, "KDP2 Comp");
			else if (KP_KDP2_FW_HICO_MODE_V2 == fw_mode)
				strcpy(fw_name, "KDP2 HICO");
			else if (KP_KDP2_FW_HOST_MODE_V2 == fw_mode)
				strcpy(fw_name, "KDP2 Host");
			else
				strcpy(fw_name, "KDP2 Unknown");

			if (KP_KDP2_FW_JTAG_TYPE_V2 == fw_type)
				strcpy(&fw_name[strlen(fw_name)], "/J");
			else if (KP_KDP2_FW_FLASH_TYPE_V2 == fw_type)
				strcpy(&fw_name[strlen(fw_name)], "/F");
			else if (KP_KDP2_FW_USB_TYPE_V2 == fw_type)
				strcpy(&fw_name[strlen(fw_name)], "/U");
		} else if ((KP_KDP2_FW & fw_serial) == KP_KDP2_FW) {
			if (KP_KDP2_FW_COMPANION_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 Comp");
				else if (KP_KDP2_FW_HICO_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 HICO");
				else if (KP_KDP2_FW_HOST_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 Host");
				else
					strcpy(fw_name, "KDP2 Unknown");

				if (KP_KDP2_FW_JTAG_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/J");
				else if (KP_KDP2_FW_FLASH_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/F");
				else if (KP_KDP2_FW_USB_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/U");
		} else {
			strcpy(fw_name, "KDP");
		}
	} else if (KP_DEVICE_KL630 == product_id) {
		if ((KP_KDP2_FW_V2 & fw_serial) == KP_KDP2_FW_V2) {
			if (KP_KDP2_FW_LOADER_V2 == fw_type)
				strcpy(fw_name, "KDP2 Loader");
			else {
				if (KP_KDP2_FW_COMPANION_MODE_V2 == fw_mode)
					strcpy(fw_name, "KDP2 Comp");
				else if (KP_KDP2_FW_HICO_MODE_V2 == fw_mode)
					strcpy(fw_name, "KDP2 HICO");
				else if (KP_KDP2_FW_HOST_MODE_V2 == fw_mode)
					strcpy(fw_name, "KDP2 Host");
				else
					strcpy(fw_name, "KDP2 Unknown");

				if (KP_KDP2_FW_JTAG_TYPE_V2 == fw_type)
					strcpy(&fw_name[strlen(fw_name)], "/J");
				else if (KP_KDP2_FW_FLASH_TYPE_V2 == fw_type)
					strcpy(&fw_name[strlen(fw_name)], "/F");
				else if (KP_KDP2_FW_USB_TYPE_V2 == fw_type)
					strcpy(&fw_name[strlen(fw_name)], "/U");
			}

			if (KP_KDP2_FW_RTOS_OS_V2 == fw_os) {
				strcpy(&fw_name[strlen(fw_name)], "/R");
			} else if (KP_KDP2_FW_LINUX_OS_V2 == fw_os) {
				strcpy(&fw_name[strlen(fw_name)], "/L");
			}
		} else if ((KP_KDP2_FW & fw_serial) == KP_KDP2_FW_V2) {
			if (KP_KDP2_FW_LOADER == fw_type)
				strcpy(fw_name, "KDP2 Loader");
			else {
				if (KP_KDP2_FW_COMPANION_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 Comp");
				else if (KP_KDP2_FW_HICO_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 HICO");
				else if (KP_KDP2_FW_HOST_MODE == fw_mode_legacy)
					strcpy(fw_name, "KDP2 Host");
				else
					strcpy(fw_name, "KDP2 Unknown");

				if (KP_KDP2_FW_JTAG_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/J");
				else if (KP_KDP2_FW_FLASH_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/F");
				else if (KP_KDP2_FW_USB_TYPE == fw_type_legacy)
					strcpy(&fw_name[strlen(fw_name)], "/U");
			}
		} else {
			strcpy(fw_name, "KDP");
		}
	} else {
		strcpy(fw_name, "Unknown");
	}
}

// *********************************************************************************************** //
// APIs for device initialization
// *********************************************************************************************** //
kp_devices_list_t *kp_usb_scan_devices()
{
	static kp_devices_list_t *kdev_list = NULL;
	static int kdev_list_size = 0;

	libusb_device **devs_list;
	ssize_t cnt;

	__increase_usb_refcnt();

	// this is a workaround for Faraday DFU status and for KN_NUMBER
	// special process for USB DFU devices (KL720)
	// if found, minion FW will be downloaded
	usb_dfu_scan_download();

	pthread_mutex_lock(&_g_mutex);
	cnt = libusb_get_device_list(NULL, &devs_list);
	pthread_mutex_unlock(&_g_mutex);

	if (cnt < 0)
	{
		__decrease_usb_refcnt();
		return NULL;
	}

	int need_buf_size = sizeof(int) + cnt * sizeof(kp_device_descriptor_t);

	if (need_buf_size > kdev_list_size)
	{
		kp_devices_list_t *temp = (kp_devices_list_t *)realloc((void *)kdev_list, need_buf_size);
		if (NULL == temp)
			return NULL;
		kdev_list = temp;
		kdev_list_size = need_buf_size;
	}

	kdev_list->num_dev = 0;

	libusb_device *dev;
	int i = 0;

	while ((dev = devs_list[i++]) != NULL)
	{

		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
		{
			continue;
		}

		if (desc.idVendor == VID_KNERON)
		{
			int sidx = kdev_list->num_dev;

			kdev_list->device[sidx].vendor_id = desc.idVendor;
			kdev_list->device[sidx].product_id = desc.idProduct;
			kdev_list->device[sidx].link_speed = (kp_usb_speed_t)libusb_get_device_speed(dev);

			get_fw_name_by_fw_serial(kdev_list->device[sidx].firmware, desc.idProduct, desc.bcdDevice);

			kdev_list->device[sidx].port_id = 0x0;

			uint8_t bus_number;
			uint8_t ports_number[7];

			bus_number = libusb_get_bus_number(dev);
			int port_depth = libusb_get_port_numbers(dev, ports_number, 7);

			kdev_list->device[sidx].port_id |= (bus_number & 0x3); // 2 bits

			for (int i = 0; i < port_depth; i++)
				kdev_list->device[sidx].port_id |= ((uint32_t)ports_number[i] << (2 + i * 5));

			kdev_list->device[sidx].port_path[0] = 0;
			sprintf(kdev_list->device[sidx].port_path, "%d", bus_number);

            char temp_str[5];
			for (int j = 0; j < port_depth; j++)
			{
				snprintf(temp_str, sizeof(temp_str), "-%d", ports_number[j]);
				strcat(kdev_list->device[sidx].port_path, temp_str);
			}

			kdev_list->device[sidx].kn_number = 0x0;

			libusb_device_handle *dev_handle = 0;
			int sts = libusb_open(dev, &dev_handle);
			if (sts == 0)
			{
				bool isConnectable = true;

				/* Since libusb_open always success on linux-based system */
				/* Double check whether device is occupied */
				sts = libusb_attach_kernel_driver(dev_handle, 0);

				if (KP_ERROR_USB_BUSY_N6 == sts) {
					isConnectable = false;
				}

				kdev_list->device[sidx].isConnectable = isConnectable;

				if (desc.iSerialNumber > 0)
				{
					unsigned char ser_string[16] = {0};
					unsigned int sernum = 0;

					int nbytes = libusb_get_string_descriptor_ascii(dev_handle, desc.iSerialNumber, ser_string, 16);

					if (nbytes == 8)
						sernum = (unsigned int)strtoul((const char *)ser_string, NULL, 16);

					kdev_list->device[sidx].kn_number = sernum;
				}

				libusb_close(dev_handle);
			}
			else
			{
				kdev_list->device[sidx].isConnectable = false;
				dbg_print("%s() libusb_open failed, error %d\n", __func__, sts);
			}

			++kdev_list->num_dev;
		}
	}

	libusb_free_device_list(devs_list, 1);

	__decrease_usb_refcnt();

	return kdev_list;
}

static void get_port_id_and_path(libusb_device *usbdev, uint32_t *port_id, char *port_path)
{
	uint32_t port_uuid = 0;
	uint8_t ports_number[7];
	uint8_t bus_number = libusb_get_bus_number(usbdev);
	int port_depth = libusb_get_port_numbers(usbdev, ports_number, 7);

	port_uuid |= (bus_number & 0x3); // 2 bits

	for (int i = 0; i < port_depth; i++)
		port_uuid |= ((uint32_t)ports_number[i] << (2 + i * 5));

	if (NULL != port_path)
	{
		port_path[0] = 0;
		sprintf(port_path, "%d", bus_number);

        char temp_str[5];
		for (int j = 0; j < port_depth; j++)
		{
			snprintf(temp_str, sizeof(temp_str), "-%d", ports_number[j]);
			strcat(port_path, temp_str);
		}
	}

	*port_id = port_uuid;
}

#define MAX_GROUP_DEVICE 20

int kp_usb_connect_multiple_devices_v2(int num_dev, int port_id[], kp_usb_device_t *output_devs[], int try_count)
{
	__increase_usb_refcnt();

	// this is a workaround for Faraday DFU status and for KN_NUMBER
	// special process for USB DFU devices (KL720)
	// if found, minion FW will be downloaded
	usb_dfu_scan_download();

	for (int i = 0; i < num_dev; i++)
		output_devs[i] = NULL;

	libusb_device *wanted_usbdev[MAX_GROUP_DEVICE] = {NULL};
	bool all_connectable = false;
	struct libusb_device_descriptor desc;
	libusb_device **devs_list = NULL;
	uint8_t endpoint_bulk_in = 0;
	uint8_t endpoint_bulk_out = 0;
	uint8_t endpoint_interrupt_in = 0;

	while (1)
	{
		do
		{
			if (NULL != devs_list)
				libusb_free_device_list(devs_list, 1);

			pthread_mutex_lock(&_g_mutex);
			ssize_t scan_cnt = libusb_get_device_list(NULL, &devs_list);
			pthread_mutex_unlock(&_g_mutex);

			// finding stage
			int found_count = 0;
			for (ssize_t i = 0; i < scan_cnt; ++i)
			{
				libusb_device *usbdev = devs_list[i];
				int sts = libusb_get_device_descriptor(usbdev, &desc);

				if (sts != 0 || desc.idVendor != VID_KNERON)
					continue;

				uint32_t port_uuid;
				get_port_id_and_path(usbdev, &port_uuid, NULL);

				for (int i = 0; i < num_dev; i++)
				{
					if ((uint32_t)port_id[i] == port_uuid)
					{
						struct libusb_config_descriptor *config_desc;
						const struct libusb_interface_descriptor *idesc;
						uint8_t endpoint_type;
						wanted_usbdev[found_count++] = usbdev;
						libusb_get_config_descriptor(usbdev, 0, &config_desc);

						if (0 < config_desc->bNumInterfaces) {
							idesc = config_desc->interface[0].altsetting;

							for (i = 0; i < idesc->bNumEndpoints; ++i) {
								endpoint_type = (idesc->endpoint[i].bmAttributes & 0x3);
								if (LIBUSB_TRANSFER_TYPE_BULK == endpoint_type) {
									if ((idesc->endpoint[i].bEndpointAddress & (1 << 7)) == LIBUSB_ENDPOINT_IN)
										endpoint_bulk_in = idesc->endpoint[i].bEndpointAddress;
									else {
										endpoint_bulk_out = idesc->endpoint[i].bEndpointAddress;
									}
								} else if (LIBUSB_TRANSFER_TYPE_INTERRUPT == endpoint_type) {
									endpoint_interrupt_in = idesc->endpoint[i].bEndpointAddress;
								}
							}
						}

						libusb_free_config_descriptor(config_desc);
						break;
					}
				}
			}

			if (found_count != num_dev)
				break;

			// try connect stage
			for (int i = 0; i < num_dev; ++i)
			{
				libusb_device_handle *usbdev_handle = 0;
				int sts = libusb_open(wanted_usbdev[i], &usbdev_handle);
				if (sts != 0)
					break;

				libusb_close(usbdev_handle);

				if (i == (num_dev - 1))
					all_connectable = true;
			}

		} while (0);

		if (all_connectable)
			break;

		try_count--;
		if (try_count <= 0)
			break;

		usleep(100 * 1000); // per 100 ms
	}

	if (!all_connectable)
	{
		libusb_free_device_list(devs_list, 1);
		__decrease_usb_refcnt();
		return KP_USB_RET_ERR;
	}

	// now all wanted devices are connectable !
	int ret_code = KP_USB_RET_OK;
	int num_connected = 0;

	for (int i = 0; i < num_dev; ++i)
	{
		kp_usb_device_t *dev = (kp_usb_device_t *)malloc(sizeof(kp_usb_device_t));
		if (NULL == dev)
		{
			ret_code = KP_USB_USB_NO_MEM;
			break;
		}

		libusb_device_handle *usbdev_handle = 0;

		int sts = libusb_get_device_descriptor(wanted_usbdev[i], &desc);
		if (sts != 0)
		{
			ret_code = sts;
			printf("[kp_usb] error to get device descriptor (idx %d), it should work but not !\n", i);
			break;
		}

		sts = libusb_open(wanted_usbdev[i], &usbdev_handle);
		if (sts != 0)
		{
			ret_code = sts;
			printf("[kp_usb] error to connect device (idx %d), it should work but not !\n", i);
			break;
		}

		dev->usb_handle = usbdev_handle;
		get_port_id_and_path(wanted_usbdev[i], &dev->dev_descp.port_id, dev->dev_descp.port_path);
		dev->dev_descp.isConnectable = true;
		dev->dev_descp.vendor_id = VID_KNERON;
		dev->dev_descp.product_id = desc.idProduct;
		dev->dev_descp.link_speed = (kp_usb_speed_t)libusb_get_device_speed(wanted_usbdev[i]);

		dev->fw_serial = desc.bcdDevice;
		dev->endpoint_cmd_in = endpoint_bulk_in;
		dev->endpoint_cmd_out = endpoint_bulk_out;
		dev->endpoint_log_in = endpoint_interrupt_in;

		get_fw_name_by_fw_serial(dev->dev_descp.firmware, desc.idProduct, dev->fw_serial);

		dev->dev_descp.kn_number = 0x0;
		if (desc.iSerialNumber > 0)
		{
			unsigned char ser_string[16] = {0};
			unsigned int sernum = 0;

			int nbytes = libusb_get_string_descriptor_ascii(usbdev_handle, desc.iSerialNumber, ser_string, 16);
			if (nbytes == 8)
				sernum = (unsigned int)strtoul((const char *)ser_string, NULL, 16);

			dev->dev_descp.kn_number = sernum;
		}

		if (0 != __kn_configure_usb_device(usbdev_handle))
		{
			kp_usb_disconnect_device(dev);
			ret_code = KP_USB_CONFIGURE_ERR;
			break; // error
		}

		pthread_mutex_init(&dev->mutex_send, NULL);
		pthread_mutex_init(&dev->mutex_recv, NULL);

		output_devs[num_connected++] = dev;

		__increase_usb_refcnt();
	}

	if (ret_code != KP_USB_RET_OK)
	{
		for (int i = 0; i < num_connected; i++)
		{
			kp_usb_disconnect_device(output_devs[i]);
			output_devs[i] = NULL;
		}
	}

	libusb_free_device_list(devs_list, 1);
	__decrease_usb_refcnt();

	return ret_code;
}

int kp_usb_disconnect_device(kp_usb_device_t *dev)
{
	libusb_device_handle *usbdev = dev->usb_handle;
	libusb_close(usbdev);
	__decrease_usb_refcnt();

	pthread_mutex_destroy(&dev->mutex_send);
	pthread_mutex_destroy(&dev->mutex_recv);

	free(dev);

	return KP_USB_RET_OK;
}

int kp_usb_disconnect_multiple_devices(int num_dev, kp_usb_device_t *devs[])
{
	for (int i = 0; i < num_dev; i++)
	{
		if (devs[i] != NULL)
		{
			kp_usb_disconnect_device(devs[i]);
			devs[i] = NULL;
		}
	}

	return KP_USB_RET_OK;
}

kp_device_descriptor_t *kp_usb_get_device_descriptor(kp_usb_device_t *dev)
{
	return &dev->dev_descp;
}

void kp_usb_flush_out_buffers(kp_usb_device_t *dev)
{
	void *temp_buf = NULL;

	/**
	 * FIXME: Ubuntu/Windows need to reserved enough buffer (4 MB) to receive previous redundent OS cache buffer
	 */
	uint32_t buff_size = 4 * 1024 * 1024;

	if (!temp_buf)
		temp_buf = (void *)malloc(buff_size);

	// MAX limit 4MB
	for (int i = 0; i < 8; i++)
	{
		int recv_size = 0;
		int sts = __kn_usb_bulk_in(dev, dev->endpoint_cmd_in, temp_buf, buff_size, &recv_size, 200);
		if (sts == 0) {
			continue; // maybe more ?
		} else {
			break; // we done ?
		}
	}

	if (temp_buf) {
		free(temp_buf);
	}
}

// *********************************************************************************************** //
// APIs for standard read/write data
// *********************************************************************************************** //

int kp_usb_write_data(kp_usb_device_t *dev, void *buf, int len, int timeout)
{
	pthread_mutex_lock(&dev->mutex_send);
	int ret = __kn_usb_bulk_out(dev, dev->endpoint_cmd_out, buf, len, timeout);
	pthread_mutex_unlock(&dev->mutex_send);

	return ret;
}

int kp_usb_read_data(kp_usb_device_t *dev, void *buf, int len, int timeout)
{
	int read_len;

	pthread_mutex_lock(&dev->mutex_recv);
	int sts = __kn_usb_bulk_in(dev, dev->endpoint_cmd_in, buf, len, &read_len, timeout);
	pthread_mutex_unlock(&dev->mutex_recv);

	if (sts == KP_USB_RET_OK)
		return read_len;
	else
		return sts;
}

int kp_usb_endpoint_write_data(kp_usb_device_t *dev, int endpoint, void *buf, int len, int timeout)
{
	int ret = __kn_usb_bulk_out(dev, (unsigned char)endpoint, buf, len, timeout);
	return ret;
}

int kp_usb_endpoint_read_data(kp_usb_device_t *dev, int endpoint, void *buf, int len, int timeout)
{
	int read_len;
	int sts = __kn_usb_bulk_in(dev, (unsigned char)endpoint, buf, len, &read_len, timeout);
	if (sts == KP_USB_RET_OK)
		return read_len;
	else
		return sts;
}

// *********************************************************************************************** //
// APIs for standard read/write in command mode
// *********************************************************************************************** //

int kp_usb_control(kp_usb_device_t *dev, kp_usb_control_t *control_request, int timeout)
{
	uint8_t bmRequestType = 0x40;
	uint8_t bRequest = control_request->command;
	uint16_t wValue = control_request->arg1;
	uint16_t wIndex = control_request->arg2;
	unsigned char *data = NULL;
	uint16_t wLength = 0;

	int ret = libusb_control_transfer(dev->usb_handle, bmRequestType, bRequest, wValue, wIndex, data, wLength, timeout);

	return ret;
}

int kp_usb_read_firmware_log(kp_usb_device_t *dev, void *buf, int len, int timeout)
{
	int read_len;
	int sts = __kn_usb_interrupt_in(dev, dev->endpoint_log_in, buf, len, &read_len, timeout);

	if (sts == KP_USB_RET_OK)
		return read_len;
	else
		return sts;
}
