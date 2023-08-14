/*
 * Kneron KL720 NCPU driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
 
#include "regbase.h" 
#include "kdrv_ncpu.h"
#include "kdrv_scu_ext.h"

typedef volatile struct{
    uint32_t    clken;      // 0x00 ncpu clock register
    uint32_t    reset;      // 0x04 [0]: ncpu main reset, high active, [1] ncpu apb resetn, low active
    uint32_t    dbreset;    // 0x08 ncpu debug reset, hight active
    uint32_t    cfg0;       // 0x0C reset vector sectect. 0: iram, 1: ddr 
    uint32_t    stall;      // 0x10 force stall, ncpu clock disable, 0:none, 1:force
    uint32_t    prid;       // 0x14 ncpu process id
    uint32_t    idmatrig;   // 0x18 idma trigger intferace
    uint32_t    status0;    // 0x1C status register
    uint32_t    status1;    // 0x20 status register
    uint32_t    nalign;     // 0x24 non-align register
    uint32_t    dbcfg;      // 0x28 debug config register
    uint32_t    dbgst;      // 0x2C debug status
} ncpu_reg_t;

ncpu_reg_t *ncpu_reg = (ncpu_reg_t*) (NCPU_REG_BASE + 0x4000);

/* ncpu_reg_t.clken, 0x4000 */
#define NCPU_CLKEN_APB              BIT(0)  // ncpu apb clock enable
/* ncpu_reg_t.reset, 0x04 */
#define NCPU_RESET_EN               1       // main resetn, high active
#define NCPU_RESET_DIS              2       // apb interface resetn, low active
/* ncpu_reg_t.dbreset, 0x08 */
#define NCPU_DBRESET_DRESET_MASK

#define NCPU_DBRESET_DIS            0       
#define NCPU_DBRESET_EN             BIT(0)  // debug reset, high active
/* ncpu_reg_t.cfg0, 0x0C */
#define NCPU_STAVECTOR_IRAM         0  
#define NCPU_STAVECTOR_DDR          BIT(0)  // 0: start vector on iram, 1: start vector on ddr
/* ncpu_reg_t.stall, 0x10 */
#define NCPU_STALL_DIS              0
#define NCPU_STALL_EN               BIT(0)  // force ncpu stall

/*
 * @ncpu_set_stall set ncpu into stall mode
 */
void kdrv_ncpu_set_stall(uint8_t is_stall)
{
    ncpu_reg->stall = is_stall;       
}

uint8_t kdrv_get_stall_status(void)
{
    return(ncpu_reg->stall);
}

/*
 * @kdrv_ncpu_reset set ncpu(dsp) reset
 */
void kdrv_ncpu_reset(void)
{
    ncpu_reg->reset = NCPU_RESET_EN;
    ncpu_reg->reset = NCPU_RESET_DIS;
}

/*
 * @brief ncpu boot release
 */
void kdrv_ncpu_boot_initialize(void)
{
    ncpu_reg->dbreset = NCPU_DBRESET_DIS;
    ncpu_reg->reset = NCPU_RESET_DIS;
    ncpu_reg->stall = NCPU_STALL_DIS;
    /* Turn on the 9x9 DSP_JTAG_TDO(PWM1) clock gated */
    regSCUEXT->spare_default_0r0 |= 0x200;
}


