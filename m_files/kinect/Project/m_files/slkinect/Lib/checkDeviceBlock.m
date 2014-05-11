function checkDeviceBlock(model, maskType)
% checkDeviceBlock
%
% This function is invoked block initFcn callback of NID device blocks.

%
%   Copyright 2011-2012 The MathWorks, Inc.
%

% According to a user request, multiple instance of Device blocks is supported.
% So comment out the below function. 
% checkBlockSingleton(model, maskType); % If not, it generates error 

% Check a NID IMAQ block is placed in a model
hasIMAQBlock(model); % If not, it generates error

% End of function
