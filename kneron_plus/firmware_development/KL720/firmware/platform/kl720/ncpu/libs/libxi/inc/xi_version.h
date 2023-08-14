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


#ifndef XI_LIBRARY_VERSION

#define XI_LIBRARY_VERSION_MAJOR  7
#define XI_LIBRARY_VERSION_MINOR  5
#define XI_LIBRARY_VERSION_PATCH  1

#define XI_MAKE_LIBRARY_VERSION(major, minor, patch)  ((major) * 100000 + (minor) * 1000 + (patch) * 10)
#define XI_AUX_STR_EXP(__A)                           #__A
#define XI_AUX_STR(__A)                               XI_AUX_STR_EXP(__A)

#define XI_LIBRARY_VERSION      (XI_MAKE_LIBRARY_VERSION(XI_LIBRARY_VERSION_MAJOR, XI_LIBRARY_VERSION_MINOR, XI_LIBRARY_VERSION_PATCH))
#define XI_LIBRARY_VERSION_STR  XI_AUX_STR(XI_LIBRARY_VERSION_MAJOR) "." XI_AUX_STR(XI_LIBRARY_VERSION_MINOR) "." XI_AUX_STR(XI_LIBRARY_VERSION_PATCH)

#endif
