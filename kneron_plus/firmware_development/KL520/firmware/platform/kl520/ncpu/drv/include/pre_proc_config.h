#ifndef __PRE_PROC_CONFIG_H_
#define __PRE_PROC_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define min(a, b)                (((a) < (b)) ? (a) : (b))

#define QUEUE_CONV               0
#define QUEUE_RDMA               1
#define QUEUE_WDMA               2
#define QUEUE_GETW               3

#define NUM_BANK_LINE            32    // for half SRAM data
#define BANK_ENTRY_CNT           512
#define BANK_ENTRY_SIZE_BYTE     16
#define NM_AREA_1                0x01  // bank 0-31

#define MAX_IMG_PREPROC_ROW_NUM  511
#define MAX_IMG_PREPROC_COL_NUM  256
#define IMG_PREPROC_UNIT_BYTES   4

#define MODEL_END_INTR_VAL       0x81

#define FMT_RGBA_8BIT            1
#define DATA_FMT                 FMT_RGBA_8BIT
#define FMT_CVT                  true

#define CFG_TYPE_MASK            0x000f
#define CFG_SUB_TYPE_MASK        0x0007
#define CFG_TYPE_BIT             4
#define CFG_ALPHA_FIRST_BIT      3

#define IMG_FMT_RGBA8888         0
#define IMG_FMT_NIR              2
#define IMG_FMT_YCbCr422         3
#define IMG_FMT_YCbCr444         5
#define IMG_FMT_RGB565           6

#define MODE_VAL(type, order)    (((type) << CFG_TYPE_BIT) + (order))

// define pad mode
#define PAD_TWO_SIDE             0
#define PAD_ONE_SIDE             1
#define PAD_NONE                 2

/* #################################
 * ##    Pre-process Parameter    ##
 * ################################# */
#define ALPHA_FIRST              false
#define Y_SUB_128                false
#define UV_SUB_128               false
#define OFF_PIXEL                0

typedef int Pad[4];
typedef int Crop[4];

typedef struct {
    float x0;
    float x1;
    float y0;
    float y1;
} CBOX;
typedef struct {
    uint32_t input_addr;
    uint32_t output_addr;
    int      orig_row;
    int      orig_col;
    int      orig_chnl;
    int      row;
    int      col;
    int      chnl;
    int      x_offset;
    int      y_offset;
    int      fmt;
    int      alpha_first;
    int      sub_val;
    int      pad_val;
    int      shift_val;
    int      pad_left;
    int      pad_top;
    int      pad_right;
    int      pad_bot;
    int      crop_left;
    int      crop_top;
    int      crop_right;
    int      crop_bot;
} cfg_param;// preproc_cfg

typedef struct {
    bool enable;
    bool fmt_cvt;
    bool y_sub_128;
    bool uv_sub_128;
    int fmt_cfg;
    int off_pixel;
    int row;
    int col;
    int chnl;
    int sub_val;
    int shift_val;
    int pad_val;
    int byte_per_pixel;
    size_t img_len;
    Pad pad_num;                    /* left, up, right, bottom */
    Crop crop_num;                  /* left, up, right, bottom */
} preproc_cfg;// preproc_cfg

typedef struct {
    int col_unit;                   /* col per entry */
    int chnl_unit;                  /* chnl per entry */
    int chnl_grp_unit;              /* chnl per grp */
    int bank_grp_unit;              /* bank per grp */
    int chnl_intlv_step;            /* chnl_interleave_step (in entry) */
} fmt_param;

/**
 * @brief Describes data layout in NMEM
 *        Note that w & l are different for FM & DMA;
 *        they have different interpretations for w & l.
 */
typedef struct {
    int area;                       /* bank area */
    int off_x;                      /* X offset in entry */
    int off_y;                      /* Y offset in line */
    int w;                          /* entry width, shall >= 4 */
    int h;                          /* height in line (FM/STORE/PSUM) */
    int l;                          /* line count to compose one row (FM/STORE/PSUM) */
    int dma_h;                      /* height in line (for DMA) */
    int dma_l;                      /* line count to compose one row (for DMA) */
} nmem_cfg;

/* DMA param used for data in external MEM */
typedef struct {
    int pitch;                      /* offset to next line start (bytes) */
    int len;                        /* block transfer length (bytes) */
    int line;                       /* block DMA lines */
    int start_l;                    /* start line offset */
} dma_param;

typedef struct {
    int src_offset;                 /* in pixel */
    int src_h;
    int src_w;
    int dst_h;
    int dst_w;
    int ratio_h;
    int ratio_w;
} resize_param;

typedef struct
{
    bool enable_sw_padding;
    bool is_bottom_padding; // if this is true, we can just make bottom area black,
                            // otherwise needs another temp buffer to output inproc then dma copy
    int img_inproc_width;   // image width after inproc, it may not equal to model input width
    int img_inproc_height;  // image hight after inproc, it may not equal to model input height
    int pad_value[4]; // left, top, right, bottom
} padding_control;

int pad_to_n(int val, int n);
inline bool is_align_16b(int addr);
inline bool is_align_4b(int addr);
int get_gcd_2(int x, int y);
static void check_addr_align(uint32_t addr, int byte);
static int get_opt_nmem_w(int input_col, int pad_mode, int pad_right);

static void pre_proc_rounding_crop_box(CBOX *box);
static void pre_proc_align_crop_box_w_to_4(CBOX *box, int w);
static void pre_proc_adjust_crop_box_by_ar(CBOX *box, float ar);
static void pre_proc_get_crop_range(CBOX *box, int image_w, int image_h);
int calculate_crop_param(int model_id, struct kdp_image_s *image_p, int in_row, int in_col, int out_row, int out_col,
                         int *src_format);

void set_init_param(struct kdp_image_s *image_p, int in_row, int in_col, int out_row, int out_col, int out_chnl);
preproc_cfg set_preproc_cfg(void);
int get_fmt_cfg(uint32_t fmt, bool alpha_first, bool fmt_cvt, bool y_sub_128, bool uv_sub_128);
int get_byte_per_pixel(int fmt_cfg);
int get_lines(nmem_cfg cfg);
int get_entry_num(nmem_cfg cfg);
fmt_param get_fmt_param(void);
int get_chnl_exp(int chnl, const fmt_param p);
int get_col_exp(int col, const fmt_param p, int w);
dma_param get_dma_param(int row, int col, int chnl);
nmem_cfg get_nmem_cfg(int row, int col, int chnl, int area, int off_x, int off_y);
int *get_pad_num(int *pad_num_orig, bool left, bool up, bool right, bool bottom);
dma_param get_rdma_param(int row_num, int col_num, const preproc_cfg prep_cfg);
size_t cal_img_data_offset(preproc_cfg prep_cfg, int start_row, int total_row);
int cal_data_len(int row, int col, int chnl);
inline uint32_t cal_scale_ratio(int src, int dst);
resize_param get_resize_param(preproc_cfg prep_cfg, int model_row, int model_col, int data_row, int data_col);
void update_nmem_pos(int *off_x, int *off_y, int line_step, int w);
int get_img_cut_pieces(int out_row, int max_row, int pad_mode, int pad_bottom, bool *big_pad, int *last_row,
                       bool col_align);

void gen_rdma_cmd(int idx, int queue, int row, int col, int off_x, int off_y, int pad_mode, int cut_index,
                  int cut_total, uint32_t addr, preproc_cfg prep_cfg);
void rdma_cmd_impl(int idx, int queue, uint32_t addr, const dma_param p, const nmem_cfg nmem_cfg, int *pad_num,
                   const resize_param rzp, preproc_cfg prep_cfg, bool last_i);
void gen_wdma_cmd(int idx, int queue, int row_num, int col_num, int off_x, int off_y, uint32_t addr);
void gen_inproc_cmd(int idx, int queue, int *pad_num, const resize_param rzp, preproc_cfg prep_cfg);

void conf_npu_reg(int queue, int reg_idx, int value);
void gen_npu_rdma_cmd(int queue, bool last_i);
void gen_npu_wdma_cmd(int queue);
void gen_npu_intr_cmd(int queue, int intr_num);

int gen_pre_proc_cmd(struct kdp_image_s *image_p, int in_row, int in_col, int out_row, int out_col, int out_chnl,
                     int pad_mode);

int pre_proc_config(int model_id, struct kdp_image_s *image_p,  padding_control *padding_ctrl);

#endif
