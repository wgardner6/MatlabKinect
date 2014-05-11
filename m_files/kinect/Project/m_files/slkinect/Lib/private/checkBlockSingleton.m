function checkBlockSingleton(model, maskType)
% checkBlockSingleton
%  Check number of instances of a NID block. All NID blocks should be single instance (singleton) in a model.  

%
%   Copyright 2011 The MathWorks, Inc.
%

blks = find_system(model, 'LookUnderMasks', 'all', 'BlockType', 'S-Function', 'MaskType', maskType);

if length(blks) > 1
    msg = [maskType ' block has to be only single instance in a model.'];
    error(msg);
end

end

% End of function
