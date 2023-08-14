/**
 * @file        install_driver_windows.c
 * @brief       example of installing driver for KL50/KL720 on Windows
 * @version     0.1
 * @date        2021-11-30
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include "kp_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

static char _target_platform[12] = "ALL";
static bool _install_kl520 = false;
static bool _install_kl720 = false;
static bool _install_kl630 = false;

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#endif

#define INSTALL_DRIVER_PROCESS_NAME     "installer_x64.exe"

void kill_install_driver_process()
{
#ifdef _WIN32
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
        if (strcmp(pEntry.szExeFile, INSTALL_DRIVER_PROCESS_NAME) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD)pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
#endif
}

void print_settings()
{
    bool target_error = false;

    if (0 == strcmp("KL520", _target_platform)) {
        _install_kl520 = true;
        _install_kl720 = false;
        _install_kl630 = false;
    } else if (0 == strcmp("KL720", _target_platform)) {
        _install_kl520 = false;
        _install_kl720 = true;
        _install_kl630 = false;
    } else if (0 == strcmp("KL630", _target_platform)) {
        _install_kl520 = false;
        _install_kl720 = false;
        _install_kl630 = true;
    } else if (0 == strcmp("ALL", _target_platform)) {
        _install_kl520 = true;
        _install_kl720 = true;
        _install_kl630 = true;
    } else {
        target_error = true;
    }

    printf("\n");
    printf("[arguments]\n");
    printf("    -h     : help\n");
    printf("    -target: [target platform] (ALL, KL520, KL720, KL630) = %s\n", _target_platform);
    printf("\n");
    printf("[note]\n");
    printf("    You must run this app as administrator on Windows\n");
    printf("\n");

    if (target_error)
    {
        printf("error ! incorrect target platform !\n");
        exit(0);
    }
}

void parse_arguments(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"target", required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int opt = 0;

    while ((opt = getopt_long_only(argc, argv, "h", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 0:
            if (0 == strcmp("target", long_options[option_index].name)) {
                strcpy(_target_platform, optarg);
            }
            break;
        case 'h':
        default:
            print_settings();
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);
    print_settings();

    int ret = KP_SUCCESS;

    signal(SIGINT, kill_install_driver_process);

    if (true == _install_kl520) {
        printf("Installing driver for KL520 ... ");
        fflush(stdout);

        ret = kp_install_driver_for_windows(KP_DEVICE_KL520);

        printf("%s (%d)\n", (KP_SUCCESS == ret) ? "Success" : "Failed", ret);
        fflush(stdout);
    }

    if (true == _install_kl720) {
        printf("Installing driver for KL720 ... ");
        fflush(stdout);
        ret = kp_install_driver_for_windows(KP_DEVICE_KL720_LEGACY);

        if (KP_SUCCESS != ret) {
            printf("Failed (%d)\n", ret);
            fflush(stdout);
        } else {
            ret = kp_install_driver_for_windows(KP_DEVICE_KL720);

            printf("%s (%d)\n", (KP_SUCCESS == ret) ? "Success" : "Failed", ret);
            fflush(stdout);
        }
    }

    if (true == _install_kl630) {
        printf("Installing driver for KL630 ... ");
        fflush(stdout);

        ret = kp_install_driver_for_windows(KP_DEVICE_KL630);

        printf("%s (%d)\n", (KP_SUCCESS == ret) ? "Success" : "Failed", ret);
        fflush(stdout);
    }

    system("pause");

    return 0;
}
