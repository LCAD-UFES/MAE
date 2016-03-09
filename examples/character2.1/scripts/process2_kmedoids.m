clc;
clear
path = './';
colorCode = 'grey'
numMaxLabels = 10;

perc = 10;

fn1 = sprintf('Lista_%s.mat', colorCode);    
load(fn1);

for iFile = 1:1024
    fn1 = sprintf('./data/%s_%04d.mat', colorCode, iFile);
    load(fn1);
    
    Linha = {};
    oldClasses=0;
    tic
    for label = 1:numMaxLabels
        
        fn1 = sprintf('Lista_%02d.mat', label);    
        load(fn1);
    
        X = zeros(size(ListaCode,1), size(vgram.memory{1}.pattern_memory,2));
        for cont = 1:size(ListaCode,1)          
            X(cont, :) = uint8(vgram.memory{ListaCode(cont)}.pattern_memory)-uint8(48);
        end
        
        numTotal = size(X,1);
        classes = floor(numTotal * perc/100.0);
        
        %[iFile label-1 classes]

	[la, energy, index] = kmedoids(X',classes);

        %opts = statset('MaxIter', 100);
        %[l, c] = kmeans(X, classes, 'distance', 'Hamming', 'emptyaction', 'singleton', 'onlinephase', 'on', 'Options', opts);
        
        nvgram.layer_id = vgram.layer_id;
        nvgram.neuron_id = vgram.neuron_id;
        nvgram.memory_used = vgram.memory_used;
    
        for memSample=1:classes
            linha.pattern_flag = 1;
            XB = '';
            for i = 1:size(vgram.memory{1}.pattern_memory,2)
                if (X(index(memSample),i) > 0)
                    XB = [XB '1'];
                else
                    XB = [XB '0'];
                end 
            end
            linha.pattern_memory = XB;
            linha.associated_label = label-1;
       
            Linha{oldClasses+memSample} = linha;
        end
        oldClasses = size(Linha,2);
    end
    [iFile size(Linha,2)]
    toc
    nvgram.memory = Linha;
    nvgram.memory_used = size(nvgram.memory,2);
    fn1 = sprintf('./data/%s_%04d_m%03d.mat', colorCode, iFile, perc);
    save(fn1, 'nvgram');
    clear vgram;
    clear nvgram;
end

