[ind_new_left, ind_1_left, ind_2_left, ind_3_left, ind_4_left, a1_left, a2_left, a3_left, a4_left, ind_new_right, ind_1_right, ind_2_right, ind_3_right, ind_4_right, a1_right, a2_right, a3_right, a4_right] = LoadRectificationIndexes ('Calib_Results_rectification_indexes.mat');
img_left = imread ('img_left0.jpg');
img_right = imread ('img_right0.jpg');
[rect_img_left, rect_img_right] = RectifyStereoPair (img_left, img_right, ind_new_left, ind_1_left, ind_2_left, ind_3_left, ind_4_left, a1_left, a2_left, a3_left, a4_left, ind_new_right, ind_1_right, ind_2_right, ind_3_right, ind_4_right, a1_right, a2_right, a3_right, a4_right);
figure(1);
imshow(rect_img_left);
figure(2);
imshow(rect_img_right);
