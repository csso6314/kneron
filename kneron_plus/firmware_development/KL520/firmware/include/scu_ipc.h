/**
 * @file        scu_ipc.h
 * @brief       Kneron Header for KL520 SCU IPC
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2018-2021 Kneron Inc. All rights reserved.
 */


#ifndef SCU_IPC_H
#define SCU_IPC_H

#if defined(TARGET_SCPU)
/* To NCPU */
void scu_ipc_enable_to_ncpu_int(void);
void scu_ipc_trigger_to_ncpu_int(void);

/* From NCPU */
void scu_ipc_clear_from_ncpu_int(void);
#endif

#if defined(TARGET_NCPU)
/* To SCPU */
void scu_ipc_enable_to_scpu_int(void);
void scu_ipc_trigger_to_scpu_int(void);

/* From SCPU */
void scu_ipc_clear_from_scpu_int(void);
#endif

#endif
