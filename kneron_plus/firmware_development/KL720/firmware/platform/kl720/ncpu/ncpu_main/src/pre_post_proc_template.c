/* ---------------------------------------------------------------------------------------
 * Copyright (c) 2018-2022 Kneron Inc. All rights reserved.
 *
 *      Name:    pre_post_proc_template.c
 *      Purpose: Template for user defined pre process and post process
 *               Demonstrate how to get the necessary data in pre process and post process
 *
 *----------------------------------------------------------------------------------------*/

#if 0

#include "kneron_api_data.h"
#include "post_utils.h"

int pre_proc_template(struct kdp_image_s *image_p)
{
    int ret = 0;

    /* The address of the source image */
    uint32_t src_image_addr = RAW_IMAGE_MEM_ADDR(image_p, 0);
    /* The width of the source image */
    uint32_t src_image_width = RAW_INPUT_COL(image_p, 0);
    /* The height of the source image */
    uint32_t src_image_height = RAW_INPUT_ROW(image_p, 0);
    /* The format of the source image */
    uint32_t src_image_format = RAW_FORMAT(image_p, 0);

    /* Pixels between top boundary of source image and top boundary of crop box */
    int crop_top = RAW_CROP_TOP(image_p, 0);
    /* Pixels between bottom boundary of source image and bottom boundary of crop box */
    int crop_bottom = RAW_CROP_BOTTOM(image_p, 0);
    /* Pixels between left boundary of source image and left boundary of crop box */
    int crop_left = RAW_CROP_LEFT(image_p, 0);
    /* Pixels between right boundary of source image and right boundary of crop box */
    int crop_right = RAW_CROP_RIGHT(image_p, 0);

    /* The width of the cropped image */
    int cropped_image_width = src_image_width - (crop_left + crop_right);
    /* The height of the cropped image  */
    int cropped_image_height = src_image_height - (crop_top + crop_bottom);

    /* Buffer for other params, assigned in user-define data in scpu */
    uint32_t *ext_params = RAW_OTHER_PARAMS(image_p);

    bool is_model_flatbuffer = image_p->is_flatbuffer;

    if (true == is_model_flatbuffer) {
        mdl_quant_factor_t quant;
        tensor_hdl_t t_hdl = mdl_parse_get_outputs_tensor_hdl(image_p->pParsedModelFlatbuffer->in_node_hdl);

        mdl_parse_get_quant_vector_cell(t_hdl, 0, 0, &quant);

        /* Scale of the input node */
        float input_node_scale = quant.scale;

        /* Radix of the input node */
        int32_t input_node_radix = quant.radix;

        /* Data format of the input node */
        uint32_t input_node_data_layout = (uint32_t)mdl_parse_get_fmt(t_hdl, 0);
    } else {
        int node_index = 0;
        NetInput_Node * in_node_p = (NetInput_Node *)image_p->pParsedModel->pNodePositions[node_index];

        /* Scale of the input node */
        float input_node_scale = 1.0;

        /* Radix of the input node */
        int32_t input_node_radix = *(int32_t*)&in_node_p->input_radix;

        /* Data format of the input node */
        uint32_t input_node_data_layout = in_node_p->input_format;
    }

    /* The address of the after-pre-process image */
    uint32_t dst_image_addr = PREPROC_INPUT_MEM_ADDR(image_p, 0);

    /* The width of the after-pre-process image */
    uint32_t dst_image_width = DIM_INPUT_COL(image_p, 0);

    /* The height of the after-pre-process image */
    uint32_t dst_image_height = DIM_INPUT_ROW(image_p, 0);

    /* The channel of the after-pre-process image */
    uint32_t dst_image_channel = DIM_INPUT_CH(image_p, 0);

    /* ==================================================== */
    /*         Performing the pre-processing here           */
    /* ==================================================== */

    return ret;
}

int post_proc_template(struct kdp_image_s *image_p)
{
    int ret = 0;

    /* Pixels between top boundary of source image and top boundary of crop box */
    int crop_top = RAW_CROP_TOP(image_p, 0);
    /* Pixels between bottom boundary of source image and bottom boundary of crop box */
    int crop_bottom = RAW_CROP_BOTTOM(image_p, 0);
    /* Pixels between left boundary of source image and left boundary of crop box */
    int crop_left = RAW_CROP_LEFT(image_p, 0);
    /* Pixels between right boundary of source image and right boundary of crop box */
    int crop_right = RAW_CROP_RIGHT(image_p, 0);

    /* The width of the source image */
    uint32_t src_image_width = RAW_INPUT_COL(image_p, 0);
    /* The hight of the source image */
    uint32_t src_image_height = RAW_INPUT_ROW(image_p, 0);

    /* The width of the cropped image */
    int cropped_image_width = src_image_width - (crop_left + crop_right);
    /* The height of the cropped image  */
    int cropped_image_height = src_image_height - (crop_top + crop_bottom);

    /* Pixels padded on top of image*/
    int pad_top = RAW_PAD_TOP(image_p, 0);
    /* Pixels padded on bottom of image*/
    int pad_bottom = RAW_PAD_BOTTOM(image_p, 0);
    /* Pixels padded on left of image*/
    int pad_left =  RAW_PAD_LEFT(image_p, 0);
    /* Pixels padded on right of image*/
    int pad_right =  RAW_PAD_RIGHT(image_p, 0);
    /* The scale value for width of image */
    int scale_width = RAW_SCALE_WIDTH(image_p, 0);
    /* The scale value for height of image */
    int scale_height = RAW_SCALE_HEIGHT(image_p, 0);

    /* The width of model input image (After-pre-proc image) */
    uint32_t model_input_width = DIM_INPUT_ROW(image_p, 0);
    /* The height of model input image (After-pre-proc image) */
    uint32_t model_input_height = DIM_INPUT_ROW(image_p, 0);

    /* The address of the model output data */
    uint32_t model_output_addr = POSTPROC_OUTPUT_MEM_ADDR(image_p);
    /* The size of the model output data */
    int32_t model_output_size = POSTPROC_OUTPUT_MEM_LEN(image_p);

    /* Buffer for other params, assigned in user-define data in scpu */
    void *post_proc_params = RAW_OTHER_PARAMS(image_p);

    /* The number of output node */
    int output_node_num = POSTPROC_OUTPUT_NUM(image_p);

    struct output_node out_node;

    for (int idx = 0; idx < output_node_num; idx++) {
        get_output_node(&out_node, image_p, idx);

        /* The address of the output node data */
        void *output_node_data = (void *)out_node.base_ptr;

        /* Number of columns of the output node */
        uint32_t output_node_col = out_node.col;

        /* Number of aligned columns of the output node */
        uint32_t output_node_col_aligned = out_node.col_len;

        /* Number of rows of the ouput node */
        uint32_t output_node_row = out_node.row;

        /* Number of channels of the ouput node */
        uint32_t output_node_channel = out_node.ch;

        /* Radix of the output node */
        int32_t output_node_radix = *(int32_t *)&out_node.radix;

        /* Scale of the output node */
        float output_node_scale = *(float *)&out_node.scale;

        /* Data layout of the output node */
        uint32_t output_node_data_layout = out_node.format;
    }

    /* The address of the result of post processing, can be cast to specified struct */
    uint32_t post_proc_result_addr = POSTPROC_RESULT_MEM_ADDR(image_p);

    /* The maximum size for the result of post processing */
    int32_t post_proc_result_size = POSTPROC_RESULT_MEM_LEN(image_p);

    /* ==================================================== */
    /*         Performing the post-processing here          */
    /* ==================================================== */

    return ret;
}

#endif
