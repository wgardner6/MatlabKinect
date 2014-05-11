function makecmexfile4nid
% makecmexfile4nid
% This function creates a C MEX S-Function for the Simulink for NID.
%

%
% Copyright 2011-2012 The MathWorks, Inc.
%

% Get information about NID SDK
simulinkfornidinfo;

%
% OpenNI
%
if strcmp(SIMULINKFORNIDDRIVER, 'OPEN_NI')
    if strcmp(computer, 'PCWIN')
        OpenNiPathLib=[SIMULINKFORNIDOPENNIPATH '\Lib'];
        OpenNiPathInclude=[SIMULINKFORNIDOPENNIPATH '\Include'];
    elseif strcmp(computer, 'PCWIN64')
        OpenNiPathLib=[SIMULINKFORNIDOPENNIPATH '\Lib64'];
        OpenNiPathInclude=[SIMULINKFORNIDOPENNIPATH '\Include'];
    elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI64')
        OpenNiPathLib=[SIMULINKFORNIDOPENNIPATH '/Lib'];       
        OpenNiPathInclude=[SIMULINKFORNIDOPENNIPATH '/Include'];
    elseif strcmp(computer, ' GLNX86') || strcmp(computer, 'GLNXA64') 
        OpenNiPathLib=[SIMULINKFORNIDOPENNIPATH '/Lib'];       
        OpenNiPathInclude=[SIMULINKFORNIDOPENNIPATH '/Include'];
    end

    clear SIMULINKFORNIDOPENNIPATH;
    clear mex;

    if strcmp(computer, 'PCWIN')
        mex('sfun_nid.cpp', 'openNI_wrapper.cpp', 'wrapper_common.cpp', ['-L' OpenNiPathLib], '-lopenNI', ['-I' OpenNiPathInclude]);
    elseif strcmp(computer, 'PCWIN64')
        mex('sfun_nid.cpp', 'openNI_wrapper.cpp', 'wrapper_common.cpp', ['-L' OpenNiPathLib], '-lopenNI64', ['-I' OpenNiPathInclude]);
    elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI64')
        mex('./sfun_nid.cpp', './openNI_wrapper.cpp', './wrapper_common.cpp', ['-L' OpenNiPathLib], '-lopenNI', ['-I' OpenNiPathInclude]);
    elseif strcmp(computer, ' GLNX86') || strcmp(computer, 'GLNXA64') 
        mex('./sfun_nid.cpp', './openNI_wrapper.cpp', './wrapper_common.cpp', ['-L' OpenNiPathLib], '-lopenNI', ['-I' OpenNiPathInclude]);
    else
        error('ERROR: Simulink for NID unsupported platform.');
    end


%
% Kinect SDK
%
elseif  strcmp(SIMULINKFORNIDDRIVER, 'KINECT_SDK')
    if strcmp(computer, 'PCWIN')
        KinectSDKPathLib = [SIMULINKFORNIDKINECTSDKPATH '\lib\x86'];
    elseif strcmp(computer, 'PCWIN64')
        KinectSDKPathLib = [SIMULINKFORNIDKINECTSDKPATH '\lib\amd64'];
    else
        error('ERROR: Simulink for NID unsupported platform.');
    end
        
    KinectSDKPathInclude=[SIMULINKFORNIDKINECTSDKPATH '\inc'];

    clear mex;
    mex('sfun_nid.cpp', 'kinectSDK_wrapper.cpp', 'wrapper_common.cpp', ['-L' KinectSDKPathLib], '-lKinect10', ['-I' KinectSDKPathInclude]);
    
    clear SIMULINKFORNIDKINECTSDKPATH;
end

clear SIMULINKFORNIDDRIVER;

% EOF
