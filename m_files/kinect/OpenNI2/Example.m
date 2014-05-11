addpath('Mex')

% Start the Kinect Process
KinectHandles=mxNiCreateContext();

figure;
I=mxNiPhoto(KinectHandles); I=permute(I,[3 2 1]);
D=mxNiDepth(KinectHandles); D=permute(D,[2 1]);
subplot(1,2,1),h1=imshow(I); 
subplot(1,2,2),h2=imshow(D,[0 9000]); colormap('jet');
    
for i=1:90
    I=mxNiPhoto(KinectHandles); I=permute(I,[3 2 1]);
    D=mxNiDepth(KinectHandles); D=permute(D,[2 1]);
    set(h1,'CDATA',I);
    set(h2,'CDATA',D);
    drawnow; 
end

% Stop the Kinect Process
mxNiDeleteContext(KinectHandles);
