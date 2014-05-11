function makeInfo=rtwmakecfg()
%RTWMAKECFG adds include and source directories to rtw make files.
%  makeInfo=RTWMAKECFG returns a structured array containing
%  following field:
%     makeInfo.includePath - cell array containing additional include
%                            directories. Those directories will be
%                            expanded into include instructions of rtw
%                            generated make files.
%
%     makeInfo.sourcePath  - cell array containing additional source
%                            directories. Those directories will be
%                            expanded into rules of rtw generated make
%                            files.

%
%       Copyright 2011 The MathWorks, Inc.
%

disp(['### Running rtwmakecfg from folder: ',pwd]);

% Get information about NID SDK
simulinkfornidinfo;

%
% OpenNI
%
if strcmp(SIMULINKFORNIDDRIVER, 'OPEN_NI')
    % Specify header include path
    makeInfo.includePath = ...
    { ...
        [SIMULINKFORNIDOPENNIPATH '\Include']
    };
                    
    % Specify library
    if strcmp(computer, 'PCWIN')
        makeInfo.linkLibsObjs = ...
            { ...
            [SIMULINKFORNIDOPENNIPATH '\Lib\openNI.lib']
            };
    elseif strcmp(computer, 'PCWIN64')
        makeInfo.linkLibsObjs = ...
            { ...
            [SIMULINKFORNIDOPENNIPATH '\Lib64\openNI64.lib']
            };
    else
        error('ERROR: Simulink for NID unsupported platform.');
    end

    makeInfo.sources  = {'openNI_wrapper.cpp', 'wrapper_common.cpp'};
    
    clear SIMULINKFORNIDOPENNIPATH;


%
% Kinect SDK
%
elseif  strcmp(SIMULINKFORNIDDRIVER, 'KINECT_SDK')
    % Specify header include path
    makeInfo.includePath = ...
    { ...
        [SIMULINKFORNIDKINECTSDKPATH '\inc']
    };
                        
    % Specify library
    if strcmp(computer, 'PCWIN')
        makeInfo.linkLibsObjs = ...
            { ...
            [SIMULINKFORNIDKINECTSDKPATH '\lib\x86\Kinect10.lib']
            };
    elseif strcmp(computer, 'PCWIN64')
        makeInfo.linkLibsObjs = ...
            { ...
            [SIMULINKFORNIDKINECTSDKPATH '\lib\amd64\Kinect10.lib']
            };
    else
        error('ERROR: Simulink for NID unsupported platform.');
    end

    makeInfo.sources  = {'kinectSDK_wrapper.cpp', 'wrapper_common.cpp'};

    clear SIMULINKFORNIDKINECTSDKPATH;
end

makeInfo.precompile = 1;

clear SIMULINKFORNIDDRIVER;


% [EOF]
