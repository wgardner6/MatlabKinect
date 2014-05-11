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
openni::Device device;
openni::VideoStream depth, color;
    
    
/* The matlab mex function */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] ) 
{
    UInt64 *MXadress;
    int Jdimsc[2]={1,5};
    openni::Status rc = openni::STATUS_OK;
        
    const char* deviceURI = openni::ANY_DEVICE;
	//if (argc > 1)
	//{
	//	deviceURI = argv[1];
	//}

	// Output the Point to the Kinect Object
    plhs[0] = mxCreateNumericArray(2, Jdimsc, mxUINT64_CLASS, mxREAL);
    MXadress = (UInt64*)mxGetData(plhs[0]);
        
	rc = openni::OpenNI::initialize();

	printf("After initialization:\n%s\n", openni::OpenNI::getExtendedError());

	rc = device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return;
	}
	else
	{
		MXadress[0] = ( UInt64)&device;
	}
	
	rc = color.create(device, openni::SENSOR_COLOR);
	if (rc == openni::STATUS_OK)
	{
		rc = color.start();
		if (rc != openni::STATUS_OK)
		{
			printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
			color.destroy();
		}
        else
        {
		    MXadress[1] = ( UInt64)&color;
            printf("Succesfull started color stream \n%");
        }
	}
	else
	{
		printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	rc = depth.create(device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		rc = depth.start();
		if (rc != openni::STATUS_OK)
		{
			printf("SimpleViewer: Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
			depth.destroy();
		}
        else
        {
		    MXadress[2] = ( UInt64)&depth;
            printf("Succesfull started depth stream \n%");
        }
	}
	else
	{
		printf("SimpleViewer: Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
	}
}
