(1) A ideia � tornar o week_return da quarta (i) igual ao maior retorno 
observado na pr�pria quarta ou na ter�a, segunda e sexta passados, se 
comparados � m�xima conta��o da quinta passada (retorno observado entre 
a m�xima cota��o de ter�a, segunda e sexta passados, e a m�xima da 
quinta passada). (1a) Podemos usar como refer�ncia a avg de quinta tamb�m
(mais otimista).

A rede aprenderia este week_return usando como entrada, inicialmente,
os retornos das �ltimas 4 quartas (i-1 a i-4), computados como acima.

(2) O pr�ximo experimento seria usar o mesmo week_return, mas usar como 
entrada da rede o retorno da quarta (i-1), ter�a, segunda e sexta da 
semana anterior (i-1) e da anterior a esta (i-2, no caso da sexta); 
estes retornos seriam computados usando como refer�ncia a quinta da 
semana (i-2). Estes retornos podem ser computados usanda a m�xima
cota��o de cada dia, ou (2a) a cota��o m�dia, ou (2c) o fechamento.
(2c+d) Como em (1a), podemos usar como refer�ncia no c�mputo do retorno a
ser aprendido a avg de quinta tamb�m (mais otimista).

-------------------------------------------
## Nova ideia ##

A rede (os neuronios) aprende a reconhecer a semana, isto ee, os neuronios
memorizam o numero identificador da semana, dada a entrada da rede.

(A) Para avaliar uma saida da rede relativa a uma accao em uma dada semana no 
passado, onde temos dados passados e futuros relativos a esta semana, nos 
aplicamos uma heuristica que avalia quao boa foi uma semana, dados criterios 
de compra e venda de accoes. Isso mede o desempenho de nosso preditor para
esta accao nesta semana.

Podemos avaliar um preditor para uma accao em um dado periodo de varias 
semananas usando o precedimento (A), acima, para todas as semanas do periodo.

(B) Em uma dada semana no passado, podemos avaliar qual ee a melhor prediccao, dado 
um conjunto de accoes, usando o procedimento (A) para avaliar o preditor
de cada accao nesta semana.

O desafio ee descobrir qual ee a melhor prediccao em uma semana para a qual 
nao sabemos o futuro. O que leva um preditor a ser bom na avaliaccao (B)?

Uma alternativa para a questao acima ee dar um ponto para o preditor toda vez que
ele for bem avaliado, e tirar um ponto toda vez que ele for mal avaliado.

+++++++++++++++++++++++++++++++++++++++++++
Para transferir os arquivos da Enfoque:
1. Rode o script que os transfere para o Linux. Eles vao para o diretorio:
/home/freitas/DSC/TOOLS/PWBROOT/DATA
2. No diretorio /home/freitas/DSC/TOOLS/PWBROOT digite
> tar cvzf data.tgz DATA
3. Copie o arquivo data.tgz para o diretorio do exemplo wnn_pred_price da MAE.
Neste diretorio, digite:
> rm -rf DATA
> tar xzvf data.tgz

Para rodar as predicoes:
1. Altere TARGET_DAY_WEEK em wnn_pred.con se necessario, recompile e
digite:
> rm TT_new/*
> ./run_all.sh DATA TT_new

Para produzir os resultados:
> ./average_signal_match.sh

Para plotar (altere o nome do ativo em saida.gp):
> gnuplot -persist saida.gp

Para rodar o trading system:
> TRADING_SYSTEM/trading_system TT_new/*

Para saber os melhores retornos (lembre-se de alterar a data entre aspas):
> LC_ALL=C; export LC_ALL; grep -H "2009-02-04" TT_new/* | sort --key=5 -n 
