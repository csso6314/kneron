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

/**@addtogroup  KDRV_CLOCK  KDRV_CLOCK
 * @{
 * @brief       Kneron generic clock driver
 * @version     v1.0
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_CLOCK_H__
#define __KDRV_CLOCK_H__

#include <stdint.h>
#include <base.h>
#include "kdrv_scu_ext.h"
#include "kdrv_scu.h"
#include "kdrv_pll.h"
#include "kdrv_config.h"

/** @brief Enumerations of PLL id */
enum pll_id {
    /* pll_0 = 0, */
    PLL1 = 0,           /**< Enum 0*/
    PLL2,               /**< Enum 1*/
    PLL3,               /**< Enum 2*/
    PLL4,               /**< Enum 3*/
    PLL5,               /**< Enum 4*/
    PLL6                /**< Enum 5*/
};

/** @brief Enumerations of SCU clock in type */
enum scuclkin_type {
    scuclkin_pll0div3 = 0,     /**< Enum 0*/
    scuclkin_osc               /**< Enum 1*/
};

/** @brief Enumerations of Bus mux division */
enum busmux_type {
    busmux_div8 = 0,    /**< Enum 0, 50MHz*/
    busmux_div4,        /**< Enum 1, 100MHz*/
    busmux_div1         /**< Enum 2, 400MHz*/
};
/** @brief Structure of clock value setting*/
struct kdrv_clock_value {
    uint16_t ms;
    uint16_t ns;
    uint16_t ps;
    uint8_t div;
    uint8_t enable;
};
enum clkmux_type {
    clkmux_default = 0,
    clkmux_1
};
typedef struct {
    uint32_t axi_ddr;
    uint32_t mrx1;
    uint32_t mrx0;
    uint32_t npu;
    uint32_t dsp;
    uint32_t audio;
}sysclockopt;
/**
 * @brief       Delay us to let caller thread entering into sleep mode
 *
 * @param[in]   usec         how many us to sleep
 * @return      N/A
 */
void kdrv_delay_us(uint32_t usec);

/**
 * @brief       Set AXI DDR Clock
 *
 * @param[in]   *pClk         Pointer to pClk, see @ref T_PLLnConfig
 * @return      N/A
 */
void kdrv_clock_axiddr_clock_set(T_PLLnConfig *pClk);

/**
 * @brief       Set MRX1 Clock
 *
 * @param[in]   *pClk         Pointer to pClk, see @ref T_PLLnConfig
 * @return      N/A
 */
void kdrv_clock_mrx1_clock_set(T_PLLnConfig *pClk);

/**
 * @brief       Set MRX0 Clock
 *
 * @param[in]   *pClk         Pointer to pClk, see @ref T_PLLnConfig
 * @return      N/A
 */
void kdrv_clock_mrx0_clock_set(T_PLLnConfig *pClk);

/**
 * @brief       Set NPU Clock
 *
 * @param[in]   *pClk         Pointer to pClk, see @ref T_PLLnConfig
 * @return      N/A
 */
void kdrv_clock_npu_clock_set(T_PLLnConfig *pClk);

/**
 * @brief       change NPU Clock
 *
 * @param[in]   npu_clock     Options for new npu clock
 * @return      N/A
 */
void kdrv_clock_npu_clock_change(npu_clk_setting npu_clock);

/**
 * @brief       Set DSP Clock
 *
 * @param[in]   *pClk         Pointer to pClk, see @ref T_PLLnConfig
 * @return      N/A
 */
void kdrv_clock_dsp_clock_set(T_PLLnConfig *pClk);

/**
 * @brief       change DSP Clock
 *
 * @param[in]   dsp_clock     Options for new npu clock
 * @return      N/A
 */
void kdrv_clock_dsp_clock_change(dsp_clk_setting dsp_clock);

/**
 * @brief       Set ADO Clock
 *
 * @param[in]   *pClk         Pointer to pClk, see @ref T_PLLnConfig
 * @return      N/A
 */
void kdrv_clock_ado_clock_set(T_PLLnConfig *pClk);

/**
 * @brief       Initialize all clock(AXI DDR/MRX1/MRX0/NPU/DSP/ADO)
 *
 * @param[in]   *clk         Pointer to clk, see @ref sysclockopt
 * @return      N/A
 */
void kdrv_clock_inititialize(sysclockopt *clk);

/**
 * @brief       Set SCU clock source in
 *
 * @param[in]   type         Pointer to pClk, see @ref scuclkin_type
 * @param[in]   enable     enable or disable PLL control register
 * @return      N/A
 */
void kdrv_clock_set_scuclkin(enum scuclkin_type type, bool enable);

/**
 * @brief       Set bus mux
 *
 * @param[in]   type         see @ref busmux_type
 * @return      N/A
 */
void kdrv_clock_set_bus_mux(enum busmux_type type);

/**
 * @brief       Enable NPU clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_enable_npu_clk(void);

/**
 * @brief       Enable DSP clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_enable_dsp_clk(void);

/**
 * @brief       Disable NPU clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_disable_npu_clk(void);

/**
 * @brief       Disable DSP clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_disable_dsp_clk(void);

/**
 * @brief       Enable U3 clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_enable_u3_clk60_clk(void);

/**
 * @brief       Disable U3 clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_disable_u3_clk60_clk(void);

/**
 * @brief       Enable TDC clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_enable_tdc_xclk_clk(void);

/**
 * @brief       Disable TDC clock
 *
 * @param[in]  N/A
 * @return      N/A
 */
void kdrv_clock_disable_tdc_xclk_clk(void);

/**
 * @brief       Set CSI clock
 *
 * @param[in]  cam_idx      camera index
 * @param[in]  enable       enable or disable camera
 * @return      N/A
 */
void kdrv_clock_set_csiclk(uint32_t cam_idx, uint32_t enable);

#endif
