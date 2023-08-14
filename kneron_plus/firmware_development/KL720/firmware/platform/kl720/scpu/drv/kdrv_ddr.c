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
#include "io.h"
#include "kdrv_ddr.h"
#include "kdrv_scu_ext.h"

//#define DDRC_ENABLE_GEXT
//#define DDRC_ENABLE_DQS_DRIFT_DETECT_COMPENSATE
//#define DDRC_DBG
#ifdef DDRC_DBG
#define ddrc_dbg(__format__, ...) printf(LOG_CUSTOM, __format__, ##__VA_ARGS__)
#else
#define ddrc_dbg(__format__, ...)
#endif

#define DDR_CTL_BASE    (DDR_REG_BASE + 0x0000)
#define DDR_PUB_BASE    (DDR_REG_BASE + 0x2000)
#define DDR_SYS_BASE    (DDR_REG_BASE + 0x3000)
#define DDR3_TOP_CH0_BASE (0x80000000)

//#define WITH_CA_TRAIN
#define _RFSHTMG_800    0x0030801a
#define _INIT3_800      0x00830004
#define _DRAMTMG0_800   0x070a0d08
#define _DRAMTMG1_800   0x0002020d
#define _DRAMTMG2_800   0x02030606
#define _DRAMTMG3_800   0x00505000
#define _DRAMTMG4_800   0x04020205
#define _DRAMTMG5_800   0x01010303
#define _DRAMTMG6_800   0x01010003
#define _DRAMTMG14_800  0x01010003
#define _ZQCTL0_800     0x40480012
#define _DFITMG0_800    0x02838202
#define _ODTCFG_800     0x06010710
#define _PUB_PLLCR_800  0x3         //PUB_PLLCR[20:19]FRQSEL
#define _PUB_MR1_800    0x00000083
#define _PUB_MR2_800    0x00000004
#define _PUB_DTPR0_800  0x04110b04
#define _PUB_DTPR1_800  0x1014040a
#define _PUB_DTPR2_800  0x00030038
#define _PUB_DTPR3_800  0x01800301
#define _PUB_DTPR4_800  0x00340803
#define _PUB_DTPR5_800  0x001c0a04
#define _PUB_PGCR2_800  0x00005ff0
#define _PUB_ZQCR_800   0x1
#define _ACLCDLR_800    0x0

#define _RFSHTMG_1333   0x0051802c
#define _INIT3_1333     0x00030018
#define _DRAMTMG0_1333  0x0a11160e
#define _DRAMTMG1_1333  0x00030215
#define _DRAMTMG2_1333  0x03050708
#define _DRAMTMG3_1333  0x00505000
#define _DRAMTMG4_1333  0x06020407
#define _DRAMTMG5_1333  0x01010505
#define _DRAMTMG6_1333  0x01010004
#define _DRAMTMG14_1333 0x0000002f
#define _ZQCTL0_1333    0x4078001e
#define _DFITMG0_1333   0x02888205
#define _ODTCFG_1333    0x0603081c
#define _PUB_PLLCR_1333 0x1         //PUB_PLLCR[20:19]FRQSEL
#define _PUB_MR1_1333   0x00000003
#define _PUB_MR2_1333   0x00000018
#define _PUB_DTPR0_1333 0x071c1205
#define _PUB_DTPR1_1333 0x1b22040a
#define _PUB_DTPR2_1333 0x0005005e
#define _PUB_DTPR3_1333 0x01800402
#define _PUB_DTPR4_1333 0x00570f05
#define _PUB_DTPR5_1333 0x002e1005
#define _PUB_PGCR2_1333 0x0000a0f0
#define _PUB_ZQCR_1333  0x4
#define _ACLCDLR_1333   0x0

#define _RFSHTMG_1600   0x00618034
#define _INIT3_1600     0x0043001a
#define _DRAMTMG0_1600  0x0b141a11
#define _DRAMTMG1_1600  0x0003031a
#define _DRAMTMG2_1600  0x03060809
#define _DRAMTMG3_1600  0x00506000
#define _DRAMTMG4_1600  0x08020408
#define _DRAMTMG5_1600  0x01010606
#define _DRAMTMG6_1600  0x01010004
#define _DRAMTMG14_1600 0x00000038
#define _ZQCTL0_1600    0x40900024
#define _DFITMG0_1600   0x028a8205
#define _ODTCFG_1600    0x06030924
#define _PUB_PLLCR_1600 0x1         //PUB_PLLCR[20:19]FRQSEL
#define _PUB_MR1_1600   0x00000043
#define _PUB_MR2_1600   0x0000001a
#define _PUB_DTPR0_1600 0x08221606
#define _PUB_DTPR1_1600 0x2028040c
#define _PUB_DTPR2_1600 0x00060070
#define _PUB_DTPR3_1600 0x01800502
#define _PUB_DTPR4_1600 0x00680f06
#define _PUB_DTPR5_1600 0x00381406
#define _PUB_PGCR2_1600 0x0000c170
#define _PUB_ZQCR_1600  0x5
#define _ACLCDLR_1600   0x3a

#define _RFSHTMG_1866   0x0071803d          //Ken0827
#define _INIT3_1866     0x0083001c
#define _DRAMTMG0_1866  0x0d181f14
#define _DRAMTMG1_1866  0x0004031e
#define _DRAMTMG2_1866  0x0407090a          //Ken0827
#define _DRAMTMG3_1866  0x00507000
#define _DRAMTMG4_1866  0x09020509
#define _DRAMTMG5_1866  0x01010707
#define _DRAMTMG6_1866  0x01010005
#define _DRAMTMG14_1866 0x00000041
#define _ZQCTL0_1866    0x40a8002a
#define _DFITMG0_1866   0x028c8207
#define _ODTCFG_1866    0x06040a28
#define _PUB_PLLCR_1866 0x0         //PUB_PLLCR[20:19]FRQSEL
#define _PUB_MR1_1866   0x00000083
#define _PUB_MR2_1866   0x0000001c
#define _PUB_DTPR0_1866 0x0a281a07
#define _PUB_DTPR1_1866 0x262f040e
#define _PUB_DTPR2_1866 0x00070083
#define _PUB_DTPR3_1866 0x01800602
#define _PUB_DTPR4_1866 0x007a0f07
#define _PUB_DTPR5_1866 0x00411707
#define _PUB_PGCR2_1866 0x0000e1d1
#define _PUB_ZQCR_1866  0x6
#define _ACLCDLR_1866   0x34

#define _RFSHTMG_2133   0x00818046
#define _INIT3_2133     0x00c3001e
#define _DRAMTMG0_2133  0x0e1b2316
#define _DRAMTMG1_2133  0x00040422
#define _DRAMTMG2_2133  0x04080a0b
#define _DRAMTMG3_2133  0x00508000
#define _DRAMTMG4_2133  0x0a02060b
#define _DRAMTMG5_2133  0x01010808
#define _DRAMTMG6_2133  0x01010005
#define _DRAMTMG14_2133 0x0000004b
#define _ZQCTL0_2133    0x40c00030
#define _DFITMG0_2133   0x028f8207
#define _ODTCFG_2133    0x06040a30
#define _PUB_PLLCR_2133 0x0
#define _PUB_MR1_2133   0x000000c3
#define _PUB_MR2_2133   0x0000001e
#define _PUB_DTPR0_2133 0x0b2d1d08
#define _PUB_DTPR1_2133 0x2b36040f
#define _PUB_DTPR2_2133 0x00080096
#define _PUB_DTPR3_2133 0x01800603
#define _PUB_DTPR4_2133 0x008b0f08
#define _PUB_DTPR5_2133 0x004a1a08
#define _PUB_PGCR2_2133 0x000101d0
#define _PUB_ZQCR_2133  0x7
#define _ACLCDLR_2133   0x31

typedef struct{
    lpddr3_opt  opt_freq;
    uint32_t rfshtmg;
    uint32_t init3;
    uint32_t dramtmg0;
    uint32_t dramtmg1;
    uint32_t dramtmg2;
    uint32_t dramtmg3;
    uint32_t dramtmg4;
    uint32_t dramtmg5;
    uint32_t dramtmg6;
    uint32_t dramtmg14;
    uint32_t zqctl0;
    uint32_t dfitmg0;
    uint32_t odtcfg;
    uint32_t pub_pllcr;
    uint32_t pub_mr1;
    uint32_t pub_mr2;
    uint32_t pub_dtpr0;
    uint32_t pub_dtpr1;
    uint32_t pub_dtpr2;
    uint32_t pub_dtpr3;
    uint32_t pub_dtpr4;
    uint32_t pub_dtpr5;
    uint32_t pub_pgcr2;
    uint32_t pub_zqcr;
    uint32_t acdlr;
}ddr_dynamic_reg;

ddr_dynamic_reg ddr_clk_dynamic_reg[LPDDR3_OPT_MAX] = {
    //AXI_DDR_200 -> LPDDR3_800
    {
        .opt_freq   = LPDDR3_800_,
        .rfshtmg    = _RFSHTMG_800,
        .init3      = _INIT3_800,
        .dramtmg0   = _DRAMTMG0_800,
        .dramtmg1   = _DRAMTMG1_800,
        .dramtmg2   = _DRAMTMG2_800,
        .dramtmg3   = _DRAMTMG3_800,
        .dramtmg4   = _DRAMTMG4_800,
        .dramtmg5   = _DRAMTMG5_800,
        .dramtmg6   = _DRAMTMG6_800,
        .dramtmg14  = _DRAMTMG14_800,
        .zqctl0     = _ZQCTL0_800,
        .dfitmg0    = _DFITMG0_800,
        .odtcfg     = _ODTCFG_800,
        .pub_pllcr  = _PUB_PLLCR_800,
        .pub_mr1    = _PUB_MR1_800,
        .pub_mr2    = _PUB_MR2_800,
        .pub_dtpr0  = _PUB_DTPR0_800,
        .pub_dtpr1  = _PUB_DTPR1_800,
        .pub_dtpr2  = _PUB_DTPR2_800,
        .pub_dtpr3  = _PUB_DTPR3_800,
        .pub_dtpr4  = _PUB_DTPR4_800,
        .pub_dtpr5  = _PUB_DTPR5_800,
        .pub_pgcr2  = _PUB_PGCR2_800,
        .pub_zqcr   = _PUB_ZQCR_800,
        .acdlr      = _ACLCDLR_800,
    },
    //AXI_DDR_333 -> LPDDR3_1333
    {
        .opt_freq   = LPDDR3_1333_,
        .rfshtmg    = _RFSHTMG_1333,
        .init3      = _INIT3_1333,
        .dramtmg0   = _DRAMTMG0_1333,
        .dramtmg1   = _DRAMTMG1_1333,
        .dramtmg2   = _DRAMTMG2_1333,
        .dramtmg3   = _DRAMTMG3_1333,
        .dramtmg4   = _DRAMTMG4_1333,
        .dramtmg5   = _DRAMTMG5_1333,
        .dramtmg6   = _DRAMTMG6_1333,
        .dramtmg14  = _DRAMTMG14_1333,
        .zqctl0     = _ZQCTL0_1333,
        .dfitmg0    = _DFITMG0_1333,
        .odtcfg     = _ODTCFG_1333,
        .pub_pllcr  = _PUB_PLLCR_1333,
        .pub_mr1    = _PUB_MR1_1333,
        .pub_mr2    = _PUB_MR2_1333,
        .pub_dtpr0  = _PUB_DTPR0_1333,
        .pub_dtpr1  = _PUB_DTPR1_1333,
        .pub_dtpr2  = _PUB_DTPR2_1333,
        .pub_dtpr3  = _PUB_DTPR3_1333,
        .pub_dtpr4  = _PUB_DTPR4_1333,
        .pub_dtpr5  = _PUB_DTPR5_1333,
        .pub_pgcr2  = _PUB_PGCR2_1333,
        .pub_zqcr   = _PUB_ZQCR_1333,
        .acdlr      = _ACLCDLR_1333,
    },
    //AXI_DDR_400 -> LPDDR3_1600
    {
        .opt_freq   = LPDDR3_1600_,
        .rfshtmg    = _RFSHTMG_1600,
        .init3      = _INIT3_1600,
        .dramtmg0   = _DRAMTMG0_1600,
        .dramtmg1   = _DRAMTMG1_1600,
        .dramtmg2   = _DRAMTMG2_1600,
        .dramtmg3   = _DRAMTMG3_1600,
        .dramtmg4   = _DRAMTMG4_1600,
        .dramtmg5   = _DRAMTMG5_1600,
        .dramtmg6   = _DRAMTMG6_1600,
        .dramtmg14  = _DRAMTMG14_1600,
        .zqctl0     = _ZQCTL0_1600,
        .dfitmg0    = _DFITMG0_1600,
        .odtcfg     = _ODTCFG_1600,
        .pub_pllcr  = _PUB_PLLCR_1600,
        .pub_mr1    = _PUB_MR1_1600,
        .pub_mr2    = _PUB_MR2_1600,
        .pub_dtpr0  = _PUB_DTPR0_1600,
        .pub_dtpr1  = _PUB_DTPR1_1600,
        .pub_dtpr2  = _PUB_DTPR2_1600,
        .pub_dtpr3  = _PUB_DTPR3_1600,
        .pub_dtpr4  = _PUB_DTPR4_1600,
        .pub_dtpr5  = _PUB_DTPR5_1600,
        .pub_pgcr2  = _PUB_PGCR2_1600,
        .pub_zqcr   = _PUB_ZQCR_1600,
        .acdlr      = _ACLCDLR_1600,
    },
    //AXI_DDR_466 -> LPDDR3_1866
    {
        .opt_freq   = LPDDR3_1866_,
        .rfshtmg    = _RFSHTMG_1866,
        .init3      = _INIT3_1866,
        .dramtmg0   = _DRAMTMG0_1866,
        .dramtmg1   = _DRAMTMG1_1866,
        .dramtmg2   = _DRAMTMG2_1866,
        .dramtmg3   = _DRAMTMG3_1866,
        .dramtmg4   = _DRAMTMG4_1866,
        .dramtmg5   = _DRAMTMG5_1866,
        .dramtmg6   = _DRAMTMG6_1866,
        .dramtmg14  = _DRAMTMG14_1866,
        .zqctl0     = _ZQCTL0_1866,
        .dfitmg0    = _DFITMG0_1866,
        .odtcfg     = _ODTCFG_1866,
        .pub_pllcr  = _PUB_PLLCR_1866,
        .pub_mr1    = _PUB_MR1_1866,
        .pub_mr2    = _PUB_MR2_1866,
        .pub_dtpr0  = _PUB_DTPR0_1866,
        .pub_dtpr1  = _PUB_DTPR1_1866,
        .pub_dtpr2  = _PUB_DTPR2_1866,
        .pub_dtpr3  = _PUB_DTPR3_1866,
        .pub_dtpr4  = _PUB_DTPR4_1866,
        .pub_dtpr5  = _PUB_DTPR5_1866,
        .pub_pgcr2  = _PUB_PGCR2_1866,
        .pub_zqcr   = _PUB_ZQCR_1866,
        .acdlr      = _ACLCDLR_1866,
    },
    //AXI_DDR_533 -> LPDDR3_2133
    {
        .opt_freq   = LPDDR3_2133_,
        .rfshtmg    = _RFSHTMG_2133,
        .init3      = _INIT3_2133,
        .dramtmg0   = _DRAMTMG0_2133,
        .dramtmg1   = _DRAMTMG1_2133,
        .dramtmg2   = _DRAMTMG2_2133,
        .dramtmg3   = _DRAMTMG3_2133,
        .dramtmg4   = _DRAMTMG4_2133,
        .dramtmg5   = _DRAMTMG5_2133,
        .dramtmg6   = _DRAMTMG6_2133,
        .dramtmg14  = _DRAMTMG14_2133,
        .zqctl0     = _ZQCTL0_2133,
        .dfitmg0    = _DFITMG0_2133,
        .odtcfg     = _ODTCFG_2133,
        .pub_pllcr  = _PUB_PLLCR_2133,
        .pub_mr1    = _PUB_MR1_2133,
        .pub_mr2    = _PUB_MR2_2133,
        .pub_dtpr0  = _PUB_DTPR0_2133,
        .pub_dtpr1  = _PUB_DTPR1_2133,
        .pub_dtpr2  = _PUB_DTPR2_2133,
        .pub_dtpr3  = _PUB_DTPR3_2133,
        .pub_dtpr4  = _PUB_DTPR4_2133,
        .pub_dtpr5  = _PUB_DTPR5_2133,
        .pub_pgcr2  = _PUB_PGCR2_2133,
        .pub_zqcr   = _PUB_ZQCR_2133,
        .acdlr      = _ACLCDLR_2133,
    },
};


#define apb_wr(a,v) outw( DDR_CTL_BASE + (a), (v) )
#define apb_rd(a,v) v = inw( DDR_CTL_BASE + (a))

#pragma push
#pragma O0
void add_delay(volatile uint32_t x)
{
    while(x--);
}
#pragma pop
void release_axi_rst(void)
{
    ddr_rst_t *ddr_rst = (ddr_rst_t *)SCU_EXTREG_DDR_RST;
    ddr_rst->ARESETN_DDR = 1;
    ddr_rst->CORE_DDRC_RSTN = 1;
}

void kdrv_ddr_read_delayline_info(void)
{
#ifdef DDRC_DBG
    //dlr delay line register
    volatile uint32_t *dx_dlr_addr[4][8] = {
        {&KDRV_DDR_PHY_REG->DX0LCDLR0, &KDRV_DDR_PHY_REG->DX0LCDLR1, &KDRV_DDR_PHY_REG->DX0LCDLR2, &KDRV_DDR_PHY_REG->DX0LCDLR3, \
            &KDRV_DDR_PHY_REG->DX0LCDLR4, &KDRV_DDR_PHY_REG->DX0LCDLR5, &KDRV_DDR_PHY_REG->DX0MDLR0, &KDRV_DDR_PHY_REG->DX0MDLR1},
        {&KDRV_DDR_PHY_REG->DX1LCDLR0, &KDRV_DDR_PHY_REG->DX1LCDLR1, &KDRV_DDR_PHY_REG->DX1LCDLR2, &KDRV_DDR_PHY_REG->DX1LCDLR3, \
            &KDRV_DDR_PHY_REG->DX1LCDLR4, &KDRV_DDR_PHY_REG->DX1LCDLR5, &KDRV_DDR_PHY_REG->DX1MDLR0, &KDRV_DDR_PHY_REG->DX1MDLR1},
        {&KDRV_DDR_PHY_REG->DX2LCDLR0, &KDRV_DDR_PHY_REG->DX2LCDLR1, &KDRV_DDR_PHY_REG->DX2LCDLR2, &KDRV_DDR_PHY_REG->DX2LCDLR3, \
            &KDRV_DDR_PHY_REG->DX2LCDLR4, &KDRV_DDR_PHY_REG->DX2LCDLR5, &KDRV_DDR_PHY_REG->DX2MDLR0, &KDRV_DDR_PHY_REG->DX2MDLR1},
        {&KDRV_DDR_PHY_REG->DX3LCDLR0, &KDRV_DDR_PHY_REG->DX3LCDLR1, &KDRV_DDR_PHY_REG->DX3LCDLR2, &KDRV_DDR_PHY_REG->DX3LCDLR3, \
            &KDRV_DDR_PHY_REG->DX3LCDLR4, &KDRV_DDR_PHY_REG->DX3LCDLR5, &KDRV_DDR_PHY_REG->DX3MDLR0, &KDRV_DDR_PHY_REG->DX3MDLR1},
    };

    //Display
    ddrc_dbg("|------------------------------------------------------------|\n");
    ddrc_dbg("|                     Show DelayLine Info                    |\n");
    ddrc_dbg("|------------------------------------------------------------|\n");

    for(uint32_t n=0; n<4; ++n)
    {
        ddrc_dbg("============================================================\n");
        ddrc_dbg("Byte %d ---> PUB Addr %p, WLD          =  0x%x \n", n, dx_dlr_addr[n][0], *dx_dlr_addr[n][0]);
        ddrc_dbg("Byte %d ---> PUB Addr %p, WDQD         =  0x%x \n", n, dx_dlr_addr[n][1], *dx_dlr_addr[n][1]);
        ddrc_dbg("Byte %d ---> PUB Addr %p, DQSG         =  0x%x \n", n, dx_dlr_addr[n][2], *dx_dlr_addr[n][2]);
        ddrc_dbg("Byte %d ---> PUB Addr %p, RDQS         =  0x%x \n", n, dx_dlr_addr[n][3], *dx_dlr_addr[n][3]);
        ddrc_dbg("Byte %d ---> PUB Addr %p, RDQS_N       =  0x%x \n", n, dx_dlr_addr[n][4], *dx_dlr_addr[n][4]);
        ddrc_dbg("Byte %d ---> PUB Addr %p, DQSG Status  =  0x%x \n", n, dx_dlr_addr[n][5], *dx_dlr_addr[n][5]);
        ddrc_dbg("Byte %d ---> PUB Addr %p, TPRD         =  0x%x \n", n, dx_dlr_addr[n][6], (*dx_dlr_addr[n][6] & 0xFFF000));  //Target Period
        ddrc_dbg("Byte %d ---> PUB Addr %p, IPRD         =  0x%x \n", n, dx_dlr_addr[n][6], (*dx_dlr_addr[n][6] & 0x0001FF));     //Initial Period
        ddrc_dbg("Byte %d ---> PUB Addr %p, MDLD         =  0x%x \n", n, dx_dlr_addr[n][7], dx_dlr_addr[n][7]);
    }
#endif
}

void kdrv_ddr_pub_read_then_modify_reg(uint32_t phy_addr, uint32_t content, uint32_t mask)
{
    uint32_t read_data;
    apb_rd((0x2000 |(phy_addr<<2)), read_data);
    read_data = ((read_data) & ~mask) | (content & mask);
    apb_wr((0x2000 |(phy_addr<<2)), read_data);
}

void kdrv_ddr_inhibit_vt(bool start)
{
    uint32_t read_data;
    uint32_t wait_stat;
    apb_rd((0x2000 | (0xa<<2)), read_data);//PGCR6
    if(start)
    {
        read_data |= 0x1;
        wait_stat = 1;
    }
    else
    {
        read_data &= 0xFFFFFFFE;
        wait_stat = 0;
    }
    apb_wr((0x2000 | (0xa<<2)), read_data);
    ddrc_dbg("kdrv_ddr_inhibit_vt : set phy_addr = 0xa, PGCR6[0] INHVT = %d\n", start);
    apb_rd((0x2000 | (0xe<<2)), read_data);//PGSR1
    while(((read_data & 0x40000000)>>30) == wait_stat)        //wait for bit 30 VTSTOP;
        apb_rd((0x2000 | (0xe<<2)), read_data);
}

void kdrv_ddr_process_reg_modify(uint32_t phy_addr, uint32_t content, uint32_t mask)
{
    kdrv_ddr_inhibit_vt(1);     //inhibit VT compensation
    kdrv_ddr_pub_read_then_modify_reg(phy_addr, content, mask);
    kdrv_ddr_inhibit_vt(0);     //enable VT compensation
}

#if ((defined DDRC_ENABLE_GEXT) && (!defined DDRC_ENABLE_DQS_DRIFT_DETECT_COMPENSATE))
/* After set DSGCR.DQSGX = 2'b10 to enlarger gate extension
manually shift back the gate delay about 0.25 ui~0.3ui to keep the margin larger.
*/
void kdrv_shift_back_gate_delay_gtext(void)
{
    uint8_t i;
    uint32_t master_delay[4];
    uint32_t left_md_unit[4]; //0.25UI for left shift
    uint32_t gate_delay_cycle[4];
    uint32_t gate_delay_line[4];
    uint32_t mdlr_iprd_mask =  0x1FF;//8:0
    uint32_t dxgtr_dgsl_mask = 0x1F; //4:0
    uint32_t dxlcdlr_dgsgd_mask = 0x1FF;//8:0

    volatile uint32_t *dx_dlr_addr[4][3] = {
        {&KDRV_DDR_PHY_REG->DX0LCDLR2, &KDRV_DDR_PHY_REG->DX0GTR0, &KDRV_DDR_PHY_REG->DX0MDLR0},
        {&KDRV_DDR_PHY_REG->DX1LCDLR2, &KDRV_DDR_PHY_REG->DX1GTR0, &KDRV_DDR_PHY_REG->DX1MDLR0},
        {&KDRV_DDR_PHY_REG->DX2LCDLR2, &KDRV_DDR_PHY_REG->DX2GTR0, &KDRV_DDR_PHY_REG->DX2MDLR0},
        {&KDRV_DDR_PHY_REG->DX3LCDLR2, &KDRV_DDR_PHY_REG->DX3GTR0, &KDRV_DDR_PHY_REG->DX3MDLR0},
    };

    ddrc_dbg("KDRV_DDR_PHY_REG->DX0LCDLR2(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX0LCDLR2, KDRV_DDR_PHY_REG->DX0LCDLR2);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX1LCDLR2(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX1LCDLR2, KDRV_DDR_PHY_REG->DX1LCDLR2);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX2LCDLR2(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX2LCDLR2, KDRV_DDR_PHY_REG->DX2LCDLR2);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX3LCDLR2(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX3LCDLR2, KDRV_DDR_PHY_REG->DX3LCDLR2);

    ddrc_dbg("KDRV_DDR_PHY_REG->DX0GTR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX0GTR0, KDRV_DDR_PHY_REG->DX0GTR0);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX1GTR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX1GTR0, KDRV_DDR_PHY_REG->DX1GTR0);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX2GTR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX2GTR0, KDRV_DDR_PHY_REG->DX2GTR0);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX3GTR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX3GTR0, KDRV_DDR_PHY_REG->DX3GTR0);

    ddrc_dbg("KDRV_DDR_PHY_REG->DX0MDLR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX0MDLR0, KDRV_DDR_PHY_REG->DX0MDLR0);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX1MDLR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX1MDLR0, KDRV_DDR_PHY_REG->DX1MDLR0);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX2MDLR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX2MDLR0, KDRV_DDR_PHY_REG->DX2MDLR0);
    ddrc_dbg("KDRV_DDR_PHY_REG->DX3MDLR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX3MDLR0, KDRV_DDR_PHY_REG->DX3MDLR0);

    for(i = 0; i < 4; i++) 
    {
        master_delay[i] = (*dx_dlr_addr[i][2] & mdlr_iprd_mask);
        left_md_unit[i] = (master_delay[i] >> 2);
        gate_delay_cycle[i] = (*dx_dlr_addr[i][1] & dxgtr_dgsl_mask);
        gate_delay_line[i] = (*dx_dlr_addr[i][0] & dxlcdlr_dgsgd_mask);
    }

    for(i = 0; i < 4; i++) {
        if(gate_delay_line[i] <= left_md_unit[i]) {
            *dx_dlr_addr[i][1] = gate_delay_cycle[i]-1;
            gate_delay_line[i] = master_delay[i];
        }
        *dx_dlr_addr[i][0] = (gate_delay_line[i]-left_md_unit[i]);
        ddrc_dbg("master_delay[%d]:0x%x, left_md_unit[%d]:0x%x, gate_delay_cycle[%d]:0x%x, gate_delay_line[%d]:0x%x\n", i, master_delay[i], i, left_md_unit[i], i, gate_delay_cycle[i], i, gate_delay_line[i]);
    }
    ddrc_dbg("\nrd_data:0x%x\n\n", KDRV_DDR_PHY_REG->DSGCR);
}
#elif ((!defined DDRC_ENABLE_GEXT) && (defined DDRC_ENABLE_DQS_DRIFT_DETECT_COMPENSATE))
void kdrv_dqs_read_drift_detect_and_compensate(void)
{
    uint32_t ret;
    uint32_t rflvt_mask = (1 << 27);
    uint32_t dqsdr0_dftdten_mask = (1 << 0);
    uint32_t dqsdr0_dftupdrd_mask = (0xF << 8); //11:8
    uint32_t dqsdr2_dftmntprd_mask = (0xFFFF << 0);
    uint32_t dqsdr2_dftthrsh_mask = (0xFF << 16);

    //---start drift detect
    //DTCR1.RDVLGDIFF keep default at preset
    //DXnGCR3

    ddrc_dbg("== KDRV_DDR_PHY_REG->DX0GCR3:0x%08x => ", KDRV_DDR_PHY_REG->DX0GCR3);
    KDRV_DDR_PHY_REG->DX0GCR3 = (KDRV_DDR_PHY_REG->DX0GCR3 & ~(rflvt_mask));
    ddrc_dbg("0x%08x ==\n", KDRV_DDR_PHY_REG->DX0GCR3);
    
    ddrc_dbg("== KDRV_DDR_PHY_REG->DX0GCR3:0x%08x => ", KDRV_DDR_PHY_REG->DX1GCR3);
    KDRV_DDR_PHY_REG->DX1GCR3 = (KDRV_DDR_PHY_REG->DX1GCR3 & ~(rflvt_mask));
    ddrc_dbg("0x%08x ==\n", KDRV_DDR_PHY_REG->DX1GCR3);

    ddrc_dbg("== KDRV_DDR_PHY_REG->DX0GCR3:0x%08x => ", KDRV_DDR_PHY_REG->DX2GCR3);
    KDRV_DDR_PHY_REG->DX2GCR3 = (KDRV_DDR_PHY_REG->DX2GCR3 & ~(rflvt_mask));
    ddrc_dbg("0x%08x ==\n", KDRV_DDR_PHY_REG->DX2GCR3);

    ddrc_dbg("== KDRV_DDR_PHY_REG->DX0GCR3:0x%08x => ", KDRV_DDR_PHY_REG->DX3GCR3);
    KDRV_DDR_PHY_REG->DX3GCR3 = (KDRV_DDR_PHY_REG->DX3GCR3 & ~(rflvt_mask));
    ddrc_dbg("0x%08x ==\n", KDRV_DDR_PHY_REG->DX3GCR3);
    
    ddrc_dbg("DQSDR2, DFTMNTPRD_BIT0_15:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR2 & dqsdr2_dftmntprd_mask));
    ddrc_dbg("DQSDR2, DFTTHRSH_BIT23_16:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR2 & dqsdr2_dftthrsh_mask));
    KDRV_DDR_PHY_REG->DQSDR2 = (KDRV_DDR_PHY_REG->DQSDR2 | dqsdr2_dftthrsh_mask);
    ddrc_dbg("[New] DQSDR2, DFTMNTPRD_BIT0_15:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR2 & dqsdr2_dftmntprd_mask));
    ddrc_dbg("[New] DQSDR2, DFTTHRSH_BIT23_16:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR2 & dqsdr2_dftthrsh_mask));

    ddrc_dbg("DQSDR0, DFTUPDRD_BIT8_11:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR0 & dqsdr0_dftupdrd_mask));
    ddrc_dbg("DQSDR0, DFTDTEN_BIT0:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR0 & dqsdr0_dftdten_mask));
    KDRV_DDR_PHY_REG->DQSDR0 = (KDRV_DDR_PHY_REG->DQSDR0 | dqsdr0_dftdten_mask);
    ddrc_dbg("[New] DQSDR0, DFTUPDRD_BIT8_11:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR0 & dqsdr0_dftupdrd_mask));
    ddrc_dbg("[New] DQSDR0, DFTDTEN_BIT0:0x%x\n", (KDRV_DDR_PHY_REG->DQSDR0 & dqsdr0_dftdten_mask));
#if 0
    //---start drift detect
    //DTCR1.RDVLGDIFF keep default at preset
    //DXnGCR3
    apb_rd((0x2000+(0x1C3<<2)), rd_data);       // 0x1C3 (PUB_DX0GCR3)
    ddrc_dbg("\n== DX0GCR3, rd_data:0x%08x => ", rd_data);
    rd_data &= ~(rflvt_mask);
    apb_wr((0x2000+(0x1C3<<2)), rd_data);
    apb_rd((0x2000+(0x1C3<<2)), rd_data);
    ddrc_dbg("0x%08x ==\n", rd_data);

    apb_rd((0x2000+(0x203<<2)), rd_data);       // 0x203 (PUB_DX1GCR3)
    ddrc_dbg("== DX1GCR3, rd_data:0x%08x => ", rd_data);
    rd_data &= ~(rflvt_mask);
    apb_wr((0x2000+(0x203<<2)), rd_data);
    apb_rd((0x2000+(0x203<<2)), rd_data);
    ddrc_dbg("0x%08x ==\n", rd_data);

    apb_rd((0x2000+(0x243<<2)), rd_data);       // 0x243 (PUB_DX2GCR3)
    ddrc_dbg("== DX2GCR3, rd_data:0x%08x => ", rd_data);
    rd_data &= ~(rflvt_mask);
    apb_wr((0x2000+(0x243<<2)), rd_data);
    apb_rd((0x2000+(0x243<<2)), rd_data);
    ddrc_dbg("0x%08x ==\n", rd_data);

    apb_rd((0x2000+(0x283<<2)), rd_data);       // 0x283 (PUB_DX3GCR3)
    ddrc_dbg("== DX3GCR3, rd_data:0x%08x => ", rd_data);
    rd_data &= ~(rflvt_mask);
    apb_wr((0x2000+(0x283<<2)), rd_data);
    apb_rd((0x2000+(0x283<<2)), rd_data);
    ddrc_dbg("0x%08x ==\n", rd_data);

    //DQSDR2    0x96
    apb_rd((0x2000+(0x96<<2)), rd_data);       // 0x96 (PUB_DQSDR2)
    ddrc_dbg("\n== DQSDR2, rd_data:0x%08x ==\n", rd_data);
    ddrc_dbg("DQSDR2, DFTMNTPRD_BIT0_15:0x%x\n", rd_data&dqsdr2_dftmntprd_mask);
    ddrc_dbg("DQSDR2, DFTTHRSH_BIT23_16:0x%x\n", rd_data&dqsdr2_dftthrsh_mask);
    rd_data |= dqsdr2_dftthrsh_mask;    //(0xFF << 16)
    apb_wr((0x2000+(0x96<<2)), rd_data);
    apb_rd((0x2000+(0x96<<2)), rd_data);
    ddrc_dbg("== [New] DQSDR2, rd_data:0x%08x ==\n", rd_data);
    ddrc_dbg("DQSDR2, DFTMNTPRD_BIT0_15:0x%x\n", rd_data&dqsdr2_dftmntprd_mask);
    ddrc_dbg("DQSDR2, DFTTHRSH_BIT23_16:0x%x\n", rd_data&dqsdr2_dftthrsh_mask);

    //DQSDR0    0x94
    apb_rd((0x2000+(0x96<<2)), rd_data);
    ddrc_dbg("\n== DQSDR0, rd_data:0x%08x ==\n", rd_data);
    ddrc_dbg("DQSDR0, DFTUPDRD_BIT8_11:0x%x\n", rd_data&dqsdr0_dftupdrd_mask);
    ddrc_dbg("DQSDR0, DFTDTEN_BIT0:0x%x\n", rd_data&dqsdr0_dftdten_mask);
    rd_data |= dqsdr0_dftdten_mask; //(1 << 0)
    apb_wr((0x2000+(0x96<<2)), rd_data);
    apb_rd((0x2000+(0x96<<2)), rd_data);
    ddrc_dbg("[New] == DQSDR0, rd_data:0x%08x ==\n", rd_data);
    ddrc_dbg("DQSDR0, DFTUPDRD_BIT8_11:0x%x\n", rd_data&dqsdr0_dftupdrd_mask);
    ddrc_dbg("DQSDR0, DFTDTEN_BIT0:0x%x\n", rd_data&dqsdr0_dftdten_mask);
#endif
}

#endif
#pragma push
#pragma O0
uint32_t kdrv_ddr_Initialize(uint32_t axi_ddr_clk)
{
    // relase APB reset
    ddr_dynamic_reg *dynamic_reg;
    dynamic_reg = &ddr_clk_dynamic_reg[axi_ddr_clk];

    if(dynamic_reg->opt_freq == LPDDR3_800_)
        ddrc_dbg("== LPDDR3_800 !! ==\n");
    else if(dynamic_reg->opt_freq == LPDDR3_1333_)
        ddrc_dbg("== LPDDR3_1333 !! ==\n");
    else if(dynamic_reg->opt_freq == LPDDR3_1600_)
        ddrc_dbg("== LPDDR3_1600 !! ==\n");
    else if(dynamic_reg->opt_freq == LPDDR3_1866_)
        ddrc_dbg("== LPDDR3_1866 !! ==\n");
    else if(dynamic_reg->opt_freq == LPDDR3_2133_)
        ddrc_dbg("== LPDDR3_2133 !! ==\n");

    ddr_rst_t *ddr_rst = (ddr_rst_t *)SCU_EXTREG_DDR_RST;
    ddr_rst->PRESETN_DDR = 1;

    ddrc_dbg("KDRV_DDR_PHY_REG->PGSR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PGSR0, KDRV_DDR_PHY_REG->PGSR0);
    if(KDRV_DDR_PHY_REG->PGSR0 & 0x01) {
        ddrc_dbg("== DDR was Initialized !! ==\n");
        release_axi_rst();
        return 0;
    }
    
    pwr_sw_rst_t *pwr_sw_rst = (pwr_sw_rst_t *)SCU_EXTREG_SWRST;
    pwr_sw_rst->BUS_NOM_APB_1 = 1;

    // Disable H2X cacheable attribute to prevent APB pre-fetch
    // 0x31C0_0000[5:4] = 2'b10
    //rd_data = inw( 0x31C00000 );
    //rd_data |= 0x20;
    //rd_data &= 0xFFFFFFEF;
    //outw( 0x31C00000, rd_data );

    ddrc_dbg("== DDR Initialize Start !! ==\n");
    // assert core_ddrc_rstn
    apb_wr(0x3040, 0x00000000);

    // Hold DRAMC operation
    KDRV_DDR_CTRL_REG->PWRCTL = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->PWRCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->PWRCTL, KDRV_DDR_CTRL_REG->PWRCTL);
		
    // Burst Length = 8, LPDDR3
    KDRV_DDR_CTRL_REG->MSTR = 0x00040008;
    ddrc_dbg("KDRV_DDR_CTRL_REG->MSTR(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->MSTR, KDRV_DDR_CTRL_REG->MSTR);
		
    KDRV_DDR_CTRL_REG->MRCTRL0 = 0x00000010;
    ddrc_dbg("KDRV_DDR_CTRL_REG->MRCTRL0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->MRCTRL0, KDRV_DDR_CTRL_REG->MRCTRL0);
    KDRV_DDR_CTRL_REG->DERATEEN = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DERATEEN(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DERATEEN, KDRV_DDR_CTRL_REG->DERATEEN);
    KDRV_DDR_CTRL_REG->DERATEINT = 0x5154500;//0xd94a7cd6;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DERATEINT(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DERATEINT, KDRV_DDR_CTRL_REG->DERATEINT);
    KDRV_DDR_CTRL_REG->DERATECTL = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DERATECTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DERATECTL, KDRV_DDR_CTRL_REG->DERATECTL);
		
    // All type of SDRAM
    // enable dfi_dram_clk_disable and power-down
    KDRV_DDR_CTRL_REG->PWRTMG = 0x00202a1c;
    ddrc_dbg("KDRV_DDR_CTRL_REG->PWRTMG(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->PWRTMG, KDRV_DDR_CTRL_REG->PWRTMG);

    KDRV_DDR_CTRL_REG->HWLPCTL = 0x00020002;
    ddrc_dbg("KDRV_DDR_CTRL_REG->HWLPCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->HWLPCTL, KDRV_DDR_CTRL_REG->HWLPCTL);
    KDRV_DDR_CTRL_REG->RFSHCTL0 = 0x00210000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->RFSHCTL0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->RFSHCTL0, KDRV_DDR_CTRL_REG->RFSHCTL0);
    KDRV_DDR_CTRL_REG->RFSHCTL3 = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->RFSHCTL3(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->RFSHCTL3, KDRV_DDR_CTRL_REG->RFSHCTL3);
    ddrc_dbg("[ken-info] : RFSHCTL3 = 32'h1 , dis_auto_refresh=1\n");
    KDRV_DDR_CTRL_REG->RFSHTMG = dynamic_reg->rfshtmg;//_RFSHTMG;
    ddrc_dbg("KDRV_DDR_CTRL_REG->RFSHTMG(%p):0x%08X, _RFSHTMG:%x%08x\n", &KDRV_DDR_CTRL_REG->RFSHTMG, KDRV_DDR_CTRL_REG->RFSHTMG, dynamic_reg->rfshtmg);

    KDRV_DDR_CTRL_REG->CRCPARCTL0 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->CRCPARCTL0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->CRCPARCTL0, KDRV_DDR_CTRL_REG->CRCPARCTL0);
    KDRV_DDR_CTRL_REG->INIT0 = 0x40720003;
    ddrc_dbg("KDRV_DDR_CTRL_REG->INIT0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->INIT0, KDRV_DDR_CTRL_REG->INIT0);
    KDRV_DDR_CTRL_REG->INIT1 = 0x00010004;
    ddrc_dbg("KDRV_DDR_CTRL_REG->INIT1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->INIT1, KDRV_DDR_CTRL_REG->INIT1);
    KDRV_DDR_CTRL_REG->INIT2 = 0x00000008;
    ddrc_dbg("KDRV_DDR_CTRL_REG->INIT2(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->INIT2, KDRV_DDR_CTRL_REG->INIT2);
    KDRV_DDR_CTRL_REG->INIT3 = dynamic_reg->init3;//_INIT3;
    ddrc_dbg("KDRV_DDR_CTRL_REG->INIT3(%p):0x%08X, _INIT3:0x%08X,\n", &KDRV_DDR_CTRL_REG->INIT3, KDRV_DDR_CTRL_REG->INIT3, dynamic_reg->init3);
    KDRV_DDR_CTRL_REG->INIT4 = 0x00020000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->INIT4(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->INIT4, KDRV_DDR_CTRL_REG->INIT4);
    KDRV_DDR_CTRL_REG->INIT5 = 0x00120007;
    ddrc_dbg("KDRV_DDR_CTRL_REG->INIT5(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->INIT5, KDRV_DDR_CTRL_REG->INIT5);

    // DIMM control
    KDRV_DDR_CTRL_REG->DIMMCTL = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DIMMCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DIMMCTL, KDRV_DDR_CTRL_REG->DIMMCTL);

    KDRV_DDR_CTRL_REG->DRAMTMG0 = dynamic_reg->dramtmg0;//_DRAMTMG0;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG0, KDRV_DDR_CTRL_REG->DRAMTMG0);
    KDRV_DDR_CTRL_REG->DRAMTMG1 = dynamic_reg->dramtmg1;//_DRAMTMG1;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG1, KDRV_DDR_CTRL_REG->DRAMTMG1);
    KDRV_DDR_CTRL_REG->DRAMTMG2 = dynamic_reg->dramtmg2;//_DRAMTMG2;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG2(%p):0x%08X, _DRAMTMG2:0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG2, KDRV_DDR_CTRL_REG->DRAMTMG2, dynamic_reg->dramtmg2);
    KDRV_DDR_CTRL_REG->DRAMTMG3 = dynamic_reg->dramtmg3;//_DRAMTMG3;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG3(%p):0x%08X, _DRAMTMG3:0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG3, KDRV_DDR_CTRL_REG->DRAMTMG3, dynamic_reg->dramtmg3);
    KDRV_DDR_CTRL_REG->DRAMTMG4 = dynamic_reg->dramtmg4;//_DRAMTMG4;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG4(%p):0x%08X, _DRAMTMG4:0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG4, KDRV_DDR_CTRL_REG->DRAMTMG4, dynamic_reg->dramtmg4);
    KDRV_DDR_CTRL_REG->DRAMTMG5 = dynamic_reg->dramtmg5;//_DRAMTMG5;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG5(%p):0x%08X, _DRAMTMG5:0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG5, KDRV_DDR_CTRL_REG->DRAMTMG5, dynamic_reg->dramtmg5);
    KDRV_DDR_CTRL_REG->DRAMTMG6 = dynamic_reg->dramtmg6;//_DRAMTMG6;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG6(%p):0x%08X, _DRAMTMG6:0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG6, KDRV_DDR_CTRL_REG->DRAMTMG6, dynamic_reg->dramtmg6);
    KDRV_DDR_CTRL_REG->DRAMTMG7 = 0x00000101;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG7(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG7, KDRV_DDR_CTRL_REG->DRAMTMG7);
    KDRV_DDR_CTRL_REG->DRAMTMG8 = 0x00000101;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG8(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG8, KDRV_DDR_CTRL_REG->DRAMTMG8);
    KDRV_DDR_CTRL_REG->DRAMTMG14 = dynamic_reg->dramtmg14;//_DRAMTMG14;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DRAMTMG14(%p):0x%08X, _DRAMTMG14:0x%08X\n", &KDRV_DDR_CTRL_REG->DRAMTMG14, KDRV_DDR_CTRL_REG->DRAMTMG14, dynamic_reg->dramtmg14);

    KDRV_DDR_CTRL_REG->ZQCTL0 = dynamic_reg->zqctl0;//_ZQCTL0;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ZQCTL0(%p):0x%08X, _ZQCTL0:0x%08X\n", &KDRV_DDR_CTRL_REG->ZQCTL0, KDRV_DDR_CTRL_REG->ZQCTL0, dynamic_reg->zqctl0);
    KDRV_DDR_CTRL_REG->ZQCTL1 = 0x01b00070;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ZQCTL1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ZQCTL1, KDRV_DDR_CTRL_REG->ZQCTL1);
    KDRV_DDR_CTRL_REG->ZQCTL2 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ZQCTL2(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ZQCTL2, KDRV_DDR_CTRL_REG->ZQCTL2);

    KDRV_DDR_CTRL_REG->DFITMG0 = dynamic_reg->dfitmg0;//_DFITMG0;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFITMG0(%p):0x%08X, _DFITMG0:0x%08X\n", &KDRV_DDR_CTRL_REG->DFITMG0, KDRV_DDR_CTRL_REG->DFITMG0, dynamic_reg->dfitmg0);
    KDRV_DDR_CTRL_REG->DFITMG1 = 0x00070202;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFITMG1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFITMG1, KDRV_DDR_CTRL_REG->DFITMG1);
    KDRV_DDR_CTRL_REG->DFILPCFG0 = 0x0751b031;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFILPCFG0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFILPCFG0, KDRV_DDR_CTRL_REG->DFILPCFG0);
    KDRV_DDR_CTRL_REG->DFIUPD0 = 0xc0400004;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFIUPD0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFIUPD0, KDRV_DDR_CTRL_REG->DFIUPD0);
    KDRV_DDR_CTRL_REG->DFIUPD1 = 0x00b600e1;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFIUPD1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFIUPD1, KDRV_DDR_CTRL_REG->DFIUPD1);
    KDRV_DDR_CTRL_REG->DFIUPD2 = 0x80000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFIUPD2(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFIUPD2, KDRV_DDR_CTRL_REG->DFIUPD2);
	
    // set DFI init complete
    KDRV_DDR_CTRL_REG->SWCTL = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SWCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SWCTL, KDRV_DDR_CTRL_REG->SWCTL);
    KDRV_DDR_CTRL_REG->DFIMISC = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFIMISC(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFIMISC, KDRV_DDR_CTRL_REG->DFIMISC);
    KDRV_DDR_CTRL_REG->SWCTL = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SWCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SWCTL, KDRV_DDR_CTRL_REG->SWCTL);
	
    ddrc_dbg("wait SWSTAT !!\n");
    while ( !(KDRV_DDR_CTRL_REG->SWSTAT & 0x01) )
        ;

    add_delay(100);

    KDRV_DDR_CTRL_REG->DFIPHYMSTR = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFIPHYMSTR(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFIPHYMSTR, KDRV_DDR_CTRL_REG->DFIPHYMSTR);

    // SAR Base & Size
    KDRV_DDR_CTRL_REG->ADDRMAP1 = 0x00141407;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ADDRMAP1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ADDRMAP1, KDRV_DDR_CTRL_REG->ADDRMAP1);
    KDRV_DDR_CTRL_REG->ADDRMAP2 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ADDRMAP2(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ADDRMAP2, KDRV_DDR_CTRL_REG->ADDRMAP2);
    KDRV_DDR_CTRL_REG->ADDRMAP3 = 0x1F000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ADDRMAP3(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ADDRMAP3, KDRV_DDR_CTRL_REG->ADDRMAP3);
    KDRV_DDR_CTRL_REG->ADDRMAP4 = 0x00001f1f;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ADDRMAP4(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ADDRMAP4, KDRV_DDR_CTRL_REG->ADDRMAP4);
    KDRV_DDR_CTRL_REG->ADDRMAP5 = 0x04040404;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ADDRMAP5(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ADDRMAP5, KDRV_DDR_CTRL_REG->ADDRMAP5);
    KDRV_DDR_CTRL_REG->ADDRMAP6 = 0x0F0F0F04;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ADDRMAP6(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ADDRMAP6, KDRV_DDR_CTRL_REG->ADDRMAP6);
		

    KDRV_DDR_CTRL_REG->ODTCFG = dynamic_reg->odtcfg;//_ODTCFG;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ODTCFG(%p):0x%08X, _ODTCFG:0x%08X\n", &KDRV_DDR_CTRL_REG->ODTCFG, KDRV_DDR_CTRL_REG->ODTCFG, dynamic_reg->odtcfg);
    KDRV_DDR_CTRL_REG->ODTMAP = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->ODTMAP(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->ODTMAP, KDRV_DDR_CTRL_REG->ODTMAP);
		
    // open page policy
    KDRV_DDR_CTRL_REG->SCHED = 0x72d11304;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SCHED(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SCHED, KDRV_DDR_CTRL_REG->SCHED);

    KDRV_DDR_CTRL_REG->SCHED1 = 0x000000f8;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SCHED1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SCHED1, KDRV_DDR_CTRL_REG->SCHED1);
    
    KDRV_DDR_CTRL_REG->PERFHPR1 = 0x84008dcf;
    ddrc_dbg("KDRV_DDR_CTRL_REG->PERFHPR1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->PERFHPR1, KDRV_DDR_CTRL_REG->PERFHPR1);
    KDRV_DDR_CTRL_REG->PERFLPR1 = 0xf2004d4f;
    ddrc_dbg("KDRV_DDR_CTRL_REG->PERFLPR1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->PERFLPR1, KDRV_DDR_CTRL_REG->PERFLPR1);
    KDRV_DDR_CTRL_REG->PERFWR1= 0xff007159;
    ddrc_dbg("KDRV_DDR_CTRL_REG->PERFWR1(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->PERFWR1, KDRV_DDR_CTRL_REG->PERFWR1);
		
    KDRV_DDR_CTRL_REG->DBG0 = 0x00000006;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DBG0(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DBG0, KDRV_DDR_CTRL_REG->DBG0);
    KDRV_DDR_CTRL_REG->DBGCMD = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DBGCMD(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DBGCMD, KDRV_DDR_CTRL_REG->DBGCMD);

    KDRV_DDR_CTRL_REG->SWCTL = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SWCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SWCTL, KDRV_DDR_CTRL_REG->SWCTL);
    KDRV_DDR_CTRL_REG->SWCTLSTATIC = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SWCTLSTATIC(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SWCTLSTATIC, KDRV_DDR_CTRL_REG->SWCTLSTATIC);
    KDRV_DDR_CTRL_REG->POISONCFG = 0x00110001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->POISONCFG(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->POISONCFG, KDRV_DDR_CTRL_REG->POISONCFG);
    KDRV_DDR_CTRL_MP_REG->PCCFG = 0x00000011;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCCFG(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCCFG, KDRV_DDR_CTRL_MP_REG->PCCFG);

    // AXI-0
    KDRV_DDR_CTRL_MP_REG->PCFGR_0 = 0x000002fe;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGR_0(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGR_0, KDRV_DDR_CTRL_MP_REG->PCFGR_0);
    KDRV_DDR_CTRL_MP_REG->PCFGW_0 = 0x0000705f;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGW_0(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGW_0, KDRV_DDR_CTRL_MP_REG->PCFGW_0);
    KDRV_DDR_CTRL_MP_REG->PCTRL_0 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_0(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_0, KDRV_DDR_CTRL_MP_REG->PCTRL_0);
    KDRV_DDR_CTRL_MP_REG->PCFGQOS0_0 = 0x00120006;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGQOS0_0(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGQOS0_0, KDRV_DDR_CTRL_MP_REG->PCFGQOS0_0);

    // AXI-1
    KDRV_DDR_CTRL_MP_REG->PCFGR_1 = 0x00000085;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGR_1(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGR_1, KDRV_DDR_CTRL_MP_REG->PCFGR_1);
    KDRV_DDR_CTRL_MP_REG->PCFGW_1 = 0x000031a3;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGW_1(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGW_1, KDRV_DDR_CTRL_MP_REG->PCFGW_1);
    KDRV_DDR_CTRL_MP_REG->PCTRL_1 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_1(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_1, KDRV_DDR_CTRL_MP_REG->PCTRL_1);
    KDRV_DDR_CTRL_MP_REG->PCFGQOS0_1 = 0x0000000d;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGQOS0_1(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGQOS0_1, KDRV_DDR_CTRL_MP_REG->PCFGQOS0_1);

    // AXI-2
    KDRV_DDR_CTRL_MP_REG->PCFGR_2 = 0x0001310e;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGR_2(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGR_2, KDRV_DDR_CTRL_MP_REG->PCFGR_2);
    KDRV_DDR_CTRL_MP_REG->PCFGW_2 = 0x0000211d;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGW_2(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGW_2, KDRV_DDR_CTRL_MP_REG->PCFGW_2);
    KDRV_DDR_CTRL_MP_REG->PCTRL_2 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_2(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_2, KDRV_DDR_CTRL_MP_REG->PCTRL_2);
    KDRV_DDR_CTRL_MP_REG->PCFGQOS0_2 = 0x00110004;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGQOS0_2(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGQOS0_2, KDRV_DDR_CTRL_MP_REG->PCFGQOS0_2);

    // AXI-3
    KDRV_DDR_CTRL_MP_REG->PCFGR_3 = 0x0001628d;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGR_3(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGR_3, KDRV_DDR_CTRL_MP_REG->PCFGR_3);
    KDRV_DDR_CTRL_MP_REG->PCFGW_3 = 0x00003216;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGW_3(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGW_3, KDRV_DDR_CTRL_MP_REG->PCFGW_3);
    KDRV_DDR_CTRL_MP_REG->PCTRL_3 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_3(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_3, KDRV_DDR_CTRL_MP_REG->PCTRL_3);
    KDRV_DDR_CTRL_MP_REG->PCFGQOS0_3 = 0x00020006;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCFGQOS0_3(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCFGQOS0_3, KDRV_DDR_CTRL_MP_REG->PCFGQOS0_3);
		
    // de-assert core_ddrc_rstn
    apb_wr(0x3040, 0x00000001);     // 0x0C1('d193), (              DBG1), DRAM_SUBSYS
    // release DRAMC and AXI reset
    release_axi_rst();

    // DDRPHY
    //kenfix dqsgx==00
    KDRV_DDR_PHY_REG->DSGCR = 0x0064403b;
    ddrc_dbg("KDRV_DDR_PHY_REG->DSGCR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DSGCR, KDRV_DDR_PHY_REG->DSGCR);
    KDRV_DDR_PHY_REG->IOVCR0 = 0x0f000009;
    ddrc_dbg("KDRV_DDR_PHY_REG->IOVCR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->IOVCR0, KDRV_DDR_PHY_REG->IOVCR0);
    KDRV_DDR_PHY_REG->IOVCR1 = 0x00000109;    //--SNPS: disable ZQVREFPEN
    ddrc_dbg("KDRV_DDR_PHY_REG->IOVCR1(%p):0x%08X\n", &KDRV_DDR_PHY_REG->IOVCR1, KDRV_DDR_PHY_REG->IOVCR1);
		
    KDRV_DDR_PHY_REG->DX0GCR4 = 0x0e00003c;
    ddrc_dbg("KDRV_DDR_PHY_REG->DX0GCR4(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX0GCR4, KDRV_DDR_PHY_REG->DX0GCR4);
    KDRV_DDR_PHY_REG->DX1GCR4 = 0x0e00003c;
    ddrc_dbg("KDRV_DDR_PHY_REG->DX1GCR4(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX1GCR4, KDRV_DDR_PHY_REG->DX1GCR4);
    KDRV_DDR_PHY_REG->DX2GCR4 = 0x0e00003c;
    ddrc_dbg("KDRV_DDR_PHY_REG->DX2GCR4(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX2GCR4, KDRV_DDR_PHY_REG->DX2GCR4);
    KDRV_DDR_PHY_REG->DX3GCR4 = 0x0e00003c;
    ddrc_dbg("KDRV_DDR_PHY_REG->DX3GCR4(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DX3GCR4, KDRV_DDR_PHY_REG->DX3GCR4);

    KDRV_DDR_PHY_REG->DTCR0 = 0x8000b0c7;    //DTMPR=1  <--- LPDDR3  RDG   --SNPS: Change RFSHDT to 8
    ddrc_dbg("KDRV_DDR_PHY_REG->DTCR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DTCR0, KDRV_DDR_PHY_REG->DTCR0);
    KDRV_DDR_PHY_REG->VTCR1 = 0x0fc00172;
    ddrc_dbg("KDRV_DDR_PHY_REG->VTCR1(%p):0x%08X\n", &KDRV_DDR_PHY_REG->VTCR1, KDRV_DDR_PHY_REG->VTCR1);
    KDRV_DDR_PHY_REG->DCR = 0x00000409;
    ddrc_dbg("KDRV_DDR_PHY_REG->DCR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DCR, KDRV_DDR_PHY_REG->DCR);
    KDRV_DDR_PHY_REG->DXCCR = 0x20d01884;    //--SNPS: set QSCNTENCTL=1 for gate training
    ddrc_dbg("KDRV_DDR_PHY_REG->DXCCR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DXCCR, KDRV_DDR_PHY_REG->DXCCR);

    //KDRV_DDR_PHY_REG->PTR0 = 0x5e001810;    //snps recomd on 7/22  for sim
    KDRV_DDR_PHY_REG->PTR0 = 0x5e021590;    //snps recomd on 7/22 for silicon
    ddrc_dbg("KDRV_DDR_PHY_REG->PTR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PTR0, KDRV_DDR_PHY_REG->PTR0);
    ddrc_dbg("[ken-monit] PUB_PTR0_tPLLPD 0x%x!!\n", (KDRV_DDR_PHY_REG->PTR0 & 0xFFE00000)>>21);
    ddrc_dbg("[ken-monit] PUB_PTR0_tPLLGS 0x%x!!\n", (KDRV_DDR_PHY_REG->PTR0&0x1FFFC0)>>6);
    KDRV_DDR_PHY_REG->PTR1 = 0x682b12c0;    ////snps recomd on 7/22
    ddrc_dbg("KDRV_DDR_PHY_REG->PTR1(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PTR1, KDRV_DDR_PHY_REG->PTR1);
    ddrc_dbg("[ken-monit] PUB_PTR1_tPLLLOCK 0x%x!!\n", (KDRV_DDR_PHY_REG->PTR1 & 0xFFFF8000)>>15);
    ddrc_dbg("[ken-monit] PUB_PTR1_tPLLRST 0x%x!!\n", KDRV_DDR_PHY_REG->PTR1&0x1FFF);
    KDRV_DDR_PHY_REG->PTR3 = 0x06b34156;
    ddrc_dbg("KDRV_DDR_PHY_REG->PTR3(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PTR3, KDRV_DDR_PHY_REG->PTR3);
    ddrc_dbg("[ken-monit] PUB_PTR3_tINIT1 0x%x!!\n", (KDRV_DDR_PHY_REG->PTR3 & 0x3FF00000)>>20);
    ddrc_dbg("[ken-monit] PUB_PTR3_tINIT0 0x%x!!\n", KDRV_DDR_PHY_REG->PTR3&0xFFFFF);
    KDRV_DDR_PHY_REG->PTR4 = 0x10af4156;
    ddrc_dbg("KDRV_DDR_PHY_REG->PTR4(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PTR4, KDRV_DDR_PHY_REG->PTR4);
    ddrc_dbg("[ken-monit] PUB_PTR4_tINIT3 0x%x!!\n", (KDRV_DDR_PHY_REG->PTR4 & 0x1FFC0000)>>18);
    ddrc_dbg("[ken-monit] PUB_PTR4_tINIT2 0x%x!!\n", KDRV_DDR_PHY_REG->PTR4&0x3FFFF);


    KDRV_DDR_PHY_REG->PLLCR = ((KDRV_DDR_PHY_REG->PLLCR & ~(0x180000))| dynamic_reg->pub_pllcr<<19);    //snps recomd on 7/22
    ddrc_dbg("KDRV_DDR_PHY_REG->PLLCR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PLLCR, KDRV_DDR_PHY_REG->PLLCR);
    ddrc_dbg("[ken-monit] PUB_PLLCR_FREQSEL  0x%x\n", (KDRV_DDR_PHY_REG->PLLCR&0x180000)>>19);//[20:19]);

    // --------------------------------------
    KDRV_DDR_PHY_REG->MR1_LPDDR3 = dynamic_reg->pub_mr1;//_PUB_MR1;
    ddrc_dbg("KDRV_DDR_PHY_REG->MR1_LPDDR3(%p):0x%08X, _PUB_MR1:0x%08X\n", &KDRV_DDR_PHY_REG->MR1_LPDDR3, KDRV_DDR_PHY_REG->MR1_LPDDR3, dynamic_reg->pub_mr1);
    KDRV_DDR_PHY_REG->MR2_LPDDR3 = dynamic_reg->pub_mr2;//_PUB_MR2;
    ddrc_dbg("KDRV_DDR_PHY_REG->MR2_LPDDR3(%p):0x%08X, _PUB_MR2:0x%08X\n", &KDRV_DDR_PHY_REG->MR2_LPDDR3, KDRV_DDR_PHY_REG->MR2_LPDDR3, dynamic_reg->pub_mr2);
    KDRV_DDR_PHY_REG->MR3_LPDDR3 = 0x00000001;    // --SNPS: Need to confirm from SI simulation for read.
    ddrc_dbg("KDRV_DDR_PHY_REG->MR3_LPDDR3(%p):0x%08X\n", &KDRV_DDR_PHY_REG->MR3_LPDDR3, KDRV_DDR_PHY_REG->MR3_LPDDR3);
    KDRV_DDR_PHY_REG->MR4_LPDDR3 = 0x00000000;
    ddrc_dbg("KDRV_DDR_PHY_REG->MR4_LPDDR3(%p):0x%08X\n", &KDRV_DDR_PHY_REG->MR4_LPDDR3, KDRV_DDR_PHY_REG->MR4_LPDDR3);
    KDRV_DDR_PHY_REG->MR5_LPDDR3 = 0x00000000;
    ddrc_dbg("KDRV_DDR_PHY_REG->MR5_LPDDR3(%p):0x%08X\n", &KDRV_DDR_PHY_REG->MR5_LPDDR3, KDRV_DDR_PHY_REG->MR5_LPDDR3);
    KDRV_DDR_PHY_REG->MR6 = 0x00000000;
    ddrc_dbg("KDRV_DDR_PHY_REG->MR6(%p):0x%08X\n", &KDRV_DDR_PHY_REG->MR6, KDRV_DDR_PHY_REG->MR6);
    KDRV_DDR_PHY_REG->MR7_LPDDR3 = 0x00000000;
    ddrc_dbg("KDRV_DDR_PHY_REG->MR7_LPDDR3(%p):0x%08X\n", &KDRV_DDR_PHY_REG->MR7_LPDDR3, KDRV_DDR_PHY_REG->MR7_LPDDR3);
    KDRV_DDR_PHY_REG->MR11_LPDDR3 = 0x00000002;
    ddrc_dbg("KDRV_DDR_PHY_REG->MR11_LPDDR3(%p):0x%08X\n", &KDRV_DDR_PHY_REG->MR11_LPDDR3, KDRV_DDR_PHY_REG->MR11_LPDDR3);;

    // --------------------------------------
    //DTPR *
    KDRV_DDR_PHY_REG->DTPR0 = dynamic_reg->pub_dtpr0;//_PUB_DTPR0;
    ddrc_dbg("KDRV_DDR_PHY_REG->DTPR0(%p):0x%08X, _PUB_DTPR0:0x%08X\n", &KDRV_DDR_PHY_REG->DTPR0, KDRV_DDR_PHY_REG->DTPR0, dynamic_reg->pub_dtpr0);
    ddrc_dbg("[ken-monit] DTPR0_tRRD 0x%x  \n" , (KDRV_DDR_PHY_REG->DTPR0&0x3F000000)>>24);         //rd_data[29:24] ) ; //
    ddrc_dbg("[ken-monit] DTPR0_tRP  0x%x  \n" , (KDRV_DDR_PHY_REG->DTPR0&0x7F00)>>8);              //rd_data[14: 8] ) ; //
    ddrc_dbg("[ken-monit] DTPR0_tRAS 0x%x  \n" , (KDRV_DDR_PHY_REG->DTPR0&0x7F0000)>>16);           //rd_data[22:16] ) ; //
    ddrc_dbg("[ken-monit] DTPR0_tRTP 0x%x  \n" , KDRV_DDR_PHY_REG->DTPR0&0xF);                      //rd_data[ 3: 0] ) ; //

    KDRV_DDR_PHY_REG->DTPR1 = dynamic_reg->pub_dtpr1;//_PUB_DTPR1;
    ddrc_dbg("KDRV_DDR_PHY_REG->DTPR1(%p):0x%08X, _PUB_DTPR1:0x%08X\n", &KDRV_DDR_PHY_REG->DTPR1, KDRV_DDR_PHY_REG->DTPR1, dynamic_reg->pub_dtpr1);
    ddrc_dbg("[ken-monit] DTPR1_tWLMRD  0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR1&0x3F000000)>>24);     //rd_data[29:24] ) ; //
    ddrc_dbg("[ken-monit] DTPR1_tFAW    0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR1&0xFF0000)>>16);       //rd_data[23:16] ) ; //
    ddrc_dbg("[ken-monit] DTPR1_tMOD    0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR1&0x700)>>8);           //rd_data[10: 8] ) ; //
    ddrc_dbg("[ken-monit] DTPR1_tMRD    0x%x   \n" , KDRV_DDR_PHY_REG->DTPR1&0x1F);                 //rd_data[ 4: 0] ) ; //

    KDRV_DDR_PHY_REG->DTPR2 = dynamic_reg->pub_dtpr2;//_PUB_DTPR2;       //
    ddrc_dbg("KDRV_DDR_PHY_REG->DTPR2(%p):0x%08X, _PUB_DTPR2:0x%08X\n", &KDRV_DDR_PHY_REG->DTPR2, KDRV_DDR_PHY_REG->DTPR2, dynamic_reg->pub_dtpr2);
    ddrc_dbg("[ken-monit] DTPR2_tRTW    0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR2 = dynamic_reg->pub_dtpr2&0x10000000)>>28);     //rd_data[28]       ) ; //
    ddrc_dbg("[ken-monit] DTPR2_tRTODT  0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR2 = dynamic_reg->pub_dtpr2&0x1000000)>>24);      //rd_data[24]       ) ; //
    ddrc_dbg("[ken-monit] DTPR2_tCKE    0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR2 = dynamic_reg->pub_dtpr2&0xF0000)>>16);        //rd_data[19:16]    ) ; //
    ddrc_dbg("[ken-monit] DTPR2_tXS     0x%x   \n" , KDRV_DDR_PHY_REG->DTPR2 = dynamic_reg->pub_dtpr2&0x3FF);                //rd_data[ 9: 0]    ) ; //

    KDRV_DDR_PHY_REG->DTPR3 = dynamic_reg->pub_dtpr3;//_PUB_DTPR3;        //
    ddrc_dbg("KDRV_DDR_PHY_REG->DTPR3(%p):0x%08X, _PUB_DTPR3:0x%08X\n", &KDRV_DDR_PHY_REG->DTPR3, KDRV_DDR_PHY_REG->DTPR3, dynamic_reg->pub_dtpr3);
    ddrc_dbg("[ken-monit] DTPR3_tCCD       0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR3 = dynamic_reg->pub_dtpr3&0x1C000000)>>26);  //rd_data[28:26]       ) ; //
    ddrc_dbg("[ken-monit] DTPR3_tDLLK      0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR3 = dynamic_reg->pub_dtpr3&0x3FF0000)>>16);   //rd_data[25:16]       ) ; //
    ddrc_dbg("[ken-monit] DTPR3_tDQSCKmax  0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR3 = dynamic_reg->pub_dtpr3&0x700)>>8);        //rd_data[10: 8]       ) ; //
    ddrc_dbg("[ken-monit] DTPR3_tDQSCK     0x%x   \n" , KDRV_DDR_PHY_REG->DTPR3 = dynamic_reg->pub_dtpr3&0x7);               //rd_data[ 2: 0]       ) ; //

    KDRV_DDR_PHY_REG->DTPR4 = dynamic_reg->pub_dtpr4;//_PUB_DTPR4;
    ddrc_dbg("KDRV_DDR_PHY_REG->DTPR4(%p):0x%08X, _PUB_DTPR4:0x%08X\n", &KDRV_DDR_PHY_REG->DTPR4, KDRV_DDR_PHY_REG->DTPR4, dynamic_reg->pub_dtpr4);
    ddrc_dbg("[ken-monit] DTPR4_tAOND_AOFD    0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR4 = dynamic_reg->pub_dtpr4&0x300000000)>>28);//rd_data[29:28]       ) ; //
    ddrc_dbg("[ken-monit] DTPR4_tRFC          0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR4 = dynamic_reg->pub_dtpr4&0x3FF0000)>>16); //rd_data[25:16]       ) ; //
    ddrc_dbg("[ken-monit] DTPR4_tWLO          0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR4 = dynamic_reg->pub_dtpr4&0xF00)>>8);      //rd_data[11: 8]       ) ; //
    ddrc_dbg("[ken-monit] DTPR4_tXP           0x%x   \n" , KDRV_DDR_PHY_REG->DTPR4 = dynamic_reg->pub_dtpr4&0x1F);            //rd_data[ 4: 0]       ) ; //

    KDRV_DDR_PHY_REG->DTPR5 = dynamic_reg->pub_dtpr5;//_PUB_DTPR5;
    ddrc_dbg("KDRV_DDR_PHY_REG->DTPR5(%p):0x%08X, _PUB_DTPR5:0x%08X\n", &KDRV_DDR_PHY_REG->DTPR5, KDRV_DDR_PHY_REG->DTPR5, dynamic_reg->pub_dtpr5);
    ddrc_dbg("[ken-monit] DTPR5_tRC           0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR5&0xFF0000)>>16) ; //rd_data[23:16]
    ddrc_dbg("[ken-monit] DTPR5_tRCD          0x%x   \n" , (KDRV_DDR_PHY_REG->DTPR5&0x7F00)>>8) ;    //rd_data[14: 8]
    ddrc_dbg("[ken-monit] DTPR5_tWTR          0x%x   \n" , KDRV_DDR_PHY_REG->DTPR5&0x1F) ;           //rd_data[ 4: 0]

    KDRV_DDR_PHY_REG->PGCR1 = 0x020046a0;
    ddrc_dbg("KDRV_DDR_PHY_REG->PGCR1(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PGCR1, KDRV_DDR_PHY_REG->PGCR1);
    KDRV_DDR_PHY_REG->PGCR2 = dynamic_reg->pub_pgcr2;//_PUB_PGCR2;    //--SNPS: Change tREFPRD according to DTCR0.RFSHDT changing to 8
    ddrc_dbg("KDRV_DDR_PHY_REG->PGCR2(%p):0x%08X, _PUB_PGCR2:0x%08X\n", &KDRV_DDR_PHY_REG->PGCR2, KDRV_DDR_PHY_REG->PGCR2, dynamic_reg->pub_pgcr2);
    KDRV_DDR_PHY_REG->PGCR3 = 0xc0aa0040;
    ddrc_dbg("KDRV_DDR_PHY_REG->PGCR3(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PGCR3, KDRV_DDR_PHY_REG->PGCR3);
    KDRV_DDR_PHY_REG->PGCR7 = 0x80040000;
    ddrc_dbg("KDRV_DDR_PHY_REG->PGCR7(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PGCR7, KDRV_DDR_PHY_REG->PGCR7);

    // --------------------------------------------------------
    // DQS Read Training Eanble
    //Ken suggest to remove 20200821  apb_wr(0x2088, 0x20c01884); // 0x022('d034), (         PUB_DXCCR), PUB

    // DTCR1
    KDRV_DDR_PHY_REG->DTCR1 = (KDRV_DDR_PHY_REG->DTCR1 | 0x1);
    ddrc_dbg("KDRV_DDR_PHY_REG->DTCR1(%p):0x%08X\n", &KDRV_DDR_PHY_REG->DTCR1, KDRV_DDR_PHY_REG->DTCR1);

    // -------------------------------------------------------------------------------------------
    //ZQCR
    KDRV_DDR_PHY_REG->ZQCR = ((KDRV_DDR_PHY_REG->ZQCR&~(0x700)) | (dynamic_reg->pub_zqcr<<8));
    ddrc_dbg("KDRV_DDR_PHY_REG->ZQCR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->ZQCR, KDRV_DDR_PHY_REG->ZQCR);
    ddrc_dbg("[ken-monit] ZQCR_PGWAIT          0x%x   \n" , (KDRV_DDR_PHY_REG->ZQCR&0x300)>>8);

    // -------------------------------------------------------------------------------------------
    // Auto Init First
    KDRV_DDR_PHY_REG->PIR = 0x00000073;    ////ZQ init
    ddrc_dbg("KDRV_DDR_PHY_REG->PIR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PIR, KDRV_DDR_PHY_REG->PIR);

    // Poll for IDONE (Initialization Done)
    ddrc_dbg("wait IDONE -- 1!!\n");
    while ( !(KDRV_DDR_PHY_REG->PGSR0 & 0x01) )
        ;

    // -------------------------------------------------------------------------------------------
    ddrc_dbg("DRAM initialization by PUB !!\n");
    // Auto Init Again
    KDRV_DDR_PHY_REG->PIR = 0x00000101;

    // Poll for IDONE (Initialization Done)
    ddrc_dbg("wait IDONE --2!!\n");
    while ( !(KDRV_DDR_PHY_REG->PGSR0 & 0x01) )
        ;

    // -------------------------------------------------------------------------------------------
    ddrc_dbg("After Dram init : Run self calibration \n ");

    //Kenfix0514
    add_delay(5000);

    if(dynamic_reg->acdlr != 0)
        KDRV_DDR_PHY_REG->ACLCDLR = ((KDRV_DDR_PHY_REG->ACLCDLR & ~0x1FF) | (dynamic_reg->acdlr));

    #if 0
    #if defined(LPDDR3_BIT_RATE) && (LPDDR3_BIT_RATE == LPDDR3_2133)
    KDRV_DDR_PHY_REG->ACLCDLR = ((KDRV_DDR_PHY_REG->ACLCDLR & ~0x1FF) | 0x31);
    #elif defined(LPDDR3_BIT_RATE) && (LPDDR3_BIT_RATE == LPDDR3_1866)
    KDRV_DDR_PHY_REG->ACLCDLR = ((KDRV_DDR_PHY_REG->ACLCDLR & ~0x1FF) | 0x34);
    #elif defined(LPDDR3_BIT_RATE) && (LPDDR3_BIT_RATE == LPDDR3_1600)
    KDRV_DDR_PHY_REG->ACLCDLR = ((KDRV_DDR_PHY_REG->ACLCDLR & ~0x1FF) | 0x3a);
    #endif
    #endif
    ddrc_dbg("KDRV_DDR_PHY_REG->ACLCDLR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->ACLCDLR, KDRV_DDR_PHY_REG->ACLCDLR);
    add_delay(5000);

//    apb_wr(0x2004, 0x00000005);   // CA train
//
//    rd_data = 0x0;
//    ddrc_dbg("waiting idle for ALL training !!");
//    WHILE_ ( !(rd_data & 0x01) )
//        apb_rd(0x2034, rd_data);       // 0x00D('d013), (         PUB_PGSR0), PUB

#ifdef WITH_CA_TRAIN
    KDRV_DDR_PHY_REG->PIR = 0x0000fe05;    // all train   --SNPS: enable CA training and disable static read training
#else
    KDRV_DDR_PHY_REG->PIR = 0x0000fe01;    // all train   --SNPS: enable CA training and disable static read training.
#endif
    ddrc_dbg("KDRV_DDR_PHY_REG->PIR(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PIR, KDRV_DDR_PHY_REG->PIR);

    ddrc_dbg ("waiting for ALL training finish\n ") ;
    while ( !(KDRV_DDR_PHY_REG->PGSR0 & 0x01) )
        ;

#if ((defined (DDRC_ENABLE_GEXT)) && (!defined (DDRC_ENABLE_DQS_DRIFT_DETECT_COMPENSATE)))
    KDRV_DDR_PHY_REG->DSGCR = (KDRV_DDR_PHY_REG->DSGCR | (0x2UL<<6));// DSGCR[7:6] DQSGX = 10
#elif ((!defined (DDRC_ENABLE_GEXT)) && (defined (DDRC_ENABLE_DQS_DRIFT_DETECT_COMPENSATE)))
    kdrv_dqs_read_drift_detect_and_compensate();
#endif

    ddrc_dbg ("Finishing ALL training reg\n ") ;
    add_delay(5000);

#if ((defined (DDRC_ENABLE_GEXT)) && (!defined (DDRC_ENABLE_DQS_DRIFT_DETECT_COMPENSATE)))
    kdrv_shift_back_gate_delay_gtext();
#endif

    //===================================================================================
    //sw_done =0  , wait sw_done_ack
    KDRV_DDR_CTRL_REG->SWCTL = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SWCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SWCTL, KDRV_DDR_CTRL_REG->SWCTL);
    //add_delay(5000);
    KDRV_DDR_CTRL_REG->DFIMISC = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFIMISC(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFIMISC, KDRV_DDR_CTRL_REG->DFIMISC);
    KDRV_DDR_CTRL_REG->SWCTL = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_REG->SWCTL(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->SWCTL, KDRV_DDR_CTRL_REG->SWCTL);

    ddrc_dbg ("waiting for sw_down ack\n ") ;
    while ( !(KDRV_DDR_PHY_REG->PGSR0 & 0x01) )
        ;

    // Wait for Normal (2'b01)
    while((KDRV_DDR_CTRL_REG->STAT & 0x3) != 0x1)
        ;

    //apb_wr(0x0320, 0x00000000);     // sw_done=0
    KDRV_DDR_CTRL_REG->SWCTL = 0x00000000;     // sw_done=0
    KDRV_DDR_CTRL_REG->DFIMISC = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->DFIMISC(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->DFIMISC, KDRV_DDR_CTRL_REG->DFIMISC);
    //apb_wr(0x0320, 0x00000001);     // sw_done=1
    KDRV_DDR_CTRL_REG->SWCTL = 0x00000001;     // sw_done=1

    ddrc_dbg("KDRV_DDR_PHY_REG->PGSR0(%p):0x%08X\n", &KDRV_DDR_PHY_REG->PGSR0, KDRV_DDR_PHY_REG->PGSR0);
    while ( !(KDRV_DDR_PHY_REG->PGSR0 & 0x01) )
        ;

    //RFSHCTL[0]: dis_auto_refresh=0
    KDRV_DDR_CTRL_REG->RFSHCTL3 = 0x00000000;
    ddrc_dbg("KDRV_DDR_CTRL_REG->RFSHCTL3(%p):0x%08X\n", &KDRV_DDR_CTRL_REG->RFSHCTL3, KDRV_DDR_CTRL_REG->RFSHCTL3);
		
    // AXI port-0~4 disable, (+0xB0)
    KDRV_DDR_CTRL_MP_REG->PCTRL_0 = 0x00000001;
    KDRV_DDR_CTRL_MP_REG->PCTRL_1 = 0x00000001;
    KDRV_DDR_CTRL_MP_REG->PCTRL_2 = 0x00000001;
    KDRV_DDR_CTRL_MP_REG->PCTRL_3 = 0x00000001;
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_0(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_0, KDRV_DDR_CTRL_MP_REG->PCTRL_0);
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_1(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_1, KDRV_DDR_CTRL_MP_REG->PCTRL_1);
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_2(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_2, KDRV_DDR_CTRL_MP_REG->PCTRL_2);
    ddrc_dbg("KDRV_DDR_CTRL_MP_REG->PCTRL_3(%p):0x%08X\n", &KDRV_DDR_CTRL_MP_REG->PCTRL_3, KDRV_DDR_CTRL_MP_REG->PCTRL_3);

    //
    //-DRAMC  (<== do not remove this mark)
    add_delay(1000);
    ddrc_dbg("Finish training : \n ");
    ddrc_dbg("== DDR Initialization Done !! ==\n");

    //NEW=========================================================================================
    {
        ddrc_dbg("== DDR Pre testing Start !! ==\n");
        uint32_t offset_addr, i, j;
        offset_addr = 0;
        for(j = 0; j < 3; j++)
        {
            ddrc_dbg("LPDDR3 Pre TEST %d \n", j);
            offset_addr = 0;
            for ( i = 0; i < 5; i++ )
            {
                offset_addr = offset_addr + (0x10<<i) ;
                outw( (DDR_MEM_BASE + (0x2000000*j)) + offset_addr , 0xfa626000+i );
            }

            ddrc_dbg("Verify LPDDR3 Pre TEST %d \n", j);
            offset_addr = 0;
            for ( i = 0; i < 5; i++ )
            {
                offset_addr = offset_addr + (0x10<<i);
                if ( inw( (DDR_MEM_BASE + (0x2000000*j)) + offset_addr ) != 0xfa626000+i )
                    return 2;
            }
        }
    }

    kdrv_ddr_read_delayline_info();
    return 0;
}
#pragma pop
