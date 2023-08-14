//#define USBD3_DBG
#define USE_FAKE_ZLP
#define ERROR_WATCH_REMOVE_IT_IN_THE_FUTURE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "base.h"
#include "regbase.h"
#include "kdrv_cmsis_core.h"

#include "kdrv_scu.h"
#include "kdrv_scu_ext.h"
#include "kdrv_usbd3.h"

#ifndef _BOARD_SN720HAPS_H_
#include "kdrv_system.h"
#endif
#ifdef USBD3_DBG

#define usbd3_dbg(__format__, ...)  printf(LOG_CUSTOM, __format__, ##__VA_ARGS__)
#else
#define usbd3_dbg(__format__, ...)
#endif

// below are FOTG330 register offset and bit definitions

// Global Interrupt Enable Register
// 0x180C
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t OTGINTR_EN : 1;  // OTG interrupt enable
        uint32_t HOSTINTR_EN : 1; // Host interrupt enable
        uint32_t DEVINTR_EN : 1;  // Device interrupt enable
        uint32_t reserved : 29;   // reserved
    } bf;
} U_regUSBD_GLBINTREN;
#define regUSBD_GLBINTREN ((U_regUSBD_GLBINTREN *)(USB3_REG_BASE + 0x180C))

// OTG Interrupt Enable Register
// 0x1808
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t IDCHG_EN : 1;   // ID change interrupt enable
        uint32_t ROLECHG_EN : 1; // Role change interrupt enable
        uint32_t reserved : 30;  // reserved
    } bf;
} U_regUSBD_OTGINTREN;
#define regUSBD_OTGINTREN ((U_regUSBD_OTGINTREN *)(USB3_REG_BASE + 0x1808))

// Global Control Register
// 0x2000
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t DEV_MODE : 2;            // Device Speed Status
        uint32_t reserved : 1;            // reserved
        uint32_t FIFO_CLR : 1;            // FIFO Clear
        uint32_t SYNC_FIFO0_CLR : 1;      // SYNC-FIFO0 Clear (For debugging usage only)
        uint32_t SYNC_FIFO1_CLR : 1;      // SYNC-FIFO1 Clear (For debugging usage only)
        uint32_t USB3_LPMEN : 1;          // Force USB 3.0 LPMEN
        uint32_t VBUS_STATUS : 1;         // VBUS Status
        uint32_t SF_RST : 1;              // SW Reset
        uint32_t POLL_TO_DIS_TIMEOUT : 1; // Timeout of POLL_TO_DIS_CNT
        uint32_t USB2_PLUG : 1;           // USB2 in the Plug Mode
        uint32_t reserved2 : 1;           // reserved
        uint32_t POLL_TO_DIS_CNT : 4;     // Counter of Polling to Disable
        uint32_t reserved3 : 16;          // reserved
    } bf;
} U_regUSBD_GCR;
#define regUSBD_GCR ((U_regUSBD_GCR *)(USB3_REG_BASE + 0x2000))

// Global Test Mode Register
// 0x2004
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t TST_MODE : 1;         // Turbo Mode
        uint32_t TST_FIFO_DEG : 1;     // FIFO Debug
        uint32_t reserved : 2;         // reserved
        uint32_t TST_EP_NUM : 4;       // EP Number in the Debug Mode
        uint32_t TST_EP_ENTRY : 4;     // EP Entry in the Debug Mode
        uint32_t TST_CUR_EP_ENTRY : 4; // Number of Current Entries
        uint32_t TST_DIS_SOFGEN : 1;   // Disable the Generation of SOF
        uint32_t TST_FORCE_SS : 1;     // Force the SuperSpeed Device
        uint32_t TST_FORCE_HS : 1;     // Force the High-Speed Device
        uint32_t TST_FORCE_FS : 1;     // Force the Full-Speed Device
        uint32_t reserved2 : 12;       // reserved
    } bf;
} U_regUSBD_GTM;
#define regUSBD_GTM ((U_regUSBD_GTM *)(USB3_REG_BASE + 0x2004))

// Device Address Register
// 0x2008
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t DEV_ADDR : 7;   // Device Address
        uint32_t SET_CONFIG : 1; // SET_CONFIGURATION done
        uint32_t reserved : 24;  // reserved
    } bf;
} U_regUSBD_DAR;
#define regUSBD_DAR ((U_regUSBD_DAR *)(USB3_REG_BASE + 0x2008))

// Control Transfer Data Port Register
// 0x2010
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t CX_DATA_PORT : 32; // The Data Port of Control Transfer
    } bf;
} U_regUSBD_CX_PORT;
#define regUSBD_CX_PORT ((U_regUSBD_CX_PORT *)(USB3_REG_BASE + 0x2010))

// Control Transfer Configuration and Status Register
// 0x200C
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t CX_DONE : 1;   // Data Transfer is Done for Control Transfer
        uint32_t CX_STL : 1;    // Stall CX
        uint32_t CXF_CLR : 1;   // Clear CX FIFO Data (For debugging usage only)
        uint32_t CXF_FUL : 1;   // CX FIFO is Full
        uint32_t CXF_EMP : 1;   // CX FIFO is Empty
        uint32_t reserved : 3;  // reserved
        uint32_t CXF_LEN : 16;  // Data Length of CX FIFO
        uint32_t reserved2 : 8; // reserved
    } bf;
} U_regUSBD_CX_Config_Status;
#define regUSBD_CX_Config_Status ((U_regUSBD_CX_Config_Status *)(USB3_REG_BASE + 0x200C))

// EPn_SET0 : 0x2020 + (n - 1) * 0x030, n = 1 ~ 15 (index = 0 ~ 14)
// EPn_SET1 : 0x2024 + (n - 1) * 0x030, n = 1 ~ 15 (index = 0 ~ 14)
// EPn_SET2 : 0x2028 + (n - 1) * 0x030, n = 1 ~ 15 (index = 0 ~ 14)
// EPn_FFR : 0x202C + (n - 1) * 0x030, n = 1 ~ 15 (index = 0 ~ 14)
typedef volatile union
{
    struct
    {
        uint32_t EPn_SET0;     // 0x2020 + 0x30 * n
        uint32_t EPn_SET1;     // 0x2024 + 0x30 * n
        uint32_t EPn_SET2;     // 0x2028 + 0x30 * n
        uint32_t EPn_FFR;      // 0x202C + 0x30 * n
        uint32_t reserved[4];  // reserved
        uint32_t EPn_STR_ID;   // 0x2040 + 0x30 * n
        uint32_t reserved2[3]; // reserved
    } dw;

    struct
    {
        // EPn_SET0
        uint32_t EPn_STL : 1;        // EPn Stall
        uint32_t reserved : 1;       // reserved
        uint32_t EPn_CLR_SEQNUM : 1; // Reset Sequence/Toggle bit of EP
        uint32_t EPn_STL_CLR : 1;    // EPn Stall Clear
        uint32_t reserved2 : 28;     // reserved

        // EPn_SET1
        uint32_t EPn_ACTIVE : 1;           // EPn Active
        uint32_t EPn_DIR : 1;              // EPn Direction
        uint32_t EPn_TYPE : 2;             // Transfer Type
        uint32_t EPn_BW_NUM : 2;           // Transaction number for the High-Speed ISO bandwidth
        uint32_t EPn_ISO_IN_PKTNUM : 6;    // The number of packets for ISO IN during each service interval in the SS mode
        uint32_t EPn_FIFO_ENTRY : 5;       // The number of the FIFO entries for EPn to store the data
        uint32_t reserved3 : 7;            // reserved
        uint32_t EPn_START_FIFO_ENTRY : 8; // The first index of the EP FIFO entry

        // EPn_SET2
        uint32_t EPn_MPS : 11;         // Max Packet Size of EPn
        uint32_t reserve4 : 5;         // reserved
        uint32_t EPn_FF_ADDR_OFS : 15; // EPn Base FIFO Address Offset
        uint32_t reserve5 : 1;         // reserved

        // EPn_FFR
        uint32_t EPn_FF_BYCNT : 21; // EPn Transfer Byte Count
        uint32_t reserve6 : 7;      // reserved
        uint32_t EPn_TX0BYTE : 1;   // EPn Transfer 0 Byte
        uint32_t EPn_FF_EMPTY : 1;  // EPn FIFO Empty
        uint32_t EPn_FF_FULL : 1;   // EPn FIFO Full
        uint32_t EPn_FF_RST : 1;    // EPn FIFO Reset

        uint32_t reserved_left[8]; // Note: Stream functionality is not supported
    } bf;
} U_regUSBD_EPn;
#define regUSBD_EP ((U_regUSBD_EPn *)(USB3_REG_BASE + 0x2020)) // per-endpoint registers in array form

// EPn_PRD_TABLE
// 0x2520 + (n - 1) * 0x10, n = 1 ~ 15 (index = 0 ~ 14)
typedef volatile union
{

    struct
    {
        uint32_t word_0;
        uint32_t word_1;
        uint32_t word_2;
        uint32_t word_nouse;
    } dw;

    struct
    {
        uint32_t BTC : 24;        // Byte Transfer Count
        uint32_t H_hw_own : 1;    // Hardware Owner (HW can update)
        uint32_t L_last_prd : 1;  // Last PRD Table
        uint32_t A_add_ctrl : 1;  // DMA Address Control
        uint32_t I_int_en : 1;    // PRD Transfer Finish Interrupt Enable
        uint32_t F_trans_fin : 1; // Transaction Finish
        uint32_t O_send_zlp : 1;  // Transmit 0 Byte
        uint32_t reserved : 2;    // reserved
        uint32_t PAR;             // Physical Address
        uint32_t EPRD_PTR;        // External PRD Table Pointer Address
        uint32_t nouse;           // no use
    } bf;

} U_regUSBD_EPn_PRD_TABLE;
#define regUSBD_EP_PRD ((U_regUSBD_EPn_PRD_TABLE *)(USB3_REG_BASE + 0x2520)) // per-endpoint IPRD in array form

// HS Control Register
// 0x2304
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t IDLE_CNT : 3;            //
        uint32_t CAP_RMWKUP : 1;          // Capability of Remote Wake-up
        uint32_t HS_GORMWKUP : 1;         // HS Go Remote Wake-up
        uint32_t HS_GOSUSP : 1;           // HS Go Suspend
        uint32_t CAP_LPM_RMWKUP : 1;      // Capability of Remote Wake-up from LPM
        uint32_t HS_LPM_RMWKUP : 1;       // Remote Wake-up from LPM
        uint32_t HS_LPM_PERMIT : 1;       // HS LPM Permission
        uint32_t LPM_BESL : 4;            // HS LPM BESL
        uint32_t LPM_BESL_PERMIT_MIN : 4; // Minimum BESL for LPM Permission
        uint32_t LPM_BESL_PERMIT_MAX : 4; // Maximum BESL for LPM Permission
        uint32_t HW_LPM_RMWKUP_IN : 1;    // Enable Hardware LPM Remote Wakeup for IN Transfer
        uint32_t HW_LPM_RMWKUP_OUT : 1;   // Enable Hardware LPM Remote Wakeup for OUT Transfer
        uint32_t HW_LPM_RMWKUP_CX : 1;    // Enable Hardware LPM Remote Wakeup for Control Status Stage
        uint32_t HW_LPM_REJECT_IN : 1;    // Enable HW LPM Reject for IN Transfer
        uint32_t HW_LPM_REJECT_OUT : 1;   // Enable HW LPM Reject for OUT Transfer
        uint32_t HW_LPM_REJECT_CX : 1;    // Enable HW LPM Reject for Control Status Stage
        uint32_t reserved : 5;            // reserved
    } bf;
} U_regUSBD_HS_CR;
#define regUSBD_HS_CR ((U_regUSBD_HS_CR *)(USB3_REG_BASE + 0x2304))

// SS Control Register 0
// 0x2308
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t U1_FUN_EN : 1;        // U1 Function Enable
        uint32_t U2_FUN_EN : 1;        // U2 Function Enable
        uint32_t reserved : 2;         // reserved
        uint32_t MAX_SINT : 3;         // Maximum Service Interval
        uint32_t SSI_EN : 1;           // Support Smart ISO Enable
        uint32_t SSI_WAKEUP_DELAY : 2; // This field indicates that the time FOTG330 needs to wake up from U2 to U0. The unit is a bus-interval (125 μs)
        uint32_t reserved2 : 22;       // reserved
    } bf;
} U_regUSBD_SS_CR0;
#define regUSBD_SS_CR0 ((U_regUSBD_SS_CR0 *)(USB3_REG_BASE + 0x2308))

// SS Control Register 1
// 0x230C
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t U1_ENTRY_EN : 1;    // Device Active U1 Entry Enable
        uint32_t U2_ENTRY_EN : 1;    // Device Active U2 Entry Enable
        uint32_t U1_EXIT_EN : 1;     // Device Active U1 Exit Enable
        uint32_t U2_EXIT_EN : 1;     // Device Active U2 Exit Enable
        uint32_t U3_WAKEUP_EN : 1;   // Device Actively U3 Wake-up Enable
        uint32_t FORCE_RECOVERY : 1; // Force Entry Recovery State
        uint32_t DIS_SCRMB : 1;      // Scramble Disable
        uint32_t reserved : 1;       // reserved
        uint32_t GO_U3_DONE : 1;     // Go U3 Done
        uint32_t TXDEEMPH_LEVEL : 2; // Txdeemph Level
        uint32_t reserved2 : 21;     // reserved
    } bf;
} U_regUSBD_SS_CR1;
#define regUSBD_SS_CR1 ((U_regUSBD_SS_CR1 *)(USB3_REG_BASE + 0x230C))

// SS Control Register 2
// 0x2310
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t U2_TIMEOUT : 8;          // Timer Initial U2 Entry
        uint32_t U1_TIMEOUT : 8;          // Timer Initial U1 Entry
        uint32_t U2_INACT_TIMEOUT : 8;    // U2 Inactivity Timeout
        uint32_t FORCE_LINKPM_ACCEPT : 1; // Force LinkPM Accept
        uint32_t SS_TX_SWING : 1;         // Tx Swing Control
        uint32_t reserved : 6;            // reserved
    } bf;
} U_regUSBD_SS_CR2;
#define regUSBD_SS_CR2 ((U_regUSBD_SS_CR2 *)(USB3_REG_BASE + 0x2310))

// MISC Control Register (VBUS)
// 0x2328
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t VBUS_DEBOUNCE_TIME : 10; // VBUS Debounce Time
        uint32_t reserved : 22;           // reserved
    } bf;
} U_regUSBD_MISC_CR;
#define regUSBD_MISC_CR ((U_regUSBD_MISC_CR *)(USB3_REG_BASE + 0x2328))

// Interrupt Group 0 Enable Register
// 0x2420
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t reserved : 1;         // reserved
        uint32_t EP1_FIFO_INT_EN : 1;  // Enable the EP1 FIFO Interrupt
        uint32_t EP2_FIFO_INT_EN : 1;  // Enable the EP2 FIFO Interrupt
        uint32_t EP3_FIFO_INT_EN : 1;  // Enable the EP3 FIFO Interrupt
        uint32_t EP4_FIFO_INT_EN : 1;  // Enable the EP4 FIFO Interrupt
        uint32_t EP5_FIFO_INT_EN : 1;  // Enable the EP5 FIFO Interrupt
        uint32_t EP6_FIFO_INT_EN : 1;  // Enable the EP6 FIFO Interrupt
        uint32_t EP7_FIFO_INT_EN : 1;  // Enable the EP7 FIFO Interrupt
        uint32_t EP8_FIFO_INT_EN : 1;  // Enable the EP8 FIFO Interrupt
        uint32_t EP9_FIFO_INT_EN : 1;  // Enable the EP9 FIFO Interrupt
        uint32_t EP10_FIFO_INT_EN : 1; // Enable the EP10 FIFO Interrupt
        uint32_t EP11_FIFO_INT_EN : 1; // Enable the EP11 FIFO Interrupt
        uint32_t EP12_FIFO_INT_EN : 1; // Enable the EP12 FIFO Interrupt
        uint32_t EP13_FIFO_INT_EN : 1; // Enable the EP13 FIFO Interrupt
        uint32_t EP14_FIFO_INT_EN : 1; // Enable the EP14 FIFO Interrupt
        uint32_t EP15_FIFO_INT_EN : 1; // Enable the EP15 FIFO Interrupt
        uint32_t reserved2 : 1;        // reserved
        uint32_t EP1_PRD_INT_EN : 1;   // Enable the EP1 PRD Interrupt
        uint32_t EP2_PRD_INT_EN : 1;   // Enable the EP2 PRD Interrupt
        uint32_t EP3_PRD_INT_EN : 1;   // Enable the EP3 PRD Interrupt
        uint32_t EP4_PRD_INT_EN : 1;   // Enable the EP4 PRD Interrupt
        uint32_t EP5_PRD_INT_EN : 1;   // Enable the EP5 PRD Interrupt
        uint32_t EP6_PRD_INT_EN : 1;   // Enable the EP6 PRD Interrupt
        uint32_t EP7_PRD_INT_EN : 1;   // Enable the EP7 PRD Interrupt
        uint32_t EP8_PRD_INT_EN : 1;   // Enable the EP8 PRD Interrupt
        uint32_t EP9_PRD_INT_EN : 1;   // Enable the EP9 PRD Interrupt
        uint32_t EP10_PRD_INT_EN : 1;  // Enable the EP10 PRD Interrupt
        uint32_t EP11_PRD_INT_EN : 1;  // Enable the EP11 PRD Interrupt
        uint32_t EP12_PRD_INT_EN : 1;  // Enable the EP12 PRD Interrupt
        uint32_t EP13_PRD_INT_EN : 1;  // Enable the EP13 PRD Interrupt
        uint32_t EP14_PRD_INT_EN : 1;  // Enable the EP14 PRD Interrupt
        uint32_t EP15_PRD_INT_EN : 1;  // Enable the EP15 PRD Interrupt
    } bf;
} U_regUSBD_IGER0;
#define regUSBD_IGER0 ((U_regUSBD_IGER0 *)(USB3_REG_BASE + 0x2420))

// Interrupt Group 0 Register (PRD/DMA/FIFO interrupt)
// 0x2400
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t reserved : 1;      // reserved
        uint32_t EP1_FIFO_INT : 1;  // EP1 FIFO Interrupt
        uint32_t EP2_FIFO_INT : 1;  // EP2 FIFO Interrupt
        uint32_t EP3_FIFO_INT : 1;  // EP3 FIFO Interrupt
        uint32_t EP4_FIFO_INT : 1;  // EP4 FIFO Interrupt
        uint32_t EP5_FIFO_INT : 1;  // EP5 FIFO Interrupt
        uint32_t EP6_FIFO_INT : 1;  // EP6 FIFO Interrupt
        uint32_t EP7_FIFO_INT : 1;  // EP7 FIFO Interrupt
        uint32_t EP8_FIFO_INT : 1;  // EP8 FIFO Interrupt
        uint32_t EP9_FIFO_INT : 1;  // EP9 FIFO Interrupt
        uint32_t EP10_FIFO_INT : 1; // EP10 FIFO Interrupt
        uint32_t EP11_FIFO_INT : 1; // EP11 FIFO Interrupt
        uint32_t EP12_FIFO_INT : 1; // EP12 FIFO Interrupt
        uint32_t EP13_FIFO_INT : 1; // EP13 FIFO Interrupt
        uint32_t EP14_FIFO_INT : 1; // EP14 FIFO Interrupt
        uint32_t EP15_FIFO_INT : 1; // EP15 FIFO Interrupt
        uint32_t reserved2 : 1;     // reserved
        uint32_t EP1_PRD_INT : 1;   // EP1 PRD Interrupt
        uint32_t EP2_PRD_INT : 1;   // EP2 PRD Interrupt
        uint32_t EP3_PRD_INT : 1;   // EP3 PRD Interrupt
        uint32_t EP4_PRD_INT : 1;   // EP4 PRD Interrupt
        uint32_t EP5_PRD_INT : 1;   // EP5 PRD Interrupt
        uint32_t EP6_PRD_INT : 1;   // EP6 PRD Interrupt
        uint32_t EP7_PRD_INT : 1;   // EP7 PRD Interrupt
        uint32_t EP8_PRD_INT : 1;   // EP8 PRD Interrupt
        uint32_t EP9_PRD_INT : 1;   // EP9 PRD Interrupt
        uint32_t EP10_PRD_INT : 1;  // EP10 PRD Interrupt
        uint32_t EP11_PRD_INT : 1;  // EP11 PRD Interrupt
        uint32_t EP12_PRD_INT : 1;  // EP12 PRD Interrupt
        uint32_t EP13_PRD_INT : 1;  // EP13 PRD Interrupt
        uint32_t EP14_PRD_INT : 1;  // EP14 PRD Interrupt
        uint32_t EP15_PRD_INT : 1;  // EP15 PRD Interrupt
    } bf;
} U_regUSBD_IGR0;
#define regUSBD_IGR0 ((U_regUSBD_IGR0 *)(USB3_REG_BASE + 0x2400))

// Interrupt Group 1 Enable Register
// 0x2424

#define INT_GRP2_EN_BIT BIT0
#define INT_GRP3_EN_BIT BIT1
#define INT_GRP4_EN_BIT BIT2
#define CX_SETUP_INT_EN_BIT BIT3
#define CX_IN_INT_EN_BIT BIT4
#define CX_OUT_INT_EN_BIT BIT5
#define CX_CMDEND_INT_EN_BIT BIT6
#define CX_CMDFAIL_INT_EN_BIT BIT7
#define CX_COMABT_INT_EN_BIT BIT8
#define DEV_MODE_CHG_INT_EN_BIT BIT9
#define LINK_IN_TEST_MODE_INT_EN_BIT BIT10
#define HS_RST_INT_EN_BIT BIT11
#define HS_LPM_INT_EN_BIT BIT12
#define HS_SUSP_INT_EN_BIT BIT13
#define HS_RESM_INT_EN_BIT BIT14
#define WARM_RST_INT_EN_BIT BIT15
#define HOT_RST_INT_EN_BIT BIT16
#define U1_ENTRY_INT_EN_BIT BIT17
#define U2_ENTRY_INT_EN_BIT BIT18
#define U3_ENTRY_INT_EN_BIT BIT19
#define U1_EXIT_INT_EN_BIT BIT20
#define U2_EXIT_INT_EN_BIT BIT21
#define U3_EXIT_INT_EN_BIT BIT22
#define U1_ENTRY_FAIL_INT_EN_BIT BIT23
#define U2_ENTRY_FAIL_INT_EN_BIT BIT24
#define U1_EXIT_FAIL_INT_EN_BIT BIT25
#define U2_EXIT_FAIL_INT_EN_BIT BIT26
#define U3_EXIT_FAIL_INT_EN_BIT BIT27
#define SYNF0_EMPTY_INT_EN_BIT BIT28
#define SYNF1_EMPTY_INT_EN_BIT BIT29
#define VBUS_CHG_INT_EN_BIT BIT30
#define INT_GRP5_EN_BIT BIT31

#define ENABLE_MASK_IGER_1 (INT_GRP4_EN_BIT | CX_SETUP_INT_EN_BIT | CX_IN_INT_EN_BIT | CX_OUT_INT_EN_BIT | CX_CMDEND_INT_EN_BIT |    \
                            CX_CMDFAIL_INT_EN_BIT | CX_COMABT_INT_EN_BIT | DEV_MODE_CHG_INT_EN_BIT | HS_RST_INT_EN_BIT |             \
                            HS_LPM_INT_EN_BIT | HS_SUSP_INT_EN_BIT | HS_RESM_INT_EN_BIT | WARM_RST_INT_EN_BIT | HOT_RST_INT_EN_BIT | \
                            U3_ENTRY_INT_EN_BIT | U3_EXIT_INT_EN_BIT | U3_EXIT_FAIL_INT_EN_BIT | VBUS_CHG_INT_EN_BIT | INT_GRP5_EN_BIT)

typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t INT_GRP2_EN : 1;              // Enable all Interrupt Bits of Interrupt Group 2
        uint32_t INT_GRP3_EN : 1;              // Enable all Interrupt Bits of Interrupt Group 3
        uint32_t INT_GRP4_EN : 1;              // Enable all Interrupt Bits of Interrupt Group 4
        uint32_t CX_SETUP_INT_EN : 1;          // Enable the EP0 Setup Data Received Interrupt
        uint32_t CX_IN_INT_EN : 1;             // Enable the EP0 Interrupt f for IN
        uint32_t CX_OUT_INT_EN : 1;            // Enable the EP0 Interrupt for OUT
        uint32_t CX_CMDEND_INT_EN : 1;         // Enable the Host End-of-command (Entering status stage) Interrupt
        uint32_t CX_CMDFAIL_INT_EN : 1;        // Enable the Host Emitting Extra IN or OUT Data Interrupt
        uint32_t CX_COMABT_INT_EN : 1;         // Enable the Command Abort Interrupt
        uint32_t DEV_MODE_CHG_INT_EN : 1;      // Enable the Device Mode Change Interrupt
        uint32_t LINK_IN_TEST_MODE_INT_EN : 1; // Enable LTSSM in Electrical Test Mode Interrupt
        uint32_t HS_RST_INT_EN : 1;            // Enable the Bus Reset Interrupt
        uint32_t HS_LPM_INT_EN : 1;            // Enable the LPM State Change Interrupt
        uint32_t HS_SUSP_INT_EN : 1;           // Enable the Suspend State Change Interrupt
        uint32_t HS_RESM_INT_EN : 1;           // Enable the Resume State Change Interrupt
        uint32_t WARM_RST_INT_EN : 1;          // Enable the SS Warm Reset Interrupt
        uint32_t HOT_RST_INT_EN : 1;           // Enable the SS Hot Reset Interrupt
        uint32_t U1_ENTRY_INT_EN : 1;          // Enable the U1 Entry Interrupt
        uint32_t U2_ENTRY_INT_EN : 1;          // Enable the U2 Entry Interrupt
        uint32_t U3_ENTRY_INT_EN : 1;          // Enable the U3 Entry Interrupt
        uint32_t U1_EXIT_INT_EN : 1;           // Enable the U1 Exit Interrupt
        uint32_t U2_EXIT_INT_EN : 1;           // Enable the U2 Exit Interrupt
        uint32_t U3_EXIT_INT_EN : 1;           // Enable the U3 Exit Interrupt
        uint32_t U1_ENTRY_FAIL_INT_EN : 1;     // Enable the U1 Entry Fail Interrupt
        uint32_t U2_ENTRY_FAIL_INT_EN : 1;     // Enable the U2 Entry Fail Interrupt
        uint32_t U1_EXIT_FAIL_INT_EN : 1;      // Enable the U1 Exit Fail Interrupt
        uint32_t U2_EXIT_FAIL_INT_EN : 1;      // Enable the U2 Exit Fail Interrupt
        uint32_t U3_EXIT_FAIL_INT_EN : 1;      // Enable the U3 Exit Fail Interrupt
        uint32_t SYNF0_EMPTY_INT_EN : 1;       // Enable the SYNC FIFO0 Empty interrupt
        uint32_t SYNF1_EMPTY_INT_EN : 1;       // Enable the SYNC FIFO1 Empty Interrupt
        uint32_t VBUS_CHG_INT_EN : 1;          // Enable the VBUS Change Interrupt
        uint32_t INT_GRP5_EN : 1;              // Enable all Interrupt Bits of the Interrupt Group 5
    } bf;
} U_regUSBD_IGER1;
#define regUSBD_IGER1 ((U_regUSBD_IGER1 *)(USB3_REG_BASE + 0x2424))

// Interrupt Group 1 Register (primary and CX interrupts)
// 0x2404
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t INT_GRP2 : 1;              // Interrupt Bits of Interrupt Group 2
        uint32_t INT_GRP3 : 1;              // Interrupt Bits of Interrupt Group 3
        uint32_t INT_GRP4 : 1;              // Interrupt Bits of Interrupt Group 4
        uint32_t CX_SETUP_INT : 1;          // EP0 Setup Data Received Interrupt
        uint32_t CX_IN_INT : 1;             // EP0 Interrupt f for IN
        uint32_t CX_OUT_INT : 1;            // EP0 Interrupt for OUT
        uint32_t CX_CMDEND_INT : 1;         // Host End-of-command (Entering status stage) Interrupt
        uint32_t CX_CMDFAIL_INT : 1;        // Host Emitting Extra IN or OUT Data Interrupt
        uint32_t CX_COMABT_INT : 1;         // Command Abort Interrupt
        uint32_t DEV_MODE_CHG_INT : 1;      // Device Mode Change Interrupt
        uint32_t LINK_IN_TEST_MODE_INT : 1; // LTSSM in Electrical Test Mode Interrupt
        uint32_t HS_RST_INT : 1;            // Bus Reset Interrupt
        uint32_t HS_LPM_INT : 1;            // LPM State Change Interrupt
        uint32_t HS_SUSP_INT : 1;           // Suspend State Change Interrupt
        uint32_t HS_RESM_INT : 1;           // Resume State Change Interrupt
        uint32_t WARM_RST_INT : 1;          // SS Warm Reset Interrupt
        uint32_t HOT_RST_INT : 1;           // SS Hot Reset Interrupt
        uint32_t U1_ENTRY_INT : 1;          // U1 Entry Interrupt
        uint32_t U2_ENTRY_INT : 1;          // U2 Entry Interrupt
        uint32_t U3_ENTRY_INT : 1;          // U3 Entry Interrupt
        uint32_t U1_EXIT_INT : 1;           // U1 Exit Interrupt
        uint32_t U2_EXIT_INT : 1;           // U2 Exit Interrupt
        uint32_t U3_EXIT_INT : 1;           // U3 Exit Interrupt
        uint32_t U1_ENTRY_FAIL_INT : 1;     // U1 Entry Fail Interrupt
        uint32_t U2_ENTRY_FAIL_INT : 1;     // U2 Entry Fail Interrupt
        uint32_t U1_EXIT_FAIL_INT : 1;      // U1 Exit Fail Interrupt
        uint32_t U2_EXIT_FAIL_INT : 1;      // U2 Exit Fail Interrupt
        uint32_t U3_EXIT_FAIL_INT : 1;      // U3 Exit Fail Interrupt
        uint32_t SYNF0_EMPTY_INT : 1;       // SYNC FIFO0 Empty interrupt
        uint32_t SYNF1_EMPTY_INT : 1;       // SYNC FIFO1 Empty Interrupt
        uint32_t VBUS_CHG_INT : 1;          // VBUS Change Interrupt
        uint32_t INT_GRP5 : 1;              // Interrupt Bits of the Interrupt Group 5
    } bf;
} U_regUSBD_IGR1;
#define regUSBD_IGR1 ((U_regUSBD_IGR1 *)(USB3_REG_BASE + 0x2404))

// Interrupt Group 4 Enable Register
// 0x2430
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t no_use : 17;         //
        uint32_t EP1_RX0_INT_EN : 1;  // Enable the EP1 Receive 0 Byte Interrupt
        uint32_t EP2_RX0_INT_EN : 1;  // Enable the EP2 Receive 0 Byte Interrupt
        uint32_t EP3_RX0_INT_EN : 1;  // Enable the EP3 Receive 0 Byte Interrupt
        uint32_t EP4_RX0_INT_EN : 1;  // Enable the EP4 Receive 0 Byte Interrupt
        uint32_t EP5_RX0_INT_EN : 1;  // Enable the EP5 Receive 0 Byte Interrupt
        uint32_t EP6_RX0_INT_EN : 1;  // Enable the EP6 Receive 0 Byte Interrupt
        uint32_t EP7_RX0_INT_EN : 1;  // Enable the EP7 Receive 0 Byte Interrupt
        uint32_t EP8_RX0_INT_EN : 1;  // Enable the EP8 Receive 0 Byte Interrupt
        uint32_t EP9_RX0_INT_EN : 1;  // Enable the EP9 Receive 0 Byte Interrupt
        uint32_t EP10_RX0_INT_EN : 1; // Enable the EP10 Receive 0 Byte Interrupt
        uint32_t EP11_RX0_INT_EN : 1; // Enable the EP11 Receive 0 Byte Interrupt
        uint32_t EP12_RX0_INT_EN : 1; // Enable the EP12 Receive 0 Byte Interrupt
        uint32_t EP13_RX0_INT_EN : 1; // Enable the EP13 Receive 0 Byte Interrupt
        uint32_t EP14_RX0_INT_EN : 1; // Enable the EP14 Receive 0 Byte Interrupt
        uint32_t EP15_RX0_INT_EN : 1; // Enable the EP15 Receive 0 Byte Interrupt
    } bf;
} U_regUSBD_IGER4;
#define regUSBD_IGER4 ((U_regUSBD_IGER4 *)(USB3_REG_BASE + 0x2430))

// OTG Interrupt Status Register
// 0x1804
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t IDCHG : 1;     // ID Change
        uint32_t ROLECHG : 1;   // Role Change
        uint32_t reserved : 30; // reserved
    } bf;
} U_regUSBD_OTGINTRSTS;
#define regUSBD_OTGINTRSTS ((U_regUSBD_OTGINTRSTS *)(USB3_REG_BASE + 0x1804))

// Device Interrupt Status Register
// 0x1810
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t DEVINTR0 : 1;  // Device group0 interrupt
        uint32_t DEVINTR1 : 1;  // Device group1 interrupt
        uint32_t reserved : 30; // reserved
    } bf;
} U_regUSBD_DEVINTRSTS;
#define regUSBD_DEVINTRSTS ((U_regUSBD_DEVINTRSTS *)(USB3_REG_BASE + 0x1810))

// Interrupt Group 4 Register (ZLP interrupt)
// 0x2410
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t no_use : 17;      //
        uint32_t EP1_RX0_INT : 1;  // EP1 Receive 0 Byte Interrupt
        uint32_t EP2_RX0_INT : 1;  // EP2 Receive 0 Byte Interrupt
        uint32_t EP3_RX0_INT : 1;  // EP3 Receive 0 Byte Interrupt
        uint32_t EP4_RX0_INT : 1;  // EP4 Receive 0 Byte Interrupt
        uint32_t EP5_RX0_INT : 1;  // EP5 Receive 0 Byte Interrupt
        uint32_t EP6_RX0_INT : 1;  // EP6 Receive 0 Byte Interrupt
        uint32_t EP7_RX0_INT : 1;  // EP7 Receive 0 Byte Interrupt
        uint32_t EP8_RX0_INT : 1;  // EP8 Receive 0 Byte Interrupt
        uint32_t EP9_RX0_INT : 1;  // EP9 Receive 0 Byte Interrupt
        uint32_t EP10_RX0_INT : 1; // EP10 Receive 0 Byte Interrupt
        uint32_t EP11_RX0_INT : 1; // EP11 Receive 0 Byte Interrupt
        uint32_t EP12_RX0_INT : 1; // EP12 Receive 0 Byte Interrupt
        uint32_t EP13_RX0_INT : 1; // EP13 Receive 0 Byte Interrupt
        uint32_t EP14_RX0_INT : 1; // EP14 Receive 0 Byte Interrupt
        uint32_t EP15_RX0_INT : 1; // EP15 Receive 0 Byte Interrupt
    } bf;
} U_regUSBD_IGR4;
#define regUSBD_IGR4 ((U_regUSBD_IGR4 *)(USB3_REG_BASE + 0x2410))

// EP PRD Ready Register
// 0x2504
typedef volatile union
{
    uint32_t dw;
    struct
    {
        uint32_t reserved : 1;     // reserved
        uint32_t EP1_PRD_RDY : 1;  // EP1 PRD is ready for the internal DMA to access
        uint32_t EP2_PRD_RDY : 1;  // EP2 PRD is ready for the internal DMA to access
        uint32_t EP3_PRD_RDY : 1;  // EP3 PRD is ready for the internal DMA to access
        uint32_t EP4_PRD_RDY : 1;  // EP4 PRD is ready for the internal DMA to access
        uint32_t EP5_PRD_RDY : 1;  // EP5 PRD is ready for the internal DMA to access
        uint32_t EP6_PRD_RDY : 1;  // EP6 PRD is ready for the internal DMA to access
        uint32_t EP7_PRD_RDY : 1;  // EP7 PRD is ready for the internal DMA to access
        uint32_t EP8_PRD_RDY : 1;  // EP8 PRD is ready for the internal DMA to access
        uint32_t EP9_PRD_RDY : 1;  // EP9 PRD is ready for the internal DMA to access
        uint32_t EP10_PRD_RDY : 1; // EP10 PRD is ready for the internal DMA to access
        uint32_t EP11_PRD_RDY : 1; // EP11 PRD is ready for the internal DMA to access
        uint32_t EP12_PRD_RDY : 1; // EP12 PRD is ready for the internal DMA to access
        uint32_t EP13_PRD_RDY : 1; // EP13 PRD is ready for the internal DMA to access
        uint32_t EP14_PRD_RDY : 1; // EP14 PRD is ready for the internal DMA to access
        uint32_t EP15_PRD_RDY : 1; // EP15 PRD is ready for the internal DMA to access
        uint32_t reserved2 : 16;   // reserved
    } bf;
} U_regUSBD_EP_PRD_RDY;
#define regUSBD_EP_PRD_RDY ((U_regUSBD_EP_PRD_RDY *)(USB3_REG_BASE + 0x2504))

// Max FIFO entry num, HW defined
#define MAX_FIFO 12

// Max EPRD for one endpoint
#define EPRD_NUM 15

#ifdef USE_FAKE_ZLP
#define FAKE_ZLP_CHECK 0x11223344
#endif

#define PRD_MAX_BUF_SIZE (4 * 1024 * 1024)

typedef int8_t (*cx_out_callback)(void);

// for SETUP packet request
enum
{
    RESP_NACK = 1, /* busy now */
    RESP_ACK,      /* reqeust is done */
    RESP_STALL,    /* request is not supported */
    RESP_RUNNING,
};

// make BOS descriptor handled internally
typedef struct __attribute__((__packed__))
{
    struct __attribute__((__packed__))
    {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t wTotalLength;
        uint8_t bNumDeviceCaps;
    } bos;

    struct __attribute__((__packed__))
    {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDevCapabilityType;
        uint32_t bmAttributes;
    } usb2_capb;

    struct __attribute__((__packed__))
    {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDevCapabilityType;
        uint8_t bmAttributes;
        uint16_t wSpeedsSupported;
        uint8_t bFunctionalitySupport;
        uint8_t bU1DevExitLat;
        uint16_t wU2DevExitLat;
    } usb3_capb;

} kdrv_usbd3_BOS_descriptor_t;

static kdrv_usbd3_BOS_descriptor_t bos_content =
    {
        .bos.bLength = 5,
        .bos.bDescriptorType = 0xF,
        .bos.wTotalLength = 22,
        .bos.bNumDeviceCaps = 2,

        .usb2_capb.bLength = 7,
        .usb2_capb.bDescriptorType = 0x10,
        .usb2_capb.bDevCapabilityType = 0x2,
        .usb2_capb.bmAttributes = 0x0, // No support of LPM

        .usb3_capb.bLength = 10,
        .usb3_capb.bDescriptorType = 0x10,
        .usb3_capb.bDevCapabilityType = 0x3,
        .usb3_capb.bmAttributes = 0x0,        // No LTM
        .usb3_capb.wSpeedsSupported = 0xC,    // suppor SS and HS
        .usb3_capb.bFunctionalitySupport = 2, // support HS(2) and above
        .usb3_capb.bU1DevExitLat = 0,
        .usb3_capb.wU2DevExitLat = 0,
};
kdrv_usbd3_BOS_descriptor_t *ss_bos_descps = &bos_content;

typedef enum
{
    ENP_NOT_AVAILABLE = 0x0,
    ENP_READY_IDLE,
    ENP_TRANSFERRING,
    ENP_TRANSFER_DONE,
    ENP_TERMINATED,
} enp_status_t;

// data structure for endpoint managemnt, static allocate in advance,
// a little bit waste of memory but sould be OK
typedef struct
{
    uint8_t bEndpointAddress; // same as endpoint descriptor
    uint16_t wMaxPacketSize;  // same as endpoint descriptor
    enp_status_t status;
} endpoint_ctrl_t;

typedef struct
{
    kdrv_usbd3_HS_descriptors_t *hs_descs;
    kdrv_usbd3_SS_descriptors_t *ss_descs;
    kdrv_usbd3_string_descriptor_t *dev_str_desc;
    kdrv_usbd3_speed_t speed;
    bool running_cx_transfer; // is it at cx transferring state
    uint32_t *cx_data;        // next send/receive data buffer address
    uint32_t cx_pkt_size;     // maximum cx packet size
    int32_t cx_txfer_len;     // residual txfer length
    cx_out_callback cx_cb;    // cx out callback
    kdrv_usbd3_link_status_t link_status;
    kdrv_usbd3_link_status_callback_t status_isr_cb; // user event notify callback
    kdrv_usbd3_user_control_callback_t usr_cx_cb;
    osEventFlagsId_t evtFlag; // internal use for blocking wait
    kdrv_usbd3_class_t class_data;
} usbd3_ctrl_t;

static volatile usbd3_ctrl_t usbd_ctrl;
static endpoint_ctrl_t ep_ctrl[MAX_USBD3_ENDPOINT + 1] = {0}; // Due to HW limitation, only few endpoints can be used, not use index 0
static uint8_t *cx_enum_buf = NULL;                           // for GET_DESCRIPTOR use

static void _default_status_isr_callback(kdrv_usbd3_link_status_t link_status)
{
}

static bool _default_usr_cx_isr_callback(kdrv_usbd3_setup_packet_t *setup)
{
    return false; // RESP_STALL
}

static void set_up_endpoint_fifo()
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    int32_t num_enp;

    if (usbd_ctrl.speed == USBD3_SUPER_SPEED)
        num_enp = usbd_ctrl.ss_descs->intf_descp->bNumEndpoints;
    else // should be USBD3_HIGH_SPEED
        num_enp = usbd_ctrl.hs_descs->intf_descp->bNumEndpoints;

    uint32_t next_start_fifo_entry = 0;
    uint32_t next_fifo_addr_offset = 0;
    uint32_t grp_0_int = 0;

    for (int i = 0; i < num_enp; i++)
    {
        kdrv_usbd3_endpoint_descriptor_t *enp;
        uint32_t ep_fifo_entry;

        if (usbd_ctrl.speed == USBD3_SUPER_SPEED)
        {
            enp = usbd_ctrl.ss_descs->enp_descp[i];
            ep_fifo_entry = usbd_ctrl.ss_descs->enp_cmpn_descp[i]->bMaxBurst + 1;
        }
        else // should be USBD3_HIGH_SPEED
        {
            enp = usbd_ctrl.hs_descs->enp_descp[i];
            ep_fifo_entry = 2; // FIXME: can also be 1 or 3
        }

        uint32_t epno = enp->bEndpointAddress & 0xF;          // endpoint number for EPn
        uint32_t ep_type = enp->bmAttributes & 0x3;           // what kind of transfer endpoint
        uint32_t ep_dir = !!(enp->bEndpointAddress & 0x80);   // direction IN or OUT
        uint32_t ep_bw_num = 1;                               // HS ISO, not supported now
        uint32_t ep_ISO_in_pktnum = 0;                        // ISO stuff, not supported now
        uint32_t ep_start_fifo_entry = next_start_fifo_entry; // fifo entry start number

        next_start_fifo_entry += ep_fifo_entry; // for next endpoint

        if (next_start_fifo_entry > MAX_FIFO)
            usbd3_dbg("USBD3 error: FIFO entry is outnumbered!\n");

        uint32_t ep_set1 = 0x0;
        ep_set1 |= (ep_start_fifo_entry << 24) | (ep_fifo_entry << 12) | (ep_ISO_in_pktnum << 6) | (ep_bw_num << 4) | (ep_type << 2) | (ep_dir << 1) | 0x1;

        // wrtie value to EPn_SET1
        regUSBD_EP[epno - 1].dw.EPn_SET1 = ep_set1;

        // debug only
        usbd3_dbg("endpoint 0x%x EPn_SET1 = 0x%x\n", enp->bEndpointAddress, regUSBD_EP[epno - 1].dw.EPn_SET1);

        uint32_t ep_mps = enp->wMaxPacketSize; // endpoint max packet size
        uint32_t ep_fifo_addr_offset = next_fifo_addr_offset;

        ep_ctrl[epno].wMaxPacketSize = enp->wMaxPacketSize;

        next_fifo_addr_offset += (ep_mps >> 3) * ep_fifo_entry; // for next endpoint

        uint32_t ep_set2 = 0x0;
        ep_set2 |= (ep_fifo_addr_offset << 16) | ep_mps;

        // wrtie value to EPn_SET2
        regUSBD_EP[epno - 1].dw.EPn_SET2 = ep_set2;

        // debug only
        usbd3_dbg("endpoint 0x%x EPn_SET2 = 0x%x\n", enp->bEndpointAddress, regUSBD_EP[epno - 1].dw.EPn_SET2);

        // init DMA/PRD stuff
        {
            ep_ctrl[epno].bEndpointAddress = enp->bEndpointAddress;

            // clear things for safety
            regUSBD_EP_PRD[epno - 1].dw.word_0 = 0;
            regUSBD_EP_PRD[epno - 1].bf.PAR = 0x0;      // because user buffer is not ready now
            regUSBD_EP_PRD[epno - 1].bf.EPRD_PTR = 0x0; // use only internal PRD
            // set up last EPRD
            regUSBD_EP_PRD[epno - 1].bf.L_last_prd = 1;
            // EPRD interrupt
            regUSBD_EP_PRD[epno - 1].bf.I_int_en = 1;

            usbd3_dbg("regUSBD_EP_PRD[%d] = 0x%x, 0x%x, 0x%x\n", epno, regUSBD_EP_PRD[epno - 1].dw.word_0, regUSBD_EP_PRD[epno - 1].dw.word_1, regUSBD_EP_PRD[epno - 1].dw.word_2);

            grp_0_int |= (0x1 << (epno + 16));
        }

        ep_ctrl[epno].status = ENP_READY_IDLE;
    }

    regUSBD_IGER0->dw = grp_0_int;
}

static void force_link_speed(kdrv_usbd3_speed_t speed)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    if (speed == USBD3_HIGH_SPEED)
        regUSBD_GTM->bf.TST_FORCE_HS = 1;
    else if (speed == USBD3_SUPER_SPEED)
        regUSBD_GTM->bf.TST_FORCE_SS = 1;
}

static void reply_to_host(int8_t resp)
{
    if (resp == RESP_ACK)
        regUSBD_CX_Config_Status->bf.CX_DONE = 1;
    else
        regUSBD_CX_Config_Status->bf.CX_STL = 1;
}

static void cx_txfer_data_send()
{
    int32_t act_txfer = MIN(usbd_ctrl.cx_txfer_len, usbd_ctrl.cx_pkt_size);
    for (int32_t r = act_txfer; r > 0; r -= 4)
    {
        regUSBD_CX_PORT->dw = *(usbd_ctrl.cx_data);
        usbd_ctrl.cx_data++;
    }
    usbd_ctrl.cx_txfer_len -= act_txfer;
}

static void cx_txfer_data_receive()
{
    int32_t recv_len = regUSBD_CX_Config_Status->bf.CXF_LEN;
    for (int32_t r = recv_len; r > 0; r -= 4)
    {
        *(usbd_ctrl.cx_data) = regUSBD_CX_PORT->dw;
        usbd_ctrl.cx_data++;
    }
    usbd_ctrl.cx_txfer_len += recv_len;

    if (recv_len < usbd_ctrl.cx_pkt_size)
    {
        int8_t resp = RESP_ACK;
        if(usbd_ctrl.cx_cb()!= NULL){
            resp = usbd_ctrl.cx_cb();
        }
        if (resp == RESP_ACK)
        {
            reply_to_host(RESP_ACK);
        }
        else if (resp == RESP_STALL)
        {
            // indicate a request error to host
            reply_to_host(RESP_STALL);
        }
        usbd_ctrl.running_cx_transfer = false;
    }
}

static int8_t send_host_device_descriptor(kdrv_usbd3_setup_packet_t *setup)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);
    
    if(usbd_ctrl.ss_descs != NULL || usbd_ctrl.hs_descs != NULL ){
        kdrv_usbd3_device_descriptor_t *dev_descp;

        if (usbd_ctrl.speed == USBD3_HIGH_SPEED)
            dev_descp = usbd_ctrl.hs_descs->dev_descp;
        else if (usbd_ctrl.speed == USBD3_SUPER_SPEED)
            dev_descp = usbd_ctrl.ss_descs->dev_descp;
        else
            return RESP_STALL; // error

        usbd_ctrl.running_cx_transfer = true;
        usbd_ctrl.cx_txfer_len = MIN(dev_descp->bLength, setup->wLength);
        usbd_ctrl.cx_data = (uint32_t *)dev_descp;
    }
    else{
        uint16_t desc_len = 0;
        uint32_t *desc = usbd_ctrl.class_data.get_device_desc(usbd_ctrl.speed, &desc_len);
        usbd_ctrl.running_cx_transfer = true;
        usbd_ctrl.cx_txfer_len = MIN(desc_len, setup->wLength);
        usbd_ctrl.cx_data = (uint32_t *)desc;
    }


    usbd3_dbg("dev descp: total txfer %d\n", usbd_ctrl.cx_txfer_len);

    regUSBD_CX_Config_Status->bf.CXF_LEN = usbd_ctrl.cx_txfer_len;

    cx_txfer_data_send();

    return RESP_RUNNING;
}

static int8_t _send_host_string_descriptor(kdrv_usbd3_setup_packet_t *setup, uint8_t type)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);
    kdrv_usbd3_string_descriptor_t *desc = usbd_ctrl.dev_str_desc;
    kdrv_usbd3_prd_string_descriptor_t **desc_str = &usbd_ctrl.dev_str_desc->desc[0];
    uint32_t *desc_ptr;
    uint16_t desc_len;
    
    usbd_ctrl.running_cx_transfer = true;
    usbd3_dbg("_send_host_string_descriptor %d\n", type);

    if (type == 0) //language id
    {
        if(usbd_ctrl.ss_descs != NULL){
            usbd_ctrl.cx_txfer_len = MIN(desc->bLength, setup->wLength);
            usbd_ctrl.cx_data = (uint32_t *)desc;
        }
        else{
            desc_ptr = usbd_ctrl.class_data.get_lang_id_str_desc(usbd_ctrl.speed, &desc_len);
            usbd_ctrl.cx_txfer_len = MIN(desc_len, setup->wLength);
            usbd_ctrl.cx_data = (uint32_t *)desc_ptr;
        }
    }
    else if (type == 1 || type == 2 || type == 3) //iManufacturer,iProduct,iSerialNumber
    {
        if(usbd_ctrl.ss_descs != NULL){
            usbd_ctrl.cx_txfer_len = MIN(desc_str[type - 1]->bLength, setup->wLength);
            usbd_ctrl.cx_data = ((uint32_t *)desc_str[type - 1]);
        }
        else{
            if(type == 1){
                desc_ptr = usbd_ctrl.class_data.get_manufacturer_str_desc(usbd_ctrl.speed, &desc_len);
            }
            else if(type == 2){
                desc_ptr = usbd_ctrl.class_data.get_product_str_desc(usbd_ctrl.speed, &desc_len);
            }
            else if(type == 3){
                desc_ptr = usbd_ctrl.class_data.get_serial_str_desc(usbd_ctrl.speed, &desc_len);
            }
            usbd_ctrl.cx_txfer_len = MIN(desc_len, setup->wLength);
            usbd_ctrl.cx_data = desc_ptr;
        }
    }
    usbd3_dbg("dev descp: total txfer %d\n", usbd_ctrl.cx_txfer_len);

    regUSBD_CX_Config_Status->bf.CXF_LEN = usbd_ctrl.cx_txfer_len;

    cx_txfer_data_send();

    return RESP_RUNNING;
}

static int8_t send_host_configuration_descriptors(kdrv_usbd3_setup_packet_t *setup)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);
    if(usbd_ctrl.ss_descs != NULL || usbd_ctrl.hs_descs != NULL){
        kdrv_usbd3_config_descriptor_t *config_descp = NULL;
        kdrv_usbd3_interface_descriptor_t *intf_descp;

        if (usbd_ctrl.speed == USBD3_HIGH_SPEED)
        {
            uint8_t *buf = cx_enum_buf;

            config_descp = usbd_ctrl.hs_descs->config_descp;
            intf_descp = usbd_ctrl.hs_descs->intf_descp;

            memcpy(buf, config_descp, config_descp->bLength);
            buf += config_descp->bLength;

            memcpy(buf, intf_descp, intf_descp->bLength);
            buf += intf_descp->bLength;

            for (int i = 0; i < intf_descp->bNumEndpoints; i++)
            {
                kdrv_usbd3_endpoint_descriptor_t *enp_descp = usbd_ctrl.hs_descs->enp_descp[i];
                memcpy(buf, enp_descp, enp_descp->bLength);
                buf += enp_descp->bLength;
            }
        }
        else if (usbd_ctrl.speed == USBD3_SUPER_SPEED)
        {
            uint8_t *buf = cx_enum_buf;

            config_descp = usbd_ctrl.ss_descs->config_descp;
            intf_descp = usbd_ctrl.ss_descs->intf_descp;

            memcpy(buf, config_descp, config_descp->bLength);
            buf += config_descp->bLength;

            memcpy(buf, intf_descp, intf_descp->bLength);
            buf += intf_descp->bLength;

            for (int i = 0; i < intf_descp->bNumEndpoints; i++)
            {
                kdrv_usbd3_endpoint_descriptor_t *enp_descp = usbd_ctrl.ss_descs->enp_descp[i];
                memcpy(buf, enp_descp, enp_descp->bLength);
                buf += enp_descp->bLength;

                kdrv_usbd3_endpoint_companion_descriptor_t *enp_cmpn_descp = usbd_ctrl.ss_descs->enp_cmpn_descp[i];
                memcpy(buf, enp_cmpn_descp, enp_cmpn_descp->bLength);
                buf += enp_cmpn_descp->bLength;
            }
        }
        else
        {
            return RESP_STALL; //error
        }

        usbd_ctrl.running_cx_transfer = true;
        usbd_ctrl.cx_txfer_len = MIN(config_descp->wTotalLength, setup->wLength);
        usbd_ctrl.cx_data = (uint32_t *)cx_enum_buf;
    }
    else{
        uint16_t desc_len = 0;
        uint32_t *desc = usbd_ctrl.class_data.get_configuration_desc(usbd_ctrl.speed, &desc_len);
        usbd_ctrl.running_cx_transfer = true;
        usbd_ctrl.cx_txfer_len = MIN(desc_len, setup->wLength);
        usbd_ctrl.cx_data = desc;
    }


    usbd3_dbg("config descp: total txfer %d\n", usbd_ctrl.cx_txfer_len);

    regUSBD_CX_Config_Status->bf.CXF_LEN = usbd_ctrl.cx_txfer_len;

    cx_txfer_data_send();

    return RESP_RUNNING;
}

static int8_t send_bos_descriptor(kdrv_usbd3_setup_packet_t *setup)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    usbd_ctrl.running_cx_transfer = true;
    usbd_ctrl.cx_txfer_len = MIN(ss_bos_descps->bos.wTotalLength, setup->wLength);
    usbd_ctrl.cx_data = (uint32_t *)ss_bos_descps;

    usbd3_dbg("bos descp: total txfer %d\n", usbd_ctrl.cx_txfer_len);

    regUSBD_CX_Config_Status->bf.CXF_LEN = usbd_ctrl.cx_txfer_len;

    cx_txfer_data_send();

    return RESP_RUNNING;
}

static int8_t set_configuration(kdrv_usbd3_setup_packet_t *setup)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    uint8_t config_val = setup->wValue & 0xFF;

    if (config_val > 1)
        return RESP_STALL;

    if (config_val == 1 && usbd_ctrl.link_status == USBD3_STATUS_DISCONNECTED) // because we support only 1 configuration
    {
        if(usbd_ctrl.ss_descs != NULL || usbd_ctrl.hs_descs != NULL){
            set_up_endpoint_fifo();
        }
        else{
            if(usbd_ctrl.class_data.init != NULL){
                usbd_ctrl.class_data.init();
            }
        }
        
        regUSBD_DAR->bf.SET_CONFIG = 1;

        usbd_ctrl.link_status = USBD3_STATUS_CONFIGURED;
        if(usbd_ctrl.status_isr_cb != NULL)
            usbd_ctrl.status_isr_cb(USBD3_STATUS_CONFIGURED);
        else
            usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_CONFIGURED);
    }
    else if (config_val == 0 && usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED)
    {
        // ToDo: clear or reset something
        regUSBD_DAR->bf.SET_CONFIG = 0;
    }

    return RESP_ACK;
}

static int8_t set_sel(void)
{
    usbd3_dbg(">> %s() recv %d bytes\n", __FUNCTION__, usbd_ctrl.cx_txfer_len);

    // FIXME: do something !

    return RESP_ACK;
}

static int8_t handle_standard_request(kdrv_usbd3_setup_packet_t *setup)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    int8_t resp = RESP_STALL;

    // handle requests which are not affected by ep0 halt
    switch (setup->bRequest)
    {
    case 0x0: // GET_STATUS
    {
        usbd3_dbg("GET_STATUS OK\n");
        usbd_ctrl.running_cx_transfer = true;
        usbd_ctrl.cx_txfer_len = setup->wLength; // should be 2 bytes at HS
        *(usbd_ctrl.cx_data) = 0;                // return GET_STATUS as all zeros
        regUSBD_CX_Config_Status->bf.CXF_LEN = usbd_ctrl.cx_txfer_len;
        cx_txfer_data_send();
        resp = RESP_RUNNING;
        break;
    }
    case 0x1: // CLEAR_FEATURE
    {
        if(setup->bmRequestType == KDRV_USB_REQ_RECIPIENT_ENDPOINT){
            uint32_t epno = setup->wIndex & 0xF;
            uint8_t val = 4;
            if(regUSBD_EP[epno - 1].bf.EPn_STL){
                val += 8;
            }
            regUSBD_EP[epno - 1].dw.EPn_SET0 |= val;
        }
        if(usbd_ctrl.class_data.feature_ctl_setup != NULL){
            resp = usbd_ctrl.class_data.feature_ctl_setup(setup);
        }
        break;
    }
    case 0x3: // SET_FEATURE
    {
        if(usbd_ctrl.class_data.feature_ctl_setup != NULL){
            resp = usbd_ctrl.class_data.feature_ctl_setup(setup);
        }
        break;
    }
    case 0x5: // SET_ADDRESS
    {
        usbd3_dbg("SET_ADDRESS : 0x%x\n", setup->wValue);
        // set device address to HW
        regUSBD_DAR->bf.DEV_ADDR = setup->wValue;
        resp = RESP_ACK;
        break;
    }
    case 0x6: // GET_DESCRIPTOR
    {
        // low byte: index of specified descriptor type
        uint8_t descp_idx = (setup->wValue & 0xFF);

        // high byte: descriptor type
        switch (setup->wValue >> 8)
        {
        case 1: // DEVICE descriptor
            usbd3_dbg("GET_DESCRIPTOR : Device\n");
            resp = send_host_device_descriptor(setup);
            break;
        case 2: // CONFIGURATION descriptor
            usbd3_dbg("GET_DESCRIPTOR : Config\n");
            resp = send_host_configuration_descriptors(setup);
            break;
        case 3: // STRING descriptor
            resp = _send_host_string_descriptor(setup, descp_idx);
            break;
        case 4: // INTERFACE descriptor
            usbd3_dbg("GET_DESCRIPTOR : INTERFACE not support\n");
            break;
        case 5: // ENDPOINT descriptor
            usbd3_dbg("GET_DESCRIPTOR : ENDPOINT not support\n");
            break;
        case 6: // DEVICE_QUALIFIER descriptor
            usbd3_dbg("GET_DESCRIPTOR : DEVICE_QUALIFIER not support\n");
            //resp = send_host_device_qual_descriptor(setup);
            break;
        case 7: // OTHER_SPEED_CONFIGURATION descriptor
            usbd3_dbg("GET_DESCRIPTOR : OTHER_SPEED_CONFIGURATION not support\n");
            break;
        case 8: // INTERFACE_POWER descriptor
            usbd3_dbg("GET_DESCRIPTOR : INTERFACE_POWER not support\n");
        case 0xF: // BOS descriptor
            usbd3_dbg("GET_DESCRIPTOR : BOS\n");
            resp = send_bos_descriptor(setup);
            break;
        default:
            usbd3_dbg("GET_DESCRIPTOR : UNKNOWN not support\n");
            break;
        }
        break;
    }
    case 0x7: // SET_DESCRIPTOR
    {
        usbd3_dbg("SET_ADDRESS not support\n");
        break;
    }
    case 0x8: // GET_CONFIGURATION
    {
        usbd3_dbg("GET_CONFIGURATION not support\n");
        break;
    }
    case 0x9: // SET_CONFIGURATION
    {
        usbd3_dbg("SET_CONFIGURATION OK\n");
        resp = set_configuration(setup);
        break;
    }
    case 0x31: // SET_ISOCH_DELAY
    {
        usbd3_dbg("SET_ISOCH_DELAY OK\n");
        resp = RESP_ACK;
        // actually do nothing here
        break;
    }
    case 0x30: // SET_SEL
    {
        usbd3_dbg("SET_ISOCH_DELAY waiting for data in\n");
        if (setup->wLength > 0)
        {
            // then waiting for CX_OUT_INT
            usbd_ctrl.running_cx_transfer = true;
            usbd_ctrl.cx_txfer_len = 0;
            usbd_ctrl.cx_data = (uint32_t *)cx_enum_buf;
            usbd_ctrl.cx_cb = &set_sel;

            resp = RESP_RUNNING;
        }
        else
        {
            resp = RESP_STALL;
        }

        // actually do nothing here
        break;
    }
    default: // UNKNOWN
    {
        usbd3_dbg("????? Unknown standard request not support\n");
        break;
    }
    }

    return resp;
}

static void handle_control_transfer()
{
    // for 8 bytes setup packet
    kdrv_usbd3_setup_packet_t setup = {0};
    uint8_t *temp = (uint8_t *)&setup;

    // check if CXF_FUL and CXF_LEN == 8, FIXME: code size
    if (regUSBD_CX_Config_Status->bf.CXF_FUL != 1 || regUSBD_CX_Config_Status->bf.CXF_LEN != 8)
    {
        usbd3_dbg("- regUSBD_CX_Config_Status is not correct = 0x%x\n", regUSBD_CX_Config_Status->dw);
    }

    // directly read DMA_CPS3 twice to get 8-byte setup packet
    *((uint32_t *)temp) = regUSBD_CX_PORT->dw;
    *((uint32_t *)(temp + 4)) = regUSBD_CX_PORT->dw;

    usbd3_dbg("- bmRequestType 0x%x bRequest 0x%x wValue 0x%x wIndex 0x%x wLength 0x%x\n", setup.bmRequestType, setup.bRequest, setup.wValue, setup.wIndex, setup.wLength);

    int8_t resp = RESP_NACK;
    uint8_t bmRequestType_type = ((setup.bmRequestType & 0x60) >> 5);
    switch (bmRequestType_type)
    {
    case 0: // Standard request
        resp = handle_standard_request(&setup);
        break;
    case 1: // Class request
        if(usbd_ctrl.class_data.class_ctl_setup != NULL){
            resp = usbd_ctrl.class_data.class_ctl_setup(&setup);
        }
        else{
            resp = RESP_STALL;
        }
        break;
    case 2: // Vendor request
        resp = usbd_ctrl.usr_cx_cb(&setup) ? RESP_ACK : RESP_STALL;
        break;
    }

    if (resp == RESP_ACK )
    {
        // indicate an OK request to host
        reply_to_host(RESP_ACK);
    }
    else if (resp == RESP_STALL)
    {
        // indicate a request error to host
        reply_to_host(RESP_STALL);
    }
    else
    {
        // do nothing for now
    }
}

// USBD ISR
static void usbd3_isr()
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    // check device interrupt status
    uint32_t dev_int = regUSBD_DEVINTRSTS->dw;
    uint32_t irg1_temp = regUSBD_IGR1->dw & regUSBD_IGER1->dw;
    if (dev_int)
    {
        if (dev_int & 0x2) // DEVINTR1
        {
            // check enabled IGR1 interrupts
            
            U_regUSBD_IGR1 igr1_en_int;
            // do this memcpy is due to strict casting
            memcpy((void *)&igr1_en_int, &irg1_temp, 4);

            usbd3_dbg("- igr1_en_int (0x%x) = 0x%x\n", regUSBD_IGR1, igr1_en_int);

            // Group 5 interrupts
            if (igr1_en_int.bf.INT_GRP5)
            {
                usbd3_dbg("- Group 5 interrupt !\n");
                // should check group 5 interrupt register (0x2434)
            }

            // VBUS change
            if (igr1_en_int.bf.VBUS_CHG_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.VBUS_CHG_INT = 1;

                // read VBUS status
                if (regUSBD_GCR->bf.VBUS_STATUS == 1)
                {
                    usbd3_dbg("- VBUS is detected from the host\n");
                    // Attached state ??
                }
                else
                {
                    usbd3_dbg("- VBUS is off from the host\n");
                    // ToDo: should do hw reset according to datasheet !

#ifndef _BOARD_SN720HAPS_H_
                    for (int i = 1; i < (MAX_USBD3_ENDPOINT + 1); i++)
                        kdrv_usbd3_reset_endpoint(i);
#endif
                    usbd_ctrl.link_status = USBD3_STATUS_DISCONNECTED;
                    if(usbd_ctrl.status_isr_cb != NULL)
                        usbd_ctrl.status_isr_cb(USBD3_STATUS_DISCONNECTED);
                    else
                        usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_DISCONNECTED);
                }
            }

            // SYNC FIFO0 empty
            if (igr1_en_int.bf.SYNF0_EMPTY_INT)
            {
                usbd3_dbg("- SYNC FIFO0 empty !\n");
                // dont enable this interupt
            }

            // U3 Exit Fail
            if (igr1_en_int.bf.U3_EXIT_FAIL_INT)
            {
                regUSBD_IGR1->bf.U3_EXIT_FAIL_INT = 1;
                usbd3_dbg("- U3 Exit Fail !\n");
            }

            // U2 Exit Fail
            if (igr1_en_int.bf.U2_EXIT_FAIL_INT)
            {
                regUSBD_IGR1->bf.U2_EXIT_FAIL_INT = 1;
                usbd3_dbg("- U2 Exit Fail !\n");
            }

            // U1 Exit Fail
            if (igr1_en_int.bf.U1_EXIT_FAIL_INT)
            {
                regUSBD_IGR1->bf.U1_EXIT_FAIL_INT = 1;
                usbd3_dbg("- U1 Exit Fail !\n");
            }

            // U2 Entry Fail
            if (igr1_en_int.bf.U2_ENTRY_FAIL_INT)
            {
                regUSBD_IGR1->bf.U2_ENTRY_FAIL_INT = 1;
                usbd3_dbg("- U2 Entry Fail !\n");
            }

            // U1 Entry Fail
            if (igr1_en_int.bf.U1_ENTRY_FAIL_INT)
            {
                regUSBD_IGR1->bf.U1_ENTRY_FAIL_INT = 1;
                usbd3_dbg("- U1 Entry Fail !\n");
            }

            // U3 Exit
            if (igr1_en_int.bf.U3_EXIT_INT)
            {
                regUSBD_IGR1->bf.U3_EXIT_INT = 1;
                usbd3_dbg("- U3 Exit !\n");
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_U3_EXIT);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_U3_EXIT);
            }

            // U2 Exit
            if (igr1_en_int.bf.U2_EXIT_INT)
            {
                regUSBD_IGR1->bf.U2_EXIT_INT = 1;
                usbd3_dbg("- U2 Exit !\n");
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_U2_EXIT);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_U2_EXIT);
            }

            // U1 Exit
            if (igr1_en_int.bf.U1_EXIT_INT)
            {
                regUSBD_IGR1->bf.U1_EXIT_INT = 1;
                usbd3_dbg("- U1 Exit !\n");
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_U1_EXIT);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_U1_EXIT);
            }

            // U3 Entry
            if (igr1_en_int.bf.U3_ENTRY_INT)
            {
                regUSBD_IGR1->bf.U3_ENTRY_INT = 1;
                usbd3_dbg("- U3 Entry !\n");
                regUSBD_SS_CR1->bf.GO_U3_DONE = 1;
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_U3_ENTRY);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_U3_ENTRY);
            }

            // U2 Entry
            if (igr1_en_int.bf.U2_ENTRY_INT)
            {
                regUSBD_IGR1->bf.U2_ENTRY_INT = 1;
                usbd3_dbg("- U2 Entry !\n");
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_U2_ENTRY);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_U2_ENTRY);
            }

            // U1 Entry
            if (igr1_en_int.bf.U1_ENTRY_INT)
            {
                regUSBD_IGR1->bf.U1_ENTRY_INT = 1;
                usbd3_dbg("- U1 Entry !\n");
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_U1_ENTRY);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_U1_ENTRY);
            }

            // SS hot reset
            if (igr1_en_int.bf.HOT_RST_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.HOT_RST_INT = 1;
                usbd3_dbg("- SS hot reset\n");
                // FIXME: do something ??
                kdrv_usbd3_link_status_t pre_status = usbd_ctrl.link_status;
                usbd_ctrl.link_status = USBD3_STATUS_DISCONNECTED;
                if(pre_status == USBD3_STATUS_CONFIGURED){
                    if(usbd_ctrl.status_isr_cb != NULL)
                        usbd_ctrl.status_isr_cb(USBD3_STATUS_DISCONNECTED);
                    else
                        usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_DISCONNECTED);
                }
                else{
                    if(usbd_ctrl.status_isr_cb != NULL)
                        usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_HOT_RESET);
                    else
                        usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_HOT_RESET);
                }
            }

            // SS warm reset
            if (igr1_en_int.bf.WARM_RST_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.WARM_RST_INT = 1;
                usbd3_dbg("- SS warm reset\n");
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_SS_WARM_RESET);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SS_WARM_RESET);
            }

            // HS resume
            if (igr1_en_int.bf.HS_RESM_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.HS_RESM_INT = 1;
                usbd3_dbg("- HS resume\n");
                if(usbd_ctrl.status_isr_cb != NULL)
                    usbd_ctrl.status_isr_cb(USBD3_STATUS_RESUME);
                else
                    usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_RESUME);
            }

            // HS suspend
            if (igr1_en_int.bf.HS_SUSP_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.HS_SUSP_INT = 1;
                usbd3_dbg("- HS suspend\n");
                regUSBD_HS_CR->bf.HS_GOSUSP = 1;
                
                // we need a bottom-half task
                if(usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED){
                    if(usbd_ctrl.status_isr_cb != NULL)
                        usbd_ctrl.status_isr_cb(USBD3_STATUS_SUSPEND);
                    else
                        usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_SUSPEND);
                }
            }

            // HS LPM
            if (igr1_en_int.bf.HS_LPM_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.HS_LPM_INT = 1;
                usbd3_dbg("- HS LPM\n");
            }

            // HS bus reset
            if (igr1_en_int.bf.HS_RST_INT)
            {
                static uint32_t cnt = 0;
                cnt++;
                // clear interrupt
                regUSBD_IGR1->bf.HS_RST_INT = 1;
                usbd3_dbg("- HS bus reset\n");
                // FIXME: do something ??
                kdrv_usbd3_link_status_t pre_status = usbd_ctrl.link_status;
                usbd_ctrl.link_status = USBD3_STATUS_DISCONNECTED;
                if(pre_status == USBD3_STATUS_CONFIGURED){
                    if(usbd_ctrl.status_isr_cb != NULL)
                        usbd_ctrl.status_isr_cb(USBD3_STATUS_DISCONNECTED);
                    else
                        usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_DISCONNECTED);
                }
                else{
                    if(usbd_ctrl.status_isr_cb != NULL)
                        usbd_ctrl.status_isr_cb(USBD3_STATUS_RESET);
                    else
                        usbd_ctrl.class_data.status_isr_cb(USBD3_STATUS_RESET);
                }
            }

            // Device mode change
            if (igr1_en_int.bf.DEV_MODE_CHG_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.DEV_MODE_CHG_INT = 1;

                // read Link speed status
                switch (regUSBD_GCR->bf.DEV_MODE)
                {
                case 0:
                    usbd_ctrl.speed = USBD3_NO_LINK_SPEED;
                    usbd3_dbg("- Link speed : No link\n");
                    break;

                case 1:
                    usbd_ctrl.speed = USBD3_SUPER_SPEED;
                    usbd_ctrl.cx_pkt_size = 512;
                    usbd3_dbg("- Link speed : Super-Speed\n");
                    break;

                case 2:
                    usbd_ctrl.speed = USBD3_HIGH_SPEED;
                    usbd_ctrl.cx_pkt_size = 64;
                    usbd3_dbg("- Link speed : High-Speed\n");
                    break;

                case 3:
                    usbd_ctrl.speed = USBD3_NO_LINK_SPEED; // not support !!
                    usbd3_dbg("- Link speed : Full-Speed\n");
                    break;
                }
            }

            // Command Abort Interrupt
            if (igr1_en_int.bf.CX_COMABT_INT)
            {
                // clear interrupt
                regUSBD_IGR1->bf.CX_COMABT_INT = 1;
                usbd_ctrl.running_cx_transfer = false;
                usbd3_dbg("- CX abort\n");
            }

            // Host Emitting Extra IN or OUT Data Interrupt
            if (igr1_en_int.bf.CX_CMDFAIL_INT)
            {
                regUSBD_CX_Config_Status->bf.CX_STL = 1;
                usbd_ctrl.running_cx_transfer = false;
                usbd3_dbg("- CX fail\n");
            }

            // Host End-of-command (Entering status stage) Interrupt
            if (igr1_en_int.bf.CX_CMDEND_INT)
            {
                if (usbd_ctrl.running_cx_transfer)
                {
                    reply_to_host(RESP_ACK);
                    usbd_ctrl.running_cx_transfer = false;
                }
            }

            // EP0 Interrupt for OUT
            if (igr1_en_int.bf.CX_OUT_INT)
            {
                // continue to transfer
                if (usbd_ctrl.running_cx_transfer)
                    cx_txfer_data_receive();
            }

            // EP0 Interrupt f for IN
            if (igr1_en_int.bf.CX_IN_INT)
            {
                // continue to transfer
                if (usbd_ctrl.running_cx_transfer)
                    cx_txfer_data_send();
            }

            // EP0 Setup Data Received Interrupt
            if (igr1_en_int.bf.CX_SETUP_INT)
            {
                if (!usbd_ctrl.running_cx_transfer)
                    handle_control_transfer();
            }

            // Group 4, ZLP interrupt, FIXME!!!!!!!!!!!!!!!
            if (igr1_en_int.bf.INT_GRP4)
            {
                uint32_t zlp_int = regUSBD_IGR4->dw >> 17;
                for (int epno = 1; (epno < 16 && zlp_int > 0); zlp_int >>= 1)
                {
                    if (zlp_int & 0x1)
                    {
                        usbd3_dbg("ep %d ZLP interrupt\n", epno);

                        // clear corresponding prd interrupt
                        regUSBD_IGR4->dw = 0x1 << (epno + 16);

                        usbd3_dbg("0x2520 = 0x%x\n", regUSBD_EP_PRD[epno - 1].dw.word_0);

                        if (ep_ctrl[epno].status == ENP_TRANSFERRING)
                        {
                            //ep_ctrl[epno].status = ENP_RECEIVED_ZLP;
                            osEventFlagsSet(usbd_ctrl.evtFlag, (0x1 << epno));
                        }
                    }
                }
            }

            // Group 3 interrupts
            if (igr1_en_int.bf.INT_GRP3)
            {
                usbd3_dbg("- Group 3 interrupt !\n");
                // should check group 3 interrupt register (0x240C)
            }

            // Group 2 interrupts
            if (igr1_en_int.bf.INT_GRP2)
            {
                usbd3_dbg("- Group 2 interrupt !\n");
                // should check group 2 interrupt register (0x2408)
            }

            // debug only, check what interrupt left not handled
            uint32_t igr1_unhandled_int = regUSBD_IGR1->dw & regUSBD_IGER1->dw;
            if (igr1_unhandled_int)
            {
                usbd3_dbg("- igr1_unhandled_int (0x%x) = 0x%x\n", regUSBD_IGR1, igr1_unhandled_int);
            }
        }

        if (dev_int & 0x1) // DEVINTR0, FIXME: maybe 'else' is enough
        {
            uint32_t igr0_en_int = regUSBD_IGR0->dw & regUSBD_IGER0->dw;
            usbd3_dbg("- igr0_en_int (0x%x) = 0x%x\n", regUSBD_IGR0, igr0_en_int);

            // check PRD interrupts
            uint32_t prd_ints = igr0_en_int >> 17; // for EP1_PRD_INT ~ EP15_PRD_INT
            for (int epno = 1; (epno < 16 && prd_ints > 0); prd_ints >>= 1)
            {
                if (prd_ints & 0x1)
                {
                    // short packet or buffer is full

                    usbd3_dbg("ep %d prd int!\n", epno);

                    // clear corresponding prd interrupt
                    regUSBD_IGR0->dw = 0x1 << (epno + 16);

                    if (ep_ctrl[epno].status == ENP_TRANSFERRING)
                    {
                        ep_ctrl[epno].status = ENP_TRANSFER_DONE;
                        osEventFlagsSet(usbd_ctrl.evtFlag, (0x1 << epno));
                    }
                }
                epno++;
            }

            // check FIFO interrupts (only for IN)
            // check PRD interrupts
            uint32_t fifo_ints = (igr0_en_int & 0xFF) >> 1; // for EP1_FIFO_INT ~ EP15_FIFO_INT
            for (int epno = 1; (epno < 16 && fifo_ints > 0); fifo_ints >>= 1)
            {
                if (fifo_ints & 0x1)
                {
                    usbd3_dbg("ep %d fifo int!\n", epno);

                    // disable this FIFO interrupt because this is one-shot interrupt
                    uint32_t grp_0_int = regUSBD_IGER0->dw;
                    grp_0_int &= ~(0x1 << epno);
                    regUSBD_IGER0->dw = grp_0_int;

                    if (ep_ctrl[epno].status == ENP_TRANSFERRING)
                    {
                        ep_ctrl[epno].status = ENP_TRANSFER_DONE;
                        osEventFlagsSet(usbd_ctrl.evtFlag, (0x1 << epno));
                    }
                }
                epno++;
            }
        }
    }

    usbd3_dbg("\n");
}

#ifndef _BOARD_SN720HAPS_H_
static void usb3_hardware_init()
{
    volatile uint32_t *reg_usb3_phy_control = (volatile uint32_t *)SCU_EXTREG_USB3_PHY_CTRL_REG;

    // assert reset for USB3 phy PONRST
    regSCU->bf.gpo_out &= ~BIT2;

    // assert reset for USB3 controller and phy
    (*reg_usb3_phy_control) &= ~(BIT25 | BIT26);

    // de-assert PONRST
    regSCU->bf.gpo_out |= BIT2;

    // de-assert USB3 phy
    (*reg_usb3_phy_control) |= BIT25;

    osDelay(5); // FIMXE, just in case

    // apply suggested values on USB3 phy (for better stability)
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x0)) = 0x48000786;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x4)) = 0x00020821;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x8)) = 0xA0C56000;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x14)) = 0x46000000;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x20)) = 0x00001224;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x50)) = 0x2F054273;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x54)) = 0x67F04900;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x58)) = 0x081090F0;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x60)) = 0x25432840;
    *((volatile uint32_t *)(USB3_PHY_REG_BASE + 0x64)) = 0x14CC3A00;

    // de-assert USB3 controller
    (*reg_usb3_phy_control) |= BIT26;

    osDelay(5); // FIMXE, just in case
}
#endif

kdrv_status_t kdrv_usbd3_init()
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

#ifndef _BOARD_SN720HAPS_H_
    usb3_hardware_init();
#endif

    // memory for ep0 standard request descriptors
    // FIXME: 256 ?
    cx_enum_buf = (uint8_t *)malloc(256);
    
    usbd_ctrl.evtFlag = osEventFlagsNew(NULL);
    
    // Global Interrupt Enable Register, host, device, otg
    regUSBD_GLBINTREN->dw = 0;
    regUSBD_GLBINTREN->bf.DEVINTR_EN = 1;
    usbd3_dbg("- regUSBD_GLBINTREN (0x%x) = 0x%x\n", regUSBD_GLBINTREN, regUSBD_GLBINTREN->dw);

    // (Device) Interrupt Group 1 Enable Register
    regUSBD_IGER1->dw = ENABLE_MASK_IGER_1;
    usbd3_dbg("- regUSBD_IGER1 (0x%x) = 0x%x\n", regUSBD_IGER1, regUSBD_IGER1->dw);

    // (Device) Interrupt Group 0 Enable Register
    // by default at starting disable all endpoint interrupts
    regUSBD_IGER0->dw = 0x0;
    usbd3_dbg("- regUSBD_IGER0 (0x%x) = 0x%x\n", regUSBD_IGER0, regUSBD_IGER0->dw);

    // for ZLP interrupts
    regUSBD_IGER4->dw = 0xFFFE0000;
    usbd3_dbg("- regUSBD_IGER4 reg (0x%x) = 0x%x\n", regUSBD_IGER4, regUSBD_IGER4->dw);

    regUSBD_SS_CR2->bf.U1_TIMEOUT = 0x0;
    regUSBD_SS_CR2->bf.U2_TIMEOUT = 0x0;

    NVIC_SetVector(USB3_IRQn, (uint32_t)usbd3_isr);
    NVIC_ClearPendingIRQ(USB3_IRQn);
    NVIC_EnableIRQ(USB3_IRQn);

    usbd_ctrl.hs_descs = NULL;
    usbd_ctrl.ss_descs = NULL;
    usbd_ctrl.dev_str_desc = NULL;
    usbd_ctrl.speed = USBD3_NO_LINK_SPEED;
    usbd_ctrl.link_status = USBD3_STATUS_DISCONNECTED;
    usbd_ctrl.running_cx_transfer = false;
    usbd_ctrl.status_isr_cb = NULL;
    //usbd_ctrl.usr_cx_cb = (usr_cx_isr_cb == NULL) ? _default_usr_cx_isr_callback : usr_cx_isr_cb;

    //if (hs_descs == NULL)
    //    force_link_speed(USBD3_SUPER_SPEED);
    //else if (ss_descs == NULL)
    //    force_link_speed(USBD3_HIGH_SPEED);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd3_initialize(
    kdrv_usbd3_HS_descriptors_t *hs_descs,
    kdrv_usbd3_SS_descriptors_t *ss_descs,
    kdrv_usbd3_string_descriptor_t *dev_str_desc,
    kdrv_usbd3_link_status_callback_t status_isr_cb,
    kdrv_usbd3_user_control_callback_t usr_cx_isr_cb)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

#ifndef _BOARD_SN720HAPS_H_
    usb3_hardware_init();
#endif

    // memory for ep0 standard request descriptors
    // FIXME: 256 ?
    cx_enum_buf = (uint8_t *)malloc(256);

    usbd_ctrl.evtFlag = osEventFlagsNew(NULL);

#ifdef USBD3_DBG
    if (!cx_enum_buf)
        usbd3_dbg("- cx_enum_buf malloc failed\n");
    else
        usbd3_dbg("- cx_enum_buf addr 0x%x\n", cx_enum_buf);
#endif

    // Global Interrupt Enable Register, host, device, otg
    regUSBD_GLBINTREN->dw = 0;
    regUSBD_GLBINTREN->bf.DEVINTR_EN = 1;
    usbd3_dbg("- regUSBD_GLBINTREN (0x%x) = 0x%x\n", regUSBD_GLBINTREN, regUSBD_GLBINTREN->dw);

    // (Device) Interrupt Group 1 Enable Register
    regUSBD_IGER1->dw = ENABLE_MASK_IGER_1;
    usbd3_dbg("- regUSBD_IGER1 (0x%x) = 0x%x\n", regUSBD_IGER1, regUSBD_IGER1->dw);

    // (Device) Interrupt Group 0 Enable Register
    // by default at starting disable all endpoint interrupts
    regUSBD_IGER0->dw = 0x0;
    usbd3_dbg("- regUSBD_IGER0 (0x%x) = 0x%x\n", regUSBD_IGER0, regUSBD_IGER0->dw);

    // for ZLP interrupts
    regUSBD_IGER4->dw = 0xFFFE0000;
    usbd3_dbg("- regUSBD_IGER4 reg (0x%x) = 0x%x\n", regUSBD_IGER4, regUSBD_IGER4->dw);

    // (Device) HS Control Register ??? FIXME
#if 0
    regUSBD_HS_CR->bf.HS_LPM_PERMIT = 1;
    usbd3_dbg("- regUSBD_HS_CR (0x%x) = 0x%x\n", regUSBD_HS_CR, regUSBD_HS_CR->dw);
#endif

    regUSBD_SS_CR2->bf.U1_TIMEOUT = 0x0;
    regUSBD_SS_CR2->bf.U2_TIMEOUT = 0x0;

    NVIC_SetVector(USB3_IRQn, (uint32_t)usbd3_isr);
    NVIC_ClearPendingIRQ(USB3_IRQn);
    NVIC_EnableIRQ(USB3_IRQn);

    usbd_ctrl.hs_descs = hs_descs;
    usbd_ctrl.ss_descs = ss_descs;
    usbd_ctrl.dev_str_desc = dev_str_desc;
    usbd_ctrl.speed = USBD3_NO_LINK_SPEED;
    usbd_ctrl.link_status = USBD3_STATUS_DISCONNECTED;
    usbd_ctrl.running_cx_transfer = false;
    usbd_ctrl.status_isr_cb = (status_isr_cb == NULL) ? _default_status_isr_callback : status_isr_cb;
    usbd_ctrl.usr_cx_cb = (usr_cx_isr_cb == NULL) ? _default_usr_cx_isr_callback : usr_cx_isr_cb;

    if (hs_descs == NULL)
        force_link_speed(USBD3_SUPER_SPEED);
    else if (ss_descs == NULL)
        force_link_speed(USBD3_HIGH_SPEED);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd3_uninitialize()
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    free(cx_enum_buf);
    osEventFlagsDelete(usbd_ctrl.evtFlag);

    NVIC_DisableIRQ(USB3_IRQn);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd3_set_enable(bool enable)
{
    usbd3_dbg(">> %s() enable: %d\n", __FUNCTION__, enable);

    // set VBUS debounce time
    if (enable)
        regUSBD_MISC_CR->bf.VBUS_DEBOUNCE_TIME = 0x200;
    else
        regUSBD_MISC_CR->bf.VBUS_DEBOUNCE_TIME = 0x3FF;

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd3_reset_device(void)
{
    kdrv_usbd3_set_enable(false);
    kdrv_usbd3_uninitialize();
    kdrv_usbd3_initialize(usbd_ctrl.hs_descs, usbd_ctrl.ss_descs, usbd_ctrl.dev_str_desc, usbd_ctrl.status_isr_cb, usbd_ctrl.usr_cx_cb);
    kdrv_usbd3_set_enable(true);
    return KDRV_STATUS_OK;
}

static void _halt_dma_work(uint8_t epno)
{
    // stop DMA and clear FIFO
    regUSBD_EP_PRD[epno - 1].bf.H_hw_own = 0;
    regUSBD_EP[epno - 1].bf.EPn_FF_RST = 1;
   // regUSBD_EP[epno - 1].bf.EPn_CLR_SEQNUM = 1;
}

kdrv_status_t kdrv_usbd3_reset_endpoint(uint8_t endpoint)
{
    uint8_t epno = endpoint & 0xF;

    _halt_dma_work(epno);

    if (ep_ctrl[epno].status == ENP_TRANSFERRING)
    {
        ep_ctrl[epno].status = ENP_TERMINATED;
        osEventFlagsSet(usbd_ctrl.evtFlag, (0x1 << epno));
    }
    else
    {
        ep_ctrl[epno].status = (usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED) ? ENP_READY_IDLE : ENP_NOT_AVAILABLE;
    }

    return KDRV_STATUS_OK;
}


kdrv_status_t kdrv_usbd3_reset_endpoint_seq_num(uint8_t endpoint)
{
    uint8_t epno = endpoint & 0xF;
    regUSBD_EP[epno - 1].bf.EPn_CLR_SEQNUM = 1;
    return KDRV_STATUS_OK;
}

kdrv_usbd3_speed_t kdrv_usbd3_get_link_speed()
{
    return usbd_ctrl.speed;
}

kdrv_usbd3_link_status_t kdrv_usbd3_get_link_status(void)
{
    return usbd_ctrl.link_status;
}

static kdrv_status_t _endpoint_receive(uint8_t epno, uint32_t buf_addr, uint32_t *blen, uint32_t timeout_ms)
{
    if (ep_ctrl[epno].status == ENP_TRANSFERRING)
        return KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS;
    else if (ep_ctrl[epno].status != ENP_READY_IDLE)
        return KDRV_STATUS_ERROR;

    ep_ctrl[epno].status = ENP_TRANSFERRING;

    uint32_t buf_len = *blen;

    regUSBD_EP_PRD[epno - 1].bf.BTC = buf_len;
    regUSBD_EP_PRD[epno - 1].bf.PAR = buf_addr;
    regUSBD_EP_PRD[epno - 1].bf.H_hw_own = 1;

    // set up endpoint ready
    uint32_t epno_bit = (0x1 << epno);

    regUSBD_EP_PRD_RDY->dw = epno_bit;

    // Now DMA is working ...

    uint32_t wait = (timeout_ms == 0) ? osWaitForever : timeout_ms;
    uint32_t flags = osEventFlagsWait(usbd_ctrl.evtFlag, epno_bit, osFlagsWaitAny, wait);

    if (flags != osFlagsErrorTimeout && ep_ctrl[epno].status == ENP_TRANSFER_DONE)
    {
        // calculate total transfered size
        *blen -= regUSBD_EP_PRD[epno - 1].bf.BTC;
        ep_ctrl[epno].status = ENP_READY_IDLE;

        return KDRV_STATUS_OK;
    }
    else if (flags == osFlagsErrorTimeout)
    {
        _halt_dma_work(epno);
        ep_ctrl[epno].status = (usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED) ? ENP_READY_IDLE : ENP_NOT_AVAILABLE;
        return KDRV_STATUS_TIMEOUT;
    }
    else if (ep_ctrl[epno].status == ENP_TERMINATED)
    {
        *blen = 0;
        ep_ctrl[epno].status = (usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED) ? ENP_READY_IDLE : ENP_NOT_AVAILABLE;
        return KDRV_STATUS_USBD_TRANSFER_TERMINATED;
    }

    return KDRV_STATUS_ERROR;
}

kdrv_status_t kdrv_usbd3_bulk_receive(uint8_t endpoint, void *buf, uint32_t *blen, uint32_t timeout_ms)
{
    usbd3_dbg(">> %s() endpoint 0x%x buf 0x%x blen %d timeout_ms %d\n", __FUNCTION__, endpoint, buf, *blen, timeout_ms);

    uint8_t epno = endpoint & 0xF;

    if ((endpoint & 0x80) != 0x0)
        return KDRV_STATUS_ERROR;

    if (ep_ctrl[epno].bEndpointAddress != endpoint)
        return KDRV_STATUS_ERROR;

    uint32_t cur_write_pos = (uint32_t)buf;
    uint32_t usr_buf_space = *blen;
    *blen = 0; // now it is total received length

    kdrv_status_t status;

    // so if user's buffer length is bigger than PRD_MAX_BUF_SIZE, the transfer has to be splited
    while (1)
    {
        uint32_t cur_buf_len = MIN(usr_buf_space, PRD_MAX_BUF_SIZE);

        status = _endpoint_receive(epno, cur_write_pos, &cur_buf_len, timeout_ms);
        if (status != KDRV_STATUS_OK)
            break;

        *blen += cur_buf_len;

        if (cur_buf_len < PRD_MAX_BUF_SIZE)
            break;

        // more transfer
        cur_write_pos += PRD_MAX_BUF_SIZE;
        usr_buf_space -= PRD_MAX_BUF_SIZE;
    }

#ifdef USE_FAKE_ZLP
    if (*blen >= 4)
    {
        if (*(uint32_t *)((uint32_t)buf + *blen - 4) == FAKE_ZLP_CHECK)
            *blen -= 4;
    }
#else
    // real ZLP handling
#endif

    return status;
}

kdrv_status_t kdrv_usbd3_bulk_receive_zlp(uint8_t endpoint)
{
    kdrv_status_t sts;
    uint8_t epno = endpoint & 0xF;

#ifdef USE_FAKE_ZLP

    // fake zlp
    uint32_t fake_zlp;
    uint32_t fake_zlp_len = 4;

    sts = _endpoint_receive(epno, (uint32_t)(&fake_zlp), &fake_zlp_len, 0);
    if (sts != KDRV_STATUS_OK)
        return sts;

    if (fake_zlp != FAKE_ZLP_CHECK || fake_zlp_len != 4)
        return KDRV_STATUS_ERROR;

#else
    // real ZLP handling
#endif

    return KDRV_STATUS_OK;
}

static kdrv_status_t _endpoint_send(uint8_t epno, uint32_t buf_addr, uint32_t txlen, uint32_t timeout_ms, bool zlp)
{
    if (ep_ctrl[epno].status == ENP_TRANSFERRING)
        return KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS;
    else if (ep_ctrl[epno].status != ENP_READY_IDLE)
        return KDRV_STATUS_ERROR;

    ep_ctrl[epno].status = ENP_TRANSFERRING;

    regUSBD_EP_PRD[epno - 1].bf.BTC = txlen;
    regUSBD_EP_PRD[epno - 1].bf.PAR = buf_addr;
    regUSBD_EP_PRD[epno - 1].bf.F_trans_fin = 1;
    regUSBD_EP_PRD[epno - 1].bf.H_hw_own = 1;
    regUSBD_EP_PRD[epno - 1].bf.O_send_zlp = 0;

    if (zlp)
    {
        // check if multiply of max_packet_size and send ZLP
        regUSBD_EP_PRD[epno - 1].bf.O_send_zlp = (txlen & (ep_ctrl[epno].wMaxPacketSize - 1)) == 0x0 ? 1 : 0;
    }

    // set up endpoint ready
    uint32_t epno_bit = (0x1 << epno);

    regUSBD_EP_PRD_RDY->dw = epno_bit;

    uint32_t wait = (timeout_ms == 0) ? osWaitForever : timeout_ms;

    // frst DMA waiting, second conditional FIFO waiting
    for (int i = 0; i < 2; i++)
    {
        uint32_t flags = osEventFlagsWait(usbd_ctrl.evtFlag, epno_bit, osFlagsWaitAny, wait);

#ifdef ERROR_WATCH_REMOVE_IT_IN_THE_FUTURE
        if (i == 1 && flags == osFlagsErrorTimeout)
        {
            printf("wow... error no 2nd event at sending\n");
            while (1)
                osDelay(10);
        }
#endif

        if (flags != osFlagsErrorTimeout && ep_ctrl[epno].status == ENP_TRANSFER_DONE)
        {
            // DMA/PRD transfer done doesn't mean all data have been transmitted to the host
            // need to check FIFO empty to make sure it really sent all bytes
            // especially when data size <= max_packet_size,
            // DMA done happens immediately whatever host receives data or not !!!

            if (i == 0 && (regUSBD_IGR0->dw & epno_bit) > 0)
            {
                // ok, let's wait for FIFO empty interrupt via thread flag wait ...

                ep_ctrl[epno].status = ENP_TRANSFERRING;

                // enable FIFO int here, one-shot interrupt
                uint32_t grp_0_int = regUSBD_IGER0->dw;
                grp_0_int |= epno_bit;
                regUSBD_IGER0->dw = grp_0_int;

#ifdef ERROR_WATCH_REMOVE_IT_IN_THE_FUTURE
                wait = 3000;
#endif

                continue; // next wait
            }

            ep_ctrl[epno].status = ENP_READY_IDLE;
            return KDRV_STATUS_OK;
        }
        else if (flags == osFlagsErrorTimeout)
        {
            _halt_dma_work(epno);
            ep_ctrl[epno].status = (usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED) ? ENP_READY_IDLE : ENP_NOT_AVAILABLE;
            return KDRV_STATUS_TIMEOUT;
        }
        else if (ep_ctrl[epno].status == ENP_TERMINATED)
        {
            ep_ctrl[epno].status = (usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED) ? ENP_READY_IDLE : ENP_NOT_AVAILABLE;

            return KDRV_STATUS_USBD_TRANSFER_TERMINATED;
        }
    }

    return KDRV_STATUS_ERROR;
}

kdrv_status_t kdrv_usbd3_bulk_send(uint8_t endpoint, void *buf, uint32_t txlen, uint32_t timeout_ms)
{
    usbd3_dbg(">> %s() endpoint 0x%x buf 0x%x txlen %d timeout_ms %d\n", __FUNCTION__, endpoint, buf, txlen, timeout_ms);

    uint8_t epno = endpoint & 0xF;

    if ((endpoint & 0x80) != 0x80)
        return KDRV_STATUS_ERROR;

    if (ep_ctrl[epno].bEndpointAddress != endpoint)
        return KDRV_STATUS_ERROR;

    kdrv_status_t status;
    uint32_t buf_addr = (uint32_t)buf;

    while (1)
    {
        uint32_t cur_txlen = MIN(txlen, PRD_MAX_BUF_SIZE);
        bool zlp = (cur_txlen == txlen);

        status = _endpoint_send(epno, buf_addr, cur_txlen, timeout_ms, zlp);
        if (status != KDRV_STATUS_OK)
            break;

        txlen -= cur_txlen;
        if (txlen == 0)
            break;

        buf_addr += cur_txlen;
    }

    return status;
}

bool kdrv_usbd3_interrupt_send_check_buffer_empty(uint32_t endpoint)
{
    uint8_t epno = endpoint & 0xF;

    if(regUSBD_EP[epno - 1].bf.EPn_FF_EMPTY == 1)
        return true;
    else
        return false;
}

kdrv_status_t kdrv_usbd3_interrupt_send(uint32_t endpoint, void *buf, uint32_t txLen, uint32_t timeout_ms)
{
    usbd3_dbg(">> %s() endpoint 0x%x buf 0x%x txlen %d timeout_ms %d\n", __FUNCTION__, endpoint, buf, txLen, timeout_ms);

    uint8_t epno = endpoint & 0xF;

    if ((endpoint & 0x80) != 0x80)
        return KDRV_STATUS_ERROR;

    if (ep_ctrl[epno].bEndpointAddress != endpoint)
        return KDRV_STATUS_ERROR;

    if (txLen > 1024)
        return KDRV_STATUS_ERROR;

    if (ep_ctrl[epno].status == ENP_TRANSFERRING)
        return KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS;
    else if (ep_ctrl[epno].status != ENP_READY_IDLE)
        return KDRV_STATUS_ERROR;

    ep_ctrl[epno].status = ENP_TRANSFERRING;

    regUSBD_EP_PRD[epno - 1].bf.BTC = txLen;
    regUSBD_EP_PRD[epno - 1].bf.PAR = (uint32_t)buf;
    regUSBD_EP_PRD[epno - 1].bf.F_trans_fin = 1;
    regUSBD_EP_PRD[epno - 1].bf.H_hw_own = 1;
    regUSBD_EP_PRD[epno - 1].bf.O_send_zlp = 0;

    // set up endpoint ready
    uint32_t epno_bit = (0x1 << epno);

    regUSBD_EP_PRD_RDY->dw = epno_bit;

    uint32_t wait = (timeout_ms == 0) ? osWaitForever : timeout_ms;

    uint32_t flags = osEventFlagsWait(usbd_ctrl.evtFlag, epno_bit, osFlagsWaitAny, wait);
    if (flags != osFlagsErrorTimeout && ep_ctrl[epno].status == ENP_TRANSFER_DONE)
    {
        ep_ctrl[epno].status = ENP_READY_IDLE;
        return KDRV_STATUS_OK;
    }
    else if (flags == osFlagsErrorTimeout)
    {
        _halt_dma_work(epno);
        ep_ctrl[epno].status = (usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED) ? ENP_READY_IDLE : ENP_NOT_AVAILABLE;
        return KDRV_STATUS_TIMEOUT;
    }
    else if (ep_ctrl[epno].status == ENP_TERMINATED)
    {
        ep_ctrl[epno].status = (usbd_ctrl.link_status == USBD3_STATUS_CONFIGURED) ? ENP_READY_IDLE : ENP_NOT_AVAILABLE;
        return KDRV_STATUS_USBD_TRANSFER_TERMINATED;
    }

    return KDRV_STATUS_ERROR;

    //return _endpoint_send(epno, (uint32_t)buf, txLen, timeout_ms, false);
}

kdrv_status_t kdrv_usbd3_register_class(kdrv_usbd3_class_t *cls){
    if(cls == NULL){
        return KDRV_STATUS_ERROR;
    }
    if(cls->init == NULL || cls->status_isr_cb == NULL \
        || cls->get_device_desc == NULL || cls->get_configuration_desc == NULL || cls->get_lang_id_str_desc == NULL){
        return KDRV_STATUS_ERROR;
    }
    usbd_ctrl.class_data.init = cls->init;
    usbd_ctrl.class_data.de_init = cls->de_init;
    usbd_ctrl.class_data.status_isr_cb = cls->status_isr_cb;
    usbd_ctrl.class_data.set_intf_setup = cls->set_intf_setup;
    usbd_ctrl.class_data.class_ctl_setup = cls->class_ctl_setup;
    usbd_ctrl.class_data.vendor_ctl_setup = cls->vendor_ctl_setup;
    usbd_ctrl.class_data.feature_ctl_setup = cls->feature_ctl_setup;
    usbd_ctrl.class_data.data_in_cb = cls->data_in_cb;
    usbd_ctrl.class_data.data_out_cb = cls->data_out_cb;
    usbd_ctrl.class_data.get_device_desc = cls->get_device_desc;
    usbd_ctrl.class_data.get_configuration_desc = cls->get_configuration_desc;
    usbd_ctrl.class_data.get_lang_id_str_desc = cls->get_lang_id_str_desc;
    usbd_ctrl.class_data.get_manufacturer_str_desc = cls->get_manufacturer_str_desc;
    usbd_ctrl.class_data.get_product_str_desc = cls->get_product_str_desc;
    usbd_ctrl.class_data.get_serial_str_desc = cls->get_serial_str_desc;
    usbd_ctrl.class_data.get_configuration_str_desc = cls->get_configuration_str_desc;
    usbd_ctrl.class_data.get_interface_str_desc = cls->get_interface_str_desc;
    return KDRV_STATUS_OK;
}

void kdrv_usbd3_open_endpoint(uint8_t ep_no, kdrv_usbd3_ep_type_t ep_type, uint16_t ep_size)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    static uint32_t _next_start_fifo_entry = 0;
    static uint32_t _next_fifo_addr_offset = 0;
    uint32_t grp_0_int = 0;
    
    uint32_t ep_fifo_entry;
    if (usbd_ctrl.speed == USBD3_SUPER_SPEED){
        ep_fifo_entry = 1;
    }
    else{
        // should be USBD3_HIGH_SPEED
        ep_fifo_entry = 2; // FIXME: can also be 1 or 3
        }
    uint32_t epno = ep_no & 0xF;          // endpoint number for EPn
    uint32_t ep_type_i = ep_type & 0x3;           // what kind of transfer endpoint
    uint32_t ep_dir = !!(ep_no & 0x80);   // direction IN or OUT
    uint32_t ep_bw_num = 1;                               // HS ISO, not supported now
    uint32_t ep_ISO_in_pktnum = 0;                        // ISO stuff, not supported now
    uint32_t ep_start_fifo_entry = _next_start_fifo_entry; // fifo entry start number

    _next_start_fifo_entry += ep_fifo_entry; // for next endpoint

    if (_next_start_fifo_entry > MAX_FIFO)
        usbd3_dbg("USBD3 error: FIFO entry is outnumbered!\n");

    uint32_t ep_set1 = 0x0;
    ep_set1 |= (ep_start_fifo_entry << 24) | (ep_fifo_entry << 12) | (ep_ISO_in_pktnum << 6) | (ep_bw_num << 4) | (ep_type_i << 2) | (ep_dir << 1) | 0x1;
    
    // wrtie value to EPn_SET1
    regUSBD_EP[epno - 1].dw.EPn_SET1 = ep_set1;

    // debug only
    usbd3_dbg("endpoint 0x%x EPn_SET1 = 0x%x\n", ep_no, regUSBD_EP[epno - 1].dw.EPn_SET1);

    uint32_t ep_mps = ep_size; // endpoint max packet size
    uint32_t ep_fifo_addr_offset = _next_fifo_addr_offset;

    ep_ctrl[epno].wMaxPacketSize = ep_size;

    _next_fifo_addr_offset += (ep_mps >> 3) * ep_fifo_entry; // for next endpoint

    uint32_t ep_set2 = 0x0;
    ep_set2 |= (ep_fifo_addr_offset << 16) | ep_mps;
    
    // wrtie value to EPn_SET2
    regUSBD_EP[epno - 1].dw.EPn_SET2 = ep_set2;
    
    // debug only
    usbd3_dbg("endpoint 0x%x EPn_SET2 = 0x%x\n", ep_no, regUSBD_EP[epno - 1].dw.EPn_SET2);
    
    // init DMA/PRD stuff
    {
        ep_ctrl[epno].bEndpointAddress = ep_no;

        // clear things for safety
        regUSBD_EP_PRD[epno - 1].dw.word_0 = 0;
        regUSBD_EP_PRD[epno - 1].bf.PAR = 0x0;      // because user buffer is not ready now
        regUSBD_EP_PRD[epno - 1].bf.EPRD_PTR = 0x0; // use only internal PRD
        // set up last EPRD
        regUSBD_EP_PRD[epno - 1].bf.L_last_prd = 1;
        // EPRD interrupt
        regUSBD_EP_PRD[epno - 1].bf.I_int_en = 1;
        usbd3_dbg("regUSBD_EP_PRD[%d] = 0x%x, 0x%x, 0x%x\n", epno, regUSBD_EP_PRD[epno - 1].dw.word_0, regUSBD_EP_PRD[epno - 1].dw.word_1, regUSBD_EP_PRD[epno - 1].dw.word_2);
        grp_0_int = (0x1 << (epno + 16));
    }
    ep_ctrl[epno].status = ENP_READY_IDLE;

    regUSBD_IGER0->dw |= grp_0_int;
}

kdrv_status_t kdrv_usbd3_control_send(uint32_t *data, uint32_t len)
{
    usbd3_dbg(">> %s()\n", __FUNCTION__);

    usbd_ctrl.running_cx_transfer = true;
    usbd_ctrl.cx_txfer_len = len;
    usbd_ctrl.cx_data = data;

    usbd3_dbg("dev descp: total txfer %d\n", usbd_ctrl.cx_txfer_len);

    regUSBD_CX_Config_Status->bf.CXF_LEN = usbd_ctrl.cx_txfer_len;

    cx_txfer_data_send();
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_usbd3_control_read(uint32_t *buf, uint32_t rx_len){
    usbd3_dbg(">> %s()\n", __FUNCTION__);
    
    int32_t recv_len = regUSBD_CX_Config_Status->bf.CXF_LEN;
    if(recv_len < rx_len){
        return KDRV_STATUS_ERROR;
    }
    for (int32_t r = recv_len; r > 0; r -= 4)
    {
        *(buf) = regUSBD_CX_PORT->dw;
        buf++;
    }
    usbd_ctrl.running_cx_transfer = false;
    return KDRV_STATUS_OK;
}
