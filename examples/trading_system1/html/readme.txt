Para limpar e trading system prepara-lo para teste:
./clear_trading_system.sh

Para rodar o treino off-line das redes neurais:
./run_wnn_train.sh 2010-10-?(18|19|20) TT_new wnn_pred_train.cml

Para para gerar as predicoes on-line:
./run_wnn_predict.sh 2010-10-?(18|19|20|21) TT_new wnn_pred_predict.cml

============================================================================
Correntemente, o trading_system está configurado (via wnn_pred.con) para
calibrar (obter estatísticas sobre os preditores) com dois dias e
trade por um dia. 

Assim, na preparação para iniciar um novo dia no mercado temos que
fazer predições para os dois dias anteriores. Para fazer predições
precisamos de pelo menos um dia de treino. Logo, precisamos de
3 dias passados para iniciar um dia no mercado.
============================================================================

Para iniciar um novo dia no mercado (supondo dia 2010-10-21):
0. Inicie a máquina virtual Windows e entre no site do Homebroker
1. Gere os arquivos de cotações do dia anterior no formato do sistema
1.1 Vá para o diretório trading_system1
1.2 Apague o conteúdo do diretório INTRADAY_XML e execute o comando abaixo para pagar os dados do dia anterior
1.3 scp -r alberto@lcad1.lcad.inf.ufes.br:/home/aalmeida/cron-uol-intraday/dataday/2010-10-20 ~/MAE/examples/trading_system1/INTRADAY_XML
1.4 Execute o comando abaixo para converter os arquivos xml para o formato do trading system
1.4.1 As cotações até o dia 2010-10-15 tem início as 10h e fim às 17h;
1.4.2 As cotações a partir do dia 2010-10-18 tem início as 11h e fim às 18h;
1.4.3 Como as cotações copiadas possuem início/fim diferentes é necessário rodar a conversão duas vezes.
1.5 ./converte_xml.sh INTRADAY_XML 10 17 (Na pasta INTRADAY_XML só pode ter os dados com inicio 10h e fim 17h)
1.6 ./converte_xml.sh INTRADAY_XML 11 18 (Na pasta INTRADAY_XML só pode ter os dados com inicio 11h e fim 18h)
1.6 Copie os arquivos com as cotações do dia anterior para o diretório html como abaixo
1.7 cp -r TXT_INTRADAY_XML/2010-10-20 html/TXT_INTRADAY_XML
2. Vá para o diretório html
3. No diretorio TXT_INTRADAY_XML/ crie um diretório para o dia corrente como abaixo
3.10 mkdir TXT_INTRADAY_XML/2010-10-21
4. Altere a linha "rm -f TXT_INTRADAY_XML/2010-10-21/*" do arquivo clear_trading_system.sh para o dia corrente
5. Repita o passo acima para o arquivo clear_trading_system_leave_memory.sh
6. Altere a linha "./run_wnn_train.sh 2010-10-?(18|19|20) TT_new wnn_pred_train.cml" do arquivo 
   clear_trading_system.sh para o dia corrente (note que o ultimo dia é o dia anterior)
7. Altere a linha "$date_to_run = "2010-10-21";" do arquivo trading_system.php para o dia corrente
8. Altere a linha "$just_collecting_data = true;" do arquivo trading_system.php para false
8. Altere a linha "$str_command = './run_pred_trad.sh "2010-10-?(18|19|20|21)" TT_new wnn_pred_predict.cml';" do arquivo 
   state_management.php para o dia corrente (note que o ultimo dia é o dia corrente)
10. Rode o comando abaixo para criar o conhecimento inicial do sistema:
10.1 ./clear_trading_system.sh
11. Inicie o apache como root com o comando abaixo:
11.1 /etc/init.d/httpd start
12. No windows, na pagina do browser com o trading system, tecle F5 para pegar a ultima versão do código
13. Inicie o script AutoHotkey

============================================
Hot Restart!
============================================
(1) Para utilzar o hot restart é só passar o dia atual como parâmetro.
./hot_restart.sh 2010-10-21





