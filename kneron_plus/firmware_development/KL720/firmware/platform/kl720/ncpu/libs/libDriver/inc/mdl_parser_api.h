/* --------------------------------------------------------------------------
 * Copyright (c) 2018-2022 Kneron Inc. All rights reserved.
 *
 *      Name:    moe_parser_api.h
 *      Purpose: Kneron model parser API
 *
 *---------------------------------------------------------------------------*/

#ifndef __MDL_PARSER_API_H__
#define __MDL_PARSER_API_H__

#include <stdint.h>

typedef enum {
    DEFAULT_TGT,
    KL520_TGT,
    KL720_TGT,
    KL530_TGT,
    KL730_TGT,
    KL630_TGT,
    KL540_TGT,
} mdl_target_t;

typedef enum {
    MDL_IN_NODE,
    MDL_OUT_NODE,
    MDL_CPU_NODE,
    MDL_CONST_NODE,
    TOTAL_MDL_NODES
} mdl_node_type_t;

typedef enum {
    MDL_SEQ_DEFAULT,
    MDL_SEG_INPUT,
    MDL_SEG_OUTPUT,
    MDL_SEG_WORK,
    MDL_SEG_CMD,
    MDL_SEG_WT,
    MDL_SEG_SETUP,
    TOTAL_SEGS
} mdl_seg_type_t;

typedef enum {
    MDL_ENCY_NON,
    MDL_ENCY_EFUSE,
    MDL_ENCY_CUSTKEY
} mdl_encrypt_mode_t;

typedef enum {
    DRAM_FMT_UNKNOWN = -1,
    /* conv format */
    DRAM_FMT_1W16C8B         = 0,
    DRAM_FMT_1W16C8BHL       = 1,
    DRAM_FMT_4W4C8B          = 2,
    DRAM_FMT_4W4C8BHL        = 3,
    DRAM_FMT_16W1C8B         = 4,
    DRAM_FMT_16W1C8BHL       = 5,
    DRAM_FMT_8W1C16B         = 6,
    /* psum data format */
    DRAM_FMT_PS_1W16C24B     = 7,
    /* row format */
    DRAM_FMT_RAW8B           = 100,
    DRAM_FMT_RAW16B          = 101,
    DRAM_FMT_RAW_FLOAT       = 102
} mdl_dram_data_fmt_t;

typedef enum {
    DT_None,
    DT_Int8,
    DT_Int16,
    DT_Int32,
    DT_Int64,
    DT_UInt8,
    DT_UInt16,
    DT_Uint32,
    DT_UInt64,
    DT_Float,
    DT_Bfloat16,
    DT_Double
} mdl_data_type_t;

typedef enum {
    ADDR_MODE_DEFAULT,
    ADDR_MODE_ABS,
    ADDR_MODE_REL,
} mdl_addr_mode_t;

typedef void * node_hdl_t;
typedef void * tensor_hdl_t;
typedef void * cpu_operator_hdl_t;
typedef void * session_hdl_t;
typedef void * param_hdl_t;
typedef void * weights_hdl_t;

typedef struct {
    uint32_t batch;
    uint32_t ch;
    uint32_t h;
    uint32_t w;
} mdl_shape_t;

typedef struct {
    uint32_t ch;
    float   scale;
    int32_t radix;
} mdl_quant_factor_t;


/*********************************************************************************
                      Control Functions
**********************************************************************************/
/**
Setup one parsing session

[In]: setup_addr - address of the setup.bin

Return: session handle; NULL - fail
*/
session_hdl_t mdl_parser_open(uint32_t setup_addr, void * pModel);

/**
Close one parsing session

[In]: session - session handle

Return: 0 - success; -1 - fail
*/
int mdl_parser_close(session_hdl_t session);

/*********************************************************************************
                      Global Properties Inqury APIs
**********************************************************************************/

/**
query the hardware target supported by this model

[In]: session_hdl - the parse session handle

Return: supported hardware target ID
*/
mdl_target_t mdl_parse_get_target_id(session_hdl_t session_hdl);

/**
query the encryption mode of this model

[In]: session_hdl - the parse session handle

Return: encryption mode
*/
mdl_encrypt_mode_t mdl_parse_get_ency_mode(session_hdl_t session_hdl);

/**
obtain encryption key of this model

[In]: session_hdl - the parse session handle
[Out]: key - point to encryption key vector flatbuffers_uint8_vec_t

Return: 0 - success; -1 - encryption key does not exist
*/
int mdl_parse_get_ency_key(session_hdl_t session_hdl, const void **key);

/**
query the addressing mode of this model

[In]: session_hdl - the parse session handle

Return: addressing mode
*/
mdl_addr_mode_t mdl_parse_get_addressing_mode(session_hdl_t session_hdl);

/**
query the segment address for the specified segment

[In]: session_hdl - the parse session handle
[In]: seg - segment ID

Return: segment address
*/
uint32_t mdl_parse_get_seg_addr(session_hdl_t session_hdl, mdl_seg_type_t seg);

/**
query the segment length for the specified segment

[In]: session_hdl - the parse session handle
[In]: seg - segment ID

Return: segment address
*/
uint32_t mdl_parse_get_seg_len(session_hdl_t session_hdl, mdl_seg_type_t seg);

/*********************************************************************************
                     Node Utility APIs
**********************************************************************************/

/**
obtain the node handle for the specified node type
this handle is used for further access to this node properties

[In]: session_hdl - the parse session handle
[In]: node_type - node ID

Return: node handle; -1 if that node does not exist (especially CPU node)
*/
node_hdl_t mdl_parse_get_node(session_hdl_t session_hdl, mdl_node_type_t node_type);

/**
query the Tensor total count for inputs for the specified node

[In]: hdl - handle of the node

Return: Tensor count(0 - inputs Tensor does not existing)
*/
int mdl_parse_get_inputs_tensor_cnt(node_hdl_t hdl);

/**
query the Tensor total count for outputs for the specified node

[In]: hdl - handle of the node

Return: Tensor count (0 - outputs Tensor does not existing)
*/
int mdl_parse_get_outputs_tensor_cnt(node_hdl_t hdl);

/**
query the Tensor total count for const inputs for the specified node

[In]: hdl - handle of the node

Return: Tensor count (0 - outputs Tensor does not existing)
*/
int mdl_parse_get_consts_tensor_cnt(node_hdl_t hdl);

/**
obtain the inputs Tensor table for the specified node

[In]: hdl - handle of the node

Return: point to Tensor table; NULL for none
*/
tensor_hdl_t mdl_parse_get_inputs_tensor_hdl(node_hdl_t hdl);

/**
obtain the outputs Tensor table for the specified node

[In]: hdl - handle of the node

Return: handle to Tensor table; NULL for none
*/
tensor_hdl_t mdl_parse_get_outputs_tensor_hdl(node_hdl_t hdl);

/**
obtain the const inputs Tensor table for the specified node

[In]: hdl - handle of the node

Return: handle to Tensor table; NULL for none
*/
tensor_hdl_t mdl_parse_get_consts_tensor_hdl(node_hdl_t hdl);

/*********************************************************************************
                     Tensor Utility APIs
**********************************************************************************/

/**
query the NPU output data format in DRAM for the specified tensor table

[In]: hdl - handle of the tensor table
[In]: idx - index of Tensor vector

Return: NPU data format
*/
mdl_dram_data_fmt_t mdl_parse_get_fmt(tensor_hdl_t hdl, int idx);

/**
query the NPU output data shape (b,c,h,w) for the specified tensor table

[In]: hdl - handle of the tensor table
[In]: idx - index of Tensor vector
[Out]: point to the Shape data (mdl_shape_t)

Return: 0 - sucess; -1 - fail
*/
int mdl_parse_get_shape(tensor_hdl_t hdl, int idx, mdl_shape_t *pShape);

/**
query the NPU output data address for the specified tensor table

[In]: session - parse session handle
[In]: hdl - handle of the tensor table
[In]: idx - index of Tensor vector

Return: the address of the NPU data
*/
uint32_t mdl_parse_get_out_addr(session_hdl_t session, tensor_hdl_t hdl, int idx);

/**
query the NPU output data length for the specified tensor table

[In]: hdl - handle of the tensor table
[In]: idx - index of Tensor vector

Return: the length of the NPU data
*/
uint32_t mdl_parse_get_out_len(tensor_hdl_t hdl, int idx);

/**
query the const input data address for the specified tensor table

[In]: hdl - handle of the tensor table
[In]: idx - index of Tensor vector

Return: the source address of the const input data
*/
uint32_t mdl_parse_get_const_data_addr(tensor_hdl_t hdl, int idx);

/**
query the const input data length for the specified tensor table

[In]: hdl - handle of the tensor table
[In]: idx - index of Tensor vector

Return: the length of the const input data
*/
uint32_t mdl_parse_get_const_data_len(tensor_hdl_t hdl, int idx);

/**
query quantization vector length for the specified tensor table

[In]: hdl - handle of the tensor table
[In]: idx - index of Tensor vector

Return: the length of quantization vector
*/
uint32_t mdl_parse_get_quant_vector_len(tensor_hdl_t hdl, int idx);

/**
query the specified quantization cell

[In]: hdl - handle of the tensor table
[In]: t_idx - index of Tensor vector
[In]: q_idx - index in the quantization vector table
[Out]: pQuant - point to the query result

Return: 0 - sucess; -1 - that cell does not exist
*/
int mdl_parse_get_quant_vector_cell(tensor_hdl_t hdl, int t_idx,
                                    int q_idx, mdl_quant_factor_t *pQuant);

/*********************************************************************************
                     CPU Utility APIs
Note: a cpu "knot" is a sub-node of cpu "node"
**********************************************************************************/

/**
query the cpu knot count for the specified operator table

[In]: hdl - handle of the cpu node

Return: total count of cpu knot
*/
int mdl_parse_get_cpu_knot_count(node_hdl_t hdl);

/**
query the operator information for the specified cpu knot

[In]: hdl - handle of the cpu node
[In]: k_idx - index of the cpu knot
[Out]: opcode - point to retrived operator opcode
[Out]: oper_name - point to operator name string address
[Out]: union_type_id - point to union Kneron_BuiltinOptions_union_type_t type, used to
       match out the parameter type
[Out]: union_type_name - point to the type name string address

Return: the handle to obtain parameter options
*/
param_hdl_t mdl_parse_get_cpu_operator_info(node_hdl_t hdl, int k_idx, uint32_t* opcode,
        char** oper_name, uint32_t* union_type_id, char** union_type_name);

/**
obtain the cpu operator parameters

[In]: pHdl: point to handle (return value of mdl_parse_get_cpu_operator_info())
[In]: opcode: operator code got from call to mdl_parse_get_cpu_operator_info()
[Out]: point to the point of operator parameter struct instance

Return: 0 - success; -1 - fail
*/
int mdl_parse_get_cpu_params(param_hdl_t *pHdl, uint32_t opcode, void **pParam);

/**
query the length of the inputs Tensor vector for the specified cpu knot

[In]: hdl - handle of the cpu node
[In]: k_idx - index of the cpu knot

Return: length of inputs tensor
*/
int mdl_parse_get_cpu_inputs_tensor_len(node_hdl_t hdl, int k_idx);

/**
query the length of the outputs Tensor vector for the specified cpu knot

[In]: hdl - handle of the cpu node
[In]: k_idx - index of the cpu knot

Return: length of outputs tensor
*/
int mdl_parse_get_cpu_outputs_tensor_len(node_hdl_t hdl, int k_idx);

/**
obtain the inputs tensor vector table for the specified cpu knot

[In]: hdl - handle of the cpu node
[In]: k_idx - knot index of the cpu knot table

Return: handle to the inputs tensor table (Kneron_Tensor_table_t), as
     the handle to access Tensor parameters via Tensor Utility APIs
*/
tensor_hdl_t mdl_parse_get_cpu_inputs_tensor_hdl(node_hdl_t hdl, int k_idx);

/**
obtain the outputs tensor vector table for the specified cpu knot

[In]: hdl - handle of the cpu node
[In]: k_idx - knot index of the cpu knot table

Return: handle to the outputs tensor table, as the handle to
        access Tensor parameters via Tensor Utility APIs
*/
tensor_hdl_t mdl_parse_get_cpu_outputs_tensor_hdl(node_hdl_t hdl, int k_idx);

/**
obtain the weight const Data vector table for the specified cpu knot

[In]: hdl - handle of the cpu node
[In]: k_idx - knot index of the cpu knot table

Return: handle to the weights const data table, as the handle to
        access Tensor parameters via Tensor Utility APIs
*/
weights_hdl_t mdl_parse_get_cpu_wights_hdl(node_hdl_t hdl, int k_idx);


#endif  //__MDL_PARSER_API_H__
