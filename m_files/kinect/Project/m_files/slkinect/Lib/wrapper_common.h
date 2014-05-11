/**
 * wrapper_common.h
 *
 * Copyright 2011 The MathWorks, Inc.
 */

#ifndef _WRAPPER_COMMON_H_
#define _WRAPPER_COMMON_H_


//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

// Maximum number of people in skeleton tracking
#define MAX_NUM_OF_SKELETON_TRACKING      (2)

// Colors for motion detected objects of Motion image ouput
#define NUM_OF_MOTION_ID                  (16)
#define MOTION_ID_MASK                    (0xF)

//
// According to OpenNI document, max. 24 skeleton joints could be
// tracked. However, as far as I tested, only 15 skeleton joints would always be tracked
// with confidence and other 9 joints seem to be always no confidence.
// So a mask table is defined to get only joints with always (except for it's lost in the view) confidence.
#define NUM_OF_SKELETON_JOINTS_OPEN_NI     (15)
#define NUM_OF_ALL_SKELETON_JOINTS_OPEN_NI (24)

#define NUM_OF_SKELETON_JOINTS_KINECT_SDK  (20)


//---------------------------------------------------------------------------
// Enum and Typedefs
//---------------------------------------------------------------------------

// NID SDK 
enum eNID_SDK 
{
	OPEN_NI = 0,
	KINECT_SDK,
};

// Structure types to copy data to Simulink
typedef struct
{
    uint8_T nRed;
    uint8_T nGreen;
    uint8_T nBlue;
} RGB24Pixel_T;

typedef struct
{
    uint8_T nBlue;
    uint8_T nGreen;
    uint8_T nRed;
    uint8_T nAlpha;
} RGB32Pixel_T;

typedef struct
{
    real32_T nX;
    real32_T nY;
    real32_T nZ;
} XYZPixel_T;

typedef struct
{
    int xRes;
    int yRes;
} Res_T;

typedef struct
{
    Res_T depthRes;
    Res_T imageRes;
    int   depthFPS;
    int   imageFPS;
} SpecNID_T;


//---------------------------------------------------------------------------
// NID SDK wrappers
//---------------------------------------------------------------------------

//
// Common functions and parameters
//
extern void CopyXYZToSimulink(real_T* outX, real_T* outY, real_T* outZ, XYZPixel_T* src, int xRes, int yRes);
extern void CopyXYZRWToSimulink(real_T* outX, real_T* outY, real_T* outZ, XYZPixel_T* src, int xRes, int yRes);
extern void CopyRGB24ToSimulink(uint8_T* outR, uint8_T* outG, uint8_T* outB, RGB24Pixel_T* src, int xRes, int yRes);
extern int_T GetSpecNID(SpecNID_T* spec);
extern void SetSpecNID(int_T res_fps);
extern boolean_T IsPoseNeededForSkeleton(void);
extern void SetNeedPoseForSkeleton(boolean_T pose);

extern const RGB24Pixel_T SKELETON_CROSS_COLOR;
extern const RGB24Pixel_T MOTION_COLORS[NUM_OF_MOTION_ID];
extern const uint32_T SKELETON_JOINTS_MASK_OPEN_NI[NUM_OF_ALL_SKELETON_JOINTS_OPEN_NI];

//
// NID SDK specific functions
//
extern void     nidDeviceOutputs(SimStruct *S);
extern void     nidDeviceTerminate(SimStruct *S);
extern eNID_SDK nidGetSDK(void);


#endif
/* [EOF] */
