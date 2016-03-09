clc;
clear
path = './';
colorCode = 'grey'
numMaxLabels = 10;

perc = 99;

fn1 = sprintf('Lista_%s.mat', colorCode);    
load(fn1);

for iFile = 51:1024
    fn1 = sprintf('./data/%s_%04d.mat', colorCode, iFile);
    load(fn1);
    
    Linha = {};
    oldClasses=0;
    tic
    for label = 1:numMaxLabels
        
        fn1 = sprintf('Lista_%02d.mat', label);    
        load(fn1);
    
        lista = [vgram.memory{ListaCode}];
              
        nvgram.layer_id = vgram.layer_id;
        nvgram.neuron_id = vgram.neuron_id;
        nvgram.memory_used = vgram.memory_used;
    
        for memSample=1:size(lista,2)
            linha.pattern_flag = lista(memSample).pattern_flag;
            linha.pattern_memory = lista(memSample).pattern_memory;
            linha.associated_label =  lista(memSample).associated_label;
       
            Linha{oldClasses+memSample} = linha;
        end
        oldClasses = size(Linha,2);
    end
    [iFile size(Linha,2)]
    toc
    nvgram.memory = Linha;
    nvgram.memory_used = size(nvgram.memory,2);
    fn1 = sprintf('./data/%s_%04d_h%03d.mat', colorCode, iFile, perc);
    save(fn1, 'nvgram');
    clear vgram;
    clear nvgram;
end

