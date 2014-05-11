function hasIMAQBlock(model)
% hasIMAQBlock
%  check the model has a IMAQ block.  

%
%   Copyright 2011 The MathWorks, Inc.
%

blks = find_system(model, 'LookUnderMasks', 'all', 'BlockType', 'S-Function', 'MaskType', 'NID IMAQ');

if length(blks) ~= 1
    error('A NID IMAQ block should be placed in the model.');
end

end
% End of function
