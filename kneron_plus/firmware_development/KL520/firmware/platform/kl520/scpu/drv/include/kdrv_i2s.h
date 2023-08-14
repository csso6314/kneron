#ifndef _CPE_SSP_H
#define _CPE_SSP_H

#include "base.h"

typedef enum 
{
  SSP_AS_MASTER = 0,
  SSP_AS_SLAVE = 1
}SSP_MODE_T;

#define SSP_CONTROL0                0x0
#define SSP_CONTROL1                0x4
#define SSP_CONTROL2                0x8
#define SSP_STATUS                  0xC
#define SSP_INT_CONTROL             0X10
#define SSP_INT_STATUS              0x14
#define SSP_DATA                    0x18
#define SSP_INFO					0x1C
#define SSP_ACLINK_SLOT_VALID		0x20

                        	      
/*  Control register 0  */

#define SSP_Format                 	0x0000  //TI SSP
#define SPI_Format              	0x1000  //Motororal SPI
#define Microwire_Format            0x2000  //NS Microwire
#define I2S_Format                	0x3000  //Philips's I2S
#define AC97_Format               	0x4000  //Intel AC-Link

#define SSP_FSDIST                  0x300
#define SSP_LBM                     0x80  /* loopback mode */
#define SSP_LSB                     0x40  /* LSB first */
#define SSP_FSPO_LOW                0x20  /* Frame sync atcive low */
#define SSP_FSPO_HIGH               0x0   /* Frame sync atcive high */


#define SSP_DATAJUSTIFY             0x10  /* data padding in front of serial data */

#define SSP_OPM_MS					0x03		
#define SSP_OPM_SL					0x01
#define SSP_OPM_MSST                0xC  /* Master stereo mode */
#define SSP_OPM_MSMO                0x8  /* Master mono mode */
#define SSP_OPM_SLST                0x4  /* Slave stereo mode */
#define SSP_OPM_SLMO                0x0  /* Slave mono mode */


#define SSP_SCLKPO_HIGH             0x2  /* SCLK Remain HIGH */
#define SSP_SCLKPO_LOW              0x0  /* SCLK Remain LOW */
#define SSP_SCLKPH_HALFCLK          0x1  /* Half CLK cycle */
#define SSP_SCLKPH_ONECLK           0x0  /* One CLK cycle */


/*  Control Register 1 */

#define SSP_PDL                     0xFF000000	/* paddinf data length */
#define SSP_SDL                     0x7F0000		/* Serial data length(actual data length-1) */
#define SSP_CLKDIV                  0xFFFF	 	/*  clk divider */


/* Control Register 2 */
#define SSP_TXEN					(1<<8)	 /* Transmit function enable */
#define SSP_RXEN					(1<<7)	 /* Receive function enable */
#define SSP_SSPRST					(1<<6)
#define SSP_ACCRST                  0x20 	 /* AC-Link Cold Reset Enable */
#define SSP_ACWRST                  0x10 	 /* AC-Link Warm Reset Enable */
#define SSP_TXFCLR                  0x8	 	 /* TX FIFO Clear */
#define SSP_RXFCLR                  0x4	 	 /* RX FIFO Clear */
#define SSP_TXDOE                   0x2	 	 /* TX Data Output Enable */
#define SSP_SSPEN                   0x1		 /* SSP Enable */

/* Status register
 */
#define SSP_TFVE                    0x3f000	 /* Tx FIFO Valid Entries */
#define SSP_RFVE                    0x3f0	 /* Rx FIFO Valid Entries */
 
#define SSP_BUSY                    0x4		 /* Busy for recv or tx */
#define SSP_TFNF                    0x2		 /* TX FIFO Not Full */
#define SSP_RFF                     0x1		 /* RX FIFO Full */


/* Interrupr Control register */
#define SSP_TXDMAEN                 0x20	 /* TX DMA Enable */
#define SSP_RXDMAEN                 0x10	 /* RX DMA Enable */
#define SSP_TFIEN                   0x8		 /* TX FIFO Int Enable */
#define SSP_RFIEN                   0x4		 /* RX FIFO Int Enable */
#define SSP_TFURIEN                 0x2		 /* TX FIFO Underrun int enable */
#define SSP_RFORIEN                 0x1		 /* RX FIFO Overrun int enable */

/* Interrupt Status register */
#define SSP_AC97FCI                 0x10   //Added by Peter Liao, 2003/6/3
#define SSP_TFTHI                   0x8		 /* TX FIFO Threshold Interrupt */
#define SSP_RFTHI                   0x4		 /* RX FIFO Threshold Interrupt */
#define TFURI                       0x2		 /* TX FIFO Underrun interrupt */
#define RFORI                       0x1		 /* RX FIFO Overrun interrupt */


/* SSP Design Information Register */
#define SSP_TXFIFO_DEPTH			0xFF0000
#define SSP_RXFIFO_DEPTH			0xFF00
#define SSP_FIFO_WIDTH				0xFF

#define MAX_SSP               0x4      /* ssp device number(include AC97 and I2S) */

#define AC97_SLOT_NUM				  13

#define AC97_TX_BUF_SIZE			16
#define AC97_RX_BUF_SIZE			16


/* Codec external Clock: Power Management => Synchronous Serial Port Clock divider Register */
#define SSPCK_BASE					CPE_PWM_BASE+0x30
#define SSP1CKDIV					0xF
#define I2SCKDIV					0xF0
#define AC97CKDIV					0xF00
#define SSP2CKDIV					0xF000


/* I2S data format */
#define UDA1345TS_I2S     	0
#define W6631TS_I2S       	1
#define NORMAL_I2S        	2
#define FSA0AC108_I2S	  	3
#define WM8510_I2S		  	4
#define PCM3793_I2S			5
#define WM8731S_I2S			6
#define WM9081_I2S			7

#define I2S_FIFO_WIDTH 				16

/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

extern void fLib_I2S_Init(u32, u32, SSP_MODE_T);
extern void fLib_WriteSSP(u32,u32);
extern u32 fLib_ReadSSP(u32 );

extern void fLib_SetSSPFrame(u32,u32);
extern void fLib_SetSSPFramePolar(u32,u32);

extern void fLib_SetSSPOPMode(u32,u32);
extern void fLib_SetSSPDataLen(u32,u32);
extern void fLib_SetSSPpaddingDataLen(u32,u32);
extern void fLib_SetSSPClkdiv(u32,u32);

extern void fLib_SSPClearTxFIFO(u32 );
extern void fLib_SSPClearRxFIFO(u32 );

extern u32 fLib_ReadSSPStatus(u32 );

extern void fLib_SetSSPSCLKPO(u32,u32);
extern void fLib_SetSSPSCLKPH(u32,u32);

extern u32 fLib_ReadSSPIntStatus(u32 );
extern void fLib_SetSSP_TXFIFO(u32,u32,u32);
extern void fLib_SetSSP_RXFIFO(u32,u32,u32);
extern void fLib_SetSSP_DMA(u32,u32,u32);
extern void fLib_SetSSP_FIFO_Threshold(u32,u32,u32);
extern void fLib_SetSSP_WarmReset(u32 );
extern void fLib_SetSSP_ColdReset(u32 );
extern u32 fLib_ReadSSP32Bit(u32 );

extern void fLib_SetSSP_Enable(u32,int );
extern void fLib_SetSSP_IntMask(u32,int );

//extern void fLib_SSP_Init(u32);//bessel:we don't use this initialization function
extern u32 fLib_SSP_GetRxFIFOValidEntries(u32 base_addr);
 
extern void fLib_InitAC97(u32);
extern void fLib_AC97_SetSlotValidReg(u32,u32);
extern int fLib_AC97_ReadRegister(u32,u32,u16*);
extern void fLib_AC97_WriteRegister(u32,u32,u32);
extern void fLib_AC97_WriteData(u32,u32*,u32);
extern void fLib_AC97_ReadData(u32,u32*,u32);
extern u32 fLib_AC97_ReadOneWordData(u32 );

extern int fLib_AC97_ReadRegisterEx(u32,u32,u16*);
extern void fLib_AC97_WriteRegisterEx(u32,u32,u32);

extern int fLib_ReturnTxFIFO_Count(u32 );
extern int fLib_ReturnRxFIFO_Count(u32 );
extern u32 fLib_SSP_GetTxFIFOLen(u32 );
extern void fLib_SetSSP_SDL_Write(u32 ,u32 ,u32);
extern u32 fLib_SSP_busy(u32 base_addr);

extern void delay(int ticks);
#endif
