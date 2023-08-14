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
*  kdrv_pinmux.c
*
*  Project:
*  --------
*  KL720
*
*  Description:
*  ------------
*  This is pinmux config driver
*
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

/* Comment block of the file */

// Sec 1: Include File

#include <stdarg.h>
#include "kdrv_scu_ext.h"
#include "kdrv_pinmux.h"
#include "kdrv_io.h"
#include "regbase.h"
#include "base.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous


/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype


/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/
void kdrv_pinmux_initialize(uint32_t num, uint32_t *p_array)
{
    for(uint32_t i = 0; i < num; i++)
    {
        uint32_t reg_addr = SCU_EXTREG_IOCTL_SPI_CS_N + (i * 4);
        SET_MASKED_BITS(reg_addr, p_array[i], 0, 31)
    }
}

void kdrv_pinmux_config(kdrv_pin_name pin, kdrv_pinmux_mode mode, kdrv_pin_pull pull_type, kdrv_pin_driving driving)
{
    uint32_t reg_addr = SCU_EXTREG_IOCTL_SPI_CS_N + (pin * 4);
    uint32_t ioctrl_bits = 0;

    /*Pinmux mode configure*/
    ioctrl_bits |= mode;

    /*Pull up, Pull down configure*/
    if(pull_type == PIN_PULL_UP)
        ioctrl_bits |= BIT16;
    else if(pull_type == PIN_PULL_DOWN)
        ioctrl_bits |= BIT17;

    /*Output Driving configure*/
    if(driving == PIN_DRIVING_8MA)
        ioctrl_bits |= BIT21;
    else if(driving == PIN_DRIVING_12MA)
        ioctrl_bits |= BIT22;
    else if(driving == PIN_DRIVING_16MA)
        ioctrl_bits |= (BIT21 | BIT22);

    SET_MASKED_BITS(reg_addr, ioctrl_bits, 0, 31)
}

