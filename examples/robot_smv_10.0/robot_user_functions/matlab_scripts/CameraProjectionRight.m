function [xL, xR] = CameraProjectionRight (XR, KK_left, KK_right, R, T)

N = size(XR,2);

%XR = R * XL + T;
XL =  R \ (XR - repmat(T, [1 N]));

XL = [XL;ones(1,N)];
P_left = KK_left * [eye(3,3) zeros(3,1)];
P_right = KK_right * [R T];

xL_aux = P_left * XL;
xR_aux = P_right * XL;

xL = [xL_aux(1,:) ./  xL_aux(3,:); xL_aux(2,:) ./  xL_aux(3,:)];
xR = [xR_aux(1,:) ./  xR_aux(3,:); xR_aux(2,:) ./  xR_aux(3,:)];
