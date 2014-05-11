function PhysicalTherapist
Choices = {'rightShoulderExternal','rightShoulderInternal',...
    'leftShoulderExternal','leftShoulderInternal','rightShoulderExtension'...
    'rightShoulderFlexion','leftShoulderExtension','leftShoulderFlexion',...
    'rightElbowFlexion','rightElbowExtension','leftElbowExtension',...
    'leftElbowFlexion','rightKneeExtension','rightKneeFlexion',...
    'leftKneeExtension','leftKneeFlexion'};
h.f = figure('units','pixels','position',[200,200,200,30*length(Choices)],...
    'toolbar','none','menu','none');
for n=1:length(Choices)
    h.c(n) = uicontrol('style','checkbox','units','pixels',...
        'position',[15,25*n+20,180,15],'string',[Choices{n}, num2str(n)]);
end
% Create OK pushbutton
h.p = uicontrol('style','pushbutton','units','pixels',...
    'position',[50,5,70,20],'string','OK',...
    'callback',@p_call);

function p_call(varargin)
        vals = get(h.c,'Value');
        checked = find([vals{:}]);
        if isempty(checked)
            checked = 'none';
        end
        save('Exercises.mat','Choices','checked')
        disp(checked)
        close(1)
end 
% Pushbutton callback
end