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

/**@addtogroup  KDRV_CRYPTO
 * @{
 * @brief       Kneron CRYPTO driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_CRYPTO_H__
#define __KDRV_CRYPTO_H__
 
#define CRYPTO_MASTER_REG                   (CRYPTO_REG_BASE + 0x00000000)
#define CRYPTO_INCL_IPS_HW_CFG_REG          (CRYPTO_REG_BASE + 0x00000400)
#define CRYPTO_AES_HW_CFG_1_REG             (CRYPTO_REG_BASE + 0x00000404)
#define CRYPTO_AES_HW_CFG_2_REG             (CRYPTO_REG_BASE + 0x00000408)
#define CRYPTO_HASH_HW_CFG_REG              (CRYPTO_REG_BASE + 0x0000040C)

// Wait type
// 0: use irq hook macro bellow
// 1: use register polling
#define WAIT_CRYPTOMASTER_WITH_REGISTER_POLLING     1
#define WAIT_RNG_WITH_REGISTER_POLLING              1
#define WAIT_PK_WITH_REGISTER_POLLING               1

// hooks for functions
void wait_cryptomaster_irq(void);
#define CRYPTOMASTER_WAITIRQ_FCT()    ;   // void
#define RNG_WAITIRQ_FCT()             ;   // void
#define PK_WAITIRQ_FCT()              ;   // void
#define TRIGGER_HARDFAULT_FCT()       ;   // void

#ifndef PK_CM_ENABLED
  #define PK_CM_ENABLED 0
#endif

// max supported sizes (number of bytes)
#define DH_MAX_KEY_SIZE       (4096/8)
#define SRP_MAX_KEY_SIZE      (4096/8)
#define RSA_MAX_SIZE          (4096/8)
#define DSA_MAX_SIZE_P        (3072/8)
#define DSA_MAX_SIZE_Q        (256/8)
#define PRIME_MAX_SIZE        (RSA_MAX_SIZE/2)
#define ECC_MAX_KEY_SIZE      roundup_32(571/8 + 1)
#define DERIV_MAX_SALT_SIZE   (512)
#define DERIV_MAX_INFO_SIZE   (512)

//RNG settings
#define RNG_CLKDIV            (0)
#define RNG_OFF_TIMER_VAL     (0)
#define RNG_FIFO_WAKEUP_LVL   (8)
#define RNG_INIT_WAIT_VAL     (512)
#define RNG_NB_128BIT_BLOCKS  (4)


#define CRYPTOSOC_INCL_IPS_HW_CFG                      (*((const volatile uint32_t*) CRYPTO_INCL_IPS_HW_CFG_REG))
#define CRYPTOSOC_HW_CFG_AES_IP_INCLUDED_MASK          BIT(0)
#define CRYPTOSOC_HW_CFG_HASH_IP_INCLUDED_MASK         BIT(4) 


/**
* @brief  Array-like data abstraction handling both contiguous memory and hardware FIFO.
*
* Embedding side by side the data pointer and only the data length provides an
* abstraction which does not cover data coming from/to a hardware FIFO (data always
* read/written at the same address). The block_t structure embeds an additional
* field carrying enough information to work in both array mode (2 different words
* are stored at 2 different addresses) and FIFO mode (2 different words are loaded
* from the same address).
*
* A set of dedicated functions is provided to handle the basic operations
* (using a similar interface to memcpy/memset/memcmp).
*/
typedef struct block_s
{
   uint8_t *addr;   /**< Start address of the data (FIFO or contiguous memory) */
   uint32_t len;    /**< Length of data expressed in bytes */
   uint32_t flags;  /**< Flags equals to ::BLOCK_S_CONST_ADDR or ::BLOCK_S_INCR_ADDR */
} block_t;

/**
 * @brief Provide a default block_t initializer
 *
 * with
 * - NULL base address
 * - zero length,
 * - contiguous memory addressing with discarding flag
 *   in case of DMA transfer (see ::DMA_AXI_DESCR_DISCARD)
 */
extern const block_t null_blk;

/** @brief Align on word boundary */
#define ALIGNED __attribute__((aligned(0x4)))


/** @brief value of block_s.flags to set addressing in constant mode (pointing to a FIFO) */
#define BLOCK_S_CONST_ADDR             0x10000000
/** @brief value of block_s.flags to set addressing in increment mode (pointing to a contiguous data array) */
#define BLOCK_S_INCR_ADDR              0x00000000
/** @brief mask for block_s.flags to only get DMA-related options */
#define BLOCK_S_FLAG_MASK_DMA_PROPS    0x70000000

/**
 * @brief Convert a pair of array address and length to a block_t
 *
 * @param[in]  array  Array address. It does not support FIFO addresses.
 * @param[in]  length Length of data expressed in bytes
 * @return block_t
 */
static inline block_t block_t_convert(const volatile void *array, uint32_t length)
{
    block_t blk = {(uint8_t *)array, length, BLOCK_S_INCR_ADDR};
    return blk;
}

#define BLK_LITARRAY(literal) (block_t){(uint8_t *)(literal), sizeof(literal), BLOCK_S_INCR_ADDR}
#define CST_BLK_LITARRAY(literal) {(uint8_t *)(literal), sizeof(literal), BLOCK_S_INCR_ADDR}

/**
 * @brief Round-up integer to next 32-bit multiple
 *
 * @param[in]  value The value to round
 * @return       The rounded value
 */
#define roundup_32(value) (((value) + 3) & ~3)


/**
 * @brief Round-up the length of a block_t to 32-bit if the address is a FIFO
 *
 * @param[in]  blk    A pointer to the block_t to adapt
 * @return N/A
 */
static inline void block_t_adapt_len(block_t * blk)
{
   if (blk->flags & BLOCK_S_CONST_ADDR)
      blk->len = roundup_32(blk->len);
}

/**
* @brief Structure that represent the register map of the DMA module  for 32 bus width.
*/
typedef volatile struct {
   volatile uint32_t fetch_addr;       /**< Start address of data block */
   volatile uint32_t reserved_0x04;    /**< Reserved */
   volatile uint32_t fetch_len;        /**< Length of data block */
   volatile uint32_t fetch_tag;        /**< User tag */
   volatile uint32_t push_addr;        /**< Start address of data block */
   volatile uint32_t reserved_0x14;    /**< Reserved */
   volatile uint32_t push_len;         /**< Length of data block */
   volatile uint32_t int_en;           /**< Interrupt enable */
   volatile uint32_t int_en_set;       /**< Interrupt enable set */
   volatile uint32_t int_en_clr;       /**< Interrupt enable clear */
   volatile uint32_t int_stat_raw;     /**< Interrupt raw status */
   volatile uint32_t int_stat;         /**< Interrupt status */
   volatile uint32_t int_stat_clr;     /**< Interrupt status clear */
   volatile uint32_t config;           /**< Configuration */
   volatile uint32_t start;            /**< Start fetch & push */
   volatile uint32_t status;           /**< Status */
} kdrv_crypto_dma_regs32_t;


/**
* @brief Structure that represent a descriptor for the DMA module (in scatter-gather mode).
*/
struct kdrv_crypto_dma_descr_s {
   volatile void * addr; /**< Address of the first byte of data to be fetched/pushed */
   volatile struct kdrv_crypto_dma_descr_s * next_descr; /**< Pointer to the next valid
                                                     descriptor or indicates
                                                     that the current descriptor
                                                     is the last. */
   volatile uint32_t length_irq; /**< length and flags associated to a descriptor. */

   volatile uint32_t tag; /**< Indicates the engine to select.
                            Could be any of dma_sg_EngineSelect_e */
};

/* dma_regs.config */
#define DMA_AXI_CONFIG_FETCHER_DIRECT           0x00000000
#define DMA_AXI_CONFIG_PUSHER_DIRECT            0x00000000
#define DMA_AXI_CONFIG_FETCHER_INDIRECT         BIT(0) //fetch 0: direct mode, 1:scater-gather mode
#define DMA_AXI_CONFIG_PUSHER_INDIRECT          BIT(1) //push 0: direct mode, 1: scater-gather mode
#define DMA_AXI_CONFIG_STOP_FETCHER             BIT(2) //start fetch, 1: stop fetch
#define DMA_AXI_CONFIG_STOP_PUSHER              BIT(3) //start push, 1: start push
#define DMA_AXI_CONFIG_SOFTRESET                BIT(4) //software reset, 1:software reset

/* dma_regs.start */
#define DMA_AXI_START_FETCH                     BIT(0) // start fetch bit
#define DMA_AXI_START_PUSH                      BIT(1) // start push bit

/* dma_regs.status */
#define DMA_AXI_STATUS_MASK_FETCHER_BUSY        BIT(0) // fetcher busy bit
#define DMA_AXI_STATUS_MASK_PUSHER_BUSY         BIT(1) // pusher busy bit
#define DMA_AXI_STATUS_MASK_FIFOIN_AF           BIT(2) // input fifo almost full bit 
#define DMA_AXI_STATUS_MASK_CORE_BUSY           BIT(3) // core module busy bit
#define DMA_AXI_STATUS_MASK_FIFOIN_NOT_EMPTY    BIT(4) // input fifo not empty bit
#define DMA_AXI_STATUS_MASK_PUSHER_WAIT         BIT(5) // pusher waiting fifo bit
#define DMA_AXI_STATUS_MASK_SOFT_RESET          BIT(6) // soft reset bit
#define DMA_AXI_STATUS_MASK_FIFOOUT_NDATA       0xFFFF0000 // mask number of data in output fifo
#define DMA_AXI_STATUS_FIFOOUT_NDATA_SHIFT      16 // right shift for number of data in output fifio


/** @brief dma_sg_regs_s.Rawstatus mask for fetcher error bit */
#define DMA_AXI_RAWSTAT_MASK_FETCHER_ERROR      BIT(2) //0x00000004

/** @brief dma_sg_regs_s.Rawstatus mask for pusher error bit */
#define DMA_AXI_RAWSTAT_MASK_PUSHER_ERROR       BIT(5) //0x00000020


/* dma_regs.int */
#define DMA_AXI_INTEN_FETCHER_ENDOFBLOCK_EN  BIT(0) //triggered at the end of each block(scatter-gather only)
#define DMA_AXI_INTEN_FETCHER_STOPPED_EN     BIT(1) //triggered when reaching a block with stop=1(or end of direct transfer)
#define DMA_AXI_INTEN_FETCHER_ERROR_EN       BIT(2) //triggered when an error response
#define DMA_AXI_INTEN_PUSHER_ENDBLOCK_EN     BIT(3) //triggered at the end each block(scatter-gather only)
#define DMA_AXI_INTEN_PUSHER_STOPPED_EN      BIT(4) //triggered when reaching a block with stop=1(or end of direct transfre 
#define DMA_AXI_INTEN_PUSHER_ERROR_EN        BIT(5) // error response 
#define DMA_AXI_INTEN_ALL_EN                 0X0000003F // all enable



/*  descr_t.length_irq */
#define DMA_AXI_DESCR_MASK_LENGTH       0x0FFFFFFF // mask for data length

/* @brief Indicates to the DMA that addressing in constant mode (pointing to a FIFO) */
#define DMA_AXI_DESCR_CONST_ADDR       BIT(28) //0x10000000
/** @brief Indicates to the DMA to realign data on 32 bits words */
#define DMA_AXI_DESCR_REALIGN          BIT(29) //0x20000000
/** @brief Indicates to the DMA to discard fetched data */
#define DMA_AXI_DESCR_DISCARD          BIT(30) //0x40000000
#define DMA_AXI_DESCR_INT_ENABLE       BIT(31) //0x80000000
#define DMA_AXI_DESCR_INT_DISABLE      0x00000000

/** @brief Indicates to the DMA to not fetch another descriptor */
#define DMA_AXI_DESCR_NEXT_STOP        ((struct kdrv_crypto_dma_descr_s*)0x00000001)



/**
* @brief Select which core the DMA will use. To set in descriptor ::kdrv_crypto_dma_descr_s.tag.
*/
enum dma_sg_EngineSelect_e
{
   DMA_SG_ENGINESELECT_BYPASS   = 0x00,         /**< Enum 0x00, direct bypass from input to output */
   DMA_SG_ENGINESELECT_AES      = 0x01,         /**< Enum 0x01, data flow through AES */
   //DMA_SG_ENGINESELECT_RES1   = 0x02,         /**< Enum 0x02, reserved */
   DMA_SG_ENGINESELECT_HASH     = 0x03,         /**< Enum 0x03, data flow through Hash */
   //DMA_SG_ENGINESELECT_RES2   = 0x04,         /**< Enum 0x04, reserved */
   DMA_SG_ENGINESELECT_MASK     = 0xF           /**< Enum 0xF, Mask on the engine */
};

/** @brief value for to direct data to parameters */
#define DMA_SG_TAG_ISCONFIG 0x00000010
/** @brief value for to direct data to processing */
#define DMA_SG_TAG_ISDATA 0x00000000
/** @brief value for specifying data as last */
#define DMA_SG_TAG_ISLAST  0x00000020

/** @brief macro to set the offset in the configuration */
#define DMA_SG_TAG_SETCFGOFFSET(a) ((((a)&0xFF)<<8))

/** @brief value for specifying data type to message */
#define DMA_SG_TAG_DATATYPE_HASHMSG     0x00000000
/** @brief value for specifying data type to initialization state */
#define DMA_SG_TAG_DATATYPE_HASHINIT    0x00000040
/** @brief value for specifying data type to HMAC key */
#define DMA_SG_TAG_DATATYPE_HASHKEY     0x00000080
/** @brief value for specifying data type payload (will be encrypted/decrypted and authenticated) */
#define DMA_SG_TAG_DATATYPE_AESPAYLOAD    0x00000000
/** @brief value for specifying data type header (will only be authenticated, not encrypted/decrypted) */
#define DMA_SG_TAG_DATATYPE_AESHEADER     0x00000040

/** @brief value for specifying data type payload (will be encrypted/decrypted and authenticated) for SM4 */
#define DMA_SG_TAG_DATATYPE_SM4_PAYLOAD    0x00000000
/** @brief value for    specifying data type header (will only be authenticated, not encrypted/decrypted) for SM4 */
#define DMA_SG_TAG_DATATYPE_SM4_HEADER     0x00000040

#define DMA_SG_TAG_PADDING_MASK  0x1F
#define DMA_SG_TAG_PADDING_OFFSET   8
 /** @brief macro to set the amount of invalid bytes */
#define DMA_SG_TAG_SETINVALIDBYTES(a) ((((a) & DMA_SG_TAG_PADDING_MASK) \
         << DMA_SG_TAG_PADDING_OFFSET))

/**
 * @brief Configure fetch and push operations in scatter-gather mode on internal DMA
 *
 * @param[in] first_fetch_descriptor physical address of the first fetcher descriptor to be configured, see @ref kdrv_crypto_dma_descr_s
 * @param[in] first_push_descriptor physical address of the first pusher descriptor to be configured, see @ref kdrv_crypto_dma_descr_s
 *
 * @return N/A
 */
void kdrv_cryptodma_config_sg(struct kdrv_crypto_dma_descr_s * first_fetch_descriptor, 
                      struct kdrv_crypto_dma_descr_s * first_push_descriptor);

/**
 * @brief Configure fetch and push operations in direct mode on internal DMA
 *
 * @param[in] src block_t to the source data to transfer
 * @param[in] dest block_t to the destination location
 * @param[in] length the length in bytes to transfer (from src to dest)
 *
 * @return N/A
 */
void kdrv_cryptodma_config_direct(block_t dest, block_t src, uint32_t length);

/**
 * @brief Check cryptodma status
 *
 * @param[in] N/A
 * @note Trigger a hardfault if any error occured
 * @return N/A
 */
void kdrv_cryptodma_check_status(void) ;

/** @brief Reset the internal DMA
 *
 * @param[in] N/A
 * @return N/A
 */
void kdrv_cryptodma_reset(void);

/** @brief Start internal DMA transfer
 *
 * @param[in] N/A
 * @return N/A
 */
void kdrv_cryptodma_start(void);

/** @brief Wait until internal DMA is done
 *
 * @param[in] N/A
 * @return N/A
 */
void kdrv_cryptodma_wait(void);

/**
 * @brief Check cryptodma error flag
 *
 * @param[in] N/A
 * @return KDRV_STATUS_CRYPTO_DMA_ERR if fifo's are not empty, KDRV_STATUS_OK otherwise
 */
uint32_t kdrv_cryptodma_check_bus_error(void);

/**
 * @brief Issues an internal DMA transfer command in indirect mode
 *
 * @details     It configures the internal DMA to issue a data transfer in indirect mode.\n
 *                  After that, it waits for the completion (interrupt or polling) and in case\n
 *                  of errors on the bus will trigger an hard fault.
 *
 * @param[in] first_fetch_descriptor    list of descriptors to fetch from
 * @param[in] first_push_descriptor     list of descriptors to push to
 * @return      N/A
 */
void kdrv_cryptodma_run_sg(struct kdrv_crypto_dma_descr_s * first_fetch_descriptor, struct kdrv_crypto_dma_descr_s * first_push_descriptor);


/**
 * @brief Map software descriptors and buffers to the hardware
 *
 * \warning \c kdrv_unmap_descriptors should be called to uninitialize after transfer.
 *
 * @param[in] first_fetch_descriptor        DMA input descriptors list
 * @param[in] first_push_descriptor         DMA output descriptors list
 * @param[in] mapped_in                         Pointer to the address of the mapped input descriptors list
 * @param[in] mapped_out                    Pointer to the address of the mapped output descriptors list
 * @return      N/A
 */
void kdrv_map_descriptors(struct kdrv_crypto_dma_descr_s *first_fetch_descriptor,
   struct kdrv_crypto_dma_descr_s *first_push_descriptor,
   struct kdrv_crypto_dma_descr_s **mapped_in,
   struct kdrv_crypto_dma_descr_s **mapped_out);

/** @brief Unmap descriptors and buffers to the hardware
*
 * @param[in] out_descs     Output DMA descriptors list
 * @return           N/A
 */
void kdrv_unmap_descriptors(struct kdrv_crypto_dma_descr_s *out_descs);

/**
 * @brief Write a descriptor and return the next updated address
 *
 * @details     Fill the descriptor (even in case of null length) with the address to\n
 *                  fetch/push data, the amount of bytes to fetch/push, the additional flags\n
 *                  required by the DMA (like fetch in a fifo mode or discard data) and the tag\n
 *                  which selects the crypto engine and extra flags for this specific crypto engine.
 *
 * @param[in] descr  pointer to a descriptor to fill with others parameters
 * @param[in] addr   the address where data will be fetched/pushed
 * @param[in] length amount of bytes to fetch/push
 * @param[in] flags  the extra flags describing if data are coming read like\n
 *               from a fifo (::DMA_AXI_DESCR_CONST_ADDR), if it is needed\n
 *               to realign on the width of the DMA bus\n
 *               (::DMA_AXI_DESCR_REALIGN) or if data can be discarded\n
 *               (::DMA_AXI_DESCR_DISCARD)
 * @param[in] tag    contains the engine from/to fetch/push data\n
 *               (see ::dma_sg_EngineSelect_e), indicates\n
 *               if descriptor contains data or configuration is the last\n
 *               and specific additional information per crypto-engine.
 *
 * @return the address of the next descriptor available (it supposes a large\n
 *         enough array of descriptors is passed as first parameter).
 */
struct kdrv_crypto_dma_descr_s* kdrv_write_desc_always(
      struct kdrv_crypto_dma_descr_s *descr,
      volatile void *addr,
      const uint32_t length,
      const uint32_t flags,
      const uint32_t tag);

 /**
 * @brief Write a descriptor and returns the next updated address only if required
 *
 * @details     Works like ::kdrv_write_desc_always but start first by checking if there is any\n
 *                  data to fetch/push (length > 0). If not, does not fill the descriptor
 *
 * @param[in]  descr  pointer to a descriptor to fill with others parameters, see @ref kdrv_crypto_dma_descr_s
 * @param[in]  addr   the address where data will be fetched/pushed
 * @param[in]  length amount of bytes to fetch/push
 * @param[in]  flags  the extra flags like realign/const address, discard
 * @param[in]  tag    contains crypto engines, is last or not, config or data, ...
 *
 * @return the address of the next descriptor available (it supposes a large \n
 *         enough array of descriptors is passed as first parameter). \n
 *          It could be the same as ::descr
 */
static inline struct kdrv_crypto_dma_descr_s* kdrv_write_desc(
      struct kdrv_crypto_dma_descr_s *descr, volatile void *addr,
      const uint32_t length, const uint32_t flags, const uint32_t tag)
{
   if (length)
      return kdrv_write_desc_always(descr, addr, length, flags, tag);
   return descr;
}


/**
 * @brief Write a descriptor and returns the next updated address
 *
 * @details Works like ::kdrv_write_desc_always but dedicated for block_t
 *
 * @param[in]  descr  pointer to a descriptor to fill with others parameters, see @ref kdrv_crypto_dma_descr_s
 * @param[in]  blk    a ::block_t containing an address, a length and flags, see @ref block_t
 * @param[in]  flags  the extra flags like realign or discard
 * @param[in]  tag    contains crypto engines, is last or not, config or data, ...
 *
 * @return      the address of the next descriptor available
 */
static inline struct kdrv_crypto_dma_descr_s *kdrv_write_desc_always_blk(
      struct kdrv_crypto_dma_descr_s *descr, const block_t *blk,
      const uint32_t flags, const uint32_t tag)
{
   return kdrv_write_desc_always(
         descr, blk->addr, blk->len, blk->flags | flags, tag);
}

/**
 * @brief Write a descriptor and returns the next updated address
 *
 * @details Works like ::kdrv_write_desc but dedicated for block_t
 *
 * @param[in] descr  pointer to a descriptor to fill with others parameters, see @ref kdrv_crypto_dma_descr_s
 * @param[in] blk    a ::block_t containing an address, a length and flags, see @ref block_t
 * @param[in] flags  the extra flags like realign or discard
 * @param[in] tag    contains crypto engines, is last or not, config or data, ...
 *
 * @return      the address of the next descriptor available (could be the same is ::descr)
 */
static inline struct kdrv_crypto_dma_descr_s *kdrv_write_desc_blk(
      struct kdrv_crypto_dma_descr_s *descr, const block_t *blk, 
      const uint32_t flags, const uint32_t tag)
{
   return kdrv_write_desc(descr, blk->addr, blk->len, blk->flags | flags, tag);
}

/**
 * @brief Mark input descriptor as needing to be realigned by the DMA
 *
 * @param[in] d     address of descriptor, see @ref kdrv_crypto_dma_descr_s
 * @return      N/A
 */
void kdrv_crypto_realign_desc(struct kdrv_crypto_dma_descr_s * d);

/**
 * @brief Mark input descriptor as last of a list of descriptors
 *
 * @param[in]  d        address of last descriptor, see @ref kdrv_crypto_dma_descr_s
 * @return      N/A
 */
void kdrv_crypto_set_last_desc(struct kdrv_crypto_dma_descr_s * d);

/**
 * @brief Update the last data descriptor with the extra invalid bytes.
 *
 * @details     For the currently supported engines in software (AES, ChaCha, SHA(1-2-3)),\n
 *                  the corresponding HW field holds invalid bytes, meaning padding after\n
 *                  the last data (see the CryptoMaster Datasheet, Table 13).\n\n
 *                  This function ensures that the invalid bytes are set on a descriptor \n
 *                  already marked as ::DMA_SG_TAG_ISLAST, ::DMA_SG_TAG_ISDATA and also\n
 *                  verifies that the selected engine is currently supported before updating the descriptor.
 *
 * @param[in]  d             the descriptor to update, see @ref kdrv_crypto_dma_descr_s
 * @param[in]  n_bytes  the extra padding to append ( < 32, 5 bits in HW).
 * @return      N/A
 */
void kdrv_crypto_set_desc_invalid_bytes(struct kdrv_crypto_dma_descr_s *d, const uint32_t n_bytes);


#endif /* __KDRV_CRYPTO_H__ */
/** @}*/

