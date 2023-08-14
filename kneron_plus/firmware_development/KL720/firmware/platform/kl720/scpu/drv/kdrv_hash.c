/**
 * Kneron Peripheral API - HASH
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
 
#include <string.h>
#include "base.h"
#include "kdrv_hash.h"
#include "kdrv_crypto.h"
#include "kdrv_status.h"
#include "regbase.h"

uint32_t kdrv_hash_hmac_array_blk(kdrv_hash_fct_t hash_fct, block_t key, block_t data_in[],
                        const unsigned int entries, block_t data_out);
uint32_t kdrv_hash_hmac_blk(kdrv_hash_fct_t hash_fct, block_t key,
                            block_t data_in, block_t data_out);
uint32_t kdrv_hash_update_blk(kdrv_hash_fct_t hash_fct, block_t state, block_t data);
uint32_t kdrv_hash_finish_blk(kdrv_hash_fct_t hash_fct, block_t state, block_t data_in, 
                            block_t data_out, uint32_t total_len);

static struct kdrv_hash_engine_config_t hash_engine_config = {
    .is_config_read = false
};

const struct kdrv_hash_engine_config_t* kdrv_hash_initialize()
{
    uint32_t configuration;

    if (!hash_engine_config.is_config_read) {
        configuration = CRYPTOSOC_INCL_IPS_HW_CFG;
        hash_engine_config.is_engine_available = \
                ((CRYPTOSOC_INCL_IPS_HW_CFG & CRYPTOSOC_HW_CFG_HASH_IP_INCLUDED_MASK) != 0);

        if (hash_engine_config.is_engine_available) {
            configuration = HASH_HW_CFG;
            hash_engine_config.is_md5_enabled = (configuration & HASH_HW_CFG_MD5_SUPPORTED_MASK) != 0;
            hash_engine_config.is_sha1_enabled = (configuration & HASH_HW_CFG_SHA1_SUPPORTED_MASK) != 0;
            hash_engine_config.is_sha256_enabled = (configuration & HASH_HW_CFG_SHA256_SUPPORTED_MASK) != 0;
            hash_engine_config.is_hmac_enabled = (configuration & HASH_HW_CFG_HMAC_SUPPORTED_MASK) != 0;
            hash_engine_config.is_sm3_enabled = (configuration & HASH_HW_CFG_SM3_SUPPORTED_MASK) != 0;
            hash_engine_config.is_padding_enabled = (configuration & HASH_HW_CFG_PADDING_SUPPORTED_MASK) != 0;
        }

        hash_engine_config.is_config_read = true;
    }

    return &hash_engine_config;
}

const struct kdrv_hash_engine_config_t* kdrv_hash_engine_get_config()
{
    return &hash_engine_config;
}
//-----------------------------------------------------------------------------

/* Internal functions */

#define OP_FULL_HASH 1
#define OP_FULL_HMAC 2
#define OP_PART_HASH 3

/**
 * @brief internal function for hash operation
 *
 * @param hash_fct hash function to use. See ::kdrv_hash_fct_t.
 * @param extra_in input K for OP_FULL_HMAC, or state for OP_PART_HASH, unused for OP_FULL_HASH
 * @param operation_type define type of operation to perform
 * @param data_in array of input data to process
 * @param entries length of array \p data_in
 * @param data_out output digest or state
 * @return ::KDRV_STATUS_OK when execution was successful
 */
static kdrv_status_t kdrv_hash_internal(kdrv_hash_fct_t hash_fct, block_t extra_in,
                                 uint8_t operation_type, block_t data_in[],
                                 const unsigned int entries, block_t data_out)
{
    uint32_t total_len;
    uint32_t ign_bytes;
    uint32_t extra_in_tag;
    struct hash_regs_s info;
    struct kdrv_crypto_dma_descr_s desc_in[SX_HASH_ARRAY_MAX_ENTRIES + 2];
    struct kdrv_crypto_dma_descr_s desc_out[2];
    struct kdrv_crypto_dma_descr_s *current_desc;
    uint32_t genlen, outlen;
    block_t extra = extra_in;

    const struct kdrv_hash_engine_config_t *hash_engine_config =
        kdrv_hash_engine_get_config();

    // Marked by Richard
    //CRYPTOLIB_ASSERT(entries <= SX_HASH_ARRAY_MAX_ENTRIES, "Too many entries in data array");
    //if(!(entries <= SX_HASH_ARRAY_MAX_ENTRIES))
    //CRYPTOLIB_PRINTF("Too many entries in data array");

    switch (hash_fct) {
    case e_MD5:
        if (!hash_engine_config->is_md5_enabled)
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        info.config = HASH_CONF_MODE_MD5;
        break;
    case e_SHA1:
        if (!hash_engine_config->is_sha1_enabled)
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        info.config = HASH_CONF_MODE_SHA1;
        break;
    case e_SHA256:
        if (!hash_engine_config->is_sha256_enabled)
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        info.config = HASH_CONF_MODE_SHA256;
        break;
    case e_SHA224:
    case e_SHA384:
    case e_SHA512:
    case e_SM3:
        return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
    default:
        return KDRV_STATUS_CRYPTO_INVALID_PARAM;
    }


    // Could be 4B for padding but due to both enabled stack protection and
    // -Werror, it has to be 8B minimum.
    uint8_t discarded_buf[8] = {0};
    switch (operation_type) {
    // complete hash operation: no need to load extra info, enable padding in
    // hardware, output will be digest
    case OP_FULL_HASH:
        if (!hash_engine_config->is_padding_enabled)
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        info.config |= HASH_CONF_HWPAD | HASH_CONF_FINAL;
        extra_in.len = 0;
        extra_in_tag = DMA_SG_ENGINESELECT_HASH;
        genlen = kdrv_hash_get_digest_size(hash_fct);
        break;

    // complete hmac operation: need to load extra info K, enable padding in
    // hardware, output will be digest
    case OP_FULL_HMAC: {
        if (!hash_engine_config->is_hmac_enabled ||
            (!hash_engine_config->is_padding_enabled))
            return KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR;
        uint32_t extra_ign_bytes;
        if (extra_in.len) {
            extra_ign_bytes = -extra_in.len&0x3;
        } else {
            extra_ign_bytes = 4;
            // In case of null key (which may come from a FIFO), we provide a local
            // buffer to the DMA with appropriate extra_ign_bytes
            extra = block_t_convert(discarded_buf, extra_ign_bytes);
        }
        info.config |= HASH_CONF_HMAC | HASH_CONF_HWPAD | HASH_CONF_FINAL;
        extra_in_tag = DMA_SG_TAG_DATATYPE_HASHKEY | DMA_SG_ENGINESELECT_HASH |
                       DMA_SG_TAG_ISDATA | DMA_SG_TAG_ISLAST |
                       DMA_SG_TAG_SETINVALIDBYTES(extra_ign_bytes);
        genlen = kdrv_hash_get_digest_size(hash_fct);
        break;
    }

    // partial hash operation: need to load initial state, don't enable padding in
    // hardware, output will be state
    case OP_PART_HASH:
    default:
        extra_in_tag = DMA_SG_TAG_DATATYPE_HASHINIT | DMA_SG_ENGINESELECT_HASH |
                       DMA_SG_TAG_ISDATA | DMA_SG_TAG_ISLAST;
        genlen = kdrv_hash_get_state_size(hash_fct);
    }


    // configuration (to register)
    current_desc = &desc_in[0];

    current_desc = kdrv_write_desc(current_desc, &info, sizeof(info), DMA_AXI_DESCR_REALIGN,
                       DMA_SG_ENGINESELECT_HASH | DMA_SG_TAG_ISCONFIG);

    // K or init value
    current_desc = kdrv_write_desc_always_blk(current_desc, &extra,
                       DMA_AXI_DESCR_REALIGN, extra_in_tag);

    // datas
    uint32_t i = 0;
    total_len = 0;
    for (i = 0; i < entries; i++) {
        total_len += data_in[i].len;
        current_desc = kdrv_write_desc_blk(current_desc, &data_in[i], 0,
                           DMA_SG_ENGINESELECT_HASH | DMA_SG_TAG_ISDATA 
                           | DMA_SG_TAG_DATATYPE_HASHMSG);
    }


    // The block size is always a power of 2. We use masking with
    // (block_size - 1) instead of doing the time consuming % block_size
    // operation.
    if (operation_type == OP_PART_HASH) {
        uint32_t state_size = kdrv_hash_get_state_size(hash_fct);
        uint32_t block_size = kdrv_hash_get_block_size(hash_fct);
        if ((extra.len != state_size) || (total_len & (block_size - 1)))
            return KDRV_STATUS_CRYPTO_INVALID_PARAM;
    }


    if (total_len == 0) {
        // The hardware always expects some data. In case of empty message,
        // 4 dummy bytes must be sent (data size must be aligned on 32 bits).
        // With the following statement, kdrv_crypto_set_desc_invalid_bytes will extended
        // the data length in the last decriptor and mark those additional
        // bytes as invalid.
        ign_bytes = 4;
        // Those 4 additional dummy bytes will be read by the DMA at the address
        // given in the descriptor. We want to avoid using the address given
        // by the user as it might have side effect (for example if the address
        // points to a FIFO or if the address is invalid). Therefore we
        // provide a local buffer (in an extra descriptor). This also allows
        // supporting empty data_in list (entries=0).
        current_desc = kdrv_write_desc_always(current_desc, discarded_buf, 0, 0,
                           DMA_SG_ENGINESELECT_HASH | DMA_SG_TAG_ISDATA | 
                           DMA_SG_TAG_DATATYPE_HASHMSG);
    } else if (total_len & 0x3) // not word aligned
        ign_bytes = (4 - (total_len & 0x3));
    else
        ign_bytes = 0;


    // last descriptor
    --current_desc;
    kdrv_crypto_set_last_desc(current_desc);


    kdrv_crypto_set_desc_invalid_bytes(current_desc, ign_bytes);

    // ouput digest or state
    outlen = MIN(genlen, data_out.len);
    current_desc = &desc_out[0];


    current_desc = kdrv_write_desc( // Can we speculate on outlen value ?
                       desc_out, data_out.addr, outlen, data_out.flags, 0);


    // discard unused
    kdrv_write_desc_always(current_desc, NULL, (genlen - outlen), 0, 0);

    kdrv_crypto_set_last_desc(current_desc);

    // launch cryptodma operation
    kdrv_cryptodma_run_sg(desc_in, desc_out);
    // if error occurs, hardfault should be trigger

    return KDRV_STATUS_OK;
}

/**
 * @brief internal function for SHA2 message padding
 * @param hash_fct hash function to use. See ::kdrv_hash_fct_t.
 * @param data_len length of the input message to be padded
 * @param total_len length of all data hashed including the data hashed before using update and including data_len
 * @param padding output for padding, length must be equal or bigger than padding length
 * @return padsize length of the padding to be added
 */
static uint32_t kdrv_hash_pad(kdrv_hash_fct_t hash_fct, uint32_t data_len, size_t total_len, block_t padding)
{
    uint32_t block_size = kdrv_hash_get_block_size(hash_fct);
    uint32_t length_field_size = (hash_fct >= e_SHA384) ? 16 : 8;
    uint32_t padsize;
    size_t i;


    /* As block_size is a power of 2, the modulo computation has been converted
     * into a binary and operation. This avoids a division operation which can
     * be very slow on some chips. */
    padsize = block_size - (data_len & (block_size-1));
    if (padsize < (length_field_size + 1))
        padsize += block_size;
    if (padding.len < padsize)
        return 0;
    /* first bit of padding should be 1 */
    padding.addr[0] = 0x80;
    /* followed by zeros until the length field. */
    for (i = 1; i < padsize; i++)
        padding.addr[i] = 0;
    /* write number of bits at end of padding in big endian order */
    size_t start = length_field_size;
    if (start > sizeof(total_len))
        start = sizeof(total_len);
    for (i = start; i; i--) {
        padding.addr[padsize - i - 1] = (total_len >> (i * 8 - 3)) & 0xFF;
    }
    padding.addr[padsize - 1] = (total_len & 0x1F) << 3;

    return padsize;
}

/* Public functions: properties */
uint32_t kdrv_hash_get_digest_size(kdrv_hash_fct_t hash_fct)
{
    switch (hash_fct) {
    case e_MD5:
        return MD5_DIGESTSIZE;
    case e_SHA1:
        return SHA1_DIGESTSIZE;
    case e_SHA224:
        return SHA224_DIGESTSIZE;
    case e_SHA256:
        return SHA256_DIGESTSIZE;
    case e_SHA384:
        return SHA384_DIGESTSIZE;
    case e_SHA512:
        return SHA512_DIGESTSIZE;
    case e_SM3:
        return SM3_DIGESTSIZE;
    default:
        return 0;
    }
}

uint32_t kdrv_hash_get_block_size(kdrv_hash_fct_t hash_fct)
{
    switch (hash_fct) {
    case e_MD5:
        return MD5_BLOCKSIZE;
    case e_SHA1:
        return SHA1_BLOCKSIZE;
    case e_SHA224:
        return SHA224_BLOCKSIZE;
    case e_SHA256:
        return SHA256_BLOCKSIZE;
    case e_SHA384:
        return SHA384_BLOCKSIZE;
    case e_SHA512:
        return SHA512_BLOCKSIZE;
    case e_SM3:
        return SM3_BLOCKSIZE;
    default:
        return 0;
    }
}

uint32_t kdrv_hash_get_state_size(kdrv_hash_fct_t hash_fct)
{
    switch (hash_fct) {
    case e_MD5:
        return MD5_INITSIZE;
    case e_SHA1:
        return SHA1_INITSIZE;
    case e_SHA224:
        return SHA224_INITSIZE;
    case e_SHA256:
        return SHA256_INITSIZE;
    case e_SHA384:
        return SHA384_INITSIZE;
    case e_SHA512:
        return SHA512_INITSIZE;
    case e_SM3:
        return SM3_INITSIZE;
    default:
        return 0;
    }
}

/* Public functions: process arrays of blocks */
kdrv_status_t kdrv_hash_array_blk(kdrv_hash_fct_t hash_fct, block_t data_in[],
                           const unsigned int entries, block_t data_out)
{
    return kdrv_hash_internal(hash_fct, block_t_convert(NULL, 0), OP_FULL_HASH,
                            data_in, entries, data_out);
}

uint32_t kdrv_hash_hmac_array_blk(kdrv_hash_fct_t hash_fct, block_t key,
                           block_t data_in[], const unsigned int entries,
                           block_t data_out)
{
    return kdrv_hash_internal(hash_fct, key, OP_FULL_HMAC, data_in, entries,
                            data_out);
}

/* process blocks */
uint32_t kdrv_hash_update_blk(kdrv_hash_fct_t hash_fct, block_t state, block_t data)
{
    return kdrv_hash_internal(hash_fct, state, OP_PART_HASH, &data, 1, state);
}

uint32_t kdrv_hash_finish_blk(kdrv_hash_fct_t hash_fct, block_t state, block_t data_in, 
                            block_t data_out, uint32_t total_len)
{
    uint8_t padding[MAX_BLOCKSIZE+16+1];
    block_t padding_blk = BLK_LITARRAY(padding);

    int padding_len = kdrv_hash_pad(hash_fct, data_in.len, total_len, padding_blk);
    padding_blk.len = padding_len;

    block_t input_padding[] = {data_in, padding_blk};

    return kdrv_hash_internal(hash_fct, state, OP_PART_HASH, input_padding, 2, data_out);
}

kdrv_status_t kdrv_hash_blk(kdrv_hash_fct_t hash_fct, block_t data_in, block_t data_out)
{
    return kdrv_hash_array_blk(hash_fct, &data_in, 1, data_out);
}

uint32_t kdrv_hash_hmac_blk(kdrv_hash_fct_t hash_fct, block_t key, block_t data_in, block_t data_out)
{
    return kdrv_hash_hmac_array_blk(hash_fct, key, &data_in, 1, data_out);
}

