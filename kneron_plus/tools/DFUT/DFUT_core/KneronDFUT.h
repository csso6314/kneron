#ifndef KNERON_DEVICE_FIRMWARE_UPGRADE_TOOL_H
#define KNERON_DEVICE_FIRMWARE_UPGRADE_TOOL_H

#include <string>
#include <vector>
#include <unordered_map>

extern "C" {
#include "kp_struct.h"
}

#ifdef _WIN32
    #include <windows.h>
    #include <libwdi.h>

    #define SLEEP(x)                Sleep(x)
#else
    #include <unistd.h>

    #define SLEEP(x)                usleep(x * 1000)
#endif

#define KL520_PRODUCT_NAME          "KL520"
#define KL630_PRODUCT_NAME          "KL630"
#define KL720_PRODUCT_NAME          "KL720"
#define UNKNOWN_PRODUCT             "Unknown"
#define KL520_PRODUCT_ID            0x100
#define KL630_PRODUCT_ID            0x630
#define KL720_PRODUCT_ID_1          0x200
#define KL720_PRODUCT_ID_2          0x720

#define KL520_SCPU_FW_SIZE_LIMIT    90112
#define KL520_NCPU_FW_SIZE_LIMIT    65536
#define KL720_SCPU_FW_SIZE_LIMIT    131072
#define KL720_NCPU_FW_SIZE_LIMIT    2097152

#define PRODUCT_NAME_OFFSET         0x140
#define IMAGE_NAME_OFFSET           0x144
#define FW_GENERATION_OFFSET        0x15C
#define FW_BIN_INFO_WIDTH           4

#define KDP2_FW_PREFIX              "kdp2"
#define KDP2_FW_PREFIX_UPPER        "KDP2"

#define INIT_SELECT_TEXT_SCPU       "SCPU"
#define INIT_SELECT_TEXT_NCPU       "NCPU"
#define INIT_SELECT_TEXT_MODEL      "Model"

#define USB_LOW_SPEED_STRING        "Low-Speed"
#define USB_FULL_SPEED_STRING       "Full-Speed"
#define USB_HIGH_SPEED_STRING       "High-Speed"
#define USB_SUPER_SPEED_STRING      "Super-Speed"
#define USB_UNKNOWN_SPEED_STRING    "Unknown"

#define VERSION_FILE_PATH           "../VERSION"

#define KDP_FIRMWARE                "KDP"
#define KDP2_LOADER_ONLY            "KDP2 Loader"
#define MAX_RETRY_CONNECT_TIMES     10
#define USB_WAIT_RETRY_CONNECT_MS   10
#define USB_WAIT_CONNECT_DELAY_MS   100
#define USB_WAIT_AFTER_REBOOT       2000

#define AUTO_REBOOT                 false

enum BinCheckErrorCode {
    BCEC_OK = 0,
    BCEC_FILE_NOT_EXIST = 1,
    BCEC_WRONG_PRODUCT_NAME = 2,
    BCEC_WRONG_IMAGE_NAME = 3,
    BCEC_WRONG_GENERATION = 4,
    BCEC_NO_INFO_EMBEDDED = 5,
    BCEC_FULL_PATH_TOO_LONG = 6,   /* windows only */
};

typedef enum {
    /* System Cmd */
    ON_HELP = 1,
    ON_LIST = 2,
    ON_VERSION = 3,
    ON_GUI = 4,
    ON_QUIET = 5,

    /* Update Cmd */
    ON_520_USB_BOOT = 6,
    ON_520_FLASH_BOOT = 7,
    ON_520_UPDATE = 8,
    ON_630_USB_BOOT = 9,
    ON_630_FLASH_BOOT = 10,
    ON_630_UPDATE_LOADER = 11,
    ON_720_UPDATE = 12,
    ON_FLASH_MODEL = 13,

    /* Parameter Related */
    ON_PORT = 14,
    ON_TYPE = 15,
    ON_SCPU = 16,
    ON_NCPU = 17,
} OptionNumber;

char *read_file_to_buffer_auto_malloc(const char *file_path, size_t *buffer_size);

void DisplayVersion();
void ScanAndPrintList();
void GetPortIdList(std::vector<int> &PortIdList, std::string ProductName);
bool IsNumber(const std::string& s);
void SplitString(std::string strSource, std::string strSplit,
                 std::vector<int> &PortIdList);
kp_device_group_t RebootAndReconnect(kp_device_group_t Devices, int PortId, int *ErrorCode);
bool GetResponseFromUser(std::string WarningMessage, std::string ConfirmMessage);
bool IsDriverInstalled(int DevicePid);
int InstallDriver(std::unordered_map<char, std::string> ArgumentMap);
int CheckArgument(std::unordered_map<char, std::string> ArgumentMap);
int CheckModel(std::string strModelFilePath);
int CheckUbuntuVersion();

BinCheckErrorCode CheckBinContent(std::string strFilePath, std::string strTargetProductName, std::string strTargetImageName, std::string strTargetGen);
bool IsFileNameCorrect(std::string strFilePath, std::string strProductName);
bool IsBinSizeReasonable(std::string strFilePath, std::string strProductName, std::string strImageName);

int UpdateKl520ToUsbLoader(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strPlusLoaderPath, std::string strFlashHelperPath);
int UpdateKl520ToFlashBoot(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strPlusLoaderPath, std::string strFlashHelperPath);
int UpdateKl630ToUsbLoader(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath);
int UpdateKl630ToFlashBoot(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath);
int UpdateFwToFlash(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath);
int UpdateModelToFlash(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath);

#endif // KNERON_DEVICE_FIRMWARE_UPGRADE_TOOL_H
