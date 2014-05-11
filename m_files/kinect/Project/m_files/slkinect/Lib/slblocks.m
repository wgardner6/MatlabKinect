function blkStruct = slblocks
% slblocks
% Register Simulink NID blocks to Simulink library browser

%   Copyright 2011 The MathWorks, Inc.

blkStruct.Name    = 'Simulink Natural Interaction Device';
blkStruct.OpenFcn = '';
blkStruct.MaskInitialization = '';
blkStruct.MaskDisplay = '';

% Define the library list for the Simulink Library browser.
% Return the name of the library model and the name for it
%
Browser(1).Library = 'nid_lib';
Browser(1).Name    = 'Simulink Natural Interaction Device';
Browser(1).IsFlat  = 1;% Is this library "flat" (i.e. no subsystems)?

blkStruct.Browser = Browser;

% Define information for model updater
blkStruct.ModelUpdaterMethods.fhDetermineBrokenLinks = [];

% End of slblocks.m
