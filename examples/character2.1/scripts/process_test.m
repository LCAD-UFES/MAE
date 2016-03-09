clc;
clear; 
load('./data/grey_0001_h099.mat')
load('./data/grey_0001.mat')
  
for i = 1: size(vgram.memory,2)
    cont = 0;
    for j = 1: size(vgram.memory,2) 
        if((sum(uint8(vgram.memory{i}.pattern_memory)==uint8(nvgram.memory{j}.pattern_memory))==128) && (vgram.memory{i}.pattern_flag==nvgram.memory{j}.pattern_flag) & (vgram.memory{i}.associated_label==nvgram.memory{j}.associated_label))
            cont=cont+1;
            %j
        end
    end
    [i cont]
end
cont
% vgram.memory{59624}.pattern_memory
% vgram.memory{59952}.pattern_memory
% vgram.memory{22646}.pattern_memory 
% 
% vgram.memory{59624}.pattern_flag
% vgram.memory{59952}.pattern_flag
% vgram.memory{22646}.pattern_flag
% 
% vgram.memory{59624}.associated_label
% vgram.memory{59952}.associated_label
% vgram.memory{22646}.associated_label 

