#ifndef __KL520_INCLUDE_H__
#define __KL520_INCLUDE_H__

#include "board.h"
#include "base.h"
//#include "framework/event.h"
//#include "framework/v2k_color.h"

/**
 * @defgroup Camera_setting
 *
 * @{
 */
#define CAMERA_RGB_IDX              (MIPI_CAM_RGB)
#define CAMERA_NIR_IDX              (MIPI_CAM_NIR)
#ifdef KDP_UVC
#define CAMERA_YCBCR422_IDX         (MIPI_CAM_RGB)
#endif
#define CAMERA_DEVICE_RGB_IDX       (MIPI_CAM_RGB)
#define CAMERA_DEVICE_NIR_IDX       (MIPI_CAM_NIR)
#define CAMERA_RGB_IMG_WIDTH        (640)
#define CAMERA_RGB_IMG_HEIGHT       (480)
#define CAMERA_NIR_IMG_WIDTH        (480)
#define CAMERA_NIR_IMG_HEIGHT       (640)
/** @} */


/**
 * @defgroup Display_setting
 *
 * @{
 */
#define LCD_WIDTH                   LCDC_WIDTH
#define LCD_HEIGHT                  LCDC_HEIGHT


/** @} */



#define LCM_DISPLAY_HINT_BOX_MARGIN         (20)
#define LCM_DISPLAY_HINT_BOX_LEN            (50)
#define LCM_DISPLAY_HINT_BOX_PEN_WIDTH      (2)
#define LCM_DISPLAY_HINT_BOX_PEN_WIDTH_ADD_FACES    (2)
#define LCM_DISPLAY_HINT_BOX_COLOR          (GRAY)

#define LCM_DISPLAY_FD_BOX_MARGIN         	(20)
#define LCM_DISPLAY_FD_BOX_LEN        		(50)
#define LCM_DISPLAY_FD_BOX_PEN_WIDTH   		(2)
#define LCM_DISPLAY_FD_BOX_COLOR       		(YELLOW)
#define LCM_DISPLAY_FD_NO_FACE_BOX_COLOR    (RED)

#define LCM_DISPLAY_DB_BOX_MARGIN      		(20)
#define LCM_DISPLAY_DB_BOX_LEN          	(50)
#define LCM_DISPLAY_DB_BOX_PEN_WIDTH      	(2)
#define LCM_DISPLAY_DB_BOX_COLOR          	(GREEN)
#define LCM_DISPLAY_DB_FAIL_BOX_COLOR       (YELLOW)

#define IMG_PIX_FMT_YCBCR           (V2K_PIX_FMT_YCBCR)
#define IMG_PIX_FMT_RGB565          (V2K_PIX_FMT_RGB565)
#define IMG_PIX_FMT_RAW10           (V2K_PIX_FMT_RAW10)
#define IMG_PIX_FMT_RAW8            (V2K_PIX_FMT_RAW8)

#define FLAGS_FDFR_ACK              (BIT0)
#define FLAGS_FDFR_NACK             (BIT1)
#define FLAGS_CAMERA_ACK            (BIT2)

#define LCD_DISPLAY_X_OFFSET        (200)
#define LCD_DISPLAY_Y_OFFSET        (80)

/* Copied for e2e */

#define NIR_X_OFFSET                        (100)//100)
#define NIR_Y_OFFSET                        (70)//160)

#define KL520_APP_FLAG_FDFR_OK              (1<<0)
#define KL520_APP_FLAG_FDFR_ERR             (1<<1)
#define KL520_APP_FLAG_FDFR_TIMEOUT         (1<<2)
#define KL520_APP_FLAG_FDFR                 (KL520_APP_FLAG_FDFR_OK | KL520_APP_FLAG_FDFR_TIMEOUT | KL520_APP_FLAG_FDFR_ERR)
#define KL520_APP_FLAG_TOUCH                (1<<3)
#define KL520_APP_FLAG_TOUCH_DEINIT         (1<<4)
#define KL520_APP_FLAG_TOUCH_ALL            (KL520_APP_FLAG_TOUCH | KL520_APP_FLAG_TOUCH_DEINIT)
#define KL520_APP_FLAG_ACTION               (1<<5)
// #define KL520_APP_FLAG_ACTION_TOUCH         (1<<6)
#define KL520_DEVICE_FLAG_OK                (1<<7)
#define KL520_DEVICE_FLAG_ERR               (1<<8)
#define KL520_APP_FLAG_ALL 					(0X000FFFFF)

#define KL520_FACE_SCORE_MIN                (-1)
#define KL520_FACE_SCORE_MAX                (48)
/* Possible return value of kl520_api_face_get_result() */
#define KL520_FACE_OK                       (0)
#define KL520_FACE_DB_OK                    (1)
#define KL520_FACE_FAIL                     (2)
#define KL520_FACE_DB_FAIL                  (3)
#define KL520_FACE_NOFACE                   (4)
#define KL520_FACE_TIMEOUT                  (5)
#define KL520_FACE_EXIST                    (6)
#define KL520_FACE_EMPTY                    (7)
#define KL520_FACE_FULL                     (8)
#define KL520_FACE_BADPOSE                  (9)
#define KL520_FACE_TOO_FAR                  (10)
#define KL520_FACE_TOO_NEAR                 (11)
#define KL520_FACE_MOVED                    (12)
#define KL520_FACE_INVALID                  (13)

#define KL520_THRESH_NORMAL_MAX_UD          (15)
#define KL520_THRESH_NORMAL_MAX_LR          (14)//8)
#define KL520_THRESH_LEFT_MIN               (55)
#define KL520_THRESH_LEFT_MAX               (80)
#define KL520_THRESH_RIGHT_MIN              (55)
#define KL520_THRESH_RIGHT_MAX              (80)
#ifdef HEAD_POSE_CHECK_PERCENT
#define KL520_THRESH_UP_MIN                 (0)
#define KL520_THRESH_UP_MAX                 (100)
#define KL520_THRESH_DOWN_MIN               (4)
#define KL520_THRESH_DOWN_MAX               (100)
#else
#define KL520_THRESH_UP_MIN                 (50)
#define KL520_THRESH_UP_MAX                 (65)
#define KL520_THRESH_DOWN_MIN               (50)
#define KL520_THRESH_DOWN_MAX               (65)
#endif

#define KL520_MIDDLE_LINE					(70)
#define KL520_MIDDLE_LINE1					(20)
#define KL520_HINT_ARROW_LEN				(20)
#define KL520_HINT_ARROW					(10)
#define LCD_DISPLAY_ARROW_COLOR          (0xFEA0)   //GOLD


#define KL520_DEVICE_OK                     (0x00000000)
#define KL520_DEVICE_ERR_NIR_ID             (0x10000001)
#define KL520_DEVICE_ERR_RGB_ID             (0x20000001)
#define KL520_DEVICE_ERR_FLASH_ID           (0x30000001)
#define KL520_DEVICE_ERR_TOUCH_ID           (0x40000001)
#define KL520_DEVICE_ERR_LCM_ID             (0x50000001)
#define KL520_DEVICE_ERR_IOEXTENDER_ID      (0x60000001)

#define KL520_DB_USER_ID_DEFAULT_VALUE      (0x80)

typedef enum _KL520_FACE_ADD_MODE_
{
    FACE_ADD_MODE_1_FACE = 0,
    FACE_ADD_MODE_5_FACES
    
} kl520_face_add_mode;

typedef enum _KL520_FACE_ADD_TYPE_
{
    FACE_ADD_TYPE_NORMAL = 0,
    FACE_ADD_TYPE_LEFT,
    FACE_ADD_TYPE_RIGHT,
    FACE_ADD_TYPE_UP,
    FACE_ADD_TYPE_DOWN,
    
} kl520_face_add_type;

typedef enum _KL520_FACE_MODE_
{
    FACE_MODE_NONE = 0,
    FACE_MODE_ADD,
    FACE_MODE_RECOGNITION,
    FACE_MODE_LIVENESS,
    FACE_MODE_RECOGNITION_TEST,
    FACE_MODE_BUSY
} kl520_face_mode;

typedef enum _KL520_MOUSE_STATE_
{
    MOUSE_NONE,
    MOUSE_DOWN,
    MOUSE_MOVE,
    MOUSE_UP,

} kl520_mouse_state;

typedef struct _KL520_MOUSE_INFO_
{
    kl520_mouse_state state;
    short x;
    short y;

} kl520_mouse_info;

typedef struct _KL520_DP_POINT_
{
    u32 x;
    u32 y;
} kl520_dp_point;

typedef struct _KL520_DP_RECT_
{
    unsigned short start_x;
    unsigned short start_y;
    unsigned short end_x;
    unsigned short end_y;
} kl520_dp_rect;

typedef struct _KL520_DP_DRAW_INFO_ 
{
    kl520_dp_point pt[5];
    kl520_dp_rect rc;
    kl520_dp_rect nir_rc;
} kl520_dp_draw_info;

#endif
