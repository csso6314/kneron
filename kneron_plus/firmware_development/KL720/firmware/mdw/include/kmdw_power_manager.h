/**
 * @file        kmdw_power_manager.h
 * @brief       Power Manager driver
 *
 * @copyright  Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KMDW_POWER_MANAGER_H__
#define __KMDW_POWER_MANAGER_H__

enum kmdw_power_manager_device_id {
    KMDW_POWER_MANAGER_DEVICE_NONE = 0,
    KMDW_POWER_MANAGER_DEVICE_CAMERA1,
    KMDW_POWER_MANAGER_DEVICE_CAMERA2,
    KMDW_POWER_MANAGER_DEVICE_DISPLAY1,
    KMDW_POWER_MANAGER_DEVICE_DISPLAY2,
    KMDW_POWER_MANAGER_DEVICE_HOST_COM,
    KMDW_POWER_MANAGER_DEVICE_dfu_UPDATE,
    KMDW_POWER_MANAGER_DEVICE_NCPU_INFERENCE,
    KMDW_POWER_MANAGER_DEVICE_UNUSED4,
    KMDW_POWER_MANAGER_DEVICE_UNUSED5,
    KMDW_POWER_MANAGER_DEVICE_UNUSED6,
    KMDW_POWER_MANAGER_DEVICE_UNUSED7,
    KMDW_POWER_MANAGER_DEVICE_UNUSED8,
    KMDW_POWER_MANAGER_DEVICE_MAX,
};

/* Prototypes for callback functions */
typedef int (*kmdw_power_manager_call)(enum kmdw_power_manager_device_id dev_id);
typedef void (*kmdw_power_manager_ptn_handler)(int released);

struct kmdw_power_manager_s {
    kmdw_power_manager_call     nap;
    kmdw_power_manager_call     wakeup_nap;
    kmdw_power_manager_call     deep_nap;
    kmdw_power_manager_call     wakeup_deep_nap;
    kmdw_power_manager_call     sleep;
    kmdw_power_manager_call     wakeup_sleep;
    kmdw_power_manager_call     deep_sleep;
    kmdw_power_manager_call     wakeup_deep_sleep;
};

/* PM APIs */
__NO_RETURN void kmdw_power_manager_cpu_idle(void);
void kmdw_power_manager_init(void);
void kmdw_power_manager_error_notify(uint32_t code, void *object_id);
void kmdw_power_manager_reset(void);
void kmdw_power_manager_sleep(void);
void kmdw_power_manager_deep_sleep(void);
void kmdw_power_manager_shutdown(void);

/* Registration APIs */
int kmdw_power_manager_register(
    enum kmdw_power_manager_device_id dev_id, 
    struct kmdw_power_manager_s *pm_p);
void kmdw_power_manager_unregister(
    enum kmdw_power_manager_device_id dev_id, 
    struct kmdw_power_manager_s *pm_p);

void kmdw_power_button_register(kmdw_power_manager_ptn_handler button_handler);

#endif
