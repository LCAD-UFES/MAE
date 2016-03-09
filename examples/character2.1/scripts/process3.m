%Salvar tudo
clc;
clear
path = './';
colorCode = 'grey'

perc = 1;

pMem = [path 'netmem_010.ascii'];
fid=fopen(pMem, 'w'); 

for i = 1:1024
    tic
    fn1 = sprintf('./data/%s_%04d_h%03d.mat', colorCode, i, perc);
    i
    load(fn1);
    vgram = nvgram;
    
    fprintf(fid, '%d\n', vgram.layer_id);
    fprintf(fid, '%d\n', vgram.neuron_id);
    fprintf(fid, '%d\n', vgram.memory_used);
    
    for memSample=1:vgram.memory_used
            fprintf(fid, '%d\n', vgram.memory{memSample}.pattern_flag);
            fprintf(fid, '%s\n', vgram.memory{memSample}.pattern_memory);
            fprintf(fid, '%d\n', vgram.memory{memSample}.associated_label);
    end
    clear vgram;
    toc
end

fclose(fid);
