#ifndef WARNING_MESSAGES_H
#define WARNING_MESSAGES_H

#define UPDATE_LOADER_WARNING                 "One or more selected dongles are already running USB Loader."
#define UPDATE_LOADER_WARNING_2               "The dongle is already running USB Loader."
#define UPDATE_KL520_FLASH_WARNING            "This update does not accept Hostlib version firmware. Downgrade firmware to earlier version is not acceptable after update."
#define UPDATE_KL630_FLASH_WARNING            "Downgrade firmware to earlier version is not acceptable after update."
#define UPDATE_KL720_FLASH_WARNING            "Downgrade firmware to earlier version is not acceptable after update."

#define SCPU_FW_FILE_NAME_WRONG_WARNING       "The filename of SCPU must contain \"scpu\" string."
#define NCPU_FW_FILE_NAME_WRONG_WARNING       "The filename of NCPU must contain \"ncpu\" string."
#define SCPU_FW_FILE_SIZE_WRONG_WARNING       "The file size of SCPU firmware is not reasonable."
#define NCPU_FW_FILE_SIZE_WRONG_WARNING       "The file size of NCPU firmware is not reasonable."

#ifdef __WIN32__
#define SCPU_FW_FILE_FULL_PATH_TOO_LONG       "SCPU firmware file full path is too long."
#define NCPU_FW_FILE_FULL_PATH_TOO_LONG       "NCPU firmware file full path is too long."
#endif

#define UPDATE_MODEL_TIME_WARNING             "Updating model may take 200 seconds."
#define UPDATE_PROCEED_MSG                    "Do you want to proceed?"
#define INSTALL_DRIVER_WARNING                "Device driver has not been installed yet.\nInstallation process may take few minutes."
#define INSTALL_DRIVER_ADMIN                  "You may need to run this application as Administrator to install driver."
#define INSTALL_DRIVER_MSG                    "Do you want to install now?"

#endif // WARNING_MESSAGES_H
