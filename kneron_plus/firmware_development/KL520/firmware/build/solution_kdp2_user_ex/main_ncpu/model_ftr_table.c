/* --------------------------------------------------------------------------
 * Copyright (c) 2018-2020 Kneron Inc. All rights reserved.
 *
 *      Name:    ncpu_extend_ftr.c
 *      Purpose: Extend new features implementation
 *
 *---------------------------------------------------------------------------*/

#include "kdpio.h"
#include "model_type.h"
#include "model_ppp.h"

extern int user_pre_yolo(struct kdp_image_s *image_p);
extern int user_post_yolo(struct kdp_image_s *image_p);

/*********************************************************************************
                Registered model pre-process features list

only need to register functions for models that default builtin pre-proc can't support
*********************************************************************************/
model_pre_post_func_t model_pre_proc_fns[MAX_MODEL_REGISTRATIONS] = {
    /* < model type ID >                                < pre-process function > */
    /* -------------------------------------------------------------------------- */
    0			// no pre-process function is specified  
	
    /* Put customized pre-process functions below: */
    //demo only
    //{ TINY_YOLO_V3_224_224_3,                           user_pre_yolo },

    /*
    { CUSTOMER_MODEL_1,                                 preproc_customer_model_1 },
    { CUSTOMER_MODEL_2,                                 preproc_customer_model_2 },
    { CUSTOMER_MODEL_3,                                 preproc_customer_model_3 },
    */
};

/*********************************************************************************
                Registered model post-process features list
*********************************************************************************/
model_pre_post_func_t model_post_proc_fns[MAX_MODEL_REGISTRATIONS] = {
    /* < model type ID >                                < post-process function > */
    /* -------------------------------------------------------------------------- */

    /*  user post-process function example*/
    { TINY_YOLO_V3_224_224_3,                           user_post_yolo },

    /* use builtin post-process function example*/
    //for face_detection and landmark here using Kneron app functions
    { KNERON_FD_MASK_MBSSD_200_200_3,                   post_ssd_face_detection },
    { KNERON_LM_5PTS_ONET_56_56_3,                      post_face_landmark_onet_5p },


    /* Put customized post-process functions below:*/
    //{ CUSTOMER_MODEL_1,                                 post_customer_model_1 },
    //{ CUSTOMER_MODEL_2,                                 post_customer_model_2 },
    //{ CUSTOMER_MODEL_3,                                 post_customer_model_3 },
};

