
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

/**@addtogroup  KDRV_USBH2
 * @{
 * @brief       Kneron USB2 host mode driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_USBH2_H__
#define __KDRV_USBH2_H__

#include "kdrv_status.h"

/** @brief Enumerations of USB Speed*/
typedef enum
{
    USBH2_SPEED_LOW = 0,        /**< Enum 0, USBH2 low speed */
    USBH2_SPEED_FULL = 1,       /**< Enum 1, USBH2 full speed */
    USBH2_SPEED_HIGH = 2,       /**< Enum 2, USBH2 high speed */
} kdrv_usbh2_speed_t;


/** @brief Structure of USB Port State*/
typedef volatile struct _ARM_USBH_PORT_STATE
{
    uint32_t connected : 1;
    uint32_t overcurrent : 1;
    uint32_t speed : 2; // kdrv_usbh2_speed_t
    uint32_t reserved : 28;
} kdrv_usbh2_port_state_t;

/** @brief Enumerations of USB Host Port Event*/
typedef enum
{
    USBH2_EVENT_CONNECT = (1UL << 0),               /**< Enum 0x1, USBH2 connect event */
    USBH2_EVENT_DISCONNECT = (1UL << 1),            /**< Enum 0x2, USBH2 disconect event */
    USBH2_EVENT_OVERCURRENT = (1UL << 2),       	/**< Enum 0x4, USBH2 overcurrent event */
    USBH2_EVENT_RESET = (1UL << 3),                 /**< Enum 0x8, USBH2 reset  event*/
    USBH2_EVENT_SUSPEND = (1UL << 4),               /**< Enum 0x10, USBH2 suspend event */
    USBH2_EVENT_RESUME = (1UL << 5),                /**< Enum 0x20, USBH2 resume event */
    USBH2_EVENT_REMOTE_WAKEUP = (1UL << 6),     	/**< Enum 0x40, USBH2 wakeup event */
} kdrv_usbh2_port_event_t;

/** @brief Enumerations of USB Host Pipe Event */
typedef enum
{
    USBH2_EVENT_TRANSFER_COMPLETE = (1UL << 0),      /**< Enum 0x1, Transfer completed */
    USBH2_EVENT_HANDSHAKE_NAK = (1UL << 1),          /**< Enum 0x2, NAK Handshake received */
    USBH2_EVENT_HANDSHAKE_NYET = (1UL << 2),         /**< Enum 0x4, NYET Handshake received */
    USBH2_EVENT_HANDSHAKE_MDATA = (1UL << 3),        /**< Enum 0x8, MDATA Handshake receivedt*/
    USBH2_EVENT_HANDSHAKE_STALL = (1UL << 4),        /**< Enum 0x10, STALL Handshake received */
    USBH2_EVENT_HANDSHAKE_ERR = (1UL << 5),          /**< Enum 0x20, ERR Handshake received */
    USBH2_EVENT_BUS_ERROR = (1UL << 6),              /**< Enum 0x40, Bus Error detected */
} kdrv_usbh2_pipe_event_t;

/** @brief Enumerations of USB Endpoint Type */
typedef enum
{
    USBH2_ENDPOINT_CONTROL = 0,             /**< Enum 0, USBH2 endpoint control */
    USBH2_ENDPOINT_ISOCHRONOUS = 1,     	/**< Enum 1, USBH2 endpoint irochrounous */
    USBH2_ENDPOINT_BULK = 2,                /**< Enum 2, USBH2 endpoint bulk */
    USBH2_ENDPOINT_INTERRUPT = 3,           /**< Enum 3, USBH2 endpoint interupt */
} kdrv_usbh2_endpoint_type_t;

/** @brief Enumerations of USB Host Packet Information */
typedef enum
{
#define USBH2_PACKET_TOKEN_Pos 0
    USBH2_PACKET_TOKEN_Msk = (0x0FUL << USBH2_PACKET_TOKEN_Pos),
    USBH2_PACKET_SETUP = (0x01UL << USBH2_PACKET_TOKEN_Pos), ///< SETUP Packet
    USBH2_PACKET_OUT = (0x02UL << USBH2_PACKET_TOKEN_Pos),   ///< OUT Packet
    USBH2_PACKET_IN = (0x03UL << USBH2_PACKET_TOKEN_Pos),    ///< IN Packet
    USBH2_PACKET_PING = (0x04UL << USBH2_PACKET_TOKEN_Pos),  ///< PING Packet

#define USBH2_PACKET_DATA_Pos 4
    USBH2_PACKET_DATA_Msk = (0x0FUL << USBH2_PACKET_DATA_Pos),
    USBH2_PACKET_DATA0 = (0x01UL << USBH2_PACKET_DATA_Pos), ///< DATA0 PID
    USBH2_PACKET_DATA1 = (0x02UL << USBH2_PACKET_DATA_Pos), ///< DATA1 PID

#define USBH2_PACKET_SPLIT_Pos 8
    USBH2_PACKET_SPLIT_Msk = (0x0FUL << USBH2_PACKET_SPLIT_Pos),
    USBH2_PACKET_SSPLIT = (0x08UL << USBH2_PACKET_SPLIT_Pos),     ///< SSPLIT Packet
    USBH2_PACKET_SSPLIT_S = (0x09UL << USBH2_PACKET_SPLIT_Pos),   ///< SSPLIT Packet: Data Start
    USBH2_PACKET_SSPLIT_E = (0x0AUL << USBH2_PACKET_SPLIT_Pos),   ///< SSPLIT Packet: Data End
    USBH2_PACKET_SSPLIT_S_E = (0x0BUL << USBH2_PACKET_SPLIT_Pos), ///< SSPLIT Packet: Data All
    USBH2_PACKET_CSPLIT = (0x0CUL << USBH2_PACKET_SPLIT_Pos),     ///< CSPLIT Packet
} kdrv_usbh2_packet_t;

typedef uint32_t kdrv_usbh2_pipe_t;
typedef void (*kdrv_usbh2_port_event_callback_t)(uint8_t port, kdrv_usbh2_port_event_t event);
typedef void (*kdrv_usbh2_pipe_event_callback_t)(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_pipe_event_t event);
typedef void (*kdrv_usbh2_isoch_data_callback_t)(uint32_t *payload, uint32_t length);
typedef uint32_t (*kdrv_usbh2_isoch_bf_callback_t)();
typedef uint32_t (*kdrv_usbh2_isoch_itd_work_func_t)();


/**
 * @brief           USBH2 host mode driver initialization
 * @param[in]       cb_port_event              see @ref kdrv_usbh2_port_event_callback_t
 * @param[in]       cb_pipe_event              see @ref kdrv_usbh2_pipe_event_callback_t
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_initialize(
    kdrv_usbh2_port_event_callback_t cb_port_event,
    kdrv_usbh2_pipe_event_callback_t cb_pipe_event);

/**
 * @brief           USBH2 host mode driver de-initialization
 *
 * @param[in]     N/A
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_uninitialize(void);

kdrv_status_t kdrv_usbh2_set_enable(bool enable);

kdrv_status_t kdrv_usbh2_vbus_on_off(uint8_t port, bool vbus);

/**
 * @brief           USBH2 port reset
 *
 * @param[in]       port
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_port_reset(uint8_t port);

/**
 * @brief           USBH2 reset suspend
 *
 * @param[in]       port
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_port_suspend(uint8_t port);

/**
 * @brief           USBH2 reset resume
 *
 * @param[in]      port
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_port_resume(uint8_t port);

/**
 * @brief           USBH2 get port state
 *
 * @param[in]      port
 * @return          kdrv_usbh2_port_state_t         see @ref kdrv_usbh2_port_state_t
 */
kdrv_usbh2_port_state_t kdrv_usbh2_port_get_state(uint8_t port);

/**
 * @brief        USBH2 create pipe
 *
 * @param[in]    dev_addr                       device address
 * @param[in]    dev_speed                      device speed
 * @param[in]    hub_addr                       usb hub address
 * @param[in]    hub_port                       usb hub port
 * @param[in]    ep_addr                        endpoint address
 * @param[in]    ep_type                        endpoint type
 * @param[in]    ep_max_packet_size     		endpoint max packet size
 * @param[in]    ep_interval                    endpoint interval
 * @return       kdrv_usbh2_pipe_t         		see @ref kdrv_usbh2_pipe_t
 */
kdrv_usbh2_pipe_t kdrv_usbh2_pipe_create(uint8_t dev_addr, uint8_t dev_speed, uint8_t hub_addr,
                                         uint8_t hub_port, uint8_t ep_addr, uint8_t ep_type,
                                         uint16_t ep_max_packet_size, uint8_t ep_interval);

/**
 * @brief        USBH2 modify pipe
 *
 * @param[in]    pipe_hndl                      see @ref kdrv_usbh2_pipe_t
 * @param[in]    dev_addr                       device address
 * @param[in]    dev_speed                      device speed
 * @param[in]    hub_addr                       usb hub address
 * @param[in]    hub_port                       usb hub port
 * @param[in]    ep_max_packet_size     		endpoint max packet size
 * @return       kdrv_status_t               	see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_pipe_modify(kdrv_usbh2_pipe_t pipe_hndl, uint8_t dev_addr,
                                     uint8_t dev_speed, uint8_t hub_addr, uint8_t hub_port, uint16_t ep_max_packet_size);

/**
 * @brief        USBH2 pipe transfer
 *
 * @param[in]    pipe_hndl                       see @ref kdrv_usbh2_pipe_t
 * @param[in]    packet                          seew @ref kdrv_usbh2_packet_t
 * @param[in]    *data                           Pointer to data buffer
 * @param[in]    num
 * @return       kdrv_status_t               	see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_pipe_transfer(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_packet_t packet, uint8_t *data, uint32_t num);

/**
 * @brief           USBH2 pipe transfer get result
 *
 * @param[in]    	pipe_hndl                               see @ref kdrv_usbh2_pipe_t
 * @return          kdrv_status_t                       	see @ref kdrv_status_t
 */
uint32_t kdrv_usbh2_pipe_transfer_get_result(kdrv_usbh2_pipe_t pipe_hndl);

/**
 * @brief           USBH2 pipe transfer abort
 *
 * @param[in]    	pipe_hndl                              see @ref kdrv_usbh2_pipe_t
 * @return          kdrv_status_t                       	see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_pipe_transfer_abort(kdrv_usbh2_pipe_t pipe_hndl);

/**
 * @brief           USBH2 pipe reset
 *
 * @param[in]    	pipe_hndl                           see @ref kdrv_usbh2_pipe_t
 * @return          kdrv_status_t                       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_pipe_reset(kdrv_usbh2_pipe_t pipe_hndl);

/**
 * @brief           USBH2 pipe delete
 *
 * @param[in]    	pipe_hndl                           see @ref kdrv_usbh2_pipe_t
 * @return          kdrv_status_t                       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_pipe_delete(kdrv_usbh2_pipe_t pipe_hndl);

/**
 * @brief           USBH2 get frame number
 *
 * @param[in]    	N/A
 * @return          frame number
 */
uint16_t kdrv_usbh2_get_frame_number(void);

/**
 * @brief           USBH2 pipe isoch create
 *
 * @param[in]    	dev_addr
 * @param[in]    	ep_addr
 * @param[in]    	max_packet_size
 * @param[in]    	mult
 * @param[in]    	ep_interval
 * @param[in]    	*buf
 * @param[in]    	buf_size
 * @return          kdrv_usbh2_pipe_t   see @ref kdrv_usbh2_pipe_t
 */
kdrv_usbh2_pipe_t kdrv_usbh2_pipe_isoch_create(uint8_t dev_addr, uint8_t ep_addr,
                                               uint16_t max_packet_size, uint8_t mult, uint8_t ep_interval,
                                               uint8_t *buf, uint32_t buf_size);

/**
 * @brief           USBH2 pipe isoch transfer start
 *
 * @param[in]    	pipe_hndl                              see @ref kdrv_usbh2_pipe_t
 * @param[in]    	isoch_data_cb                       see @ref kdrv_usbh2_isoch_data_callback_t
 * @return          kdrv_status_t                       see @ref kdrv_status_t
 */
kdrv_status_t kdrv_usbh2_pipe_isoch_start(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_isoch_data_callback_t isoch_data_cb);

/**
 * @brief           USBH2 pipe isoch transfer stop
 *
 * @param[in]    	pipe_hndl                              see @ref kdrv_usbh2_pipe_t
 * @return          KDRV_STATUS_OK
 */
int32_t kdrv_usbh2_pipe_isoch_stop(kdrv_usbh2_pipe_t pipe_hndl);

/**
 * @brief           USBH2 pipe isoch transfer enable bh
 *
 * @param[in]    	pipe_hndl                           see @ref kdrv_usbh2_pipe_t
 * @param[in]    	isoch_bf_callback                  	see @ref kdrv_usbh2_isoch_bf_callback_t
 * @return          kdrv_usbh2_isoch_itd_work_func_t    see @ref kdrv_usbh2_isoch_itd_work_func_t
 */
kdrv_usbh2_isoch_itd_work_func_t kdrv_usbh2_pipe_isoch_enable_bh(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_isoch_bf_callback_t isoch_bf_callback);

void kdrv_usbh2_pipe_isoch_set_pause(bool bPause);

#endif
/** @}*/

