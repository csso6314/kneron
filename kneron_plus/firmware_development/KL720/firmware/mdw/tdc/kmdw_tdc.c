#include "kdrv_status.h"
#include "kdrv_clock.h"
#include "kdrv_pll.h"
#include "kmdw_console.h"
#include "kdrv_tdc.h"
#include "kmdw_tdc.h"
#include "kmdw_power_manager.h"
#include "project.h"   /* for TDC_HW_PROTECTION_DFS*/

#if ((defined TDC_HW_PROTECTION_DFS) && (TDC_HW_PROTECTION_DFS == 1)) 
#include "kmdw_dfs.h"

#endif

#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))
#if ((defined TDC_HW_PROTECTION) && (TDC_HW_PROTECTION == 1))
static bool npu_is_running = false;
static npu_clk_setting curr_npu_clock = NPU_700_CFG1;
#endif

#define USE_POLLING_MODE        1

static uint32_t kmdw_tdc_intsts = 0; 
static osThreadId_t tdc_thread_id = NULL;
static int32_t int_temp;
static bool rgst_tdc_cb = false;

static uint32_t poll_time_external = 0;

#define ROUND(X)  ((X>=0)?(int32_t)(X + 0.5f) : ((int32_t)(X - 0.5f)))
#define DLY_TIME_100MS          (1000 * 1000) //2 sec
#define KMDW_TDC_FLAG_SET       0x1001

#define PERIOD_MONITOR          (2 * 1000)      // 2 secs
#define KMDW_TDC_DBG            1
#define ABS(a)                  (((a >> 31) ^ a) - (a >> 31))


#if ((defined TDC_HW_PROTECTION) && (TDC_HW_PROTECTION == 1))
static void kmdw_tdc_temperature_protection(void)
{
    if((int_temp > TDC_DEGREE_CELSIUS_DANGEROUS) && (npu_is_running == false)) {
        if(curr_npu_clock != NPU_300_CFG1) {
            kdrv_clock_npu_clock_change(NPU_300_CFG1);
            curr_npu_clock = NPU_300_CFG1;
        }
    } else if((int_temp < TDC_DEGREE_CELSIUS_SAFE) && (npu_is_running == false)){
        if(curr_npu_clock != NPU_700_CFG1) {
            kdrv_clock_npu_clock_change(NPU_700_CFG1);
            curr_npu_clock = NPU_700_CFG1;
        }
    }
}

void hook_ncpu_start_for_tdc_check(void)
{
    npu_is_running = true;
}

void hook_ncpu_stop_for_tdc_check(void)
{
    npu_is_running = false;
}
#endif

/*
 * @brief kmdw_tdc_callback(), tdc isr callback function
 */
void kmdw_tdc_callback(void *arg)
{
    kmdw_tdc_intsts = kdrv_tdc_int_status_read();
    osThreadFlagsSet(tdc_thread_id, KMDW_TDC_FLAG_SET);
}

static void kmdw_tdc_update_internal(void)
{
    static int32_t curr_print_temp = 0;
    int8_t temp_quotient;

    if (USE_POLLING_MODE == 1) {
        kdrv_tdc_update();
    }

    int_temp = kdrv_tdc_get_temp_fixed_point();
    
#if ((defined TDC_HW_PROTECTION_DFS) && (TDC_HW_PROTECTION_DFS == 1)) 
    uint16_t npu_f = kmdw_dfs_algorithm_execution(int_temp);

    #if KMDW_TDC_DBG
    //critical_msg("Temp %d NPU %d\n", int_temp, kmdw_dfs_get_npu_frequency());
    #endif
#endif
    //critical_msg("Temp %d\n", int_temp);
    if( rgst_tdc_cb )
    {
        if (kmdw_tdc_intsts & KDRV_TDC_INTST_CH0_DONE) {
            #if KMDW_TDC_DBG
            dbg_msg("Detect TDC channel done Temperature[%d].\n", int_temp);
            #endif
        }
        if (kmdw_tdc_intsts & KDRV_TDC_INTST_THRD_CH0_UNDR) {
            critical_msg("Detect TDC under Threshold[%d].\n", int_temp);
        }
        else if (kmdw_tdc_intsts & KDRV_TDC_INTST_THRD_CH0_OVR) {
            critical_msg("Detect TDC over Threshold[%d].\n", int_temp);
        }
        kmdw_tdc_intsts = 0;
    }

#if ((defined TDC_HW_PROTECTION) && (TDC_HW_PROTECTION == 1))
        kmdw_tdc_temperature_protection();

    if (int_temp > TDC_DEGREE_CELSIUS_DANGEROUS) {
       // critical_msg("[WARNING] Detect TDC Temperature[%d] over Threshold[%d].\n", int_temp, TDC_DEGREE_CELSIUS_DANGEROUS);
    }
#endif
    if( curr_print_temp != int_temp ) {// Avoid repeatedly print the same temperature
        // Print temp for 5 multiple degree celcuis more or less
        temp_quotient = int_temp;
        temp_quotient -= curr_print_temp;
        if(ABS(temp_quotient) >= 5) {
            curr_print_temp = int_temp;
            kmdw_printf("Current TDC Temperature is about [%d]\n", int_temp);
        }
        else {
            return;
        }
    }

    //critical_msg("polling_temp:%d curr_print_temp [%d] int_temp [%d] \n", polling_temp, curr_print_temp, int_temp);
}

void kmdw_tdc_monitor_thread(void *argument)
{
    uint32_t timeout = PERIOD_MONITOR;
    uint32_t curr_ticks, elapsed_ticks;

#if (!(defined TDC_HW_PROTECTION_DFS) || (TDC_HW_PROTECTION_DFS == 0))    
    kdrv_tdc_initialize(USE_POLLING_MODE);
    kdrv_tdc_set_thrd(TDC_DEGREE_CELSIUS_SAFE, TDC_DEGREE_CELSIUS_DANGEROUS);
    //rgst_tdc_cb = kdrv_tdc_register_callback(kmdw_tdc_callback, NULL);
    //kdrv_tdc_set_thrd_enflag(1, 1);
    //kdrv_tdc_set_thrd_int_enable(1,1);
    kdrv_tdc_enable(TDC_SCAN_CONTINUE, TDC_AVG_4);
#else
    if(kmdw_dfs_initialize(TDC_DEGREE_CELSIUS_SAFE_TARGET) != KMDW_STATUS_OK){
        critical_msg("kdev_dfs_initialize fail, not in valid range 90~120\n");
    }
#endif

#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))    
	#if ((defined TDC_HW_PROTECTION) && (TDC_HW_PROTECTION == 1) && (defined NPU_MHZ))
     curr_npu_clock = (npu_clk_setting)NPU_MHZ;
    #endif
#endif

    if (USE_POLLING_MODE == 1)
        timeout = TDC_UPDATE_PERIOD_MS;

    tdc_thread_id = osThreadGetId();
    if (tdc_thread_id == NULL)
        err_msg("[******** TDC ERROR ********] TDC thread not launched\n");

    for(;;) {
        osThreadFlagsWait(KMDW_TDC_FLAG_SET, osFlagsWaitAny, timeout);

        curr_ticks = osKernelGetTickCount();
        elapsed_ticks = curr_ticks - poll_time_external;

        if (elapsed_ticks >= (TDC_UPDATE_PERIOD_MS * 10)) {
            // Add internal polling when external one has stopped
            kmdw_tdc_update_internal();
        }
    }
}
#endif

int32_t kmdw_tdc_get_temperature(void)
{
#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))
    return int_temp;
#else
    return 0;
#endif
}

bool kmdw_tdc_is_ncpu_overheating(void)
{
#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))
    #if ((defined TDC_SW_PROTECTION) && (TDC_SW_PROTECTION == 1))
    return (int_temp > TDC_DEGREE_CELSIUS_DANGEROUS);
    #elif ((defined TDC_HW_PROTECTION_DFS) && (TDC_HW_PROTECTION_DFS == 1))
    return kmdw_dfs_is_overheating();
    #else
    return false;
    #endif
#else
    return false;
#endif
}

bool kmdw_tdc_sw_protection_en(void)
{
#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))
    #if ((defined TDC_SW_PROTECTION) && (TDC_SW_PROTECTION == 1))
        return true;
    #else
        return false;
    #endif    
#else
    return false;
#endif
}

void kmdw_tdc_update(void)
{
#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))
    poll_time_external = osKernelGetTickCount();
    kmdw_tdc_update_internal();
#endif
}

void kmdw_tdc_start(void)
{
/*
#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))    
    kdrv_tdc_initialize(USE_POLLING_MODE);
    //rgst_tdc_cb = kdrv_tdc_register_callback(kmdw_tdc_callback, NULL);
    kdrv_tdc_set_thrd(TDC_DEGREE_CELSIUS_SAFE, TDC_DEGREE_CELSIUS_DANGEROUS);
    kdrv_tdc_set_thrd_enflag(1, 1);
    kdrv_tdc_enable(TDC_SCAN_CONTINUE, TDC_AVG_4);

    osThreadAttr_t thread_attr;
    memset(&thread_attr, 0, sizeof(thread_attr));
    thread_attr.stack_size = 1024;
    thread_attr.priority = osPriorityNormal;
    tdc_thread_id = osThreadNew(kmdw_tdc_monitor_thread, NULL, &thread_attr);
    if (tdc_thread_id == NULL)
        err_msg("[******** TDC ERROR ********] TDC thread not launched\n");

	#if ((defined TDC_HW_PROTECTION) && (TDC_HW_PROTECTION == 1) && (defined NPU_MHZ))
    switch(NPU_MHZ) {
        case 700:
            curr_npu_clock = NPU_700_CFG1;
            break;
        case 600:
            curr_npu_clock = NPU_600_CFG1;
            break;
        case 500:
            curr_npu_clock = NPU_500_CFG1;
            break;
        case 400:
            curr_npu_clock = NPU_400_CFG1;
            break;
        case 350:
            curr_npu_clock = NPU_350_CFG1;
            break;
        case 300:
            curr_npu_clock = NPU_300_CFG1;
            break;
        case 200:
            curr_npu_clock = NPU_200_CFG1;
            break;
        default:
            curr_npu_clock = NPU_700_CFG1;
            break;
    }
    #endif
#endif
*/
}

