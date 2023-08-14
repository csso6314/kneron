#include "kmdw_status.h"
#include "kmdw_isp.h"
#include "string.h"
static isp_init_para_* pinit_para_ = {0};
kmdw_status_t kmdw_isp_initialize(isp_init_para_* init_para_)
{
    kdrv_status_t ret;
    pinit_para_ = init_para_;
    ret |= (kmdw_status_t)kdrv_isp_set_para(pinit_para_);

    if(ret != KDRV_STATUS_OK)
        return KMDW_STATUS_ERROR;
    pinit_para_->inited = true;
    return KMDW_STATUS_OK;
}
kmdw_status_t kmdw_isp_uninitialize()
{
    kdrv_isp_stop();
    memset(pinit_para_, 0, sizeof(isp_init_para_));
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_isp_open( isp_int_cb cb_event,  void *arg)
{
    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    kdrv_isp_open(cb_event, arg);

    return KMDW_STATUS_OK;
}
kmdw_status_t kmdw_isp_close()
{
    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    kdrv_isp_close();

    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_isp_set_cfg( uint32_t cfg)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->run.cfg = cfg;

    ret |= (kmdw_status_t)kdrv_isp_run_set_cfg(pinit_para_->run.cfg);
    return ret;
}

kmdw_status_t kmdw_isp_set_bin_en( uint32_t bin_en)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->run.bin_en = bin_en;

    ret |= (kmdw_status_t)kdrv_isp_run_set_bin_en(bin_en);
    return ret;
}

kmdw_status_t kmdw_isp_set_go( uint32_t go)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;

    pinit_para_->run.go = go;
    if(go == 1)
    {
        ret = (kmdw_status_t)kdrv_isp_start();
    }
    else
    {
        ret = (kmdw_status_t)kdrv_isp_stop();
    }
    return ret;
}

kmdw_status_t kmdw_isp_set_rdma(uint32_t hdr, uint32_t sa, uint32_t sa_t2, uint32_t row, uint32_t col)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;

    kdrv_isp_set_rdma_config( hdr,  sa,  sa_t2,  row,  col, pinit_para_);

    return ret;
}

kmdw_status_t kmdw_isp_set_wdma(uint32_t id)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;

    kdrv_isp_set_wdma_config(id, pinit_para_);

    return ret;
}

kmdw_status_t kmdw_isp_set_isize(uint32_t bank_id, isp_isize * isize)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].isize.row = isize->row;
    pinit_para_->bk_reg[bank_id].isize.row = isize->col;
    ret |= (kmdw_status_t)kdrv_isp_set_isize(bank_id, &pinit_para_->bk_reg[bank_id].isize);

    return ret;
}

kmdw_status_t kmdw_isp_set_bayer(uint32_t bank_id, isp_bayer * bayer)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].bayer.mono = bayer->mono;
    pinit_para_->bk_reg[bank_id].bayer.hdr  = bayer->hdr;
    pinit_para_->bk_reg[bank_id].bayer.cfa  = bayer->cfa;
    ret |= (kmdw_status_t)kdrv_isp_set_bayer(bank_id, &pinit_para_->bk_reg[bank_id].bayer);

    return ret;
}

kmdw_status_t kmdw_isp_set_bls(uint32_t bank_id, isp_bls * bls)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].bls.k = bls->k;
    ret |= (kmdw_status_t)kdrv_isp_set_bls(bank_id, &pinit_para_->bk_reg[bank_id].bls);

    return ret;
}

kmdw_status_t kmdw_isp_set_gain(uint32_t bank_id, isp_gain * gain)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].gain.b = gain->b;
    pinit_para_->bk_reg[bank_id].gain.g = gain->g;
    pinit_para_->bk_reg[bank_id].gain.r = gain->r;
    ret |= (kmdw_status_t)kdrv_isp_set_gain(bank_id, &pinit_para_->bk_reg[bank_id].gain);

    return ret;
}

kmdw_status_t kmdw_isp_set_fusion(uint32_t bank_id, isp_fusion * fusion)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].fusion.t1_max = fusion->t1_max;
    pinit_para_->bk_reg[bank_id].fusion.t2_min = fusion->t2_min;
    pinit_para_->bk_reg[bank_id].fusion.mdth = fusion->mdth;
    pinit_para_->bk_reg[bank_id].fusion.t2_ratio = fusion->t2_ratio;
    pinit_para_->bk_reg[bank_id].fusion.t1_ratio = fusion->t1_ratio;
    ret |= (kmdw_status_t)kdrv_isp_set_fusion(bank_id, &pinit_para_->bk_reg[bank_id].fusion);

    return ret;
}


kmdw_status_t kmdw_isp_set_demo(uint32_t bank_id, isp_demo * demo)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].demo.weight  = demo->weight;
    pinit_para_->bk_reg[bank_id].demo.lpf     = demo->lpf;
    ret |= (kmdw_status_t)kdrv_isp_set_demo(bank_id, &pinit_para_->bk_reg[bank_id].demo);

    return ret;
}

kmdw_status_t kmdw_isp_set_cm(uint32_t bank_id, isp_cm * cm)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].cm.c0  = cm->c0;
    pinit_para_->bk_reg[bank_id].cm.c1  = cm->c1;
    pinit_para_->bk_reg[bank_id].cm.c2  = cm->c2;
    pinit_para_->bk_reg[bank_id].cm.c3  = cm->c3;
    pinit_para_->bk_reg[bank_id].cm.c4  = cm->c4;
    pinit_para_->bk_reg[bank_id].cm.c5  = cm->c5;
    pinit_para_->bk_reg[bank_id].cm.c6  = cm->c6;
    pinit_para_->bk_reg[bank_id].cm.c7  = cm->c7;
    pinit_para_->bk_reg[bank_id].cm.c8  = cm->c8;
    ret |= (kmdw_status_t)kdrv_isp_set_cm(bank_id, &pinit_para_->bk_reg[bank_id].cm);

    return ret;
}

kmdw_status_t kmdw_isp_set_bias(uint32_t bank_id, isp_bias * bias)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].bias.k0  = bias->k0;
    pinit_para_->bk_reg[bank_id].bias.k1  = bias->k1;
    pinit_para_->bk_reg[bank_id].bias.k2  = bias->k2;
    ret |= (kmdw_status_t)kdrv_isp_set_bias(bank_id, &pinit_para_->bk_reg[bank_id].bias);

    return ret;
}

kmdw_status_t kmdw_isp_set_gamma(uint32_t bank_id, isp_gamma * gamma)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].gamma.p0  = gamma->p0;
    pinit_para_->bk_reg[bank_id].gamma.p1  = gamma->p1;
    pinit_para_->bk_reg[bank_id].gamma.p2  = gamma->p2;
    pinit_para_->bk_reg[bank_id].gamma.p3  = gamma->p3;
    pinit_para_->bk_reg[bank_id].gamma.p4  = gamma->p4;
    pinit_para_->bk_reg[bank_id].gamma.p5  = gamma->p5;
    pinit_para_->bk_reg[bank_id].gamma.p6  = gamma->p6;
    pinit_para_->bk_reg[bank_id].gamma.p7  = gamma->p7;
    pinit_para_->bk_reg[bank_id].gamma.p8  = gamma->p8;
    pinit_para_->bk_reg[bank_id].gamma.p9  = gamma->p9;
    pinit_para_->bk_reg[bank_id].gamma.p10 = gamma->p10;
    pinit_para_->bk_reg[bank_id].gamma.p11 = gamma->p11;
    pinit_para_->bk_reg[bank_id].gamma.p12 = gamma->p12;
    pinit_para_->bk_reg[bank_id].gamma.p13 = gamma->p13;
    pinit_para_->bk_reg[bank_id].gamma.p14 = gamma->p14;
    pinit_para_->bk_reg[bank_id].gamma.p15 = gamma->p15;
    pinit_para_->bk_reg[bank_id].gamma.p16 = gamma->p16;
    ret |= (kmdw_status_t)kdrv_isp_set_gamma(bank_id, &pinit_para_->bk_reg[bank_id].gamma);

    return ret;
}

kmdw_status_t kmdw_isp_set_roi(uint32_t bank_id, isp_roi * roi)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].roi.we  = roi->we;
    pinit_para_->bk_reg[bank_id].roi.wb  = roi->wb;
    pinit_para_->bk_reg[bank_id].roi.he  = roi->he;
    pinit_para_->bk_reg[bank_id].roi.hb  = roi->hb;
    ret |= (kmdw_status_t)kdrv_isp_set_roi(bank_id, &pinit_para_->bk_reg[bank_id].roi);

    return ret;
}

kmdw_status_t kmdw_isp_set_stats(uint32_t bank_id, isp_stats * stats)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].stats.sat_th     = stats->sat_th;
    pinit_para_->bk_reg[bank_id].stats.dark_th    = stats->dark_th;
    ret |= (kmdw_status_t)kdrv_isp_set_stats(bank_id, &pinit_para_->bk_reg[bank_id].stats);

    return ret;
}

kmdw_status_t kmdw_isp_set_yhpf(uint32_t bank_id, isp_yhpf * yhpf)
{
    kmdw_status_t ret = KMDW_STATUS_OK;

    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_->bk_reg[bank_id].yhpf.high_th    = yhpf->high_th;
    pinit_para_->bk_reg[bank_id].yhpf.low_th     = yhpf->low_th;
    pinit_para_->bk_reg[bank_id].yhpf.high_scale = yhpf->high_scale;
    pinit_para_->bk_reg[bank_id].yhpf.low_scale  = yhpf->low_scale;
    ret |= (kmdw_status_t)kdrv_isp_set_yhpf(bank_id, &pinit_para_->bk_reg[bank_id].yhpf);

    return ret;
}

kmdw_status_t kmdw_isp_read_stats(isp_sta_rd* stats)
{
    if(pinit_para_->inited == false)
        return KMDW_STATUS_ERROR;

    pinit_para_->stats_read.sat      = kdrv_isp_stats1_read_sat();
    pinit_para_->stats_read.dark     = kdrv_isp_stats2_read_dark();
    pinit_para_->stats_read.y        = kdrv_isp_stats3_read_y();
    pinit_para_->stats_read.sat_roi  = kdrv_isp_stats4_read_sat_roi();
    pinit_para_->stats_read.dark_roi = kdrv_isp_stats5_read_dark_roi();
    pinit_para_->stats_read.y_roi    = kdrv_isp_stats6_read_y_roi();

    stats->sat       =   pinit_para_->stats_read.sat;
    stats->dark      =   pinit_para_->stats_read.dark;
    stats->y         =   pinit_para_->stats_read.y;
    stats->sat_roi   =   pinit_para_->stats_read.sat_roi;
    stats->dark_roi  =   pinit_para_->stats_read.dark_roi;
    stats->y_roi     =   pinit_para_->stats_read.y_roi;

    return KMDW_STATUS_OK;
}

