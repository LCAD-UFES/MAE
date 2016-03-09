clc;
clear
path = './';
colorCode = 'grey'
numMaxLabels = 10;

for iFile = 1:1
    fn2 = sprintf('./data/%s_%04d.mat', colorCode, iFile);
    load(fn2); 
    for label = 1:numMaxLabels
        tic
        ListaCode = [];
        for cont = 1:size(vgram.memory,2)           
            if(vgram.memory{cont}.associated_label+1 == label)                
                ListaCode = [ListaCode; cont];
            end
        end
        toc
        size(ListaCode)
        ListaCode(1)
        fn1 = sprintf('Lista_%02d.mat', label);    
        save(fn1, 'ListaCode');
    end
end

