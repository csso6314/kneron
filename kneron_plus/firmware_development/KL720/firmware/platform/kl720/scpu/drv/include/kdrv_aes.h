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

/**@addtogroup  KDRV_AES  KDRV_AES
 * @{
 * @brief       Kneron aes driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_AES_H__
#define __KDRV_AES_H__

#include <stdint.h>
#include "kdrv_status.h"
#include "kdrv_crypto.h"

/** @brief AES offset for Configuration word in DMA Scatter-Gather Tag */
#define AES_OFFSET_CFG        0
#define AES_OFFSET_KEY        8
#define AES_OFFSET_IV        40
#define AES_OFFSET_IV2       56
#define AES_OFFSET_KEY2      72
#define AES_OFFSET_MASK      104

/** @brief AES Mode Register value for ECB mode of operation */
#define AES_MODEID_ECB        BIT(8) 
/** @brief AES Mode Register value for CBC mode of operation */
#define AES_MODEID_CBC        BIT(9) 
/** @brief AES Mode Register value for CTR mode of operation */
#define AES_MODEID_CTR        BIT(10) 
/** @brief AES Mode Register value for CFB mode of operation */
#define AES_MODEID_CFB        BIT(11) 
/** @brief AES Mode Register value for OFB mode of operation */
#define AES_MODEID_OFB        BIT(12) 
/** @brief AES Mode Register value for CCM mode of operation */
#define AES_MODEID_CCM        BIT(13) 
/** @brief AES Mode Register value for GCM mode of operation */
#define AES_MODEID_GCM        BIT(14) 
/** @brief AES Mode Register value for XTS mode of operation */
#define AES_MODEID_XTS        BIT(15) 
/** @brief AES Mode Register value for CMAC mode of operation */
#define AES_MODEID_CMA        BIT(16) 
/** @brief AES Mode Register value for AES context saving */
#define AES_MODEID_CX_SAVE    BIT(5) 
/** @brief AES Mode Register value for AES context loading */
#define AES_MODEID_CX_LOAD    BIT(4)
/** @brief AES Mode Register value for AES no context */
#define AES_MODEID_NO_CX      0x00000000
/** @brief AES Mode Register value for AES keysize of 128 bits */
#define AES_MODEID_AES128     0x00000000
/** @brief AES Mode Register value for AES keysize of 192 bits */
#define AES_MODEID_AES192     BIT(3) 
/** @brief AES Mode Register value for AES keysize of 256 bits */
#define AES_MODEID_AES256     BIT(2) 
/** @brief AES Mode Register value for encryption mode */
#define AES_MODEID_ENCRYPT    0x00000000
/** @brief AES Mode Register value for decryption mode */
#define AES_MODEID_DECRYPT    BIT(0) 
/** @brief AES Mode Register value to use Key1 */
#define AES_MODEID_KEY1       BIT(6) 
/** @brief AES Mode Register value to use Key2 */
#define AES_MODEID_KEY2       BIT(7) 


/** @brief AES Mode Register mask for hardware key 1 & 2*/
#define AES_MODEID_KEYX_MASK     0x000000C0


/* AES hardware configuration - register 1*/
#define AES_HW_CFG_ECB_SUPPORTED_MASK     BIT(0) 
#define AES_HW_CFG_CBC_SUPPORTED_MASK     BIT(1)
#define AES_HW_CFG_CTR_SUPPORTED_MASK     BIT(2)
#define AES_HW_CFG_CFB_SUPPORTED_MASK     BIT(3)
#define AES_HW_CFG_OFB_SUPPORTED_MASK     BIT(4)
#define AES_HW_CFG_CCM_SUPPORTED_MASK     BIT(5)
#define AES_HW_CFG_GCM_SUPPORTED_MASK     BIT(6)
#define AES_HW_CFG_XTS_SUPPORTED_MASK     BIT(7)
#define AES_HW_CFG_CMAC_SUPPORTED_MASK    BIT(8)
#define AES_HW_CFG_KEY_SIZE_LSB           24
#define AES_HW_CFG_KEY_SIZE_MASK          (0x7 << AES_HW_CFG_KEY_SIZE_LSB)
#define AES_HW_CFG_KEY_SIZE_128_SUPPORTED_MASK BIT(24)
#define AES_HW_CFG_KEY_SIZE_192_SUPPORTED_MASK BIT(25) 
#define AES_HW_CFG_KEY_SIZE_256_SUPPORTED_MASK BIT(26) 

#define AES_HW_CFG_1 (*(const volatile uint32_t*)CRYPTO_AES_HW_CFG_1_REG)

/* AES hardware configuration - register 2*/
#define AES_HW_CFG_MAX_CTR_SIZE_LSB       0
#define AES_HW_CFG_MAX_CTR_SIZE_MASK      (0xFFFF<<AES_HW_CFG_MAX_CTR_SIZE_LSB)

#define AES_HW_CFG_2 (*(const volatile uint32_t*)CRYPTO_AES_HW_CFG_2_REG)

/** This structure hosts a copy of the HW configuration registers */
struct kdrv_aes_engine_config_t {
   bool is_config_read; /**< is set the first time we retrieve the hardware
                             configuration from the engine */
   bool is_engine_available;
   bool is_ecb_supported; /**< Indicates if the ECB mode is enabled */
   bool is_cbc_supported; /**< Indicates if the CBC mode is enabled */
   bool is_ctr_supported; /**< Indicates if the CTR mode is enabled */
   bool is_cfb_supported; /**< Indicates if the CFB mode is enabled */
   bool is_ofb_supported; /**< Indicates if the OFB mode is enabled */
   bool is_gcm_supported; /**< Indicates if the GCM mode is enabled */
   bool is_ccm_supported; /**< Indicates if the CCM mode is enabled */
   bool is_xts_supported; /**< Indicates if the XTS mode is enabled */
   bool is_cmac_supported; /**< Indicates if the CMAC mode is enabled */
   bool is_cipher_stealing_enabled; /**< Indicates if cipher stealing is
                                         enabled for CBC and XTS, allowing last
                                         data block to not be aligned on 128b */
   bool is_coutermeasure_enabled; /**< Indicates if countermeasure is enabled */
   bool is_key_128b_supported; /**< Indicates if 128b keys are supported */
   bool is_key_192b_supported; /**< Indicates if 192b key are supported */
   bool is_key_256b_supported; /**< Indicates if 256b key are supported */
   uint32_t n_bits_counter_modes; /**< size in bits of counter used in CTR/CCM */
};

/** @brief Reads and returns the HW configuration of the AES engine.
* @note kdrv_aes_initialize() must be previously called
*/
const struct kdrv_aes_engine_config_t* kdrv_aes_initialize(void);


/** @brief Size for IV in GCM mode */
#define AES_IV_GCM_SIZE       12
/** @brief Size for IV in all modes except GCM */
#define AES_IV_SIZE           16
/** @brief Size for Context in GCM and CCM modes */
#define AES_CTX_xCM_SIZE      32
/** @brief Size for Context in all modes except GCM and CCM */
#define AES_CTX_SIZE          16
/** @brief Size of a GCM/GMAC/CMAC message authentification code (MAC) or maximum size of a CCM MAC */
#define AES_MAC_SIZE          16

/**
 * @brief Dummy variables to use hardware keys Key1 and Key2
 *
 * @note They are declared \c extern for internal reasons, user should \e not\n
 *       use them because they may disappear in future release.
 */
extern uint8_t aes_hw_key2;
extern uint8_t aes_hw_key1;

/**
 * @brief First Hardware Key (of 128bits)
 * @details Two secret hardware keys may be wired directly into to AES module, preventing\n
 * the CPU to read them back. This block_t provides an abstraction to pass them\n
 * as input (user should not use them for anything else as input) in the same way that user AES keys.
 */
#define AES_KEY1_128 block_t_convert(&aes_hw_key1, 128/8)
/** @brief First Hardware Key of 256b (for description, see ::AES_KEY1_128) */
#define AES_KEY1_256 block_t_convert(&aes_hw_key1, 256/8)
/** @brief Second Hardware Key of 128b (for description, see ::AES_KEY1_128) */
#define AES_KEY2_128 block_t_convert(&aes_hw_key2, 128/8)
/** @brief Second Hardware Key of 256b (for description, see ::AES_KEY1_128) */
#define AES_KEY2_256 block_t_convert(&aes_hw_key2, 256/8)

/** 
 * @brief kdrv_aes_cbc_encrypt encryption operation using AES-CBC
 *
 * @param[in]   key is the key involved to encrypt the plaintext
 * @param[in]   iv is the input initialization vector
 * @param[in]   plaintext is the input data to encrypt
 * @param[out]  ciphertext is the output encrypted data
 * @return      kdrv_status_t
 */
kdrv_status_t kdrv_aes_cbc_encrypt(const block_t *key, const block_t *iv, 
                    const block_t *plaintext, block_t *ciphertext);

/** 
 * @brief kdrv_aes_cbc_decrypt decrypt operation using AES-CBC
 *
 * @param[in]   key is the key involved to decrypt the ciphertext
 * @param[in]   iv is the input initialization vector
 * @param[in]   ciphertext is the input data to decrypt
 * @param[out]  plaintext is the output decrypted data
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_aes_cbc_decrypt(const block_t *key, const block_t *iv, 
                    const block_t *ciphertext, block_t *plaintext);

#endif
