clear all, close all
%Asks user who is logging in. Giving options of patient and physical
%therapist
choice = questdlg('Who is logging in?', ...
    'PersonSelection', ...
    'Physical Therapist','Patient','Cancel','Patient');

%Switches based on choice
switch choice
    case 'Physical Therapist'
        PT_Choices;
    case 'Patient'
        kinect;
end