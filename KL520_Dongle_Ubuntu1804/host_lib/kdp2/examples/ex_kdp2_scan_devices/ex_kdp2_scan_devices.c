#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kdp2_core.h"

int main(int argc, char *argv[])
{
    printf("\n");
    printf("scanning kneron devices ...\n");

    kdp2_devices_list_t *list;
    kdp2_scan_devices(&list);

    printf("number of Kneron devices found: %d\n", list->num_dev);

    if (list->num_dev == 0)
        return 0;

    printf("\n");
    printf("listing devices infomation as follows:\n");

    for (int i = 0; i < list->num_dev; i++)
    {
        kdp2_device_descriptor_t *dev_descp = &(list->device[i]);

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
        printf("[%d] Connectable: '%s'\n", sidx, dev_descp->isConnectable == true ? "True" : "False");
    }

    free(list);

    return 0;
}
