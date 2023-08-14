#include "kmdw_usbh.h"
#include "kmdw_uvc.h"
#include "kmdw_memory.h"
#include "kdrv_gdma.h"
#include "kmdw_camera.h"
#include "kmdw_console.h"
#ifdef USBH_UVC_ERR
#include "kmdw_console.h"
#endif

#define UVC_MAX_FRAME 10

typedef struct _UVC_FRAME_LINK
{
    uint32_t *user_frame;
    uint32_t size;
    uint8_t in_use; // 1: in use, 0: not in use
    struct _UVC_FRAME_LINK *next;
} UVC_FRAME_LINK;

static UVC_FRAME_LINK uvc_block_list[UVC_MAX_FRAME] = {0};
static UVC_FRAME_LINK *frame_head = NULL;
static UVC_FRAME_LINK *frame_last = NULL;
#ifdef KDP_UVC
typedef struct write_frame
{
    uint32_t buffer;
    uint32_t size;
	  int index;

} write_frame_t;

static write_frame_t curr_write_frame = {0, 0, -1};

extern uint32_t ping_pong_buf_addr[2];
#endif

// GDMA handle
static kdrv_gdma_handle_t dma_handle;

__weak uint8_t USBH_UVC_Configure(uint8_t device, const USB_DEVICE_DESCRIPTOR *ptr_dev_desc, const USB_CONFIGURATION_DESCRIPTOR *ptr_cfg_desc)
{
    return 0;
}

__weak usbStatus USBH_UVC_Initialize(uint8_t instance)
{
    return usbUnknownError;
}


__weak usbStatus USBH_UVC_Disconnected(uint8_t instance)
{
    return usbUnknownError;
}

#ifdef KDP_UVC
__weak void USBH_UVC_Get_Frame(uint32_t *frame_ptr, uint32_t *frame_size, int *index)
{
    return;
}
#else
__weak void USBH_UVC_Get_Frame(uint32_t *frame_ptr, uint32_t frame_size)
{
    return;
}
#endif
// callback from usbh middleware
uint8_t USBH_CustomClass_Configure(uint8_t device, const USB_DEVICE_DESCRIPTOR *ptr_dev_desc, const USB_CONFIGURATION_DESCRIPTOR *ptr_cfg_desc)
{
    // can do something for UVC
    return USBH_UVC_Configure(device, ptr_dev_desc, ptr_cfg_desc);
}

// callback from usbh middleware
usbStatus USBH_CustomClass_Initialize(uint8_t instance)
{
    // UVC internal initialization

    // can do something for UVC
    return USBH_UVC_Initialize(instance);
}

usbStatus USBH_CustomClass_Disconnected(uint8_t instance)
{
    // UVC internal initialization

    // can do something for UVC
    return USBH_UVC_Disconnected(instance);
}

usbStatus USBH_UVC_VS_Control(uint8_t device, UVC_VS_Request_t vs_req, UVC_VS_ControlSelector_t cs, UVC_PROBE_COMMIT_CONTROL *upc_ctrl)
{
    USB_SETUP_PACKET setup;

    setup.bmRequestType.Recipient = 0x1; // interface
    setup.bmRequestType.Type = 0x1;      // class
    setup.bmRequestType.Dir = (vs_req == SET_CUR) ? 0 : 1;
    setup.bRequest = vs_req;
    setup.wValue = cs;
    setup.wIndex = 1; // zero and interface
    setup.wLength = 26;

    return USBH_ControlTransfer(device, &setup, (uint8_t *)upc_ctrl, 26);
}

USBH_PIPE_HANDLE USBH_UVC_PipeCreate_Isoch(uint8_t device, uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval)
{
#define ITD_BUF_SIZE (4 * 1024 * 1024)
    static uint8_t *buf = NULL;
    if (NULL == buf)
        buf = (uint8_t *)kmdw_ddr_reserve(ITD_BUF_SIZE);
    return USBH_Pipe_ISOCH_PipeCreate(device, ep_addr, wMaxPacketSize, bInterval, buf, ITD_BUF_SIZE);
}

#define UVC_HEADER_SIZE 12
#define USBH_UVC_Send_Frame USBH_UVC_Get_Frame // uvc middleware send frames and uvc user get frames

#ifdef KDP_UVC
#define ty_msg(fmt, ...) info_msg(fmt, ##__VA_ARGS__)

void uvc_isoch_cb(uint32_t *payload, uint32_t length)
{
    static uint32_t frame_cur_offset = 0;
    static uint32_t pay_load_num =0;
    static uint32_t pay_load_size =0;    
    if (curr_write_frame.index == -1)
		    USBH_UVC_Get_Frame(&curr_write_frame.buffer, &curr_write_frame.size, &curr_write_frame.index);	

    if ((2 >= length ) || (0 == payload ))
        return;
   
    uint32_t *frame_buf_offset = (uint32_t *)((uint32_t)(curr_write_frame.buffer) + frame_cur_offset);
    uint8_t *uv_header = (uint8_t *)payload;
    uint32_t *img_body = (uint32_t *)((uint32_t)payload + UVC_HEADER_SIZE);
    int32_t payload_size = (length - UVC_HEADER_SIZE);
    uint16_t *p = (uint16_t *)img_body;

    if (payload_size > 0) {
        pay_load_num++;
        pay_load_size +=payload_size;
      
        for (int i = 0; i  < (payload_size >> 1) + (payload_size & 0x1); i++)
            p[i] = p[i] >> 8 | p[i] << 8;

        if ((frame_cur_offset + payload_size) <= ITD_BUF_SIZE)
        {
            kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, (uint32_t)frame_buf_offset, (uint32_t)img_body, payload_size);

#ifdef USBH_UVC_ERR
            if (dma_sts != KDRV_STATUS_OK)
                kmdw_printf("UVC: DMA failed, dst 0x%x src 0x%x size %d\n", (uint32_t)frame_buf_offset, (uint32_t)img_body, payload_size);
#endif
        }
        else
        {
#ifdef USBH_UVC_ERR
            kmdw_printf("UVC: detect frame size overrun\n");			
#endif			
            frame_cur_offset = 0;
            kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, (uint32_t)frame_buf_offset, (uint32_t)img_body, payload_size);
        }
    }
    if (uv_header[1] & 0x2) // check bit D1:End of Frame, UVC SPEC 1.0 Table 2-5
    {
        pay_load_num = 0;
        pay_load_size = 0;

        // FIXME: temp solution (ping pong buffer) to remove kdrv_fb_mgr, need to test if 520 uvc device is ready
        if (curr_write_frame.buffer == ping_pong_buf_addr[0])
            curr_write_frame.buffer = ping_pong_buf_addr[1];
        else
            curr_write_frame.buffer = ping_pong_buf_addr[0];

        frame_cur_offset = 0;
    }
    else {
        frame_cur_offset += payload_size;
 
    }

}
#else
void uvc_isoch_cb(uint32_t *payload, uint32_t length)
{
    static uint32_t frame_cur_offset = 0;
   
    if (frame_head == NULL || frame_head->user_frame == NULL)
    {
#ifdef USBH_UVC_ERR
        kmdw_printf("UVC: no available frame buffer to write\n");
#endif
        return;
    }
    if ((0 == length ) || (0 == payload ))
			return;
		
    uint32_t *frame_buf_offset = (uint32_t *)((uint32_t)(frame_head->user_frame) + frame_cur_offset);
    uint8_t *uv_header = (uint8_t *)payload;
    uint32_t *img_body = (uint32_t *)((uint32_t)payload + UVC_HEADER_SIZE);
    uint32_t payload_size = (length - UVC_HEADER_SIZE);
    uint16_t *p = (uint16_t *)img_body;
	
    for (int i = 0; i  < (payload_size >> 1) + (payload_size & 0x1); i++)
        p[i] = p[i] >> 8 | p[i] << 8;

    if ((frame_cur_offset + payload_size) <= frame_head->size)
    {
        kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, (uint32_t)frame_buf_offset, (uint32_t)img_body, payload_size);

#ifdef USBH_UVC_ERR
        if (dma_sts != KDRV_STATUS_OK)
            kmdw_printf("UVC: DMA failed, dst 0x%x src 0x%x size %d\n", (uint32_t)frame_buf_offset, (uint32_t)img_body, payload_size);
#endif
    }
#ifdef USBH_UVC_ERR
    else
    {
        frame_cur_offset = 0;
        kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, (uint32_t)frame_buf_offset, (uint32_t)img_body, payload_size);
    }
#endif
    if (uv_header[1] & 0x2) // check bit D1:End of Frame, UVC SPEC 1.0 Table 2-5
    {
        // send completed frame to user and advacne to the next frame
        USBH_UVC_Send_Frame(frame_head->user_frame, frame_head->size);

        frame_head->in_use = 0;
        frame_head = frame_head->next;
        if (frame_head == NULL)
            frame_last = NULL;

        frame_cur_offset = 0;
    }
    else {
        frame_cur_offset += payload_size;

    }
}
#endif
usbStatus USBH_UVC_PipeStart_Isoch(USBH_PIPE_HANDLE pipe_hndl)
{
    // acquire a DMA channel here and configure it in advance

    // this is to make sure GDMA initialization
    kdrv_gdma_initialize();

    kdrv_status_t sts = kdrv_gdma_acquire_handle(&dma_handle);
#ifdef USBH_UVC_ERR
    if (sts != KDRV_STATUS_OK)
    {
        kmdw_printf("UVC: acquire GDMA handle failed\n");
        return usbUnknownError;
    }
#endif

    gdma_setting_t dma_setting;
    dma_setting.dst_width = GDMA_TXFER_WIDTH_32_BITS;
    dma_setting.src_width = GDMA_TXFER_WIDTH_32_BITS;
    dma_setting.burst_size = GDMA_BURST_SIZE_16;
    dma_setting.dst_addr_ctrl = GDMA_INCREMENT_ADDRESS;
    dma_setting.src_addr_ctrl = GDMA_INCREMENT_ADDRESS;
    dma_setting.dma_mode = GDMA_NORMAL_MODE;
    dma_setting.dma_dst_req = 0;
    dma_setting.dma_src_req = 0;

    kdrv_gdma_configure_setting(dma_handle, &dma_setting);

    // then start ISOCH transfer
    return USBH_Pipe_ISOCH_Start(pipe_hndl, uvc_isoch_cb);
}

usbStatus USBH_UVC_PipeStop_Isoch(USBH_PIPE_HANDLE pipe_hndl)
{
    kdrv_gdma_release_handle(dma_handle);
    return USBH_Pipe_ISOCH_Stop(pipe_hndl);
}

usbStatus USBH_UVC_Queue_Frame(USBH_PIPE_HANDLE pipe, uint32_t *frame_ptr, uint32_t size)
{
    // FIXME : pipe ?

    UVC_FRAME_LINK *new_frame;

    // find a free one
    int i;
    for (i = 0; i < UVC_MAX_FRAME; i++)
        if (uvc_block_list[i].in_use == 0)
        {
            new_frame = &uvc_block_list[i];
            new_frame->in_use = 1;
            break;
        };

#ifdef USBH_UVC_ERR
    if (i >= UVC_MAX_FRAME)
    {
        kmdw_printf("UVC: no available UVC block to queue\n");
    }
#endif

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

    return usbOK;
}
