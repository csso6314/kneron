/**
 * @file        kmdw_dfs.h
 * @brief       APIs for Dynamic Frequency Selection
 *
 * @copyright   Copyright (c) 2020 Kneron Inc. All rights reserved.
 */
#ifndef _KMDW_DFS_H_
#define _KMDW_DFS_H_

#include <stdint.h>
#include <stdbool.h>
#include "kmdw_status.h"

/**
* @brief       Initialize the DFS 
* @param[in]   target_temp   The target temperature in celsius of the DFS algorithm, valid range 90~120
* @return      @ref kmdw_status_t
*/
kmdw_status_t kmdw_dfs_initialize(int16_t target_temp);

/**
* @brief       Execute the DFS algorithm
* @param[in]   temperature   The current temperature
* @return      computed NPU frequency
*/
uint16_t kmdw_dfs_algorithm_execution(int16_t temperature);

/**
* @brief       Set the NPU frequency
*/
void kmdw_dfs_set_npu_frequency(void);

/**
* @brief       Get the NPU frequency
* @return      Current NPU frequency
*/
uint16_t kmdw_dfs_get_npu_frequency(void);

/**
* @brief       Get the DFS algorithm running state
* @return      running state
*/
uint8_t kmdw_dfs_is_algorithm_running(void);

/**
* @brief       Get the temperature overheating state. The overheating condition is defined to be temperature > target + 5 while npu frequency is the lowest
* @return      overheating state
*/
bool kmdw_dfs_is_overheating(void);


#endif // _KMDW_DFS_H_
