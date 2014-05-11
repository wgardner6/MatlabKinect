function point_cloud_viewer(block)
% Level-2 MATLAB file S-Function for displaying 3-D Point Cloud view for Simulink NID Depth XYZ data and Image data.
%
% Copyright 2011 The MathWorks, Inc.
%

% Register number of input and output ports
block.NumInputPorts   = 3;
block.NumOutputPorts  = 0;
% Setup functional port properties to dynamically inherited.
block.AllowSignalsWithMoreThan2D = 1;
block.SetPreCompInpPortInfoToDynamic;
% Register the parameters.
block.NumDialogPrms     = 1;
block.DialogPrmsTunable = {'Nontunable'};
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
set(gcf, 'Name', 'Point Cloud Viewer', 'NumberTitle', 'off');
whitebg('k'); % Backgroud color is black
if block.DialogPrm(1).Data == 0
    view(37.5, 30); % Set default view
end
%endfunction

function Output(block)
cla; % Clear current axis and objects
if block.DialogPrm(1).Data == 1
   view(block.InputPort(3).Data(1), block.InputPort(3).Data(2)); % Enable view signal
end
surface('XData', block.InputPort(1).Data(:,:,1), ...
        'YData', block.InputPort(1).Data(:,:,3), ... 
        'ZData', block.InputPort(1).Data(:,:,2), ...
        'Cdata', double(block.InputPort(2).Data)/255, ...
        'CDataMapping', 'direct', ...
        'EdgeColor', 'flat', ...
        'FaceColor', 'none', ...
        'LineStyle', 'none', ...
        'Marker', '.', ...
        'MarkerSize', 1, ...
        'MarkerFaceColor', 'flat');
set(gca, 'XLim', [-3.2 3.2], 'YLim', [0 4.8], 'ZLim', [-1.6 3.2]); % Set X/Y/Z limits in meters
%endfunction
