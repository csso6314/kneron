#ifndef __SERIAL_H
#define __SERIAL_H


#include "base.h"


typedef enum {
    DRVUART_PORT0=0,
    DRVUART_PORT1=1,
    DRVUART_PORT2=2,
    DRVUART_PORT3=3,
} DRVUART_PORT;

#define UART_CLOCK            (30000000UL) //kneron
extern u32 UART_PORT[4];
/*
#define UART_FTUART010_0_PA_BASE 0xC1400000UL //UART_u0
#define UART_FTUART010_1_PA_BASE 0xC1500000UL //UART_u1
#define UART_FTUART010_2_PA_BASE 0xC1600000UL //UART_u1_1
#define UART_FTUART010_3_PA_BASE 0xC1700000UL //UART_u1_2
*/
#define SERIAL_THR                     0x00             /*  Transmitter Holding Register(Write).*/
#define SERIAL_RBR                     0x00             /*  Receive Buffer register (Read).*/
#define SERIAL_IER                     0x04             /*  Interrupt Enable register.*/
#define SERIAL_IIR                     0x08             /*  Interrupt Identification register(Read).*/
#define SERIAL_FCR                     0x08             /*  FIFO control register(Write).*/
#define SERIAL_LCR                     0x0C             /*  Line Control register.*/
#define SERIAL_MCR                     0x10             /*  Modem Control Register.*/
#define SERIAL_LSR                     0x14             /*  Line status register(Read) .*/
#define SERIAL_MSR                     0x18             /*  Modem Status register (Read).*/
#define SERIAL_SPR                     0x1C         /*  Scratch pad register */
#define SERIAL_DLL                     0x0          /*  Divisor Register LSB */
#define SERIAL_DLM                     0x4          /*  Divisor Register MSB */
#define SERIAL_PSR                     0x8             /* Prescale Divison Factor */

#define SERIAL_MDR                     0x20
#define SERIAL_ACR                     0x24
#define SERIAL_TXLENL                  0x28
#define SERIAL_TXLENH                  0x2C
#define SERIAL_MRXLENL                 0x30
#define SERIAL_MRXLENH                 0x34
#define SERIAL_PLR                     0x38
#define SERIAL_FMIIR_PIO               0x3C
#define SERIAL_FEATURE                 0x68

/* IER Register */
#define SERIAL_IER_DR                  0x1          /* Data ready Enable */
#define SERIAL_IER_TE                  0x2          /* THR Empty Enable */
#define SERIAL_IER_RLS                 0x4          /* Receive Line Status Enable */
#define SERIAL_IER_MS                  0x8          /* Modem Staus Enable */

/* IIR Register */
#define SERIAL_IIR_NONE                0x1            /* No interrupt pending */
#define SERIAL_IIR_RLS                 0x6            /* Receive Line Status */
#define SERIAL_IIR_DR                  0x4            /* Receive Data Ready */
#define SERIAL_IIR_TIMEOUT             0xc            /* Receive Time Out */
#define SERIAL_IIR_TE                  0x2            /* THR Empty */
#define SERIAL_IIR_MODEM               0x0            /* Modem Status */

/* FCR Register */
#define SERIAL_FCR_FE                  0x1              /* FIFO Enable */
#define SERIAL_FCR_RXFR                0x2              /* Rx FIFO Reset */
#define SERIAL_FCR_TXFR                0x4              /* Tx FIFO Reset */

/* LCR Register */
#define SERIAL_LCR_LEN5                0x0
#define SERIAL_LCR_LEN6                0x1
#define SERIAL_LCR_LEN7                0x2
#define SERIAL_LCR_LEN8                0x3

#define SERIAL_LCR_STOP                0x4
#define SERIAL_LCR_EVEN                0x18          /* Even Parity */
#define SERIAL_LCR_ODD                 0x8          /* Odd Parity */
#define SERIAL_LCR_PE                  0x8            /* Parity Enable */
#define SERIAL_LCR_SETBREAK            0x40             /* Set Break condition */
#define SERIAL_LCR_STICKPARITY         0x20             /* Stick Parity Enable */
#define SERIAL_LCR_DLAB                0x80         /* Divisor Latch Access Bit */

/* LSR Register */
#define SERIAL_LSR_DR                  0x1          /* Data Ready */
#define SERIAL_LSR_OE                  0x2          /* Overrun Error */
#define SERIAL_LSR_PE                  0x4          /* Parity Error */
#define SERIAL_LSR_FE                  0x8          /* Framing Error */
#define SERIAL_LSR_BI                  0x10         /* Break Interrupt */
#define SERIAL_LSR_THRE                0x20         /* THR Empty */
#define SERIAL_LSR_TE                  0x40         /* Transmitte Empty */
#define SERIAL_LSR_DE                  0x80         /* FIFO Data Error */

/* MCR Register */
#define SERIAL_MCR_DTR                 0x1        /* Data Terminal Ready */
#define SERIAL_MCR_RTS                 0x2        /* Request to Send */
#define SERIAL_MCR_OUT1                0x4        /* output    1 */
#define SERIAL_MCR_OUT2                0x8        /* output2 or global interrupt enable */
#define SERIAL_MCR_LPBK                0x10         /* loopback mode */


/* MSR Register */
#define SERIAL_MSR_DELTACTS            0x1        /* Delta CTS */
#define SERIAL_MSR_DELTADSR            0x2        /* Delta DSR */
#define SERIAL_MSR_TERI                0x4        /* Trailing Edge RI */
#define SERIAL_MSR_DELTACD             0x8        /* Delta CD */
#define SERIAL_MSR_CTS                 0x10         /* Clear To Send */
#define SERIAL_MSR_DSR                 0x20         /* Data Set Ready */
#define SERIAL_MSR_RI                  0x40         /* Ring Indicator */
#define SERIAL_MSR_DCD                 0x80         /* Data Carrier Detect */


/* MDR register */
#define SERIAL_MDR_MODE_SEL            0x03
#define SERIAL_MDR_UART                0x0
#define SERIAL_MDR_SIR                 0x1
#define SERIAL_MDR_FIR                 0x2

/* ACR register */
#define SERIAL_ACR_TXENABLE            0x1
#define SERIAL_ACR_RXENABLE            0x2
#define SERIAL_ACR_SET_EOT             0x4

#define BAUD_921600                    (UART_CLOCK / 14745600)
#define BAUD_460800                    (UART_CLOCK / 7372800)
#define BAUD_115200                    (UART_CLOCK / 1843200)
#define BAUD_57600                     (UART_CLOCK / 921600)
#define BAUD_38400                     (UART_CLOCK / 614400)
#define BAUD_19200                     (UART_CLOCK / 307200)
#define BAUD_14400                     (UART_CLOCK / 230400)
#define BAUD_9600                      (UART_CLOCK / 153600)
#define BAUD_4800                      (UART_CLOCK / 76800)
#define BAUD_2400                      (UART_CLOCK / 38400)
#define BAUD_1200                      (UART_CLOCK / 19200)

#define DEBUG_CONSOLE                  DRVUART_PORT0
#define DEFAULT_CONSOLE_BAUD           BAUD_115200

#ifndef PARITY_NONE
#define PARITY_NONE             0
#endif

#ifndef PARITY_ODD
#define PARITY_ODD              1
#endif

#ifndef PARITY_EVEN
#define PARITY_EVEN             2
#endif

#ifndef PARITY_MARK
#define PARITY_MARK             3
#endif

#ifndef PARITY_SPACE
#define PARITY_SPACE    4
#endif


#ifndef ON
#define ON              1
#endif

#ifndef OFF
#define OFF             0
#endif

#define BACKSP_KEY 0x08
#define RETURN_KEY 0x0D
#define DELETE_KEY 0x7F
#define BELL       0x07

/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
extern void fLib_SetSerialMode(DRVUART_PORT port_no, u32 mode);
extern void fLib_EnableIRMode(DRVUART_PORT port_no, u32 TxEnable, u32 RxEnable);
extern void fLib_SerialInit (DRVUART_PORT port_no, u32 baudrate, u32 parity,u32 num,u32 len, u32 interruptMode);
extern void fLib_SetSerialLoopback(DRVUART_PORT port_no, u32 onoff);
extern void fLib_SetSerialFifoCtrl(DRVUART_PORT port_no, u32 level_tx, u32 level_rx, u32 resettx, u32 resetrx);  //V1.20//ADA10022002
extern void fLib_DisableSerialFifo(DRVUART_PORT port_no);
extern void fLib_SetSerialInt(DRVUART_PORT port_no, u32 IntMask);
extern char fLib_GetSerialChar(DRVUART_PORT port_no);
extern void fLib_PutSerialChar(DRVUART_PORT port_no, char Ch);
extern void fLib_PutSerialStr(DRVUART_PORT port_no, char *Str);
extern void fLib_Modem_waitcall(DRVUART_PORT port_no);
extern void fLib_Modem_call(DRVUART_PORT port_no, char *tel);
extern int fLib_Modem_getchar(DRVUART_PORT port_no,int TIMEOUT);
extern bool fLib_Modem_putchar(DRVUART_PORT port_no, s8 Ch);
extern void fLib_EnableSerialInt(DRVUART_PORT port_no, u32 mode);
extern void fLib_DisableSerialInt(DRVUART_PORT port_no, u32 mode);
extern u32 fLib_ReadSerialIER(DRVUART_PORT port_no);
extern u32 fLib_SerialIntIdentification(DRVUART_PORT port_no);
extern void fLib_SetSerialLineBreak(DRVUART_PORT port_no);
extern void fLib_SerialRequestToSend(DRVUART_PORT port_no);
extern void fLib_SerialStopToSend(DRVUART_PORT port_no);
extern void fLib_SerialDataTerminalReady(DRVUART_PORT port_no);
extern void fLib_SerialDataTerminalNotReady(DRVUART_PORT port_no);
extern u32 fLib_ReadSerialLineStatus(DRVUART_PORT port_no);
extern u32 fLib_ReadSerialModemStatus(DRVUART_PORT port_no);
extern u32 GetUartStatus(DRVUART_PORT port_no);
extern u32 IsThrEmpty(u32 status);
extern u32 IsDataReady(u32 status);
extern void CheckRxStatus(DRVUART_PORT port_no);
extern void CheckTxStatus(DRVUART_PORT port_no);
extern u32 fLib_kbhit(DRVUART_PORT port_no);
extern char fLib_getch(DRVUART_PORT port_no);
extern char fLib_getchar(DRVUART_PORT port_no);
extern char fLib_getchar_timeout(DRVUART_PORT port_no, unsigned long timeout);
extern void fLib_putchar(DRVUART_PORT port_no, char Ch);
extern void fLib_putc(DRVUART_PORT port_no, char Ch);
extern void fLib_putstr(DRVUART_PORT port_no, char *str);
extern void fLib_printf(const char *f, ...);    /* variable arguments */
extern int fLib_gets(DRVUART_PORT port_no, char *buf);
extern void fLib_DebugPrintChar(DRVUART_PORT port_no, char ch);
extern void fLib_DebugPrintString(DRVUART_PORT port_no, char *str);
extern char fLib_DebugGetChar(DRVUART_PORT port_no);
extern u32 fLib_DebugGetUserCommand(DRVUART_PORT port_no, u8 * buffer, u32 Len);
extern void Seg7_Show(u32 value);
#endif // __SERIAL_H
