/** @file mainpage.h
* @brief Trabalho de Cogni��o Visual
*/ 

/** @mainpage Neural Computer
*
* @authors LCAD
*
* @section summary Sum�rio
* O principal objetivo do trabalho � implementar um computador neural (CN) capaz de executar programas especificados como sequ�ncias de padr�es de ativa��o de camadas neurais.
* Outro objetivo importante deste trabalho � avaliar o desempenho do CN no controle de um rob� aut�nomo, sendo este desempenho medido em termos da capacidade do CN de viabilizar
* a proceduraliza��o de atividades complexas do rob� aut�nomo (transforma��o de processos controlados em processos autom�ticos, pag. 112 do livro).
*
* @section architecture Arquitetural do Computador Neural
* A arquitetura b�sica do CN � apresentada na Figura 1, abaixo.
* \image html ./images/nc_arch_intro.png "Figura 1: Computador Neural (CN)"
*
* Como a Figura 1 mostra, o CN � composto por quatro partes fundamentais:
*
* (i) a entrada principal (Entrada);
*
* (ii) 3 Camadas Neurais;
*
* (iii) uma entrada para sensores (Sensores); e
*
* (iv) uma sa�da (Sa�da).
*
* Todas estas partes s�o recept�culos de imagens bidimensionais (Entrada e Camadas Neurais), trazem imagens bidimensionais (Sensores), ou disponibilizam imagens bidimensionais (Sa�da).
*
* As Camadas Neurais do CN s�o redes neurais sem peso do tipo VG-RAM bidimensionais com n x m neur�nios. Cada neur�nio destas Camadas Neurais possui um conjunto de sinapses que o interconectam com outras Camadas Neurais ou outros elementos da arquitetura do CN como mostrado pelas setas na Figura 1. O padr�o de interconex�o da arquitetura do CN forma uma estrutura hier�rquica de camadas neurais, que permite denomin�-las como mostrado na Figura 2.
* \image html ./images/nc_arch_detail.png "Figura 2: Arquitetura neural hier�rquica do CN"
*
* As denomina��es das Camadas Neurais mostrada na Figura 2 indicam o papel esperado de cada Camada. A Camada Neural Programa, no n�vel hier�rquico que ocupa, aprende programas demandados por Entrada e representados como uma sequ�ncia de fun��es. A Camada Neural Fun��o aprende fun��es, representadas como sequ�ncias de comandos. Por fim, a Camada Neural Comando aprende comandos, representados por sequ�ncias de a��es sobre a Sa�da. A entrada Sensores permite ao CN monitorar a execu��o dos comandos e, assim, a execu��o do programa como um todo.
*
* Estruturalmente as Camadas Neurais do CN n�o diferem - todas elas s�o capazes de memorizar (aprender) rela��es entre imagens de entrada e imagens de sa�da. Isto �, dado um conjunto de imagens de entrada e uma imagem de sa�da, uma Camada Neural pode ser treinada de modo a, quando um conjunto de imagens de entrada previamente aprendido for reapresentado (ou uma vers�o aproximada dele), disponibilizar a sa�da correspondente (aprendida). Contudo, do ponto de vista l�gico, elas diferem, pois, como discutido acima, as Camadas Neurais mais � esquerda ocupam um n�vel hier�rquico mais alto que as mais � direita.
*
* @section vg-ram Redes Neurais Sem Peso VG-RAM
*
* Redes Neurais Sem-Peso (RNSP) s�o redes neurais artificiais tamb�m conhecidas como redes neurais Booleanas ou n-uplas Classificadoras, ou seja, redes neurais artificiais cujos neur�nios guardam conhecimento em mem�rias do tipo RAM (Random Access Memories). Estes neur�nios artificiais operam em fun��o dos valores bin�rios que s�o apresentados em suas entradas e usam RAMs como tabelas-verdade: as sinapses de cada neur�nio coletam valores de um vetor de bits da entrada da RNSP, que s�o usados como um endere�o de mem�ria RAM, onde � armazenado o valor de sa�da do neur�nio. O treinamento de um padr�o em uma RNSP pode ser feito em um �nico passo e consiste basicamente em armazenar a sa�da desejada nos endere�os associados ao respectivo vetor de entrada apresentado aos neur�nios.
*
* Apesar da not�vel simplicidade, as RNSP s�o muito efetivas como ferramentas de reconhecimento de padr�es, oferecendo treinamento r�pido e de f�cil implementa��o. No entanto, se o tamanho da entrada da RNSP for muito grande, o tamanho das RAMs dos neur�nios torna proibitivo a sua implementa��o, j� que deve possuir um n�mero de entradas igual a 2 elevado � pot�ncia deste tamanho. As redes VG-RAM (Virtual Generalizing RAMs) s�o redes baseadas em RAMs que requerem somente capacidade de mem�ria suficiente para guardar dados relacionados ao conjunto de treinamento. Nos neur�nios deste tipo de rede, s�o armazenados os pares de entrada-sa�da apresentados durante a fase de treinamento, e n�o somente as sa�das. Na fase de teste, todas entradas dos pares entrada-sa�da aprendidos e armazenados nas mem�rias dos neur�nios VG-RAM s�o comparadas com a entrada da rede. A sa�da de cada neur�nio VG-RAM � determinada pela sa�da do par de entrada-sa�da (j� aprendido) mais "pr�ximo". A medida de proximidade adotada � a Dist�ncia de Hamming, i.e., o n�mero de bits diferentes entre dois vetores de bits de igual tamanho. No caso de mais de um par de entrada-sa�da apresentarem a mesma dist�ncia, a escolha � feita rand�micamente entre estes pares.
*
* A Figura 3 ilustra a tabela-verdade de um neur�nio VG-RAM com tr�s sinapses: X1, X2 e X3. Esta tabela-verdade cont�m tr�s posi��es, cada posi��o contendo um par entrada-sa�da armazenado durante a fase de treinamento: entradas #1, #2 e #3. Durante a fase de teste, quando um vetor de entrada � apresentado � RNSP, o algoritmo de teste VG-RAM calcula a dist�ncia entre este vetor de entrada e cada entrada j� armazenada na tabela-verdade. No exemplo da Figura 1, a dist�ncia de Hamming entre o vetor de entrada (input) e #1 � dois (2), porque ambos os bits X2 e X3 n�o apresentam pareamento/semelhan�a com o vetor de entrada. A dist�ncia de #2 � um (1) pois X1 � o �nico bit diferente. A dist�ncia da entrada #3 � tr�s (3) pois todos os bits s�o diferentes. Para este vetor de entrada, � produzido o resultado label2, que � o valor armazenado em #2, como a sa�da do neur�nio Y.
* \image html ./images/vgram_truth_table.png "Figura 3: Tabela-verdade de um neur�nio VG-RAM"
*
* RNSP podem ser usadas para o reconhecimento de imagens. Em problemas de reconhecimento de imagens baseados em aprendizado de m�quina, deseja-se saber qual padr�o deve ser atribu�do a uma imagem dado um conjunto de pares imagem-padr�o associado previamente aprendido pelo sistema. Para se reconhecer imagens automaticamente usando RNSP do tipo VG-RAM, uma Camada Neural de neur�nios VG-RAM pode ser utilizada. Cada um dos neur�nios desta Camada Neural possui um conjunto de sinapses que s�o conectadas de modo rand�mico � imagem de entrada e captura desta um padr�o de bits.
*
* Durante a fase de treinamento, cada imagem pertencente ao conjunto de pares imagem-padr�o de treinamento � apresentada � entrada da Camada Neural e as sa�das dos seus neur�nios s�o conectadas ao padr�o associado � imagem. Os neur�nios da Camada Neural s�o treinados de forma a externar este padr�o (uma imagem) dada a imagem de entrada. Ap�s o treinamento, a opera��o de uma Camada Neural ocorre em ciclos cont�nuos de tr�s fases executadas por cada um de seus neur�nios:
*
* (i) leitura das entradas;
*
* (ii) busca na mem�ria pelo padr�o de bits de entrada aprendido mais pr�ximo ao padr�o de bits colhido da imagem de entrada corrente; e
*
* (iii) disponibiliza��o na sa�da do pixel do padr�o associado � imagem mais pr�xima aprendida.
*
* � importante notar que, no NC (Figura 1), uma camada neural possui 3 imagens de entrada (uma imagem vinda da camada mais acima na hierarquia, uma imagem vinda da pr�pria camada neural, e uma imagem vinda da camada mais abaixo na hierarquia). No entanto, estas 3 imagens podem ser vistas como sendo uma s�, composta pela combina��o das 3.
*
* @section example Exemplo de Programa��o e Execu��o de um Programa no CN
* Para compreender como o CN funciona vamos examinar um exemplo de programa��o do mesmo para que ele aprenda a contar de 0 a 9 tr�s vezes. Isto �, vamos mostrar como ele pode ser usado para "proceduralizar" a atividade de contar de 0 a 9 tr�s vezes. Vamos chamar este programa de "Contar de 0 a 9 tr�s vezes" e implement�-lo de modo top-down. A Figura 4 mostra o primeiro passo (i) de especifica��o do programa.
* \image html ./images/nc_step_1.png "Figura 4: Passo (i) do programa Contar de 0 a 9 tr�s vezes"
*
* Como apresentado na Figura 4, no passo (i) do programa "Contar de 0 a 9 tr�s vezes", especificamos na Entrada o nome do programa a ser executado e indicamos qual deve ser a sa�da da Camada Neural Programa. Na verdade, especificamos na Entrada apenas uma imagem que, por conveni�ncia, pode ser interpretada por humanos como a mensagem "Contar de 0 a 9 tr�s vezes"; e indicamos � Camada Neural Programa apenas uma imagem que deve ser aprendida, que tamb�m por conveni�ncia pode ser lida por humanos como "Contar at� 10 1", ou seja, parte 1 de contar de 0 a 9 tr�s vezes.
*
* Especificados os itens que comp�em o passo (i) do programa, podemos ent�o treinar a Camada Neural Programa para gerar como sa�da a imagem "Contar at� 10 1" quando observar nas suas entradas 1, 2 e 3 as imagens "Contar de 0 a 9 tr�s vezes", vazio (todos os pixels iguais a 0) e vazio, respectivamente.
*
* No passo (ii) do programa "Contar de 0 a 9 tr�s vezes", mostrado na Figura 5, especificamos novamente na Entrada o nome do programa e indicamos qual deve ser a sa�da da Camada Neural Programa; al�m disso, especificamos o conte�do da Camada Neural Programa, "Contar at� 10 1". Especificados os itens que comp�em o passo (ii) do programa, podemos ent�o treinar a Camada Neural Programa para gerar como sa�da a imagem "Contar at� 10 1" quando observar nas suas entradas 1, 2 e 3 as imagens "Contar de 0 a 9 tr�s vezes", "Contar at� 10 1" e vazio, respectivamente. Ou seja, at� este ponto da programa��o do CN, a Camada Neural Programa foi treinada para gerar a sa�da "Contar at� 10 1" em duas circunst�ncias: uma quando suas entradas 1, 2 e 3 contiverem as imagens "Contar de 0 a 9 tr�s vezes", vazio e vazio, respectivamente; e outra quando suas entradas 1, 2 e 3 contiverem as imagens "Contar de 0 a 9 tr�s vezes", "Contar at� 10 1" e vazio, respectivamente.
* \image html ./images/nc_step_2.png "Figura 5: Passo (ii) do programa Contar de 0 a 9 tr�s vezes"
*
* Como mencionado na se��o anterior, ap�s o treinamento, a opera��o de uma Camada Neural do CN ocorre em ciclos cont�nuos de tr�s fases: leitura das entradas, busca na mem�ria pelo padr�o de entrada aprendido mais pr�ximo ao padr�o de entrada corrente, e disponibiliza��o na sa�da dos pixels associados a este padr�o de entrada aprendido. Com o que programamos at� aqui, se o CN for inicializado com suas camadas neurais vazias e com a Entrada "Contar de 0 a 9 tr�s vezes", ap�s o primeiro ciclo teremos na Camada Neural Programa a imagem "Contar at� 10 1". Em ciclos posteriores, dado o que foi programado at� este ponto no CN, a Camada Programa n�o se alteraria.
*
* No passo (iii) de nosso programa, indicamos qual deve ser a sa�da da Camada Neural Fun��o - "Depois de vazio" - quando esta receber na entrada 1 a imagem "Contar at� 10 1", e nas entradas 2 e 3 a imagem vazio (ver Figura 6).
* \image html ./images/nc_step_3.png "Figura 6: Passo (iii) do programa Contar de 0 a 9 tr�s vezes"
*
* Especificado o item (iii) do programa, podemos ent�o treinar a Camada Neural Fun��o para gerar como sa�da a imagem "Depois de vazio" quando observar nas suas entradas 1, 2 e 3 as imagens "Contar at� 10 1", vazio e vazio, respectivamente.
*
* No passo (iv) de nosso programa, treinamos a Camada Neural Fun��o para gerar a sa�da "Depois de vazio" quando observar nas suas entradas 1, 2 e 3 as imagens "Contar at� 10 1", "Depois de vazio" e vazio, respectivamente. Treinamos tamb�m a Camada Neural Programa novamente para gerar como sa�da a imagem "Contar at� 10 1" quando observar, agora, nas suas entradas 1, 2 e 3, as imagens "Contar de 0 a 9 tr�s vezes", "Contar at� 10 1" e "Depois de vazio", respectivamente.
* \image html ./images/nc_step_4.png "Figura 7: Passo (iv) do programa Contar de 0 a 9 tr�s vezes"
*
* No passo (v) de nosso programa, Figura 8, treinamos a Camada Neural Comando para gerar a sa�da "0" quando observar nas suas entradas 1, 2 e 3 as imagens "Depois de vazio", vazio e vazio, respectivamente (a entrada "Sensores" n�o � utilizada neste programa).
* \image html ./images/nc_step_5.png "Figura 8: Passo (v) do programa Contar de 0 a 9 tr�s vezes"
*
* No passo (vi) de nosso programa, Figura 9, treinamos a Camada Neural Fun��o para gerar a sa�da "Depois de 0" quando observar nas suas entradas 1, 2 e 3 as imagens "Contar at� 10 1", "Depois de vazio" e "0", respectivamente.
* \image html ./images/nc_step_6.png "Figura 9: Passo (vi) do programa Contar de 0 a 9 tr�s vezes"
*
* No passo (vii) do nosso programa, Figura 10, treinamos a Camada Neural Programa novamente para gerar como sa�da a imagem "Contar at� 10 1" quando observar, agora, nas suas entradas 1, 2 e 3, as imagens "Contar de 0 a 9 tr�s vezes", "Contar at� 10 1" e "Depois de 0", respectivamente.
* \image html ./images/nc_step_7.png "Figura 10: Passo (vii) do programa Contar de 0 a 9 tr�s vezes"
*
* No passo (viii) de nosso programa, Figura 11, treinamos a Camada Neural Comando para gerar a sa�da "1" quando observar nas suas entradas 1, 2 e 3 as imagens "Depois de 0", "0" e vazio, respectivamente.
* \image html ./images/nc_step_8.png "Figura 11: Passo (viii) do programa Contar de 0 a 9 tr�s vezes"
*
* No passo (ix), Figura 12, treinamos a Camada Neural Fun��o para gerar a sa�da "Depois de 1" quando observar nas suas entradas 1, 2 e 3 as imagens "Contar at� 10 1", "Depois de 0" e "1", respectivamente.
* \image html ./images/nc_step_9.png "Figura 12: Passo (ix) do programa Contar de 0 a 9 tr�s vezes"
*
* No passo (x) do nosso programa, Figura 13, treinamos a Camada Neural Programa novamente para gerar como sa�da a imagem "Contar at� 10 1" quando observar, agora, nas suas entradas 1, 2 e 3, as imagens "Contar de 0 a 9 tr�s vezes", "Contar at� 10 1" e "Depois de 1", respectivamente.
* \image html ./images/nc_step_10.png "Figura 13: Passo (x) do programa Contar de 0 a 9 tr�s vezes"
*
* Repetimos os passos (viii) - (x) para os n�meros de 2 a 9 e, na situa��o mostrada na Figura 14, programamos o avan�o da Camada Neural Programa para "Contar at� 10 2", e o recome�o das Camadas Neurais Fun��o e Comando para uma nova contagem de 0 a 9. Procedimentos de treinamento equivalentes aos apresentados podem ser usados para avan�ar para "Contar at� 10 3" e para finalizar o programa colocando uma imagem como "Fim de Programa" na Camada Neural Programa e vazio (quadro preto) nas demais.
*
* Para iniciar a execu��o do programa basta colocar a imagem vazio (quadro preto) nas 3 Camadas Neurais e a imagem "Contar de 0 a 9 tr�s vezes" na Entrada.
* \image html ./images/nc_step_11.png "Figura 14: Passo (xi) do programa Contar de 0 a 9 tr�s vezes"
*
* @section attention Aten��o Seletiva
* Dependendo da capacidade de generaliza��o da arquitetura da RNSP VG-RAM empregada na implementa��o da Camada Neural Programa, com o avan�o de sua imagem de "Contar at� 10 1" para "Contar at� 10 2", pode ser necess�rio retreinar a Camada Fun��o para gerar corretamente cada uma das imagens ("Depois de 0" a "Depois de 9") necess�rias para cada situa��o poss�vel da Camada Comando ("0" a "9"). Uma forma de evitar o risco da necessidade deste retreinamento � empregar na arquitetura do NC um mecanismo equivalente � Aten��o Seletiva (pag. 135 do livro).
*
* O mecanismo de aten��o seletiva poderia ser implementado por meio da viabiliza��o da sa�da das Camadas Neurais em duas modalidades (duas cores combinadas, por exemplo): uma com a fun��o j� descrita, isto �, uma imagem usada como informa��o de entrada de outras Camadas Neurais do NC; outra com a fun��o de servir como m�scara, que atuaria como filtro da imagem de entrada da pr�pria Camada Neural ou de outras Camadas Neurais do NC. A segunda modalidade de sa�da de uma Camada Neural seria aprendida junto com primeira.
*
* A Figura 15 ilustra o uso da m�scara mencionada. Na figura, a m�scara aprendida junto com cada padr�o de sa�da aprendido pela Camada Fun��o controla um filtro que permite apenas aos neur�nios da Camada Neural Fun��o visualizar a (ou atender �) parte "Contar at� 10" da imagem "Contar at� 10 1".
* \image html ./images/nc_mask_usage_1.png "Figura 15: Mascara para viabilizar o mecanismo de Aten��o Seletiva"
*
* Um filtro de aten��o empregado na entrada da Camada Neural Comando como mostrado na Figura 16 pode tamb�m simplificar o treinamento desta Camada Neural. Na verdade, existem in�meras maneiras de empregar o mecanismo de aten��o para simplificar o treinamento do NC.
* \image html ./images/nc_mask_usage_2.png "Figura 16: Outro exemplo de uso da Aten��o Seletiva"
*
* @section robot Uso de um CN no Controle de um Rob� Aut�nomo
* Um CN pode ser usado para controlar um rob� aut�nomo como mostrado na Figura 17.
* \image html ./images/nc_robot.png "Figura 17: Arquitetura neural de controle de rob�s aut�nomos baseada no CN"
*
* Para se comunicar com o rob� o CN usa seis mem�rias, sendo 3 de entrada e 3 de sa�da. As mem�rias de entrada s�o:
*
* (i) Mem�ria Auditiva - mem�ria de sons captados por um microfone, pr�-processados por algoritmos de interesse;
*
* (ii) Mem�ria Visual - mem�ria de imagens est�reo capturadas por uma c�mera est�reo, pr�-processadas por algoritmos de interesse;
*
* (iii) Mem�ria de Mapas - mem�ria de mapas produzidos por um algoritmo de SLAM.
*
* As mem�rias de sa�da s�o:
*
* (iv) Mem�ria de Imagens - mem�ria de imagens que o CN gostaria de encontrar no campo visual das c�meras do rob�;
*
* (v) Mem�ria Motora - mem�ria de comandos de navega��o a serem enviados do CN para o sistema de navega��o do rob�;
*
* (vi) Mem�ria de Voz - mem�ria de palavras a serem ditas pelo CN.
*
* As mem�rias mencionadas nos itens (i) - (vi) s�o implementadas com Camadas Neurais de RNSP VG-RAM como as empregadas na implementa��o do CN. A entrada das Camadas Neurais que implementam as mem�rias de entrada s�o imagens: (i) representando o som (tipicamente palavras) captado por um microfone; (ii) representando objetos imageados pela c�mera est�reo; e (iii) representando o mapa visto de uma posi��o espec�fica. As sa�das aprendidas por estas Camadas Neurais s�o (i) imagens com textos descrevendo palavras ou outros sons captados pelo microfone, (ii) objetos imageados pela c�mera est�reo, e (iii) posi��es no mapa. Estas sa�das s�o agrupadas em uma �nica imagem pelo m�dulo "Agrega Sensores". As entradas deste m�dulo est�o sujeitas aos mecanismos de aten��o previamente discutidos neste documento.
*
* A entrada das Camadas Neurais que implementam as mem�rias de sa�da s�o tamb�m imagens com textos. Estas imagens, geradas pela Camada Neural Comando do CN, s�o coloridas, e cada mem�ria de sa�da � sens�vel a apenas uma cor. Deste modo, a Camada Neural Comando pode enviar comandos para mais de uma mem�ria de sa�da simultaneamente.
*
* As sa�das aprendidas pelas Camadas Neurais que implementam as mem�rias de sa�da s�o imagens: (iv) com padr�es que definem um objeto espec�fico de interesse imageado pela c�mera est�reo; (v) com padr�es que definem comandos de navega��o, tais como "girar -10�", "andar para frente 30cm", ou "navegar para a posi��o 12, 15"; e (vi) que representam a transformada r�pida de Fourier (FFT) de palavras a serem comunicadas pelo CN a humanos, respectivamente.
*
* @subsection loop Loop Fonol�gico
* O som capturado pelo microfone � filtrado (s�mbolo ) via transformada r�pida de Fourier (FFT). FFTs de sinais de voz feitas a intervalos regulares de cerca de 25ms (usualmente com sobreposi��o de cerca de 15ms), ou espectrogramas, podem ser agrupados ao longo do tempo de uns poucos segundos formando imagens bidimensionais. Estas imagens formam a entrada da Mem�ria Auditiva (�rea de Wernicke \ref <http://en.wikipedia.org/wiki/Wernicke's_area>). O treinamento da Mem�ria Auditiva com pares espectrograma-imagem de palavras constitui um sistema de reconhecimento de voz.
*
* A Mem�ria Auditiva � a �nica ligada � Entrada do CN (ver Figura 17) e esta (a Entrada do CN) � tamb�m uma Camada Neural, que possui arquitetura de interconex�es igual � dos outros n�veis hier�rquicos do CN. A arquitetura de interconex�es da Entrada do CN permite � mesma manter uma mem�ria do programa verbal requisitado via microfone e equivaleria ao Loop Fonol�gico da mente humana (pag. 168 do livro - circuito fonol�gico).
*
* � importante notar que a Mem�ria Auditiva � tamb�m ligada � entrada Sensores do CN. Esta liga��o � importante para permitir a influ�ncia de comandos de voz na execu��o de programas durante a execu��o dos mesmos.
*
* @subsection what Caminho Visual "O Que (What)"
* A Mem�ria Visual (Figura 17) busca emular o caminho What existente na mente humana (\ref <http://en.wikipedia.org/wiki/Two_Streams_hypothesis>). Ela � tamb�m, na verdade, um sistema de reconhecimento de padr�es, que gera como sa�da um padr�o (r�tulo) aprendido previamente para cada imagem est�reo de interesse apresentada em sua entrada. Ou seja, a Mem�ria Visual permite rotular objetos no campo visual do rob�. Ela permite ainda identificar em que medida estes objetos est�o mais a esquerda ou a direita do rob�.
*
* Para identificar a que dist�ncia est� um objeto de interesse e em que medida este objeto de interesse est� mais a esquerda ou a direita do rob�, uma transformada SURF � aplicada � imagem est�reo.
*
* As features SURF mais proeminentes de objetos de interesse s�o usadas para guiar um filtro (s�mbolo ) que leva � entrada da Mem�ria Visual apenas o quadro associado � estas features do objeto de interesse. O �ngulo e a dist�ncia da regi�o da imagem com as features SURF do objeto de interesse s�o codificadas como barras na imagem de sa�da da Mem�ria Visual, que permitem ao CN saber o posicionamento do objeto reconhecido no seu campo visual.
*
* @subsection sacade Escolha de Objetos de Interesse (Movimentos Oculares de Sacada)
* A escolha do objeto de interesse � feita pelo CN por meio de imagens de sa�da disponibilizadas em sua Camada Neural Comando contendo texto descrevendo o objeto de interesse, que s�o reconhecidas pela Mem�ria de Imagens.  A Mem�ria de Imagens � treinada com pares imagem com texto descrevendo objetos de interesse-c�digos dos objetos de interesse. Todos os neur�nios da Mem�ria de Imagens aprendem o mesmo c�digo para um dado texto descrevendo um objeto de interesse; este c�digo � usado como um �ndice para uma lista de objetos de interesse que o filtro de entrada da Mem�ria Visual (s�mbolo ) conhece.
*
* @subsection where Caminho Visual "Onde (Where)"
* O mapa usado como entrada da Mem�ria do Mapa (Figura 17) � um mapa do tipo grid 2D centrado no rob�; isto �, o rob� fica sempre no centro deste mapa e apontando para frente, e o mundo � que se move com rela��o ao rob�. Este mapa representa o caminho Where existente na mente humana (\ref <http://en.wikipedia.org/wiki/Two_Streams_hypothesis>).
*
* A Mem�ria do Mapa �, na verdade, um sistema de reconhecimento de padr�es, que gera como sa�da um padr�o (r�tulo) aprendido previamente para cada imagem de interesse apresentada em sua entrada. Ou seja, a Mem�ria do Mapa permite rotular posi��es do mapa.
*
* @subsection motor Controle do Sistema de Navega��o do Rob� (C�rtex Motor)
* A Mem�ria Motora � treinada com pares imagem com texto descrevendo comandos motores-c�digos dos comandos motores. Todos os neur�nios da Mem�ria Motora aprendem o mesmo c�digo para um dado texto descrevendo um comando; este c�digo � usado como um �ndice para uma lista de comandos de interesse (s�mbolo ).
*
* @subsection voice Gera��o de Mensagens de Voz (�rea de Broca)
* A Mem�ria de Voz � treinada com pares imagem com texto descrevendo palavras a serem pronunciadas pelo CN-espectrogramas e representa a �rea de Broca do c�rebro (\ref <http://en.wikipedia.org/wiki/Broca's_area>). Os espectrogramas aprendidos pela Mem�ria de Voz s�o transformados em som por um filtro que implementa a transformada inversa de Fourier (s�mbolo ).
*
* @section homework Descri��o do Trabalho
* O trabalho consiste na implementa��o da arquitetura neural de controle de rob�s aut�nomos baseada no CN mostrada na Figura 17 usando a MAE. O trabalho deve ser feito segundo as etapas listadas abaixo:
* -# Implementa��o e teste do CN usando o programa exemplo "Contar de 0 a 9 tr�s vezes" apresentado acima.
* -# Implementa��o da Mem�ria do Mapa e da Mem�ria Motora e sistemas associados. Interliga��o da Mem�ria do Mapa e da Mem�ria Motora ao software de controle do rob� pioneer e avalia��o do desempenho do CN na proceduraliza��o de movimentos do rob�.
* -# Implementa��o da Mem�ria Visual e da Mem�ria de Imagens e sistemas associados, interliga��o das mesmas ao software do rob� Pioneer, e avalia��o do desempenho do CN na proceduraliza��o de movimentos do rob� que requeiram sensoriamento de imagens.
* 	- fluxo robot |=> NC |=> robot : IMAGEM |=> mem�ria visual => IMAGEM => mem�ria de imagens |=> R�TULO
* 	- ser� adotado um Middleware conhecido como Mule ESB para fazer a interface entre o robot e o NC.
* 	- IDA: do lado do NC haver� um servidor socket n�o bloqueante rodando numa thread separada para n�o afetar a execu��o do NC.
* 	- IDA: do lado do robot haver� um c�digo para consumir um servi�o do Mule ESB que receber� uma imagem e a enviar� no tamanho esperado pela rede via socket para o NC.
* 	- VOLTA: do lado do NC haver� um cliente socket para enviar o r�tulo da mem�ria de imagem para o Mule ESB
* 	- VOLTA: do lado do robot haver� um servi�o para receber o r�tulo repassado pelo Mule ESB.
*
* -# Implementa��o da Mem�ria Auditiva e da Mem�ria de Voz e sistemas associados, e avalia��o do desempenho do CN na proceduraliza��o de movimentos do rob� que requeiram comunica��o via voz.
*
* Segue abaixo o desenho esquem�tico da implementa��o da hierarquia do computador neural, que est� descrita no arquivo nc.con.
* \image html ./images/nc_network.png "Figura 18: Arquitetura descrita no arquivo nc.con"
*
* @section conclusion Conclus�o
* O treinamento do CN ter� que ser feito manualmente. Contudo, o ideal seria que ele ocorresse automaticamente. Mas como? Eu conjecturo que existe um n�vel hier�rquico acima do CN que seria total ou em parte o Hipocampo (\ref <http://en.wikipedia.org/wiki/Hippocampus>).
*
* <hr>
* @section notes release.notes
* release.notes
* <hr>
* @section requirements requirements
* @verbinclude requirements
* <hr> 
* @todo [optionally include text about more work to be done]
* @todo Give each todo item its own line
*
*/ 
