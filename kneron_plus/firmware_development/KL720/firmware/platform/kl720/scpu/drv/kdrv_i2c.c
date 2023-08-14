/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */

/******************************************************************************
*  Filename:
*  ---------
*  kdrv_i2c.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  This is i2c driver
*
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

#include <stdlib.h>
#include <cmsis_os2.h>
#include "kdrv_scu.h"
#include "kdrv_clock.h" // for kdrv_delay_us
#include "kdrv_i2c.h"
#include "regbase.h"
#include "base.h"
#include "system_config.h"
#include "kdrv_cmsis_core.h"

#define I2C_ENABLE_THREAD_SYNC // enable I2C thread synchronization
//#define I2C_DEBUG

/************************************************************************
*                     I2C Register Releated Macros                      *
************************************************************************/
typedef volatile union U_regIIC
{
    struct
    {
        uint32_t CR;            //0x00  I2C Control Register (CR)
        uint32_t SR;            //0x04  I2C Status Register (SR)
        uint32_t CDR;           //0x08  I2C Clock Division Register (CDR)
        uint32_t DR;            //0x0C  I2C Data Register (DR)
        uint32_t SAR;           //0x10  I2C Address Register (AR)
        uint32_t TGSR;          //0x14  I2C Setup/Hold Time and Glitch Suppression Setting Register (TGSR)
        uint32_t BMR;           //0x18  I2C Bus Monitor Register (BMR)
        uint32_t BSTMR;         //0x1C  I2C Burst Mode Register (BSTMR)
    }dw;    //double word

    struct
    {
        //I2C Control Register (CR, Offset: 0x00)
        uint32_t I2C_RST        :1;     //Reset the I2C controller
        uint32_t I2C_EN         :1;     //Enable the I2C bus interface controller
        uint32_t MST_EN         :1;     //Enable the I2C controller clock output for the master mode operation
        uint32_t GC_EN          :1;     //Enable the I2C controller to respond to a general call message as a slave
        uint32_t CR_START_      :1;     //I2C controller initiates a start condition when the I2C bus is idle or initiates a repeated start condition after transferring the next data byte on the I2C bus in the master mode.
        uint32_t CR_STOP_       :1;     //I2C controller initiates a stop condition after transferring the next data byte on the I2C bus when I2C is in the master mode.
        uint32_t ACK            :1;     //0: ACK  1: NACK
        uint32_t TB_EN          :1;     //When Transfer Byte Enable (TB_EN) is set, the FTIIC010 I2C controller is ready to receive or transmit one byte. Otherwise, the FTIIC010 I2C controller will insert the wait state by pulling SCLout low in the I2C bus.
        uint32_t BSTTHODI_EN    :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller read data reaches the threshold (bits [5:0] of register offset 0x1C) in the master RX burst mode.
        uint32_t TDI_EN         :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller DR register has transmitted/received one data byte from the I2C bus, or has transmitted/received the total data count (bits [13:8] of register offset 0x1C) in the master RX/TX burst mode.
        uint32_t NACKI_EN       :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller detects the non-ACK responses from the I2C bus.
        uint32_t STOPI_EN       :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller detects a stop condition happening on the I2C bus.
        uint32_t SAMI_EN        :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller detects a slave address that matches the SAR register or a general call address (When GC_EN is set).
        uint32_t ALI_EN         :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller loses the arbitration in the master mode.
        uint32_t STARTI_EN      :1;     //If it is set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller detects a start condition on the I2C bus.
        uint32_t SCL_LOW        :1;     //If it is set, SCLout will be tied to '0'. For a normal case, it is suggested setting this bit to '0'.
        uint32_t SDA_LOW        :1;     //If it is set, SDAout will be tied to '0'. For a normal case, it is suggested setting this bit to '0'.
        uint32_t Test_bit       :1;     //Special test mode This bit must be set to '0'.
        uint32_t Arb_off        :1;     //This register is used to ignore the arbitration lose detection in the single I2C master environment.
        uint32_t HS_mode        :1;     //This register is used to switch between HS-mode and F/S-mode. When users set this register to 1, the I2C controller will use CDR[31:20] (0x08) to generate the corresponding HS-mode SCL.
        uint32_t HSI_EN         :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller detects an HS-mode code in the slave mode.
        uint32_t SBI_EN         :1;     //If set, this bit will enable the FTIIC010 I2C controller to interrupt the host processor when the FTIIC010 I2C controller detects a START byte in the slave mode.
        uint32_t rsvd           :2;     //Reserved
        uint32_t FLOW_CTRL_EN   :2;     //2'h0: No action. 2'h1: Master TX Burst Mode.   2'h2: Master RX Burst Mode.
        uint32_t rsvd0          :6;     //Reserved

        //I2C Status Register (SR, Offset: 0x04)
        uint32_t RW             :1;     //0: Maxter in the TX mode, 1: Maxter in the RX mode,
        uint32_t rsvd1          :1;     //Reserved
        uint32_t I2CB           :1;     //This signal will be set when the I2C controller is busy, i.e. during the time period between START and STOP.
        uint32_t BB             :1;     //This signal will be set when the I2C bus is busy, but the I2C controller is not involved in the transaction.
        uint32_t BSTTHOD_SR     :1;     //This signal will be set when the FTIIC010 I2C controller read data reaches the threshold (bits [5:0] of register offset 0x1C) in the master RX burst mode.
        uint32_t TD             :1;     //This signal will be set when data finishes transferring (TX/RX) a byte, or the total data count (bits [13:8] of register offset 0x1C) is finished in the master RX/TX burst mode.
        uint32_t NACK           :1;     //This signal will be set when the I2C controller detects non-ACK responses from the slave device after transmitting one byte of data when the I2C controller is in the master mode.
        uint32_t STOP           :1;     //This signal will be set when the I2C controller detects a stop condition in the I2C bus.
        uint32_t SAM            :1;     //When the I2C controller is in the slave mode, this signal will be set while the I2C controller is receiving slave address that matches the address in the Address Register (AR), and it should assert while GC is hitting.
        uint32_t GC             :1;     //This signal will be set when the I2C controller receives slave address that matches the general-call address, and the I2C controller is in the slave mode.
        uint32_t AL             :1;     //This signal will be set when the I2C controller loses arbitration in the master mode.
        uint32_t START          :1;     //This signal will be set when the I2C controller detects a start condition on the I2C bus.
        uint32_t rsvd2          :10;    //Reserved
        uint32_t HSS            :1;     //This signal will be set when the I2C controller detects an HS-mode code on the I2C bus.
        uint32_t SBS            :1;     //This signal will be set when the I2C controller detects a START byte on the I2C bus.
        uint32_t rsvd3          :8;     //Reserved

        //I2C Clock Division Register (CDR, Offset: 0x08)
        uint32_t COUNT          :20;    /*Counter value for F/S-mode, is used to generate an I2C clock (SCLout) from the internal bus clock PCLK. The relation between PCLK and the I2C bus clock (SCLout) is shown in the following equation:
                                            SCLout (Hz) = PCLK (Hz)/( 2 * COUNT + GSR + 4) Please note the minimum value of COUNT is equal to or larger than GSR + TSR + 4, where GSR, TSR are from register offset 0x14.
                                            For example:
                                            If PCLK is 20 MHz, COUNT is 7, GSR is 2, and then SCLout is 1 MHz.*/
        uint32_t COUNTH         :8;     /*Counter value for the HS mode is used to generate an I2C clock (SCLout) from the internal bus clock, PCLK. The relation between PCLK and the I2C bus clock (SCLout) is as shown in the following equation:
                                            SCLout (Hz) = PCLK (Hz)/(2 * COUNTH + GSR + 4)
                                            Please note that the minimum value of COUNTH is equal to or larger than GSR + TSR + 4, where GSR and TSR are from the register offset 0x14.
                                            For example:
                                            If PCLK is 60 MHz, COUNTH is 7, and GSR is 2, and then SCLout is 3.0 MHz.*/
        uint32_t DUTY_OFFSET    :4;     /*This offset can change the I2C clock duty. Users must configure
                                            this register offset to fit the LOW period of the SCL clock that is claimed in I2C bus specification.
                                            High period = ((COUNT) - DUTY_OFFSET) + GSR + 3) * PCLK
                                            Low period = ((COUNT) + DUTY_OFFSET) + 1) * PCLK period
                                            Please note that COUNT must be larger than DUTY_OFFSET and ((COUNT) - DUTY_OFFSET) is equal to or larger than 2.*/

        //I2C Data Register (DR, Offset: 0x0C)
        uint32_t DR             :8;     //Buffer for the I2C bus data transmission and reception
        uint32_t rsvd4          :24;    //Reserved

        //I2C Address Register (AR, Offset: 0x10)
        uint32_t ADDR           :7;     /*In the 7-bit addressing mode (ADDR10EN = '0'), the 7-bit address is for the I2C controller transmitted in  the master burst mode, or for the I2C controller gives a response when I2C operates in the slave mode.*/
        uint32_t ADDR2          :3;     //The most significant 3-bit address to which the I2C controller transmits in the master RX/TX burst mode, or gives a response when I2C operates in the slave  mode if ADDR10EN = '1'.   When ADDR10EN = '0', the I2C controller ignores these three bits.
        uint32_t rsvd5          :2;     //Reserved
        uint32_t ADDR10EN       :1;     //Set to 1 to enable 10-bit addressing mode
        uint32_t M2BIDX_EN      :1;     //Set to 1 to transfer 2nd index bytes
        uint32_t rsvd6          :2;     //Reserved
        uint32_t MEM_IDX        :8;     //1st transmitted index for the master RX/TX burst mode
        uint32_t MEM_IDX2       :8;     //2nd transmitted index for the master RX/TX burst mode

        //I2C Set/Hold Time and Glitch Suppression Setting Register (TGSR, Offset: 0x14)
        uint32_t TSR            :10;    //These bits define the delay values of the PCLK clock cycles that the data or acknowledgement will be driven into the I2C SDA bus after I2C SCL bus goes low. The actual delay value is GSR + TSR + 4. Please refer to Figure 2-1 for more details. Please note that TSR cannot be set to '0'
        uint32_t GSR            :4;     //These bits define the values of the PCLK clock period when the I2C bus interface has the built-in glitch suppression logic. Glitch is suppressed according to the 'GSR * PCLK' clock period.
        uint32_t rsvd7          :18;    //Reserved

        //2C Bus Monitor Register (BMR, Offset: 0x18)
        uint32_t SDAin          :1;     //This bit continuously reflects the value of the SDAin pin.
        uint32_t SCLin          :1;     //This bit continuously reflects the value of the SCLin pin.
        uint32_t rsvd8          :30;    //Reserved
        //I2C Burst Mode Register (BSTMR, Offset: 0x1C)
        uint32_t BSTTHOD        :6;     //These bits must be less than or equal to BSTTDC. Please note that BSTTHOD is only valid in the master RX burst mode. Please do not set them to 0.
        uint32_t rsvd9          :2;     //Reserved
        uint32_t BSTTDC         :6;     //These bits are the total burst data count, which must be less than or equal to BUFHW. Please do not set them to 0.
        uint32_t rsvd10         :2;     //Reserved
        uint32_t BUFHW          :3;     //Buffer depth 3'h1: 2 bytes, 3'h2: 4 bytes, 3'h3: 8 bytes, 3'h4: 16 bytes, 3'h5: 32 bytes
        uint32_t rsvd11         :13;    //Reserved
    }bf;    //bit-field
}U_regIIC;

#define IIC_REG_0  ((U_regIIC*) I2C0_REG_BASE)
#define IIC_REG_1  ((U_regIIC*) I2C1_REG_BASE)
#define IIC_REG_2  ((U_regIIC*) I2C2_REG_BASE)
U_regIIC* reg_addr[3]={IIC_REG_0, IIC_REG_1, IIC_REG_2};


#define I2C_MODE_SLAVE          0
#define I2C_MODE_MASTER         1
// Constant macro definition for the status of I2C state machine
#define I2C_HS_CODE         4
#define I2C_SLAVE_INIT      0
#define I2C_SAM_MATCH       1
#define I2C_SLAVE_TX_DATA   2
#define I2C_SLAVE_RX_DATA   3// FTIIC010 as slave device, the current state of I2C_Slave_ISR()
#define I2C_BUFFER_SIZE     256
static volatile int gI2CState;
static volatile int gIndex = 0; //index of required to access from I2C_DataBuf
volatile uint32_t I2C_DataBuf[I2C_BUFFER_SIZE];
static volatile kdrv_i2c_ctx_t i2c_ctx[3] = {0};
static uint32_t slave_port_id;
#ifdef I2C_DEBUG
void i2c_dump_register(kdrv_i2c_ctrl_t ctrl_id)
{
    info_msg("APB clock = %d\n", APB_CLOCK);
    info_msg("(0x00) control register = 0x%x\n",    reg_addr[ctrl_id]->CR);
    info_msg("(0x04) status register = 0x%x\n",     reg_addr[ctrl_id]->SR);
    info_msg("(0x08) clock division = 0x%x\n",      reg_addr[ctrl_id]->CDR);
    info_msg("(0x0C) data register = 0x%x\n",       reg_addr[ctrl_id]->DR);
    info_msg("(0x10) address register = 0x%x\n",    reg_addr[ctrl_id]->SAR);
    info_msg("(0x14) TGSR register = 0x%x\n",       reg_addr[ctrl_id]->TGSR);
    info_msg("(0x18) bus monitor = 0x%x\n",         reg_addr[ctrl_id]->BMR);
    info_msg("(0x1C) burst mode = 0x%x\n",          reg_addr[ctrl_id]->BSTMR);
}
#endif

static void i2c_power(kdrv_i2c_ctrl_t ctrl_id, int on)
{
    switch(ctrl_id)
    {
        case KDRV_I2C_CTRL_0:
            regSCU->bf.i2c0_pclk = on;
            break;
        case KDRV_I2C_CTRL_1:
            regSCU->bf.i2c1_pclk = on;
            break;
        case KDRV_I2C_CTRL_2:
            regSCU->bf.i2c2_pclk = on;
            break;
        default:
            break;
    }
    kdrv_delay_us(500);
}

static kdrv_status_t i2c_polling_completion(kdrv_i2c_ctrl_t ctrl_id, bool ignore_NAK)
{
    // polling SR and take actions correspondingly
    for (int i = 0; i < 10000; i++)
    {
        uint32_t sr_status = reg_addr[ctrl_id]->dw.SR;
        reg_addr[ctrl_id]->dw.SR = sr_status;
        if (!ignore_NAK && (sr_status & SR_NACK))
            return KDRV_STATUS_I2C_DEVICE_NACK;
        else if (sr_status & (SR_AL))
            return KDRV_STATUS_I2C_BUS_BUSY;
        else if (sr_status & SR_TD)
            return KDRV_STATUS_OK;
    }
    return KDRV_STATUS_I2C_TIMEOUT;
}

#if I2C_INTERRUPT_ENABLE
void kdrv_i2c_irqhandler(uint32_t id)
{
    volatile uint32_t sr_status     = reg_addr[id]->dw.SR;
    volatile uint32_t cr_status     = reg_addr[id]->dw.CR;
    #ifdef USE_EVENT_FLAG
    volatile osEventFlagsId_t threadId  = i2c_ctx[id].tid;
    #else
    volatile osThreadId_t threadId  = i2c_ctx[id].tid;
    #endif
    i2c_slv_cb_fr_isr_t isr_cb      = i2c_ctx[id].cb;        
    reg_addr[id]->dw.SR = sr_status;    //w1c
    if(reg_addr[id]->bf.MST_EN == 1)
    {
        if(threadId != NULL) {
            #ifdef USE_EVENT_FLAG
            osEventFlagsSet(threadId, (sr_status&FLAGS_I2C_MST_ALL_EVENTS));
            #else
            osThreadFlagsSet(threadId, (sr_status&FLAGS_I2C_MST_ALL_EVENTS));
            #endif
        }
    }
    else
    {
        if(sr_status & SR_SAM)
        {
            // the I2C controller receives slave address that matches 
            // the address in the Slave Register (SAR) when the I2C controller is in the slave mode
            if ((sr_status & SR_RW)) // slave (tx)
            {
                gI2CState = I2C_SLAVE_TX_DATA;
                if (gIndex >= I2C_BUFFER_SIZE)
                {
                    gIndex = 0;
                }

                reg_addr[id]->dw.DR = I2C_DataBuf[gIndex++];
            }
            else// rx 
            {
                gI2CState = I2C_SAM_MATCH;
            }
            reg_addr[id]->dw.CR = CR_TBEN|cr_status;
            
        }
        //Host read process 
        else if ((gI2CState == I2C_SLAVE_TX_DATA) && (sr_status & SR_TD))
        {
            if ((sr_status & SR_STOP) || (sr_status & SR_NACK))
            {
                gI2CState = I2C_SLAVE_INIT;
            }
            else
            {
                if (gIndex > I2C_BUFFER_SIZE)
                {
                    gIndex = 0;
                }

                reg_addr[id]->dw.DR = I2C_DataBuf[gIndex++];
                reg_addr[id]->dw.CR = CR_TBEN|cr_status;
            }
        }

        
        //Host write process 
        else if ((gI2CState==I2C_SAM_MATCH) && (sr_status&SR_TD))//First Rx data tranfer finish
        {
            // device address
            gIndex = 0;
            gI2CState = I2C_SLAVE_RX_DATA;
            I2C_DataBuf[gIndex++] = reg_addr[id]->dw.DR;
            reg_addr[id]->dw.CR = CR_TBEN|cr_status;
        }
        else if (((gI2CState == I2C_SLAVE_RX_DATA) && (sr_status & SR_TD)) || ((gI2CState == I2C_SLAVE_RX_DATA) && (sr_status & SR_STOP)))
        {
            if (sr_status & SR_STOP)
            {
                gI2CState = I2C_SLAVE_INIT;
            }
            else
            {
                if (gIndex > I2C_BUFFER_SIZE)
                gIndex = 0;

                I2C_DataBuf[gIndex++] = reg_addr[id]->dw.DR;
                reg_addr[id]->dw.CR = CR_TBEN|cr_status;
            }
        }
        else if ((sr_status & SR_START) || (sr_status & SR_STOP))
        {};
        
        if(isr_cb != NULL)
        {
            if(gI2CState == I2C_SLAVE_TX_DATA)
                isr_cb(I2C_SLV_Tx_HOST_READ, (uint32_t*)&I2C_DataBuf[gIndex-1]);
            else if(gI2CState == I2C_SLAVE_RX_DATA)
                isr_cb(I2C_SLV_Rx_HOST_WRITE, (uint32_t*)&I2C_DataBuf[gIndex-1]);
        }
        else if(threadId != NULL)
        {
            #ifdef USE_EVENT_FLAG
            osEventFlagsSet(threadId, (sr_status & FLAGS_I2C_SLV_ALL_EVENTS));    
            #else
            osThreadFlagsSet(threadId, (sr_status & FLAGS_I2C_SLV_ALL_EVENTS));    
            #endif
        }
    }
    
}
void I2C0_IRQ_Handler(void)
{
    kdrv_i2c_irqhandler(0);
}
void I2C1_IRQ_Handler(void)
{
    kdrv_i2c_irqhandler(1);
}
void I2C2_IRQ_Handler(void)
{
    kdrv_i2c_irqhandler(2);
}
void kdrv_i2c_set_nvic(kdrv_i2c_ctrl_t id)
{
    switch(id)
    {
        case KDRV_I2C_CTRL_0:
            NVIC_ClearPendingIRQ(I2C0_IRQn);
            NVIC_EnableIRQ(I2C0_IRQn);
            break;
        case KDRV_I2C_CTRL_1:
            NVIC_ClearPendingIRQ(I2C1_IRQn);
            NVIC_EnableIRQ(I2C1_IRQn);
            break;
        case KDRV_I2C_CTRL_2:
            NVIC_ClearPendingIRQ(I2C2_IRQn);
            NVIC_EnableIRQ(I2C2_IRQn);
            break;
        default:
            break;
    }
}

static kdrv_status_t kdrv_i2c_wait(kdrv_i2c_ctrl_t ctrl_id, bool ignore_NAK)
{
    kdrv_status_t ret = KDRV_STATUS_OK;
    uint32_t flags;
    if(i2c_ctx[ctrl_id].tid  != NULL)
    {
        #ifdef USE_EVENT_FLAG
        flags = osEventFlagsWait(i2c_ctx[ctrl_id].tid,FLAGS_I2C_MST_ALL_EVENTS, osFlagsWaitAny , 2000);
        #else
        flags = osThreadFlagsWait(FLAGS_I2C_MST_ALL_EVENTS, osFlagsWaitAny, 2000);
        #endif

        if(!ignore_NAK && (flags & SR_NACK))
            return KDRV_STATUS_I2C_DEVICE_NACK;
        else if (flags & SR_AL)
            return KDRV_STATUS_I2C_BUS_BUSY;
        else if (flags & SR_TD)
            return KDRV_STATUS_OK;
    }
    else
    {
        ret = i2c_polling_completion(ctrl_id, ignore_NAK);
    }

    return ret;
}
#endif

static bool i2c_waiting_for_bus_available(kdrv_i2c_ctrl_t ctrl_id)
{
    for (int i = 0; i < 10000; i++)
    {
        if(reg_addr[ctrl_id]->bf.BB)
            continue;
        else
            return true;
    }
    return false;
}

// data1 and num1 are first part of data
// data2 and num2 are first part of data
// doing this is for kdrv_i2c_write_register(), kind of workaround way of implementation
// data 1 must not be NULL, num1 must be greater than 0
// data 2 can be NULL, num2 can be 0 if no 2nd part of data
static kdrv_status_t i2c_tx_rx(kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr,
                               uint8_t *data1, uint32_t num1, uint8_t *data2, uint32_t num2, bool isRead, bool with_STOP)
{
    kdrv_status_t status = KDRV_STATUS_OK;
    bool ignore_NAK = false;

#ifdef I2C_ENABLE_THREAD_SYNC
    static osMutexId_t mutex_i2c = NULL;

    if (mutex_i2c == NULL)
    {
        mutex_i2c = osMutexNew(NULL);
#ifdef I2C_DEBUG
        if (mutex_i2c == NULL)
            printf("kdrv_i2c: %s: create mutex failed\n", __FUNCTION__);
#endif
    }

    osStatus_t osts = osMutexAcquire(mutex_i2c, osWaitForever);
#ifdef I2C_DEBUG
    if (osts != osOK)
        printf("kdrv_i2c: %s: mutex lock failed, osStatus_t error = %d\n", __FUNCTION__, status);
#endif

#endif

    do
    {
        if (!i2c_waiting_for_bus_available(ctrl_id))
        {
            status = KDRV_STATUS_I2C_BUS_BUSY;
            break;
        }

        // send slave address and check status

        // write address byte into DR
        // TODO: support 10-bit address
        uint8_t byte_addr = (uint8_t)((slave_addr << 1) & 0xFE);
        if (isRead)
            byte_addr |= 0x1;

        reg_addr[ctrl_id]->dw.DR = byte_addr;

#if I2C_INTERRUPT_ENABLE
        // send out slave address with START condition by setting CR
        #ifndef USE_EVENT_FLAG
        i2c_ctx[ctrl_id].tid = osThreadGetId();
        #endif
        if(i2c_ctx[ctrl_id].tid  != NULL)
            reg_addr[ctrl_id]->dw.CR = (CR_I2C_EN | CR_MST_EN | CR_TBEN | CR_START | CR_DRIRQ | CR_NAKRIRQ |CR_ALIRQ);
        else
            reg_addr[ctrl_id]->dw.CR = (CR_I2C_EN | CR_MST_EN | CR_TBEN | CR_START);
        // wait for complete status
        status = kdrv_i2c_wait(ctrl_id, ignore_NAK);
#else
        // send out slave address with START condition by setting CR
        reg_addr[ctrl_id]->dw.CR = (CR_I2C_EN | CR_MST_EN | CR_TBEN | CR_START);

        // wait for complete status
        status = i2c_polling_completion(ctrl_id, ignore_NAK);
#endif
        if (status != KDRV_STATUS_OK)
        {
            if (status == KDRV_STATUS_I2C_DEVICE_NACK)
            {
                reg_addr[ctrl_id]->bf.CR_STOP_ = 1;
            }
            break;
        }

        if (!i2c_waiting_for_bus_available(ctrl_id))
        {
            status = KDRV_STATUS_I2C_BUS_BUSY;
            break;
        }

        uint32_t num = num1 + num2;
        uint8_t *data = data1;

        // send or receive data
        for (int i = 0; i < num; i++)
        {
            uint32_t ctrl_flag;
            ignore_NAK = false;
        #if I2C_INTERRUPT_ENABLE
            if(i2c_ctx[ctrl_id].tid  != NULL)
                ctrl_flag = (CR_I2C_EN | CR_MST_EN | CR_TBEN | CR_DRIRQ | CR_NAKRIRQ |CR_ALIRQ);
            else
                ctrl_flag = (CR_I2C_EN | CR_MST_EN | CR_TBEN);
        #else
            ctrl_flag = (CR_I2C_EN | CR_MST_EN | CR_TBEN);
        #endif
            if (with_STOP && (i == (num - 1)))
            {
                ctrl_flag |= CR_STOP;
                if (isRead)
                {
                    ctrl_flag |= CR_NAK;
                    ignore_NAK = true;
                }
            }

            if (!isRead) // write data to the DR
            {
                reg_addr[ctrl_id]->dw.DR = *data;
            }

            // start transmission
            reg_addr[ctrl_id]->dw.CR = ctrl_flag;


            // check status
            // wait for complete status
#if I2C_INTERRUPT_ENABLE
            status = kdrv_i2c_wait(ctrl_id, ignore_NAK);
#else
            status = i2c_polling_completion(ctrl_id, ignore_NAK);
#endif
            if (status != KDRV_STATUS_OK)
            {
                if (status == KDRV_STATUS_I2C_DEVICE_NACK)
                {
                    reg_addr[ctrl_id]->bf.CR_STOP_ = 1;
                }
                break;
            }

            if (isRead) // read data
            {
                *data = (uint8_t)(0xFF & reg_addr[ctrl_id]->dw.DR);
            }

            ++data;
            // switch to 2nd part of data buffer, so strange ...
            if ((i + 1) == num1)
                data = data2;
        }
    } while (0);

#ifdef I2C_ENABLE_THREAD_SYNC
    osMutexRelease(mutex_i2c);
#endif
    return status;
}

/*******************************************************************/
/***                    Public APIs                              ***/
/*******************************************************************/
kdrv_status_t kdrv_i2c_slave_open(i2c_slv_cb_fr_isr_t cb)
{
    #ifdef USE_EVENT_FLAG
    i2c_ctx[slave_port_id].tid  = osEventFlagsNew(NULL);
    #else
    i2c_ctx[slave_port_id].tid  = osThreadGetId();
    #endif
    i2c_ctx[slave_port_id].cb   = cb;
    return KDRV_STATUS_OK;
}
kdrv_status_t kdrv_i2c_set_slave(i2c_attr_context* attr_ctx)
{
    uint32_t dev_addr;
    slave_port_id = attr_ctx->i2c_port;
    dev_addr = attr_ctx->i2c_devaddr;
    reg_addr[slave_port_id]->dw.SAR = dev_addr;
    reg_addr[slave_port_id]->dw.CR = 0;
    reg_addr[slave_port_id]->dw.CR = CR_I2C_EN|CR_DRIRQ|CR_STOPIRQ|CR_SAMIRQ;//|CR_STARTI_EN;

    return KDRV_STATUS_OK;
}
kdrv_status_t kdrv_i2c_set_attribute(i2c_attr_context* attr_ctx)
{
    kdrv_i2c_ctrl_t ctrl_id = (kdrv_i2c_ctrl_t)attr_ctx->i2c_port;
    uint32_t bus_speed = attr_ctx->i2c_speed;
    if ((ctrl_id >= TOTAL_KDRV_I2C_CTRL) || (bus_speed > KDRV_I2C_SPEED_1M))
        return KDRV_STATUS_INVALID_PARAM;
    i2c_power(ctrl_id, 1);

    // reset i2c controller
    reg_addr[ctrl_id]->bf.I2C_RST = 1;

    int i;
    for (i = 0; i < 500; i++)
        if (!(reg_addr[ctrl_id]->bf.I2C_RST))
            break;

    if (i >= 500)
    {
        return KDRV_STATUS_ERROR;
    }

    // set bus speed, SCL = PCLK / (2 * COUNT  + GSR + 4)

    // set GSR and TSR value
    reg_addr[ctrl_id]->bf.GSR = 1;
    reg_addr[ctrl_id]->bf.TSR = 1;

    // select CDR Count value depending on bus speed
    uint32_t count_bits;
    switch (bus_speed)
    {
    case KDRV_I2C_SPEED_100K:
        count_bits = CDR_COUNT_100KHZ;
        break;
    case KDRV_I2C_SPEED_200K:
        count_bits = CDR_COUNT_200KHZ;
        break;
    case KDRV_I2C_SPEED_400K:
        count_bits = CDR_COUNT_400KHZ;
        break;
    case KDRV_I2C_SPEED_1M:
        count_bits = CDR_COUNT_1MHZ;
        break;
    default:
        // could be user specified clock in Hz
        count_bits = ((APB_CLOCK) / (2 * (uint32_t)bus_speed) - (GSR_VALUE / 2) - 2);
        break;
    }
    reg_addr[ctrl_id]->dw.CDR = count_bits;       // not care about COUNTH and DUTY bits
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_i2c_set_parameter(i2c_attr_context* attr_ctx)
{
    kdrv_status_t ret;
    ret = kdrv_i2c_set_attribute(attr_ctx);
    if(ret != KDRV_STATUS_OK)
        return ret;
    if(attr_ctx->i2c_mode == I2C_MODE_SLAVE)
    {
        kdrv_i2c_set_slave(attr_ctx);
        for(uint32_t i = 0; i< I2C_BUFFER_SIZE; i++)
        {
            I2C_DataBuf[i] = i;//0xAA;
        }
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_i2c_initialize(uint32_t num, i2c_attr_context* attr_ctx)
{
    i2c_attr_context* i2c_attr_ctx;
    for(uint32_t i = 0 ; i < num ; i++)
    {
        i2c_attr_ctx = attr_ctx + i;
        kdrv_i2c_set_parameter(i2c_attr_ctx);
        #if I2C_INTERRUPT_ENABLE
        kdrv_i2c_set_nvic((kdrv_i2c_ctrl_t)i2c_attr_ctx->i2c_port);
        #ifdef USE_EVENT_FLAG
        i2c_ctx[(kdrv_i2c_ctrl_t)i2c_attr_ctx->i2c_port].tid = osEventFlagsNew(NULL);
        #endif
        #endif
    }
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_i2c_uninitialize(kdrv_i2c_ctrl_t ctrl_id)
{
    i2c_power(ctrl_id, 0);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_i2c_write_register(kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr,
                                uint16_t reg, uint16_t reg_size, uint16_t len, uint16_t *data)
{
    uint8_t reg_data[2];
    int32_t i = 0;

    if (reg_size == 2)
        reg_data[i++] = reg >> 8; // store MSB of reg
    reg_data[i++] = reg & 0xff;

    return i2c_tx_rx(ctrl_id, slave_addr, reg_data, reg_size, (uint8_t *)data, len, false, true);
}

kdrv_status_t kdrv_i2c_read_register(kdrv_i2c_ctrl_t ctrl_id, uint16_t slave_addr,
                                uint16_t reg, uint16_t reg_size, uint16_t len, uint16_t *data)
{
    uint8_t reg_data[2];
    int32_t i = 0;

    if (reg_size == 2)
        reg_data[i++] = reg >> 8; // store MSB of reg
    reg_data[i++] = reg & 0xff;

    // first write register address to the device without STOP condition
    kdrv_status_t ret = i2c_tx_rx(ctrl_id, slave_addr, reg_data, reg_size, NULL, 0, false, false);

    // then read data from device
    if (ret == KDRV_STATUS_OK)
        ret = i2c_tx_rx(ctrl_id, slave_addr, (uint8_t*)data, len, NULL, 0, true, true);

    return ret;
}

