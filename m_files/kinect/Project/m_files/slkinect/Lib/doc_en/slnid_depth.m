%% NID Depth
%
% Return depth data from Kinect device.
%
% Copyright 2011-2012 The MathWorks, Inc.

%% Input Signal
%
% * SYNC (uint32): IMAQ synchronization time in msec.

%% Output Signals
%
% * Depth (double): Depth in meters from device at each pixel. Depth = 0 means that depth measurement is lost.
%
% * XYZ (double): XYZ coordinates. Either Real world or Projective coordinate can be configured.
%
% * X/Y/Z (double): Separated X, Y, Z coordinates. Either Real world or Projective coordinates can be configured.
%
% * Image (uint8): RGB24 image of depth. Brighter means to be closer to the device. Black color means depth data is lost.

%% Block Parameters
%
% * Output: Specify depth output mode.
%
% * Real world coordinates (Default: Projective): Choose Real world (X, Y, Z in meter) or Projective (X, Y in pixel and Z in meter) coordinates for XYZ and X/Y/Z
% depth output modes.
%
% Projective coodinates:
%
% <<ProjectiveXYZ.png>>
%
% Real world coodinates:
%
% <<RealWorldXYZ.png>>
%
% * Image output: Return RGB24 image of depth.
