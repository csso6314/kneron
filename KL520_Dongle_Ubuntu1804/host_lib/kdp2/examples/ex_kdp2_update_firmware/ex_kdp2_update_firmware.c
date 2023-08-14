#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kdp2_core.h"

// some given default settings for a demo
// user can change below settings or using command parameters to change them
static int _scan_index = 1;
static char _folder_path[] = "../../app_binaries/KL520/kdp2/";

void print_settings()
{
    printf("\n");
    printf("-h : help\n");
    printf("-s : [device index] = '%d'\n", _scan_index);
    printf("-p : [firmware folder path] = '%s'\n", _folder_path);
    printf("\n");
}

bool parse_arguments(int argc, char *argv[])
{
    int opt = 0;
    while ((opt = getopt(argc, argv, "hs:p:")) != -1)
    {
        switch (opt)
        {
        case 's':
            _scan_index = atoi(optarg);
            break;
        case 'p':
            strcpy(_folder_path, optarg);
            break;
        case 'h':
        default:
            print_settings();
            exit(0);
        }
    }

    return true;
}

int update_scpu_firmware(const char *port_path)
{
    printf("\n");

    kdp2_device_t device = NULL;

    // let's try a few times to connect the device.
    for (int i = 0; i < 5; i++)
    {
        device = kdp2_connect_device_by_port_path(port_path);
        if (device)
            break;

        sleep(1);
    }

    if (!device)
    {
        printf("error ! connect to the device failed, please check it\n");
        return -1;
    }

    char fw_path[256];

    sprintf(fw_path, "%s/fw_scpu.bin", _folder_path);

    printf("update SCPU firmware from file %s\n", fw_path);

    // if update done, the device will reboot by itself
    int ret = kdp2_update_firmware_from_file(device, KDP2_FIRMWARE_SCPU, fw_path);

    kdp2_disconnect_device(device);

    if (ret == KDP2_SUCCESS)
    {
        printf("update SCPU firmware OK\n");
        return 0;
    }
    else
    {
        printf("update SCPU firmware failed, error = %d\n", ret);
        return -1;
    }
}

int update_ncpu_firmware(const char *port_path)
{
    printf("\n");

    kdp2_device_t device = NULL;

    // let's try a few times to connect the device.
    for (int i = 0; i < 50; i++)
    {
        device = kdp2_connect_device_by_port_path(port_path);
        if (device)
            break;

        sleep(1);
    }

    if (!device)
    {
        printf("error ! connect to the device failed, please check it\n");
        return -1;
    }

    char fw_path[256];

    sprintf(fw_path, "%s/fw_ncpu.bin", _folder_path);

    printf("update NCPU firmware from file %s\n", fw_path);

    // if update done, the device will reboot by itself
    int ret = kdp2_update_firmware_from_file(device, KDP2_FIRMWARE_NCPU, fw_path);

    kdp2_disconnect_device(device);

    if (ret == KDP2_SUCCESS)
    {
        printf("update NCPU firmware OK\n");
        return 0;
    }
    else
    {
        printf("update NCPU firmware failed, error = %d\n", ret);
        return -1;
    }
}

int main(int argc, char *argv[])
{
    kdp2_devices_list_t *list;
    char port_path[20] = "";

    parse_arguments(argc, argv);
    print_settings();

    kdp2_scan_devices(&list);

    if (_scan_index > list->num_dev)
    {
        printf("device cannot be found for the index\n");
        free(list);
        return -1;
    }

    for (int i = 0; i < list->num_dev; i++)
    {
        if (_scan_index == list->device[i].scan_index)
        {
            // we found it, but need to check if connectable
            if (list->device[i].isConnectable == false)
            {
                printf("device is not connectable\n");
                free(list);
                return -1;
            }

            // here remember the device path of the target device
            strcpy(port_path, list->device[i].port_path);
            break;
        }
    }

    free(list);

    if (strlen(port_path) == 0)
    {
        printf("device is not available\n");
        return -1;
    }

    printf("start to update SCPU and NCPU firmwares to the device with port_path = %s\n", port_path);

    update_scpu_firmware(port_path);

    // rebooting

    sleep(2);

    update_ncpu_firmware(port_path);

    return 0;
}
