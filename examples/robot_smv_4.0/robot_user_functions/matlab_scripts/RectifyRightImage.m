function [rect_img_right] = RectifyRightImage (img_right, ind_new_right, ind_1_right, ind_2_right, ind_3_right, ind_4_right, a1_right, a2_right, a3_right, a4_right)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% img_right = imread ('~stiven/calib_img/img_right0.jpg');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% right image:
[ny, nx, colors_channels] = size(img_right);
R = double(img_right(:,:,1));
G = double(img_right(:,:,2));
B = double(img_right(:,:,3));
R2 = 255*ones(ny,nx);
G2 = 255*ones(ny,nx);
B2 = 255*ones(ny,nx);
R2(ind_new_right) = a1_right .* R(ind_1_right) + a2_right .* R(ind_2_right) + a3_right .* R(ind_3_right) + a4_right .* R(ind_4_right);
G2(ind_new_right) = a1_right .* G(ind_1_right) + a2_right .* G(ind_2_right) + a3_right .* G(ind_3_right) + a4_right .* G(ind_4_right);
B2(ind_new_right) = a1_right .* B(ind_1_right) + a2_right .* B(ind_2_right) + a3_right .* B(ind_3_right) + a4_right .* B(ind_4_right);
rect_img_right(:,:,1) = uint8(R2);
rect_img_right(:,:,2) = uint8(G2);
rect_img_right(:,:,3) = uint8(B2);

%imwrite (rect_img_right,'rect_img_right.jpg','JPEG');