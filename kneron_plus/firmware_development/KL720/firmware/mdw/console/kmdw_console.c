#include <string.h>
#include <stdarg.h>
#include "kmdw_console.h"
#include "kdrv_uart.h"
#include "project.h"     /*for MSG_PORT */
#include "ipc.h"
#include "cmsis_armcc.h"

#define BACKSP_KEY 0x08
#define RETURN_KEY 0x0D
#define DELETE_KEY 0x7F
#define BELL 0x07

#define MAX_LOG_LENGTH 256
#define DDR_MAX_LOG_COUNT 2000 // if using DDR
#define DDR_LOG_BUFFER_SIZE (1 * 1024 * 1024)

uint32_t scpu_debug_flags = 0;
osThreadId_t logger_tid = NULL;
osMutexId_t logger_mutex_id = NULL;

static kdrv_uart_handle_t handle0 = MSG_PORT;

print_callback _print_callback = NULL;
extern ncpu_to_scpu_result_t *in_comm_p;

logger_mgt_t logger_mgt;

void kmdw_level_printf(int level, const char *fmt, ...)
{
    uint32_t length;
    va_list arg_ptr;

    uint32_t lvl = kmdw_console_get_log_level_scpu();
    lvl >>= 16;

    if (!((level == LOG_PROFILE && level == lvl) || (level <= lvl))) return;

    if (logger_mgt.init_done == false)
    {
        char buffer[MAX_LOG_LENGTH];
        va_start(arg_ptr, fmt);
        length = vsnprintf((char*)buffer, MAX_LOG_LENGTH - 1, fmt, arg_ptr);
        if(length > MAX_LOG_LENGTH - 1) {
        	length = MAX_LOG_LENGTH - 1;
        }
        buffer[length] = '\0';
        va_end(arg_ptr);
        kdrv_uart_write(handle0, (uint8_t *)buffer, strlen(buffer));

        if (_print_callback)
            _print_callback((const char *)buffer);
    }
    else
    {
        //TODO:adjust logger_thread to low priority in task_handler.h
        //because it is not important

        //log Q full?
        while( ((logger_mgt.w_idx+1)%LOG_QUEUE_NUM) == logger_mgt.r_idx) {      //<-- ((idx+1)%LOG_QUEUE_NUM)

            //yes, Q full, must digest some logs
            //if Q full is often, must review system design(thread priority, resource, process)
            //to avoid this situation

            //TODO:
            //raise logger thread priority
            //osThreadSetPriority(logger_tid, osPriorityRealtime);

            //trigger logger thread to flush logs
            //if( logger_mgt.willing[LOGGER_OUT] == false) {      //<-- why need this?????
                osThreadFlagsSet(logger_tid, FLAG_LOGGER_SCPU_IN);

                //logger thread has highest priority,
                //it gets scheduled immediately after flags set

                //TODO: consider that log Q size is now 200(LOG_QUEUE_NUM)
                //TODO: flushing 200 logs takes very long time and may affect other processs
            //}

            //now, Q should be empty

            //TODO:
            //set priority back to low
            //osThreadSetPriority(logger_tid, osPriorityLow);
            }

        //Start protecting whole log process, it must be atomic process
        osMutexAcquire(logger_mutex_id, osWaitForever);

        //enter SCPU_IN state
        logger_mgt.willing[LOGGER_SCPU_IN] = true;

        //next, we worry that NCPU is also writting log Q in DDR

        //assume it is NCPU turn
        logger_mgt.turn = LOGGER_NCPU_IN;

        __ISB();    //avoid instruction reorder by compiler
        //check NCPU willing status and assume NCPU is writting log Q
        while( logger_mgt.willing[LOGGER_NCPU_IN] && logger_mgt.turn == LOGGER_NCPU_IN )
            ;

        //here, NCPU log done
        //start processing SCPU log

        //prepare timestamp
        sprintf((char *)(logger_mgt.p_msg + logger_mgt.w_idx * MAX_LOG_LENGTH), "[%.03f]", (float)osKernelGetTickCount() / osKernelGetTickFreq());
        int timelog_len = strlen((char *)(logger_mgt.p_msg + logger_mgt.w_idx * MAX_LOG_LENGTH));

        //prepare log string
        va_start(arg_ptr, fmt);
        length = vsnprintf((char *)(logger_mgt.p_msg + logger_mgt.w_idx * MAX_LOG_LENGTH + timelog_len), MAX_LOG_LENGTH - 1 - timelog_len, fmt, arg_ptr);
        if(length > MAX_LOG_LENGTH - 1) {
        	length = MAX_LOG_LENGTH - 1;
        }
        *((logger_mgt.p_msg + logger_mgt.w_idx * MAX_LOG_LENGTH) + length + timelog_len) = '\0';
        va_end(arg_ptr);

        //update write index, willing state
        logger_mgt.w_idx = (++logger_mgt.w_idx) % LOG_QUEUE_NUM;
        logger_mgt.willing[LOGGER_SCPU_IN] = false;

        //unprotect log process
        osMutexRelease(logger_mutex_id);

        //finally, trigger logger thread to flush logs
        //if( logger_mgt.willing[LOGGER_OUT] == false) {              //<-- why need this?????
            osThreadFlagsSet(logger_tid, FLAG_LOGGER_SCPU_IN);
        //}
    }
}

void kmdw_level_ipc_printf(int level, const char *fmt, ...)
{
    // note: this log function will be called in ISR, please be aware of the kdrv_uart_write conflict with logger_thread.
    uint16_t length;
    va_list arg_ptr;
    uint32_t lvl = kmdw_console_get_log_level_scpu();
    lvl >>= 16;

    if ((level == LOG_PROFILE && level == lvl) || (level > 0 && level <= lvl))
    {
        char buffer[MAX_LOG_LENGTH];
        va_start(arg_ptr, fmt);
        length = vsnprintf((char*)buffer, MAX_LOG_LENGTH - 1, fmt, arg_ptr);
        if(length > MAX_LOG_LENGTH - 1) {
        	length = MAX_LOG_LENGTH - 1;
        }
        buffer[length] = '\0';
        va_end(arg_ptr);
        kdrv_uart_write(handle0, (uint8_t *)buffer, strlen(buffer));
    }
}


void logger_thread(void *arg)
{
    uint32_t flags;
    osMutexAttr_t logger_mutex_attr;

    logger_mgt.w_idx = 0;
    logger_mgt.r_idx = 0;
    logger_mgt.willing[LOGGER_SCPU_IN] = false;
    logger_mgt.willing[LOGGER_NCPU_IN] = false;
    logger_mgt.willing[LOGGER_OUT] = false;
    logger_mgt.p_msg = (uint8_t *)kmdw_ddr_reserve(MAX_LOG_LENGTH * LOG_QUEUE_NUM);
    if( !logger_mgt.p_msg ) {
        printf("[logger] No enough moery reserved\n");
    }

    logger_tid = osThreadGetId();
    if (logger_tid == NULL)
    {
        printf("[logger] osThreadNew failed\n");
    }

    logger_mutex_attr.attr_bits = (osMutexPrioInherit|osMutexRobust);
    logger_mutex_attr.name = NULL;
    logger_mutex_attr.cb_mem = NULL;
    logger_mutex_attr.cb_size = 0;
    logger_mutex_id = osMutexNew(&logger_mutex_attr);

    //set init done flag after thread and mutex are both init done
    logger_mgt.init_done = true;

    while (1)
    {
        flags = osThreadFlagsWait(FLAG_LOGGER_SCPU_IN | FLAG_LOGGER_NCPU_IN, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(flags);

        uint8_t *p_str;
        logger_mgt.willing[LOGGER_OUT] = true;
        while( logger_mgt.r_idx != logger_mgt.w_idx )
        {
            p_str = (uint8_t *)(logger_mgt.p_msg + logger_mgt.r_idx * MAX_LOG_LENGTH);
            kdrv_uart_write(handle0, p_str, strlen((char *)p_str));

            if (_print_callback)
                _print_callback((const char *)p_str);

            logger_mgt.r_idx = ((++logger_mgt.r_idx) % LOG_QUEUE_NUM);
        }
        logger_mgt.willing[LOGGER_OUT] = false;
    }
}

void kmdw_console_hook_callback(print_callback print_cb)
{
    _print_callback = print_cb;
}

char kmdw_console_getc(void)
{
   char c;
   kdrv_uart_read(handle0, (uint8_t *)&c, 1);
   return c;
}

__weak uint32_t kmdw_ddr_reserve(uint32_t numbyte)
{
    return 0;
}

//This function is reserved for validation FW code
//because old validation FW still call this old console init function.
//SDK FW doesn't call this function.
kmdw_status_t kmdw_console_queue_init(void)
{
    osThreadAttr_t thread_attr;
    osMutexAttr_t logger_mutex_attr;

    memset(&thread_attr, 0, sizeof(thread_attr));
    thread_attr.stack_size = 1024;
    thread_attr.priority = osPriorityHigh;

    logger_tid = osThreadNew(logger_thread, NULL, &thread_attr);
    if (logger_tid == NULL)
    {
        printf("[logger] osThreadNew failed\n");
        return KMDW_STATUS_ERROR;
    }

    logger_mutex_attr.attr_bits = (osMutexPrioInherit|osMutexRobust);
    logger_mutex_attr.name = NULL;
    logger_mutex_attr.cb_mem = NULL;
    logger_mutex_attr.cb_size = 0;
    logger_mutex_id = osMutexNew(&logger_mutex_attr);

    return KMDW_STATUS_OK;
}

void kmdw_console_putc(char Ch)
{
    char cc;

    if (Ch != '\0')
    {
        cc = Ch;
        kdrv_uart_write(handle0, (uint8_t *)&cc, 1);

        if (_print_callback)
            _print_callback((const char *)&cc);
    }

    if (Ch == '\n')
    {
        cc = '\r';
        kdrv_uart_write(handle0, (uint8_t *)&cc, 1);

        if (_print_callback)
            _print_callback((const char *)&cc);
    }
}

void kmdw_console_puts(char *str)
{
    char *cp;
    for (cp = str; *cp != 0; cp++)
        kmdw_console_putc(*cp);
}

int kmdw_console_echo_gets(char *buf, int len)
{
    char *cp;
    char data;
    uint32_t count;
    count = 0;
    cp = buf;

    do
    {
        kdrv_uart_get_char(handle0, &data);
        switch (data)
        {
        case RETURN_KEY:
            if (count < len)
            {
                *cp = '\0';
                kmdw_console_putc('\n');
            }
            break;
        case BACKSP_KEY:
        case DELETE_KEY:
            if ((count > 0) && (count < len))
            {
                count--;
                *(--cp) = '\0';
                kmdw_console_puts("\b \b");
            }
            break;
        default:
            if (data > 0x1F && data < 0x7F && count < len)
            {
                *cp = (char)data;
                cp++;
                count++;
                kmdw_console_putc(data);
            }
            break;
        }
    } while (data != RETURN_KEY);

    return (count);
}
__weak void kdrv_ncpu_set_scpu_debug_lvl(uint32_t lvl)
{
}

__weak void kdrv_ncpu_set_ncpu_debug_lvl(uint32_t lvl)
{
}

void kmdw_console_set_log_level_scpu(uint32_t level)
{
    scpu_debug_flags = (scpu_debug_flags & ~0x000F0000) | (((level) << 16) & 0x000F0000);
    kdrv_ncpu_set_scpu_debug_lvl(level);
}

uint32_t kmdw_console_get_log_level_scpu(void)
{
    return scpu_debug_flags;
}

void kmdw_console_set_log_level_ncpu(uint32_t level)
{
    kdrv_ncpu_set_ncpu_debug_lvl(level);
}
