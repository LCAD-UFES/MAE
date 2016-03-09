##################################### Manual de utilização do robot_smv ############################################

O robot_smv é uma aplicação MAE adaptada para ser utilizada no SMV. Ele utiliza o modelo para visão 3D proposto por 
Hallysson Oliveira para realizar a vergência e reconstruir superfícies 3D.


####################################### Pré-requisitos da instalação ###############################################

Ambiente de compilação Dev-Cpp instalado e atualizado na máquina alvo, FLEX, BISON.


######################################### Intruções de compilação ##################################################

No Linux
]# cd $MAE_HOME
]# make
]# cd $MAE_HOME/examples/robot_smv
]# make

No Windows
]# cd $MAE_HOME
]# make -f Makefile.win
]# cd $MAE_HOME\examples\robot_smv
]# make -f Makefile.win


############################################## Como executar #######################################################

No Linux
]# ./robot

No Windows
]# robot.exe


############################################ Instruções de uso #####################################################


############################################ Formato de imagem #####################################################

A seguir, são esclarecidos alguns itens que dizem respeito ao formato de imagem utilizado pela MAE:

1) A nova versão do GIMP (2.2) gera os arquivos PNM com uma outra extensão, PPM;

2) Não utilizar formato de imagem PNM P6 (crú), só utilizar o PNM P3 (codificação ascii), caso contrário a
MAE não conseguirá carregar a imagem completa (provavelmente, no formato crú, o GIMP insere no meio do arquivo
o caractere EOF);


################################### Descrição dos parâmetros de compilação #########################################

    Parâmetro   Unidade	  Descrição
01) Ncx		[sel]     Número de elementos sensores na horizontal da matriz CCD da câmera digital
02) Nfx		[pix]     Número de pixels na horizontal do quadro coletor
03) dx		[mm/sel]  Largura do elemento sensor da câmera
04) dy		[mm/sel]  Altura do elemento sensor da câmera
05) dpx		[mm/pix]  Lagura efetiva do pixel no quadro coletor
06) dpy		[mm/pix]  Altura efetiva do pixel no quadro coletor
07) Cx		[pix]     Abscissa do ponto principal
08) Cy		[pix]     Ordenada do ponto principal
09) sx		[]        Fator de escala para compensar qualquer erro em dpx
10) f		[mm]      Distância principal (foco)
11) kappa1	[1/mm^2]  Coeficiente de distorção
12) Tx		[mm]	  Translação ao longo de X
13) Ty		[mm]	  Translação ao longo de Y
14) Tz		[mm]	  Translação ao longo de Z 
15) Rx		[rad]	  Rotação em torno de X
16) Ry		[rad]	  Rotação em torno de Y
17) Rz		[rad]     Rotação em torno de Z
18) p1		[1/mm]
19) p2		[1/mm]
