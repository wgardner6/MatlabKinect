function checkIMAQBlock(model, blk)
% checkIMAQBlock
%
% This function is invoked block initFcn callback of IMAQ blocks.

%
%   Copyright 2011 The MathWorks, Inc.
%

% Check the number of instance of NID IMAQ block (it has to be single instance).
checkBlockSingleton(model, 'NID IMAQ'); % If not, it generates error 

% Check at least, one device block is placed in the model.
depthBlk    = find_system(model, 'LookUnderMasks', 'all', 'BlockType', 'S-Function', 'MaskType', 'NID Depth');
motionBlk   = find_system(model, 'LookUnderMasks', 'all', 'BlockType', 'S-Function', 'MaskType', 'NID Motion');
imageBlk    = find_system(model, 'LookUnderMasks', 'all', 'BlockType', 'S-Function', 'MaskType', 'NID Image');
skeletonBlk = find_system(model, 'LookUnderMasks', 'all', 'BlockType', 'S-Function', 'MaskType', 'NID Skeleton');
irBlk       = find_system(model, 'LookUnderMasks', 'all', 'BlockType', 'S-Function', 'MaskType', 'NID IR');

if isempty(depthBlk) && isempty(motionBlk) && isempty(imageBlk) && isempty(skeletonBlk) && isempty(irBlk)
    error('None of Depth/Motion/Image/Skeleton/IR blocks is placed in the model.');
end

% If IR block is used, check other device block is used except for Depth block.
if (~isempty(motionBlk) || ~isempty(imageBlk) || ~isempty(skeletonBlk)) && ~isempty(irBlk)
    error('IR block and Image/Motion/Skeleton blocks can not be used together.');
end

% Set IMAQ block parameters of NID IMAQ block in the model.
if isempty(depthBlk)
    set_param(blk, 'imaq_depth', 'off');
else
    set_param(blk, 'imaq_depth', 'on');
end

if isempty(motionBlk)
    set_param(blk, 'imaq_motion', 'off');
else
    set_param(blk, 'imaq_motion', 'on');
end

if isempty(imageBlk)
    set_param(blk, 'imaq_image', 'off');
else
    set_param(blk, 'imaq_image', 'on');
end
    
if isempty(skeletonBlk)
    set_param(blk, 'imaq_skeleton', 'off');
else
    set_param(blk, 'imaq_skeleton', 'on');
end

if isempty(irBlk)
    set_param(blk, 'imaq_ir', 'off');
else
    set_param(blk, 'imaq_ir', 'on');
end


% End of function
