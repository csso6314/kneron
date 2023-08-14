#include "kmdw_dfs.h"
#include "kdrv_clock.h"
#include "kdrv_tdc.h"
#include "project.h"   /*for NPU_MHZ */
#include "kmdw_console.h"

extern uint8_t dfs_fuzzy_init(int16_t);
extern uint16_t dfs_fuzzy_execution(int16_t, uint16_t);

static uint16_t npu_freq_array[] = {200, 250, 300, 350, 400, 500, 600, 650, 700};

typedef struct{
    uint8_t running;
    uint8_t update;
    uint8_t current_freq_index;
    uint16_t current_freq;
    uint16_t computed_freq;
    int16_t current_temp;
    int16_t target_temp;
    uint8_t (*algorithm_init)(int16_t in_temp);
    uint16_t (*algorithm_exe)(int16_t in_temp, uint16_t int_freq);
}kmdw_dfs;

static kmdw_dfs kmdw_dfs_handle = {
    .running = 0,
    .update = 0,
    .current_freq_index = NPU_MHZ,
    .current_freq = 0,
    .computed_freq = 0,
    .current_temp = 0,
    .target_temp = 0,
    .algorithm_init = dfs_fuzzy_init,
    .algorithm_exe = dfs_fuzzy_execution
};

static void kmdw_dfs_tdc_callback(void *arg)
{
    uint32_t status = 0;
    status = kdrv_tdc_int_status_read();
    if(status & KDRV_TDC_INTST_THRD_CH0_OVR){
        //critical_msg("KDRV_TDC_INTST_THRD_CH0_OVR\n");
        kmdw_dfs_handle.running = 1;
        kdrv_tdc_set_thrd(kmdw_dfs_handle.target_temp - 10 - 5, kmdw_dfs_handle.target_temp - 10);
        kdrv_tdc_set_thrd_enflag(1,0);
        kdrv_tdc_set_thrd_int_enable(1,0);
    }
    else if(status & KDRV_TDC_INTST_THRD_CH0_UNDR){
        //critical_msg("KDRV_TDC_INTST_THRD_CH0_UNDR\n");
        if(kmdw_dfs_handle.current_freq == 700){
            kmdw_dfs_handle.running = 0;
            kmdw_dfs_handle.update = 0;
        }
        kdrv_tdc_set_thrd(kmdw_dfs_handle.target_temp - 10 - 5, kmdw_dfs_handle.target_temp - 10);
        kdrv_tdc_set_thrd_enflag(0,1);
        kdrv_tdc_set_thrd_int_enable(0,1);
    }
}
/*
uint8_t get_closest(uint16_t *arr, uint8_t val1, uint8_t val2, uint16_t target){
    if (target - arr[val1] >= arr[val2] - target)
        return val2;
    else
        return val1;
}

static uint8_t kmdw_dfs_get_available_npu_freq_index(uint16_t freq){
    uint16_t arr_length = sizeof(npu_freq_array) >> 1;
    uint16_t *arr = npu_freq_array;
    if(freq >= arr[arr_length - 1]){
        return arr_length - 1;
    }
    if(freq <= arr[0]){
        return 0;
    }
    int i = 0, j = arr_length, mid = 0;
    while (i < j) {
        mid = (i + j) / 2;
        if (arr[mid] == freq)
            return mid;
        if (freq < arr[mid]) {
            if (mid > 0 && freq > arr[mid - 1])
                return get_closest(arr, mid-1, mid, freq);
            j = mid;
        }
        else {
            if (mid < arr_length - 1 && freq < arr[mid + 1])
                return get_closest(arr, mid, mid+1, freq);
            i = mid + 1;
        }
    }
    return mid;
}
*/
kmdw_status_t kmdw_dfs_initialize(int16_t target_temp){
    uint8_t status = kmdw_dfs_handle.algorithm_init(target_temp);
    if(status != 0){
        return KMDW_STATUS_ERROR;
    }
    kmdw_dfs_handle.current_freq = npu_freq_array[kmdw_dfs_handle.current_freq_index];
    kmdw_dfs_handle.target_temp = target_temp;
    kdrv_tdc_initialize(1);
    kdrv_tdc_register_callback(kmdw_dfs_tdc_callback, NULL);
    kdrv_tdc_set_thrd(target_temp - 10 - 5, target_temp - 10);
    kdrv_tdc_set_thrd_enflag(1, 1);
    kdrv_tdc_set_thrd_int_enable(0,1);
    kdrv_tdc_enable(TDC_SCAN_CONTINUE, TDC_AVG_1);
    return KMDW_STATUS_OK;
}

uint16_t kmdw_dfs_algorithm_execution(int16_t temperature){
    if(kmdw_dfs_handle.running){
        kmdw_dfs_handle.current_temp = temperature;
        kmdw_dfs_handle.computed_freq = kmdw_dfs_handle.algorithm_exe(temperature, kmdw_dfs_handle.current_freq);
        kmdw_dfs_handle.update = 1;
        return kmdw_dfs_handle.computed_freq;
    }
    return 0;
}

uint8_t kmdw_dfs_is_algorithm_running(void){
    return kmdw_dfs_handle.running;
}

void kmdw_dfs_set_npu_frequency(){
    if(!kmdw_dfs_handle.update || !kmdw_dfs_handle.running){
        return;
    }
    uint16_t freq = kmdw_dfs_handle.computed_freq;
    npu_clk_setting clock_sel = NPU_700_CFG1;
    if(freq >= 675){
        freq = 700;
        clock_sel = NPU_700_CFG1;
    }
    else if(freq < 675 && freq >= 625){
        freq = 650;
        clock_sel = NPU_650_CFG1;
    }
    else if(freq < 625 && freq >= 550){
        freq = 600;
        clock_sel = NPU_600_CFG1;
    }
    else if(freq < 550 && freq >= 450){
        freq = 500;
        clock_sel = NPU_500_CFG1;
    }
    else if(freq < 450 && freq >= 375){
        freq = 400;
        clock_sel = NPU_400_CFG1;
    }
    else if(freq < 375 && freq >= 325){
        freq = 350;
        clock_sel = NPU_350_CFG1;
    }
    else if(freq < 325 && freq >= 275){
        freq = 300;
        clock_sel = NPU_300_CFG1;
    }
    else if(freq < 275 && freq >= 225){
        freq = 250;
        clock_sel = NPU_250_CFG1;
    }
    else{
        freq = 200;
        clock_sel = NPU_200_CFG1;
    }
    if(freq != kmdw_dfs_handle.current_freq ){
        kdrv_clock_npu_clock_change(clock_sel);
        kmdw_dfs_handle.current_freq = freq;
        kmdw_dfs_handle.current_freq_index = clock_sel;
    }
    kmdw_dfs_handle.update = 0;
}

uint16_t kmdw_dfs_get_npu_frequency(void){
    return kmdw_dfs_handle.current_freq;
}

bool kmdw_dfs_is_overheating(void){
    if(kmdw_dfs_handle.current_freq == 200 && kmdw_dfs_handle.current_temp > kmdw_dfs_handle.target_temp + 5){
        return true;
    }
    return false;
}
