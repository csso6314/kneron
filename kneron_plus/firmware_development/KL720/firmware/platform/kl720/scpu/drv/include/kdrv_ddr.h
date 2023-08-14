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
#ifndef __KDRV_DDR_H__
#define __KDRV_DDR_H__

#include "base.h"
#include "kneron_kl720.h"
#include "kdrv_pll.h"

#define DO_SELFTEST             1
#define DO_DDR_DGB_INTERFACE    1
#define DDR_INIT_PRINT_LOG

typedef enum{
    LPDDR3_800_,             //Option LPDDR3-800 for bring up debug
    LPDDR3_1333_,            //Option LPDDR3-1333  for bring up debug
    LPDDR3_1600_,            //Option LPDDR3-1600  for bring up debug
    LPDDR3_1866_,            //Option LPDDR3-1866  for bring up debug
    LPDDR3_2133_,            //Default LPDDR3-2133
    LPDDR3_OPT_MAX
}lpddr3_opt;

uint32_t kdrv_ddr_Initialize(uint32_t axi_ddr_clk);

#endif //__KDRV_DDRC_H__

