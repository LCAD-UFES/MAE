function [XL, XR] = GetWorldPointAtDistance (xR, KK_left, KK_right, R, T, D)

XR = [XR; 1];
P_left = KK_left * [R T];
P_right = KK_right * [eye(3,3) zeros(3,1)];

xL = P_left * XR;
xR = P_right * XR;

xL = xL ./ xL(3); 
xR = xR ./ xR(3); 