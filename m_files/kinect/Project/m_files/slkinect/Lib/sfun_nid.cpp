/**
 * sfun_nid.cpp
 *
 * C MEX S-Function for Natural Interaction Device (NID). This function returns variety types of information
 * which NID provides depending on the block parameters configuration and invoked from
 * the following NID blocks.
 *
 * NID IMAQ block
 * NID Depth block
 * NID Motion block
 * NID Image block
 * NID Skeleton block
 * NID IR block
 *  
 * All NID objects should be handled according Singleton design pattern 
 * (does not create NID objects for each block, but only single instance for all blocks)
 * to get the faster simulation performance.
 *
 * Copyright 2011 The MathWorks, Inc.
 */

#include <string.h>

#include "sfun_nid.h"
#include "wrapper_common.h"


//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------

static char msg[1024];


//---------------------------------------------------------------------------
// S-function methods
//---------------------------------------------------------------------------

/**
 * Function: mdlInitializeSizes
 * Abstract:
 *    The sizes information is used by Simulink to determine the S-function
 *    block's characteristics (number of inputs, outputs, states, etc.).
 */
static void mdlInitializeSizes(SimStruct *S)
{
    // Check block parameters
    ssSetNumSFcnParams(S, NUM_OF_PARAMS);  /* Number of expected parameters */
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) 
    {
        ssSetErrorStatus(S, "S-Function Parameter Error: Number of parameters");
        return;
    }

    //
    // Inport/Outport configuartions
    //
    if (BLOCK_TYPE(S) == IMAQ_BLOCK)
    {
		//  set NID resolution and fps
		if (nidGetSDK() == OPEN_NI)
		{
        	SetSpecNID(IMAQ_RES_FPS(S));
       		SetNeedPoseForSkeleton(boolean_T(NEED_PSI_POSE(S) == _ON));

        	ssSetNumInputPorts(S, 0);

        	ssSetNumOutputPorts(S, 1);
        	ssSetOutputPortWidth(S, 0, 1);
        	ssSetOutputPortDataType(S, 0, SS_UINT32);
        }
        else // Kinect SDK
        {
			SetSpecNID(VGA_30FPS);

			// Inport
			if (KINECT_ANGLE_INPUT(S) == _ON)
			{
        		ssSetNumInputPorts(S, 1);
        		ssSetInputPortWidth(S, 0, 1);
        		ssSetInputPortDataType(S, 0, SS_INT32);
        		ssSetInputPortRequiredContiguous(S, 0, true);
        		ssSetInputPortDirectFeedThrough(S, 0, 1);
        	}
        	else
        	{
        		ssSetNumInputPorts(S, 0);
			}

			// Outport
        	if (KINECT_ANGLE_READ(S) == _OFF)
        	{
        		ssSetNumOutputPorts(S, 1);
        		ssSetOutputPortWidth(S, 0, 1);
        		ssSetOutputPortDataType(S, 0, SS_UINT32);
			}
			else
			{
        		ssSetNumOutputPorts(S, 2);
        		// SYNC outport
        		ssSetOutputPortWidth(S, 0, 1);
        		ssSetOutputPortDataType(S, 0, SS_UINT32);
        		// Angle outport
        		ssSetOutputPortWidth(S, 1, 1);
        		ssSetOutputPortDataType(S, 1, SS_INT32);
			}
		}
    }
    else
    {
        SpecNID_T spec;
        if (GetSpecNID(&spec))
        {
            ssSetErrorStatus(S, "S-Function Parameter Error: NID IMAQ Resolution and FPS");
		}
		
        ssAllowSignalsWithMoreThan2D(S);
        
        ssSetNumInputPorts(S, 1);
        ssSetInputPortWidth(S, 0, 1);
        ssSetInputPortDataType(S, 0, SS_UINT32);
        ssSetInputPortRequiredContiguous(S, 0, true);
        ssSetInputPortDirectFeedThrough(S, 0, 1);
        
        if (BLOCK_TYPE(S) == DEPTH_BLOCK)
        {
            if (DEPTH_OUTPUT(S) == DEPTH_SIGNAL)
            {
                if (VIEWER(S) == _OFF)
                {
                    ssSetNumOutputPorts(S, 1);
                }
                else if (VIEWER(S) == _ON)
                {
                    ssSetNumOutputPorts(S, 2);
                }
                    
                // Depth [m] with double data type
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[2];
                di.numDims = 2;
                dims[0] = spec.depthRes.yRes;
                dims[1] = spec.depthRes.xRes;
                di.dims = dims;
                di.width = dims[0]*dims[1];
                ssSetOutputPortDimensionInfo(S, 0, &di);
                ssSetOutputPortDataType(S, 0, SS_DOUBLE);

                if  (VIEWER(S) == _ON)
                {
                    // RGB with uint8 data type
                    DECL_AND_INIT_DIMSINFO(di);
                    int_T dims[3];
                    di.numDims = 3;
                    dims[0] = spec.depthRes.yRes;
                    dims[1] = spec.depthRes.xRes;
                    dims[2] = 3;
                    di.dims = dims;
                    di.width = dims[0]*dims[1]*dims[2];
                    ssSetOutputPortDimensionInfo(S, 1, &di);
                    ssSetOutputPortDataType(S, 1, SS_UINT8);
                }
            }
            else if (DEPTH_OUTPUT(S) == XYZ_SIGNAL)
            {
                if (VIEWER(S) == _OFF)
                {
                    ssSetNumOutputPorts(S, 1);
                }
                else if (VIEWER(S) == _ON)
                {
                    ssSetNumOutputPorts(S, 2);
                }

                // XYZ coordinates [m] with double data type
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[3];
                di.numDims = 3;
                dims[0] = spec.depthRes.yRes;
                dims[1] = spec.depthRes.xRes;
                dims[2] = 3;
                di.dims = dims;
                di.width = dims[0]*dims[1]*dims[2];
                ssSetOutputPortDimensionInfo(S, 0, &di);
                ssSetOutputPortDataType(S, 0, SS_DOUBLE);

                if  (VIEWER(S) == _ON)
                {
                    // RGB with uint8 data type
                    DECL_AND_INIT_DIMSINFO(di);
                    int_T dims[3];
                    di.numDims = 3;
                    dims[0] = spec.depthRes.yRes;
                    dims[1] = spec.depthRes.xRes;
                    dims[2] = 3;
                    di.dims = dims;
                    di.width = dims[0]*dims[1]*dims[2];
                    ssSetOutputPortDimensionInfo(S, 1, &di);
                    ssSetOutputPortDataType(S, 1, SS_UINT8);
                }
            }
            else if (DEPTH_OUTPUT(S) == X_Y_Z_SIGNAL)
            {
                if (VIEWER(S) == _OFF)
                {
                    ssSetNumOutputPorts(S, 3);
                }
                else if (VIEWER(S) == _ON)
                {
                    ssSetNumOutputPorts(S, 4);
                }

                // X, Y, Z coordinates [m] with double data type
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[2];
                di.numDims = 2;
                dims[0] = spec.depthRes.yRes;
                dims[1] = spec.depthRes.xRes;
                di.dims = dims;
                di.width = dims[0]*dims[1];
                ssSetOutputPortDimensionInfo(S, 0, &di);
                ssSetOutputPortDimensionInfo(S, 1, &di);
                ssSetOutputPortDimensionInfo(S, 2, &di);
                ssSetOutputPortDataType(S, 0, SS_DOUBLE);
                ssSetOutputPortDataType(S, 1, SS_DOUBLE);
                ssSetOutputPortDataType(S, 2, SS_DOUBLE);

                if  (VIEWER(S) == _ON)
                {
                    // RGB with uint8 data type
                    DECL_AND_INIT_DIMSINFO(di);
                    int_T dims[3];
                    di.numDims = 3;
                    dims[0] = spec.depthRes.yRes;
                    dims[1] = spec.depthRes.xRes;
                    dims[2] = 3;
                    di.dims = dims;
                    di.width = dims[0]*dims[1]*dims[2];
                    ssSetOutputPortDimensionInfo(S, 3, &di);
                    ssSetOutputPortDataType(S, 3, SS_UINT8);
                }
            }
            else
            {
                ssSetErrorStatus(S, "S-Function Parameter Error: Depth output");
            }
        }
        else if (BLOCK_TYPE(S) == MOTION_BLOCK)
        {
            if (VIEWER(S) == _OFF)
            {
                ssSetNumOutputPorts(S, 1);
            }
            else if (VIEWER(S) == _ON)
            {
                ssSetNumOutputPorts(S, 2);
            }

            // Motion with uint16 data type
            DECL_AND_INIT_DIMSINFO(di);
            int_T dims[2];
            di.numDims = 2;
            dims[0] = spec.depthRes.yRes;
            dims[1] = spec.depthRes.xRes;
            di.dims = dims;
            di.width = dims[0]*dims[1];
            ssSetOutputPortDimensionInfo(S, 0, &di);
            ssSetOutputPortDataType(S, 0, SS_UINT16);

            if (VIEWER(S) == _ON)
            {
                // RGB with uint8 data type
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[3];
                di.numDims = 3;
                dims[0] = spec.depthRes.yRes;
                dims[1] = spec.depthRes.xRes;
                dims[2] = 3;
                di.dims = dims;
                di.width = dims[0]*dims[1]*dims[2];
                ssSetOutputPortDimensionInfo(S, 1, &di);
                ssSetOutputPortDataType(S, 1, SS_UINT8);
            }
        }
        else if (BLOCK_TYPE(S) == IMAGE_BLOCK)
        {
            if (IMAGE_OUTPUT(S) == RGB_SIGNAL)
            {
                // RGB with uint8 data type
                ssSetNumOutputPorts(S, 1);            
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[3];
                di.numDims = 3;
                dims[0] = spec.imageRes.yRes;
                dims[1] = spec.imageRes.xRes;
                dims[2] = 3;
                di.dims = dims;
                di.width = dims[0]*dims[1]*dims[2];
                ssSetOutputPortDimensionInfo(S, 0, &di);
                ssSetOutputPortDataType(S, 0, SS_UINT8);
            }
            else if (IMAGE_OUTPUT(S) == R_G_B_SIGNAL)
            {
                // R, G, B with uint8 data type
                ssSetNumOutputPorts(S, 3);            
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[2];
                di.numDims = 2;
                dims[0] = spec.imageRes.yRes;
                dims[1] = spec.imageRes.xRes;
                di.dims = dims;
                di.width = dims[0]*dims[1];
                ssSetOutputPortDimensionInfo(S, 0, &di);
                ssSetOutputPortDimensionInfo(S, 1, &di);
                ssSetOutputPortDimensionInfo(S, 2, &di);
                ssSetOutputPortDataType(S, 0, SS_UINT8);
                ssSetOutputPortDataType(S, 1, SS_UINT8);
                ssSetOutputPortDataType(S, 2, SS_UINT8);
            }
            else
            {
                ssSetErrorStatus(S, "S-Function Parameter Error: Image output");
            }
        }
        else if (BLOCK_TYPE(S) == SKELETON_BLOCK)
        {
			if (NUM_OF_SKELETON_TRACKING(S) > MAX_NUM_OF_SKELETON_TRACKING)
			{
                ssSetErrorStatus(S, "S-Function Parameter Error: Max number of Skeleton Tracking");
			}
			
            if (VIEWER(S) == _OFF)
            {
                ssSetNumOutputPorts(S, NUM_OF_SKELETON_TRACKING(S)+1);
            }
            else if (VIEWER(S) == _ON)
            {
                ssSetNumOutputPorts(S, NUM_OF_SKELETON_TRACKING(S)+2);
            }

			int outport_idx = 0;

            // Number of people in skeleton tracking
            ssSetOutputPortWidth(S, outport_idx, 1);
            ssSetOutputPortDataType(S, outport_idx, SS_INT32);
            ++outport_idx;

            // Skelton joints(XYZ) with double data type
            for (int i = 0; i < NUM_OF_SKELETON_TRACKING(S); i++)
            {
            	int numOfSkeletonJoints;
            	if (nidGetSDK() == OPEN_NI)
            	{
                	numOfSkeletonJoints = NUM_OF_SKELETON_JOINTS_OPEN_NI;
            	}
            	else if (nidGetSDK() == KINECT_SDK)
            	{
                	numOfSkeletonJoints = NUM_OF_SKELETON_JOINTS_KINECT_SDK;
            	}
            	else
            	{   
                	ssSetErrorStatus(S, "S-Function Parameter Error: Invalid NID SDK");
            	}            
            	DECL_AND_INIT_DIMSINFO(di);
            	int_T dims[3];
            	di.numDims = 3;
            	dims[0] = 1;
            	dims[1] = numOfSkeletonJoints;
            	dims[2] = 3;
            	di.dims = dims;
            	di.width = dims[0]*dims[1]*dims[2];
            
            	ssSetOutputPortDimensionInfo(S, outport_idx, &di);
            	ssSetOutputPortDataType(S, outport_idx, SS_DOUBLE);
            	++outport_idx;
            }

            if (VIEWER(S) == _ON)
            {
                // RGB with uint8 data type
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[3];
                di.numDims = 3;
                dims[0] = spec.depthRes.yRes;
                dims[1] = spec.depthRes.xRes;
                dims[2] = 3;
                di.dims = dims;
                di.width = dims[0]*dims[1]*dims[2];
                
                ssSetOutputPortDimensionInfo(S, outport_idx, &di);
                ssSetOutputPortDataType(S, outport_idx, SS_UINT8);
            }
        }
        else if (BLOCK_TYPE(S) == IR_BLOCK)
        {
            if (VIEWER(S) == _OFF)
            {
                ssSetNumOutputPorts(S, 1);
            }
            else if (VIEWER(S) == _ON)
            {
                ssSetNumOutputPorts(S, 2);
            }
                    
            // IR with uint16 data type
            DECL_AND_INIT_DIMSINFO(di);
            int_T dims[2];
            di.numDims = 2;
            dims[0] = spec.depthRes.yRes;
            dims[1] = spec.depthRes.xRes;
            di.dims = dims;
            di.width = dims[0]*dims[1];
            ssSetOutputPortDimensionInfo(S, 0, &di);
            ssSetOutputPortDataType(S, 0, SS_UINT16);

            if  (VIEWER(S) == _ON)
            {
                // RGB with uint8 data type
                DECL_AND_INIT_DIMSINFO(di);
                int_T dims[3];
                di.numDims = 3;
                dims[0] = spec.depthRes.yRes;
                dims[1] = spec.depthRes.xRes;
                dims[2] = 3;
                di.dims = dims;
                di.width = dims[0]*dims[1]*dims[2];
                ssSetOutputPortDimensionInfo(S, 1, &di);
                ssSetOutputPortDataType(S, 1, SS_UINT8);
            }
        }
        else
        {   
            ssSetErrorStatus(S, "S-Function Parameter Error: Block Type");
        }
    }
    
    // Other configurations
    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);
    ssSetNumSampleTimes(S, 1);
    ssSetNumRWork(S, 0);
    ssSetNumIWork(S, 0);
    ssSetNumPWork(S, 0);
    ssSetNumModes(S, 0);
    ssSetNumNonsampledZCs(S, 0);
    ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
}


/**
 * Function: mdlInitializeSampleTimes
 * Abstract:
 *    This function is used to specify the sample time(s) for your
 *    S-function. You must register the same number of sample times as
 *    specified in ssSetNumSampleTimes.
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    if (BLOCK_TYPE(S) == IMAQ_BLOCK)
    {
		if (nidGetSDK() == KINECT_SDK && IMAQ_SAMPLE_TIME(S) < MIN_IMAQ_SAMPLE_TIME_VGA)
		{
			// There is possiblity to configure sample time less than 1/30 by configuring QVGA with Kinect SDK,
			// So this error check is needed.
           	ssSetErrorStatus(S, "S-Function Parameter Error: Kinect SDK does not support sample time less than 1/30 [second].");
		}
       	// Range check of IMAQ_SAMPLE_TIME(S) parameter is done in the block parameter call backs of 
       	// NID IMAQ block. Thus, no range check is needed here.
       	ssSetSampleTime( S, 0, IMAQ_SAMPLE_TIME(S) );
    }
    else
    {
        // Non IMAQ blocks inherit the sample time from IMAQ block.
        ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );
    }
    
    ssSetOffsetTime( S, 0, 0 );
    ssSetModelReferenceSampleTimeDefaultInheritance(S);
}


/**
 * Function: mdlOutputs
 * Abstract:
 *    In this function, you compute the outputs of your S-function
 *    block.
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{
    nidDeviceOutputs(S);
}


/**
 * Function: mdlTerminate
 * Abstract:
 *    In this function, you should perform any actions that are necessary
 *    at the termination of a simulation.  For example, if memory was
 *    allocated in mdlStart, this is the place to free it.
 */
static void mdlTerminate(SimStruct *S)
{
    nidDeviceTerminate(S);
}


//---------------------------------------------------------------------------
// Required S-function trailer
//---------------------------------------------------------------------------

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif

/* [EOF] */
