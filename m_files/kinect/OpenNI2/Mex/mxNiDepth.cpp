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
openni::VideoStream* depth;

    
/* The matlab mex function */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
    unsigned short *Iout;
    UInt64 *MXadress;
    
    if(nrhs==0)
    {
       printf("Open failed: Give Pointer to Kinect as input\n");
       mexErrMsgTxt("Kinect Error"); 
    }

    MXadress = (UInt64*)mxGetData(prhs[0]);
    if(MXadress[2]>0)
	{ 
		depth = ((openni::VideoStream*)MXadress[2]); 
	}
	else
	{
		mexErrMsgTxt("No Depth Node in Kinect Context"); 
	}
    
	openni::VideoStream& m_depthStream = depth[0];
    openni::VideoMode depthVideoMode;
    depthVideoMode = m_depthStream.getVideoMode();
  	int g_nXRes = depthVideoMode.getResolutionX();
	int g_nYRes = depthVideoMode.getResolutionY();
	
	openni::VideoFrameRef m_depthFrame;
    m_depthStream.readFrame(&m_depthFrame);
	
	uint16_t* pDepthRow = (uint16_t*)m_depthFrame.getData();
		
	if (!m_depthFrame.isValid())
	{
		printf("SimpleViewer: Depth frame invalid\n");
    }
    
	int Jdimsc[2];
    Jdimsc[0]=g_nXRes; 
	Jdimsc[1]=g_nYRes;
    
    plhs[0] = mxCreateNumericArray(2, Jdimsc, mxUINT16_CLASS, mxREAL);
    Iout = (unsigned short*)mxGetData(plhs[0]);
    memcpy (Iout,pDepthRow,Jdimsc[0]*Jdimsc[1]*2);  
}
