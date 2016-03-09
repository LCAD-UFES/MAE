%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% TransformPoints - 
% Inputs:
%	PIN - Input Points
%	E - Euler Angles
%	D - Displacements
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [POUT] = TransformPoints (PIN, E, D, MC)

N = size(PIN,2);

% Calculates the baricenter
% MC=sum(PIN,2)./N;

% Put the input points in homogeneous coordinates
P1=[PIN;ones(1,N)];

% Put the origin at the mass center
T1=[1 0 0 -MC(1); 0 1 0 -MC(2); 0 0 1 -MC(3); 0 0 0 1];
P2=T1*P1;

% Calculate the rotation matrix from the Euler angles
[R]=rodrigues(E);
R=[R zeros(3,1);zeros(1,3) 1];

% Rotate the points around the new origin
P3=R*P2;

% Back to the old origin
T2=[1 0 0 MC(1); 0 1 0 MC(2); 0 0 1 MC(3); 0 0 0 1];
P4=T2*P3;

% Translate the points
T3=[1 0 0 D(1); 0 1 0 D(2); 0 0 1 D(3); 0 0 0 1];
P5=T3*P4;

POUT=[P5(1,:)./P5(4,:);P5(2,:)./P5(4,:);P5(3,:)./P5(4,:)];

