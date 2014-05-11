function corner_detection_viewer(block)
% Level-2 MATLAB file S-Function for displaying 3-D Corner Detection view for Simulink NID Depth XYZ data and Image data.
%
% Copyright 2011 The MathWorks, Inc.
%

% Register number of input and output ports
block.NumInputPorts   = 1;
block.NumOutputPorts  = 0;
% Setup functional port properties to dynamically inherited.
block.AllowSignalsWithMoreThan2D = 1;
block.SetPreCompInpPortInfoToDynamic;
block.InputPort(1).DimensionsMode    = 'Variable';
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

function SetInputDimsMode(block, port, dm)
% Set dimension mode
block.InputPort(port).DimensionsMode = dm;
%endfunction

function SetInputPortDims(block, idx, di)
block.InputPort(idx).Dimensions = di; % Set compiled dimensions
%endfunction

function DoPostPropSetup(block)
% Set the type of signal size to be dependent on input values, i.e.,
% dimensions have to be updated at output
block.SignalSizesComputeType = 'FromInputValueAndSize';
%endfunction

function Start(block)
set(gcf, 'Name', 'Corner Detection Viewer', 'NumberTitle', 'off');
whitebg('k'); % Backgroud color is black
view(37.5, 30); % Set default view
%endfunction

function Output(block)
cla; % Clear current axis and objects
for i = 1:size(block.InputPort(1).Data,1)
    if block.InputPort(1).Data(i,3) ~= 0 % Depth is valid
        line(block.InputPort(1).Data(i,1), block.InputPort(1).Data(i,3), block.InputPort(1).Data(i,2), ...
            'LineStyle', 'none', 'Marker', 'o', 'MarkerFaceColor', 'g', 'MarkerSize', 4);
    end
end
set(gca, 'XLim', [-3.2 3.2], 'YLim', [0 10.0], 'ZLim', [-2.4 2.4]); % Set X/Y/Z limits in meters
set(gca, 'XGrid', 'on', 'YGrid', 'on', 'ZGrid', 'on'); % Draw X/Y/Z grid

%endfunction
