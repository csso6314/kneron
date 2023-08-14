#ifndef __COM_ERR_H__
#define __COM_ERR_H__

/* These values should not be modified since already released. */

enum com_err {
    SUCCESS = 0,
    UNKNOWN_ERR = -1,

    MSG_APP_DB_NO_MATCH = 10,
    MSG_APP_NOT_LOADED = 100,
    MSG_APP_BAD_MODE = 245,   // bad application mode
    MSG_DB_ADD_FM_FAIL,       // add DB uid_indx failed
    MSG_DB_DEL_FM_FAIL,       // delete DB uid_indx failed
    MSG_DB_BAD_PARAM,         // database action/format/config error
    MSG_SFID_OUT_FAIL,        // data upload failed
    MSG_SFID_NO_MEM,          // memory allocation failed
    MSG_AUTH_FAIL,            // authentication failed
    MSG_FLASH_FAIL,           // flash programming failed (bad sector?)
    MSG_DATA_ERROR,           // data error (I/O)
    MSG_SFID_USR_ZERO,        // user id zero error
    MSG_CONFIG_ERROR = 255,   // no appropriate Start issued previously

    MSG_APP_BAD_IMAGE = 256,
    MSG_APP_BAD_INDEX,
    MSG_APP_UID_EXIST,
    MSG_APP_UID_DIFF,
    MSG_APP_IDX_FIRST,
    MSG_APP_IDX_MISSING,
    MSG_APP_DB_NO_USER,
    MSG_APP_DB_FAIL,
    MSG_APP_LV_FAIL,

    MSG_DB_QUERY_FM_FAIL     // query DB fm data failed
};

#endif
