%% NID IR
%
% Return IR (Infrared) data from Kinect device. IR block can co-exist only with IMAQ and Depth blocks.
%
% Copyright 2011-2012 The MathWorks, Inc.

%% Input Signal
%
% * SYNC (uint32): IMAQ synchronization time in msec.

%% Output Signals
%
% * IR (uint16): IR data at each pixel.
%
% * Image (uint8): RGB24(grayscale) image of IR.

%% Block Parameters
%
% * Image output: Return RGB24 image of IR.