#include "kmdw_status.h"
#include "kmdw_vi.h"
#include "string.h"
static vi_init_para_* pinit_para_[2] = {0};
kmdw_status_t kmdw_vi_initialize(uint32_t vi_id, vi_init_para_* init_para_)
{
    kdrv_status_t ret;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id] = init_para_;
    ret |= kdrv_vi_set_para(vi_id, init_para_);

    if(ret != KDRV_STATUS_OK)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->inited = true;
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_vi_uninitialize(uint32_t vi_id)
{
    kdrv_vi_stop(vi_id);
    memset(&pinit_para_[vi_id], 0, sizeof(vi_init_para_));
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_vi_open(uint32_t vi_id, vi_int_cb cb_event,  void *arg)
{
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    kdrv_vi_open(vi_id, cb_event, arg);

    return KMDW_STATUS_OK;
}
kmdw_status_t kmdw_vi_close(uint32_t vi_id)
{
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;

    kdrv_vi_close(vi_id);

    return KMDW_STATUS_OK;
}
kmdw_status_t kmdw_vi_group_update(uint32_t vi_id, vi_init_para_* init_para_)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;

    ret = (kmdw_status_t)kdrv_vi_set_upate(vi_id, 0);
    /*aec_roi*/
    ret |= (kmdw_status_t)kdrv_vi_set_aec_roi(vi_id, &init_para_->aec);
    /*fbb*/
    ret |= (kmdw_status_t)kdrv_vi_set_fbb(vi_id, &init_para_->fbb);
    /*wdma*/
    ret |= (kmdw_status_t)kdrv_vi_set_wdma_da(vi_id, 0, &init_para_->dma.wdma0);
    ret |= (kmdw_status_t)kdrv_vi_set_wdma_da(vi_id, 1, &init_para_->dma.wdma1);

    ret |= (kmdw_status_t)kdrv_vi_set_upate(vi_id, 1);

    return ret;
}

kmdw_status_t kmdw_vi_set_origin(uint32_t vi_id, uint32_t origin)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->cfg.origin = origin;

    ret |= (kmdw_status_t)kdrv_vi_cfg_set_origin(vi_id, origin);
    return ret;
}

kmdw_status_t kmdw_vi_set_bin_en(uint32_t vi_id, uint32_t bin_en)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->run.bin_en = bin_en;

    ret |= (kmdw_status_t)kdrv_vi_run_bin_en(vi_id, bin_en);
    return ret;
}

kmdw_status_t kmdw_vi_set_fbb_en(uint32_t vi_id, uint32_t fbb_en)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->run.fbb_en = fbb_en;

    ret |= (kmdw_status_t)kdrv_vi_run_fbb_en(vi_id, fbb_en);
    return ret;
}

kmdw_status_t kmdw_vi_set_awb_en(uint32_t vi_id, uint32_t awb_en)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->run.awb_en = awb_en;

    ret |= (kmdw_status_t)kdrv_vi_run_awb_en(vi_id, awb_en);
    return ret;
}

kmdw_status_t kmdw_vi_set_roi_en(uint32_t vi_id, uint32_t roi_en)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->run.roi_en = roi_en;
    ret |= (kmdw_status_t)kdrv_vi_run_roi_en(vi_id, roi_en);
    return ret;
}

kmdw_status_t kmdw_vi_set_aec_en(uint32_t vi_id, uint32_t aec_en)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->run.aec_en = aec_en;

    ret |= (kmdw_status_t)kdrv_vi_run_aec_en(vi_id, aec_en);
    return ret;
}

kmdw_status_t kmdw_vi_force_stop(uint32_t vi_id)
{
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;

    kdrv_vi_force_stop(vi_id);
    return KMDW_STATUS_OK;
}

kmdw_status_t kmdw_vi_dpi_active(uint32_t vi_id, uint32_t dpi)
{
    kmdw_status_t ret = KMDW_STATUS_OK;
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    pinit_para_[vi_id]->run.dpi = dpi;
    if(dpi == 1)
    {
        ret = (kmdw_status_t)kdrv_vi_start(vi_id);
    }
    else
    {
        ret = (kmdw_status_t)kdrv_vi_stop(vi_id);
    }
    return ret;
}

kmdw_status_t kmdw_vi_read_stats(vi_stat_opt* opt, uint32_t vi_id, uint8_t* da)
{
    if(vi_id >= VI_ID_MAX)
        return KMDW_STATUS_ERROR;
    if(pinit_para_[vi_id]->inited == false)
        return KMDW_STATUS_ERROR;
    kdrv_vi_read_status(opt, vi_id, da, (uint8_t *)pinit_para_[vi_id]->stat_ddr_address);
    return KMDW_STATUS_OK;
}

