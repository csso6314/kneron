/* USB Device Mode Communication middleware */

#ifndef __KMDW_USBD_COM_H__
#define __KMDW_USBD_COM_H__

#include "dual_fifo.h"

void usbd_com_init(dual_fifo_t image_dfifo, dual_fifo_t result_dfifo);

#endif
