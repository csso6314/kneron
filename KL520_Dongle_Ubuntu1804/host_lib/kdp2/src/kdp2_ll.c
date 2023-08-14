#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <libusb-1.0/libusb.h>
#include "kdp2_ll.h"

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) printf(format, ##__VA_ARGS__)
#else
#define dbg_print(format, ...)
#endif

#define VID_KNERON 0x3231

// for command mode communication
#define USBD_ENDPOINT_CMD_OUT 0x02
#define USBD_ENDPOINT_CMD_IN 0x81

// for data/image communication
#define USBD_ENDPOINT_DATA_OUT 0x03
#define USBD_ENDPOINT_DATA_IN 0x84

#define MAX_TXFER_SIZE (2 * 1024 * 1024)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// *********************************************************************************************** //
// Below are internal or static data structure or functions
// *********************************************************************************************** //

static int _g_libusb_ref_count = 0; // reference count of libusb

typedef struct
{
	libusb_device_handle *usb_handle;
	kdp2_device_descriptor_t dev_descp;
} _kn_usb_device_t;

static int __kn_usb_bulk_out(_kn_usb_device_t *kndev, unsigned char endpoint, void *buf, int length, unsigned int timeout)
{
	int status;
	int transferred;

	int total_txfer = length;
	int one_txfer = 0;
	uintptr_t cur_write_address = (uintptr_t)buf;

	libusb_device_handle *usbdev = kndev->usb_handle;
	int speed = kndev->dev_descp.link_speed;
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
		int len;
		int transferred;

		if (kndev->dev_descp.product_id == KDP2_DEVICE_KL720)
		{
			// use fake ZLP as workaround
			zlp_buf = 0x11223344;
			len = 4;
		}
		else
		{
			len = 0; // zero length as ZLP
		}

		status = libusb_bulk_transfer(usbdev, endpoint, (unsigned char *)&zlp_buf, len, &transferred, timeout);

		if (status != 0)
		{
			dbg_print("[%s] [kdp2_usb] send fake ZLP failed error: %s\n", __func__, libusb_strerror((enum libusb_error)status));
			return status;
		}
	}

	return KDP2_LL_RET_OK;
}

static int __kn_usb_bulk_in(_kn_usb_device_t *kndev, unsigned char endpoint, void *buf, int buf_size, int *recv_size, unsigned int timeout)
{
	libusb_device_handle *usbdev = kndev->usb_handle;

	int status;
	int transferred;
	uintptr_t cur_read_address = (uintptr_t)buf;
	int speed = kndev->dev_descp.link_speed;
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
			//printf("[kdp2_usb] recv data failed error: %s\n", libusb_strerror((enum libusb_error)status));
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
			//printf("[kdp2_usb] libusb_bulk_transfer ZLP failed error: %s\n", libusb_strerror((enum libusb_error)status));
			return status;
		}

		if (transferred != 0)
		{
			dbg_print("[%s] [kdp2_usb] error, should be ZLP !!\n", __func__);
			return KDP2_LL_RET_ERR;
		}
	}

	return KDP2_LL_RET_OK;
}

static void __increase_usb_refcnt()
{
	if (_g_libusb_ref_count == 0)
	{
		if (0 != libusb_init(NULL))
			// FIXME: do something ?
			dbg_print("[%s] [kdp2_usb] libusb_init() failed\n", __func__);
	}
	++_g_libusb_ref_count;
}

static void __decrease_usb_refcnt()
{
	--_g_libusb_ref_count;
	if (_g_libusb_ref_count == 0)
		libusb_exit(NULL);
}

static _kn_usb_device_t *__kn_open_usb_device(int scan_index)
{
	__increase_usb_refcnt();

	libusb_device **devs_list;

	ssize_t cnt = libusb_get_device_list(NULL, &devs_list);
	if (cnt < 0)
	{
		dbg_print("[%s] [kdp2_usb] libusb_get_device_list() failed\n", __func__);
		__decrease_usb_refcnt();
		return 0;
	}

	libusb_device *usbdev = 0;
	libusb_device_handle *usbdev_handle = 0;
	int found_order = 0;

	int i = 0;
	_kn_usb_device_t *kndev = 0;

	while ((usbdev = devs_list[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(usbdev, &desc);
		if (r < 0)
			continue;

		if (desc.idVendor == VID_KNERON)
		{
			++found_order;

			if (found_order == scan_index)
			{
				int sts = libusb_open(usbdev, &usbdev_handle);
				if (sts == 0)
				{
					kndev = (_kn_usb_device_t *)malloc(sizeof(_kn_usb_device_t));

					kndev->usb_handle = usbdev_handle;
					kndev->dev_descp.scan_index = scan_index;
					kndev->dev_descp.isConnectable = true;
					kndev->dev_descp.vendor_id = desc.idVendor;
					kndev->dev_descp.product_id = desc.idProduct;
					kndev->dev_descp.link_speed = (kdp2_usb_speed_t)libusb_get_device_speed(usbdev);

					kndev->dev_descp.port_path[0] = 0;
					sprintf(kndev->dev_descp.port_path, "%d", libusb_get_bus_number(usbdev));

					uint8_t ports[7];
					int pnum = libusb_get_port_numbers(usbdev, ports, 7);
					for (int j = 0; j < pnum; j++)
					{
						char temp_str[4];
						sprintf(temp_str, "-%d", ports[j]);
						strcat(kndev->dev_descp.port_path, temp_str);
					}

					kndev->dev_descp.kn_number = 0x0;
					if (desc.iSerialNumber > 0)
					{
						unsigned char ser_string[16];
						unsigned int sernum = 0;

						int nbytes = libusb_get_string_descriptor_ascii(usbdev_handle, desc.iSerialNumber, ser_string, 16);
						if (nbytes == 8)
							sernum = (unsigned int)strtol((const char *)ser_string, NULL, 16);

						kndev->dev_descp.kn_number = sernum;
					}

					__increase_usb_refcnt();
				}
				else
				{
					dbg_print("[%s] [kdp2_usb] libusb_open() failed, sts %d\n", __func__, sts);
					usbdev_handle = 0; // just in case
				}

				break;
			}
		}
	}

	libusb_free_device_list(devs_list, 1);

	__decrease_usb_refcnt();

	return kndev;
}

static int __kn_configure_usb_device(libusb_device_handle *usbdev_handle)
{
	int status;
	status = libusb_set_configuration(usbdev_handle, 1);
	if (status)
	{
		dbg_print("[%s] [kdp2_usb] set config failed: %s\n", __func__, libusb_strerror((enum libusb_error)status));
		return -1;
	}

	status = libusb_claim_interface(usbdev_handle, 0);
	if (status)
	{
		dbg_print("[%s] [kdp2_usb] libusb_claim_interface() failed: %s\n", __func__, libusb_strerror((enum libusb_error)status));
		return -2;
	}

	return 0;
}

// *********************************************************************************************** //
// APIs for device initialization
// *********************************************************************************************** //

int kdp2_ll_scan_devices(kdp2_devices_list_t **pkdev_list)
{
	libusb_device **devs_list;
	ssize_t cnt;

	__increase_usb_refcnt();

	cnt = libusb_get_device_list(NULL, &devs_list);
	if (cnt < 0)
	{
		__decrease_usb_refcnt();
		return KDP2_LL_RET_ERR;
	}

	kdp2_devices_list_t *kdev_list = (kdp2_devices_list_t *)malloc(sizeof(int) + cnt * sizeof(kdp2_device_descriptor_t));
	kdev_list->num_dev = 0;
	*pkdev_list = kdev_list;

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

			kdev_list->device[sidx].scan_index = sidx + 1;
			kdev_list->device[sidx].vendor_id = desc.idVendor;
			kdev_list->device[sidx].product_id = desc.idProduct;
			kdev_list->device[sidx].link_speed = (kdp2_usb_speed_t)libusb_get_device_speed(dev);

			kdev_list->device[sidx].port_path[0] = 0;
			sprintf(kdev_list->device[sidx].port_path, "%d", libusb_get_bus_number(dev));

			uint8_t ports[7];
			int pnum = libusb_get_port_numbers(dev, ports, 7);
			for (int j = 0; j < pnum; j++)
			{
				char temp_str[4];
				sprintf(temp_str, "-%d", ports[j]);
				strcat(kdev_list->device[sidx].port_path, temp_str);
			}

			kdev_list->device[sidx].kn_number = 0x0;

			libusb_device_handle *dev_handle = 0;
			int sts = libusb_open(dev, &dev_handle);
			if (sts == 0)
			{
				kdev_list->device[sidx].isConnectable = true;

				if (desc.iSerialNumber > 0)
				{
					unsigned char ser_string[16];
					unsigned int sernum = 0;

					int nbytes = libusb_get_string_descriptor_ascii(dev_handle, desc.iSerialNumber, ser_string, 16);
					if (nbytes == 8)
						sernum = (unsigned int)strtol((const char *)ser_string, NULL, 16);

					kdev_list->device[sidx].kn_number = sernum;
				}

				libusb_close(dev_handle);
			}
			else
			{
				kdev_list->device[sidx].isConnectable = false;
				printf("%s() libusb_open failed, error %d\n", __func__, sts);
			}

			++kdev_list->num_dev;
		}
	}

	libusb_free_device_list(devs_list, 1);

	__decrease_usb_refcnt();

	return KDP2_LL_RET_OK;
}

kdp2_ll_device_t kdp2_ll_connect_device(int scan_index)
{
	_kn_usb_device_t *kndev = __kn_open_usb_device(scan_index);
	if (kndev == 0)
		return 0;

	if (0 != __kn_configure_usb_device(kndev->usb_handle))
	{
		kdp2_ll_disconnect_device((kdp2_ll_device_t)kndev);
		return 0;
	}

	return (kdp2_ll_device_t)kndev;
}

int kdp2_ll_disconnect_device(kdp2_ll_device_t dev)
{
	_kn_usb_device_t *kndev = (_kn_usb_device_t *)dev;
	libusb_device_handle *usbdev = kndev->usb_handle;
	libusb_close(usbdev);
	__decrease_usb_refcnt();

	free(kndev);

	return KDP2_LL_RET_OK;
}

kdp2_device_descriptor_t *kdp2_ll_get_device_descriptor(kdp2_ll_device_t dev)
{
	_kn_usb_device_t *kndev = (_kn_usb_device_t *)dev;
	return &kndev->dev_descp;
}

// *********************************************************************************************** //
// APIs for standard read/write data
// *********************************************************************************************** //

int kdp2_ll_write_data(kdp2_ll_device_t dev, void *buf, int len, int timeout)
{
	return __kn_usb_bulk_out((_kn_usb_device_t *)dev, USBD_ENDPOINT_DATA_OUT, buf, len, timeout);
}

int kdp2_ll_read_data(kdp2_ll_device_t dev, void *buf, int len, int timeout)
{
	int read_len;
	int sts = __kn_usb_bulk_in((_kn_usb_device_t *)dev, USBD_ENDPOINT_DATA_IN, buf, len, &read_len, timeout);

	if (sts == KDP2_LL_RET_OK)
		return read_len;
	else
		return sts;
}

// *********************************************************************************************** //
// APIs for standard read/write in command mode
// *********************************************************************************************** //

int kdp2_ll_write_command(kdp2_ll_device_t dev, void *buf, int len, int timeout)
{
	return __kn_usb_bulk_out((_kn_usb_device_t *)dev, USBD_ENDPOINT_CMD_OUT, buf, len, timeout);
}

int kdp2_ll_read_command(kdp2_ll_device_t dev, void *buf, int len, int timeout)
{
	int read_len;
	int sts = __kn_usb_bulk_in((_kn_usb_device_t *)dev, USBD_ENDPOINT_CMD_IN, buf, len, &read_len, timeout);

	if (sts == KDP2_LL_RET_OK)
		return read_len;
	else
		return sts;
}

// *********************************************************************************************** //
// APIs for standard read/write in command mode
// *********************************************************************************************** //

int kdp2_ll_control(kdp2_ll_device_t dev, kdp2_ll_control_t *control_request, int timeout)
{
	_kn_usb_device_t *kndev = (_kn_usb_device_t *)dev;

	uint8_t bmRequestType = 0x40;
	uint8_t bRequest = control_request->command;
	uint16_t wValue = control_request->arg1;
	uint16_t wIndex = control_request->arg2;
	unsigned char *data = NULL;
	uint16_t wLength = 0;

	int ret = libusb_control_transfer(kndev->usb_handle, bmRequestType, bRequest, wValue, wIndex, data, wLength, timeout);

	return ret;
}
