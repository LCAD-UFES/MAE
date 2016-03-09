clear all;
%{
%% load images for trainning
fid = fopen('random_faces_t_10.txt', 'r');
fileindex = fscanf(fid, '%d %c %d %d', [4 inf])' ;
fclose(fid);

[rows cols] = size(fileindex);

train_x = [];
train_y = [];
for i = 1:rows
    filename = strcat('/dados/ARPhotoDataBase/small/', fileindex(i,2), '-', num2str(fileindex(i,3), '%03d'), '-', strtrim(num2str(fileindex(i,4))), '.ppm' );
    image = imread(filename);
    
    %tratar como RGB da MAE
    gray = rgb2gray(image);
    [irows icols] = size(gray);
    
    vector = reshape(gray', irows*icols, 1);
    
    label = zeros(10,1);
    label(fileindex(i,3)) = 1;

    train_x = [train_x vector];
    train_y = [train_y label];
end

%% load images for test
%fid = fopen('faces_te_all_side_lights.txt', 'r');
fid = fopen('faces_te_anger_10.txt', 'r');
%fid = fopen('faces_te_glasses.txt', 'r');
%fid = fopen('faces_te_left_light.txt', 'r');
%fid = fopen('faces_te_right_light.txt', 'r');
%fid = fopen('faces_te_scarf.txt', 'r');
%fid = fopen('faces_te_scream.txt', 'r');
%fid = fopen('faces_te_smile.txt', 'r');
%fid = fopen('faces_tr.txt', 'r');
%fid = fopen('faces_tv.txt', 'r');
%fid = fopen('faces_va.txt', 'r');
fileindex = fscanf(fid, '%d %c %d %d', [4 inf])' ;
fclose(fid);

[rows cols] = size(fileindex);

test_x = [];
test_y = [];
for i = 1:rows
    filename = strcat('/dados/ARPhotoDataBase/small/', fileindex(i,2), '-', num2str(fileindex(i,3), '%03d'), '-', strtrim(num2str(fileindex(i,4))), '.ppm' );
    image = imread(filename);
    
    gray = rgb2gray(image);
    [irows icols] = size(gray);
    
    vector = reshape(gray', irows*icols, 1);
    
    label = zeros(10,1);
    label(fileindex(i,3)) = 1;
    
    test_x = [test_x vector];
    test_y = [test_y label];
end

%%  conversions
train_x = double(train_x') / 255;
test_x  = double(test_x')  / 255;
train_y = double(train_y');
test_y  = double(test_y');

save ar_face_database train_x train_y test_x test_y;
%}

load ar_face_database;

train_x = train_x(1:10,:);
test_x = test_x(1:10,:);
train_y = train_y(1:10,1:10);
test_y = test_y(1:10,1:10);

%DEBUG images
%imshow(reshape(train_x(1,:), irows, icols));

%%  ex1 train a 100 hidden unit RBM and visualize its weights
rand('state',0)
dbn.sizes = [100];
opts.numepochs = 10;
opts.batchsize = 10;
opts.momentum  =   0;
opts.alpha     =   1;
dbn = dbnsetup(dbn, train_x, opts);
dbn = dbntrain(dbn, train_x, opts);
figure; visualize(dbn.rbm{1}.W');   %  Visualize the RBM weights

%%  ex2 train a 100-100 hidden unit DBN and use its weights to initialize a NN
rand('state',0)
%train dbn
dbn.sizes = [100 100];
opts.numepochs = 10;
opts.batchsize = 10;
opts.momentum  =   0;
opts.alpha     =   1;
dbn = dbnsetup(dbn, train_x, opts);
dbn = dbntrain(dbn, train_x, opts);

%unfold dbn to nn
nn = dbnunfoldtonn(dbn, 10);
nn.activation_function = 'sigm';

%train nn
opts.numepochs = 10;
opts.batchsize = 10;
nn = nntrain(nn, train_x, train_y, opts);
[er, bad] = nntest(nn, test_x, test_y);

assert(er < 0.10, 'Too big error');
