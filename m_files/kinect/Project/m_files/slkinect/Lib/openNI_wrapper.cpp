/**
 * openNI_wrapper.cpp
 *
 * This is OpenNI wrapper for Simulink NID (Natural Interaction Device).
 * It provides two main functions need to be called in CMEX S-Function methods
 *
 * void nidDeviceOutputs(SimStruct *S): needs to be called in mdlOutputs
 * void nidDeviceTerminate(SimStruct *S): needs to be called in mdlTerminates
 *
 * Copyright 2011 The MathWorks, Inc.
 */

#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    //
    // Windows specific part is located in _WIN32 or _WIN64 compile switch.  
    // timeGetTime API is used to get high resolution system time [msec] to 
    // wait for the completion of NID data transaction in stable way according to test results.
    //
    #include <Windows.h>
    #include <mmsystem.h>
    #pragma comment(lib,"winmm.lib") // for timeGetTime
#else
	//
	// Unix based OS (Linux and MacOSX) are assumed here
	// gettimeofday API is used to get high resolution system time [msec] to 
    // wait for the completion of NID data transaction in stable way according to test results.
	#include <sys/time.h>
#endif

// OpenNI
#include <XnOpenNI.h>
#include <XnCppWrapper.h>

// S-Function
#include "sfun_nid.h"
#include "wrapper_common.h"


//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

#define ERROR_INIT_NID "NID Error: Failed to initialize NID. It might be caused by\n- NID is not connected.\n- NID unsupported block is used in the model\n- NID unsupported IMAQ Resolution and FPS is configured."

#define CAL_POSE        "Psi" // Calibration pose for skeleton tracking


//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------

xn::Context* gContext = NULL;
xn::DepthGenerator* gDepth = NULL;
xn::SceneAnalyzer*  gScene = NULL;
xn::ImageGenerator* gImage = NULL;
xn::UserGenerator* gUser = NULL;
xn::IRGenerator* gIr = NULL;

RGB24Pixel_T* gRGB = NULL;
XYZPixel_T* gXYZ = NULL;
XYZPixel_T* gXYZ_RW = NULL;

xn::DepthMetaData gDepthMD;
xn::SceneMetaData gSceneMD;
xn::ImageMetaData gImageMD;
xn::IRMetaData gIrMD;

XnCallbackHandle hUserCallbacks;
XnCallbackHandle hCalibrationStart;
XnCallbackHandle hCalibrationEnd;
XnCallbackHandle hPoseDetected;
XnCallbackHandle hPoseLost;

volatile uint32_T lastIMAQTime;
volatile boolean_T gNeedPose;
volatile boolean_T gIsCalibrating;
XnUserID gUserIdInCalibration;

static char msg[1024];


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

/**
 * OpenNI callback for Skeleton (user detection)
 * @param generator User generator
 * @param nId       User ID
 * @param pCookie   User data
 * @return None
 */
void XN_CALLBACK_TYPE UserDetected(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	if (gNeedPose)
	{
   		printf("NID Skeleton: Detected user ID%d.\n", nId);  
    	printf("NID Skeleton: Look for \"%s\" pose by user ID%d...\n", CAL_POSE, nId);
    	generator.GetPoseDetectionCap().StartPoseDetection(CAL_POSE, nId);
    }
    else
    {
		generator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
}


/**
 * OpenNI callback for Skeleton (user lost)
 * @param generator User generator
 * @param nId       User ID
 * @param pCookie   User data
 * @return None
 */
void XN_CALLBACK_TYPE UserLost(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	if (gNeedPose)
	{
    	printf("NID Skeleton: Lost user ID%d.\n", nId);
    }
}


/**
 * OpenNI callback for Skelteton (pose detection)
 * @param capability Skeleton pose detection capability
 * @param strPose    String to describe the type of pose to be detected
 * @param nId        User ID
 * @param pCookie    User data
 * @return None
 */
void XN_CALLBACK_TYPE PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	if (gNeedPose)
	{
    	printf("NID Skeleton: Detected \"%s\" pose by user ID%d.\n", CAL_POSE, nId);
    }
    gUser->GetSkeletonCap().RequestCalibration(nId, TRUE);
    gUser->GetPoseDetectionCap().StopPoseDetection(nId);
}


/**
 * OpenNI callback for Skeleton (pose lost)
 * @param capability Skeleton pose detection capability
 * @param strPose    String to describe the type of pose to be detected
 * @param nId        User ID
 * @param pCookie    User data
 * @return None
 */
void XN_CALLBACK_TYPE PoseLost(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	if (gNeedPose)
	{
    	printf("NID Skeleton: Lost %s pose lost by user ID%d.\n", CAL_POSE, nId);
    	printf("NID Skeleton: Move out from the NID view until NID loses the user or re-start the simulation.\n");
    }
}


/**
 * OpenNI callback for Skeleton (calibration started)
 * @param capability Skeleton pose detection capability
 * @param nId        User ID
 * @param pCookie    User data
 * @return None
 */
void XN_CALLBACK_TYPE CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
    gIsCalibrating = TRUE;
    gUserIdInCalibration = nId;
	if (gNeedPose)
	{
    	printf("NID Skeleton: Calibration started for user ID%d, keep \"%s\" pose until calibration is completed...\n", nId, CAL_POSE);
    }
}


/**
 * OpenNI callback for Skeleton (calibration ended)
 * @param capability Skeleton pose detection capability
 * @param nId        User ID
 * @param eStatus    Status of calibration 
 * @param pCookie    User data
 * @return None
 */
void XN_CALLBACK_TYPE CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
    gIsCalibrating = FALSE;
    
    // Calibration done successfully
    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
		if (gNeedPose)
		{
        	printf("NID Skeleton: Calibration completed successfully for user ID%d.\n", nId);
        	printf("NID Skeleton: Start tracking user ID%d...\n", nId);
        }
        gUser->GetSkeletonCap().StartTracking(nId);
    }
    // Calibration failed
    else
    {
		if (gNeedPose)
		{
        	printf("NID Skeleton: Calibration failed for user ID%d.\n", nId);
			gUser->GetPoseDetectionCap().StartPoseDetection(CAL_POSE, nId);
		}
		else
		{
			gUser->GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
    }
}


/**
 * Create Depth RGB image 
 * @param S Simulink S-Function structure
 * @param depthMD Depth meta data
 * @return None
 */
static void nidCreateDepthRGB(SimStruct *S, xn::DepthMetaData* depthMD) 
{
    int xRes = depthMD->FullXRes();
    int yRes = depthMD->FullYRes();
    int npixels = xRes * yRes;
    const int MAX_DEPTH = gDepth->GetDeviceMaxDepth();
    std::vector<real_T> depthHist(MAX_DEPTH);
    XnDepthPixel* depthRun;
    
    // Count up each non-zero depth value 
    int numOfPoints = 0;
    depthRun = (XnDepthPixel*)depthMD->Data();
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
    for (int i = 1; i < MAX_DEPTH; ++i)
    {
        depthHist[i] += depthHist[i-1];
    }

    // Convert to RGB value (0 to 255)
    if (numOfPoints != 0)
    {
        for (int i = 1; i < MAX_DEPTH; ++i)
        {
            // Brighter color means closer to NID
            depthHist[i] = (real_T)(255 * (1.0f - (depthHist[i] / numOfPoints)));
        }
    }
        
    // Overdraw image by color histogram
    memset(gRGB, 0, xRes * yRes * sizeof(RGB24Pixel_T)); // Black: Depth = 0 
	RGB24Pixel_T* rgbRun = gRGB;
    depthRun = (XnDepthPixel*)depthMD->Data();
    for (int i = 0; i < npixels; i++)
    {
        if (*depthRun != 0)
        {
            // Depth histogram image with green color
            rgbRun->nGreen = (XnUInt8)depthHist[*depthRun];
        }
        rgbRun++;
        depthRun++;
    }
}


/**
 * Create Motion RGB image
 * @param S Simulink S-Function structure
 * @param sceneMD Scene meta data
 * @return None
 */
static void nidCreateMotionRGB(SimStruct *S, xn::SceneMetaData* sceneMD) 
{
    uint16_T* pScene = (uint16_T*)sceneMD->Data();
	RGB24Pixel_T* rgbRun = gRGB;
    int nPixels = sceneMD->FullXRes() * sceneMD->FullYRes();

    for (int i = 0; i < nPixels; i++)
    {
    	*rgbRun = *(MOTION_COLORS + (*pScene & MOTION_ID_MASK));
        pScene++;
        rgbRun++;
    }
}


/**
 * Create Skeleton RGB image
 * @param S Simulink S-Function structure
 * @param sceneMD Scene meta data
 * @param numOfSkeletonTracking number of people in skeleton tracking
 * @param pSkeletonData Pointer to Skeleton data in tracking
 * @return None
 */
static void nidCreateSkeletonRGB(SimStruct *S, xn::SceneMetaData* sceneMD, int32_T numOfSkeletonTracking, const XnUserID* pSkeletonData) 
{
	// Draw Motion image
	nidCreateMotionRGB(S, sceneMD);

    // Overdraw crosses on skeleton joints
    const xn::LabelMap& labels = sceneMD->LabelMap();
    int xRes = sceneMD->FullXRes();
    int yRes = sceneMD->FullYRes();
    int nPixels = xRes * yRes;
	for (int skeletonDataIdx = 0; skeletonDataIdx < numOfSkeletonTracking; skeletonDataIdx++)
	{
        for (int jointIdx = 0; jointIdx < NUM_OF_ALL_SKELETON_JOINTS_OPEN_NI; jointIdx++)
        {
            XnSkeletonJointPosition joint;
            gUser->GetSkeletonCap().GetSkeletonJointPosition(pSkeletonData[skeletonDataIdx], (XnSkeletonJoint)(jointIdx+1), joint);

            if (SKELETON_JOINTS_MASK_OPEN_NI[jointIdx] == 1)
            {
                XnPoint3D pt;
                gDepth->ConvertRealWorldToProjective(1, &(joint.position), &pt);

                if (((int)pt.X < 0 || (int)pt.X >= xRes) || ((int)pt.Y < 0 || (int)pt.Y >= yRes))
                {
                    // Joint position is out side of the view range, so skip to overdraw. 
                    continue;
                }
                
                //
                // Draw a cross on each skeleton joint
                //
            	int LINE_LENGTH = 11; // should be odd number. Default value is for VGA(640x480)
            	if (sceneMD->FullXRes() == 320)
            	{
            		LINE_LENGTH = 5; // for QVGA(320x240)
				}
                int LINE_LENGTH_OFFSET = (LINE_LENGTH-1)/2;
                int centerOfCross = (int)pt.X + (int)pt.Y * sceneMD->FullXRes();
                int modRow = centerOfCross % sceneMD->FullXRes();
                int modCol = centerOfCross % sceneMD->FullYRes();
                XnLabel label = labels((int)pt.X, (int)pt.Y) & MOTION_ID_MASK;
                for (int length = -LINE_LENGTH_OFFSET; length <= LINE_LENGTH_OFFSET; length++)
                {
                	// Draw a line horizontally
                    int hpixel = centerOfCross + length;
                    if ((hpixel >= 0) && (hpixel < nPixels) && 
                        (modRow + length >= 0) && (modRow + length < (int)sceneMD->FullXRes()))
                    {
                        *(gRGB + hpixel) = SKELETON_CROSS_COLOR;
                    }
                	// Draw a line vertically
                    int vpixel = centerOfCross + length * sceneMD->FullXRes();
                    if ((vpixel >= 0) && (vpixel < nPixels) && 
                        (modCol + length >= 0) && (modCol + length < (int)sceneMD->FullYRes()))
                    {                          
                        *(gRGB + vpixel) = SKELETON_CROSS_COLOR;
                    }
                }
            }
        }
    }

   	// While a user is in calibration, the user color is turned to be white to indicate it
	if (gNeedPose && gIsCalibrating)
	{
		const RGB24Pixel_T CALIB_COLOR = {255, 255, 255}; // White
       	// User is not in tracking
		RGB24Pixel_T* rgbRun = gRGB;
       	for (int y = 0; y < yRes; ++y)
       	{
           	for (int x = 0; x < xRes; ++x)
           	{
               	XnLabel label = labels(x, y) & MOTION_ID_MASK;
               	if (label == gUserIdInCalibration)
               	{
                   	*rgbRun = CALIB_COLOR;
               	}
               	rgbRun++;
           	}
       	}
    }
}


/**
 * Create IR RGB image
 * @param S Simulink S-Function structure
 * @param irMD IR meta data
 * @return None
 */
static void nidCreateIrRGB(SimStruct *S, xn::IRMetaData* irMD) 
{
    const uint16_T* pIr = irMD->Data();
	RGB24Pixel_T* rgbRun = gRGB;
    int nPixels = irMD->FullXRes() * irMD->FullYRes();

    for (int i = 0; i < nPixels; i++)
    {
        uint16_T ir = *(pIr + i);
        if (ir > 255)
        {
            ir = 255;
        }
        rgbRun->nRed = rgbRun->nGreen = rgbRun->nBlue = (uint8_T)ir;
        rgbRun++;
    }
}


/**
 * Convert NID XYZ coordinates with MATLAB matrix alignment and
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param depthMD Depth meta data
 * @return None
 */
static void nidDepthOutputs(SimStruct *S, xn::DepthMetaData* depthMD)
{
    XnDepthPixel* pDepth = (XnDepthPixel*)depthMD->Data();
    int xRes = depthMD->FullXRes();
    int yRes = depthMD->FullYRes();
    int npixels = xRes * yRes;
        
    if (DEPTH_OUTPUT(S) == DEPTH_SIGNAL)
    {
    	// Convert row based memory alignment to column based
        real_T *out = (real_T*)ssGetOutputPortSignal(S,0);
        for (int x = 0; x < xRes; x++)
        {
            for (int y = 0; y < yRes; y++)
            {
                 *out = (real_T)*(pDepth + x + xRes * y) * 0.001; // convert mm to m 
                 out++;
            }
        }

        if (VIEWER(S) == _ON)
        {
            nidCreateDepthRGB(S, depthMD);
            
            uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,1);
            uint8_T* outG = (uint8_T*)(outR + npixels);
            uint8_T* outB = (uint8_T*)(outG + npixels);
            
            CopyRGB24ToSimulink(outR, outG, outB, gRGB, xRes, yRes);
        }
    }
    else if (DEPTH_OUTPUT(S) == XYZ_SIGNAL || DEPTH_OUTPUT(S) == X_Y_Z_SIGNAL)
    {
        // Convert depth to XYZ coordinates
        for(int y = 0; y < yRes; y++)
        {
            for(int x = 0; x < xRes; x++)
            {
                XYZPixel_T* buf = (XYZPixel_T*)(gXYZ + x + y * xRes);
                buf->nX = (XnFloat)x;
                buf->nY = (XnFloat)y;
                buf->nZ = (XnFloat)*pDepth;
                pDepth++;
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
            gDepth->ConvertProjectiveToRealWorld(npixels, (const XnPoint3D*)gXYZ, (XnPoint3D*)gXYZ_RW);
            CopyXYZRWToSimulink(outX, outY, outZ, gXYZ_RW, xRes, yRes);
        }
        else
        {
            // Default depth XYZ is projective coordinates
            CopyXYZToSimulink(outX, outY, outZ, gXYZ, xRes, yRes);
        }
        
        if (VIEWER(S) == _ON)
        {
             nidCreateDepthRGB(S, depthMD);
             
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
             
             CopyRGB24ToSimulink(outR, outG, outB, gRGB, xRes, yRes);
        }
    }
}


/**
 * Convert motion data with MATLAB matrix alignment and
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param sceneMD Scene meta data
 * @return None
 */
static void nidMotionOutputs(SimStruct *S, xn::SceneMetaData* sceneMD)
{
    uint16_T* pScene = (uint16_T*)sceneMD->Data();
    int xRes = sceneMD->FullXRes();
    int yRes = sceneMD->FullYRes();
        
    // Convert row based memory alignment to column based
    uint16_T *out = (uint16_T*)ssGetOutputPortSignal(S,0);
    for (int x = 0; x < xRes; x++)
    {
        for (int y = 0; y < yRes; y++)
        {
             *out = (uint16_T)*(pScene + x + xRes * y);
             out++; 
        }
    }

    if (VIEWER(S) == _ON)
    {
        nidCreateMotionRGB(S, sceneMD);
        
    	int npixels = xRes * yRes;
        uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,1);
        uint8_T* outG = (uint8_T*)(outR + npixels);
        uint8_T* outB = (uint8_T*)(outG + npixels);
        
        CopyRGB24ToSimulink(outR, outG, outB, gRGB, xRes, yRes);
    }
}


/**
 * Convert NID RGB with MATLAB matrix alignment and 
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param imageMD Image meta data
 * @reutnr None
 */
static void nidImageOutputs(SimStruct *S, xn::ImageMetaData* imageMD)
{
    RGB24Pixel_T* rgb = (RGB24Pixel_T*)imageMD->Data();
    int xRes = imageMD->FullXRes();
    int yRes = imageMD->FullYRes();
    int npixels = xRes * yRes;

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
    
    CopyRGB24ToSimulink(outR, outG, outB, rgb, xRes, yRes);
}


/**
 * Inject Skeleton data to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param sceneMD Scene meta data
 * @param numOfSkeletonTracking number of people in skeleton tracking
 * @param pSkeletonData Pointer to Skeleton data in tracking
 * @return None
 */
static void nidSkeletonOutputs(SimStruct *S, xn::SceneMetaData* sceneMD, int32_T numOfSkeletonTracking, const XnUserID* pSkeletonData)
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
    	outY = (real_T*)(outX + NUM_OF_SKELETON_JOINTS_OPEN_NI);
    	outZ = (real_T*)(outY + NUM_OF_SKELETON_JOINTS_OPEN_NI);
		// Flush output ports
    	memset((char*)outX, 0, sizeof(real_T) * NUM_OF_SKELETON_JOINTS_OPEN_NI * 3);

    	if (skeletonDataIdx < numOfSkeletonTracking)
    	{
        	for (int jointIdx = 0; jointIdx < NUM_OF_ALL_SKELETON_JOINTS_OPEN_NI; jointIdx++)
        	{
            	XnSkeletonJointPosition joint;
            	gUser->GetSkeletonCap().GetSkeletonJointPosition(pSkeletonData[skeletonDataIdx], (XnSkeletonJoint)(jointIdx+1), joint);
            	if (SKELETON_JOINTS_MASK_OPEN_NI[jointIdx] == 1)
            	{
                	XnPoint3D pt_rw;  // Real World XYZ coordinates
                	pt_rw = joint.position;

                	if (CONV_XYZ_RW(S) == _ON)
                	{
                    	*outX = (real_T)pt_rw.X * 0.001;
                    	*outY = (real_T)pt_rw.Y * 0.001;
                    	*outZ = (real_T)pt_rw.Z * 0.001;
                	}
                	else
                	{
                		XnPoint3D pt_prj; // Projective XYZ coordinates
                		gDepth->ConvertRealWorldToProjective(1, &(joint.position), &pt_prj);
                    	*outX = (real_T)((int_T)pt_prj.X);
                    	*outY = (real_T)((int_T)pt_prj.Y);
                    	*outZ = (real_T)pt_prj.Z * 0.001;
                	}
                	outX++;
                	outY++;
                	outZ++;
            	}
        	}
        }
    }

	// Image output
    if (VIEWER(S) == _ON)
    {
        uint16_T* pScene = (uint16_T*)sceneMD->Data();
        int xRes = sceneMD->FullXRes();
        int yRes = sceneMD->FullYRes();
        int npixels = xRes * yRes;

        nidCreateSkeletonRGB(S, sceneMD, numOfSkeletonTracking, pSkeletonData);
        
        uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,port_idx);
        uint8_T* outG = (uint8_T*)(outR + npixels);
        uint8_T* outB = (uint8_T*)(outG + npixels);
        
        CopyRGB24ToSimulink(outR, outG, outB, gRGB, xRes, yRes);
    }
}


/**
 * Convert IR data with MATLAB matrix alignment and
 * inject to Simulink outports. This method should be called in mdlOutputs
 * @param S Simulink S-Function structure
 * @param irMD IR meta data
 * @return None
 */
static void nidIrOutputs(SimStruct *S, xn::IRMetaData* irMD)
{
    uint16_T* pIr = (uint16_T*)irMD->Data();
    int xRes = irMD->FullXRes();
    int yRes = irMD->FullYRes();
    int npixels = xRes * yRes;
        
    // Convert row based memory alignment to column based
    uint16_T *out = (uint16_T*)ssGetOutputPortSignal(S,0);
    for (int x = 0; x < xRes; x++)
    {
        for (int y = 0; y < yRes; y++)
        {
             *out = (uint16_T)*(pIr + x + xRes * y);
             out++; 
        }
    }

    if (VIEWER(S) == _ON)
    {
        nidCreateIrRGB(S, irMD);
        
        uint8_T* outR = (uint8_T*)ssGetOutputPortSignal(S,1);
        uint8_T* outG = (uint8_T*)(outR + npixels);
        uint8_T* outB = (uint8_T*)(outG + npixels);
        
        CopyRGB24ToSimulink(outR, outG, outB, gRGB, xRes, yRes);
    }
}


/**
 * Inject NID outputs. This method should be called in mdlOutputs.
 * @param S Simulink S-Function structure
 * @return None
 */
void nidDeviceOutputs(SimStruct *S)
{
    if (BLOCK_TYPE(S) == IMAQ_BLOCK)
    {
        if (gContext == NULL)
        {
            // Reach here only at the beginning of the first simulation step        
            SpecNID_T spec;
            GetSpecNID(&spec);

            // Initialize NID
            gContext = new xn::Context;
            XnStatus rc = gContext->Init();
            if (rc != XN_STATUS_OK || gContext == NULL)
            {
                ssSetErrorStatus(S, ERROR_INIT_NID);
                return;
            }
            
            // Set global mirror
            if (MIRROR_IMAGE(S) == _ON)
            {
                gContext->SetGlobalMirror(TRUE);
            }
            else
            {
                gContext->SetGlobalMirror(FALSE);
            }

            if ((IMAQ_DEPTH(S) == _ON || IMAQ_MOTION(S) == _ON || IMAQ_SKELETON(S) == _ON || IMAQ_IR(S) == _ON) && gDepth == NULL)
            {
                // Motion(Scene), Skeleton and IR need to have Depth
                gDepth = new xn::DepthGenerator;
                XnStatus rc = gDepth->Create(*gContext);
                if (rc != XN_STATUS_OK || gDepth == NULL)
                {
                    ssSetErrorStatus(S, ERROR_INIT_NID);
                    return;
                }
                XnMapOutputMode outputmode = { spec.depthRes.xRes, spec.depthRes.yRes, spec.depthFPS };
                gDepth->SetMapOutputMode(outputmode);
            }

            if ((IMAQ_MOTION(S) == _ON || IMAQ_SKELETON(S) == _ON) && gScene == NULL)
            {  
                // Skeleton needs to have Scene
                gScene = new xn::SceneAnalyzer;
                XnStatus rc = gScene->Create(*gContext);
                if (rc != XN_STATUS_OK || gScene == NULL)
                {
                    ssSetErrorStatus(S, ERROR_INIT_NID);
                    return;
                }
                XnMapOutputMode outputmode = { spec.depthRes.xRes, spec.depthRes.yRes, spec.depthFPS };
                gScene->SetMapOutputMode(outputmode);
            }

            if (IMAQ_IMAGE(S) == _ON && gImage == NULL)
            {  
                gImage = new xn::ImageGenerator;
                XnStatus rc = gImage->Create(*gContext);
                if (rc != XN_STATUS_OK || gImage == NULL)
                {
                    ssSetErrorStatus(S, ERROR_INIT_NID);
                    return;
                }
                XnMapOutputMode outputmode = { spec.imageRes.xRes, spec.imageRes.yRes, spec.imageFPS };
                gImage->SetMapOutputMode(outputmode);
            }
        
            if (IMAQ_IR(S) == _ON && gIr == NULL)
            {
                gIr = new xn::IRGenerator;
                XnStatus rc = gIr->Create(*gContext);
                if (rc != XN_STATUS_OK || gIr == NULL)
                {
                    ssSetErrorStatus(S, ERROR_INIT_NID);
                    return;
                }
                XnMapOutputMode outputmode = { spec.depthRes.xRes, spec.depthRes.yRes, spec.depthFPS };
                gIr->SetMapOutputMode(outputmode);
            }

            if (IMAQ_SKELETON(S) == _ON && gUser == NULL)
            {  
                gUser = new xn::UserGenerator;
                XnStatus rc = gUser->Create(*gContext);
                if (rc != XN_STATUS_OK || gUser == NULL)
                {
                    ssSetErrorStatus(S, ERROR_INIT_NID);
                    return;
                }
                
   				if (!gUser->IsCapabilitySupported(XN_CAPABILITY_SKELETON))
   				{
       				ssSetErrorStatus(S, "NID Error: User recognition is not supported by the version of OpenNI in use.");
       				return;
   				}
            
                // Register callbacks
                gIsCalibrating = FALSE; // reset calibration flag
                gUser->RegisterUserCallbacks(UserDetected, UserLost, NULL, hUserCallbacks);
                gUser->GetSkeletonCap().RegisterToCalibrationStart(CalibrationStart, NULL, hCalibrationStart);
                gUser->GetSkeletonCap().RegisterToCalibrationComplete(CalibrationEnd, NULL, hCalibrationEnd);
                // Since OpenNI 1.4.0.2, calibration pose less skeleton tracking has been supported.
                // However, depending on use cases, explicit calibration pose might be prefereble, therefore,
                // need of calibration pose can be configured in NID Skeleton block parameters 
				gNeedPose = TRUE; // Default
				
				// Checks pose less calibration is supported by the version of OpenNI in use
				if (!IsPoseNeededForSkeleton())
				{
               		if (gUser->GetSkeletonCap().NeedPoseForCalibration())
           			{
						printf("NID Skeleton: Pose less calibration is not supported by the version of OpenNI in use.\n");
               		}
               		else
               		{
						gNeedPose = FALSE; // Pose less calibration is supported 
					}
               	}

				if (gNeedPose)
				{
       				if (!gUser->IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
       				{
           				ssSetErrorStatus(S, "NID Error: Pose detection is not supported by the version of OpenNI in use.");
           				return;
					}
					gUser->GetPoseDetectionCap().RegisterToPoseDetected(PoseDetected, NULL, hPoseDetected);
					gUser->GetPoseDetectionCap().RegisterToOutOfPose(PoseLost, NULL, hPoseLost);
				}
                // Tracking all in skeleton
                gUser->GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
            }
        
            // Adjust other view points to Image
            if (ADJUST_VIEW_POINT(S) == _ON && gDepth != NULL && gImage != NULL)
            {
                xn::AlternativeViewPointCapability viewPoint = gDepth->GetAlternativeViewPointCap();
                viewPoint.SetViewPoint(*gImage);
            }

            if ((IMAQ_DEPTH(S) == _ON || IMAQ_MOTION(S) == _ON || IMAQ_SKELETON(S) == _ON || IMAQ_IR(S) == _ON) && gRGB == NULL)
            {
                gRGB = new RGB24Pixel_T[spec.depthRes.xRes * spec.depthRes.yRes];
                if (gRGB == NULL)
                {
                    ssSetErrorStatus(S, "NID Error: Failed to heap RGB buffer.");
                    return;
                }
            }

            if (IMAQ_DEPTH(S) == _ON && gXYZ == NULL && gXYZ_RW == NULL)
            {
                gXYZ = new XYZPixel_T[spec.depthRes.xRes * spec.depthRes.yRes];
                gXYZ_RW = new XYZPixel_T[spec.depthRes.xRes * spec.depthRes.yRes];
                if (gXYZ == NULL || gXYZ_RW == NULL)
                {
                    ssSetErrorStatus(S, "NID Error: Failed to heap XYZ buffer.");
                    return;
                }
            }

        
#if defined(_WIN32) || defined(_WIN64)
            // timeGetTime is the time since Windows started, so no possibility to be less than IMAQ_DT.
            // The first IMAQ block output does not have any wait. 
            lastIMAQTime = timeGetTime() - (int32_T)(IMAQ_SAMPLE_TIME(S) * 1000);
#else
			// For Linux and MacOSX
            struct timeval tv;
            gettimeofday(&tv, NULL);
            lastIMAQTime = (int32_T)(tv.tv_sec*1000 + tv.tv_usec/1000) - (int32_T)(IMAQ_SAMPLE_TIME(S) * 1000);
#endif
            gContext->StartGeneratingAll();
        }

#if defined(_WIN32) || defined(_WIN64)
        // It seems to be more consistent wait time than Windows Sleep API
        // The next three lines should be atomic and should not change the order
        while(timeGetTime() < (lastIMAQTime + (int32_T)(IMAQ_SAMPLE_TIME(S) * 1000))); // software loop wait
        lastIMAQTime = timeGetTime(); // Save the last update time for the next update
#else
		// For Linux and MacOSX
        struct timeval tv;
        while(1)
        {
	        gettimeofday(&tv, NULL); // get time in micro seconds
	        if ((int32_T)(tv.tv_sec*1000 + tv.tv_usec/1000) >= (lastIMAQTime + (int32_T)(IMAQ_SAMPLE_TIME(S) * 1000)))
	        {
				break;
			}
		}
		lastIMAQTime = (int32_T)(tv.tv_sec*1000 + tv.tv_usec/1000); // Save the last update time for the next update
#endif
        gContext->WaitAndUpdateAll(); // Update all devices
        
        // Update device data 
        if (gDepth != NULL)
        {
            gDepth->GetMetaData(gDepthMD);
        }

        if (gScene != NULL)
        {
            gScene->GetMetaData(gSceneMD);
        }

        if (gIr != NULL)
        {
            gIr->GetMetaData(gIrMD);
        }

        if (gImage != NULL)
        {
            gImage->GetMetaData(gImageMD);
        }
        
        // Inject last FPS time to handshake with other device blocks
        uint32_T *out = (uint32_T*)ssGetOutputPortSignal(S,0);
        *out = lastIMAQTime;
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
            nidDepthOutputs(S, &gDepthMD);
        }
        else if (BLOCK_TYPE(S) == MOTION_BLOCK)
        {
            nidMotionOutputs(S, &gSceneMD);
        }
        else if (BLOCK_TYPE(S) == IMAGE_BLOCK)
        {
            nidImageOutputs(S, &gImageMD);
        }
        else if (BLOCK_TYPE(S) == IR_BLOCK)
        {
            nidIrOutputs(S, &gIrMD);
        }
        else if (BLOCK_TYPE(S) == SKELETON_BLOCK)
        {
            // Search for the users in tracking
            XnUserID aUsers[MOTION_ID_MASK];
            XnUInt16 nUsers = MOTION_ID_MASK;
            gUser->GetUsers(aUsers, nUsers);
			//
			// Since ver.0.4.2, multiple skeleton tracking has been supported.
			//
            XnUserID skeletonData[MAX_NUM_OF_SKELETON_TRACKING];
            int numOfSkeletonTracking = 0;
            for (int idx = 0; idx < nUsers; idx++)
            {
                if (gUser->GetSkeletonCap().IsTracking(aUsers[idx]))
                {
					skeletonData[numOfSkeletonTracking++] = aUsers[idx];
            	    if (numOfSkeletonTracking >= NUM_OF_SKELETON_TRACKING(S))
            	    {
               			break;
					}
                }
            }

            nidSkeletonOutputs(S, &gSceneMD, numOfSkeletonTracking, skeletonData);
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
    if (gContext != NULL)
    {
		 // Shut down NID and destroy all device objects
        gContext->Shutdown();
    }

   	// Release memory (device objects have to be deleted by the Shutdown, just for double check)
   	delete gContext;
   	delete gDepth;
   	delete gScene;
   	delete gImage;
   	delete gUser;
   	delete gIr;
   	
   	delete [] gRGB;
   	delete [] gXYZ;
   	delete [] gXYZ_RW;

   	// Reset for next simulation
   	gContext = NULL;
   	gDepth   = NULL;
   	gScene   = NULL;
   	gImage   = NULL;
   	gUser    = NULL;
   	gIr      = NULL;
   	gRGB     = NULL;
   	gXYZ     = NULL;
   	gXYZ_RW  = NULL;
}


/**
 * Get NID SDK
 * @return NID SDK enum
 */
eNID_SDK nidGetSDK(void)
{
    return OPEN_NI;
}


/* [EOF] */