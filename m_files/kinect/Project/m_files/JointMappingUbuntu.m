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
        A = 6;
        B = 7;
        C = 8;
    case rightShoulderInternal
        A = 6;
        B = 7;
        C = 8;
    case leftShoulderExternal
        A = 3;
        B = 4;
        C = 5;
    case leftShoulderInternal
        A = 3;
        B = 4;
        C = 5;
    case rightShoulderExtension
        A = 3;
        B = 6;
        C = 7;
    case rightShoulderFlexion
        A = 3;
        B = 6;
        C = 7;
    case leftShoulderExtension
        A = 6;
        B = 3;
        C = 4;
    case leftShoulderFlexion
        A = 6;
        B = 3;
        C = 4;
    case rightElbowFlexion
        A = 6;
        B = 7;
        C = 8;
    case rightElbowExtension
        A = 6;
        B = 7;
        C = 8;
    case leftElbowExtension
        A = 3;
        B = 4;
        C = 5;
    case leftElbowFlexion
        A = 3;
        B = 4;
        C = 5;
    case rightKneeExtension
        A = 10;
        B = 11;
        C = 12;
    case rightKneeFlexion
        A = 10;
        B = 11;
        C = 12;
    case leftKneeExtension
        A = 13;
        B = 14;
        C = 15;
    case leftKneeFlexion
        A = 13;
        B = 14;
        C = 15;
end
