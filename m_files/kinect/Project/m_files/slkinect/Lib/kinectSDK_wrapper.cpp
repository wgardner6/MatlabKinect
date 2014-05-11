/**
 * kinectSDK_wrapper.cpp
 *
 * This is Microsoft kinect SDK wrapper for Simulink NID (Natural Interaction Device).
 * It provides two main functions need to be called in CMEX S-Function methods
 *
 * void nidDeviceOutputs(SimStruct *S): needs to be called in mdlOutputs
 * void nidDeviceTerminate(SimStruct *S): needs to be called in mdlTerminates
 *
 * Copyright 2012 The MathWorks, Inc.
 */

#include <vector>

#include <Windows.h>
#include <NuiApi.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib") // for timeGetTime

// S-Function
#include "sfun_nid.h"
#include "wrapper_common.h"


//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

#define ERROR_INIT_NID "NID Error: Failed to initialize NID. It might be caused by\n- NID is not connected.\n- NID unsupported block is used in the model."

#define KINECT_SENSOR_ANGLE_TIME_OUT_MS  (5000) // msec
// This interval value is derived from the hardware restriction of the Kinect according to
// Kinect SDK C++ reference.
#define KINECT_SET_SENSOR_ANGLE_INTERVAL (1000) // msec


//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------

volatile bool gIsFirstTimePerSim = TRUE;

INuiSensor* gNuiSensor = NULL;

HANDLE gDepthEvent = NULL;
HANDLE gImageEvent = NULL;
HANDLE gIrEvent = NULL;
HANDLE gSkeletonEvent = NULL;

HANDLE gDepthFrameHandle = NULL;
HANDLE gImageFrameHandle = NULL;
HANDLE gIrFrameHandle = NULL;

INuiCoordinateMapper* gCoordinateMapper = NULL;

NUI_IMAGE_FRAME gDepthFrame;
NUI_IMAGE_FRAME gImageFrame;
NUI_IMAGE_FRAME gIrFrame;
NUI_SKELETON_FRAME gSkeletonFrame;

uint16_T* gDepthBuffer = NULL;
uint16_T* gMotionBuffer = NULL;
uint16_T* gIrBuffer = NULL;
RGB24Pixel_T* gImageBuffer = NULL;
XYZPixel_T* gSkeletonBuffer = NULL;

RGB24Pixel_T* gRGB = NULL;
XYZPixel_T* gXYZ = NULL;
XYZPixel_T* gXYZ_RW = NULL;

volatile uint32_T lastIMAQTime;
volatile uint32_T lastSetSensorAngleTime;

static bool INFER_SKELETON;

static char msg[1024];


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

/**
 * Mirror (flip) depth or motion frame horizontally.
 * @param bug Depth/Motion frame data
 * @param res Frame resolution
 */
static void nidMirrorDepthMotion(const uint16_T* buf, Res_T res)
{
	int cRes = res.xRes / 2;
	for (int y = 0; y < res.yRes; y++)
	{
		for (int x = 0; x < cRes; x++)
		{
			// Swap left and right pixels
			uint16_T* lp = (uint16_T*)(buf + x + y * res.xRes); 
			uint16_T* rp = (uint16_T*)(buf + res.xRes - 1 - x + y * res.xRes);
			uint16_T tmp = *lp;
			*lp = *rp;
			*rp = tmp; 
		}
	}
}


/**
 * Mirror (flip) depth or motion frame horizontally.
 * @param bug Image frame data
 * @param res Frame resolution
 */
static void nidMirrorImage(const RGB24Pixel_T* buf, Res_T res)
{
	int cRes = res.xRes / 2;
	for (int y = 0; y < res.yRes; y++)
	{
		for (int x = 0; x < cRes; x++)
		{
			// Swap left and right pixels
			RGB24Pixel_T* lp = (RGB24Pixel_T*)(buf + x + y * res.xRes); 
			RGB24Pixel_T* rp = (RGB24Pixel_T*)(buf + res.xRes - 1 - x + y * res.xRes);
			RGB24Pixel_T tmp = *lp;
			*lp = *rp;
			*rp = tmp; 
		}
	}
}


/**
 * Convert projective XYZ coordinates to real world XYZ coordinates
 * @param dp Real world XYZ coordinates
 * @param sp Projective XYZ coordinates
 * @param res Frame resolution
 */
void nidConvertProjectiveToRealWorld(XYZPixel_T* dp, const XYZPixel_T* sp, Res_T res)
{
	//
	// The formula used to calculate real world X/Y/Z coordinates which is excerpt from Mr Nicoras Burrus's website:
	// http://nicolas.burrus.name/index.php/Research/KinectCalibration
	//
	const real32_T SCALE_X = 0.001682894418929f; // = 1 / 5.9421434211923247e+02
	const real32_T SCALE_Y = 0.001691931326959f; // = 1 / 5.9104053696870778e+02
	
	real32_T cpixel_X = (real32_T)res.xRes / 2; 
	real32_T cpixel_Y = (real32_T)res.yRes / 2; 
	int npixels = res.xRes * res.yRes;
	for (int i = 0; i < npixels; i++)
	{
		dp->nX = (sp->nX - cpixel_X) * sp->nZ * SCALE_X;
		dp->nY = (cpixel_Y - sp->nY) * sp->nZ * SCALE_Y;
		dp->nZ = sp->nZ;
		dp++;
		sp++;
	} 
}


/**
 * Create Depth RGB image 
 * @param S Simulink S-Function structure
 * @param pDepth Depth frame data
 * @param res Frame resolution
 * @param maxDepth Maximum depth per a frame
 * @return None
 */
static void nidCreateDepthRGB(SimStruct *S, const uint16_T* pDepth, Res_T res, uint16_T maxDepth) 
{
    int npixels = res.xRes * res.yRes;
	uint16_T* depthRun;
	int SIZE_OF_DEPTH_HIST = maxDepth + 1; // Size of buffer > maxDepth
    std::vector<real_T> depthHist(SIZE_OF_DEPTH_HIST); 

    // Count up each non-zero depth value 
    int numOfPoints = 0;
	depthRun = (uint16_T*)pDepth;
    for (int i = 0; i < npixels; i++)
    {
        if (*depthRun != 0)
        {
            depthHist[*depthRun]++;
            numOfPoints++;
        }
        depthRun++;
    }

    // Create depth histogram
    for (int i = 1; i < SIZE_OF_DEPTH_HIST; ++i)
    {
        depthHist[i] += depthHist[i-1];
    }

    // Convert to RGB value (0 to 255)
    if ( numOfPoints != 0)
    {
        for (int i = 1; i < SIZE_OF_DEPTH_HIST; ++i)
        {
            // Set color histogram for depth (brighter color means closer)
            depthHist[i] = (real_T)(255 * (1.0f - (depthHist[i] / numOfPoints)));
        }
    }
        
    // Overdraw image by color histogram
    memset(gRGB, 0, res.xRes * res.yRes * sizeof(RGB24Pixel_T)); // Black: Depth = 0 
	RGB24Pixel_T* rgbRun = gRGB;
	depthRun = (uint16_T*)pDepth;
    for (int i = 0; i < npixels; i++)
    {
        if (*depthRun != 0)
        {
           // Depth histogram image with green color
           rgbRun->nGreen = (uint8_T)depthHist[*depthRun];
        }
        rgbRun++;
        depthRun++;
    }
}


/**
 * Create Motion RGB image
 * @param S Simulink S-Function structure
 * @param pScene Scene (Motion) frame data
 * @param res Frame resolution
 * @return None
 */
static void nidCreateMotionRGB(SimStruct *S, const uint16_T* pScene, Res_T res) 
{
    int npixels = res.xRes * res.yRes;

	RGB24Pixel_T* rgbRun = gRGB;
    for (int i = 0; i < npixels; i++)
    {
        *rgbRun = *(MOTION_COLORS + (*pScene & MOTION_ID_MASK));
        rgbRun++;
        pScene++;
    }
}


/**
 * Create IR RGB image
 * @param S Simulink S-Function structure
 * @param pIr IR frame data
 * @param res Frame resolution
 * @return None
 */
static void nidCreateIrRGB(SimStruct *S, const uint16_T* pIr, Res_T res) 
{
    int npixels = res.xRes * res.yRes;
	RGB24Pixel_T* rgbRun = gRGB;
    for (int i = 0; i < npixels; i++)
    {
        uint16_T ir = *(pIr + i);
        rgbRun->nRed = rgbRun->nGreen = rgbRun->nBlue = (uint8_T)(ir >> 8);
        rgbRun++;
    }
}


/**
 * Create Skeleton RGB image
 * @param S Simulink S-Function structure
 * @param pScene Scene (Motion) frame data
 * @param res Frame resolution
 * @param numOfSkeletonTracking number of people in skeleton tracking
 * @param pSkeletonData Pointer to Skeleton data in tracking
 * @return None
 */
static void nidCreateSkeletonRGB(SimStruct *S, const uint16_T* pScene, Res_T res, int32_T numOfSkeletonTracking, const NUI_SKELETON_DATA* pSkeletonData) 
{
	// Draw Motion image
	nidCreateMotionRGB(S, pScene, res); 

  	// Skeleton data
	int npixels = res.xRes * res.yRes;
	for (int skeletonDataIdx = 0; skeletonDataIdx < numOfSkeletonTracking; skeletonDataIdx++)
	{
       	// Overdraw crosses on skeleton joints
		for (int jointIdx = 0; jointIdx < NUM_OF_SKELETON_JOINTS_KINECT_SDK; jointIdx++)
		{
			bool isTracked;
			if (INFER_SKELETON)
			{
				isTracked = (pSkeletonData[skeletonDataIdx].eSkeletonPositionTrackingState[jointIdx] != NUI_SKELETON_POSITION_NOT_TRACKED);
			}
			else
			{
				isTracked = (pSkeletonData[skeletonDataIdx].eSkeletonPositionTrackingState[jointIdx] == NUI_SKELETON_POSITION_TRACKED);
			}

			if (isTracked)
			{
				real32_T normX, normY;
				NuiTransformSkeletonToDepthImage(pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx], &normX, &normY);
   				LONG colorX, colorY;
				NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, NULL, (LONG)normX, (LONG)normY, 
						(uint16_T)pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx].z, &colorX, &colorY);
				int x = (int)(colorX);
				int y = (int)(colorY);

				if (x >= 0 && x < res.xRes && y >= 0 && y < res.yRes)
				{
               		// Draw a cross on each skeleton joint
           			int LINE_LENGTH = 11; // should be odd number
               		int LINE_LENGTH_OFFSET = (LINE_LENGTH-1)/2;
               		int centerOfCross = (int)x + (int)y * res.xRes;
               		int modRow = centerOfCross % res.xRes;
               		int modCol = centerOfCross % res.yRes;
               		for (int length = -LINE_LENGTH_OFFSET; length <= LINE_LENGTH_OFFSET; length++)
               		{
               			// Draw a line horizontally
                   		int hpixel = centerOfCross + length;
                   		if ((hpixel >= 0) && (hpixel < npixels) && (modRow + length >= 0) && (modRow + length < res.xRes))
                   		{
                       		*(gRGB + hpixel) = SKELETON_CROSS_COLOR;
                   		}
               			// Draw a line vertically
                   		int vpixel = centerOfCross + length * res.xRes;
                   		if ((vpixel >= 0) && (vpixel < npixels) && (modCol + length >= 0) && (modCol + length < res.yRes))
                   		{                          
                       		*(gRGB + vpixel) = SKELETON_CROSS_COLOR;
                   		}
               		}
               	}
			}
		}
	}
}


/**
 * Convert NID XYZ coordinates with MATLAB matrix alignment and
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param pDepth Depth frame data
 * @param res Frame resolution
 * @return None
 */
static void nidDepthOutputs(SimStruct *S, const uint16_T* pDepth, Res_T res)
{
    const uint16_T* pDepthRun = pDepth;
    int npixels = res.xRes * res.yRes;
   	uint16_T maxDepth = 0;
        
    if (DEPTH_OUTPUT(S) == DEPTH_SIGNAL)
    {
    	// Convert row based memory alignment to column based
        real_T *out = (real_T*)ssGetOutputPortSignal(S,0);
        for (int x = 0; x < res.xRes; x++)
        {
            for (int y = 0; y < res.yRes; y++)
            {
				uint16_T depth = *(pDepthRun + x + res.xRes * y);
				if (depth > maxDepth)
				{
					maxDepth = depth;
				}
                *out = (real_T)depth * 0.001; // convert mm to m
                out++; 
            }
        }

        if (VIEWER(S) == _ON)
        {
            nidCreateDepthRGB(S, pDepth, res, maxDepth);
            
            uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,1);
            uint8_T* outG = (uint8_T*)(outR + npixels);
            uint8_T* outB = (uint8_T*)(outG + npixels);
            
            CopyRGB24ToSimulink(outR, outG, outB, gRGB, res.xRes, res.yRes);
        }
    }
    else if (DEPTH_OUTPUT(S) == XYZ_SIGNAL || DEPTH_OUTPUT(S) == X_Y_Z_SIGNAL)
    {
        // Convert depth to XYZ coordinates
        for(int y = 0; y < res.yRes; y++)
        {
            for(int x = 0; x < res.xRes; x++)
            {
                XYZPixel_T* buf = (XYZPixel_T*)(gXYZ + x + y * res.xRes);
                buf->nX = (real32_T)x;
                buf->nY = (real32_T)y;
                buf->nZ = (real32_T)*pDepthRun;
				if (buf->nZ > maxDepth)
				{
					maxDepth = (uint16_T)buf->nZ;
				}
                pDepthRun++;
            }
        }
        
        // Convert to MATLAB matrix alignment and inject to Simulink
        real_T *outX, *outY, *outZ;
        if (DEPTH_OUTPUT(S) == XYZ_SIGNAL)
        {
            outX = (real_T*)ssGetOutputPortSignal(S,0);
            outY = (real_T*)(outX + npixels);
            outZ = (real_T*)(outY + npixels);
        }
        else if (DEPTH_OUTPUT(S) == X_Y_Z_SIGNAL)
        {
            outX = (real_T*)ssGetOutputPortSignal(S,0);
            outY = (real_T*)ssGetOutputPortSignal(S,1);
            outZ = (real_T*)ssGetOutputPortSignal(S,2);
        }
        
        if (CONV_XYZ_RW(S) == _ON)
        {
			nidConvertProjectiveToRealWorld(gXYZ_RW, (const XYZPixel_T*)gXYZ, res);
            CopyXYZRWToSimulink(outX, outY, outZ, gXYZ_RW, res.xRes, res.yRes);
        }
        else
        {
            // Default depth XYZ is projective coordinates
            CopyXYZToSimulink(outX, outY, outZ, gXYZ, res.xRes, res.yRes);
        }
        
        if (VIEWER(S) == _ON)
        {
            nidCreateDepthRGB(S, pDepth, res, maxDepth);
             
            uint8_T *outR, *outG, *outB;        
            if (DEPTH_OUTPUT(S) == XYZ_SIGNAL)
            {
                outR = (uint8_T*)ssGetOutputPortSignal(S,1);
                outG = (uint8_T*)(outR + npixels);
                outB = (uint8_T*)(outG + npixels);
            }
            else if (DEPTH_OUTPUT(S) == X_Y_Z_SIGNAL)
            {
                outR = (uint8_T*)ssGetOutputPortSignal(S,3);
                outG = (uint8_T*)(outR + npixels);
                outB = (uint8_T*)(outG + npixels);
            }
             
            CopyRGB24ToSimulink(outR, outG, outB, gRGB, res.xRes, res.yRes);
        }
    }
}


/**
 * Convert motion data with MATLAB matrix alignment and
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param pScene Scene (Motion) frame data
 * @param res Frame resolution
 * @return None
 */
static void nidMotionOutputs(SimStruct *S, const uint16_T* pScene, Res_T res)
{
    const uint16_T* pSceneRun = pScene;
    int npixels = res.xRes * res.yRes;
        
    // Convert row based memory alignment to column based
    uint16_T *out = (uint16_T*)ssGetOutputPortSignal(S,0);
    for (int x = 0; x < res.xRes; x++)
    {
        for (int y = 0; y < res.yRes; y++)
        {
            *out = (uint16_T)*(pSceneRun + x + res.xRes * y);
            out++; 
        }
    }

    if (VIEWER(S) == _ON)
    {
        nidCreateMotionRGB(S, pScene, res);
        
        uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,1);
        uint8_T* outG = (uint8_T*)(outR + npixels);
        uint8_T* outB = (uint8_T*)(outG + npixels);
        
        CopyRGB24ToSimulink(outR, outG, outB, gRGB, res.xRes, res.yRes);
    }
}


/**
 * Convert NID RGB with MATLAB matrix alignment and 
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param pImage Image frame data
 * @param res Frame resolution
 * @return None
 */
static void nidImageOutputs(SimStruct *S, RGB24Pixel_T* pImage, Res_T res)
{
    int npixels = res.xRes * res.yRes;

    // Convert to MATLAB matrix alignment and inject to Simulink
    uint8_T *outR, *outG, *outB;        
    if (IMAGE_OUTPUT(S) == RGB_SIGNAL)
    {
        outR = (uint8_T*)ssGetOutputPortSignal(S,0);
        outG = (uint8_T*)(outR + npixels);
        outB = (uint8_T*)(outG + npixels);
    }
    else if (IMAGE_OUTPUT(S) == R_G_B_SIGNAL)
    {
        outR = (uint8_T*)ssGetOutputPortSignal(S,0);
        outG = (uint8_T*)ssGetOutputPortSignal(S,1);
        outB = (uint8_T*)ssGetOutputPortSignal(S,2);
    }
    
    CopyRGB24ToSimulink(outR, outG, outB, pImage, res.xRes, res.yRes);
}


/**
 * Convert NID IR with MATLAB matrix alignment and 
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param pIr Ir frame data
 * @param res Frame resolution
 * @return None
 */
static void nidIrOutputs(SimStruct *S, const uint16_T* pIr, Res_T res)
{
    const uint16_T* pIrRun = pIr;
    int npixels = res.xRes * res.yRes;
        
    // Convert row based memory alignment to column based
    uint16_T *out = (uint16_T*)ssGetOutputPortSignal(S,0);
    for (int x = 0; x < res.xRes; x++)
    {
        for (int y = 0; y < res.yRes; y++)
        {
            *out = (uint16_T)*(pIrRun + x + res.xRes * y);
            out++; 
        }
    }

    if (VIEWER(S) == _ON)
    {
        nidCreateIrRGB(S, pIr, res);

        uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,1);
        uint8_T* outG = (uint8_T*)(outR + npixels);
        uint8_T* outB = (uint8_T*)(outG + npixels);

        CopyRGB24ToSimulink(outR, outG, outB, gRGB, res.xRes, res.yRes);
    }
}


/**
 * Inject Skeleton data to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param pScene Scene (Motion) frame data
 * @param res Frame resolution
 * @param numOfSkeletonTracking number of people in skeleton tracking
 * @param pSkeletonData Pointer to Skeleton data in tracking
 * @return None
 */
static void nidSkeletonOutputs(SimStruct *S, const uint16_T* pScene, Res_T res, int32_T numOfSkeletonTracking, const NUI_SKELETON_DATA* pSkeletonData)
{
	int port_idx = 0;
	
	// Number of skeleton tracking
	int32_T* out = (int32_T*)ssGetOutputPortSignal(S,port_idx++);
    *out = numOfSkeletonTracking;
  
  	// Skeleton data
	for (int skeletonDataIdx = 0; skeletonDataIdx < NUM_OF_SKELETON_TRACKING(S); skeletonDataIdx++)
	{
    	real_T *outX, *outY, *outZ;
    	outX = (real_T*)ssGetOutputPortSignal(S,port_idx++);
    	outY = (real_T*)(outX + NUM_OF_SKELETON_JOINTS_KINECT_SDK);
    	outZ = (real_T*)(outY + NUM_OF_SKELETON_JOINTS_KINECT_SDK);
		// Flush output ports
   		memset((char*)outX, 0, sizeof(real_T) * NUM_OF_SKELETON_JOINTS_KINECT_SDK * 3);

    	if (skeletonDataIdx < numOfSkeletonTracking)
    	{
			for (int jointIdx = 0; jointIdx < NUM_OF_SKELETON_JOINTS_KINECT_SDK; jointIdx++)
			{
				bool isTracked;
				if (INFER_SKELETON)
				{
					isTracked = (pSkeletonData[skeletonDataIdx].eSkeletonPositionTrackingState[jointIdx] != NUI_SKELETON_POSITION_NOT_TRACKED);
				}
				else
				{
					isTracked = (pSkeletonData[skeletonDataIdx].eSkeletonPositionTrackingState[jointIdx] == NUI_SKELETON_POSITION_TRACKED);
				}

				if (isTracked)
				{	
					if (CONV_XYZ_RW(S) == _ON)
					{
						*outX = (real_T)pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx].x;
						*outY = (real_T)pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx].y;
						*outZ = (real_T)pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx].z;
					}
					else
					{
						real32_T normX, normY;
						NuiTransformSkeletonToDepthImage(pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx], &normX, &normY);
         				LONG colorX, colorY;
						NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, NULL, (LONG)normX, (LONG)normY, 
							(uint16_T)pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx].z, &colorX, &colorY);
						*outX = (real_T)colorX;
						*outY = (real_T)colorY;
						*outZ = (real_T)pSkeletonData[skeletonDataIdx].SkeletonPositions[jointIdx].z;
					}
				}
				outX++;
				outY++;
				outZ++;
			}
		}
	}

	// Image output
    if (VIEWER(S) == _ON)
    {
        nidCreateSkeletonRGB(S, pScene, res, numOfSkeletonTracking, pSkeletonData);
        
    	int npixels = res.xRes * res.yRes;
        uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,port_idx);
        uint8_T* outG = (uint8_T*)(outR + npixels);
        uint8_T* outB = (uint8_T*)(outG + npixels);
         
        CopyRGB24ToSimulink(outR, outG, outB, gRGB, res.xRes, res.yRes);
    }
}


void CALLBACK kinectSdkStatusCallback(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName)
{      
	printf("Kinect SDK status was changed.\n");
}


/**
 * Inject NID outputs. This method should be called in mdlOutputs.
 * @param S Simulink S-Function structure
 * @return None
 */
void nidDeviceOutputs(SimStruct *S)
{
	// Static variables to be used during simulation
    static SpecNID_T spec;
	static bool MIRROR_ON;
	static real_T SKELETON_SMOOTH[5];
	
	HRESULT hr;
    if (BLOCK_TYPE(S) == IMAQ_BLOCK)
    {
		if (gNuiSensor == NULL)
		{
           	// Reach here only once through a MATLAB session.
            
            // Search for Kinect 	
			int numOfNui;
			hr = NuiGetSensorCount(&numOfNui);
			if (FAILED(hr) || numOfNui == 0)
			{
           		ssSetErrorStatus(S, ERROR_INIT_NID);
           		return;
			}
			
           	// NuiSetDeviceStatusCallback has to be invoked once before initializing Kinect
           	// through the application's lifetime (in case of Simulink, the lifetime means one MATLAB session).
           	// See: http://social.msdn.microsoft.com/Forums/en-US/kinectsdknuiapi/thread/0c62b444-bf05-4700-a1e7-a9b3a1a2dcec
       		NuiSetDeviceStatusCallback(reinterpret_cast<NuiStatusProc>(&kinectSdkStatusCallback), NULL);

			hr = NuiCreateSensorByIndex(0, &gNuiSensor); // Establish the connection with the first one.
			if (FAILED(hr) || gNuiSensor == NULL)
			{
           		ssSetErrorStatus(S, ERROR_INIT_NID);
           		return;
			}
		}
			
		if (gIsFirstTimePerSim == TRUE && gNuiSensor != NULL)
		{
			// Reach here only once through a simulation of a model 
			DWORD nuiFlags = 0;
			if (IMAQ_DEPTH(S) == _ON || IMAQ_MOTION(S) == _ON)
			{
				nuiFlags |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
			}

			if (IMAQ_IMAGE(S) == _ON || IMAQ_IR(S) == _ON)
			{
				nuiFlags |= NUI_INITIALIZE_FLAG_USES_COLOR;
			}
			
			if (IMAQ_SKELETON(S) == _ON)
			{
				// Depth and Motion are needed for Skeleton tracking
				nuiFlags |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
				nuiFlags |= NUI_INITIALIZE_FLAG_USES_SKELETON;
			}
			
			// Initialize Kinect
			hr = gNuiSensor->NuiInitialize(nuiFlags);
			if (FAILED(hr))
			{
               	ssSetErrorStatus(S, ERROR_INIT_NID);
               	return;
			}

   			GetSpecNID(&spec); // Set Resolution and FPS (In case of Kinect SDK, it is fixed)

			// Kinect SDK IMAQ data is mirrored by default.
       		// So, if IMAQ Mirror image block parameter is off, mirror them.
       		MIRROR_ON = TRUE; // Default
       		if (MIRROR_IMAGE(S) == _ON)
       		{
            	MIRROR_ON = FALSE;
			}

			// Save Kinect Skeleton smoothing parameters on IMAQ block
			real_T* param = KINECT_SKELETON_SMOOTH(S);
			SKELETON_SMOOTH[0] = param[0];
			SKELETON_SMOOTH[1] = param[1];
			SKELETON_SMOOTH[2] = param[2];
			SKELETON_SMOOTH[3] = param[3];
			SKELETON_SMOOTH[4] = param[4];

			INFER_SKELETON = TRUE; // Default
			if (KINECT_INFER_SKELETON(S) == _OFF)
			{
				INFER_SKELETON = FALSE;
			}

			if (KINECT_ANGLE_INPUT(S) == _OFF)
			{
				// Get the Kinect sensor angle parameter and check the value
				// Limit check is already done by block parameter callback. But, check it again. 
				LONG targetSensorAngle = KINECT_ANGLE(S);
				if (targetSensorAngle > NUI_CAMERA_ELEVATION_MAXIMUM)
				{
					targetSensorAngle =  NUI_CAMERA_ELEVATION_MAXIMUM;
				}
				else if (targetSensorAngle < NUI_CAMERA_ELEVATION_MINIMUM)
				{
					targetSensorAngle = NUI_CAMERA_ELEVATION_MINIMUM;
				}

				// Set the sensor angle
				hr = gNuiSensor->NuiCameraElevationSetAngle(targetSensorAngle);
				if (FAILED(hr))
				{
           			ssSetErrorStatus(S, "NID Error: Failed to set the Kinect sensor angle.");
               		return;
				}

				// Wait until the sensor points the specified angle
				LONG actualSensorAngle;
				int32_T elevationStart = timeGetTime();
				while(1)
				{
					hr = gNuiSensor->NuiCameraElevationGetAngle(&actualSensorAngle);
					if (FAILED(hr))
					{
               			ssSetErrorStatus(S, "NID Error: Failed to set the Kinect sensor angle.");
               			return;
					}

					if (actualSensorAngle == targetSensorAngle)
					{
						// The Kinect sensor could be elevated successfully.
						break;
					}

					if (timeGetTime() - elevationStart > KINECT_SENSOR_ANGLE_TIME_OUT_MS)
					{
						// Time out due to some reason. 
						// It might be caused by inproper location of Kinect sensor such as put on steep slope, so inform the result as warning. 
						printf("NID Warning: Failed to elevate the Kinect sensor angle to %d [deg].\n", targetSensorAngle); 
						return;
					}
				}
			}
			else
			{
				lastSetSensorAngleTime = 0;
			}

			// Setup NUI device for Depth, Motion and Skeleton
			if (IMAQ_DEPTH(S) == _ON || IMAQ_MOTION(S) == _ON || IMAQ_SKELETON(S) == _ON)
			{
				if (gDepthEvent == NULL && gDepthFrameHandle == NULL)
				{
					gDepthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
					hr = gNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, 
	       		    	        NUI_IMAGE_RESOLUTION_320x240, 
								NULL, 2, gDepthEvent, &gDepthFrameHandle);
					if (FAILED(hr))
					{
      					ssSetErrorStatus(S, ERROR_INIT_NID);
       					return;
					}
					
					// Set Kinect Near mode
					DWORD imageFlags;
					hr = gNuiSensor->NuiImageStreamGetImageFrameFlags(gDepthFrameHandle, &imageFlags);
					if (FAILED(hr))
					{
   						ssSetErrorStatus(S, ERROR_INIT_NID);
   						return;
					}

					if (KINECT_NEAR_MODE(S) == _ON)
					{
						hr = gNuiSensor->NuiImageStreamSetImageFrameFlags(gDepthFrameHandle, (imageFlags | NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE));
						if (FAILED(hr))
						{
      						printf("NID Warning: Failed to set Near mode. The Near mode is valid for only Kinect for Windows hardware.\n");
						}
					}
					else
					{
						hr = gNuiSensor->NuiImageStreamSetImageFrameFlags(gDepthFrameHandle, (imageFlags & ~NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE));
						if (FAILED(hr))
						{
      						ssSetErrorStatus(S, ERROR_INIT_NID);
       						return;
						}
					}

					// Create global coordinate mapper object for adjusting view point between depth and image to speed up
					if ((IMAQ_DEPTH(S) == _ON || IMAQ_IMAGE(S) == _ON) && gCoordinateMapper == NULL)
					{
						gNuiSensor->NuiGetCoordinateMapper(&gCoordinateMapper);
					}
				}

				if (IMAQ_SKELETON(S) == _ON && gSkeletonEvent == NULL)
				{
					gSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

					// Set skeleton tracking modes
					DWORD skeletonTrackingFrag = 0; // Default
					// Support v1.5 new feature "Skeleton Tracking in Near mode"
					if (KINECT_NEAR_MODE(S) == _ON)
					{
						skeletonTrackingFrag |= NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE;
					}

					// Support v1.5 new feature "Seated mode for Skeleton Tracking"
					if (KINECT_SEATED_MODE(S) == _ON)
					{
						skeletonTrackingFrag |= NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT;
					}

					hr = gNuiSensor->NuiSkeletonTrackingEnable(gSkeletonEvent, skeletonTrackingFrag);
					if (FAILED(hr))
					{
       					ssSetErrorStatus(S, ERROR_INIT_NID);
       					return;
					}
				}
			}

			// Set up NUI device for Image
			if (IMAQ_IMAGE(S) == _ON && gImageEvent == NULL && gImageFrameHandle == NULL)
			{
				gImageEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				hr = gNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, 
		                    NUI_IMAGE_RESOLUTION_640x480, 
							NULL, 2, gImageEvent, &gImageFrameHandle);
				if (FAILED(hr))
				{
               		ssSetErrorStatus(S, ERROR_INIT_NID);
               		return;
				}
			}

			// Set up NUI device for IR
			if (IMAQ_IR(S) == _ON && gIrEvent == NULL && gIrFrameHandle == NULL)
			{
				gIrEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				hr = gNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR_INFRARED, 
		                    NUI_IMAGE_RESOLUTION_640x480, 
							NULL, 2, gIrEvent, &gIrFrameHandle);
				if (FAILED(hr))
				{
               		ssSetErrorStatus(S, ERROR_INIT_NID);
               		return;
				}
			}

			// Heap buffer for Depth and Motion outputs
			if (IMAQ_DEPTH(S) == _ON || IMAQ_MOTION(S) == _ON || IMAQ_SKELETON(S) == _ON)
			{
				if (gDepthBuffer == NULL)
				{
					gDepthBuffer = new uint16_T[spec.depthRes.xRes*spec.depthRes.yRes];
					if (gDepthBuffer == NULL)
					{
           				ssSetErrorStatus(S, "NID Error: Failed to heap Depth buffer.");
           				return;
					}
				}

				if (gMotionBuffer == NULL)
				{
					gMotionBuffer = new uint16_T[spec.depthRes.xRes*spec.depthRes.yRes];
					if (gMotionBuffer == NULL)
					{
               			ssSetErrorStatus(S, "NID Error: Failed to heap Motion buffer.");
               			return;
					}
				}
			}
			
			// Heap buffer for Skeleton output
			if (IMAQ_SKELETON(S) == _ON && gSkeletonBuffer == NULL)
			{
				gSkeletonBuffer = new XYZPixel_T[NUM_OF_SKELETON_JOINTS_KINECT_SDK];
				if (gSkeletonBuffer == NULL)
				{
               		ssSetErrorStatus(S, "NID Error: Failed to heap Skeleton buffer.");
               		return;
				}
			}
			
			// Heap buffer for Image output
			if (IMAQ_IMAGE(S) == _ON && gImageBuffer == NULL)
			{
				gImageBuffer = new RGB24Pixel_T[spec.imageRes.xRes*spec.imageRes.yRes];
            	if (gImageBuffer == NULL)
            	{
                	ssSetErrorStatus(S, "NID Error: Failed to heap Image buffer.");
                	return;
            	}
			}
			
			// Heap buffer for IR output
			if (IMAQ_IR(S) == _ON && gIrBuffer == NULL)
			{
				gIrBuffer = new uint16_T[spec.depthRes.xRes*spec.depthRes.yRes];
            	if (gIrBuffer == NULL)
            	{
                	ssSetErrorStatus(S, "NID Error: Failed to heap IR buffer.");
                	return;
            	}
			}
			
			// Heap buffer for Depth XYZ output
           if (IMAQ_DEPTH(S) == _ON && gXYZ == NULL && gXYZ_RW == NULL)
         	{
            	gXYZ = new XYZPixel_T[spec.depthRes.xRes*spec.depthRes.yRes];
            	gXYZ_RW = new XYZPixel_T[spec.depthRes.xRes*spec.depthRes.yRes];
            	if (gXYZ == NULL || gXYZ_RW == NULL)
            	{
                	ssSetErrorStatus(S, "NID Error: Failed to heap XYZ buffer.");
                	return;
            	}
        	}

			// Heap buffer for Depth/Motion/Skeleton/IR image output
            if ((IMAQ_DEPTH(S) == _ON || IMAQ_MOTION(S) == _ON || IMAQ_SKELETON(S) == _ON || IMAQ_IR(S) == _ON) && gRGB == NULL)
        	{
            	gRGB = new RGB24Pixel_T[spec.depthRes.xRes*spec.depthRes.yRes];
            	if (gRGB == NULL)
            	{
                	ssSetErrorStatus(S, "NID Error: Failed to heap RGB buffer.");
                	return;
            	}
        	}

            // timeGetTime is the time since Windows started, so no possibility to be less than IMAQ_DT.
            // The first IMAQ block output does not have any wait. 
            lastIMAQTime = timeGetTime() - (int32_T)(IMAQ_SAMPLE_TIME(S) * 1000);

			gIsFirstTimePerSim = FALSE;
		}


        // It seems to be more consistent wait time than Windows Sleep API
        while(timeGetTime() < (lastIMAQTime + (int32_T)(IMAQ_SAMPLE_TIME(S) * 1000))); // software loop wait
        lastIMAQTime = timeGetTime();  // Save the last update time for the next update


		// Acquire Depth and Moiton frame
        if (gDepthEvent != NULL)
        {
			// Flush buffers
			memset(gDepthBuffer, 0, spec.depthRes.xRes*spec.depthRes.yRes*sizeof(uint16_T));
			memset(gMotionBuffer, 0, spec.depthRes.xRes*spec.depthRes.yRes*sizeof(uint16_T));

			// Wait for next Depth and Motion frame 
			WaitForSingleObject(gDepthEvent, INFINITE);
			hr = gNuiSensor->NuiImageStreamGetNextFrame(gDepthFrameHandle, 0, &gDepthFrame);
			if (FAILED(hr))
			{
               	ssSetErrorStatus(S, "NID Error: Failed to acquire Depth frame.");
               	return;
			}

			//
			// Copy frame data to buffer
			//
			BOOL nearMode;
			INuiFrameTexture* pTexture;
			// Get the depth image pixel texture
    		hr = gNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(gDepthFrameHandle, &gDepthFrame, &nearMode, &pTexture);
    		if (FAILED(hr))
    		{
        		gNuiSensor->NuiImageStreamReleaseFrame(gDepthFrameHandle, &gDepthFrame);
               	ssSetErrorStatus(S, "NID Error: Failed to acquire Depth frame texture.");
               	return;
    		}

			NUI_LOCKED_RECT LockedRect;
			pTexture->LockRect(0, &LockedRect, NULL, 0);
			if (LockedRect.Pitch != 0)
			{
				const NUI_DEPTH_IMAGE_PIXEL* pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);
				uint16_T* dpDepth;
				uint16_T* dpMotion;

				if (ADJUST_VIEW_POINT(S) == _ON && gImageEvent != NULL)
				{
					NUI_DEPTH_IMAGE_POINT pDepthPoint;
					NUI_COLOR_IMAGE_POINT pColorPoint;
					for (int y = 0; y < 240; y++)
					{
						for (int x = 0; x < 320; x++)
						{
							pDepthPoint.x = x;
							pDepthPoint.y = y;
							pDepthPoint.depth = pBufferRun->depth;
							// MapDepthPointToColorPoint is a new method in the version 1.6 SDK which returns more accurate color points of depth data
							gCoordinateMapper->MapDepthPointToColorPoint(NUI_IMAGE_RESOLUTION_320x240, &pDepthPoint,
									NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, &pColorPoint);
							if (pColorPoint.x >= 0 && pColorPoint.x < 639 && pColorPoint.y >= 0 && pColorPoint.y < 479)
							{
								dpDepth = gDepthBuffer + pColorPoint.x + pColorPoint.y * 640; 
								dpMotion = gMotionBuffer + pColorPoint.x + pColorPoint.y * 640;
								*dpDepth = *(dpDepth+1) = *(dpDepth+640) = *(dpDepth+641) = pBufferRun->depth;
								*dpMotion = *(dpMotion+1) = *(dpMotion+640) = *(dpMotion+641) = pBufferRun->playerIndex;
							}
							++pBufferRun;
						}
					}
				}
				else
				{
					dpDepth = gDepthBuffer;
					dpMotion = gMotionBuffer;
					for (int y = 0; y < 240; y++)
					{
						for (int x = 0; x < 320; x++)
						{
							*dpDepth = *(dpDepth+1) = *(dpDepth+640) = *(dpDepth+641) = pBufferRun->depth;
							*dpMotion = *(dpMotion+1) = *(dpMotion+640) = *(dpMotion+641) = pBufferRun->playerIndex;
							++pBufferRun;
							dpDepth += 2;
							dpMotion += 2;
						}
						dpDepth += 640;  // 320x2
						dpMotion += 640; // 320x2
					}
				}

            	if (MIRROR_ON == TRUE)
            	{
					nidMirrorDepthMotion(gDepthBuffer, spec.depthRes);
					nidMirrorDepthMotion(gMotionBuffer, spec.depthRes);
				}
			}

			// Release texture
    		pTexture->UnlockRect(0);
    		pTexture->Release();

            // Release Depth stream frame
            gNuiSensor->NuiImageStreamReleaseFrame(gDepthFrameHandle, &gDepthFrame);
		}

		// Acquire Image frame
		if (gImageEvent != NULL)
        {
			// Flush buffer
			memset(gImageBuffer, 0, spec.imageRes.xRes*spec.imageRes.yRes*sizeof(RGB24Pixel_T));

			// Wait for next Image frame
			WaitForSingleObject(gImageEvent, INFINITE);
			hr = gNuiSensor->NuiImageStreamGetNextFrame(gImageFrameHandle, 0, &gImageFrame);
			if (FAILED(hr))
			{
               	ssSetErrorStatus(S, "NID Error: Failed to acquire Image frame.");
               	return;
			}

			// Copy frame data to buffer
			INuiFrameTexture* pTexture = gImageFrame.pFrameTexture;
			NUI_LOCKED_RECT LockedRect;
			pTexture->LockRect(0, &LockedRect, NULL, 0);
			if (LockedRect.Pitch != 0)
			{
				BYTE* pBuffer = (BYTE*)LockedRect.pBits;
                // Kinect SDK uses RGB32 format (B/G/R/A)
                // Simulink for NID uses RGB24 format (R/G/B)
                RGB32Pixel_T* pRGB32 = (RGB32Pixel_T*)pBuffer; 
                RGB24Pixel_T* pRGB24 = (RGB24Pixel_T*)gImageBuffer; 
				int npixels = spec.imageRes.xRes*spec.imageRes.yRes;
				for (int pixel = 0; pixel < npixels; pixel++)
				{
                    pRGB24[pixel].nRed = pRGB32[pixel].nRed;
                    pRGB24[pixel].nGreen = pRGB32[pixel].nGreen;
                    pRGB24[pixel].nBlue = pRGB32[pixel].nBlue;
                }

            	if (MIRROR_ON == TRUE)
            	{
					nidMirrorImage(gImageBuffer, spec.imageRes);
				}
            }

            // Release image stream frame
            gNuiSensor->NuiImageStreamReleaseFrame(gImageFrameHandle, &gImageFrame);
		}

		// Acquire IR frame
        if (gIrEvent != NULL)
        {
			// Flush buffers
			memset(gIrBuffer, 0, spec.depthRes.xRes*spec.depthRes.yRes*sizeof(uint16_T));

			// Wait for next IR frame 
			WaitForSingleObject(gIrEvent, INFINITE);
			hr = gNuiSensor->NuiImageStreamGetNextFrame(gIrFrameHandle, 0, &gIrFrame);
			if (FAILED(hr))
			{
               	ssSetErrorStatus(S, "NID Error: Failed to acquire IR frame.");
               	return;
			}

			// Copy frame data to buffer
			INuiFrameTexture* pTexture = gIrFrame.pFrameTexture;
			NUI_LOCKED_RECT LockedRect;
			pTexture->LockRect(0, &LockedRect, NULL, 0);
			if (LockedRect.Pitch != 0)
			{
				BYTE* pBuffer = (BYTE*)LockedRect.pBits;
				uint16_T* pBufferRun = (uint16_T*)pBuffer;
				uint16_T* dpIr = gIrBuffer;
				int npixels = spec.depthRes.xRes*spec.depthRes.yRes;

				for (int pixel = 0; pixel < npixels; pixel++)
				{
					*(dpIr++) = *(pBufferRun++);
				}
			}

           	if (MIRROR_ON == TRUE)
           	{
				nidMirrorDepthMotion(gIrBuffer, spec.depthRes);
			}

            // Release IR stream frame
            gNuiSensor->NuiImageStreamReleaseFrame(gIrFrameHandle, &gIrFrame);
		}

		// Acquire Skeleton frame
		if (gSkeletonEvent != NULL)
		{
			// Wait for next Skeleton frame
			WaitForSingleObject(gSkeletonEvent, INFINITE);
			hr = gNuiSensor->NuiSkeletonGetNextFrame(0, &gSkeletonFrame);
			if (FAILED(hr))
			{
               	ssSetErrorStatus(S, "NID Error: Failed to acquire Skeleton frame.");
               	return;
			}
			
			// No (image) stream was open for Skeleton, so, no stream release is needed.
		}

        // Inject last FPS time to handshake with other device blocks
        uint32_T* out = (uint32_T*)ssGetOutputPortSignal(S,0);
        *out = lastIMAQTime;

		// Set Kinect sensor angle when was configured
		if (KINECT_ANGLE_INPUT(S) == _ON)
		{
			uint32_T interval = timeGetTime() - lastSetSensorAngleTime;
			if (interval > KINECT_SET_SENSOR_ANGLE_INTERVAL)
			{
				int32_T* targetSensorAngle = (int32_T*)ssGetInputPortSignal(S,0);
				if ((*targetSensorAngle <= (int32_T)NUI_CAMERA_ELEVATION_MAXIMUM) &&
					(*targetSensorAngle >= (int32_T)NUI_CAMERA_ELEVATION_MINIMUM))
				{
					// Set the sensor angle only when the target angle was within the angle range.
					hr = gNuiSensor->NuiCameraElevationSetAngle((LONG)(*targetSensorAngle));
					if (FAILED(hr))
					{
       					ssSetErrorStatus(S, "NID Error: Failed to set the Kinect sensor angle.");
           				return;
					}
				}

				lastSetSensorAngleTime = timeGetTime();
			}
		}

        // Inject Kinect sensor angle if it was configured.
        if (KINECT_ANGLE_READ(S) == _ON)
        {
        	int32_T* angle = (int32_T*)ssGetOutputPortSignal(S,1);

			hr = gNuiSensor->NuiCameraElevationGetAngle((LONG*)angle);
			if (FAILED(hr))
			{
           		ssSetErrorStatus(S, "NID Error: Failed to read the Kinect sensor angle.");
           		return;
			}
        }
    }
    else
    {
        // Check the input is equal to the last FPS time acquired by IMAQ block
        uint32_T *in = (uint32_T*)ssGetInputPortSignal(S, 0);
        if (*in != lastIMAQTime)
        {
           ssSetErrorStatus(S, "NID Error: IMAQ block and other NID blocks are not synchronized.");
           return;
        }

        // Inject output depending on the device blocks
        if (BLOCK_TYPE(S) == DEPTH_BLOCK)
        {
       		nidDepthOutputs(S, gDepthBuffer, spec.depthRes);
        }
        else if (BLOCK_TYPE(S) == MOTION_BLOCK)
        {
       		nidMotionOutputs(S, gMotionBuffer, spec.depthRes);
		}
        else if (BLOCK_TYPE(S) == IMAGE_BLOCK)
        {
           	nidImageOutputs(S, gImageBuffer, spec.imageRes);
        }
        else if (BLOCK_TYPE(S) == IR_BLOCK)
        {
           	nidIrOutputs(S, gIrBuffer, spec.depthRes);
        }
        else if (BLOCK_TYPE(S) == SKELETON_BLOCK)
        {
			//
			// Since ver.0.4.2, multiple skeleton tracking has been supported.
			//
            NUI_SKELETON_DATA skeletonData[MAX_NUM_OF_SKELETON_TRACKING];
            int numOfSkeletonTracking = 0;
            for (int idx = 0; idx < NUI_SKELETON_COUNT; idx++)
            {
				if(gSkeletonFrame.SkeletonData[idx].eTrackingState == NUI_SKELETON_TRACKED)
            	{
                	// Smooth out the skeleton data
                	NUI_TRANSFORM_SMOOTH_PARAMETERS param;
                	param.fSmoothing          = (FLOAT)SKELETON_SMOOTH[0];
                	param.fCorrection         = (FLOAT)SKELETON_SMOOTH[1];
                	param.fPrediction         = (FLOAT)SKELETON_SMOOTH[2];
                	param.fJitterRadius       = (FLOAT)SKELETON_SMOOTH[3];
                	param.fMaxDeviationRadius = (FLOAT)SKELETON_SMOOTH[4];
                	gNuiSensor->NuiTransformSmooth(&gSkeletonFrame, &param);
            		
            		if (MIRROR_ON == TRUE)
       				{
                        // Mirror (flip) skeleton joints data horizontally (in X axis).
						for (int i = 0; i < NUM_OF_SKELETON_JOINTS_KINECT_SDK; i++)
						{
							gSkeletonFrame.SkeletonData[idx].SkeletonPositions[i].x *= -1.0f;
						}
					}
					
					skeletonData[numOfSkeletonTracking++] = gSkeletonFrame.SkeletonData[idx]; 
            	    if (numOfSkeletonTracking >= NUM_OF_SKELETON_TRACKING(S))
            	    {
               			break;
					}
            	}
        	}
			
        	nidSkeletonOutputs(S, gMotionBuffer, spec.depthRes, numOfSkeletonTracking, skeletonData); 
		}
        else
        {   
            ssSetErrorStatus(S, "S-Function Parameter Error: Block Type");
        }
    }
}


/**
 * Terminate NID device
 * @param S Simulink S-Function structure
 * @return None
 */
void nidDeviceTerminate(SimStruct *S)
{
	if (gNuiSensor != NULL)
	{
		gNuiSensor->NuiShutdown();
	}
	
	// Close handles
   	if(gDepthEvent && (gDepthEvent != INVALID_HANDLE_VALUE ) )
   	{
       	CloseHandle(gDepthEvent);
   	}

   	if(gImageEvent && (gImageEvent != INVALID_HANDLE_VALUE ) )
   	{
       	CloseHandle(gImageEvent);
   	}

   	if(gIrEvent && (gIrEvent != INVALID_HANDLE_VALUE ) )
   	{
       	CloseHandle(gIrEvent);
   	}

   	if(gSkeletonEvent && (gSkeletonEvent != INVALID_HANDLE_VALUE ) )
   	{
       	CloseHandle(gSkeletonEvent);
   	}

   	if(gDepthFrameHandle && (gDepthFrameHandle != INVALID_HANDLE_VALUE ) )
   	{
       	CloseHandle(gDepthFrameHandle);
   	}

   	if(gImageFrameHandle && (gImageFrameHandle != INVALID_HANDLE_VALUE ) )
   	{
       	CloseHandle(gImageFrameHandle);
   	}

   	if(gIrFrameHandle && (gIrFrameHandle != INVALID_HANDLE_VALUE ) )
   	{
       	CloseHandle(gIrFrameHandle);
   	}

   	// Release memory
   	delete [] gDepthBuffer;
   	delete [] gMotionBuffer;
   	delete [] gImageBuffer;
   	delete [] gIrBuffer;
   	delete [] gSkeletonBuffer;
   	delete [] gRGB;
   	delete [] gXYZ;
   	delete [] gXYZ_RW;

   	// Reset for next simulation
	gDepthEvent = NULL;
	gImageEvent = NULL;
	gIrEvent = NULL;
	gSkeletonEvent = NULL;
	gDepthFrameHandle = NULL;
	gImageFrameHandle = NULL;
	gIrFrameHandle = NULL;
	gCoordinateMapper = NULL;

	gDepthBuffer = NULL;
	gMotionBuffer = NULL;
	gImageBuffer = NULL;
	gIrBuffer = NULL;
	gSkeletonBuffer = NULL;

   	gRGB = NULL;
   	gXYZ = NULL;
   	gXYZ_RW = NULL;

	gIsFirstTimePerSim = TRUE;
}


/**
 * Get NID SDK
 * @return NID SDK enum
 */
eNID_SDK nidGetSDK(void)
{
    return KINECT_SDK;
}


/* [EOF] */