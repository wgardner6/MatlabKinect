function kinect
%Try to call patient and determine if the USB has been released by Linux.
%If not then continue to call Patient.m until it has been released. Not a
%very well written recursion, but it does what we need.
try
    Patient
catch err
    disp(err)
    Patient
end
end