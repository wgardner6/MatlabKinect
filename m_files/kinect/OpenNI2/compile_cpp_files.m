function compile_cpp_files(OpenNiPath)
% This function compile_cpp_files will compile the c++ code files
% which wraps OpenNI 2.* for the Kinect in Matlab.
%
% Please install first on your computer:
% - NiTE-Windows-x64-2.0.0
% - OpenNI-Windows-x64-2.2.0
%
% Just execute by:
%
%   compile_c_files 
%
% or with specifying the OpenNI path
% 
%   compile_cpp_files('C:\Program Files\OpenNI2\');
%
% Note!, on strange compile errors change ['-I' OpenNiPathInclude '\'] to ['-I' OpenNiPathInclude '']

% Detect 32/64bit and Linux/Mac/PC
c = computer;
is64=length(c)>2&&strcmp(c(end-1:end),'64');

if(nargin<1)
	if(is64)
		OpenNiPathLib=getenv('OPENNI2_LIB64');
		OpenNiPathInclude=getenv('OPENNI2_INCLUDE64');
	    OpenNiPathRedist=getenv('OPENNI2_REDIST64');
	else
		OpenNiPathLib=getenv('OPENNI2_LIB');
		OpenNiPathInclude=getenv('OPENNI2_INCLUDE');
		OpenNiPathRedist=getenv('OPENNI2_REDIST');
	end

	if(isempty(OpenNiPathInclude)||isempty(OpenNiPathLib))
        error('OpenNI path not found, Please call the function like compile_cpp_files(''examplepath\openNI'')');
    end
else
    OpenNiPathInclude=[OpenNiPath 'Include'];	
	OpenNiPathLib=[OpenNiPath 'Lib'];
	OpenNiPathRedist=[OpenNiPath 'Redist'];
end

setenv('path',[getenv('path'), ';',OpenNiPathInclude, ';',OpenNiPathLib, ';', OpenNiPathRedist]);

cd('Mex');
files=dir('*.cpp');
for i=1:length(files)
    Filename=files(i).name;
    clear(Filename); 
    mex('-v',['-L' OpenNiPathLib],'-lopenNI2',['-I' OpenNiPathInclude '\'],Filename);
end
cd('..');
addpath('Mex')