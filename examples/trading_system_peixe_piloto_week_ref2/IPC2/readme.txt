A nova versao do trading_system ee chamada de trading_system_peixe_piloto_week_ref2.
Para baixa-la execute:
- cd MAE/examples
- svn up trading_system_peixe_piloto_week_ref2

Ela tem que ser rodada de dentro do diretorio IPC2 ao inves de de dentro de 
seu diretorio pai (MAE/examples/trading_system_peixe_piloto_week_ref2), e
vem do svn sem os links simbolicos para os diretorios de dados. Assim, execute:
- cd MAE/examples/trading_system_peixe_piloto_week_ref2/IPC2
- ln -s ../../dados/DATA .
- ln -s ../../dados/DADOS_PEIXE_PILOTO .

Para compila-la certifique-se de que o Makefile esta configurado corretamente
verificando se as linhas abaixo estao como abaixo:
#IPC_FLAGS =
#IPC_FLAGS = -DUSE_IPC -DIPC_TEST
IPC_FLAGS = -DUSE_IPC -DIPC_ON_LINE


A nova versao do trading_system tem uma interface linha de comando que deve ser
usada. Quando executado sem parâmetros ele informa:
- cd MAE/examples/trading_system_peixe_piloto_week_ref2/IPC2
- ./trading_system
- Erro: Missing input parameters.
-  Usage: trading_system <stock_list.txt> <reference> <yyyy-mm-dd yyyy-mm-dd>
-  Example: ./trading_system ../acoes_de_interesse.txt PETR3 2010-01-07 2010-01-20

Para invoca-lo devem ser passados um arquivo com a lista de acoes de interesse,
uma acao de referencia (esta acao sempre sera comprada ou vendida na semana),
e uma faixa de datas. 

A faixa de datas indica o dia inicial e final de operacao do trading_system. 
A data inicial tem que ser uma quinta e a final a quarta seguinte.
