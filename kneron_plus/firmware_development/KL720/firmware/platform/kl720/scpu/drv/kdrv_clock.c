/* -------------------------------------------------------------------------- 
 * Copyright (c) 2018-2019 Kneron Inc. All rights reserved.
 *
 *      Name:    kdrv_clock.c
 *      Purpose: Kneron SCPU memory configuration and protection
 *
 *---------------------------------------------------------------------------*/
 
#include "kdrv_clock.h"
#include "kdrv_pll.h"
#include "kdrv_scu_ext.h"
#include "kdrv_scu.h"
#include "kdrv_ddr.h"

extern uint32_t SystemCoreClock;

#define ENABLE_ALL_GCK      //for bring up
#define CYCLES_PER_LOOP     2   // Typical: SUBS (1) + BCS (1 + P)

#define CAT_AUX(a, b) a##b
#define CAT(a, b) CAT_AUX(a, b)

#define CLK_SET_(clk,num)       _##clk##_CFG##num
#define CLK_SET(clk,num)        CLK_SET_(clk,num)
#define SCPU_CLK(clk,num)       CAT(SCPU,       CLK_SET(clk,num))
#define AXI_DDR_CLK(clk,num)    CAT(AXI_DDR,    CLK_SET(clk,num))
#define MRX0_CLK(clk,num)       CAT(MRX0,       CLK_SET(clk,num))
#define MRX1_CLK(clk,num)       CAT(MRX1,       CLK_SET(clk,num))
#define NPU_CLK(clk,num)        CAT(NPU,        CLK_SET(clk,num))
#define DSP_CLK(clk,num)        CAT(DSP,        CLK_SET(clk,num))
#define ADO_CLK(clk,num)        CAT(ADO,        CLK_SET(clk,num))

#define DIVIDER(x)              (x-1)
#define SCPU_CFG_NUM            1
#define AXI_DDR_CFG_NUM         1
#define MRX1_CFG_NUM            1
#define MRX0_CFG_NUM            1
#define NPU_CFG_NUM             1
#define DSP_CFG_NUM             1
#define ADO_CFG_NUM             1

const T_PLL0Config scpu_clk_tbl[SCPU_CLK_TOTAL_SUPPORTED] =
{
    //400MHz CFG1
    {
        (400000000),
        (PLL0_1200_MS_MASK|PLL0_1200_NS_MASK|PLL0_1200_PS_MASK|PLL0_1200_IS_MASK|SCPU_CLKIN_MUX_PLL0_DIV3_MASK|PLLn_DISABLE_MASK),
        (NULL)
    },
};

const T_PLLnConfig axi_ddr_clk_tbl[AXI_DDR_CLK_TOTAL_SUPPORTED] =
{
    //200MHz CFG1
    {
        (200000000),
        (PLL1_400_MS_MASK|PLL1_400_NS_MASK|PLL1_400_PS_MASK|PLL1_400_IS_MASK|PLLn_DISABLE_MASK),
        (PLL1_TIMER),
        (NULL),
        (NULL)
    },
    //333MHz CFG1
    {
        (333000000),
        (PLL1_668_MS_MASK|PLL1_668_NS_MASK|PLL1_668_PS_MASK|PLL1_668_IS_MASK|PLLn_DISABLE_MASK),
        (PLL1_TIMER),
        (NULL),
        (NULL)
    },
    //400MHz CFG1
    {
        (400000000),
        (PLL1_800_MS_MASK|PLL1_800_NS_MASK|PLL1_800_PS_MASK|PLL1_800_IS_MASK|PLLn_DISABLE_MASK),
        (PLL1_TIMER),
        (NULL),
        (NULL)
    },
    //466MHz CFG1
    {
        (466000000),
        (PLL1_933_MS_MASK|PLL1_933_NS_MASK|PLL1_933_PS_MASK|PLL1_933_IS_MASK|PLLn_DISABLE_MASK),
        (PLL1_TIMER),
        (NULL),
        (NULL)
    },
    //533MHz CFG1
    {
        (533000000),
        (PLL1_1066_MS_MASK|PLL1_1066_NS_MASK|PLL1_1066_PS_MASK|PLL1_1066_IS_MASK|PLLn_DISABLE_MASK),
        (PLL1_TIMER),
        (NULL),
        (NULL)
    },

};

const T_PLLnConfig mrx1_clk_tbl[MRX1_CLK_TOTAL_SUPPORTED] =
{
    //720MHz CFG1
    {
        (720000000),
        (PLL2_1440_MS_MASK|PLL2_1440_NS_MASK|PLL2_1440_PS_MASK|PLL2_1440_IS_MASK|PLLn_DISABLE_MASK),
        (PLL2_TIMER),
        (NULL),
        (PLL2_DIV_MASK)
    }
};
const T_PLLnConfig mrx0_clk_tbl[MRX0_CLK_TOTAL_SUPPORTED] =
{
    //720MHz CFG1
    {
        (720000000),
        (PLL3_1440_MS_MASK|PLL3_1440_NS_MASK|PLL3_1440_PS_MASK|PLL3_1440_IS_MASK|PLLn_DISABLE_MASK),
        (PLL3_TIMER),
        (NULL),
        (PLL3_DIV_MASK)
    },
    
    //1320MHz CFG1
    {
        (1320000000),
        (PLL3_2640_MS_MASK|PLL3_2640_NS_MASK|PLL3_2640_PS_MASK|PLL3_2640_IS_MASK|PLLn_DISABLE_MASK),
        (PLL3_TIMER),
        (NULL),
        (PLL3_DIV_MASK_2)
    }
};

const T_PLLnConfig npu_clk_tbl[NPU_CLK_TOTAL_SUPPORTED] =
{
    //200MHz CFG1
    {
        (200000000),
        (PLL4_800_MS_MASK|PLL4_800_NS_MASK|PLL4_800_PS_MASK|PLL4_800_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(2))
    },
    //250MHz CFG1
    {
        (250000000),
        (PLL4_1000_MS_MASK|PLL4_1000_NS_MASK|PLL4_1000_PS_MASK|PLL4_1000_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(2))
    },
    //300MHz CFG1
    {
        (300000000),
        (PLL4_1200_MS_MASK|PLL4_1200_NS_MASK|PLL4_1200_PS_MASK|PLL4_1200_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(2))
    },
    //350MHz CFG1
    {
        (350000000),
        (PLL4_1400_MS_MASK|PLL4_1400_NS_MASK|PLL4_1400_PS_MASK|PLL4_1400_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(2))
    },
    //400MHz CFG1
    {
        (400000000),
        (PLL4_800_MS_MASK|PLL4_800_NS_MASK|PLL4_800_PS_MASK|PLL4_800_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(1))
    },
    //500MHz CFG1
    {
        (500000000),
        (PLL4_1000_MS_MASK|PLL4_1000_NS_MASK|PLL4_1000_PS_MASK|PLL4_1000_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(1))
    },
    //600MHz CFG1
    {
        (600000000),
        (PLL4_1200_MS_MASK|PLL4_1200_NS_MASK|PLL4_1200_PS_MASK|PLL4_1200_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(1))
    },
    //650MHz CFG1
    {
        (650000000),
        (PLL4_1300_MS_MASK|PLL4_1300_NS_MASK|PLL4_1300_PS_MASK|PLL4_1300_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(1))
    },
    //700MHz CFG1
    {
        (700000000),
        (PLL4_1400_MS_MASK|PLL4_1400_NS_MASK|PLL4_1400_PS_MASK|PLL4_1400_IS_MASK|PLLn_DISABLE_MASK),
        (PLL4_TIMER),
        (0),
        (DIVIDER(1))
    }
};

const T_PLLnConfig dsp_clk_tbl[DSP_CLK_TOTAL_SUPPORTED] =
{
    //200MHz CFG1
    {
        (200000000),
        (PLL5_1200_MS_MASK|PLL5_1200_NS_MASK|PLL5_1200_PS_MASK|PLL5_1200_IS_MASK|PLLn_DISABLE_MASK),
        (PLL5_TIMER),
        (0),
        (DIVIDER(2))
    },
    //300MHz CFG1
    {
        (300000000),
        (PLL5_900_MS_MASK|PLL5_900_NS_MASK|PLL5_900_PS_MASK|PLL5_900_IS_MASK|PLLn_DISABLE_MASK),
        (PLL5_TIMER),
        (0),
        (DIVIDER(1))
    },
    //400MHz CFG1
    {
        (400000000),
        (PLL5_1200_MS_MASK|PLL5_1200_NS_MASK|PLL5_1200_PS_MASK|PLL5_1200_IS_MASK|PLLn_DISABLE_MASK),
        (PLL5_TIMER),
        (0),
        (DIVIDER(1))
    },
    //500MHz CFG1
    {
        (500000000),
        (PLL5_1500_MS_MASK|PLL5_1500_NS_MASK|PLL5_1500_PS_MASK|PLL5_1500_IS_MASK|PLLn_DISABLE_MASK),
        (PLL5_TIMER),
        (0),
        (DIVIDER(1))
    }
};

const T_PLLnConfig ado_clk_tbl[ADO_CLK_TOTAL_SUPPORTED] =
{
    //12.288MHz CFG1
    {
        (12288000),
        (PLL6_1536_MS_MASK|PLL6_1536_NS_MASK|PLL6_1536_PS_MASK|PLL6_1536_IS_MASK|PLLn_DISABLE_MASK),
        (PLL6_TIMER),
        (0),
        (DIVIDER(125))
    }
};

static sysclockopt *sysclk_opt_ = NULL;

void kdrv_delay_us(uint32_t usec)
{
    uint32_t cycles_per_us;
    volatile uint32_t count;

    cycles_per_us = SystemCoreClock / 1000000;
    count = usec * cycles_per_us / CYCLES_PER_LOOP;
    while(count--);
}
void kdrv_clock_axiddr_clock_set(T_PLLnConfig *pClk)
{
    SET_PLL_DDR_EN(0);
    SET_PLL(PLL_AXI_DDR, pClk->dwPLLnSetting);
    SET_PLL_TIMER(PLL_AXI_DDR, pClk->dwPLLnLockTime);
    SET_PLL_DDR_EN(1);
    while(!PLL_DDR_LOCK_STAT){};
}
void kdrv_clock_mrx1_clock_set(T_PLLnConfig *pClk)
{
#if (defined(MRX_PW) && (MRX_PW == ENABLE))
    //turn off csirx1 GCK;
    SET_CLOCK_EN(CSIRX1_ESC_CLK_EN, 0);
    SET_CLOCK_EN(CSIRX1_CSI_CLK_EN, 0);
    SET_CLOCK_EN(CSIRX1_VC_PCLK_EN, 0);
    SET_PLL_MRX1_EN(0);
    SET_PLL(PLL_MRX1, pClk->dwPLLnSetting);
    SET_PLL_TIMER(PLL_MRX1, pClk->dwPLLnLockTime);
    SET_MRX1_CLOCK_DIVIDER(pClk->dwDivider);
    SET_PLL_MRX1_EN(1);
    while(!PLL_MRX1_LOCK_STAT){};
    //trun on csirx1 GCK;
    SET_CLOCK_EN(CSIRX1_ESC_CLK_EN, 1);
    SET_CLOCK_EN(CSIRX1_CSI_CLK_EN, 1);
    SET_CLOCK_EN(CSIRX1_VC_PCLK_EN, 1);
#endif
}
void kdrv_clock_mrx0_clock_set(T_PLLnConfig *pClk)
{
#if (defined(MRX_PW) && (MRX_PW == ENABLE))
    //turn off csirx0 GCK;
    SET_CLOCK_EN(CSIRX0_ESC_CLK_EN, 0);
    SET_CLOCK_EN(CSIRX0_CSI_CLK_EN, 0);
    SET_CLOCK_EN(CSIRX0_VC_PCLK_EN, 0);
    SET_PLL_MRX0_EN(0);
    SET_PLL(PLL_MRX0, pClk->dwPLLnSetting);
    SET_PLL_TIMER(PLL_MRX0, pClk->dwPLLnLockTime);
    SET_MRX0_CLOCK_DIVIDER(pClk->dwDivider);
    SET_PLL_MRX0_EN(1);
    while(!PLL_MRX0_LOCK_STAT){};
    //trun on csirx0 GCK;
    SET_CLOCK_EN(CSIRX0_ESC_CLK_EN, 1);
    SET_CLOCK_EN(CSIRX0_CSI_CLK_EN, 1);
    SET_CLOCK_EN(CSIRX0_VC_PCLK_EN, 1);
#endif
}
void kdrv_clock_npu_clock_set(T_PLLnConfig *pClk)
{
    //turn off npu GCK;
    SET_CLOCK_EN(PLL4_NPU_MUX_EN, 0);
    SET_CLOCK_EN(NPU_CLK_EN, 0);
    SET_PLL_NPU_EN(0);
    SET_PLL(PLL_NPU, pClk->dwPLLnSetting);
    SET_PLL_TIMER(PLL_NPU, pClk->dwPLLnLockTime);
    SET_NPU_CLOCK_DIVIDER(pClk->dwDivider);
    SET_CLOCK_MUX_(NPU_CLK_SEL, pClk->dwMux);
    SET_PLL_NPU_EN(1);              //NPU
    while(!PLL_NPU_LOCK_STAT){};
    //trun on npu GCK;
    SET_CLOCK_EN(NPU_CLK_EN, 1);
    SET_CLOCK_EN(PLL4_NPU_MUX_EN, 1);
}

void kdrv_clock_npu_clock_change(npu_clk_setting npu_clock)
{
    kdrv_clock_npu_clock_set((T_PLLnConfig *)&npu_clk_tbl[npu_clock]);
}

void kdrv_clock_dsp_clock_set(T_PLLnConfig *pClk)
{
    //turn off dsp GCK;
    SET_CLOCK_EN(PLL5_DSP_MUX_EN, 0);
    SET_CLOCK_EN(DSP_GATE_EN, 0);
    SET_PLL_DSP_EN(0);
    SET_PLL(PLL_DSP, pClk->dwPLLnSetting);
    SET_PLL_TIMER(PLL_DSP, pClk->dwPLLnLockTime);
    SET_DSP_CLOCK_DIVIDER(pClk->dwDivider);
    SET_CLOCK_MUX_(DSP_CLK_SEL, pClk->dwMux);
    SET_PLL_DSP_EN(1);              //DSP
    while(!PLL_DSP_LOCK_STAT){};
    //trun on dsp GCK;
    SET_CLOCK_EN(DSP_GATE_EN, 1);
    SET_CLOCK_EN(PLL5_DSP_MUX_EN, 1);
}

void kdrv_clock_dsp_clock_change(dsp_clk_setting dsp_clock)
{
    kdrv_clock_dsp_clock_set((T_PLLnConfig *)&dsp_clk_tbl[dsp_clock]);
}

void kdrv_clock_ado_clock_set(T_PLLnConfig *pClk)
{
    //turn off ado GCK;
    SET_CLOCK_EN(I2S_MCLK_EN, 0);
    SET_PLL_ADO_EN(0);
    SET_PLL(PLL_ADO, pClk->dwPLLnSetting);
    SET_PLL_TIMER(PLL_ADO, pClk->dwPLLnLockTime);
    SET_ADO_CLOCK_DIVIDER(pClk->dwDivider);
    SET_PLL_ADO_EN(1);              //Audio
    while(!PLL_ADO_LOCK_STAT){};
    //trun on dwp GCK;
    SET_CLOCK_EN(I2S_MCLK_EN, 1);
}

void kdrv_clock_inititialize(sysclockopt* clk)
{
    sysclk_opt_ = clk;
    kdrv_clock_axiddr_clock_set((T_PLLnConfig *)&axi_ddr_clk_tbl[sysclk_opt_->axi_ddr]);
    kdrv_clock_npu_clock_set((T_PLLnConfig *)&npu_clk_tbl[sysclk_opt_->npu]);
    kdrv_clock_dsp_clock_set((T_PLLnConfig *)&dsp_clk_tbl[sysclk_opt_->dsp]);
    kdrv_clock_ado_clock_set((T_PLLnConfig *)&ado_clk_tbl[sysclk_opt_->audio]);

#ifdef ENABLE_ALL_GCK
    //Bus clock gate is default enable in scu register
    SET_CLOCK_EN0(0xFFFFFFFF);
    SET_CLOCK_EN1(1);
#endif

}

void kdrv_clock_set_scuclkin(enum scuclkin_type type, bool enable)
{
    regSCU->bf.clkin_mux = type;
    regSCU->bf.pllen = enable;
}

void kdrv_clock_set_bus_mux(enum busmux_type type)
{
    regSCU->bf.bus_mux = (uint32_t)type;
}

void kdrv_clock_enable_npu_clk(void)
{
    SET_CLOCK_EN(PLL4_NPU_MUX_EN, 1);
}

void kdrv_clock_enable_dsp_clk(void)
{
    SET_CLOCK_EN(PLL5_DSP_MUX_EN, 1);
}

void kdrv_clock_disable_npu_clk(void)
{
    SET_CLOCK_EN(PLL4_NPU_MUX_EN, 0);
}

void kdrv_clock_disable_dsp_clk(void)
{
    SET_CLOCK_EN(PLL5_DSP_MUX_EN, 0);
}

void kdrv_clock_enable_u3_clk60_clk(void)
{
    SET_CLOCK_EN(U3_CLK60_EN, 1);
}

void kdrv_clock_disable_u3_clk60_clk(void)
{
    SET_CLOCK_EN(U3_CLK60_EN, 0);
}

void kdrv_clock_enable_tdc_xclk_clk(void)
{
    SET_CLOCK_EN(TDC_XCLK_EN, 1);
}

void kdrv_clock_disable_tdc_xclk_clk(void)
{
    SET_CLOCK_EN(TDC_XCLK_EN, 0);
}


void kdrv_clock_set_csiclk(uint32_t idx, uint32_t enable)
{
    kdrv_clock_mrx1_clock_set((T_PLLnConfig *)&mrx1_clk_tbl[sysclk_opt_->mrx1]);
    kdrv_clock_mrx0_clock_set((T_PLLnConfig *)&mrx0_clk_tbl[sysclk_opt_->mrx0]);
    if(enable)
    {
        if (idx == 0)
        {
            SET_CLOCK_EN(CSIRX0_ESC_CLK_EN, 1);
            SET_CLOCK_EN(CSIRX0_CSI_CLK_EN, 1);
            SET_CLOCK_EN(CSIRX0_VC_PCLK_EN, 1);
        }
        else
        {
            SET_CLOCK_EN(CSIRX1_ESC_CLK_EN, 1);
            SET_CLOCK_EN(CSIRX1_CSI_CLK_EN, 1);
            SET_CLOCK_EN(CSIRX1_VC_PCLK_EN, 1);
        }
    }
    else
    {
        if(idx == 0)
        {
            SET_CLOCK_EN(CSIRX0_ESC_CLK_EN, 0);
            SET_CLOCK_EN(CSIRX0_CSI_CLK_EN, 0);
            SET_CLOCK_EN(CSIRX0_VC_PCLK_EN, 0);
        }
        else
        {
            SET_CLOCK_EN(CSIRX1_ESC_CLK_EN, 0);
            SET_CLOCK_EN(CSIRX1_CSI_CLK_EN, 0);
            SET_CLOCK_EN(CSIRX1_VC_PCLK_EN, 0);
        }
    }
}


