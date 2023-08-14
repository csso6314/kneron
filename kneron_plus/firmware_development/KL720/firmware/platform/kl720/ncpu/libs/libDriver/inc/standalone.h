/**
 * @file      standalone.h
 * @brief     customized feature functions
 * @copyright (c) 2020-2022 Kneron Inc. All right reserved.
 */

#ifndef __STANDALONE_H__
#define __STANDALONE_H__

/*********************************************************************************
                  Non-CNN/RNN related functions
*********************************************************************************/
int standalone_crop_resize_process(void  *pIn,  void  *pOut);
int standalone_tof_dec_process(void  *pIn,  void  *pOut);
int standalone_tof_ir_bright_process(void *pIn,  void  *pOut);

#if ENABLE_JPEGLIB
int standalone_jpeg_enc_process(void  *pIn,  void  *pOut);
int standalone_jpeg_dec_process(void  *pIn,  void  *pOut);
#endif

#endif    /* __STANDALONE_H__ */

