clc;
clear
path = './';
colorCode = 'green'
numMaxLabels = 42 + 1;

perc = 99;

fn1 = sprintf('Lista_%s.mat', colorCode);    
load(fn1);
rng('shuffle')
for iFile = 1:1377
    fn1 = sprintf('%s_%04d.mat', colorCode, iFile);
    fn2 = ['./' colorCode '/' fn1];
    load(fn2);
    
    Linha = {};
    oldClasses=0;
    tic
    for label = 1:numMaxLabels
        %tic
        X = zeros(ListaCode(label,4), size(vgram.memory{1}.pattern_memory,2));
        for cont = ListaCode(label,2):ListaCode(label,3)          
            X(cont - ListaCode(label,2) + 1, :) = uint8(vgram.memory{cont}.pattern_memory)-uint8(48);
        end
        
        numTotal = size(X,1);
        classes = 1;%floor(numTotal * perc/100.0);
        
        %[iFile label-1 classes]

        [la, energy, index] = kmedoids(X',classes);

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
        %toc
    end
    [iFile size(Linha,2)]
    toc
    nvgram.memory = Linha;
    nvgram.memory_used = size(nvgram.memory,2);
    fn1 = sprintf('%s_%04d_med%03d.mat', colorCode, iFile, perc);
    fn2 = ['./' colorCode '/' fn1];
    save(fn2, 'nvgram');
    clear vgram;
    clear nvgram;
end

