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
openni::VideoStream* color;

    
/* The matlab mex function */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
    unsigned char *Iout;
    UInt64 *MXadress;
    
    if(nrhs==0)
    {
       printf("Open failed: Give Pointer to Kinect as input\n");
       mexErrMsgTxt("Kinect Error"); 
    }

    MXadress = (UInt64*)mxGetData(prhs[0]);
    if(MXadress[1]>0)
	{ 
		color = ((openni::VideoStream*)MXadress[1]); 
	}
	else
	{
		mexErrMsgTxt("No Video Node in Kinect Context"); 
	}
    
	openni::VideoStream& m_colorStream = color[0];
    openni::VideoMode colorVideoMode;
    
    colorVideoMode =  m_colorStream.getVideoMode();
  	int g_nXRes = colorVideoMode.getResolutionX();
	int g_nYRes = colorVideoMode.getResolutionY();
	
	openni::VideoFrameRef m_colorFrame;
    m_colorStream.readFrame(&m_colorFrame);
	
		
	if (!m_colorFrame.isValid())
	{
		printf("SimpleViewer: Color frame invalid\n");
    }
    
	const unsigned char* pImage = ( unsigned char*)m_colorFrame.getData();
    int Jdimsc[3];
    Jdimsc[0]=3; 
    Jdimsc[1]=g_nXRes;
    Jdimsc[2]=g_nYRes;
    
    plhs[0] = mxCreateNumericArray(3, Jdimsc, mxUINT8_CLASS, mxREAL);
    Iout = ( unsigned char*)mxGetData(plhs[0]);
    memcpy (Iout,pImage,Jdimsc[0]*Jdimsc[1]*Jdimsc[2]);  
}
