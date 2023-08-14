// #define ENABLE_DBG_LOG

#include <string.h>
#include "cmsis_os2.h"
#include "kmdw_power_manager.h"

#include "kmdw_console.h"
#include "kdrv_power.h"
#include "kdrv_scu_ext.h"
#include "kdp_system.h"

#include "usbd_hal.h"

#include "buffer_object.h"

#include "kmdw_fifoq_manager.h"
#include "kdp2_usb_companion.h"
#include "kdp2_ipc_cmd.h"

#ifdef ENABLE_DBG_LOG
#define dbg_log(__format__, ...) kmdw_printf("[kp companion]"__format__, ##__VA_ARGS__)
#else
#define dbg_log(__format__, ...)
#endif

#define FLAG_WAIT_USB_CONNECTION 0x1

static osThreadId_t result_thread_id = NULL;
static osThreadId_t image_thread_id = NULL;

static bool _do_reset_queue = false;
static bool _enable_inf_droppable = false;

static bool allocate_memory_for_inference_queue(uint32_t image_count, uint32_t image_size, uint32_t result_count, uint32_t result_size)
{
    if (true == kmdw_fifoq_manager_get_fifoq_allocated())
        return false; // already inited

    uint32_t total_need_size = (image_count * image_size) + (result_count * result_size);

    kmdw_printf("allocating memory for fifoq: image %d x %d, result %d x %d, total %u bytes\n", image_count, image_size, result_count, result_size, total_need_size);

    uint32_t buf_addr = kmdw_ddr_reserve(total_need_size);
    if (buf_addr > 0)
    {
        dbg_log("allocated fifoq buffers OK\n");

        kmdw_fifoq_manager_store_fifoq_config(image_count, image_size, result_count, result_size);

        osStatus_t sts;

        // queue image and result buffers into queues correspondingly
        for (uint32_t i = 0; i < image_count; i++)
        {
            sts = kmdw_fifoq_manager_image_put_free_buffer(buf_addr, (int)image_size, 0);
            if (sts != osOK)
            {
                dbg_log("kmdw_fifoq_manager_image_put_free_buffer error = %d\n", sts);
            }

            buf_addr += image_size;
        }
        for (uint32_t i = 0; i < result_count; i++)
        {
            sts = kmdw_fifoq_manager_result_put_free_buffer(buf_addr, (int)result_size, 0);
            if (sts != osOK)
            {
                dbg_log("kmdw_fifoq_manager_image_put_free_buffer error = %d\n", sts);
            }

            buf_addr += result_size;
        }

        return true;
    }
    else
    {
        kmdw_printf("error ! not enough memory for inference queue buffers\n");
        return false;
    }
}

// usb link status notify
void usb_link_status_callback(usbd_hal_link_status_t link_status)
{
    switch (link_status)
    {
    case USBD_STATUS_DISCONNECTED:
        kmdw_printf("USB is disconnected\n");
        break;

    case USBD_STATUS_CONFIGURED:
        kmdw_printf("USB is connected\n");
        osThreadFlagsSet(image_thread_id, FLAG_WAIT_USB_CONNECTION);
        break;
    }
}

// vendor-specific control transfer setup packet notify
static bool usb_user_control_callback(usbd_hal_setup_packet_t *setup)
{
    bool ret = false;

    dbg_log("control bRequest = 0x%x\n", setup->bRequest);

    switch (setup->bRequest)
    {
    case KDP2_CONTROL_REBOOT:
    {
        dbg_log("control reboot\n");
        kdrv_power_sw_reset();
        break;
    }
    case KDP2_CONTROL_SHUTDOWN:
    {
        dbg_log("control shutdown\n");
        kmdw_power_manager_shutdown();
        break;
    }
    case KDP2_CONTROL_FIFOQ_RESET:
    {
        dbg_log("control fifoq reset\n");

        if (true == kmdw_fifoq_manager_get_fifoq_allocated())
        {
            _do_reset_queue = true;

            // also reset any inf defautls
            _enable_inf_droppable = false;

            usbd_hal_terminate_all_endpoint();
        }

        ret = true;
        break;
    }
    case KDP2_CONTROL_FIFOQ_CONFIGURE:
    {
        if (true == kmdw_fifoq_manager_get_fifoq_allocated())
            break; // already inited

        uint16_t arg1_image = setup->wValue;
        uint16_t arg2_result = setup->wIndex;

        uint32_t image_count = (arg1_image & 0x7) + 1;                         // lower 3 bits for number of image, 1~8
        uint32_t image_size = (10 * 1024) * (uint32_t)((arg1_image >> 3) + 1); // higher 13 bits for image buffer size in 10KB, 10KB~80MB

        uint32_t result_count = (arg2_result & 0x7) + 1;                         // lower 3 bits for number of image, 1~8
        uint32_t result_size = (10 * 1024) * (uint32_t)((arg2_result >> 3) + 1); // higher 13 bits for image buffer size in 10KB, 10KB~80MB

        ret = allocate_memory_for_inference_queue(image_count, image_size, result_count, result_size);

        break;
    }
    case KDP2_CONTROL_FIFOQ_ENABLE_DROPPABLE:
    {
        _enable_inf_droppable = (setup->wValue == 1);
        ret = true;
        break;
    }
    case KDP2_CONTROL_DDR_HEAP_BOUNDARY_ADJUST:
    {
        uint32_t arg1 = (uint32_t)setup->wValue;
        uint32_t arg2 = (uint32_t)setup->wIndex;
        uint32_t boundary_addr = (arg1 << 16) | arg2;

        ret = (0 == kmdw_ddr_set_ddr_boundary(boundary_addr));
        break;
    }
    case KDP2_CONTROL_REBOOT_SYSTEM:
    {
        dbg_log("control reboot system\n");
        kdrv_power_sw_reset();
        break;
    }

    default:
        ret = false;
        break;
    }

    return ret;
}

static void process_plus_command(uint32_t cmd_buf)
{
    kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)cmd_buf;

    if (header_stamp->magic_type == KDP2_MAGIC_TYPE_COMMAND)
    {
        dbg_log("handle kdp2 command = 0x%x\n", header_stamp->job_id);
        // handle kdp2 commands ...
        kdp2_cmd_handle_kp_command(cmd_buf);
    }
    else if ((header_stamp->magic_type & 0xFFFF) == KDP_MSG_HDR_CMD) // very speical case for old arch. fw update
    {
        // handle legendary kdp commands, should be as few as possible
        dbg_log("handle legendary kdp command = 0x%x\n", header_stamp->job_id);
        kdp2_cmd_handle_legend_kdp_command(cmd_buf);
    }
    else
    {
        dbg_log("[%s] error ! received un-recognized buffer beginning with incorrect magic_type 0x%x\n", __FUNCTION__, header_stamp->magic_type);
    }
}

enum
{
    RECV_TYPE_INF_IMAGE = 1,
    RECV_TYPE_COMMAND = 2,
    RECV_TYPE_UNKNOWN = 3,
};

void kdp2_usb_companion_image_thread(void *arg)
{
    dbg_log("[%s] starting ..\n", __FUNCTION__);

    uint32_t temp_cmd_buffer = 0;
    uint32_t temp_cmd_buffer_size = 0;

    kmdw_ddr_get_system_reserve(&temp_cmd_buffer, &temp_cmd_buffer_size);

    // this is due to "static thread allocation", refer to task_handler.h
    image_thread_id = osThreadGetId();
    if (image_thread_id == NULL)
        kmdw_printf("%s creation failed !\n", __FUNCTION__);

    // wait until usb connection is established
    osThreadFlagsWait(FLAG_WAIT_USB_CONNECTION, osFlagsWaitAny, osWaitForever);

    // run infinitely
    while (1)
    {
        uint32_t buf_addr; // contains a inference image or a command
        int buf_size;      // buffer size should bigger than inference image size

        // take a free buffer to receive a inf image or a command
        if (true == kmdw_fifoq_manager_get_fifoq_allocated()) {
            osStatus_t sts = kmdw_fifoq_manager_image_get_free_buffer(&buf_addr, &buf_size, osWaitForever, _enable_inf_droppable);

            while (_enable_inf_droppable && osErrorResource == sts)
            {
                sts = kmdw_fifoq_manager_image_get_free_buffer(&buf_addr, &buf_size, osWaitForever, _enable_inf_droppable);
            }
        } else {
            buf_addr = temp_cmd_buffer;
            buf_size = temp_cmd_buffer_size;
            dbg_log("[%s] got system reserve buffer: 0x%X size %d\n", __FUNCTION__, buf_addr, buf_size);
        }

        dbg_log("[%s] free queue --> buf 0x%X size %d\n", __FUNCTION__, buf_addr, buf_size);

        uint32_t total_recv_len = 0;
        uint32_t total_wanted_len = 0;
        int recv_type = RECV_TYPE_UNKNOWN;
        uint32_t total_image_count = 0;
        uint32_t image_index = 0;

        // loop done when receiving size-matched data
        while (1)
        {
            uint32_t txLen = buf_size - total_recv_len;
            kdrv_status_t usb_sts = usbd_hal_bulk_receive(KDP2_USB_ENDPOINT_DATA_OUT, (uint32_t *)(buf_addr + total_recv_len), &txLen, osWaitForever);
            if (usb_sts != KDRV_STATUS_OK) // KDRV_STATUS_USBD_TRANSFER_TERMINATED or KDRV_STATUS_USBD_TRANSFER_DISCONNECTED
            {
                dbg_log("[%s] bulk receive is terminated, sts %d\n", __FUNCTION__, usb_sts);

                // guess this is good time to clear/reset queue
                if (_do_reset_queue)
                {
                    dbg_log("[%s] do reset fifo queue !!!\n", __FUNCTION__);

                    _do_reset_queue = false;

                    // abandon all unprocessed data
                    kmdw_fifoq_manager_clean_queues();

                    // sending enpoint may still hold a buffer, terminate it
                    usbd_hal_terminate_all_endpoint();

                    recv_type = RECV_TYPE_UNKNOWN;

                    break;
                }
            }

            if (total_recv_len == 0) // buffer should begin with header stamp 'magic_type'
            {
                kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)buf_addr;

                if (header_stamp->magic_type == KDP2_MAGIC_TYPE_INFERENCE)
                {
                    recv_type = RECV_TYPE_INF_IMAGE; // indicate this buffer contains inf image
                    total_wanted_len = header_stamp->total_size;
                    total_image_count = header_stamp->total_image;
                    image_index = header_stamp->image_index;

                    if ((buf_addr == temp_cmd_buffer) && (true == kmdw_fifoq_manager_get_fifoq_allocated())) {
                        osStatus_t sts = kmdw_fifoq_manager_image_get_free_buffer(&buf_addr, &buf_size, osWaitForever, _enable_inf_droppable);

                        while (_enable_inf_droppable && osErrorResource == sts) {
                            sts = kmdw_fifoq_manager_image_get_free_buffer(&buf_addr, &buf_size, osWaitForever, _enable_inf_droppable);
                        }

                        memcpy((void *)buf_addr, (void *)temp_cmd_buffer, txLen);

                        dbg_log("[%s] swap data from system reserve buffer to fifoq buffer: 0x%X\n", __FUNCTION__, buf_addr);
                    }

                    if (header_stamp->total_size > buf_size) {
                        // FIXME, serious error, make host SW pending (timeout)
                        // reboot and configure bigger buffer !!!!
                        kmdw_printf("[%s] error !! inf image size (%d) is bigger than buffer size (%d)\n", __FUNCTION__, total_wanted_len, buf_size);
                        return; // a way to inform host SW ?
                    }
                }
                else
                {
                    recv_type = RECV_TYPE_COMMAND; // indicate this buffer contains a command (if no failuire)
                    total_wanted_len = txLen;      // assume this is a command (or not if failure)
                }
            }
            else
            {
                // extra check for inference image data
                kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)(buf_addr + total_recv_len);
                if (header_stamp->magic_type == KDP2_MAGIC_TYPE_COMMAND ||
                    header_stamp->magic_type == KDP2_MAGIC_TYPE_COMMAND)
                {
                    // this should not happen
                    kmdw_printf("[%s] error !! receiving inf image corrupted ! start over.\n", __FUNCTION__);
                    recv_type = RECV_TYPE_UNKNOWN;
                }
            }

            total_recv_len += txLen;

            dbg_log("[%s] host -- usb --> buf 0x%x len %d (total %d)\n", __FUNCTION__, (void *)buf_addr, txLen, total_recv_len);

            if (total_recv_len == total_wanted_len)
                break; // recv done
            else if (total_recv_len > total_wanted_len)
            {
                kmdw_printf("[%s] warning !! actual received size (%d) is bigger than expected size (%d)\n", __FUNCTION__, total_recv_len, total_wanted_len);
                break;
            }
        }

        if (recv_type == RECV_TYPE_INF_IMAGE)
        {
            dbg_log("[%s] buf 0x%x -- > inference queue\n", __FUNCTION__, (void *)buf_addr);
            kmdw_fifoq_manager_image_enqueue(total_image_count, image_index, buf_addr, buf_size, osWaitForever, false);
        }
        else if (recv_type == RECV_TYPE_COMMAND)
        {
            dbg_log("[%s] buf 0x%x -- > command handler\n", __FUNCTION__, (void *)buf_addr);
            process_plus_command(buf_addr);
            // as the buffer is not put into inference queue, return it back to free buffer queue
            if (temp_cmd_buffer != buf_addr)
                kmdw_fifoq_manager_image_put_free_buffer(buf_addr, buf_size, osWaitForever);
        }
        else
        {
            // invalid data !! ignore it !
            dbg_log("[%s] XXXXXX buf 0x%x -- > free buffer queue\n", __FUNCTION__, (void *)buf_addr);
            if (temp_cmd_buffer != buf_addr)
                kmdw_fifoq_manager_image_put_free_buffer(buf_addr, buf_size, osWaitForever);
        }
    }
}

void kdp2_usb_companion_result_thread(void *arg)
{
    bool bRunning_dbg = false;

    dbg_log("[%s] starting ..\n", __FUNCTION__);

    result_thread_id = osThreadGetId();
    if (result_thread_id == NULL)
        kmdw_printf("%s creation failed !\n", __FUNCTION__);

    while (1)
    {
        uint32_t buf_addr;
        int buf_size;

        // get result data from queue blocking wait
        kmdw_fifoq_manager_result_dequeue(&buf_addr, &buf_size, osWaitForever);

        kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)buf_addr;

        dbg_log("[%s] buf 0x%x len %d -- usb --> host\n", __FUNCTION__, (void *)buf_addr, header_stamp->total_size);

        // kp inference debug code
        if(bRunning_dbg && header_stamp->magic_type != KDP2_MAGIC_TYPE_CHECKPOINT_DATA)
        {
            // send a signal notify the end of dbg process
            kp_inference_header_stamp_t dbg_ending;
            dbg_ending.magic_type = KDP2_MAGIC_TYPE_CHECKPOINT_DATA;
            dbg_ending.total_size = sizeof(kp_inference_header_stamp_t);
            dbg_ending.job_id = 0;
            dbg_ending.status_code = KP_SUCCESS;
            usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&dbg_ending, dbg_ending.total_size, osWaitForever);
            bRunning_dbg = false;
        }

        // send result to the host, blocking wait
        kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)buf_addr, header_stamp->total_size, osWaitForever);

        if (usb_sts != KDRV_STATUS_OK) // KDRV_STATUS_USBD_TRANSFER_TERMINATED or KDRV_STATUS_USBD_TRANSFER_DISCONNECTED
        {
            dbg_log("[%s] bulk send is terminated, sts %d\n", __FUNCTION__, usb_sts);
        }

        // kp inference debug code
        if(header_stamp->magic_type == KDP2_MAGIC_TYPE_CHECKPOINT_DATA)
        {
            bRunning_dbg = true;
            header_stamp->status_code = 1; // to let ncpu go on
            continue;
        }

#ifdef AUTOTEST /* CI_PACK_REMOVE_START */
        if(header_stamp->magic_type == KDP2_MAGIC_TYPE_JSON)
        {
            // FIXME: figure out a better way to find offset from kdp2_ipc_json_result_t
            uint32_t offset = *(uint32_t *)(buf_addr + sizeof(kp_inference_header_stamp_t) + 8);
            buf_addr -= offset;
        }
#endif /* CI_PACK_REMOVE_END */

        // return free buf back to queue
        kmdw_fifoq_manager_result_put_free_buffer(buf_addr, buf_size, osWaitForever);
    }
}

////////////////////////////////////////////////////////////
// for usb-boot, flash-boot, jtag-boot, kdp2 loader

#define JTAG_MAGIC_ADDRESS 0x10100000
#define CONTROL_BYPASS_ADDRESS 0x10100004
#define KDP2_BOOT_CONFIG_ADDRESS 0x10100100
#define RECOVERY_MARK_POS (SdRAM_MEM_BASE + SdRAM_MEM_SIZE - 64)

#define JTAG_MAGIC_VALUE 0xFEDCBA01
#define BOOT_FROM_FLASH 0xA

typedef struct
{
    uint32_t boot_type;     // 0xA = flash-boot, others = usb-boot
    uint8_t loader_ver[4];  // fw loader version numbers
    uint8_t scpu_fw_ver[4]; // SCPU fw version numbers
    uint8_t ncpu_fw_ver[4]; // NCPU fw version numbers
} kdp2_boot_config_t;

////////////////////////////////////////////////////////////

int kdp2_usb_companion_init()
{
    // retrieve real serial number here from efuse
    // then convert it to hex string format

    uint32_t uid = 0;

    uid = kdp_sys_get_kn_number();

    int32_t sidx = 0;
    uint8_t kn_num_string[32] = {0};
    for (int i = 7; i >= 0; i--)
    {
        uint32_t hex = (uid >> i * 4) & 0xF;
        kn_num_string[sidx] = (hex < 10) ? '0' + hex : 'A' + (hex - 10);
        sidx += 2;
    }

    // Companion Mode
    uint16_t bcdDevice = KP_KDP2_FW_COMPANION_MODE;

    if (*((uint32_t *)JTAG_MAGIC_ADDRESS) == JTAG_MAGIC_VALUE)
    {
        kmdw_printf("FW is running in JTAG mode\n");
        bcdDevice |= KP_KDP2_FW_JTAG_TYPE;
    }
    else
    {
        kdp2_boot_config_t *bConfig = (kdp2_boot_config_t *)KDP2_BOOT_CONFIG_ADDRESS;
        if (bConfig->boot_type == BOOT_FROM_FLASH)
        {
            kmdw_printf("KDP2 FW is running in flash-boot mode\n");
            bcdDevice |= KP_KDP2_FW_FLASH_TYPE;

            kmdw_printf("boot ncpu fw from flash\n");
            SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1); // run ncpu
        }
        else
        {
            kmdw_printf("KDP2 FW is running in usb-boot mode\n");
            bcdDevice |= KP_KDP2_FW_USB_TYPE;
        }
    }

    // this is about recovery mode
    *(uint32_t *)RECOVERY_MARK_POS = 0;

    usbd_hal_initialize(kn_num_string, bcdDevice, usb_link_status_callback, usb_user_control_callback);
    usbd_hal_set_enable(true);

    // wow ! fifoq can also handle command
    kdp2_cmd_handler_initialize();

#ifdef FIFIOQ_LOG_VIA_USB
    kdp2_usb_log_initialize();
#endif

    return 0;
}
