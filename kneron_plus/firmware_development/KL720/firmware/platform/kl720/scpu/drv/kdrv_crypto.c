/**
 * Kneron Peripheral API - CRYPTO
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "base.h"
#include "kdrv_io.h"
#include "kdrv_crypto.h"
#include "kdrv_status.h"
#include "regbase.h"


const block_t null_blk = {
    .addr = NULL,
    .len = 0,
    .flags = BLOCK_S_INCR_ADDR | DMA_AXI_DESCR_DISCARD // FIXME we are mixing BLOCK_S_ with DMA_AXI_
};

static kdrv_crypto_dma_regs32_t *regs = (kdrv_crypto_dma_regs32_t *) CRYPTO_MASTER_REG;

void kdrv_cryptodma_config_sg(struct kdrv_crypto_dma_descr_s * first_fetch_descriptor,
                        struct kdrv_crypto_dma_descr_s * first_push_descriptor)
{
    regs->fetch_addr = (uint32_t) first_fetch_descriptor;
    regs->push_addr = (uint32_t) first_push_descriptor;
    regs->config = DMA_AXI_CONFIG_FETCHER_INDIRECT|DMA_AXI_CONFIG_PUSHER_INDIRECT;
}

void kdrv_cryptodma_config_direct(block_t dest, block_t src, uint32_t length)
{
    uint32_t transfer_len = dest.len<length ? dest.len : length;
    // if destination is a FIFO, size needs to be a multiple of 32-bits.
    if (dest.flags & BLOCK_S_CONST_ADDR)
        transfer_len = roundup_32(transfer_len);

    transfer_len &= DMA_AXI_DESCR_MASK_LENGTH;
    regs->fetch_addr = (uint32_t) src.addr;
    regs->fetch_len = transfer_len | (src.flags & BLOCK_S_FLAG_MASK_DMA_PROPS) | DMA_AXI_DESCR_REALIGN;
    regs->fetch_tag = DMA_SG_ENGINESELECT_BYPASS;
    regs->push_addr = (uint32_t) dest.addr;
    regs->push_len = transfer_len | (dest.flags & BLOCK_S_FLAG_MASK_DMA_PROPS) | DMA_AXI_DESCR_REALIGN;
    regs->config = DMA_AXI_CONFIG_PUSHER_DIRECT | DMA_AXI_CONFIG_FETCHER_DIRECT;
}

void kdrv_cryptodma_start(void)
{
    regs->int_stat_clr = DMA_AXI_INTEN_ALL_EN;
    regs->int_en = DMA_AXI_INTEN_PUSHER_STOPPED_EN;
    regs->start = DMA_AXI_START_FETCH | DMA_AXI_START_PUSH;
}

void kdrv_cryptodma_wait(void)
{
    // Wait until DMA is done
#if WAIT_CRYPTOMASTER_WITH_REGISTER_POLLING // polling
    while (regs->status & DMA_AXI_STATUS_MASK_PUSHER_BUSY);
#else  // wait interrupt
    CRYPTOMASTER_WAITIRQ_FCT();
#endif
}

uint32_t kdrv_cryptodma_check_bus_error(void)
{
    if (regs->int_stat_raw & (DMA_AXI_RAWSTAT_MASK_FETCHER_ERROR | DMA_AXI_RAWSTAT_MASK_PUSHER_ERROR)) {
        // printf("CRYPTODMA bus error\n");
        return KDRV_STATUS_CRYPTO_DMA_ERR;
    } else {
        return KDRV_STATUS_OK;
    }
}

/**
 * @brief Check cryptodma fifo status
 * @return KDRV_STATUS_CRYPTO_DMA_ERR if bus error occured, KDRV_STATUS_OK otherwise
 */
static uint32_t kdrv_cryptodma_check_fifo_empty(void)
{
    uint32_t dma_status = regs->status;
    if (dma_status & (DMA_AXI_STATUS_MASK_FIFOIN_NOT_EMPTY|DMA_AXI_STATUS_MASK_FIFOOUT_NDATA)) {
        //printf("CRYPTODMA fifo error %08x\n", dma_status);
        return KDRV_STATUS_CRYPTO_DMA_ERR;
    } else {
        return KDRV_STATUS_OK;
    }
}

void kdrv_cryptodma_check_status(void)
{
    if (kdrv_cryptodma_check_bus_error() | kdrv_cryptodma_check_fifo_empty()) {
        TRIGGER_HARDFAULT_FCT();
    }
}

void kdrv_cryptodma_reset(void)
{
    regs->config = DMA_AXI_CONFIG_SOFTRESET;
    regs->config = 0;                                          // Clear soft-reset
    while (regs->status & DMA_AXI_STATUS_MASK_SOFT_RESET); // Wait for soft-reset deassertion
}

//#define DMA_SG_DEBUG 1

#if DMA_SG_DEBUG
void kdrv_crypto_debug_print_sg(struct kdrv_crypto_dma_descr_s * desc_in)
{
    printf("\n-------------------------------------------------------\n");
    printf("descriptor        : dataptr  nextptr  length   tag\n");
    volatile struct kdrv_crypto_dma_descr_s * desc_ptr = desc_in;
    uint32_t i = 0;
    while(desc_ptr != DMA_AXI_DESCR_NEXT_STOP) {
        if(i>50)
            TRIGGER_HARDFAULT_FCT();

        printf("desc #%02d @%08X: %08X %08X %08X %08X\n", i, desc_ptr, desc_ptr->addr, desc_ptr->next_descr, desc_ptr->length_irq, desc_ptr->tag);
        desc_ptr = desc_ptr->next_descr;
        i++;
    }
    printf("-------------------------------------------------------\n");
}
#endif

#ifndef MAP_DESCRIPTOR_HOOKS

void kdrv_map_descriptors(struct kdrv_crypto_dma_descr_s *first_fetch_descriptor,
                     struct kdrv_crypto_dma_descr_s *first_push_descriptor,
                     struct kdrv_crypto_dma_descr_s **mapped_in,
                     struct kdrv_crypto_dma_descr_s **mapped_out)
{
    *mapped_in = first_fetch_descriptor;
    *mapped_out = first_push_descriptor;
}

void kdrv_unmap_descriptors(struct kdrv_crypto_dma_descr_s *out_descs)
{
}
#endif

void kdrv_cryptodma_run_sg(struct kdrv_crypto_dma_descr_s * first_fetch_descriptor, struct kdrv_crypto_dma_descr_s * first_push_descriptor)
{
    struct kdrv_crypto_dma_descr_s *mapped_in, *mapped_out;
#if DMA_SG_DEBUG
    kdrv_crypto_debug_print_sg(first_fetch_descriptor);
    kdrv_crypto_debug_print_sg(first_push_descriptor);
#endif

    kdrv_map_descriptors(first_fetch_descriptor, first_push_descriptor, &mapped_in, &mapped_out);
    kdrv_cryptodma_config_sg(mapped_in, mapped_out);
    kdrv_cryptodma_start();
    kdrv_cryptodma_wait();
    kdrv_cryptodma_check_status();
    kdrv_unmap_descriptors(first_push_descriptor);
}

struct kdrv_crypto_dma_descr_s* kdrv_write_desc_always(struct kdrv_crypto_dma_descr_s *descr,
                            volatile void *addr, const uint32_t length, 
                            const uint32_t flags, const uint32_t tag)
{
    struct kdrv_crypto_dma_descr_s *nextdescr = descr + 1;
    descr->next_descr = nextdescr;
    descr->addr = addr;

    // Build flags[31:28] and length[27:0]
    descr->length_irq = length & DMA_AXI_DESCR_MASK_LENGTH;
    if(addr)
        descr->length_irq |= (flags & ~DMA_AXI_DESCR_MASK_LENGTH);
    else
        descr->length_irq |= DMA_AXI_DESCR_DISCARD;

    //printf("descr->length_irq = 0x%x", descr->length_irq);

    descr->tag = tag;
    return nextdescr;
}

void kdrv_crypto_realign_desc(struct kdrv_crypto_dma_descr_s * d)
{
    d->length_irq |= DMA_AXI_DESCR_REALIGN;
}

void kdrv_crypto_set_last_desc(struct kdrv_crypto_dma_descr_s * d)
{
    d->next_descr = DMA_AXI_DESCR_NEXT_STOP;
    d->tag |= DMA_SG_TAG_ISLAST;
    d->length_irq |= DMA_AXI_DESCR_REALIGN;
}

void kdrv_crypto_set_desc_invalid_bytes(struct kdrv_crypto_dma_descr_s *d, const uint32_t n_bytes)
{
    // Check it is the last data descriptor

    //CRYPTOLIB_ASSERT(!(d->tag & DMA_SG_TAG_ISCONFIG) &&
    //      (d->tag & DMA_SG_TAG_ISLAST), "Descriptor is not tagged as last data");
    //if((d->tag & DMA_SG_TAG_ISCONFIG) || !(d->tag & DMA_SG_TAG_ISLAST))
        //kdrv_printf("Descriptor is not tagged as last data");

    // Check the engine supports padding
    switch(d->tag & DMA_SG_ENGINESELECT_MASK) {
    case DMA_SG_ENGINESELECT_AES:
    case DMA_SG_ENGINESELECT_HASH: {
        uint32_t pad = (n_bytes & DMA_SG_TAG_PADDING_MASK);
        d->tag |= pad << DMA_SG_TAG_PADDING_OFFSET;
        d->length_irq += pad;
        break;
    }
    default:
        //CRYPTOLIB_ASSERT(0, "Crypto engine does not support invalid bytes/ is not implemented");
        //kdrv_printf("Crypto engine does not support invalid bytes/ is not implemented");
        break;
    }
}

