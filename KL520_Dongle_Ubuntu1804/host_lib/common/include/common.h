#ifndef COMMON_H
#define COMMON_H

#define DEVICE_ID           0x520       /**< Kneron device id for KL520 */

#define ALL_USER_UID        0           /**< an alias means ALL USERS */

#define DB_VERSION_DEFAULT  0xFFFFFFFF  /**< DB default version */

#define FID_THRESHOLD       (0.475f)    /**< comparision threshold for 1st DB */

/**
 * @brief to enable FW for MASK FDR
 */
// #define MASK_FDR

/**
 * @brief to enable/disable db compare by NPU
 */
// #define EMBED_CMP_NPU

#ifdef MASK_FDR
  // #define FID_MASK_THRESHOLD  (0.625f)
  #define FID_MASK_THRESHOLD        FID_THRESHOLD

  #define NO_MASK_FDR_DB_INDEX      0
  #define MASK_FDR_DB_INDEX         1

#ifdef EMBED_CMP_NPU
  //limited by current flash layout = ~ 8000 faces
  #define MAX_VALID_FM_DB_IDX       2
  #define MAX_USER                  1000        // MAX user count in DB
  #define MAX_FID                   2           // MAX face count for one user
#else
  #define MAX_VALID_FM_DB_IDX       2
  #define MAX_USER                  20          // MAX user count in DB
  #define MAX_FID                   5           // MAX face count for one user
#endif
#else
#ifdef EMBED_CMP_NPU
  //limited by current flash layout = ~ 8000 faces
  #define MAX_VALID_FM_DB_IDX   2     /**< MAX DB count */
  #define MAX_USER              1000  /**< MAX user count in DB */
  #define MAX_FID               2     /**< MAX face count for one user */
#else
  #define MAX_VALID_FM_DB_IDX   4     /**< MAX DB count */
  #define MAX_USER              20    /**< MAX user count in DB */
  #define MAX_FID               5     /**< MAX face count for one user */
#endif
#endif


#endif
