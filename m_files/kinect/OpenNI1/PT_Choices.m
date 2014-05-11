%Gives options for each exercies available
TotalChoices = {'RightShoulderExternal','RightShoulderInternal',...
    'LeftShoulderExternal','LeftShoulderInternal','RightShoulderExtension'...
    'RightShoulderFlexion','LeftShoulderExtension','LeftShoulderFlexion',...
    'RightElbowFlexion','RightElbowExtension','LeftElbowExtension',...
    'LeftElbowFlexion','RightLegSquat','LeftLegSquat'};

%Preallocation for user to store username and password
user = cell(2,1);

%Waits for strcmp to be true for the program to continue
while ~strcmp(user{1},'PT')|| ~strcmp(user{2},'abcd')
    user = PT_Prompt;
end

%Choice between view old patient and start new patient
choice = questdlg('What would you like to do?', ...
    'OptionMenu', ...
    'Start New','Review Old','Cancel','Start New');

%Switches for choice
switch choice
    case 'Start New'
        
        %Prompts PT to enter name, age, and gender
        prompt = {'Enter Patient First Name:','Enter Patient Last Name:'...
            ,'Age:','Gender:'};
        dlg_title = 'Patient Info';
        num_lines = 1;
        Info = inputdlg(prompt,dlg_title,num_lines);
        
        %Delte Exercises.mat to get rid of previous Patient data
        delete('Exercises.mat')
        
        %Passes Choices, savefile, and spacing. Saves the exercises that
        %are inputted.
        PhysicalTherapist(TotalChoices, 'Exercises.mat',25)
        %Loads the chosen exercises
        load('Exercises.mat')
        %For loop to input angles and length of regimen for each exercise
        dlg_title = 'Goals';
        for n = 1:length(checked)
            prompt = {strcat('Start Angle_', Choices{checked(n)},':'),'End Angle:','Length of Regimen:'};
            data{:,n} = inputdlg(prompt,dlg_title,num_lines);
            z = msgbox(data{n});
            uiwait(z)
        end
        %Finds the length of regimen for each exercises 
        for t = 1:length(data)
            Z(t) = str2double(cell2mat(data{t}(3)));
        end
        
        %Generates blank dataTable to store all achieved angles
        dataTable = zeros(length(data),max(Z));
        
        %Initialize goalTable and store linear goal data
        goalTable = dataTable;
        for y = 1:length(data)
            Data = data{y};
            start_angle = str2double(Data{1});
            end_angle = str2double(Data{2});
            lengthofRegimen(y) = str2double(Data{3});
            goalTable(y,1:lengthofRegimen(y)) = linspace(start_angle, end_angle, lengthofRegimen(y));
        end
        %Saves the goal angles, blank dataTable, and goalTable
        save('Angles.mat','data','dataTable','goalTable')
        
    case 'Review Old'
        %Load the exercises chosen
        load('Exercises.mat')
        
        %Inquires PT which exercises should be displayed in the graph
        PhysicalTherapist(Choices(checked),'PTExerciseChoice.mat',15)
        
        %Loads patientdata and the goal tables
        load('PTExerciseChoice.mat')
        load('Angles.mat')
        
        %Clears the blank dataTable in Angles.mat
        clearvars dataTable
        load('PatientData.mat')
        
        %Plots the data for viewing
        figure(1)
        hold on
        for n = 1:length(checked)
            plot(1:size(goalTable,2),goalTable(checked(n),:),'--',1:size(goalTable,2),dataTable(n,:),'r')
            xlabel('Day Number')
            ylabel('Angle in Degrees')
            legend('GoalAngles','ActualAngles')
        end
        hold off
        
    case 'Cancel'
end

