Compilando e Executando o Projeto de Exemplo "Robot SMV #.0"
------------------------------------------------------------

O projeto de exemplo "Robot SMV 4.0" foi modificado para executar parte do seu processamento na plataforma MATLAB. Para compilar e rodar o projeto. Na versao 13 os dois primeiros passos abaixo sao executados automaticamente. Passe ao passo 3.

1- Dentro do ambiente MATLAB, entre no diretorio robot_smv_#.0/robot_user_functions/matlab_scripts/ e execute o script makelibstereo.m;

2- No ambiente shell, entre no diretorio robot_smv_#.0/robot_user_functions/matlab_scripts/ e copie os seguintes arquivos para robot_smv_#.0/:

* libstereo.dll [apenas no Windows]
* libstereo.so  [apenas no Linux]
* libstereo.ctf

Alem disso, copie o arquivo "stereo_api.h" para robot_smv_#.0/robot_user_functions/;

3- No ambiente shell, entre no diretorio robot_smv_#.0/ e execute o make (apenas make):

*DEPRECATED: No windows: make -f Makefile.win

*DEPRECATED: No Linux:   make -f Makefile.unix

4- Rode o projeto de teste digitando "robot" no ambiente shell.

Para utilizar o SMV 13.0
------------------------

Obs: os indices [i] contam a partir do zero. Portanto, o primeiro par de cameras tem indice [i] = 0, o segundo par [i] = 1 e assim por diante.

1 - Colocar os arquivos de calibracao no diretorio da aplicacao. Os arquivos devem ser renomeados segundo o formato:

"Calib_Results_rectification_indexes.mat" do par de cameras [i]: camera_pair_[i]_rectification_indexes.mat
"Calib_Results_stereo_rectified.mat"      do par de cameras [i]: camera_pair_[i]_rectification_stereo.mat

2 - Colocar as imagens a serem exibidas no diretorio da aplicacao. Os arquivos devem ser renomeados segundo o formato:

Imagem esquerda da camera [i]: <image.basename>_half0_side[i]_left.ppm 
Imagem direita  da camera [i]: <image.basename>_half0_side[i]_right.ppm

* para este exemplo o parametro image.basename=s4n1680 (ver arquivo robot_smv_#.0/config.txt)

3 - Inicializar a aplicacao. Para carregar as imagens de um par de cameras, digite o indice correspondente (ex: "0").

4 - Para gerar a reconstrucao do lado atual, digite "v".

* para maiores detalhes consulte o arquivo comandos.txt neste mesmo diretório.
