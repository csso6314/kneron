#ifndef __KHOST_IPC_SYS_VERSION_H__
#define __KHOST_IPC_SYS_VERSION_H__

#include "khost_ipc_generic.h"

typedef struct khost_ipc_sys_version_s khost_ipc_sys_version_t;

/********************************************************/
/*************** version descriptor data ****************/
/*************** (KHOST_REQUEST_TYPE_VERSION) ***********/
/********************************************************/
/*
 *       |    0    |    1    |    2    |    3    |
 *       -----------------------------------------
 *  0    |                total_size             |
 *       -----------------------------------------
 *  4    |                desc_type              |
 *       -----------------------------------------
 *  8    |                return_code            |
 *       -----------------------------------------
 * 12    |                version                |
 *       -----------------------------------------
 */
struct khost_ipc_sys_version_s {
    /* 1. header */
    uint32_t                      total_size;
    uint32_t                      desc_type;    // see enum KHOST_DESC_TYPE
    uint32_t                      return_code;  // see enum KHOST_IPC_RETURN_CODE

    /* app specifc part */
    uint32_t                      version;
};

#endif    // __KHOST_IPC_SYS_VERSION_h__
