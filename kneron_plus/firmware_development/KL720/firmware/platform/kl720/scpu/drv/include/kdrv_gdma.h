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

/**@addtogroup  KDRV_GDMA KDRV_GDMA
 * @{
 * @brief       Kneron generic DMA driver
 * @version    v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef _KDRV_GDMA_H_
#define _KDRV_GDMA_H_

#include "cmsis_os2.h"
#include "kdrv_status.h"

/** @brief GDMA handle type which represents for a DMA channel and related DMA operations */
typedef int32_t kdrv_gdma_handle_t;

/** @brief This presents GDMAG copy list data structure for users to do chain transfer */
typedef void *kdrv_gdma_copy_list_t;

/** @brief Enumeration of GDMA transfer size: 8/16/32 bits, this is about byte-alignment */
typedef enum
{
    GDMA_TXFER_WIDTH_8_BITS = 0x0, /**< Enum 0, GDMA transfer size: 8 bits */
    GDMA_TXFER_WIDTH_16_BITS,      /**< Enum 1, GDMA transfer size: 16 bits */
    GDMA_TXFER_WIDTH_32_BITS,      /**< Enum 2, GDMA transfer size: 32 bits, default value */
} gdma_transfer_width_t;

/** @brief Enumeration of GDMA transfer burst : 1/4/8/16/32/64/128/256, this is about performance */
typedef enum
{
    GDMA_BURST_SIZE_1 = 0x0, /**< Enum 0, GDMA transfer burst size: 1 */
    GDMA_BURST_SIZE_4,       /**< Enum 1, GDMA transfer burst size: 4 */
    GDMA_BURST_SIZE_8,       /**< Enum 2, GDMA transfer burst size: 8 */
    GDMA_BURST_SIZE_16,      /**< Enum 3, GDMA transfer burst size: 16, default value */
    GDMA_BURST_SIZE_32,      /**< Enum 4, GDMA transfer burst size: 32 */
    GDMA_BURST_SIZE_64,      /**< Enum 5, GDMA transfer burst size: 64 */
    GDMA_BURST_SIZE_128,     /**< Enum 6, GDMA transfer burst size: 128 */
    GDMA_BURST_SIZE_256,     /**< Enum 7, GDMA transfer burst size: 256 */
} gdma_burst_size_t;

/** @brief Enumeration of DMA address control, auto-increasing/descreading or fixed */
typedef enum
{
    GDMA_INCREMENT_ADDRESS = 0x0, /**< Enum 0, DMA address control, auto-increasing, default value */
    GDMA_DECREMENT_ADDRESS,       /**< Enum 1, DMA address control, auto-descreading */
    GDMA_FIXED_ADDRESS,           /**< Enum 2, DMA address control, fixed */
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
 * @brief Structure of GDMA settings data setting.
 */
typedef struct
{
    gdma_transfer_width_t dst_width;      /**< see @ref gdma_transfer_width_t */
    gdma_transfer_width_t src_width;      /**< see @ref gdma_transfer_width_t */
    gdma_burst_size_t burst_size;         /**< see @ref gdma_burst_size_t */
    gdma_address_control_t dst_addr_ctrl; /**< see @ref gdma_address_control_t */
    gdma_address_control_t src_addr_ctrl; /**< see @ref gdma_address_control_t */
    gdma_work_mode_t dma_mode;            /**< see @ref gdma_work_mode_t */
    int dma_dst_req;                      /**< see @ref gdma_hw_request_t */
    int dma_src_req;                      /**< see @ref gdma_hw_request_t */
} gdma_setting_t;

/** @brief Structure of a GDMA cropping copy descriptor */
typedef struct
{
    uint32_t start_dst_addr; /**< start destination address */
    uint32_t start_src_addr; /**< start source address */
    uint32_t txfer_bytes;    /**< number of bytes for one section of DAM copy */
    uint32_t dst_offset;     /**< destination address offset for next section DMA copy */
    uint32_t src_offset;     /**< source address offset for next section DMA copy */
    uint32_t num_txfer;      /**< total number of transfer DMA copy will be performed */
} gdma_cropping_descriptor_t;

/**
 * @brief Callback function for notifying users of DMA transfer completeion. \n
 *
 * @param[in] status Transfer status, it can be one of followings: \n
 *          KDRV_STATUS_OK : Transfer is successuflly completed. \n
 *          KDRV_STATUS_ERROR : Transfer is failed due to an errror or abortion.
 *
 * @param[in] arg User's own input argument from transfer functions.
 * @note  the context of this callback function is from ISR.
 *
 */
typedef void (*gdma_xfer_callback_t)(kdrv_status_t status, void *arg);

/**
 * @details 
 * Initialize GDMA driver and allocate necessary memory for internal use. \n
 * Initialization must be performed before any GDMA functionality.
 *
 * @param[in] N/A
 * @return
 * KDRV_STATUS_OK \n
 * KDRV_STATUS_ERROR
 */
kdrv_status_t kdrv_gdma_initialize(void);

/**
 * @brief   Uninitialize GDMA driver and release allocated memory.
 *
 * @param[in] N/A
 * @return KDRV_STATUS_OK
 */
kdrv_status_t kdrv_gdma_uninitialize(void);

/**
 * @details
 * An easy-to-use function for memory-to-memory copy. \n
 * It behaves just like memcpy() (from <stdlib.h>) but it uses DMA hardware to complete the work instead of CPU resources. \n
 * If 'xfer_isr_cb' is NULL then it works as blocking mode API, otherwise as non-blocking mode API and will callback to users when it is done. \n
 * Users can directly use this functions without acquireing a GDMA handle or configuring GDMA settings.
 *
 * @param[in] dst_addr destination address in memory
 * @param[in] src_addr source address in memory
 * @param[in] num_bytes number of bytes to be transfered
 * @param[in] xfer_isr_cb callback function to be invoked on transfer completion. \n
 *                  If it is NULL, the function is in blocking mode, otherwise non-blocking mode.
 * @param[in] usr_arg user's own argument which will be feeded as an input in the callback function.
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
 * Acquire a GDMA handle from driver for further DMA operations. \n
 * A GDMA handle actually represents a GDMA channel. \n
 * The total number of GDMA channels depends on hardware configurations, and some are used for kdrv_gdma_memcpy().
 * After successfully acquiring a GDMA handle, next is to configure GDMA setting on this handle through kdrv_gdma_configure_setting()
 *
 * @param[in] N/A
 * @return
 * >= 0 : Successfully acquired a GDMA handle. \n
 * < 0 : Failed to acquire a GDMA handle.
 * 
 */
kdrv_gdma_handle_t kdrv_gdma_acquire_handle(void);

/**
 * @brief Release the GDMA handle which is acquired from kdrv_gdma_acquire_handle()
 *
 * @param[in] handle Acquired GDMA handle from kdrv_gdma_acquire_handle()
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
 * @brief   Start DMA transfer with specified DMA handle, in blocking or non-blocking mode.
 *
 * @details Before invoking this function, user must acquire a GDMA handle by kdrv_gdma_acquire_handle() and configure appropriate settings by kdrv_gdma_configure_setting(). \n
 *
 * @param[in] handle A handle of a GDMA channel.
 * @param[in] dst_addr destination address in memory
 * @param[in] src_addr source address in memory
 * @param[in] num_bytes number of bytes to be transfered
 * @param[in] xfer_isr_cb callback function to be invoked on transfer completion. \n
 *                  If it is NULL, the function is in blocking mode, otherwise non-blocking mode.
 * @param[in] usr_arg user's own argument which will be feeded as an input in the callback function.
 *
 * @return
 * KDRV_STATUS_OK \n
 * KDRV_STATUS_ERROR \n
 */
kdrv_status_t kdrv_gdma_transfer(kdrv_gdma_handle_t handle, uint32_t dst_addr, uint32_t src_addr,
                                 uint32_t num_bytes, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg);

/**
 * @brief Abort running DMA trasnfer.
 *
 * @param[in] handle A handle of a GDMA channel.
 * @return  KDRV_STATUS_OK
 */
kdrv_status_t kdrv_gdma_abort_transfer(kdrv_gdma_handle_t handle);

/**
 * @details
 * A 'copy_list' transfer represents multiple of DMA transfer on different blocks of memory from/to source and destination address. \n
 * Users can use this function to prepare a GDMA 'copy_list' data structure by information settings of memory blocks. \n
 * This function internally allocates a memory to store the 'copy_list' for users, so if it is done, remember to invoke the kdrv_gdma_free_copy_list() to free allocated memory.
 *
 * @param[in] dst_addr An array of destination adress for a chain of DMA write destination blocks.
 * @param[in] src_addr An array of source adress for a chain of DMA read destination blocks.
 * @param[in] num_bytes An array of numbers to describe the number of bytes to transfer for each block.
 * @param[in] count Total number of DMA chain-transfer work to be performed, i.e. array set size.
 *
 * @return  see @ref kdrv_gdma_copy_list_t
 */
kdrv_gdma_copy_list_t kdrv_gdma_allocate_copy_list(kdrv_gdma_handle_t handle, uint32_t dst_addr[],
                                                   uint32_t src_addr[], uint32_t num_bytes[], uint32_t count);

/**
 * @brief  Free the 'copy_list' which is allocated previously by kdrv_gdma_allocate_copy_list()
 *
 * @param[in] copy_list A data struture for describing a chain of DMA transfer work.
 * @return KDRV_STATUS_OK
 *
 */
kdrv_status_t kdrv_gdma_free_copy_list(kdrv_gdma_copy_list_t copy_list);

/**
 * @brief   Perform a chain of DMA transfer work by a 'copy_list' in blocing or non-blocking mode.
 *
 * @details
 * This is the multiple-copied version of kdrv_gdma_transfer(). \n
 * Before invoking this function, user must perform following calls: \n
 * @ Acquire a GDMA handle by kdrv_gdma_acquire_handle(). \n
 * @ Configure appropriate settings by kdrv_gdma_configure_setting(). \n
 * @ Allocate a copy_list by kdrv_gdma_allocate_copy_list().
 *
 * @param[in] handle A handle of a GDMA channel.
 * @param[in] copy_list A data struture for describing a chain of DMA transfer work.
 * @param[in] xfer_isr_cb Callback function to be invoked on transfer completion. \n
 *                    If it is NULL, the function is in blocking mode, otherwise non-blocking mode.
 * @param[in] usr_arg user's own argument which will be feeded as an input in the callback function.
 *
 * @return
 * KDRV_STATUS_OK \n
 * KDRV_STATUS_ERROR \n
 */
kdrv_status_t kdrv_gdma_transfer_copy_list(kdrv_gdma_handle_t handle, kdrv_gdma_copy_list_t copy_list,
                                           gdma_xfer_callback_t xfer_isr_cb, void *usr_arg);

/**
 * @brief   Perform a DMA cropping-like copy by user-specified @ref gdma_cropping_descriptor_t.
 *
 * @details
 * This function is like kdrv_gdma_memcpy(), and it avoid users to confgiure settings and allocate the copy_list.
 * However the limitation is that all addresses and size and offset must be double-world (32-bit) aligned.
 *
 * @param[in] cropping_desc Describe the source and destination cropping area and transfer bytes, see @ref gdma_cropping_descriptor_t.
 * @param[in] xfer_isr_cb Callback function to be invoked on transfer completion. \n
 *                    If it is NULL, the function is in blocking mode, otherwise non-blocking mode.
 * @param[in] usr_arg User's own argument which will be feeded as an input in the callback function.
 *
 * @return
 * KDRV_STATUS_OK \n
 * KDRV_STATUS_ERROR \n
 */
kdrv_status_t kdrv_gdma_memcpy_cropping(gdma_cropping_descriptor_t *cropping_desc, gdma_xfer_callback_t xfer_isr_cb, void *usr_arg);

#endif // _KDRV_GDMA_H_
/** @}*/

