#ifndef __BOARD_H__
#define __BOARD_H__

/* Common definitions for all boards with KL520 */

#define YES                             1
#define NO                              0

//Protocol
#define COMM_SUPPORT_I2C                0
#define COMM_SUPPORT_SPI                1
#define COMM_SUPPORT_UART               2
#define COMM_SUPPORT_I2S                3

#define COMM_PORT_ID_0                  0
#define COMM_PORT_ID_1                  1
#define COMM_PORT_ID_2                  2
#define COMM_PORT_ID_3                  3

#define COMM_I2CSPEED_100K              0
#define COMM_I2CSPEED_200K              1
#define COMM_I2CSPEED_400K              2
#define COMM_I2CSPEED_1000K             3

#define COMM_I2CMODE_SLAVE              0
#define COMM_I2CMODE_MASTER             1


#define COMM_SPIMODE_MODE_0             0
#define COMM_SPIMODE_MODE_1             1
#define COMM_SPIMODE_MODE_2             2
#define COMM_SPIMODE_MODE_3             3

#define COMM_UART_BAUDRATE_1200         0
#define COMM_UART_BAUDRATE_2400         1
#define COMM_UART_BAUDRATE_4800         2
#define COMM_UART_BAUDRATE_9600         3
#define COMM_UART_BAUDRATE_14400        4
#define COMM_UART_BAUDRATE_19200        5
#define COMM_UART_BAUDRATE_38400        6
#define COMM_UART_BAUDRATE_57600        7
#define COMM_UART_BAUDRATE_115200       8
#define COMM_UART_BAUDRATE_460800       9
#define COMM_UART_BAUDRATE_921600       10

//flash
//flash manufacturer
#define FLASH_TYPE_NULL                 0x00     /**< No flash */
#define FLASH_TYPE_WINBOND_NOR          0X01
#define FLASH_TYPE_WINBOND_NAND         0X02
#define FLASH_TYPE_MXIC_NOR             0X11
#define FLASH_TYPE_MXIC_NAND            0X12
#define FLASH_TYPE_GIGADEVICE_NOR       0X21
#define FLASH_TYPE_GIGADEVICE_NAND      0X22

//flash SIZE
#define FLASH_SIZE_64MBIT               1   //8MBYTES
#define FLASH_SIZE_128MBIT              2   //16MBYTES
#define FLASH_SIZE_256MBIT              3   //32MBYTES

//speed(25MHZ/50MHZ/100MHZ)
#define FLASH_COMM_SPEED_25MHZ          1
#define FLASH_COMM_SPEED_50MHZ          2
#define FLASH_COMM_SPEED_100MHZ         3

//IO pin DRIVING STRENGTH
#define FLASH_DRV_NORMAL_MODE           1
#define FLASH_DRV_DUAL_IO_MODE          2
#define FLASH_DRV_DUAL_OUTPUT_MODE      3
#define FLASH_DRV_QUAD_IO_MODE          4
#define FLASH_DRV_QUAD_OUTPUT_MODE      5


#define SENSOR_ID_HMX2056           0
#define SENSOR_ID_OV9286            1
#define SENSOR_ID_HMXRICA           2
#define SENSOR_ID_GC2145            3
#define SENSOR_ID_SC132GS           4
#define SENSOR_ID_MAX               5
#define SENSOR_ID_EXTERN            0xFE
#define SENSOR_ID_NONE              0xFF

#define IMGSRC_IN_PORT_NONE         0
#define IMGSRC_IN_PORT_MIPI         1
#define IMGSRC_IN_PORT_DPI          2  //DVP port
#define IMGSRC_IN_PORT_UVC          3

#define SENSOR_RES_640_480          0
#define SENSOR_RES_480_640          1
#define SENSOR_RES_480_272          2
#define SENSOR_RES_272_480          3
/* align with 720, use SENSOR_RES_xxx instead of RES_XXX
#define RES_640_480                 0
#define RES_480_640                 1
#define RES_480_272                 2
#define RES_272_480                 3
*/
#define IMG_FORMAT_RGB565           0
#define IMG_FORMAT_RAW10            1
#define IMG_FORMAT_RAW8             2
#define IMG_FORMAT_YCBCR            3

#define IMG_TYPE_RGB                0
#define IMG_TYPE_IR                 1
/* align with 720, use IMG_XXX instead of IMAGE_XXX
#define IMAGE_FORMAT_RGB565         0
#define IMAGE_FORMAT_RAW10          1
#define IMAGE_FORMAT_RAW8           2
#define IMAGE_FORMAT_YCBCR          3

#define IMAGE_TYPE_RGB              0
#define IMAGE_TYPE_IR               1
*/
#define DISPLAY_DEVICE_LCDC         0
#define DISPLAY_DEVICE_LCM          1

#define PANEL_MZT_480X272           1
#define PANEL_ST7789_240X320        2

#define SENSOR_RES_RGB              SENSOR_RES_640_480
#define SENSOR_RES_NIR              SENSOR_RES_480_640
#define IMGSRC_FORMAT_RGB           IMG_FORMAT_RGB565
#define IMGSRC_FORMAT_NIR           IMG_FORMAT_RAW8

#define CFG_AI_3D_LIVENESS_IN_NONE  0
#define CFG_AI_3D_LIVENESS_IN_SCPU  1
#define CFG_AI_3D_LIVENESS_IN_NCPU  2

/* Specific definitions for each board */
#ifndef BOARD_DVP_EXAMPLE

/* original board_kl520_96.h*/
#define IMGSRC_IN_0             YES
#define IMGSRC_IN_1             YES

#if (IMGSRC_IN_0 == YES)
#define IMGSRC_IN_0_PORT        IMGSRC_IN_PORT_MIPI
#define IMGSRC_0_SENSORID       SENSOR_ID_GC2145
#define IMGSRC_0_FORMAT         IMG_FORMAT_RGB565
#define IMGSRC_0_TYPE           IMG_TYPE_RGB
#define IMGSRC_0_RES            SENSOR_RES_640_480
#define IMGSRC_0_WIDTH          640
#define IMGSRC_0_HEIGHT         480
#define IMGSRC_0_TILE_AVG       0
#define IMGSRC_0_MIPI_LANE      2
#else
#define IMGSRC_IN_0_PORT        IMGSRC_IN_PORT_NONE
#define IMGSRC_0_SENSORID       SENSOR_ID_NONE
#define IMGSRC_0_FORMAT         IMG_FORMAT_RGB565
#define IMGSRC_0_TYPE           IMG_TYPE_RGB
#define IMGSRC_0_RES            SENSOR_RES_640_480
#define IMGSRC_0_WIDTH          640
#define IMGSRC_0_HEIGHT         480
#define IMGSRC_0_TILE_AVG       0
#define IMGSRC_0_MIPI_LANE      2
#endif

#if (IMGSRC_IN_1 == YES)
#define IMGSRC_IN_1_PORT        IMGSRC_IN_PORT_MIPI
#define IMGSRC_1_SENSORID       SENSOR_ID_SC132GS
#define IMGSRC_1_FORMAT         IMG_FORMAT_RAW8
#define IMGSRC_1_TYPE           IMG_TYPE_IR
#define IMGSRC_1_RES            SENSOR_RES_480_640
#define IMGSRC_1_WIDTH          480
#define IMGSRC_1_HEIGHT         640
#define IMGSRC_1_TILE_AVG       1
#define IMGSRC_1_MIPI_LANE      2
#else
#define IMGSRC_IN_1_PORT        IMGSRC_IN_PORT_NONE
#define IMGSRC_1_SENSORID       SENSOR_ID_NONE
#define IMGSRC_1_FORMAT         IMG_FORMAT_RAW8
#define IMGSRC_1_TYPE           IMG_TYPE_IR
#define IMGSRC_1_RES            SENSOR_RES_480_640
#define IMGSRC_1_WIDTH          480
#define IMGSRC_1_HEIGHT         640
#define IMGSRC_1_TILE_AVG       1
#define IMGSRC_1_MIPI_LANE      2
#endif

#if (IMGSRC_IN_0_PORT == IMGSRC_IN_PORT_MIPI || IMGSRC_IN_1_PORT == IMGSRC_IN_PORT_MIPI)
#define IMGSRC_IN_HAS_MIPI
#define MIPI_LANE_RGB           2
#define MIPI_LANE_NIR           2
#endif

#if (IMGSRC_IN_0_PORT ==  IMGSRC_IN_PORT_DPI || IMGSRC_IN_1_PORT ==  IMGSRC_IN_PORT_DPI)
#define IMGSRC_IN_HAS_DPI
#if (IMGSRC_IN_0_PORT ==  IMGSRC_IN_PORT_DPI)
#define IMAGE_DVP_PORT_NO       0
#else
#define IMAGE_DVP_PORT_NO       1
#endif
#endif

#if (IMGSRC_IN_0_PORT ==  IMGSRC_IN_PORT_UVC || IMGSRC_IN_1_PORT == IMGSRC_IN_PORT_UVC)
#define IMGSRC_IN_HAS_UVC
#endif

#if (IMGSRC_IN_0 && IMGSRC_IN_1 && IMGSRC_IN_2)
#define CAM_ID_MAX              3
#elif (IMGSRC_IN_0 && IMGSRC_IN_1)
#define CAM_ID_MAX              2
#elif (IMGSRC_IN_0 || IMGSRC_IN_1)
#define CAM_ID_MAX              1
#else
#define CAM_ID_MAX              0
#endif

#define IMGSRC_NUM              CAM_ID_MAX
#define MIPI_CAM_RGB            0
#define MIPI_CAM_NIR            1

#define LCDC_WIDTH              640
#define LCDC_HEIGHT             480

#define RGB_IMG_SOURCE_W        IMGSRC_0_WIDTH
#define RGB_IMG_SOURCE_H        IMGSRC_0_HEIGHT
#define NIR_IMG_SOURCE_W        IMGSRC_1_WIDTH
#define NIR_IMG_SOURCE_H        IMGSRC_1_HEIGHT

#define PANEL_TYPE              PANEL_MZT_480X272
#define DISPLAY_DEVICE          DISPLAY_DEVICE_LCDC

#define CAM_CLK_MS      2
#define CAM_CLK_NS      242
#define CAM_CLK_PS      2
#define CSI0_TXESC      4
#define CSI0_CSI        11
#define CSI0_VC0        5
#define CSI1_TXESC      4
#define CSI1_CSI        7
#define CSI1_VC0        1

/* original board_cfg_96.h*/
#define CFG_V2K_TYPE                                  8
#define CFG_PANEL_TYPE                                1
#define CFG_DISPLAY_DMA_ENABLE                        0
#define CFG_PREFER_DISPLAY                            1
#define CFG_I2C_0_ENABLE                              1
#define CFG_I2C_1_ENABLE                              0
#define CFG_I2C_2_ENABLE                              0
#define CFG_I2C_3_ENABLE                              0
#define CFG_UART0_ENABLE                              1
#define CFG_UART1_ENABLE                              0
#define CFG_UART1_TX_DMA_ENABLE                       0
#define CFG_UART1_RX_DMA_ENABLE                       0
#define CFG_UART2_ENABLE                              0
#define CFG_UART2_TX_DMA_ENABLE                       0
#define CFG_UART2_RX_DMA_ENABLE                       0
#define CFG_UART3_ENABLE                              0
#define CFG_UART3_TX_DMA_ENABLE                       0
#define CFG_UART3_RX_DMA_ENABLE                       0
#define CFG_UART4_ENABLE                              0
#define CFG_UART4_TX_DMA_ENABLE                       0
#define CFG_UART4_RX_DMA_ENABLE                       0
#define CFG_ADC0_ENABLE                               0
#define CFG_ADC0_DMA_ENABLE                           0
#define CFG_ADC1_ENABLE                               0
#define CFG_ADC1_DMA_ENABLE                           0
#define CFG_ADC2_ENABLE                               0
#define CFG_ADC2_DMA_ENABLE                           0
#define CFG_ADC3_ENABLE                               0
#define CFG_ADC3_DMA_ENABLE                           0
#define CFG_PWM1_DMA_ENABLE                           0
#define CFG_PWM2_DMA_ENABLE                           0
#define CFG_PWM3_DMA_ENABLE                           0
#define CFG_PWM4_DMA_ENABLE                           0
#define CFG_PWM5_DMA_ENABLE                           0
#define CFG_PWM6_DMA_ENABLE                           0
#define CFG_SSP0_ENABLE                               0
#define CFG_SSP0_TX_DMA_ENABLE                        0
#define CFG_SSP0_RX_DMA_ENABLE                        0
#define CFG_SSP1_ENABLE                               1
#define CFG_SSP1_TX_DMA_ENABLE                        1
#define CFG_SSP1_RX_DMA_ENABLE                        1
#define CFG_SPI_ENABLE                                1
#define CFG_SPI_DMA_ENABLE                            0
#define CFG_SD_ENABLE                                 1
#define CFG_SD_DMA_ENABLE                             0
#define CFG_USBD_ENABLE                               0
#define CFG_USBH_ENABLE                               0
#define CFG_AI_USE_FIXED_IMG                          0
#define CFG_AI_3D_ENABLE                              1
#define CFG_AI_3D_LIVENESS_IN                         2
#define CFG_UI_USR_IMG                                1
#define CFG_CONSOLE_MODE                              1
#define CFG_TOUCH_ENABLE                              1
#define CFG_USE_FRAME_BUFFER_DRIVER                   1
#define CFG_SNAPSHOT_ENABLE                           1
#define CFG_SNAPSHOT_NUMS                            10
#define CFG_DFU_FLASH_BUF_ENABLE                      1
#define CFG_DFU_IMAGE_BUF_ENABLE                      1
#define CFG_KDP_SETTINGS_ENABLE                       1
#define CFG_KDP_SETTINGS_SIZE                      8192
#define CFG_USR_SETTINGS_ENABLE                       1
#define CFG_USR_SETTINGS_SIZE                      4096

#else
#include "board_kl520_dvp_example.h"
#endif

/* original board_uvc.h*/
#define YCBCR422_IMG_SOURCE_W            640
#define YCBCR422_IMG_SOURCE_H            480

// #ifdef BOARD_96
// #include "board_kl520_96.h"
// #include "board_cfg_96.h"
// #elif BOARD_DVP_EXAMPLE
// #include "board_kl520_dvp_example.h"
// #else
// #include "board_kl520_evb.h"
// #endif
// #ifdef KDP_UVC
// #include "board_uvc.h"
// #endif
#endif // __BOARD_KDP520_H__
