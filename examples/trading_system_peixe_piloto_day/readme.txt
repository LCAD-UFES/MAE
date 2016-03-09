(1) A ideia é tornar o week_return da quarta (i) igual ao maior retorno 
observado na própria quarta ou na terça, segunda e sexta passados, se 
comparados à máxima contação da quinta passada (retorno observado entre 
a máxima cotação de terça, segunda e sexta passados, e a máxima da 
quinta passada). (1a) Podemos usar como referência a avg de quinta também
(mais otimista).

A rede aprenderia este week_return usando como entrada, inicialmente,
os retornos das últimas 4 quartas (i-1 a i-4), computados como acima.

(2) O próximo experimento seria usar o mesmo week_return, mas usar como 
entrada da rede o retorno da quarta (i-1), terça, segunda e sexta da 
semana anterior (i-1) e da anterior a esta (i-2, no caso da sexta); 
estes retornos seriam computados usando como referência a quinta da 
semana (i-2). Estes retornos podem ser computados usanda a máxima
cotação de cada dia, ou (2a) a cotação média, ou (2c) o fechamento.
(2c+d) Como em (1a), podemos usar como referência no cômputo do retorno a
ser aprendido a avg de quinta também (mais otimista).

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
> ./run_all.sh DATA TT_new

Para produzir os resultados:
> ./average_signal_match.sh

Para plotar (altere o nome do ativo em saida.gp):
> gnuplot -persist saida.gp

Para rodar o trading system:
> TRADING_SYSTEM/trading_system TT_new/*

Para saber os melhores retornos (lembre-se de alterar a data entre aspas):
> LC_ALL=C; export LC_ALL; grep -H "2009-02-04" TT_new/* | sort --key=5 -n 


novo mecanismo de teste:
shopt -s extglob
TRADING_SYSTEM/trading_system TT_new/?(PETR4*|VALE5*|PETR3*|ITSA4*|TCSL4*|EMBR3*|USIM5*|ITUB4*|BBAS3*|GGBR4*|BBDC4*|VALE3*|CSNA3*|BRFS3*|RSID3*|LAME4*|POMO4*|KLBN4*|BRAP4*|GOLL4*|AMBV4*|TCSL3*|GOAU4*|LIGT3*|CMIG4*|FIBR3*|BRKM5*|ELET3*|ELET6*|PLAS3*|TNLP4*|PSSA3*|RAPT4*|TBLE3*|BRTO4*|CNFB4*|LREN3*|CCRO3*|TAMM4*)

# **** capital inicial: R$40000.00  capital final: R$69697.55  retorno acumulado:74.84%
[alberto@cachoeiro trading_system_peixe_piloto_week]$ TRADING_SYSTEM/trading_system TT_new/?(PETR4*|VALE5*|PETR3*|ITSA4*|TCSL4*|EMBR3*|USIM5*|ITUB4*|BBAS3*|GGBR4*|BBDC4*|VALE3*|CSNA3*|BRFS3*|LAME4*|POMO4*|KLBN4*|BRAP4*|GOLL4*|AMBV4*|TCSL3*|GOAU4*|LIGT3*|CMIG4*|FIBR3*|BRKM5*|ELET3*|ELET6*|PLAS3*|TNLP4*|PSSA3*|RAPT4*|TBLE3*|BRTO4*|CNFB4*|LREN3*|CCRO3*)


IPC2/trading_system TT_new_all_weeks/?(PETR4*|VALE5*|PETR3*|ITSA4*|TCSL4*|EMBR3*|USIM5*|ITUB4*|BBAS3*|GGBR4*|BBDC4*|VALE3*|CSNA3*|BRFS3*|LAME4*|POMO4*|KLBN4*|BRAP4*|GOLL4*|AMBV4*|TCSL3*|GOAU4*|LIGT3*|CMIG4*|FIBR3*|BRKM5*|ELET3*|ELET6*|PLAS3*|TNLP4*|PSSA3*|RAPT4*|TBLE3*|BRTO4*|CNFB4*|LREN3*|CCRO3*)

2010-12-22 Wed 11:15 - WAIT_SELL: sell order exec.,	GGBR4 price = R$23.04, order price = R$23.04, sell price = R$23.04, quant. = 3600
2010-12-22 Wed 11:15 - WAIT_SELL: current capital, 	R$83217.07
2010-12-22 Wed 17:59 - QUIT: Final Capital		R$83217.07  retorno acumulado: 108.04%

Ver graficos:
gnuplot GRAFICOS/*.gnu*

Para gerar dados de buy e sell diarios:
- Compilar o trading system IPC2 mundando o Makefile de acordo
./compute_buy_sell_prices.sh
