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

Transferir dados intraday para o preditor:
1. Copiar os arquivos do diretório /home/aalmeida/cron-uol-intraday/dataday para ~/MAE/examples/trading_system1/INTRADAY_XML
> cp -r /home/aalmeida/cron-uol-intraday/dataday/* ~/MAE/examples/trading_system1/INTRADAY_XML
2. Remover os dados antigos
> rm -rf TXT_INTRADAY_XML
3. Criar novamente o diretório TXT_INTRADAY_XML
> mkdir TXT_INTRADAY_XML
4. Rodar o script que convert as cotações de XML para TXT.
Parametros:
# $1 = diretorio com os aquivos XML (todos devem ter aquivos com hora de inicio e fim iguais);
# $2 = hora de inicio do pregao. Ex.: 10 ou 11
# $3 = hora de fim do pregao. Ex.: 17 ou 18

> converte_xml.sh INTRADAY_XML 11 18

Para rodar as predicoes:
1. Altere TARGET_DAY_WEEK em wnn_pred.con se necessario, recompile e
digite:
> rm TT_new/*
> shopt -s extglob
> ./run_all.sh 2010-09-?(14|15|16|17) TT_new

Para produzir os resultados:
> ./average_signal_match.sh

Para plotar:
> ./gnuplot.sh

Para rodar o trading system:
> TRADING_SYSTEM/trading_system TT_new/*

Para saber os melhores retornos (lembre-se de alterar a data entre aspas):
> LC_ALL=C; export LC_ALL; grep -H "2009-02-04" TT_new/* | sort --key=5 -n 


Para avaliar o trading system no mes de setembro de 2010
> gera_dados_treinamento.sh (se precisar (não existirem os diretorios TT_new-*))
> ./avalia_trading_system.sh | bc bc_prog.bc (resultado final acumulado para um investimento inicial de 40000)

Melhor (mais que 30M de volume minimo em um dia):
./avalia_trading_system2.sh "?(TCSL4*|JBSS3*|LREN3*|PCAR5*|ECOD3*|GOAU4*|NATU3*|LAME4*|RDCD3*|MRVE3*|FIBR3*|GFSA3*|PDGR3*|CYRE3*|ITSA4*|CSNA3*|AMBV4*|USIM5*|BVMF3*|BBAS3*|GGBR4*|OGXP3*|PETR3*|VALE3*|BBDC4*|ITUB4*|VALE5*|PETR4*)" | bc bc_prog.bc 
100069.56

Talvez mais prodente (mais que 20M de volume minimo em um dia):
./avalia_trading_system2.sh "?(RSID3*|TNLP4*|BRFS3*|CMIG4*|ELPL6*|MMXM3*|BRAP4*|CSAN3*|ELET3*|TCSL4*|JBSS3*|LREN3*|PCAR5*|ECOD3*|GOAU4*|NATU3*|LAME4*|RDCD3*|MRVE3*|FIBR3*|GFSA3*|PDGR3*|CYRE3*|ITSA4*|CSNA3*|AMBV4*|USIM5*|BVMF3*|BBAS3*|GGBR4*|OGXP3*|PETR3*|VALE3*|BBDC4*|ITUB4*|VALE5*|PETR4*)" | bc bc_prog.bc 
96430.16


Para avaliar quantas amostras são necessárias em média para se verificar um retorno; uma ação:
php test_sample_group_size.php TXT_INTRADAY_XML/2010-10-21/JBSS3.SA.xml.txt 0.001

Para avaliar quantas amostras são necessárias em média para se verificar um retorno; várias ações:
./test_sample_group_size.sh 2010-10-25 0.003 | sort -n --key=2
