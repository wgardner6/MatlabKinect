clear all, close all, clc
flag = 0;
Choices = {'rightShoulderExternal','rightShoulderInternal',...
    'leftShoulderExternal','leftShoulderInternal','rightShoulderExtension'...
    'rightShoulderFlexion','leftShoulderExtension','leftShoulderFlexion',...
    'rightElbowFlexion','rightElbowExtension','leftElbowExtension',...
    'leftElbowFlexion','rightKneeExtension','rightKneeFlexion',...
    'leftKneeExtension','leftKneeFlexion'};
checked = PhysicalTherapist(Choices)
while(~flag)
end
 load('Exercises.mat')
Exercises = {Choices{checked}}