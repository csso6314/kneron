#ifndef _JPEG_DEC_CMD_H_
#define _JPEG_DEC_CMD_H_

#define OUT_YUV_NOT_SUPPORT 0
#define OUT_YUV_420 1
#define OUT_YUV_422 2
#define OUT_YUV_444 3


typedef struct DECODER_SETUP_CMD_ {

    // input setup, fill by HOST
    int cmd_id;                  // TBD
    void *bs_start;              // bitstream buffer provied by host
    int bs_len;                  // bitstream len
    int restart;                 // restart bitstream decode from beginning buffer address
    struct dec_enable {
        unsigned int enable_out_gif   : 1;    // MANDATORY!!!
        unsigned int enable_yuv_to_rgb: 1;    // always 0 now, reserved for futher
        unsigned int reserved_1       :30;
    } dec_enable;
    void *out_y_buf;             // Y buffer provide by HOST, decode frameY into this buf
    void *out_cb_buf;            // U buffer provide by HOST, decode frameCb into this buf
    void *out_cr_buf;            // V buffer provide by HOST, decode frameCr into this buf
    void *gif_addr;              // only valid when enable_out_gif is enabled

    // output results, fill by DSP
    int pic_width;               //
    int pic_height;              //
    int frame_num;	             // start from 0
    int out_fmt;                 //  1: 420  2: 422   3: 444
    int dec_frame_time_stamp;    //  used for debug
    int gif_len;                 // only valid when enable_out_gif is enabled

} DECODER_SETUP_CMD ;


typedef DECODER_SETUP_CMD DECODER_STATUS;

#endif
