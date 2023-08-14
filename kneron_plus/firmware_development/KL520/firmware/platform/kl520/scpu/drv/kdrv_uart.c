/**
 * Kneron Peripheral API - UART
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

//#define UART_DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//#include "kmdw_console.h"
#include "kdrv_uart.h"
#include "kdrv_scu_ext.h"
#include "kdrv_clock.h" // for delay_us()

static kdrv_uart_handle_t handle0 = 0;

#define SERIAL_THR 0x00 /*  Transmitter Holding Register(Write).*/
#define SERIAL_RBR 0x00 /*  Receive Buffer register (Read).*/
#define SERIAL_IER 0x04 /*  Interrupt Enable register.*/
#define SERIAL_IIR 0x08 /*  Interrupt Identification register(Read).*/
#define SERIAL_FCR 0x08 /*  FIFO control register(Write).*/
#define SERIAL_LCR 0x0C /*  Line Control register.*/
#define SERIAL_MCR 0x10 /*  Modem Control Register.*/
#define SERIAL_LSR 0x14 /*  Line status register(Read) .*/
#define SERIAL_MSR 0x18 /*  Modem Status register (Read).*/
#define SERIAL_SPR 0x1C /*  Scratch pad register */
#define SERIAL_DLL 0x0  /*  Divisor Register LSB */
#define SERIAL_DLM 0x4  /*  Divisor Register MSB */
#define SERIAL_PSR 0x8  /* Prescale Divison Factor */

#define SERIAL_MDR 0x20
#define SERIAL_ACR 0x24
#define SERIAL_TXLENL 0x28
#define SERIAL_TXLENH 0x2C
#define SERIAL_MRXLENL 0x30
#define SERIAL_MRXLENH 0x34
#define SERIAL_PLR 0x38
#define SERIAL_FMIIR_PIO 0x3C
#define SERIAL_FEATURE 0x68

/* IER Register */
#define SERIAL_IER_DR 0x1  /* Data ready Enable */
#define SERIAL_IER_TE 0x2  /* THR Empty Enable */
#define SERIAL_IER_RLS 0x4 /* Receive Line Status Enable */
#define SERIAL_IER_MS 0x8  /* Modem Staus Enable */

/* IIR Register */
#define SERIAL_IIR_NONE 0x1    /* No interrupt pending */
#define SERIAL_IIR_RLS 0x6     /* Receive Line Status */
#define SERIAL_IIR_DR 0x4      /* Receive Data Ready */
#define SERIAL_IIR_TIMEOUT 0xc /* Receive Time Out */
#define SERIAL_IIR_TE 0x2      /* THR Empty */
#define SERIAL_IIR_MODEM 0x0   /* Modem Status */

/* FCR Register */
#define SERIAL_FCR_FE 0x1   /* FIFO Enable */
#define SERIAL_FCR_RXFR 0x2 /* Rx FIFO Reset */
#define SERIAL_FCR_TXFR 0x4 /* Tx FIFO Reset */

/* LCR Register */
#define SERIAL_LCR_LEN5 0x0
#define SERIAL_LCR_LEN6 0x1
#define SERIAL_LCR_LEN7 0x2
#define SERIAL_LCR_LEN8 0x3

#define SERIAL_LCR_STOP 0x4
#define SERIAL_LCR_EVEN 0x18        /* Even Parity */
#define SERIAL_LCR_ODD 0x8          /* Odd Parity */
#define SERIAL_LCR_PE 0x8           /* Parity Enable */
#define SERIAL_LCR_SETBREAK 0x40    /* Set Break condition */
#define SERIAL_LCR_STICKPARITY 0x20 /* Stick Parity Enable */
#define SERIAL_LCR_DLAB 0x80        /* Divisor Latch Access Bit */

/* LSR Register */
#define SERIAL_LSR_DR 0x1    /* Data Ready */
#define SERIAL_LSR_OE 0x2    /* Overrun Error */
#define SERIAL_LSR_PE 0x4    /* Parity Error */
#define SERIAL_LSR_FE 0x8    /* Framing Error */
#define SERIAL_LSR_BI 0x10   /* Break Interrupt */
#define SERIAL_LSR_THRE 0x20 /* THR Empty */
#define SERIAL_LSR_TE 0x40   /* Transmitte Empty */
#define SERIAL_LSR_DE 0x80   /* FIFO Data Error */

/* MCR Register */
#define SERIAL_MCR_DTR 0x1   /* Data Terminal Ready */
#define SERIAL_MCR_RTS 0x2   /* Request to Send */
#define SERIAL_MCR_OUT1 0x4  /* output    1 */
#define SERIAL_MCR_OUT2 0x8  /* output2 or global interrupt enable */
#define SERIAL_MCR_LPBK 0x10 /* loopback mode */

/* MSR Register */
#define SERIAL_MSR_DELTACTS 0x1 /* Delta CTS */
#define SERIAL_MSR_DELTADSR 0x2 /* Delta DSR */
#define SERIAL_MSR_TERI 0x4     /* Trailing Edge RI */
#define SERIAL_MSR_DELTACD 0x8  /* Delta CD */
#define SERIAL_MSR_CTS 0x10     /* Clear To Send */
#define SERIAL_MSR_DSR 0x20     /* Data Set Ready */
#define SERIAL_MSR_RI 0x40      /* Ring Indicator */
#define SERIAL_MSR_DCD 0x80     /* Data Carrier Detect */

/* MDR register */
#define SERIAL_MDR_MODE_SEL 0x03
#define SERIAL_MDR_UART 0x0
#define SERIAL_MDR_SIR 0x1
#define SERIAL_MDR_FIR 0x2

/* ACR register */
#define SERIAL_ACR_TXENABLE 0x1
#define SERIAL_ACR_RXENABLE 0x2
#define SERIAL_ACR_SET_EOT 0x4

#define IIR_CODE_MASK 0xf
#define SERIAL_IIR_TX_FIFO_FULL 0x10

#define UART_INITIALIZED (1 << 0)
#define UART_BASIC_CONFIGURED (1 << 2)
#define UART_FIFO_RX_CONFIGURED (1 << 3)
#define UART_FIFO_TX_CONFIGURED (1 << 4)
#define UART_TX_ENABLED (1 << 5)
#define UART_RX_ENABLED (1 << 6)
#define UART_LOOPBACK_ENABLED (1 << 7)

#define SERIAL_FIFO_DEPTH_REG 0x68

#define SERIAL_FIFO_DEPTH_16B 0x1
#define SERIAL_FIFO_DEPTH_32B 0x2
#define SERIAL_FIFO_DEPTH_64B 0x4
#define SERIAL_FIFO_DEPTH_128B 0x8

#define SERIAL_FIFO_TRIG_LVEL_1 0x0
#define SERIAL_FIFO_TRIG_LVEL_4 0x1
#define SERIAL_FIFO_TRIG_LVEL_8 0x2
#define SERIAL_FIFO_TRIG_LVEL_14 0x3

#define DEFAULT_SYNC_TIMEOUT_CHARS_TIME 0 //default is no timeout

#define MAX_UART_INST 5 //max uart instance

typedef void (*uart_isr_t)(void);

typedef struct
{
    volatile uint8_t tx_busy; // Transmitter busy flag
    volatile uint8_t rx_busy; // Receiver busy flag
    uint8_t tx_underflow;     // Transmit data underflow detected (cleared on start of next send operation)
    uint8_t rx_overflow;      // Receive data overflow detected (cleared on start of next receive operation)
    uint8_t rx_break;         // Break detected on receive (cleared on start of next receive operation)
    uint8_t rx_framing_error; // Framing error detected on receive (cleared on start of next receive operation)
    uint8_t rx_parity_error;  // Parity error detected on receive (cleared on start of next receive operation)
} UART_STATUS_t;

// UART Transfer Information (Run-Time)
typedef struct
{
    volatile uint32_t rx_num; // Total number of data to be received
    volatile uint32_t tx_num; // Total number of data to be send
    volatile uint8_t *rx_buf; // Pointer to in data buffer
    volatile uint8_t *tx_buf; // Pointer to out data buffer
    volatile uint32_t rx_cnt; // Number of data received
    volatile uint32_t tx_cnt; // Number of data sent
} UART_TRANSFER_INFO_t;

// UART Information (Run-Time)
typedef struct
{
    ARM_USART_SignalEvent_t cb_event; // Event callback
    UART_STATUS_t status;             // Status flags
    UART_TRANSFER_INFO_t xfer;        // Transfer information
    uint32_t flags;                   // UART driver flags: UART_FLAG_T
    uint32_t mode;                    // UART mode
} UART_INFO_T;

typedef enum
{
    UART_UNINIT,
    UART_INIT_DONE,
    UART_WORKING,
    UART_CLOSED
} uart_state_t;

// UART Resources definitions
typedef struct
{
    uint32_t irq_num;           // UART TX IRQ Number
    uart_isr_t isr;             // ISR route
    uint32_t fifo_depth;        //16/32/64/128 depth, set by UART_CTRL_CONFIG
    uint32_t tx_fifo_threshold; // FIFO tx trigger threshold
    uint32_t rx_fifo_threshold; // FIFO rx trigger threshold
    uint32_t fifo_len;          // FIFO tx buffer len
    uint32_t clock;             //clock
    uint32_t hw_base;           // hardware base address
} UART_RESOURCES_T;

/* driver instance handle */
typedef struct
{
    uint32_t uart_port;
    uart_state_t state;
    kdrv_uart_config_t config;
    UART_INFO_T info;
    UART_RESOURCES_T res;
    int32_t nTimeOutTx; //Tx timeout (ms) for UART_SYNC_MODE
    int32_t nTimeOutRx; //Rx timeout (ms) for UART_SYNC_MODE
    uint32_t iir;       //store IIR register value (IIR register will be reset once it is
                        //read once, need to store for further process)
} uart_driver_handle_t;

typedef struct
{
    int8_t total_open_uarts;
    bool active_dev[TOTAL_UART_DEV];
    uart_driver_handle_t *uart_dev[MAX_UART_INST];

} uart_drv_ctx_t;

/***********************************************************************************
              Global variables
************************************************************************************/
extern uint32_t UART_PORT[5];
void UART0_ISR(void);
void UART1_ISR(void);
void UART2_ISR(void);
void UART3_ISR(void);
void UART4_ISR(void);

/***********************************************************************************
              local variables
************************************************************************************/
static uart_drv_ctx_t gDrvCtx;

IRQn_Type gUartIRQTbl[5] = {
    UART_FTUART010_0_IRQ,   //UART0
    UART_FTUART010_1_IRQ,   //UART1
    UART_FTUART010_1_1_IRQ, //UART2
    UART_FTUART010_1_2_IRQ, //UART3
    UART_FTUART010_1_3_IRQ  //UART4
};

uart_isr_t gUartISRs[5] = {
    UART0_ISR,
    UART1_ISR,
    UART2_ISR,
    UART3_ISR,
    UART4_ISR};

uint32_t gUartClk[5] = {
    UART_CLOCK,
    UART_CLOCK_2,
    UART_CLOCK_2,
    UART_CLOCK,
    UART_CLOCK};

static bool gDriverInitialized = false;

/***********************************************************************************
              local functions
************************************************************************************/

static uint32_t UART_PORT[5] = {UART_FTUART010_0_PA_BASE, UART_FTUART010_1_PA_BASE,
                                UART_FTUART010_1_1_PA_BASE, UART_FTUART010_1_2_PA_BASE, UART_FTUART010_1_3_PA_BASE};
uint32_t uart_baud_rate_map[11] = {BAUD_1200,BAUD_2400,BAUD_4800,BAUD_9600,BAUD_14400 ,BAUD_19200 ,BAUD_38400 ,BAUD_57600 ,BAUD_115200,BAUD_460800,BAUD_921600};
static void uart_serial_init(DRVUART_PORT port_no, uint32_t baudrate, uint32_t parity, uint32_t num, uint32_t len, uint32_t interruptMode)
{
    uint32_t lcr;

    lcr = inw(UART_PORT[port_no] + SERIAL_LCR) & ~SERIAL_LCR_DLAB;
    /* Set DLAB=1 */
    outw(UART_PORT[port_no] + SERIAL_LCR, SERIAL_LCR_DLAB);
    /* Set baud rate */
    outw(UART_PORT[port_no] + SERIAL_DLM, ((baudrate & 0xff00) >> 8)); //ycmo090930
    outw(UART_PORT[port_no] + SERIAL_DLL, (baudrate & 0xff));
    lcr &= 0xc0;

    switch (parity)
    {
    case PARITY_NONE:
        //do nothing
        break;
    case PARITY_ODD:
        lcr |= SERIAL_LCR_ODD;
        break;
    case PARITY_EVEN:
        lcr |= SERIAL_LCR_EVEN;
        break;
    case PARITY_MARK:
        lcr |= (SERIAL_LCR_STICKPARITY | SERIAL_LCR_ODD);
        break;
    case PARITY_SPACE:
        lcr |= (SERIAL_LCR_STICKPARITY | SERIAL_LCR_EVEN);
        break;

    default:
        break;
    }

    if (num == 2)
        lcr |= SERIAL_LCR_STOP;

    len -= 5;

    lcr |= len;

    outw(UART_PORT[port_no] + SERIAL_LCR, lcr);
    if (1 == interruptMode)
        outw(UART_PORT[port_no] + SERIAL_FCR, SERIAL_FCR_FE);
}

static char uart_get_serial_char(DRVUART_PORT port_no)
{
    char Ch;
    uint32_t status;

    do
    {
        status = inw(UART_PORT[port_no] + SERIAL_LSR);
    } while (!((status & SERIAL_LSR_DR) == SERIAL_LSR_DR)); // wait until Rx ready
    Ch = inw(UART_PORT[port_no] + SERIAL_RBR);
    return (Ch);
}

static uint32_t uart_get_status(DRVUART_PORT port_no)
{
    uint32_t status;
    status = inw(UART_PORT[port_no] + SERIAL_LSR);
    return status;
}

static void CheckTxStatus(DRVUART_PORT port_no)
{
    uint32_t status;
    do
    {
        status = uart_get_status(port_no);
    } while (!((status & SERIAL_LSR_THRE) == SERIAL_LSR_THRE)); // wait until Tx ready
}

static void CheckRxStatus(DRVUART_PORT port_no)
{
    uint32_t status;
    do
    {
        status = uart_get_status(port_no);
    } while (!((status & SERIAL_IER_DR) == SERIAL_IER_DR)); // wait until Rx ready
}

#if 0
static int32_t kdp_uart_get_default_timeout(uint32_t baud)
{
    int32_t timeout;
    switch (baud)
    {
    case BAUD_921600:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 9216;
        break;
    }
    case BAUD_460800:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 4608;
        break;
    }
    case BAUD_115200:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 1152;
        break;
    }
    case BAUD_57600:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 576;
        break;
    }
    case BAUD_38400:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 384;
        break;
    }
    case BAUD_19200:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 192;
        break;
    }
    case BAUD_14400:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 144;
        break;
    }
    case BAUD_9600:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 96;
        break;
    }
    case BAUD_4800:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 48;
        break;
    }
    case BAUD_2400:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 240;
        break;
    }
    case BAUD_1200:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 12;
        break;
    }
    default:
        timeout = DEFAULT_SYNC_TIMEOUT_CHARS_TIME;
    }
    return timeout;
}
#endif

static uart_driver_handle_t *uart_get_drv_hdl(uint16_t port)
{
#ifdef UART_DEBUG
    if (port >= MAX_UART_INST)
    {

        dbg_msg("Error: invalid port number\n");

        return NULL;
    }
#endif
    return gDrvCtx.uart_dev[port];
}

/* calculate fifo config: depth and trigger level
   pCfg->bEnFifo and pCfg->fifo_depth was set by client before calling in
*/
static kdrv_status_t kdp_calculate_fifo_cfg(uart_driver_handle_t *const pDrv, uint32_t val, kdrv_uart_fifo_config_t *pCfg)
{
    int32_t depth;

    if (pCfg->bEnFifo == false)
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_1;
        return KDRV_STATUS_OK;
    }

    depth = pDrv->res.fifo_depth;

    if (val <= depth * 4)
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_1;
    }
    else if ((val > depth * 4) && (val <= depth * 8))
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_4;
    }
    else if ((val > depth * 8) && (val <= depth * 14))
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_8;
    }
    else if (val > depth * 14)
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_14;
    }
    return KDRV_STATUS_OK;
}

/**
  \fn          void UART_IRQHandler (UART_RESOURCES  *uart)
  \brief       UART Interrupt handler.
  \param[in]   uart  Pointer to UART resources
*/
static void UART_TX_ISR(uart_driver_handle_t *const uart)
{
    int16_t tx;
    uint32_t status;
    uint32_t ww;
    uint16_t port_no;
    bool bFifo;

    ww = uart->iir;
    bFifo = ((ww & 0xc0) != 0) ? true : false;
    port_no = uart->uart_port;

    if (uart->info.xfer.tx_num > uart->info.xfer.tx_cnt)
    {
        if (bFifo == false) //NO FIFO
        {
            do
            {
                status = inw(UART_PORT[port_no] + SERIAL_LSR);
            } while (!((status & SERIAL_LSR_THRE) == SERIAL_LSR_THRE)); // wait until Tx ready
            outw(UART_PORT[port_no] + SERIAL_THR, *(uart->info.xfer.tx_buf++));
            uart->info.xfer.tx_cnt++;
        }
        else //FIFO mode
        {
            do
            {
                status = inw(UART_PORT[port_no] + SERIAL_LSR);
            } while (!((status & SERIAL_LSR_THRE) == SERIAL_LSR_THRE)); // wait until Tx ready

            tx = uart->info.xfer.tx_num - uart->info.xfer.tx_cnt;
            if (tx <= 0)
                tx = 0;

            if (tx > uart->res.fifo_len)
            {
                tx = 16;
            }

            while (tx > 0)
            {
                uart->info.xfer.tx_cnt++;
                outw(UART_PORT[port_no] + SERIAL_THR, *(uart->info.xfer.tx_buf++));
                tx--;
            }
        }
    }

    if (uart->info.xfer.tx_num == uart->info.xfer.tx_cnt)
    {
        // need to add: determine if TX fifo is empty
        status = inw(UART_PORT[port_no] + SERIAL_LSR);
        if ((status & SERIAL_LSR_THRE) == SERIAL_LSR_THRE) //TX empty to make sure FIFO data to tranmit shift
        {
            uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
            ww &= ~SERIAL_IER_TE; // disable Tx empty interrupt
            outw(UART_PORT[port_no] + SERIAL_IER, ww);

            // Clear TX busy flag
            uart->info.status.tx_busy = 0;
            if (uart->info.cb_event)
                uart->info.cb_event(ARM_USART_EVENT_SEND_COMPLETE);
        }
    }
}

static void UART_RX_ISR(uart_driver_handle_t *const uart)
{
    uint32_t ww;
    uint16_t port_no;

    ww = uart->iir;

    port_no = uart->uart_port;

    if (((ww & SERIAL_IIR_DR) == SERIAL_IIR_DR) || ((ww & SERIAL_IIR_TIMEOUT) == SERIAL_IIR_TIMEOUT))
    {

        if ((ww & 0xc0) == 0) //non FIFO mode
        {
            *(uart->info.xfer.rx_buf++) = (uint8_t)uart_get_serial_char((DRVUART_PORT)port_no);
            uart->info.xfer.rx_cnt++;

            if (uart->info.xfer.rx_cnt == uart->info.xfer.rx_num)
            {
                ww = inw(UART_PORT[port_no] + SERIAL_IER);
                ww &= ~SERIAL_IER_DR; // disable Rx empty interrupt
                outw(UART_PORT[port_no] + SERIAL_IER, ww);

                // Clear RX busy flag and set receive transfer complete event
                uart->info.status.rx_busy = 0;
                if (uart->info.cb_event)
                    uart->info.cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
            }
        }
        else //FIFO mode
        {
            while (uart->info.xfer.rx_cnt < uart->info.xfer.rx_num)
            {
                /* Read the data from FIFO buffer */

                ww = inw(UART_PORT[port_no] + SERIAL_LSR);
                if ((ww & SERIAL_LSR_DR) == SERIAL_LSR_DR)
                {
                    *(uart->info.xfer.rx_buf++) = (uint8_t)inw(UART_PORT[port_no] + SERIAL_RBR);
                    uart->info.xfer.rx_cnt++;

                    if (uart->info.xfer.rx_cnt == uart->info.xfer.rx_num)
                    {
                        ww = inw(UART_PORT[port_no] + SERIAL_IER);
                        ww &= ~SERIAL_IER_DR; // disable Rx empty interrupt
                        outw(UART_PORT[port_no] + SERIAL_IER, ww);

                        // Clear RX busy flag and set receive transfer complete event
                        uart->info.status.rx_busy = 0;
                        if (uart->info.cb_event)
                            uart->info.cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
                    }
                }
                else
                {
                    ww = uart->iir;
                    if ((ww & SERIAL_IIR_TIMEOUT) == SERIAL_IIR_TIMEOUT) // means end of frame
                    {
                        ww = inw(UART_PORT[port_no] + SERIAL_IER);
                        ww &= ~SERIAL_IER_DR; // disable Rx empty interrupt
                        outw(UART_PORT[port_no] + SERIAL_IER, ww);

                        // Clear RX busy flag and set receive transfer complete event
                        uart->info.status.rx_busy = 0;
                        if (uart->info.cb_event)
                            uart->info.cb_event(ARM_USART_EVENT_RX_TIMEOUT);
                    }
                    break;
                }
            }
        }
    }
}

/***********************************************************************************
              global functions
************************************************************************************/

void UART_ISR(uint8_t port_no)
{
    uart_driver_handle_t *pHdl = uart_get_drv_hdl(port_no);

    uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IIR);

    pHdl->iir = ww;

    if ((ww & IIR_CODE_MASK) == SERIAL_IIR_RLS) // errors: overrun/parity/framing/break
    {
        ww = inw(UART_PORT[port_no] + SERIAL_LSR); //Read LSR to reset interrupt

        if (ww & SERIAL_LSR_OE)
        {
            pHdl->info.status.rx_overflow = 1;
        }

        if (ww & SERIAL_LSR_PE)
        {
            pHdl->info.status.rx_parity_error = 1;
        }

        if (ww & SERIAL_LSR_BI)
        {
            pHdl->info.status.rx_break = 1;
        }

        if (ww & SERIAL_LSR_FE)
        {
            pHdl->info.status.rx_framing_error = 1;
        }

        if (pHdl->info.status.rx_busy)
        {
            UART_RX_ISR(pHdl);
        }

        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }
    }
    else if (((ww & IIR_CODE_MASK) == SERIAL_IIR_DR)          // Rx data ready in FIFO
             || ((ww & IIR_CODE_MASK) == SERIAL_IIR_TIMEOUT)) // Character Reception Timeout
    {
        if (pHdl->info.status.rx_busy)
        {
            UART_RX_ISR(pHdl);
        }
        else
        {
            ww = inw(UART_PORT[port_no] + SERIAL_RBR); //Read RBR to reset interrupt
        }

        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }
    }
    else if ((ww & IIR_CODE_MASK) == SERIAL_IIR_TE) // Transmitter Holding Register Empty
    {
        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }

        if (pHdl->info.status.rx_busy)
        {
            UART_RX_ISR(pHdl);
        }
    }
    else
    {
        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }
    }

    NVIC_ClearPendingIRQ((IRQn_Type)pHdl->res.irq_num);
    NVIC_EnableIRQ((IRQn_Type)pHdl->res.irq_num);
}

void UART0_ISR(void)
{
    UART_ISR(0);
}

void UART1_ISR(void)
{
    UART_ISR(1);
}

void UART2_ISR(void)
{
    UART_ISR(2);
}

void UART3_ISR(void)
{
    UART_ISR(3);
}

void UART4_ISR(void)
{
    UART_ISR(4);
}

/* Init the UART device driver, it shall be called once in lifecycle
*/
kdrv_status_t kdrv_uart_initialize(void)
{
    if (gDriverInitialized == false)
    {
        memset(&gDrvCtx, 0, sizeof(gDrvCtx));
        gDriverInitialized = true;
    }

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_uart_console_init(uint8_t uart_dev, uint32_t baudrate)
{
    kdrv_uart_handle_t handle;
    kdrv_status_t sts = kdrv_uart_open(&handle, uart_dev,
                                       UART_MODE_SYNC_RX | UART_MODE_SYNC_TX, NULL);
    if (sts != KDRV_STATUS_OK)
        return sts;

    kdrv_uart_config_t cfg;
    cfg.baudrate = uart_baud_rate_map[baudrate];
    cfg.data_bits = 8;
    cfg.frame_length = 0;
    cfg.stop_bits = 1;
    cfg.parity_mode = PARITY_NONE;
    cfg.fifo_en = false;
#ifdef UART_RX_IRQ
    cfg.irq_en = false;
#endif

    sts = kdrv_uart_configure(handle, UART_CTRL_CONFIG, (void *)&cfg);
    if (sts != KDRV_STATUS_OK)
        return sts;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_uart_uninitialize(void)
{
    return KDRV_STATUS_OK;
}

/*
  Open one UART port
Input:
  com_port: UART port id
  mode: that is a combination of Tx mode|Rx mode, make sure set both, such as UART_MODE_ASYN_RX|UART_MODE_SYNC_TX
  cb: callback function

Output:
  return device handle: >=0 means success; -1 means open fail
*/
kdrv_status_t kdrv_uart_open(kdrv_uart_handle_t *handle, uint8_t com_port, uint32_t mode, kdrv_uart_callback_t cb)
{
    uint32_t ww;

#ifdef UART_DEBUG
    if (com_port >= TOTAL_UART_DEV)
    {
        dbg_msg("Invalid com_port\n");
        return KDRV_STATUS_ERROR;
    }
#endif

    if (gDrvCtx.active_dev[com_port] == true)
    {
#ifdef UART_DEBUG
        dbg_msg("This UART device has been opened\n");
#endif
        return KDRV_STATUS_ERROR;
    }

    uart_driver_handle_t *pDrv = (uart_driver_handle_t *)malloc(sizeof(uart_driver_handle_t));
    if (pDrv == NULL)
    {
#ifdef UART_DEBUG
        err_msg("Error: memory alloc failed\n");
#endif
        return KDRV_STATUS_ERROR;
    }

    if (((mode & UART_MODE_ASYN_TX) || (mode & UART_MODE_ASYN_RX)) && (cb == NULL))
    {
#ifdef UART_DEBUG
        err_msg("Error: Async mode needs callback function\n");
#endif
        return KDRV_STATUS_ERROR;
    }

    gDrvCtx.uart_dev[com_port] = pDrv;

    pDrv->uart_port = com_port;
    pDrv->state = UART_UNINIT;
    // now use a common capbilities for all UARTs, may introduce an array if diff UART has diff capbilities
    pDrv->info.cb_event = cb;
    pDrv->info.mode = mode;
    pDrv->info.status.tx_busy = 0;
    pDrv->info.status.rx_busy = 0;
    pDrv->info.status.tx_underflow = 0;
    pDrv->info.status.rx_overflow = 0;
    pDrv->info.status.rx_break = 0;
    pDrv->info.status.rx_framing_error = 0;
    pDrv->info.status.rx_parity_error = 0;

    pDrv->info.xfer.tx_num = 0;
    pDrv->info.xfer.rx_num = 0;
    pDrv->info.xfer.tx_cnt = 0;
    pDrv->info.xfer.rx_cnt = 0;

    pDrv->res.irq_num = gUartIRQTbl[com_port];
    pDrv->res.isr = gUartISRs[com_port];

    ww = inw(UART_PORT[com_port] + SERIAL_FIFO_DEPTH_REG);
    ww &= 0x0f;
    pDrv->res.fifo_depth = ww;

    pDrv->res.fifo_len = 16 * ww;

    pDrv->res.hw_base = UART_PORT[com_port];
    pDrv->res.clock = gUartClk[com_port];

    pDrv->nTimeOutRx = DEFAULT_SYNC_TIMEOUT_CHARS_TIME; //suppose default baud = 115200
    pDrv->nTimeOutTx = DEFAULT_SYNC_TIMEOUT_CHARS_TIME; //suppose default baud = 115200

    gDrvCtx.total_open_uarts++;
    gDrvCtx.active_dev[com_port] = true;

    pDrv->info.flags |= UART_INITIALIZED;

    // enable clocks / power / irq
    {
        uint32_t clock_enable_reg2 = inw(SCU_EXTREG_PA_BASE + 0x1c);

        if (com_port == UART0_DEV)
            clock_enable_reg2 |= (0x1 << 8);
        else
            clock_enable_reg2 |= (0x1 << (11 + com_port));

        outw(SCU_EXTREG_PA_BASE + 0x1c, clock_enable_reg2);

        kdrv_uart_fifo_config_t cfg;

        /* config FIFO trigger value with default value or client set via control API*/
        cfg.fifo_trig_level = pDrv->res.rx_fifo_threshold;
        cfg.bEnFifo = pDrv->config.fifo_en;
        kdrv_uart_configure(com_port, UART_CTRL_FIFO_RX, (void *)&cfg);

        cfg.fifo_trig_level = pDrv->res.tx_fifo_threshold;
        cfg.bEnFifo = (cfg.fifo_trig_level == 0) ? false : true;
        kdrv_uart_configure(com_port, UART_CTRL_FIFO_TX, (void *)&cfg);

        NVIC_ClearPendingIRQ((IRQn_Type)pDrv->res.irq_num);
    }

    *handle = (kdrv_uart_handle_t)com_port;

    return KDRV_STATUS_OK;
}

/* close the device
Input:
   handle: device handle
return:
   0 - success; -1 - failure
*/
kdrv_status_t kdrv_uart_close(kdrv_uart_handle_t handle)
{
    uint32_t com_port = handle;

#ifdef UART_DEBUG
    if (com_port >= TOTAL_UART_DEV)
    {
        dbg_msg("Invalid parameter\n");
        return KDRV_STATUS_ERROR;
    }

    if (gDrvCtx.active_dev[com_port] == false)
    {
        dbg_msg("This UART device has been closed\n");
        return KDRV_STATUS_ERROR;
    }
#endif

    uart_driver_handle_t *pDrv = gDrvCtx.uart_dev[com_port];

    if (pDrv->info.flags == 0)
    {
        // Driver not initialized
        return KDRV_STATUS_ERROR;
    }

    // Reset USART status flags
    pDrv->info.flags = 0;

    free(gDrvCtx.uart_dev[com_port]);
    gDrvCtx.uart_dev[com_port] = NULL;
    gDrvCtx.active_dev[com_port] = false;
    gDrvCtx.total_open_uarts--;

    return KDRV_STATUS_OK;
}

/*
Set control for the device
Input:
    handle: device handle
    prop: control enumeration
    pVal: pointer to control value/structure
return:
    None
*/
kdrv_status_t kdrv_uart_configure(kdrv_uart_handle_t handle, kdrv_uart_control_t prop, uint8_t *pVal)
{
    uint32_t com_port = handle;

#ifdef UART_DEBUG
    if (com_port >= TOTAL_UART_DEV)
    {
        dbg_msg("Invalid parameter\n");
        return KDRV_STATUS_ERROR;
    }

    if (gDrvCtx.active_dev[com_port] == false)
    {
        dbg_msg("This UART device has been closed\n");
        return KDRV_STATUS_ERROR;
    }
#endif

    uart_driver_handle_t *pDrv = gDrvCtx.uart_dev[com_port];

#ifdef UART_DEBUG
    if ((pDrv->info.flags & UART_INITIALIZED) == 0)
    {
        // Return error, if USART is not initialized
        return KDRV_STATUS_ERROR;
    }
#endif

    DRVUART_PORT port_no = (DRVUART_PORT)pDrv->uart_port;
    switch (prop)
    {
    case UART_CTRL_CONFIG:
    {
        kdrv_uart_config_t cfg = *(kdrv_uart_config_t *)pVal;
        uint32_t baudrate = cfg.baudrate;
        uint32_t parity = cfg.parity_mode;
        uint32_t num = cfg.stop_bits;
        uint32_t len = cfg.data_bits;

        uart_serial_init(port_no, baudrate, parity, num, len, 0);

        pDrv->config.baudrate = baudrate;
        pDrv->config.data_bits = cfg.data_bits;
        pDrv->config.stop_bits = num;
        pDrv->config.parity_mode = parity;
        pDrv->config.fifo_en = cfg.fifo_en;

        pDrv->res.tx_fifo_threshold = 0;
        pDrv->res.rx_fifo_threshold = 0;

        pDrv->nTimeOutTx = 0; //kdp_uart_get_default_timeout(baudrate);
        pDrv->nTimeOutRx = 0; //kdp_uart_get_default_timeout(baudrate);

        pDrv->info.flags |= UART_BASIC_CONFIGURED;

        pDrv->state = UART_INIT_DONE;
        break;
    }

    case UART_CTRL_TX_EN:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_IER_TE; // disable TX
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags &= ~UART_TX_ENABLED;
        }
        else
        {
            ww |= SERIAL_IER_TE; // enable TX
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags |= UART_TX_ENABLED;
        }

        break;
    }

    case UART_CTRL_RX_EN:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_IER_DR; // disable Rx
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags &= ~UART_RX_ENABLED;
        }
        else
        {
            ww |= SERIAL_IER_DR; // enable Rx
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags |= UART_RX_ENABLED;
        }

        break;
    }

    case UART_CTRL_ABORT_TX:
    {
        /* disable Tx interrupt */
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        ww &= ~SERIAL_IER_TE;
        outw(UART_PORT[port_no] + SERIAL_IER, ww);

        /* reset Tx FIFO */
        ww = inw(UART_PORT[port_no] + SERIAL_FCR);
        ww |= (SERIAL_FCR_TXFR | SERIAL_FCR_FE);
        outw(UART_PORT[port_no] + SERIAL_FCR, ww);

        pDrv->info.status.tx_busy = 0;

        break;
    }

    case UART_CTRL_ABORT_RX:
    {
        /* disable Rx interrupt */
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        ww &= ~SERIAL_IER_DR;
        outw(UART_PORT[port_no] + SERIAL_IER, ww);

        /* reset Rx FIFO */
        ww = inw(UART_PORT[port_no] + SERIAL_FCR);
        ww |= (SERIAL_FCR_RXFR | SERIAL_FCR_FE);
        outw(UART_PORT[port_no] + SERIAL_FCR, ww);

        pDrv->info.status.rx_busy = 0;

        break;
    }

    case UART_CTRL_FIFO_RX:
    {
        uint32_t ww;
        kdrv_uart_fifo_config_t *pCfg = (kdrv_uart_fifo_config_t *)pVal;
        uint8_t trig_lvl = 0x3 & pCfg->fifo_trig_level;

        pDrv->res.rx_fifo_threshold = trig_lvl;

        if (pCfg->bEnFifo)
        {
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 6);
            ww |= (trig_lvl << 6) | SERIAL_FCR_RXFR | SERIAL_FCR_FE; // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }
        else
        {
            /*
              becasue RX/TX FIFO can only be enable/disabled simutanously,
              cannot be set individually, so set trigger val to 0 for FIFO disable
            */
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 6);
            ww |= (0 << 6) | SERIAL_FCR_RXFR | SERIAL_FCR_FE; // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }

        pDrv->info.flags |= UART_FIFO_RX_CONFIGURED;

        break;
    }

    case UART_CTRL_FIFO_TX:
    {
        uint32_t ww;
        kdrv_uart_fifo_config_t *pCfg = (kdrv_uart_fifo_config_t *)pVal;
        uint8_t trig_lvl = 0x3 & pCfg->fifo_trig_level;

        pDrv->res.tx_fifo_threshold = trig_lvl;

        if (pCfg->bEnFifo)
        {
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 4);
            ww |= (trig_lvl << 4) | SERIAL_FCR_TXFR | SERIAL_FCR_FE; // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }
        else
        {
            /*
              becasue RX/TX FIFO can only be enable/disabled simutanously,
              cannot be set individually, so set trigger val to 0 for FIFO disable
            */
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 4);
            ww |= (0 << 4) | SERIAL_FCR_TXFR | SERIAL_FCR_FE; // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }

        pDrv->info.flags |= UART_FIFO_TX_CONFIGURED;

        break;
    }

    case UART_CTRL_LOOPBACK:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_MCR);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_MCR_LPBK; // disable loopback
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }
        else
        {
            ww |= SERIAL_MCR_LPBK; // enable loopback
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }

        pDrv->info.flags |= UART_LOOPBACK_ENABLED;

        break;
    }
    case UART_CTRL_TIMEOUT_RX:
    {
        pDrv->nTimeOutRx = (int32_t)*pVal;

        break;
    }

    case UART_CTRL_TIMEOUT_TX:
    {
        pDrv->nTimeOutTx = (int32_t)*pVal;

        break;
    }

    default:
        break;
    }
    return KDRV_STATUS_OK;
}

/*
Write data to KL520 device, such as command, parameters, but not suitable for chunk data
Input:
    hdl: device handle
    buf: data buffer
    len: data buffer length
return:
    driver status
*/
kdrv_status_t kdrv_uart_write(kdrv_uart_handle_t handle, uint8_t *data, uint32_t len)
{
    uint32_t com_port = handle;

#ifdef UART_DEBUG
    if (com_port >= TOTAL_UART_DEV)
    {
        dbg_msg("Invalid parameter\n");
        return KDRV_STATUS_ERROR;
    }

    if (gDrvCtx.active_dev[com_port] == false)
    {
        dbg_msg("This UART device has been closed\n");
        return KDRV_STATUS_ERROR;
    }
#endif

    uart_driver_handle_t *pDrv = gDrvCtx.uart_dev[com_port];

#ifdef UART_DEBUG
    if ((data == NULL) || (len == 0))
    {
        // Invalid parameters
        return KDRV_STATUS_ERROR;
    }
#endif

    if (pDrv->info.status.tx_busy == 1)
    {
        // Send is not completed yet
        return KDRV_STATUS_UART_TX_RX_BUSY;
    }

    if (pDrv->info.mode & UART_MODE_ASYN_TX)
    {
        /* setup TX FIFO
           re-calculate FIFO trigger value based on buffer length
        */
        if (pDrv->config.fifo_en == true)
        {
            kdrv_uart_fifo_config_t cfg;
            cfg.bEnFifo = true;
            cfg.fifo_trig_level = 0; // init with 0
            kdp_calculate_fifo_cfg(pDrv, len, &cfg);
            kdrv_uart_configure(com_port, UART_CTRL_FIFO_TX, (uint8_t *)&cfg);
        }

        pDrv->info.status.tx_busy = 1;

        // Save transmit buffer info
        pDrv->info.xfer.tx_buf = (uint8_t *)data;
        pDrv->info.xfer.tx_num = len;
        pDrv->info.xfer.tx_cnt = 0;

        outw(UART_PORT[pDrv->uart_port] + SERIAL_IER, SERIAL_IER_TE);

        NVIC_SetVector((IRQn_Type)pDrv->res.irq_num, (uint32_t)pDrv->res.isr);
        NVIC_EnableIRQ((IRQn_Type)pDrv->res.irq_num);

        UART_TX_ISR(pDrv);

        return KDRV_STATUS_UART_TX_RX_BUSY;
    }
    else if (pDrv->info.mode & UART_MODE_SYNC_TX)
    {

        pDrv->info.status.tx_busy = 1;

        while (len > 0)
        {

            CheckTxStatus((DRVUART_PORT)com_port);
            outw(UART_PORT[com_port] + SERIAL_THR, *data++);

            len--;
        }

        pDrv->info.status.tx_busy = 0;

        return KDRV_STATUS_OK;
    }
    else
    {
#ifdef UART_DEBUG
        dbg_msg("Error: Sync/Async mode was not set\n");
#endif
        return KDRV_STATUS_ERROR;
    }
}

/*
Read character data from KL520 device
Input:
    handle: device handle
return:
    character data
*/
kdrv_status_t kdrv_uart_get_char(kdrv_uart_handle_t handle, char *ch)
{
    uint32_t com_port = handle;

#ifdef UART_DEBUG
    if (com_port >= TOTAL_UART_DEV)
    {
        dbg_msg("Invalid parameter\n");
        return KDRV_STATUS_ERROR;
    }

    if (gDrvCtx.active_dev[com_port] == false)
    {
        dbg_msg("This UART device has been closed\n");
        return KDRV_STATUS_ERROR;
    }
#endif

    uart_driver_handle_t *pDrv = gDrvCtx.uart_dev[com_port];

    // Check if receiver is busy
    if (pDrv->info.status.rx_busy == 1)
    {
        return KDRV_STATUS_UART_TX_RX_BUSY;
    }

    if (pDrv->info.mode & UART_MODE_ASYN_RX)
    {
        return KDRV_STATUS_UART_TX_RX_BUSY;
    }
    else if (pDrv->info.mode & UART_MODE_SYNC_RX)
    {
        CheckRxStatus((DRVUART_PORT)com_port);
        *ch = inw(UART_PORT[com_port] + SERIAL_RBR);
    }

    return KDRV_STATUS_OK;
}

/**
  Read data from UART receiver.
Input
  data:  buffer for receving data
  len:   size  Data buffer size in bytes
  handle:  Driver handle
return
    driver status
*/
kdrv_status_t kdrv_uart_read(kdrv_uart_handle_t handle, uint8_t *data, uint32_t len)
{
    uint32_t com_port = handle;

#ifdef UART_DEBUG
    if (com_port >= TOTAL_UART_DEV)
    {
        dbg_msg("Invalid parameter\n");
        return KDRV_STATUS_ERROR;
    }

    if (gDrvCtx.active_dev[com_port] == false)
    {
        dbg_msg("This UART device has been closed\n");
        return KDRV_STATUS_ERROR;
    }
#endif

    uart_driver_handle_t *pDrv = gDrvCtx.uart_dev[com_port];

#ifdef UART_DEBUG
    if ((data == NULL) || (len == 0))
    {
        // Invalid parameters
        return UART_API_INVALID_PARAM;
    }
#endif

    // Check if receiver is busy
    if (pDrv->info.status.rx_busy == 1)
    {
        return KDRV_STATUS_UART_TX_RX_BUSY;
    }

    // Clear RX status
    pDrv->info.status.rx_break = 0;
    pDrv->info.status.rx_framing_error = 0;
    pDrv->info.status.rx_overflow = 0;
    pDrv->info.status.rx_parity_error = 0;

    // Save receive buffer info
    pDrv->info.xfer.rx_buf = (uint8_t *)data;
    pDrv->info.xfer.rx_cnt = 0;
    // Save number of data to be received
    pDrv->info.xfer.rx_num = len;

    /* setup RX FIFO trigger level based on buf len*/
    if (pDrv->config.fifo_en == true)
    {
        kdrv_uart_fifo_config_t cfg;
        cfg.bEnFifo = true;
        cfg.fifo_trig_level = 0; // init with 0
        kdp_calculate_fifo_cfg(pDrv, len, &cfg);
        kdrv_uart_configure(com_port, UART_CTRL_FIFO_RX, (uint8_t *)&cfg);
    }

    pDrv->info.status.rx_busy = 1;

    outw(UART_PORT[pDrv->uart_port] + SERIAL_IER, SERIAL_IER_DR);

    NVIC_SetVector((IRQn_Type)pDrv->res.irq_num, (uint32_t)pDrv->res.isr);
    NVIC_EnableIRQ((IRQn_Type)pDrv->res.irq_num);

    if (pDrv->info.mode & UART_MODE_ASYN_RX)
    {
        return KDRV_STATUS_UART_TX_RX_BUSY;
    }
    else if (pDrv->info.mode & UART_MODE_SYNC_RX)
    {
        int32_t timeout = pDrv->nTimeOutRx * 20; // 20*50 = 1000us = 1ms

        if (timeout == 0)
        {
            while (len > 0)
            {
                CheckRxStatus((DRVUART_PORT)com_port);
                *data++ = inw(UART_PORT[com_port] + SERIAL_RBR);
                len--;
                pDrv->info.xfer.rx_cnt++;
            }
        }
        else
        {
            while ((len > 0) && (timeout > 0))
            {
                uint32_t status;
                do
                {
                    status = uart_get_status((DRVUART_PORT)com_port);
                    kdrv_delay_us(50);
                    timeout--;

                } while ((!((status & SERIAL_IER_DR) == SERIAL_IER_DR)) && (timeout > 0));

                if (timeout == 0)
                    return KDRV_STATUS_UART_TIMEOUT;

                *data++ = inw(UART_PORT[com_port] + SERIAL_RBR);
                len--;
                pDrv->info.xfer.rx_cnt++;
            }
        }

        pDrv->info.status.rx_busy = 0;

        return KDRV_STATUS_OK;
    }
    else
    {
#ifdef UART_DEBUG
        dbg_msg("Error: Sync/Async mode was not set\n");
#endif
        return KDRV_STATUS_ERROR;
    }
}

/* get char number in Rx buffer 
Input:
    handle: device handle
Return:
    Received bytes 
*/
uint32_t kdrv_uart_get_rx_count(kdrv_uart_handle_t handle)
{
    uint32_t data;
    uint32_t com_port = handle;

    uart_driver_handle_t *pDrv = gDrvCtx.uart_dev[com_port];
    data = pDrv->info.xfer.rx_cnt;
    return data;
}

uint32_t kdrv_uart_get_tx_count(kdrv_uart_handle_t handle)
{
    uint32_t data;
    uint32_t com_port = handle;

    uart_driver_handle_t *pDrv = gDrvCtx.uart_dev[com_port];
    data = pDrv->info.xfer.tx_cnt;
    return data;
}

int32_t fputc(int ch, FILE *f)
{
    char cc;

    if (ch != '\0')
    {
        cc = ch;
        kdrv_uart_write(handle0, (uint8_t *)&cc, 1);
    }

    if (ch == '\n')
    {
        cc = '\r';
        kdrv_uart_write(handle0, (uint8_t *)&cc, 1);
    }
    return ch;
}

int32_t fgetc(FILE *f)
{
    char c;
    kdrv_uart_read(handle0, (uint8_t *)&c, 1);
    return c;
}
