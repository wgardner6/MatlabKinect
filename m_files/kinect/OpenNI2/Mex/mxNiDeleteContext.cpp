#include "mex.h"
#include "math.h"
#include "OpenNI.h"
        
#ifdef WINDOWS
    typedef unsigned __int64 UInt64; 
#else
    typedef unsigned long long UInt64; 
#endif
    
//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
openni::Device* device;
openni::VideoStream* depth;
openni::VideoStream* color;

/* The matlab mex function */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] ) {
    UInt64 *MXadress;
    if(nrhs==0)
    {
       printf("Close failed: Give Pointer to Kinect as input\n");
       mexErrMsgTxt("Kinect Error"); 
    }
    MXadress = (UInt64*)mxGetData(prhs[0]);
   
    if(MXadress[1]>0)
	{ 
		color = ((openni::VideoStream*)MXadress[1]); 
        color[0].destroy();
	}
    
    if(MXadress[2]>0)
	{ 
		depth = ((openni::VideoStream*)MXadress[2]); 
        depth[0].destroy();
	}
    
    if(MXadress[0]>0)
    { 
        device = ((openni::Device*) MXadress[0]);
		device[0].close();
    }
    	
	//g_Context.Shutdown();
	// depth.destroy();
    //color.destroy();
    openni::OpenNI::shutdown();
}
