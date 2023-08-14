#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kdp2_core.h"

int main(int argc, char *argv[])
{
    printf("\n");

    if (argc < 2)
    {
        printf("usage:\n");
        printf("%s -s [device scan index] (can be known by kdp2_scan_devices())\n", argv[0]);
        printf("%s -p [product_name] (KL520, KL720 or after)\n", argv[0]);
        printf("%s -k [kn_number] (ex: 0x7F1A0000)\n", argv[0]);
        printf("\n");
        return -1;
    }

    kdp2_device_t device;
    int opt = 0;

    while ((opt = getopt(argc, argv, "s:p:k:")) != -1)
    {
        switch (opt)
        {
        case 's':
        {
            int scan_index = atoi(optarg);
            printf("connecting to device by scan_index = %d ...\n", scan_index);
            device = kdp2_connect_device(scan_index);
            break;
        }
        case 'p':
        {
            uint32_t product_id = 0;
            printf("connecting to device by product_id = %s ...\n", optarg);

            if (strcmp(optarg, "KL520") == 0)
                product_id = KDP2_DEVICE_KL520;
            if (strcmp(optarg, "KL720") == 0)
                product_id = KDP2_DEVICE_KL720;

            device = kdp2_connect_device_by_product(product_id);
            break;
        }
        case 'k':
        {
            uint32_t kn_num = strtol(optarg, NULL, 16);
            printf("connecting to device by kn_number = 0x%X ...\n", kn_num);

            device = kdp2_connect_device_by_kn_number(kn_num);
            break;
        }
        default:
            printf("wrong argument!\n");
            return -1;
        }
    }

    if (!device)
    {
        printf("error ! connect to the device failed, please check it\n");
        return -1;
    }

    kdp2_device_descriptor_t *dev_descp = kdp2_get_device_descriptor(device);

    int sidx = dev_descp->scan_index;

    printf("\n");
    printf("[%d] scan_index: '%d'\n", sidx, dev_descp->scan_index);
    printf("[%d] product_id: '0x%x'", sidx, dev_descp->product_id);

    if (dev_descp->product_id == KDP2_DEVICE_KL520)
        printf(" (KL520)");
    else if (dev_descp->product_id == KDP2_DEVICE_KL720)
        printf(" (KL720)");
    printf("\n");

    printf("[%d] USB link speed: ", sidx);
    switch (dev_descp->link_speed)
    {
    case KDP2_USB_SPEED_LOW:
        printf("'Low-Speed'\n");
        break;
    case KDP2_USB_SPEED_FULL:
        printf("'Full-Speed'\n");
        break;
    case KDP2_USB_SPEED_HIGH:
        printf("'High-Speed'\n");
        break;
    case KDP2_USB_SPEED_SUPER:
        printf("'Super-Speed'\n");
        break;
    default:
        printf("'Unknown'\n");
        break;
    }
    printf("[%d] USB port path: '%s'\n", sidx, dev_descp->port_path);
    printf("[%d] kn_number: '0x%X' %s\n", sidx, dev_descp->kn_number, dev_descp->kn_number == 0x0 ? "(invalid)" : "");

    printf("\n");
    printf(">> press 'enter' to disconnect the device ...\n");

    fgetc(stdin);

    printf("disconnecting device ...\n");

    kdp2_disconnect_device(device);

    return 0;
}
