##################################### Manual de utiliza��o do robot_smv ############################################

O robot_smv � uma aplica��o MAE adaptada para ser utilizada no SMV. Ele utiliza o modelo para vis�o 3D proposto por 
Hallysson Oliveira para realizar a verg�ncia e reconstruir superf�cies 3D.


####################################### Pr�-requisitos da instala��o ###############################################

Ambiente de compila��o Dev-Cpp instalado e atualizado na m�quina alvo, FLEX, BISON.


######################################### Intru��es de compila��o ##################################################

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


############################################ Instru��es de uso #####################################################


############################################ Formato de imagem #####################################################

A seguir, s�o esclarecidos alguns itens que dizem respeito ao formato de imagem utilizado pela MAE:

1) A nova vers�o do GIMP (2.2) gera os arquivos PNM com uma outra extens�o, PPM;

2) N�o utilizar formato de imagem PNM P6 (cr�), s� utilizar o PNM P3 (codifica��o ascii), caso contr�rio a
MAE n�o conseguir� carregar a imagem completa (provavelmente, no formato cr�, o GIMP insere no meio do arquivo
o caractere EOF);


################################### Descri��o dos par�metros de compila��o #########################################

    Par�metro   Unidade	  Descri��o
01) Ncx		[sel]     N�mero de elementos sensores na horizontal da matriz CCD da c�mera digital
02) Nfx		[pix]     N�mero de pixels na horizontal do quadro coletor
03) dx		[mm/sel]  Largura do elemento sensor da c�mera
04) dy		[mm/sel]  Altura do elemento sensor da c�mera
05) dpx		[mm/pix]  Lagura efetiva do pixel no quadro coletor
06) dpy		[mm/pix]  Altura efetiva do pixel no quadro coletor
07) Cx		[pix]     Abscissa do ponto principal
08) Cy		[pix]     Ordenada do ponto principal
09) sx		[]        Fator de escala para compensar qualquer erro em dpx
10) f		[mm]      Dist�ncia principal (foco)
11) kappa1	[1/mm^2]  Coeficiente de distor��o
12) Tx		[mm]	  Transla��o ao longo de X
13) Ty		[mm]	  Transla��o ao longo de Y
14) Tz		[mm]	  Transla��o ao longo de Z 
15) Rx		[rad]	  Rota��o em torno de X
16) Ry		[rad]	  Rota��o em torno de Y
17) Rz		[rad]     Rota��o em torno de Z
18) p1		[1/mm]
19) p2		[1/mm]
