#ifndef _KDEV_FLASH_NULL_H_
#define _KDEV_FLASH_NULL_H_

#include "Driver_Common.h"
#include "kdev_status.h"

typedef struct {
  uint32_t busy  : 1;                   /**< Flash busy flag */
  uint32_t error : 1;                   /**< Read/Program/Erase error flag (cleared on start of next operation) */
} kdev_flash_status_t;

bool bGigaDeive_Fseries=0;

__weak kdev_status_t kdev_flash_initialize(void)
{
	return KDEV_STATUS_OK;
}

__weak kdev_status_t kdev_flash_uninitialize(void)
{
	return KDEV_STATUS_OK;
}

__weak kdev_status_t kdev_flash_power_control(ARM_POWER_STATE state)
{
	return KDEV_STATUS_OK;
}

__weak kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt)
{
	return KDEV_STATUS_OK;
}

__weak void kdev_flash_read(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf)
{
}

__weak kdev_status_t kdev_flash_programdata (uint32_t addr, const void *data, uint32_t cnt)
{
	return KDEV_STATUS_OK;
}

__weak void kdev_flash_write(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf, uint32_t  buf_offset)
{
}

__weak kdev_status_t kdev_flash_programdata_memxfer(uint32_t addr, const void *data, uint32_t cnt)
{
	return KDEV_STATUS_OK;
}

__weak kdev_status_t kdev_flash_erase_sector(uint32_t addr)
{
	return KDEV_STATUS_OK;
}

__weak kdev_status_t kdev_flash_erase_multi_sector(uint32_t start_addr, uint32_t end_addr)
{
	return KDEV_STATUS_OK;
}

__weak kdev_status_t kdev_flash_erase_chip(void)
{
	return KDEV_STATUS_OK;
}

__weak kdev_flash_status_t kdev_flash_get_status(void)
{
    kdev_flash_status_t tt;
	return tt;
}

__weak kdev_status_t kdev_flash_get_info(void)
{
	return KDEV_STATUS_OK;
}

__weak void kdev_flash_128kErase(uint32_t  offset)
{
}

__weak kdev_status_t kdev_memxfer_flash_to_ddr(uint32_t dst, uint32_t src, size_t bytes, uint8_t mode)
{
	return KDEV_STATUS_OK;
}
#endif
