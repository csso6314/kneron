/**
 * @file        kmdw_model.h
 * @brief       model manager APIs
 *
 * @copyright   Copyright (c) 2019 Kneron Inc. All rights reserved.
 */
#ifndef __MODEL_MGR_H__
#define __MODEL_MGR_H__

#include "cmsis_os2.h"
#include "base.h"
#include "ipc.h"             /* for MULTI_MODEL_MAX */
#include "model_type.h"
#include "jpeg_intf.h"
#include "tof_intf.h"

#define KMDW_MODEL_ALL_MODELS  -1               /**<  A term means ALL_MODELS */
#define KMDW_MODEL_MAX_MODEL_COUNT MULTI_MODEL_MAX /**< MAX model count for DME and flash */

/* These crc values will ONLY change if model is changed in FLASH */
#define MODEL_INFO_CRC16_VALUE  0x5341          /**< CRC16 value */
#define MODEL_INFO_SUM32_VALUE  0x00eee074      /**< SUM32 value */

/**
 * @brief return code of kmdw_model_run()
 */
enum kmdw_model_rc {
    // 0 - 9 is reserved for ncpu return
    // defined in ipc.h
    // IMAGE_STATE_INACTIVE == 0
    // IMAGE_STATE_ACTIVE == 1
    // IMAGE_STATE_NPU_DONE == 2
    // IMAGE_STATE_DONE == 3
    KMDW_MODEL_RUN_RC_ABORT = 10,           /**< return code: abort */
    KMDW_MODEL_RUN_RC_ERROR = 11,           /**< return code: error */
    KMDW_MODEL_RUN_RC_END                   /**< dummy item for enum table end */
};

/**
 * @brief data structure of model run time
 */
typedef struct kmdw_model_run_time_s {
    float    round_trip_time;               /**< round trip time */
    float    pre_proc_time;                 /**< pre process time */
    float    npu_proc_time;                 /**< npu process time */
    float    post_proc_time;                /**< post process time */
    float    rslt_pop_interval;             /**< the interval between 2 NCPU interrupts for result ( exclude img request interrupt) */
    float    scpu_send_ipc_ack;             /**< the interval from scpu send ipc signal till acknowledge from NCPU */
    float    ncpu_req_ipc_ack;              /**< the interval from ncpu result signal till acknowledge from SCPU */
    float    ncpu_rslt_ipc_ack;             /**< the interval from ncpu result signal till acknowledge from SCPU */
    float    ncpu_img_req_to_rcv_new_img;   /**< the interval from ncpu image request till a new image is received */
} kmdw_model_run_time_t;

/**
 * @brief a mask for easy read fw_info data
 */
typedef struct kmdw_model_fw_info_s {
    uint32_t                model_count;    /**< model count */
    struct kdp_model_s      models[1];      /**< the address of dynamical count of models */
} kmdw_model_fw_info_t;

/**
 * @brief a mask for easy read fw_info_ext data
 */
typedef struct kmdw_model_fw_info_ext_s {
    uint32_t                model_dram_addr_end;    /**< model end address */
    uint32_t                model_total_size;       /**< model total size */
    uint32_t                model_checksum;         /**< checksum */
} kmdw_model_fw_info_ext_t;

/**
 * @brief a basic descriptor for a input/output node in model
 */
typedef struct
{
    uint32_t    index;              /**< index of node */
    uint32_t    shape_npu_len;      /**< length of npu shape */
    uint32_t    shape_npu[4];       /**< npu shape BxCxHxW (KL720 NEFv1 only support 4-dims shape) */
    uint32_t    data_layout;        /**< npu memory layout */
    float       scale;              /**< scale of node (KL720 NEFv1 only support layer wised quantization param) */
    int32_t     radix;              /**< radix of node (KL720 NEFv1 only support layer wised quantization param) */
} kmdw_model_tensor_descriptor_t;

/* ############################
 * ##    Public Functions    ##
 * ############################ */

/**
 * @brief Init model functionality
 */
void kmdw_model_init(void);

/**
 * @brief A wrapper of load_model from flash
 * @param model_index_p: model info index,
 *                    0-n: info_index of model to load
 *                    -1 means to load all models
 * @return 0: failes;  1: OK(means 1 model is loaded)
 */
int32_t kmdw_model_load_model(int8_t model_info_index_p);


/**
 * @brief A wrapper of load_model_info
 * @param [in] is_model_from_ddr: if model is from ddr/host command
 * @return reloaded model count; 0 means failed
 */
int32_t kmdw_model_reload_model_info(bool from_ddr);


/**
 * @brief Refresh all models
 * @return refreshed model count; 0 means failed
 */
int32_t kmdw_model_refresh_models(void);


/**
 * @brief Output model_info of specified index
 * @param[in] idx_p the index of programmed models
 * @return model_info defined in ipc.h
 */
struct kdp_model_s* kmdw_model_get_model_info(int idx_p);


/**
 * @brief Output total number of models and all model IDs
 * @param[in] trust_ddr_data if true, no need to check validness of ddr data
 * @return a list of model count, id0, id1, id2
 * @note only obtain data from ddr space
 *       model must be loaded from file or flash to ddr first
 */
uint32_t *kmdw_model_get_all_model_info(bool trust_ddr_data);


/**
 * @brief Output model end address defined in model file
 * @param[in] trust_ddr_data if true, no need to check validness of ddr data
 * @return model end address when loaded
 * @note only obtain data from ddr space
 *       model must be loaded from file or flash to ddr first
 */
uint32_t kmdw_model_get_model_end_addr(bool trust_ddr_data);

/**
 * @brief Output crc value
 * @param[in] trust_ddr_data if true, no need to check validness of ddr data
 * @return crc value
 * @note only obtain data from ddr space
 *       model must be loaded from file or flash to ddr first
 */
uint32_t kmdw_model_get_crc(bool trust_ddr_data);

/**
 * @brief Get the buffer address of fw_info
 * @param[in] trust_ddr_data if true, no need to check validness of ddr data
 * @return the buffer address of fw_info or NULL when examination failed
 */
kmdw_model_fw_info_t* kmdw_model_get_fw_info(bool trust_ddr_data);

/**
 * @brief Specify output address for model run in ncpu/npu
 * @return always 0
 * @note must be called after kmdw_model_config_model()
 */
int32_t kmdw_model_config_result(osEventFlagsId_t result_evt, uint32_t result_evt_flag);


/**
 * @brief Config model image
 * @param img_cfg image config
 * @param ext_param extra param
 */
void kmdw_model_config_img(struct kdp_img_cfg *img_cfg, void *ext_param);


/**
 * @brief Get raw image config
 * @param idx image index
 * @return raw image config
 */
struct kdp_img_raw_s* kmdw_model_get_raw_img(int idx);


/**
 * @brief Run model
 * @param tag model tag
 * @param output model output
 * @param model_type model type
 * @param dme DME mode
 * @return kmdw_model_rc
 */
int kmdw_model_run(const char *tag, void *output, uint32_t model_type, bool dme);


/**
 * @brief Abort model execution
 */
void kmdw_model_abort(void);


/**
 * @brief Get round-trip/pre/npu/post times
 * @param [in]  img_idx raw image index
 * @param [out] run_time run time of pre/npu/post/
 * @return void
 */
void kmdw_model_get_run_time(int img_idx, kmdw_model_run_time_t *run_time);


/**
 * @brief Check if a model is loaded in memory
 * @param[in] model_type model type
 * @return 1: loaded in memory; 0: not loaded
 */
int kmdw_model_is_model_loaded(uint32_t model_type);


/**
 * @brief Get model input tensor/node number
 * @param[in] model_type model type
 * @return input tensor/node number, 0 means failed
 */
int kmdw_model_get_input_tensor_num(uint32_t model_type);

/**
 * @brief Get model input tensor/node information
 * @param[in] model_type model type
 * @param[in] tensor_idx input tensor index
 * @param[out] tensor_info input tensor information
 * @return 1: success; 0: fail
 */
int kmdw_model_get_input_tensor_info(uint32_t model_type, uint32_t tensor_idx, kmdw_model_tensor_descriptor_t *tensor_info);


/**
 * @brief Get model output tensor/node number
 * @param[in] model_type model type
 * @return output tensor/node number, 0 means failed
 */
int kmdw_model_get_output_tensor_num(uint32_t model_type);


/**
 * @brief Check if a model is loaded in memory
 * @param[in] model_type model type
 * @param[in] tensor_idx output tensor index
 * @param[out] tensor_info output tensor information
 * @return 1: success; 0: fail
 */
int kmdw_model_get_output_tensor_info(uint32_t model_type, uint32_t tensor_idx, kmdw_model_tensor_descriptor_t *tensor_info);

#if DEBUG

/**
 * @brief Dump model debug info
 */
void kmdw_model_dump_model_info(void);

#endif

#endif
