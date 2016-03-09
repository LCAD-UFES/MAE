function [KK_left, KK_right, R, T, om, fc_left, cc_left, kc_left, alpha_c_left, fc_right, cc_right, kc_right, alpha_c_right] = LoadStereoParameters (FileName)
load(FileName);
KK_left = KK_left_new;
KK_right = KK_right_new;
R = R_new;
T = T_new;
om = om_new;
fc_left = fc_left_new;
cc_left = cc_left_new;
kc_left = kc_left_new;
alpha_c_left = alpha_c_left_new;
fc_right = fc_right_new;
cc_right = cc_right_new;
kc_right = kc_right_new;
alpha_c_right = alpha_c_right_new;
