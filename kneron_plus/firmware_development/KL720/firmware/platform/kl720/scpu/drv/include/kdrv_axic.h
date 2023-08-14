
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

/**@addtogroup  KDRV_AXIC
 * @{
 * @brief       Kneron AXI interconnect driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_AXIC_H__
#define __KDRV_AXIC_H__

#include <stdint.h>
#include "kdrv_status.h"

/**
 * @brief AXIC master port number definition
 * 
 */
typedef enum{
    KDRV_AXIC_MASTER_PORT_AHB_MIPI1 = 0,    /**< Enum 0, AXIC port number for AHB and MIPI1 */
    KDRV_AXIC_MASTER_PORT_DMAC030,          /**< Enum 1, AXIC port number for DMAC030 */
    KDRV_AXIC_MASTER_PORT_USB3,             /**< Enum 2, AXIC port number for USB3 */
    KDRV_AXIC_MASTER_PORT_NPU,              /**< Enum 3, AXIC port number for NPU */
    KDRV_AXIC_MASTER_PORT_DSP,              /**< Enum 4, AXIC port number for DSP */
    KDRV_AXIC_MASTER_PORT_DSP_DMA,          /**< Enum 5, AXIC port number for DMA module in DSP */
    KDRV_AXIC_MASTER_PORT_USB2,             /**< Enum 6, AXIC port number for USB2 */
    KDRV_AXIC_MASTER_PORT_LCDC,             /**< Enum 7, AXIC port number for LCDC */
    KDRV_AXIC_MASTER_PORT_MIPI0 = 9,        /**< Enum 9, AXIC port number for MIPI0 */
    KDRV_AXIC_MASTER_PORT_CPU               /**< Enum 10, AXIC port number for CPU(CM4) */
}kdrv_axic_master_port_t;

/**
 * @brief AXIC slave port number definition
 * 
 */
typedef enum{
    KDRV_AXIC_SLAVE_PORT_USB2 = 1,          /**< Enum 1, AXIC port number for UBS2 */
    KDRV_AXIC_SLAVE_PORT_DMAC030,           /**< Enum 2, AXIC port number for DMAC030 */
    KDRV_AXIC_SLAVE_PORT_USB3 = 5,          /**< Enum 5, AXIC port number for UBS3 */
    KDRV_AXIC_SLAVE_PORT_DDR_1 = 7,         /**< Enum 7, AXIC port number for DDR */
    KDRV_AXIC_SLAVE_PORT_DDR_2,             /**< Enum 8, AXIC port number for DDR */
    KDRV_AXIC_SLAVE_PORT_DDR_3,             /**< Enum 9, AXIC port number for DDR */
    KDRV_AXIC_SLAVE_PORT_DDR_4,             /**< Enum 10, AXIC port number for DDR */
    KDRV_AXIC_SLAVE_PORT_NPU = 13,          /**< Enum 13, AXIC port number for NPU */
    KDRV_AXIC_SLAVE_PORT_DSP = 15,          /**< Enum 15, AXIC port number for DSP */
    KDRV_AXIC_SLAVE_PORT_X2P,               /**< Enum 16, AXIC port number for X2P */
    KDRV_AXIC_SLAVE_PORT_X2H = 31           /**< Enum 31, AXIC port number for X2H */
}kdrv_axic_slave_port_t;

/**
 * @brief AXIC port number priotory definition
 * 
 */
typedef enum{
    KDRV_AXIC_PRIORITY_LOW = 0,             /**< Lowest priotiry */
    KDRV_AXIC_PRIORITY_MEDIUM,              /**< Medium priotiry */
    KDRV_AXIC_PRIORITY_HIGH,                /**< High priotiry */
    KDRV_AXIC_PRIORITY_URGENT               /**< Highest priotiry */
}kdrv_axic_priority_t;

/**
 * @brief AXIC port qos definition
 * 
 */
typedef struct 
{
    uint8_t bw_limit;                       /**< The maximum allowed data cycles within the bandwidth monitor period
                                                0: N/A, 1: 1/256 of bandwidth period, 255: 255/256 of bandwidth period*/
    uint8_t bw_period;                      /**< Channel bandwidth monitor period.
                                                0: 512 cycles, 1: 1024 cycles, 2: 2048 cycles, 3: 4096 cycles, 4: 8192 cycles*/
    uint8_t bw_en;                          /**< Bandwidth control enable/disable */
    uint8_t mask_period;                    /**< Predefined request mask period when bandwidth reaches limit. 
                                                0: 3 cycles, 1: 5 cycles, 2: 7 cycles, ... ,15: 33 cycles */
    uint8_t req_mask;                       /**< Channel command request mask function enable. Only takes effect when bw_en = 1,
                                                0: request signal is masked when the bandwidth reaches the limit, 1: request signal is masked for a predefined period when the bandwidth reaches the limit */
    uint8_t outstd_num;                     /**< Channel outstanding command number. Range: 0~7 */
    uint8_t outstd_en;                      /**< Channel outstanding command control */
}kdrv_axic_qos_t;

/**
 * @brief           Set default configuration of the AXIC
 *
 * @param[in]       N/A
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_set_default_config(void);

/**
 * @brief           Set AwQos to a certain master
 * 
 * @param[in]       port                    port number
 * @param[in]       qos                     pointer to a user allocated kdrv_axic_qos_t object 
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_set_master_awqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos);

/**
 * @brief           Get AwQos of a certain master
 * 
 * @param[in]       port                    port number
 * @param[out]      qos                     pointer to a user allocated kdrv_axic_qos_t object 
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_get_master_awqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos);

/**
 * @brief           Set ArQos to a certain master
 * 
 * @param[in]       port                    port number
 * @param[in]       qos                     pointer to a user allocated kdrv_axic_qos_t object 
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_set_master_arqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos);

/**
 * @brief           Get ArQos of a certain master
 * 
 * @param[in]       port                    port number
 * @param[out]      qos                     pointer to a user allocated kdrv_axic_qos_t object 
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_get_master_arqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos);

/**
 * @brief           Set priority to a certain master
 * 
 * @param[in]       port                    port number
 * @param[in]       aw_p                    priority for aw channel
 * @param[in]       ar_p                    priority for ar channel
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_set_master_priority(kdrv_axic_master_port_t port, kdrv_axic_priority_t aw_p, kdrv_axic_priority_t ar_p);

/**
 * @brief           Get priority to a certain master
 * 
 * @param[in]       port                    port number
 * @param[out]      aw_p                    pointer to the object to store the priority for aw channel
 * @param[out]      ar_p                    pointer to the object to store the priority for ar channel
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_get_master_priority(kdrv_axic_master_port_t port, kdrv_axic_priority_t *aw_p, kdrv_axic_priority_t *ar_p);

/**
 * @brief           Set priority to a certain slave
 * 
 * @param[in]       port                    port number
 * @param[in]       p                       priority
 * @return          kdrv_status_t           see @ref kdrv_status_t
 */
kdrv_status_t kdrv_axic_set_slave_priority(kdrv_axic_slave_port_t port, kdrv_axic_priority_t p);

/**
 * @brief           Get priority to a certain slave
 * 
 * @param[in]       port                    port number
 * @return          kdrv_axic_priority_t    priority
 */
kdrv_axic_priority_t kdrv_axic_get_slave_priority(kdrv_axic_master_port_t port);

#endif
/** @}*/
