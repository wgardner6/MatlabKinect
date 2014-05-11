clear all, close all, clc
%Add the mexed files to the path
addpath('Mex')
%Finds the xml path
SAMPLE_XML_PATH='Config/SamplesConfig.xml';
%Names of all the gifs to display as the patient goes through the exercises
gifNames = {'RightShoulderExternal.gif','L_Shoulder_Internal.gif','LeftShoulderExternal.gif',...
    'L_Shoulder_Internal.gif','RightShoulderFlexion.gif',...
    'RightShoulderFlexion.gif','LeftShoulderFlexion.gif','LeftShoulderFlexion.gif',...
    'RightElbowFlex.gif','RightElbowFlex.gif','LeftElbowFlex.gif',...
    'LeftElbowFlex.gif','R_L_Knee_Squat.gif','R_L_Knee_Squat.gif'};

% Start the Kinect Process
% To use the Kinect hardware use :
KinectHandles=mxNiCreateContext(SAMPLE_XML_PATH);

%Initializes figure 
figure(1)
subplot(1,2,2)

%Finds the skeleton
Pos= mxNiSkeleton(KinectHandles);

%Finds the image of the patient
I=mxNiPhoto(KinectHandles); I=permute(I,[3 2 1]);

%Shows the image
h=imshow(I);

%Tries to calibrate the patient
while(Pos(1)==0);
    mxNiUpdateContext(KinectHandles);
    I=mxNiPhoto(KinectHandles); I=permute(I,[3 2 1]);
    Pos = mxNiSkeleton(KinectHandles);
    set(h,'Cdata',I); drawnow;
end

%Play iPhone Tri-Tone
filename = ('Classic iPhone Tri-Tone Alert Sound.mp3');
[y,Fs] = audioread(filename);
sound(y,Fs)

%Load the chosen exercises and blank angle tables
hh=zeros(1,9);
load('Exercises.mat');
load('Angles.mat');

%Determine which day it is
dayNumber = floor(now-c)+1;

for n = 1:length(checked)
    
    aquiringAngleCounter=1;
    %If dayNumer is equal to the length of the regimen for the specific
    %exercise or if the dayNumber is equal to the day the PT inputted the
    %exercieses then the for loop is skipped
    if dayNumber> str2double(cell2mat(data{n}(3))) || dayNumber==0
        continue
    end
    %Sets upper and lower tolerances of +-5 degrees of the goalAngle for
    %that day
    upp_tol = goalTable(n,dayNumber)+5;
    low_tol = goalTable(n,dayNumber)-5;
    
    %Determines which choice the current iteration of the for loop is
    choice = checked(n);
    
    %Sets the prev_angle to inf to initialize it
    prev_angle = inf;
    
    %Initializes frameCounter and counter to zero
    patientMovingFrameCounter = 0;
    currentFrame = 0;
    
    %Initialization of while loop
    while(Pos(1)>0&&aquiringAngleCounter<50)
        
        %Plays gif every 40 frames. This gets rid of issue of gif playing
        %every iteration through.
        subplot(1,2,1)
        if(mod(currentFrame,40)==0)
            gifplayer(gifNames{choice})
        end
        
        %Update skeleton and image
        mxNiUpdateContext(KinectHandles);
        I=mxNiPhoto(KinectHandles); I=permute(I,[3 2 1]);
        set(h,'Cdata',I); drawnow;
        
        %Update skeleton points
        Pos= mxNiSkeleton(KinectHandles,1);
        %Deletes where the skeleton was plotted previously
        if(hh(1)>0);
            for i=1:9, delete(hh(i)); end
        end
        
        hold on
        %X and Y points of skeleton
        yp=Pos(1:15,7);
        xp=Pos(1:15,6);
        %Had to adjust upper part of skeleton to get it to display
        %properly
        yp([1:9,10,13]) = yp([1:9,10,13])+25;
        %     X = mxNiDepthRealWorld(KinectHandles)
        %x,y,z real world points
        x = Pos(1:15,3);
        y = Pos(1:15,4);
        z = Pos(1:15,5);
        
        %Joint mapping on Ubuntu is slightly different than windows because
        %several of the joints are lost in the C++ files we used.
        [A,B,C] = JointMappingUbuntu(choice);
        
        %If choice is less than 9, meaning it is an exercise that needs an
        %imaginary point to do the calculations then the point is created.
        %This point needs to be created on exercises such as external
        %rotation where the point needs to be created in front of the elbow
        %so the angle can be measured with respect to the horizontal
        if choice<9
            x(A) = x(B);
            y(A) = y(B);
            z(A) = z(B)-100;
        end
        %Law of cosines to get angle of interest
        distA = sqrt((x(A)-x(B))^2 + (y(A)-y(B))^2 + (z(A)-z(B))^2);
        distB = sqrt((x(B)-x(C))^2 + (y(B)-y(C))^2 + (z(B)-z(C))^2);
        distC = sqrt((x(A)-x(C))^2 + (y(A)-y(C))^2 + (z(A)-z(C))^2);
        angleC = acos((distB^2 + distA^2 - distC^2)/(2*distB*distA));
        %Convert from rads to degrees
        angleC = angleC.*(180/pi);
        %If the difference between the current angle and the angle in the
        %previous frame is less than one
        if (abs(angleC-prev_angle)<=1)
            %If the patient has been previously moving for more than 10
            %frames
            if (patientMovingFrameCounter > 10)
                %Save data into angle 
                angle(aquiringAngleCounter) = angleC;
                %Increment aquiringAngleCounter to only collect as many as
                %dictated by while loop above
                aquiringAngleCounter = aquiringAngleCounter+1;
            end
        else
            %If difference between previous angle and current angle is
            %greater than 5 it means the patient is moving
            if (abs(angleC-prev_angle) > 5)
                %Increment patientMovingFrameCounter
                patientMovingFrameCounter = patientMovingFrameCounter + 1;
            end
            %Reset aquiringAngleCounter to 1 each time the difference
            %between the two angles is not less than one
            aquiringAngleCounter = 1;
        end
        %Set previous angle to angle C
        prev_angle = angleC;
        %Display current data to command line
        [distA, distB, distC, angleC, aquiringAngleCounter, patientMovingFrameCounter]
        %Plot skeleton data
        subplot(1,2,2)
        hold on
        hh(1)=plot(xp,yp,'r.');
        hh(2)=plot(xp([13 14 15]),yp([13 14 15]),'g');
        hh(3)=plot(xp([10 11 12]),yp([10 11 12]),'y');
        hh(4)=plot(xp([9 10]),yp([9 10]),'m');
        hh(5)=plot(xp([9 13]),yp([9 13]),'w');
        hh(6)=plot(xp([2 3 4 5]),yp([2 3 4 5]),'b');
        hh(7)=plot(xp([2 6 7 8]),yp([2 6 7 8]),'r');
        hh(8)=plot(xp([1 2]),yp([1 2]),'c');
        hh(9)=plot(xp([2 9]),yp([2 9]),'k');
        drawnow
        %Increment currentFrame
        currentFrame = currentFrame+1;
        
    end
    %Store mean of angle in current day slot of dataTable
    dataTable(n,dayNumber) = mean(angle);
    %Determine if PatientData has been written or not
    if ~exist('PatientData.mat','file')
        save('PatientData.mat','dataTable')
    else
        save('PatientData.mat','dataTable')
    end
    
    %Prompt the user to determine if they are ready for the next exercise
    M = msgbox('Are you ready for the next exercise?');
    uiwait(M)
end
%Cleans up the Kinect interface
mxNiDeleteContext(KinectHandles);