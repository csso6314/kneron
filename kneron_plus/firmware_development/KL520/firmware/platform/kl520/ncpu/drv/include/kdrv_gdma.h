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

/**@addtogroup  KDRV_GDMA  KDRV_GDMA
 * @{
 * @brief       Kneron generic DMA driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef _KDRV_GDMA_H_
#define _KDRV_GDMA_H_

#include "cmsis_os2.h"
#include "kdrv_status.h"

/** @brief GDMA handle type which represents for a DMA channel and related DMA operations */
typedef int32_t kdrv_gdma_handle_t;

/** @brief Enumeration of GDMA transfer size: 8/16/32 bits, this is about byte-alignment */
typedef enum
{
    GDMA_TXFER_WIDTH_8_BITS = 0x0,  /**< GDMA transfer size: 8 bits */
    GDMA_TXFER_WIDTH_16_BITS,       /**< GDMA transfer size: 16 bits */
    GDMA_TXFER_WIDTH_32_BITS,       /**< GDMA transfer size: 32 bits, default value */
} gdma_transfer_width_t;

/** @brief Enumeration of GDMA transfer burst : 1/4/8/16/32/64/128/256, this is about performance */
typedef enum
{
    GDMA_BURST_SIZE_1 = 0x0,        /**< GDMA transfer burst size: 1 */
    GDMA_BURST_SIZE_4,              /**< GDMA transfer burst size: 4 */
    GDMA_BURST_SIZE_8,              /**< GDMA transfer burst size: 8 */
    GDMA_BURST_SIZE_16,             /**< GDMA transfer burst size: 16, default value */
    GDMA_BURST_SIZE_32,             /**< GDMA transfer burst size: 32 */
    GDMA_BURST_SIZE_64,             /**< GDMA transfer burst size: 64 */
    GDMA_BURST_SIZE_128,            /**< GDMA transfer burst size: 128 */
    GDMA_BURST_SIZE_256,            /**< GDMA transfer burst size: 256 */
} gdma_burst_size_t;

/** @brief Enumeration of DMA address control, auto-increasing/descreading or fixed */
typedef enum
{
    GDMA_INCREMENT_ADDRESS = 0x0,   /**< DMA address control, auto-increasing, default value */
    GDMA_DECREMENT_ADDRESS,         /**< DMA address control, auto-descreading */
    GDMA_FIXED_ADDRESS,             /**< DMA address control, fixed */
} gdma_address_control_t;

/** @brief Enumeration of DMA working mode, can be normal or hardware handshake mode */
typedef enum
{
    GDMA_NORMAL_MODE = 0x0,     /**< DMA working mode, normal mode , default value*/
    GDMA_HW_HANDSHAKE_MODE,     /**< DMA working mode, hardware handshake mode */
} gdma_work_mode_t;

/** @brief Structure of GDMA advanced settings for a specified DMA handle (channel) */
typedef struct
{
    gdma_transfer_width_t dst_width;        /**< see @ref gdma_transfer_width_t */
    gdma_transfer_width_t src_width;        /**< see @ref gdma_transfer_width_t */
    gdma_burst_size_t burst_size;           /**< see @ref gdma_burst_size_t */
    gdma_address_control_t dst_addr_ctrl;   /**< see @ref gdma_address_control_t */
    gdma_address_control_t src_addr_ctrl;   /**< see @ref gdma_address_control_t */
    gdma_work_mode_t dma_mode;              /**< see @ref gdma_work_mode_t */
    uint32_t dma_dst_req;                   /**< for HW handshake mode, refer to kneron_mozart.h XXX_DMA_REQ */
    uint32_t dma_src_req;                   /**< for HW handshake mode, refer to kneron_mozart.h XXX_DMA_REQ */
} gdma_setting_t;

/** @brief GDMA user callback function with transfer status notification. Note that this is callback form ISR context. */
typedef void (*gdma_xfer_callback_t)(kdrv_status_t status, void *arg);

/**
 * @brief           GDMA driver initialization
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_gdma_initialize(void);

/**
 * @brief           GDMA driver uninitialization
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_gdma_uninitialize(void);

/**
 * @brief           Acquire a GDMA handle
 *
 * @param[out]      handle                a handle of a DMA channel, see @ref kdrv_gdma_handle_t
 * @return          kdrv_status_t         see @ref kdrv_status_t
 *
 *                  Example:\n
 *                  kdrv_gdma_handle_t dma_handle;
 *                  kdrv_status_t sts = kdrv_gdma_acquire_handle(&dma_handle);
 *                  if(sts == KDRV_STATUS_OK) printf("Succeeds to get valid dma_handle");
 */
kdrv_status_t kdrv_gdma_acquire_handle(kdrv_gdma_handle_t *handle);

/**
 * @brief           Configure the DMA working behavior on specified DMA handle with specified dma settings
 *
 * @param[in]       handle                  a handle of a DMA channel, see @ref kdrv_gdma_handle_t
 * @param[in]       dma_setting             pointer of dma_setting, see @ref gdma_setting_t
 * @return          @ref kdrv_status_t
 *
 * @note            Before call this API, you should get a valid dma_handle via @ref kdrv_gdma_acquire_handle() firstly.
 */
kdrv_status_t kdrv_gdma_configure_setting(kdrv_gdma_handle_t handle, gdma_setting_t *dma_setting);

/**
 * @brief           Release the DMA handle
 *
 * @param[in]       handle              a handle of a DMA channel, see @ref kdrv_gdma_handle_t
 * @return          @ref kdrv_status_t
 */
kdrv_status_t kdrv_gdma_release_handle(kdrv_gdma_handle_t handle);

/**
 * @brief          Start DMA transfer with specified DMA handle running in asynchronous (non-blocking) mode
 *
 * @param[in]      handle                a handle of a DMA channel, see @ref kdrv_gdma_handle_t
 * @param[in]      dst_addr              destination address
 * @param[in]      src_addr              source address
 * @param[in]      num_bytes             number of bytes to be transfered
 * @param[in]      xfer_isr_cb           user callback function, see @ref gdma_xfer_callback_t
 * @param[in]      usr_arg               user's argument
 * @return         @ref kdrv_status_t
 */
kdrv_status_t kdrv_gdma_transfer_async(kdrv_gdma_handle_t handle, uint32_t dst_addr, uint32_t src_addr,
                                       uint32_t num_bytes, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg);

/**
 * @brief          Start DMA transfer with specified DMA handle running in synchronous (blocking) mode
 *
 * @param[in]      handle                a handle of a DMA channel, see @ref kdrv_gdma_handle_t
 * @param[in]      dst_addr              destination address
 * @param[in]      src_addr              source address
 * @param[in]      num_bytes             number of bytes to be transfered
 * @return         @ref kdrv_status_t
 */
kdrv_status_t kdrv_gdma_transfer(kdrv_gdma_handle_t handle, uint32_t dst_addr,
                                 uint32_t src_addr, uint32_t num_bytes);

/**
 * @brief          Start DMA transfer with automatic DMA handle running in asynchronous (non-blocking) mode
 *
 * @param[in]      dst_addr              destination address
 * @param[in]      src_addr              source address
 * @param[in]      num_bytes             number of bytes to be transfered
 * @param[in]      xfer_isr_cb           user callback function, see @ref gdma_xfer_callback_t
 * @param[in]      usr_arg               user's own argument
 * @return         @ref kdrv_status_t
 */
kdrv_status_t kdrv_gdma_memcpy_async(uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes,
                                     gdma_xfer_callback_t xfer_isr_cb, void *usr_arg);

/**
 * @brief          Start DMA transfer with automatic DMA handle running in synchronous (blocking) mode
 *
 * @param[in]      dst_addr              destination address
 * @param[in]      src_addr              source address
 * @param[in]      num_bytes             number of bytes to be transfered
 * @return         @ref kdrv_status_t
 */
kdrv_status_t kdrv_gdma_memcpy(uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes);

#endif // _KDRV_GDMA_H_
