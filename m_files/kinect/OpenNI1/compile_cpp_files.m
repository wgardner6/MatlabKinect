function compile_cpp_files(OpenNiPath)
% This function compile_cpp_files will compile the c++ code files
% which wraps OpenNI 1.* for the Kinect in Matlab.
%
% Please install first on your computer:
% - NITE-Win64-1.5.2.21-Dev.msi
% - OpenNI-Win64-1.5.4.0-Dev
%
% Just execute by:
%
%   compile_c_files 
%
% or with specifying the OpenNI path
% 
%   compile_cpp_files('C:\Program Files\OpenNI);
%
% Note!, on strange compile errors change ['-I' OpenNiPathInclude '\'] to ['-I' OpenNiPathInclude '']

% Detect 32/64bit and Linux/Mac/PC
c = computer;
is64=length(c)>2&&strcmp(c(end-1:end),'64');

if(nargin<1)
    OpenNiPathInclude=getenv('OPEN_NI_INCLUDE');
	if(is64)
		OpenNiPathInclude = '/usr/include/ni';
        OpenNiPathLib = '/home/will/kinect/OpenNi/Platform/Linux/Redist/OpenNI-Bin-Dev-Linux-x64-v1.5.4.0/Lib';
        %OpenNiPathLib=getenv('OPEN_NI_LIB64');
    else
		OpenNiPathLib=getenv('OPEN_NI_LIB');
	end

	if(isempty(OpenNiPathInclude)||isempty(OpenNiPathLib))
        error('OpenNI path not found, Please call the function like compile_cpp_files(''examplepath\openNI'')');
    end
else
    OpenNiPathInclude=[OpenNiPath 'Include'];
	if(is64)
		OpenNiPathLib=[OpenNiPath 'Lib64'];
	else
		OpenNiPathLib=[OpenNiPath 'Lib'];
	end
end

cd('Mex');
files=dir('*.cpp');
for i=1:length(files)
    Filename=files(i).name
    clear(Filename); 
	if(is64)
        OpenNiPathLib
		mex('-v',['-L' OpenNiPathLib],'-lOpenNI',['-I' OpenNiPathInclude ''],Filename);
	else
		mex('-v',['-L' OpenNiPathLib],'-lopenNI',['-I' OpenNiPathInclude '\'],Filename);
	end
end
cd('..');