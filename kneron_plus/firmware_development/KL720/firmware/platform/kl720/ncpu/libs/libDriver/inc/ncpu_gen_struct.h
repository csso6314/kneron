/**
 * @file      ncpu_gen_struct.h
 * @brief     NCPU generic data structures used for API
 * @copyright (c) 2020-2022 Kneron Inc. All right reserved.
 */

#ifndef __NCPU_GEN_STRUCT_H__
#define __NCPU_GEN_STRUCT_H__

#include <stdint.h>
#include <stdbool.h>
#include "ipc.h"
#include "mdl_parser_api.h"

/* Helper macros */
#define RAW_INFERENCE_FORMAT(image_p)           (image_p->raw_img_p->inf_format)
#define RAW_IMAGE_NUM_INPUT(image_p)            (image_p->raw_img_p->num_image)
#define RAW_IMAGE_MEM_ADDR(image_p, i)          (image_p->raw_img_p->image_list[i].image_mem_addr)
#define RAW_IMAGE_MEM_LEN(image_p, i)           (image_p->raw_img_p->image_list[i].image_mem_len)
#define RAW_FORMAT(image_p, i)                  (image_p->raw_img_p->image_list[i].format)
#define RAW_INPUT_ROW(image_p, i)               (image_p->raw_img_p->image_list[i].input_row)
#define RAW_INPUT_COL(image_p, i)               (image_p->raw_img_p->image_list[i].input_col)
#define RAW_CROP_TOP(image_p, i)                (image_p->raw_img_p->image_list[i].params_s.crop_top)
#define RAW_CROP_BOTTOM(image_p, i)             (image_p->raw_img_p->image_list[i].params_s.crop_bottom)
#define RAW_CROP_LEFT(image_p, i)               (image_p->raw_img_p->image_list[i].params_s.crop_left)
#define RAW_CROP_RIGHT(image_p, i)              (image_p->raw_img_p->image_list[i].params_s.crop_right)
#define RAW_PAD_TOP(image_p, i)                 (image_p->raw_img_p->image_list[i].params_s.pad_top)
#define RAW_PAD_BOTTOM(image_p, i)              (image_p->raw_img_p->image_list[i].params_s.pad_bottom)
#define RAW_PAD_LEFT(image_p, i)                (image_p->raw_img_p->image_list[i].params_s.pad_left)
#define RAW_PAD_RIGHT(image_p, i)               (image_p->raw_img_p->image_list[i].params_s.pad_right)
#define RAW_SCALE_WIDTH(image_p, i)             (image_p->raw_img_p->image_list[i].params_s.scale_width)
#define RAW_SCALE_HEIGHT(image_p, i)            (image_p->raw_img_p->image_list[i].params_s.scale_height)
#define RAW_OTHER_PARAMS(image_p)               (image_p->raw_img_p->ext_params)

#define DIM_INPUT_ROW(image_p, i)               (image_p->model_preproc[i].dim.input_row)
#define DIM_INPUT_COL(image_p, i)               (image_p->model_preproc[i].dim.input_col)
#define DIM_INPUT_CH(image_p, i)                (image_p->model_preproc[i].dim.input_channel)

#define PREPROC_INPUT_MEM_ADDR(image_p, i)      (image_p->model_preproc[i].preproc.input_mem_addr)
#define PREPROC_INPUT_MEM_LEN(image_p, i)       (image_p->model_preproc[i].preproc.input_mem_len)
#define PREPROC_INPUT_MEM_ADDR2(image_p, i)     (image_p->model_preproc[i].preproc.input_mem_addr2)
#define PREPROC_INPUT_MEM_LEN2(image_p, i)      (image_p->model_preproc[i].preproc.input_mem_len2)
#define PREPROC_INPUT_RADIX(image_p, i)         (image_p->model_preproc[i].preproc.input_radix)
#define PREPROC_INPUT_FORMAT(image_p, i)        (image_p->model_preproc[i].preproc.input_format)
#define PREPROC_PARAMS_P(image_p, i)            (image_p->model_preproc[i].preproc.params_p)

#define POSTPROC_OUTPUT_NUM(image_p)            (image_p->postproc.output_num)
#define POSTPROC_OUTPUT_FORMAT(image_p)         (image_p->postproc.output_format)
#define POSTPROC_OUTPUT_MEM_ADDR(image_p)       (image_p->postproc.output_mem_addr)
#define POSTPROC_OUTPUT_MEM_LEN(image_p)        (image_p->postproc.output_mem_len)
#define POSTPROC_RESULT_MEM_ADDR(image_p)       (image_p->postproc.result_mem_addr)
#define POSTPROC_RESULT_MEM_LEN(image_p)        (image_p->postproc.result_mem_len)
#define POSTPROC_RESULT_MODEL_ID(image_p)       (image_p->postproc.model_id)
#define POSTPROC_PARAMS_P(image_p)              (image_p->postproc.params_p)
#define POSTPROC_RAW_RESULT_MEM_ADDR(image_p)   (image_p->postproc.raw_result_mem_addr)
#define POSTPROC_OUTPUT_MEM_ADDR3(image_p)      (image_p->postproc.output_mem_addr3)
#define POSTPROC_OUTPUT_MEM_ADDR4(image_p)      (image_p->postproc.output_mem_addr4)

/*****************************************************************
 * Legacy setup.bin model information
 *****************************************************************/

/* Type of Operations */
#define FW_IN_NODE            0
#define FW_CPU_NODE           1
#define FW_OUTPUT_NODE        2
#define FW_NETWORK_INPUT_NODE 5

typedef struct CNN_Header {
    uint32_t crc;
    uint32_t version;
    uint32_t key_offset;
    uint32_t model_type;
    uint32_t application_type;
    uint32_t dram_start;
    uint32_t dram_size;
    uint32_t cmd_start;
    uint32_t cmd_size;
    uint32_t weight_start;
    uint32_t weight_size;
    uint32_t input_start;
    uint32_t input_size;
    uint32_t input_num;
    uint32_t output_num;
} CNN_Header;

// node_id = 0
typedef struct In_Node {
    uint32_t node_id;
    uint32_t next_npu;
} In_Node;

// node_id = 4
typedef struct Super_Node {
    uint32_t node_id;
    uint32_t addr;
    uint32_t row_start;
    uint32_t col_start;
    uint32_t ch_start;
    uint32_t row_length;
    uint32_t col_length;
    uint32_t ch_length;
} Super_Node;

// node_id = 3
typedef struct Data_Node {
    uint32_t node_id;
    uint32_t supernum;
    uint32_t data_format;
    uint32_t data_radix;
    uint32_t data_scale;
    uint32_t row_start;
    uint32_t col_start;
    uint32_t ch_start;
    uint32_t row_length;
    uint32_t col_length;
    uint32_t ch_length;
    Super_Node* node_list;
} Data_Node;

// node_id = 1
typedef struct CPU_Node {
    uint32_t node_id;
    uint32_t input_datanode_num;
    uint32_t op_type;
    uint32_t in_num_row;
    uint32_t in_num_col;
    uint32_t in_num_ch;
    uint32_t out_num_row;
    uint32_t out_num_col;
    uint32_t out_num_ch;
    uint32_t h_pad;
    uint32_t w_pad;
    uint32_t kernel_h;
    uint32_t kernel_w;
    uint32_t stride_h;
    uint32_t stride_w;
    Data_Node* output_datanode;
    Data_Node* input_datanodes;
} CPU_Node;

// node_id = 2
typedef struct Out_Node {
    uint32_t node_id;
    uint32_t supernum;
    uint32_t data_format;
    uint32_t row_start;
    uint32_t col_start;
    uint32_t ch_start;
    uint32_t row_length;
    uint32_t col_length;
    uint32_t ch_length;
    uint32_t output_index;
    uint32_t output_radix;
    uint32_t output_scale;
    Super_Node* node_list;
} Out_Node;

// node_id = 5
typedef struct NetInput_Node {
    uint32_t node_id;
    uint32_t input_index;
    uint32_t input_format;
    uint32_t input_row;
    uint32_t input_col;
    uint32_t input_channel;
    uint32_t input_start;
    uint32_t input_size;
    uint32_t input_radix;
} NetInput_Node;

typedef struct Operation_Node {
    uint32_t node_id;
    uint32_t buffer_index;
    In_Node* inN;
    Out_Node* outN;
    CPU_Node* cpuN;
    NetInput_Node* netinN;
    struct Operation_Node* next;
} Operation_Node;


/* Structure of setup.bin file */
struct setup_struct_s {
    CNN_Header           header;
};

/* Structure of kdp_model_dim */
struct kdp_model_dim_s {
    /* CNN input dimensions */
    uint32_t    input_row;
    uint32_t    input_col;
    uint32_t    input_channel;
};

/* Structure of kdp_pre_proc_s */
struct kdp_pre_proc_s {
    /* input image in memory for NPU */
    uint32_t    input_mem_addr;
    int32_t     input_mem_len;

    /* Input working buffers for NPU */
    uint32_t    input_mem_addr2;
    int32_t     input_mem_len2;

    /* number of bits for input fraction */
    uint32_t    input_radix;

    /* input data format from NPU*/
    uint32_t    input_format;

    /* Other parameters for the model */
    void        *params_p;
};

struct kdp_model_pre_proc_s {
    /* Model dimension */
    struct kdp_model_dim_s dim;

    /* Pre process struct */
    struct kdp_pre_proc_s preproc;
};

/* Structure of kdp_post_proc_s */
struct kdp_post_proc_s {
    /* output number */
    uint32_t    output_num;

    /* output data memory from NPU */
    uint32_t    output_mem_addr;
    int32_t     output_mem_len;

    /* result data memory from post processing */
    uint32_t    result_mem_addr;
    int32_t     result_mem_len;

    uint32_t    output_mem_addr3;        // for FD SSR or shared
    uint32_t    output_mem_addr4;        // for CenterNet or shared

    /* output data format from NPU
     *     BIT(0): =0, 8-bits
     *             =1, 16-bits
     */
    uint32_t    output_format;

    /* output node parameter */
    Out_Node    *node_p;

    /* Other parameters for the model */
    void        *params_p;
};

/* Parsed model setup.bin information (legacy) */
typedef struct {
    int                nTotalNodes;
    CNN_Header         *pSetupHead;
    Operation_Node     *pNodeHead;
    kdp_model_t        oModel;
    int                total_nodes;
    int                current_node_id;
    uint32_t pNodePositions[MAX_CNN_NODES];
} parsed_fw_model_t;


/*****************************************************************
 * Flatbuffer setup.bin model information
 *****************************************************************/

/* Parsed model setup.bin information (flatbuffer) */
typedef struct {
    session_hdl_t       parser_session_hdl;
    node_hdl_t          in_node_hdl;
    node_hdl_t          out_node_hdl;
    node_hdl_t          cpu_node_hdl;
    node_hdl_t          const_node_hdl;
} parsed_fw_model_flatbuffer_t;


/*****************************************************************
 * Main NCPU inference flow information
 *****************************************************************/

/* KDP image structure */
typedef struct kdp_image_s {
    /* a NCPU copy of raw image struct in in_comm_p */
    struct kdp_img_raw_s            *raw_img_p;

    /**
     * To compatible legacy/flatbuffer version setup.bin information
     *
     * is_flatbuffer:           false - legacy. true - flatbuffer.
     * pParsedModel:            for legacy setup.bin model. (NULL when is_flatbuffer is true)
     * pParsedModelFlatbuffer:  for flatbuffer setup.bin model. (NULL when is_flatbuffer is false)
     */
    bool                            is_flatbuffer;
    parsed_fw_model_t               *pParsedModel;
    parsed_fw_model_flatbuffer_t    *pParsedModelFlatbuffer;

    int         model_id;
    int         slot_idx;
    char        *setup_mem_p;

    void        *pExtParam;
    int32_t     nLenExtParam;

    /* Model dimension and Pre process struct */
    struct kdp_model_pre_proc_s  model_preproc[MAX_INPUT_NODE_COUNT];

    /* Post process struct */
    struct kdp_post_proc_s   postproc;
} kdp_image_t;




#define    TOTAL_STANDALONE_MODULE    5
typedef int (*dsp_ftr_fn)(void  *pIn,  void  *pOut);
typedef struct {
    int           standalone_cmd;
    dsp_ftr_fn    fn;
} dsp_ftr_node_t;

#define MAX_MODEL_REGISTRATIONS       40
typedef int (* cnn_pre_post_fn)(struct kdp_image_s *pKdpImage);
typedef struct {
    int              model_id;
    cnn_pre_post_fn  fn;
} model_pre_post_func_t;

#endif    /* __NCPU_GEN_STRUCT_H__ */
