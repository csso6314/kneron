/**
 * @file        generic_inference.c
 * @brief       main code of generic inference example - output raw data
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "kp_core.h"
#include "helper_functions.h"

#define MAX_DEV_COUNT 10       /* MAX device count to connect */
enum FILTER_TYPE_E {
    FILTER_NOT_SET,
    FILTER_BY_TARGET,
    FILTER_BY_SCAN_INDEX,
    FILTER_BY_PORT_ID,
    FILTER_BY_KN,
    FILTER_ERROR
};
static int _filter_type = FILTER_NOT_SET;
static char _filter_settings[128] = {};

void print_settings()
{
    printf("connect_devices example\n");
    printf("\n");
    printf("  show different filter ways of finding target devices to connect\n");
    printf("  Notice that it is not allowed to connect different target platform devices into a deivce group\n");
    printf("\n");
    printf("Arguments:\n");
    printf("-help, h   : print help message\n");
    printf("-target, t : [by target platform] = (KL520|KL720|KL630)\n");
    printf("-sidx, s   : [by scan index set] = (specified scan index set, can also be \"0,1,2\" for multiple devices)\n");
    printf("-port, p   : [by port id set] = (specified port id set, can also be \"13,537\" for multiple devices)\n");
    printf("-kn, k     : [by KN number set] = (specified KN number set, scan also be \"0x1111aaaa, 0x2222bbbb\" for multiple devices)\n");
    printf("\n");

    return;
}

bool parse_arguments(int argc, char *argv[])
{
    int opt = 0;

    static struct option long_options[] = {
        {"help",   no_argument,       0, 'h'},
        {"target", optional_argument, 0, 't'},
        {"sidx",   optional_argument, 0, 's'},
        {"port",   optional_argument, 0, 'p'},
        {"kn",     optional_argument, 0, 'k'},
        {0, 0, 0, 0}};

    int option_index = 0;
    bool helper_specified = false;

    while ((opt = getopt_long(argc, argv, "ht:s:p:k:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 0:
            break;
        case 't':
            if(_filter_type != FILTER_NOT_SET) {
                printf("\n\nError! This arg must be exclusive to other args\n\n");
                helper_specified = true;
                break;
            }

            _filter_type = FILTER_BY_TARGET;
            strcpy(_filter_settings, optarg);
            break;
        case 's':
            if(_filter_type != FILTER_NOT_SET) {
                printf("\n\nError! This arg must be exclusive to other args\n\n");
                helper_specified = true;
                break;
            }

            _filter_type = FILTER_BY_SCAN_INDEX;
            strcpy(_filter_settings, optarg);
            break;
        case 'p':
            if(_filter_type != FILTER_NOT_SET) {
                printf("\n\nError! This arg must be exclusive to other args\n\n");
                helper_specified = true;
                break;
            }

            _filter_type = FILTER_BY_PORT_ID;
            strcpy(_filter_settings, optarg);
            break;
        case 'k':
            if(_filter_type != FILTER_NOT_SET) {
                printf("\n\nError! This arg must be exclusive to other args\n\n");
                helper_specified = true;
                break;
            }

            _filter_type = FILTER_BY_KN;
            strcpy(_filter_settings, optarg);
            break;
        case 'h':
        case '?':
        default:
            helper_specified = true;
        }
    }

    if (helper_specified)
    {
        // print settings after all argv are parsed
        print_settings();
        exit(0);
    }

    return true;
}


static void collect_all_port_ids(kp_devices_list_t *list, int *dev_port_ids, int *num_devices, kp_product_id_t target_product)
{
    // want to connect all connectable KDP2 FW devices with specified target platform
    for (int i = 0; i < list->num_dev; i++)
    {
        if (list->device[i].product_id != target_product)
        {
            continue;
        }

        if (list->device[i].isConnectable && strncmp(list->device[i].firmware, "KDP2", 4) == 0)
        {
            dev_port_ids[(*num_devices)++] = list->device[i].port_id;
            printf("connect target: index '%d', port ID '%d'\n", i, list->device[i].port_id);
        }
    }
}

/**
 * return true means invalid = some port_ids are connecteble KL520 and anothers are connectable KL720
 */
static bool _check_if_dev_port_ids_valid(kp_devices_list_t* list_p, int *dev_port_ids_to_test, int *num_devices)
{
    kp_product_id_t first_found_target_product = 0;
    int updated_num_devices = 0;
    bool is_found;

    int i, j;
    for (i = 0; i < *num_devices ;i++) {
        is_found = false;
        for (j = 0; j < list_p->num_dev; j++) {
            if (list_p->device[j].port_id == dev_port_ids_to_test[i])
            {
                is_found = true;
                // check if valid: connectable and with KDP2 related firmware
                if (list_p->device[j].isConnectable && strncmp(list_p->device[j].firmware, "KDP2", 4) == 0)
                {
                    dev_port_ids_to_test[updated_num_devices++] = list_p->device[j].port_id;
                    printf("to connect target: scan index '%d', port ID '%d'\n", i, list_p->device[j].port_id);

                    // check if specified port ids in different device type
                    if( 0 == first_found_target_product )
                        first_found_target_product = list_p->device[j].product_id;
                    else if(first_found_target_product != list_p->device[j].product_id)
                        return true;  //invalid

                    continue;
                }
                else {
                    printf("Removed not connecteable or not KDP2 releated FW device: port id = %d\n", dev_port_ids_to_test[i]);
                }
            }
        }

        if(! is_found )
            printf("Error! port_id not found = %d\n", dev_port_ids_to_test[i]);

    }

    *num_devices = updated_num_devices;
    return false;
}

static kp_device_group_t connect_devices(kp_devices_list_t* list_p, enum FILTER_TYPE_E filter_type_p, char* filter_setting_p)
{
    kp_device_group_t devices_group = NULL;

    int num_devices = 0;
    int dev_port_ids[MAX_DEV_COUNT] = {0};

    int filter_setting_items[MAX_DEV_COUNT] = {0};
    int num_items = 0;
    int i, j;
    bool is_found;
    int error_code;

    switch (filter_type_p)
    {
    case FILTER_BY_TARGET:
        if (0 == strcmp(filter_setting_p, "KL520"))
            collect_all_port_ids(list_p, dev_port_ids, &num_devices, KP_DEVICE_KL520);
        else if (0 == strcmp(filter_setting_p, "KL720"))
            collect_all_port_ids(list_p, dev_port_ids, &num_devices, KP_DEVICE_KL720);
        else if (0 == strcmp(filter_setting_p, "KL630"))
            collect_all_port_ids(list_p, dev_port_ids, &num_devices, KP_DEVICE_KL630);
        else
            printf("ERROR! Wrong device type string is given = %s\n", filter_setting_p);
        break;
    case FILTER_BY_PORT_ID:
        // If the input string is not a number array, dev_port_ids[0] == 0 and num_devices = 1
        helper_string_to_number_array(filter_setting_p, dev_port_ids, &num_devices);

        //check if specified scan indexes are in to differnt device targets
        if(dev_port_ids[0] != 0 &&
             _check_if_dev_port_ids_valid(list_p, dev_port_ids/*in/output*/, &num_devices/*in/output*/) )
        {
           printf("Error, found specified devices contains different Kneron device type\n");
           exit(0);
        }
        break;

    case FILTER_BY_SCAN_INDEX:
        // If the input string is not a number array, filter_setting_items[0] == 0 and num_devices = 1
        helper_string_to_number_array(filter_setting_p, filter_setting_items, &num_items);

        // convert scan_index to port_ids
        for(i = 0; i< num_items ;i++) {
            if ( filter_setting_items[i] >= list_p->num_dev) {
                printf("Error! specified scan_index not exiseted = %d\n", filter_setting_items[i]);
                continue;
            }

            dev_port_ids[num_devices++] = list_p->device[filter_setting_items[i]].port_id;
            printf("scan_index=%d -> port_id=%d\n", filter_setting_items[i], list_p->device[filter_setting_items[i]].port_id);
        }

        // check if port_ids are in to differnt device targets
        if(  _check_if_dev_port_ids_valid(list_p, dev_port_ids/*in/output*/, &num_devices/*in/output*/) )
        {
           printf("Error, found specified devices contains different Kneron device type\n");
           exit(0);
        }
        break;

    case FILTER_BY_KN:
    {
        // If the input string is not a number array, filter_setting_items[0] == 0 and num_devices = 1
        char *token;
        long kn_num;

        /* get the first token */
        token = strtok(filter_setting_p, ",");

        /* walk through other tokens */
        while (token != NULL)
        {
            //save kn numbers as integers
            kn_num = strtoll(token, NULL, 0);
            filter_setting_items[num_items++] = (unsigned int)kn_num;

            token = strtok(NULL, ",");
        }

        // convert kn to port_ids
        for(i = 0; i< num_items ;i++) {
            is_found=false;
            for (j=0;j<list_p->num_dev;j++){
                if ( filter_setting_items[i] == list_p->device[j].kn_number) {
                    is_found = true;

                    dev_port_ids[num_devices++] = list_p->device[j].port_id;
                    printf("kn_number=0x%x-> port_id=%d\n", filter_setting_items[i], list_p->device[j].port_id);
                }
            }
            if(!is_found)
                printf("Error! specified kn_number not existed = 0x%x\n", filter_setting_items[i]);
        }

        // check if port_ids are in to differnt device targets
        if(  _check_if_dev_port_ids_valid(list_p, dev_port_ids/*in/output*/, &num_devices/*in/output*/) )
        {
           printf("Error, found specified devices contains different Kneron device type\n");
           exit(0);
        }
        break;
    }
    default:
        printf("ERROR! Wrong filter type\n");
        break;
    }

    if(num_devices) {
        devices_group = kp_connect_devices(num_devices, dev_port_ids, &error_code);
        if (!devices_group)
        {
            printf("error ! connect device(s) failed, error = %d (%s)\n", error_code, kp_error_string(error_code));
            exit(0);
        }
    }

    printf("\n");
    printf("connect %d device(s) ... OK\n\n", num_devices);

    return devices_group;
}

int main(int argc, char *argv[])
{
    // parsing command arguments
    parse_arguments(argc, argv);
    print_settings();


    // default setting
    if(FILTER_NOT_SET == _filter_type) {
        _filter_type = FILTER_BY_SCAN_INDEX;
        _filter_settings[0] = '0';
    }


    // scan devices
    kp_devices_list_t *list;
    list = kp_scan_devices();


    // connect KDP2 devices
    kp_device_group_t devices_group = NULL;
    devices_group = connect_devices(list, _filter_type, _filter_settings);
    if(NULL == devices_group)
        return 0;


    // do any operation
    printf("operation: set transfer timeout as 5 seconds for connected devices\n");
    kp_set_timeout(devices_group, 5000);    // 5 secs timeout for usb transfer


    // disconnet devices
    printf("\n");
    printf("disconnected devices\n");
    kp_disconnect_devices(devices_group);

    return 0;
}
