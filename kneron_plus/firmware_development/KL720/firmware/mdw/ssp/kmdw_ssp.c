#include <cmsis_os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "io.h"

#include "kdrv_ssp.h"
#include "kdrv_clock.h"
#include "kmdw_console.h"
//-----------------------------------

typedef void (*fn_power_hook)(void);


fn_power_hook m_cb_ssp_SPI1_power_on = NULL;
fn_power_hook m_cb_ssp_SPI1_power_off = NULL;

extern struct st_ssp_spi	driver_ssp_ctx;
#if(SSP_SPI_MASTER_EN==COM_BUS_TYPE_SSP1 || SSP_SPI_MASTER_EN==COM_BUS_TYPE_SSP0)
extern struct st_ssp_spi driver_ssp_master_ctx;
#endif

//-----------------------------------
void kmdw_ssp_api_spi1_register_hook(fn_power_hook fn_power_on, fn_power_hook fn_power_off)
{
    m_cb_ssp_SPI1_power_on = fn_power_on;
    m_cb_ssp_SPI1_power_off = fn_power_off;
}

void ssp_spi1_power_on(void)
{
    // uint32_t data = 0;
    // data = inw(SCU_EXTREG_PA_BASE + 0x1C);
    // outw(SCU_EXTREG_PA_BASE + 0x1C, data | 0x40);
}

void ssp_spi1_power_off(void)
{
    // uint32_t data = 0;
    // data = inw(SCU_EXTREG_PA_BASE + 0x1C);
    // data &= ~(0x40);
    // outw(SCU_EXTREG_PA_BASE + 0x1C, data );
}

//---
uint8_t kmdw_ssp_api_spi_init(kdrv_ssp_spi_dev_id_t handle, enum e_spi edata, ARM_SPI_SignalEvent_t cb)
{
    if( kdrv_ssp_statemachine( handle, &driver_ssp_ctx, edata, cb ) == e_spi_ret_init_done ){
        return 1;
    }
    return 0;
}

uint8_t kmdw_ssp_api_spi_enable(kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *stspi)
{
    if( kdrv_ssp_statemachine( handle, stspi, e_spi_enable, NULL ) == e_spi_ret_enable_done ){
        return 1;
    }
    return 0;
}

uint8_t kmdw_ssp_api_spi_disable(kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *stspi)
{
    if( kdrv_ssp_statemachine( handle, stspi, e_spi_disable, NULL ) == e_spi_ret_disableDone ){
        return 1;
    }
    return 0;
}

uint8_t kmdw_ssp_api_spi_receive(kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *stspi)
{
    if( kdrv_ssp_statemachine( handle, stspi, e_spi_rx, NULL ) == e_spi_ret_rxbusy ){
        return 0;		//rx on-going
    }
    else{
        return 1;		//rx done
    }
}

uint8_t kmdw_ssp_api_spi_receive_xor(kdrv_ssp_spi_dev_id_t handle)
{

    if( kdrv_ssp_statemachine( handle, &driver_ssp_ctx, e_spi_rx_check, NULL ) == e_spi_ret_rx_xor_OK ){
            return 1;		//rx data correct
    }
    else{
            return 0;		//rx data fail
    }

}

uint8_t kmdw_ssp_api_spi_transfer(kdrv_ssp_spi_dev_id_t handle)
{

    if( kdrv_ssp_statemachine( handle, &driver_ssp_ctx, e_spi_tx, NULL ) == e_spi_ret_txbusy ){
            return 1;		//rx data correct
    }
    else{
            return 0;		//rx data fail
    }

}

uint8_t kmdw_ssp_api_spi_transfer_checks(kdrv_ssp_spi_dev_id_t handle)
{

    if( kdrv_ssp_statemachine( handle, &driver_ssp_ctx, e_spi_tx_status_check, NULL ) == e_spi_ret_txbusy )
    {
            return 0;
    }
    else
    {
            return 1;
    }

}

void kmdw_ssp_api_spi_write_tx_buff( uint8_t *src, uint16_t nlen )
{
    kdrv_ssp_write_buff( &driver_ssp_ctx, src, nlen );
}

uint8_t kmdw_ssp_api_spi1_tx_xor(kdrv_ssp_spi_dev_id_t handle)
{
    if( kdrv_ssp_statemachine( handle, &driver_ssp_ctx, e_spi_tx_xor, NULL ) == e_spi_ret_tx_xor_done )
    {
            return 0;
    }
    else
    {
            return 1;
    }
}
