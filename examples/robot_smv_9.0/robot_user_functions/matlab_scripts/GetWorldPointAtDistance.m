function [XL, XR] = GetWorldPointAtDistance (xR, KK_left, KK_right, R, T, D)

xR = [xR; 1];
P_left = KK_left * [eye(3,3) zeros(3,1)];
P_right = KK_right * [R T];


%xR = P_left * XL;
%xR = P_left(:,1)*XL(1)+P_left(:,2)*XL(2)+P_left(:,3)*XL(3)+P_left(:,4)*XL(4);
%xR = P_left(:,1)*XL(1)+P_left(:,2)*XL(2)+P_left(:,3)*D+P_left(:,4)*1;
%xR = [P_left(:,1) P_left(:,2)]*[XL(1) XL(2)]+P_left(:,3)*D+P_left(:,4)*1;

%XL = ([P_left(:,1) P_left(:,2)]) \ (xR - P_left(:,3)*D - P_left(:,4)*1);
%XL(3) = D;
%XR = ([P_right(:,1) P_right(:,2)]) \ (xR - P_right(:,3)*D - P_right(:,4)*1);
%XR(3) = D;

XL = pinv([P_left(:,1) P_left(:,2)]) * (xR - P_left(:,3) - P_left(:,4)./D);
XL = XL .* D;
XL(3) = D;
XR = pinv([P_right(:,1) P_right(:,2)]) * (xR - P_right(:,3) - P_right(:,4)./D);
XR = XR .* D;
XR(3) = D;