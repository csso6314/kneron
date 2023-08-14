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

/**@addtogroup  dma_drv
 * @{
 * @brief       DMA driver for DMA030
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef _KDRV_GDMA_H_
#define _KDRV_GDMA_H_

#include "cmsis_os2.h"
#include "kdrv_status.h"

typedef int32_t kdrv_gdma_handle_t;

/** @brief Enumeration of GDMA transfer size: 8/16/32/64/128 bits, this is about byte-alignment */
typedef enum {
    GDMA_TXFER_WIDTH_8_BITS = 0x0, /**< GDMA transfer size: 8 bits */
    GDMA_TXFER_WIDTH_16_BITS,      /**< GDMA transfer size: 16 bits */
    GDMA_TXFER_WIDTH_32_BITS,      /**< GDMA transfer size: 32 bits, default value */
    GDMA_TXFER_WIDTH_64_BITS,	   /**< GDMA transfer size: 64 bits, default value */
    GDMA_TXFER_WIDTH_128_BITS,	   /**< GDMA transfer size: 128 bits, default value */
} gdma_transfer_width_t;

/** @brief Enumeration of GDMA transfer beats count of one DMA handshake operation */
typedef enum {
    GDMA_BURST_SIZE_1 = 0x0,
    GDMA_BURST_SIZE_2,
    GDMA_BURST_SIZE_4,
    GDMA_BURST_SIZE_8,
    GDMA_BURST_SIZE_16,
    GDMA_BURST_SIZE_32,
    GDMA_BURST_SIZE_64,
    GDMA_BURST_SIZE_128
} gdma_burst_size_t;

/** @brief Enumeration of DMA address control, auto-increasing or fixed */
typedef enum {
    GDMA_INCREMENT_ADDRESS = 0x0, /**< DMA address control, auto-increasing, default value */
    GDMA_FIXED_ADDRESS= 0x2,           /**< DMA address control, fixed */
} gdma_address_control_t;

/** @brief Enumeration of DMA working mode, can be normal or hardware handshake mode */
typedef enum
{
    GDMA_NORMAL_MODE = 0x0, /**< Enum 0, DMA working mode, normal mode , default value*/
    GDMA_HW_HANDSHAKE_MODE, /**< Enum 1, DMA working mode, hardware handshake mode */
} gdma_work_mode_t;

/** @brief DMA hardware handshake mode peripherral request definitions */
typedef enum
{
    GDMA_HW_REQ_NONE = -1,       /**< Enum -1, No hardware handshake mode with any peripheral controllers  */
    GDMA_HW_REQ_UART0_TX = 0,    /**< Enum 0, UART0 TX (kdrv_uart) */
    GDMA_HW_REQ_UART0_RX = 1,    /**< Enum 1, UART0 RX (kdrv_uart) */
    GDMA_HW_REQ_UART1_TX = 2,    /**< Enum 2, UART1 TX (kdrv_uart) */
    GDMA_HW_REQ_UART1_RX = 3,    /**< Enum 3, UART1 RX (kdrv_uart) */
    GDMA_HW_REQ_SPIF = 4,        /**< Enum 4, SPI flash (kdrv_spif) */
    GDMA_HW_REQ_SSP0_TX = 5,     /**< Enum 5, SSP0 TX (kdrv_spi) */
    GDMA_HW_REQ_SSP0_RX = 6,     /**< Enum 6, SSP0 RX (kdrv_spi) */
    GDMA_HW_REQ_SSP1_TX = 7,     /**< Enum 7, SSP1 TX (kdrv_spi) */
    GDMA_HW_REQ_SSP1_RX = 8,     /**< Enum 8, SSP1 RX (kdrv_spi) */
    GDMA_HW_REQ_PWM_TIMER1 = 9,  /**< Enum 9, PWM TIMER 1 (kdrv_pwm) */
    GDMA_HW_REQ_PWM_TIMER2 = 10, /**< Enum 10, PWM TIMER 2 (kdrv_pwm) */
    GDMA_HW_REQ_SDC0 = 11,       /**< Enum 11, SDC0 (kdrv_sdc) */
    GDMA_HW_REQ_SDC1 = 12,       /**< Enum 12, SDC0 (kdrv_sdc) */
} gdma_hw_request_t;

/**
 * @details
 * GDMA settings data structure.
 *
 */
typedef struct {
    gdma_transfer_width_t dst_width;      /**< see @ref gdma_transfer_width_t */
    gdma_transfer_width_t src_width;      /**< see @ref gdma_transfer_width_t */
    gdma_burst_size_t burst_size;         /**< see @ref gdma_burst_size_t */
    gdma_address_control_t dst_addr_ctrl; /**< see @ref gdma_address_control_t */
    gdma_address_control_t src_addr_ctrl; /**< see @ref gdma_address_control_t */
    gdma_work_mode_t dma_mode;            /**< see @ref gdma_work_mode_t */
    int32_t dma_dst_req;                      /**< see @ref gdma_hw_request_t */
    int32_t dma_src_req;                      /**< see @ref gdma_hw_request_t */
} gdma_setting_t;

/**
 * @details
 * Callback function for notifying users of DMA transfer completeion. \n
 * Note that the context of this callback function is from ISR.
 *
 * @param status Transfer status, it can be one of followings: \n
 *          KDRV_STATUS_OK : Transfer is successuflly completed. \n
 *          KDRV_STATUS_ERROR : Transfer is failed due to an errror or abortion.
 *
 * @param arg User's own input argument from transfer functions.
 *
 */
typedef void (*gdma_xfer_callback_t)(kdrv_status_t status, void *arg);

/**
 * @details
 * Initialize GDMA driver and allocate necessary memory for internal use. \n
 * Initialization must be performed before any GDMA functionality.
 * @param pCtx point to dsp context
 *
 * @return
 * KDRV_STATUS_OK \n
 * KDRV_STATUS_ERROR
 */
kdrv_status_t kdrv_gdma_initialize(void);

/**
 * @details
 * Uninitialize GDMA driver and release allocated memory.
 *
 * @return
 * KDRV_STATUS_OK
 */
kdrv_status_t kdrv_gdma_uninitialize(void);

/**
 * @details
 * An easy-to-use function for memory-to-memory copy. \n
 * It behaves just like memcpy() (from <stdlib.h>) but it uses DMA hardware to complete the work instead of CPU resources. \n
 * If 'xfer_isr_cb' is NULL then it works as blocking mode API, otherwise as non-blocking mode API and will callback to users when it is done. \n
 * Users can directly use this functions without acquireing a GDMA handle or configuring GDMA settings.
 *
 * @param dst_addr destination address in memory
 * @param src_addr source address in memory
 * @param num_bytes number of bytes to be transfered
 *
 * @return
 * KDRV_STATUS_OK \n
 * KDRV_STATUS_ERROR \n
 * KDRV_STATUS_GDMA_ERROR_NO_RESOURCE
 *
 */
kdrv_status_t kdrv_gdma_memcpy(uint32_t dst_addr, uint32_t src_addr, uint32_t num_bytes,
                               gdma_xfer_callback_t xfer_isr_cb, void *usr_arg);

/**
 * @details
 * open dma deivice. \n
 * A GDMA handle actually represents a GDMA channel. \n
 * The total number of GDMA channels depends on hardware configurations.
 *
 * @return
 * >= 0 : Successfully acquired a GDMA handle. \n
 * < 0 : Failed to acquire a GDMA handle.
 *
 */
kdrv_gdma_handle_t kdrv_gdma_acquire_handle(void);

/**
 * @details
 * Close the specified dma device
 *
 * @param handle Acquired GDMA handle from kdrv_gdma_acquire_handle()
 *
 * @return
 * KDRV_STATUS_OK
 *
 */
kdrv_status_t kdrv_gdma_release_handle(kdrv_gdma_handle_t handle);

/**
 * @brief  Configure GDMA settings for a given GDMA handle (channel).
 *
 * @details
 * There are two working mode for DMA operations : GDMA_NORMAL_MODE and GDMA_HW_HANDSHAKE_MODE. \n
 * \n
 * For memory-to-memory based DMA transfer, user should configure following fields into 'dma_setting'. \n
 * @ dma_mode = GDMA_NORMAL_MODE \n
 * @ dst_width to suitable values \n
 * @ src_width to suitable values \n
 * @ burst_size to suitable values \n
 * @ dst_addr_ctrl to suitable values \n
 * @ src_addr_ctrl to suitable values \n
 * \n
 * For memory-to-peripheral DMA trasfner, followings are also need to be configured properly.
 * @ dma_mode = GDMA_HW_HANDSHAKE_MODE \n
 * @ dma_dst_req to specified value \b
 * @ dma_src_req to specified value
 *
 * @param[in] handle A handle of a GDMA channel.
 * @param[in] dma_setting Specify DMA operation mode and advanced settings. see @ref gdma_setting_t
 * @return  KDRV_STATUS_OK
 *
 */
kdrv_status_t kdrv_gdma_configure_setting(kdrv_gdma_handle_t handle, gdma_setting_t *dma_setting);

/**
 * @details
 * Start DMA transfer with specified DMA handle, in blocking or non-blocking mode. \n
 * Before invoking this function, user must acquire a GDMA handle by kdrv_gdma_acquire_handle() \n
 *
 * @param handle A handle of a GDMA channel.
 * @param the client event
 * @param dst_addr destination address in memory
 * @param src_addr source address in memory
 * @param num_bytes number of bytes to be transfered
 * @param xfer_isr_cb callback function to be invoked on transfer completion. \n
 *                  If it is NULL, the function is in blocking mode, otherwise non-blocking mode.
 * @param usr_arg user's own argument which will be feeded as an input in the callback function.
 *
 * @return
 * KDRV_STATUS_OK \n
 * KDRV_STATUS_ERROR \n
 */
kdrv_status_t kdrv_gdma_transfer(kdrv_gdma_handle_t handle, uint32_t dst_addr, uint32_t src_addr,
                                 uint32_t num_bytes, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg);

#endif // _KDRV_GDMA_H_
