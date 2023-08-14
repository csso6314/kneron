//#define USBH2_MDW_DBG // turn on this for debug logs

#include "cmsis_os2.h"

#include "kmdw_usbh2.h"
#include "kdrv_usbh2.h"
#include <string.h>
#include <stdlib.h>
#include "kmdw_console.h"

#ifdef USBH2_MDW_DBG
#define dbg_printf(__format__, ...) kmdw_printf(__format__, ##__VA_ARGS__)
#else
#define dbg_printf(__format__, ...)
#endif

#define USBH_EVENT_QUEUE_LEN 16 // number of Message Queue Objects

typedef enum
{
    USBH2_STM_INITED = 0,
    USBH2_STM_CONNECTED_FS,
    USBH2_STM_DISCONNECTED,
    USBH2_STM_RESET_HS_DONE,   // reset and at high-speed
    USBH2_STM_RESET_HS_DONE_2, // reset and at high-speed, 2nd
    USBH2_STM_CUSTOM_CONFIGURE,
    USBH2_STM_CUSTOM_INITIALIZE,
    USBH2_STM_CUSTOM_INITIALIZE_DONE,
    USBH2_STM_FAILED = 0x100,
} USBH2_STM_t;

typedef struct
{                 // object data type
    uint8_t type; // 0x11 = port, 0x22 = pipe
    uint8_t port; // for port event
    //uint32_t pipe;  // for pipe event
    uint32_t event; // for both port and pipe events
} USBH_Event_t;

// below record what thread is running what pipe
typedef struct
{
    kmdw_usbh2_pipe_t pipe;
    osThreadId_t thread;
} USBH_PIPE_TID_t;

#define MAX_PIPE_NUM 6 // FIXME
#define USER_FLAG_XFER_COMPLETE 0x100U
#define MDW_FLAG_MESSAGE 0x1000U
#define MDW_FLAG_ITD_WORK 0x2000U

#define DEV_ADDR 0x3 // FIXME: constant value ?

static osThreadId_t usbh2_mdw_tid;
static int usbh_state; // state machine control

static uint32_t cur_pipe_num = 0;
static USBH_PIPE_TID_t dev_pipetid[MAX_PIPE_NUM] = {0}; // if pipe is not 0, it is in use
static osMessageQueueId_t usbh_msgq = NULL;
static kdrv_usbh2_port_state_t port_st;

kdrv_usbh2_isoch_itd_work_func_t itd_work_func = 0; // this will point to a callback from USBH driver

static kmdw_usbh2_device_descriptor_t dev_descp;
static kmdw_usbh2_configuration_descriptor_t config_descp;

// USB Descriptor Types
#define USBH2_DEVICE_DESCRIPTOR_TYPE 1U
#define USBH2_CONFIGURATION_DESCRIPTOR_TYPE 2U
#define USBH2_STRING_DESCRIPTOR_TYPE 3U
#define USBH2_INTERFACE_DESCRIPTOR_TYPE 4U
#define USBH2_ENDPOINT_DESCRIPTOR_TYPE 5U
#define USBH2_DEVICE_QUALIFIER_DESCRIPTOR_TYPE 6U
#define USBH2_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE 7U
#define USBH2_INTERFACE_POWER_DESCRIPTOR_TYPE 8U
#define USBH2_OTG_DESCRIPTOR_TYPE 9U
#define USBH2_DEBUG_DESCRIPTOR_TYPE 10U
#define USBH2_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE 11U

#define USBH2_REQUEST_GET_STATUS 0U
#define USBH2_REQUEST_CLEAR_FEATURE 1U
#define USBH2_REQUEST_SET_FEATURE 3U
#define USBH2_REQUEST_SET_ADDRESS 5U
#define USBH2_REQUEST_GET_DESCRIPTOR 6U
#define USBH2_REQUEST_SET_DESCRIPTOR 7U
#define USBH2_REQUEST_GET_CONFIGURATION 8U
#define USBH2_REQUEST_SET_CONFIGURATION 9U
#define USBH2_REQUEST_GET_INTERFACE 10U
#define USBH2_REQUEST_SET_INTERFACE 11U
#define USBH2_REQUEST_SYNC_FRAME 12U

// bmRequestType.Dir
#define USBH2_REQUEST_HOST_TO_DEVICE 0U
#define USBH2_REQUEST_DEVICE_TO_HOST 1U

// bmRequestType.Type
#define USBH2_REQUEST_STANDARD 0U
#define USBH2_REQUEST_CLASS 1U
#define USBH2_REQUEST_VENDOR 2U
#define USBH2_REQUEST_RESERVED 3U

// bmRequestType.Recipient
#define USBH2_REQUEST_TO_DEVICE 0U
#define USBH2_REQUEST_TO_INTERFACE 1U
#define USBH2_REQUEST_TO_ENDPOINT 2U
#define USBH2_REQUEST_TO_OTHER 3U

__weak void default_configured_cb(const kmdw_usbh2_device_descriptor_t *ptr_dev_desc,
                                  const kmdw_usbh2_configuration_descriptor_t *ptr_cfg_desc)
{
}

__weak void default_disconnected_cb()
{
}

kmdw_usbh2_configured_callback_t config_callback = default_configured_cb;
kmdw_usbh2_disconnected_callback_t discon_callback = default_disconnected_cb;

static void usbh_port_event_cb(uint8_t port, kdrv_usbh2_port_event_t event)
{
    // dont take too much time here, because it is invoked from ISR
    USBH_Event_t uevent;
    uevent.type = 0x11;
    uevent.port = port;
    uevent.event = event;
    osMessageQueuePut(usbh_msgq, &uevent, 0U, 0U);
    osThreadFlagsSet(usbh2_mdw_tid, MDW_FLAG_MESSAGE);
}

static void usbh_pipe_event_cb(kdrv_usbh2_pipe_t pipe_hndl, kdrv_usbh2_pipe_event_t event)
{
    // dont take too much time here, because it is invoked from ISR

    if (event != USBH2_EVENT_TRANSFER_COMPLETE)
    {
        dbg_printf("@@ %s() error: event is not transfer_complete !!\n", __FUNCTION__);
    }

    // there must be a pipe transfer waiting for this

    for (int i = 0; i < MAX_PIPE_NUM; i++)
    {
        if (pipe_hndl == dev_pipetid[i].pipe)
        {
            osThreadFlagsSet(dev_pipetid[i].thread, USER_FLAG_XFER_COMPLETE);
        }
    }
}

static void usbh_handle_port_evnet(uint8_t port, kdrv_usbh2_port_event_t event)
{
    switch (event)
    {
    case USBH2_EVENT_CONNECT:
    {
        // NOTE: we support only high-speed
        port_st = kdrv_usbh2_port_get_state(0);

        dbg_printf("@@ %s() Connect: isCnt 0x%x, speed 0x%x\n", __FUNCTION__, port_st.connected, port_st.speed);

        if (port_st.connected && port_st.speed == USBH2_SPEED_FULL)
        {
            // here make sure state is connected and FULL speed (pre state for high-speed)
            usbh_state = USBH2_STM_CONNECTED_FS;
        }
        else if (port_st.connected && port_st.speed == USBH2_SPEED_HIGH && usbh_state == USBH2_STM_DISCONNECTED)
        {
            // here make sure state is connected and FULL speed (pre state for high-speed)
            usbh_state = USBH2_STM_CONNECTED_FS;
        }
        else
        {

            dbg_printf("@@ %s() Connect: state is incorrect\n", __FUNCTION__);

            usbh_state = USBH2_STM_FAILED;
        }
    }
    break;

    case USBH2_EVENT_DISCONNECT:
    {
        port_st = kdrv_usbh2_port_get_state(0);

        dbg_printf("@@ %s() Disconnect: isCnt 0x%x, speed 0x%x\n", __FUNCTION__, port_st.connected, port_st.speed);

        if (port_st.speed == USBH2_SPEED_HIGH)
        {
            usbh_state = USBH2_STM_DISCONNECTED;
        }
        else
        {

            dbg_printf("@@ %s() Reset: error connection or speed\n", __FUNCTION__);

            usbh_state = USBH2_STM_FAILED;
        }
    }
    break;

    case USBH2_EVENT_RESET:
    {
        port_st = kdrv_usbh2_port_get_state(0);

        dbg_printf("@@ %s() Reset: isCnt 0x%x, speed 0x%x\n", __FUNCTION__, port_st.connected, port_st.speed);

        if (port_st.connected && port_st.speed == USBH2_SPEED_HIGH)
        {
            if (usbh_state == USBH2_STM_RESET_HS_DONE)
                usbh_state = USBH2_STM_RESET_HS_DONE_2;
            else
                usbh_state = USBH2_STM_RESET_HS_DONE;
        }
        else
        {

            dbg_printf("@@ %s() Reset: error connection or speed\n", __FUNCTION__);

            usbh_state = USBH2_STM_FAILED;
        }
    }
    break;

    default:

        dbg_printf("@@ %s() error: this event is not handled\n", __FUNCTION__);

        break;
    }
}

static void usbh_protocol_stm()
{
    switch (usbh_state)
    {
    case USBH2_STM_CONNECTED_FS: // connected at FS
    {

        dbg_printf("@@ %s() doing USBH2_STM_CONNECTED_FS\n", __FUNCTION__);

        // reset it to see if it can be HS
        osDelay(500); // FIXME
        kdrv_usbh2_port_reset(0);
    }
    break;

    case USBH2_STM_DISCONNECTED:
    {

        dbg_printf("@@ %s() doing USBH2_STM_DISCONNECTED_FS\n", __FUNCTION__);

        discon_callback();
        //    osDelay(500); // FIXME
        //   usbh_state = USBH2_STM_INITED;
    }
    break;

    case USBH2_STM_RESET_HS_DONE: // state is after reset, at HS
    {

        dbg_printf("@@ %s() doing USBH2_STM_RESET_HS_DONE\n", __FUNCTION__);

        // suspend & resume port
        kdrv_usbh2_port_suspend(0);
        osDelay(200); // FIXME

        kdrv_usbh2_port_resume(0);
        osDelay(200); // FIXME

        kdrv_usbh2_port_reset(0);
    }
    break;

    case USBH2_STM_RESET_HS_DONE_2: // state is after reset, at HS, 2nd
    {

        dbg_printf("@@ %s() doing USBH2_STM_RESET_HS_DONE_2\n", __FUNCTION__);

        uint8_t *config_buf = 0;
        // modify pipe to be HS pipe
        kdrv_usbh2_pipe_modify(dev_pipetid[0].pipe, 0x0, USBH2_SPEED_HIGH, 0x0, 0x0, 64);

        uint8_t data[64];
        kmdw_usbh2_setup_packet_t setup;

        // GET_DESCRIPTOR - device, 64 bytes
        {
            setup.bmRequestType.Recipient = USBH2_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USBH2_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USBH2_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USBH2_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USBH2_DEVICE_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = 64;

            kmdw_usbh2_control_transfer(&setup, data, 64);
        }

        // SET_ADDRESS (set adrees to DEV_ADDR)
        {
            setup.bmRequestType.Recipient = USBH2_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USBH2_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USBH2_REQUEST_HOST_TO_DEVICE;
            setup.bRequest = USBH2_REQUEST_SET_ADDRESS;
            setup.wValue = DEV_ADDR;
            setup.wIndex = 0;
            setup.wLength = 0;

            kmdw_usbh2_control_transfer(&setup, NULL, 0);
        }

        // update dev address of the pipe
        kdrv_usbh2_pipe_modify(dev_pipetid[0].pipe, DEV_ADDR, USBH2_SPEED_HIGH, 0x0, 0x0, 64);

        // GET_DESCRIPTOR - device, actual bytes with new address
        {
            setup.bmRequestType.Recipient = USBH2_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USBH2_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USBH2_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USBH2_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USBH2_DEVICE_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = 18;

            kmdw_usbh2_control_transfer(&setup, data, 64);
            memcpy(&dev_descp, data, sizeof(kmdw_usbh2_device_descriptor_t));
        }

        // GET_DESCRIPTOR - configuration, first 9 bytes
        {
            setup.bmRequestType.Recipient = USBH2_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USBH2_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USBH2_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USBH2_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USBH2_CONFIGURATION_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = 9;

            kmdw_usbh2_control_transfer(&setup, data, 64);
            memcpy(&config_descp, data, sizeof(kmdw_usbh2_configuration_descriptor_t));
        }

        // GET_DESCRIPTOR - configuration, full length
        {
            config_buf = malloc(config_descp.wTotalLength);
            memset(config_buf, 0, config_descp.wTotalLength);
            setup.bmRequestType.Recipient = USBH2_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USBH2_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USBH2_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USBH2_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USBH2_CONFIGURATION_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = config_descp.wTotalLength;

            kmdw_usbh2_control_transfer(&setup, config_buf, config_descp.wTotalLength);
        }

        // SET_CONFIGURATION - 0x1
        {
            setup.bmRequestType.Recipient = USBH2_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USBH2_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USBH2_REQUEST_HOST_TO_DEVICE;
            setup.bRequest = USBH2_REQUEST_SET_CONFIGURATION;
            setup.wValue = 1;
            setup.wIndex = 0;
            setup.wLength = 0;

            kmdw_usbh2_control_transfer(&setup, NULL, 0);
        }

        usbh_state = USBH2_STM_CUSTOM_CONFIGURE; // FIXME

        config_callback(&dev_descp, (const kmdw_usbh2_configuration_descriptor_t *)config_buf);
        memset(config_buf, 0, config_descp.wTotalLength);
        free(config_buf);

        usbh_state = USBH2_STM_CUSTOM_INITIALIZE; // FIXME
    }
    break;

    default:

        dbg_printf("@@ %s() error: invalid state\n", __FUNCTION__);

        break;
    }
}

static void usbh2_mdw_thread(void *argument)
{

    dbg_printf("@@ %s(), start USBH middleware thread.....\n", __FUNCTION__);

    while (1)
    {
        uint32_t flags = osThreadFlagsWait(MDW_FLAG_MESSAGE | MDW_FLAG_ITD_WORK, osFlagsWaitAny, 5000);

        if(flags == osFlagsErrorTimeout)
        {
            kmdw_printf("timeout, force itd work\n");
            itd_work_func();
            continue;
        }

        if (flags & MDW_FLAG_MESSAGE)
        {
            USBH_Event_t uevent;
            osStatus_t status = osMessageQueueGet(usbh_msgq, &uevent, NULL, osWaitForever); // wait for message
            if (status == osOK)
            {
                if (uevent.type == 0x11)
                    usbh_handle_port_evnet(uevent.port, (kdrv_usbh2_port_event_t)uevent.event);

                // state machine
                usbh_protocol_stm();
            }
        }
        if (flags & MDW_FLAG_ITD_WORK)
        {
            // here do the bottom-half iTD work
            itd_work_func();
        }
    }
}

kmdw_usbh2_status_t kmdw_usbh2_initialize(kmdw_usbh2_configured_callback_t config_cb, kmdw_usbh2_disconnected_callback_t discon_cb)
{

    dbg_printf("@@ %s()\n", __FUNCTION__);

    // need a message queue
    usbh_msgq = osMessageQueueNew(USBH_EVENT_QUEUE_LEN, sizeof(USBH_Event_t), NULL);

    if (usbh_msgq == NULL)
    {
        dbg_printf("@@ osMessageQueueNew() failed\n");
    }

    kdrv_usbh2_initialize(&usbh_port_event_cb, &usbh_pipe_event_cb);
    kdrv_usbh2_set_enable(true);
    dev_pipetid[cur_pipe_num++].pipe = kdrv_usbh2_pipe_create(0x0, USBH2_SPEED_LOW, 0x0, 0x0, 0x0, 0x0, 8, 0);
    kdrv_usbh2_vbus_on_off(0, true);

    usbh_state = USBH2_STM_INITED;

    // first, we need a thread
    usbh2_mdw_tid = osThreadNew(usbh2_mdw_thread, NULL, NULL);
    if( !usbh2_mdw_tid )
        err_msg("[******** ERROR ********] usbh2_mdw_thread not launched\n");
    
    osThreadSetPriority(usbh2_mdw_tid, osPriorityHigh); // FIXME: what priority is proper ?

    config_callback = config_cb;
    discon_callback = discon_cb;

    return USBH_OK;
}

kmdw_usbh2_status_t kmdw_usbh2_uninitialize(void)
{
    kdrv_usbh2_uninitialize();
    return USBH_OK;
}

kmdw_usbh2_pipe_t kmdw_usbh2_pipe_create(uint8_t ep_addr, uint8_t ep_type, uint16_t ep_max_packet_size, uint8_t ep_interval)
{
    kmdw_usbh2_pipe_t pipe_h;

    pipe_h = kdrv_usbh2_pipe_create(DEV_ADDR, USBH2_SPEED_HIGH, 0x0, 0x0, ep_addr, ep_type, ep_max_packet_size, ep_interval);

    dev_pipetid[cur_pipe_num++].pipe = pipe_h;

    dbg_printf("@@ %s() ep_addr 0x%x ep_type %d max_packet %d ep_interval %d, pipe 0x%p\n", __FUNCTION__, ep_addr, ep_type, ep_max_packet_size, ep_interval, pipe_h);

    return pipe_h;
}

static kmdw_usbh2_status_t _usbh_transfer_payload(kmdw_usbh2_pipe_t pipe_hndl, uint8_t *buf, uint32_t len, bool isSend)
{
    // look up the pipe index from the pipetid table
    int i;
    for (i = 0; i < MAX_PIPE_NUM; i++)
        if (pipe_hndl == dev_pipetid[i].pipe)
        {
            dev_pipetid[i].thread = osThreadGetId(); // register thread id for thread flag notification
            break;
        }

    if (i >= MAX_PIPE_NUM)
    {
        dbg_printf("-- pipe 0x%p is invalid\n", pipe_hndl);
        return USBH_INVALID_PARAMETER;
    }

    kdrv_status_t sts = kdrv_usbh2_pipe_transfer(pipe_hndl, isSend ? USBH2_PACKET_OUT : USBH2_PACKET_IN, buf, len);

    uint32_t flags = osThreadFlagsWait(USER_FLAG_XFER_COMPLETE, osFlagsWaitAny, 5000); // 5 secs timeout, should be long enough

    if (flags == osFlagsErrorTimeout)
    {
        dbg_printf("-- pipe 0x%p transfer timeout\n", pipe_hndl);
        return USBH_TIMEOUT;
    }

    if (sts != KDRV_STATUS_OK)
    {
        dbg_printf("-- pipe 0x%p error, sts = %d\n", pipe_hndl, sts);
        return USBH_TRANSFER_ERROR;
    }

    return USBH_OK;
}

#define MAX_TXFER_SIZE (16 * 1024) // FIXME !

int32_t kmdw_usbh2_bulk_out(kmdw_usbh2_pipe_t pipe_hndl, uint8_t *buf, uint32_t len)
{
    uint32_t wanted_bytes = len;
    uint32_t buff_addr = (uint32_t)buf;

    while (1)
    {
        uint32_t sendBytes = MAX_TXFER_SIZE; // set a max pipe send

        if (wanted_bytes < sendBytes)
            sendBytes = wanted_bytes;

        // NOTE: for now sendBytes MAX = 20 KB due to host driver implementation
        kmdw_usbh2_status_t sts = _usbh_transfer_payload(pipe_hndl, (uint8_t *)buff_addr, sendBytes, true);
        if (sts != USBH_OK)
            return sts;

        sendBytes = kdrv_usbh2_pipe_transfer_get_result(pipe_hndl);

        wanted_bytes -= sendBytes;
        buff_addr += sendBytes;

        if (wanted_bytes == 0)
        {
            if ((sendBytes % 512) == 0)
            {
                // zero-length packet
                kmdw_usbh2_status_t sts = _usbh_transfer_payload(pipe_hndl, (uint8_t *)buff_addr, 0, true);
                if (sts != USBH_OK)
                {
                    dbg_printf("@@ %s() send ZLP fialed, sts = %d\n", sts);
                    return sts;
                }
            }
            break;
        }
    }

    return len;
}

int32_t kmdw_usbh2_bulk_in(kmdw_usbh2_pipe_t pipe_hndl, uint8_t *buf, uint32_t buf_len)
{
    uint32_t total_recvd = 0;
    uint32_t buff_addr = (uint32_t)buf;

    while (1)
    {
        uint32_t recvBytes = MAX_TXFER_SIZE;

        if (buf_len < recvBytes)
            recvBytes = buf_len;

        kmdw_usbh2_status_t sts = _usbh_transfer_payload(pipe_hndl, (uint8_t *)buff_addr, recvBytes, false);
        if (sts != USBH_OK)
            return sts;

        recvBytes = kdrv_usbh2_pipe_transfer_get_result(pipe_hndl);

        total_recvd += recvBytes;
        buff_addr += recvBytes;
        buf_len -= recvBytes;

        if (recvBytes < MAX_TXFER_SIZE)
            break;
    }

    return (int32_t)total_recvd;
}

kmdw_usbh2_status_t kmdw_usbh2_interrupt_in(kmdw_usbh2_pipe_t pipe_hndl, uint8_t *buf, uint32_t len)
{
    return _usbh_transfer_payload(pipe_hndl, buf, len, false);
}

kmdw_usbh2_status_t kmdw_usbh2_control_transfer(const kmdw_usbh2_setup_packet_t *setup_packet, uint8_t *data, uint32_t len)
{

    dbg_printf("@@ %s() setup_packet : 0x ", __FUNCTION__);
    for (int i = 0; i < 8; i++)
    {
        dbg_printf("%02x ", ((uint8_t *)setup_packet)[i]);
    }
    dbg_printf(", data ptr 0x%p, len %u\n", data, len);

    kdrv_usbh2_pipe_t ctrl_pipe = dev_pipetid[0].pipe;
    dev_pipetid[0].thread = osThreadGetId();
    uint32_t packet;
    uint8_t *payload;
    uint32_t txfer_len;

    // three stages
    for (int stage = 0; stage < 3; stage++)
    {
        switch (stage)
        {
        case 0: // Setup stage

            packet = USBH2_PACKET_DATA0 | USBH2_PACKET_SETUP;
            payload = (uint8_t *)setup_packet;
            txfer_len = 8;

            break;
        case 1: // Data stage (optional)

            // skeip Data stage if wLength is 0
            if (setup_packet->wLength == 0)
                continue;

            // Contorl IN or OUT
            packet = USBH2_PACKET_DATA1 |
                     ((setup_packet->bmRequestType.Dir) ? USBH2_PACKET_IN : USBH2_PACKET_OUT);
            payload = data;
            txfer_len = len;

            break;
        case 2: // Status stage

            packet = USBH2_PACKET_DATA1 |
                     ((setup_packet->bmRequestType.Dir) ? USBH2_PACKET_OUT : USBH2_PACKET_IN);
            payload = NULL;
            txfer_len = 0;

            break;
        }

        kdrv_usbh2_pipe_transfer(ctrl_pipe, (kdrv_usbh2_packet_t)packet, payload, txfer_len);

        uint32_t flags = osThreadFlagsWait(USER_FLAG_XFER_COMPLETE, osFlagsWaitAny, 5000); // 5 secs timeout, should be long enough

        if (flags == osFlagsErrorTimeout)
        {
            dbg_printf("@@ %s() control transfer timeout\n", __FUNCTION__);
            return USBH_TIMEOUT;
        }
    }

    return USBH_OK;
}

kmdw_usbh2_pipe_t kmdw_usbh2_isoch_delete(kmdw_usbh2_pipe_t pipe_hndl)
{
    kdrv_usbh2_pipe_delete(pipe_hndl);
    return USBH_OK;
}

kmdw_usbh2_pipe_t kmdw_usbh2_isoch_create(uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval, uint8_t *buf, uint32_t buf_size)
{
    kmdw_usbh2_pipe_t pipe_h;

    uint16_t max_packet_size = wMaxPacketSize & 0x7FF;
    uint8_t mult = (wMaxPacketSize >> 11) + 1;

    pipe_h = kdrv_usbh2_pipe_isoch_create(DEV_ADDR, ep_addr, max_packet_size, mult, bInterval, buf, buf_size);

    dev_pipetid[cur_pipe_num++].pipe = pipe_h;

    dbg_printf("@@ %s() ep_addr 0x%x ep_type 'isoch' wMaxPacketSize 0x%x bInterval %d buf 0x%p buf_size %u pipe 0x%p\n",
               __FUNCTION__, ep_addr, wMaxPacketSize, bInterval, buf, buf_size, pipe_h);

    return pipe_h;
}

static uint32_t handle_itd_cb()
{
#ifdef USBH2_USE_TOP_HALF
    itd_work_func();
#else
    // wake up bottom-half thread
    osThreadFlagsSet(usbh2_mdw_tid, MDW_FLAG_ITD_WORK);
#endif
    return 1;
}

kmdw_usbh2_status_t kmdw_usbh2_isoch_start(kmdw_usbh2_pipe_t pipe_hndl, kmdw_usbh2_isoch_transfer_callback_t user_isoch_cb)
{
    // enable bottom-half mechanism
    itd_work_func = kdrv_usbh2_pipe_isoch_enable_bh(pipe_hndl, handle_itd_cb);

    // start isoch transfer with user data callback
    kdrv_usbh2_pipe_isoch_start(pipe_hndl, user_isoch_cb);

    return USBH_OK;
}

kmdw_usbh2_status_t kmdw_usbh2_isoch_stop(kmdw_usbh2_pipe_t pipe_hndl)
{
    kdrv_usbh2_pipe_isoch_stop(pipe_hndl);
    return USBH_OK;
}

kmdw_usbh2_status_t kmdw_usbh2_set_interface(uint8_t index, uint8_t alternate)
{
    kmdw_usbh2_setup_packet_t setup;

    setup.bmRequestType.Recipient = USBH2_REQUEST_TO_INTERFACE;
    setup.bmRequestType.Type = USBH2_REQUEST_STANDARD;
    setup.bmRequestType.Dir = USBH2_REQUEST_HOST_TO_DEVICE;
    setup.bRequest = USBH2_REQUEST_SET_INTERFACE;
    setup.wValue = alternate;
    setup.wIndex = index;
    setup.wLength = 0;

    return kmdw_usbh2_control_transfer(&setup, NULL, 0);
}
