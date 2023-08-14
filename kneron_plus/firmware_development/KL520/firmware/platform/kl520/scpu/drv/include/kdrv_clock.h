/**
 * @file        kdrv_clock.h
 * @brief       Kneron generic clock driver
 * @version     1.0
 * @copyright   (c) 2020 Kneron Inc. All right reserved.
 */
#ifndef __KDRV_CLOCK_H__
#define __KDRV_CLOCK_H__

#include <stdint.h>
#include <base.h>
#include "kdrv_status.h"

/**
 * @brief Enumeration of clock type
 */
enum clk {
    CLK_PLL1            = 1,        /**< Enum 1*/
    CLK_PLL1_OUT,                   /**< Enum 2*/
    CLK_PLL2,                       /**< Enum 3*/
    CLK_PLL2_OUT,                   /**< Enum 4*/
    CLK_PLL3,                       /**< Enum 5*/
    CLK_PLL3_OUT1,                  /**< Enum 6*/
    CLK_PLL3_OUT2,                  /**< Enum 7*/
    CLK_PLL4,                       /**< Enum 8*/
    CLK_PLL4_OUT,                   /**< Enum 9*/
    CLK_PLL5,                       /**< Enum 10*/
    CLK_PLL5_OUT1,                  /**< Enum 11*/
    CLK_PLL5_OUT2,                  /**< Enum 12*/

    CLK_FCS_PLL2        = 20,       /**< Enum 20*/
    CLK_FCS_DLL,                    /**< Enum 21*/
    CLK_PLL4_FREF_PLL0,             /**< Enum 22*/

    CLK_BUS_SAHB        = 30,       /**< Enum 30*/
    CLK_BUS_NAHB,                   /**< Enum 31*/
    CLK_BUS_PAHB1,                  /**< Enum 32*/
    CLK_BUS_PAHB2,                  /**< Enum 33*/
    CLK_BUS_APB0,                   /**< Enum 34*/
    CLK_BUS_APB1,                   /**< Enum 35*/

    CLK_SCPU            = 50,       /**< Enum 50*/
    CLK_SCPU_TRACE,                 /**< Enum 51*/

    CLK_NCPU            = 60,       /**< Enum 60*/
    CLK_NCPU_TRACE,                 /**< Enum 61*/
    CLK_NPU,                        /**< Enum 62*/

    /* Peripheral clocks */
    CLK_SPI_CLK         = 100,      /**< Peripheral clocks, Enum 100*/
    CLK_ADC_CLK,                    /**< Peripheral clocks, Enum 101*/
    CLK_WDT_EXT_CLK,                /**< Peripheral clocks, Enum 102*/
    CLK_SD_CLK,                     /**< Peripheral clocks, Enum 103*/
    CLK_MIPI_TXHSPLLREF_CLK,        /**< Peripheral clocks, Enum 104*/
    CLK_MIPI_TX_ESC_CLK,            /**< Peripheral clocks, Enum 105*/
    CLK_MIPI_CSITX_DSI_CLK,         /**< Peripheral clocks, Enum 106*/
    CLK_MIPI_CSITX_CSI_CLK,         /**< Peripheral clocks, Enum 107*/
    CLK_MIPI_CSIRX1_TXESC_CLK,      /**< Peripheral clocks, Enum 108*/
    CLK_MIPI_CSIRX1_CSI_CLK,        /**< Peripheral clocks, Enum 109*/
    CLK_MIPI_CSIRX1_VC0_CLK,        /**< Peripheral clocks, Enum 110*/
    CLK_MIPI_CSIRX0_TXESC_CLK,      /**< Peripheral clocks, Enum 111*/
    CLK_MIPI_CSIRX0_CSI_CLK,        /**< Peripheral clocks, Enum 112*/
    CLK_MIPI_CSIRX0_VC0_CLK,        /**< Peripheral clocks, Enum 113*/
    CLK_LC_SCALER,                  /**< Peripheral clocks, Enum 114*/
    CLK_LC_CLK,                     /**< Peripheral clocks, Enum 115*/
    CLK_TMR1_EXTCLK3,               /**< Peripheral clocks, Enum 116*/
    CLK_TMR1_EXTCLK2,               /**< Peripheral clocks, Enum 117*/
    CLK_TMR1_EXTCLK1,               /**< Peripheral clocks, Enum 118*/
    CLK_TMR0_EXTCLK3,               /**< Peripheral clocks, Enum 119*/
    CLK_TMR0_EXTCLK2,               /**< Peripheral clocks, Enum 120*/
    CLK_TMR0_EXTCLK1,               /**< Peripheral clocks, Enum 121*/
    CLK_PWM_EXTCLK6,                /**< Peripheral clocks, Enum 122*/
    CLK_PWM_EXTCLK5,                /**< Peripheral clocks, Enum 123*/
    CLK_PWM_EXTCLK4,                /**< Peripheral clocks, Enum 124*/
    CLK_PWM_EXTCLK3,                /**< Peripheral clocks, Enum 125*/
    CLK_PWM_EXTCLK2,                /**< Peripheral clocks, Enum 126*/
    CLK_PWM_EXTCLK1,                /**< Peripheral clocks, Enum 127*/
    CLK_UART1_3_FREF,               /**< Peripheral clocks, Enum 128*/
    CLK_UART1_2_FREF,               /**< Peripheral clocks, Enum 129*/
    CLK_UART1_1_FREF,               /**< Peripheral clocks, Enum 130*/
    CLK_UART1_0_FREF,               /**< Peripheral clocks, Enum 131*/
    CLK_UART0_FREF,                 /**< Peripheral clocks, Enum 132*/    
    CLK_SSP1_1_SSPCLK,              /**< Peripheral clocks, Enum 133*/
    CLK_SSP1_0_SSPCLK,              /**< Peripheral clocks, Enum 134*/
    CLK_SSP0_1_SSPCLK,              /**< Peripheral clocks, Enum 135*/
    CLK_SSP0_0_SSPCLK               /**< Peripheral clocks, Enum 136*/
};

//#define MIPI_CLOCK_INIT_AT_DRIVER

//enum clock_mux_selection {
//    ncpu_traceclk_default = 0x10000000,
//    ncpu_traceclk_from_scpu_traceclk = 0x20000000,
//    scpu_traceclk_src_pll0div3 = 0x01000000,
//    scpu_traceclk_src_pll0div2 = 0x02000000,    
//};

/**
 * @brief Enumeration of PLL ID
 */
enum pll_id {
    /* pll_0 = 0, */
    pll_1 = 0,      /**< Enum 0*/
    pll_2,          /**< Enum 1*/
    pll_3,          /**< Enum 2*/
    pll_4,          /**< Enum 3*/
    pll_5           /**< Enum 4*/
};

/**
 * @brief Enumeration of SCPU clock in type
 */
enum scuclkin_type {
    scuclkin_osc = 0,       /**< Enum 0*/
    scuclkin_rtcosc,        /**< Enum 1*/
    scuclkin_pll0div3,      /**< Enum 2*/
    scuclkin_pll0div4       /**< Enum 3*/
};

/**
 * @brief Structure of clock value
 */
struct kdrv_clock_value {
    uint16_t ms;        /**<ms*/
    uint16_t ns;        /**<ns*/
    uint16_t ps;        /**<ps*/
    uint8_t div;        /**< Divider */
    uint8_t enable;     /**< Enable or disable */
};

/**
 * @brief Structure of clock list
 */
struct kdrv_clock_list {
    struct kdrv_clock_list *next;        /**< next pointer to @ref struct kdrv_clock_list*/
};


struct kdrv_clock_node;
/**
 * @brief Function pointer to set clock node
 */
typedef int (*fn_set)(struct kdrv_clock_node *, struct kdrv_clock_value *);

/**
 * @brief Structure of clock list node element
 */
struct kdrv_clock_node {
    struct kdrv_clock_node *parent;         /**< Parent pointer to @ref struct kdrv_clock_node*/
    struct kdrv_clock_node *child_head;     /**< Child head pointer to @ref struct kdrv_clock_node*/
    struct kdrv_clock_node *child_next;     /**< Child next pointer to @ref struct kdrv_clock_node*/
    fn_set set;                             /**< Function pointer, @ref fn_set*/
    uint8_t is_enabled;                     /**< Is the clock node enabled*/
    char name[15];                          /**< String of clock name*/
};

/**
 * @brief       Delay us
 *
 * @param[in]   usec    usec what to delay
 */
void kdrv_delay_us(uint32_t usec);

/**
 * @brief       Initialize all clock
 */
void kdrv_clock_mgr_init(void);

/**
 * @brief       Open the specific clock and active it
 *
 * @param[in]  *node        Pointer to struct @ref kdrv_clock_node
 * @param[in]  *clock_val   Pointer to struct @ref kdrv_clock_value
 */
void kdrv_clock_mgr_open(struct kdrv_clock_node *node, struct kdrv_clock_value *clock_val);

/**
 * @brief       Close the specific clock and deactive it
 *
 * @param[in]  *node        Pointer to struct @ref kdrv_clock_node
 */
void kdrv_clock_mgr_close(struct kdrv_clock_node *node);

/**
 * @brief       Set SCU clock source in
 *
 * @param[in]   type        see @ref scuclkin_type
 * @param[in]   enable     enable or disable PLL control register
 */
void kdrv_clock_mgr_set_scuclkin(enum scuclkin_type type, bool enable);

/**
 * @brief       Set clock mux selection
 *
 * @param[in]   flags         Flags mask
 */
void kdrv_clock_mgr_set_muxsel(uint32_t flags);

/**
 * @brief       Open Clock PLL1
 */
void kdrv_clock_mgr_open_pll1(void);

/**
 * @brief       Open Clock PLL2
 */
void kdrv_clock_mgr_open_pll2(void);

/**
 * @brief       Open Clock PLL3
 */
void kdrv_clock_mgr_open_pll3(void);

/**
 * @brief       Open Clock PLL4
 */
void kdrv_clock_mgr_open_pll4(void);

/**
 * @brief       Open Clock PLL5
 */
void kdrv_clock_mgr_open_pll5(void);

/**
 * @brief       Close Clock PLL1
 */
void kdrv_clock_mgr_close_pll1(void);

/**
 * @brief       Close Clock PLL2
 */
void kdrv_clock_mgr_close_pll2(void);

/**
 * @brief       Close Clock PLL4
 *
 */
void kdrv_clock_mgr_close_pll4(void);

/**
 * @brief       Change clock PLL3
 *
 * @param[in]   ms              milli-seconds
 * @param[in]   ns              nano-seconds
 * @param[in]   ps              pico-seconds
 * @param[in]   csi0_txesc      CSI0 txesc
 * @param[in]   csi0_csi        CSI0 csi
 * @param[in]   csi0_vc0        CSI0 vc0
 * @param[in]   csi1_txesc      CSI1 txesc
 * @param[in]   csi1_csi        CSI1 csi
 * @param[in]   csi1_vc0        CSI1 vc0
 */
void kdrv_clock_mgr_change_pll3_clock(uint32_t ms, uint32_t ns, uint32_t ps, 
        uint32_t csi0_txesc, uint32_t csi0_csi, uint32_t csi0_vc0,
        uint32_t csi1_txesc, uint32_t csi1_csi, uint32_t csi1_vc0);

/**
 * @brief       Change clock PLL5
 *
 * @param[in]   ms  milli-seconds
 * @param[in]   ns  nano-seconds
 * @param[in]   ps  pico-seconds
 */
void kdrv_clock_mgr_change_pll5_clock(uint32_t ms, uint32_t ns, uint32_t ps);

/**
 * @brief       Set MIPI CSI clock
 *
 * @param[in]   cam_idx   Index of MIPI camera
 * @param[in]   enable    Enable or Disable
 */
void kdrv_clock_set_csiclk(uint32_t cam_idx, uint32_t enable);

/**
 * @brief       Enable Clock
 *
 * @param[in]   clk @ref enum clk
 */
void kdrv_clock_enable(enum clk clk);

/**
 * @brief       Disable Clock
 *
 * @param[in]   clk     @ref enum clk
 */
void kdrv_clock_disable(enum clk clk);


#endif
