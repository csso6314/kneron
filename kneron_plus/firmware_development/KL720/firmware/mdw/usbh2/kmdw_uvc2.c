//#define UVC2_MDW_DBG
#define UVC2_USE_GDMA

#include <string.h> // for memcpy

#include "kmdw_usbh2.h"
#include "kmdw_uvc2.h"
#include "kmdw_console.h"

#include "kmdw_memory.h"
#ifdef UVC2_USE_GDMA
#include "kdrv_gdma3.h"
#endif

#ifdef UVC2_MDW_DBG
#define dbg_printf(__format__, ...) kmdw_printf(__format__, ##__VA_ARGS__)
#else
#define dbg_printf(__format__, ...)
#endif

#define ITD_BUF_SIZE (24 * 1024 * 1024)
#define UVC_MAX_FRAME 10
#define UVC_HEADER_SIZE 12

typedef struct _UVC_FRAME_LINK
{
    uint32_t *user_frame;
    uint32_t size;
    uint8_t in_use; // 1: in use, 0: not in use
    struct _UVC_FRAME_LINK *next;
} _UVC_FRAME_LINK;

static _UVC_FRAME_LINK uvc_block_list[UVC_MAX_FRAME] = {0};
static _UVC_FRAME_LINK *frame_head = NULL;
static _UVC_FRAME_LINK *frame_last = NULL;

static kmdw_uvc2_get_frame_callback_t g_frame_cb = 0;

__weak void default_frame_cb(uint32_t *frame_ptr, uint32_t frame_size)
{
}

kmdw_usbh2_status_t kmdw_uvc2_vs_control(kmdw_uvc2_vs_request_t vs_req, kmdw_uvc2_vs_control_selector_t cs, kmdw_uvc2_probe_commit_control_t *upc_ctrl)
{
    kmdw_usbh2_setup_packet_t setup;

    setup.bmRequestType.Recipient = 0x1; // interface
    setup.bmRequestType.Type = 0x1;      // class
    setup.bmRequestType.Dir = (vs_req == UVC_SET_CUR) ? 0 : 1;
    setup.bRequest = vs_req;
    setup.wValue = cs;
    setup.wIndex = 1; // zero and interface
    setup.wLength = 26;

    return kmdw_usbh2_control_transfer(&setup, (uint8_t *)upc_ctrl, 26);
}

kmdw_usbh2_pipe_t kmdw_uvc2_isoch_create(uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval, kmdw_uvc2_get_frame_callback_t frame_cb)
{
    g_frame_cb = frame_cb ? frame_cb : default_frame_cb;

    uint8_t *buf = (uint8_t *)kmdw_ddr_reserve(ITD_BUF_SIZE);

    if(buf == NULL)
    {
        kmdw_printf("\nUVC alloc memory failed !!!!!!!!!!!!!!!!!!!!! \n\n");
        return 0;
    }

    return kmdw_usbh2_isoch_create(ep_addr, wMaxPacketSize, bInterval, buf, ITD_BUF_SIZE);
}

void uvc_isoch_cb(uint32_t *payload, uint32_t length)
{
    static uint32_t frame_cur_offset = 0;

    if (frame_head == NULL || frame_head->user_frame == NULL)
    {
        dbg_printf("UVC: no available frame buffer to write\n");
        return;
    }

    uint32_t *frame_buf_offset = (uint32_t *)((uint32_t)(frame_head->user_frame) + frame_cur_offset);
    uint8_t *uv_header = (uint8_t *)payload;
    uint32_t *img_body = (uint32_t *)((uint32_t)payload + UVC_HEADER_SIZE);
    uint32_t payload_size = (length - UVC_HEADER_SIZE);

    if (payload_size > 0 && (frame_cur_offset + payload_size) <= frame_head->size)
    {
#ifdef UVC2_USE_GDMA
        kdrv_gdma_memcpy((uint32_t)frame_buf_offset, (uint32_t)img_body, payload_size, NULL, NULL);

#else
        memcpy(frame_buf_offset, img_body, payload_size);
#endif
    }
    else
    {
        dbg_printf("UVC: detect frame size overrun\n");
    }

    frame_cur_offset += payload_size;

    if (uv_header[1] & 0x2) // check bit D1:End of Frame, UVC SPEC 1.0 Table 2-5
    {
        // send completed frame to user and advacne to the next frame
        g_frame_cb(frame_head->user_frame, frame_cur_offset);

        frame_head->in_use = 0;
        frame_head = frame_head->next;
        if (frame_head == NULL)
            frame_last = NULL;

        frame_cur_offset = 0;
    }
}

kmdw_usbh2_status_t kmdw_uvc2_isoch_start(kmdw_usbh2_pipe_t pipe_hndl)
{
#ifdef UVC2_USE_GDMA
    // this is to make sure GDMA initialization
    kdrv_gdma_initialize();
#endif
    // then start ISOCH transfer
    return kmdw_usbh2_isoch_start(pipe_hndl, uvc_isoch_cb);
}

kmdw_usbh2_status_t kmdw_uvc2_isoch_stop(kmdw_usbh2_pipe_t pipe_hndl)
{
    return kmdw_usbh2_isoch_stop(pipe_hndl);
}

kmdw_usbh2_status_t kmdw_uvc2_queue_frame(kmdw_usbh2_pipe_t pipe, uint32_t *frame_ptr, uint32_t size)
{
    // FIXME : pipe ?

    _UVC_FRAME_LINK *new_frame;

    // find a free one
    int i;
    for (i = 0; i < UVC_MAX_FRAME; i++)
        if (uvc_block_list[i].in_use == 0)
        {
            new_frame = &uvc_block_list[i];
            new_frame->in_use = 1;
            break;
        };

    if (i >= UVC_MAX_FRAME)
    {
        dbg_printf("UVC: no available UVC block to queue\n");
    }

    new_frame->user_frame = frame_ptr;
    new_frame->size = size; // it should match image size
    new_frame->next = NULL;

    if (frame_head)
    {
        frame_last->next = new_frame;
        frame_last = new_frame;
    }
    else
    {
        frame_head = frame_last = new_frame;
    }

    return USBH_OK;
}
