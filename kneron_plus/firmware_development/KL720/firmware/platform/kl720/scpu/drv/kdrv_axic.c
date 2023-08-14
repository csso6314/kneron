/**
 * Kneron Peripheral API - AXIC
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "kdrv_axic.h"
#include "regbase.h"
#include "kdrv_io.h"

typedef struct{
    uint32_t bw_limit:8;
    uint32_t bw_period:3;
    uint32_t rsv:1;
    uint32_t mask_period:4;
    uint32_t outstd_num:3;
    uint32_t rsv2:10;
    uint32_t req_mask:1;
    uint32_t outstd_en:1;
    uint32_t bw_en:1;
}kdrv_axic_awrqos_reg_t;

typedef enum{
    KDRV_AXIC_PRI_SEL_REG = 0,
    KDRV_AXIC_PRI_SEL_QOS,
    KDRV_AXIC_PRI_SEL_QOS_INPPUT
}kdrv_axic_priotity_sel_t;

static volatile uint32_t *AXIC_SLAVE_PRI_1 = (uint32_t *)(AXIC_REG_BASE + 0x124);
//static volatile uint32_t *AXIC_INTERCTRL = (uint32_t *)(AXIC_REG_BASE + 0x12C);
//static volatile uint32_t *AXIC_AXICCTRL = (uint32_t *)(AXIC_REG_BASE + 0x130);
//static volatile uint32_t *AXIC_STAT = (uint32_t *)(AXIC_REG_BASE + 0x134);
//static volatile uint32_t *AXIC_FEATURE_1 = (uint32_t *)(AXIC_REG_BASE + 0x140);
//static volatile uint32_t *AXIC_FEATURE_2 = (uint32_t *)(AXIC_REG_BASE + 0x144);
//#define AXIC_Master_Feature(n)  ((uint32_t *)(AXIC_REG_BASE + (0x150+8*n)))
//#define AXIC_Master_Sparse(n)  ((uint32_t *)(AXIC_REG_BASE + (0x154+8*n)))
//#define AXIC_Slave_Feature(n)  ((uint32_t *)(AXIC_REG_BASE + (0x1d0+4*(n-1))))
//static volatile uint32_t *AXIC_REVISION = (uint32_t *)(AXIC_REG_BASE + 0x250);
#define AXIC_Master_AwQos(n)  ((kdrv_axic_awrqos_reg_t *)(AXIC_REG_BASE + (0x300+4*n)))
#define AXIC_Master_ArQos(n)  ((kdrv_axic_awrqos_reg_t *)(AXIC_REG_BASE + (0x340+4*n)))
static volatile uint32_t *AXIC_MASTER_AW_PRI = (uint32_t *)(AXIC_REG_BASE + 0x380);
static volatile uint32_t *AXIC_MASTER_AR_PRI = (uint32_t *)(AXIC_REG_BASE + 0x384);
static volatile uint32_t *AXIC_MASTER_AW_PRI_SEL = (uint32_t *)(AXIC_REG_BASE + 0x388);
static volatile uint32_t *AXIC_MASTER_AR_PRI_SEL = (uint32_t *)(AXIC_REG_BASE + 0x38C);

kdrv_status_t kdrv_axic_set_default_config(void){ 
    AXIC_Master_AwQos(KDRV_AXIC_MASTER_PORT_DMAC030)->outstd_num = 1;
    AXIC_Master_AwQos(KDRV_AXIC_MASTER_PORT_DMAC030)->outstd_en = 1;
    AXIC_Master_ArQos(KDRV_AXIC_MASTER_PORT_DMAC030)->outstd_num = 1;
    AXIC_Master_ArQos(KDRV_AXIC_MASTER_PORT_DMAC030)->outstd_en = 1;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_axic_set_master_awqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos){
    AXIC_Master_AwQos(port)->bw_limit = qos->bw_limit;
    AXIC_Master_AwQos(port)->bw_period = qos->bw_period;
    AXIC_Master_AwQos(port)->mask_period = qos->mask_period;
    AXIC_Master_AwQos(port)->outstd_num = qos->outstd_num;
    AXIC_Master_AwQos(port)->outstd_en = qos->outstd_en;
    AXIC_Master_AwQos(port)->bw_en = qos->bw_en;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_axic_get_master_awqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos){
    qos->bw_limit = AXIC_Master_AwQos(port)->bw_limit;
    qos->bw_period = AXIC_Master_AwQos(port)->bw_period;
    qos->mask_period = AXIC_Master_AwQos(port)->mask_period;
    qos->outstd_num = AXIC_Master_AwQos(port)->outstd_num;
    qos->outstd_en = AXIC_Master_AwQos(port)->outstd_en;
    qos->bw_en = AXIC_Master_AwQos(port)->bw_en;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_axic_set_master_arqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos){
    AXIC_Master_ArQos(port)->bw_limit = qos->bw_limit;
    AXIC_Master_ArQos(port)->bw_period = qos->bw_period;
    AXIC_Master_ArQos(port)->mask_period = qos->mask_period;
    AXIC_Master_ArQos(port)->outstd_num = qos->outstd_num;
    AXIC_Master_ArQos(port)->outstd_en = qos->outstd_en;
    AXIC_Master_ArQos(port)->bw_en = qos->bw_en;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_axic_get_master_arqos(kdrv_axic_master_port_t port, kdrv_axic_qos_t *qos){
    qos->bw_limit = AXIC_Master_ArQos(port)->bw_limit;
    qos->bw_period = AXIC_Master_ArQos(port)->bw_period;
    qos->mask_period = AXIC_Master_ArQos(port)->mask_period;
    qos->outstd_num = AXIC_Master_ArQos(port)->outstd_num;
    qos->outstd_en = AXIC_Master_ArQos(port)->outstd_en;
    qos->bw_en = AXIC_Master_ArQos(port)->bw_en;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_axic_set_master_priority(kdrv_axic_master_port_t port, kdrv_axic_priority_t aw_p, kdrv_axic_priority_t ar_p){
    outw(AXIC_MASTER_AW_PRI_SEL, 0);
    outw(AXIC_MASTER_AR_PRI_SEL, 0);
    
    uint32_t _p = inw(AXIC_MASTER_AW_PRI);
    uint32_t val = aw_p & 0x03;
    val <<= port*2;
    _p |= val;
    outw(AXIC_MASTER_AW_PRI, _p);
    
    _p = inw(AXIC_MASTER_AR_PRI);
    val = ar_p & 0x03;
    val <<= port*2;
    _p |= val;
    outw(AXIC_MASTER_AW_PRI, _p);
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_axic_get_master_priority(kdrv_axic_master_port_t port, kdrv_axic_priority_t *aw_p, kdrv_axic_priority_t *ar_p){
    kdrv_axic_priority_t tmp;
    uint32_t p = inw(AXIC_MASTER_AW_PRI);
    uint32_t mask = 0x03;
    mask <<= port*2;
    p &= mask;
    p >>= port*2;
    tmp = (kdrv_axic_priority_t)p;
    *aw_p = tmp;
    p = inw(AXIC_MASTER_AR_PRI);
    mask = 0x03;
    mask <<= port*2;
    p &= mask;
    p >>= port*2;
    tmp = (kdrv_axic_priority_t)p;
    *ar_p = tmp;
    return KDRV_STATUS_OK;
}

kdrv_status_t kdrv_axic_set_slave_priority(kdrv_axic_slave_port_t port, kdrv_axic_priority_t p){
    uint32_t _p = inw(AXIC_SLAVE_PRI_1);
    uint32_t val = p & 0x03;
    val <<= port*2;
    _p |= val;
    outw(AXIC_SLAVE_PRI_1, _p);
    return KDRV_STATUS_OK;
}

kdrv_axic_priority_t kdrv_axic_get_slave_priority(kdrv_axic_master_port_t port){
    uint32_t p = inw(AXIC_SLAVE_PRI_1);
    uint32_t mask = 0x03;
    mask <<= port*2;
    p &= mask;
    p >>= port*2;
    return (kdrv_axic_priority_t)p;
}
