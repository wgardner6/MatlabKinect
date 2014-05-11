%% NID Motion
%
% Return motion detected object ID from Kinect device.
%
% Copyright 2011-2012 The MathWorks, Inc.

%% Input Signal
%
% * SYNC (uint32): IMAQ synchronization time in msec.

%% Output Signals
%
% * Motion (uint16): Motion detected object ID (0 to 15) at each pixel. ID 0 means no motion object.
%
% * Image (uint8): RGB24 image of Motion.
% Object IDs are represented as the following colors.
% Red = 1, Green = 2, Blue = 3, Yellow = 4, Cyan = 5, Magenda = 6, Orange = 7,
% Dark green = 8, Teal = 9, Purple = 10, Olive = 11, Maroon = 12, Silver = 13,
% Navy = 14, Gold = 15.

%% Block Parameters
%
% * Image output: Return RGB24 image of Motion.
