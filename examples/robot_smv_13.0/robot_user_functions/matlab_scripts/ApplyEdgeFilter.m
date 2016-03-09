function [edge_img] = ApplyEdgeFilter (img)

[ny, nx, colors_channels] = size(img);

%imwrite (img,'img.jpg','JPEG');

gaussianFilter = [1,4,7,4,1;4,20,33,20,4;7,33,55,33,7;4,20,33,20,4;1,4,7,4,1]
gaussianFilter = gaussianFilter / sum(sum(gaussianFilter));
edgeFilter = [-1,-1,-1;-1,8,-1;-1,-1,-1]

gaussianImg = imfilter(img, gaussianFilter);
%imwrite (gaussianImg,'img_gaussian.jpg','JPEG');
edgeImg = imfilter(gaussianImg, edgeFilter);
%imwrite (edgeImg,'img_edge.jpg','JPEG');

edge_img = gaussianImg + edgeImg;

%imwrite (edge_img,'img_edge_final.jpg','JPEG');
