/**
 * wrapper_common.cpp
 *
 * This is common device SDK wrapper functions for Simulink NID.
 *
 * Copyright 2011 The MathWorks, Inc.
 */


// S-Function
#include "sfun_nid.h"
#include "wrapper_common.h"


//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------

const RGB24Pixel_T SKELETON_CROSS_COLOR = {255, 255, 255};

const RGB24Pixel_T MOTION_COLORS[NUM_OF_MOTION_ID] = 
{
    // Background color 
    { 0,   0,   0   }, // black
    // Colors for motion detected objects 
    { 255, 0,   0   }, // red
    { 0,   255, 0   }, // green
    { 0,   0,   255 }, // blue
    { 255, 255, 0   }, // yellow
    { 255, 0,   255 }, // cyan
    { 0,   255, 255 }, // magenda
    { 255, 165, 0   }, // orange
    { 0,   128, 0   }, // dark green
    { 0,   128, 128 }, // teal
    { 128, 0,   128 }, // purple
    { 128, 128, 0   }, // olive
    { 128, 0,   0,  }, // maroon
    { 192, 192, 192 }, // silver
    { 0,   0,   128 }, // navy
    { 255, 215, 0   }, // gold
};

// Table mask value = 0 means (always) no confidence, so skip them.
const uint32_T SKELETON_JOINTS_MASK_OPEN_NI[NUM_OF_ALL_SKELETON_JOINTS_OPEN_NI] = 
{
    1, // XN_SKEL_HEAD
    1, // XN_SKEL_NECK
    1, // XN_SKEL_TORSO
    0, // XN_SKEL_WAIST

    0, // XN_SKEL_LEFT_COLLAR
    1, // XN_SKEL_LEFT_SHOULDER
    1, // XN_SKEL_LEFT_ELBOW
    0, // XN_SKEL_LEFT_WRIST
    1, // XN_SKEL_LEFT_HAND
    0, // XN_SKEL_LEFT_FINGERTIP

    0, // XN_SKEL_RIGHT_COLLAR
    1, // XN_SKEL_RIGHT_SHOULDER
    1, // XN_SKEL_RIGHT_ELBOW
    0, // XN_SKEL_RIGHT_WRIST
    1, // XN_SKEL_RIGHT_HAND
    0, // XN_SKEL_RIGHT_FINGERTIP

    1, // XN_SKEL_LEFT_HIP
    1, // XN_SKEL_LEFT_KNEE
    0, // XN_SKEL_LEFT_ANKLE
    1, // XN_SKEL_LEFT_FOOT

    1, // XN_SKEL_RIGHT_HIP
    1, // XN_SKEL_RIGHT_KNEE
    0, // XN_SKEL_RIGHT_ANKLE
    1, // XN_SKEL_RIGHT_FOOT 
};

static int_T  imaq_res_fps = RESET_RES_FPS;

static boolean_T need_pose = true;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

//
// NID matrix data (general matrix data in PC software) vs. MATLAB matrix data in terms of memory alignment.
// - NID matrix data is packed as each matrix element (e.g. RGB) and aligned from bottom left to top right with column oriented direction.
// - MATLAB matrix data is packed as each dimension (e.g. R/G/B) and aligned from bottom left to top right with row oriented direction.
//
// [NID matrix data alignment]
//        ____________________________________ End of 3 dims data
//        |                                  |
//        |                                  |
//        |                                  |
//        |                                  |
//        |                                  |
//        |                                  |
//        |__________________________________|
//        |__________________________________|
//  Start of 3 dims data 
//
//
// [MATLAB matrix data alignment]
//        ____________________________________ End of dim3 (End of 3 dims data)
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        |_|________________________________|
//  Start of dim3                                      
//        ____________________________________ End of dim2 
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        |_|________________________________|
//  Start of dim2                                      
//        ____________________________________ End of dim1 
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        | |                                |
//        |_|________________________________|
//  Start of dim1 (Start of 3 dims data)                                     
//

/**
 * Copy XYZ projective cordinates data to Simulink
 * @param  outX X coodinate output
 * @param  outY Y coodinate output
 * @param  outZ Z coodinate output
 * @param  src  XYZ source 
 * @param  xRes Resolution of X axis
 * @param  yRes Resolution of Y axis
 * @return None
 */
void CopyXYZToSimulink(real_T* outX, real_T* outY, real_T* outZ, XYZPixel_T* src, int xRes, int yRes)
{
    for (int x=0; x<xRes; x++)
    {
        for (int y=0; y<yRes; y++)
        {
            XYZPixel_T* pXYZ = (XYZPixel_T*)(src + x + xRes * y);
            *(outX++) = (real_T)((int_T)pXYZ->nX); // pixel position
            *(outY++) = (real_T)((int_T)pXYZ->nY); // pixel position
            *(outZ++) = (real_T)pXYZ->nZ * 0.001;  // Convert mm to m
        }
    }
}


/**
 * Copy XYZ real world cordinates data to Simulink
 * @param  outX X coodinate output
 * @param  outY Y coodinate output
 * @param  outZ Z coodinate output
 * @param  src  XYZ source 
 * @param  xRes Resolution of X axis
 * @param  yRes Resolution of Y axis
 * @return None
 */
void CopyXYZRWToSimulink(real_T* outX, real_T* outY, real_T* outZ, XYZPixel_T* src, int xRes, int yRes)
{
    for (int x=0; x<xRes; x++)
    {
        for (int y=0; y<yRes; y++)
        {
            XYZPixel_T* pXYZ = (XYZPixel_T*)(src + x + xRes * y);
            // convert mm to m
            *(outX++) = (real_T)pXYZ->nX * 0.001;
            *(outY++) = (real_T)pXYZ->nY * 0.001;
            *(outZ++) = (real_T)pXYZ->nZ * 0.001;
        }
    }
}


/**
 * Copy RGB24 data to Simulink
 * @param  outR Red output
 * @param  outG Green output
 * @param  outB Blue output
 * @param  src  RGB24 source 
 * @param  xRes Resolution of X axis
 * @param  yRes Resolution of Y axis
 * @return None
 */
void CopyRGB24ToSimulink(uint8_T* outR, uint8_T* outG, uint8_T* outB, RGB24Pixel_T* src, int xRes, int yRes)
{
    for (int x=0; x<xRes; x++)
    {
        for (int y=0; y<yRes; y++)
        {
            RGB24Pixel_T* pRGB = (RGB24Pixel_T*)(src + x + xRes * y);
            *(outR++) = pRGB->nRed;
            *(outG++) = pRGB->nGreen;
            *(outB++) = pRGB->nBlue;
        }
    }
}


/**
 * Get specifications (resolution and FPS) of depth and image depeding on NID
 * @param  spec Specification of NID
 * @return status 0(OK)/1(Error)
 */
int_T GetSpecNID(SpecNID_T* spec)
{
	int_T ret = 0;
	
    if (imaq_res_fps == VGA_30FPS)
    {
        // VGA
        spec->depthRes.xRes = 640;
        spec->depthRes.yRes = 480;
        spec->depthFPS = 30;

        spec->imageRes.xRes = 640;
        spec->imageRes.yRes = 480;
        spec->imageFPS = 30;
    }
    else if (imaq_res_fps == QVGA_60FPS)
    {
		// Currently works with only Asus Xtion Pro series
        // QVGA
        spec->depthRes.xRes = 320;
        spec->depthRes.yRes = 240;
        spec->depthFPS = 60;

        spec->imageRes.xRes = 320;
        spec->imageRes.yRes = 240;
        spec->imageFPS = 60;
	}
	else
	{
        // VGA (just to avoid SegV)
        spec->depthRes.xRes = 640;
        spec->depthRes.yRes = 480;
        spec->depthFPS = 30;

        spec->imageRes.xRes = 640;
        spec->imageRes.yRes = 480;
        spec->imageFPS = 30;

		ret = 1; // error
	}
	
	return ret;
}


/**
 * Set specifications (resolution and FPS) of depth and image based on an IMAQ block parameter
 * @param  res_fps resolution and fps
 * @return none
 */
void SetSpecNID(int_T res_fps)
{
	imaq_res_fps = res_fps;
}


/**
 * Check the status of "Need PSI pose for calibration" parameter in Skeleton block
 * @param none
 * @return status of "Need PSI pose for calibration" parameter (_ON/_OFF)
 */
boolean_T IsPoseNeededForSkeleton(void)
{
	return need_pose;
}


/**
 * Set the status of "Need PSI pose for calibration" parameter in Skeleton block
 * @param pose status of "Need PSI pose for calibration" parameter (_ON/_OFF)
 * @return none
 */ 
void SetNeedPoseForSkeleton(boolean_T pose)
{
	need_pose = pose;
}

/* [EOF] */