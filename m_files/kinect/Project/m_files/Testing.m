% Senior Design Project
% Written by William Gardner Fall 2013
% Saint Louis University
close all, clear all, closepreview
utilpath = fullfile(matlabroot, 'toolbox', 'imaq', 'imaqdemos', ...
    'html', 'KinectForWindows');
addpath(utilpath);
hwInfo = imaqhwinfo('kinect');
% hwInfo.DeviceInfo(1)
colorVid = videoinput('kinect',1,'RGB_640x480');
depthVid = videoinput('kinect',2,'Depth_640x480');
% Set the triggering mode to 'manual'
triggerconfig([colorVid depthVid],'manual');
set([colorVid depthVid], 'FramesPerTrigger', 100);
start([colorVid depthVid]);
% Trigger the devices to start logging of data.
trigger([colorVid depthVid]);
% Retrieve the acquired data
[colorFrameData, colorTimeData, colorMetaData] = getdata(colorVid);
[depthFrameData, depthTimeData, depthMetaData] = getdata(depthVid);
% Stop the devices
stop([colorVid depthVid]);

% src = getselectedsource(vid);
preview(colorVid);
preview(depthVid);
src = getselectedsource(depthVid);
% src.TrackingMode = 'Skeleton';
set(src,'TrackingMode','Skeleton')
set(src, 'BodyPosture','Standing')
numFrames = 110;
colorVid.FramesPerTrigger = numFrames;
depthVid.FramesPerTrigger = numFrames;
start([colorVid depthVid]);
trigger([colorVid depthVid]);
% set(src, 'BodyPosture', 'Standing')
% set(src, 'SkeletonsToTrack', [1])
% set(src, 'BacklightCompensation' , 'LowLightsPriority');
[frameDataColor] = getdata(colorVid);
[frameDataDepth, timeDataDepth, metaDataDepth] = getdata(depthVid);
metaDataDepth.JointWorldCoordinates
% anyPositionsTracked = any(metaDataDepth(95).IsPositionTracked ~= 0);
% anySkeletonsTracked = any(metaDataDepth(95).IsSkeletonTracked ~= 0);
SkeleVector = find([metaDataDepth.IsSkeletonTracked]);
V = mod(SkeleVector,6);
if(all(V==V(1)))
    trackedSkeletons = V(1);
end
% jointCoordinates = metaDataDepth(95).JointWorldCoordinates(:, :, trackedSkeletons);
% Skeleton's joint indices with respect to the color image
% jointIndices = metaDataDepth(95).JointImageIndices(:, :, trackedSkeletons);
metaCell = {metaDataDepth.JointImageIndices};
meta = cell(1,length(metaCell));
for n = 1 : length(metaCell)
    meta{n} = metaCell{n}(:,:,trackedSkeletons);
end
% Pull out the 95th color frame
% image = frameDataColor(:, :, :, 95);
% image = frameDataColor(:, :, :, :);

ThreeD = {metaDataDepth.JointWorldCoordinates};
figure(5)
for n = 1:length(ThreeD)
%     scatter3(ThreeD{n}(:,1,1),ThreeD{n}(:,2,1),ThreeD{n}(:,3,1))
%     ylabel('Y Coords')
%     xlabel('X Coords')
%     zlabel('Z Coords')
%     title(num2str(n))
    % Test Right Elbow exercises - will need Shoulder_Right(9), Elbow_Right(10), and Wrist_Right(11)
% distA = upper arm, distB = forearm, distC = hypotenuse
% angleC = elbow angle
     distA = sqrt((ThreeD{n}(9,1,1)-ThreeD{n}(10,1,1))^2 + (ThreeD{n}(9,2,1)-ThreeD{n}(10,2,1))^2 + (ThreeD{n}(9,3,1)-ThreeD{n}(10,3,1))^2);
     distB = sqrt((ThreeD{n}(11,1,1)-ThreeD{n}(10,1,1))^2 + (ThreeD{n}(11,2,1)-ThreeD{n}(10,2,1))^2 + (ThreeD{n}(11,3,1)-ThreeD{n}(10,3,1))^2);
     distC = sqrt((ThreeD{n}(9,1,1)-ThreeD{n}(11,1,1))^2 + (ThreeD{n}(9,2,1)-ThreeD{n}(11,2,1))^2 + (ThreeD{n}(9,3,1)-ThreeD{n}(11,3,1))^2);
     angleC(n) = acos((distB^2 + distC^2 - distA^2)/(2*distB*distC))
    pause(0.5)
end
angleC = angleC.*(180/pi)
% Find number of Skeletons tracked
nSkeleton = length(trackedSkeletons);
% Plot the skeleton
% SP_skeletonViewer(jointIndices, image, nSkeleton);
SP_skeletonViewer(meta, frameDataColor, nSkeleton);

waitforbuttonpress
closepreview