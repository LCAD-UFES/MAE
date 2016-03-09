Compilando e Executando o Projeto de Exemplo "Robot SMV #.0"
------------------------------------------------------------

O projeto de exemplo "Robot SMV 4.0" foi modificado para executar parte do seu processamento na plataforma MATLAB. Para compilar e rodar o projeto, siga estes passos:

1- Dentro do ambiente MATLAB, entre no diretório robot_smv_#.0/robot_user_functions/matlab_scripts/ e execute o script makelibstereo.m;

2- No ambiente shell, entre no diretório robot_smv_#.0/robot_user_functions/matlab_scripts/ e copie os seguintes arquivos para robot_smv_#.0/:

* libstereo.dll [apenas no Windows]
* libstereo.so  [apenas no Linux]
* libstereo.ctf

Além disso, copie o arquivo "stereo_api.h" para robot_smv_#.0/robot_user_functions/;

3- No ambiente shell, entre no diretório robot_smv_#.0/ e execute o make:

* No windows: make -f Makefile.win

* No Linux:   make -f Makefile.unix

4- Rode o projeto de teste digitando "robot" no ambiente shell.
