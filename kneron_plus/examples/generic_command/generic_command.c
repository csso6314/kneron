/**
 * @file        generic_command.c
 * @brief       example of generic commands for system/model informaiton
 * @version     0.1
 * @date        2021-04-28
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

// some given default settings for a demo
// user can change below settings or using command parameters to change them
static char _target_platform[12] = "default";
static char _scan_index_str[64] = "default";
static char _port_id_str[64] = "default";
static char _command_type[64] = "default";

static kp_device_group_t _device = NULL;
static int _port_id = 0;

void print_settings()
{
    printf("\n");
    printf("[note - using default parameter values if no value is passed]\n");
    printf("-target: [target platform] (KL520, KL720, KL630) = %s (default: KL520)\n", _target_platform);
    printf("-sidx  : [scan index] = %s (specified scan index) (default: scan index of the first scanned Kneron device)\n", _scan_index_str);
    printf("-port  : [port id] = %s (specified port id) (default: port ID of the first scanned Kneron device)\n", _port_id_str);
    printf("         Notice that scan index has higher priority than port id\n");
    printf("-cmd   : [command type] (system-showSystemInfo, model-showModelInfo, reboot, shutdown) = %s (default: system)\n", _command_type);
    printf("         Notice that shutdown command is not supported by KL720\n");
    printf("\n");
}

bool parse_arguments(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"target", required_argument, 0, 0},
        {"sidx", required_argument, 0, 0},
        {"port", required_argument, 0, 0},
        {"cmd", required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;

    int opt = 0;
    while ((opt = getopt_long_only(argc, argv, "h", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 0:
            if (0 == strcmp("target", long_options[option_index].name))
            {
                strcpy(_target_platform, optarg);
            }
            else if (0 == strcmp("sidx", long_options[option_index].name))
            {
                strcpy(_scan_index_str, optarg);
            }
            else if (0 == strcmp("port", long_options[option_index].name))
            {
                strcpy(_port_id_str, optarg);
            }
            else if (0 == strcmp("cmd", long_options[option_index].name))
            {
                strcpy(_command_type, optarg);
            }
            break;
        case 'h':
        default:
            print_settings();
            exit(0);
        }
    }

    return true;
}

static void connect_devices()
{
    /******* connect the device *******/
    kp_devices_list_t *list;

    kp_product_id_t target_product;
    int error_code;

    if  (0 == strcmp(_target_platform, "default"))
        target_product = KP_DEVICE_KL520;
    else if (0 == strcmp(_target_platform, "KL520"))
        target_product = KP_DEVICE_KL520;
    else if (0 == strcmp(_target_platform, "KL720"))
        target_product = KP_DEVICE_KL720;
    else if (0 == strcmp(_target_platform, "KL630"))
        target_product = KP_DEVICE_KL630;
    else
    {
        printf("error ! unsupported target platform !\n");
        exit(0);
    }

    // scan devices to loop up each device's port ID
    list = kp_scan_devices();

    if (list->num_dev < 1)
    {
        printf("error ! no Kneron device !\n");
        exit(0);
    }

    int scan_index = 0;

    // Connecting by scan index has higher priority than port id
    if (0 == strcmp(_scan_index_str, "default"))
    {
        if (0 != strcmp(_port_id_str, "default"))
        {
            _port_id = atoi(_port_id_str);

            // Find corresponding scan index
            for (scan_index = 0; scan_index < list->num_dev; scan_index++)
            {
                if (_port_id == list->device[scan_index].port_id)
                    break;
            }

            if (scan_index == list->num_dev)
            {
                printf("error ! invalid port_id '%d' !\n", _port_id);
                exit(0);
            }
        }
        else
            _port_id = list->device[scan_index].port_id;
    }
    else
    {
        scan_index = atoi(_scan_index_str);
        if (scan_index >= list->num_dev)
        {
            printf("error ! invalid scan_index '%d' !\n", scan_index);
            exit(0);
        }

        _port_id = list->device[scan_index].port_id;
    }

    if (0 == strcmp(list->device[scan_index].firmware, "KDP"))
    {
        printf("error ! the firmware type of device with index '%d', port ID '%d' is not supported !\n",
                scan_index, _port_id);
        exit(0);
    }

    if (list->device[scan_index].product_id != target_product)
    {
        printf("error ! device with index '%d', port ID '%d' does not match target platform (%s) !\n",
                scan_index, _port_id, _target_platform);
        exit(0);
    }

    printf("connect target: index '%d', port ID '%d'\n", scan_index, _port_id);

    // connect one device with specified port_id and target product id
    // note that this API function is for high level force connection to Kneron devices, not recommended in common use
    _device = kp_connect_devices_without_check(1, &_port_id, &error_code);
    if (!_device)
    {
        printf("error ! connect device failed, port ID = '%d', error = '%d'\n", _port_id, error_code);
        exit(0);
    }

    kp_set_timeout(_device, 5000);

    printf("connect device ... OK\n");

    if ((0 == strncmp(list->device[scan_index].firmware, "KDP2 Loader", strlen("KDP2 Loader"))) ||
        (NULL == strstr(list->device[scan_index].firmware, "KDP2")))
    {
        printf("\n");
        printf("error ! device is not running KDP2 firmware ...\n");
        printf("please upload firmware first via 'kp_load_firmware_from_file()'\n");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    // parsing command arguments
    parse_arguments(argc, argv);
    print_settings();

    int ret;

    connect_devices();

    // execute KDP2 command

    if ((0 == strcmp(_command_type, "system")) || (0 == strcmp(_command_type, "default")))
    {
        kp_system_info_t system_info;
        ret = kp_get_system_info(_device, _port_id, &system_info);
        if (ret == KP_SUCCESS)
        {
            printf("\nkn_number:    0x%8X\n", system_info.kn_number);

            if (system_info.firmware_version.reserved != 0) // For backward compatibility
            {
                printf("FW_version: %d.%d.%d.%d-build.%d\n",
                   system_info.firmware_version.reserved,
                   system_info.firmware_version.major,
                   system_info.firmware_version.minor,
                   system_info.firmware_version.update,
                   system_info.firmware_version.build);
            }
            else
            {
                printf("FW_version:   %d.%d.%d-build.%d\n",
                   system_info.firmware_version.major,
                   system_info.firmware_version.minor,
                   system_info.firmware_version.update,
                   system_info.firmware_version.build);
            }
        }
        else
            printf("\ngetting system info error, error code: %d\n", ret);

        printf("PLUS_version: %s\n", kp_get_version());

    }
    else if (0 == strcmp(_command_type, "model"))
    {
        kp_model_nef_descriptor_t all_models_desc;
        ret = kp_get_model_info(_device, _port_id, &all_models_desc);
        if (ret == KP_SUCCESS)
        {
            printf("\nfw memory contains %d model(s):\n", all_models_desc.num_models);

            if (all_models_desc.num_models > 0)
            {
                printf("metadata:\n");
                printf("    KN number: 0x%X\n", all_models_desc.metadata.kn_num);
                printf("    toolchain version: %s\n", all_models_desc.metadata.toolchain_version);
                printf("    compiler version: %s\n", all_models_desc.metadata.compiler_version);
                printf("    NEF schema version: %u.%u.%u\n", all_models_desc.metadata.nef_schema_version.major,
                                                             all_models_desc.metadata.nef_schema_version.minor,
                                                             all_models_desc.metadata.nef_schema_version.revision);
                printf("    platform: %s\n", all_models_desc.metadata.platform);
                printf("    target chip: %s\n", helper_kp_model_target_chip_to_string(all_models_desc.target));
                printf("    all models crc = 0x%08X\n", all_models_desc.crc);

                for (int i = 0; i < all_models_desc.num_models; i++)
                {
                    printf("[%d] model ID = %d\n", i + 1, all_models_desc.models[i].id);
                    printf("    model raw output size = %d\n", all_models_desc.models[i].max_raw_out_size);

                    printf("    model input:\n");
                    for (int node_idx = 0; node_idx < all_models_desc.models[i].input_nodes_num; node_idx++) {
                        kp_tensor_descriptor_t *tensor_desc = &(all_models_desc.models[i].input_nodes[node_idx]);

                        printf("        [%d] input name = %s\n", node_idx, tensor_desc->name);
                        printf("        [%d] input shape = [", node_idx);
                        for (int dim_idx = 0; dim_idx < tensor_desc->shape_npu_len; dim_idx++) {
                            printf("%u", tensor_desc->shape_npu[dim_idx]);

                            if (dim_idx == (tensor_desc->shape_npu_len - 1)) {
                                printf("]\n");
                            } else {
                                printf(", ");
                            }
                        }

                        printf("        [%d] data layout format = %s\n", node_idx, helper_kp_model_tensor_data_layout_to_string(tensor_desc->data_layout));
                        printf("        [%d] quantization parameters [radix, scale] = [%u, %f]\n",
                                node_idx,
                                tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].radix,
                                tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].scale);
                    }

                    printf("    model output:\n");
                    for (int node_idx = 0; node_idx < all_models_desc.models[i].output_nodes_num; node_idx++) {
                        kp_tensor_descriptor_t *tensor_desc = &(all_models_desc.models[i].output_nodes[node_idx]);

                        printf("        [%d] output name = %s\n", node_idx, tensor_desc->name);
                        printf("        [%d] output shape = [", node_idx);
                        for (int dim_idx = 0; dim_idx < tensor_desc->shape_npu_len; dim_idx++) {
                            printf("%u", tensor_desc->shape_npu[dim_idx]);

                            if (dim_idx == (tensor_desc->shape_npu_len - 1)) {
                                printf("]\n");
                            } else {
                                printf(", ");
                            }
                        }

                        printf("        [%d] data layout format = %s\n", node_idx, helper_kp_model_tensor_data_layout_to_string(tensor_desc->data_layout));
                        printf("        [%d] quantization parameters [radix, scale] = [%u, %f]\n",
                                node_idx,
                                tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].radix,
                                tensor_desc->quantization_parameters.quantized_fixed_point_descriptor[0].scale);
                    }

                    printf("\n");
                }
            }

            kp_release_model_nef_descriptor(&all_models_desc);
        }
        else
            printf("\ngetting model info error, error code: %d, %s.\n", ret, kp_error_string(ret));

        printf("\nNote that if you want to query the model info in the flash,\n");
        printf("please load model first via 'kp_load_model_from_flash()'\n");
        printf("Be careful that 'kp_load_model_from_flash()' will clean up and replace the model data stored in fw memory!\n");
    }
    else if (0 == strcmp(_command_type, "reboot"))
    {
        ret = kp_reset_device(_device, KP_RESET_REBOOT);
        if (ret == KP_SUCCESS)
            printf("reboot device ... OK\n");
        else
            printf("reboot command failed\n");
    }
    else if (0 == strcmp(_command_type, "shutdown"))
    {
        ret = kp_reset_device(_device, KP_RESET_SHUTDOWN);
        if (ret == KP_SUCCESS)
            printf("shutdown device ... OK\n");
        else
            printf("shutdown command failed\n");
    }
    else if (0 == strcmp(_command_type, "reboot_system"))
    {
        ret = kp_reset_device(_device, KP_RESET_REBOOT_SYSTEM);
        if (ret == KP_SUCCESS)
            printf("reboot device ... OK\n");
        else
            printf("reboot command failed\n");
    }
    else
    {
        printf("error ! unsupported command type\n");
        ret = -1;
    }

    printf("\ndisconnecting device ...\n");

    kp_disconnect_devices(_device);

    return 0;
}
