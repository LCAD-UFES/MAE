function [xL, xR] = CameraProjection (XL, KK_left, KK_right, R, T)

XL = [XL;ones(1,size(XL,2))];
P_left = KK_left * [eye(3,3) zeros(3,1)];
P_right = KK_right * [R T];

xL_aux = P_left * XL;
xR_aux = P_right * XL;

%xL(1,1) = xL_aux(1) / xL_aux(3);
%xL(2,1) = xL_aux(2) / xL_aux(3); 
%xR(1,1) = xR_aux(1) / xR_aux(3);
%xR(2,1) = xR_aux(2) / xR_aux(3);
xL = [xL_aux(1,:) ./  xL_aux(3,:); xL_aux(2,:) ./  xL_aux(3,:)];
xR = [xR_aux(1,:) ./  xR_aux(3,:); xR_aux(2,:) ./  xR_aux(3,:)];
