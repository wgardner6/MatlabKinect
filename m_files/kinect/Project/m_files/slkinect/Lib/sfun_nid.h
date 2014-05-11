/**
 * sfun_nid.h
 *
 * Copyright 2011 The MathWorks, Inc.
 */

#ifndef _SFUN_NID_H_
#define _SFUN_NID_H_


// S-Function macros
#define S_FUNCTION_NAME  sfun_nid
#define S_FUNCTION_LEVEL 2

// S-Function
#include "simstruc.h"


//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

//
// Block parameters
//
#define NUM_OF_PARAMS            (23)

// Paramter accessors
#define BLOCK_TYPE(S)                (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,0))))
#define IMAQ_RES_FPS(S)              (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,1))))
#define IMAQ_SAMPLE_TIME(S)                  *(real_T*)(mxGetPr(ssGetSFcnParam(S,2)))
#define IMAQ_DEPTH(S)                (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,3))))
#define IMAQ_MOTION(S)               (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,4))))
#define IMAQ_IMAGE(S)                (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,5))))
#define IMAQ_SKELETON(S)             (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,6))))
#define IMAQ_IR(S)                   (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,7))))
#define DEPTH_OUTPUT(S)              (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,8))))
#define IMAGE_OUTPUT(S)              (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,9))))
#define VIEWER(S)                    (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,10))))
#define ADJUST_VIEW_POINT(S)         (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,11))))
#define MIRROR_IMAGE(S)              (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,12))))
#define CONV_XYZ_RW(S)               (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,13))))
#define NEED_PSI_POSE(S)             (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,14))))
#define KINECT_ANGLE(S)              (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,15))))
#define KINECT_NEAR_MODE(S)          (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,16))))
#define KINECT_ANGLE_READ(S)         (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,17))))
#define NUM_OF_SKELETON_TRACKING(S)  (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,18))))
#define KINECT_SEATED_MODE(S)        (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,19))))
#define KINECT_ANGLE_INPUT(S)        (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,20))))
#define KINECT_SKELETON_SMOOTH(S)             (real_T*)(mxGetPr(ssGetSFcnParam(S,21)))
#define KINECT_INFER_SKELETON(S)     (int_T)(*(real_T*)(mxGetPr(ssGetSFcnParam(S,22))))

// Check box on/off
#define _OFF                     (0)
#define _ON                      (1)

// Block types
#define IMAQ_BLOCK               (1)
#define DEPTH_BLOCK              (2)
#define MOTION_BLOCK             (3)
#define IMAGE_BLOCK              (4)
#define SKELETON_BLOCK           (5)
#define IR_BLOCK                 (6)

// IMAQ resolution and fps
#define RESET_RES_FPS            (0) // parameter value for reset
#define QVGA_60FPS               (1) // valid for only ASUS Xtion Pro series
#define VGA_30FPS                (2)

#define MIN_IMAQ_SAMPLE_TIME_QVGA (1.0/60.0)
#define MIN_IMAQ_SAMPLE_TIME_VGA  (1.0/30.0)

// Depth outputs
#define DEPTH_SIGNAL             (1)
#define XYZ_SIGNAL               (2)
#define X_Y_Z_SIGNAL             (3)

// Image outputs
#define RGB_SIGNAL               (1)
#define R_G_B_SIGNAL             (2)


#endif
/* [EOF] */
