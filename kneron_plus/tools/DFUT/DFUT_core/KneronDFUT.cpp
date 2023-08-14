/**
 * @file        KneronDFUT.cpp
 * @brief
 * @version     0.1
 * @date        2021-04-13
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#include "KneronDFUT.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string.h>

extern "C" {
#include "kp_core.h"
#include "kp_update_flash.h"
#include "internal_func.h"
#include "kdp2_ipc_cmd.h"
}

#ifdef __WIN32
#include "libwdi.h"
#endif

#include "WarningMessages.h"

#define MAX_GROUP_DEVICE                    20
#define KNERON_PRODUCT_USB_VID              0x3231
#define UBUNTU_ACCEPT_VERSION               "18.04"

// FIXME: workaround to avoid libusb.h include issue in windows
// [NOTICE] please align following 2 structs to kp_internal.h and kp_usb.h
// PS: qmake does not know Msys2 environment (where libusb is installed),
// so that Msys2's default include path (absolute path) should be explicitly added to *.pro or *.pri
// however, this issue doesn't exist in Ubuntu or Raspberry Pi OS since libusb is installed in system path
typedef struct libusb_device_handle libusb_device_handle;
typedef struct
{
    libusb_device_handle *usb_handle;
    kp_device_descriptor_t dev_descp;
    pthread_mutex_t mutex_send;
    pthread_mutex_t mutex_recv;
    uint16_t fw_serial; // for KL520 workaround, if = 0xB*, it uses USBD2V, fake short packet
} kp_usb_device_t;
typedef struct
{
    // public
    int timeout;
    int num_device;
    kp_product_id_t product_id;
    kp_model_nef_descriptor_t loaded_model_desc;
    kp_ddr_manage_attr_t ddr_attr;

    // private
    int cur_send; // record current sending device index
    int cur_recv; // record current receiving device index
    kp_usb_device_t *ll_device[MAX_GROUP_DEVICE];

} __attribute__((aligned(4))) _kp_devices_group_t;

char *read_file_to_buffer_auto_malloc(const char *file_path, size_t *buffer_size)
{
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = static_cast<size_t>(ftell(file)); //get the size

    *buffer_size = file_size;

    char *buffer = static_cast<char *>(malloc(file_size));
    if (nullptr == buffer) {
        fclose(file);
        return nullptr;
    }

    fseek(file, 0, SEEK_SET); //move to begining

    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != static_cast<size_t>(file_size)) {
        free(buffer);
        buffer = nullptr;
        *buffer_size = 0;
    }

    fclose(file);

    return buffer;
}

void DisplayVersion()
{
    std::cout << std::endl;
    std::cout << "DFUT console Version : " << kp_get_version() << std::endl;
    std::cout << std::endl;
}

void ScanAndPrintList()
{
    kp_devices_list_t *pDeviceList;

    pDeviceList = kp_scan_devices();

    std::cout << std::endl;

    if (1 > pDeviceList->num_dev) {
        std::cout << "error ! no Kneron device !" << std::endl;
        return;
    }

    for (int i = 0; i < pDeviceList->num_dev; i++) {
        std::string strProductName;
        std::string strConnectable = (true == pDeviceList->device[i].isConnectable) ? "true" : "false";
        std::string strUsbSpeed;
        char strKnNumber[15] = {0};

        if (KL520_PRODUCT_ID == pDeviceList->device[i].product_id) {
            strProductName = KL520_PRODUCT_NAME;
        } else if ((KL720_PRODUCT_ID_1 == pDeviceList->device[i].product_id) || (KL720_PRODUCT_ID_2 == pDeviceList->device[i].product_id)) {
            strProductName = KL720_PRODUCT_NAME;
        } else if (KL630_PRODUCT_ID == pDeviceList->device[i].product_id) {
            strProductName = KL630_PRODUCT_NAME;
        } else {
            strProductName = UNKNOWN_PRODUCT;
        }

        if (0 == pDeviceList->device[i].kn_number) {
            sprintf(strKnNumber, "Not Supported");
        } else {
            sprintf(strKnNumber, "0x%X", pDeviceList->device[i].kn_number);
        }

        switch (pDeviceList->device[i].link_speed) {
        case KP_USB_SPEED_LOW:
            strUsbSpeed = USB_LOW_SPEED_STRING;
            break;
        case KP_USB_SPEED_FULL:
            strUsbSpeed = USB_FULL_SPEED_STRING;
            break;
        case KP_USB_SPEED_HIGH:
            strUsbSpeed = USB_HIGH_SPEED_STRING;
            break;
        case KP_USB_SPEED_SUPER:
            strUsbSpeed = USB_SUPER_SPEED_STRING;
            break;
        default:
            strUsbSpeed = USB_UNKNOWN_SPEED_STRING;
            break;
        }

        std::cout << "===========================================" << std::endl;
        std::cout << "Index:\t\t" << i + 1 << std::endl;
        std::cout << "Port Id:\t" << pDeviceList->device[i].port_id << std::endl;
        std::cout << "Kn Number:\t" << strKnNumber << std::endl;
        std::cout << "Device Type:\t" << strProductName << std::endl;
        std::cout << "FW Type:\t" << pDeviceList->device[i].firmware << std::endl;
        std::cout << "Usb Speed:\t" << strUsbSpeed << std::endl;
        std::cout << "Connectable:\t" << strConnectable << std::endl;
    }

    std::cout << "===========================================" << std::endl;
}

void GetPortIdList(std::vector<int> &PortIdList, std::string ProductName)
{
    kp_devices_list_t *pDeviceList;

    pDeviceList = kp_scan_devices();

    if (1 > pDeviceList->num_dev) {
        std::cout << "Error ! no Kneron device !" << std::endl;
        return;
    }

    for (int i = 0; i < pDeviceList->num_dev; i++) {
        if (((KL520_PRODUCT_ID == pDeviceList->device[i].product_id) && (KL520_PRODUCT_NAME == ProductName)) ||
            ((KL630_PRODUCT_ID == pDeviceList->device[i].product_id) && (KL630_PRODUCT_NAME == ProductName)) ||
            (((KL720_PRODUCT_ID_1 == pDeviceList->device[i].product_id) || (KL720_PRODUCT_ID_2 == pDeviceList->device[i].product_id)) && (KL720_PRODUCT_NAME == ProductName))) {
            PortIdList.push_back(static_cast<int>(pDeviceList->device[i].port_id));
        }
    }
}

bool IsNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();

    while (it != s.end() && std::isdigit(*it)) {
        ++it;
    }

    return !s.empty() && it == s.end();
}

void SplitString(std::string strSource, std::string strSplit,
                 std::vector<int> &PortIdList)
{
    char *buffer = new char[strSource.length() + 1];
    memset(buffer, 0, (strSource.length() + 1));
    std::copy(strSource.begin(), strSource.end(), buffer);
    char *pTemp = strtok(buffer, strSplit.c_str());

    do {
        if (true == IsNumber(pTemp)) {
            PortIdList.push_back(std::atoi(pTemp));
        }
    } while ((pTemp = strtok(nullptr, strSplit.c_str())));

    if (nullptr != buffer) {
        delete [] buffer;
    }
}

kp_device_group_t RebootAndReconnect(kp_device_group_t Devices, int PortId, int *ErrorCode)
{
    int TryConnectTimes = 0;

    SLEEP(USB_WAIT_CONNECT_DELAY_MS);
    kp_reset_device(Devices, KP_RESET_REBOOT);
    SLEEP(USB_WAIT_AFTER_REBOOT);

    if (nullptr != Devices) {
        kp_disconnect_devices(Devices);
    }

    while (true) {
        *ErrorCode = KDP_MAGIC_CONNECTION_PASS;
        Devices = kp_connect_devices(1, &PortId, ErrorCode);

        if (nullptr != Devices) {
            break;
        }

        SLEEP(USB_WAIT_RETRY_CONNECT_MS);
        TryConnectTimes++;

        if (MAX_RETRY_CONNECT_TIMES < TryConnectTimes) {
            Devices = nullptr;
            break;
        }
    }

    return Devices;
}

// return true: User enter 'y' or 'Y'
// return false: User enter 'n' or 'N'
bool GetResponseFromUser(std::string WarningMessage, std::string ConfirmMessage)
{
    std::string response;

    std::cout << std::endl << WarningMessage << std::endl;
    std::cout << std::endl << ConfirmMessage << " (y/n) " << std::endl;

    while(true)
    {
        std::cin >> response;

        std::transform(response.begin(), response.end(), response.begin(), ::toupper);
        if (response == "Y") {
            return true;
        }
        else if (response == "N") {
            return false;
        }
        else {
            std::cout << "Sorry, please enter again." << std::endl;
        }
    }
}

bool IsDriverInstalled(int DevicePid)
{
    bool blInstalled = true;

#ifdef __WIN32__
    struct wdi_device_info *ldev;
    struct wdi_options_create_list ocl;
    int Ret = WDI_ERROR_OTHER;

    int DevicePid2 = (KP_DEVICE_KL720 == DevicePid) ? KP_DEVICE_KL720_LEGACY : DevicePid;

    ocl.list_all = TRUE;
    ocl.list_hubs = TRUE;
    ocl.trim_whitespaces = TRUE;

    Ret = wdi_create_list(&ldev, &ocl);

    if (WDI_SUCCESS == Ret) {
        for (; (NULL != ldev) && (WDI_SUCCESS == Ret); ldev = ldev->next) {
            if ((KNERON_PRODUCT_USB_VID == ldev->vid) && ((DevicePid == ldev->pid) || (DevicePid2 == ldev->pid)) && (NULL == ldev->driver)) {
                blInstalled = false;
                break;
            }
        }

        wdi_destroy_list(ldev);
    }

#else
    (void)DevicePid;
#endif

    return blInstalled;
}

int InstallDriver(std::unordered_map<char, std::string> ArgumentMap)
{
    int Ret = 0;

#ifdef __WIN32__
    std::string strResult;

    if (((false == ArgumentMap[ON_520_USB_BOOT].empty()) ||
         (false == ArgumentMap[ON_520_FLASH_BOOT].empty()) ||
         (false == ArgumentMap[ON_520_UPDATE].empty()) ||
         ((false == ArgumentMap[ON_FLASH_MODEL].empty()) && (KL520_PRODUCT_NAME == ArgumentMap[ON_TYPE]))) &&
        (false == IsDriverInstalled(KP_DEVICE_KL520))) {

        if (true == ArgumentMap[ON_QUIET].empty() &&
            false == GetResponseFromUser(INSTALL_DRIVER_WARNING, INSTALL_DRIVER_MSG)) {
            exit(0);
        }

        std::cout << std::endl << "Installing driver for KL520 ... ";

        Ret = kp_install_driver_for_windows(KP_DEVICE_KL520);

        strResult = (0 == Ret) ? "Success" : "Failed";

        std::cout << strResult << "(" << Ret << ")" << std::endl;
    } else if (((false == ArgumentMap[ON_720_UPDATE].empty()) ||
                ((false == ArgumentMap[ON_FLASH_MODEL].empty()) && (KL720_PRODUCT_NAME == ArgumentMap[ON_TYPE]))) &&
               (false == IsDriverInstalled(KP_DEVICE_KL720))) {

        if (true == ArgumentMap[ON_QUIET].empty() &&
            false == GetResponseFromUser(INSTALL_DRIVER_WARNING, INSTALL_DRIVER_MSG)) {
            exit(0);
        }

        std::cout << std::endl << "Installing driver for KL720 ... ";

        Ret = kp_install_driver_for_windows(KP_DEVICE_KL720_LEGACY);

        if (0 != Ret) {
            std::cout << "Failed. (" << Ret << ")" << std::endl;
        } else {
            Ret = kp_install_driver_for_windows(KP_DEVICE_KL720);

            strResult = (0 == Ret) ? "Success" : "Failed";

            std::cout << strResult << "(" << Ret << ")" << std::endl;
        }
    } else if (((false == ArgumentMap[ON_630_USB_BOOT].empty()) ||
                (false == ArgumentMap[ON_630_FLASH_BOOT].empty()) ||
                ((false == ArgumentMap[ON_FLASH_MODEL].empty()) && (KL630_PRODUCT_NAME == ArgumentMap[ON_TYPE]))) &&
               (false == IsDriverInstalled(KP_DEVICE_KL630))) {
        if (true == ArgumentMap[ON_QUIET].empty() &&
            false == GetResponseFromUser(INSTALL_DRIVER_WARNING, INSTALL_DRIVER_MSG)) {
            exit(0);
        }

        std::cout << std::endl << "Installing driver for KL630 ... ";

        Ret = kp_install_driver_for_windows(KP_DEVICE_KL630);

        strResult = (0 == Ret) ? "Success" : "Failed";

        std::cout << strResult << "(" << Ret << ")" << std::endl;
    }
#else
    (void)ArgumentMap;
#endif

    if (0 != Ret) {
        std::cout << std::endl << INSTALL_DRIVER_ADMIN << std::endl;
    }

    return Ret;
}

int CheckArgument(std::unordered_map<char, std::string> ArgumentMap)
{
    if (true == ArgumentMap[ON_PORT].empty()) {
        std::cout << "[Error] Port Ids of devices to be updated must be provided." << std::endl;
        return -1;
    }

    if (((false == ArgumentMap[ON_520_USB_BOOT].empty()) && (false == ArgumentMap[ON_520_FLASH_BOOT].empty())) ||
        ((false == ArgumentMap[ON_520_USB_BOOT].empty()) && (false == ArgumentMap[ON_630_USB_BOOT].empty())) ||
        ((false == ArgumentMap[ON_520_USB_BOOT].empty()) && (false == ArgumentMap[ON_630_FLASH_BOOT].empty())) ||
        ((false == ArgumentMap[ON_520_USB_BOOT].empty()) && (false == ArgumentMap[ON_520_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_520_USB_BOOT].empty()) && (false == ArgumentMap[ON_720_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_520_USB_BOOT].empty()) && (false == ArgumentMap[ON_FLASH_MODEL].empty())) ||
        ((false == ArgumentMap[ON_520_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_630_USB_BOOT].empty())) ||
        ((false == ArgumentMap[ON_520_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_630_FLASH_BOOT].empty())) ||
        ((false == ArgumentMap[ON_520_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_520_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_520_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_720_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_520_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_FLASH_MODEL].empty())) ||
        ((false == ArgumentMap[ON_630_USB_BOOT].empty()) && (false == ArgumentMap[ON_630_FLASH_BOOT].empty())) ||
        ((false == ArgumentMap[ON_630_USB_BOOT].empty()) && (false == ArgumentMap[ON_520_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_630_USB_BOOT].empty()) && (false == ArgumentMap[ON_720_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_630_USB_BOOT].empty()) && (false == ArgumentMap[ON_FLASH_MODEL].empty())) ||
        ((false == ArgumentMap[ON_630_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_630_UPDATE_LOADER].empty())) ||
        ((false == ArgumentMap[ON_630_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_520_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_630_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_720_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_630_FLASH_BOOT].empty()) && (false == ArgumentMap[ON_FLASH_MODEL].empty())) ||
        ((false == ArgumentMap[ON_630_UPDATE_LOADER].empty()) && (false == ArgumentMap[ON_520_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_630_UPDATE_LOADER].empty()) && (false == ArgumentMap[ON_720_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_630_UPDATE_LOADER].empty()) && (false == ArgumentMap[ON_FLASH_MODEL].empty())) ||
        ((false == ArgumentMap[ON_520_UPDATE].empty()) && (false == ArgumentMap[ON_720_UPDATE].empty())) ||
        ((false == ArgumentMap[ON_520_UPDATE].empty()) && (false == ArgumentMap[ON_FLASH_MODEL].empty())) ||
        ((false == ArgumentMap[ON_720_UPDATE].empty()) && (false == ArgumentMap[ON_FLASH_MODEL].empty()))) {
        std::cout << "[Error] Only one update method can be chosen." << std::endl;
        return -1;
    } else if ((true == ArgumentMap[ON_520_USB_BOOT].empty()) && (true == ArgumentMap[ON_520_FLASH_BOOT].empty()) &&
               (true == ArgumentMap[ON_630_USB_BOOT].empty()) && (true == ArgumentMap[ON_630_FLASH_BOOT].empty()) &&
               (true == ArgumentMap[ON_520_UPDATE].empty()) && (true == ArgumentMap[ON_720_UPDATE].empty()) && (true == ArgumentMap[ON_FLASH_MODEL].empty())) {
        std::cout << "[Error] One update method must be chosen." << std::endl;
        return -1;
    }

    if ((false == ArgumentMap[ON_520_FLASH_BOOT].empty()) ||
        (false == ArgumentMap[ON_520_UPDATE].empty()) ||
        (false == ArgumentMap[ON_720_UPDATE].empty())) {
        if (((true == ArgumentMap[ON_SCPU].empty()) || (true == ArgumentMap[ON_NCPU].empty()))) {
            std::cout << "[Error] Firmware of scpu and ncpu must be both provided." << std::endl;
            return -1;
        }
    } else if (false == ArgumentMap[ON_630_FLASH_BOOT].empty()) {
        if (true == ArgumentMap[ON_SCPU].empty()) {
            std::cout << "[Error] Firmware must be provided." << std::endl;
            return -1;
        }
    } else if (false == ArgumentMap[ON_FLASH_MODEL].empty()) {
        if (true == ArgumentMap[ON_TYPE].empty()) {
            std::cout << "[Error] Please specify the device type you want to update." << std::endl;
            return -1;
        } else {
            std::string strType = ArgumentMap[ON_TYPE];
            std::transform(strType.begin(), strType.end(), strType.begin(), ::toupper);

            ArgumentMap[ON_TYPE] = strType;

            if ((KL520_PRODUCT_NAME != strType) && (KL720_PRODUCT_NAME != strType) && (KL630_PRODUCT_NAME != strType)) {
                std::cout << "[Error] Device type is not recognized." << std::endl;
                return -1;
            }
        }
    } else if (false == ArgumentMap[ON_630_UPDATE_LOADER].empty()) {
        if (true == ArgumentMap[ON_SCPU].empty()) {
            std::cout << "[Error] Loader must be provided." << std::endl;
            return -1;
        }
    }

    return 0;
}

int CheckModel(std::string strModelFilePath)
{
    size_t nef_size;
    char *nef_buf = read_file_to_buffer_auto_malloc(strModelFilePath.c_str(), &nef_size);

    if (!nef_buf) {
        return -1;
    }

    kp_metadata_t metadata;
    memset(&metadata, 0, sizeof(kp_metadata_t));
    kp_nef_info_t nef_info;
    memset(&nef_info, 0, sizeof(kp_nef_info_t));

    int Ret = read_nef(nef_buf, static_cast<uint32_t>(nef_size), &metadata, &nef_info);
    if (0 != Ret) {
        return -1;
    }

    if (KP_MODEL_TARGET_CHIP_KL520 == metadata.target) {
        return KL520_PRODUCT_ID;
    } else if (KP_MODEL_TARGET_CHIP_KL720 == metadata.target) {
        return KL720_PRODUCT_ID_2;
    } else if (KP_MODEL_TARGET_CHIP_KL630 == metadata.target) {
        return KL630_PRODUCT_ID;
    } else {
        return -1;
    }
}

int CheckUbuntuVersion()
{
    std::fstream File;
    char Line[100] = {0};
    bool bl_18_04 = false;

    File.open("/etc/lsb-release", std::ios::in);

    if (true == File.is_open()) {
        while (File.getline(Line, 100)) {
            std::string strLine = Line;

            if (std::string::npos != strLine.find(UBUNTU_ACCEPT_VERSION)) {
                bl_18_04 = true;
                break;
            }
        }

        File.close();

        if (false == bl_18_04) {
            return -1;
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

BinCheckErrorCode CheckBinContent(std::string strFilePath, std::string strTargetProductName, std::string strTargetImageName, std::string strTargetGen)
{
    BinCheckErrorCode ret = BCEC_OK;

    // convert KL520_PRODUCT_NAME and KL720_PRODUCT_NAME to product name in bin file
    std::string strTargetProductNameInBin;
    if (KL520_PRODUCT_NAME == strTargetProductName) {
        strTargetProductNameInBin = "0520";
    }
    if (KL720_PRODUCT_NAME == strTargetProductName) {
        strTargetProductNameInBin = "0720";
    }

    // check the content of fw bin files
    std::ifstream file(strFilePath, std::ios::binary);

#ifdef __WIN32__
#define WIN_MAX_FILE_PATH_LENGTH 255
    if (strFilePath.length() >= WIN_MAX_FILE_PATH_LENGTH ) {
        std::cout << strTargetImageName << " firmware file full path is too long" << std::endl;
        return BCEC_FULL_PATH_TOO_LONG;
    }
#endif

    if (!file) {
        std::cout << strTargetImageName << " firmware file does not exist" << std::endl;
        return BCEC_FILE_NOT_EXIST;
    }

    // 720 ncpu firmware bin file does not contain any extra info
    if (("0720" == strTargetProductNameInBin) && (INIT_SELECT_TEXT_NCPU == strTargetImageName)) {
        return BCEC_NO_INFO_EMBEDDED;
    }

    // get pointer to associate buffer object
    std::filebuf* fileBuf = file.rdbuf();

    // temp buffer to save info from bin file
    char binInfo[FW_BIN_INFO_WIDTH];

    // get product name
    fileBuf->pubseekpos(PRODUCT_NAME_OFFSET, file.in);
    fileBuf->sgetn(binInfo, FW_BIN_INFO_WIDTH);
    std::string productName(reinterpret_cast<const char *>(binInfo), FW_BIN_INFO_WIDTH);

    // get image name
    fileBuf->pubseekpos(IMAGE_NAME_OFFSET, file.in);
    fileBuf->sgetn(binInfo, FW_BIN_INFO_WIDTH);
    std::string imageName(reinterpret_cast<const char *>(binInfo), FW_BIN_INFO_WIDTH);

    // get firmware generation
    fileBuf->pubseekpos(FW_GENERATION_OFFSET, file.in);
    fileBuf->sgetn(binInfo, FW_BIN_INFO_WIDTH);
    std::string fwGen(reinterpret_cast<const char *>(binInfo), FW_BIN_INFO_WIDTH);
    std::transform(fwGen.begin(), fwGen.end(), fwGen.begin(), ::toupper);

    file.close();

    if ((productName == "0520") || (productName == "0720")) {
        if (productName != strTargetProductNameInBin) {
            if (productName == "0520")
                std::cout << "Target product name is " << strTargetProductName << " while the product name read from " << strTargetImageName << " bin file is KL520" << std::endl;
            else // productName == "0720"
                std::cout << "Target product name is " << strTargetProductName << " while the product name read from " << strTargetImageName << " bin file is KL720" << std::endl;
            return BCEC_WRONG_PRODUCT_NAME;
        }
    }
    else {
        ret = BCEC_NO_INFO_EMBEDDED;
    }

    if ((imageName == INIT_SELECT_TEXT_SCPU) || (imageName == INIT_SELECT_TEXT_NCPU)) {
        if (imageName != strTargetImageName) {
            std::cout << "Target image name is " << strTargetImageName << " while the image name read from " << strTargetImageName << " bin file is " << imageName << std::endl;
            return BCEC_WRONG_IMAGE_NAME;
        }
    }
    else {
        ret = BCEC_NO_INFO_EMBEDDED;
    }

    // 720 scpu firmware bin file does not contain generation info
    if (("0720" == strTargetProductNameInBin) && (INIT_SELECT_TEXT_SCPU == strTargetImageName)) {
        return ret;
    }

    if ((fwGen == "KDP2") || (fwGen == "KDP ")) {
        if (fwGen != strTargetGen) {
            std::cout << "Target generation is " << strTargetGen << " while the generation read from " << strTargetImageName << " bin file is " << fwGen << std::endl;
            return BCEC_WRONG_GENERATION;
        }
    }
    else {
        ret = BCEC_NO_INFO_EMBEDDED;
    }

    return ret;
}

bool IsFileNameCorrect(std::string strFilePath, std::string strImageName)
{
    std::string strFileName = strFilePath.substr(strFilePath.find_last_of("/\\") + 1);

    std::transform(strFileName.begin(), strFileName.end(), strFileName.begin(), ::toupper);

    if (std::string::npos == strFileName.find(strImageName)) {
        return false;
    }
    else {
        return true;
    }
}

bool IsBinSizeReasonable(std::string strFilePath, std::string strProductName, std::string strImageName)
{
    std::ifstream file(strFilePath, std::ios::binary);

    if (!file) {
        std::cout << strImageName << " firmware file does not exist" << std::endl;
        return false;
    }

    // get pointer to associated buffer object
    std::filebuf* fileBuf = file.rdbuf();

    // check the size of fw bin file
    long fileSize = static_cast<long>(fileBuf->pubseekoff(0, file.end, file.in)); // avoid compile warning in some platforms
    file.close();

    long fw_size_limit = 0;

    if ((strProductName == KL520_PRODUCT_NAME) && (strImageName == INIT_SELECT_TEXT_SCPU)) {
        fw_size_limit = KL520_SCPU_FW_SIZE_LIMIT;
    }
    else if ((strProductName == KL520_PRODUCT_NAME) && (strImageName == INIT_SELECT_TEXT_NCPU)) {
        fw_size_limit = KL520_NCPU_FW_SIZE_LIMIT;
    }
    else if ((strProductName == KL720_PRODUCT_NAME) && (strImageName == INIT_SELECT_TEXT_SCPU)) {
        fw_size_limit = KL720_SCPU_FW_SIZE_LIMIT;
    }
    else if ((strProductName == KL720_PRODUCT_NAME) && (strImageName == INIT_SELECT_TEXT_NCPU)) {
        fw_size_limit = KL720_NCPU_FW_SIZE_LIMIT;
    }

    if (fileSize > fw_size_limit) {
        return false;
    }
    else {
        return true;
    }
}

int UpdateKl520ToUsbLoader(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strPlusLoaderPath, std::string strFlashHelperPath)
{
    for (size_t i = 0; i < PortIdList.size(); i++) {
        int ErrorCode = KDP_MAGIC_CONNECTION_PASS;
        kp_device_group_t Devices;
        _kp_devices_group_t *pDeviceList;
        int Ret = -1;
        std::string strMessage;
        int PortId = PortIdList[i];

        std::cout << std::endl;
        std::cout << "Start Update Device with Port Id " << PortId << " to USB Boot" << std::endl;

        Devices = kp_connect_devices(1, &PortId, &ErrorCode);
        pDeviceList = reinterpret_cast<_kp_devices_group_t *>(Devices);

        if ((nullptr == Devices) || (1 > pDeviceList->num_device)) {
            Ret = KP_ERROR_DEVICE_NOT_EXIST_10;
            goto USB_LOOP_OUT;
        }

        if (KL520_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) {
            std::cout << std::endl;
            std::cout << "Update to USB Loader only accept KL520. Device with Port Id: " << PortId << " is not KL520" << std::endl;
            goto USB_LOOP_OUT;
        }

        if (((KP_KDP2_FW_LOADER_V2 == (pDeviceList->ll_device[0]->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK_V2)) ||
             (KP_KDP2_FW_LOADER == (pDeviceList->ll_device[0]->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK))) &&
            (true == ArgumentMap[ON_QUIET].empty()) &&
            (false == GetResponseFromUser(UPDATE_LOADER_WARNING_2, UPDATE_PROCEED_MSG)))
        {
            if (nullptr != Devices) {
                kp_disconnect_devices(Devices);
            }
            return -1;
        }

        kp_set_timeout(Devices, 20000); // 20 secs timeout

        if (KDP_FIRMWARE == std::string(pDeviceList->ll_device[0]->dev_descp.firmware)) {
            Ret = kp_update_kdp2_usb_loader_from_file(Devices, strPlusLoaderPath.c_str(), AUTO_REBOOT);

            if (KP_SUCCESS != Ret) {
                goto USB_LOOP_OUT;
            }

            if (false == AUTO_REBOOT) {
                Devices = RebootAndReconnect(Devices, PortId, &Ret);

                if (nullptr == Devices) {
                    goto USB_LOOP_OUT;
                }
            }

            kp_set_timeout(Devices, 20000); // 20 secs timeout
        }

        Ret = kp_load_firmware_from_file(Devices, strFlashHelperPath.c_str(), nullptr);

        if (KP_SUCCESS != Ret) {
            goto USB_LOOP_OUT;
        }

        SLEEP(USB_WAIT_CONNECT_DELAY_MS);

        Ret = kp_switch_to_kdp2_usb_boot(Devices, AUTO_REBOOT);

        if (KP_SUCCESS != Ret) {
            goto USB_LOOP_OUT;
        }

        if (false == AUTO_REBOOT) {
            Devices = RebootAndReconnect(Devices, PortId, &Ret);
        }

USB_LOOP_OUT:

        if (nullptr != Devices) {
            kp_disconnect_devices(Devices);
        }

        strMessage = (0 == Ret) ? " Succeeded" : (" Failed with Error Code: " + std::to_string(Ret));

        std::cout << std::endl;
        std::cout << "==== Update of Device with Port Id: " << PortId << strMessage << " ====" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

int UpdateKl520ToFlashBoot(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strPlusLoaderPath, std::string strFlashHelperPath)
{
    std::string strScpuFilePath = ArgumentMap[ON_SCPU];
    std::string strNcpuFilePath = ArgumentMap[ON_NCPU];

    BinCheckErrorCode scpuBinCheckRet = CheckBinContent(strScpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_SCPU, "KDP2");
    BinCheckErrorCode ncpuBinCheckRet = CheckBinContent(strNcpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_NCPU, "KDP2");

    int ret = 0;

    // check failed
    if (((BCEC_OK != scpuBinCheckRet) && (BCEC_NO_INFO_EMBEDDED != scpuBinCheckRet)) ||
        ((BCEC_OK != ncpuBinCheckRet) && (BCEC_NO_INFO_EMBEDDED != ncpuBinCheckRet))) {
        ret = -1;
    }

    // further check if firmware bin files not contain extra info
    // check scpu firmware
    if (BCEC_NO_INFO_EMBEDDED == scpuBinCheckRet) {
        if (false == IsFileNameCorrect(strScpuFilePath, INIT_SELECT_TEXT_SCPU)) {
            std::cout << std::endl;
            std::cout << SCPU_FW_FILE_NAME_WRONG_WARNING << std::endl;
            ret = -1;
        }

        if (false == IsBinSizeReasonable(strScpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_SCPU)) {
            std::cout << std::endl;
            std::cout << SCPU_FW_FILE_SIZE_WRONG_WARNING << std::endl;
            ret = -1;
        }
    }

    // check ncpu firmware
    if (BCEC_NO_INFO_EMBEDDED == ncpuBinCheckRet) {
        if (false == IsFileNameCorrect(strNcpuFilePath, INIT_SELECT_TEXT_NCPU)) {
            std::cout << std::endl;
            std::cout << NCPU_FW_FILE_NAME_WRONG_WARNING << std::endl;
            ret = -1;
        }

        if (false == IsBinSizeReasonable(strNcpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_NCPU)) {
            std::cout << std::endl;
            std::cout << NCPU_FW_FILE_SIZE_WRONG_WARNING << std::endl;
            ret = -1;
        }
    }

    if (ret < 0)
        return -1;

    if (true == ArgumentMap[ON_QUIET].empty() &&
        false == GetResponseFromUser(UPDATE_KL520_FLASH_WARNING, UPDATE_PROCEED_MSG)) {
        return -1;
    }

    for (size_t i = 0; i < PortIdList.size(); i++) {
        int ErrorCode = KDP_MAGIC_CONNECTION_PASS;
        kp_device_group_t Devices;
        _kp_devices_group_t *pDeviceList;
        int Ret = -1;
        std::string strMessage;
        int PortId = PortIdList[i];

        std::cout << std::endl;
        std::cout << "Start Update Device with Port Id " << PortId << " to Flash Boot" << std::endl;

        Devices = kp_connect_devices(1, &PortId, &ErrorCode);
        pDeviceList = reinterpret_cast<_kp_devices_group_t *>(Devices);

        if ((nullptr == Devices) || (1 > pDeviceList->num_device)) {
            Ret = KP_ERROR_DEVICE_NOT_EXIST_10;
            goto FLASH_LOOP_OUT;
        }

        if (KL520_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) {
            std::cout << std::endl;
            std::cout << "Update to Flash Loader only accept KL520. Device with Port Id: " << PortId << " is not KL520" << std::endl;
            goto FLASH_LOOP_OUT;
        }

        kp_set_timeout(Devices, 20000); // 20 secs timeout

        if (KDP_FIRMWARE == std::string(pDeviceList->ll_device[0]->dev_descp.firmware)) {
            Ret = kp_update_kdp2_usb_loader_from_file(Devices, strPlusLoaderPath.c_str(), AUTO_REBOOT);

            if (KP_SUCCESS != Ret) {
                goto FLASH_LOOP_OUT;
            }

            if (false == AUTO_REBOOT) {
                Devices = RebootAndReconnect(Devices, PortId, &Ret);

                if (nullptr == Devices) {
                    goto FLASH_LOOP_OUT;
                }
            }

            kp_set_timeout(Devices, 20000); // 20 secs timeout
        }

        Ret = kp_load_firmware_from_file(Devices, strFlashHelperPath.c_str(), nullptr);

        if (KP_SUCCESS != Ret) {
            goto FLASH_LOOP_OUT;
        }

        SLEEP(USB_WAIT_CONNECT_DELAY_MS);

        if (true == AUTO_REBOOT) {
            Ret = kp_update_kdp2_firmware_from_files(Devices, ArgumentMap[ON_SCPU].c_str(), ArgumentMap[ON_NCPU].c_str(), true);
        } else {
            Ret = kp_update_kdp2_firmware_from_files(Devices, ArgumentMap[ON_SCPU].c_str(), nullptr, false);

            if (KP_SUCCESS != Ret) {
                goto FLASH_LOOP_OUT;
            }

            Devices = RebootAndReconnect(Devices, PortId, &Ret);

            if (nullptr == Devices) {
                goto FLASH_LOOP_OUT;
            }

            kp_set_timeout(Devices, 20000); // 20 secs timeout

            Ret = kp_update_kdp2_firmware_from_files(Devices, nullptr, ArgumentMap[ON_NCPU].c_str(), false);

            if (KP_SUCCESS != Ret) {
                goto FLASH_LOOP_OUT;
            }

            Devices = RebootAndReconnect(Devices, PortId, &Ret);
        }

FLASH_LOOP_OUT:

        if (nullptr != Devices) {
            kp_disconnect_devices(Devices);
        }

        strMessage = (0 == Ret) ? " Succeeded" : (" Failed with Error Code: " + std::to_string(Ret));

        std::cout << std::endl;
        std::cout << "==== Update of Device with Port Id: " << PortId << strMessage << " ====" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

int UpdateKl630ToUsbLoader(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath)
{
    for (size_t i = 0; i < PortIdList.size(); i++) {
        int ErrorCode = KDP_MAGIC_CONNECTION_PASS;
        kp_device_group_t Devices;
        _kp_devices_group_t *pDeviceList;
        int Ret = -1;
        std::string strMessage;
        int PortId = PortIdList[i];

        std::cout << std::endl;
        std::cout << "Start Update Device with Port Id " << PortId << " to USB Boot" << std::endl;

        Devices = kp_connect_devices(1, &PortId, &ErrorCode);
        pDeviceList = reinterpret_cast<_kp_devices_group_t *>(Devices);

        if ((nullptr == Devices) || (1 > pDeviceList->num_device)) {
            Ret = KP_ERROR_DEVICE_NOT_EXIST_10;
            goto USB_LOOP_OUT;
        }

        if (KL630_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) {
            std::cout << std::endl;
            std::cout << "Update to USB Loader only accept KL630. Device with Port Id: " << PortId << " is not KL630" << std::endl;
            goto USB_LOOP_OUT;
        }

        if ((KP_KDP2_FW_LOADER_V2 == (pDeviceList->ll_device[0]->fw_serial & KP_KDP2_FW_FIND_TYPE_MASK_V2)) &&
            (true == ArgumentMap[ON_QUIET].empty()) &&
            (false == GetResponseFromUser(UPDATE_LOADER_WARNING_2, UPDATE_PROCEED_MSG)))
        {
            if (nullptr != Devices) {
                kp_disconnect_devices(Devices);
            }
            return -1;
        }

        kp_set_timeout(Devices, 20000); // 20 secs timeout

        Ret = kp_load_firmware_from_file(Devices, strFlashHelperPath.c_str(), nullptr);

        if (KP_SUCCESS != Ret) {
            goto USB_LOOP_OUT;
        }

        SLEEP(USB_WAIT_CONNECT_DELAY_MS);

        Ret = kp_switch_to_kdp2_usb_boot(Devices, AUTO_REBOOT);

        if (KP_SUCCESS != Ret) {
            goto USB_LOOP_OUT;
        }

        if (false == AUTO_REBOOT) {
            Devices = RebootAndReconnect(Devices, PortId, &Ret);
        }

USB_LOOP_OUT:

        if (nullptr != Devices) {
            kp_disconnect_devices(Devices);
        }

        strMessage = (0 == Ret) ? " Succeeded" : (" Failed with Error Code: " + std::to_string(Ret));

        std::cout << std::endl;
        std::cout << "==== Update of Device with Port Id: " << PortId << strMessage << " ====" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

int UpdateKl630ToFlashBoot(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath)
{
    std::string strScpuFilePath = ArgumentMap[ON_SCPU];

    if (true == ArgumentMap[ON_QUIET].empty() &&
        false == GetResponseFromUser(UPDATE_KL520_FLASH_WARNING, UPDATE_PROCEED_MSG)) {
        return -1;
    }

    for (size_t i = 0; i < PortIdList.size(); i++) {
        int ErrorCode = KDP_MAGIC_CONNECTION_PASS;
        kp_device_group_t Devices;
        _kp_devices_group_t *pDeviceList;
        int Ret = -1;
        std::string strMessage;
        int PortId = PortIdList[i];

        std::cout << std::endl;
        std::cout << "Start Update Device with Port Id " << PortId << " to Flash Boot" << std::endl;

        Devices = kp_connect_devices(1, &PortId, &ErrorCode);
        pDeviceList = reinterpret_cast<_kp_devices_group_t *>(Devices);

        if ((nullptr == Devices) || (1 > pDeviceList->num_device)) {
            Ret = KP_ERROR_DEVICE_NOT_EXIST_10;
            goto FLASH_LOOP_OUT;
        }

        if (KL630_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) {
            std::cout << std::endl;
            std::cout << "Update to Flash Loader only accept KL630. Device with Port Id: " << PortId << " is not KL630" << std::endl;
            goto FLASH_LOOP_OUT;
        }

        kp_set_timeout(Devices, 20000); // 20 secs timeout

        Ret = kp_load_firmware_from_file(Devices, strFlashHelperPath.c_str(), nullptr);

        if (KP_SUCCESS != Ret) {
            goto FLASH_LOOP_OUT;
        }

        SLEEP(USB_WAIT_CONNECT_DELAY_MS);

        if (true == AUTO_REBOOT) {
            Ret = kp_update_kdp2_firmware_from_files(Devices, ArgumentMap[ON_SCPU].c_str(), nullptr, true);
        } else {
            Ret = kp_update_kdp2_firmware_from_files(Devices, ArgumentMap[ON_SCPU].c_str(), nullptr, false);

            if (KP_SUCCESS != Ret) {
                goto FLASH_LOOP_OUT;
            }

            Devices = RebootAndReconnect(Devices, PortId, &Ret);
        }

FLASH_LOOP_OUT:

        if (nullptr != Devices) {
            kp_disconnect_devices(Devices);
        }

        strMessage = (0 == Ret) ? " Succeeded" : (" Failed with Error Code: " + std::to_string(Ret));

        std::cout << std::endl;
        std::cout << "==== Update of Device with Port Id: " << PortId << strMessage << " ====" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

int UpdateFwToFlash(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath)
{
    std::string strScpuFilePath = ArgumentMap[ON_SCPU];
    std::string strNcpuFilePath = ArgumentMap[ON_NCPU];

    if (false == ArgumentMap[ON_520_UPDATE].empty())
    {
        BinCheckErrorCode scpuBinCheckRet = CheckBinContent(strScpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_SCPU, "KDP ");
        BinCheckErrorCode ncpuBinCheckRet = CheckBinContent(strNcpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_NCPU, "KDP ");

        int ret = 0;

        // check failed
        if (((BCEC_OK != scpuBinCheckRet) && (BCEC_NO_INFO_EMBEDDED != scpuBinCheckRet)) ||
            ((BCEC_OK != ncpuBinCheckRet) && (BCEC_NO_INFO_EMBEDDED != ncpuBinCheckRet))) {
            ret = -1;
        }

        // further check if firmware bin files not contain extra info
        // check scpu firmware
        if (BCEC_NO_INFO_EMBEDDED == scpuBinCheckRet) {
            if (false == IsFileNameCorrect(strScpuFilePath, INIT_SELECT_TEXT_SCPU)) {
                std::cout << std::endl;
                std::cout << SCPU_FW_FILE_NAME_WRONG_WARNING << std::endl;
                ret = -1;
            }

            if (false == IsBinSizeReasonable(strScpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_SCPU)) {
                std::cout << std::endl;
                std::cout << SCPU_FW_FILE_SIZE_WRONG_WARNING << std::endl;
                ret = -1;
            }
        }

        // check ncpu firmware
        if (BCEC_NO_INFO_EMBEDDED == ncpuBinCheckRet) {
            if (false == IsFileNameCorrect(strNcpuFilePath, INIT_SELECT_TEXT_NCPU)) {
                std::cout << std::endl;
                std::cout << NCPU_FW_FILE_NAME_WRONG_WARNING << std::endl;
                ret = -1;
            }

            if (false == IsBinSizeReasonable(strNcpuFilePath, KL520_PRODUCT_NAME, INIT_SELECT_TEXT_NCPU)) {
                std::cout << std::endl;
                std::cout << NCPU_FW_FILE_SIZE_WRONG_WARNING << std::endl;
                ret = -1;
            }
        }

        if (ret < 0) {
            return -1;
        }
    }
    else if (false == ArgumentMap[ON_720_UPDATE].empty())
    {
        // 720 scpu firmware bin file does not contain generation info
        BinCheckErrorCode scpuBinCheckRet = CheckBinContent(strScpuFilePath, KL720_PRODUCT_NAME, INIT_SELECT_TEXT_SCPU, "");

        // 720 ncpu firmware bin file does not contain any extra info, only check if it exists
        BinCheckErrorCode ncpuBinCheckRet = CheckBinContent(strNcpuFilePath, KL720_PRODUCT_NAME, INIT_SELECT_TEXT_NCPU, "");

        int ret = 0;

        // check failed
        if (((BCEC_OK != scpuBinCheckRet) && (BCEC_NO_INFO_EMBEDDED != scpuBinCheckRet)) ||
            ((BCEC_OK != ncpuBinCheckRet) && (BCEC_NO_INFO_EMBEDDED != ncpuBinCheckRet))) {
            ret = -1;
        }

        // further check if firmware bin file does not contain extra info
        // check scpu firmware
        if (BCEC_NO_INFO_EMBEDDED == scpuBinCheckRet)
        {
            if (false == IsFileNameCorrect(strScpuFilePath, INIT_SELECT_TEXT_SCPU)) {
                std::cout << std::endl;
                std::cout << SCPU_FW_FILE_NAME_WRONG_WARNING << std::endl;
                ret = -1;
            }

            if (false == IsBinSizeReasonable(strScpuFilePath, KL720_PRODUCT_NAME, INIT_SELECT_TEXT_SCPU)) {
                std::cout << std::endl;
                std::cout << SCPU_FW_FILE_SIZE_WRONG_WARNING << std::endl;
                ret = -1;
            }
        }

        // further check if firmware bin file does not contain extra info
        // check ncpu firmware
        if (BCEC_NO_INFO_EMBEDDED == ncpuBinCheckRet)
        {
            if (false == IsFileNameCorrect(strNcpuFilePath, INIT_SELECT_TEXT_NCPU)) {
                std::cout << std::endl;
                std::cout << NCPU_FW_FILE_NAME_WRONG_WARNING << std::endl;
                ret = -1;
            }

            if (false == IsBinSizeReasonable(strNcpuFilePath, KL720_PRODUCT_NAME, INIT_SELECT_TEXT_NCPU)) {
                std::cout << std::endl;
                std::cout << NCPU_FW_FILE_SIZE_WRONG_WARNING << std::endl;
                ret = -1;
            }
        }

        if (ret < 0) {
            return -1;
        }
    }

    if (true == ArgumentMap[ON_QUIET].empty() && false == ArgumentMap[ON_720_UPDATE].empty() &&
        false == GetResponseFromUser(UPDATE_KL720_FLASH_WARNING, UPDATE_PROCEED_MSG)) {
        return -1;
    }

    for (size_t i = 0; i < PortIdList.size(); i++) {
        int ErrorCode = KDP_MAGIC_CONNECTION_PASS;
        kp_device_group_t Devices;
        _kp_devices_group_t *pDeviceList;
        int Ret = -1;
        int PortId = PortIdList[i];

        std::cout << std::endl;
        std::cout << "Start Update Firmware to Device with Port Id " << PortId << std::endl;

        Devices = kp_connect_devices(1, &PortId, &ErrorCode);
        pDeviceList = reinterpret_cast<_kp_devices_group_t *>(Devices);

        if ((nullptr == Devices) || (1 > pDeviceList->num_device)) {
            Ret = KP_ERROR_DEVICE_NOT_EXIST_10;
            goto KDP_LOOP_OUT;
        }

        if ((KL520_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) && (false == ArgumentMap[ON_520_UPDATE].empty())) {
            std::cout << std::endl;
            std::cout << "Device with Port Id " << PortId << " is not " << KL520_PRODUCT_NAME << std::endl;
            goto KDP_LOOP_OUT;
        } else if ((KL720_PRODUCT_ID_1 != pDeviceList->ll_device[0]->dev_descp.product_id) &&
                    (KL720_PRODUCT_ID_2 != pDeviceList->ll_device[0]->dev_descp.product_id) &&
                    (false == ArgumentMap[ON_720_UPDATE].empty())) {
            std::cout << std::endl;
            std::cout << "Device with Port Id " << PortId << " is not " << KL720_PRODUCT_NAME << std::endl;
            goto KDP_LOOP_OUT;
        } else if ((KL630_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) &&
                   (false == ArgumentMap[ON_630_UPDATE_LOADER].empty())) {
           std::cout << std::endl;
           std::cout << "Device with Port Id " << PortId << " is not " << KL720_PRODUCT_NAME << std::endl;
           goto KDP_LOOP_OUT;
       } else if (KP_USB_SPEED_SUPER != pDeviceList->ll_device[0]->dev_descp.link_speed) {
            if ((KL720_PRODUCT_ID_1 == pDeviceList->ll_device[0]->dev_descp.product_id) || (KL720_PRODUCT_ID_2 == pDeviceList->ll_device[0]->dev_descp.product_id)) {
                std::cout << std::endl;
                std::cout << "KL720 with Port Id " << PortId << " is not on Usb Super-Speed. Update process skips this device..." << std::endl;
                goto KDP_LOOP_OUT;
            }
        }

        kp_set_timeout(Devices, 20000); // 20 secs timeout

        if (KDP2_LOADER_ONLY == std::string(pDeviceList->ll_device[0]->dev_descp.firmware)) {
            Ret = kp_load_firmware_from_file(Devices, strFlashHelperPath.c_str(), nullptr);

            if (KP_SUCCESS != Ret) {
                goto KDP_LOOP_OUT;
            }

            SLEEP(USB_WAIT_CONNECT_DELAY_MS);
        }

        if ((false == ArgumentMap[ON_520_UPDATE].empty()) || (false == ArgumentMap[ON_720_UPDATE].empty())) {
            if (true == AUTO_REBOOT) {
                Ret = kp_update_kdp_firmware_from_files(Devices, ArgumentMap[ON_SCPU].c_str(), ArgumentMap[ON_NCPU].c_str(), true);
            } else {
                Ret = kp_update_kdp_firmware_from_files(Devices, ArgumentMap[ON_SCPU].c_str(), nullptr, false);

                if (KP_SUCCESS != Ret) {
                    goto KDP_LOOP_OUT;
                }

                Devices = RebootAndReconnect(Devices, PortId, &Ret);

                if (nullptr == Devices) {
                    goto KDP_LOOP_OUT;
                }

                kp_set_timeout(Devices, 20000); // 20 secs timeout

                Ret = kp_update_kdp_firmware_from_files(Devices, nullptr, ArgumentMap[ON_NCPU].c_str(), false);

                if (KP_SUCCESS != Ret) {
                    goto KDP_LOOP_OUT;
                }

                Devices = RebootAndReconnect(Devices, PortId, &Ret);
            }
        } else if (false == ArgumentMap[ON_630_UPDATE_LOADER].empty()) {
            if (true == AUTO_REBOOT) {
                Ret = kp_update_kdp2_usb_loader_from_file(Devices, ArgumentMap[ON_SCPU].c_str(), true);
            } else {
                Ret = kp_update_kdp2_usb_loader_from_file(Devices, ArgumentMap[ON_SCPU].c_str(), false);

                if (KP_SUCCESS != Ret) {
                    goto KDP_LOOP_OUT;
                }

                Devices = RebootAndReconnect(Devices, PortId, &Ret);

                if (nullptr == Devices) {
                    goto KDP_LOOP_OUT;
                }
            }
        }

KDP_LOOP_OUT:

        if ((nullptr != Devices) && (KP_ERROR_DEVICE_NOT_EXIST_10 != Ret)) {
            kp_disconnect_devices(Devices);
        } else if (KP_ERROR_DEVICE_NOT_EXIST_10 == Ret) {
            if ((false == ArgumentMap[ON_520_UPDATE].empty()) &&
                (false == IsDriverInstalled(KP_DEVICE_KL520))) {
                Ret = InstallDriver(ArgumentMap);
            }
            if ((false == ArgumentMap[ON_720_UPDATE].empty()) &&
                ((false == IsDriverInstalled(KP_DEVICE_KL720)) ||
                (false == IsDriverInstalled(KP_DEVICE_KL720_LEGACY)))) {
                Ret = InstallDriver(ArgumentMap);
            }
            if ((false == ArgumentMap[ON_630_UPDATE_LOADER].empty()) &&
                (false == IsDriverInstalled(KP_DEVICE_KL630))) {
                Ret = InstallDriver(ArgumentMap);
            }
        }

        std::string strMessage = (0 == Ret) ? " Succeeded" : (" Failed with Error Code: " + std::to_string(Ret));

        std::cout << std::endl;
        std::cout << "==== Update Firmware to Device with Port Id: " << PortId << strMessage << " ====" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

int UpdateModelToFlash(std::vector<int> PortIdList, std::unordered_map<char, std::string> ArgumentMap, std::string strFlashHelperPath)
{
    int ModelTarget = CheckModel(ArgumentMap[ON_FLASH_MODEL]);

    for (size_t i = 0; i < PortIdList.size(); i++) {
        int ErrorCode = KDP_MAGIC_CONNECTION_PASS;
        kp_device_group_t Devices;
        _kp_devices_group_t *pDeviceList;
        int Ret = -1;
        int PortId = PortIdList[i];

        std::cout << std::endl;
        std::cout << "Start Update Model to Device with Port Id " << PortId << std::endl;

        if (true == ArgumentMap[ON_QUIET].empty() &&
            false == GetResponseFromUser(UPDATE_MODEL_TIME_WARNING, UPDATE_PROCEED_MSG))
        {
            exit(0);
        }

        Devices = kp_connect_devices(1, &PortId, &ErrorCode);
        pDeviceList = reinterpret_cast<_kp_devices_group_t *>(Devices);

        if (nullptr == Devices) {
            Ret = KP_ERROR_DEVICE_NOT_EXIST_10;
            goto MODEL_LOOP_OUT;
        }

        if (KL520_PRODUCT_NAME == ArgumentMap[ON_TYPE]) {
            if (KL520_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) {
                std::cout << std::endl;
                std::cout << "Device with Port Id " << PortId << " is not " << ArgumentMap[ON_TYPE] << std::endl;
                goto MODEL_LOOP_OUT;
            } else if (KL520_PRODUCT_ID != ModelTarget) {
                std::cout << std::endl;
                std::cout << "This Model is not for "<< KL520_PRODUCT_NAME << ", but Device with Port Id " << PortId << " is " << KL520_PRODUCT_NAME << std::endl;
                goto MODEL_LOOP_OUT;
            }
        } else if (KL630_PRODUCT_NAME == ArgumentMap[ON_TYPE]) {
            if (KL630_PRODUCT_ID != pDeviceList->ll_device[0]->dev_descp.product_id) {
                std::cout << std::endl;
                std::cout << "Device with Port Id " << PortId << " is not " << ArgumentMap[ON_TYPE] << std::endl;
                goto MODEL_LOOP_OUT;
            } else if (KL630_PRODUCT_ID != ModelTarget) {
                std::cout << std::endl;
                std::cout << "This Model is not for "<< KL630_PRODUCT_NAME << ", but Device with Port Id " << PortId << " is " << KL520_PRODUCT_NAME << std::endl;
                goto MODEL_LOOP_OUT;
            }
        } else if (KL720_PRODUCT_NAME == ArgumentMap[ON_TYPE]) {
            if ((KL720_PRODUCT_ID_1 != pDeviceList->ll_device[0]->dev_descp.product_id) && (KL720_PRODUCT_ID_2 != pDeviceList->ll_device[0]->dev_descp.product_id)) {
                std::cout << std::endl;
                std::cout << "Device with Port Id " << PortId << " is not " << ArgumentMap[ON_TYPE] << std::endl;
                goto MODEL_LOOP_OUT;
            } else if (KP_USB_SPEED_SUPER != pDeviceList->ll_device[0]->dev_descp.link_speed) {
                std::cout << std::endl;
                std::cout << "KL720 with Port Id " << PortId << " is not on Usb Super-Speed." << std::endl;
                goto MODEL_LOOP_OUT;
            } else if (KL720_PRODUCT_ID_2 != ModelTarget) {
                std::cout << std::endl;
                std::cout << "This Model is not for "<< KL720_PRODUCT_NAME << ", but Device with Port Id " << PortId << " is " << KL720_PRODUCT_NAME << std::endl;
                goto MODEL_LOOP_OUT;
            }
        } else {
            std::cout << std::endl;
            std::cout << ArgumentMap[ON_TYPE] << " is not supported." << std::endl;
            goto MODEL_LOOP_OUT;
        }

        kp_set_timeout(Devices, 200000); // 200 secs timeout, write model to flash need longer time than usual

        if ((KDP_FIRMWARE != std::string(pDeviceList->ll_device[0]->dev_descp.firmware)) &&
            ((KL520_PRODUCT_ID == pDeviceList->ll_device[0]->dev_descp.product_id) ||
             (KL630_PRODUCT_ID == pDeviceList->ll_device[0]->dev_descp.product_id))) {
            Ret = kp_load_firmware_from_file(Devices, strFlashHelperPath.c_str(), nullptr);

            if (KP_SUCCESS != Ret) {
                goto MODEL_LOOP_OUT;
            }
        }

        Ret = kp_update_model_from_file(Devices, ArgumentMap[ON_FLASH_MODEL].c_str(), AUTO_REBOOT, NULL);

        if (KP_SUCCESS != Ret) {
            goto MODEL_LOOP_OUT;
        }

        if (false == AUTO_REBOOT) {
            Devices = RebootAndReconnect(Devices, PortId, &Ret);
        }

MODEL_LOOP_OUT:

        if (nullptr != Devices) {
            kp_disconnect_devices(Devices);
        }

        std::string strMessage = (0 == Ret) ? " Succeeded" : (" Failed with Error Code: " + std::to_string(Ret));

        std::cout << std::endl;
        std::cout << "==== Update Model to Device with Port Id: " << PortId << strMessage << " ====" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
