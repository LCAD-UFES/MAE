% this script resizes MNIST database
function mnistresize(no_training_samples_per_digit, no_test_samples_per_digit)

if nargin < 2
    no_test_samples_per_digit = -1;
end

if nargin < 1
    no_training_samples_per_digit = -1;
end

fprintf('Resizing test dataset for first %d rows\n', no_test_samples_per_digit);
for d=0:9,
  load(['test' num2str(d) '.mat'],'-mat');
  if ( (no_test_samples_per_digit > 0) && (no_test_samples_per_digit < size(D,1)) )
    D = D(1:no_test_samples_per_digit,:);
  end
  fprintf('%5d Digits of class %d from test dataset.\n',size(D,1),d);
  save(['test' num2str(d) '.mat'],'D','-mat');
end;

fprintf('Resizing training dataset for first %d rows\n', no_training_samples_per_digit);
for d=0:9,
  load(['digit' num2str(d) '.mat'],'-mat');
  if ( (no_training_samples_per_digit > 0) && (no_test_samples_per_digit < size(D,1)) )
    D = D(1:no_training_samples_per_digit,:);
  end
  fprintf('%5d Digits of class %d from training dataset.\n',size(D,1),d);
  save(['digit' num2str(d) '.mat'],'D','-mat');
end;

