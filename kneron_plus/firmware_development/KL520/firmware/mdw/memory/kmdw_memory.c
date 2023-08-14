#include <string.h>
#include "kmdw_memory.h"
#include "kmdw_console.h"

/* ddr malloc direction : from tail(bigger address) to head (smaller) */

static uint32_t s_ddr_addr_tail = 0;
static uint32_t s_ddr_addr_boundary = 0;

static uint32_t s_ddr_system_reserve_addr = 0;
static uint32_t s_ddr_system_reserve_size = 0;

void kmdw_ddr_init(uint32_t start_addr, uint32_t end_addr)
{
    s_ddr_addr_boundary = start_addr; //(lower addr)  ex. 0x11100000
    s_ddr_addr_tail = end_addr;       //(higher addr) ex. 0x1111FFFF
}

int kmdw_ddr_set_ddr_boundary(uint32_t boundary)
{
    if (boundary >= s_ddr_addr_tail)
        return -1;
    else {
        s_ddr_addr_boundary = boundary;
        return 0;
    }
}


uint32_t kmdw_ddr_reserve(uint32_t numbyte)
{
    uint32_t aligned_numbyte;
    uint32_t tail_tmp;

    if(numbyte == 0)
        return 0;

    if(s_ddr_addr_boundary == 0)
        return 0; //not initialized yet

    aligned_numbyte = ALIGN16(numbyte);
    tail_tmp = s_ddr_addr_tail;
    tail_tmp = ALIGN16_FLOOR(tail_tmp - aligned_numbyte);

    if(tail_tmp <= s_ddr_addr_boundary)
    {
        err_msg("Failed DDR allocation: %8d(before aligned) bytes [ 0x%x(<=0x%x) : 0x%x]\n",
           numbyte, tail_tmp, s_ddr_addr_boundary, s_ddr_addr_tail);
        return 0;
    }
    else
    {
        dbg_msg("[DBG] DDR allocated: %8d [ *0x%x : 0x%x]\n",
            numbyte, tail_tmp, s_ddr_addr_tail);

        s_ddr_addr_tail = tail_tmp - 1;
        return tail_tmp; // aligned address
    }
}

uint32_t kmdw_ddr_get_heap_tail()
{
    return s_ddr_addr_tail;
}

void kmdw_ddr_store_system_reserve(uint32_t start_addr, uint32_t end_addr)
{
    s_ddr_system_reserve_addr = start_addr;
    s_ddr_system_reserve_size = (end_addr - start_addr + 1);
}

void kmdw_ddr_get_system_reserve(uint32_t *start_addr, uint32_t *ddr_size)
{
    *start_addr = s_ddr_system_reserve_addr;
    *ddr_size = s_ddr_system_reserve_size;
}
