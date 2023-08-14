#include <string.h>

#include "kdrv_ssp.h"
#include "kdrv_clock.h"
#include "kdrv_gpio.h"
#include "kmdw_memory.h"
#include "io.h"
//#include "ARMCM4_FP.h"

//#define SSP_WAIT_REMOVE
#define SPI_Buffer_size     (0x00001400)

//uint8_t   *gRx_buff_SP_SLAVE = (uint8_t *)KDP_DDR_DRV_COM_BUS_RX0_START_ADDR ;          //(uint8_t *)KDP_DDR_DRV_SSP1_RX0_START_ADDR;
//volatile uint8_t  *gTx_buff_SP_SLAVE = (uint8_t *)KDP_DDR_DRV_COM_BUS_TX_START_ADDR;    //(uint8_t *)KDP_DDR_DRV_SSP1_TX_START_ADDR;
uint8_t   gRx_buff_temp[300];

uint32_t  gTx_buff_index_SP_SLAVE = 0;    //data length
uint32_t  gTx_buff_current_index_SP_SLAVE = 0 ;
uint32_t  gRx_buff_index_SP_SLAVE = 0;    //data length


struct st_ssp_spi driver_ssp_ctx;


#if(SSP_SPI_MASTER_DEV==COM_BUS_TYPE_SSP1 || SSP_SPI_MASTER_DEV==COM_BUS_TYPE_SSP0)

#define SSP_MASTER_BUFFER   (1024)
uint8_t   gTx_buff_SP_MASTER[SSP_MASTER_BUFFER];
uint8_t   gRx_buff_SP_MASTER[SSP_MASTER_BUFFER];
uint32_t  gTx_buff_index_SP_MASTER = 0;    //data length
uint32_t  gTx_buff_current_index_SP_MASTER = 0 ;
uint32_t  gRx_buff_index_SP_MASTER = 0;    //data length
struct st_ssp_spi driver_ssp_master_ctx;
#endif

static void SPI0_ISR(void);
static void SPI1_ISR(void);
static void kdrv_ssp_spi_irqhandler(kdrv_ssp_spi_dev_id_t handle);

IRQn_Type gSPIIRQTbl[2] = {
    SSP_FTSSP010_0_1_IRQ,   //SPI0
    SSP_FTSSP010_1_1_IRQ,   //SPI1
};

kdrev_ssp_spi_isr_t gSPIISRs[2] = {
    SPI0_ISR,
    SPI1_ISR
};

uint32_t sdl_in_bytes = 16;		//1; //12;
uint32_t pcl = 0; //25; //8; //*0*/8;

static void SPI0_ISR(void)
{
    kdrv_ssp_spi_irqhandler(SSP_SPI_PORT0);
}

static void SPI1_ISR(void)
{
    kdrv_ssp_spi_irqhandler(SSP_SPI_PORT1);
}

static void kdrv_ssp_set_sclkdiv(kdrv_ssp_spi_dev_id_t handle, uint32_t sclkdiv)
{
    uint32_t cr1;

    cr1 = regSSP0_ctrl(handle)->st.dw.kdrv_ssp_sspcr1;
    cr1 &= ~ssp_CR1_SCLKDIV_MASK;
    cr1 |= ssp_CR1_SCLKDIV(sclkdiv);

    regSSP0_ctrl(handle)->st.dw.kdrv_ssp_sspcr1 = cr1;

}

static void kdrv_ssp_reset(kdrv_ssp_spi_dev_id_t handle)
{
    regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.SSPRST = 1;
}

static void kdrv_ssp_clear_txhw(uint32_t port)
{
    regSSP0_ctrl(port)->st.bf.kdrv_ssp_sspcr2.TXFCLR = 1;
}

static void kdrv_ssp_clear_rxhw(kdrv_ssp_spi_dev_id_t handle)
{
    regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.RXFCLR = 1;
}
/**
 * CR1.SDL has 7 bits means maximum: 2^7 = 127 + 1 bits.
 * 128 bits = 16 bytes
 */
static uint32_t kdrv_ssp_set_data_length(kdrv_ssp_spi_dev_id_t handle, uint32_t sdl)
{
    uint32_t cr1;

    cr1 = regSSP0_ctrl(handle)->st.dw.kdrv_ssp_sspcr1;
    cr1 &= ~ssp_CR1_SDL_MASK;

    cr1 |= ssp_CR1_SDL(sdl);

    regSSP0_ctrl(handle)->st.dw.kdrv_ssp_sspcr1 = cr1;

    return 0;
}

static void kdrv_ssp_spi_enable(kdrv_ssp_spi_dev_id_t handle, uint32_t tx, uint32_t rx)
{
    //uint32_t cr2 = 0;

    if( tx == 0 )
    {
        regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.SSPEN = 0;
        regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.TXDOE = 0; 
    }

    if( rx == 0 )
    {
        regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.SSPEN = 0;
        regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.TXDOE = 0; 
    }

    if (tx || rx)
    {
        regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.SSPEN = 1;
        regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.TXDOE = 1;

        if (tx)
            regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.TXEN = 1;

        if (rx)
            regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspcr2.RXEN = 1;
    }
}

void kdrv_ssp_enable_fs(kdrv_ssp_spi_dev_id_t handle, uint32_t tx, uint32_t rx, uint32_t fs)
{
    uint32_t cr2 = 0;

    if (tx || rx)
        cr2 = (ssp_CR2_SSPEN | ssp_CR2_TXDOE);

    if (tx)
        cr2 |= ssp_CR2_TXEN;

    if (rx)
        cr2 |= ssp_CR2_RXEN;
    
    if (fs)
        cr2 |= ssp_CR2_FS;

    regSSP0_ctrl(handle)->st.dw.kdrv_ssp_sspcr2 = cr2;

}

/**
 * Return the number of entries TX FIFO can be written to
 */
static uint32_t kdrv_ssp_txfifo_depth( uint32_t port )
{
    uint32_t depth;

    depth = ssp_FEA_TXFIFO_DEPTH(
            regSSP0_feature(port)->st.dw.kdrv_ssp_sspfeature);
    depth += 1;
    return depth;
}

static uint32_t kdrv_ssp_txfifo_not_full(uint32_t port)	//0: full, 1: not full
{
    return regSSP0_ctrl(port)->st.bf.kdrv_ssp_sspstatus.TFNF;
}


/**
 * Return the number of entries RX FIFO can be written to
 */
static uint32_t kdrv_ssp_rxfifo_depth(uint32_t port)
{
    uint32_t depth;

    depth = ssp_FEA_RXFIFO_DEPTH(
            regSSP0_feature(port)->st.dw.kdrv_ssp_sspfeature);
    depth += 1;
    return depth;
}

uint32_t kdrv_ssp_rxfifo_full( kdrv_ssp_spi_dev_id_t handle )	//1:full, 0: not full
{
    return regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspstatus.RFF;
}

static void kdrv_ssp_spi_set_interrupt( kdrv_ssp_spi_dev_id_t handle, uint8_t nval)
{
    uint32_t data = 0;

    data = regSSP0_ctrl(handle)->st.dw.kdrv_ssp_intrcr;
    if( (nval & 0x10 ) == 0x10 ) //DMA_RX
    {
        data &= ~(0x10);
        data |= (0x10);
    }

    if( (nval & 0x08) == 0x08 )
    {
        data &= ~(0x08);
        data |= (0x08);
    }

    if( (nval & 0x04 ) == 0x04 )
    {
        data &= ~(0x04);
        data |= (0x04);
    }
    regSSP0_ctrl(handle)->st.dw.kdrv_ssp_intrcr = data;
}



static uint32_t kdrv_ssp_get_rxfifo_int_thflag( kdrv_ssp_spi_dev_id_t handle)
{
    uint32_t ent;

    ent = regSSP0_ctrl(handle)->st.bf.kdrv_ssp_intrstatus.RFTHI;
    return ent;
}


static uint32_t kdrv_ssp_get_txfifo_int_thflag( kdrv_ssp_spi_dev_id_t handle )
{
    uint32_t ent;

    ent = regSSP0_ctrl(handle)->st.bf.kdrv_ssp_intrstatus.TFTHI;
    return ent;
}

static void kdrv_ssp_write_word(uint32_t port, const void *data, uint32_t wsize)
{
    uint32_t tmp = 0;

    if (data) {
        switch (wsize) {
        case 1:
            tmp = *(const uint8_t *)data;
            break;

        case 2:
            tmp = *(const uint16_t *)data;
            break;

        default:
            tmp = *(const uint32_t *)data;
            break;
        }
    }

    regSSP0_ctrl(port)->st.dw.kdrv_ssp_txrxdr = tmp;
}

static void kdrv_ssp_read_word_new( uint32_t port, volatile uint8_t *buf )
{
    uint32_t data = regSSP0_ctrl(port)->st.dw.kdrv_ssp_txrxdr;
    *buf = data;
    return;
}

static void kdrv_ssp_read_word(uint32_t port, void *buf, uint32_t wsize)
{
    uint32_t data = regSSP0_ctrl(port)->st.dw.kdrv_ssp_txrxdr;

    if (buf) {
        switch (wsize) {
        case 1:
            *(uint8_t *) buf = data;
            break;

        case 2:
            *(uint16_t *) buf = data;
            break;

        default:
            *(uint32_t *) buf = data;
            break;
        }
    }
}

/**
 * len unit is bytes
 *
 * Return number of fifo written.
 */
uint32_t kdrv_ssp_fill_in_fifo(uint32_t tx_addr, const void *buf,
              uint32_t rx_addr, uint32_t *len)
{
    uint32_t count = 0;
    uint32_t rxfifo, fifo, wsize, i;


    rxfifo = kdrv_ssp_rxfifo_depth(rx_addr);	//get Rx FIOF level


    //clear before start filling in
    kdrv_ssp_clear_txhw(tx_addr);

    i = 0;
    while (kdrv_ssp_txfifo_not_full(tx_addr))
    {

        if (!*len || !buf)
            break;

        if (i == 0) {
            i = sdl_in_bytes;
            fifo = (sdl_in_bytes + 3) / 4;
        }

        //rx fifo doesn't have enough entries to receive
        if ((count + fifo) > rxfifo)
            break;

        if (i > 3)
            wsize = 4;
        else
            wsize = i;

        kdrv_ssp_write_word(tx_addr, buf, wsize);

        i -= wsize;
        *len -= wsize;
        fifo--;
        count++;
        // Always add 4 bytes for buffer pointer
        (*(int8_t*)buf) += 4;
    }

    return count;
}

/**
 * count is the number of FIFO entries wanted to be read.
 *
 * Return the remaining number of fifo not read yet.
 */
uint32_t kdrv_ssp_take_out_fifo(uint32_t rx_addr, void *buf, uint32_t count)
{
    uint32_t i, wsize;

    i = 0;
    while (count) {

        if (!buf)
            break;

        while (!kdrv_ssp_rxfifo_valid_entries(rx_addr)) {
            ;
        }

        if (i == 0)
            i = sdl_in_bytes;

        if (i > 3)
            wsize = 4;
        else
            wsize = i;

        kdrv_ssp_read_word(rx_addr, buf, wsize);

        i -= wsize;
        count--;
        (*(int8_t*)buf) += 4;

    }

    return count;
}

static void kdrv_ssp_write_byte_new(uint32_t base, volatile uint8_t *data)
{
    regSSP0_ctrl(base)->st.dw.kdrv_ssp_txrxdr = *data;
}

static uint32_t kdrv_ssp_set_pcl(uint32_t base, uint32_t val)
{
    uint32_t cr3;

    if (val & ~ssp_CR3_PCL_MASK) {
        return 1;
    }

    cr3 = regSSP0_ctrl(base)->st.dw.kdrv_ssp_sspcr3;
    cr3 &= ~ssp_CR3_PCL_MASK;
    cr3 |= ssp_CR3_PCL(val);

    regSSP0_ctrl(base)->st.dw.kdrv_ssp_sspcr3=cr3;

    return 0;
}

static void kdrv_ssp_spi_tx_fifo_threshold(uint32_t nbase , uint8_t nval)
{
    uint32_t	ntemp ;

    ntemp = regSSP0_ctrl(nbase)->st.dw.kdrv_ssp_intrcr;

    ntemp &= ~ssp_INTCR_TFTHOD_MASK;

    ntemp = ntemp  | ( ssp_INTCR_TFTHOD( nval )	 );
    regSSP0_ctrl(nbase)->st.dw.kdrv_ssp_intrcr = ntemp;

}


static void kdrv_ssp_spi_rx_fifo_threshold(uint32_t nbase, uint8_t nval)
{
    uint32_t	ntemp ;

    ntemp = regSSP0_ctrl(nbase)->st.dw.kdrv_ssp_intrcr;

    ntemp &= ~ssp_INTCR_RFTHOD_MASK;

    ntemp = ntemp  | ( ssp_INTCR_RFTHOD( nval )	 );
    regSSP0_ctrl(nbase)->st.dw.kdrv_ssp_intrcr = ntemp;

}

uint32_t kdrv_ssp_rxfifo_valid_entries( uint32_t port )
{
    uint32_t ent;

    ent = ssp_STS_RFVE(regSSP0_ctrl(port)->st.dw.kdrv_ssp_sspstatus);
	
    return ent;
}

uint32_t kdrv_ssp_busy(kdrv_ssp_spi_dev_id_t handle)
{
    return (regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspstatus.BUSY);
}

static uint32_t kdrv_ssp_tx_fifo_waiting_cnt(kdrv_ssp_spi_dev_id_t handle)
{
    return (regSSP0_ctrl(handle)->st.bf.kdrv_ssp_sspstatus.TFVE);
}
//check Tx done flag
static void kdrv_ssp_set_tx_done_flag( struct st_ssp_spi *stspi )
{
    stspi->Tx_done_flag = 1;

    #ifdef COM_BUS_RESPONSE_REQUEST_PIN
    if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
    {
        kdrv_ssp_slave_request_inactive();
    }
    #endif
}

uint8_t kdrv_ssp_get_tx_done_flag( struct st_ssp_spi *stspi )
{
    return stspi->Tx_done_flag ;
}

void kdrv_ssp_spi_tx_dma_en( uint32_t nbase)
{
    regSSP0_ctrl(nbase)->st.bf.kdrv_ssp_intrcr.TFDMAEN = 1;
}

void kdrv_ssp_spi_rx_dma_en(uint32_t nbase)
{
    regSSP0_ctrl(nbase)->st.bf.kdrv_ssp_intrcr.RFDMAEN = 1;
}

static void kdrv_ssp_tx_write_fifo( struct st_ssp_spi *stspi )
{

    uint32_t	base_address = stspi->port_no;
    volatile uint8_t dummy_data = 0xAA;
    uint8_t i =0;

    for( i=0; i < ( SPI_TX_FIFO_TH * 2 ); i++ )
    {
        if(  stspi->Tx_buffer_current_index < stspi->Tx_buffer_index  )
        {
            //write data
            kdrv_ssp_write_byte_new( base_address, ( stspi->Tx_buffer+ stspi->Tx_buffer_current_index ) );
            stspi->Tx_buffer_current_index = stspi->Tx_buffer_current_index+1;

        }
        else
        {
            kdrv_ssp_write_byte_new( base_address, &dummy_data );
            kdrv_ssp_set_tx_done_flag(stspi);
        	//break;
        }

        if(  kdrv_ssp_txfifo_not_full(base_address) == 0 )
        {
            break;
        }
    }
}

static void kdrv_ssp_rx_read_fifo_partial(  struct st_ssp_spi *stspi )
{
    uint32_t	base_address = stspi->port_no;
    uint8_t nsize = kdrv_ssp_rxfifo_valid_entries( base_address );
    uint8_t i=0;

    //Rx
    for( i=0; i<nsize; i++  )
    {
        kdrv_ssp_read_word_new( base_address , stspi->Rx_buffer+ stspi->Rx_buffer_index  );
        stspi->Rx_buffer_index = stspi->Rx_buffer_index + 1 ;
        if( stspi->Rx_buffer_index >= stspi->pre_size )
        {
            stspi->Rx_buffer_index=0;
            if(stspi->cb)
                stspi->cb(ARM_SPI_EVENT_RECEIVE_COMPLETE);
        }
    }
}

void kdrv_ssp_rx_polling_receive_all( struct st_ssp_spi *stspi )
{

    uint32_t	base_address = stspi->port_no;

    /*uint8_t   temp = 100, nfetch;
    //kmdw_printf("------0x%X---------",(stspi->Rx_buffer_index));
    while( (nfetch = kdrv_ssp_rxfifo_valid_entries( base_address )) != temp )
    {
        temp = nfetch;
        #if ( SSP_SPI_MASTER_DEV == COM_BUS_TYPE_SSP1 )||  ( SSP_SPI_MASTER_DEV == COM_BUS_TYPE_SSP0 )
        kdrv_delay_us(5);
        #else
        kdrv_delay_us(50);
        #endif
    }*/

    while( (uint8_t)kdrv_ssp_rxfifo_valid_entries( base_address )  != 0 )
    {
        //nfetch--;
        kdrv_ssp_read_word_new( base_address , ( stspi->Rx_buffer+ (stspi->Rx_buffer_index) )  );
        stspi->Rx_buffer_index = stspi->Rx_buffer_index + 1 ;
    }
}

void kdrv_ssp_spi_CS_set( uint32_t pin, uint8_t ndata )
{
    kdrv_gpio_write_pin( (kdrv_gpio_pin_t) pin, ndata ); //as example
}

/**************************************************
**************************************************/

static void kdrv_ssp_spi_slave_initialize(kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *st_spi)
{
    st_spi->port_no = handle;
    st_spi->SDL = 7;
    st_spi->target_Txfifo_depth = kdrv_ssp_txfifo_depth( st_spi->port_no );
    st_spi->target_Rxfifo_depth = kdrv_ssp_rxfifo_depth( st_spi->port_no );
    st_spi->tx_rx_en = 3;

    st_spi->interrupt_en = 0x04;
    st_spi->IP_type = 0;
    st_spi->spi_sw_type = 1;
    st_spi->sclkdiv = 0;

    st_spi->Tx_buffer = (uint8_t *)kmdw_ddr_reserve(SPI_Buffer_size);
    st_spi->Tx_buffer_index = 0;
    st_spi->Tx_buffer_current_index = gTx_buff_current_index_SP_SLAVE;
    st_spi->Rx_buffer = (uint8_t *)kmdw_ddr_reserve(SPI_Buffer_size);
    st_spi->Rx_buffer_index = 0;
    st_spi->buffer_max_size = SPI_Buffer_size;
    st_spi->Rx_tempbuffer = gRx_buff_temp;
}

static void kdrv_ssp_spi_master_initialize(kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *st_spi)
{
    st_spi->port_no = handle;
    st_spi->SDL = 7;
    st_spi->target_Txfifo_depth = kdrv_ssp_txfifo_depth( st_spi->port_no );
    st_spi->target_Rxfifo_depth = kdrv_ssp_rxfifo_depth( st_spi->port_no );
    st_spi->tx_rx_en = 3;

    st_spi->interrupt_en = 0x00;
    st_spi->IP_type = 3;
    st_spi->spi_sw_type = 0;
    st_spi->sclkdiv = 1;

    st_spi->Tx_buffer = (uint8_t *)gTx_buff_SP_MASTER;
    //st_spi->Tx_buffer_index = &gTx_buff_index_SP_MASTER;
    st_spi->Tx_buffer_current_index = gTx_buff_current_index_SP_MASTER;
    st_spi->Rx_buffer = (uint8_t *)gRx_buff_SP_MASTER;
    //st_spi->Rx_buffer_index = &gRx_buff_index_SP_MASTER;
    st_spi->buffer_max_size = SSP_MASTER_BUFFER;
    st_spi->Rx_tempbuffer = gRx_buff_temp;
}

//clear Tx sw buf index
static void kdrv_ssp_clear_tx_buf_index( struct st_ssp_spi *stspi )
{
    stspi->Tx_buffer_index = 0;
}

//Get Tx sw buf index
uint32_t kdrv_ssp_get_tx_buf_index( struct st_ssp_spi *stspi )
{
    return stspi->Tx_buffer_index;
}

//check Tx current buf index
static void kdrv_ssp_clear_tx_current_buf_index( struct st_ssp_spi *stspi )
{
    stspi->Tx_buffer_current_index = 0;
}

uint16_t kdrv_ssp_get_tx_current_buf_index( struct st_ssp_spi *stspi )
{
    return stspi->Tx_buffer_current_index;
}

//clear Tx done flag
static void kdrv_ssp_clear_tx_done_flag( struct st_ssp_spi *stspi )
{
    stspi->Tx_done_flag = 0;
}

//clear Rx sw buf index
static void kdrv_ssp_clear_rx_buf_index( struct st_ssp_spi *stspi )
{
    stspi->Rx_buffer_index = 0;
}

//Get Rx sw buf index
uint32_t kdrv_ssp_get_rx_buf_index( struct st_ssp_spi *stspi )
{
    return stspi->Rx_buffer_index;
}

void kdrv_ssp_slave_request_active(void)
{
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    kdrv_gpio_write_pin( COM_BUS_RESPONSE_REQUEST_PIN, 1 );
#endif
}

void kdrv_ssp_slave_request_inactive(void)
{
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    kdrv_gpio_write_pin( COM_BUS_RESPONSE_REQUEST_PIN, 0 );
#endif
}

void kdrv_ssp_write_buff( struct st_ssp_spi *stspi, uint8_t *src, uint16_t nlen )
{
    uint16_t i = 0;
    stspi->Tx_buffer_current_index = 0;
    stspi->Tx_buffer_index = 0;
		
	memset((void *)stspi->Tx_buffer, 0 ,SSP_MASTER_BUFFER);
    for( i = 0; i < nlen; i++ )
    {
        *( stspi->Tx_buffer + i ) = *( src + i );
        stspi->Tx_buffer_index = stspi->Tx_buffer_index + 1;
    }
}

void kdrv_ssp_pre_write_to_fifo( struct st_ssp_spi *stspi, uint8_t  target_byte )
{
    volatile uint8_t	*tx_buf = stspi->Tx_buffer;
    volatile uint32_t	tx_buf_current_index = stspi->Tx_buffer_current_index;
    volatile uint32_t	tx_buf_index = stspi->Tx_buffer_index;
    uint16_t i = 0;

    for( i = 0; i < target_byte; i++ )
    {
        //dbg_msg( "*tx_index : %d, 0x%x", *tx_buf_index , *(tx_buf+i)  );
        kdrv_ssp_write_byte_new( stspi->port_no , (tx_buf+i) );
        tx_buf_current_index = tx_buf_current_index + 1;
    }
}

uint8_t kdrv_ssp_SPI_XOR_check(struct st_ssp_spi *stspi)
{
    uint16_t	i = 0;
    uint16_t rx_size = stspi->Rx_buffer_index;
    volatile uint8_t	*rx_ptr = stspi->Rx_buffer;
    uint8_t ntemp = 0;

    ntemp = *rx_ptr;
    for( i=1 ; i < (rx_size-1) ; i++ )
    {
        ntemp ^= *(rx_ptr +i);
    }

    if( ntemp != *(rx_ptr+rx_size-1) )
    {
        return 0;		//xor check fail
    }
    return 1;		 //xor check pass
}

void kdrv_ssp_SPI_XOR_clc( struct st_ssp_spi *stspi )
{
    uint16_t	i = 0;
    volatile uint32_t tx_size = stspi->Tx_buffer_index;
    volatile uint8_t	*tx_ptr = stspi->Tx_buffer;
    uint8_t ntemp = 0;

    ntemp = *tx_ptr;
    for( i=1 ; i < tx_size ; i++ )
    {
        ntemp ^= *(tx_ptr +i);
    }

    *( tx_ptr + tx_size ) = ntemp;
    tx_size = tx_size + 1 ;
}

static void kdrv_ssp_spi_irqhandler( kdrv_ssp_spi_dev_id_t handle )
{
    if(  kdrv_ssp_get_txfifo_int_thflag( handle ) != 0 )
    {
        kdrv_ssp_tx_write_fifo( &driver_ssp_ctx );
    }
    if( kdrv_ssp_get_rxfifo_int_thflag( handle ) != 0 )
    {
        kdrv_ssp_rx_read_fifo_partial( &driver_ssp_ctx );
    }
}

/**************************************************
kdrv_ssp_spi_dev_id_t handle: SPI port
uint16_t op_mode: config SPI signal type
 **************************************************/
kdrv_status_t kdrv_ssp_spi_open( kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *st_spi, uint16_t op_mode, ARM_SPI_SignalEvent_t cb )
{
    uint32_t ntemp;

    sdl_in_bytes = st_spi->SDL;
    kdrv_ssp_reset(handle);
    if( st_spi->IP_type == 0 ||  st_spi->IP_type == 1 )
    {
        //SPI act as slave
        ntemp = op_mode | ( ssp_CR0_FFMT_SPI | ssp_CR0_SLV_SPI );
        regSSP0_ctrl(handle)->st.dw.kdrv_ssp_sspcr0 = ntemp;
    }
    else
    {
        //SPI act as master
        ntemp = op_mode | ( ssp_CR0_FFMT_SPI | ssp_CR0_MSTR_SPI );//| ssp_CR0_SCLKFDBK);
        regSSP0_ctrl(handle)->st.dw.kdrv_ssp_sspcr0 = ntemp;
    }
    st_spi->cb = cb;
    kdrv_ssp_set_sclkdiv( handle, st_spi->sclkdiv );


    kdrv_ssp_set_data_length( handle, sdl_in_bytes );


    kdrv_ssp_set_pcl(handle, pcl);

    //SPI DMA setting......
    //kdrv_ssp_spi_tx_dma_en( st_spi->port_no );	//SPI0 as slave
    //kdrv_ssp_spi_rx_dma_en(st_spi->port_no);

    //clear before start filling in
    kdrv_ssp_clear_txhw( handle );
    kdrv_ssp_clear_rxhw( handle );

    //add interrupt IRQ enable bit!!
    #if ( SSP_SPI_MASTER_DEV == COM_BUS_TYPE_SSP1 )||  ( SSP_SPI_MASTER_DEV == COM_BUS_TYPE_SSP0 )
    if( (st_spi->interrupt_en &0x10)>0 || (st_spi->interrupt_en &0x08)>0 || (st_spi->interrupt_en &0x04)>0 )
    #endif
    {
        kdrv_ssp_spi_set_interrupt( handle, st_spi->interrupt_en );      // TxFIFO threshold:  1<<3, RxFIFO threshold flag: 1<<2
    }
    kdrv_ssp_spi_tx_fifo_threshold( handle, SPI_TX_FIFO_TH );
    kdrv_ssp_spi_rx_fifo_threshold( handle, SPI_RX_FIFO_TH );
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_ssp_spi_uninitialize(kdrv_ssp_spi_dev_id_t handle)
{
    return KDRV_STATUS_OK;
}


kdrv_status_t kdrv_ssp_api_spi_enable( kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *st_spi )
{
    IRQn_Type irq_num = gSPIIRQTbl[handle];
    kdrev_ssp_spi_isr_t isr = gSPIISRs[handle];
    
    //if( st_spi->IP_type == 0 )
    if(st_spi->interrupt_en & 0x0C)
    {
        NVIC_SetVector( irq_num, (uint32_t)isr );
        NVIC_EnableIRQ( irq_num );
    }

    //SPI enable
    kdrv_ssp_spi_enable( handle, 1, 1);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_ssp_api_spi_disable( kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *st_spi )
{
    IRQn_Type irq_num = gSPIIRQTbl[handle];
    //if( st_spi->IP_type == 0 )
    {
        NVIC_DisableIRQ(irq_num);
    }

    //SPI enable
    kdrv_ssp_spi_enable(handle, 0, 0);
    return KDRV_STATUS_OK;
}

/************************************************************************
    SPI master read and write data continuously in the same transaction time
    support write command and read data in the same time.
    rx_all: 0: receive data after transmit done
            1: receive all data during transmit and receive
    ps:
        step 1. assign write command data size
        step 2. assign read data size
        step 3. SPI transmit write command and then read data continuously
************************************************************************/
void kdrv_ssp_SPI_master_transmit( struct st_ssp_spi *st_spi , uint32_t rx_target_size, uint8_t rx_all )
{

    uint32_t i;
    uint32_t  tx_size = st_spi->Tx_buffer_index;
    uint8_t   dummy = 0xcc;

    //dbg_msg_flash("[SPI]Tx size: %d", tx_size);
    //dbg_msg_flash("[SPI]Rx size: %d", rx_target_size);
    kdrv_ssp_spi_CS_set(chip_select_pin, SPI_CS_LOW);
	//clear before start filling in
    kdrv_ssp_clear_txhw(st_spi->port_no);
    //send tx data
    for ( i = 0; i < tx_size; i++ )
    {		
        kdrv_ssp_write_byte_new( st_spi->port_no , (st_spi->Tx_buffer+i) );
				
        if( ( (i+1) % (SPI_TX_FIFO_TH<<2) ) != 0 )
        {
            continue;
        }
        while(regSSP0_ctrl(st_spi->port_no)->st.bf.kdrv_ssp_sspstatus.TFVE);
        kdrv_ssp_rx_polling_receive_all( st_spi );
    }
    while(regSSP0_ctrl(st_spi->port_no)->st.bf.kdrv_ssp_sspstatus.TFVE);
    kdrv_ssp_rx_polling_receive_all( st_spi );
    kdrv_ssp_set_tx_done_flag(st_spi);
		
   // if( rx_all != 1)
   // {
        kdrv_ssp_clear_rx_buf_index( st_spi );
   // }
		
    for ( i = 0; i < rx_target_size; i++ )
    {
        kdrv_ssp_write_byte_new( st_spi->port_no , (uint8_t *)&dummy );
        if( ( i % (SPI_MAX_FIFO/2) ) != 0 )
        {
            continue;
        }
//        while( ( inw(st_spi->port_no + SSP_REG_STS)& (0x3F<<12) ) != 0 );
//        kdrv_ssp_rx_polling_receive_all(st_spi);
        while(regSSP0_ctrl(st_spi->port_no)->st.bf.kdrv_ssp_sspstatus.TFVE > SPI_TX_FIFO_TH);
        kdrv_ssp_rx_read_fifo_partial(st_spi);
    }

    while(regSSP0_ctrl(st_spi->port_no)->st.bf.kdrv_ssp_sspstatus.TFVE);
    while(regSSP0_ctrl(st_spi->port_no)->st.bf.kdrv_ssp_sspstatus.BUSY);
    kdrv_ssp_spi_CS_set(chip_select_pin, SPI_CS_HI);
    kdrv_ssp_rx_polling_receive_all(st_spi);
}



enum e_spi kdrv_ssp_statemachine( kdrv_ssp_spi_dev_id_t handle, struct st_ssp_spi *st_spi, enum e_spi espi_flow, ARM_SPI_SignalEvent_t cb )
{
    st_spi->eflow = espi_flow;

    switch( st_spi->eflow )
    {
        case e_spi_init_slave:
        {
            kdrv_ssp_spi_slave_initialize( handle, st_spi );
            kdrv_ssp_spi_open( handle, st_spi, SPI_MODE_0, cb );
            st_spi->eflow = e_spi_idle;
            return e_spi_ret_init_done;
        }
//			break;
        case e_spi_init_master:
        {
            kdrv_ssp_spi_master_initialize( handle, st_spi );
            kdrv_ssp_spi_open( handle, st_spi, SPI_MODE_0, NULL );
            st_spi->eflow = e_spi_idle;
            return e_spi_ret_init_done;
        }
//            break;
        case e_spi_enable:
        {
            st_spi->pre_size = 0;
            kdrv_ssp_api_spi_enable(handle, st_spi);
            st_spi->eflow = e_spi_idle;
            return e_spi_ret_enable_done;
        }
//			break;
        case e_spi_rx:
        {
            st_spi->eflow = e_spi_idle;

            if( /*st_spi->pre_size == 0 ||*/ st_spi->pre_size != st_spi->Rx_buffer_index )
            {
                st_spi->pre_size = st_spi->Rx_buffer_index;
                //delay_us(20);
                kdrv_delay_us(10);

                return e_spi_ret_rxbusy;
            }
            else
            {
                st_spi->pre_size = 0;
                //no data to be updated
                //read the remaining data in the FIFO
                kdrv_ssp_rx_polling_receive_all( st_spi );
                //for( i = 0; i< (st_spi->Rx_buffer_index) ; i++ ){
                //    *( st_spi->Rx_tempbuffer + st_spi->Rx_tempbuffer_index + i )= *( st_spi->Rx_buffer + i );
                //}
                //st_spi->Rx_tempbuffer_index = st_spi->Rx_tempbuffer_index + (st_spi->Rx_buffer_index);
                return e_spi_ret_rxdone;
            }
            //return e_spi_ret_rxbusy;
        }
//			break;
        case e_spi_txrx_reinit:
        {
            kdrv_ssp_clear_rxhw(handle);
            kdrv_ssp_clear_txhw(handle);
            kdrv_ssp_clear_rx_buf_index(st_spi);
            kdrv_ssp_clear_tx_buf_index(st_spi);
            kdrv_ssp_clear_tx_current_buf_index(st_spi);
            kdrv_ssp_clear_tx_done_flag(st_spi);
            return e_spi_ret_init_done;
        }
//        break;
        case e_spi_rx_check:
        {
            st_spi->eflow = e_spi_idle;

            //check packet reliability
            if( kdrv_ssp_SPI_XOR_check( st_spi ) == 1 )
            {
                return e_spi_ret_rx_xor_OK;			//data is correct
            }
            else
            {
                return e_spi_ret_rx_xor_error ;			//data is in-correct
            }
        }
        //break;
        case e_spi_tx:
        {
            kdrv_ssp_api_spi_disable( handle, st_spi );
            kdrv_ssp_clear_rxhw( handle );
            kdrv_ssp_clear_txhw( handle );
            kdrv_ssp_clear_tx_current_buf_index( st_spi );
            kdrv_ssp_clear_tx_done_flag( st_spi );

            //kdrv_ssp_write_buff( st_spi, temp_buffer, temp_buffer_index );		//for future use
            kdrv_ssp_pre_write_to_fifo( st_spi, 16 );
            kdrv_ssp_api_spi_enable(handle, st_spi);

            st_spi->eflow = e_spi_idle;
            return	e_spi_ret_txbusy;
        }
//			break;
        case e_spi_tx_status_check:
        {
            st_spi->eflow = e_spi_idle;

            if(kdrv_ssp_tx_fifo_waiting_cnt(handle)==0)
            {
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
                if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
                {
                    kdrv_ssp_slave_request_inactive();
                }
#endif
                return e_spi_ret_txdone;
            }
            else
            {
                return e_spi_ret_txbusy;
            }
        }
//			break;
        case e_spi_master_tx_rx:


            break;

        case e_spi_disable:
            {
                kdrv_ssp_api_spi_disable( handle, st_spi );
                st_spi->eflow = e_spi_idle;
                return e_spi_ret_disableDone;
            }
//			break;
        case e_spi_tx_xor:
            kdrv_ssp_SPI_XOR_clc( st_spi );

            return e_spi_ret_tx_xor_done;
        case e_spi_idle:

            break;
        default :
            break;
    }

    return e_spi_ret_idle;
}


void kdrv_ssp_slave_request_init(void)
{
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    uint32_t data;
    //init GPIO as Output low

    //Please modify gpio pin assignment as your real design
    //LC_Data[14] 
    data = inw(SCU_EXTREG_PA_BASE + 0x174);
    data &= 0xFFFFFFF8;     //clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x174, data | 0x3);

    kdrv_gpio_set_attribute( COM_BUS_RESPONSE_REQUEST_PIN , GPIO_DIR_OUTPUT );
    kdrv_gpio_write_pin( COM_BUS_RESPONSE_REQUEST_PIN, 0 );
#endif
}

void kdrv_ssp_spi_loopback_test(bool en)
{
    regSSP0_ctrl(driver_ssp_ctx.port_no)->st.bf.kdrv_ssp_sspcr0.LBM=en; //please set to 0 if not test in loop back mode
}

void kdrv_ssp_write_tx_buffer( struct st_ssp_spi *sspsw, uint8_t *src_ptr, uint32_t size )
{
    int32_t i = 0;
    sspsw->Tx_buffer_index = 0;

    for(i = 0; i < size; i++ )
    {
        *(sspsw->Tx_buffer + sspsw->Tx_buffer_index) = *(src_ptr+i);
        sspsw->Tx_buffer_index = sspsw->Tx_buffer_index+1;
    }
}

