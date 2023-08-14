#pragma once

#include <stddef.h>
#include "khost_ll.h"
#include "model_result.h"
#include "model_type.h"

/* ####################
 * ##    Base API    ##
 * #################### */

int khost_init(void);
void khost_deinit(void);

/* ###########################
 * ##    Application API    ##
 * ########################### */

int khost_send_img(void *img, size_t img_len);

int khost_recv_yolo_resp(struct bounding_box_s **boxes, size_t boxes_len);
