/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
*
* The information contained herein is property of Kneron, Inc.
* Terms and conditions of usage are described in detail in Kneron
* STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information.
* NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
* from the file.
*/

/******************************************************************************
*  Filename:
*  ---------
*  kdrv_adc.c
*
*  Project:
*  --------
*  KL520
*
*  Description:
*  ------------
*  This ADC driver is for Generic Analog-to-Digital Converter
*  HW: Faraday FTADC010
*
*  Author:
*  -------
*  Teresa Chen
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "kdrv_cmsis_core.h"
#include "kdrv_adc.h"
#include "kmdw_console.h"

#define NUM_TDC         4
#define SCANMODE        SCANMODE_CONT

/**
 * @brief ADC Register Configuration
 */
struct kdrv_adc_regs_t {
    uint32_t        data[8];        /**< DATA: offset 0x000~0x01C  */
    uint32_t        reserve[24];
    uint32_t        thrhold[8];     /**< THRHOLD: offset 0x080~0x09C  */
#define HTHR_EN             (1<<31)
#define HTHR(x)             (((x)&0xFFF)<<16)
#define LTHR_EN             (1<<15)
#define LTHR(x)             (((x)&0xFFF)<<0)
    uint32_t        reserve1[24];
    /* CTRL: offset 0x100 */
    uint32_t        ctrl;
#define SCAN_NUM(x)         (x<<16)
#define SCANMODE_CONT       (1<<9)
#define SCANMODE_SGL        (1<<8)
#define SWSTART             (1<<4)
#define ADC_EN              (1<<0)
    /* TRIM: offset 0x104 */
    uint32_t        trim;
    /* INTEN: offset 0x108 */
    uint32_t        inten;
#define CHDONE_INTEN(x)     (1<<((x)+8))
#define TS_OVR_INTREN       (1<<3)
#define TS_UDR_INTREN       (1<<2)
#define STOP_INTEN          (1<<1)
#define DONE_INTEN          (1<<0)
    /* INTST: offset 0x10C */
    uint32_t        intst;
#define CH_INTRSTS(x)       (1<<((x)+8))
#define TS_THDOD_INTRSTS    (1<<3)
#define TS_THDUD_INTRSTS    (1<<2)
#define ADC_STOP_INTRSTS    (1<<1)
#define ADC_DONE_INTSTS     (1<<0)
    /* TPARAM: offset 0x110 */
    uint32_t        tparam;
    /* TPARAM1: offset 0x114 */
    uint32_t        smpr;
    /* reserve */
    uint32_t        reserve2;
    /* PRESCAL: offset 0x11C */
    uint32_t        prescal;
    /* SQR:         offset 0x120 */
    uint32_t        sqr;

};

kdrv_adc_resource_t kdrv_adc_resource = {
    ADC_FTTSC010_0_PA_BASE,
    ADC_FTADCC010_IRQ,
};

void _kdrv_adc_config(void)
{
    struct kdrv_adc_regs_t *regs = (struct kdrv_adc_regs_t *)kdrv_adc_resource.io_base;

    uint32_t val;

    /* set ADC clock pre-scaler control register (offset 0x11C) */
    //outw(regs->prescal, val);

    /* set ADC timing parameter register (offset 0x110) */
    //outw(regs->tparam, val);

    /* set ADC sampling timing parameter control register (offset 0x114) */
    //outw(regs->smpr, val);

    /* set ADC scan sequence register (offset 0x120) */
    //outw(regs->sqr, val);

    /* set ADC interrupt enable register (offset 0x108) */
    //outw(regs->inten, val);

    /* set ADC control register (offset 0x100) */
    val = 0;
    val = ADC_EN | SWSTART | SCANMODE_CONT | SCAN_NUM(4);
    //dbg_msg_console("val = 0x%x", val);
    outw(&regs->ctrl, val);
    //_print_adc_register(&kdrv_adc_resource);
}

kdrv_status_t kdrv_adc_initialize(void)
{
    uint32_t i;
    uint32_t val = 0;
    //char Ch;
    struct kdrv_adc_regs_t *regs = (struct kdrv_adc_regs_t *)kdrv_adc_resource.io_base;

    //Enable ADCCLK in SCU base 0xC2380000
    //Set the 22th bit of the "Clock Enable Register1"(offset = 0x018) to 1
    val = inw(SCU_EXTREG_PA_BASE + 0x018);
    outw(SCU_EXTREG_PA_BASE + 0x018, (val | (1 << 22 )));

    osDelay(1);
    _kdrv_adc_config();

    // Wait for scanning done.
    for(i = 0; i < NUM_TDC; i++) {
        while(!(readl(&regs->intst) & CH_INTRSTS(i))) {}
    }

	while(!(readl(&regs->intst) & ADC_DONE_INTSTS)){};
    /*while(1)
    {
        dbg_msg_console("AIN0: %d \r\nAIN1: %d \r\nAIN2: %d \r\nAIN3: %d \r\n ===\r\n",regs->data[0],regs->data[1],regs->data[2],regs->data[3]);
        dbg_msg_console("press anykey to measureing, and 'q' to exit");
        //_print_adc_register(&kdrv_adc_resource);
        Ch = kmdw_console_getc();
        if (Ch == 'q')
            break;
    }*/
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_adc_uninitialize(kdrv_adc_resource_t *res)
{
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_adc_rest(kdrv_adc_resource_t *res)
{
    int val;
    struct kdrv_adc_regs_t *regs = (struct kdrv_adc_regs_t *)res->io_base;

    //disable
    val = readl(&regs->ctrl);
    val &= ~ADC_EN; //clear enable bit
    writel(val, &regs->ctrl);

    //enable
    val |= ADC_EN; //clear enable bit
    writel(val, &regs->ctrl);

    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_adc_enable(kdrv_adc_resource_t *res, int mode)
{
    int val;
    struct kdrv_adc_regs_t *regs = (struct kdrv_adc_regs_t *)res->io_base;

    val = readl(&regs->ctrl);
    val &= ~ADC_EN; //clear enable bit
    val |= mode;
    writel(val, &regs->ctrl);

    return KDRV_STATUS_OK;
}

static void kdrv_adc_force_update_temp(kdrv_adc_resource_t *res)
{
    kdrv_adc_enable(res, ADC_EN);
}

int kdrv_adc_read(int id)
{
    uint32_t val = 0;
    struct kdrv_adc_regs_t *regs = (struct kdrv_adc_regs_t *)kdrv_adc_resource.io_base;

    if(SCANMODE == SCANMODE_SGL)
        kdrv_adc_force_update_temp(&kdrv_adc_resource);

    val = readl(&regs->data[id]);

    DSG("print raw data: %d\n", val);
    
    return val;
}

void _print_adc_register(kdrv_adc_resource_t *res)
{
    uint32_t val = 0;
    struct kdrv_adc_regs_t *regs = (struct kdrv_adc_regs_t *)res->io_base;

    val = inw(&regs->prescal);  //0x11C
    DSG("0x%x = 0x%x", &regs->prescal, val);
    val = inw(&regs->tparam);   //0x110
    DSG("0x%x = 0x%x", &regs->tparam, val);
    val = inw(&regs->smpr);     //0x114
    DSG("0x%x = 0x%x", &regs->smpr, val);
    val = inw(&regs->sqr);          //0x120
    DSG("0x%x = 0x%x", &regs->sqr, val);
    val = inw(&regs->inten);        //0x108
    DSG("0x%x = 0x%x", &regs->inten, val);
    val = inw(&regs->ctrl);     //0x100
    DSG("0x%x = 0x%x", &regs->ctrl, val);

}

