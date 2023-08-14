/**
 * Kneron Peripheral API - AES
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <stdbool.h>
#include "base.h"
#include "kdrv_aes.h"
#include "kdrv_crypto.h"
#include "kdrv_status.h"
#include "regbase.h"

uint8_t aes_hw_key1;
uint8_t aes_hw_key2;

static struct kdrv_aes_engine_config_t kdrv_aes_engine_config = {
    .is_config_read = false
};

const struct kdrv_aes_engine_config_t* kdrv_aes_initialize()
{
    uint32_t config_word;

    if (!kdrv_aes_engine_config.is_config_read) {
        kdrv_aes_engine_config.is_engine_available = \
                        ((CRYPTOSOC_INCL_IPS_HW_CFG & CRYPTOSOC_HW_CFG_AES_IP_INCLUDED_MASK) != 0);
        if (kdrv_aes_engine_config.is_engine_available) {
            config_word = (uint32_t) AES_HW_CFG_1;
            kdrv_aes_engine_config.is_cbc_supported =
                (config_word & AES_HW_CFG_CBC_SUPPORTED_MASK) != 0;
            kdrv_aes_engine_config.is_key_128b_supported =
                (config_word & AES_HW_CFG_KEY_SIZE_128_SUPPORTED_MASK) != 0;
            kdrv_aes_engine_config.is_key_192b_supported =
                (config_word & AES_HW_CFG_KEY_SIZE_192_SUPPORTED_MASK) != 0;
            kdrv_aes_engine_config.is_key_256b_supported =
                (config_word & AES_HW_CFG_KEY_SIZE_256_SUPPORTED_MASK) != 0;

            config_word = (uint32_t) AES_HW_CFG_2;
            kdrv_aes_engine_config.n_bits_counter_modes =
                (config_word & AES_HW_CFG_MAX_CTR_SIZE_MASK) >>
                AES_HW_CFG_MAX_CTR_SIZE_LSB;
        }
    }

    kdrv_aes_engine_config.is_config_read = true;
    return &kdrv_aes_engine_config;
}

const struct kdrv_aes_engine_config_t* kdrv_aes_engine_get_config()
{
    return &kdrv_aes_engine_config;
}


//-----------------------------------------------------------------------------
static kdrv_status_t kdrv_aes_build_descr(uint32_t hw_config, const block_t *key,
    const block_t *xtskey, const block_t *iv, const block_t *datain,
    block_t *dataout, const block_t *aad1, const block_t *aad2,
    const block_t *extrain, block_t *tag_out, block_t *ctx_ptr);

/** Update fields related to the key characteristics for AES operation
 *
 * The AES has a 32b configuration register with many fields. A few of them
 * depends of the AES key itself. The function set these fields only and let
 * the others unchanged. It allows to decouple key related configuration with
 * the algorithm (CBC/ECB,...) and the operation (encryption/decryption, ...)
 *
 * @param key AES key involved.
 * @param config A 32b word to update based on the \c key characteristics
 * @return KDRV_STATUS_OK if key length is recognised (128 ,192 or 256b) and
 *         supported.
 *         KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR if key length is recognized but not supported
 *         KDRV_STATUS_CRYPTO_INVALID_PARAM if key length is not recognised.
 */
static kdrv_status_t kdrv_set_hw_config_for_key(block_t *key, uint32_t *config)
{
    // Check and configure for key length
    switch(key->len) {
    case 16:
        if (!kdrv_aes_engine_get_config()->is_key_128b_supported)
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        *config |= AES_MODEID_AES128;
        break;
    case 24:
        if (!kdrv_aes_engine_get_config()->is_key_192b_supported)
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        *config |= AES_MODEID_AES192;
        break;
    case 32:
        if (!kdrv_aes_engine_get_config()->is_key_256b_supported)
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        *config |= AES_MODEID_AES256;
        break;
    default:
        return KDRV_STATUS_CRYPTO_INVALID_PARAM;
    }

    // Configure to use HW keys
    if (key->addr == &aes_hw_key1) {
        *config |= AES_MODEID_KEY1;
        key->len = 0; // We don't need to transfer key content when using HW ones
    } else if (key->addr == &aes_hw_key2) {
        *config |= AES_MODEID_KEY2;
        key->len = 0; // We don't need to transfer key content when using HW ones
    }

    return KDRV_STATUS_OK;
}

/**
 * @brief kdrv_is_len_valid_for_cbc_xts check the aes valid length.
 *      AES is block cipher, the input size always the output size.
 *      The input to be multiple of block size (16 bytes).
 * @param[in]       len         input length
 * @return          bool 
 */
static bool kdrv_is_len_valid_for_cbc_xts(uint32_t len)
{
    if (kdrv_aes_engine_get_config()->is_cipher_stealing_enabled)
        return len >= 16;
    return !(len % 16) && (len > 0);
}


// -----------------------------------------------------------------------------
/**
 * @brief kdrv_aes_cbc AES CBC encryption/decryption function
 * 
 * @param[in] key           aes key value
 * @param[in] datain        data input
 * @param[out] dataout      data output
 * @param[in] iv            aes initial vector
 * @param[in] dir           aes cbc dirction(0: encryp, 1: decrypt)
 *
 * @return kdrv_status_t
 */
static kdrv_status_t kdrv_aes_cbc(const block_t *key, const block_t *datain, block_t *dataout,
                           const block_t *iv, uint32_t dir)
{
    uint32_t config = AES_MODEID_CBC | AES_MODEID_NO_CX | dir;
    if (!kdrv_aes_engine_get_config()->is_cbc_supported)
        return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;

    if (!kdrv_is_len_valid_for_cbc_xts(datain->len) || (dataout->len != datain->len) ||
        (iv->len != AES_IV_SIZE))
        return KDRV_STATUS_CRYPTO_INVALID_PARAM;

    block_t dummy = null_blk;
    return kdrv_aes_build_descr(config, key, &dummy, iv, datain, dataout,
                              &dummy, &dummy, &dummy, &dummy, &dummy);
}

/**
 * @brief kdrv_aes_cbc AES CBC encrypt function
 * 
 * @param[in] key           aes key value
 * @param[in] iv            aes initial vector
 * @param[in] plaintext     plain text source input
 * @param[out] ciphertext   cypher text output
 *
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_aes_cbc_encrypt(const block_t *key, const block_t *iv, 
                        const block_t *plaintext, block_t *ciphertext)
{
    return kdrv_aes_cbc(key, plaintext, ciphertext, iv, AES_MODEID_ENCRYPT);
}

/**
 * @brief kdrv_aes_cbc AES CBC decrypt function
 * 
 * @param[in] key           aes key value
 * @param[in] iv            aes initial vector
 * @param[out] ciphertext   cypher text source input 
 * @param[in] plaintext     plain text output
 *
 * @return kdrv_status_t
 */
kdrv_status_t kdrv_aes_cbc_decrypt(const block_t *key, const block_t *iv,
                            const block_t *ciphertext, block_t *plaintext)
{
    return kdrv_aes_cbc(key, ciphertext, plaintext, iv, AES_MODEID_DECRYPT);
}

/**
 * @brief Get aes padding length (realign on aes block size)
 *
 * @param[in] input_length      input length
 *
 * @return length of padding
 */
static uint32_t kdrv_get_aes_pad_len(uint32_t input_length)
{
    return (16-input_length)&15;
}

/**
 * @brief Build descriptors and call cryptoDMA for AES operation
 * @param config value for cfg mode register
 * @param key AES key
 * @param xtskey XTS key.
 * @param iv initialization vector
 * @param datain input data (plaintext or ciphertext)
 * @param dataout output data (ciphertext or plaintext)
 * @param aad1 additional authenticated data part #1
 * @param aad2 additional authenticated data part #2
 * @param extrain additional input data expected by ::GCM or ::CCM
 * @param tag_out authentication tag input for ::CCM, ::GCM & ::CMAC
 * @param ctx_ptr AES context output
 * @return kdrv_status_t
 */
static kdrv_status_t kdrv_aes_build_descr(uint32_t hw_config, const block_t *key,
    const block_t *xtskey, const block_t *iv, const block_t *datain,
    block_t *dataout, const block_t *aad1, const block_t *aad2,
    const block_t *extrain, block_t *tag_out, block_t *ctx_ptr)
{
    block_t config = block_t_convert(&hw_config, sizeof(hw_config));
    block_t keyb = *key;
    kdrv_status_t status = kdrv_set_hw_config_for_key(&keyb, &hw_config);
    if (status)
        return status;

    struct kdrv_crypto_dma_descr_s desc_to[8];  //could be reduces as no use case where 
                                       //all inputs are used, but safer like this
    struct kdrv_crypto_dma_descr_s desc_fr[6];
    struct kdrv_crypto_dma_descr_s *d;  // pointer to current descriptor
    block_t datainb = *datain;
    // input padding
    uint32_t aad_zeropad_len      = kdrv_get_aes_pad_len(aad1->len + aad2->len);
    uint32_t datain_zeropad_len   = kdrv_get_aes_pad_len(datain->len);
    uint32_t extrain_zeropad_len    = kdrv_get_aes_pad_len(extrain->len);

    // handle alignment for data out
    if(dataout->flags & DMA_AXI_DESCR_CONST_ADDR)
        dataout->len = roundup_32(dataout->len);

    if(tag_out->flags & DMA_AXI_DESCR_CONST_ADDR)
        tag_out->len = roundup_32(tag_out->len);

    // output discards
    block_t aads_discard      = block_t_convert(NULL, aad1->len + aad2->len + aad_zeropad_len);
    block_t dataout_discard   = block_t_convert(NULL, kdrv_get_aes_pad_len(dataout->len));
    block_t tagout_discard    = block_t_convert(NULL, kdrv_get_aes_pad_len(tag_out->len));

    // no input provided, -> empty input = 1 block of zero padding (i.e. for CMAC)
    uint8_t zeroes[16] = {0};
    if (!datain->len && !extrain->len && !aad1->len && !aad2->len ) {
        datain_zeropad_len = 16;
        datainb = block_t_convert(zeroes, datain_zeropad_len);
    }

    // fetcher descriptors
    d = desc_to;

    // Config
    d = kdrv_write_desc_blk(d, &config, DMA_AXI_DESCR_REALIGN,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISCONFIG | 
            DMA_SG_TAG_SETCFGOFFSET(AES_OFFSET_CFG));

    // Symmetric key
    d = kdrv_write_desc_blk(d, &keyb, DMA_AXI_DESCR_REALIGN,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISCONFIG |
            DMA_SG_TAG_SETCFGOFFSET(AES_OFFSET_KEY));

    // IV or context (if existing)
    d = kdrv_write_desc_blk(d, iv, DMA_AXI_DESCR_REALIGN,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISCONFIG |
            DMA_SG_TAG_SETCFGOFFSET(AES_OFFSET_IV));

    // XTS key (if existing)
    d = kdrv_write_desc_blk(d, xtskey, DMA_AXI_DESCR_REALIGN,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISCONFIG |
            DMA_SG_TAG_SETCFGOFFSET(AES_OFFSET_KEY2));

    // authentication data (if existing)
    d = kdrv_write_desc_blk(d, aad1, 0,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISDATA |
            DMA_SG_TAG_DATATYPE_AESHEADER);
    d = kdrv_write_desc_blk(d, aad2, DMA_AXI_DESCR_REALIGN,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISDATA |
            DMA_SG_TAG_DATATYPE_AESHEADER |
            DMA_SG_TAG_SETINVALIDBYTES(aad_zeropad_len));

    // Input data (if existing)
    d = kdrv_write_desc_blk(d, &datainb, DMA_AXI_DESCR_REALIGN,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISDATA |
            DMA_SG_TAG_DATATYPE_AESPAYLOAD |
            DMA_SG_TAG_SETINVALIDBYTES(datain_zeropad_len));

    // Additional input data (may be tag_in for CCM or len_a_c for GCM)
    d = kdrv_write_desc_blk(
            d, extrain, DMA_AXI_DESCR_REALIGN,
            DMA_SG_ENGINESELECT_AES | DMA_SG_TAG_ISDATA |
            DMA_SG_TAG_DATATYPE_AESPAYLOAD |
            DMA_SG_TAG_SETINVALIDBYTES(extrain_zeropad_len));

    kdrv_crypto_set_last_desc(d - 1);

    // pusher descriptors
    d = desc_fr;

    // discard output aad and associated padding
    d = kdrv_write_desc_blk(d, &aads_discard, 0, 0);

    // Output data
    d = kdrv_write_desc_blk(d, dataout, 0, 0);
    d = kdrv_write_desc_blk(d, &dataout_discard, 0, 0);

    // Output tag (if existing)
    d = kdrv_write_desc_blk(d, tag_out, 0, 0);
    d = kdrv_write_desc_blk(d, &tagout_discard, 0, 0);

    // Output context (if existing)
    d = kdrv_write_desc_blk(d, ctx_ptr, 0, 0);

    kdrv_crypto_set_last_desc(d - 1);

    // launch cryptodma
    kdrv_cryptodma_run_sg(desc_to, desc_fr);

    return KDRV_STATUS_OK;
}

