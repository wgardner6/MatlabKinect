% Senior Design Project
% Written by William Gardner, Michael Dryden, Jacob Howenstein,
% Jordan Wisch and Benjamin Ford
% Spring 2014 BME 496
% Saint Louis University

close all, clear all, closepreview
% Adds the appropriate toolboxes and adds them to the path
utilpath = fullfile(matlabroot, 'toolbox', 'imaq', 'imaqdemos', ...
    'html', 'KinectForWindows');
addpath(utilpath);
% Allows viewing of the connection to Kinect
hwInfo = imaqhwinfo('linuxvideo');
% hwInfo.DeviceInfo(1)

% Denotes the video inputs of the color and depth vids
colorVid = videoinput('linuxvideo',1);
%depthVid = videoinput('linuxvideo',2);

% Set the triggering mode to 'manual'
%triggerconfig([colorVid depthVid],'manual');

% Previews the color and depth vids
preview(colorVid);
preview(depthVid);

% Gets the depth vid
src = getselectedsource(depthVid);

% Sets the body posture and tracking mode for a standing patient
set(src,'TrackingMode','Skeleton')
set(src, 'BodyPosture','Standing')

% Sets number of frames per trigger
numFrames = 110;

% Triggers the color and depth vids
colorVid.FramesPerTrigger = numFrames;
depthVid.FramesPerTrigger = numFrames;

% Starts the vids
start([colorVid depthVid]);

% Starts the trigger
trigger([colorVid depthVid]);

% Gets data that allows accessing of metaData
[frameDataColor] = getdata(colorVid);
[frameDataDepth, timeDataDepth, metaDataDepth] = getdata(depthVid);

% Find which skeletons are tracked
SkeleVector = find([metaDataDepth.IsSkeletonTracked]);
V = mod(SkeleVector,6);
if(all(V==V(1)))
    trackedSkeletons = V(1);
end

% This creates the data required for skeleton overlay
metaCell = {metaDataDepth.JointImageIndices};
meta = cell(1,length(metaCell));
for n = 1 : length(metaCell)
    meta{n} = metaCell{n}(:,:,trackedSkeletons);
end

% This plots the skeleton overlay over the image of the patient

% Find number of Skeletons tracked
nSkeleton = length(trackedSkeletons);
% Plot the skeleton
SP_skeletonViewer(meta, frameDataColor, nSkeleton);


% This creates the data required for 3D coordinates

ThreeD = {metaDataDepth.JointWorldCoordinates};

figure(5)
for n = 1:length(metaCell)
    scatter3(ThreeD{n}(:,1,1),ThreeD{n}(:,2,1),ThreeD{n}(:,3,1))
    ylabel('Y Coords')
    xlabel('X Coords')
    zlabel('Z Coords')
    title(num2str(n))
    % Test Right Elbow exercises - will need Shoulder_Right(9), Elbow_Right(10), and Wrist_Right(11)
    % distA = upper arm, distB = forearm, distC = hypotenuse
    % angleC = elbow angle
    distA = sqrt((ThreeD(9,1,1)-ThreeD(10,1,1))^2 + (ThreeD(9,2,1)-ThreeD(10,2,1))^2 + (ThreeD(9,3,1)-ThreeD(10,3,1))^2);
    distB = sqrt((ThreeD(11,1,1)-ThreeD(10,1,1))^2 + (ThreeD(11,2,1)-ThreeD(10,2,1))^2 + (ThreeD(11,3,1)-ThreeD(10,3,1))^2);
    distC = sqrt((ThreeD(9,1,1)-ThreeD(11,1,1))^2 + (ThreeD(9,2,1)-ThreeD(11,2,1))^2 + (ThreeD(9,3,1)-ThreeD(11,3,1))^2);
    angleC = arccos((distB^2 + distC^2 - distA^2)/(2*distB*distC))
    pause(0.5)
end