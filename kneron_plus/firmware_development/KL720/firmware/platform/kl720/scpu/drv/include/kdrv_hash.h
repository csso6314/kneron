/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/

/**@addtogroup  KDRV_HASH  KDRV_HASH
 * @{
 * @brief       Kneron hash driver
 * @version v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_HASH_H__
#define __KDRV_HASH_H__

#include <stdint.h>
#include "kdrv_status.h"
#include "kdrv_crypto.h"


/** @brief Structure of hash config information */
struct hash_regs_s {
   volatile uint32_t config;
};

/** @brief Value for hash_regs_s.config, set mode to MD5 */
#define HASH_CONF_MODE_MD5      BIT(0) //0x00000001 // MD5 mode
#define HASH_CONF_MODE_SHA1     BIT(1) //0x00000002 // SHA1 mode
#define HASH_CONF_MODE_SHA224   BIT(2) //0x00000004 // SHA224 mode 
#define HASH_CONF_MODE_SHA256   BIT(3) //0x00000008 // SHA256 mode
#define HASH_CONF_MODE_SHA384   BIT(4) //0X00000010 // SHA384 mode
#define HASH_CONF_MODE_SHA512   BIT(5) //0x00000020 // SHA512 mode
#define HASH_CONF_MODE_SM3      BIT(6) //0x00000040 // SM3 mode
#define HASH_CONF_HMAC    0x00000100 // enable HMAC
#define HASH_CONF_HWPAD   0x00000200 // enable padding
#define HASH_CONF_FINAL   0x00000400 // set as final (return digest and not state)

/* Hardware configuration register. */
#define HASH_HW_CFG_MD5_SUPPORTED_MASK           BIT(0) // (1L<<HASH_HW_CFG_MD5_SUPPORTED_LSB)
#define HASH_HW_CFG_SHA1_SUPPORTED_MASK          BIT(1) // (1L<<HASH_HW_CFG_SHA1_SUPPORTED_LSB)
#define HASH_HW_CFG_SHA224_SUPPORTED_MASK        BIT(2) // (1L<<HASH_HW_CFG_SHA224_SUPPORTED_LSB)
#define HASH_HW_CFG_SHA256_SUPPORTED_MASK        BIT(3) // (1L<<HASH_HW_CFG_SHA256_SUPPORTED_LSB)
#define HASH_HW_CFG_SHA384_SUPPORTED_MASK        BIT(4) // (1L<<HASH_HW_CFG_SHA384_SUPPORTED_LSB)
#define HASH_HW_CFG_SHA512_SUPPORTED_MASK        BIT(5) // (1L<<HASH_HW_CFG_SHA512_SUPPORTED_LSB)
#define HASH_HW_CFG_SM3_SUPPORTED_MASK           BIT(6) // (1L<<HASH_HW_CFG_SM3_SUPPORTED_LSB)
#define HASH_HW_CFG_PADDING_SUPPORTED_MASK       BIT(16) // (1L<<HASH_HW_CFG_PADDING_SUPPORTED_LSB)
#define HASH_HW_CFG_HMAC_SUPPORTED_MASK          BIT(17) // (1L<<HASH_HW_CFG_HMAC_SUPPORTED_LSB)
#define HASH_HW_CFG_VERIFY_DIGEST_SUPPORTED_MASK BIT(18) // (1L<<HASH_HW_CFG_VERIFY_DIGEST_SUPPORTED_LSB)

#define HASH_HW_CFG (*(const volatile uint32_t*)CRYPTO_HASH_HW_CFG_REG)

/** Structures hosts a copy of the HW configuration registers */
struct kdrv_hash_engine_config_t {
   bool is_config_read; /**< is set the first time we retrieve the hardware configuration from the engine */
   bool is_engine_available;
   bool is_md5_enabled;
   bool is_sha1_enabled;
   bool is_sha224_enabled;
   bool is_sha256_enabled;
   bool is_sha384_enabled;
   bool is_sha512_enabled;
   bool is_hmac_enabled;
   bool is_sm3_enabled;
   bool is_padding_enabled;
};

/**
 * @brief Reads and returns the HW configuration of the hash engine.
 *
 * @param[in]  N/A
 * @return      kdrv_hash_engine_config_t*, Pointer to const @ref kdrv_hash_engine_config_t
 */
const struct kdrv_hash_engine_config_t* kdrv_hash_initialize(void);

/**
 * @brief Simply returns the HW configuration, which should have been
 * previously read using kdrv_hash_initialize().
 * @warning kdrv_hash_initialize() must be previously called
 *
 * @param[in]  N/A
 * @return      kdrv_hash_engine_config_t*, Pointer to const r @ref kdrv_hash_engine_config_t
 */
const struct kdrv_hash_engine_config_t* kdrv_hash_engine_get_config(void);
//-----------------------------------------------------------------------------

/** @brief Size of MD5 data block in bytes */
#define MD5_BLOCKSIZE      64
/** @brief Size of MD5 initialization value in bytes */
#define MD5_INITSIZE       16
/** @brief Size of MD5 digest in bytes */
#define MD5_DIGESTSIZE     16
/** @brief Size of SHA1 data block in bytes */
#define SHA1_BLOCKSIZE     64
/** @brief Size of SHA1 initialization value in bytes */
#define SHA1_INITSIZE      20
/** @brief Size of SHA1 digest in bytes */
#define SHA1_DIGESTSIZE    20
/** @brief Size of SHA224 data block in bytes */
#define SHA224_BLOCKSIZE   64
/** @brief Size of SHA224 initialization value in bytes */
#define SHA224_INITSIZE    32
/** @brief Size of SHA224 digest in bytes */
#define SHA224_DIGESTSIZE  28
/** @brief Size of SHA256 data block in bytes */
#define SHA256_BLOCKSIZE   64
/** @brief Size of SHA256 initialization value in bytes */
#define SHA256_INITSIZE    32
/** @brief Size of SHA256 digest in bytes */
#define SHA256_DIGESTSIZE  32
/** @brief Size of SHA384 data block in bytes */
#define SHA384_BLOCKSIZE   128
/** @brief Size of SHA384 initialization value in bytes */
#define SHA384_INITSIZE    64
/** @brief Size of SHA384 digest in bytes */
#define SHA384_DIGESTSIZE  48
/** @brief Size of SHA512 data block in bytes */
#define SHA512_BLOCKSIZE   128
/** @brief Size of SHA512 initialization value in bytes */
#define SHA512_INITSIZE    64
/** @brief Size of SHA512 digest in bytes */
#define SHA512_DIGESTSIZE  64
/** @brief Size of SM3 digest in bytes */
#define SM3_BLOCKSIZE      64
/** @brief Size of SM3 initialization value in bytes */
#define SM3_INITSIZE       32
/** @brief Size of SM3 digest in bytes */
#define SM3_DIGESTSIZE     32
/** @brief Maximum block size to be supported */
#define MAX_BLOCKSIZE   SHA512_BLOCKSIZE
/** @brief Maximum digest size to be supported */
#define MAX_DIGESTSIZE  SHA512_DIGESTSIZE
/** @brief Maximum number of entries in kdrv_hash_array_blk and kdrv_hash_hmac_array_blk */
#define SX_HASH_ARRAY_MAX_ENTRIES 8

/**
* @brief Enumerations of the supported hash algorithms
*/
typedef enum kdrv_hash_fct_e
{
   e_MD5     = 1,    /**< Enum 1, MD5    */
   e_SHA1    = 2,    /**< Enum 2, SHA1   */
   e_SHA224  = 3,    /**< Enum 3, SHA224 */
   e_SHA256  = 4,    /**< Enum 4, SHA256 */
   e_SHA384  = 5,    /**< Enum 5, SHA384 */
   e_SHA512  = 6,    /**< Enum 6, SHA512 */
   e_SM3     = 7     /**< Enum 7, SM3    */
} kdrv_hash_fct_t;


/**
 * @brief Get digest size in bytes for the given \p hash_fct
 *
 * @param[in]   hash_fct hash function. See @ref kdrv_hash_fct_t.
 * @return digest size in bytes, or 0 if invalid \p hash_fct
 */
uint32_t kdrv_hash_get_digest_size(kdrv_hash_fct_t hash_fct);

/**
 * @brief Get block size in bytes for the given \p hash_fct
 *
 * @param[in]   hash_fct hash function. see @ref  kdrv_hash_fct_t.
 * @return block size in bytes, or 0 if invalid \p hash_fct
 */
uint32_t kdrv_hash_get_block_size(kdrv_hash_fct_t hash_fct);

/**
 * @brief Get state size in bytes for the given \p hash_fct
 *
 * @param[in]   hash_fct hash function. see @ref kdrv_hash_fct_t.
 * @return state size in bytes, or 0 if invalid \p hash_fct
 */
uint32_t kdrv_hash_get_state_size(kdrv_hash_fct_t hash_fct);

/**
 * @brief Compute hash digest of the content of data_in and write the result in data_out.
 *
 * @param[in]   hash_fct hash function to use. See@ref kdrv_hash_fct_t.
 * @param[in]   data_in  array of input data to process, see @ref block_t
 * @param[in]   entries  length of array \p data_in
 * @param[out]  data_out output digest, see @ref block_t
 * @return see @ref kdrv_status_t
 */
kdrv_status_t kdrv_hash_array_blk(kdrv_hash_fct_t hash_fct, block_t data_in[],
                        const unsigned int entries, block_t data_out);

/**
 * @brief Compute hash digest of the content of data_in and write the result in data_out.
 *
 * @param[in]   hash_fct hash function to use, see @ref kdrv_hash_fct_t.
 * @param[in]   data_in input data to process, see @ref block_t
 * @param[out]  data_out output digest, see @ref block_t
 * @return KDRV_STATUS_OK if execution was successful, see @ref kdrv_status_t
 */
kdrv_status_t kdrv_hash_blk(kdrv_hash_fct_t hash_fct, block_t data_in, block_t data_out);


#endif
