/**
 * Kneron System API - IPC
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "regbase.h"
#include "io.h"
#include "base.h"
#include "kdrv_ipc.h"


/* IPC Registers */
#define SCPU_IPC_REG_ADDR(base)     (base + 0x0070)
#define NCPU_IPC_REG_ADDR(base)     (base + 0x0074)

/* IPC-From bits */
#define IPC_CLR_FROM           BIT(5)
#define IPC_STATUS_FROM        BIT(4)

/* IPC-To bits */
#define IPC_INT_TO             BIT(1)
#define IPC_ENABLE_TO          BIT(0)

void kdrv_ipc_enable_to_ncpu_int(void)
{
    masked_outw(SCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_ENABLE_TO, IPC_ENABLE_TO);
}

void kdrv_ipc_trigger_to_ncpu_int(void)
{
    masked_outw(SCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_INT_TO, IPC_INT_TO);
}

void kdrv_ipc_clear_from_ncpu_int(void)
{
    masked_outw(SCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_CLR_FROM, IPC_CLR_FROM);
}
