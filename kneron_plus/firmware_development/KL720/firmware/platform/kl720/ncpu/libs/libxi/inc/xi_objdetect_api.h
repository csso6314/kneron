/*
 * Copyright (c) 2016 by Cadence Design Systems, Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Cadence Design Systems Inc.  They may be adapted and modified by bona fide
 * purchasers for internal use, but neither the original nor any adapted
 * or modified version may be disclosed or distributed to third parties
 * in any manner, medium, or form, in whole or in part, without the prior
 * written consent of Cadence Design Systems Inc.  This software and its
 * derivatives are to be executed solely on products incorporating a Cadence
 * Design Systems processor.
 */

#ifndef __XI_OBJDETECT_API_H__
#define __XI_OBJDETECT_API_H__

#include "xi_core_api.h"

/* RANSAC */
_XI_API_ XI_ERR_TYPE xiRANSAC_line(const xi_pArray data, unsigned model_inliers, unsigned dist_threshold, unsigned max_iters, unsigned term_threshold, int *a, int *b, int *c, int seed);
_XI_API_ XI_ERR_TYPE xiRANSAC_parabola(const xi_pArray data, unsigned model_inliers, unsigned dist_threshold, unsigned max_iters, unsigned term_threshold, int *a, int *b, int *c, int seed);
_XI_API_ XI_ERR_TYPE xiRANSAC_affine(const xi_pArray data_from, const xi_pArray data_to,
                                     unsigned model_inliers, unsigned dist_threshold, unsigned max_iters,
                                     unsigned term_threshold_mme, xi_affine_fpt* model, int seed);


/* Haar */
_XI_API_ XI_ERR_TYPE xiHaarPrepareCascade(int integral_stride, int16_t *cascade);
_XI_API_ XI_ERR_TYPE xiHaarStump_U16U8(const xi_pArray integral_image, const xi_pArray stdDev, int16_t* cascade, xi_pArray detect_map /*inout*/, int xstep, int ystep, xi_size window, int numStages, int *pskipSum);


/* HOG Histogram */
_XI_API_ XI_ERR_TYPE xiHOGBinPlanes_S16(const xi_pArray src_mag, const xi_pArray src_ang_Q3_12, xi_pArray* dst /*inout*/, int nbins);
_XI_API_ XI_ERR_TYPE xiHOGBinPlanes_S16A(const xi_pArray src_mag, const xi_pArray src_ang_Q3_12, xi_pArray* dst /*inout*/, int nbins);
_XI_API_ XI_ERR_TYPE xiHOGGradient_U8S16(const xi_pTile src, xi_pArray dst_dx, xi_pArray dst_dy);
_XI_API_ XI_ERR_TYPE xiHOGWeighted2_S16U32_Q16_15(const xi_pTile src_mag, const xi_pArray src_ang_Q3_12, const xi_pArray weight_Q15, xi_pArray hist /*inout*/, xi_point pt, xi_size cellSize, int nbins, xi_pArray tmp /*inout*/);
_XI_API_ XI_ERR_TYPE xiHOGNormalize_U32U16_Q15(const xi_pArray src, xi_pArray dst, int count);


/* SVM */
_XI_API_ XI_ERR_TYPE xiSVMPredict_S16(const xi_pArray sample, const xi_pArray support_vectors, xi_pArray result);

#endif
