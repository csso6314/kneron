#ifndef __USBH_MDW_H__
#define __USBH_MDW_H__

#ifndef KNERON_USBH_MDW
#error "Please define 'KNERON_USBH_MDW' in global scope, it is necessary for Kneron USBH middleware."
// stop compliation
#endif

//#define USBH_MDW_DBG // turn on this can help trace code, a lot of code-size needed
//#define USBH_MDW_ERR // turn on this can add some more error checking, slightly code-size needed

#include "kmdw_usbh.h"
#include "Driver_USBH.h"
#include "kmdw_memory.h"
#include <string.h>
#include <stdlib.h>

#if defined(USBH_MDW_DBG) | defined(USBH_MDW_ERR)
#include "kdrv_uart.h"
#endif

#define USBH_EVENT_QUEUE_LEN 16 // number of Message Queue Objects

typedef enum
{
    USBH_STM_INITED = 0,
    USBH_STM_CONNECTED_FS,
    USBH_STM_DISCONNECTED,	
    USBH_STM_RESET_HS_DONE,   // reset and at high-speed
    USBH_STM_RESET_HS_DONE_2, // reset and at high-speed, 2nd
    USBH_STM_CUSTOM_CONFIGURE,
    USBH_STM_CUSTOM_INITIALIZE,
    USBH_STM_CUSTOM_INITIALIZE_DONE,
    USBH_STM_FAILED = 0x100,
} USBH_STM_t;

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
    USBH_PIPE_HANDLE pipe;
    osThreadId_t thread;
} USBH_PIPE_TID_t;

#define MAX_PIPE_NUM 6 // FIXME
#define USER_FLAG_XFER_COMPLETE 0x100U
#define MDW_FLAG_MESSAGE 0x1000U
#define MDW_FLAG_ITD_WORK 0x2000U

#define DEV_ADDR 0x3 // FIXME: constant value ?

static osThreadId_t usbh_mdw_tid;
static int usbh_state; // state machine control

static uint32_t cur_pipe_num = 0;
static USBH_PIPE_TID_t dev_pipetid[MAX_PIPE_NUM] = {0}; // if pipe is not 0, it is in use
static osMessageQueueId_t usbh_msgq = NULL;
static ARM_USBH_PORT_STATE port_st;

ARM_USBH_ISOCH_ITD_WORK_FUNC itd_work_func = 0; // this will point to a callback from USBH driver

static USB_DEVICE_DESCRIPTOR dev_descp;
static USB_CONFIGURATION_DESCRIPTOR config_descp;

__weak uint8_t USBH_CustomClass_Configure(uint8_t device, const USB_DEVICE_DESCRIPTOR *ptr_dev_desc, const USB_CONFIGURATION_DESCRIPTOR *ptr_cfg_desc)
{
    return 0;
}

__weak usbStatus USBH_CustomClass_Initialize(uint8_t instance)
{
    return usbUnknownError;
}

__weak usbStatus USBH_CustomClass_Disconnected(uint8_t instance)
{
    return usbUnknownError;
}

static void usbh_port_event_cb(uint8_t port, uint32_t event)
{
    // dont take too much time here, because it is invoked from ISR
    USBH_Event_t uevent;
    uevent.type = 0x11;
    uevent.port = port;
    uevent.event = event;
    osMessageQueuePut(usbh_msgq, &uevent, 0U, 0U);
    osThreadFlagsSet(usbh_mdw_tid, MDW_FLAG_MESSAGE);
}

static void usbh_pipe_event_cb(ARM_USBH_PIPE_HANDLE pipe_hndl, uint32_t event)
{
    // dont take too much time here, because it is invoked from ISR

#ifdef USBH_MDW_ERR
    if (event != ARM_USBH_EVENT_TRANSFER_COMPLETE)
        kmdw_printf("@@ %s() error: event is not transfer_complete !!\n", __FUNCTION__);
#endif

    // there must be a pipe transfer waiting for this

    for (int i = 0; i < MAX_PIPE_NUM; i++)
    {
        if (pipe_hndl == dev_pipetid[i].pipe)
        {
            osThreadFlagsSet(dev_pipetid[i].thread, USER_FLAG_XFER_COMPLETE);
        }
    }
}

static void usbh_handle_port_evnet(uint8_t port, uint32_t event)
{
    switch (event)
    {
    case ARM_USBH_EVENT_CONNECT:
    {
        // NOTE: we support only high-speed
        port_st = Driver_USBH0.PortGetState(0);

#ifdef USBH_MDW_DBG
        kmdw_printf("@@ %s() Connect: isCnt 0x%x, speed 0x%x\n", __FUNCTION__, port_st.connected, port_st.speed);
#endif

        if (port_st.connected && port_st.speed == ARM_USB_SPEED_FULL)
        {
            // here make sure state is connected and FULL speed (pre state for high-speed)
            usbh_state = USBH_STM_CONNECTED_FS;
        }
				else if (port_st.connected && port_st.speed == ARM_USB_SPEED_HIGH && usbh_state == USBH_STM_DISCONNECTED)
        {
            // here make sure state is connected and FULL speed (pre state for high-speed)
            usbh_state = USBH_STM_CONNECTED_FS;
        }
        else
        {
#ifdef USBH_MDW_ERR
            kmdw_printf("@@ %s() Connect: state is incorrect\n", __FUNCTION__);
#endif
            usbh_state = USBH_STM_FAILED;
        }
    }
    break;

    case ARM_USBH_EVENT_DISCONNECT:
    {
        port_st = Driver_USBH0.PortGetState(0);

#ifdef USBH_MDW_DBG
        kmdw_printf("@@ %s() Disconnect: isCnt 0x%x, speed 0x%x\n", __FUNCTION__, port_st.connected, port_st.speed);
#endif

        if (port_st.speed == ARM_USB_SPEED_HIGH)
        {
            usbh_state = USBH_STM_DISCONNECTED;
        }
        else
        {
#ifdef USBH_MDW_ERR
            kmdw_printf("@@ %s() Reset: error connection or speed\n", __FUNCTION__);
#endif
            usbh_state = USBH_STM_FAILED;
        }
    }
    break;

		
    case ARM_USBH_EVENT_RESET:
    {
        port_st = Driver_USBH0.PortGetState(0);

#ifdef USBH_MDW_DBG
        kmdw_printf("@@ %s() Reset: isCnt 0x%x, speed 0x%x\n", __FUNCTION__, port_st.connected, port_st.speed);
#endif

        if (port_st.connected && port_st.speed == ARM_USB_SPEED_HIGH)
        {
            if (usbh_state == USBH_STM_RESET_HS_DONE)
                usbh_state = USBH_STM_RESET_HS_DONE_2;
            else
                usbh_state = USBH_STM_RESET_HS_DONE;
        }
        else
        {
#ifdef USBH_MDW_ERR
            kmdw_printf("@@ %s() Reset: error connection or speed\n", __FUNCTION__);
#endif
            usbh_state = USBH_STM_FAILED;
        }
    }
    break;

    default:
#ifdef USBH_MDW_ERR
        kmdw_printf("@@ %s() error: this event is not handled\n", __FUNCTION__);
#endif
        break;
    }
}

static void usbh_protocol_stm()
{
    switch (usbh_state)
    {
    case USBH_STM_CONNECTED_FS: // connected at FS
    {
#ifdef USBH_MDW_DBG
        kmdw_printf("@@ %s() doing USBH_STM_CONNECTED_FS\n", __FUNCTION__);
#endif
        // reset it to see if it can be HS
        osDelay(500); // FIXME
        Driver_USBH0.PortReset(0);
    }
    break;
		
    case USBH_STM_DISCONNECTED:
    {
#ifdef USBH_MDW_DBG
        kmdw_printf("@@ %s() doing USBH_STM_DISCONNECTED_FS\n", __FUNCTION__);
#endif
        USBH_CustomClass_Disconnected(0);
    //    osDelay(500); // FIXME			
    //   usbh_state = USBH_STM_INITED;
    }
    break;
		
    case USBH_STM_RESET_HS_DONE: // state is after reset, at HS
    {
#ifdef USBH_MDW_DBG
        kmdw_printf("@@ %s() doing USBH_STM_RESET_HS_DONE\n", __FUNCTION__);
#endif

        // suspend & resume port
        Driver_USBH0.PortSuspend(0);
        osDelay(200); // FIXME

        Driver_USBH0.PortResume(0);
        osDelay(200); // FIXME

        Driver_USBH0.PortReset(0);
    }
    break;

    case USBH_STM_RESET_HS_DONE_2: // state is after reset, at HS, 2nd
    {
	
#ifdef USBH_MDW_DBG
        kmdw_printf("@@ %s() doing USBH_STM_RESET_HS_DONE_2\n", __FUNCTION__);
#endif
		    uint8_t *config_buf = 0;
        // modify pipe to be HS pipe
        Driver_USBH0.PipeModify(dev_pipetid[0].pipe, 0x0, ARM_USB_SPEED_HIGH, 0x0, 0x0, 64);
			
        uint8_t data[64];
        USB_SETUP_PACKET setup;
   
        // GET_DESCRIPTOR - device, 64 bytes
        {
            setup.bmRequestType.Recipient = USB_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USB_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USB_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USB_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USB_DEVICE_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = 64;

            USBH_ControlTransfer(0, &setup, data, 64);
        }

        // SET_ADDRESS (set adrees to DEV_ADDR)
        {
            setup.bmRequestType.Recipient = USB_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USB_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USB_REQUEST_HOST_TO_DEVICE;
            setup.bRequest = USB_REQUEST_SET_ADDRESS;
            setup.wValue = DEV_ADDR;
            setup.wIndex = 0;
            setup.wLength = 0;

            USBH_ControlTransfer(0, &setup, NULL, 0);
        }

        // update dev address of the pipe
        Driver_USBH0.PipeModify(dev_pipetid[0].pipe, DEV_ADDR, ARM_USB_SPEED_HIGH, 0x0, 0x0, 64);

        // GET_DESCRIPTOR - device, actual bytes with new address
        {
            setup.bmRequestType.Recipient = USB_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USB_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USB_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USB_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USB_DEVICE_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = 18;

            USBH_ControlTransfer(0, &setup, data, 64);
            memcpy(&dev_descp, data, sizeof(USB_DEVICE_DESCRIPTOR));
        }

        // GET_DESCRIPTOR - configuration, first 9 bytes
        {
            setup.bmRequestType.Recipient = USB_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USB_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USB_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USB_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USB_CONFIGURATION_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = 9;

            USBH_ControlTransfer(0, &setup, data, 64);
            memcpy(&config_descp, data, sizeof(USB_CONFIGURATION_DESCRIPTOR));
        }

        // GET_DESCRIPTOR - configuration, full length
        {
				
            if (0 == (config_buf = malloc(config_descp.wTotalLength)))
                while (1) {};
            memset(config_buf, 0,config_descp.wTotalLength);
            setup.bmRequestType.Recipient = USB_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USB_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USB_REQUEST_DEVICE_TO_HOST;
            setup.bRequest = USB_REQUEST_GET_DESCRIPTOR;
            setup.wValue = USB_CONFIGURATION_DESCRIPTOR_TYPE << 8;
            setup.wIndex = 0;
            setup.wLength = config_descp.wTotalLength;

            USBH_ControlTransfer(0, &setup, config_buf, config_descp.wTotalLength);
			
        }

        // SET_CONFIGURATION - 0x1
        {
            setup.bmRequestType.Recipient = USB_REQUEST_TO_DEVICE;
            setup.bmRequestType.Type = USB_REQUEST_STANDARD;
            setup.bmRequestType.Dir = USB_REQUEST_HOST_TO_DEVICE;
            setup.bRequest = USB_REQUEST_SET_CONFIGURATION;
            setup.wValue = 1;
            setup.wIndex = 0;
            setup.wLength = 0;

            USBH_ControlTransfer(0, &setup, NULL, 0);
        }

        usbh_state = USBH_STM_CUSTOM_CONFIGURE; // FIXME

        // next : callback user's USBH_CustomClass_Configure()
    
        USBH_CustomClass_Configure(0, &dev_descp, (const USB_CONFIGURATION_DESCRIPTOR *)config_buf);
        memset(config_buf, 0,config_descp.wTotalLength); 
        free(config_buf);
        
        usbh_state = USBH_STM_CUSTOM_INITIALIZE; // FIXME

        // next : callback user's USBH_CustomClass_Initialize()
        USBH_CustomClass_Initialize(0);
				
        usbh_state = USBH_STM_CUSTOM_INITIALIZE_DONE; // FIXME
    }
    break;

    default:
#ifdef USBH_MDW_ERR
        kmdw_printf("@@ %s() error: invalid state\n", __FUNCTION__);
#endif
        break;
    }
}

static void usbh_mdw_thread(void *argument)
{
#ifdef USBH_MDW_DBG
    kmdw_printf("@@ %s(), start USBH middleware thread.....\n", __FUNCTION__);
#endif

    while (1)
    {
        uint32_t flags = osThreadFlagsWait(MDW_FLAG_MESSAGE | MDW_FLAG_ITD_WORK, osFlagsWaitAny, osWaitForever);

	      if (flags & MDW_FLAG_MESSAGE)
        {
            USBH_Event_t uevent;
            osStatus_t status = osMessageQueueGet(usbh_msgq, &uevent, NULL, osWaitForever); // wait for message
            if (status == osOK)
            {
                if (uevent.type == 0x11)
                    usbh_handle_port_evnet(uevent.port, uevent.event);

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

usbStatus USBH_Initialize(uint8_t ctrl)
{
#ifdef USBH_MDW_DBG
    kmdw_printf("@@ %s()\n", __FUNCTION__);
#endif

    // need a message queue
    usbh_msgq = osMessageQueueNew(USBH_EVENT_QUEUE_LEN, sizeof(USBH_Event_t), NULL);

#ifdef USBH_MDW_ERR
    if (usbh_msgq == NULL)
    {
        kmdw_printf("@@ osMessageQueueNew() failed\n");
    }
#endif

    Driver_USBH0.Initialize(&usbh_port_event_cb, &usbh_pipe_event_cb);
    Driver_USBH0.PowerControl(ARM_POWER_FULL);
    dev_pipetid[cur_pipe_num++].pipe = Driver_USBH0.PipeCreate(0x0, ARM_USB_SPEED_LOW, 0x0, 0x0, 0x0, 0x0, 8, 0);
    Driver_USBH0.PortVbusOnOff(0, true);

    usbh_state = USBH_STM_INITED;

    // first, we need a thread
    usbh_mdw_tid = osThreadNew(usbh_mdw_thread, NULL, NULL);
    osThreadSetPriority(usbh_mdw_tid, osPriorityHigh); // FIXME: what priority is proper ?

    return usbOK;
}

USBH_PIPE_HANDLE USBH_PipeCreate(uint8_t device, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_max_packet_size, uint8_t ep_interval)
{
    USBH_PIPE_HANDLE pipe_h;

    pipe_h = Driver_USBH0.PipeCreate(DEV_ADDR, ARM_USB_SPEED_HIGH, 0x0, 0x0, ep_addr, ep_type, ep_max_packet_size, ep_interval);

    dev_pipetid[cur_pipe_num++].pipe = pipe_h;

#ifdef USBH_MDW_DBG
    kmdw_printf("@@ %s() ep_addr 0x%x ep_type %d max_packet %d ep_interval %d, pipe 0x%p\n",
                __FUNCTION__, ep_addr, ep_type, ep_max_packet_size, ep_interval, pipe_h);
#endif

    return pipe_h;
}

static usbStatus _usbh_transfer_payload(USBH_PIPE_HANDLE pipe_hndl, uint8_t *buf, uint32_t len, bool isSend)
{
    // look up the pipe index from the pipetid table
    int i;
    for (i = 0; i < MAX_PIPE_NUM; i++)
        if (pipe_hndl == dev_pipetid[i].pipe)
        {
            dev_pipetid[i].thread = osThreadGetId(); // register thread id for thread flag notification
            break;
        }

#ifdef USBH_MDW_ERR
    if (i >= MAX_PIPE_NUM)
    {
        kmdw_printf("-- pipe 0x%p is invalid\n", pipe_hndl);
        return usbInvalidParameter;
    }
#endif

    int32_t sts = Driver_USBH0.PipeTransfer(pipe_hndl, isSend ? ARM_USBH_PACKET_OUT : ARM_USBH_PACKET_IN, buf, len);

    uint32_t flags = osThreadFlagsWait(USER_FLAG_XFER_COMPLETE, osFlagsWaitAny, 5000); // 5 secs timeout, should be long enough

#ifdef USBH_MDW_ERR
    if (flags == osFlagsErrorTimeout)
    {
        kmdw_printf("-- pipe 0x%p transfer timeout\n", pipe_hndl);
        return usbTimeout;
    }
#endif

#ifdef USBH_MDW_ERR
    if (sts != ARM_DRIVER_OK)
    {
        kmdw_printf("-- pipe 0x%p error, sts = %d\n", pipe_hndl, sts);
        return usbTransferError;
    }
#endif

    return usbOK;
}

usbStatus USBH_PipeSend(USBH_PIPE_HANDLE pipe_hndl, const uint8_t *buf, uint32_t len)
{
#ifdef USBH_MDW_DBG
    kmdw_printf("@@ %s() pipe_hndl 0x%p buf 0x%p len %d\n", __FUNCTION__, pipe_hndl, buf, len);
#endif

    return _usbh_transfer_payload(pipe_hndl, (uint8_t *)buf, len, true);
}

usbStatus USBH_PipeReceive(USBH_PIPE_HANDLE pipe_hndl, uint8_t *buf, uint32_t len)
{
#ifdef USBH_MDW_DBG
    kmdw_printf("@@ %s() pipe_hndl 0x%p buf 0x%p len %d\n", __FUNCTION__, pipe_hndl, buf, len);
#endif

    return _usbh_transfer_payload(pipe_hndl, buf, len, false);
}

static uint32_t _usbh_get_txfer_bytes(USBH_PIPE_HANDLE pipe_hndl)
{
    uint32_t txfer_bytes = Driver_USBH0.PipeTransferGetResult(pipe_hndl);

#ifdef USBH_MDW_DBG
    kmdw_printf("@@ pipe_hndl txfered %d bytes\n", pipe_hndl, txfer_bytes);
#endif

    return txfer_bytes;
}

uint32_t USBH_PipeSendGetResult(USBH_PIPE_HANDLE pipe_hndl)
{
    return _usbh_get_txfer_bytes(pipe_hndl);
}

uint32_t USBH_PipeReceiveGetResult(USBH_PIPE_HANDLE pipe_hndl)
{
    return _usbh_get_txfer_bytes(pipe_hndl);
}

usbStatus USBH_ControlTransfer(uint8_t device, const USB_SETUP_PACKET *setup_packet, uint8_t *data, uint32_t len)
{
#ifdef USBH_MDW_DBG
    kmdw_printf("@@ %s() device %d, setup_packet : 0x ", __FUNCTION__, device);
    uint8_t *sp = (uint8_t *)setup_packet;
    for (int i = 0; i < 8; i++)
        kmdw_printf("%02x ", sp[i]);
    kmdw_printf(", data ptr 0x%p, len %u\n", data, len);
#endif

    ARM_USBH_PIPE_HANDLE ctrl_pipe = dev_pipetid[0].pipe;
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

            packet = ARM_USBH_PACKET_DATA0 | ARM_USBH_PACKET_SETUP;
            payload = (uint8_t *)setup_packet;
            txfer_len = 8;

            break;
        case 1: // Data stage (optional)

            // skeip Data stage if wLength is 0
            if (setup_packet->wLength == 0)
                continue;

            // Contorl IN or OUT
            packet = ARM_USBH_PACKET_DATA1 |
                     ((setup_packet->bmRequestType.Dir) ? ARM_USBH_PACKET_IN : ARM_USBH_PACKET_OUT);
            payload = data;
            txfer_len = len;

            break;
        case 2: // Status stage

            packet = ARM_USBH_PACKET_DATA1 |
                     ((setup_packet->bmRequestType.Dir) ? ARM_USBH_PACKET_OUT : ARM_USBH_PACKET_IN);
            payload = NULL;
            txfer_len = 0;

            break;
        }

        Driver_USBH0.PipeTransfer(ctrl_pipe, packet, payload, txfer_len);

        uint32_t flags = osThreadFlagsWait(USER_FLAG_XFER_COMPLETE, osFlagsWaitAny, 5000); // 5 secs timeout, should be long enough
#ifdef USBH_MDW_ERR
        if (flags == osFlagsErrorTimeout)
        {
            kmdw_printf("@@ %s() control transfer timeout\n", __FUNCTION__);
            return usbTimeout;
        }
#endif
    }

    return usbOK;
}

USBH_PIPE_HANDLE USBH_Pipe_ISOCH_PipeDelete(USBH_PIPE_HANDLE pipe_hndl)
{
    Driver_USBH0.PipeDelete(pipe_hndl);
    return usbOK;
	
}

USBH_PIPE_HANDLE USBH_Pipe_ISOCH_PipeCreate(uint8_t device, uint8_t ep_addr, uint32_t wMaxPacketSize, uint8_t bInterval, uint8_t *buf, uint32_t buf_size)
{
    USBH_PIPE_HANDLE pipe_h;

    uint16_t max_packet_size = wMaxPacketSize & 0x7FF;
    uint8_t mult = (wMaxPacketSize >> 11) + 1;

    pipe_h = Driver_USBH0.PipeCreate_ISOCH(DEV_ADDR, ep_addr, max_packet_size, mult, bInterval, buf, buf_size);

    dev_pipetid[cur_pipe_num++].pipe = pipe_h;

#ifdef USBH_MDW_DBG
    kmdw_printf("@@ %s() ep_addr 0x%x ep_type 'isoch' wMaxPacketSize 0x%x bInterval %d buf 0x%p buf_size %u pipe 0x%p\n",
                __FUNCTION__, ep_addr, wMaxPacketSize, bInterval, buf, buf_size, pipe_h);
#endif

    return pipe_h;
}

static uint32_t handle_itd_cb()
{
    // wake up bottom-half thread
    osThreadFlagsSet(usbh_mdw_tid, MDW_FLAG_ITD_WORK);
    return 1;
}

usbStatus USBH_Pipe_ISOCH_Start(USBH_PIPE_HANDLE pipe_hndl, USBH_CB_ISR_Isoch_transfer user_isoch_cb)
{
    // enable bottom-half mechanism
    itd_work_func = Driver_USBH0.PipeEnableBH_ISOCH(pipe_hndl, handle_itd_cb);

    // start isoch transfer with user data callback
    Driver_USBH0.PipeStart_ISOCH(pipe_hndl, user_isoch_cb);

    return usbOK;
}

usbStatus USBH_Pipe_ISOCH_Stop(USBH_PIPE_HANDLE pipe_hndl)
{
    Driver_USBH0.PipeStop_ISOCH(pipe_hndl);
    return usbOK;
}

usbStatus USBH_DeviceRequest_SetInterface(uint8_t device, uint8_t index, uint8_t alternate)
{
    USB_SETUP_PACKET setup;

    setup.bmRequestType.Recipient = USB_REQUEST_TO_INTERFACE;
    setup.bmRequestType.Type = USB_REQUEST_STANDARD;
    setup.bmRequestType.Dir = USB_REQUEST_HOST_TO_DEVICE;
    setup.bRequest = USB_REQUEST_SET_INTERFACE;
    setup.wValue = alternate;
    setup.wIndex = index;
    setup.wLength = 0;

    return USBH_ControlTransfer(device, &setup, NULL, 0);
}

#endif
