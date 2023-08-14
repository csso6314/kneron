/*
 * Kneron Header for KDP on KL520
 *
 * Copyright (C) 2018-2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef KDPIO_H
#define KDPIO_H

#include <stdint.h>
#include "ipc.h"

#define MAX_MODEL_REGISTRATIONS 32

/* Type of Operations */
enum {
    NODE_TYPE_IN,
    NODE_TYPE_CPU,
    NODE_TYPE_OUT,
};

/* Structures of Data Nodes */
struct super_node_s {
    uint32_t node_id;
    uint32_t addr;
    uint32_t row_start;
    uint32_t col_start;
    uint32_t ch_start;
    uint32_t row_length;
    uint32_t col_length;
    uint32_t ch_length;
};

struct data_node_s {
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
    struct super_node_s node_list[1];
};

/* Structure of Input Operation */
struct in_node_s {
    uint32_t node_id;
    uint32_t next_npu;
};

/* Structure of Output Operation */
struct out_node_s {
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
    struct super_node_s node_list[1];
};

/* Structure of CPU Operation */
struct cpu_node_s {
    uint32_t node_id;
    uint32_t input_datanode_num;
    uint32_t op_type;
    /* There will be more parameter here for cpu operation  */
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
    struct data_node_s output_datanode;
    struct data_node_s input_datanode[1];
};

/* Structure of CNN Header in setup.bin */
struct cnn_header_s {
    uint32_t crc;
    uint32_t version;
    uint32_t key_offset;
    uint32_t model_type;
    uint32_t app_type;
    uint32_t dram_start;
    uint32_t dram_size;
    uint32_t input_row;
    uint32_t input_col;
    uint32_t input_channel;
    uint32_t cmd_start;
    uint32_t cmd_size;
    uint32_t weight_start;
    uint32_t weight_size;
    uint32_t input_start;
    uint32_t input_size;
    uint32_t input_radix;
    uint32_t output_nums;
};

/* Structure of setup.bin file */
struct setup_struct_s {
    struct cnn_header_s header;

    union {
        struct in_node_s in_nd;
        struct out_node_s out_nd;
        struct cpu_node_s cpu_nd;
    } nodes[1];
};

/* Structure of kdp_model_dim */
struct kdp_model_dim_s {
    /* CNN input dimensions */
    uint32_t input_row;
    uint32_t input_col;
    uint32_t input_channel;
};

/* Structure of kdp_pre_proc_s */
struct kdp_pre_proc_s {
    /* input image in memory for NPU */
    uint32_t input_mem_addr;
    int32_t input_mem_len;

    /* Input working buffers for NPU */
    uint32_t input_mem_addr2;
    int32_t input_mem_len2;

    /* data memory for inproc array */
    uint32_t inproc_mem_addr;

    /* number of bits for input fraction */
    uint32_t input_radix;

    /* Other parameters for the model */
    void *params_p;
};

/* Structure of kdp_post_proc_s */
struct kdp_post_proc_s {
    /* output number */
    uint32_t output_num;

    /* output data memory from NPU */
    uint32_t output_mem_addr;
    int32_t output_mem_len;

    /* result data memory from post processing */
    uint32_t result_mem_addr;
    int32_t result_mem_len;

    /* 2nd output data memory for parallel processing  */
    uint32_t output_mem_addr2;
    uint32_t output_mem_len2;

    /* data memory for post processing */
    uint32_t output_mem_addr3;
    uint32_t output_mem_addr4;
    /* output data format from NPU
     *     BIT(0): =0, 8-bits
     *             =1, 16-bits
     */
    uint32_t output_format;

    /* output node parameter */
    struct out_node_s *node_p;

    /* Other parameters for the model */
    void *params_p;
};

/* Structure of kdp_cpu_op_s */
struct kdp_cpu_op_s {
    /* cpu op node parameter */
    struct cpu_node_s *node_p;
};

/* KDP image structure */
struct kdp_image_s {
    /* Original image and model */
    struct kdp_img_raw_s *raw_img_p;
    struct kdp_model_s *model_p;

    int model_id;
    char *setup_mem_p;

    /* Model dimension */
    struct kdp_model_dim_s dim;

    /* Pre process struct */
    struct kdp_pre_proc_s preproc;

    /* Post process struct */
    struct kdp_post_proc_s postproc;

    /* CPU operation struct */
    struct kdp_cpu_op_s cpu_op;
};

/* Helper macros */
#define RAW_INFERENCE_FORMAT(image_p)       (image_p->raw_img_p->inf_format)
#define RAW_IMAGE_MEM_ADDR(image_p)         (image_p->raw_img_p->image_list[0].image_mem_addr)
#define RAW_IMAGE_MEM_LEN(image_p)          (image_p->raw_img_p->image_list[0].image_mem_len)
#define RAW_FORMAT(image_p)                 (image_p->raw_img_p->image_list[0].format)
#define RAW_INPUT_ROW(image_p)              (image_p->raw_img_p->image_list[0].input_row)
#define RAW_INPUT_COL(image_p)              (image_p->raw_img_p->image_list[0].input_col)
#define RAW_CROP_TOP(image_p)               (image_p->raw_img_p->image_list[0].params_s.crop_top)
#define RAW_CROP_BOTTOM(image_p)            (image_p->raw_img_p->image_list[0].params_s.crop_bottom)
#define RAW_CROP_LEFT(image_p)              (image_p->raw_img_p->image_list[0].params_s.crop_left)
#define RAW_CROP_RIGHT(image_p)             (image_p->raw_img_p->image_list[0].params_s.crop_right)
#define RAW_PAD_TOP(image_p)                (image_p->raw_img_p->image_list[0].params_s.pad_top)
#define RAW_PAD_BOTTOM(image_p)             (image_p->raw_img_p->image_list[0].params_s.pad_bottom)
#define RAW_PAD_LEFT(image_p)               (image_p->raw_img_p->image_list[0].params_s.pad_left)
#define RAW_PAD_RIGHT(image_p)              (image_p->raw_img_p->image_list[0].params_s.pad_right)
#define RAW_SCALE_WIDTH(image_p)            (image_p->raw_img_p->image_list[0].params_s.scale_width)
#define RAW_SCALE_HEIGHT(image_p)           (image_p->raw_img_p->image_list[0].params_s.scale_height)
#define RAW_OTHER_PARAMS(image_p)           (image_p->raw_img_p->ext_params)

#define RAW_TICK_START_PRE(image_p)         (image_p->raw_img_p->tick_start_pre)
#define RAW_TICK_END_PRE(image_p)           (image_p->raw_img_p->tick_end_pre)
#define RAW_TICK_START_NPU(image_p)         (image_p->raw_img_p->tick_start_npu)
#define RAW_TICK_END_NPU(image_p)           (image_p->raw_img_p->tick_end_npu)
#define RAW_TICK_START_POST(image_p)        (image_p->raw_img_p->tick_start_post)
#define RAW_TICK_END_POST(image_p)          (image_p->raw_img_p->tick_end_post)

#define DIM_INPUT_ROW(image_p)              (image_p->dim.input_row)
#define DIM_INPUT_COL(image_p)              (image_p->dim.input_col)
#define DIM_INPUT_CH(image_p)               (image_p->dim.input_channel)

#define PREPROC_INPROC_MEM_ADDR(image_p)    (image_p->preproc.inproc_mem_addr)
#define PREPROC_INPUT_MEM_ADDR(image_p)     (image_p->preproc.input_mem_addr)
#define PREPROC_INPUT_MEM_LEN(image_p)      (image_p->preproc.input_mem_len)
#define PREPROC_INPUT_MEM_ADDR2(image_p)    (image_p->preproc.input_mem_addr2)
#define PREPROC_INPUT_MEM_LEN2(image_p)     (image_p->preproc.input_mem_len2)
#define PREPROC_INPUT_RADIX(image_p)        (image_p->preproc.input_radix)
#define PREPROC_PARAMS_P(image_p)           (image_p->preproc.params_p)

#define POSTPROC_OUTPUT_NUM(image_p)        (image_p->postproc.output_num)
#define POSTPROC_OUTPUT_FORMAT(image_p)     (image_p->postproc.output_format)
#define POSTPROC_OUTPUT_MEM_ADDR(image_p)   (image_p->postproc.output_mem_addr)
#define POSTPROC_OUTPUT_MEM_LEN(image_p)    (image_p->postproc.output_mem_len)
#define POSTPROC_RESULT_MEM_ADDR(image_p)   (image_p->postproc.result_mem_addr)
#define POSTPROC_RESULT_MEM_LEN(image_p)    (image_p->postproc.result_mem_len)
#define POSTPROC_PARAMS_P(image_p)          (image_p->postproc.params_p)
#define POSTPROC_OUTPUT_MEM_ADDR2(image_p)  (image_p->postproc.output_mem_addr2)
#define POSTPROC_OUTPUT_MEM_LEN2(image_p)   (image_p->postproc.output_mem_len2)
#define POSTPROC_OUTPUT_MEM_ADDR3(image_p)  (image_p->postproc.output_mem_addr3)
#define POSTPROC_OUTPUT_MEM_ADDR4(image_p)  (image_p->postproc.output_mem_addr4)

#define POSTPROC_OUT_NODE(image_p)          (image_p->postproc.node_p)
#define POSTPROC_OUT_NODE_COL(image_p)      (image_p->postproc.node_p->col_length)
#define POSTPROC_OUT_NODE_ROW(image_p)      (image_p->postproc.node_p->row_length)
#define POSTPROC_OUT_NODE_CH(image_p)       (image_p->postproc.node_p->ch_length)
#define POSTPROC_OUT_NODE_RADIX(image_p)    (image_p->postproc.node_p->output_radix)
#define POSTPROC_OUT_NODE_SCALE(image_p)    (image_p->postproc.node_p->output_scale)
#define POSTPROC_OUT_NODE_ADDR(image_p)     (image_p->postproc.node_p->node_list[0].addr)

#define OUT_NODE_COL(out_p)                 (out_p->col_length)
#define OUT_NODE_ROW(out_p)                 (out_p->row_length)
#define OUT_NODE_CH(out_p)                  (out_p->ch_length)
#define OUT_NODE_RADIX(out_p)               (out_p->output_radix)
#define OUT_NODE_SCALE(out_p)               (out_p->output_scale)
#define OUT_NODE_ADDR(out_p)                (out_p->node_list[0].addr)

#define OUT_NODE_ADDR_PARALLEL(out_p, image_p) \
    (OUT_NODE_ADDR(out_p) + POSTPROC_OUTPUT_MEM_ADDR(image_p) - MODEL_OUTPUT_MEM_ADDR(image_p))

#define CPU_OP_NODE(image_p)                (image_p->cpu_op.node_p)
#define CPU_OP_NODE_OP_TYPE(image_p)        (image_p->cpu_op.node_p->op_type)
#define CPU_OP_NODE_INPUT_COL(image_p)      (image_p->cpu_op.node_p->in_num_col)
#define CPU_OP_NODE_INPUT_ROW(image_p)      (image_p->cpu_op.node_p->in_num_row)
#define CPU_OP_NODE_INPUT_CH(image_p)       (image_p->cpu_op.node_p->in_num_ch)
#define CPU_OP_NODE_INPUT_ADDR(image_p)     (image_p->cpu_op.node_p->input_datanode[0].node_list[0].addr)
#define CPU_OP_NODE_OUTPUT_COL(image_p)     (image_p->cpu_op.node_p->out_num_col)
#define CPU_OP_NODE_OUTPUT_ROW(image_p)     (image_p->cpu_op.node_p->out_num_row)
#define CPU_OP_NODE_OUTPUT_CH(image_p)      (image_p->cpu_op.node_p->out_num_ch)
#define CPU_OP_NODE_OUTPUT_ADDR(image_p)    (image_p->cpu_op.node_p->output_datanode.node_list[0].addr)

#define MODEL_P(image_p)                    (image_p->model_p)
#define MODEL_ID(image_p)                   (image_p->model_id)
#define MODEL_SETUP_MEM_P(image_p)          (image_p->setup_mem_p)
#define MODEL_CMD_MEM_ADDR(image_p)         (MODEL_P(image_p)->cmd_mem_addr)
#define MODEL_CMD_MEM_LEN(image_p)          (MODEL_P(image_p)->cmd_mem_len)
#define MODEL_WEIGHT_MEM_ADDR(image_p)      (MODEL_P(image_p)->weight_mem_addr)
#define MODEL_BUF_ADDR(image_p)             (MODEL_P(image_p)->buf_addr)
#define MODEL_SETUP_MEM_ADDR(image_p)       (MODEL_P(image_p)->setup_mem_addr)
#define MODEL_INPUT_MEM_ADDR(image_p)       (MODEL_P(image_p)->input_mem_addr)
#define MODEL_INPUT_MEM_LEN(image_p)        (MODEL_P(image_p)->input_mem_len)
#define MODEL_OUTPUT_MEM_ADDR(image_p)      (MODEL_P(image_p)->output_mem_addr)
#define MODEL_OUTPUT_MEM_LEN(image_p)       (MODEL_P(image_p)->output_mem_len)

/* API */

/* Return code */
#define RET_ERROR               -1
#define RET_NO_ERROR            0
#define RET_NEXT_PRE_PROC       1
#define RET_NEXT_NPU            2
#define RET_NEXT_CPU            3
#define RET_NEXT_POST_PROC      4

/* Prototypes for callback functions */
// used for both pre/post process 
typedef int (*model_pre_post_fn)(struct kdp_image_s *image_p);
typedef struct model_pre_post_func_s {
    int	model_id;
    model_pre_post_fn	ppf;
} model_pre_post_func_t;

typedef void (*pre_post_fn)(void *dst_p, void *src_p, int size);

/**
 * kdpio_init() - initialize kdpio
 *
 * This function tells kdpio to initialize the platform and
 * resources for NPU support.
 *
 */
void kdpio_init(void);
void npu_reset(void);
void kdpio_sdk_init(void);


/**
 * kdpio_handle_npu_int() - Kneron NPU interrupt handler
 *
 * This is the interrupt handler for Kneron NPU.
 *
 * Return value:
 * 0 : success
 * <0 : error
 */
int kdpio_handle_npu_int(void);

/**
 * kdp_preproc_inproc() - Kneron preprocessing procedure
 *
 * @image_p: pointer to struct kdp_image with buffer of raw image
 *          and dimension, and data for pre/cpu/post processing.
 *
 * @model_id: the model id this function was registered for
 *
 * This is a preprocess function which uses Kneron NPU to accelerate the
 * processing and uses Kneron NCPU to do rotation and right-shift. It can take parameters passed in to do resize, crop, padding
 * normalization like -128, rotation, and right-shift.
 *
 * Return value:
 * 0 : success
 * <0 : error
 */
int kdp_preproc_inproc(int model_id, struct kdp_image_s *image_p);

/**
 * kdpio_set_model() - set the model for an input image
 *
 * @image_p: pointer to struct kdp_image with buffer of raw image
 *          and dimension, and data for pre/cpu/post processing.
 *
 * @model_p: pointer to struct kdp_model with buffers of setup,
 *          command, weights, input and output for npu.
 *
 * This function tells kdpio the CNN model to use for processing next
 * image(s).
 *
 * Return value:
 *  none
 */
void kdpio_set_model(struct kdp_image_s *image_p, struct kdp_model_s *model_p);

/**
 * kdpio_run_preprocess() - run preprocessing for the image
 *
 * @image_p: pointer to struct kdp_image with buffer of raw image
 *          and dimension, and data for pre/cpu/post processing.
 *
 * This function tells kdpio to pre-process the raw image for npu
 * before npu running the model. Its output will be put in model's
 * input buffer for npu.
 *
 * Return value:
 * 0 : success
 * <0 : error
 */
int kdpio_run_preprocess(struct kdp_image_s *image_p);

/**
 * kdpio_run_npu_op() - run cnn model in npu for the image
 *
 * @image_p: pointer to struct kdp_image with buffer of raw image
 *          and dimension, and data for pre/cpu/post processing.
 *
 * This function tells kdpio to run NPU to process the input data
 * with the cnn model previously set.
 *
 * Return value:
 * 0 : success
 * <0 : error
 */
int kdpio_run_npu_op(struct kdp_image_s *image_p);

/**
 * kdpio_run_cpu_op() - run cpu operation for the image
 *
 * @image_p: pointer to struct kdp_image with buffer of raw image
 *          and dimension, and data for pre/cpu/post processing.
 *
 * This function tells kdpio to let cpu run the input image when
 * npu finishes its running. If there is no cpu operation to run as
 * specified by model compiler, the function will still return
 * success so that next step (postprocess) can continue.
 *
 * Return value:
 * 0 : success
 * <0 : error
 */
int kdpio_run_cpu_op(struct kdp_image_s *image_p);

/**
 * kdpio_run_postprocess() - run postprocessing for the image
 *
 * @image_p: pointer to struct kdp_image with buffer of raw image
 *          and dimension, and data for pre/cpu/post processing.
 *
 * @perf_improv_fn: pointer to pre_post_fn callback function to move
 *          npu output to additional buffer for postprocessing while
 *          the original one could be used by npu again.
 *          This is intended to improve fps performance if possible
 *          and desired. Additional output buffer needs to be allocated
 *          for the purpose, and DMA could be used in the callback.
 *
 * This function tells kdpio to post-process the output data from npu
 * before returning to the calling system.
 *
 * Return value:
 * 0 : success
 * <0 : error
 */
int kdpio_run_postprocess(struct kdp_image_s *image_p, pre_post_fn perf_improv_fn);

/**
 * kdpio_exit() - exit kdpio
 *
 * This function tells kdpio to free allocated resources and
 * quit from NPU support.
 *
 */
void kdpio_exit(void);

#endif
