/*
 * Kneron Power Manager driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
 
#include <string.h>
#include "cmsis_os2.h"
#include "base.h"
#include "os_tick.h"
#include "kmdw_power_manager.h"
#include "kdrv_scu.h"
#include "kdrv_clock.h"
#include "kdrv_rtc.h"
#include "kmdw_console.h"
#include "io.h"
#include "RTX_Config.h"
#include "kdrv_cmsis_core.h"

#define FLAG_SYSTEM_RESET       BIT0
#define FLAG_SYSTEM_NAP         BIT1
#define FLAG_SYSTEM_NAP2        BIT2
#define FLAG_SYSTEM_SLEEP       BIT3
#define FLAG_SYSTEM_DEEP_SLEEP  BIT4
#define FLAG_SYSTEM_TIMER       BIT5
#define FLAG_SYSTEM_SHUTDOWN    BIT6
#define FLAG_SYSTEM_ERROR       BIT8
#define FLAG_SYSTEM_PWRBTN_FALL BIT16
#define FLAG_SYSTEM_PWRBTN_RISE BIT17

#define FLAGS_ALL       (FLAG_SYSTEM_RESET | FLAG_SYSTEM_SHUTDOWN \
                        | FLAG_SYSTEM_NAP | FLAG_SYSTEM_NAP2 \
                        | FLAG_SYSTEM_SLEEP | FLAG_SYSTEM_DEEP_SLEEP \
                        | FLAG_SYSTEM_TIMER | FLAG_SYSTEM_ERROR \
                        | FLAG_SYSTEM_PWRBTN_FALL | FLAG_SYSTEM_PWRBTN_RISE)

#define PERIOD_PRINT        (3 * OS_TICK_FREQ)                  // 3 secs
#define PERIOD_COUNT        (PERIOD_PRINT + PERIOD_PRINT/100)   // add 1% margin

/* Inactivity timers in seconds */
#define NAP_TIME_1              30
#define NAP_TIME_2              60

osThreadId_t    power_tid;
uint32_t cpu_idle_counter = 0;

uint32_t idle_entry_time_in_secs;
uint32_t idle_exit_time_in_secs;
uint32_t sleep_state;

kmdw_power_manager_ptn_handler ptn_cb;

static struct pm_device_func_s {
    enum kmdw_power_manager_device_id   dev_id;
    int                 inuse;
    struct kmdw_power_manager_s         pm;
} _pm_dev_fns[KMDW_POWER_MANAGER_DEVICE_MAX];

static void _scu_system_isr(void)
{
    static int pwr_button_wakeup = 1;   // = 1 for cold boot by PWR button
    uint32_t status;

    status = inw(SCU_REG_INT_STS);

    if (status & SCU_INT_RTC_ALARM)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_NAP);
    if (status & SCU_INT_RTC_PERIODIC)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_TIMER);
    if (status & SCU_INT_PWRBTN_FALL) {
        if (pwr_button_wakeup) {
            pwr_button_wakeup = 0;
        } else {
            osThreadFlagsSet(power_tid, FLAG_SYSTEM_PWRBTN_FALL);
        }
    }
    if (status & SCU_INT_PWRBTN_RISE) {
        if (sleep_state == 1) {
            pwr_button_wakeup = 1;
        } else {
            pwr_button_wakeup = 0;
            osThreadFlagsSet(power_tid, FLAG_SYSTEM_PWRBTN_RISE);
        }
    }

    outw(SCU_REG_INT_STS, status);
    NVIC_ClearPendingIRQ(SYS_IRQn);
}

static void _scu_system_init(void)
{
    NVIC_DisableIRQ(SYS_IRQn);

    kdrv_rtc_initialize();
    outw(SCU_REG_INT_STS, 0xffffffff);  // Clear all old ones

    /* Enable PWR button interrupt and wakeup */
    outw(SCU_REG_INT_EN, SCU_INT_PWRBTN_FALL | SCU_INT_PWRBTN_RISE | SCU_INT_WAKEUP);

    /* Enable nap alarm interrupt */
    uint32_t nap_time = NAP_TIME_1;
    kdrv_rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
    masked_outw(SCU_REG_INT_EN, SCU_INT_RTC_ALARM, SCU_INT_RTC_ALARM);

    NVIC_SetVector(SYS_IRQn, (uint32_t)_scu_system_isr);
    NVIC_EnableIRQ(SYS_IRQn);
}

#define MMFAR 0xE000ED34
#define FLAG_WAIT_FOREVER 0x40000000
static void _scpu_wait_reset(void)
{
    osThreadId_t calling_tid = osThreadGetId();
    if ((calling_tid == 0) || (calling_tid == power_tid)){  // no os or if power mgmnt thread is in trouble
#if 0
            kdrv_power_sw_reset();
#else
            for (;;);
#endif
    }
    else  // let power mgmnt thread handles the reset
        osThreadFlagsWait((uint32_t)calling_tid , FLAG_WAIT_FOREVER, osWaitForever);
}

register unsigned int _msp __asm("msp");
register unsigned int _psp __asm("psp");
register unsigned int _lr  __asm("lr");
static unsigned int stack, pc;

static void _scpu_hard_fault(void)
{
    if (_lr & 4) {
        stack = _psp;
        pc = stack + 24;
    }
    else {
        stack = _msp;
        pc = stack + 40;
    }

    err_msg("scpu: hard fault @ %08X, PC = %08X, LR = %08X, SP = %08X\n", *(uint32_t*)MMFAR,
        *(uint32_t*)pc, *(uint32_t*)(pc-4), (uint32_t)pc+8);
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
    *(uint32_t*)pc = (uint32_t)&_scpu_wait_reset;  // modify stack to go to the wait forever loop
}

static void _scpu_mem_mnmt(void)
{
    if (_lr & 4) {
        stack = _psp;
        pc = stack + 24;
    }
    else {
        stack = _msp;
        pc = stack + 40;
    }

    err_msg("scpu: memory fault @ %08X, PC = %08X, LR = %08X, SP = %08X\n", *(uint32_t*)MMFAR,
        *(uint32_t*)pc, *(uint32_t*)(pc-4), (uint32_t)pc+8);
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
    *(uint32_t*)pc = (uint32_t)&_scpu_wait_reset;  // modify stack to go to the wait forever loop
}

static void _scpu_bus_fault(void)
{
    err_msg("scpu: scpu_bus_fault !\n");
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
}

static void _scpu_usage_fault(void)
{
    err_msg("scpu: _scpu_usage_fault !\n");
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
}

void kmdw_power_manager_error_notify(uint32_t code, void *object_id)
{
    err_msg("scpu: exception: code=%d, object_id=0x%p\n", code, object_id);
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
}

void kmdw_power_manager_reset(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_RESET);
}

void kmdw_power_manager_sleep(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_SLEEP);
}

void kmdw_power_manager_deep_sleep(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_DEEP_SLEEP);
}

void kmdw_power_manager_shutdown(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_SHUTDOWN);
}

static void _power_mgr_cpu_usage(void)
{
    static uint32_t last_record=0, diff;
  #ifdef LOG_ENABLE
    static uint32_t print_count=0;
  #endif

    diff = (cpu_idle_counter - last_record);
    last_record = cpu_idle_counter;

    if (diff > PERIOD_COUNT)
        diff = PERIOD_COUNT;

    info_msg("#%04d cpu loading %d %%\n", ++print_count,
            (PERIOD_COUNT - diff) * 100 / PERIOD_COUNT);
}

__NO_RETURN void power_manager_cpu_idle(void)
{
    uint32_t tick_start, tick_end, tick_idle;

    while(1) {
        kdrv_rtc_get_date_time_in_secs(&idle_entry_time_in_secs);
        tick_start = osKernelGetTickCount();
        __WFI();
        tick_end = osKernelGetTickCount();
        tick_idle = tick_end - tick_start;
        cpu_idle_counter += tick_idle;
        kdrv_rtc_get_date_time_in_secs(&idle_exit_time_in_secs);
    }
}

static void _power_manager_do_nap(void)
{
    int i;

    for (i = KMDW_POWER_MANAGER_DEVICE_MAX - 1; i >= 0; i--) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.nap)
        {
            if(_pm_dev_fns[i].pm.nap(_pm_dev_fns[i].dev_id) < 0)
            {
                info_msg("Can't take a nap\n");
                return;
            }
        }
    }
    dbg_msg("Take a nap\n");
    /* Disable npu/ncpu clocks */
    kdrv_clock_disable_npu_clk();
    kdrv_clock_disable_dsp_clk();
    __WFI();
    kdrv_clock_enable_npu_clk();
    kdrv_clock_enable_dsp_clk();

    for (i = 0; i < KMDW_POWER_MANAGER_DEVICE_MAX; i++) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.wakeup_nap)
            _pm_dev_fns[i].pm.wakeup_nap(_pm_dev_fns[i].dev_id);
    }
}

static void _power_manager_do_deep_nap(void)
{
    int i;

    for (i = KMDW_POWER_MANAGER_DEVICE_MAX - 1; i >= 0; i--) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.deep_nap)
        {
            if(_pm_dev_fns[i].pm.deep_nap(_pm_dev_fns[i].dev_id) < 0)
            {
                info_msg("Can't take a deep nap\n");
                return;
            }
        }
    }
    dbg_msg("Take a deep nap\n");
    /* Disable npu/ncpu clocks + DDR self refresh */
    kdrv_clock_disable_npu_clk();
    kdrv_clock_disable_dsp_clk();
    //TODO: kdrv_ddr_self_refresh_enter();
    __WFI();
    //TODO: kdrv_ddr_self_refresh_exit();
    kdrv_clock_enable_npu_clk();
    kdrv_clock_enable_dsp_clk();

    for (i = 0; i < KMDW_POWER_MANAGER_DEVICE_MAX; i++) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.wakeup_deep_nap)
            _pm_dev_fns[i].pm.wakeup_deep_nap(_pm_dev_fns[i].dev_id);
    }
}

static void _power_manager_do_sleep(void)
{
    int i;

    for (i = KMDW_POWER_MANAGER_DEVICE_MAX - 1; i >= 0; i--) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.sleep)
        {
            if(_pm_dev_fns[i].pm.sleep(_pm_dev_fns[i].dev_id) < 0)
            {
                info_msg("Can't sleep\n");
                return;
            }
        }
    }
    dbg_msg("!!! sleep\n");

    sleep_state = 1;

    /* Retention: NPU power domain off */
    kdrv_clock_disable_npu_clk();
    kdrv_clock_disable_dsp_clk();
    //TODO: kdrv_ddr_self_refresh_enter();
    //TODO: kdrv_power_set_domain(POWER_DOMAIN_NPU, 0);
    __WFI();
    //TODO: kdrv_power_set_domain(POWER_DOMAIN_NPU, 1);
    //TODO: kdrv_ddr_self_refresh_exit();
    kdrv_clock_enable_npu_clk();
    kdrv_clock_enable_dsp_clk();
    //TODO: load_ncpu_fw(0);  // reload ncpu fw but don't start it yet
    //TODO: system_wakeup_ncpu(0, 1);
    
    // ddr doesn't seemed correct, reload default models for testing ???

    sleep_state = 0;
    dbg_msg("!!! sleep -> wakeup\n");

    for (i = 0; i < KMDW_POWER_MANAGER_DEVICE_MAX; i++) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.wakeup_sleep)
            _pm_dev_fns[i].pm.wakeup_sleep(_pm_dev_fns[i].dev_id);
    }
}

static void _power_manager_do_deep_sleep(void)
{
    int i;

    for (i = KMDW_POWER_MANAGER_DEVICE_MAX - 1; i >= 0; i--) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.deep_sleep)
        {
            if(_pm_dev_fns[i].pm.deep_sleep(_pm_dev_fns[i].dev_id) < 0)
            {
                info_msg("Can't deep sleep\n");
                return;
            }
        }
    }

    dbg_msg("!!! deep sleep\n\n");

    /* Deep Retention: NPU+Default domain off */
    //TODO: kdrv_power_softoff(POWER_MODE_DEEP_RETENTION);
    __WFI();

    err_msg("!!! deep sleep failed!\n");

    /* TODO: resume here */
    for (i = 0; i < KMDW_POWER_MANAGER_DEVICE_MAX; i++) {
        if (_pm_dev_fns[i].inuse && _pm_dev_fns[i].pm.wakeup_deep_sleep)
            _pm_dev_fns[i].pm.wakeup_deep_sleep(_pm_dev_fns[i].dev_id);
    }
}

static void _power_manager_do_shutdown(void)
{
    dbg_msg("!!! shutdown ...\n\n");

    /* Disable alarm */
    kdrv_rtc_alarm_disable();

    /* Power off everything except RTC */
    //TODO: kdrv_power_softoff(POWER_MODE_RTC);
    __WFI();

    err_msg("!!! shutdown failed!\n");
    for (;;);
}

//#define PRINT_CPU_USAGE

void _kmdw_power_manager_thread(void *arg)
{
    uint32_t status, timeout;
    uint32_t current_time, elapsed_time, nap_time, pwrbtn_press_time, pwrbtn_release_time;

    /* Init system/power/rtc control on SCU */
    _scu_system_init();

#ifdef PRINT_CPU_USAGE
    timeout = PERIOD_PRINT;
#else
    timeout = osWaitForever;
#endif

    while(1)
    {
        status = osThreadFlagsWait(FLAGS_ALL, osFlagsWaitAny, timeout);

        if (status == osFlagsErrorTimeout) {
            _power_mgr_cpu_usage();
            continue;
        }

        if (status & FLAG_SYSTEM_SLEEP) {
            _power_manager_do_sleep();
        }

        if (status & FLAG_SYSTEM_DEEP_SLEEP) {
            _power_manager_do_deep_sleep();
        }

        if (status & FLAG_SYSTEM_RESET) {
            info_msg("!!! reset\r\n");
            // will not come back
            //TODO: kdrv_power_sw_reset();
        }

        if (status & FLAG_SYSTEM_SHUTDOWN) {
            // will not come back
            _power_manager_do_shutdown();
        }

        if (status & FLAG_SYSTEM_NAP) {

            if (idle_exit_time_in_secs > idle_entry_time_in_secs)
                elapsed_time = idle_exit_time_in_secs - idle_entry_time_in_secs;
            else
                elapsed_time = 0;

            /* update */
            kdrv_rtc_get_date_time_in_secs(&idle_entry_time_in_secs);

            /* set next alarm */
            if (elapsed_time < NAP_TIME_1) {
                nap_time = NAP_TIME_1;
                kdrv_rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
            } else if (elapsed_time < NAP_TIME_2) {
                //rtc_current_time_info();
                dbg_msg("Idle: %d seconds -> nap\n", elapsed_time);
                /* Set longer nap time */
                nap_time = NAP_TIME_2;
                kdrv_rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
                /* Take nap */
                _power_manager_do_nap();
                /* regular nap time upon wakeup */
                nap_time = NAP_TIME_1;
                kdrv_rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
            } else {
                //rtc_current_time_info();
                dbg_msg("Idle: %d seconds -> deep nap\n", elapsed_time);
                /* Set even longer nap time */
                nap_time = NAP_TIME_2 * 10;
                kdrv_rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
                /* Take deep nap */
                _power_manager_do_deep_nap();
                /* regular nap time upon wakeup */
                nap_time = NAP_TIME_1;
                kdrv_rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
            }
        }

        if (status & FLAG_SYSTEM_TIMER) {
            kdrv_rtc_get_date_time_in_secs(&current_time);
            elapsed_time = current_time - idle_entry_time_in_secs;

            dbg_msg("Idle: %d\n", elapsed_time);
        }

        if (status & FLAG_SYSTEM_ERROR) {
            err_msg("!!! scpu: error\n");
#if 0
            kdrv_power_sw_reset();
#else
            // for debug
            for (;;);
#endif
        }

        if (status & FLAG_SYSTEM_PWRBTN_FALL) {
            kdrv_rtc_get_date_time_in_secs(&pwrbtn_release_time);
            elapsed_time = pwrbtn_release_time - pwrbtn_press_time;
            info_msg("!!! PWR Button pressed for %d seconds:\n", elapsed_time);
            if (elapsed_time > 6)
                _power_manager_do_shutdown();
            else if (ptn_cb)
                ptn_cb(1);
        }

        if (status & FLAG_SYSTEM_PWRBTN_RISE) {
            kdrv_rtc_get_date_time_in_secs(&pwrbtn_press_time);
            info_msg("!!! PWR Button Press&Hold 7+ seconds for shutdown (RTC mode)\n");
        }
    }
}

/* Registration APIs */
int kmdw_power_manager_register(enum kmdw_power_manager_device_id dev_id, struct kmdw_power_manager_s *pm_p)
{
    int i;

    if (dev_id >= KMDW_POWER_MANAGER_DEVICE_MAX || pm_p == NULL)
        return -1;

    for (i = 0; i < KMDW_POWER_MANAGER_DEVICE_MAX; i++) {
        if (_pm_dev_fns[i].inuse == 0) {
            memcpy(&_pm_dev_fns[i].pm, pm_p, sizeof(struct kmdw_power_manager_s));
            _pm_dev_fns[i].dev_id = dev_id;
            _pm_dev_fns[i].inuse = 1;
            break;
        }
    }

    return 0;
}

void kmdw_power_manager_unregister(enum kmdw_power_manager_device_id dev_id, struct kmdw_power_manager_s *pm_p)
{
    int i;

    if (dev_id >= KMDW_POWER_MANAGER_DEVICE_MAX || pm_p == NULL)
        return;

    for (i = 0; i < KMDW_POWER_MANAGER_DEVICE_MAX; i++) {
        if (_pm_dev_fns[i].dev_id == dev_id && _pm_dev_fns[i].pm.sleep == pm_p->sleep && _pm_dev_fns[i].inuse) {
            memset(&_pm_dev_fns[i].pm, 0, sizeof(struct kmdw_power_manager_s));
            _pm_dev_fns[i].dev_id = KMDW_POWER_MANAGER_DEVICE_NONE;
            _pm_dev_fns[i].inuse = 0;
            return;
        }
    }
}

void kmdw_power_manager_power_button_register(kmdw_power_manager_ptn_handler button_handler)
{
    ptn_cb = button_handler;
}

void kmdw_power_manager_init(void)
{
    osThreadAttr_t attr;

    memset(&attr, 0, sizeof(attr));
    attr.priority = osPriorityRealtime7;
    power_tid = osThreadNew(_kmdw_power_manager_thread, NULL, &attr);
    if( !power_tid )
        err_msg("[******** ERROR ********] power_manager_thread not launched\n");

    NVIC_SetVector(HardFault_IRQn, (uint32_t)_scpu_hard_fault);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)_scpu_mem_mnmt);
    NVIC_SetVector(BusFault_IRQn, (uint32_t)_scpu_bus_fault);
    NVIC_SetVector(UsageFault_IRQn, (uint32_t)_scpu_usage_fault);
}
