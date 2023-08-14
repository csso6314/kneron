#ifndef __KDP520_SSP_H__
#define __KDP520_SSP_H__


//#include "types.h"
#include "kdrv_status.h"
#include "kdrv_gpio.h"
#include "kneron_kl720.h"

#pragma anon_unions
/*******************************************
 * for constant definition
 ********************************************/
typedef void (*ARM_SPI_SignalEvent_t ) (uint32_t event);  ///< Pointer to \ref ARM_SPI_SignalEvent : Signal SPI Event.
typedef void (*kdrev_ssp_spi_isr_t)(void);
typedef void (*kdrv_ssp_spi_callback_t)(uint32_t event);
/**
 * @brief Enumerations of SPI port parameters.
 */
typedef enum
{
    SSP_SPI_PORT0 = 0,  /**< Enum 0, SPI port 0 */
    SSP_SPI_PORT1 = 1,  /**< Enum 1, SPI port 1 */
} kdrv_ssp_spi_dev_id_t;

/**
 * @brief Enumerations of SSP_SPI control hardware signals
 */
typedef enum
{
    SSP_SPI_CTRL_CONFIG,       /**< Enum 0, set @ref kdrv_ssp_spi_config_t */
    SSP_SPI_CTRL_FIFO_RX,      /**< Enum 1, set @ref kdrv_ssp_spi_fifo_config_t */
    SSP_SPI_CTRL_FIFO_TX,      /**< Enum 2, set @ref kdrv_ssp_spi_fifo_config_t */
    SSP_SPI_CTRL_LOOPBACK,     /**< Enum 3, SSP_SPI loopback enable */
    SSP_SPI_CTRL_TX_EN,        /**< Enum 4, SSP_SPI transmitter enable */
    SSP_SPI_CTRL_RX_EN,        /**< Enum 5, SSP_SPI receiver enable */
    SSP_SPI_CTRL_ABORT_TX,     /**< Enum 6, SSP_SPI abort transmitter */
    SSP_SPI_CTRL_ABORT_RX,     /**< Enum 7, SSP_SPI abort receiver */
    SSP_SPI_CTRL_TIMEOUT_RX,   /**< Enum 8, SSP_SPI receiver timeout value */
    SSP_SPI_CTRL_TIMEOUT_TX    /**< Enum 9, SSP_SPI transmitter timeout value */
} kdrv_ssp_spi_control_t;

#define COM_BUS_TYPE_SSP0 0
#define COM_BUS_TYPE_SSP1 1
#define SSP_SPI_MASTER_DEV (COM_BUS_TYPE_SSP0)
#define SPI_SLAVE_PORT_DEV (COM_BUS_TYPE_SSP1)
#define chip_select_pin GPIO_PIN_20


#if( SSP_SPI_MASTER_DEV == SPI_SLAVE_PORT_DEV )
#error "Please select correct SSP_SPI_MASTER_DEV for SPI master"
#endif

#define	SPI_TX_FIFO_TH		(1)
#define	SPI_RX_FIFO_TH		(1)
#define SPI_MAX_FIFO        (8)

#define		SPI_PACKET_HEAD					(0x78875A01)
#define		SPI_PACKET_READ_LARGE_HEAD		(0x78875A02)
#define		SPI_PACKET_WRITE_LARGE_HEAD		(0x78875A03)

/*******************************************
 * for operation define definition
 ********************************************/
#define	SSP_SPI_TIME_TEST_EN		(NO)
#if 0
#define SSP_REG_CR0	0x0
#define SSP_REG_CR1	0x4
#define SSP_REG_CR2	0x8
#define SSP_REG_STS	0xc
#define SSP_REG_INTR_CR	0x10
#define SSP_REG_INTR_STS	0x14
#define SSP_REG_DATA_PORT	0x18
#define SSP_REG_CR3	0x1C
#define SSP_REG_REVISION	0x60
#define SSP_REG_FEATURE	0x64
#endif

/* REG_CR0 field */
#define ssp_CR0_FSDBK	(1 << 17)
#define ssp_CR0_SCLKFDBK	(1 << 16)
#define ssp_CR0_SPIFSPO	(1 << 15) /* Frame/Sync polarity, SPI only */
#define ssp_CR0_FFMT_MASK	(7 << 12)
#define ssp_CR0_FFMT_SSP	(0 << 12)
#define ssp_CR0_FFMT_SPI	(1 << 12)
#define ssp_CR0_FFMT_MWR	(2 << 12)
#define ssp_CR0_FFMT_I2S	(3 << 12)
#define ssp_CR0_FFMT_ACL	(4 << 12)
#define ssp_CR0_FFMT_SPDIF	(5 << 12)
#define ssp_CR0_SPI_FLASH	(1 << 11) 
#define ssp_CR0_VALIDITY	(1 << 10) // SPDIF validity
#define ssp_CR0_FSDIS_MASK	(3 << 8)
#define ssp_CR0_FSDIST(x)	((x & 0x3) << 8) // frame/sync and data distance, I2S only
#define ssp_CR0_LOOPBACK	(1 << 7)
#define ssp_CR0_LSB	(1 << 6) // 0: MSB, 1:LSB tx and rx first
#define ssp_CR0_FSPO	(1 << 5) // Frame/Sync polarity, I2S or MWR only
#define ssp_CR0_FSJSTFY	(1 << 4) // Padding data in front(1) or back(0) of serial data
#define ssp_CR0_MODE_MASK	(3 << 2)
#define ssp_CR0_MSTR_STREO	(3 << 2)
#define ssp_CR0_MSTR_MONO	(2 << 2)
#define ssp_CR0_SLV_STREO	(1 << 2)
#define ssp_CR0_SLV_MONO	(0 << 2)
#define ssp_CR0_MSTR_SPI	(3 << 2)
#define ssp_CR0_SLV_SPI	(1 << 2)
#define ssp_CR0_SCLKPO_0	(0 << 1) // SCLK polarity, SPI only
#define ssp_CR0_SCLKPO_1	(1 << 1) // SCLK polarity, SPI only
#define FTSSP020_CR0_SCLKPH_0	(0 << 0) // SCLK phase, SPI only
#define FTSSP020_CR0_SCLKPH_1	(1 << 0) // SCLK phase, SPI only

/* REG_CR1 field */
#define ssp_CR1_PDL(x)	((x & 0xff) << 24) // Padding data length
#define ssp_CR1_PDL_MASK	(0xff << 24)
#define ssp_CR1_SDL(x)	((x & 0x7f) << 16) // Serial data length
#define ssp_CR1_SDL_MASK	(0x7f << 16)
#define ssp_SDL_MAX_BYTES_MASK	(0x7f)
#define ssp_CR1_SCLKDIV(x)	(x & 0xffff)
#define ssp_CR1_SCLKDIV_MASK	(0xffff)

/* REG_CR2 field */
#define ssp_CR2_FSOS(x)	((x & 0x3) << 10) // frame/sync output select, SPI only
#define ssp_CR2_FSOS_MASK	(3 << 10)
#define ssp_CR2_FS 	(1 << 9) // 0: low, 1: high frame/sync output
#define ssp_CR2_TXEN	(1 << 8)
#define ssp_CR2_RXEN	(1 << 7)
#define ssp_CR2_SSPRST	(1 << 6)
#define ssp_CR2_ACRST	(1 << 5)
#define ssp_CR2_ACWRST	(1 << 4)
#define ssp_CR2_TXFCLR	(1 << 3) // W1C, Clear TX FIFO
#define ssp_CR2_RXFCLR	(1 << 2) // W1C, Clear RX FIFO
#define ssp_CR2_TXDOE	(1 << 1) // TX Data Output Enable, SSP slave only
#define ssp_CR2_SSPEN	(1 << 0)

/* REG_STS 0xC field */
#define ssp_STS_TFVE(x)	((x >> 12) & 0x3f) // TX FIFO valid entries
#define ssp_STS_RFVE(x)	((x >> 4) & 0x3f) // RX FIFO valid entries
#define ssp_STS_BUSY	(1 << 2)
#define ssp_STS_TFNF	(1 << 1) // TX FIFO not full
#define ssp_STS_RFF	(1 << 0) // RX FIFO full

/* REG_INTR_CR 0x10 field */
#define ssp_INTCR_RFTHOD_UNIT	(1 << 17)
#define ssp_INTCR_TFTHOD(x)	((x & 0x1f) << 12)
#define ssp_INTCR_TFTHOD_MASK	(0x1f << 12)
#define ssp_INTCR_RFTHOD(x)	((x & 0x1f) << 7)
#define ssp_INTCR_RFTHOD_MASK	(0x1f << 7)
#define ssp_INTCR_AC97FCEN	(1 << 6)
#define ssp_INTCR_TFDMAEN	(1 << 5)
#define ssp_INTCR_RFDMAEN	(1 << 4)
#define ssp_INTCR_TFTHIEN	(1 << 3)
#define ssp_INTCR_RFTHIEN	(1 << 2)
#define ssp_INTCR_TFURIEN	(1 << 1)
#define ssp_INTCR_RFORIEN	(1 << 0)

/* REG_INTR_STS 0x14 field */
#define ssp_INTSTS_AC97CI	(1 << 4)
#define ssp_INTSTS_TFTHI	(1 << 3) // TX FIFO threshold
#define ssp_INTSTS_RFTHI	(1 << 2) // RX FIFO threshold
#define ssp_INTSTS_TFUI	(1 << 1) // TX FIFO underrun
#define ssp_INTSTS_RFORI	(1 << 0) // RX FIFO overrun

/* REG_CR3 0x1C field */
#define ssp_CR3_DPDL(x)	((x & 0xff) << 16) // Padding Data length
#define ssp_CR3_DPDL_MASK	(0xff << 16)
#define ssp_CR3_DPDLEN	(1 << 12)
#define ssp_CR3_PCL(x)	(x & 0x3ff) // Padding Cycle length
#define ssp_CR3_PCL_MASK	0x3ff

// REG_FEATURE 0x64 field
#define ssp_FEA_TXFIFO_DEPTH(x)	((x >> 16) & 0xff)
#define ssp_FEA_RXFIFO_DEPTH(x)	((x >> 8) & 0xff)


/****** SPI Event *****/
#define ARM_SPI_EVENT_SEND_COMPLETE       (1UL << 0)  ///< Send completed; however SPI may still transmit data
#define ARM_SPI_EVENT_RECEIVE_COMPLETE    (1UL << 1)  ///< Receive completed
#define ARM_SPI_EVENT_TRANSFER_COMPLETE   (1UL << 2)  ///< Transfer completed
#define ARM_SPI_EVENT_TX_COMPLETE         (1UL << 3)  ///< Transmit completed (optional)
#define ARM_SPI_EVENT_TX_UNDERFLOW        (1UL << 4)  ///< Transmit data not available (Synchronous Slave)
#define ARM_SPI_EVENT_RX_OVERFLOW         (1UL << 5)  ///< Receive data overflow
#define ARM_SPI_EVENT_RX_TIMEOUT          (1UL << 6)  ///< Receive character timeout (optional)
#define ARM_SPI_EVENT_RX_BREAK            (1UL << 7)  ///< Break detected on receive
#define ARM_SPI_EVENT_RX_FRAMING_ERROR    (1UL << 8)  ///< Framing error detected on receive
#define ARM_SPI_EVENT_RX_PARITY_ERROR     (1UL << 9)  ///< Parity error detected on receive
/*******************************************************************
 * for register address / bits definition
 ********************************************************************/
//SSP Control Register 0 (SSPCR0, Offset = 0x00)
typedef struct {
    uint32_t SCLKPH            : 1;         /* SCLK phase */ /* Default Value 0 */
    uint32_t SCLKPO            : 1;         /* SCLK polarity */ /* Default Value 0 */
    uint32_t OPM               : 2;         /* Operation mode */ /* Default Value 0x3 */
                                            /* If the SSP, SPI, or MICROWIRE frame format is specified, these
                                             * bits will define the operation modes as follows:
                                             * 00, 01: Slave mode
                                             * 10, 11: Master mode
                                             */
    uint32_t FSJSTFY           : 1;         /* Data justify */ /* Default Value 0 */
                                            /* This bit will be valid only when the I2S frame format is specified. */
    uint32_t FSPO              : 1;         /* Frame/Sync. polarity */ /* Default Value 0 */
                                            /* This bit will take effect only when the I2S or MWR frame format is specified. */
    uint32_t LSB               : 1;         /* Bit sequence indicator */ /* Default Value 0 */
                                            /* This bit will take effect only when the I2S or SPI frame format is specified. */
    uint32_t LBM               : 1;         /* Loopback mode */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the transmitted data will be connected to the received data internally. This bit is used for self-test only. */
                                            /* If this bit is set to ．0・, SSP will operate in the normal mode and the transmitted/received data will be independent. */
    uint32_t FSDIST            : 2;         /* Frame/Sync. and data distance */ /* Default Value 0x1 */
                                            /* These bits will take effect only when the I2S frame format is specified. */
    uint32_t rsvd              : 1;         /* Reserved */
    uint32_t FLASH             : 1;         /* This bit indicates that the current application is SPI Flash. */ /* Default Value 0 */
                                            /* This bit will take effect only when the SPI frame format is specified. */
    uint32_t FFMT              : 3;         /* Frame format */ /* Default Value 0 */
                                            /* This register defines the pre-defined frame format according to the following encodings:
                                             * 000: Texas Instrument Synchronous Serial Port (SSP)
                                             * 001: Motorola Serial Peripheral Interface (SPI)
                                             * 010: National Semiconductor MICROWIRE
                                             * 011: Philips I2S
                                             * 100 ~ 111: Not defined
                                             */
    uint32_t SPIFSPO           : 1;         /* Frame/Sync. polarity for the SPI mode */ /* Default Value 0 */
                                            /* This bit will take effect only when the SPI frame format is specified. */
    uint32_t SCLKFDBK          : 1;         /* This bit indicates that sclk_in is the internal feedback from sclk_out_r. */ /* Default Value 0 */
                                            /* If the master mode is specified, FTSSP010 will need sclk_in for reference. */
                                            /* If this bit is set to ．1・, sclk_in will be the internal feedback from sclk_out_r. */
                                            /* If this bit is set to ．0・, sclk_in will be inputted from the input pin. */
    uint32_t FSFDBK            : 1;         /* This bit indicates that fs_in is the internal feedback from fs_out_r. */ /* Default Value 0 */
                                            /* If the master mode is specified, FTSSP010 will need fs_in for reference. */
                                            /* If this bit is set to ．1・, fs_in will be the internal feedback from fs_out_r. */
                                            /* If this bit is set to ．0・, fs_in will be inputted from input pin. */
    uint32_t rsvd1             : 14;        /* Reserved and read as zero */
} kdrv_ssp_sspcr0_t;

//SSP Control Register 1 (SSPCR1, Offset = 0x04)
typedef struct {
    uint32_t SCLKDIV           : 16;        /* SCLK divider */ /* Default Value 0x8000 */
                                            /* These bits define the serial clock divider.
                                             * These bits should larger than 2, 4 or 6 by setting.
                                             */
    uint32_t SDL               : 7;         /* Serial data length */ /* Default Value 0x7 */
                                            /* These bits define the bit length of a transmitted/received data word. 
                                             * The actual data length equals to these bits plus one. The minimum SDL value should not be fewer than four.
                                             */
    uint32_t rsvd              : 1;         /* Reserved */
    uint32_t PDL               : 8;         /* Padding data length */ /* Default Value 0 */
} kdrv_ssp_sspcr1_t;

//SSP Control Register 2 (SSPCR2, Offset = 0x08)
typedef struct {
    uint32_t SSPEN             : 1;         /* SSP Enable */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the SSP controller will start transmitting/receiving data if possible.
                                             * If this bit is set to ．0・, the serial data will stop toggling.
                                             */
    uint32_t TXDOE             : 1;         /* Transmit Data Output Enable */ /* Default Value 1 */
                                            /* This bit is only valid when the slave mode is specified.
                                             * In the multi-slave system, it is possible for an SSP master to broadcast a message to all slaves
                                             * within the system while ensuring that only one slave drives data onto its serial output line.
                                             */
    uint32_t RXFCLR            : 1;         /* Receive FIFO Clear */ /* Default Value 0 */
                                            /* This is a write only bit. If this bit is written as ．1・, all data in receive FIFO will be cleared. */
    uint32_t TXFCLR            : 1;         /* Transmit FIFO Clear */ /* Default Value 0 */
                                            /* This is a write only bit. If this bit is written as ．1・, all data in receive FIFO will be cleared. */
    uint32_t rsvd              : 2;         /* Reserved */
    uint32_t SSPRST            : 1;         /* SSP Reset */ /* Default Value 0 */
                                            /* The software reset of the SSP controller state machine */
                                            /* Writing ．1・ to this bit will reset the SSP controller state machine. */
    uint32_t RXEN              : 1;         /* Receive Function Enable */ /* Default Value 0 */
                                            /* When the I2S, SPI, or MICROWIRE frame format is specified, the transmit and receive functions will work independently.
                                             * This bit controls the receive function.
                                             * 1: Receive function is enabled.
                                             * 0: Receive function is disabled.
                                             * For the SPI and MICROWIRE frame formats, this bit can only be changed when the SSP controller is idle.
                                             */
    uint32_t TXEN              : 1;         /* Transmit Function Enable */ /* Default Value 0 */
                                            /* When the I2S, SPI, or MICROWIRE frame format is specified, the transmit and receive functions will work independently.
                                             * This bit controls the receive function.
                                             * 1: Transmit function is enabled.
                                             * 0: Transmit function is disabled.
                                             * For the SPI and MICROWIRE frame formats, this bit can only be changed when the SSP controller is idle.
                                             */
    uint32_t FS                : 1;         /* Frame Sync. output */ /* Default Value 0 */
                                            /* This bit controls the frame/sync. output level in the SPI frame format.
                                             * If this bit is set to ．0・, the FS output will be ：LOW；.
                                             * If this bit is set to ．1・, the FS output will be ：HIGH；.
                                             */
    uint32_t FSOS              : 2;         /* Frame/Sync. Output Select */ /* Default Value 0 */
                                            /* When the SPI frame format is specified and these two bits are set as:
                                             * 00: Frame/Sync. output from the output port, fs_out_r
                                             * 01: Frame/Sync. output from the output port, fs1_out_r
                                             * 10: Frame/Sync. output from the output port, fs2_out_r
                                             * 11: Frame/Sync. output from the output port, fs3_out_r
                                             */
    uint32_t rsvd1             : 20;        /* Reserved and read as zero */
} kdrv_ssp_sspcr2_t;

//SSP Status Register (SSPStatus, Offset = 0x0C)
typedef struct {
    uint32_t RFF               : 1;         /* Receive FIFO Full */ /* Default Value 0 */
                                            /* This bit will be set to ．1・ whenever the receive FIFO is full.
                                             * This bit will be cleared to ．0・ when the DMA controller or host processor reads data from the receive FIFO.
                                             */
    uint32_t TFNF              : 1;         /* Transmit FIFO Not Full */ /* Default Value 1 */
                                            /* This bit will be set to ．1・ whenever the transmit FIFO is available for DMA or host processor to write.
                                             * This bit will be cleared to ．0・ when FIFO is completely full.
                                             */
    uint32_t BUSY              : 1;         /* Busy Indicator */
                                            /* If this bit is read to ．1・, it will indicate that SSP is transmitting and/or receiving data.
                                             * If this bit is read to ．0・, it will indicate that SSP is idle or disabled.
                                             */
    uint32_t rsvd              : 1;         /* Reserved */
    uint32_t RFVE              : 6;         /* Receive FIFO Valid Entry */ /* Default Value 0 */
                                            /* These bits indicate the number of entries in the receive FIFO waiting for DMA or host processor to read them. */
    uint32_t rsvd1             : 2;         /* Reserved */
    uint32_t TFVE              : 6;         /* Transmit FIFO Valid Entry */ /* Default Value 0 */
                                            /* These bits indicate the number of entries in the transmit FIFO waiting to be transmitted. */
    uint32_t rsvd2             : 14;        /* Reserved */
} kdrv_ssp_sspstatus_t;

//Interrupt Control Register (IntrCR, Offset = 0x10)
typedef struct {
    uint32_t RFORIEN           : 1;         /* Receive FIFO Overrun Interrupt Enable */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the receive FIFO overrun will cause SSP to assert interrupt.
                                             * If this bit is set to ．0・, the interrupt will be masked even when the receive FIFO overrun occurs.
                                             */
    uint32_t TFURIEN           : 1;         /* Transmit FIFO Underrun Interrupt Enable */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the transmit FIFO underrun will cause SSP to assert interrupt.
                                             * If this bit is set to ．0・, the interrupt will be masked even when the transmit FIFO underrun happens.
                                             */
    uint32_t RFTHIEN           : 1;         /* Receive FIFO Threshold Interrupt Enable */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the interrupt will be issued when the valid entries
                                             * in the receive FIFO are greater than or equal to threshold value.
                                             */
    uint32_t TFTHIEN           : 1;         /* Transmit FIFO Threshold Interrupt Enable */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the interrupt will be issued when the valid entries
                                             * in the transmit FIFO are less than or equal to the threshold value.
                                             */
    uint32_t RFDMAEN           : 1;         /* Receive DMA Request Enable */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the DMA request will be issued when the receive FIFO threshold is hit.
                                             * If this bit is set to ．0・, no DMA request will be issued.
                                             */
    uint32_t TFDMAEN           : 1;         /* Transmit DMA Request Enable */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the DMA request will be issued when the transmit FIFO threshold is hit.
                                             * If this bit is set to ．0・, no DMA request will be issued.
                                             */
    uint32_t rsvd              : 1;         /* Reserved */
    uint32_t RFTHOD            : 5;         /* Receive FIFO Threshold */ /* Default Value 0x2 */
                                            /* If the valid data in the receive FIFO is equal to or greater than the actual threshold, the DMA request
                                             * and/or the interrupt will be asserted. If this bit is set to ．0・, the interrupt will be disabled.
                                             */
    uint32_t TFTHOD            : 5;         /* Transmit FIFO Threshold */ /* Default Value 0x2 */
                                            /* If the valid data in the transmit FIFO is equal to or less than the actual threshold, the DMA request
                                             * and/or the interrupt will be asserted. If this bit is set to ．0・, the interrupt will be disabled.
                                             */
    uint32_t RFTHOD_UNIT       : 1;         /* Receive FIFO Threshold Unit */ /* Default Value 0 */
                                            /* If this bit is set to ．1・, the RxFIFO threshold = RFTHOD + 1. 
                                             * If this bit is set to ．0・, the RxFIFO threshold = RFTHOD. 
                                             */
    uint32_t rsvd1             : 14;        /* Reserved */
} kdrv_ssp_intrcr_t;

//Interrupt Status Register (IntrStatus, Offset = 0x14)
typedef struct {
    uint32_t RFORI             : 1;         /* Receive FIFO Overrun Interrupt */ /* Default Value 0 */
                                            /* If the receive logic tries to receive data when the receive FIFO is full, this bit will be set to ．1・.
                                             * This bit will never be cleared to ．0・ until users read this bit.
                                             */
    uint32_t TFURI             : 1;         /* Transmit FIFO Underrun Interrupt */ /* Default Value 0 */
                                            /* If the transmit logic tries to retrieve data from the empty transmit FIFO, this bit will be set to ．1・
                                             * This bit will never be cleared to ．0・ until users read this bit.
                                             */
    uint32_t RFTHI             : 1;         /* Receive FIFO Threshold Interrupt */ /* Default Value 0 */
                                            /* If the receive FIFO is equal to or greater than the threshold, this bit will be set to ．1・.
                                             * This bit will be cleared when the condition above is removed.
                                             */
    uint32_t TFTHI             : 1;         /* Transmit FIFO Threshold Interrupt */ /* Default Value 1 */
                                            /* If the transmit FIFO is equal to or less than the threshold, this bit will be set to ．1・.
                                             * If the valid entries in the transmit FIFO are larger than TFTHOD, this bit will be automatically cleared.
                                             */
    uint32_t rsvd              : 28;        /* Reserved */
} kdrv_ssp_intrstatus_t;

//SSP Transmit/Receive Data Register (TxRxDR, Offset = 0x18)
typedef struct {
    uint32_t txrx_data_reg;         /* SPI Flash address */ /* Default Value 0 */
                                    /* This register decides the values of the SPI Flash
                                     * address and issues this address to the SPI Flash. The
                                     * byte of the address is executed by the address length
                                     * when offset = 0x04.
                                     */
} kdrv_ssp_txrxdr_t;

//SSP Control Register 3 (SSPCR3, Offset = 0x1C)
typedef struct {
    uint32_t PCL               : 10;        /* Padding Cycle Length */ /* Default Value 0 */
                                            /* These bits are relevant only when the SPI frame format is specified.
                                             * In the master mode, SPI will wait (PCL + 1) SCLK cycles between each successive transfer.
                                             */
    uint32_t rsvd              : 2;         /* Reserved */
    uint32_t DPDLEN            : 1;         /* Different Padding Data Length Enable */ /* Default Value 0 */
                                            /* This bit is relevant only when the Philips I2S frame format is specified. */
    uint32_t rsvd1             : 3;         /* Reserved */
    uint32_t DPDL              : 8;         /* Different Padding Data Length */ /* Default Value 0 */
                                            /* These bits are relevant only when the Philips I2S frame format is specified. */
    uint32_t rsvd2             : 8;         /* Reserved */
} kdrv_ssp_sspcr3_t;

//SSP Revision Register (SSPRevision, Offset = 0x60)
typedef struct {
    uint32_t REL_REV           : 8;         /* Release number */
    uint32_t MINOR_REV         : 8;         /* Minor revision number */
    uint32_t MAJOR_REV         : 8;         /* Major revision number */
    uint32_t rsvd              : 8;         /* Reserved */
} kdrv_ssp_ssprevision_t;

//SSP Feature Register (SSPFeature, Offset = 0x64)
typedef struct {
    uint32_t FIFO_WIDTH        : 8;         /* Transmit/Receive FIFO width */ /* The actual depth is the read value plus one. */
                                            /* This bit is always read as 31. */
    uint32_t RXFIFO_DEPTH      : 8;         /* Receive FIFO depth configuration */ /* The actual depth is the read value plus one. */
    uint32_t TXFIFO_DEPTH      : 8;         /* Transmit FIFO depth configuration */ /* The actual depth is the read value plus one. */
    uint32_t rsvd              : 1;         /* Reserved */
    uint32_t I2S_FCFG          : 1;         /* Philip I2S function configurations */
    uint32_t SPIMWR_FCFG       : 1;         /* Motorola SPI and National Semiconductor MICROWIRE function configurations */
    uint32_t SSP_FCFG          : 1;         /* TI SSP function configurations. */
    uint32_t rsvd1             : 1;         /* Reserved */
    uint32_t EXT_FSNUM         : 2;         /* These bits extend the frame/sync. output number configuration. */
    uint32_t rsvd2             : 1;         /* Reserved */
} kdrv_ssp_sspfeature_t;

typedef volatile union {
    uint32_t array[8];
    struct
    {
        union
        {
            struct
            {
                uint32_t    kdrv_ssp_sspcr0;
                uint32_t    kdrv_ssp_sspcr1;
                uint32_t    kdrv_ssp_sspcr2;
                uint32_t    kdrv_ssp_sspstatus;
                uint32_t    kdrv_ssp_intrcr;
                uint32_t    kdrv_ssp_intrstatus;
                uint32_t    kdrv_ssp_txrxdr;
                uint32_t    kdrv_ssp_sspcr3;
            } dw;                         //double word

            struct
            {
                kdrv_ssp_sspcr0_t    kdrv_ssp_sspcr0;
                kdrv_ssp_sspcr1_t    kdrv_ssp_sspcr1;
                kdrv_ssp_sspcr2_t    kdrv_ssp_sspcr2;
                kdrv_ssp_sspstatus_t    kdrv_ssp_sspstatus;
                kdrv_ssp_intrcr_t      kdrv_ssp_intrcr;
                kdrv_ssp_intrstatus_t    kdrv_ssp_intrstatus;
                kdrv_ssp_txrxdr_t      kdrv_ssp_txrxdr;
                kdrv_ssp_sspcr3_t      kdrv_ssp_sspcr3;
            } bf;
        };
    } st;
} U_regSSP_ctrl;

typedef volatile union {
    uint32_t array[2];
    struct
    {
        union
        {
            struct
            {
                uint32_t    kdrv_ssp_ssprevision;
                uint32_t    kdrv_ssp_sspfeature;
            } dw;                         //double word

            struct
            {
                kdrv_ssp_ssprevision_t    kdrv_ssp_ssprevision;
                kdrv_ssp_sspfeature_t    kdrv_ssp_sspfeature;
            } bf;
        };
    } st;
} U_regSSP_feature;

#if 0
#define regSSP0_ctrl ((volatile U_regSSP_ctrl *)SSP0_REG_BASE)
#define regSSP0_irq  ((volatile U_regSSP_feature *)(SSP0_REG_BASE+0x60))
#define regSSP1_ctrl ((volatile U_regSSP_ctrl *)SSP1_REG_BASE)
#define regSSP1_irq  ((volatile U_regSSP_feature *)(SSP1_REG_BASE+0x60))
#else
#define regSSP0_ctrl(n) ((volatile U_regSSP_ctrl *)(SSP0_REG_BASE+(n*0x00100000)))
#define regSSP0_feature(n)  ((volatile U_regSSP_feature *)(SSP0_REG_BASE+(n*0x00100000)+0x60))
#endif

//===========================================================================
// SPI only
typedef enum {
    SPI_CS_LOW = 0,
    SPI_CS_HI = 1,
} SPI_CHIP_SELECT;


typedef enum {
    // CLKPO = 0, CLKPHA = 0
    SPI_MODE_0 = (ssp_CR0_SCLKPO_0 | FTSSP020_CR0_SCLKPH_0),
    // CLKPO = 0, CLKPHA = 1
    SPI_MODE_1 = (ssp_CR0_SCLKPO_0 | FTSSP020_CR0_SCLKPH_1),
    // CLKPO = 1, CLKPHA = 0
    SPI_MODE_2 = (ssp_CR0_SCLKPO_1 | FTSSP020_CR0_SCLKPH_0),
    // CLKPO = 1, CLKPHA = 1
    SPI_MODE_3 = (ssp_CR0_SCLKPO_1 | FTSSP020_CR0_SCLKPH_1),
    SPI_MODE_MAX
} SPI_MODE_TYPE;

enum e_spi
{
    //case
    e_spi_init_slave = 0,
    e_spi_init_master,
    e_spi_enable,
    e_spi_txrx_reinit,
    e_spi_rx,
    e_spi_rx_check,
    e_spi_tx ,
    e_spi_tx_status_check ,
    e_spi_tx_large ,        //for future command mode use
    e_spi_rx_large ,        //receive command action
    e_spi_master_tx_rx,
    e_spi_disable ,
    e_spi_idle ,
    e_spi_tx_xor,


    //return status
    e_spi_ret_init_done,
    e_spi_ret_enable_done,
    e_spi_ret_rxbusy,
    e_spi_ret_rxdone,
    e_spi_ret_rx_xor_OK,
    e_spi_ret_rx_xor_error,

    e_spi_ret_txbusy,
    e_spi_ret_txdone,
    e_spi_ret_disableDone,
    e_spi_ret_tx_xor_done,
    e_spi_ret_idle,
};

struct st_ssp_spi
{
    uint8_t   port_no;                        //0: spi0, 1: spi1
    uint8_t   IP_type;                        //3: master 0:slave
    uint8_t   SDL;                            //data length
    uint8_t   target_Txfifo_depth;            //max is 16Byte
    uint8_t   target_Rxfifo_depth;            //max is 16Byte
    uint8_t   tx_rx_en;                       //0: all disable, 1:Tx enable, 2:Rx enable, 3:Tx and Rx are all enable
    uint8_t   spi_sw_type;                    //0:polling(no interrupt IRQ), 1:interrupt+polling, 2:DMA only
    uint8_t   interrupt_en;                   //0x00:no interurpt, 0x08: Tx int ebable, 0x04:Rx interupt enable,
                                              //0x0C: Tx and Rx interrupt are all enable
    uint8_t   mode;
    uint32_t  sclkdiv;                        //0; //1; /*7:6.5MHZ*/
    ARM_SPI_SignalEvent_t   cb;

    //buffer related
    volatile uint8_t  *Tx_buffer;
    volatile uint32_t Tx_buffer_index;
    volatile uint32_t Tx_buffer_current_index;
    volatile uint8_t  Tx_done_flag;           //0: no done, 1: done flag


    volatile uint8_t  *Rx_buffer;
    volatile uint32_t Rx_buffer_index;
    volatile uint32_t buffer_max_size;
    volatile uint32_t pre_size;
    
    enum e_spi  eflow;

    volatile uint8_t      *Rx_tempbuffer;
    volatile uint32_t     Rx_tempbuffer_index;
};

#define FLAG_SPI0_RX_DONE BIT(0)
#define FLAG_SPI0_RX_TIMEOUT BIT(1)
#define FLAG_SPI0_TX_DONE BIT(2)
#define FLAG_SPI1_RX_DONE BIT(3)
#define FLAG_SPI1_RX_TIMEOUT BIT(4)
#define FLAG_SPI1_TX_DONE BIT(5)

#define MASTER_OP_MODE SPI_MODE_1
#define SLAVE_OP_MODE SPI_MODE_0

#if(SSP_SPI_MASTER_DEV==COM_BUS_TYPE_SSP1 || SSP_SPI_MASTER_DEV==COM_BUS_TYPE_SSP0)
extern uint8_t   gTx_buff_SP_MASTER[];
extern uint8_t   gRx_buff_SP_MASTER[];
extern uint32_t  gTx_buff_index_SP_MASTER;
extern uint32_t  gTx_buff_current_index_SP_MASTER;
extern uint32_t  gRx_buff_index_SP_MASTER;

#endif


uint32_t kdrv_ssp_rxfifo_valid_entries( uint32_t port );
void kdrv_ssp_write_buff( struct st_ssp_spi *stspi, uint8_t *src, uint16_t nlen );
void kdrv_ssp_spi_CS_set( kdrv_gpio_pin_t pin, bool level );
void kdrv_ssp_rx_polling_receive_all( struct st_ssp_spi *stspi );
void kdrv_ssp_SPI_master_transmit( struct st_ssp_spi *st_spi , uint32_t rx_target_size, uint8_t rx_all  );
void kdrv_ssp_clear_tx_buf_index( struct st_ssp_spi *stspi );
uint32_t kdrv_ssp_txfifo_depth( uint32_t port );
uint32_t kdrv_ssp_rxfifo_depth(uint32_t port);
void kdrv_ssp_spi_enable(kdrv_ssp_spi_dev_id_t handle, uint32_t tx, uint32_t rx, uint32_t tx_dma, uint32_t rx_dma);

void kdrv_spi_master_write_dma(kdrv_ssp_spi_dev_id_t handle, uint16_t data_len, uint8_t *p_data);
void kdrv_spi_master_read_dma( kdrv_ssp_spi_dev_id_t handle, uint8_t *w_data, uint8_t *r_data, uint16_t data_len);
void kdrv_spi_slave_write_dma(kdrv_ssp_spi_dev_id_t handle, uint16_t data_len, uint8_t *p_data);
void kdrv_spi_slave_read_dma( kdrv_ssp_spi_dev_id_t handle, uint8_t *w_data, uint8_t *r_data, uint16_t data_len);
enum e_spi kdrv_ssp_statemachine( kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *st_spi, enum e_spi espi_flow, ARM_SPI_SignalEvent_t cb);

#endif 
