function [XL, XR] = GetWorldPointAtDistance (xR, KK_left, KK_right, R, T, D)

P_left = KK_left * [eye(3,3) zeros(3,1)];
P_right = KK_right * [R T];

xR = [xR; 1];
XL = pinv([P_right(:,1) P_right(:,2)]) * (xR - P_right(:,3) - P_right(:,4)./D);
XL = XL .* D;
XL(3) = D;

XR = R * XL + T;
