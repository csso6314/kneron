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

/**@addtogroup  KDRV_I2S  KDRV_I2S
 * @{
 * @brief       Kneron i2s driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
 
#ifndef __KDRV_I2S_H__
#define __KDRV_I2S_H__

#include <stdint.h>
#include "kdrv_status.h"
#include "regbase.h"
#include "kdrv_gdma.h"

/** @defgroup I2S_BIT_SEQ I2S Bit Sequence
  * @{
  */
#define I2S_MSB_FIRST   0
#define I2S_LSB_FIRST   1
/**
  * @}
  */
  
/** @defgroup I2S_Audio_Freq I2S Audio Frequency
  * @{
  */
#define I2S_AUDIO_FREQ_192K               (192000U)
#define I2S_AUDIO_FREQ_96K                (96000U)
#define I2S_AUDIO_FREQ_48K                (48000U)
#define I2S_AUDIO_FREQ_44K                (44100U)
#define I2S_AUDIO_FREQ_32K                (32000U)
#define I2S_AUDIO_FREQ_22K                (22050U)
#define I2S_AUDIO_FREQ_16K                (16000U)
#define I2S_AUDIO_FREQ_11K                (11025U)
#define I2S_AUDIO_FREQ_8K                 (8000U)
/**
  * @}
  */
  
/** @defgroup I2S_PAD_MODE I2S Padding Mode
  * @{
  */
#define I2S_PAD_FRONT                     1
#define I2S_PAD_BACK                      0
/**
  * @}
  */
  
/** @defgroup kdrv_i2s_mode_t I2S Mode definition
  * @{
  */
typedef enum{
    I2S_SLAVE_MONO,
    I2S_SLAVE_STEREO,
    I2S_MASTER_MONO,    
    I2S_MASTER_STEREO
}kdrv_i2s_mode_t;
/**
  * @}
  */

  
/** @defgroup kdrv_i2s_instance_t I2S Instance definition
  * @{
  */
typedef enum{
    I2S_INSTANCE_0,
    I2S_INSTANCE_1,
    TOTAL_KDRV_I2S_INSTANCE
}kdrv_i2s_instance_t;

/**
  * @}
  */

/** @defgroup I2S_HANDLE I2S handle definition
  * @{
  */
typedef struct{
    uint8_t instance; /* This parameter can be a value of @ref kdrv_i2s_instance_t */
    uint8_t mode;
    uint32_t sample_freq;   /* This parameter can be a value of @ref I2S_Audio_Freq */
    uint8_t bit_seq;        /* This parameter can be a value of @ref I2S_BIT_SEQ */
    uint8_t data_len;       
    uint8_t padding_len;    
    uint8_t padding_mode;   /* This parameter can be a value of @ref I2S_PAD_MODE */
    uint8_t output_mclk;
}kdrv_i2s_attr_context;

/**
  * @}
  */

/**
 * @brief       Init the i2s module
 *
 * @param[in]   *handle         Pointer to the i2s handle, see @ref I2S_HANDLE
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_i2s_init(uint8_t num, kdrv_i2s_attr_context *attr);

/**
 * @brief       Send data with i2s module
 *
 * @param[in]   *handle         Pointer to the i2s handle, see @ref I2S_HANDLE
 * @param[in]   val             The data to be sent
 * @return      kdrv_status_t   see @ref kdrv_status_t. If the return is KDRV_STATUS_BUSY, it means the fifo is full. Simply try again
 */
kdrv_status_t kdrv_i2s_send(kdrv_i2s_instance_t instance, uint32_t val);

/**
 * @brief       Read data from i2s module
 *
 * @param[in]   *handle         Pointer to the i2s handle, see @ref I2S_HANDLE
 * @param[in]   *val            Pointer to the data to be sent
 * @return      kdrv_status_t   see @ref kdrv_status_t. If the return is KDRV_STATUS_ERROR, it means the fifo empty.
 */
kdrv_status_t kdrv_i2s_read(kdrv_i2s_instance_t instance, uint32_t *val);

/**
 * @brief       Enable i2s module transmit
 *
 * @param[in]   *handle         Pointer to the i2s handle, see @ref I2S_HANDLE
 * @param[in]   enable         0: disable, 1: enable
 * @return      N/A
 */
void kdrv_i2s_enable_tx(kdrv_i2s_instance_t instance, uint8_t enable);

/**
 * @brief       Enable i2s module receive
 *
 * @param[in]   *handle         Pointer to the i2s handle, see @ref I2S_HANDLE
 * @param[in]   enable         0: disable, 1: enable
 * @return      N/A
 */
void kdrv_i2s_enable_rx(kdrv_i2s_instance_t instance, uint8_t enable);

/**
 * @brief       Send data with i2s module with DMA. 
 *
 * @param[in]   *handle         Pointer to the i2s handle, see @ref I2S_HANDLE
 * @param[in]   *dma_handle     Pointer to the kdrv_gdma_handle_t, Note that the gdma need to be initialized first and the dma_handle need to be acquired
 * @param[in]   *buf            Pointer to the data to be sent
 * @return      kdrv_status_t   see @ref kdrv_status_t. 
 */
kdrv_status_t kdrv_i2s_dma_send(kdrv_i2s_instance_t instance, kdrv_gdma_handle_t dma_handle, uint32_t *buf, uint32_t size, gdma_xfer_callback_t xfer_isr_cb);

/**
 * @brief       Rend data with i2s module with DMA. 
 *
 * @param[in]   *handle         Pointer to the i2s handle, see @ref I2S_HANDLE
 * @param[in]   *dma_handle     Pointer to the kdrv_gdma_handle_t, Note that the gdma need to be initialized first and the dma_handle need to be acquired
 * @param[in]   *buf            Pointer to the data to be sent
 
 * @return      kdrv_status_t   see @ref kdrv_status_t. 
 */
kdrv_status_t kdrv_i2s_dma_read(kdrv_i2s_instance_t instance, kdrv_gdma_handle_t dma_handle, uint32_t *buf, uint32_t size, gdma_xfer_callback_t xfer_isr_cb);
#endif
