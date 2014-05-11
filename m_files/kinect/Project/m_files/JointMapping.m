function [A, B, C] = JointMapping(Choice)
%Joint Mapping
rightShoulderExternal = 1; %Upward
rightShoulderInternal = 2; %Downward
leftShoulderExternal = 3;
leftShoulderInternal = 4;
rightShoulderExtension = 5; %Downward
rightShoulderFlexion = 6; %Upward
leftShoulderExtension = 7; %Downward
leftShoulderFlexion = 8; %Upward
rightElbowFlexion = 9;
rightElbowExtension = 10;
leftElbowExtension = 11;
leftElbowFlexion = 12;
rightKneeExtension = 13;
rightKneeFlexion = 14;
leftKneeExtension = 15;
leftKneeFlexion = 16;
switch Choice
    case rightShoulderExternal
        A = 10;
        B = 10;
        C = 11;
    case rightShoulderInternal
        A = 10;
        B = 10;
        C = 11;
    case leftShoulderExternal
        A = 6;
        B = 6;
        C = 7;
    case leftShoulderInternal
        A = 6;
        B = 6;
        C = 7;
    case rightShoulderExtension
        A = 5;
        B = 9;
        C = 10;
    case rightShoulderFlexion
        A = 5;
        B = 9;
        C = 10;
    case leftShoulderExtension
        A = 9;
        B = 5;
        C = 6;
    case leftShoulderFlexion
        A = 9;
        B = 5;
        C = 6;
    case rightElbowFlexion
        A = 9;
        B = 10;
        C = 11;
    case rightElbowExtension
        A = 9;
        B = 10;
        C = 11;
    case leftElbowExtension
        A = 5;
        B = 6;
        C = 7;
    case leftElbowFlexion
        A = 5;
        B = 6;
        C = 7;
    case rightKneeExtension
        A = 13;
        B = 14;
        C = 15;
    case rightKneeFlexion
        A = 13;
        B = 14;
        C = 15;
    case leftKneeExtension
        A = 17;
        B = 18;
        C = 19;
    case leftKneeFlexion
        A = 17;
        B = 18;
        C = 19;
end
