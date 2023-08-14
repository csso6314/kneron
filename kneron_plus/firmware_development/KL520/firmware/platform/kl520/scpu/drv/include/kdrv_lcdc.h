/**
  * @file        kdrv_lcdc.h
  * @brief       Kneron LCDC driver
  * @version     1.0
  * @copyright   (c) 2020 Kneron Inc. All right reserved.
  */
#ifndef __KDRV_LCDC_H__
#define __KDRV_LCDC_H__

#include "io.h"
#include "regbase.h"
#include "board.h"
#include <cmsis_os2.h>
#include "kdrv_display.h"

/**
 * @brief Definition of Display Draw Event
 */ 
#define FLAGS_KDP520_LCDC_START_DRAW_RECT_EVT    BIT0
#define FLAGS_KDP520_LCDC_STOP_DRAW_RECT_EVT     BIT1

/**
 * @brief Definition of maximum number of frame buffer
 */ 
#define MAX_FRAME_NUM                           (1)

/**
 * @brief Definition of bounding-box margin length
 */ 
#define LCDC_HINT_BOUNDINGBOX_MARGIN_LEN        (30)

/**
 * @brief Enumerations of lcdc screen control
 */
typedef enum
{
    KDRV_LCDC_SCREEN_OFF = 0,    /**< LCDC screen control off */
    KDRV_LCDC_SCREEN_ON          /**< LCDC screen control on  */
} kdrv_lcdc_screen_ctrl_t;

/**
 * @brief Enumerations of lcdc panel pixel parameter, image pixel format in FIFO
 */
typedef enum
{
    KDRV_LCDC_IMG_PIXFMT_1BPP = 0,  /**< 000: 1 bpp */
    KDRV_LCDC_IMG_PIXFMT_2BPP,      /**< 001: 2 bpp */
    KDRV_LCDC_IMG_PIXFMT_4BPP,      /**< 010: 4 bpp */
    KDRV_LCDC_IMG_PIXFMT_8BPP,      /**< 011: 8 bpp */
    KDRV_LCDC_IMG_PIXFMT_16BPP,     /**< 100: 16 bpp */
    KDRV_LCDC_IMG_PIXFMT_24BPP,     /**< 101: 24 bpp */
    KDRV_LCDC_IMG_PIXFMT_ARGB8888,  /**< 110: ARGB8888 */
    KDRV_LCDC_IMG_PIXFMT_ARGB1555   /**< 111: ARGB1555 */
} kdrv_lcdc_img_pixfmt_t;

/**
 * @brief Enumerations of lcdc panel pixel parameter, TFT panel color depth selection
 */
typedef enum
{
    KDRV_LCDC_6BIT_PER_CHANNEL = 0, /**< 6 bits per channel with a 18-bit panel interface */
    KDRV_LCDC_8BIT_PER_CHANNEL      /**< 8 bits per channel with a 24-bit panel interface */
} kdrv_lcdc_panel_type_t;

/**
 * @brief Enumerations of lcdc panel pixel parameter, output format selection
 */
typedef enum
{
    KDRV_LCDC_OUTPUT_FMT_RGB = 0,   /**< RGB normal output */
    KDRV_LCDC_OUTPUT_FMT_BGR        /**< BGR red and blue swapped output */
} kdrv_lcdc_output_fmt_t;

/**
 * @brief Enumerations of lcdc serial panel pixel parameter, shift rotate
 */
typedef enum
{
    KDRV_LCDC_SERIAL_PIX_RSR = 0, /**< Even line sequence from through the odd line rotating right */
    KDRV_LCDC_SERIAL_PIX_LSR      /**< Even line sequence from through the odd line rotating left */
} kdrv_lcdc_serial_pix_sr_t;

/**
 * @brief Enumerations of lcdc serial panel pixel parameter, color sequence of odd line
 */
typedef enum
{
    KDRV_LCDC_SERIAL_PIX_COLORSEQ_RGB = 0,  /**< RGB decides the sub-pixel sequence of the odd line */
    KDRV_LCDC_SERIAL_PIX_COLORSEQ_BRG,      /**< BRG decides the sub-pixel sequence of the odd line */
    KDRV_LCDC_SERIAL_PIX_COLORSEQ_GBR       /**< GBR decides the sub-pixel sequence of the odd line */       
} kdrv_lcdc_serial_pix_colorseq_t;

/**
 * @brief Enumerations of lcdc serial panel pixel parameter, delta type arrangement of color filter
 */
typedef enum
{
    KDRV_LCDC_SERIAL_PIX_DELTA_TYPE_SAME_SEQ = 0,   /**< Odd line and even line have the same data sequence */
    KDRV_LCDC_SERIAL_PIX_DELTA_TYPE_DIFF_SEQ        /**< Odd line and even line have the difference data sequence */       
} kdrv_lcdc_serial_pix_delta_type_t;

/**
 * @brief Enumerations of lcdc serial panel pixel parameter, RGB serial output mode
 */
typedef enum
{
    KDRV_LCDC_SERIAL_PIX_RGB_PARALLEL_OUTPUT = 0,   /**< RGB parallel format output */
    KDRV_LCDC_SERIAL_PIX_RGB_SERIAL_OUTPUT          /**< RGB serial format output */       
} kdrv_lcdc_serial_pix_output_mode_t;

/**
 * @brief Enumerations of lcdc image format parameter, endian control
 */
typedef enum
{
    KDRV_LCDC_FB_DATA_ENDIAN_LBLP = 0,  /**< 00: Little-endian byte little-endian pixel*/
    KDRV_LCDC_FB_DATA_ENDIAN_BBBP,      /**< 01: Big-endian byte big-endian pixel*/
    KDRV_LCDC_FB_DATA_ENDIAN_LBBP,      /**< 10: Little-endian byte big-endian pixel (WinCE)*/
} kdrv_lcdc_fb_data_endianness_t;

/**
 * @brief Enumerations of lcdc function enable parameter, test pattern generator
 */
typedef enum
{
    KDRV_LCDC_PAT_GEN_DISABLE = 0,      /**< Turn-off pattern generator*/
    KDRV_LCDC_PAT_GEN_ENABLE,           /**< Turn-on pattern generator*/
} kdrv_lcdc_pat_gen_t;

/**
 * @brief Enumerations of lcdc panel pixel parameter, AUO052 mode
 */
typedef enum
{
    KDRV_LCDC_AUO052_OFF = 0, /**< 0: Turn off the AUO052 mode */
    KDRV_LCDC_AUO052_ON       /**< 1: Turn on the AUO052 mode */
} kdrv_lcdc_auo052_mode_t;

/**
 * @brief       Control display screen ON/OFF
 *
 * @param[in]   ctrl            see @ref kdrv_lcdc_screen_ctrl_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_display_screen_control(kdrv_lcdc_screen_ctrl_t ctrl);

/**
 * @brief       Set TFT panel color depth selection of LCD serial panel pixel parameter
 *
 * @param[in]   type            see @ref kdrv_lcdc_panel_type_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_panel_type(kdrv_lcdc_panel_type_t type);

/**
 * @brief       Set output format selection of LCD serial panel pixel parameter
 *
 * @param[in]   type            see @ref kdrv_lcdc_output_fmt_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_bgrsw(kdrv_lcdc_output_fmt_t format);

/**
 * @brief       Set odd line shift rotate of LCD serial panel pixel parameter
 *
 * @param[in]   type            see @ref kdrv_lcdc_serial_pix_sr_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_pixel_sr(kdrv_lcdc_serial_pix_sr_t rotate);

/**
 * @brief       Set color sequence of odd line of LCD serial panel pixel parameter
 *
 * @param[in]   type            see @ref kdrv_lcdc_serial_pix_colorseq_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_pixel_colorseq(kdrv_lcdc_serial_pix_colorseq_t color);

/**
 * @brief       Set delta type arrangement of color filter of LCD serial panel pixel parameter
 *
 * @param[in]   type            see @ref kdrv_lcdc_serial_pix_delta_type_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_pixel_delta_type(kdrv_lcdc_serial_pix_delta_type_t type);

/**
 * @brief       Set RGB serial output mode of LCD serial panel pixel parameter
 *
 * @param[in]   mode            see @ref kdrv_lcdc_serial_pix_output_mode_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_pixel_serial_mode(kdrv_lcdc_serial_pix_output_mode_t mode);

/**
 * @brief       Set data endian
 *
 * @param[in]   mode            see @ref kdrv_lcdc_fb_data_endianness_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_endian(kdrv_lcdc_fb_data_endianness_t endian_type);

/**
 * @brief       Set data endian
 *
 * @param[in]   mode            see @ref kdrv_lcdc_auo052_mode_t
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_auo052_mode(kdrv_lcdc_auo052_mode_t mode);

/**
 * @brief       Set image down scale
 *
 * @param[in]   hor_no_in       Width of input image source
 * @param[in]   hor_no_out      Height of input image source
 * @param[in]   ver_no_in       Width of output image source
 * @param[in]   ver_no_out      Height of output image source
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_down_scale(uint16_t hor_no_in, uint16_t hor_no_out, uint16_t ver_no_in, uint16_t ver_no_out);

/**
 * @brief       Set frame rate of lcdc vsync
 *
 * @param[in]   framerate       Frame rate
 * @param[in]   framerate       Image width
 * @param[in]   framerate       Image height
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_framerate(int framerate, int width, int height);

/**
 * @brief       Set LCD color management parameter
 *
 * @param[in]   color0          [BIT13_BIT8] Saturation value.\n
 *                                           Cb(sat) = Cb(org) * (SatValue/32).\n
 *                                           Cr(sat) = Cr(org) * (SatValue/32).\n
 *                              [BIT7] Sign bit of brightness value.\n
 *                                     0: The value of brightness is positive.\n
 *                                     1: The value of brightness is negative.\n
 *                              [BIT6:BIT0] Brightness level\n
 *                                          The range of the brightness level is from 0 to 127.\n
 * @param[in]   color1          This register value defines the coefficient of the hur operation\n
 *                              [BIT14] Sign bit of HuCosValue.\n
 *                                      0: The value of HuCosValue is positive.\n
 *                                      1: The value of HuCosValue is negative.\n
 *                              [BIT13:BIT8] Hue value of coefficient Cos -180~180 degree.\n
 *                              [BIT6] Sigh bit of HuSinValue\n
 *                                      0: The value of HuSinValue is positive.\n
 *                                      1: The value of HuSinValue is negative.\n
 *                              [BIT5:BIT0] Hue value of coefficient Sin -180~180 degree.\n
 * @param[in]   color2          This register value defines the coefficient of the sharpness operation\n
 *                              [BIT23:BIT20] Sharpness weight value 1.\n
 *                                            The value determines the second weight of sharpness.\n
 *                              [BIT19:BIT16] Sharpness weight value 0.\n
 *                                            The value determines the first weight of sharpness.\n
 *                              [BIT15:BIT8]  Sharpness threshold value 1.\n
 *                                            The value determines the second threshold of sharpness.\n
 *                              [BIT7:BIT0]   Sharpness threshold value 0.\n
 *                                            The value determines the second threshold of sharpness.\n
 * @param[in]   color3          This register value defines the coefficient of the contast operation\n
 *                              [BIT20:BIT16] Contrast cure slope.\n
 *                                            The value determines the slope of contrast cure. The actual slope is the value devided by 4.\n
 *                                            Note: This value cannot be programmed to 0.\n
 *                              [BIT12]       Contrast offset sign\n
 *                                            1: (Contr_slope x 128) > 512.\n
 *                                            0: (Contr_slope x 128) < 512.\n
 *                              [BIT11:BIT0]  Contrast offset value.\n
 *                                            The value is defined as absolute of "Contr_slope x 128 - 512".\n
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_image_color_params(uint32_t color0, uint32_t color1, uint32_t color2, uint32_t color3);


/**
 * @brief       Set frame rate of lcdc vsync
 *
 * @param[in]   ctrl            [BIT9]          Enable the LOCK command\n
 *                                              0:Disable the LOCK command\n
 *                                              1:Enable the LOCK command\n
 *                              [BIT8]          Enable the bus bandwidth ratio\n
 *                                              0:Disable the bus bandwidth ratio\n
 *                                              1:Enable the bus bandwidth ratio\n
 *                              [BIT7:BIT6]     Bus bandwidth control ratio for the Image3 Frame buffer\n
 *                                              00:Ratio 1\n
 *                                              01:Ratio 2\n
 *                                              10:Ratio 4\n
 *                              [BIT5:BIT4]     Bus bandwidth control ratio for the Image2 Frame buffer\n
 *                                              00:Ratio 1\n
 *                                              01:Ratio 2\n
 *                                              10:Ratio 4\n
 *                              [BIT3:BIT2]     Bus bandwidth control ratio for the Image1 Frame buffer\n
 *                                              00:Ratio 1\n
 *                                              01:Ratio 2\n
 *                                              10:Ratio 4\n
 *                              [BIT1:BIT0]     Bus bandwidth control ratio for the Image0 Frame buffer\n
 *                                              00:Ratio 1\n
 *                                              01:Ratio 2\n
 *                                              10:Ratio 4\n
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_bus_bandwidth_ctrl(uint32_t ctrl);

/**
 * @brief       Set frame buffer parameter
 *
 * @param[in]   img_scal_down   [BIT15:BIT14] Scaling down for image3\n
 *                              The image from LCDImage3FrameBase can be scaled down depending on the value.\n
 *                              00: Disable\n
 *                              01:Image3 will be scaling down to 1/2 x 1/2\n
 *                              10:Image3 will be scaling down to 1/2 x 1\n
 *                              [BIT13:BIT12] Scaling down for image2\n
 *                              The image from LCDImage2FrameBase can be scaled down depending on the value.\n
 *                              00: Disable\n
 *                              01:Image2 will be scaling down to 1/2 x 1/2\n
 *                              10:Image2 will be scaling down to 1/2 x 1\n
 *                              [BIT11:BIT10] Scaling down for image1\n
 *                              The image from LCDImage1FrameBase can be scaled down depending on the value.\n
 *                              00: Disable\n
 *                              01:Image1 will be scaling down to 1/2 x 1/2\n
 *                              10:Image1 will be scaling down to 1/2 x 1\n
 *                              [BIT9:BIT8] Scaling down for image0\n
 *                              The image from LCDImage0FrameBase can be scaled down depending on the value.\n
 *                              00: Disable\n
 *                              01:Image0 will be scaling down to 1/2 x 1/2\n
 *                              10:Image0 will be scaling down to 1/2 x 1\n
 *
 * @note        Please note that these filed values can only be set to '00' under the folloing conditions:\n
 *              - VirtualScreenEn or LCM_En is set\n
 *              - PiP has a chance to be turned-on when TV is enabled.
 * @return      kdrv_status_t   see @ref kdrv_status_t
 */
kdrv_status_t kdrv_lcdc_set_frame_buffer(uint32_t img_scal_down);

uint32_t lcdc_kdp2_get_disp_idx(int *read_done_idx);
uint32_t lcdc_kdp2_set_disp_buf(uint32_t buf_addr, int write_done_idx);
uint32_t lcdc_kdp2_get_disp_buf(int cam_idx, int *disp_idx);

#endif
