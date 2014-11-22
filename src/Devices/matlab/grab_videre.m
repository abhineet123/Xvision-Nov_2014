% grab_videre grabs images from SRI's stereo head
% [ result, [result]] = grab_videre(new_image_flag, mode)
% new_image_flag=0 - use the previous image
% new_image_flag=1 - grab a new image for the result
%
% mode   0 - 2 grayscale images
%        1 - 1 color image from the left camera
%	 2 - 1 disparity image
%
% [ left,right ] =grab_videre(1,0);
% [ color ] = grab_videre(1,1);
% [ dispar ] = grab_videre(1,2);
