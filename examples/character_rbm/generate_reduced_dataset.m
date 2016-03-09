clear;

%Number of train and test samples, must be a multiple of 10
new_number_of_train_examples = 500
new_number_of_test_examples = 100

new_number_of_train_examples_div10 = new_number_of_train_examples/10; 
new_number_of_test_examples_div10 = new_number_of_test_examples/10;

%Open the file descriptors
train_images_in = fopen('/dados/MNIST/train-images-idx3-ubyte','r');
train_labels_in = fopen('/dados/MNIST/train-labels-idx1-ubyte','r');
test_images_in = fopen('/dados/MNIST/train-images-idx3-ubyte','r');
test_labels_in = fopen('/dados/MNIST/train-labels-idx1-ubyte','r');

train_images_out = fopen('/dados/MNIST/train-images-idx3-ubyte-small','w');
train_labels_out = fopen('/dados/MNIST/train-labels-idx1-ubyte-small','w');
test_images_out = fopen('/dados/MNIST/t10k-images-idx3-ubyte-small','w');
test_labels_out = fopen('/dados/MNIST/t10k-labels-idx1-ubyte-small','w');

%Loads the train image parameters
train_images_in_MN = fread(train_images_in,1,'*uint32','b');
train_images_in_number_of_images = fread(train_images_in,1,'*uint32','b');
train_images_in_number_of_rows = fread(train_images_in,1,'*uint32','b');
train_images_in_number_of_columns = fread(train_images_in,1,'*uint32','b');
train_images_in_data = fread(train_images_in,'*uint8','b');

%Loads train label parameters
train_labels_in_MN = fread(train_labels_in,1,'*uint32','b');
train_labels_in_number_of_items = fread(train_labels_in,1,'*uint32','b');
train_labels_in_data = fread(train_labels_in,'*uint8','b');

%Loads the test image parameters
test_images_in_MN = fread(test_images_in,1,'*uint32','b');
test_images_in_number_of_images = fread(test_images_in,1,'*uint32','b');
test_images_in_number_of_rows = fread(test_images_in,1,'*uint32','b');
test_images_in_number_of_columns = fread(test_images_in,1,'*uint32','b');
test_images_in_data = fread(test_images_in,'*uint8','b');

%Loads test label parameters
test_labels_in_MN = fread(test_labels_in,1,'*uint32','b');
test_labels_in_number_of_items = fread(test_labels_in,1,'*uint32','b');
test_labels_in_data = fread(test_labels_in,'*uint8','b');

%Test for consistent datasets
if(train_images_in_number_of_images ~= train_labels_in_number_of_items)
	sprintf('Inconsistent source training data set\n')
end

if(test_images_in_number_of_images ~= test_labels_in_number_of_items)
	sprintf('Inconsistent source testing data set\n')
end

%Generates the new training dataset

image_size = train_images_in_number_of_rows*train_images_in_number_of_columns;	%Calculates the image size

train_images_out_number_of_images =	new_number_of_train_examples;		%Set the number of images in the reduced set
train_labels_out_number_of_items = 	new_number_of_train_examples;		%Set the number of labels in the reduced set

train_images_out_data = zeros(image_size*train_images_out_number_of_images,1,'uint8');	%initialize images data
train_labels_out_data = zeros(train_images_out_number_of_images,1,'uint8');		%initialize labels data

training_set_generated_samples = 0;						%No image has been generated yet
img_counter= zeros(1,10);							%Initialize the image counter for its labels
used_indexes(1) = 0;								%Initialize the used indexes vector

while(training_set_generated_samples < train_images_out_number_of_images)	%While not all images have been generated yet in the reduced set

	A = randi(train_images_in_number_of_images,1,1);			%Pick a random index from the number of images 
	
	if(~ismember(A,used_indexes))						%If this random index isn't a member of the used_indexes

		B = train_labels_in_data(A);					%View which label this random index represents

		if(img_counter(B+1) < new_number_of_train_examples_div10)	%If the number of counted samples for this label isn't the max
			
			img_counter(B+1) = img_counter(B+1) + 1;		%Increase the counter for this label
			
			train_images_out_data(training_set_generated_samples*image_size +1 :(training_set_generated_samples+1)*image_size + 1) = train_images_in_data((A-1)*image_size:A*image_size);				%Copy the image memory from the full to the reduced data set

			training_set_generated_samples = training_set_generated_samples + 1;	%Increases the training set counter

			used_indexes(training_set_generated_samples) = A;	%Add the index to the used indexes list
			
			train_labels_out_data(training_set_generated_samples) = B;	%Copy the desired label 					
		end
	end	
end

%Writes reduced training set data to an output file
fwrite(train_images_out,train_images_in_MN,'*uint32');
fwrite(train_images_out,train_images_out_number_of_images,'*uint32');
fwrite(train_images_out,train_images_in_number_of_rows,'*uint32');
fwrite(train_images_out,train_images_in_number_of_columns,'*uint32');
fwrite(train_images_out,train_images_out_data,'*uint8');

fwrite(train_labels_out,train_labels_in_MN,'*uint32');
fwrite(train_labels_out,train_labels_out_number_of_items,'*uint32');
fwrite(train_labels_out,train_labels_out_data,'*uint8');

%Generates the new testing dataset

image_size = test_images_in_number_of_rows*test_images_in_number_of_columns;	%Calculates the image size

test_images_out_number_of_images =	new_number_of_test_examples;		%Set the number of images in the reduced set
test_labels_out_number_of_items = 	new_number_of_test_examples;		%Set the number of labels in the reduced set

test_images_out_data = zeros(image_size*test_images_out_number_of_images,1,'uint8');	%initialize images data
test_labels_out_data = zeros(test_images_out_number_of_images,1,'uint8');		%initialize labels data

test_set_generated_samples = 0;							%No image has been generated yet
img_counter= zeros(1,10);							%Initialize the image counter for its labels
%used_indexes(1) = 0;								%Initialize the used indexes vector
%The line above was commented out to pick images for test without intersection.
%It was assumed that both training and test images point to the same database.

while(test_set_generated_samples < test_images_out_number_of_images)	%While not all images have been generated yet in the reduced set

	A = randi(test_images_in_number_of_images,1,1);				%Pick a random index from the number of images 
	
	if(~ismember(A,used_indexes))						%If this random index isn't a member of the used_indexes

		B = test_labels_in_data(A);					%View which label this random index represents

		if(img_counter(B+1) < new_number_of_test_examples_div10)	%If the number of counted samples for this label isn't the max
			
			img_counter(B+1) = img_counter(B+1) + 1;		%Increase the counter for this label
			
			test_images_out_data(test_set_generated_samples*image_size +1 :(test_set_generated_samples+1)*image_size + 1) = test_images_in_data((A-1)*image_size:A*image_size);				%Copy the image memory from the full to the reduced data set

			test_set_generated_samples = test_set_generated_samples + 1;	%Increases the training set counter

			%used_indexes(test_set_generated_samples) = A;	%Add the index to the used indexes list
			used_indexes(training_set_generated_samples+test_set_generated_samples) = A;	%Add the index to the used indexes list
            
			test_labels_out_data(test_set_generated_samples) = B;	%Copy the desired label 					
		end
	end	
end

%Writes reduced test set data to an output file
fwrite(test_images_out,test_images_in_MN,'*uint32');
fwrite(test_images_out,test_images_out_number_of_images,'*uint32');
fwrite(test_images_out,test_images_in_number_of_rows,'*uint32');
fwrite(test_images_out,test_images_in_number_of_columns,'*uint32');
fwrite(test_images_out,test_images_out_data,'*uint8');

fwrite(test_labels_out,test_labels_in_MN,'*uint32');
fwrite(test_labels_out,test_labels_out_number_of_items,'*uint32');
fwrite(test_labels_out,test_labels_out_data,'*uint8');


%Close the file descriptors
fclose(train_images_in);
fclose(train_labels_in);
fclose(test_images_in);
fclose(test_labels_in);

fclose(train_images_out);
fclose(train_labels_out);
fclose(test_images_out);
fclose(test_labels_out);

