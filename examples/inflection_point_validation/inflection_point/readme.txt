Instruções para executar o preditor baseado em VG-RAM WNN de máximos e mínimos locais em séries de valores de ações

1) Marcar os pontos de máximo e mínimo. Para isso, execute o código no diretório 
MAE/examples/inflection_point_validation/inflection_point/:

% cd MAE/examples/inflection_point_validation/inflection_point/
% ./inflection_point

2) Treinar e testar o preditor. Para isso, execute o código no diretório  MAE/examples/inflection_point_validation/inflection_point/ 
com o arquivo cml para treino e teste:

% cd MAE/examples/inflection_point_validation/inflection_point/
% ./inflection_point inflection_point_validation.cml
