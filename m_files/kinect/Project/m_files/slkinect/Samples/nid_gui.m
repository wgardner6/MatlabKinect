function varargout = nid_gui(varargin)
% NID_GUI MATLAB code for nid_gui.fig
%      NID_GUI, by itself, creates a new NID_GUI or raises the existing
%      singleton*.
%
%      H = NID_GUI returns the handle to a new NID_GUI or the handle to
%      the existing singleton*.
%
%      NID_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in NID_GUI.M with the given input arguments.
%
%      NID_GUI('Property','Value',...) creates a new NID_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before nid_gui_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to nid_gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help nid_gui

% Last Modified by GUIDE v2.5 02-Oct-2012 14:43:54

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @nid_gui_OpeningFcn, ...
    'gui_OutputFcn',  @nid_gui_OutputFcn, ...
    'gui_LayoutFcn',  [] , ...
    'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end

% End initialization code - DO NOT EDIT


% --- Executes just before nid_gui is made visible.
function nid_gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to nid_gui (see VARARGIN)

% Choose default command line output for nid_gui
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);


global Tag_ax1 rt_out;
Tag_ax1 = findall(0,'Tag','axes1');
rt_out = findall(0,'Tag','rt_out');



% UIWAIT makes nid_gui wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = nid_gui_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in pushbutton1.
function pushbutton1_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
load_system('nid_sl')
%set_param('nid_gui/NID Depth', 'UserData', handles.figure1)
set_param('nid_sl','SimulationCommand','start')
%set(handles.axes1,235.7340)

% global c;
% h = findall(0,'Tag','axes1');
% c = h;




% --- Executes on button press in pushbutton2.
function pushbutton2_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set_param('nid_sl','SimulationCommand','stop')




function Image_out(block, eventdata)
%  hFig  = get_param(block.BlockHandle,'UserData');
%   tAxis = findobj(hFig, 'Type','axes');
%hF  = get_param(block.BlockHandle,'UserData');
%rto = get_param('nid_sl/NID Depth','RuntimeObject');
%frame = rto.OutputPort(1).Data;
%   global c;
global Tag_ax1 pop_menu rt_out;

rt = get_param('nid_sl/Sum','RuntimeObject');
rt_val = rt.OutputPort(1).Data;
set(rt_out,'String',rt_val)
rto = get_param('nid_sl/NID Image','RuntimeObject');

if pop_menu == 0
    rto = get_param('nid_sl/NID Image','RuntimeObject');
elseif pop_menu == 1
    rto = get_param('nid_sl/NID Depth','RuntimeObject');
end

frame = rto.OutputPort(1).Data;
imshow(frame,'Parent',Tag_ax1);


% --- Executes on selection change in popupmenu1.
function popupmenu1_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global pop_menu;

str = get(hObject, 'String');
val = get(hObject,'Value');
% Set current data to the selected data set.
switch str{val};
    case 'RGB' % User selects peaks.
        pop_menu = 0;
    case 'Depth' % User selects membrane.
        pop_menu = 1;
end
% Save the handles structure.
guidata(hObject,handles)


% --- Executes during object creation, after setting all properties.
function popupmenu1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes during object creation, after setting all properties.
function uipanel1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to uipanel1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called



function rt_out_Callback(hObject, eventdata, handles)
% hObject    handle to rt_out (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of rt_out as text
%        str2double(get(hObject,'String')) returns contents of rt_out as a double


% --- Executes during object creation, after setting all properties.
function rt_out_CreateFcn(hObject, eventdata, handles)
% hObject    handle to rt_out (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
