%Generates a checkbox list to allow PT to input exercises
function PhysicalTherapist(Choices, Filename, Spacing)

%Starts the figure with appropriate initializing data

h.f = figure('units','pixels','position',[200,200,200,30*length(Choices)],...
    'toolbar','none','menu','none');

%Generates the options for the user to select exercises

for n=1:length(Choices)
    h.c(n) = uicontrol('style','checkbox','units','pixels',...
        'position',[15,Spacing*n+20,180,15],'string',[Choices{n}, num2str(n)]);
end

% Create OK pushbutton
h.p = uicontrol('style','pushbutton','units','pixels',...
    'position',[50,5,70,20],'string','OK',...
    'callback',@p_call);
uiwait(h.f)

%Callback function to save the data and the day that the PT inputted the
%choices. This gets rid of the problem of the patient running many trials
%on the same day

function p_call(varargin)
        vals = get(h.c,'Value');
        checked = find([vals{:}]);
        if isempty(checked)
            checked = 'none';
        end
        c = floor(now);
        save(Filename,'Choices','checked','c')
        close(gcf)
end 
end