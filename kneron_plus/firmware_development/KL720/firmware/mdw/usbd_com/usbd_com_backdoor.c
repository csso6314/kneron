#include "cmsis_os2.h"
#include "kdrv_usbd3.h"
#include "kmdw_console.h"
#include <stdlib.h>

#define USBD_ENDPOINT_DBG_RECV 0x03
#define USBD_ENDPOINT_DBG_SEND 0x84

#define TFLAG_STM_START 0x100

static uint32_t bulk_MaxPacketSize = 0;

#define CMD_ID_MEM_WRITE 0x1
#define CMD_ID_MEM_READ 0x2
#define CMD_ID_NCPU_RUN 0x3
#define CMD_ID_STOP_YOLO_EXEC 0x04
#define CMD_ID_NOTIFY_FW_INFO_LOADED 0x05
#define CMD_ID_RUN_JPEG_ENC 0x06
#define CMD_ID_RUN_JPEG_DEC 0x07
#define CMD_ID_RUN_CROP_RESIZE 0x08
#define CMD_ID_RUN_TOF_DEC 0x0A
#define CMD_ID_RUN_TOF_IR_AEC 0x0B



extern void dsp_ipc_send_dsp_interrupt(uint32_t cmd); // for CMD_ID_NCPU_RUN

static void dbg_mem_write(uint32_t address, uint32_t write_size)
{
    if (write_size == 0)
        return;

    uint32_t txfer_len = write_size;

    // check address region is memory or register
    if ((address >= SiRAM_MEM_BASE && address < (SiRAM_MEM_BASE + SiRAM_MEM_SIZE)) ||
        (address >= SdRAM_MEM_BASE && address < (SdRAM_MEM_BASE + SdRAM_MEM_SIZE)) ||
        (address >= NiRAM_MEM_BASE))
    {
        // memory-based region

        // receive data from host and directly write to specified address
        kdrv_usbd3_bulk_receive(USBD_ENDPOINT_DBG_RECV, (void *)address, &txfer_len, 0);

        // special handling for ZLP
        if ((txfer_len & (bulk_MaxPacketSize - 1)) == 0x0)
            kdrv_usbd3_bulk_receive_zlp(USBD_ENDPOINT_DBG_RECV);

        // error check
        if (txfer_len != write_size)
            kmdw_printf("[dbg] mem_write actual size mismatch %d vs %d !\n", txfer_len, write_size);
    }
    else
    {
        // register-based region

        uint32_t reg_buf[20]; // for registers write buffers
        if (write_size > (20 * 4))
        {
            kmdw_printf("[dbg] mem_write failed, write size too big ...\n");
            return;
        }

        // receive data from host through a temp buffer
        kdrv_usbd3_bulk_receive(USBD_ENDPOINT_DBG_RECV, (void *)reg_buf, &txfer_len, 0);

        // error check
        if (txfer_len != write_size)
            kmdw_printf("[dbg] mem_write actual size mismatch %d vs %d !\n", txfer_len, write_size);

        int count = write_size / 4;
        for (int i = 0; i < count; i++)
            *(volatile uint32_t *)(address + i * 4) = reg_buf[i];
    }

    kmdw_printf("[dbg] mem_write 0x%8X %d bytes done\n", address, txfer_len);
}

static void dbg_mem_read(uint32_t address, uint32_t read_size)
{
    if (read_size == 0)
        return;

    kdrv_usbd3_bulk_send(USBD_ENDPOINT_DBG_SEND, (void *)address, read_size, 0);
    kmdw_printf("[dbg] cmd_mem_read 0x%8X %d bytes done\n", address, read_size);
}

static void dbg_ncpu_run()
{
    dsp_ipc_send_dsp_interrupt(2); //CMD_RUN_NPU = 2
}

static void dbg_backdoor_thread(void *arg)
{
    // the begin of STM, FIXME
    osThreadFlagsWait(TFLAG_STM_START, osFlagsWaitAny, osWaitForever);

    bulk_MaxPacketSize = (kdrv_usbd3_get_link_speed() == USBD3_SUPER_SPEED) ? 1024 : 512;

    kmdw_printf("[dbg] ready !\n");

    uint32_t cmd_buf[3];

    while (1)
    {
        uint32_t txfer_len = sizeof(cmd_buf);

        // blocking waiting for 3 bytes cmd header
        if (kdrv_usbd3_bulk_receive(USBD_ENDPOINT_DBG_RECV, (void *)cmd_buf, &txfer_len, 0) != KDRV_STATUS_OK)
        {
            // FIXME
            kmdw_printf("[dbg] cmd receiving failed, terminated !\n");
            return;
        }

        // first 4 bytes present cmd ID, invoked corresonding work functions

        switch (cmd_buf[0])
        {
        case CMD_ID_MEM_WRITE:
            dbg_mem_write(cmd_buf[1], cmd_buf[2]);
            break;

        case CMD_ID_MEM_READ:
            dbg_mem_read(cmd_buf[1], cmd_buf[2]);
            break;

        case CMD_ID_RUN_YOLO3:
            dsp_ipc_send_dsp_interrupt(CMD_ID_RUN_YOLO3);

        case CMD_ID_STOP_YOLO_EXEC:
            npu_stop_yolo_exec();
            break;

        case CMD_ID_NOTIFY_FW_INFO_LOADED:
            fw_info_loaded();
            break;

        case CMD_ID_RUN_JPEG_ENC:
            dsp_ipc_send_dsp_interrupt(CMD_ID_RUN_JPEG_ENC); //CMD_JPEG_ENCODE = 4
            break;

        case CMD_ID_RUN_JPEG_DEC:
            dsp_ipc_send_dsp_interrupt(CMD_ID_RUN_JPEG_DEC); //CMD_JPEG_DECODE = 5
            break;

        case CMD_ID_RUN_CROP_RESIZE:
            dsp_ipc_send_dsp_interrupt(CMD_ID_RUN_CROP_RESIZE); //CMD_CROP_RESIZE = 6
            break;

        case CMD_ID_RUN_TOF_DEC:
            dsp_ipc_send_dsp_interrupt(CMD_ID_RUN_TOF_DEC); //CMD_TOF_DECODE = 7
            break;

        case CMD_ID_RUN_TOF_IR_AEC:
            dsp_ipc_send_dsp_interrupt(CMD_ID_RUN_TOF_IR_AEC); //CMD_TOF_CALC_IR_BRIGHT = 8
            break;
        
        default:
            kmdw_printf("[dbg] unknown command, terminated !\n");
            return;
        }
    }
}

osThreadId_t usbd_com_backdoor_init()
{
    return osThreadNew(dbg_backdoor_thread, NULL, NULL);
}
