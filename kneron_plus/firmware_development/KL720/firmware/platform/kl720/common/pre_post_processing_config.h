#pragma once

#include <stdint.h>

typedef struct
{
    // pre-proc config
    struct {
        uint32_t x;
        uint32_t y;
    } landmark[4];                  /**< license plate corner landmark */

    bool enable_hw_inproc;          /**< to enable hw inproc or run warp perspective transform instead */

    // post-proc config
    bool is_maxpool_in_model;       /**< is maxpool layer in ocr model */
    uint32_t ocr_rule_base_code;    /**< ocr rule base code for ocr correction (ref: enum OCR_RULE_BASE_CODE) */
    uint32_t max_ocr_number;        /**< max number of detected characters  */
    float ocr_score_threshold;      /**< score threshold for ocr character filter  */
    float ocr_iou_threshold;        /**< IoU threshold for ocr character bounding box filter  */
} ocr_pre_post_proc_config_t;
