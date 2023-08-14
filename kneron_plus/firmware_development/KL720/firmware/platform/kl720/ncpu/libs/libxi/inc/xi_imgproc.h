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


#ifndef __XI_IMGPROC_H__
#define __XI_IMGPROC_H__

#include "xi_imgproc_api.h"
#include "xi_core.h"


// affine transform decomposition flags
enum { AFFINE_INVALID = 0,
       AFFINE_A,
       AFFINE_B,
       AFFINE_C,
       AFFINE_D,
       AFFINE_A_NEG,
       AFFINE_B_NEG,
       AFFINE_C_NEG,
       AFFINE_D_NEG,
       AFFINE_A_INV,
       AFFINE_B_INV,
       AFFINE_C_INV,
       AFFINE_D_INV,
       AFFINE_A_NEG_INV,
       AFFINE_B_NEG_INV,
       AFFINE_C_NEG_INV,
       AFFINE_D_NEG_INV,


       AFFINE_ENFORCE_A   = 1 << 30,
       AFFINE_ENFORCE_B   = 1 << 29,
       AFFINE_ENFORCE_C   = 1 << 28,
       AFFINE_ENFORCE_D   = 1 << 27,

       AFFINE_ENFROCE_ABC = AFFINE_ENFORCE_A | AFFINE_ENFORCE_B | AFFINE_ENFORCE_C,
       AFFINE_ENFROCE_ABD = AFFINE_ENFORCE_A | AFFINE_ENFORCE_B | AFFINE_ENFORCE_D,
       AFFINE_ENFROCE_ACD = AFFINE_ENFORCE_A | AFFINE_ENFORCE_C | AFFINE_ENFORCE_D,
       AFFINE_ENFROCE_BCD = AFFINE_ENFORCE_B | AFFINE_ENFORCE_C | AFFINE_ENFORCE_D
     };


// internal API

_XI_EXTERN_C_ void xiFlipHorizontal_I8_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiFlipHorizontal_I8A_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiFlipHorizontal_I8Oa_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiFlipHorizontal_I16_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiFlipHorizontal_I16A_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiFlipHorizontal_I16Oa_unchecked(const xi_pArray src, xi_pArray dst);

_XI_EXTERN_C_ void xiTranspose_I8_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiTranspose_I8A_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiTranspose_I8MSOa_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiTranspose_I8MSIa_unchecked(const xi_pArray src, xi_pArray dst);

_XI_EXTERN_C_ void xiTranspose_I16_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiTranspose_I16A_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiTranspose_I16MSOa_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiTranspose_I16MSIa_unchecked(const xi_pArray src, xi_pArray dst);

_XI_EXTERN_C_ void xiRotateClockwise_I8_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiRotateClockwise_I8A_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiRotateClockwise_I8MSIa_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiRotateClockwise_I16_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiRotateClockwise_I16A_unchecked(const xi_pArray src, xi_pArray dst);
_XI_EXTERN_C_ void xiRotateClockwise_I16MSIa_unchecked(const xi_pArray src, xi_pArray dst);

#endif
