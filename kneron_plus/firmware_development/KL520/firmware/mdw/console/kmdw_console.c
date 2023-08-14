#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "kmdw_console.h"
#include "project.h"

#define BACKSP_KEY 0x08
#define RETURN_KEY 0x0D
#define DELETE_KEY 0x7F
#define BELL 0x07

#define MAX_LOG_LENGTH 256
#define DDR_MAX_LOG_COUNT 1000 // if using DDR
#define DDR_LOG_BUFFER_SIZE (1 * 1024 * 1024)

static uint32_t scpu_debug_flags = 0;

static kdrv_uart_handle_t handle0 = MSG_PORT;
static osMessageQueueId_t log_msgq = NULL;

print_callback _print_callback = NULL;

static void _print_to_uart(const char *str)
{
    kdrv_uart_write(handle0, (uint8_t *)str, strlen(str));
}

kmdw_status_t kmdw_printf(const char *fmt, ...)
{
    va_list arg_ptr;
    static char *buffer_s = NULL;

    if (NULL == buffer_s)
        buffer_s = (char *)malloc(sizeof(char) * MAX_LOG_LENGTH);

    sprintf(buffer_s, "[%.03f] ", (float)osKernelGetTickCount() / osKernelGetTickFreq());
    int pre_len = strlen(buffer_s);

    va_start(arg_ptr, fmt);
    vsnprintf(buffer_s + pre_len, MAX_LOG_LENGTH - 1, fmt, arg_ptr);
    va_end(arg_ptr);

    buffer_s[MAX_LOG_LENGTH - 1] = 0; // just in case

    if (log_msgq == NULL || osThreadGetId() == NULL)
        _print_to_uart(buffer_s);
    else
    {
        osStatus_t oss = osMessageQueuePut(log_msgq, buffer_s, NULL, 0);
        if (oss != osOK)
        {
            //_print_to_uart("[logger] enqueue log1 failed\n");
            return KMDW_STATUS_ERROR;
        }
    }

    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_level_printf(int level, const char *fmt, ...)
{
    static char *buffer_s = NULL;
    uint32_t lvl = kmdw_console_get_log_level_scpu();
    lvl >>= 16;

    if ((level == LOG_PROFILE && level == lvl) || (level > 0 && level <= lvl))
    {
        va_list arg_ptr;

        if (NULL == buffer_s)
            buffer_s = (char *)malloc(sizeof(char) * MAX_LOG_LENGTH);

        sprintf(buffer_s, "[%.03f] ", (float)osKernelGetTickCount() / osKernelGetTickFreq());
        int pre_len = strlen(buffer_s);

        va_start(arg_ptr, fmt);
        vsnprintf(buffer_s + pre_len, MAX_LOG_LENGTH - 1, fmt, arg_ptr);
        va_end(arg_ptr);

        buffer_s[MAX_LOG_LENGTH - 1] = 0; // just in case

        if (log_msgq == NULL || osThreadGetId() == NULL)
            _print_to_uart(buffer_s);
        else
        {
            osStatus_t oss = osMessageQueuePut(log_msgq, buffer_s, NULL, 0);
            if (oss != osOK)
            {
                //_print_to_uart("[logger] enqueue log failed\n");
                return KMDW_STATUS_ERROR;
            }
        }
    }

    return KMDW_STATUS_OK;
}

void logger_thread(void *arg)
{
    void *log_pool = (void *)kmdw_ddr_reserve(DDR_LOG_BUFFER_SIZE);

    if (log_pool)
    {
        osMessageQueueAttr_t msgq_attr;
        memset(&msgq_attr, 0, sizeof(msgq_attr));
        msgq_attr.mq_mem = log_pool;
        msgq_attr.mq_size = DDR_LOG_BUFFER_SIZE;
        //_print_to_uart("[logger] Using DDR\n");

        log_msgq = osMessageQueueNew(DDR_MAX_LOG_COUNT, MAX_LOG_LENGTH, &msgq_attr);
        if (log_msgq == NULL)
        {
            printf("[logger] osMessageQueueNew failed\n");
        }
    }
    else
    {
        //_print_to_uart("[logger] Using HEAP\n");
    }

    uint8_t log[MAX_LOG_LENGTH];
    osThreadSetPriority(osThreadGetId(), osPriorityBelowNormal);

    while (1)
    {
        osStatus_t oss = osMessageQueueGet(log_msgq, &log[0], NULL, osWaitForever);
        if (oss != osOK)
        {
            _print_to_uart("[logger] dequeue log failed\n");
            // if (_print_callback)
            //     _print_callback("[logger] dequeue log failed\n");
            continue;
        }

        _print_to_uart((const char *)log);

        if (_print_callback)
            _print_callback((const char *)log);
    }
}

__weak uint32_t kmdw_ddr_reserve(uint32_t numbyte)
{
    return 0;
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

void kmdw_console_putc(char Ch)
{
    char cc;

    if (Ch != '\0')
    {
        cc = Ch;
        kdrv_uart_write(handle0, (uint8_t *)&cc, 1);
    }

    if (Ch == '\n')
    {
        cc = '\r';
        kdrv_uart_write(handle0, (uint8_t *)&cc, 1);
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
