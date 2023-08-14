#ifdef USE_LOGGER
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "base.h"
#include "kdrv_log.h"
#include "kmdw_memory.h"
#include "kl520_include.h"

#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
        n = list_next_entry(pos, member);			\
        &pos->member != (head); 					\
        pos = n, n = list_next_entry(n, member))

struct node {
	struct node *next, *prev;
};

struct poollist
{
	struct node list_hook;
	int8_t cn_list;
};

struct pool_item
{
	struct node list_hook;
	void *item;
};

struct pool_info {
    struct node list_hook;
    int8_t len;
    char buf[LOG_BUF_SIZE];
};

typedef struct log_pool_t {
    struct poollist list;
    uint8_t w_idx;
    struct pool_info *p;
} log_pool_t;
static log_pool_t log_pool;

static osThreadId_t tid_log = NULL;
static kdrv_uart_handle_t uart_handle;
static bool en_logbuffer = FALSE;

/*******************************************************************************************************/
/*                  		                Private APIs		                                       */
/*******************************************************************************************************/ 
static inline void list_add(struct node *new_node, struct node *head)
{
    struct node *prev = head;
    struct node *next = head->next;

    next->prev = new_node;
	new_node->next = next;
	new_node->prev = prev;
	prev->next = new_node;    
}

static inline void list_del_init(struct node *entry)
{
    struct node *prev = entry->prev;
    struct node *next = entry->next;

    next->prev = prev;
	prev->next = next;
    entry->next = entry;
	entry->prev = entry;    
}

static void poollist_item_push(struct poollist* p, struct pool_item* pitem)
{
	list_add(&pitem->list_hook, &p->list_hook);
	++(p->cn_list);
}

static struct pool_item* poollist_item_pop(struct poollist* p)
{
	struct pool_item* pitem = NULL, * pnext;

	if (p->cn_list > 0)
	{
		list_for_each_entry_safe(pitem, pnext, &p->list_hook, list_hook)
		{
			--(p->cn_list);
			list_del_init(&pitem->list_hook);
			break;
		}
	}
	return pitem;
}
static bool kdrv_init_log_pool(void)
{
    if((log_pool.p = (struct pool_info *)kmdw_ddr_reserve(sizeof(struct pool_info)*LOG_BUF_SIZE)) != 0) {
        log_pool.w_idx = 0;
        return true;
    }
    return false;
}

static void kdrv_log_thread_running(void *argument)
{
    struct pool_info *p_info;
    
    for (;;) {
        osThreadFlagsWait(FLAGS_KDRV_PRINT_LOG_EVENTS, osFlagsWaitAny, osWaitForever);
        while((p_info = (struct pool_info *)poollist_item_pop(&log_pool.list)) != NULL)
    	{
    	    kdrv_uart_write(uart_handle, (uint8_t *)&p_info->buf[0], p_info->len);
        }
        continue;
    }
}

/*******************************************************************************************************/
/*                  		                   Public APIs		                                       */
/*******************************************************************************************************/
void kdrv_log_push(char *str)
{
    int8_t str_len = strlen(str);

    if(en_logbuffer == TRUE) {
        memcpy((void *)&log_pool.p[log_pool.w_idx].buf[0], (void *)str, str_len);
        log_pool.p[log_pool.w_idx].len = str_len;
        log_pool.p[log_pool.w_idx].buf[str_len] = '\0';
        poollist_item_push(&log_pool.list, (struct pool_item*)&log_pool.p[log_pool.w_idx]);
        if(log_pool.w_idx++ == LOG_BUF_NUM)
            log_pool.w_idx = 0;

        set_thread_event(tid_log, FLAGS_KDRV_PRINT_LOG_EVENTS);
    }
    else {
        kdrv_uart_write(uart_handle, (uint8_t *)str, str_len);
    }
}

kdrv_status_t kdrv_logger_thread_create(kdrv_uart_handle_t handle)
{
    if (tid_log != NULL)
        return KDRV_STATUS_ERROR;

    if(kdrv_init_log_pool() == false)
        return KDRV_STATUS_ERROR;

    if((tid_log = osThreadNew(kdrv_log_thread_running, NULL, NULL)) != NULL) {
        osThreadSetPriority (tid_log, osPriorityNormal);
        uart_handle = handle;
        en_logbuffer = TRUE;
        return KDRV_STATUS_OK;
    }
    return KDRV_STATUS_ERROR;
}
#endif
