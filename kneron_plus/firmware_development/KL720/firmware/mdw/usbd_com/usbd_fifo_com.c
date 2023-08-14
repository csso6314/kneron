#include "cmsis_os2.h"

#include "kdrv_usbd3.h"

#include "usbd_com.h"
#include "kmdw_console.h"
#include "dual_fifo.h"

#define TFLAG_STM_START 0x100 // BIT(8)

static dual_fifo_t gImg_dfifo = 0;
static dual_fifo_t gResult_dfifo = 0;

static uint8_t g_enp_in = 0x0;
static uint8_t g_enp_out = 0x0;

static osThreadId_t usbd_queue_rx_tid = 0;
static osThreadId_t usbd_queue_tx_tid = 0;

static void data_recv_thread(void *arg)
{
    // the begin of STM, FIXME
    osThreadFlagsWait(TFLAG_STM_START, osFlagsWaitAny, osWaitForever);
    // then wake up result_send_thread
    osThreadFlagsSet(usbd_queue_tx_tid, TFLAG_STM_START);

    kmdw_printf("[data_rx] ready !\n");

    osStatus_t fifo_sts;
    kdrv_status_t usb_sts;
    uint32_t buf_addr;
    uint32_t buf_size = dual_fifo__buffer_size(gImg_dfifo);

    while (1)
    {
        // get a free buffer with forcely grabbing mode
        fifo_sts = dual_fifo__get_free_buffer(gImg_dfifo, &buf_addr, 0, true);
        if (fifo_sts != osOK)
            break;

        uint32_t recv_len = buf_size;
        usb_sts = kdrv_usbd3_bulk_receive(g_enp_out, (void *)buf_addr, &recv_len, 0);
        if (usb_sts != KDRV_STATUS_OK)
            break;

        // check the file size match or not, if not receive more
        uint32_t actual_size = *(uint32_t *)buf_addr;
        if (actual_size > recv_len)
        {
            // receive the left part
            uint32_t recv_len2 = buf_size - recv_len;
            usb_sts = kdrv_usbd3_bulk_receive(g_enp_out, (void *)(buf_addr + recv_len), &recv_len2, 0);
            if (usb_sts != KDRV_STATUS_OK)
                break;
        }

        kmdw_printf("[data_rx] received data on buf 0x%x size %d\n", buf_addr, actual_size);

        fifo_sts = dual_fifo__enqueue_data(gImg_dfifo, &buf_addr, osWaitForever);
        if (fifo_sts != osOK)
            break;
    }

    kmdw_printf("[data_rx] stopped !\n");
}

static void result_send_thread(void *arg)
{
    // the begin of STM, FIXME
    osThreadFlagsWait(TFLAG_STM_START, osFlagsWaitAny, osWaitForever);

    kmdw_printf("[result_tx] ready !\n");

    osStatus_t fifo_sts;
    kdrv_status_t usb_sts;
    uint32_t buf_addr;

    while (1)
    {
        // get result data from queue blocking wait
        fifo_sts = dual_fifo__dequeue_data(gResult_dfifo, &buf_addr, osWaitForever);
        if (fifo_sts != osOK)
            break;

        uint32_t txlen = *(uint32_t *)buf_addr;

        kmdw_printf("[data_tx] sending data from buf 0x%x size %d\n", buf_addr, txlen);

        // send result to the host, blocking wait
        usb_sts = kdrv_usbd3_bulk_send(g_enp_in, (void *)buf_addr, txlen, 0);
        if (usb_sts != KDRV_STATUS_OK)
            break;

        // return free buf back to queue
        fifo_sts = dual_fifo__put_free_buffer(gResult_dfifo, &buf_addr, osWaitForever);
        if (fifo_sts != osOK)
            break;
    }

    kmdw_printf("[data_tx] stopped !\n");
}

////////////////////////////////////////////////////////////

osThreadId_t usbd_fifo_com_init(dual_fifo_t image_dfifo, dual_fifo_t result_dfifo, uint8_t enpoint_in, uint8_t enpoint_out)
{
    usbd_queue_rx_tid = osThreadNew(data_recv_thread, NULL, NULL);
    if( !usbd_queue_rx_tid )
        err_msg("[******** ERROR ********] result_send_thread not launched\n");
    
    usbd_queue_tx_tid = osThreadNew(result_send_thread, NULL, NULL);
    if( !usbd_queue_tx_tid )
        err_msg("[******** ERROR ********] result_send_thread not launched\n");

    g_enp_in = enpoint_in;
    g_enp_out = enpoint_out;

    gImg_dfifo = image_dfifo;
    gResult_dfifo = result_dfifo;

    return usbd_queue_rx_tid;
}
