function [rect_img_left] = RectifyLeftImage (img_left, ind_new_left, ind_1_left, ind_2_left, ind_3_left, ind_4_left, a1_left, a2_left, a3_left, a4_left)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% img_left = imread ('~stiven/calib_img/img_left0.jpg');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%img_left = imread('img_left0.jpg');

% left image:
[ny, nx, colors_channels] = size(img_left);
R = double(img_left(:,:,1));
G = double(img_left(:,:,2));
B = double(img_left(:,:,3));
R2 = 255*ones(ny,nx);
G2 = 255*ones(ny,nx);
B2 = 255*ones(ny,nx);
R2(ind_new_left) = a1_left .* R(ind_1_left) + a2_left .* R(ind_2_left) + a3_left .* R(ind_3_left) + a4_left .* R(ind_4_left);
G2(ind_new_left) = a1_left .* G(ind_1_left) + a2_left .* G(ind_2_left) + a3_left .* G(ind_3_left) + a4_left .* G(ind_4_left);
B2(ind_new_left) = a1_left .* B(ind_1_left) + a2_left .* B(ind_2_left) + a3_left .* B(ind_3_left) + a4_left .* B(ind_4_left);
rect_img_left(:,:,1) = uint8(R2);
rect_img_left(:,:,2) = uint8(G2);
rect_img_left(:,:,3) = uint8(B2);

%imwrite (rect_img_left,'rect_img_left.jpg','JPEG');