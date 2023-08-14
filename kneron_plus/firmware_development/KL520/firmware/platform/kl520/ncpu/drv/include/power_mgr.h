/*
 * Kneron NCPU Power Manager driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __POWER_MGR_H__
#define __POWER_MGR_H__

__NO_RETURN void power_mgr_cpu_idle(void);

void power_mgr_init(void);
void power_mgr_error_notify(uint32_t code, void *object_id);

#endif
