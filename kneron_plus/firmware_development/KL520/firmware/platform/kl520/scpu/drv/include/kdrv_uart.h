/**
 * @file        kdrv_uart.h
 * @brief       Kneron UART driver
 * @details     Here are the design highlight points:\n
 *              * The architecture adopts a lightweight non-thread design\n
 *              * ISR driven architecture.\n
 *              * Can support both synchronous and asynchronous mode\n
 *              * Utilizes FIFO advantage to reduce interrupts and improve robust to accommodate more latency than normal.
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_UART_H_
#define __KDRV_UART_H_

#ifndef NON_OS
#include "cmsis_os2.h"
#endif
#include "kdrv_status.h"
#include "Driver_USART.h"
#include "system_config.h"
#include "regbase.h"

/**
 * @brief Enumerations of UART baud rate.
 */
#define BAUD_921600 (UART_CLOCK / 14745600)     /**< UART baud rate: 921600. */
#define BAUD_460800 (UART_CLOCK / 7372800)      /**< UART baud rate: 460800. */
#define BAUD_115200 (UART_CLOCK / 1843200)      /**< UART baud rate: 115200. */
#define BAUD_57600 (UART_CLOCK / 921600)        /**< UART baud rate: 57600. */
#define BAUD_38400 (UART_CLOCK / 614400)        /**< UART baud rate: 38400. */
#define BAUD_19200 (UART_CLOCK / 307200)        /**< UART baud rate: 19200. */
#define BAUD_14400 (UART_CLOCK / 230400)        /**< UART baud rate: 14400. */
#define BAUD_9600 (UART_CLOCK / 153600)         /**< UART baud rate: 9600. */
#define BAUD_4800 (UART_CLOCK / 76800)          /**< UART baud rate: 4800. */
#define BAUD_2400 (UART_CLOCK / 38400)          /**< UART baud rate: 2400. */
#define BAUD_1200 (UART_CLOCK / 19200)          /**< UART baud rate: 1200. */

/**
 * @brief The definition of UART parity.
 */
#define PARITY_NONE 0       /**< Disable Parity */
#define PARITY_ODD 1        /**< Odd Parity */
#define PARITY_EVEN 2       /**< Even Parity */
#define PARITY_MARK 3       /**< Stick odd Parity */
#define PARITY_SPACE 4      /**< Stick even Parity */

typedef int32_t kdrv_uart_handle_t;

typedef void (*kdrv_uart_callback_t)(uint32_t event);

/**
 * @brief Enumerations of UART mode parameters.
 */
typedef enum
{
    UART_MODE_ASYN_RX = 0x1,    /**< Enum 0x1, UART asynchronous receiver mode. */
    UART_MODE_ASYN_TX = 0x2,    /**< Enum 0x2, UART asynchronous transmitter mode. */
    UART_MODE_SYNC_RX = 0x4,    /**< Enum 0x4, UART synchronous receiver mode. */
    UART_MODE_SYNC_TX = 0x8     /**< Enum 0x8,  UART synchronous transmitter mode. */
} kdrv_uart_mode_t;

/**
 * @brief Enumerations of UART device instance parameters.
 */
typedef enum
{
    UART0_DEV = 0,          /**< Enum 0, UART device instance 0 */
    UART1_DEV,              /**< Enum 1, UART device instance 1 */
    UART2_DEV,              /**< Enum 2, UART device instance 2 */
    UART3_DEV,              /**< Enum 3, UART device instance 3 */
    UART4_DEV,              /**< Enum 4, UART device instance 4 */
    TOTAL_UART_DEV          /**< Enum 5, Total UART device instances */
} kdrv_uart_dev_id_t;

/**
 * @brief Enumerations of UART port parameters.
 */
typedef enum
{
    DRVUART_PORT0 = 0,  /**< Enum 0, UART port 0 */
    DRVUART_PORT1 = 1,  /**< Enum 1, UART port 1 */
    DRVUART_PORT2 = 2,  /**< Enum 2, UART port 2 */
    DRVUART_PORT3 = 3,  /**< Enum 3, UART port 3 */
    DRVUART_PORT4 = 4   /**< Enum 4, UART port 4 */
} DRVUART_PORT;

/**
 * @brief Enumerations of UART control hardware signals
 */
typedef enum
{
    UART_CTRL_CONFIG,       /**< Enum 0, set @ref kdrv_uart_config_t */
    UART_CTRL_FIFO_RX,      /**< Enum 1, set @ref kdrv_uart_fifo_config_t */
    UART_CTRL_FIFO_TX,      /**< Enum 2, set @ref kdrv_uart_fifo_config_t */
    UART_CTRL_LOOPBACK,     /**< Enum 3, UART loopback enable */
    UART_CTRL_TX_EN,        /**< Enum 4, UART transmitter enable */
    UART_CTRL_RX_EN,        /**< Enum 5, UART receiver enable */
    UART_CTRL_ABORT_TX,     /**< Enum 6, UART abort transmitter */
    UART_CTRL_ABORT_RX,     /**< Enum 7, UART abort receiver */
    UART_CTRL_TIMEOUT_RX,   /**< Enum 8, UART receiver timeout value */
    UART_CTRL_TIMEOUT_TX    /**< Enum 9, UART transmitter timeout value */
} kdrv_uart_control_t;

/**
 * @brief The structure of UART configuration parameters.
 */
typedef struct
{
    uint32_t baudrate;      /**< UART baud rate. */
    uint8_t data_bits;      /**< UART data bits, a data character contains 5~8 data bits. */
    uint8_t frame_length;   /**< UART frame length, non-zero value for FIR mode*/
    uint8_t stop_bits;      /**< UART stop bit, a data character is proceded by a start bit \n
                                 and is followed by an optional parity bit and a stop bit. */
    uint8_t parity_mode;    /**< UART partity mode, see @ref UART_PARITY_DEF */
    bool fifo_en;           /**< UART fifo mode. */
} kdrv_uart_config_t;

/**
 * @brief The structure of UART FIFO configuration parameters.
 */
typedef struct
{
    bool bEnFifo;               /**< Is FIFO enabled */
    uint8_t fifo_trig_level;    /**< FIFO trigger level */
} kdrv_uart_fifo_config_t;


/**
 * @brief           UART driver initialization, it shall be called once in lifecycle
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_initialize(void);

/**
 * @brief           UART driver uninitialization
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_uninitialize(void);

/**
 * @brief           Open one UART port and acquire a uart port handle
 *
 * @details         This API will open a UART device (com_port: 0-5) for use.\n
 *                  It will return a UART device handle for future device reference.\n
 *                  The client can choose work mode: asynchronization or synchronization.\n
 *                  Synchronization mode will poll the hardware status to determine send/receiving point,\n
 *                  it will consume more power and introduce more delay to system execution.\n
 *                  But in the case of non-thread light weight environment, such as message log function, this mode is easy and suitable.\n
 *                  Asynchronization mode lets the driver interrupt driven, save more system power and more efficient,\n
 *                  the client needs to have a thread to listen/wait for the event/signal sent from callback function.\n
 *                  Callback function parameter 'callback' will be registered with this device which is mandatory for async mode,\n
 *                  will be invoked whenever Tx/Rx complete or timeout occur.\n
 *                  This callback function should be very thin, can only be used to set flag or send signals
 *
 * @param[out]      handle                a handle of an UART port
 * @param[in]       com_port              UART port id
 * @param[in]       mode                  bit combination of kdrv_uart_mode_t
 * @param[in]       callback              user callback function
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_open(kdrv_uart_handle_t *handle, uint8_t com_port, uint32_t mode, kdrv_uart_callback_t callback);

/**
 * @brief           set control for the opened UART port
 *
 * @param[in]       handle                device handle for an UART port
 * @param[in]       prop                  control enumeration
 * @param[in]       val                   pointer to control value/structure
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_configure(kdrv_uart_handle_t handle, kdrv_uart_control_t prop, uint8_t *val);

/**
 * @brief           write data to uart port, such as command, parameters, but not suitable for chunk data
 *
 * @details         The client calls this API to send data out to remote side.\n
 *                  Depending on the work mode, a little bit different behavior exists there.\n
 *                  In synchronous mode, the API call will not return until all data was sent out physically;\n
 *                  In asynchronous mode, the API call shall return immediately with UART_API_TX_BUSY.\n
 *                  When all the buffer data is sent out, the client registered callback function will be invoked.\n
 *                  The client shall have a very thin code there to set flags/signals. The client thread shall be listening the signal after this API call.\n
 *
 * @param[in]       handle                device handle for an UART port
 * @param[in]       buf                   data buffer
 * @param[in]       len                   data buffer length
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_write(kdrv_uart_handle_t hdl, uint8_t *buf, uint32_t len);

/**
 * @brief           read character data from UART port
 *
 * @param[in]       handle                device handle for an UART port
 * @param[out]      ch                    character data
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_get_char(kdrv_uart_handle_t handle, char *ch);

/**
 * @brief           read data from the UART port
 *
 * @details         The client can call this API to receive UART data from remote side.\n
 *                  Depending on the work mode, a little bit different behavior exists there.\n
 *                  In synchronous mode, the API call will not return until all data was received physically.\n
 *                  In asynchronous mode, the API call shall return immediately with UART_API_RX_BUSY.\n
 *                  When enough bytes are received or timeout occurs, the client registered callback function will be invoked.\n
 *                  The client shall have a very thin code there to set flags/signals. The client thread shall be listening the signal after this API call.\n
 *                  The client shall allocate the receiving buffer with max possible receiving length.\n
 *                  When one frame is sent out, after 4 chars transmission time, a timeout interrupt will be generated.
 *
 * @param[in]       handle                device handle for an UART port
 * @param[out]      buf                   data buffer
 * @param[in]       len                   data buffer length
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_read(kdrv_uart_handle_t handle, uint8_t *buf, uint32_t len);

/**
 * @brief           close the UART port
 *
 * @param[in]       handle                device handle for an UART port
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_close(kdrv_uart_handle_t handle);

/**
 * @brief           get char number in RX buffer 
 *
 * @param[in]       handle                device handle for an UART port
 * @return          number of RX count in the buffer
 */
uint32_t kdrv_uart_get_rx_count(kdrv_uart_handle_t handle);

/**
 * @brief           get char number in TX buffer 
 *
 * @param[in]       handle                device handle for an UART port
 * @return          number of TX count in the buffer
 */
uint32_t kdrv_uart_get_tx_count(kdrv_uart_handle_t handle);

/**
 * @brief           uart debug console port init
 *
 * @param[in]       uart_dev              uart device id, @ref kdrv_uart_dev_id_t
 * @param[in]       baudrate              uart baud rate
 * @return          uart initialize status, see @ref kdrv_status_t
 */
kdrv_status_t kdrv_uart_console_init(uint8_t uart_dev, uint32_t baudrate);

#endif //__KDRV_UART_H_
