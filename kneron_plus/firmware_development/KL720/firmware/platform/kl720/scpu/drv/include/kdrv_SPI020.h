#ifndef __SPI020_H__
#define __SPI020_H__

//#include "KNERONCM4.h"
#include "kneron_kl720.h"

#pragma anon_unions
/*******************************************
 * for constant definition
 ********************************************/
#define FLASH_64K               0x10000
#define FLASH_PAGE_SIZE         256
#define FLASH_NORMAL            0x00
#define FLASH_DTR_RW            0x01
#define FLASH_DUAL_READ         0x02
#define FLASH_DMA_READ          0x04
#define FLASH_DMA_WRITE         0x08
#define FLASH_IO_RW             0x10
#define FLASH_BYTE_MODE         0x20
#define FLASH_QUAD_RW           0x40
#define FLASH_FAST_READ         0x80 //Winbond W25P16
//#define FLASH_OP_MODE           (FLASH_QUAD_RW|FLASH_IO_RW)//FLASH_NORMAL//FLASH_QUAD_RW//(FLASH_QUAD_RW|FLASH_IO_RW)

typedef enum
{
    SPI_CLK_MODE0=0,
    SPI_CLK_MODE3=0x10,
}spi_clk_mode_t;




/*******************************************
 * for operation define definition
 ********************************************/
#define SPI_CLK_DIVIDER_2   0x00
#define SPI_CLK_DIVIDER_4   0x01
#define SPI_CLK_DIVIDER_6   0x02
#define SPI_CLK_DIVIDER_8   0x03


#define SPI020_CE_0         0x0000
#define SPI020_CE_1         0x0100
#define SPI020_CE_2         0x0200
#define SPI020_CE_3         0x0300
#define SPI020_CE_VALUE     SPI020_CE_0
#define SPI020_INTR_CFG     0x00000000


/*******************************************************************
 * for register address / bits definition
 ********************************************************************/
//#define SPI020REG_CMD0      (SPI_FTSPI020_PA_BASE+0x00) /* Command Queue first Word (Cmd_w3) */
typedef struct {
    uint32_t spif_addr;         /* SPI Flash address */ /* Default Value 0 */
                                /* This register decides the values of the SPI Flash
                                 * address and issues this address to the SPI Flash. The
                                 * byte of the address is executed by the address length
                                 * when offset = 0x04.
                                 */
} kdrv_spif_cmd0_t;

//#define SPI020REG_CMD1      (SPI_FTSPI020_PA_BASE+0x04) /* Command Queue Second Word (Cmd_w3) */
typedef struct {
    uint32_t address_length     : 3;    /* Default Value 0x3 */
                                        /* This register decides the SPI Flash address byte number.
                                         * Users can set this register to decide the address byte
                                         * ranging from 1 byte to 4 bytes.
                                         * 3!|b000: No address state
                                         * 3!|b001: 1-byte address
                                         * 3!|b010: 2-byte address
                                         * 3!|b011: 3-byte address
                                         * 3!|b100: 4-byte address
                                         * Others: Reserved */
    uint32_t rsvd               : 13;   /* Reserved */
    uint32_t dum_2nd_cyc        : 8;    /* Second dummy state cycle */ /* Default Value HwCfg */
                                        /* Second dummy state is located between the address and the data state that
                                         * excludes the continuous read mode state. Users can check whether the dummy
                                         * state exists between the address and the data state or not in the SPI Flash
                                         * specification. The host controller will issue logic 1 in the dummy cycle.
                                         * 8!|d0: No second dummy state
                                         * 8!|d1 ~ 8!|d32: 1 dummy second cycle ~ 32 dummy second cycles
                                         * When FTSPI020 has the XIP port, the default value will depend on FTSPI020_XIP_DUMCYC;
                                         * otherwise, it is !¢D0!|. */
    uint32_t instruction_length : 2;    /* instruction code length */ /* Default Value 1 */
                                        /* When users want to execute a SPI Flash command,
                                         * the instructin code must be included.
                                         * Different SPI Flash vendors have different instruction lengths;
                                         * therefore, users can set this register to meet different behaviors.
                                         * Instruction code is normally 1 byte; however, if users set the 2-byte
                                         * instruction length, the host controller will issue this instruction code twice.
                                         * 2!|b00: No instruction code. It can only be used after the contiunous read mode command has been finished.
                                         * 2!|b01: 1-byte instruction code
                                         * 2!|b10: 2-byte instruction code (Repeat the instruction code) */
    uint32_t rsvd1              : 2;    /* Reserved */
    uint32_t continuous_read_en : 1;    /* Continuous Read Mode Enable */ /* Default Value HwCfg */
                                        /* 0: Disable the continuous read mode
                                         * 1: Enable the 1-byte continuous read mode */
    uint32_t rsvd2               : 3;    /* Reserved */
} kdrv_spif_cmd1_t;

//#define SPI020REG_CMD2      (SPI_FTSPI020_PA_BASE+0x08) /* Command Queue Third Word (Cmd_w3) */
typedef struct {
    uint32_t Data_counter       : 32;   /* Read/Write data counter */ /* Default Value 0 */
                                        /* This register must be set to !¢D0!| when performing the read status command.
                                         * 32!|b0: No read/write data
                                         * 0x1 ~ 0xFFFF_FFFF: 1-byte data ~ 0xFFFF_FFFF data
                                         * Please note that no matter it is a data read or data write,
                                         * this register is not allowed to be filled as !¢D0!|.
                                         * However,  such as !¡±read status!¡L or !¡±write enable!¡L instruction,
                                         * this register must set to !¢D0!|. */
} kdrv_spif_cmd2_t;

//#define SPI020REG_CMD3      (SPI_FTSPI020_PA_BASE+0x0C) /* Command Queue Fourth Word (Cmd_w3) */
typedef struct {
    uint32_t rsvd                       : 1;    /* Reserved */
    uint32_t write_enable               : 1;    /* Default Value 0 */
                                                /* Enable the SPI write data, except for the read data or read status (Read the data return path);
                                                 * users must set the write enable = !¢D1!| for other SPI commands.
                                                 * Please note that in the write data or erase Flash command, write enable is set to !¢D1!|.
                                                 * Only when in the read data or read status command, write enable must be set to !¢D0!|. */
    uint32_t read_status_en             : 1;    /* Enable the Read SPI status */ /* Default Value 0 */
                                                /* It is available at write_enable = !¢D0!| and users must issue the SPI read status command.
                                                 * 1'b0: Disable.
                                                 * 1!|b1: Enable. */

    uint32_t read_status                : 1;    /* Read the SPI Flash status by using software or hardware */ /* Default Value 0 */
                                                /* It is only available when the read status is enabled and the write enable = 1!|b0.
                                                 * Users must issue the SPI Read Status command.
                                                 * 1!|b0: Read status by hardware, the controller will poll the status until the status is ready (Not busy) and report the status register.
                                                 * 1!|b1: Read status by software, read status once and report the status register until users can read it. */
    uint32_t DTR_mode                   : 1;    /* DTR (Double Transfer Rate) mode */ /* Default Value 0 */
                                                /* 1'b0: Disable
                                                 * The read status, write data command, and DTR mode must be set to !¢D0!|.
                                                 * 1'b1: Enable
                                                 * The DTR mode is valid at the read data command.
                                                 * The DTR mode will be invalid when hardware configuration is !¡±FTSPI020_DTR_MODE_OFF!¡L.
                                                 * The DTR mode does not support the write operation and the DTR_mode register must be set to "0";
                                                 * otherwise, the sequence will fail. */

    uint32_t operation_mode             : 3;    /* SPI operate mode */ /* Default Value HwCfg */
                                                /* 3'b000: Serial mode
                                                 * 3'b001: Dual mode
                                                 * 3'b011: dual_io mode
                                                 * 3'b010: Quad mode
                                                 * 3'b100: quad_io mode
                                                 * Others: Reserved
                                                 * When FTSPI020 has the XIP port, the default value will depend on FTSPI020_XIP_OPMODE;
                                                 * otherwise, it is !¢D0!|.*/
    uint32_t start_cs                   : 2;    /* FTSPI020 can connect four SPI Flash (Max.) and this bit is used to select cs. */ /* Default Value 0 */
                                                /* 2'b00: cs 0
                                                 * 2'b01: cs 1
                                                 * 2'b10: cs 2
                                                 * 2'b11: cs 3 */
    uint32_t rsvd1                      : 6;    /* Reserved */
    uint32_t continuous_read_op_code    : 8;    /* Continuous read mode operate code */ /* Default Value HwCfg */
                                                /* Users can fill this code to execute the continuous read mode.
                                                 * To do the continuous read mode with this Flash, users have to
                                                 * issue 0xA0 as the mode bit because it does not need the instruction
                                                 * code for the next read command. When FTSPI020 has the XIP port, the
                                                 * default value will depend on FTSPI020_XIP_OPCODE; otherwise, it is !¢D0!|. */
    uint32_t instruction_code           : 8;    /* Instruction code */ /* Default Value HwCfg */
                                                /* Users can set this code to execute the SPI Flash command.
                                                 * When FTSPI020 has the XIP port, the default value will
                                                 * depend on FTSPI020_XIP_INSTRUCTION; otherwise, it is !¢D0!|. */
} kdrv_spif_cmd3_t;

/* SPI control */
//#define SPI020REG_CONTROL   (SPI_FTSPI020_PA_BASE+0x10) /* Control Register */
typedef struct {
    uint32_t spi_clk_divider  : 2;  /* spi_clk divider */ /* Default Value 0x3 */
                                        /* sck_out is divided by spi_clk and factor is listed as follows:
                                        * 2'b00: Divided by 2
                                        * 2'b01: Divided by 4
                                        * 2'b10: Divided by 6
                                        * 2'b11: Divided by 8
                                        * Note: When programming the DTR mode, only the divider by 4/8 is allowed. */
    uint32_t rsvd             : 2;  /* Reserved */
    uint32_t spi_clk_mode     : 1;  /* spi clk mode at the IDLE state */ /* Default Value 0 */
                                        /* 0: For mode0, sck_out will be low at the IDLE state.
                                        * 1: For mode3, sck_out will be high at the IDLE state. */
    uint32_t rsvd1            : 2;  /* Reserved */
    uint32_t XIP_port_idle    : 1;  /* XIP port idle status */ /* Default Value 0 */
    uint32_t abort            : 1;  /* Flush all commands/FIFOs and reset the state machine */ /* Default Value 0 */
                                        /* When abort occurs, users must fill commands again. This bit will be automatically cleared to zero.
                                        * It is strongly suggested that there is no need to set this bit when setting bit 20 (Change port). */
    uint32_t rsvd2            : 7;  /* Reserved */
    uint32_t rdy_loc          : 3;  /* Busy bit of the SPI status */ /* Default Value 0 */
                                        /* Host polls this busy bit and is ready at the HW read status.
                                        * 3!|b000 ~ 3!|b111: Bit 0 ~ bit 7 */
    uint32_t rsvd3            : 1;  /* Reserved */
    uint32_t XIP_port_sel     : 1;  /* XIP port selection */ /* Default Value HwCfg */
                                        /* Read SPI Flash data from the XIP port
                                        * 0: Command slave port
                                        * 1: XIP port
                                        * When the bit is toggled, FTSPI020 will automatically issue the abort function.
                                        * Users must wait until the abort function is finished to issue a command. */
    uint32_t rsvd4            : 11; /* Reserved */
} kdrv_spif_cr_t;

//#define SPI020REG_ACTIMER   (SPI_FTSPI020_PA_BASE+0x14) /* AC Timing Register */
typedef struct {
    uint32_t cs_delay         : 4;  /* Default Value 0xF */
                                    /* cs delay is the time from inactive cs to active cs, which means that the timing is from high to low.
                                    * Please follow the specification for details.
                                    * The unit is the sck_out period, and the default setting is 16 sck_out periods.
                                    * 4!|b0000 ~ 4!|b1111: 1 cycle ~ 16 cycles */
    uint32_t rsvd             : 28; /* Reserved */
} kdrv_spif_actr_t;

//#define SPI020REG_STATUS    (SPI_FTSPI020_PA_BASE+0x18) /* Status Register */
typedef struct {
    uint32_t TXFIFO_Ready     : 1;  /* TxFIFO ready status */ /* Default Value 1 */
                                    /* When TxFIFO is ready, it indicates that TxFIFO will be empty,
                                    * and users can transfer the data into TxFIFO until it is full. */
    uint32_t RXFIFO_Ready     : 1;  /* RxFIFO ready status */ /* Default Value 0 */
                                    /* When RxFIFO is ready, it indicates that RxFIFO is full,
                                    * and the remained data in RxFIFO will be less than the RXFIFO depth and this is the last data. */
    uint32_t rsvd             : 30; /* Reserved */
} kdrv_spif_sr_t;

//#define SPI020REG_INTERRUPT (SPI_FTSPI020_PA_BASE+0x20)
typedef struct {
    uint32_t DMA_EN           : 1;  /* Enable the DMA handshake */ /* Default Value 0 */
                                    /* Note: Please disable this bit first before switching from the command-based slave port to the XIP port. */
    uint32_t cmd_cmplt_intr_en: 1;  /* Command complete interrupt enable */ /* Default Value 0 */
                                    /* 1!|b0: No interrupt
                                    * 1!|b1: Enable command complete interrupt */
    uint32_t rsvd             : 6;  /* Reserved */
    uint32_t TXFIFO_THOD      : 2;  /* Default Value 0 */
                                    /* This signal is used to set the trigger level for the TxFIFO threshold interrupt.
                                    * The unit is WORD.
                                    * 2'b00 2(if TXFIFO_DEPTH is 8) 4(if TXFIFO_DEPTH is 16) 8(if TXFIFO_DEPTH is 32)
                                    * 2'b01 4(if TXFIFO_DEPTH is 8) 8(if TXFIFO_DEPTH is 16) 16(if TXFIFO_DEPTH is 32)
                                    * 2'b10 6(if TXFIFO_DEPTH is 8) 12(if TXFIFO_DEPTH is 16) 24(if TXFIFO_DEPTH is 32) */
    uint32_t rsvd1            : 2;  /* Reserved */
    uint32_t RXFIFO_THOD      : 2;  /* Default Value 0 */
                                    /* This signal is used to set the trigger level for the RxFIFO threshold interrupt for the DMA handshake mode.
                                    * The unit is WORD.
                                    * 2'b00 2(if RXFIFO_DEPTH is 8) 4(if RXFIFO_DEPTH is 16) 8(if RXFIFO_DEPTH is 32)
                                    * 2'b01 4(if RXFIFO_DEPTH is 8) 8(if RXFIFO_DEPTH is 16) 16(if RXFIFO_DEPTH is 32)
                                    * 2'b10 6(if RXFIFO_DEPTH is 8) 12(if RXFIFO_DEPTH is 16) 24(if RXFIFO_DEPTH is 32) */
    uint32_t rsvd2            : 18; /* Reserved */
} kdrv_spif_icr_t;

//#define SPI020REG_INTR_ST   (SPI_FTSPI020_PA_BASE+0x24) /* Interrupt Status Register */
typedef struct {
    uint32_t cmd_cmplt_sts    : 1;  /* Default Value 0 */
                                    /* Command complete status will be set when the command is complete. */
    uint32_t rsvd             : 31; /* Reserved */
} kdrv_spif_isr_t;

//#define SPI020REG_READ_ST   (SPI_FTSPI020_PA_BASE+0x28) /* SPI Read Status Register */
typedef struct {
    uint32_t SPI_read_status  : 8;  /* Default Value 0 */
                                    /* Host issues the read SPI Flash status command and stores the return data at this register.
                                    * Users can read this register to check the SPI Flash status. */
    uint32_t rsvd             : 24; /* Reserved */
} kdrv_spif_spisr_t;

//#define SPI020REG_FLASH_SIZE   (SPI_FTSPI020_PA_BASE+0x2C) /* SPI Flash Size Register */
/* This register will exist when the hardware configuration, FTSPI020_XIP_PORT, is set to ON. */
typedef struct {
    uint32_t SPI_Flash_Size   : 32; /* Default Value HwCfg */
                                    /* For the direct address mapped function, the address of system is always larger than the address of the SPI Flash.
                                     * Users need to mask the higher bits of the system address to fit the SPI Flash size. */
} kdrv_spif_spifsr_t;

//#define SPI020REG_XIP_CMD   (SPI_FTSPI020_PA_BASE+0x30) /* XIP Command Word */
/* This register will exist when the hardware configuration, FTSPI020_XIP_PORT, is set to ON. */
typedef struct {
    uint32_t dum_2nd_cyc      : 8;  /* Second dummy state cycle for the XIP port */
    uint32_t operation_mode   : 3;  /* SPI operate mode for the XIP port */
                                    /* 3'b000: Serial mode
                                    * 3'b001: Dual mode
                                    * 3'b010: Quad mode
                                    * 3'b011: dual_io mode
                                    * 3'b100: quad_io mode
                                    * Others: Reserved */
    uint32_t address_length   : 1;  /* This register decides the SPI Flash address byte number.
                                    * Users can set this register to decide the address 3 bytes or 4 bytes for the XIP port.
                                    * 1!|b0: 3-byte address
                                    * 1!|b1: 4-byte address */
    uint32_t instruction_code : 8;  /* Instruction code */
                                    /* Users can set this code to execute the SPI Flash command for XIP read.
                                    * If this byte set to 0, XIP  will transfer in the 1-bit mode. */
    uint32_t qd_io_mode_mode  : 8;  /* Quad/Dual I/O mode code for the XIP port */
                                    /* Users have to issue the code, for example, 0xB0, as the mode bit because every command needs instruction code
                                    * while the XIP port is reading. The code depends on the data sheet of the user Flash devices. */
    uint32_t qd_io_mode_en    : 1;    /* Quad/Dual I/O mde allw the address to transmit in 2-bit or 4-bit, deending on the data sheet of the user devices.
                                    * 1!|b0: Disable the Quad/Dual I/O mode
                                    * 1!|b1: Enable the Quad/Dual I/O mode */
    uint32_t start_cs         : 2;  /* To select which SPI Flash chp t  read */ /* Default Value 0 */
                                    /* 2'b00: cs 0
                                    * 2'b01: cs 1
                                    * 2'b10: cs 2
                                    * 2'b11: cs 3 */
    uint32_t rsvd             : 1;  /* Reserved */
} kdrv_spif_xipcmd_t;


//#define SPI020REG_VERSION   (SPI_FTSPI020_PA_BASE+0x50) /* Revision Register */
typedef struct {
    uint32_t revision_number  : 32; /* It indicates this IP version. */
} kdrv_spif_version_t;

//#define SPI020REG_FEATURE   (SPI_FTSPI020_PA_BASE+0x54) /* Feature Register */
typedef struct {
    uint32_t TXFIFO_DEPTH     : 8;  /* FTSPI020 TXFIFO depth */
    uint32_t RXFIFO_DEPTH     : 8;  /* FTSPI020 RXFIFO depth */
    uint32_t rsvd             : 3;  /* Reserved */
    uint32_t AXI_ID_DW        : 5;  /* AXI ID width */
                                    /* 0: No ID port.
                                     * 1 ~ 19: ID width. */
    uint32_t DTR_MODE         : 1;  /* FTSPI020 DTR mode */
                                    /* 0: Disable.
                                     * 1: Enable. */
    uint32_t CLK_MODE         : 1;  /* FTSPI020 c0lck mode */
                                    /* 0: ASYN mode.
                                     * 1: SYNC mode. */
    uint32_t rsvd1            : 3;  /* Reserved */
    uint32_t XIP_PORT         : 1;  /* FTSPI020 XIP prt */
                                    /* 0: Only command-based slave port.
                                     * 1: Command-based slave port and XIP ports. */
    uint32_t HOST_IF_DW       : 1;  /* FTSPI020 host inteface data wdth */
                                    /* 0: Data width is 32 bits.
                                     * 1: Data width is 64 bits. */
    uint32_t HOST_IF          : 1;  /* FTSPI020 host interface */
                                    /* 0: AHB I/Fs
                                     * 1: AXI I/Fs */
} kdrv_spif_feature_t;

//#define SPI020REG_DATAPORT  (SPI_FTSPI020_PA_BASE+0x100) /* Data Port Registe */
typedef struct {
    uint32_t data_port        : 32; /* Data port register */
                                    /* Users can read/write data from the data port. */
} kdrv_spif_dp_t;

typedef volatile union {
    uint32_t array[7];
    struct
    {
        union
        {
            struct
            {
                uint32_t    kdrv_spif_cmd0;
                uint32_t    kdrv_spif_cmd1;
                uint32_t    kdrv_spif_cmd2;
                uint32_t    kdrv_spif_cmd3;
                uint32_t    kdrv_spif_cr;
                uint32_t    kdrv_spif_actr;
                uint32_t    kdrv_spif_sr;
            } dw;                         //double word

            struct
            {
                kdrv_spif_cmd0_t    kdrv_spif_cmd0;
                kdrv_spif_cmd1_t    kdrv_spif_cmd1;
                kdrv_spif_cmd2_t    kdrv_spif_cmd2;
                kdrv_spif_cmd3_t    kdrv_spif_cmd3;
                kdrv_spif_cr_t      kdrv_spif_cr;
                kdrv_spif_actr_t    kdrv_spif_actr;
                kdrv_spif_sr_t      kdrv_spif_sr;
            } bf;
        };
    } st;
} U_regSPIF_ctrl;

typedef volatile union {
    uint32_t array[5];
    struct
    {
        union
        {
            struct
            {
                uint32_t    kdrv_spif_icr;
                uint32_t    kdrv_spif_isr;
                uint32_t    kdrv_spif_spisr;
                uint32_t    kdrv_spif_spifsr;
                uint32_t    kdrv_spif_xipcmd;
            } dw;                         //double word
            struct
            {
                kdrv_spif_icr_t     kdrv_spif_icr;
                kdrv_spif_isr_t     kdrv_spif_isr;
                kdrv_spif_spisr_t   kdrv_spif_spisr;
                kdrv_spif_spifsr_t  kdrv_spif_spifsr;
                kdrv_spif_xipcmd_t  kdrv_spif_xipcmd;
            } bf;                         //double word
        };
    } st;
} U_regSPIF_irq;

typedef volatile union {
    uint32_t array[2];
    struct
    {
        union
        {
            struct
            {
                uint32_t kdrv_spif_version;
                uint32_t kdrv_spif_feature;
            } dw;                         //double word
            struct
            {
                kdrv_spif_version_t kdrv_spif_version;
                kdrv_spif_feature_t kdrv_spif_feature;
            } bf;                         //double word
        };
    } st;
} U_regSPIF_info;

typedef volatile union {
    struct
    {
        uint32_t      kdrv_spif_dp;
    } dw;                         //double word
    struct
    {
        kdrv_spif_dp_t      kdrv_spif_dp;
    } bf;                         //double word
} U_regSPIF_data;

#define regSPIF_ctrl ((volatile U_regSPIF_ctrl *)SPIF_REG_BASE)
#define regSPIF_irq  ((volatile U_regSPIF_irq *)(SPIF_REG_BASE+0x20))
#define regSPIF_info ((volatile U_regSPIF_info *)(SPIF_REG_BASE+0x50))
#define regSPIF_data ((volatile U_regSPIF_data *)(SPIF_REG_BASE+0x100))


/* for SPI020REG_CONTROL */
#define SPI020_ABORT        BIT8
#define SPI020_CLK_MODE     BIT4
#define SPI020_CLK_DIVIDER  (BIT0|BIT1)
/* for SPI020REG_STATUS */
#define SPI020_RXFIFO_READY BIT1
#define SPI020_TXFIFO_READY BIT0
/* for SPI020REG_INTERRUPT */
#define SPI020_cmd_cmplt_intr_en BIT1
#define SPI020_DMA_EN       BIT0
/* for SPI020REG_INTR_ST */
#define SPI020_CMD_CMPLT    BIT0
/* for SPI020REG_FEATURE */
#define SPI020_RX_DEPTH     0xFF00
#define SPI020_TX_DEPTH     0x00FF


/*******************************************
 * for command definition
 ********************************************/
//#if FLASH_4BYTES_CMD_EN
/*******************************************
 * set 4Bytes command (0xB7)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_B7_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_B7_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_B7_CMD2      0x0
/* Set command word 4, instrction code = 0x06, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_B7_CMD3      (0xB7000002|SPI020_CE_VALUE|SPI020_INTR_CFG)



/*******************************************
 * for 4Bytes read data command (0x13)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_13_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_13_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_13_CMD2      0x0
/* Set command word 4, instrction code = 0x13, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_13_CMD3      (0x13000000|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * for 4Bytes read data command (0x0C)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_0C_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_0C_CMD1      0x01080004  //bessel:change value from 0x01000004 to 0x01080004(Fast Read instruction need to add eight "dummy"clocks after 24-bit address)
/* Set command word 2, set data count by input parameter */
#define SPI020_0C_CMD2      0x0
/* Set command word 4, instrction code = 0x0C, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_0C_CMD3      (0x0C000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes page write command (0x12)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_12_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_12_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_12_CMD2      0x0
/* Set command word 4, instrction code = 0x12, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_12_CMD3      (0x12000002|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * for 4Bytes Quad page write command (0x34)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_34_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_34_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_34_CMD2      0x0
/* Set command word 4, instrction code = 0x34, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_34_CMD3      (0x34000042|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes sector erase command (0x21):Sector Erase(4K)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_21_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_21_CMD1      0x01000004
/* Set command word 2, set data count to 0 */
#define SPI020_21_CMD2      0x0
/* Set command word 4, instrction code = 0x21, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_21_CMD3      (0x21000002|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * for 4Bytes block erase command (0xDC):64KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_DC_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_DC_CMD1      0x01000004
/* Set command word 2, set data count to 0 */
#define SPI020_DC_CMD2      0x0
/* Set command word 4, instrction code = 0xDC, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_DC_CMD3      (0xDC000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes quad read data command for windond device (0x6C)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_6C_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 4 */
#define SPI020_6C_CMD1      0x01080004
/* Set command word 2, set data count by input parameter */
#define SPI020_6C_CMD2      0x0
/* Set command word 4, instrction code = 0x6C, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_6C_CMD3      (0x6C000040|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes 2xIO read data command for MXIC device (0xBC)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_BC_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 4, dum_1st_cyc = 0, address length = 4 */
#define SPI020_BC_CMD1      0x01040004
/* Set command word 2, set data count by input parameter */
#define SPI020_BC_CMD2      0x0
/* Set command word 4, instrction code = 0xBC, contiune read = 0,
   start_ce = ??, spi mode = 3, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_BC_CMD3      (0xBC000060|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes 4xIO read data command device (0xEC)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_EC_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 4 */
#define SPI020_EC_CMD1    0x01060004
/* Set command word 2, set data count by input parameter */
#define SPI020_EC_CMD2      0x0
/* Set command word 4, instrction code = 0xEB, contiune read = 0,
   start_ce = ??, spi mode = 4, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_EC_CMD3      (0xEC000080|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes dual read data command for windond device (0x3C)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_3C_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 3 */
#define SPI020_3C_CMD1    0x01080004
/* Set command word 2, set data count by input parameter */
#define SPI020_3C_CMD2      0x0
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_3C_CMD3      (0x3C000020|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************************
 * for Quad page write command with 4byte address (0x34)
 ********************************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_34_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_34_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_34_CMD2      0x0
/* Set command word 4, instrction code = 0x32, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_34_CMD3      (0x34000042|SPI020_CE_VALUE|SPI020_INTR_CFG)

//#endif // end of, #if FLASH_4BYTES_CMD_EN


/*******************************************
 * Exit 4Bytes command (0xE9)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_E9_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_E9_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_E9_CMD2      0x0
/* Set command word 4, instrction code = 0x06, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_E9_CMD3      (0xE9000002|SPI020_CE_VALUE|SPI020_INTR_CFG)



/*******************************************
 * for SFDP command (0x5A)
 * JEDEC STANDARD
 * Serial Flash Discoverable Parameters
********************************************/
#define SPI020_5A_CMD0      0x0C
#define SPI020_5A_CMD1      0x01080003
#define SPI020_5A_CMD2      0x8
#define SPI020_5A_CMD3      (0x5A000008)

//---flash information----
#define FLASH_SIGNATURE             (0x50444653)
#define FLASH_PAGE_SIZE_256_CODE    (0x8)
#define FLASH_PAGE_SIZE_X_CODE      (0xFF)

/*******************************************
 * for read chip id command (0x9F)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_9F_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_9F_CMD1      0x01020000
/* Set command word 2, set data count to 3 */
#define SPI020_9F_CMD2      0x3
/* Set command word 4, instrction code = 0x9F, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_9F_CMD3      (0x9F000000|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for read chip id command (0xAB)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_AB_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_AB_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_AB_CMD2      0x0
/* Set command word 4, instrction code = 0xAB, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_AB_CMD3      (0xAB000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for read chip id command (0xB9)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_B9_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_B9_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_B9_CMD2      0x0
/* Set command word 4, instrction code = 0xAB, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_B9_CMD3      (0xB9000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x05)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_05_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_05_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_05_CMD2      0x0
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
#define SPI020_05_CMD3      (0x05000004|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x05)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_05_CMD0_w      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_05_CMD1_w      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_05_CMD2_w      0x1
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
#define SPI020_05_CMD3_w      (0x05000008|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x35)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_35_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_35_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_35_CMD2      0x1
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
/* shantai, 2014/5/2. 0x35 command can not use read status by hardware. (i.e. bit3 can not be 1) */
#define SPI020_35_CMD3      (0x35000008|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for read status register 3 command (0x15)
 ********************************************/
#define SPI020_15_CMD0      0x0
#define SPI020_15_CMD1      0x01000000
#define SPI020_15_CMD2      0x1
#define SPI020_15_CMD3      (0x15000008|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for write status register 3 command (0x11)
 ********************************************/
#define SPI020_11_CMD0      0x0
#define SPI020_11_CMD1      0x01000000
#define SPI020_11_CMD2      0x1
#define SPI020_11_CMD3      (0x11000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for write status register 3 command (0x31)
 ********************************************/
#define SPI020_31_CMD0      0x0
#define SPI020_31_CMD1      0x01000000
#define SPI020_31_CMD2      0x1
#define SPI020_31_CMD3      (0x31000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for read data command (0x03)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_03_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_03_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_03_CMD2      0x0
/* Set command word 4, instrction code = 0x03, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_03_CMD3      (0x03000000|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for read data command (0x0B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_0B_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_0B_CMD1      0x01080003  //bessel:change value from 0x01000003 to 0x01080003(Fast Read instruction need to add eight "dummy"clocks after 24-bit address)
/* Set command word 2, set data count by input parameter */
#define SPI020_0B_CMD2      0x0
/* Set command word 4, instrction code = 0x03, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_0B_CMD3      (0x0B000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for Fast DTR read data command (0x0D)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_0D_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_0D_CMD1      0x01060003
/* Set command word 2, set data count by input parameter */
#define SPI020_0D_CMD2      0x0
/* Set command word 4, instrction code = 0x0D, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 1, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_0D_CMD3      (0x0D000010|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for Dual IO DTR read data command (0xBD)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_BD_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_BD_CMD1      0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_BD_CMD2      0x0
/* Set command word 4, instrction code = 0xBD, contiune read = 0,
   start_ce = ??, spi mode = 3, DTR mode = 1, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_BD_CMD3      (0xBD000070|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for Quad IO DTR read data command (0xED)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_ED_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_ED_CMD1      0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_ED_CMD2      0x0
/* Set command word 4, instrction code = 0x0D, contiune read = 0,
   start_ce = ??, spi mode = 4, DTR mode = 1, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_ED_CMD3      (0xED000090|SPI020_CE_VALUE|SPI020_INTR_CFG)




/*******************************************
 * for GD Clear SR Flags command (0x30)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_30_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_30_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_30_CMD2      0x0
/* Set command word 4, instrction code = 0x30, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_30_CMD3      (0x30000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for write enable command (0x06)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_06_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_06_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_06_CMD2      0x0
/* Set command word 4, instrction code = 0x06, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_06_CMD3      (0x06000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for write disable command (0x04)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_04_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_04_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_04_CMD2      0x0
/* Set command word 4, instrction code = 0x04, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_04_CMD3      (0x04000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for page write command (0x02)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_02_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_02_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_02_CMD2      0x0
/* Set command word 4, instrction code = 0x02, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_02_CMD3      (0x02000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for Quad page write command (0x32)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_32_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_32_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_32_CMD2      0x0
/* Set command word 4, instrction code = 0x32, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_32_CMD3      (0x32000042|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for chip erase command (0xC7):Erase all
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_C7_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_C7_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_C7_CMD2      0x0
/* Set command word 4, instrction code = 0xC7, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_C7_CMD3      (0xC7000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for chip erase command (0x60):Erase all
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_60_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_60_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_60_CMD2      0x0
/* Set command word 4, instrction code = 0x60, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_60_CMD3      (0x60000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for block erase command (0xD8):64KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_D8_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_D8_CMD1      0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_D8_CMD2      0x0
/* Set command word 4, instrction code = 0xD8, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_D8_CMD3      (0xD8000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for block erase command (0x52):32KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_52_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
 dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_52_CMD1      0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_52_CMD2      0x0
/* Set command word 4, instrction code = 0xD8, contiune read = 0,
 start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
 status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_52_CMD3      (0x52000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for sector erase command (0x20):Sector Erase(4K)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_20_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_20_CMD1      0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_20_CMD2      0x0
/* Set command word 4, instrction code = 0x20, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_20_CMD3      (0x20000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for dual read data command for windond device (0x3B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_3B_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 3 */
#define SPI020_3B_CMD1    0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_3B_CMD2      0x0
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_3B_CMD3      (0x3B000020|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for quad read data command for windond device (0x6B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_6B_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 3 */
#define SPI020_6B_CMD1    0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_6B_CMD2      0x0
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_6B_CMD3      (0x6B000040|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 2xIO read data command for MXIC device (0xBB)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_BB_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 4, dum_1st_cyc = 0, address length = 3 */
#define SPI020_BB_CMD1    0x01040003
/* Set command word 2, set data count by input parameter */
#define SPI020_BB_CMD2      0x0
/* Set command word 4, instrction code = 0xBB, contiune read = 0,
   start_ce = ??, spi mode = 3, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_BB_CMD3      (0xBB000060|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4xIO read data command device (0xEB)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_EB_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_EB_CMD1    0x01060003
/* Set command word 2, set data count by input parameter */
#define SPI020_EB_CMD2      0x0
/* Set command word 4, instrction code = 0xEB, contiune read = 0,
   start_ce = ??, spi mode = 4, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_EB_CMD3      (0xEB000080|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for EWSR (enable-write-status-register) command (0x50)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_50_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_50_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_50_CMD2      0x0
/* Set command word 4, instrction code = 0x50, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_50_CMD3      (0x50000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for WRSR (write-status-register) command (0x01)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_01_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_01_CMD1      0x01000000
/* Set command word 2, set data count to 1 */
#define SPI020_01_CMD2      0x2
/* Set command word 4, instrction code = 0x01, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_01_CMD3      (0x01000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for REMS (read electronic manufacturer & device ID) command (0x90)
 ********************************************/
/* Set command word 0x000001, output the manufacturer ID first, the second byte is device ID */
#define SPI020_90_CMD0      0x0 //0x01 -> 0x00
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_90_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_90_CMD2      0x02 //0x4 //0x04 -> 0x02
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_90_CMD3      (0x90000000|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * 0x66 reset enable
 ********************************************/

//enable reset
#define SPI020_66_CMD0      0x0 //0x01 -> 0x00
#define SPI020_66_CMD1      0x01000000
#define SPI020_66_CMD2      0x00
#define SPI020_66_CMD3      (0x66000002|SPI020_CE_VALUE|SPI020_INTR_CFG)


#define SPI020_66_CMD0_ORG      0x0 //0x01 -> 0x00
#define SPI020_66_CMD1_ORG      0x01000000
#define SPI020_66_CMD2_ORG      0x00
#define SPI020_66_CMD3_ORG      (0x66000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

// reset device
#define SPI020_99_CMD0      0x0
#define SPI020_99_CMD1      0x01000000
#define SPI020_99_CMD2      0x00
#define SPI020_99_CMD3      (0x99000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for structure definition
 ********************************************/
/* For system information. Be careful, this must be 4 alignment */
typedef struct {
    uint8_t   reserved;       /* original -> FLASH_CTL_SPI010/FLASH_CTL_SPI020 - for spi010/spi020 */
    uint8_t   manufacturer;   /* Manufacturer id */
    uint16_t  flash_id;       /* device id */
    uint32_t  flash_size;     /* flash size in byte */
    uint8_t   support_dual;   /* flash support dual read mode or not */
    uint8_t   sys_version;    /* system version, get from SYS_VERSION_ADDR */
    uint8_t   dev_mode;       /* current usb link type, 0/1/2/3 for unknow/SS/HS/FS */
    uint8_t   vender_specific; /* specific purpose for vendor */
} spi_flash_t;

//--------------------------------------------------
// Function Prototypes
//--------------------------------------------------

#endif/* __SPI020_H__ */

