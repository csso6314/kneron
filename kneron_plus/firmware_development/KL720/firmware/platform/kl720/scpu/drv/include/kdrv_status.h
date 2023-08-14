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

#ifndef __KDRV_STATUS_H__
#define __KDRV_STATUS_H__

#include <stdbool.h> // for type 'bool', ture and false.
typedef enum
{
    KDRV_STATUS_OK = 0,                       /**< driver status OK */
    KDRV_STATUS_ERROR,                        /**< driver status error */
    KDRV_STATUS_INVALID_PARAM,                /**< driver invalid parameters */
    KDRV_STATUS_TIMEOUT,                      /**< execution timeout */
    KDRV_STATUS_BUSY,                         /**< execution busy */
    KDRV_STATUS_OUT_OF_MEMORY,                /**< driver allocates memory failed */
    KDRV_STATUS_I2C_BUS_BUSY,                 /**< I2C : bus is busy */
    KDRV_STATUS_I2C_DEVICE_NACK,              /**< I2C : slave device does not send ACK */
    KDRV_STATUS_I2C_TIMEOUT,                  /**< I2C : transfer timeout */
    KDRV_STATUS_USBD_INVALID_ENDPOINT,        /**< USBD : invalid endpoint */
    KDRV_STATUS_USBD_TRANSFER_TIMEOUT,        /**< USBD : transfer timeout */
    KDRV_STATUS_USBD_INVALID_TRANSFER,        /**< USBD : invalid transfer operation */
    KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS,    /**< USBD : transfer is in progress */
    KDRV_STATUS_USBD_TRANSFER_TERMINATED,     /**< USBD : transfer is terminated */
    KDRV_STATUS_GDMA_ERROR_NO_RESOURCE,       /**< GDMA : DMA channel is not available */
    KDRV_STATUS_TIMER_ID_NOT_IN_USED,         /**< Timer: TIMER Id NOT in used */
    KDRV_STATUS_TIMER_ID_IN_USED,             /**< Timer: TIMER Id in used */
    KDRV_STATUS_TIMER_ID_NOT_AVAILABLE,       /**< Timer: TIMER Id is NOT available */
    KDRV_STATUS_TIMER_INVALID_TIMER_ID,       /**< Timer: TIMER Id is invalid */
    KDRV_STATUS_UART_TX_RX_BUSY,              /**< UART: TX or RX is busy */
    KDRV_STATUS_UART_TIMEOUT,                 /**< UART: timeout */
    KDRV_STATUS_SDC_CMD_ERR,                  /**< SDC: command erro */
    KDRV_STATUS_SDC_INIT_ERR,                 /**< SDC: init erro */  
    KDRV_STATUS_SDC_CARD_NO_EXISTED,          /**< SDC: card not existed */  
    KDRV_STATUS_SDC_CARD_TYPE_ERR,            /**< SDC: card type error */  
    KDRV_STATUS_SDC_CSD_EXT_READ_ERR,         /**< SDC: csd info read error */
    KDRV_STATUS_SDC_CID_READ_ERR,             /**< SDC: cid info read error */
    KDRV_STATUS_SDC_MEM_ALLOC_ERR,            /**< SDC: mem alloc fail */
    KDRV_STATUS_SDC_READ_FAIL,                /**< SDC: sdc read fail */
    KDRV_STATUS_SDC_WRITE_FAIL,               /**< SDC: sdc write fail */
    KDRV_STATUS_SDC_TRANSFER_FAIL,            /**< SDC: sdc transfer fail */
    KDRV_STATUS_SDC_TIMEOUT,                  /**< SDC: sdc timeout */
    KDRV_STATUS_SDC_CMD_NOT_SUPPORT,          /**< SDC: sdc command not support */
    KDRV_STATUS_SDC_BUS_WIDTH_NOT_SUPPORT,    /**< SDC: sdc bus width not support */
    KDRV_STATUS_SDC_BUS_WIDTH_ERR,           /**< SDC: sdc bus width fail */
    KDRV_STATUS_SDC_SPEED_MOD_ERR,            /**< SDC: sdc set speed mode error */
    KDRV_STATUS_SDC_VOL_ERR,                  /**< SDC: sdc voltage error */
    KDRV_STATUS_SDC_INHIBIT_ERR,              /**< SDC: sdc inhibit error */
    KDRV_STATUS_SDC_RECOVERABLE_ERR,          /**< SDC: sdc none recoverable */
    KDRV_STATUS_SDC_ABORT_ERR,                /**< SDC: sdc abort error */
    KDRV_STATUS_SDC_SWITCH_ERR,               /**< SDC: sdc switch fail */
    KDRV_STATUS_SDC_PWR_SET_ERR,              /**< SDC: sdc pwr set fail */
    KDRV_STATUS_CRYPTO_UNSUPPORTED_ERR,    /**< CRYPTO: unsupported */ 
    KDRV_STATUS_CRYPTO_INVALID_SIGN_ERR,   /**< CRYPTO: signature verification error */ 
    KDRV_STATUS_CRYPTO_DMA_ERR,            /**< CRYPTO: dma data transfer error, data to too much/not enought to transfer */ 
    KDRV_STATUS_CRYPTO_INVALID_POINT,      /**< CRYPTO: invalid ECC point */ 
    KDRV_STATUS_CRYPTO_ERR,                /**< CRYPTO: engine error */ 
    KDRV_STATUS_CRYPTO_INVALID_PARAM,      /**< CRYPTO: invalid parameter */ 
    KDRV_STATUS_CRYPTO_DRBG_RESEED_NEEDED, /**< CRYPTO: DRBG random generation can't complete */ 
    KDRV_STATUS_CRYPTO_NON_INVERT_SIGN,    /**< CRYPTO: signature is not invertible */ 
} kdrv_status_t;

#endif
