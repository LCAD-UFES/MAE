clc;
clear; 

numBits = 128;

path = '../MAE/';
numNeuron = 32*32; 
pMem = [path 'netmem.ascii'];  

fid=fopen(pMem); 
for neuron = 1:numNeuron
tic
    vgram.layer_id = fscanf(fid, '%d\n', 1);
    vgram.neuron_id = fscanf(fid, '%d\n', 1);
    vgram.memory_used = fscanf(fid, '%d\n', 1);
    neuron
    Linha = [];
    for memSample=1:vgram.memory_used
       linha.pattern_flag = fscanf(fid, '%d\n', 1);
       linha.pattern_memory = fscanf(fid, '%c\n', numBits);
       linha.associated_label = fscanf(fid, '%d\n', 1);
       
       Linha{memSample} = linha;
    end
    
    fn = sprintf('%04d', neuron);
    filename = ['./data/grey_' fn '.mat']
    
    vgram.memory = Linha;
    save( filename, 'vgram')
    clear vgram;
toc
end 
 
