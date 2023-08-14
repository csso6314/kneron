/**
 * Kneron System API - IPC
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */


#include "base.h"
#include "regbase.h"
#include "kdrv_ipc.h"
//#include "scpu_log.h"

typedef volatile struct{
    uint32_t    scpu;     // 0x0128 scpu ipc register
    uint32_t    ncpu;     // 0x012C ncpu(dsp) ipc register
} kdrv_ipc_reg_t;

kdrv_ipc_reg_t *ipc_reg = (kdrv_ipc_reg_t*) (SCU_EXT_REG_BASE + 0x0128);


/* kdrv_ipc_reg_t.scpu */
#define SCPU_IPC_INT_EN         BIT(0)  // SCPU IPC interrupt enable (RW)
#define SCPU_IPC_INT_TRG        BIT(1)  // SCPU IPC trigger (WO)
#define SCPU_IPC_HI_EN          BIT(2)  // SCPU IPC high priority enable (RW)
#define SCPU_IPC_HI_TRG         BIT(3)  // SCPU IPC high priority trigger (WO
#define SCPU_IPC_STS            BIT(4)  // SCPU IPC status (RO)
#define SCPU_IPC_CLR            BIT(5)  // SCPU IPC clear (W1C)

/* kdrv_ipc_reg_t.ncpu */
#define NCPU_IPC_INT_EN         BIT(0)  // NCPU IPC interrupt enable (RW)
#define NCPU_IPC_INT_TRG        BIT(1)  // NCPU IPC trigger (WO)
#define NCPU_IPC_STS            BIT(4)  // NCPU IPC status (RO)
#define NCPU_IPC_CLR            BIT(5)  // NCPU IPC clear (W1C) 
#define NCPU_IPC_STS_HI         BIT(6)  // NCPU IPC high status (RO)
#define NCPU_IPC_CLR_HI         BIT(7)  // NCPU IPC high status clear (W1C)


void kdrv_ipc_enable_to_ncpu_int(void)
{
    ipc_reg->scpu = SCPU_IPC_INT_EN;
}

void kdrv_ipc_trigger_to_ncpu_int(void)
{
    ipc_reg->scpu = SCPU_IPC_INT_TRG | SCPU_IPC_INT_EN;
}

void kdrv_ipc_clear_from_ncpu_int(void)
{
    ipc_reg->scpu = SCPU_IPC_CLR | SCPU_IPC_INT_EN;
}


