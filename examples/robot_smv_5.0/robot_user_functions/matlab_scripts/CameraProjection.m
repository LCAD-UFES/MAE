function [xL, xR] = CameraProjection (XR, KK_left, KK_right, R, T)

XR = [XR; 1];
P_left = KK_left * [eye(3,3) zeros(3,1)];
P_right = KK_right * [R T];

xL_aux = P_left * XR;
xR_aux = P_right * XR;

xL(1,1) = xL_aux(1) / xL_aux(3);
xL(2,1) = xL_aux(2) / xL_aux(3); 
xR(1,1) = xR_aux(1) / xR_aux(3);
xR(2,1) = xR_aux(2) / xR_aux(3);