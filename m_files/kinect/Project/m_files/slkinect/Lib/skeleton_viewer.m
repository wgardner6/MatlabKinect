function skeleton_viewer(block)
% Level-2 MATLAB file S-Function for displaying Skeleton joints for Simulink NID Skeleton data.
%
% Copyright 2011 The MathWorks, Inc.
%

% Register number of input and output ports
block.NumInputPorts   = 2;
block.NumOutputPorts  = 0;
% Setup functional port properties to dynamically inherited.
block.AllowSignalsWithMoreThan2D = 1;
block.SetPreCompInpPortInfoToDynamic;
% Set block sample time to inherited
block.SampleTimes = [-1 0];
% Register methods
block.RegBlockMethod('SetInputPortDimensions', @SetInputPortDims);
block.RegBlockMethod('Start',   @Start);
block.RegBlockMethod('Outputs', @Output);  
%endfunction

% -----------------------------------------------------------------
% Callback functions
% -----------------------------------------------------------------

function SetInputPortDims(block, idx, di)
block.InputPort(idx).Dimensions = di; % Set compiled dimensions
%endfunction

function Start(block)
set(gcf, 'Name', 'Skeleton Viewer', 'NumberTitle', 'off');
whitebg('k'); % Backgroud color is black
view(60, 20); % Set default view
%endfunction

function Output(block)
cla; % Clear current axis and objects
if block.InputPort(1).Data == 1 % In tracking
    %
    % OpenNI SDK
    %
    if length(block.InputPort(2).Data) == 15
        % Connect all Skeleton joints by the line
        id =  [6 5 4 3 10 11 12 11 10 13 14 15 14 13 3 7 8 9 8 7 2 1 2 4]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'Color', 'w', 'LineStyle', '-', 'LineWidth', 2, 'Marker', 'none');
        % Skeleton joints at the left side of the body
        id = [4 5 6 10 11 12]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'LineStyle', 'none', 'Marker', 'o', 'MarkerFaceColor', 'g', 'MarkerEdgeColor', 'g', 'MarkerSize', 4);
        % Skeleton joints at the center of the body
        id = [1 2 3]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'LineStyle', 'none', 'Marker', 'o', 'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'r', 'MarkerSize', 4);
        % Skeleton joints at the right side of the body
        id = [7 8 9 13 14 15]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'LineStyle', 'none', 'Marker', 'o', 'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'c',  'MarkerSize', 4);

    %
    % Kinect SDK
    %
    elseif length(block.InputPort(2).Data) == 20
        % Connect all Skeleton joints by the line
        id =  [12 11 10 9 3 4 3 5 6 7 8 7 6 5 3 9 2 5 2 1 17 18 19 20 19 18 17 1 13 14 15 16]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'Color', 'w', 'LineStyle', '-', 'LineWidth', 2, 'Marker', 'none');
        % Skeleton joints at the right side of the body
        id = [9 10 11 12 17 18 19 20]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'LineStyle', 'none', 'Marker', 'o', 'MarkerFaceColor', 'g', 'MarkerEdgeColor', 'g', 'MarkerSize', 4);
        % Skeleton joints at the center of the body
        id = [1 2 3 4]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'LineStyle', 'none', 'Marker', 'o', 'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'r', 'MarkerSize', 4);
        % Skeleton joints at the left side of the body
        id = [5 6 7 8 13 14 15 16]; % Each joint ID is described in help of NID Skeleton block
        joints = zeros(1, length(id), 3);
        for i = 1:length(id)
            for j = 1:3
                joints(1,i,j) = block.InputPort(2).Data(1,id(i),j);
            end
        end
        line(joints(:,:,1), joints(:,:,3), joints(:,:,2), 'LineStyle', 'none', 'Marker', 'o', 'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'c',  'MarkerSize', 4);
    end
end
set(gca, 'XLim', [-1.5 1.5], 'YLim', [0 4.0], 'ZLim', [-1.0 2.0]); % Set X/Y/Z limits in meters
set(gca, 'XGrid', 'on', 'YGrid', 'on'); % Draw X/Y grid
%endfunction
