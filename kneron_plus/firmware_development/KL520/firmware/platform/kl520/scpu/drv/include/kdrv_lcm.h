/**
  * @file        kdrv_lcm.h
  * @brief       Kneron LCM driver
  * @version     1.0
  * @copyright   (c) 2020 Kneron Inc. All right reserved.
  */
#ifndef __KDRV_LCM_H__
#define __KDRV_LCM_H__

#include "io.h"
#include "regbase.h"
#include "kdrv_display.h"

/**
 * @brief       Get the lightness value of backlight
 *
 * @return      u8  lightness value
 *
 */
u8 kdrv_lcm_get_backlight(void);

/**
 * @brief       Update frame data of RGB camerea on display
 *
 * @param[in]   *display_drv    @ref kdrv_display_t
 * @param[in]   addr            Address of frame buffer
 * @return      kdrv_status_t   @ref kdrv_status_t
 *
 */
kdrv_status_t kdrv_lcm_pressing(kdrv_display_t *display_drv, u32 addr);

/**
 * @brief       Update image data of NIR camera n display
 *
 * @param[in]   *display_drv    @ref kdrv_display_t
 * @param[in]   addr            Address of frame buffer
 * @return      kdrv_status_t   @ref kdrv_status_t
 *
 */
kdrv_status_t kdrv_lcm_pressingnir(kdrv_display_t *display_drv, u32 addr);

/**
 * @brief       Write command to LCM
 *
 * @param[in]   base            base address
 * @param[in]   data            data to write
 * @return      kdrv_status_t   @ref kdrv_status_t
 *
 */   
kdrv_status_t kdrv_lcm_write_cmd(uint32_t base, unsigned char data);

/**
* @brief       Write data to LCM
*
* @param[in]   base            base address
* @param[in]   data            data to write
* @return      kdrv_status_t   @ref kdrv_status_t
*
*/
kdrv_status_t kdrv_lcm_write_data(uint32_t base, unsigned char data);

/**
* @brief       Read data from LCM
*
* @param[in]   base            base address
* @return      unsigned int    data
*
*/
unsigned int kdrv_lcm_read_data(uint32_t base);

/**
* @brief       Get address of frame buffer which was showed on display
*
* @return      uint32_t   Address of frame buffer
*
*/
uint32_t kdrv_lcm_get_db_frame(void);
#endif

